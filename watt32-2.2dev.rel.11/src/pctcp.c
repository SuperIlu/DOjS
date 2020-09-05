/*!\file pctcp.c
 *  Main TCP handler.
 *
 *  PCTCP, the true worker of Waterloo TCP.
 *    - contains all opens, closes, major read/write routines and
 *      basic IP handler for incomming packets
 *    - Note: much of the TCP/UDP/IP layering is done at the data
 *      structure level, not in separate routines or tasks
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "copyrigh.h"
#include "wattcp.h"
#include "chksum.h"
#include "strings.h"
#include "language.h"
#include "pcdns.h"
#include "bsdname.h"
#include "pcconfig.h"
#include "pcqueue.h"
#include "pcsed.h"
#include "pcstat.h"
#include "pcpkt.h"
#include "pcicmp.h"
#include "pcicmp6.h"
#include "pcigmp.h"
#include "pcdbug.h"
#include "pcdhcp.h"
#include "pcarp.h"
#include "pcbuf.h"
#include "netaddr.h"
#include "ip4_frag.h"
#include "ip4_in.h"
#include "ip4_out.h"
#include "misc.h"
#include "run.h"
#include "timer.h"
#include "rs232.h"
#include "split.h"
#include "pppoe.h"
#include "pctcp.h"

#if defined(USE_BSD_API) || defined(USE_IPV6)
#include "socket.h"
#endif

#ifndef __inline  /**< normally in <sys/cdefs.h> */
#define __inline
#endif

/** Our configured hostname. Not the FQDN.
 */
char     hostname [MAX_HOSTLEN+1] = "random-pc";

unsigned _mss          = ETH_MAX_DATA - TCP_OVERHEAD;
unsigned _mtu          = ETH_MAX_DATA;
BOOL     mtu_discover  = 0;   /**< \todo Add PMTU discovery method */
BOOL     mtu_blackhole = 0;   /**< \todo Add PMTU blackhole detection */
BOOL     block_tcp     = 0;   /**< when application handles TCP itself */
BOOL     block_udp     = 0;   /**< when application handles UDP itself */
BOOL     block_icmp    = 0;   /**< when application handles ICMP itself */
BOOL     block_ip      = 0;   /**< when application handles IP itself */

DWORD    my_ip_addr    = 0L;          /**< our IP address */
DWORD    sin_mask      = 0xFFFFFF00L; /**< our net-mask, 255.255.255.0 */

_udp_Socket *_udp_allsocs = NULL;  /**< list of udp-sockets */

W32_CLANG_PACK_WARN_OFF()

#include <sys/pack_on.h>

struct ip4_packet {
       in_Header  in;
       udp_Header udp;
    /* BYTE       data[]; */
     };

struct ip6_packet {
       in6_Header in;
       udp_Header udp;
    /* BYTE       data[]; */
     };

struct tcp_pkt {
       in_Header  in;
       tcp_Header tcp;
     };

struct tcp6_pkt {
       in6_Header in;
       tcp_Header tcp;
     };

#include <sys/pack_off.h>

W32_CLANG_PACK_WARN_DEF()

#if defined(USE_BSD_API)
  /**
   * This hook is to prevent the BSD-socket API being linked in
   * by default. It is only set from the BSD functions when needed.
   */
  void *(MS_CDECL *_bsd_socket_hook) (enum BSD_SOCKET_OPS op, ...) = NULL;
#endif

#if !defined(USE_UDP_ONLY)

  /** TCP timer values.
   */
  unsigned tcp_OPEN_TO     = DEF_OPEN_TO;     /**< Default open timeout */
  unsigned tcp_CLOSE_TO    = DEF_CLOSE_TO;    /**< Default close timeout */
  unsigned tcp_RTO_ADD     = DEF_RTO_ADD;     /**< Time added in RTO calculation */
  unsigned tcp_RTO_BASE    = DEF_RTO_BASE;    /**< Base time for RTO calculation */
  unsigned tcp_RTO_SCALE   = DEF_RTO_SCALE;   /**< Scaling used in RTO calculation */
  unsigned tcp_RST_TIME    = DEF_RST_TIME;    /**< Don't generate RST too often */
  unsigned tcp_RETRAN_TIME = DEF_RETRAN_TIME; /**< Default retransmission time */
  unsigned tcp_MAX_VJSA    = DEF_MAX_VJSA;    /**< Default max VJ std. average */
  unsigned tcp_MAX_VJSD    = DEF_MAX_VJSD;    /**< Default max VJ std. deviation */

  /** TCP option config flags (RFC 1323).
   */
  BOOL tcp_opt_ts     = FALSE;
  BOOL tcp_opt_wscale = FALSE;
  BOOL tcp_opt_sack   = FALSE;

  /** Misc TCP values.
   */
  BOOL     tcp_nagle      = TRUE;         /**< Nagle algo. is on globally */
  unsigned tcp_keep_idle  = 2*60;         /**< idle time before sending keepalive */
  unsigned tcp_keep_intvl = 30;           /**< time between keepalive probes */
  unsigned tcp_max_idle   = 60;           /**< max idle time before kill */
  DWORD    tcp_recv_win   = DEF_RECV_WIN; /**< RWIN for BSD sockets only */

  _tcp_Socket *_tcp_allsocs = NULL;       /**< list of tcp-sockets */

  extern int _tcp_fsm (_tcp_Socket **s, const in_Header *ip);

  static _tcp_Socket *tcp_findseq (const in_Header *ip, const tcp_Header *tcp);
  static void         tcp_sockreset (_tcp_Socket *s, BOOL proxy);

  static void tcp_no_arp   (_tcp_Socket *s);
  static void tcp_rtt_win  (_tcp_Socket *s);
  static void tcp_upd_win  (_tcp_Socket *s, unsigned line);
  static BOOL tcp_checksum (const in_Header *ip, const tcp_Header *tcp, int len);
#endif

static void udp_close (const _udp_Socket *s);

static void (W32_CALL *system_yield)(void) = NULL;

/**
 * UDP passive open. Listen for a connection on a particular port.
 */
int W32_CALL udp_listen (_udp_Socket *s, WORD lport, DWORD ip,
                         WORD port, ProtoHandler handler)
{
  SIO_TRACE (("udp_listen"));

  udp_close (s);
  WATT_LARGE_CHECK (s, sizeof(*s));
  memset (s, 0, sizeof(*s));

  if (!_eth_is_init) /* GvB 2002-09, Lets us survive without a working eth */
  {
    SOCK_ERRNO (ENETDOWN);
    s->err_msg = _LANG (_eth_not_init);
    return (0);
  }

  s->rx_data      = &s->rx_buf[0];
  s->max_rx_data  = sizeof(s->rx_buf) - 1;
  s->ip_type      = UDP_PROTO;
  s->sockmode     = SOCK_MODE_BINARY | SOCK_MODE_UDPCHK;
  s->myport       = find_free_port (lport, 0); /* grab a local port */
  s->hisport      = port;
  s->hisaddr      = ip;
  s->ttl          = _default_ttl;
  s->protoHandler = handler;
  s->usr_yield    = system_yield;
  s->safetysig    = SAFETY_UDP;
  s->next         = _udp_allsocs;            /* insert into chain */
  _udp_allsocs    = s;
  return (1);
}

/**
 * UDP active open. Open a connection on a particular port.
 */
int W32_CALL udp_open (_udp_Socket *s, WORD lport, DWORD ip,
                       WORD port, ProtoHandler handler)
{
  BOOL bcast;

  SIO_TRACE (("udp_open"));

  udp_close (s);
  WATT_LARGE_CHECK (s, sizeof(*s));
  memset (s, 0, sizeof(*s));

  if (!_eth_is_init) /* GvB 2002-09, Lets us survive without a working eth */
  {
    SOCK_ERRNO (ENETDOWN);
    s->err_msg = _LANG (_eth_not_init);
    return (0);
  }

  if (ip && _ip4_is_multihome_addr(ip))  /* 0.0.0.0 is legal */
  {
    SOCK_ERRNO (EINVAL);
    strcpy (s->err_buf, _LANG("Illegal destination "));
    strcat (s->err_buf, _inet_ntoa(NULL,ip));
    s->err_msg = s->err_buf;
    return (0);
  }

  bcast = ((ip == IP_BCAST_ADDR) || (~ip & ~sin_mask) == 0);

  if (bcast || !ip)      /* check for broadcast */
  {
    memset (s->his_ethaddr, 0xFF, sizeof(eth_address));
    if (!ip)
       ip = IP_BCAST_ADDR; /* s->hisaddr = 255.255.255.255 (this network) */
  }
#if defined(USE_MULTICAST)
  else if (_ip4_is_multicast(ip))   /* check for multicast */
  {
    multi_to_eth (ip, &s->his_ethaddr);
    s->ttl = 1;     /* so we don't send worldwide as default */
  }
#endif
  else if (!_arp_resolve(ip,&s->his_ethaddr))
  {
    SOCK_ERRNO (EHOSTUNREACH);
    strcpy (s->err_buf, _LANG("ARP failed "));
    strcat (s->err_buf, _inet_ntoa(NULL,ip));
    s->err_msg = s->err_buf;
    STAT (ip4stats.ips_noroute++);
    return (0);
  }

  s->rx_data      = &s->rx_buf[0];
  s->max_rx_data  = sizeof(s->rx_buf) - 1;
  s->ip_type      = UDP_PROTO;
  s->sockmode     = SOCK_MODE_BINARY | SOCK_MODE_UDPCHK;
  s->myport       = find_free_port (lport, 0);
  s->myaddr       = my_ip_addr;
  s->ttl          = _default_ttl;
  s->hisaddr      = ip;
  s->hisport      = port;
  s->protoHandler = handler;
  s->usr_yield    = system_yield;
  s->safetysig    = SAFETY_UDP;
  s->next         = _udp_allsocs;
  _udp_allsocs    = s;
  return (1);
}

/**
 * Since UDP is stateless, simply reclaim the local-port and
 * unthread the socket from the list.
 */
static void udp_close (const _udp_Socket *udp)
{
  _udp_Socket *s, *prev;

  SIO_TRACE (("udp_close"));

  for (s = prev = _udp_allsocs; s; prev = s, s = s->next)
  {
    if (udp != s)
       continue;

    reuse_localport (s->myport);

    if (s == _udp_allsocs)
         _udp_allsocs = s->next;
    else prev->next   = s->next;
    SET_ERR_MSG (s, _LANG("UDP Close called"));
    break;
  }
}

/**
 * Set the TTL on an outgoing UDP datagram.
 */
int W32_CALL udp_SetTTL (_udp_Socket *s, BYTE ttl)
{
  s->ttl = ttl;
  return (0);
}


#if !defined(USE_UDP_ONLY)

/**
 * Actively open a TCP connection.
 *
 * Make connection to a particular destination.
 * Not used for IPv6 (see _TCP6_open()).
 *  \retval 0 on error. 's->err_msg' filled.
 *
 *  \note 'lport' is local port to associate with the connection.
 *  \note 'rport' is remote port for same connection.
 */
int W32_CALL tcp_open (_tcp_Socket *s, WORD lport, DWORD ip,
                       WORD rport, ProtoHandler handler)
{
  SIO_TRACE (("tcp_open"));

  WATT_LARGE_CHECK (s, sizeof(*s));
  _tcp_unthread (s, FALSE);    /* just in case not totally closed */

  memset (s, 0, sizeof(*s));
  s->state = tcp_StateCLOSED; /* otherwise is tcp_StateLISTEN on failure */

  STAT (tcpstats.tcps_connattempt++);

  if (!_eth_is_init) /* GvB 2002-09, Lets us survive without a working eth */
  {
    SOCK_ERRNO (ENETDOWN);
    s->err_msg = _LANG (_eth_not_init);
    return (0);
  }

  if (!ip || _ip4_is_multihome_addr(ip) || _ip4_is_multicast(ip))
  {
    SOCK_ERRNO (EINVAL);
    strcpy (s->err_buf, _LANG("Illegal destination "));
    strcat (s->err_buf, _inet_ntoa(NULL,ip));
    s->err_msg = s->err_buf;
    return (0);
  }

  if (!_arp_start_lookup(ip)) /* GvB 2002-09, now non-blocking */
  {
    SOCK_ERRNO (EHOSTUNREACH);
    strcpy (s->err_buf, _LANG("Failed to start ARP lookup "));
    strcat (s->err_buf, _inet_ntoa(NULL,ip));
    s->err_msg = s->err_buf;
    return (0);
  }

  s->rx_data      = &s->rx_buf[0];
  s->max_rx_data  = sizeof(s->rx_buf) - 1;
  s->tx_data      = &s->tx_buf[0];
  s->max_tx_data  = sizeof(s->tx_buf) - 1;
  s->ip_type      = TCP_PROTO;
  s->max_seg      = _mss;        /**< \todo use \b mss from setsockopt() */
  s->state        = tcp_StateRESOLVE;

  s->cwindow      = 1;
  s->wwindow      = 0;                        /* slow start VJ algorithm */
  s->vj_sa        = INIT_VJSA;
  s->rto          = tcp_OPEN_TO;              /* added 14-Dec 1999, GV */
  s->myaddr       = my_ip_addr;
  s->myport       = find_free_port (lport,1); /* get a nonzero port val */
  s->locflags     = LF_LINGER;                /* close via TIMEWT state */
  s->ttl          = _default_ttl;
  s->hisaddr      = ip;
  s->hisport      = rport;
  s->send_next    = INIT_SEQ();
  s->flags        = tcp_FlagSYN;
  s->unhappy      = TRUE;
  s->protoHandler = handler;
  s->usr_yield    = system_yield;

  s->safetysig    = SAFETY_TCP;             /* marker signatures */
  s->safetytcp    = SAFETY_TCP;
  s->next         = _tcp_allsocs;           /* insert into chain */
  _tcp_allsocs    = s;

  /** \todo use \b TCP_NODELAY set in setsockopt()
   */
  if (tcp_nagle)
     s->sockmode = SOCK_MODE_NAGLE;
  s->sockmode |= SOCK_MODE_BINARY;

  return (1);
}

/**
 * Passively opens TCP a connection.
 *
 * Listen for a connection on a particular port
 */
int W32_CALL tcp_listen (_tcp_Socket *s, WORD lport, DWORD ip,
                         WORD port, ProtoHandler handler, WORD timeout)
{
  SIO_TRACE (("tcp_listen"));

  WATT_LARGE_CHECK (s, sizeof(*s));
  _tcp_unthread (s, FALSE);     /* just in case not totally closed */
  memset (s, 0, sizeof(*s));

  s->state = tcp_StateCLOSED;   /* otherwise is tcp_StateLISTEN if success */

  if (!_eth_is_init) /* GvB 2002-09, Lets us survive without a working eth */
  {
    SOCK_ERRNO (ENETDOWN);
    s->err_msg = _LANG (_eth_not_init);
    return (0);
  }

  if (_ip4_is_multicast(ip))     /* 0.0.0.0 is legal */
  {
    SOCK_ERRNO (EINVAL);
    strcpy (s->err_buf, _LANG("Illegal destination "));
    strcat (s->err_buf, _inet_ntoa(NULL,ip));
    s->err_msg = s->err_buf;
    return (0);
  }

  s->rx_data      = &s->rx_buf[0];
  s->max_rx_data  = sizeof(s->rx_buf) - 1;
  s->tx_data      = &s->tx_buf[0];
  s->max_tx_data  = sizeof(s->tx_buf) - 1;
  s->ip_type      = TCP_PROTO;
  s->max_seg      = _mss;        /**< \todo use \b mss from setsockopt() */
  s->cwindow      = 1;
  s->wwindow      = 0;               /* slow start VJ algorithm */
  s->vj_sa        = INIT_VJSA;
  s->state        = tcp_StateLISTEN;
  s->locflags     = LF_LINGER | LF_IS_SERVER;
  s->myport       = find_free_port (lport, 0);
  s->hisport      = port;
  s->hisaddr      = ip;
  s->send_next    = INIT_SEQ();
  s->unhappy      = FALSE;
  s->ttl          = _default_ttl;
  s->protoHandler = handler;
  s->usr_yield    = system_yield;
  s->safetysig    = SAFETY_TCP;     /* marker signatures */
  s->safetytcp    = SAFETY_TCP;
  s->next         = _tcp_allsocs;   /* insert into chain */
  _tcp_allsocs    = s;

  if (tcp_nagle)
     s->sockmode = SOCK_MODE_NAGLE;
  s->sockmode |= SOCK_MODE_BINARY;

  if (timeout != 0)
     s->timeout = set_timeout (1000 * timeout);
  return (1);
}

/**
 * Close a TCP connection.
 *
 * Send a FIN on a particular port -- only works if it is open.
 * Must still allow receives.
 */
void _tcp_close (_tcp_Socket *s)
{
  SIO_TRACE (("_tcp_close"));

  if (s->ip_type != TCP_PROTO)
  {
    SIO_TRACE (("_tcp_close~0"));
    return;
  }

  if (s->state == tcp_StateESTAB ||
      s->state == tcp_StateESTCL ||
      s->state == tcp_StateSYNREC)
  {
    if (s->tx_datalen)      /* must first flush all Tx data */
    {
      s->flags |= (tcp_FlagPUSH | tcp_FlagACK);
      if (s->state < tcp_StateESTCL)
      {
        s->state = tcp_StateESTCL;
        TCP_SENDSOON (s);
      }
    }
    else  /* really closing */
    {
      SET_ERR_MSG (s, _LANG("Connection closed normally"));
      s->flags    = (tcp_FlagACK | tcp_FlagFIN);
      s->state    = tcp_StateFINWT1;
      s->timeout  = set_timeout (tcp_TIMEOUT);
      s->rtt_time = 0UL;   /* stop RTT timer */
      TCP_SEND (s);
    }
    s->unhappy = TRUE;
  }
  else if (s->state == tcp_StateCLOSWT)
  {
   /* need to ACK the FIN and get on with it
    */
    s->timeout = set_timeout (tcp_LASTACK_TIME); /* Added AGW 6 Jan 2001 */
    s->state   = tcp_StateLASTACK;
    s->flags  |= tcp_FlagFIN;
    TCP_SEND (s);
    s->unhappy = TRUE;
  }
  else if (s->state == tcp_StateRESOLVE ||  /* unlink failed connection */
           s->state == tcp_StateSYNSENT)
  {
    s->state = tcp_StateCLOSED;
    maybe_reuse_localport (s);
    _tcp_unthread (s, FALSE);
  }
  SIO_TRACE (("_tcp_close~"));
}

/**
 * Abort a TCP connection.
 */
_tcp_Socket *_tcp_abort (_tcp_Socket *s, const char *file, unsigned line)
{
  SIO_TRACE (("_tcp_abort"));

  SET_ERR_MSG (s, _LANG("TCP Abort"));

  TCP_TRACE_MSG (("_tcp_abort(%" ADDR_FMT ") called from %s (%u). State %s\n",
                  ADDR_CAST(s), file, line, tcpStateName(s->state)));

  if (s->state >= tcp_StateSYNSENT &&
      s->state <= tcp_StateLASTACK)
  {
    s->flags   = (tcp_FlagRST | tcp_FlagACK);
    s->unhappy = TRUE;
    if (s->state <= tcp_StateSYNREC)
    {
      s->rtt_time = 0UL;  /* Stop RTT timer */
      tcp_rtt_clr (s);    /* Clear cached RTT */
    }
    s->tx_datalen = 0;    /* RST should not carry any data */
    TCP_SEND (s);
  }
  s->unhappy    = FALSE;
  s->ip_type    = 0;
  s->tx_datalen = 0;      /* discard Tx buffer, but not Rx buffer */

  maybe_reuse_localport (s);
  ARGSUSED (file);
  ARGSUSED (line);
  return _tcp_unthread (s, TRUE);
}

/**
 * Schedule a transmission pretty soon.
 *
 * This one has an imperfection at midnight, but it
 * is not significant to the connection performance.
 *
 * \note gv: Added - 5 May 2000: Relax retransmission period to
 *           \b tcp_CLOSE_TO when \b CLOSEWT state is entered.\n
 *           Relax retransmission period to \b tcp_OPEN_TO in
 *           \b SYNSENT state.
 */
int _tcp_sendsoon (_tcp_Socket *s, const char *file, unsigned line)
{
  DWORD timeout;

  SIO_TRACE (("_tcp_sendsoon"));

  if (s->ip_type != TCP_PROTO)
     return (0);

  if (s->state >= tcp_StateCLOSWT)
       timeout = set_timeout (tcp_CLOSE_TO);
  else timeout = set_timeout (tcp_RTO_BASE);

  if (s->rto <= tcp_RTO_BASE && s->recent == 0 &&
      cmp_timers(s->rtt_time,timeout) <= 0)
  {                         /* !! was == */
    int rc;

    s->karn_count = 0;
    rc = _tcp_send (s, file, line);
    s->recent = 1;
    return (rc);
  }

  if ((s->unhappy || s->tx_datalen > 0 || s->karn_count == 1) &&
      (s->rtt_time && cmp_timers(s->rtt_time,timeout) < 0))
     return (0);

  if (s->state == tcp_StateSYNSENT)  /* relaxed in SYNSENT state */
       s->rtt_time = set_timeout (tcp_OPEN_TO);
  else s->rtt_time = set_timeout (s->rto / tcp_RTO_SCALE);

  s->karn_count = 1;

  return (0);
}

/**
 * Unthread a socket from the tcp socket list, if it's there.
 * Free Tx-buffer if set in tcp_set_window().
 */
_tcp_Socket *_tcp_unthread (_tcp_Socket *ds, BOOL free_tx)
{
  _tcp_Socket *s, *prev;
  _tcp_Socket *next = NULL;

  SIO_TRACE (("_tcp_unthread"));

  if (ds == NULL)
     return (NULL);

  for (s = prev = _tcp_allsocs; s; prev = s, s = s->next)
  {
    if (ds != s)
       continue;

    if (s == _tcp_allsocs)
         _tcp_allsocs = s->next;
    else prev->next   = s->next;
    next = s->next;
    break;
  }

  if (ds->rx_datalen == 0 || (ds->state > tcp_StateESTCL))
      ds->ip_type = 0;             /* fail further I/O */
  ds->state = tcp_StateCLOSED;     /* tcp_tick needs this */

  if (free_tx && ds->tx_data != NULL && ds->tx_data != &ds->tx_buf[0] &&
      *(DWORD*)(ds->tx_data-4) == SAFETY_TCP)
  {
    free (ds->tx_data-4);
    ds->tx_data    = NULL;
    ds->tx_datalen = 0;    /* should already be 0 */
  }
  return (next);
}

/**
 * Returns 1 if TCP connection is established.
 */
int W32_CALL tcp_established (const _tcp_Socket *s)
{
  return (s->state >= tcp_StateESTAB);
}

/**
 * The main TCP input handler.
 *
 * All TCP input processing is done from here.
 */
_tcp_Socket *_tcp_handler (const in_Header *ip, BOOL broadcast)
{
#if defined(USE_IPV6)
  const in6_Header *ip6 = (const in6_Header*)ip;
  ip6_address       ip6_src, ip6_dst;
#endif

  tcp_Header  *tcp;
  _tcp_Socket *s;
  int          len;
  BYTE         flags;
  DWORD        source = intel (ip->source);
  DWORD        destin = intel (ip->destination);
  DWORD        seq;
  WORD         dstPort, srcPort;
  BOOL         is_ip4 = (ip->ver == 4);

  SIO_TRACE (("_tcp_handler"));

  if (is_ip4)
  {
    if (broadcast || block_tcp ||
        !_ip4_is_local_addr(destin) || _ip4_is_multicast(source))
    {
      DEBUG_RX (NULL, ip);
      return (NULL);
    }

    len   = in_GetHdrLen (ip);                /* len of IP header  */
    tcp   = (tcp_Header*) ((BYTE*)ip + len);  /* tcp frame pointer */
    len   = intel16 (ip->length) - len;       /* len of tcp+data */
    flags = tcp->flags & tcp_FlagMASK;        /* get TCP flags */

    if (!tcp_checksum(ip,tcp,len))
    {
      DEBUG_RX (NULL, ip);
      return (NULL);
    }
  }
#if defined(USE_IPV6)
  else
  {
    if (broadcast || block_tcp ||
        !IN6_IS_ADDR_LINKLOCAL(&ip6->destination) ||
         IN6_IS_ADDR_MULTICAST(&ip6->source))
    {
      DEBUG_RX (NULL, ip);
      return (NULL);
    }

    len   = intel16 (ip6->len);
    tcp   = (tcp_Header*) pkt_get_type_in (TYPE_TCP_HEAD)->data;
    flags = tcp->flags & tcp_FlagMASK;
    memcpy (&ip6_dst, &ip6->destination, sizeof(ip6_dst));
    memcpy (&ip6_src, &ip6->source, sizeof(ip6_src));
    if (!_ip6_tcp_checksum(ip6,tcp,len))
    {
      DEBUG_RX (NULL, ip6);
      return (NULL);
    }
  }
#else
  else
  {
    DEBUG_RX (NULL, ip);
    return (NULL);
  }
#endif


  dstPort = intel16 (tcp->dstPort);
  srcPort = intel16 (tcp->srcPort);

  /* demux to active sockets
   */
  for (s = _tcp_allsocs; s; s = s->next)
  {
    if (s->safetysig != SAFETY_TCP || s->safetytcp != SAFETY_TCP)
    {
      outsnl (_LANG("Error in _tcp_handler()"));
      DEBUG_RX (s, ip);
      return (NULL);
    }

    if (s->is_ip6 && !is_ip4)
    {
#if defined(USE_IPV6)
      if (s->hisport                                       &&
          !memcmp(&ip6_dst, &s->my6addr, sizeof(ip6_dst))  &&
          !memcmp(&ip6_src, &s->his6addr, sizeof(ip6_src)) &&
          dstPort == s->myport                             &&
          srcPort == s->hisport)
         break;
#endif
    }
    else if (is_ip4)
    {
      if (s->hisport            &&   /* IP4: not a listening socket */
          destin  == s->myaddr  &&   /* addressed to my IP */
          source  == s->hisaddr &&   /* and from my peer address */
          dstPort == s->myport  &&   /* addressed to my local port */
          srcPort == s->hisport)     /* and from correct remote port */
        break;
    }
  }

  if (!s && (flags & tcp_FlagSYN))
  {
    /* demux to passive (listening) sockets, must be a new session
     */
    for (s = _tcp_allsocs; s; s = s->next)
        if (s->hisport == 0 &&     /* =0, listening socket */
            s->myport  == dstPort) /* addressed to my local port */
        {
          if (s->is_ip6)
          {
#if defined(USE_IPV6)
            if (!is_ip4)
            {
              s->hisaddr = source;
              s->hisport = srcPort;
              memcpy (&s->my6addr, ip6_dst, sizeof(s->my6addr));
            }
#endif
          }
          else if (is_ip4)
          {
            s->hisaddr = source;     /* remember his IP-address */
            s->hisport = srcPort;    /*   and src-port */
            s->myaddr  = destin;     /* socket is now active (should be same) */
          }
          break;
        }
  }

  DEBUG_RX (s, ip);

  if (!s)
  {
    if (!(flags & tcp_FlagRST))                  /* don't answer RST */
    {
      TCP_SEND_RESET (NULL, ip, tcp);
    }
    else if ((flags & tcp_FlagACK) &&            /* got ACK,RS */
             (s = tcp_findseq(ip,tcp)) != NULL)  /* ACK = SEQ + 1 */
    {
      /* e.g. a firewall is sending RST for host on inside.
       */
      tcp_sockreset (s, TRUE);
    }
    return (NULL);
  }

#if defined(USE_TCP_MD5)
  if (is_ip4)
  {
    /* \todo: check in a src-host database and drop if not found
     *        and MD5 fails.
     */
    check_md5_signature (s, ip);
  }
#endif

  /* Restart idle-timer unless overridden by tcp_connect().
   */
  if (sock_inactive && !(s->locflags & LF_RCVTIMEO))
     s->inactive_to = set_timeout (1000 * sock_inactive);

  /* Got a RST with correct SEQ (RCV.NXT - RCV.NXT-RCV.RWIN+1).
   * Reset socket (if socket is a server, go back to listening).
   */
  seq = intel (tcp->seqnum);
  if ((flags & tcp_FlagRST)      &&
      SEQ_GEQ(seq, s->recv_next) &&
      SEQ_LEQ(seq, s->recv_next+s->adv_win+1))
  {
    TCP_TRACE_MSG (("_tcp_handler(): got RST, SEQ %lu\n", (u_long)seq));
    tcp_sockreset (s, FALSE);
    return (NULL);
  }

  if (flags & tcp_FlagPUSH)       /* EE 2002.2.28 */
       s->locflags |= LF_GOT_PUSH;
  else s->locflags &= ~LF_GOT_PUSH;

  tcp_rtt_win (s);        /* update retrans timer, windows etc. */

  if (_tcp_fsm(&s,ip) &&  /* do input tcp state-machine */
      s->unhappy)         /* if "unhappy", retransmit soon */
     TCP_SENDSOON (s);

#if defined(USE_DEBUG)
  _sock_check_tcp_buffers (s);
#endif

  return (s);
}
#endif  /* !USE_UDP_ONLY */


/**
 * Verify checksum for an UDP packet.
 */
static BOOL udp_checksum (const in_Header *ip, const udp_Header *udp, int len)
{
  tcp_PseudoHeader ph = { 0,0,0,0,0,0 };

  ph.src      = ip->source;      /* already network order */
  ph.dst      = ip->destination;
  ph.protocol = UDP_PROTO;
  ph.length   = udp->length;
  ph.checksum = CHECKSUM (udp, len);

  if (CHECKSUM(&ph,sizeof(ph)) != 0xFFFF)
  {
    TCP_CONSOLE_MSG (1, (_LANG("Bad udp checksum. ")));
    TCP_TRACE_MSG (("Bad udp checksum\n"));
    STAT (udpstats.udps_badsum++);
    return (FALSE);
  }
  return (TRUE);
}


/**
 * Demultiplexer for incoming UDP packets.
 * Don't debug packet if no match was found (except if '*udp_err').
 */
static _udp_Socket *udp_demux (const in_Header *ip, BOOL ip_bcast,
                               DWORD destin, WORD srcPort, WORD dstPort,
                               BOOL *udp_err)
{
  _udp_Socket *s;
  BOOL is_ip4 = (ip->ver == 4);

#if defined(USE_IPV6)
  const in6_Header *ip6 = (const in6_Header*)ip;
  ip6_address       ip6_src, ip6_dst;

  if (!is_ip4)
  {
    memcpy (&ip6_dst, &ip6->destination, sizeof(ip6_dst));
    memcpy (&ip6_src, &ip6->source, sizeof(ip6_src));
  }
  else
  {
    /*
     * To supress GCC 5.x warning:
     *   pctcp.c:951:14: warning: 'ip6_dst' may be used uninitialized in this function [-Wmaybe-uninitialized]
     *                memcpy (&s->my6addr, &ip6_dst, sizeof(s->my6addr));
     */
    memset (&ip6_dst, 0, sizeof(ip6_dst));
  }
#endif

  *udp_err = FALSE;   /* assume socket-list is OK */

  /** Demux to active sockets.
   * \todo use some kind of hashing to speed up the search.
   */
  for (s = _udp_allsocs; s; s = s->next)
  {
    if (s->safetysig != SAFETY_UDP)
    {
      outsnl (_LANG("Error in udp_demux()"));
      s->safetysig = SAFETY_UDP;
      DEBUG_RX (s, ip);
      *udp_err = TRUE;
      return (NULL);
    }

    if (!ip_bcast              &&
        (s->hisport != 0)      &&
        (dstPort == s->myport) &&
        (srcPort == s->hisport))
    {
      if (s->is_ip6)
      {
#if defined(USE_IPV6)
        if (!is_ip4 &&
            IN6_ARE_ADDR_EQUAL(&ip6_src,&s->his6addr))  /* !!mask */
        {
          DEBUG_RX (s, ip);
          break;
        }
#endif
      }
      else if (is_ip4 &&
               ((destin & sin_mask) == (s->myaddr & sin_mask)) &&
               (intel(ip->source)   == s->hisaddr))
      {
        DEBUG_RX (s, ip);
        break;
      }
    }
  }

  if (!s)
  {
    /* demux to passive sockets
     */
    for (s = _udp_allsocs; s; s = s->next)
    {
      if (s->is_ip6)
      {
#if defined(USE_IPV6)
        if (!is_ip4 && dstPort == s->myport &&
            IN6_IS_ADDR_UNSPECIFIED(&s->his6addr))
        {
          DEBUG_RX (s, ip);

          memcpy (&s->his6addr, &ip6_src, sizeof(s->his6addr));
          s->hisport = srcPort;
          SET_PEER_MAC_ADDR (s, ip6);
          if (!ip_bcast)
             memcpy (&s->my6addr, &ip6_dst, sizeof(s->my6addr));
          break;
        }
#endif
      }
      else if (is_ip4 && (s->hisaddr == 0 || s->hisaddr == IP_BCAST_ADDR) &&
               dstPort == s->myport)
      {
        DEBUG_RX (s, ip);

        if (s->hisaddr == 0)
        {
          s->hisaddr = intel (ip->source);  /* socket now active */
          s->hisport = srcPort;
          SET_PEER_MAC_ADDR (s, ip);

          /* take on value of expected destination
           * unless it is broadcast
           */
          if (!ip_bcast)
             s->myaddr = destin;
        }
        break;
      }
    }
  }


#if defined(USE_MULTICAST)
  if (is_ip4 && !s)
  {
    /* demux to multicast sockets
     */
    for (s = _udp_allsocs; s; s = s->next)
    {
      if (s->hisport != 0         &&
          s->hisaddr == destin    &&
          dstPort    == s->myport &&
          _ip4_is_multicast(destin))
      {
        DEBUG_RX (s, ip);
        break;
      }
    }
  }
#endif

  if (!s)
  {
    /* Demux to broadcast sockets.
     */
    for (s = _udp_allsocs; s; s = s->next)
    {
      if (s->is_ip6 && !is_ip4)
      {
#if defined(USE_IPV6)
        if (dstPort == s->myport && IN6_IS_ADDR_UNSPECIFIED(&s->his6addr))
        {
          DEBUG_RX (s, ip);
          break;
        }
#endif
      }
      else if (is_ip4)
      {
        if (s->hisaddr == IP_BCAST_ADDR && dstPort == s->myport)
        {
          DEBUG_RX (s, ip);
          break;
        }
      }
    }
  }
  return (s);
}


/**
 * Handler for incoming UDP packets.
 */
_udp_Socket *_udp_handler (const in_Header *ip, BOOL broadcast)
{
#if defined(USE_IPV6)
  const in6_Header *ip6 = (const in6_Header*)ip;
  ip6_address       ip6_src, ip6_dst;
#endif

  _udp_Socket *s;
  UINT         len = 0;
  WORD         dst_port, src_port;
  DWORD        destin = 0;
  BOOL         is_ip4 = (ip->ver == 4);
  BOOL         ip_bcast = 0;
  BOOL         udp_err;

  const BYTE       *data;
  const udp_Header *udp = NULL;

  SIO_TRACE (("_udp_handler"));

  if (is_ip4)
  {
    destin   = intel (ip->destination);
    ip_bcast = broadcast ||             /* link-layer broadcast */
               _ip4_is_ip_brdcast(ip);  /* (directed) ip-broadcast */
  }
#if defined(USE_IPV6)
  else
  {
    memcpy (&ip6_dst, &ip6->destination, sizeof(ip6_dst));
    memcpy (&ip6_src, &ip6->source, sizeof(ip6_src));
    ip_bcast = broadcast ||
               IN6_IS_ADDR_MULTICAST (&ip6->destination); /* !! */
  }
#endif

#if !defined(USE_MULTICAST)

  /* dst = ip number
   *     or 255.255.255.255
   *     or sin_mask.255.255
   * This is the only really gross hack in the multicasting stuff.
   * I'll fix it as soon as I can figure out what I want to do here.
   * -JRM 8/1/93
   */
  if (is_ip4 && !ip_bcast &&            /* not a IPv4 broadcast packet */
      my_ip_addr &&                     /* and I know my address */
      !_ip4_is_multihome_addr(destin))  /* and not my address */
  {
    DEBUG_RX (NULL, ip);
    STAT (udpstats.udps_noport++);
    return (NULL);
  }
#endif

  if (is_ip4)
  {
    len = in_GetHdrLen (ip);
    udp = (const udp_Header*) ((BYTE*)ip + len);   /* udp segment pointer */
    len = intel16 (udp->length);
  }
#if defined(USE_IPV6)
  else
  {
    len = intel16 (ip6->len);
    udp = (const udp_Header*) pkt_get_type_in (TYPE_UDP_HEAD)->data;
    len = intel16 (udp->length);
  }
#endif

  if (len < sizeof(*udp))
  {
    DEBUG_RX (NULL, ip);
    STAT (udpstats.udps_hdrops++);
    return (NULL);
  }

  src_port = intel16 (udp->srcPort);
  dst_port = intel16 (udp->dstPort);

  if (dst_port == 0 || src_port == 0)
  {
    DEBUG_RX (NULL, ip);
    return (NULL);
  }

  s = udp_demux (ip, ip_bcast, destin, src_port, dst_port, &udp_err);

  if (!s)              /* no demultiplexer found anything */
  {
    if (udp_err)       /* error in udp_allsocs, don't send ICMP */
       return (NULL);

    DEBUG_RX (NULL, ip);

#if defined(USE_IPV6)
    if (!is_ip4 && !IN6_IS_ADDR_UNSPECIFIED(&in6addr_my_ip) &&
        !memcmp(&in6addr_my_ip,&ip6_dst,sizeof(in6addr_my_ip)))
    {
      if (!ip_bcast && src_port != DOM_DST_PORT)
         icmp6_unreach (ip6, 3);

      if (ip_bcast)
           STAT (udpstats.udps_noportbcast++);
      else STAT (udpstats.udps_noport++);
    }
    else
#endif
    if (is_ip4 && my_ip_addr && _ip4_is_multihome_addr(destin))
    {
      if (!ip_bcast &&               /* not broadcast */
          src_port != DOM_DST_PORT)  /* not a late reply from a nameserver */
         icmp_send_unreach (ip, ICMP_UNREACH_PORT);

      if (ip_bcast)
           STAT (udpstats.udps_noportbcast++);
      else STAT (udpstats.udps_noport++);
    }
    return (NULL);
  }

  if (is_ip4)
  {
    if (udp->checksum && (s->sockmode & SOCK_MODE_UDPCHK) &&
        !udp_checksum (ip,udp,len))
       return (s);
  }
#if defined(USE_IPV6)
  else
  {
    if (_ip6_udp_checksum(ip6,udp,len)) /* checksums are compulsary */
       return (s);
  }
#endif


  /* Process user data. 0-byte probe is legal for s->protoHandler.
   */
  data = (const BYTE*) (udp+1);
  len -= sizeof(*udp);

  if (s->protoHandler)
  {
    if (is_ip4)
    {
      tcp_PseudoHeader ph = { 0,0,0,0,0,0 };
      ph.src = ip->source;  /* only source needed by protoHandler */
      (*s->protoHandler) (s, data, len, &ph, udp);
    }
#if defined(USE_IPV6)
    else
    {
      tcp_PseudoHeader6 ph6;
      memset (&ph6, 0, sizeof(ph6));
      memcpy (&ph6.src, &ip6->source, sizeof(ph6.src));
      (*s->protoHandler) (s, data, len, &ph6, udp);
    }
#endif
  }
  else if (len > 0 /* && s->rx_datalen == 0 !! overwrite current data */)
  {
    if (len > s->max_rx_data)   /* truncate data :-( */
    {
      len = s->max_rx_data;
      STAT (udpstats.udps_fullsock++);
    }
    memcpy (s->rx_data, data, len);
    s->rx_datalen = len;
  }
  return (s);
}


#if !defined(USE_UDP_ONLY)
/**
 * Called periodically to perform retransmissions.
 * \arg if 'force == 1' do it now.
 */
void tcp_Retransmitter (BOOL force)
{
  _tcp_Socket *s, *next;

  static DWORD timeout = 0UL;

  SIO_TRACE (("tcp_Retransmitter"));

  /* do this once per tcp_RETRAN_TIME
   */
  if (!force && timeout && !chk_timeout(timeout))
     return;

  timeout = set_timeout (tcp_RETRAN_TIME);

  for (s = _tcp_allsocs; s; s = next)
  {
    next = s->next;

    /* Check on ARP resolve status, GvB 2002-09
     */
    if (s->state == tcp_StateRESOLVE)
    {
      if (_arp_lookup(s->hisaddr, &s->his_ethaddr))   /* Success */
      {
        UINT rtt, MTU;

        s->state   = tcp_StateSYNSENT;
        s->timeout = set_timeout (tcp_LONGTIMEOUT);
        TCP_SEND (s);  /* send opening SYN */

        /* find previous RTT replacing RTT set in tcp_send() above
         */
        if (tcp_rtt_get(s, &rtt, &MTU))
             s->rtt_time = set_timeout (rtt);
        else s->rtt_time = set_timeout (tcp_OPEN_TO);
      }

      /* If ARP no longer pending (timed out), hence we abort
       */
      else if (!_arp_lookup_pending(s->hisaddr))    /* ARP timed out */
      {
        tcp_no_arp (s);
        next = TCP_ABORT (s);
      }
      continue;    /* don't do anything more on this TCB */
    }

    /* possible to be closed with Rx-data still queued
     */
    if (s->state == tcp_StateCLOSED)
    {
      if (s->rx_datalen == 0)
      {
        maybe_reuse_localport (s);
        next = _tcp_unthread (s, TRUE);
      }
      continue;
    }

    /* Need to send a window update? Because we advertised a 0 window
     * in a previous _tcp_send() (but only in ESTAB state).
     */
    if ((s->locflags & LF_WINUPDATE) && sock_rbleft((sock_type*)s) > 0)
    {
      STAT (tcpstats.tcps_sndwinup++);
      s->locflags &= ~LF_WINUPDATE;
      s->flags |= tcp_FlagACK;
      TCP_SEND (s);
    }

    else if (s->tx_datalen > 0 || s->unhappy || s->karn_count == 1)
    {
      if (chk_timeout(s->rtt_time))  /* retransmission timeout */
      {
        s->rtt_time = 0UL;           /* stop RTT timer */

        TCP_CONSOLE_MSG (3, ("Regular retran TO set unacked back to "
                             "0 from %ld\n", s->send_una));

        /* strategy handles closed windows.  JD + EE
         */
        if (s->window == 0 && s->karn_count == 2)
            s->window = 1;

        if (s->karn_count == 0)
        {
          /* Simple "Slow start" algorithm:
           * Use the backed off RTO - implied, no code necessary.
           * Reduce the congestion window by 25%
           */
          unsigned cwindow = ((unsigned)(s->cwindow + 1) * 3) >> 2;

          s->cwindow = (BYTE) cwindow;
          if (s->cwindow == 0)
              s->cwindow = 1;

          s->wwindow = 0;       /**< dup ACK counter ? */
#if 0
          /**< \todo set "Slow-start" threshold */
          s->send_ssthresh = s->cwindow * s->max_seg;
#endif
          /* if really did timeout
           */
          s->karn_count = 2;
          s->send_una   = 0;
        }

        if (s->tx_datalen > 0)
        {
          s->flags |= (tcp_FlagPUSH | tcp_FlagACK);
          if (s->cwindow > 1)
             s->cwindow--;
        }

        if (s->unhappy)
           STAT (tcpstats.tcps_rexmttimeo++);  /* Normal re-xmit */
        else if (s->flags & tcp_FlagACK)
           STAT (tcpstats.tcps_delack++);

        TCP_SEND (s);

        if (s->state == tcp_StateSYNSENT)
        {
          /**< \todo Allow for 3 SYN before giving up
           */
          TCP_TRACE_MSG (("SYN (re)sent in tcp_Retransmitter(), "
                          " rtt_time %ld\n",
                          get_timediff(s->rtt_time,set_timeout(0))));
        }
      }

      /* handle inactive tcp timeouts (not sending data)
       */
      else if (chk_timeout(s->datatimer))  /* EE 99.08.23 */
      {
        next = TCP_ABORT (s);
        s->datatimer = 0UL;
        s->err_msg   = _LANG ("Connection timed out - no data sent");
      }
    }  /* end of retransmission strategy */


    /* handle inactive TCP timeouts (not received anything)
     */
    if (chk_timeout(s->inactive_to))
    {
      /* this baby has timed out. Don't do this again.
       */
      s->inactive_to = 0UL;
      s->err_msg     = _LANG ("Timeout, nothing received");
      sock_close ((sock_type*)s);
    }
    else if (chk_timeout(s->timeout))
    {
      if (s->state == tcp_StateTIMEWT)
      {
        s->state = tcp_StateCLOSED;
        break;
      }
      if (s->state != tcp_StateESTAB && s->state != tcp_StateESTCL)
      {
        next = TCP_ABORT (s);
        s->err_msg = _LANG ("Timeout, aborting");
        break;
      }
    }
  }
}
#endif /* !USE_UDP_ONLY */


/**
 * \b Must be called periodically by user application (or BSD socket API).
 * \arg if 's != NULL', check this socket for timeout.
 * \arg if 's == NULL', check all sockets in list.
 *
 * \retval 0   if 's' is non-NULL and 's' closes.
 * \retval !0  if 's' is NULL or 's' is still open.
 */
WORD W32_CALL tcp_tick (sock_type *s)
{
  static int tick_active = 0;

  if (tick_active > 0)
  {
    TCP_CONSOLE_MSG (1, ("tcp_tick() reentered\n"));
    return (s ? s->tcp.ip_type : 0);
  }
  tick_active++;

  SIO_TRACE (("tcp_tick"));

#if !defined(USE_UDP_ONLY)
  /*
   * Finish off dead sockets
   */
  if (s)
  {
    if (s->tcp.ip_type    == TCP_PROTO       &&
        s->tcp.state      == tcp_StateCLOSED &&
        s->tcp.rx_datalen == 0)
    {
      _tcp_unthread (&s->tcp, TRUE);
      s->tcp.ip_type = 0;   /* fail further I/O */
    }
  }
#endif

  /**
   * Don't enter this loop if reentered. That could return the same
   * packet twice (before we call _eth_free() on the 1st packet).
   * \todo Limit the time spent here (clamp # of loops)
   */
  while (tick_active == 1)
  {
    WORD  eth_type = 0;
    BOOL  brdcast  = FALSE;
    void *packet   = _eth_arrived (&eth_type, &brdcast);

    if (!packet)  /* packet points to network layer protocol */
       break;

    switch (eth_type)
    {
      case IP4_TYPE:
           _ip4_handler ((in_Header*)packet, brdcast);
           break;

      case ARP_TYPE:
           _arp_handler ((arp_Header*)packet, brdcast);
           break;
#if 0
      case RARP_TYPE:
           _rarp_handler ((rarp_Header*)packet, brdcast);
           break;
#endif

#if defined(USE_IPV6)
      case IP6_TYPE:
           _ip6_handler ((in6_Header*)packet, brdcast);
           break;
#endif

#if defined(USE_PPPOE)
      case PPPOE_DISC_TYPE:
      case PPPOE_SESS_TYPE:
           pppoe_handler ((pppoe_Packet*)packet);
           break;
#endif

      default:
#if defined(USE_DEBUG)
           dbug_printf ("\n%s (%d): Unhandled Rx packet, type %04X, "
                        "broadcast %d\n", __FILE__, __LINE__,
                        intel16(eth_type), brdcast);
#endif
           break;

           /* RARP is only used during boot. Not needed here */
    }
    _eth_free (packet);
  }

#if !defined(USE_UDP_ONLY)
  tcp_Retransmitter (FALSE); /* check for our outstanding packets */
#endif

  daemon_run();    /* check and optionally run background processes */

  --tick_active;
  return (s ? s->tcp.ip_type : 0);
}

/**
 * Write an UDP packet.
 *
 * \note Assumes data fits in one datagram, else only the first fragment
 *       will be sent!  Because MTU is used for splits, the guaranteed
 *       max data size is 'MTU - UDP_OVERHEAD = 548' for a non-fragmented
 *       datagram. Routers should handle at least 576 bytes datagrams.
 */
static int udp_write (_udp_Socket *s, const BYTE *data, int len)
{
  struct ip4_packet *ip4_pkt = NULL;

#if defined(USE_IPV6)
  struct ip6_packet *ip6_pkt = NULL;
  struct in6_Header *ip6     = NULL;
#endif

  mac_address *dst = (_pktserial ? NULL : &s->his_ethaddr);
  in_Header   *ip4 = NULL;
  udp_Header  *udp;

  SIO_TRACE (("udp_write"));

#if defined(USE_IPV6)
  if (s->is_ip6)
  {
    ip6_pkt = (struct ip6_packet*) _eth_formatpacket (dst, IP6_TYPE);
    ip6     = &ip6_pkt->in;
    udp     = &ip6_pkt->udp;
  }
  else
#endif
  {
    ip4_pkt = (struct ip4_packet*) _eth_formatpacket (dst, IP4_TYPE);
    ip4     = &ip4_pkt->in;
    udp     = &ip4_pkt->udp;
  }

  /* build udp header
   */
  udp->srcPort  = intel16 (s->myport);
  udp->dstPort  = intel16 (s->hisport);
  udp->checksum = 0;
  udp->length   = intel16 (sizeof(*udp)+len);

#if defined(USE_IPV6)
  if (s->is_ip6)
  {
    memcpy (&ip6->source, &in6addr_my_ip, sizeof(ip6->source));
    memcpy (&ip6->destination, &s->his6addr, sizeof(ip6->destination));
    memcpy (ip6_pkt+1, data, len);   /* copy 'data' to 'ip6_pkt->data[]' */
    udp->checksum = _ip6_checksum (ip6, IP6_NEXT_UDP, udp, sizeof(*udp)+len);
    if (!IP6_OUTPUT(ip6, &ip6->source, &ip6->destination, IP6_NEXT_UDP,
                    sizeof(*udp)+len, 0, s))
       return (-1);
    return (len);
  }
  else
#endif
  {
    tcp_PseudoHeader ph;

    memset (&ph, 0, sizeof(ph));
    ph.src = intel (s->myaddr);
    ph.dst = intel (s->hisaddr);
    if (len > 0)
       memcpy (ip4_pkt+1, data, len);  /* copy 'data' to 'ip4_pkt->data[]' */

    if (s->sockmode & SOCK_MODE_UDPCHK)
    {
      ph.protocol = UDP_PROTO;
      ph.length   = udp->length;
      ph.checksum = CHECKSUM (udp, sizeof(*udp)+len);
      udp->checksum = ~CHECKSUM (&ph, sizeof(ph));
    }
    if (!IP4_OUTPUT(ip4, ph.src, ph.dst, UDP_PROTO, s->ttl,
                    (BYTE)_default_tos, 0, sizeof(*udp)+len, s))
       return (-1);
    return (len);
  }
}

/**
 * Read from UDP socket.
 * \arg \b buf where data is put.
 * \arg \b maxlen read max this amount.
 * \note does large buffering.
 */
static int udp_read (_udp_Socket *s, BYTE *buf, int maxlen)
{
  int len = s->rx_datalen;

  SIO_TRACE (("udp_read"));

  if (maxlen < 0)
      maxlen = INT_MAX;

  if (len > maxlen)
      len = maxlen;

  if (len > 0)
  {
    if (buf)
       memcpy (buf, s->rx_data, len);
    s->rx_datalen -= len;
    if (s->rx_datalen > 0)
       memmove (s->rx_data, s->rx_data+len, s->rx_datalen);
  }
  return (len);
}

/**
 * Reduce socket MSS upon receiving ICMP_UNREACH_NEEDFRAG.
 * 'MTU' is next-hop suggested MTU or 0 if router doesn't support RFC-1191.
 */
static void sock_reduce_mss (sock_type *s, WORD MTU)
{
  if (s->tcp.ip_type == TCP_PROTO)
  {
#if !defined(USE_UDP_ONLY)
    _tcp_Socket *tcp = &s->tcp;

    if (MTU)
         tcp->max_seg = _mtu - TCP_OVERHEAD;
    else if (tcp->max_seg > MSS_MIN)
         tcp->max_seg -= MSS_REDUCE;
    tcp->max_seg = min (max(MSS_MIN, tcp->max_seg), _mss);

    TCP_TRACE_MSG (("MSS for %s reduced to %u\n",
                    _inet_ntoa(NULL,tcp->hisaddr), tcp->max_seg));
#endif
  }
  else
  {
    /* doesn't use MSS yet (always MTU-28) */
  }
  ARGSUSED (MTU);
}

/**
 * Cancel an UDP socket.
 * Called upon receiving ICMP errors: REDIRECT, TIMXCEED, PARAMPROB,
 * or host/port UNREACH. See icmp_redirect() or icmp_handler().
 */
void _udp_cancel (const in_Header *ip, int icmp_type, int icmp_code,
                  const char *msg, const void *arg) /* use a var-arg here ? */
{
  WORD         src_port, dst_port;
  BOOL         passive = FALSE;
  int          len     = in_GetHdrLen (ip);
  udp_Header  *udp     = (udp_Header*) ((BYTE*)ip + len);
  _udp_Socket *s;

  SIO_TRACE (("_udp_cancel"));

  src_port = intel16 (udp->srcPort);
  dst_port = intel16 (udp->dstPort);

  for (s = _udp_allsocs; s; s = s->next)  /* demux to active sockets */
  {
    if (s->hisport > 0 &&
        dst_port == s->hisport && src_port == s->myport &&
        intel(ip->destination) == s->hisaddr)
       break;
  }

  if (!s)       /* check passive sockets */
  {
    passive = TRUE;
    for (s = _udp_allsocs; s; s = s->next)
        if (s->hisport == 0 && dst_port == s->myport)
           break;
  }

  if (s)
  {
    SET_ERR_MSG (s, msg);

    /* handle ICMP-errors on active sockets
     */
    if (icmp_type == ICMP_REDIRECT && !passive)
    {
      DWORD gateway;

      WATT_ASSERT (arg != NULL);
      gateway = *(DWORD*)arg;
      _ip_recursion = 1;
      _arp_resolve (gateway, &s->his_ethaddr);
      _ip_recursion = 0;
    }
    else if (icmp_type == ICMP_UNREACH && icmp_code == ICMP_UNREACH_NEEDFRAG && !passive)
    {
      WORD next_mtu;

      WATT_ASSERT (arg != NULL);
      next_mtu = *(WORD*) arg;
      sock_reduce_mss ((sock_type*)s, next_mtu);
    }
    else if (icmp_type != ICMP_TIMXCEED && !passive)
    {
      /* UDP isn't sturdy, close it on 1st ICMP error
       */
      SET_ERR_MSG (s, _LANG("Port unreachable"));
      s->ip_type   = 0;
      s->locflags |= LF_GOT_ICMP;
      udp_close (s);
    }
    if (s->icmp_callb)         /* tell the socket layer about it */
      (*s->icmp_callb) (s, icmp_type, icmp_code);
  }
  else
  {
    /* tell the INADDR_ANY sockets about it
     */
    for (s = _udp_allsocs; s; s = s->next)
    {
      if (s->icmp_callb)
        (*s->icmp_callb) (s, icmp_type, icmp_code);
    }
  }
}

#if !defined(USE_UDP_ONLY)
/*
 * Default is to terminate socket on receiving 2nd
 * ICMP error. Use sock_sturdy() to change.
 */
static BOOL tcp_stress_test (_tcp_Socket *s, const char *msg)
{
  if (s->stress++ > s->rigid && s->rigid < 100)  /* halt it */
  {
    SET_ERR_MSG (s, msg);
    s->rx_datalen = 0;
    s->tx_datalen = 0;
    s->unhappy    = FALSE;
    s->locflags  |= LF_GOT_ICMP;
    return (FALSE);
  }
  return (TRUE);
}

/**
 * Cancel a TCP socket.
 * Called upon receiving ICMP errors: host/port UNREACH, REDIRECT,
 * SOURCEQUENCH, TIMXCEED or PARAMPROB. See icmp_redirect() or
 * icmp_handler().
 *
 * Note: 'ip' is a small copy of the IP-packet originating the ICMP message.
 *       We may *not* have received a copy of the original TCP-header.
 *       But we probably won't do any harm if got less TCP-header data
 *       than needed. I.e. src_port/dst_port will be zero.
 */
void _tcp_cancel (const in_Header *ip, int icmp_type, int icmp_code,
                  const char *msg, const void *arg) /* use a var-arg here ? */
{
  tcp_Header  *tcp = (tcp_Header*) ((BYTE*)ip + in_GetHdrLen (ip));
  _tcp_Socket *s;
  DWORD  gateway;
  WORD   next_mtu;
  WORD   src_port = intel16 (tcp->srcPort);
  WORD   dst_port = intel16 (tcp->dstPort);

  SIO_TRACE (("_tcp_cancel"));

  /* demux to active sockets (passive cannot get ICMP)
   */
  for (s = _tcp_allsocs; s; s = s->next)
  {
    if (src_port != s->myport || dst_port != s->hisport ||
        intel(ip->destination) != s->hisaddr)
       continue;

    switch (icmp_type)
    {
      case ICMP_TIMXCEED:
           if (s->ttl < 255)
               s->ttl++;
           if (tcp_stress_test(s,msg))
              goto quench_it;
           TCP_ABORT (s);
           break;

      case ICMP_UNREACH:
           if (icmp_code == ICMP_UNREACH_NEEDFRAG)
           {
             WATT_ASSERT (arg != NULL);
             next_mtu = *(WORD*) arg;

             /* This should never happen since we normally don't send
              * with "Don't Fragment" set. As an optimisation we should
              * maybe reduce 'max_seg' for all sockets to this destination.
              */
             sock_reduce_mss ((sock_type*)s, next_mtu);
             break;
           }
           /* Other types are "fatal"
            */
           if (!tcp_stress_test(s,msg))
           {
             TCP_ABORT (s);
             break;
           }
           /* FALLTHROUGH */

      case ICMP_SOURCEQUENCH:
      quench_it:
           s->cwindow = 1;       /* slow-down tx-rate */
           s->wwindow = 1;
           s->vj_sa <<= 2;
           s->vj_sd <<= 2;
           s->rto   <<= 2;
           tcp_rtt_add (s, s->rto, _mtu);
           break;

      case ICMP_REDIRECT:
           /* don't bother handling redirect if we're closing
            */
           if (s->state < tcp_StateFINWT1)
           {
             WATT_ASSERT (arg != NULL);
             gateway = *(DWORD*)arg;
             SET_ERR_MSG (s, msg);
             _ip_recursion = 1;
             _arp_resolve (gateway, &s->his_ethaddr);
             _ip_recursion = 0;
           }
           break;

      case ICMP_PARAMPROB:
           SET_ERR_MSG (s, msg);
           TCP_ABORT (s);
           break;
    }
    if (s->icmp_callb)
      (*s->icmp_callb) (s, icmp_type, icmp_code);  /* Notify BSD-socket */

    return;  /* One ICMP message should only match one TCP socket */
  }
}

/**
 * Read from a TCP socket.
 * \arg  \b buf where to put data.
 * \note does large buffering.
 */
static int tcp_read (_tcp_Socket *s, BYTE *buf, int maxlen)
{
  int len;

  SIO_TRACE (("tcp_read"));

  if (maxlen < 0)
      maxlen = INT_MAX;

  len = min (maxlen, s->rx_datalen);

  if (len > 0)
  {
    int to_move;

    if (buf)
       memcpy (buf, s->rx_data, len);
    s->rx_datalen -= len;

    to_move = s->rx_datalen;
    if (s->missed_seq[0] != s->missed_seq[1])
       to_move += s->missed_seq[1] - s->recv_next;

    if (to_move > 0)
    {
      memmove (s->rx_data, s->rx_data + len, to_move);

      TCP_SENDSOON (s);     /* delayed ACK */
    }
    else
      tcp_upd_win (s, __LINE__);
  }
  else if (s->state == tcp_StateCLOSWT)
          _tcp_close (s);

  /* Added new EOF condition (GV, 11-Oct-2003)
   */
  if (len == 0 && (s->locflags & LF_GOT_FIN))
     return (-1);
  return (len);
}

/**
 * Write data to a TCP connection.
 * \retval >0  Number of bytes written.
 * \retval 0   Connection is not established.
 * \retval -1  Error in lower layer.
 */
static int tcp_write (_tcp_Socket *s, const BYTE *data, UINT len)
{
  UINT room;

  SIO_TRACE (("tcp_write"));

  if (s->state != tcp_StateESTAB)
     return (0);

  room = s->max_tx_data - s->tx_datalen - 1;
  if (len > room)
      len = room;
  if (len > 0)
  {
    int rc = 0;

#if defined(USE_DEBUG)
    _sock_check_tcp_buffers (s);
#endif

    memcpy (s->tx_data + s->tx_datalen, data, len);
    s->tx_datalen += len;

    s->unhappy = TRUE;         /* redundant because we have outstanding data */
    if (sock_data_timeout)
         s->datatimer = set_timeout (1000*sock_data_timeout); /* EE 99.08.23 */
    else s->datatimer = 0;

    if (s->sockmode & SOCK_MODE_LOCAL) /* queue up data, flush on next write */
    {
      s->sockmode &= ~SOCK_MODE_LOCAL;
      return (len);
    }

    if (!(s->sockmode & SOCK_MODE_NAGLE))   /* Nagle mode off */
       rc = TCP_SEND (s);
    else
    {
      /* Transmit if first segment or reached min (socket_MSS,MSS).
       */
      if (s->tx_datalen == len ||
          s->tx_datalen >= min(s->max_seg,_mss))
           rc = TCP_SEND (s);
      else rc = TCP_SENDSOON (s);
    }
    if (rc < 0)
       return (-1);
  }
  return (len);
}


/**
 * Find the TCP socket that matches the tripplet:
 * \c DESTADDR=MYADDR, \c DESTPORT=MYPORT and \c ACKNUM=SEQNUM+1
 * Can only be one socket.
 */
static _tcp_Socket *tcp_findseq (const in_Header *ip, const tcp_Header *tcp)
{
  _tcp_Socket *s;
  DWORD        dst_host = intel (ip->destination);
  DWORD        ack_num  = intel (tcp->acknum);
  WORD         dst_port = intel16 (tcp->dstPort);

  SIO_TRACE (("tcp_findseq"));

  for (s = _tcp_allsocs; s; s = s->next)
  {
    if (s->hisport != 0       &&
        dst_host == s->myaddr &&
        dst_port == s->myport &&
        ack_num  == s->send_next+1)
      break;
  }
  return (s);
}

/**
 * Resets a TCP connection.
 */
static void tcp_sockreset (_tcp_Socket *s, BOOL proxy)
{
  const char *str = proxy ? "Proxy reset connection"
                          : "Remote reset connection";

  SIO_TRACE (("tcp_sockreset"));

  if (debug_on)
     outsnl (_LANG(str));

  if (s->state == tcp_StateSYNREC)
       STAT (tcpstats.tcps_conndrops++); /* embryonic connection drop */
  else STAT (tcpstats.tcps_drops++);

  if (s->state != tcp_StateCLOSED && s->state != tcp_StateLASTACK)
      s->rx_datalen = 0;

  s->missed_seq[0] = s->missed_seq[1] = 0; /* discard buffered out-of-order */

  s->tx_datalen = 0;   /* Empty Tx buffer */

  if (s->locflags & LF_IS_SERVER)
  {
    CLR_PEER_MAC_ADDR (s);
    s->cwindow  = 1;
    s->rtt_time = 0UL;
    s->unhappy  = FALSE;
    s->hisport  = 0;
    s->hisaddr  = 0UL;
    s->locflags &= ~(LF_WINUPDATE | LF_KEEPALIVE | LF_GOT_FIN | LF_GOT_ICMP);

    s->datatimer   = 0UL;
    s->inactive_to = 0;
    s->timeout     = 0;
    s->karn_count  = 0;
    s->state       = tcp_StateLISTEN;
    TCP_TRACE_MSG (("tcp_sockreset(): continue to LISTEN\n"));
  }
  else
  {
    s->err_msg = _LANG (str);
    s->state   = tcp_StateCLOSED;
    s->ip_type = 0;      /* 2001.1.18 - make it fail tcp_tick() */

#if defined(USE_BSD_API)
    if (_bsd_socket_hook)
      (*_bsd_socket_hook) (BSO_RST_CALLBACK, s);
#endif
    _tcp_unthread (s, TRUE);
  }
}

/**
 * Called for a TCP socket when ARP lookup fails.
 * If socket allocated by BSD API, do a read wakeup on it.
 */
static void tcp_no_arp (_tcp_Socket *s)
{
  SIO_TRACE (("tcp_no_arp"));

  s->err_msg = _LANG ("No ARP reply");
  STAT (ip4stats.ips_noroute++);

#if defined(USE_BSD_API)
  if (_bsd_socket_hook)
  {
    Socket *sock = (Socket*) (*_bsd_socket_hook) (BSO_FIND_SOCK, s);

    if (sock)  /* do a "read-wakeup" on the SOCK_STREAM socket */
    {
      sock->so_state |= SS_ISDISCONNECTING;
      sock->so_error = EHOSTUNREACH; /* !! Or ETIMEDOUT? */
    }
  }
#endif
}

/**
 * Verify the TCP header checksum.
 * \note Not used for IPv6.
 */
static BOOL tcp_checksum (const in_Header *ip, const tcp_Header *tcp, int len)
{
  tcp_PseudoHeader ph = { 0,0,0,0,0,0 };

  SIO_TRACE (("tcp_checksum"));

  ph.src      = ip->source;
  ph.dst      = ip->destination;
  ph.protocol = TCP_PROTO;
  ph.length   = intel16 (len);
  ph.checksum = CHECKSUM (tcp, len);

  if (CHECKSUM(&ph,sizeof(ph)) != 0xFFFF)
  {
    STAT (tcpstats.tcps_rcvbadsum++);
    TCP_CONSOLE_MSG (1, (_LANG("Bad tcp checksum. ")));
    TCP_TRACE_MSG (("Bad tcp checksum\n"));
    return (FALSE);
  }
  return (TRUE);
}

/**
 * Update retransmission timer, VJ algorithm and TCP windows.
 * Only called after we received something on socket.
 */
static void tcp_rtt_win (_tcp_Socket *s)
{
  DWORD timeout;

  SIO_TRACE (("tcp_rtt_win"));

  /* update our retransmission stuff (Karn algorithm)
   */
  if (s->karn_count == 2)    /* Wake up from slow-start */
  {
    TCP_CONSOLE_MSG (2, ("Finally got it safely zapped from %ld to ???\n",
                     s->send_una));
  }
  else if (s->vj_last)     /* We expect an immediate response */
  {
    long  dT;      /* Time (msec) since last (re)transmission */
    DWORD now;

    chk_timeout (0UL);          /* Update date/date_ms */
    now = set_timeout (0);

#if 0 /** \todo Use \b TimeStamp option values */
    if (s->ts_echo && s->ts_echo >= s->ts_sent)
       dT = get_timediff (s->ts_echo, s->ts_sent) >> 1;
    else
#else
       dT = get_timediff (now, s->vj_last);
#endif

    if (dT >= 0)
    {
      dT -= (DWORD)(s->vj_sa >> 3);
      s->vj_sa += dT;

      if (dT < 0)
          dT = -dT;

      dT -= (s->vj_sd >> 2);
      s->vj_sd += dT;               /* vj_sd = RTTVAR, rtt variance */

      if (s->vj_sa > tcp_MAX_VJSA)  /* vj_sa = SRTT, smoothed RTT */
          s->vj_sa = tcp_MAX_VJSA;
      if (s->vj_sd > tcp_MAX_VJSD)
          s->vj_sd = tcp_MAX_VJSD;
    }

    /* only recompute RTT hence RTO after success
     */
    s->rto = tcp_RTO_BASE + (((s->vj_sa >> 2) + (s->vj_sd)) >> 1);

    tcp_rtt_add (s, s->rto, _mtu);

    TCP_CONSOLE_MSG (2, ("RTO %u  sa %lu  sd %lu  cwindow %u"
                     "  wwindow %u  unacked %ld\n",
                     s->rto, (u_long)s->vj_sa, (u_long)s->vj_sd, s->cwindow,
                     s->wwindow, s->send_una));
  }

  s->karn_count = 0;
  if (s->wwindow != 255)
  {
    /** \todo Use the threshold to signify "end-of-Slow-Start" (equilibrium)
     */
#if 0
    if (s->cwindow * s->max_seg < s->send_ssthresh)
        s->cwindow++;
#else

    /* A. Iljasov (iljasov@oduurl.ru) suggested this pre-increment
     */
    if (++s->wwindow >= s->cwindow)
    {
      if (s->cwindow != 255)
          s->cwindow++;
      s->wwindow = 0;    /* mdurkin -- added 95.05.02 */
    }
#endif
  }

  /* Restart RTT timer or postpone retransmission based on
   * calculated RTO. Make sure date/date_ms variables are updated
   * close to midnight.
   */
  chk_timeout (0UL);
  timeout = set_timeout (s->rto + tcp_RTO_ADD);

  if (s->rtt_time == 0UL || cmp_timers(s->rtt_time,timeout) < 0)
      s->rtt_time = timeout;

  s->datatimer = 0UL; /* resetting tx-timer, EE 99.08.23 */
}

/**
 * Check if receive window needs an update.
 */
static void tcp_upd_win (_tcp_Socket *s, unsigned line)
{
  UINT winfree = s->max_rx_data - (UINT)s->rx_datalen;

  SIO_TRACE (("tcp_upd_win"));

  if (winfree < s->max_seg/2)
  {
    _tcp_send (s, __FILE__, line);  /* update window now */
    TCP_CONSOLE_MSG (2, ("tcp_upd_win(%d): win-free %u\n", line, winfree));
  }
}

/**
 * TCP option routines.
 * \note Each of these \b MUST add multiple of 4 bytes of options.
 */

/**
 * Insert MSS option.
 */
static __inline int tcp_opt_maxsegment (const _tcp_Socket *s, BYTE *opt)
{
  *opt++ = TCPOPT_MAXSEG;    /* option: MAXSEG,length,MSS */
  *opt++ = 4;
  *(WORD*) opt = intel16 ((WORD)s->max_seg);
  return (4);
}

/**
 * Insert TimeStamp option.
 * nmap uses TSval (ts_now) as indication of uptime, so send
 * # of msec since started.
 */
static __inline int tcp_opt_timestamp (_tcp_Socket *s, BYTE *opt, DWORD ts_echo)
{
  DWORD ts_now = set_timeout (0) - start_time;  /* TSval */
  BYTE *start  = opt;

  *opt++ = TCPOPT_NOP;     /* NOP,NOP,TIMESTAMP,length,TSval,TSecho */
  *opt++ = TCPOPT_NOP;     /*         ---------- = 10 ------------- */
  *opt++ = TCPOPT_TIMESTAMP;
  *opt++ = 10;
  *(DWORD*) opt = intel (ts_now);
  opt += sizeof(ts_now);
  *(DWORD*) opt = intel (ts_echo);
  opt += sizeof(ts_echo);
  s->ts_sent = ts_now;      /* remember TSval */
  return (opt - start);     /* 20 bytes */
}

#if defined(USE_TCP_MD5)
/**
 * Prepare to insert MD5 option.
 */
static BYTE *sign_opt;

static __inline int tcp_opt_md5_sign (BYTE *opt)
{
  *opt++ = TCPOPT_SIGNATURE;  /* option: 19,length,MD5-signature,NOP,NOP */
  *opt++ = 2+TCPOPT_SIGN_LEN;
  sign_opt = opt;             /* remember for finalise_md5_sign() */
  opt += TCPOPT_SIGN_LEN;
  *opt++ = TCPOPT_NOP;
  *opt++ = TCPOPT_NOP;
  return (4+TCPOPT_SIGN_LEN);
}

static __inline void finalise_md5_sign (const in_Header *ip,
                                        const tcp_Header *tcp,
                                        WORD tcp_len, const char *secret)
{
  if (secret && sign_opt)
     make_md5_signature (ip, tcp, tcp_len, secret, sign_opt);
  sign_opt = NULL;
}
#endif


#if defined(NOT_USED_YET)
/**
 * Pad options to multiple of 4 bytes.
 */
static __inline int tcp_opt_padding (BYTE *opt, int len)
{
  int pad = len % 4;

  if (pad > 0)
     *opt++ = TCPOPT_NOP;
  if (pad > 1)
     *opt++ = TCPOPT_NOP;
  if (pad > 2)
     *opt++ = TCPOPT_NOP;
  if (pad > 3)
     *opt++ = TCPOPT_NOP;
  return (pad);
}

static __inline int tcp_opt_winscale (const _tcp_Socket *s, BYTE *opt)
{
  *opt++ = TCPOPT_WINDOW;    /* option: WINDOW,length,wscale */
  *opt++ = 3;
  *opt   = s->tx_wscale;
  return (4);
}

static __inline int tcp_opt_sack_ok (const _tcp_Socket *s, BYTE *opt)
{
  *opt++ = TCPOPT_SACKOK;
  *opt++ = 2;
  *opt++ = TCPOPT_NOP;
  *opt++ = TCPOPT_NOP;
  return (4);
}

static __inline int tcp_opt_sack (const _tcp_Socket *s, BYTE *opt,
                                  const struct SACK_list *sack)
{
  int i, len = 2 + 8 * sack->num_blk;

  *opt++ = TCPOPT_SACK;       /* option: SACK,length,left,right,.. */
  *opt++ = len;
  for (i = 0; i < sack->num_blk; i++)
  {
    *(DWORD*) opt = intel (sack->list[i].left_edge);
    opt += sizeof(DWORD);
    *(DWORD*) opt = intel (sack->list[i].right_edge);
    opt += sizeof(DWORD);
  }
  return (len + tcp_opt_padding(opt,len));
}
#endif  /* NOT_USED_YET */


/**
 * Append options to output TCP header.
 */
static __inline int tcp_options (_tcp_Socket *s, BYTE *opt, BOOL is_syn)
{
  int len = 0;

  if (s->locflags & LF_NOOPT)   /* we suppress options */
     return (0);

  if (is_syn)
  {
    len = tcp_opt_maxsegment (s, opt);

    if (tcp_opt_ts)
       len += tcp_opt_timestamp (s, opt+len, 0UL);
#if 0
    if (s->locflags & LF_REQ_SCALE)
       len += tcp_opt_winscale (s, opt+len);
    if (tcp_opt_sack)
       len += tcp_opt_sack_ok (s, opt+len);
#endif
  }
  else if (tcp_opt_ts &&
           !(s->flags & (tcp_FlagFIN|tcp_FlagRST)) &&
           (s->locflags & LF_USE_TSTAMP))
  {
    /* We got a TS option in a previous SYN or SYN-ACK.
     * Echo it if "tcp.opt.ts = 1" and not sending a FIN/RST.
     */
    len = tcp_opt_timestamp (s, opt, s->ts_recent);
    s->locflags &= ~LF_USE_TSTAMP;  /* don't echo this again */
  }

#if defined(USE_TCP_MD5)
  if (s->secret)
     len += tcp_opt_md5_sign (opt+len);
#endif

  WATT_ASSERT (len == 0 || (len % 4) == 0);
  return (len);
}

/**
 * Format and send an outgoing TCP segment.
 * Several packets may be sent depending on peer's window.
 */
int _tcp_send (_tcp_Socket *s, const char *file, unsigned line)
{
  struct tcp_pkt *pkt;

#if defined(USE_IPV6)
  struct tcp6_pkt   *pkt6;
  struct in6_Header *ip6 = NULL;
#endif

  BOOL         tx_ok;
  BYTE        *data;             /* where to copy user's data */
  mac_address *dst;
  in_Header   *ip = NULL;
  tcp_Header  *tcp;
  int          send_tot_len = 0; /* count of data length we've sent */
  int          send_data_len;    /* how much data in this segment */
  int          start_data;       /* where data starts in tx-buffer */
  int          send_tot_data;    /* total amount of data to send */
  int          tcp_len;          /* total length of TCP segment */
  int          opt_len;          /* total length of TCP options */
  int          pkt_num;          /* 0 .. s->cwindow-1 */
  int          rtt;

  SIO_TRACE (("_tcp_send"));

  s->recent = 0;
  dst = (_pktserial ? NULL : &s->his_ethaddr);

#if defined(USE_IPV6)
  if (s->is_ip6)
  {
    pkt6 = (struct tcp6_pkt*) _eth_formatpacket (dst, IP6_TYPE);
    ip6  = &pkt6->in;
    tcp  = &pkt6->tcp;
  }
  else
#endif
  {
    pkt = (struct tcp_pkt*) _eth_formatpacket (dst, IP4_TYPE);
    ip  = &pkt->in;
    tcp = &pkt->tcp;
  }

  data = (BYTE*) (tcp+1);   /* data starts here if no options */

  if (s->karn_count == 2)   /* doing slow-start */
  {
    send_tot_data = min (s->tx_datalen, s->window);
    start_data = 0;
  }
  else
  {
    /* Morten Terstrup <MorTer@dk-online.dk> found this signed bug
     */
    int size = min (s->tx_datalen, s->window);

    send_tot_data = size - s->send_una;
    if (send_tot_data < 0)
        send_tot_data = 0;
    start_data = s->send_una;   /* relative tx_data[] */
  }

  /* step through our packets
   */
  for (pkt_num = 0; pkt_num < s->cwindow; pkt_num++)
  {
#if 1
    if (s->safetysig != SAFETY_TCP || s->safetytcp != SAFETY_TCP)
    {
      outsnl (_LANG("Error in _tcp_send()"));
      s->safetysig = s->safetytcp = SAFETY_TCP;
      return (0);
    }
#else
    WATT_ASSERT (s->safetysig == SAFETY_TCP);  /* hard to find GoAhead bug! */
    WATT_ASSERT (s->safetytcp == SAFETY_TCP);
#endif

    /* make tcp header
     */
    tcp->srcPort  = intel16 (s->myport);
    tcp->dstPort  = intel16 (s->hisport);
    tcp->seqnum   = intel (s->send_next + start_data); /* unacked - no longer send_tot_len */
    tcp->acknum   = intel (s->recv_next);

    s->adv_win    = s->max_rx_data - s->rx_datalen;    /* Our new advertised recv window */
    tcp->window   = intel16 ((WORD)s->adv_win);
    tcp->flags    = (BYTE) s->flags;
    tcp->unused   = 0;
    tcp->checksum = 0;
    tcp->urgent   = 0;

    /* Insert any TCP options after header.
     */
    if (pkt_num == 0 && (s->flags & tcp_FlagSYN) == tcp_FlagSYN)
    {
      opt_len = tcp_options (s, data, TRUE);
      send_data_len = 0;   /* no data, only options */
    }
    else
    {
      int data_free;

      opt_len   = tcp_options (s, data, FALSE);
      data_free = s->max_seg - opt_len;
      if (data_free < 0)
          data_free = 0;
      send_data_len = min (send_tot_data, data_free);
    }

    tcp_len = sizeof(*tcp) + opt_len;  /* TCP header length */
    data   += opt_len;
    tcp->offset = tcp_len/4;           /* # of 32-bit dwords */

    if (send_data_len > 0)             /* non-SYN packets with data */
    {
      tcp_len += send_data_len;
      if (s->tx_queuelen)
           memcpy (data, s->tx_queue+start_data, send_data_len);
      else memcpy (data, s->tx_data +start_data, send_data_len);
    }

    if (s->locflags & LF_NOPUSH)
       tcp->flags &= ~tcp_FlagPUSH;

#if defined(USE_TCP_MD5)
    finalise_md5_sign (ip, tcp, tcp_len-sizeof(*tcp), s->secret);
#endif

    /* Send using _ip?_output()
     */
#if defined(USE_IPV6)
    if (s->is_ip6)
    {
      memcpy (&ip6->source, &s->my6addr, sizeof(ip6->source));
      memcpy (&ip6->destination, &s->his6addr, sizeof(ip6->destination));
      tcp->checksum = _ip6_checksum (ip6, IP6_NEXT_TCP, tcp, tcp_len);
      tx_ok = _ip6_output (ip6, &ip6->source, &ip6->destination, IP6_NEXT_TCP,
                           tcp_len, 0, s, file, line) != 0;
    }
    else
#endif
    {
      tcp_PseudoHeader ph = { 0,0,0,0,0,0 };

      ph.src      = intel (s->myaddr);
      ph.dst      = intel (s->hisaddr);
      ph.protocol = TCP_PROTO;
      ph.length   = intel16 (tcp_len);
      ph.checksum = CHECKSUM (tcp, tcp_len);
      tcp->checksum = ~CHECKSUM (&ph, sizeof(ph));

      tx_ok = _ip4_output (ip, ph.src, ph.dst, TCP_PROTO,
                           s->ttl, s->tos, 0, tcp_len, s, file, line) != 0;
    }

    if (!tx_ok)
    {
      TCP_SENDSOON (s);
      return (-1);
    }

    /* do next packet
     */
    if (send_data_len > 0)
    {
      send_tot_len  += send_data_len;
      start_data    += send_data_len;   /* new start of data */
      send_tot_data -= send_data_len;   /* total data remaining */
    }
    if (send_tot_data <= 0)
    {
      pkt_num++;
      break;
    }
  }

  s->send_una = start_data;  /* relative start of tx_data[] buffer */

  TCP_CONSOLE_MSG (2, ("tcp_send (called from %s/%u): sent %u bytes in %u "
                   "packets with (%ld) unacked. SND.NXT %lu\n",
                   file, line, send_tot_len, pkt_num, s->send_una,
                   (u_long)s->send_next));

  s->vj_last = 0UL;
  if (s->karn_count == 2)
  {
    if (s->rto)
         s->rto = (s->rto * 3) / 2;  /* increase by 50% */
    else s->rto = 2*tcp_RTO_ADD;     /* !!was 4 tick */
  }
  else
  {
    /* vj_last nonzero if we expect an immediate response
     */
    if (s->unhappy || s->tx_datalen)
        s->vj_last = set_timeout (0);
    s->karn_count = 0;
  }

  /* Determine when a retransmission should be sent
   */
  rtt = s->rto + tcp_RTO_ADD;

  s->rtt_time = set_timeout (rtt);

  if (send_tot_len > 0)
     s->rtt_lasttran = s->rtt_time;

  return (send_tot_len);
}

/**
 * Format and send a reset (RST) tcp packet.
 * \note We modify the orignal segment (orig_tcp) to use it for sending.
 */
int _tcp_send_reset (_tcp_Socket *s, const in_Header *his_ip,
                     const tcp_Header *org_tcp, const char *file, unsigned line)
{
  tcp_PseudoHeader  ph;

  W32_CLANG_PACK_WARN_OFF()

  #include <sys/pack_on.h>

  struct ip4_packet {
         in_Header  ip;
         tcp_Header tcp;
       } *ip4_pkt = NULL;

#if defined(USE_IPV6)
  struct ip6_packet {
         in6_Header ip;
         tcp_Header tcp;
       } *ip6_pkt = NULL;
#endif

  #include <sys/pack_off.h>

  W32_CLANG_PACK_WARN_DEF()

  BYTE         flags;
  tcp_Header  *tcp;
  BOOL         is_ip4 = (his_ip->ver == 4);
  DWORD        acknum, seqnum;
  WORD         tcp_len;

  if (tcp_RST_TIME)   /* tcp_RST_TIME = 0: always send RST */
  {
    static DWORD next_RST_time = 0UL;

    if (next_RST_time && !chk_timeout(next_RST_time))
       return (-1);

    next_RST_time = set_timeout (tcp_RST_TIME);
  }

  flags = org_tcp->flags;
  if (flags == 0 ||           /* ignore if no flags set */
      (flags & tcp_FlagRST))  /* or RST set */
     return (-1);

  if ((flags & (tcp_FlagSYN|tcp_FlagACK)) == tcp_FlagSYN) /* SYN only */
  {
    acknum = intel (org_tcp->seqnum) + 1;
    seqnum = 0;
    flags  = tcp_FlagACK;
  }
  else if (flags & tcp_FlagACK)
  {
    seqnum = intel (org_tcp->acknum);
    acknum = 0;
    flags  = 0;
  }
  else
  {
    const in6_Header *his_ip6 = (const in6_Header*) his_ip;
    int   len, ofs = org_tcp->offset << 2;

    if (is_ip4)
         len = intel16 (his_ip->length) - in_GetHdrLen (his_ip);
    else len = intel16 (his_ip6->len);

    acknum = intel (org_tcp->seqnum) + (len - ofs);
    seqnum = 0;
    flags  = 0;
  }

#if defined(USE_IPV6)
  if (!is_ip4)
  {
    ip6_pkt = (struct ip6_packet*) _eth_formatpacket (MAC_SRC(his_ip),
                                                      IP6_TYPE);
    tcp = &ip6_pkt->tcp;
  }
  else
#endif
  {
    ip4_pkt = (struct ip4_packet*) _eth_formatpacket (MAC_SRC(his_ip),
                                                      IP4_TYPE);
    tcp = &ip4_pkt->tcp;
  }

  /* tcp header
   */
  tcp_len       = sizeof(*tcp);
  tcp->srcPort  = org_tcp->dstPort;
  tcp->dstPort  = org_tcp->srcPort;
  tcp->seqnum   = intel (seqnum);
  tcp->acknum   = intel (acknum);
  tcp->window   = 0;     /* non-zero means "BSD derived" */
  tcp->flags    = (flags & tcp_FlagMASK) | tcp_FlagRST;
  tcp->unused   = 0;
  tcp->checksum = 0;
  tcp->urgent   = 0;

#if defined(USE_TCP_MD5)
  if (is_ip4 && s && s->secret)
  {
    tcp_len += tcp_opt_md5_sign ((BYTE*)(tcp+1));
    finalise_md5_sign (&ip4_pkt->ip, tcp, 0, s->secret);
  }
#endif

  tcp->offset = tcp_len / 4;

#if defined(USE_IPV6)
  if (!is_ip4)
  {
    in6_Header *ip6 = &ip6_pkt->ip;

    memcpy (&ip6->source, &in6addr_my_ip, sizeof(ip6->source));
    memcpy (&ip6->destination, ((in6_Header*)his_ip)->source, sizeof(ip6->destination));
    tcp->checksum = 0;
    tcp->checksum = _ip6_checksum (ip6, IP6_NEXT_TCP, tcp, tcp_len);

    return _ip6_output (ip6, &ip6->source, &ip6->destination,
                        IP6_NEXT_TCP, tcp_len, 0, s, file, line);
  }
#endif

  memset (&ph, 0, sizeof(ph));
  ph.src      = his_ip->destination;
  ph.dst      = his_ip->source;
  ph.protocol = TCP_PROTO;
  ph.length   = intel16 (tcp_len);
  ph.checksum = CHECKSUM (tcp, tcp_len);

  tcp->checksum = ~CHECKSUM (&ph, sizeof(ph));

  return _ip4_output (&ip4_pkt->ip, ph.src, ph.dst, TCP_PROTO,
                      s ? s->ttl : _default_ttl, his_ip->tos, 0,
                      tcp_len, s, file, line);
}

/**
 * TCP keepalive transmission.
 * If connection is idle (tx_datalen == 0), force the peer to send
 * us a segment by sending a keep-alive packet:
 *  \verbatim
 *    <SEQ=SND.UNA-1><ACK=RCV.NXT><CTL=ACK>
 *  \endverbatim
 */
int _tcp_keepalive (_tcp_Socket *tcp)
{
  DWORD ack, seq;
  BYTE  kc;
  int   rc;

  SIO_TRACE (("_tcp_keepalive"));

  if (tcp->ip_type != TCP_PROTO || tcp->state < tcp_StateSYNSENT ||
      tcp->tx_datalen > 0)
     return (0);

  ack = tcp->recv_next;
  seq = tcp->send_next;
  kc  = tcp->karn_count;

  tcp->recv_next  = tcp->send_next;
  tcp->send_next  = tcp->send_next + tcp->send_una - 1;
  tcp->flags      = tcp_FlagACK;
  tcp->karn_count = 2;
  tcp->tx_datalen = 1;   /* BSD 4.2 requires data to respond to a keepalive */
  tcp->tx_data[0] = '0';
  rc = TCP_SEND (tcp);

  tcp->tx_datalen = 0;
  tcp->recv_next  = ack;
  tcp->send_next  = seq;
  tcp->karn_count = kc;
  tcp->locflags  &= ~LF_KEEPALIVE;

  STAT (tcpstats.tcps_keepprobe++);
  STAT (tcpstats.tcps_keeptimeo++);
  return (rc);
}

int W32_CALL sock_keepalive (sock_type *s)
{
  return _tcp_keepalive (&s->tcp);
}
#endif  /* !USE_UDP_ONLY */


/**
 * Set binary or ascii mode for UDP/TCP sockets.
 * - Effects sock_gets(), sock_dataready().
 * - Enable/disable UDP checksums.
 *
 * Never called within Watt-32. Map <tcp.h> values to internal
 * SOCK_MODE values.
 */
#define TCP_H_SM_BINARY           0x01
#define TCP_H_SM_ASCII            0x02
#define TCP_H_SM_UDP_MODE_CHK     0x04
#define TCP_H_SM_UDP_MODE_NOCHK   0x08
#define TCP_H_SM_TCP_MODE_NAGLE   0x10
#define TCP_H_SM_TCP_MODE_NONAGLE 0x20
#define TCP_H_SM_MASK             0x3F

WORD W32_CALL sock_mode (sock_type *s, WORD mode)
{
  if (s->tcp.ip_type == TCP_PROTO || s->tcp.ip_type == UDP_PROTO)
  {
    int old_nagle  = (s->tcp.sockmode & SOCK_MODE_NAGLE);
    int old_bmode  = (s->tcp.sockmode & SOCK_MODE_BINARY);
    int old_udpchk = (s->udp.sockmode &= SOCK_MODE_UDPCHK);

    mode &= TCP_H_SM_MASK;
    if (mode == 0)
       return (s->tcp.sockmode & SOCK_MODE_MASK);

    s->tcp.sockmode &= ~SOCK_MODE_MASK; /* turn all bits off */

    if (mode & TCP_H_SM_BINARY)
       s->tcp.sockmode |= SOCK_MODE_BINARY;
    else if (mode & TCP_H_SM_ASCII)
       s->tcp.sockmode &= ~SOCK_MODE_BINARY;
    else
       s->tcp.sockmode |= old_bmode;

    if (s->tcp.ip_type == UDP_PROTO)
    {
      if (mode & TCP_H_SM_UDP_MODE_CHK)
         s->udp.sockmode |= SOCK_MODE_UDPCHK;
      else if (mode & TCP_H_SM_UDP_MODE_NOCHK)
         s->udp.sockmode &= ~SOCK_MODE_UDPCHK;
      else
         s->udp.sockmode |= old_udpchk;
      return (s->udp.sockmode);
    }

    if (mode & TCP_H_SM_TCP_MODE_NAGLE)
       s->tcp.sockmode |= SOCK_MODE_NAGLE;
    else if (mode & TCP_H_SM_TCP_MODE_NONAGLE)
       s->tcp.sockmode &= ~SOCK_MODE_NAGLE;
    else
       s->tcp.sockmode |= old_nagle;
    return (s->tcp.sockmode);
  }
  return (0);
}

/**
 * Enable user defined yield function.
 * Return address of previous yield function.
 */
VoidProc W32_CALL sock_yield (_tcp_Socket *s, VoidProc func)
{
  VoidProc old;

  if (s)
  {
    old = s->usr_yield;
    s->usr_yield = func;
  }
  else
  {
    old = system_yield;
    system_yield = func;
  }
  return (old);
}

/**
 * Abort a UDP/TCP/Raw socket.
 */
int W32_CALL sock_abort (sock_type *s)
{
  SIO_TRACE (("sock_abort"));

  switch (s->tcp.ip_type)
  {
#if !defined(USE_UDP_ONLY)
    case TCP_PROTO:
         TCP_ABORT (&s->tcp);
         break;
#endif
    case UDP_PROTO:
         udp_close (&s->udp);
         break;
    case IP4_TYPE:
         s->raw.ip_type = 0;
         s->raw.used    = 0;
         break;
  }
  return (1);
}

#if defined(USE_BSD_API)
/**
 * Read data from a raw-socket. Don't copy IP-header to buf.
 */
static int raw_read (_raw_Socket *raw, BYTE *buf, int maxlen)
{
  int len = 0;

  SIO_TRACE (("raw_read"));

  if (raw->used)
  {
    int   hlen = in_GetHdrLen (&raw->ip);
    BYTE *data = (BYTE*)&raw->ip + hlen;

    len = intel16 (raw->ip.length) - hlen;
    len = min (len, maxlen);
    memcpy (buf, data, len);
    raw->used = 0;
  }
  return (len);
}
#endif

/**
 * Read a socket with maximum 'maxlen' bytes.
 * \note   busywaits until 'buf' is full (and call 's->usr_yield').
 * \retval 0        socket is not open.
 * \retval <=maxlen socket has data to be read.
 */
int W32_CALL sock_read (sock_type *s, BYTE *buf, size_t maxlen)
{
  int count = 0;

  SIO_TRACE (("sock_read"));

  do
  {
    int len = 0;
    int raw = 0;

    switch (s->udp.ip_type)
    {
#if !defined(USE_UDP_ONLY)
      case TCP_PROTO:
           len = tcp_read (&s->tcp, buf, maxlen);
           break;
#endif
      case UDP_PROTO:
           len = udp_read (&s->udp, buf, maxlen);
           break;

#if defined(USE_BSD_API)
      case IP4_TYPE:
           raw = TRUE;
           len = raw_read (&s->raw, buf, maxlen);
           break;
#endif
    }

    if (len < 1)
    {
      if (!tcp_tick(s) || len < 0)
         break;
    }
    else
    {
      count  += len;
      buf    += len;
      maxlen -= len;
    }
    if (maxlen > 0 && !raw)
    {
      if (s->tcp.usr_yield)       /* yield only when room */
          (*s->tcp.usr_yield)();  /* 99.07.01 EE */
      else WATT_YIELD();
    }
  }
  while (maxlen);
  return (count);
}

/**
 * Read a socket with maximum 'len' bytes.
 * \note does \b not busywait until buffer is full.
 */
int W32_CALL sock_fastread (sock_type *s, BYTE *buf, int len)
{
  SIO_TRACE (("sock_fastread"));

  if (s->udp.ip_type == UDP_PROTO)
     return udp_read (&s->udp, buf, len);

#if !defined(USE_UDP_ONLY)
  if (s->tcp.ip_type == TCP_PROTO || s->tcp.rx_datalen > 0)
     return tcp_read (&s->tcp, buf, len);
#endif

#if defined(USE_BSD_API)
  if (s->raw.ip_type == IP4_TYPE)
     return raw_read (&s->raw, buf, len);
#endif

  return (-1);
}


/**
 * Writes data and returns length written.
 *  \note         sends with PUSH-bit (flush data).
 *  \note         repeatedly calls 's->usr_yield'.
 *  \note         UDP packetsare sent in chunks of MTU-28.
 *  \retval 0     if some error in lower layer.
 *  \retval 'len' if all data sent okay.
 */
int W32_CALL sock_write (sock_type *s, const BYTE *data, int len)
{
  size_t chunk;
  size_t remain  = len;
  int    written = 0;

  SIO_TRACE (("sock_write"));

  while (remain > 0)
  {
    switch (s->udp.ip_type)
    {
#if !defined(USE_UDP_ONLY)
      case TCP_PROTO:
           s->tcp.flags |= tcp_FlagPUSH;
           written = tcp_write (&s->tcp, data, remain);
           break;
#endif
      case UDP_PROTO:
           chunk   = min (_mtu - UDP_OVERHEAD, remain);
           written = udp_write (&s->udp, data, chunk);
           break;

#if defined(USE_BSD_API)
      case IP4_TYPE:
      case IP6_TYPE:
           return (0);   /* not supported yet */
#endif
      default:           /* EE 99.06.14 */
#if defined(USE_DEBUG)
           dbug_printf ("sock_write() called with unknown proto %04Xh\n",
                        s->udp.ip_type);
#endif
           return (0);
    }

    if (written < 0)
    {
      s->udp.err_msg = _LANG ("Tx Error");
      return (0);
    }
    data   += written;
    remain -= written;

    if (s->udp.usr_yield)
        (*s->udp.usr_yield)();
    else WATT_YIELD();

    if (!tcp_tick(s))
       return (0);  /* !! should be 'len - remain' ? */
  }
  return (len);
}

/**
 * Simpler, non-blocking (non-looping) version of sock_write().
 * \note UDP writes may truncate; check the return value.
 */
int W32_CALL sock_fastwrite (sock_type *s, const BYTE *data, int len)
{
  SIO_TRACE (("sock_fastwrite"));

  switch (s->udp.ip_type)
  {
    case UDP_PROTO:
         len = min ((int)(_mtu - UDP_OVERHEAD), len);
         len = udp_write (&s->udp, data, len);
         return (len < 0 ? 0 : len);

#if !defined(USE_UDP_ONLY)
    case TCP_PROTO:
         len = tcp_write (&s->tcp, data, len);
         return (len < 0 ? 0 : len);
#endif
  }
  return (0);
}

/**
 * For UDP, this function is same as sock_write().
 * For TCP, this function enqueues 'data' to transmit queue.
 *
 * \note user \b must not touch 'data' while sock_tbused() returns > 0.
 */
int W32_CALL sock_enqueue (sock_type *s, const BYTE *data, int len)
{
  SIO_TRACE (("sock_enqueue"));

  if (len <= 0)
     return (0);

  if (s->udp.ip_type == UDP_PROTO)
  {
    int written = 0;
    int total   = 0;
    do
    {
      len = min ((int)(_mtu - UDP_OVERHEAD), len);
      written = udp_write (&s->udp, data, len);
      if (written < 0)
      {
        s->udp.err_msg = _LANG ("Tx Error");
        break;
      }
      data  += written;
      len   -= written;
      total += written;
    }
    while (len > 0);
    return (total);
  }

#if !defined(USE_UDP_ONLY)
  if (s->tcp.ip_type == TCP_PROTO)
  {
    s->tcp.tx_queue    = data;
    s->tcp.tx_queuelen = len;
    s->tcp.tx_datalen  = len;
    return TCP_SEND (&s->tcp);
  }
#endif
  return (0);
}

#if !defined(USE_UDP_ONLY)
/**
 * Sets non-flush mode on next TCP write.
 */
void W32_CALL sock_noflush (sock_type *s)
{
  SIO_TRACE (("sock_noflush"));

  if (s->tcp.ip_type == TCP_PROTO)
  {
    s->tcp.flags &= ~tcp_FlagPUSH;
    s->tcp.sockmode |= SOCK_MODE_LOCAL;
  }
}

/**
 * Send pending TCP data.
 * If there is Tx-data to be sent, set the PUSH bit.
 */
void W32_CALL sock_flush (sock_type *s)
{
  SIO_TRACE (("sock_flush"));

  if (s->tcp.ip_type == TCP_PROTO)
  {
    _tcp_Socket *tcp = &s->tcp;

    tcp->sockmode &= ~SOCK_MODE_LOCAL;
    if (tcp->tx_datalen > 0)
    {
      tcp->flags |= tcp_FlagPUSH;
      if (s->tcp.send_una == 0)  /* !! S. Lawson - only if data not moving */
         TCP_SEND (tcp);
    }
  }
}

/**
 * Causes next transmission to have a flush (PUSH bit set).
 */
void W32_CALL sock_flushnext (sock_type *s)
{
  SIO_TRACE (("sock_flushnext"));

  if (s->tcp.ip_type == TCP_PROTO)
  {
    s->tcp.flags |= tcp_FlagPUSH;
    s->tcp.sockmode &= ~SOCK_MODE_LOCAL;
  }
}
#endif  /* !USE_UDP_ONLY */

/**
 * Close a UDP/TCP socket.
 */
int W32_CALL sock_close (sock_type *s)
{
  SIO_TRACE (("sock_close"));

  switch (s->tcp.ip_type)
  {
    case UDP_PROTO:
         udp_close (&s->udp);
         break;

#if !defined(USE_UDP_ONLY)
    case TCP_PROTO:
         _tcp_close (&s->tcp);
         tcp_tick (s);
         break;
#endif
  }
  return (0);
}

#if !defined(USE_UDP_ONLY)

/**
 * Round-trip timing cache routines.
 * These functions implement a very simple system for keeping track of
 * network performance for future use in new connections.
 * The emphasis here is on speed of update (rather than optimum cache
 * hit ratio) since tcp_rtt_add() is called every time a TCP connection
 * updates its round trip estimate.
 * \note 'rto' is either in ticks or milli-sec depending on if PC has an
 *       8254 Time chip.
 *
 * These routines are modified versions from \b KA9Q by \b Phil Karn.
 */
static struct tcp_rtt rtt_cache [RTTCACHE];

void tcp_rtt_add (const _tcp_Socket *s, UINT rto, UINT MTU)
{
  struct tcp_rtt *rtt;
  DWORD  addr = s->hisaddr;

  SIO_TRACE (("tcp_rtt_add"));

  if (~addr & ~sin_mask)  /* 0.0.0.0 or broadcast addresses? */
     return;

  rtt = &rtt_cache [(WORD)addr % RTTCACHE];

  /* Cache-slot is vacant or we're updating previous RTO
   * for same peer
   */
  if (!rtt->ip || rtt->ip == addr)
  {
    rtt->ip  = addr;
    rtt->rto = rto;
    rtt->MTU = MTU;
    STAT (tcpstats.tcps_cachedrtt++);
  }
}

BOOL tcp_rtt_get (const _tcp_Socket *s, UINT *rto, UINT *MTU)
{
  struct tcp_rtt *rtt = &rtt_cache [(s->hisaddr & 0xFFFF) % RTTCACHE];

  SIO_TRACE (("tcp_rtt_get"));

  STAT (tcpstats.tcps_segstimed++);

  if (s->hisaddr && rtt->ip == s->hisaddr && rtt->rto > 0)
  {
#if defined(USE_DEBUG) && !defined(_MSC_VER) /* MSC6 crashes below */
    dbug_printf ("\nRTT-cache: host %s: %ss\n\n",
                 _inet_ntoa(NULL, rtt->ip), time_str(rtt->rto));
#endif

    STAT (tcpstats.tcps_usedrtt++);
    if (rto)
       *rto = rtt->rto;
    if (MTU)
       *MTU = rtt->MTU;
    return (TRUE);
  }
  return (FALSE);
}

void tcp_rtt_clr (const _tcp_Socket *s)
{
  struct tcp_rtt *rtt = &rtt_cache [(s->hisaddr & 0xFFFF) % RTTCACHE];

  if (s->hisaddr && rtt->ip == s->hisaddr)
  {
    rtt->ip  = 0;
    rtt->rto = 0;
    rtt->MTU = 0;
  }
}
#endif /* !USE_UDP_ONLY */
