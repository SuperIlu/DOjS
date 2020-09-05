/*!\file pcdbug.c
 *
 * Watt-32 protocol debugger.
 * Writes to `debug.file' specified in config-file.
 * File may be stdout/stderr/nul.
 *
 * Most functions are prefixed with `dbug_'. And variables
 * with `dbg_'.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <setjmp.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <arpa/nameser.h>
#include <resolv.h>

#if defined(__DJGPP__) && (DJGPP_MINOR >= 4)
  #include <sys/xdevices.h>
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
  #include <fcntl.h>

  #if !defined(__POCC__)
    #include <share.h>
  #endif
#endif

#include "wattcp.h"
#include "strings.h"
#include "pcdns.h"
#include "misc.h"
#include "run.h"
#include "timer.h"
#include "sock_ini.h"
#include "chksum.h"
#include "ip4_in.h"
#include "ip6_in.h"
#include "ip6_out.h"
#include "pctcp.h"
#include "pcsed.h"
#include "pcarp.h"
#include "pcqueue.h"
#include "pcpkt.h"
#include "pcstat.h"
#include "pcicmp.h"
#include "pcicmp6.h"
#include "pcconfig.h"
#include "netaddr.h"
#include "language.h"
#include "printk.h"
#include "gettod.h"
#include "get_xby.h"
#include "pppoe.h"
#include "ppp.h"
#include "split.h"
#include "bsddbug.h"
#include "pcdbug.h"

#if defined(USE_GZIP)
  #include "zlib.h"
#endif

#if defined(USE_SCTP)
  #include "sctp.h"
#endif

#define DEBUG_RTP 1       /**< \todo include detailed RTP debugging */

const char *tcpStateName (UINT state)
{
  static char  buf[20];
  static const char *tcpStates[] = {
                    "LISTEN",   "RESOLVE", "SYNSENT", "SYNREC", "ESTAB",
                    "ESTCLOSE", "FINWT1",  "FINWT2",  "CLOSWT", "CLOSING",
                    "LASTACK",  "TIMEWT",  "CLOSED"
                  };

  if (state < DIM(tcpStates))
     return tcpStates [(int)state];
  return itoa (state, buf, 10);
}

DebugProc debug_xmit = NULL;
DebugProc debug_recv = NULL;

#if !defined(USE_DEBUG)

void dbug_init (void)
{
  outsnl ("Debug-mode disabled");
}

#else  /* rest of file */

static void  W32_CALL dbug_exit  (void);
static void           dbug_close (void);

static void  dump_ip_opt (const void *opt, int len);
static void  dump_tcp_opt(const void *opt, int len, DWORD ack);
static DWORD dump_data   (const void *data, UINT datalen);
static int   udp_dump    (const _udp_Socket *sock, const in_Header *ip);
static int   tcp_dump    (const _tcp_Socket *sock, const in_Header *ip);

static unsigned    dns_dump     (const BYTE *, unsigned, unsigned);
static const BYTE *dns_resource (const BYTE *, const BYTE *, const BYTE *);
static const BYTE *dns_labels   (const BYTE *, const BYTE *, const BYTE *);

static int write_pcap_header (void);
static int write_pcap_packet (const void *pkt, BOOL out);

#if defined(USE_PPPOE)
  static unsigned ccp_dump  (const BYTE *bp);
  static unsigned lcp_dump  (const BYTE *bp);
  static unsigned ipcp_dump (const BYTE *bp);
#endif

#if (DOSX) && !(defined(__CYGWIN__) && defined(__x86_64__)) && \
    !defined(__NO_INLINE__)   /* Don't do this if 'gcc -O0' is used */
  #define PRINT_CPU_INFO
  #define COMPILING_PCDBUG_C
  #define CPU_TEST

  #include "cpumodel.h"
  #include "tests/cpu.c"
  static void print_cpu_info (void);
#endif


static void (W32_CALL *prev_hook) (const char*, const char*) = NULL;

static jmp_buf dbg_jmp;
static char    dbg_name [MAX_PATHLEN+1] = "WATTCP.DBG";
static BOOL    dbg_linebuf = FALSE;
static BOOL    pcap_mode   = FALSE;
static DWORD   now;               /* ticks or milli-sec */
static BOOL    outbound;          /* transmitting packet? */
static BOOL    in_dbug_dump;
static BOOL    use_gzip;          /* if using pcap-mode, use gzip-compression */
static BOOL    use_ods = FALSE;   /* Win32: use OutputDebugString() */
static char    ip4_src [20];
static char    ip4_dst [20];

static union {
       FILE  *stream;
       void  *gz_stream;
     } dbg_file;


/* IP-fragment handling
 */
#define IS_FRAG   0x01      /* is this a fragment? */
#define IS_LAST   0x02      /* is this last fragment? */
#define IS_FIRST  0x04      /* is this first fragment? */

static int frag_status;     /* IPv4 fragment bits */


#if defined(USE_IPV6)
  static char ip6_src [50];
  static char ip6_dst [50];
  static int  udp6_dump (const _udp_Socket *sock, const in6_Header *ip);
#endif

static struct {
       char MAC;
       char ARP;
       char RARP;
       char IP;    /* both IPv4 and IPv6 */
       char BCAST;
       char MCAST;
       char LOOP;
       char NONE;
     } filter = { 1,1,1,1,1,1,0,0 };

static struct {
       char MAC;
       char LLC;
       char ARP;
       char RARP;
       char IP;
       char UDP;
       char TCP;
       char ICMP;
       char IGMP;
       char SCTP;
     } debug = { 0,0,1,1,1,1,1,1,1,1 };

/**
 * These are public so they can be set by application if
 * running without a config-file. \todo make them static.
 */
BOOL dbg_mode_all    = 1;
BOOL dbg_print_stat  = 1;
BOOL dbg_dns_details = 1;

#if defined(USE_GZIP)
static DWORD bytes_raw = 0UL;  /* raw uncompressed bytes written */
#endif


/*----------------------------------------------------------------------*/

BOOL dbug_file (void)
{
  if (pcap_mode)        /* don't let anyone mess with this file */
     return (FALSE);
  return (dbg_file.stream != NULL || use_ods);
}

void dbug_open (void)
{
  const char *fmode = pcap_mode ? "w+b" : "w+t";
  const char *end;

  if (dbg_file.stream || use_ods)  /* Already opened; quit */
     return;

#if defined(__DJGPP__) && (DJGPP_MINOR >= 4) /* Just for testing */
  if (!stricmp(dbg_name,"/dev/zero"))
     __install_dev_zero();

  else if (!stricmp(dbg_name,"/dev/full"))
     __install_dev_full();
#endif

  end = strchr (dbg_name, '\0');
  use_gzip = (end && !stricmp(end-3,".gz"));

  if (!stricmp(dbg_name,"con") || !stricmp(dbg_name,"stdout"))
     dbg_file.stream = stdout;

  else if (!stricmp(dbg_name,"stderr"))
     dbg_file.stream = stderr;

#if defined(_WIN32)
  else if (!stricmp(dbg_name,"$ods"))
  {
    use_ods = TRUE;
    goto quit;
  }
#endif

#if defined(USE_GZIP)
  else if (pcap_mode && use_gzip)
  {
#if defined(_WIN32) && !defined(__WATCOMC__) && !defined(__CYGWIN__)
    int fd = _sopen (dbg_name, _O_CREAT | _O_TRUNC | _O_WRONLY,
                     SH_DENYWR, S_IREAD | S_IWRITE);
    if (fd > -1)
       dbg_file.gz_stream = gzdopen (fd, "w+b2");   /* compression level 2 */
#else
    errno = 0;
    dbg_file.gz_stream = gzopen (dbg_name, "w+b2"); /* compression level 2 */
#endif
  }
#endif  /* USE_GZIP */
  else
  {
    dbg_file.stream = fopen_excl (dbg_name, fmode);
    use_gzip = FALSE;
  }

  if (!dbg_file.stream)
  {
    (*_printf) (_LANG("ERROR: unable to open debug file %s; %s\n"),
                dbg_name, strerror(errno));
    errno = 0;  /* clear it. It coult confuse callers */
    return;
  }

  if (pcap_mode)
  {
    if (dbg_file.stream == stdout || dbg_file.stream == stderr)
    {
      outsnl (_LANG("ERROR: cannot debug to screen with \"DEBUG.PCAP=1\""));
      dbug_close();
      return;
    }
    if (write_pcap_header() < 0)
    {
      (*_printf) (_LANG("ERROR: Failed to write pcap header; %s\n"),
                  strerror(errno));
      dbug_close();
      return;
    }
  }
#if !defined(__BORLANDC__)  /* buggy output with this */
  else if (dbg_linebuf)
  {
    static char buf[256];
    setvbuf (dbg_file.stream, buf, _IOLBF, sizeof(buf));
  }
#endif

#if defined(_WIN32)
  quit:
#endif

  RUNDOWN_ADD (dbug_exit, 5);
}

/*
 * Return TRUE if MAC address is multicast (LSB bit 0 set).
 */
static __inline BOOL MAC_is_mcast (const void *addr)
{
  return ((*(const BYTE*)addr & 1) == 1);
}

/*
 * Return TRUE if MAC address is broadcast.
 */
static __inline BOOL MAC_is_bcast (const void *addr)
{
  return (!memcmp(addr, _eth_brdcast, _eth_mac_len));
}

/*
 * Using NDIS3PKT causes all non-broadcast packets sent to
 * appear in receive buffer approx 100 msec later.
 * WinPcap: Also if _pkt_rxmode == RXMODE_ALL_LOCAL.
 *
 * To avoid confusion, we print a warning in this case.
 */
static __inline BOOL is_looped (const in_Header *ip)
{
  if (outbound || _pktserial)
     return (FALSE);

  if (memcmp(MAC_SRC(ip), _eth_addr, _eth_mac_len))
     return (FALSE);

  if (MAC_TYP(ip) != IP4_TYPE) /* assume looped if not IPv4 */
     return (TRUE);

  /* if MAC-src matches, but IP-src is different it could be
   * Winsock sending. Thus it's not a looped packet.
   */
  return (ntohl(ip->source) == my_ip_addr);
}

/*
 * Return TRUE if MAC destination address of received/sent link-layer
 * packet:
 *  - matches our link-layer address.
 *  - is broadcast and we don't filter broadcast.
 *  - is multicast and we don't filter multicast.
 */
static __inline BOOL match_link_destination (const void *addr)
{
  if (!memcmp(addr, _eth_addr, _eth_mac_len) ||
      (!filter.BCAST && MAC_is_bcast(addr)) ||
      (!filter.MCAST && MAC_is_mcast(addr)))
     return (TRUE);
  return (FALSE);
}

/*
 * Return TRUE if destination address of received/sent ARP packet:
 *  - matches our ether/IP-address.
 *  - is broadcast and we don't filter broadcast
 *    (ARP/RARP packets should never use multicast in it's header).
 */
static __inline BOOL match_arp_rarp (const arp_Header *arp)
{
  if (!memcmp(&arp->dstEthAddr, _eth_addr, _eth_mac_len) ||
      _ip4_is_local_addr(intel(arp->dstIPAddr)))
     return (TRUE);

  if (!filter.BCAST && MAC_is_bcast(&arp->dstEthAddr))
     return (TRUE);
  return (FALSE);
}

/*
 * Return TRUE if destination address of received/sent IP packet:
 *  - matches our IP-address.
 *  - is broadcast and we don't filter (directed) IP-broadcast.
 */
static __inline BOOL match_ip4_dest (const in_Header *ip)
{
  DWORD destin = intel (ip->destination);

  if (_ip4_is_local_addr(destin) || (!filter.BCAST && _ip4_is_ip_brdcast(ip)))
     return (TRUE);
  return (FALSE);
}

#if defined(USE_IPV6)
static __inline BOOL match_ip6_dest (const in6_Header *ip)
{
  const void *dest = &ip->destination;

  if (_ip6_is_local_addr(dest) ||
      IN6_IS_ADDR_MC_GLOBAL(dest))   /** \todo needs work */
     return (TRUE);
  return (FALSE);
}
#endif

/*
 * Return checksum and print "ok" or "ERROR"
 */
static __inline const char *do_check_sum (WORD value, const void *p, int len)
{
  static char buf[20];
  WORD   chk = CHECKSUM (p, len);

  sprintf (buf, "%04X (%s)", value, (chk == 0xFFFF) ? "ok" : "ERROR");
  return (buf);
}

/*
 * Return name of some known link-layer protocols.
 */
static __inline const char *link_protocol (WORD type)
{
  switch (type)
  {
    case IP4_TYPE:
         return ("IP");
    case IP6_TYPE:
         return ("IP6");
    case ARP_TYPE:
         return ("ARP");
    case RARP_TYPE:
         return ("RARP");
    case IEEE802_1Q_TYPE:
         return ("VLAN");
    case PPPOE_DISC_TYPE:
         return ("PPPoE DISCOVERY");
    case PPPOE_SESS_TYPE:
         return ("PPPoE SESSION");
    default:
         return ("unknown");
  }
}

/*
 * Return name of known IP-protocols.
 */
static __inline const char *ip4_protocol (BYTE prot)
{
  static char buf[4];

  switch (prot)
  {
    case UDP_PROTO:
         return ("UDP");
    case TCP_PROTO:
         return ("TCP");
    case ICMP_PROTO:
         return ("ICMP");
    case IGMP_PROTO:
         return ("IGMP");
    case SCTP_PROTO:
         return ("SCTP");
  }
  return itoa (prot, buf, 10);
}

/*
 * Return name for IP's "Type Of Service".
 */
static const char *type_of_service (BYTE tos)
{
  static char buf[30];
  char  *p = buf;

  if (tos == 0)
     return (" 0");

  *p = '\0';
  if (tos & IP_MINCOST)
  {
    strcat (buf, " Mcost");
    tos &= ~IP_MINCOST;
  }
  if (tos & IP_RELIABILITY)
  {
    strcat (buf, " Rel");
    tos &= ~IP_RELIABILITY;
  }
  if (tos & IP_THROUGHPUT)
  {
    strcat (buf, " ThPut");
    tos &= ~IP_THROUGHPUT;
  }
  if (tos & IP_LOWDELAY)
  {
    strcat (buf, " LwDly");
    tos &= ~IP_LOWDELAY;
  }
  p = strchr (buf, '\0');
  if (tos)
     sprintf (p, " %02X", tos);
  return (buf);
}

/*
 * Format time for "Round Trip Time" in msec.
 */
static __inline const char *RTT_str (DWORD rtt, DWORD now)
{
  static char buf[40];

  if (rtt == 0UL || rtt < now)
     return ("--");      /* RTT timer stopped */

  sprintf (buf, "%ld", get_timediff (rtt, now));
  return (buf);
}


/*
 * Return string for ARP/RARP opcodes.
 */
static __inline const char *arp_opcode (WORD code)
{
  if (code == ARP_REQUEST || code == RARP_REQUEST)
     return ("Request");

  if (code == ARP_REPLY || code == RARP_REPLY)
     return ("Reply");

  return ("? op");
}

/*
 * Print IP source/destination addresses and ports;
 *   "host1 (a) -> host2 (b)"
 *
 * Note: the udp_Header src/dst port field are not printed.
 */
static void dump_addr_port (const char      *proto,
                            const void      *sock,
                            const in_Header *ip)
{
  const tcp_Header *tcp = (const tcp_Header*) ((BYTE*)ip + in_GetHdrLen(ip));

  if (!sock)
     dbug_printf ("%s:   NO SOCKET: %s (%d) -> %s (%d)\n", proto,
                  ip4_src, intel16(tcp->srcPort),
                  ip4_dst, intel16(tcp->dstPort));
  else
  {
    const _tcp_Socket *sk = (const _tcp_Socket*) sock;

    if (outbound)
         dbug_printf ("%s:   %s (%d) -> %s (%d), sock %" ADDR_FMT "\n", proto,
                      _inet_ntoa(NULL,my_ip_addr),  sk->myport,
                      _inet_ntoa(NULL,sk->hisaddr), sk->hisport,
                      ADDR_CAST(sock));

    else dbug_printf ("%s:   %s (%d) -> %s (%d), sock %" ADDR_FMT "\n", proto,
                      _inet_ntoa(NULL,sk->hisaddr), sk->hisport,
                      _inet_ntoa(NULL,my_ip_addr),  sk->myport,
                      ADDR_CAST(sock));
  }
}

#if defined(USE_IPV6)
static __inline void ip6_addr_type (char *ret, const void *addr)
{
  if (IN6_IS_ADDR_UNSPECIFIED(addr))
     strcpy (ret, "(anycast) ");

  else if (IN6_IS_ADDR_MULTICAST(addr))
     strcpy (ret, "(multicast) ");

  else if (IN6_IS_ADDR_MC_NODELOCAL(addr))
     strcpy (ret, "(MC node) ");

  else if (IN6_IS_ADDR_MC_LINKLOCAL(addr))
     strcpy (ret, "(MC link) ");

  else if (IN6_IS_ADDR_MC_SITELOCAL(addr))
     strcpy (ret, "(MC site) ");

  else if (IN6_IS_ADDR_MC_GLOBAL(addr))
     strcpy (ret, "(MC global) ");

  else if (IN6_IS_ADDR_V4MAPPED(addr))
     strcpy (ret, "(ip4-mapped) ");

  else if (IN6_IS_ADDR_V4COMPAT(addr))
     strcpy (ret, "(ip4-compat) ");

  else if (IN6_IS_ADDR_LOOPBACK(addr))
     strcpy (ret, "(loopback) ");

  else *ret = '\0';
}

static void dump_addr6_port (const char       *proto,
                             const void       *sock,
                             const in6_Header *ip)
{
  const tcp_Header  *tcp = (const tcp_Header*) (ip + 1);
  const _tcp_Socket *sk  = (const _tcp_Socket*) sock;

  if (!sock)
       dbug_printf ("%s:   NO SOCKET: %s (%d) -> %s (%d)\n", proto,
                    ip6_src, intel16(tcp->srcPort),
                    ip6_dst, intel16(tcp->dstPort));

  else if (outbound)
       dbug_printf ("%s:   %s (%d) -> %s (%d)\n", proto,
                    _inet6_ntoa(&in6addr_my_ip),sk->myport,
                    _inet6_ntoa(sk->his6addr),  sk->hisport);

  else dbug_printf ("%s:   %s (%d) -> %s (%d)\n", proto,
                    _inet6_ntoa(sk->his6addr),  sk->hisport,
                    _inet6_ntoa(&in6addr_my_ip),sk->myport);
}
#endif


/*----------------------------------------------------------------------*/

static void link_head_dump (const union link_Packet *pkt)
{
  const struct eth_Header *eth;
  const char *src_name, *dst_name;
  WORD  type;

  if (_pktdevclass == PDCLASS_TOKEN || _pktdevclass == PDCLASS_TOKEN_RIF)
  {
    const struct tok_Header *tok = &pkt->tok.head;

    dbug_printf ("TR:    destin %s, AC %02X, FC %02X\n"
                 "       source %s, DSAP %02X, SSAP %02X, Ctrl %02X\n"
                 "       type %s (%04X)\n",
                 MAC_address (tok->destination), tok->accessCtrl, tok->frameCtrl,
                 MAC_address (tok->source), tok->DSAP, tok->SSAP, tok->ctrl,
                 link_protocol(tok->type), intel16(tok->type));
    return;
  }

  if (_pktdevclass == PDCLASS_FDDI)
  {
    const struct fddi_Header *fddi = &pkt->fddi.head;

    dbug_printf ("FDDI:  destin %s, FC %02X\n"
                 "       source %s, DSAP %02X, SSAP %02X, Ctrl %02X\n"
                 "       type %s (%04X)\n",
                 MAC_address (fddi->destination), fddi->frameCtrl,
                 MAC_address (fddi->source), fddi->DSAP, fddi->SSAP,
                 fddi->ctrl, link_protocol(fddi->type), intel16(fddi->type));
    return;
  }

  if (_pktdevclass == PDCLASS_ARCNET)
  {
    static const struct search_list arcnet_protos[] = {
         { ARCNET_DP_BOOT,    "DataPoint Boot"   },
         { ARCNET_DP_MOUNT,   "DataPoint Mount"  },
         { ARCNET_PL_BEACON,  "PowerLAN Beacon"  },
         { ARCNET_PL_BEACON2, "PowerLAN Beacon2" },
         { ARCNET_DIAG,       "Diagnose"         },
         { ARCNET_IP6,        "IPv6"             },
         { ARCNET_BACNET,     "BACNET"           },
         { ARCNET_IP_1201,    "IPv4 (RFC-1201)"  },
         { ARCNET_ARP_1201,   "ARP (RFC-1201)"   },
         { ARCNET_RARP_1201,  "RARP (RFC-1201)"  },
         { ARCNET_ATALK,      "AppleTalk"        },
         { ARCNET_ETHER,      "Raw Ether"        }, /* MS LAN-Man/WfWg */
         { ARCNET_NOVELL,     "Novell Encap"     },
         { ARCNET_IP_1051,    "IPv4 (RFC-1051)"  },
         { ARCNET_ARP_1051,   "ARP (RFC-1051)"   },
         { ARCNET_BANYAN,     "Banyan"           },
         { ARCNET_IPX,        "IPX"              },
         { ARCNET_LANSOFT,    "LanSoft"          }
       };
    const struct arcnet_Header *arc = &pkt->arc.head;
    const char  *proto = list_lookup (arc->type, arcnet_protos,
                                      DIM(arcnet_protos));

    dbug_printf ("ARCNET:destin %02X, flags %02X, sequence %04X\n"
                 "       source %02X  %s\n"
                 "       type %s\n",
                 arc->destination, arc->flags, arc->sequence,
                 arc->source, arc->flags == 0xFF ? "Exc-packet" : "",
                 proto);
    return;
  }

  WATT_ASSERT (_pktdevclass == PDCLASS_ETHER);

  eth = &pkt->eth.head;

  /* src/dst_names comes from /etc/ethers file
   */
  src_name = GetEtherName (&eth->source);
  dst_name = GetEtherName (&eth->destination);

  dbug_printf ("ETH:   destin %s %s %s\n"
               "       source %s %s %s\n",
               MAC_address (&eth->destination),
               MAC_is_bcast(&eth->destination) ? "(broadcast)" :
               MAC_is_mcast(&eth->destination) ? "(multicast)" : "",
               dst_name ? dst_name : "",
               MAC_address (&eth->source),
               MAC_is_bcast(&eth->source) ? "(broadcast)" :
               MAC_is_mcast(&eth->source) ? "(multicast)" : "",
               src_name ? src_name : "");

  type = intel16 (eth->type);
  if (type == 0xFFFF)
  {
    dbug_printf ("       Novell IPX\n");
  }
  else if (type < 0x600)      /* type is LLC length field */
  {
    const llc_Header *llc = (const llc_Header*) (eth+1);
    unsigned          len = type;

    dbug_printf ("       IEEE 802.3 encap (LLC: DSAP %02X, SSAP %02X, len %u)\n",
                 llc->DSAP, llc->SSAP, len);
  }
  else
    dbug_printf ("       type %s (%04X)\n", link_protocol(eth->type), type);
}

/*----------------------------------------------------------------------*/

static int arp_dump (const arp_Header *arp)
{
  _inet_ntoa (ip4_src, intel(arp->srcIPAddr));
  _inet_ntoa (ip4_dst, intel(arp->dstIPAddr));

  return dbug_printf ("ARP:   %s (%d), hw %04X, type %04X\n"
                      "       %s (%s) -> %s (%s)\n",
                      arp_opcode(arp->opcode), intel16(arp->opcode),
                      arp->hwType, intel16(arp->protType),
                      MAC_address (&arp->srcEthAddr), ip4_src,
                      MAC_address (&arp->dstEthAddr), ip4_dst);
}

/*----------------------------------------------------------------------*/

static int rarp_dump (const rarp_Header *rarp)
{
  _inet_ntoa (ip4_src, intel(rarp->srcIPAddr));
  _inet_ntoa (ip4_dst, intel(rarp->dstIPAddr));

  return dbug_printf ("RARP:  %s (%d), hw %04X, type %04X\n"
                      "       %s (%s) -> %s (%s)\n",
                      arp_opcode(rarp->opcode), intel16(rarp->opcode),
                      rarp->hwType, intel16(rarp->protType),
                      MAC_address (&rarp->srcEthAddr), ip4_src,
                      MAC_address (&rarp->dstEthAddr), ip4_dst);
}

/*----------------------------------------------------------------------*/

static void ip4_dump (const in_Header *ip)
{
  WORD  ihl, flg;
  DWORD ofs;
  int   opt_len;

  ofs = intel16 (ip->frag_ofs);
  flg = (WORD) (ofs & ~IP_OFFMASK);
  ofs = (ofs & IP_OFFMASK) << 3;

  frag_status = 0;

  if (flg & IP_MF)
  {
    frag_status |= IS_FRAG;

    /* This is 1st fragment. */
    if (ofs == 0)
      frag_status |= IS_FIRST;
  }
  else if (ofs)
          frag_status = (IS_FRAG | IS_LAST);  /* last fragment */

  if (!debug.IP)
     return;

  dbug_printf ("IP4:   %s -> %s\n", ip4_src, ip4_dst);

  ihl = in_GetHdrLen (ip);
  if (ihl < sizeof(*ip))
  {
    dbug_write ("       Bad header\n");
    return;
  }

  dbug_printf ("       IHL %u, ver %u, tos%s, len %u,"
               " ttl %u, prot %s, chksum %s\n"
               "       id %04X, ofs %lu",
               ihl, (BYTE)ip->ver, type_of_service(ip->tos), intel16(ip->length),
               (BYTE)ip->ttl, ip4_protocol (ip->proto),
               do_check_sum (ip->checksum, ip, ihl),
               intel16 (ip->identification), (u_long)ofs);

  if (flg & IP_CE)
     dbug_write (", CE");

  if (flg & IP_DF)
     dbug_write (", DF");

  if (frag_status)
  {
    if (frag_status & IS_FIRST)
       dbug_write (", 1st frag");
    if (frag_status & IS_LAST)
       dbug_write (", last frag");
  }

  dbug_putc ('\n');
  opt_len = ihl - sizeof(*ip);
  if (opt_len > 0)
     dump_ip_opt (ip+1, opt_len);
}

/*----------------------------------------------------------------------*/

static int ip4_orig_dump (const in_Header *this_ip,
                          const in_Header *orig_ip, int icmp_len)
{
  WORD  ihl;
  DWORD ofs;

  if (icmp_len < SIZEOF(*orig_ip))
  {
    dbug_write ("           Too little of original IP-header\n");
    return dump_data (this_ip, intel16(this_ip->length));
  }

  ihl = in_GetHdrLen (orig_ip);
  if (ihl < sizeof(*orig_ip))
  {
    dbug_write ("           Bad orig. header\n");
    return dump_data (this_ip, intel16(this_ip->length));
  }

  dbug_printf ("  Orig IP: %s -> %s\n",
               _inet_ntoa(NULL,intel(orig_ip->source)),
               _inet_ntoa(NULL,intel(orig_ip->destination)));

  ofs = (intel16(orig_ip->frag_ofs) & IP_OFFMASK) << 3;

  dbug_printf ("           IHL %u, ver %u, tos%s, len %u,"
               " ttl %u, prot %s\n"
               "           chksum %s, id %04X, ofs %lu\n",
               ihl, (BYTE)orig_ip->ver, type_of_service(orig_ip->tos),
               intel16(orig_ip->length), (BYTE)orig_ip->ttl,
               ip4_protocol (orig_ip->proto),
               do_check_sum (orig_ip->checksum, orig_ip, ihl),
               intel16 (orig_ip->identification), (u_long)ofs);

  if (icmp_len <= ihl)
     dbug_write ("           No transport header present\n");

  else if (orig_ip->proto == TCP_PROTO && icmp_len - ihl >= 4)
  {
    const tcp_Header *tcp = (const tcp_Header*) ((const BYTE*)orig_ip + ihl);
    dbug_printf ("           TCP: port %u -> %u\n",
                 intel16(tcp->srcPort), intel16(tcp->dstPort));
  }
  else if (orig_ip->proto == UDP_PROTO && icmp_len - ihl >= 4)
  {
    const udp_Header *udp = (const udp_Header*) ((const BYTE*)orig_ip + ihl);
    dbug_printf ("           UDP: port %u -> %u\n",
                 intel16(udp->srcPort), intel16(udp->dstPort));
  }

  if (icmp_len > 0)  /* consider only original IP as payload */
     return dump_data (orig_ip, icmp_len);
  return (1);
}

/*----------------------------------------------------------------------*/

static int icmp4_dump (const in_Header *ip)
{
  WORD  len  = in_GetHdrLen (ip);
  const ICMP_PKT *icmp = (const ICMP_PKT*) ((const BYTE*)ip + len);
  const char     *type_str, *chk_ok;
  char  buf[200] = "";
  char *p = buf;
  int   type, code;

  len  = intel16 (ip->length) - len;   /* ICMP length */
  type = icmp->unused.type;
  code = icmp->unused.code;

  if (len < sizeof(struct ICMP_info))
  {
    dbug_write ("ICMP:  Short header\n");
    return (1);
  }

  /* Simply dump the remaining data if this is a fragment.
   */
  if (frag_status)
     return dump_data (icmp, len);

  switch (type)
  {
    case ICMP_UNREACH:
         type_str = icmp_type_str [ICMP_UNREACH];
         if (code < DIM(icmp_unreach_str))
              sprintf (buf, "%s: %s", type_str, icmp_unreach_str[code]);
         else sprintf (buf, "%s: code %d", type_str, code);
         break;

    case ICMP_TIMXCEED:
         type_str = icmp_type_str [ICMP_TIMXCEED];
         if (code < DIM(icmp_exceed_str))
              p += sprintf (p, "%s: %s", type_str, icmp_exceed_str[code]);
         else p += sprintf (buf, "%s: code %d", type_str, code);
         break;

    case ICMP_REDIRECT:
         if (code < DIM(icmp_redirect_str))
              strcpy (buf, icmp_redirect_str[code]);
         else sprintf (buf, "Redirect; code %d", code);
         break;

    case ICMP_PARAMPROB:
         if (code)
              sprintf (buf, "Param prob code %d", code);
         else sprintf (buf, "Param prob at %d", icmp->pointer.pointer);
         break;

    case ICMP_MASKREQ:
    case ICMP_MASKREPLY:
         sprintf (buf, "ICMP %s: %s", icmp_type_str[type],
                  _inet_ntoa(NULL, intel(icmp->mask.mask)));
         break;

#if 0
    /** \todo Handle debugging of these
     */
    case ICMP_ROUTERADVERT:
    case ICMP_ROUTERSOLICIT:
    case ICMP_TSTAMP:
    case ICMP_TSTAMPREPLY:
    case ICMP_IREQ:
    case ICMP_IREQREPLY:
#endif

    default:
         sprintf (buf, "%s (%d), code %d",
                  type < DIM(icmp_type_str) ?
                    icmp_type_str[type] : "Unknown", type, code);
  }

  chk_ok = do_check_sum (icmp->unused.checksum, icmp, len);

  dbug_printf ("ICMP:  %s -> %s\n"
               "       %s, chksum %s\n",
               ip4_src, ip4_dst, buf, chk_ok);

  if (type == ICMP_UNREACH || type == ICMP_PARAMPROB)
     return ip4_orig_dump (ip, &icmp->ip.ip, len - 8);

  if (type == ICMP_TIMXCEED)
     return ip4_orig_dump (ip, &icmp->unused.ip, len - 8);

  return dump_data (icmp, len);
}

/*----------------------------------------------------------------------*/

#if defined(USE_MULTICAST)

static int igmp0_dump (const IGMPv0_packet *igmp, WORD len)
{
  ARGSUSED (len);
  return dbug_printf ("IGMP:  unfinished IGMP v0 parser. addr %s\n",
                      _inet_ntoa(NULL, intel(igmp->address)));
}

static int igmp1_dump (const IGMPv1_packet *igmp, WORD len)
{
  static const struct search_list types[] = {
                                { IGMPv1_QUERY,  "Query" },
                                { IGMPv1_REPORT, "Report" },
                                { IGMPv1_DVMRP,  "DVMRP" }
                              };
  return dbug_printf ("IGMP:  %s, ver %d, chksum %s, addr %s\n",
                      list_lookup(igmp->type, types, DIM(types)),
                      igmp->version, do_check_sum(igmp->checksum, igmp, len),
                      _inet_ntoa(NULL, intel(igmp->address)));
}

static int igmp2_dump (const IGMPv2_packet *igmp, WORD len)
{
  ARGSUSED (len);
  return dbug_printf ("IGMP:  unfinished IGMP v2 parser. addr %s\n",
                      _inet_ntoa(NULL, intel(igmp->address)));
}

static int igmp3_dump (const IGMPv3_packet *igmp, WORD len)
{
  ARGSUSED (len);
  return dbug_printf ("IGMP:  unfinished IGMP v3 parser. addr %s\n",
                      _inet_ntoa(NULL, intel(igmp->address)));
}

static int igmp_dump (const in_Header *ip)
{
  WORD  hdr_len              = in_GetHdrLen (ip);           /* length of IP-header */
  const IGMPv0_packet *igmp0 = (const IGMPv0_packet*) ((const BYTE*)ip + hdr_len);
  const IGMPv1_packet *igmp1 = (const IGMPv1_packet*) igmp0;
  const IGMPv2_packet *igmp2 = (const IGMPv2_packet*) igmp0;
  const IGMPv3_packet *igmp3 = (const IGMPv3_packet*) igmp0;
  WORD  igmp_len             = intel16 (ip->length) - hdr_len;
  BOOL  is_v[4]              = { FALSE, FALSE, FALSE, FALSE };

  if (igmp_len == sizeof(*igmp1) && igmp1->version == 1) /* 8 byte */
      is_v[1] = TRUE;

  else if (igmp_len == sizeof(*igmp2))                   /* 8 byte */
      is_v[2] = TRUE;

  else if (igmp_len == sizeof(*igmp0))                   /* 16 byte */
      is_v[0] = TRUE;

  else if (igmp_len >= sizeof(*igmp3))                   /* >= 14 byte, multiple of 4 (14,18,22..) */
      is_v[3] = TRUE;

  else
      return dbug_printf ("IGMP:  Header length %u??\n", igmp_len);

  if (is_v[0])
     return igmp0_dump (igmp0, igmp_len);
  if (is_v[1])
     return igmp1_dump (igmp1, igmp_len);
  if (is_v[2])
     return igmp2_dump (igmp2, igmp_len);
  if (is_v[3])
     return igmp3_dump (igmp3, igmp_len);

  return dbug_printf ("IGMP:  Unknown pkt, len %u\n", igmp_len);
}
#endif  /* USE_MULTICAST */

/*----------------------------------------------------------------------*/

#if defined(USE_SCTP)
/*
 * A simple SCTP dumper used for both SCTP inside IPv4 and IPv6 packets.
 *
 * Refs:
 * RFC 2960, 3309, 3758, 4460
 */
static const struct search_list sctp_chunks[] = {
                    { SCTP_DATA,                  "DATA"              },
                    { SCTP_INITIATION,            "INIT"              },
                    { SCTP_INITIATION_ACK,        "INIT-ACK"          },
                    { SCTP_SELECTIVE_ACK,         "SACK"              },
                    { SCTP_HEARTBEAT_REQUEST,     "HB REQ"            },
                    { SCTP_HEARTBEAT_ACK,         "HB ACK"            },
                    { SCTP_ABORT_ASSOCIATION,     "ABORT"             },
                    { SCTP_SHUTDOWN,              "SHUTDOWN"          },
                    { SCTP_SHUTDOWN_ACK,          "SHUTDOWN ACK"      },
                    { SCTP_OPERATION_ERR,         "OP ERR"            },
                    { SCTP_COOKIE_ECHO,           "COOKIE ECHO"       },
                    { SCTP_COOKIE_ACK,            "COOKIE ACK"        },
                    { SCTP_ECN_ECHO,              "ECN ECHO"          },
                    { SCTP_ECN_CWR,               "ECN CWR"           },
                    { SCTP_SHUTDOWN_COMPLETE,     "SHUTDOWN COMPLETE" },
                    { SCTP_FORWARD_CUM_TSN,       "FOR CUM TSN"       },
                    { SCTP_RELIABLE_CNTL,         "REL CTRL"          },
                    { SCTP_RELIABLE_CNTL_ACK,     "REL CTRL ACK"      },
                    { SCTP_ASCONF_ACK_CHUNK_ID,   "ASCONF ACK CHUNK ID"   },
                    { SCTP_PKTDROP_CHUNK_ID,      "PKTDROP CHUNK ID"      },
                    { SCTP_STREAM_RESET_CHUNK_ID, "STREAM RESET_CHUNK ID" },
                    { SCTP_IETF_EXT,              "IETF EXT" }
                  };

static const struct search_list sctp_params[] = {
                    { SCTP_IPV4_PARAM_TYPE,     "IPV4 PARAM TYPE"    },
                    { SCTP_IPV6_PARAM_TYPE,     "IPV6 PARAM TYPE"    },
                    { SCTP_RESPONDER_COOKIE,    "RESPONDER COOKIE"   },
                    { SCTP_UNRECOG_PARAM,       "UNRECOG_PARAM"      },
                    { SCTP_COOKIE_PRESERVE,     "COOKIE PRESERVE"    },
                    { SCTP_HOSTNAME_VIA_DNS,    "HOSTNAME VIA_DNS"   },
                    { SCTP_RESTRICT_ADDR_TO,    "RESTRICT ADDR_TO"   },
                    { SCTP_ECN_I_CAN_DO_ECN,    "I CAN DO ECN"       },
                    { SCTP_OPERATION_SUCCEED,   "OPERATION SUCCEED"  },
                    { SCTP_ERROR_NOT_EXECUTED,  "ERROR NOT EXECUTED" },
                    { SCTP_UNRELIABLE_STRM,     "UNRELIABLE STRM"    },
                    { SCTP_ADD_IP_ADDRESS,      "ADD IP ADDRESS"     },
                    { SCTP_DEL_IP_ADDRESS,      "DEL IP ADDRESS"     },
                    { SCTP_STRM_FLOW_LIMIT,     "STRM FLOW LIMIT"    },
                    { SCTP_PARTIAL_CSUM,        "PARTIAL CSUM"       },
                    { SCTP_ERROR_CAUSE_TLV,     "ERROR CAUSE TLV"    },
                    { SCTP_MIT_STACK_NAME,      "MIT STACK NAME"     },
                    { SCTP_SETADDRESS_PRIMARY,  "SETADDRESS PRIMARY" },
                    { SCTP_RANDOM_PARAM,        "RANDOM PARAM"       },
                    { SCTP_AUTH_CHUNK,          "AUTH CHUNK"         },
                    { SCTP_REQ_HMAC_ALGO,       "REQ HMAC ALGO"      },
                    { SCTP_SUPPORTED_EXT,       "SUPPORTED EXT"      }
                  };

static void sctp_dump_params (const BYTE *p, int len)
{
  while (len > 0)
  {
    const struct sctp_ParamDesc *param = (const struct sctp_ParamDesc*) p;
    WORD  plen  = intel16 (param->paramLength);
    WORD  ptype = intel16 (param->paramType);
    WORD  align = 0;

    dbug_printf ("       param: plen %d, %s\n",
                 plen, list_lookupX(ptype, sctp_params, DIM(sctp_params)));
    align = plen % 4;
    if (align != 0)
        align = 4 - align;
    len -= plen + align;
    p   += plen + align;
  }
}

static void sctp_dump_init (const struct sctp_Initiation *init, const BYTE *chunk_end)
{
  dbug_printf ("       init tag %lu, rwnd %lu, OS %u, MIS %u, init TSN %lu\n",
               intel(init->initTag), intel(init->rcvWindowCredit),
               intel16(init->NumPreopenStreams), intel16(init->MaxInboundStreams),
               intel(init->initialTSN));
  init++;
  /* optional parameters after init-message */
  if ((const BYTE*)init < chunk_end)
     sctp_dump_params ((const BYTE*)init, chunk_end - (const BYTE*)init);
}

static int sctp_dump (const in_Header *ip4, const in6_Header *ip6)
{
  const struct sctp_Header    *head;
  const struct sctp_ChunkDesc *chunk;
  WORD   hlen, chunk_cnt, chunk_len;
  size_t len;

  if (ip4)
  {
    hlen = in_GetHdrLen (ip4);
    head = (const struct sctp_Header*) ((const BYTE*)ip4 + hlen);
    len  = intel16 (ip4->length) - hlen;
  }
  else
  {
    len  = intel16 (ip6->len);
    len  = min (len, MAX_IP6_DATA-sizeof(*ip6));
    head = (const struct sctp_Header*) (ip6 + 1);
  }

  if (len < sizeof(*head))
  {
    dbug_printf ("SCTP:  Bogus length %u\n", (unsigned)len);
    return (0);
  }

  dbug_printf ("SCTP:  %s (%d) -> %s (%d), Verif 0x%08lX, Adler 0x%08lX\n",
               ip4 ? ip4_src : ip6_src, intel16(head->source),
               ip4 ? ip4_dst : ip6_dst, intel16(head->destination),
               intel(head->verificationTag), intel(head->adler32));

  len -= sizeof(*head);
  if (len < sizeof(*chunk))
  {
    dbug_printf ("       no chunks!!, len %u\n", (unsigned)len);
    return (0);
  }
  chunk = (const struct sctp_ChunkDesc*) (head+1);

  for (chunk_cnt = 1; len > 0; chunk_cnt++)
  {
    const BYTE *chunk_end;
    WORD  align;

    chunk_len = intel16 (chunk->chunkLength);
    chunk_end = ((const BYTE*)chunk + chunk_len);

    if (chunk_len < sizeof(*chunk))
    {
      dbug_printf ("%d) [Bad chunk length %u]\n", chunk_cnt+1, chunk_len);
      break;
    }
    dbug_printf ("       chunk %d: [%s], len %d, flag 0x%02X\n",
                 chunk_cnt, list_lookup(chunk->chunkID, sctp_chunks, DIM(sctp_chunks)),
                 chunk_len, chunk->chunkFlg);

    if (chunk->chunkID == SCTP_INITIATION || chunk->chunkID == SCTP_INITIATION_ACK)
       sctp_dump_init ((const struct sctp_Initiation*)(chunk+1), chunk_end);

    align = chunk_len % 4;
    if (align != 0)
        align = 4 - align;

    chunk = (const struct sctp_ChunkDesc*) (chunk_end + align);
    len  -= chunk_len + align;
  }
  return dump_data (chunk, len);
}
#endif

/*----------------------------------------------------------------------*/

static unsigned ip4_payload_dump (const void *sock, const in_Header *ip)
{
#if defined(USE_MULTICAST)
  if (ip->proto == IGMP_PROTO && debug.IGMP)
     return igmp_dump (ip);
#endif

#if defined(USE_SCTP)
  if (ip->proto == SCTP_PROTO && debug.SCTP)
     return sctp_dump (ip, FALSE);
#endif

  if (ip->proto == ICMP_PROTO && debug.ICMP)
     return icmp4_dump (ip);

  if (ip->proto == UDP_PROTO && debug.UDP)
     return udp_dump ((const _udp_Socket*)sock, ip);

  if (ip->proto == TCP_PROTO && debug.TCP)
     return tcp_dump ((const _tcp_Socket*)sock, ip);

  if (debug.IP)  /* dump unsupported IPv4 protocols as hex */
  {
    WORD  hlen       = in_GetHdrLen (ip);
    WORD  len        = intel16 (ip->length) - hlen;
    const BYTE *data = (const BYTE*)ip + hlen;
    return dump_data (data, min(len, (WORD)(MAX_IP4_DATA-hlen)));
  }
  return (0);
}

/*----------------------------------------------------------------------*/

#if defined(USE_IPV6)
static const char *ip6_next_hdr (BYTE nxt)
{
  switch (nxt)
  {
    case IP6_NEXT_HOP:
         return ("HBH");
    case IP6_NEXT_TCP:
         return ("TCP");
    case IP6_NEXT_UDP:
         return ("UDP");
    case IP6_NEXT_IPV6:
         return ("IPV6");
    case IP6_NEXT_ROUTING:
         return ("ROUTE");
    case IP6_NEXT_FRAGMENT:
         return ("FRAG");
    case IP6_NEXT_ESP:
         return ("ESP");
    case IP6_NEXT_AUTH:
         return ("AUTH");
    case IP6_NEXT_ICMP:
         return ("ICMP6");
    case IP6_NEXT_NONE:
         return ("NONE");
    case IP6_NEXT_DEST:
         return ("DEST");
    case IP6_NEXT_SCTP:
         return ("SCTP");
    default:
         return ("??");
  }
}

const char *icmp6_options (const BYTE *opt, int tot_len)
{
  static char buf[50];
  char  *p = buf;
  int    len, val;

  while (tot_len > 0 && p < buf + DIM(buf))
  {
    switch (val = *opt++)
    {
      case ND_OPT_SOURCE_LINKADDR:
           p += sprintf (p, "src MAC %s, ", MAC_address(opt));
           break;
      case 0:
           p += sprintf (p, "EOL, ");
           break;
      default:
           p += sprintf (p, "opt %d?, ", val);
           break;
    }
    len = (*opt++) << 3;
    tot_len -= len;
    if (tot_len <= 0)
    {
      p -= 2;
      *p = '\0';
      return (buf);
    }
  }
  return (NULL);
}

static int icmp6_dump (const in6_Header *ip)
{
  const union ICMP6_PKT *icmp = (const union ICMP6_PKT*) (ip + 1);
  unsigned    len      = intel16 (ip->len);
  unsigned    type     = icmp->unused.type;
  char        buf[200] = "";
  char       *p   = buf;
  const BYTE *opt = NULL;
  int         rc;

  if (len < sizeof(icmp->unused))
  {
    dbug_write ("ICMP6: Short packet\n");
    return (1);
  }

  switch (type)
  {
    case ICMP6_DST_UNREACH:
         sprintf (buf, "Dest unreach");
         break;

    case ICMP6_PACKET_TOO_BIG:
         sprintf (buf, "Pkt too big");
         break;

    case ICMP6_TIME_EXCEEDED:
         sprintf (buf, "Time exceeded");
         break;

    case ICMP6_PARAM_PROB:
         sprintf (buf, "Param problem");
         break;

    case ICMP6_ECHO_REQUEST:
         sprintf (buf, "Echo request");
         break;

    case ICMP6_ECHO_REPLY:
         sprintf (buf, "Echo reply");
         break;

    case ICMP6_MEMBERSHIP_QUERY:
         sprintf (buf, "Member query");
         break;

    case ICMP6_MEMBERSHIP_REPORT:
         sprintf (buf, "Member report");
         break;

    case ICMP6_MEMBERSHIP_REDUCTION:
         sprintf (buf, "Member reduction");
         break;

    case ICMP6_ROUTER_RENUMBERING:
         sprintf (buf, "Router renum");
         break;

    case ICMP6_WRUREQUEST:
         sprintf (buf, "Who are you request");
         break;

    case ICMP6_WRUREPLY:
         sprintf (buf, "Who are you reply");
         break;

    case ND_ROUTER_SOLICIT:
         sprintf (buf, "Router solicitation");
         break;

    case ND_ROUTER_ADVERT:
         p += sprintf (p, "Router advert");
         if (len >= sizeof(icmp->radvert))
              p += sprintf (p, ", hop-lim %d, managed %d, lifetime %u\n"
                               "       reach-time %lu, retrans-time %lu",
                            icmp->radvert.hop_limit, icmp->radvert.managed,
                            intel16(icmp->radvert.lifetime),
                            intel(icmp->radvert.reach_time),
                            intel(icmp->radvert.retrans_time));
         else p += sprintf (p, ", short packet");
         break;

    case ND_NEIGHBOR_SOLICIT:
         sprintf (buf, "Neighbor solicitation, target %s",
                  _inet6_ntoa(&icmp->nd_solic.target));
         opt = (const BYTE*)icmp + sizeof(icmp->nd_solic);
         opt = (const BYTE*)icmp6_options (opt, len - sizeof(icmp->nd_solic));
         break;

    case ND_NEIGHBOR_ADVERT:
         sprintf (buf, "Neighbor advert");
         break;

    case ND_REDIRECT:
         sprintf (buf, "Redirect");
         break;

    default:
         sprintf (buf, "Unknown type %d", type);
         break;
  }

  rc = dbug_printf ("ICMP6: %s, code %d\n"
                    "       chksum %04X (%s)\n",
                    buf, icmp->unused.code, icmp->unused.checksum,
                    _ip6_icmp_checksum(ip,icmp,len) ? "ok" : "ERROR");
  if (opt)
      rc += dbug_printf ("       Opt: %s\n", opt);
  return (rc);
}

static int ip6_header_dump (const in6_Header *ip)
{
  char src_type [20];
  char dst_type [20];
  int  len;

  ip6_addr_type (src_type, &ip->source);
  ip6_addr_type (dst_type, &ip->destination);

  len = dbug_printf ("IP6:   %s %s-> %s %s\n",
                     ip6_src, src_type, ip6_dst, dst_type);
  len += dbug_printf ("       ver %d, pri %d, flow %d,%d,%d, len %u, "
                      "next-hdr %s (%u), hop-lim %d\n",
                      ip->ver, ip->pri, ip->flow_lbl[0],
                      ip->flow_lbl[1], ip->flow_lbl[2], intel16(ip->len),
                      ip6_next_hdr(ip->next_hdr), ip->next_hdr, ip->hop_limit);
  return (len);
}

static unsigned ip6_payload_dump (const void *sock, const in6_Header *ip)
{
  if (ip->next_hdr == IP6_NEXT_ICMP && debug.ICMP)
     return icmp6_dump (ip);

  if (ip->next_hdr == IP6_NEXT_UDP && debug.UDP)
     return udp6_dump ((const _udp_Socket*)sock, ip);

  if (ip->next_hdr == IP6_NEXT_TCP && debug.TCP)
     return tcp_dump ((const _tcp_Socket*)sock, (const in_Header*)ip);

#if defined(USE_SCTP)
  if (ip->next_hdr == IP6_NEXT_SCTP && debug.SCTP)
     return sctp_dump (NULL, ip);
#endif

  if (debug.IP)    /* dump IPv6-network protocols as hex */
  {
    WORD len = intel16 (ip->len);
    len = min (len, MAX_IP6_DATA-sizeof(*ip));
    return dump_data (ip+1, len);
  }
  return (0);
}
#endif  /* USE_IPV6 */

/*----------------------------------------------------------------------*/

#if defined(USE_PPPOE)
struct token {
       int         v;
       const char *s;
     };

struct token ppp_type2str[] = {
             { PPP_IP,    "IP"    },
             { PPP_IPV6,  "IP6"   },
             { PPP_IPX,   "IPX"   },
             { PPP_VJC,   "VJC"   },
             { PPP_VJNC,  "VJNC"  },
             { PPP_IPCP,  "IPCP"  },
             { PPP_IPXCP, "IPXCP" },
             { PPP_LCP,   "LCP"   },
             { PPP_LQR,   "LQR"   },
             { PPP_CCP,   "CCP"   },
             { PPP_PAP,   "PAP"   },
             { PPP_CHAP,  "CHAP"  },
             { PPP_CBCP,  "CBCP"  },
             { PPP_COMP,  "COMP"  },
             { 0,         NULL    }
           };

/*
 * Convert a token value to a string; use "fmt" if not found.
 */
static const char *tok2str (const struct token *lp, const char *fmt, int v)
{
  static char buf[128];

  while (lp->s)
  {
    if (lp->v == v)
       return (lp->s);
    ++lp;
  }
  if (!fmt)
     fmt = "#%d";
  sprintf (buf, fmt, v);
  return (buf);
}

static const char *pppoe_get_tag (const BYTE *tag)
{
  static char buf[100];
  char  *p = buf;
  WORD   type = *(WORD*)tag;
  int    len  = intel16 (*(WORD*)(tag+2));

  switch (type)
  {
    case PPPOE_TAG_END_LIST:
         return ("end");

    case PPPOE_TAG_SERVICE_NAME:
         len = min (len, SIZEOF(buf)-12);
         sprintf (buf, "service-name `%.*s'", len, tag+PPPOE_TAG_HDR_SIZE);
         break;

    case PPPOE_TAG_AC_NAME:
         len = min (len, SIZEOF(buf)-12);
         sprintf (buf, "AC-name `%.*s'", len, tag+PPPOE_TAG_HDR_SIZE);
         break;

    case PPPOE_TAG_AC_COOKIE:
         p   += sprintf (p, "AC-cookie ");
         tag += PPPOE_TAG_HDR_SIZE;
         while (len > 0 && p < buf+sizeof(buf)-4)
         {
           p += sprintf (p, "%02X-", *tag & 255);
           tag++;
           len--;
         }
         *(p-1) = '\0';
         break;

    case PPPOE_TAG_HOST_UNIQ:
         p   += sprintf (p, "host-uniq ");
         tag += PPPOE_TAG_HDR_SIZE;
         while (len > 0 && p < buf+sizeof(buf)-4)
         {
           p += sprintf (p, "%02X-", *tag & 255);
           tag++;
           len--;
         }
         *(p-1) = '\0';
         break;

    case PPPOE_TAG_VENDOR_SPES:
         sprintf (buf, "vendor spec ID %08lX", *(DWORD*)(tag+4));
         break;

    case PPPOE_TAG_RELAY_SESS:
         sprintf (buf, "relay session %d", *(WORD*)(tag+4));
         break;

    case PPPOE_TAG_HOST_URL:
         len = min (len, SIZEOF(buf)-12);
         sprintf (buf, "host URL `%.*s'", len, tag+PPPOE_TAG_HDR_SIZE);
         break;

    case PPPOE_TAG_MOTM:
         len = min (len, SIZEOF(buf)-12);
         sprintf (buf, "msg-of-the-minute `%.*s'",
                  len, tag+PPPOE_TAG_HDR_SIZE);
         break;

    case PPPOE_TAG_IP_ROUTE_ADD:
         {
           char  buf1[20], buf2[20], buf3[20];
           const char *dest_net  = _inet_ntoa (buf1, intel(*(DWORD*)(tag+4)));
           const char *dest_mask = _inet_ntoa (buf2, intel(*(DWORD*)(tag+8)));
           const char *gateway   = _inet_ntoa (buf3, intel(*(DWORD*)(tag+12)));
           DWORD metric          = intel (*(DWORD*)(tag+16));

           sprintf (buf, "route add: %s %s %s / %lu",
                    dest_net, dest_mask, gateway, metric);
         }
         break;

    case PPPOE_TAG_SERVICE_ERR:
         return ("service error");

    case PPPOE_TAG_AC_SYSTEM_ERR:
         return ("AC-system err");

    case PPPOE_TAG_GENERIC_ERR:
         return ("generic error");

    default:
         sprintf (buf, "unknown %04X", intel16(type));
         break;
  }
  return (buf);
}

const char *pppoe_get_code (WORD code)
{
  switch (code)
  {
    case PPPOE_CODE_PADI:
         return ("PADI");
    case PPPOE_CODE_PADO:
         return ("PADO");
    case PPPOE_CODE_PADR:
         return ("PADR");
    case PPPOE_CODE_PADS:
         return ("PADS");
    case PPPOE_CODE_PADT:
         return ("PADT");
    case PPPOE_CODE_PADM:
         return ("PADM");
    case PPPOE_CODE_PADN:
         return ("PADN");
    default:
         return ("??");
  }
}

static __inline int pppoe_head_dump (const struct pppoe_Packet *pppoe,
                                     const char *proto)
{
  WORD len = intel16 (pppoe->length);

  if (pppoe->ver != 1 || pppoe->type != 1 || len > PPPOE_MAX_DATA)
  {
    dbug_printf ("PPPOE:  bogus header: ver %u, type %u, len %u\n",
                 pppoe->ver, pppoe->type, len);
    return (0);
  }
  return dbug_printf ("PPPOE:  %s, len %u, code %s (%04X), session %d\n",
                      proto, len, pppoe_get_code(pppoe->code), pppoe->code,
                      intel16(pppoe->session));
}

static int pppoe_disc_dump (const struct pppoe_Packet *pppoe)
{
  WORD  tlen = intel16 (pppoe->length);
  const BYTE *tags;

  if (!pppoe_head_dump(pppoe, "Discovery"))
     return (1);

  tags = (const BYTE*) &pppoe->data[0];
  while (tlen > 0)
  {
    WORD tag_len = intel16 (*(WORD*)(tags+2));

    dbug_printf ("        tag: %s\n", pppoe_get_tag(tags));
    tlen -= (PPPOE_TAG_HDR_SIZE + tag_len);
    tags += (PPPOE_TAG_HDR_SIZE + tag_len);
  }
  return (1);
}

static int pppoe_sess_dump (const void *sock, const struct pppoe_Packet *pppoe)
{
  WORD  proto, len;
  const BYTE *buf;

  if (!pppoe_head_dump(pppoe, "Session"))
     return (1);

  len   = intel16 (pppoe->length) - 2;
  proto = intel16 (*(WORD*)&pppoe->data[0]);
  buf   = &pppoe->data[2];

  dbug_printf ("        Protocol %04X (%s)\n", proto,
               tok2str(ppp_type2str,"%u??",proto));

  if (proto == PPP_IP)
  {
    const in_Header *ip = (const in_Header*)buf;

    _inet_ntoa (ip4_src, intel(ip->source));
    _inet_ntoa (ip4_dst, intel(ip->destination));

    ip4_dump (ip);
    return ip4_payload_dump (sock, ip);
  }

  if (proto == PPP_CCP)
     return ccp_dump (buf);

  if (proto == PPP_LCP)
     return lcp_dump (buf);

  if (proto == PPP_IPCP)
     return ipcp_dump (buf);

  return dump_data (buf, len);
}
#endif

/*----------------------------------------------------------------------*/

static const char *udp_tcp_checksum (const in_Header  *ip,
                                     const udp_Header *udp,
                                     const tcp_Header *tcp)
{
  tcp_PseudoHeader ph;
  int              len;

  memset (&ph, 0, sizeof(ph));
  if (udp)
  {
    len = intel16 (udp->length);
    ph.protocol = UDP_PROTO;
    ph.checksum = CHECKSUM (udp, len);
  }
  else
  {
    len = intel16 (ip->length) - in_GetHdrLen (ip);
    ph.protocol = TCP_PROTO;
    ph.checksum = CHECKSUM (tcp, len);
  }

  ph.src    = ip->source;
  ph.dst    = ip->destination;
  ph.length = intel16 (len);

  if (CHECKSUM(&ph,sizeof(ph)) == 0xFFFF)
     return ("ok");
  return ("ERROR");
}

#if defined(USE_IPV6)
static const char *udp_checksum (const in6_Header *ip, const udp_Header *udp)
{
  if (_ip6_checksum(ip, IP6_NEXT_UDP, udp, intel16(ip->len)) == 0xFFFF)
     return ("ok");
  return ("ERROR");
}

static const char *tcp_checksum (const in6_Header *ip, const tcp_Header *tcp)
{
  if (_ip6_checksum(ip, IP6_NEXT_TCP, tcp, intel16(ip->len)) == 0xFFFF)
     return ("ok");
  return ("ERROR");
}
#endif

/*----------------------------------------------------------------------*/

static BOOL is_dns_packet (WORD src_port, WORD dst_port)
{
  WORD dns_port;

  if (!dbg_dns_details)
     return (FALSE);

  dns_port = intel16 (DOM_DST_PORT);

#if 1
  if (src_port == dns_port || dst_port == dns_port)
     return (TRUE);
#else
  if ((!outbound && src_port == dns_port) || /* Rx from DNS */
      ( outbound && dst_port == dns_port))   /* Tx to DNS */
     return (TRUE);
#endif
  return (FALSE);
}

/*----------------------------------------------------------------------*/

static int udp_dump (const _udp_Socket *sock, const in_Header *ip)
{
  const char *chk_ok    = "n/a";
  WORD  iplen           = intel16 (ip->length) - sizeof(*ip);
  const udp_Header *udp = (const udp_Header*) ((BYTE*)ip + in_GetHdrLen(ip));
  WORD  udplen          = intel16 (udp->length) - sizeof(*udp);
  const BYTE *data      = (const BYTE*) (udp+1);

  if (udp->checksum && !frag_status)
     chk_ok = udp_tcp_checksum (ip, udp, NULL);

  if (frag_status || udplen > iplen)
     udplen = min (udplen, iplen);

  if (frag_status && !(frag_status & IS_FIRST)) /* is a fragment and not 1st */
     return dump_data (udp, iplen);

  dump_addr_port ("UDP", sock, ip);

  dbug_printf ("       len %d, chksum %04X (%s)\n",
               intel16(udp->length), intel16(udp->checksum), chk_ok);

  if (udplen >= sizeof(struct DNS_Header) &&
      is_dns_packet(udp->srcPort,udp->dstPort))
     return dns_dump (data, udplen, 0);
  return dump_data (data, udplen);
}

#if defined(USE_IPV6)
static int udp6_dump (const _udp_Socket *sock, const in6_Header *ip)
{
  WORD  iplen           = intel16 (ip->len);
  const udp_Header *udp = (const udp_Header*) (ip + 1);
  WORD  udplen          = intel16 (udp->length) - sizeof(*udp);
  const BYTE *data      = (const BYTE*) (udp + 1);
  const char *chk_ok    = udp_checksum (ip, udp);

  udplen = min (udplen, (WORD)(iplen - sizeof(*udp)));

  dump_addr6_port ("UDP", sock, ip);

  dbug_printf ("       len %d, chksum %04X (%s)\n",
               intel16(udp->length), intel16(udp->checksum), chk_ok);

  if (udplen >= sizeof(struct DNS_Header) &&
      is_dns_packet(udp->srcPort,udp->dstPort))
     return dns_dump (data, udplen, 0);
  return dump_data (data, udplen);
}
#endif   /* USE_IPV6 */

/*----------------------------------------------------------------------*/

static int tcp_dump (const _tcp_Socket *sock, const in_Header *ip)
{
  const tcp_Header *tcp;
  const BYTE       *data;
  const char       *chk_ok = "n/a";
  WORD  hlen;
  WORD  iplen, dlen, olen;
  DWORD win, ack, seq;
  char  flgBuf [4*8+1] = { 0 };

#if defined(USE_IPV6)
  const in6_Header *ip6 = (const in6_Header*) ip;

  if (ip->ver == 6)
  {
    iplen = intel16 (ip6->len);
    tcp   = (const tcp_Header*) (ip6 + 1);
    hlen  = tcp->offset << 2;
    dlen  = iplen - hlen;
  }
  else
#endif
  {
    int j;
    iplen = intel16 (ip->length) - sizeof(*ip);
    j     = in_GetHdrLen (ip);
    tcp   = (const tcp_Header*) ((const BYTE*)ip + j);
    hlen  = tcp->offset << 2;
    dlen  = intel16 (ip->length) - j - hlen;
  }

  data = (const BYTE*) tcp + hlen;
  win  = intel16 (tcp->window);

  if (frag_status || dlen > iplen)
     dlen = min (dlen, iplen);

  if (hlen < sizeof(*tcp))
  {
    dbug_write ("       Bad header\n");
    return (0);
  }

  if (frag_status && !(frag_status & IS_FIRST))
     return dump_data (tcp, iplen);

  if (tcp->flags & tcp_FlagACK)  strcat (flgBuf, " ACK");
  if (tcp->flags & tcp_FlagFIN)  strcat (flgBuf, " FIN");
  if (tcp->flags & tcp_FlagSYN)  strcat (flgBuf, " SYN");
  if (tcp->flags & tcp_FlagPUSH) strcat (flgBuf, " PSH");
  if (tcp->flags & tcp_FlagRST)  strcat (flgBuf, " RST");
  if (tcp->flags & tcp_FlagURG)  strcat (flgBuf, " URG");
  if (tcp->flags & tcp_FlagECN)  strcat (flgBuf, " ECN");
  if (tcp->flags & tcp_FlagCWR)  strcat (flgBuf, " CWR");

  if (!frag_status || (frag_status & IS_FIRST))
  {
#if defined(USE_IPV6)
    if (ip->ver == 6)
       chk_ok = tcp_checksum (ip6, tcp);
    else
#endif
       chk_ok = udp_tcp_checksum (ip, NULL, tcp);
  }

#if defined(USE_IPV6)
  if (ip->ver == 6)
     dump_addr6_port ("TCP", sock, ip6);
  else
#endif
     dump_addr_port ("TCP", sock, ip);

  if (sock)
     win <<= outbound ? sock->tx_wscale : sock->rx_wscale;

  ack = intel (tcp->acknum);
  seq = intel (tcp->seqnum);

  dbug_printf ("       flags%s, win %lu, chksum %04X (%s), urg %u\n"
               "                  SEQ %10lu,  ACK %10lu\n",
               flgBuf, (u_long)win, intel16(tcp->checksum), chk_ok,
               intel16(tcp->urgent), (u_long)seq, (u_long)ack);
  if (sock)
  {
    BOOL  in_seq_space;
    UINT  state = sock->state;
    long  delta_seq, delta_ack;
    long  ms_left, ms_right;

    if (outbound)
    {
      if (state == tcp_StateSYNREC && sock->last_acknum[0] == 0)
           delta_ack = 0;
      else delta_ack = ack - sock->last_acknum[0];

      if (state == tcp_StateSYNSENT && sock->last_seqnum[0] == 0)
           delta_seq = 0;
      else delta_seq = seq - sock->last_seqnum[0];

      ((_tcp_Socket*)sock)->last_seqnum[0] = seq;  /* !! unconst */
      ((_tcp_Socket*)sock)->last_acknum[0] = ack;
    }
    else
    {
      if (state == tcp_StateLISTEN && sock->last_seqnum[1] == 0)
           delta_seq = 0;
      else delta_seq = seq - sock->last_seqnum[1];

      if (state == tcp_StateSYNSENT && sock->last_acknum[1] == 0)
           delta_ack = 0;
      else delta_ack = ack - sock->last_acknum[1];

      ((_tcp_Socket*)sock)->last_seqnum[1] = seq;
      ((_tcp_Socket*)sock)->last_acknum[1] = ack;
    }

    if (sock->missed_seq[0] != sock->missed_seq[1] && !outbound)
    {
      /* show missing edges relative to his SEQ
       */
      ms_left  = (long) (seq - sock->missed_seq[1]);
      ms_right = (long) (seq - sock->missed_seq[0]);
      in_seq_space = SEQ_BETWEEN (seq, sock->missed_seq[1], sock->missed_seq[0]);
    }
    else if (sock->missed_seq[0] != sock->missed_seq[1] && outbound)
    {
      /* show missing edges relative to our ACK
       */
      ms_left  = (long) (ack - sock->missed_seq[1]);
      ms_right = (long) (ack - sock->missed_seq[0]);
      in_seq_space = FALSE;
    }
    else
    {
      ms_left  = 0UL;
      ms_right = 0UL;
      in_seq_space = FALSE;
    }

    dbug_printf ("       %-8.8s (dSEQ %10ld, dACK %10ld), MS %ld/%ld%s%s\n"
                 "       RCV.NXT %lu, SND.NXT %lu, SND.UNA %ld\n"
                 "       KC %d, vjSA %lu, vjSD %lu, CW %d, WW %d, RTO %d, ",
                 tcpStateName(state), delta_seq, delta_ack, ms_left, ms_right,
                 in_seq_space  ? ", in-SEQ"  : "",
                 sock->unhappy ? ", Unhappy" : "",
                 (u_long)sock->recv_next, (u_long)sock->send_next, (u_long)sock->send_una,
                 sock->karn_count, (u_long)sock->vj_sa, (u_long)sock->vj_sd,
                 sock->cwindow, sock->wwindow, sock->rto);
    dbug_printf ("RTT-diff %s\n", RTT_str(sock->rtt_time, now));
  }

  olen = (WORD) (hlen - sizeof(*tcp));
  if (olen > 0)
     dump_tcp_opt (tcp+1, olen, ack);

  if (dlen > sizeof(struct DNS_Header) &&
      is_dns_packet(tcp->srcPort,tcp->dstPort))
  {
    WORD dns_len = intel16 (*(WORD*)data);

    if (dns_len == dlen-2)
       return dns_dump (data+2, dns_len, dns_len);
  }
  return dump_data (data, dlen);
}

/*----------------------------------------------------------------------*/

static BOOL dbug_filter (void)
{
#if defined(NEED_PKT_SPLIT)
  const struct pkt_split *ps = outbound ? pkt_get_split_out() :
                                          pkt_get_split_in();
  for ( ; ps->data; ps++)
  {
    switch (ps->type)
    {
      case TYPE_ETHER_HEAD:
           if (!outbound)
           {
             const eth_Header *eth = (const eth_Header*) ps->data;

             if (filter.MAC && !match_link_destination(&eth->destination))
                return (FALSE);
             if (filter.LOOP && !memcmp(&eth->source,_eth_addr,_eth_mac_len))
                return (FALSE);
           }
           break;

      case TYPE_TOKEN_HEAD:
           if (!outbound)
           {
             const tok_Header *tok = (const tok_Header*) ps->data;

             if (filter.MAC && !match_link_destination(&tok->destination))
                return (FALSE);
             if (filter.LOOP && !memcmp(&tok->source,_eth_addr,_eth_mac_len))
                return (FALSE);
           }
           break;

      case TYPE_FDDI_HEAD:
           if (!outbound)
           {
             const fddi_Header *fddi = (const fddi_Header*) ps->data;

             if (filter.MAC && !match_link_destination(&fddi->destination))
                return (FALSE);
             if (filter.LOOP && !memcmp(&fddi->source,_eth_addr,_eth_mac_len))
                return (FALSE);
           }
           break;

      case TYPE_ARP:
           if (!outbound)
           {
             const arp_Header *arp = (const arp_Header*) ps->data;

             if (filter.ARP && !match_arp_rarp(arp))
                return (FALSE);
             if (filter.LOOP && !memcmp(&arp->srcEthAddr,_eth_addr,_eth_mac_len))
                return (FALSE);
           }
           return (debug.ARP);

      case TYPE_RARP:
           if (!outbound)
           {
             const rarp_Header *rarp = (const rarp_Header*) ps->data;

             if (filter.RARP && !match_arp_rarp(rarp))
                return (FALSE);
             if (filter.LOOP && !memcmp(&rarp->srcEthAddr,_eth_addr,_eth_mac_len))
                return (FALSE);
           }
           return (debug.RARP);

      case TYPE_PPPOE_SESS:
      case TYPE_PPPOE_DISC:
           break;

      case TYPE_IP4:
           if (!outbound)
           {
             const in_Header *ip = (const in_Header*) ps->data;

             if (filter.IP && !match_ip4_dest(ip))
                return (FALSE);
             if (filter.LOOP && intel(ip->source) == my_ip_addr)
                return (FALSE);
           }
           return (TRUE);

#if defined(USE_IPV6)
      case TYPE_IP6:
           if (!outbound)
           {
             const in6_Header *ip6 = (const in6_Header*) ps->data;

             if (filter.IP && !match_ip6_dest(ip6))
                return (FALSE);
             if (filter.LOOP &&
                 IN6_ARE_ADDR_EQUAL(&ip6->source, &in6addr_my_ip))
                return (FALSE);
           }
           return (TRUE);
#endif

      default:
           return (TRUE);  /* don't have filter for anything else */
    }
  }
#endif                     /* NEED_PKT_SPLIT */
  return (TRUE);           /* didn't match the component; allow it */
}

/*----------------------------------------------------------------------*/

static const char *pcdbug_driver_ver (void)
{
  static char buf[20];
  WORD major, minor, unused, build;

  if (!pkt_get_drvr_ver(&major,&minor,&unused,&build))
     return ("?");

#if defined(_WIN32)  /* NPF.SYS/SwsVpkt.sys ver e.g. 3.1.0.22 */
  sprintf (buf, "%u.%u.%u.%u", major, minor, unused, build);
#else
  sprintf (buf, "%d.%02d", major, minor);
#endif

  return (buf);
}

static const char *pcdbug_api_ver (void)
{
  static char buf[10];
  WORD   ver;

  /* DOS:   return pktdrvr API version (1.09 hopefully).
   * Win32: return NDIS version.
   */
  if (!pkt_get_api_ver(&ver))
     return ("?");

#if defined(_WIN32)
  sprintf (buf, "%d.%d", hiBYTE(ver), loBYTE(ver));
#else
  if (ver >= 255)  /* assumed to be MSB.LSB */
       sprintf (buf, "%d.%02d", hiBYTE(ver), loBYTE(ver));
  else sprintf (buf, "%d", loBYTE(ver));
#endif
  return (buf);
}

/*----------------------------------------------------------------------*/

static void print_driver_info (void)
{
#if defined(_WIN32)
  /* What is needed to enable promiscous receive mode?*/
  #define PROMISC_RECV()  (_pkt_rxmode & RXMODE_PROMISCOUS)
  #define CONFIG_RXMODE   "winpkt.rxmode"

  const char *api = (_pkt_inf ? _pkt_inf->api_name      : "??");
  const char *sys = (_pkt_inf ? _pkt_inf->sys_drvr_name : "??");

  dbug_printf ("%s\n"
               "Using: %s, \"%s\" (%s)\n"
               "       %s ver %s, NDIS ver %s, mode 0x%02X\n",
               wattcpVersion(), api, _pktdrvrname, _pktdrvr_descr,
               sys, pcdbug_driver_ver(), pcdbug_api_ver(), _pkt_rxmode);

#else
  #define PROMISC_RECV()  (_pkt_rxmode >= RXMODE_PROMISCOUS)
  #define CONFIG_RXMODE   "pkt.rxmode"

  dbug_printf ("%s\nPKTDRVR: \"%s\", ver %s, API ver %s, mode %d\n",
               wattcpVersion(), _pktdrvrname, pcdbug_driver_ver(),
               pcdbug_api_ver(), _pkt_rxmode);
#endif
}

/*----------------------------------------------------------------------*/

static void dbug_dump (const void *sock, const in_Header *ip,
                       const char *fname, unsigned line, BOOL out)
{
  static BOOL print_once = FALSE;
  int    err;

  WATT_ASSERT (ip);

  outbound = out;
  frag_status = 0;
  now = set_timeout (0);

  if ((err = setjmp(dbg_jmp)) != 0)
  {
    (*_printf) ("\ndbug_dump: write failed; %s (%d)\n", strerror(err), err);
    dbug_close();
    goto quit;
  }

  if (pcap_mode && !use_ods)
  {
    int rc = 0;

    if (!dbg_file.stream || !(filter.NONE || dbug_filter()))
       goto quit;

    if (out && _eth_last.tx.size)
       rc = write_pcap_packet (MAC_HDR(ip), TRUE);

    else if (!out && _eth_last.rx.size)
       rc = write_pcap_packet (MAC_HDR(ip), FALSE);

    if (rc < 0)
    {
      (*_printf) (_LANG("\ndbug_dump: write failed; %s\n"), strerror(errno));
      dbug_close();
    }
    goto quit;
  }

  if (!print_once)
  {
    print_once = TRUE;
    print_driver_info();

#if defined(PRINT_CPU_INFO)
    print_cpu_info();
#endif

    if (!PROMISC_RECV() && filter.NONE)
       dbug_printf ("\nNB: receive-mode (%s = 0x%02X) is not suitable for "
                    "receiving all traffic.\n", CONFIG_RXMODE, _pkt_rxmode);
    dbug_putc ('\n');

#if !defined(__BORLANDC__) && 0 /* Some weird bug with Borland */
    _arp_debug_dump();          /* GvB 2002-09 now encapsuled in pcarp.c */

    if (_bootp_on || _dhcp_on || _rarp_on)
       dbug_printf ("       Above routing data may be overridden by "
                    "DHCP/BOOTP/RARP\n");
#endif
  }

  if (!filter.NONE && !dbug_filter())
     goto quit;

  if (use_ods)
     dbug_putc ('\n');   /* !! Why do I need this? */

  dbug_printf ("\n%s: %s (%u), time %s%s\n",
               outbound ? "Transmitted" : "Received",
               fname, line, elapsed_str(now),
               is_looped(ip) ? ", Link-layer loop!" : "");

  /* Either PDCLASS_ETHER, PDCLASS_TOKEN, PDCLASS_FDDI or PDCLASS_ARCNET
   */
  if (!_pktserial)
  {
    union link_Packet *pkt  = (union link_Packet*) MAC_HDR (ip);
    WORD               type = MAC_TYP (ip);

    if (debug.MAC)
       link_head_dump (pkt);

    if (_pktdevclass == PDCLASS_ARCNET)
    {
      if (type == ARCNET_IP_1051 ||  /* Map to IANA proto-numbers */
          type == ARCNET_IP_1201)
           type = IP4_TYPE;
      else if (type == ARCNET_ARP_1051 ||
               type == ARCNET_ARP_1201)
           type = ARP_TYPE;
      else type = 0;                 /* unsupported */
    }
    else if (type == 0xFFFF)
    {
      dbug_printf ("IPX:   unsupported\n");
      goto quit;
    }
    else if (intel16(type) < 0x600)  /* 0x600=1536; type is IEEE 802.3 length field */
    {
      const llc_Header *llc = (const llc_Header*) (&pkt->eth.head + 1);

      /* !! could be NetBeui, VLAN etc. */
      dbug_printf ("LLC:   unsupported\n");
      if (debug.LLC)
         dump_data (llc+1, intel16(type));
      goto quit;
    }

    if (type == ARP_TYPE)
    {
      arp_dump ((const arp_Header*)ip);
      goto quit;
    }

    if (type == RARP_TYPE)
    {
      rarp_dump ((const rarp_Header*)ip);
      goto quit;
    }

#if defined(USE_PPPOE)
    if (type == PPPOE_DISC_TYPE)
    {
      pppoe_disc_dump ((const struct pppoe_Packet*)pkt->eth.data);
      goto quit;
  }
    if (type == PPPOE_SESS_TYPE)
    {
      pppoe_sess_dump (sock, (const struct pppoe_Packet*)pkt->eth.data);
      goto quit;
    }
#endif

#if defined(USE_IPV6)
    if (type == IP6_TYPE && ip->ver == 6)
       goto ip6_dmp;
#endif

    if (type != IP4_TYPE)
    {
      const BYTE *data;

      if (!debug.MAC)
         dbug_printf ("Unknown type %04X\n",
                      _pktdevclass == PDCLASS_ARCNET ? type : intel16(type));

      data = (const BYTE*)pkt + _pkt_ip_ofs;
      if (out && _eth_last.tx.size > (unsigned)_pkt_ip_ofs)
         dump_data (data, _eth_last.tx.size - _pkt_ip_ofs);
      else if (!out && _eth_last.rx.size > (unsigned)_pkt_ip_ofs)
         dump_data (data, _eth_last.rx.size - _pkt_ip_ofs);
      goto quit;
    }
  }

  if (ip->ver == 4)
  {
    _inet_ntoa (ip4_src, intel(ip->source));
    _inet_ntoa (ip4_dst, intel(ip->destination));

    ip4_dump (ip);
    ip4_payload_dump (sock, ip);
  }
#if defined(USE_IPV6)
  else if (ip->ver == 6)
  {
    const struct in6_Header *ip6;
    const char  *addr;

ip6_dmp:
    ip6  = (const struct in6_Header*) ip;
    addr = _inet6_ntoa (ip6->source);
    strcpy (ip6_src, addr ? addr : "(null)");

    addr = _inet6_ntoa (ip6->destination);
    strcpy (ip6_dst, addr ? addr : "(null)");

    if (debug.IP)
       ip6_header_dump (ip6);
    ip6_payload_dump (sock, ip6);
  }
#endif
  else
  {
    WORD len = intel16 (ip->length);
    dbug_printf ("IP?:   ver %d\n", ip->ver);
    dump_data (ip, min(ETH_MAX_DATA,len));
  }

quit:
  return;
}

/*----------------------------------------------------------------------*/

static void trace_xmit_pkt (const void *sock, const in_Header *ip,
                            const char *fname, unsigned line)
{
  if (use_ods || dbg_file.stream)
  {
    in_dbug_dump = TRUE;
    dbug_dump (sock, ip, fname, line, TRUE);
    in_dbug_dump = FALSE;
    dbug_flush();
  }
}

static void trace_recv_pkt (const void *sock, const in_Header *ip,
                            const char *fname, unsigned line)
{
  if (use_ods || dbg_file.stream)
  {
    in_dbug_dump = TRUE;
    dbug_dump (sock, ip, fname, line, FALSE);
    in_dbug_dump = FALSE;
    dbug_flush();
  }
}

/*
 * Print IP-options
 */
static void dump_ip_opt (const void *opt_p, int len)
{
  const BYTE *opt   = (const BYTE*) opt_p;
  const BYTE *start = opt;
  WORD  val16;
  DWORD val32, ip, ts;
  int   i, num, num_opt = 0;

  dbug_write ("       Options:");

  while (opt < start+len && num_opt < 10)
  {
    switch (*opt)  /* Note: IP-option includes copy/class bits */
    {
      case IPOPT_EOL:
           dbug_write (" EOL\n");
           return;

      case IPOPT_NOP:
           dbug_write (" NOP");
           opt++;
           continue;

      case IPOPT_RR:
           dbug_write (" RR");
           num = *(opt+1) - 3;
           num = min (num, len-1);
           for (i = 0; i < num; i += sizeof(DWORD))
           {
             val32 = intel (*(DWORD*)(opt+3+i));
             dbug_printf (" %s", _inet_ntoa(NULL,val32));
           }
           break;

      case IPOPT_TS:
           ip = intel (*(DWORD*)(opt+4));
           ts = intel (*(DWORD*)(opt+8));
           dbug_printf (" TS <%s/%lus..>", _inet_ntoa(NULL,ip), (u_long)ts);
           break;

      case IPOPT_SECURITY:
           val32 = intel (*(DWORD*)(opt+2));
           dbug_printf (" SEC %08lX", (u_long)val32);
           break;

      case IPOPT_SATID:
           val16 = intel16 (*(WORD*)(opt+2));
           dbug_printf (" SATID %04X", val16);
           break;

      case IPOPT_RA:
           val16 = intel16 (*(WORD*)(opt+2));
           dbug_printf (" RA %u", val16);
           break;

      case IPOPT_LSRR:
      case IPOPT_SSRR:
           dbug_write (*opt == IPOPT_LSRR ? " LSRR" : " SSRR");
           num = *(opt+1) - 3;
           num = min (num, len-1);
           for (i = 0; i < num; i += sizeof(DWORD))
           {
             DWORD route = intel (*(DWORD*)(opt+3+i));
             dbug_printf (" %s", _inet_ntoa(NULL,route));
           }
           break;

      default:
           dbug_printf (" opt %d?", *opt);
    }
    opt += *(opt+1);
    num_opt++;
  }
  dbug_putc ('\n');
}

/*
 * Print TCP-options
 */
static void dump_tcp_opt (const void *opt_p, int len, DWORD ack)
{
  const BYTE *opt   = (const BYTE*) opt_p;
  const BYTE *start = opt;
  BYTE  val8;
  WORD  val16;
  DWORD val32;
  int   i, num, num_opt = 0;

  dbug_write ("       Options:");

  while (opt < start+len && num_opt < 10)
  {
    switch (*opt)
    {
      case TCPOPT_EOL:
           dbug_write (" EOL\n");
           return;

      case TCPOPT_NOP:
           dbug_write (" NOP");
           opt++;
           continue;

      case TCPOPT_MAXSEG:
           val16 = intel16 (*(WORD*)(opt+2));
           dbug_printf (" MSS %u", val16);
           break;

      case TCPOPT_WINDOW:
           val8 = opt[2];
           dbug_printf (" Wscale 2^%d", val8);
           break;

      case TCPOPT_SACK:
           dbug_write (" SACK ");
           num = (*(opt+1) - 2) / SIZEOF(DWORD);
           num = min (num, len/SIZEOF(DWORD));

           for (i = 0; i < num; i++)
           {
             DWORD origin  = intel (*(DWORD*)(opt+2+4*i));
             DWORD relsize = intel (*(DWORD*)(opt+6+4*i));
             dbug_printf ("blk %d {%ld/%ld}, ", i+1,
                          (long)(ack-origin-1),
                          (long)(ack-origin+relsize));
           }
           break;

      case TCPOPT_SACK_PERM:
           dbug_write (" SACK-OK");
           break;

      case TCPOPT_ECHO:
           val32 = intel (*(DWORD*)(opt+2));
           dbug_printf (" Echo %lu", (u_long)val32);
           break;

      case TCPOPT_CHKSUM_REQ:
           val8 = opt[2];
           dbug_printf (" ChkReq %d", val8);
           break;

      case TCPOPT_CHKSUM_DAT:
           num = opt[1];
           dbug_write (" ChkDat ");
           for (i = 0; i < num; i++)
               dbug_printf ("%02X", opt[2+i]);
           break;

      case TCPOPT_ECHOREPLY:
           val32 = intel (*(DWORD*)(opt+2));
           dbug_printf (" Echoreply %lu", (u_long)val32);
           break;

      case TCPOPT_TIMESTAMP:
           val32 = intel (*(DWORD*)(opt+2));
           dbug_printf (" TS <%lu", (u_long)val32);
           val32 = intel (*(DWORD*)(opt+6));
           dbug_printf ("/%lu>", (u_long)val32);
           break;

      case TCPOPT_CC:
           val32 = intel (*(DWORD*)(opt+2));
           dbug_printf (" CC %lu", (u_long)val32);
           break;

      case TCPOPT_CCNEW:
           val32 = intel (*(DWORD*)(opt+2));
           dbug_printf (" CCnew %lu", (u_long)val32);
           break;

      case TCPOPT_CCECHO:
           val32 = intel (*(DWORD*)(opt+2));
           dbug_printf (" CCecho %lu", (u_long)val32);
           break;

      case TCPOPT_SIGNATURE:
           if (opt > (const BYTE*)opt_p)
              dbug_write ("\n               ");
           dbug_write (" MD5 ");
           for (i = 0; i < TCPOPT_SIGN_LEN; i++)
               dbug_printf ("%02X ", opt[2+i]);
           break;

      default:
           dbug_printf (" opt %d?", *opt);
    }
    opt += *(opt+1);
    num_opt++;
  }
  dbug_putc ('\n');
}

/*----------------------------------------------------------------------*/

static DWORD dump_data (const void *data_p, UINT datalen)
{
  const BYTE *data = (const BYTE*) data_p;
  UINT  ofs;
  DWORD len = 0;

  if (!dbg_mode_all)
     return (1);

  for (ofs = 0; ofs < datalen; ofs += 16)
  {
    UINT j;

    if (ofs == 0)
         len += dbug_printf ("%u:%s%04X: ", datalen,
                             datalen > 9999 ? " "    :
                             datalen > 999  ? "  "   :
                             datalen > 99   ? "   "  :
                             datalen > 9    ? "    " :
                                              "     ",
                             ofs);
    else len += dbug_printf ("       %04X: ", ofs);

    for (j = 0; j < 16 && j+ofs < datalen; j++)
        len += dbug_printf ("%02X%c", (unsigned)data[j+ofs],
                            j == 7 ? '-' : ' ');  /* no beeps */

    for ( ; j < 16; j++)       /* pad line to 16 positions */
        len += dbug_write ("   ");

    for (j = 0; j < 16 && j+ofs < datalen; j++)
    {
      int ch = data[j+ofs];

      if (ch < ' ')            /* non-printable */
           dbug_putc ('.');
      else dbug_putc (ch);
      len++;
    }
    dbug_putc ('\n');
    len++;
  }
  return (len);
}

/*----------------------------------------------------------------------*/

static void set_debug_file (const char *value)
{
  _strlcpy (dbg_name, value, sizeof(dbg_name)-1);
}

static void set_debug_mode (const char *value)
{
  if (!stricmp(value,"ALL"))
     dbg_mode_all = 1;
  if (!stricmp(value,"HEADER"))
     dbg_mode_all = 0;
}

static void set_debug_filter (const char *value)
{
  char val[80];

  _strlcpy (val, value, sizeof(val));
  strupr (val);

  if (strstr(val,"ALL"))
  {
    memset (&filter, 1, sizeof(filter));
    filter.NONE = FALSE;
  }
  else if (strstr(val,"NONE"))
  {
    memset (&filter, 0, sizeof(filter));
    filter.NONE = TRUE;
  }
  else
  {
    filter.NONE  = FALSE;
    filter.MAC   = (strstr(val,"ETH" )  != NULL) || (strstr(val,"MAC") != NULL);
    filter.ARP   = (strstr(val,"ARP" )  != NULL);
    filter.RARP  = (strstr(val,"RARP")  != NULL);
    filter.IP    = (strstr(val,"IP"  )  != NULL);
    filter.BCAST = (strstr(val,"BCAST") != NULL);
    filter.MCAST = (strstr(val,"MCAST") != NULL);
    filter.LOOP  = (strstr(val,"LOOP")  != NULL);
  }
}

static void set_debug_proto (const char *value)
{
  char val[80];

  _strlcpy (val, value, sizeof(val));
  strupr (val);

  if (!strcmp(val,"ALL"))
     memset (&debug, 1, sizeof(debug));
  else
  {
    memset (&debug, 0, sizeof(debug));
    debug.MAC  = (strstr(val,"ETH" ) != NULL) || (strstr(val,"MAC") != NULL);
    debug.LLC  = (strstr(val,"LLC" ) != NULL);
    debug.ARP  = (strstr(val,"ARP" ) != NULL);
    debug.RARP = (strstr(val,"RARP") != NULL);
    debug.IP   = (strstr(val,"IP"  ) != NULL);
    debug.TCP  = (strstr(val,"TCP" ) != NULL);
    debug.UDP  = (strstr(val,"UDP" ) != NULL);
    debug.ICMP = (strstr(val,"ICMP") != NULL);
    debug.IGMP = (strstr(val,"IGMP") != NULL);
    debug.SCTP = (strstr(val,"SCTP") != NULL);
  }
}

static void W32_CALL dbug_cfg_parse (const char *name, const char *value)
{
  static const struct config_table debug_cfg[] = {
             { "FILE",    ARG_FUNC, (void*)set_debug_file   },
             { "MODE",    ARG_FUNC, (void*)set_debug_mode   },
             { "LINEBUF", ARG_ATOI, (void*)&dbg_linebuf     },
             { "FILTER",  ARG_FUNC, (void*)set_debug_filter },
             { "PROTO",   ARG_FUNC, (void*)set_debug_proto  },
             { "STAT",    ARG_ATOI, (void*)&dbg_print_stat  },
             { "DNS",     ARG_ATOI, (void*)&dbg_dns_details },
             { "PCAP",    ARG_ATOI, (void*)&pcap_mode       },
             { NULL,      0,        NULL                    }
           };
  if (!parse_config_table(&debug_cfg[0], "DEBUG.", name, value) && prev_hook)
     (*prev_hook) (name, value);
}

#if defined(_WIN32)
  static char  ods_buf [4000];
  static char *ods_ptr = ods_buf;

  static int flush_ods (void)
  {
    WATT_ASSERT (ods_ptr >= ods_buf);
    WATT_ASSERT (ods_ptr < ods_buf + sizeof(ods_buf) - 2);

    if (ods_ptr == ods_buf)
       return (0);

    if (ods_ptr[-2] != '\r' && ods_ptr[-1] != '\n')
    {
      *ods_ptr++ = '\r';
      *ods_ptr++ = '\n';
    }
    *ods_ptr = '\0';
    ods_ptr = ods_buf;   /* restart buffer */
    OutputDebugStringA (ods_buf);
    return (1);
  }
#endif


int MS_CDECL dbug_printf (const char *format, ...)
{
  va_list arg;
  int len = 0;

#if defined(_WIN32)
  if (use_ods)
  {
    len = ods_buf + sizeof(ods_buf) - ods_ptr - 1;   /* length left on ods_buf[] */
    va_start (arg, format);
    if (len > 2)
       len = VSNPRINTF (ods_ptr, len, format, arg);
    va_end (arg);
    if (len > 0)
       ods_ptr += len;
    return (len);
  }
#endif

  /* don't let anyone else write if pcap-mode
   */
  if (dbg_file.stream && !pcap_mode)
  {
    va_start (arg, format);
    len = vfprintf (dbg_file.stream, format, arg);
    va_end (arg);
    if (in_dbug_dump && ferror(dbg_file.stream))
       longjmp (dbg_jmp, errno);
  }
  return (len);
}

int dbug_write (const char *buf)
{
  int len = -1;

#if defined(_WIN32)
  if (use_ods)
  {
    len = strlen (buf);
    len = min (len, ods_buf + sizeof(ods_buf) - ods_ptr - 1);
    memcpy (ods_ptr, buf, len);
    ods_ptr += len;
    return (len);
  }
#endif

  if (dbg_file.stream && !pcap_mode)
  {
    len = fputs (buf, dbg_file.stream);
    if (in_dbug_dump && (len < 0 || ferror(dbg_file.stream)))
       longjmp (dbg_jmp, errno);
  }
  return (len);
}

int dbug_putc (int c)
{
#if defined(_WIN32)
  if (use_ods)
  {
    WATT_ASSERT (ods_ptr);
    WATT_ASSERT (ods_ptr >= ods_buf);
    WATT_ASSERT (ods_ptr <= ods_buf + sizeof(ods_buf) - 2);

    if (c == '\n')
    {
      *ods_ptr++ = '\r';
      *ods_ptr++ = '\n';
      return flush_ods();
    }
    *ods_ptr++ = c;
    return (1);
  }
#endif
  return fputc (c, dbg_file.stream);
}

int db_write_raw (const char *buf) /* old name */
{
  return dbug_write (buf);
}

static void dbug_close (void)
{
  if (dbg_file.stream &&
      dbg_file.stream != stdout && dbg_file.stream != stderr)
  {
#if defined(USE_GZIP)
    if (use_gzip)
       gzclose (dbg_file.gz_stream);
    else
#endif
       fclose (dbg_file.stream);
  }
  dbg_file.stream = NULL;
}

void dbug_flush (void)
{
#if defined(_WIN32)
  if (use_ods)
     flush_ods();
  else
#endif
  if (dbg_file.stream && !pcap_mode)
  {
    fflush (dbg_file.stream);
#if 0
    if (dbg_file.stream != stdout &&  /* no point */
        dbg_file.stream != stderr)
       _dos_commit (fileno(dbg_file.stream));
#endif
  }
}

/*
 * Public initialisation
 */
void W32_CALL dbug_init (void)
{
  static BOOL init = FALSE;

  if (init)
  {
    outsnl ("dbug_init() already called");
    return;
  }
  init = TRUE;
  prev_hook  = usr_init;
  usr_init   = dbug_cfg_parse;
  debug_xmit = trace_xmit_pkt;
  debug_recv = trace_recv_pkt;

#if defined(USE_BSD_API)
  _sock_dbug_init();
#endif

  if (_watt_is_init && !dbg_file.stream)
       fprintf (stderr, "`dbug_init()' called after `sock_init()'.\n\7");
  else memset (&filter, 0, sizeof(filter));
}


/**
 * Print ARP-cache and statistics counters to one of the debug-files.
 *
 * Called from tcp_shutdown() after DHCP_release() and _tcp_abort() but
 * before _eth_release() is called. pkt-drop counters are not reliable
 * once _eth_release() has been called.
 */
static void W32_CALL dbug_exit (void)
{
  int (MS_CDECL *save_printf) (const char*, ...) = _printf;

  if (!_watt_fatal_error)  /* an assert isn't fatal */
  {
    if (dbg_file.stream && !pcap_mode)
         _printf = dbug_printf;          /* dump to wattcp.dbg */
#if defined(USE_BSD_API)
    else if (_sock_dbug_active())
         _printf = _sock_debugf;         /* dump to wattcp.sk */
#endif
    else _printf = NULL;

    if (_printf)
    {
#if defined(USE_STATISTICS)
      if (dbg_print_stat)
      {
      //_arp_debug_dump();    /* Moved from startup to closing */
        _arp_cache_dump();
        _arp_gateways_dump();
        _arp_routes_dump();
        print_all_stats();
      }
#endif

      if (_watt_cbroke)
         dbug_printf ("\nTerminating on SIGINT\n");
      if (_watt_assert_buf[0])
         dbug_printf ("\n%s\n", _watt_assert_buf);
      _watt_assert_buf[0] = '\0';
    }
  }
  dbug_close();

#if defined(USE_GZIP)
  if (use_gzip && !_watt_fatal_error)
  {
    struct stat st;

    if (stat(dbg_name,&st) == 0 && st.st_size > 0)
       (*_printf) (" \ngzip compression: %lu raw, %lu compressed (%lu%%)\n",
                   (u_long)bytes_raw, (u_long)st.st_size,
                   (u_long)(100 - (100*st.st_size)/bytes_raw));
  }
#endif

  _printf = save_printf;
}


/**
 * Debug of DNS (udp) records
 *
 * \author Mike Borella <mike_borella@mw.3com.com>
 *
 * Changed for Watt-32 and pcdbug.c by G.Vanem 1998 <gvanem@yahoo.no>
 *
 * \todo parse the SRV resource record (RFC 2052)
 */
static const char *dns_opcodes[16] = {
            "standard",       /* DNS_OP_QUERY */
            "inverse",        /* DNS_OP_INVERSE */
            "server status",
            "op 3??",
            "op 4??",
            "update",
            "op 6??",
            "op 7??",
            "op 8??",
            "op 9??",
            "op 10??",
            "op 11??",
            "op 12??",
            "op 13??",
            "op 14??",
            "op 15??"
          };

static const char *dns_responses[16] = {
            "no error",         /* DNS_SRV_OK */
            "format error",     /* DNS_SRV_FORM */
            "server error",     /* DNS_SRV_FAIL */
            "NXDOMAIN",         /* DNS_SRV_NAME */
            "not implemented",  /* DNS_SRV_NOTIMP */
            "service refused",  /* DNS_SRV_REFUSE */
            "resp 6??",
            "resp 7??",
            "resp 8??",
            "resp 9??",
            "resp 10??",
            "resp 11??",
            "resp 12??",
            "resp 13??",
            "resp 14??",
            "resp 15??"
          };

/*
 * dns_query()
 *
 * Return a string describing the numeric value of a DNS query type
 */
static __inline const char *dns_query (WORD type)
{
  switch (type)
  {
    case T_A:
         return ("A");
    case T_AAAA:
         return ("AAAA");
    case T_NS:
         return ("NS");
    case T_CNAME:
         return ("CNAME");
    case T_PTR:
         return ("PTR");
    case T_HINFO:
         return ("HINFO");
    case T_MX:
         return ("MX");
    case T_MB:
         return ("MB");
    case T_SOA:
         return ("SOA");
    case T_SRV:
         return ("SRV");
    case T_AXFR:
         return ("AXFR");
    case T_TXT:
         return ("TXT");
    case T_CAA:
         return ("CAA");
    case T_WINS:
         return ("WINS");
    case T_WINSR:
         return ("WINS-R");
    case T_ANY:
         return ("ANY");
    default:
         return ("??");
  }
}

/*
 * Parse DNS packet and dump fields
 */
static unsigned dns_dump (const BYTE *bp, unsigned length, unsigned dns_len)
{
  #define CHK_BOGUS(p) if (p > bp+length) goto bogus

  const struct DNS_Header *dns = (const struct DNS_Header*) bp;
  const BYTE  *body   = (const BYTE*)bp + sizeof(*dns);
  const BYTE  *end_p  = (const BYTE*)bp + length;
  const char  *opcode = dns_opcodes [dns->dns_fl_opcode];
  const char  *rcode  = dns_responses [dns->dns_fl_rcode];
  int      i, t;
  unsigned len = 0;

  if (!dbg_mode_all)
     return (1);

  dbug_write ("DNS:   ");
  if (dns_len)
     dbug_printf ("length %u, ", dns_len);

  len += dbug_printf ("Ident %u, %s, Opcode: %s, %sAuth\n",
                      intel16(dns->dns_id), dns->dns_fl_qr ?
                      "Response" : "Query", opcode ? opcode : "??",
                      !dns->dns_fl_ad ? "Non-" : "");

  if (!dns->dns_fl_qr && rcode)  /* Response code only in responses */
     rcode = "";

  len += dbug_printf ("       auth answer %d, truncated %d, rec-req %d, "
                      "rec-avail %d, %s\n",
                      dns->dns_fl_aa, dns->dns_fl_tc, dns->dns_fl_rd,
                      dns->dns_fl_ra, rcode ? rcode : "??");
  if (!opcode || !rcode)
     goto bogus;

  if (length <= sizeof(*dns))  /* no body present */
     return (length);

  /* Do the question part of the packet.
   */
  i = intel16 (dns->dns_num_q);

  while (i > 0)
  {
    WORD qtype, qclass;

    dbug_write ("       Question: query name ");
    body = dns_labels (body, bp, end_p);
    CHK_BOGUS (body);

    qtype  = intel16 (*(WORD*)body);  body += sizeof(qtype);
    qclass = intel16 (*(WORD*)body);  body += sizeof(qclass);

    dbug_printf ("       query type %d (%s), class %d\n",
                 qtype, dns_query(qtype), qclass);
    i--;
  }

  /* Dump the resource records for the answers
   */
  i = intel16 (dns->dns_num_ans);
  t = i;
  while (i > 0)
  {
    dbug_printf ("       Answer %d: ", t-i+1);
    body = dns_resource (body, bp, end_p);
    CHK_BOGUS (body);
    i--;
  }

  /* Dump the resource records for the authoritative answers
   */
  i = intel16 (dns->dns_num_auth);
  t = i;
  while (i > 0)
  {
    dbug_printf ("       Auth %d: ", t-i+1);
    body = dns_resource (body, bp, end_p);
    CHK_BOGUS (body);
    i--;
  }

  /* Dump the resource records for the additional info
   */
  i = intel16 (dns->dns_num_add);
  t = i;
  while (i > 0)
  {
    dbug_printf ("       Additional %d: ", t-i+1);
    body = dns_resource (body, bp, end_p);
    CHK_BOGUS (body);
    i--;
  }
  return (len);

bogus:
  dbug_printf ("       Looks like a bogus packet; length %d\n", length);
  return (len);
}

/*
 * Print the contents of a resource record
 */
static const BYTE *dns_resource (const BYTE *p, const BYTE *bp, const BYTE *ep)
{
  DWORD ttl, val;
  WORD  qtype, qclass, reslen;
  const char *ip;

  dbug_write ("server name: ");
  p = dns_labels (p, bp, ep);

  /* Do query type, class, ttl and resource length
   */
  qtype  = intel16 (*(WORD*)p);  p += sizeof(qtype);
  qclass = intel16 (*(WORD*)p);  p += sizeof(qclass);
  ttl    = intel  (*(DWORD*)p);  p += sizeof(ttl);
  reslen = intel16 (*(WORD*)p);  p += sizeof(reslen);

  dbug_printf ("         type %d (%s), class %d, TTL %s, length %u\n",
               qtype, dns_query(qtype), qclass, hms_str(ttl), reslen);

  /* Do resource data.
   */
  switch (qtype)
  {
    case T_A:     /* A record; ip4 address(es) */
         if (reslen == 4)
         {
           ip = _inet_ntoa (NULL, intel(*(DWORD*)p));
           dbug_printf ("         IP4 address: %s\n", ip);
         }
         else if (reslen == 7)
         {
           ip = _inet_ntoa (NULL, intel(*(DWORD*)p));
           dbug_printf ("         IP4 address: %s, proto %u, port %u\n",
                        ip, *(p+4), intel16(*(WORD*)(p+5)));
         }
         else
           dbug_printf ("         IP4 address: bogus reslen %u\n", reslen);

         p += reslen;
         break;

#if defined(USE_IPV6)
    case T_AAAA:     /* AAAA record; ip6 address(es) */
         while (reslen >= sizeof(ip6_address))
         {
           const char *addr = _inet6_ntoa ((const void*)p);
           dbug_printf ("         IP6 address: %s\n", addr);
           p      += sizeof (ip6_address);
           reslen -= sizeof (ip6_address);
         }
         break;
#endif
    case T_NS:    /* NS record; Name Server */
         dbug_write ("         auth host: ");
         p = dns_labels (p, bp, ep);
         break;

    case T_MX:    /* MX record; Mail Exchange */
         dbug_printf ("         mail exchange host (%u): ", intel16(*(WORD*)p));
         p += 2;
         p = dns_labels (p, bp, ep);
         break;

    case T_CNAME: /* CNAME record; canonical name */
         dbug_write ("         canon host: ");
         p = dns_labels (p, bp, ep);
         break;

    case T_PTR:   /* PTR record; hostname for IP */
         dbug_write ("         host name: ");
         p = dns_labels (p, bp, ep);
         break;

    case T_TXT:   /* TXT record; 1 or more text labels */
         dbug_write ("         TXT info: ");
         p = dns_labels (p, bp, ep);
         break;

    case T_WINS:  /* WINS record;  */
         dbug_write ("         WINS name: ");
         p = dns_labels (p, bp, ep);
         break;

    case T_WINSR:  /* WINS-reverse record;  */
         dbug_write ("         WINS-R addr: ");
         p = dns_labels (p, bp, ep);
         break;

    case T_SOA:
         dbug_write ("         SOA: ");
         p = dns_labels (p, bp, ep);
         if (p < ep)
         {
           dbug_write ("              ");
           p = dns_labels (p, bp, ep);

           val = intel (*(DWORD*)p);
           p += 4;
           dbug_printf ("              serial %lu", (u_long)val);

           val = intel (*(DWORD*)p);
           p += 4;
           dbug_printf (", refresh %s", hms_str(val));

           val = intel (*(DWORD*)p);
           p += 4;
           dbug_printf (", retry %s\n", hms_str(val));

           val = intel (*(DWORD*)p);
           p += 4;
           dbug_printf ("              expire %s", hms_str(val));

           val = intel (*(DWORD*)p);
           p += 4;
           dbug_printf (", minimum %s\n", hms_str(val));
         }
         break;

    default:
         p += reslen;
  }
  return (p);
}

/*
 * Recursively parse a label entry in a DNS packet
 * 'p'  points to a DNS label.
 * 'bp' points to a beginning of DNS header.
 */
static const BYTE *dns_labels (const BYTE *p, const BYTE *bp, const BYTE *ep)
{
  while (1)
  {
    BYTE count = *p++;

    if (count >= 0xC0)
    {
      /* There's a pointer (rel to 'bp' start) in this label.
       * Let's grab the 14 low-order bits and run with them.
       */
      WORD offset = (WORD) (((unsigned)(count - 0xC0) << 8) + *p++);

   /* dbug_printf ("(ofs %u) ", offset); !! show where the offset is */
      dns_labels (bp+offset, bp, ep);
      return (p);
    }
    if (count == 0)
       break;

    while (count > 0)
    {
      if (p <= ep)
      {
        dbug_putc (*p++);
        count--;
      }
      else
      {
        dbug_write ("\nPacket length exceeded\n");
        return (p);
      }
    }
    dbug_putc ('.');
  }
  dbug_putc ('\n');
  return (p);
}


#if defined(USE_PPPOE)

/* NCP Codes */
enum ncp_codes {
     CONFREQ = 1,
     CONFACK = 2,
     CONFNAK = 3,
     CONFREJ = 4,
     TERMREQ = 5,
     TERMACK = 6,
     CODEREJ = 7,
     PROTREJ = 8,
     ECHOREQ = 9,
     ECHOREP = 10,
     DISCARD = 11
   };

enum lcp_options {
     LCP_RESERVED  = 0,
     LCP_MRU       = 1,
     LCP_ASYNCMAP  = 2,
     LCP_AUTHPROTO = 3,
     LCP_QUALPROTO = 4,
     LCP_MAGICNUM  = 5,
     LCP_PCOMP     = 7,
     LCP_ACFCOMP   = 8,
     LCP_CALLBACK  = 13
   };

enum ipcp_options {
     IPCP_RESERVED  = 0,
     IPCP_ADDRS     = 1,
     IPCP_COMPRTYPE = 2,
     IPCP_ADDR      = 3,
     IPCP_MS_DNS1   = 129,
     IPCP_MS_WINS1  = 130,
     IPCP_MS_DNS2   = 131,
     IPCP_MS_WINS2  = 132,
   };

static const struct token ncp_code2str[] = {   /* LCP/IPCP codes */
                    { CONFREQ, "ConfReq" },
                    { CONFACK, "ConfAck" },
                    { CONFNAK, "ConfNak" },
                    { CONFREJ, "ConfRej" },
                    { TERMREQ, "TermReq" },
                    { TERMACK, "TermAck" },
                    { CODEREJ, "CodeRej" },
                    { PROTREJ, "ProtRej" },
                    { ECHOREQ, "EchoReq" },
                    { ECHOREP, "EchoRep" },
                    { DISCARD, "Discard" },
                    { 0,       NULL      }
                  };

static const struct token lcp_option2str[] = {
                    { LCP_RESERVED, "reserved" },
                    { LCP_MRU,      "MRU"      },
                    { LCP_ASYNCMAP, "AsyncMap" },
                    { LCP_AUTHPROTO,"AUTH"     },
                    { LCP_QUALPROTO,"QUAL"     },
                    { LCP_MAGICNUM, "MAGIC"    },
                    { LCP_PCOMP,    "PCOMP"    },
                    { LCP_ACFCOMP,  "ACFcomp"  },
                    { LCP_CALLBACK, "CallBack" },
                    { 0,            NULL       }
                  };

static const struct token ipcp_option2str[] = {
                    { IPCP_RESERVED, "reserved" },
                    { IPCP_ADDRS,    "Addrs"    },
                    { IPCP_COMPRTYPE,"Compr"    },
                    { IPCP_ADDR,     "Addr"     },
                    { IPCP_MS_DNS1,  "DNS1"     },
                    { IPCP_MS_WINS1, "Wins1"    },
                    { IPCP_MS_DNS2,  "DNS2"     },
                    { IPCP_MS_WINS2, "Win2"     },
                    { 0,             NULL       }
                  };

static const struct token lcpauth2str[] = {
                    { 0xc023, "PAP"  },
                    { 0xc223, "CHAP" },
                    { 0,       NULL  }
                  };

static const struct token lcpqual2str[] = {
                    { 0xc025, "LQR" },
                    { 0,      NULL  }
                  };

static const struct token lcpchap2str[] = {
                    { 0x05, "MD5" },
                    { 0x80, "MS"  },
                    { 0,    NULL  }
                  };

/*
 * Dump PPP-LCP contents. Functions taken from tcpdump.
 */
static unsigned lcp_dump (const BYTE *bp)
{
  WORD  lcp_code, lcp_id, lcp_length, arg;
  const BYTE *lcp_data, *p;
  unsigned len = 0;

  lcp_code   = bp[0];
  lcp_id     = bp[1];
  lcp_length = intel16 (*(WORD*)(bp+2));
  lcp_data   = bp + 4;

  len += dbug_printf ("LCP:   %s id=0x%02x ",
                      tok2str(ncp_code2str,"LCP-#%d",lcp_code), lcp_id);

  switch (lcp_code)
  {
    case CONFREQ:
    case CONFACK:
    case CONFNAK:
    case CONFREJ:
         p = lcp_data;

         while (p + 2 < lcp_data + lcp_length)
         {
           BYTE opt_type   = p[0];
           BYTE opt_length = p[1];

           p += 2;
           dbug_printf ("<%s ", tok2str(lcp_option2str,"option-#%d",opt_type));
           if (opt_length)
           {
             switch (opt_type)
             {
               case LCP_MRU:
                    arg = intel16 (*(WORD*)p);
                    dbug_printf ("%u", arg);
                    break;

               case LCP_AUTHPROTO:
                    arg = intel16 (*(WORD*)p);
                    dbug_printf (tok2str (lcpauth2str, "AUTH-%x", arg));
                    if (opt_length >= 5)
                       dbug_printf (" %s", tok2str(lcpchap2str, "%x", p[0]));
                    break;

               case LCP_QUALPROTO:
                    arg = intel16 (*(WORD*)p);
                    dbug_printf (tok2str(lcpqual2str, "QUAL-%x", arg));
                    break;

               case LCP_ASYNCMAP:
               case LCP_MAGICNUM:
                    arg = intel16 (*(WORD*)(p+4));
                    dbug_printf ("0x%08lX%04X", intel(*(DWORD*)p), arg);
                    break;

               case LCP_PCOMP:
               case LCP_ACFCOMP:
               case LCP_RESERVED:
                    break;

               default:
                    break;
             }
           }
           dbug_putc ('>');
           p += opt_length - 2;
         }
         break;

    case ECHOREQ:
    case ECHOREP:
    case DISCARD:
         dbug_printf ("magic=0x%08lX", intel(*(DWORD*)lcp_data));
         lcp_data += 4;
         break;

    case PROTREJ:
         arg = intel16 (*(WORD*)lcp_data);
         dbug_printf ("prot=%s", tok2str(ppp_type2str, "PROT-%#x",arg));
         break;

    case CODEREJ:
         dbug_putc ('\n');
         lcp_dump (lcp_data);
         break;

    case TERMREQ:
    case TERMACK:
         break;

    default:
         break;
  }
  dbug_putc ('\n');
  return (len);
}

/*
 * Dump PPP-CCP contents (compression control protocol).
 */
static unsigned ccp_dump (const BYTE *bp)
{
  ARGSUSED (bp);
  return (0);
}

/*
 * Dump PPP-IPCP contents.
 */
static unsigned ipcp_dump (const BYTE *bp)
{
  WORD  ipcp_code, ipcp_id, ipcp_length;
  const BYTE *ipcp_data, *p;
  unsigned len = 0;

  ipcp_code   = bp[0];
  ipcp_id     = bp[1];
  ipcp_length = intel16 (*(WORD*)(bp+2));
  ipcp_data   = bp + 4;

  len += dbug_printf ("IPCP:  %s id=0x%02x ",
                      tok2str(ncp_code2str,"IPCP-#%d",ipcp_code), ipcp_id);

  switch (ipcp_code)
  {
    case CONFREQ:
    case CONFACK:
    case CONFNAK:
    case CONFREJ:
         p = ipcp_data;

         while (p + 2 < ipcp_data + ipcp_length)
         {
           BYTE  opt_type   = p[0];
           BYTE  opt_length = p[1];
           DWORD addr;

           p += 2;
           dbug_printf ("<%s ", tok2str(ipcp_option2str,"option-#%d",opt_type));
           if (opt_length)
           {
             switch (opt_type)
             {
               case IPCP_ADDRS:
                    addr = intel (*(DWORD*)p);
                    dbug_printf ("addrs %s ", _inet_ntoa(NULL,addr));
                    addr = intel (*(DWORD*)(p+4));
                    dbug_printf ("%s", _inet_ntoa(NULL,addr));
                    break;

               case IPCP_ADDR:
                    addr = intel (*(DWORD*)p);
                    dbug_printf ("addr %s", _inet_ntoa(NULL,addr));
                    break;

               case IPCP_COMPRTYPE:
                    dbug_printf ("compress %u", intel16(*(WORD*)p));
                    break;

               case IPCP_MS_DNS1:
               case IPCP_MS_DNS2:
                    addr = intel (*(DWORD*)p);
                    dbug_printf ("MS-DNS %s", _inet_ntoa(NULL,addr));
                    break;

               case IPCP_MS_WINS1:
               case IPCP_MS_WINS2:
                    addr = intel (*(DWORD*)p);
                    dbug_printf ("MS-WINS %s", _inet_ntoa(NULL,addr));
                    break;

               default:
                    break;
             }
           }
           dbug_putc ('>');
           p += opt_length - 2;
         }
         break;

    case TERMREQ:
    case TERMACK:
         break;

    default:
         break;
  }
  dbug_putc ('\n');
  return (len);
}
#endif  /* USE_PPPOE */

/*
 * Functions for writing dump-file in pcap-format
 * (gzip compressed or non-compressed pcap format).
 */
#define TCPDUMP_MAGIC       0xA1B2C3D4
#define PCAP_VERSION_MAJOR  2
#define PCAP_VERSION_MINOR  4

W32_CLANG_PACK_WARN_OFF()

#include <sys/pack_on.h>

struct pcap_file_header {
       DWORD  magic;
       WORD   version_major;
       WORD   version_minor;
       DWORD  thiszone;        /* GMT to local correction */
       DWORD  sigfigs;         /* accuracy of timestamps */
       DWORD  snap_len;        /* max length saved portion of each pkt */
       DWORD  linktype;        /* data link type (DLT_*) */
     };

/*
 * Each packet in the dump file is prepended with this generic header.
 * This gets around the problem of different headers for different
 * packet interfaces.
 */
struct pcap_pkt_header {
       struct timeval ts;      /* time stamp */
       DWORD          caplen;  /* length of portion present */
       DWORD          len;     /* length of this packet (off wire) */
       DWORD          family;  /* protocol family value (for DLT_NULL) */
     };

#include <sys/pack_off.h>

W32_CLANG_PACK_WARN_DEF()

static int write_pcap_header (void)
{
  struct pcap_file_header pf_hdr;
  struct timeval  tv;
  struct timezone tz;
  size_t rc;

  gettimeofday2 (&tv, &tz);
  memset (&pf_hdr, 0, sizeof(pf_hdr));

  pf_hdr.magic         = TCPDUMP_MAGIC;
  pf_hdr.version_major = PCAP_VERSION_MAJOR;
  pf_hdr.version_minor = PCAP_VERSION_MINOR;
  pf_hdr.thiszone      = 60 * tz.tz_minuteswest;
  pf_hdr.sigfigs       = 0;
  pf_hdr.snap_len      = _mtu + _pkt_ip_ofs;
  pf_hdr.linktype      = _pktserial ? 101 : _eth_get_hwtype(NULL, NULL);

#if defined(USE_GZIP)
  if (use_gzip)
  {
    rc = gzwrite (dbg_file.gz_stream, &pf_hdr, sizeof(pf_hdr));
    bytes_raw += sizeof(pf_hdr);
  }
  else
#endif
     rc = fwrite (&pf_hdr, 1, sizeof(pf_hdr), dbg_file.stream);
  return (rc == 0 ? -1 : (int)rc);
}

static int write_pcap_packet (const void *pkt, BOOL out)
{
  const struct in_Header *ip = (const struct in_Header*) pkt;
  struct pcap_pkt_header  pc_hdr;
  size_t hlen, pkt_len, rc;

  memset (&pc_hdr, 0, sizeof(pc_hdr));

  if (out)
  {
#if (DOSX) && defined(HAVE_UINT64) && defined(__MSDOS__)
    if (!_eth_last.tx.tstamp.lo ||
        !get_tv_from_tsc (&_eth_last.tx.tstamp, &pc_hdr.ts))
#endif
       gettimeofday2 (&pc_hdr.ts, NULL);
    pkt_len = _eth_last.tx.size;
  }
  else
  {
#if (DOSX) && defined(HAVE_UINT64) && defined(__MSDOS__)
    if (!_eth_last.rx.tstamp.lo ||
        !get_tv_from_tsc (&_eth_last.rx.tstamp, &pc_hdr.ts))
#endif
       gettimeofday2 (&pc_hdr.ts, NULL);
    pkt_len = _eth_last.rx.size;
  }

  if (_pktserial)  /* write DLT_NULL header */
  {
    pc_hdr.len    = (DWORD) (pkt_len + sizeof(pc_hdr.family));
    pc_hdr.family = (ip->ver == 4) ? AF_INET : AF_INET6;
    hlen = sizeof (pc_hdr);
  }
  else
  {
    pc_hdr.len = (DWORD)pkt_len;
    hlen = sizeof (pc_hdr) - sizeof(pc_hdr.family);
  }
  pc_hdr.caplen = (DWORD)pkt_len;

#if defined(USE_GZIP)
  if (use_gzip)
  {
    rc = gzwrite (dbg_file.gz_stream, &pc_hdr, (unsigned)hlen);
    bytes_raw += (DWORD) hlen;
    rc = gzwrite (dbg_file.gz_stream, pkt, (unsigned)pkt_len);
    bytes_raw += (DWORD) pkt_len;
  }
  else
#endif
  {
    fwrite (&pc_hdr, hlen, 1, dbg_file.stream);
    rc = fwrite (pkt, 1, pkt_len, dbg_file.stream);
  }
  return (rc == 0 ? -1 : (int)rc);
}

/*
 * This stuff is taken mostly from tests/cpu.c.
 */
#if defined(PRINT_CPU_INFO)

#if 0 /* Use code in tests/cpu.c instead */
static const char *i486_model (int model)
{
  static const char *models[] = {
                    "0", "DX", "SX", "DX/2", "4", "SX/2", "6",
                    "DX/2-WB", "DX/4", "DX/4-WB", "10", "11", "12", "13",
                    "Am5x86-WT", "Am5x86-WB"
                  };
  if (model < DIM(models))
     return (models[model]);
  return (NULL);
}

static const char *i586_model (int model)
{
  static const char *models[] = {
                    "0", "Pentium 60/66", "Pentium 75+", "OverDrive PODP5V83",
                    "Pentium MMX", NULL, NULL, "Mobile Pentium 75+",
                    "Mobile Pentium MMX"
                  };
  if (model < DIM(models))
     return (models[model]);
  return (NULL);
}

static const char *Cx86_model (int type)
{
  int    model;
  static const char *models[] = {
                    "unknown", "6x86", "6x86L", "6x86MX", "MII"
                  };
  switch (type)
  {
    case 5:
         /* CX8 flag only on 6x86L */
         model = ((x86_capability & X86_CAPA_CX8) ? 2 : 1);
         break;
    case 6:
         model = 3;
         break;
    default:
         model = 0;
  }
  return (models[model]);
}

static const char *i686_model (int model)
{
  static const char *models[] = {
                    "PPro A-step", "Pentium Pro"
                  };
  if (model < DIM(models))
     return (models[model]);
  return (NULL);
}

struct model_info {
       int         type;
       const char *names[16];
     };

static const char *AMD_model (int type, int model)
{
  static const struct model_info amd_models[] = {
    { 4,
      { NULL, NULL, NULL, "DX/2", NULL, NULL, NULL, "DX/2-WB", "DX/4",
        "DX/4-WB", NULL, NULL, NULL, NULL, "Am5x86-WT", "Am5x86-WB"
      }
    },
    { 5,
      { "K5/SSA5 (PR-75, PR-90, PR-100)", "K5 (PR-120, PR-133)",
        "K5 (PR-166)", "K5 (PR-200)", NULL, NULL,
        "K6 (166-266)", "K6 (166-300)", "K6-2 (200-450)",
        "K6-3D-Plus (200-450)", NULL, NULL, NULL, NULL, NULL, NULL
      }
    }
  };
  int i;

  if (model < 16)
     for (i = 0; i < DIM(amd_models); i++)
         if (amd_models[i].type == type)
            return (amd_models[i].names[model]);
  return (NULL);
}

static const char *cpu_get_model (int type, int model)
{
  const  char *p = NULL;
  const  char *vendor = x86_vendor_id;
  static char buf[12];

  if (!x86_have_cpuid)
     return ("unknown");

  if (!strncmp(vendor, "Cyrix", 5))
     p = Cx86_model (type);
  else if (!strcmp(vendor, "AuthenticAMD"))
     p = AMD_model (type, model);
#if 0  /** \todo */
  else if (!strcmp(vendor, "UMC UMC UMC "))
     p = UMC_model (type, model);
  else if (!strcmp(vendor, "NexGenDriven"))
     p = NexGen_model (type, model);
  else if (!strcmp(vendor, "CentaurHauls"))
     p = Centaur_model (type, model);
  else if (!strcmp(vendor, "RiseRiseRise"))  /* Rise Technology */
     p = Rise_model (type, model);
  else if (!strcmp(vendor, "GenuineTMx86"))  /* Transmeta */
     p = Transmeta_model (type, model);
  else if (!strcmp(vendor, "Geode by NSC"))  /* National Semiconductor */
     p = National_model (type, model);
#endif
  else   /* Intel */
  {
    switch (type & 0x07)  /* mask off extended family bit */
    {
      case 4:
           p = i486_model (model);
           break;
      case 5:
           p = i586_model (model);   /* Pentium I */
           break;
      case 6:
           p = i686_model (model);   /* Pentium II */
           break;
      case 7:
           p = "Pentium 3";
           break;
      case 8:
           p = "Pentium 4";
           break;
    }
  }
  if (p)
     return (p);
  return itoa (model, buf, 10);
}
#endif   /* 0 */

static void print_cpu_info (void)
{
  uint64 Hz;
  char   speed [20];

  CheckCpuType();

  Hz = get_cpu_speed();
  if (Hz)
       sprintf (speed, "%.3f", (double)Hz/1E6);
  else strcpy (speed, "??");

  dbug_printf ("CPU: %d86, model: %s, vendor: %s, speed: %s MHz (%d CPU core%s)\n",
               x86_type, cpu_get_model(x86_type,x86_model),
               x86_vendor_id[0] ? x86_vendor_id : "unknown", speed,
               num_cpus, num_cpus > 1 ? "s" : "");
}
#endif  /* PRINT_CPU_INFO */
#endif  /* USE_DEBUG */
