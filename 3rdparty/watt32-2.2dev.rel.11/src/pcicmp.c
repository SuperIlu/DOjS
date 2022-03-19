/*!\file pcicmp.c
 *
 * ICMP for IPv4 (RFC 792).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "copyrigh.h"
#include "wattcp.h"
#include "strings.h"
#include "language.h"
#include "misc.h"
#include "timer.h"
#include "chksum.h"
#include "ip4_in.h"
#include "pctcp.h"
#include "pcsed.h"
#include "pcarp.h"
#include "pcstat.h"
#include "pcdbug.h"
#include "pcqueue.h"
#include "pcconfig.h"
#include "pcpkt.h"
#include "sock_ini.h"
#include "netaddr.h"
#include "ip4_out.h"
#include "pcicmp.h"

const char *icmp_type_str [ICMP_MAXTYPE+1] = {
      __LANG("Echo Reply"),  "1", "2",
      __LANG("Destination Unreachable"),
      __LANG("Source Quench"),
      __LANG("Redirect"),    "6", "7",
      __LANG("Echo Request"),
      __LANG("Router Advert"),
      __LANG("Router Solic"),
      __LANG("TTL exceeded"),
      __LANG("Param Problem"),
      __LANG("Timestamp Message"),
      __LANG("Timestamp Reply"),
      __LANG("Info Request"),
      __LANG("Info Reply"),
      __LANG("Mask Request"),
      __LANG("Mask Reply")
    };

/* code-strings for type ICMP_UNREACH
 */
const char *icmp_unreach_str [16] = {
      __LANG("Network Unreachable"),              /* 0 */
      __LANG("Host Unreachable"),
      __LANG("Protocol Unreachable"),             /* 2 */
      __LANG("Port Unreachable"),
      __LANG("Fragmentation needed and DF set"),  /* 4 */
      __LANG("Source Route Failed"),
      __LANG("Network unknown"),                  /* 6 */
      __LANG("Host unknown"),
      __LANG("Source host isolated"),             /* 8 */
      __LANG("Net-access denied"),
      __LANG("Host-access denied"),               /* 10 */
      __LANG("Bad TOS for net"),
      __LANG("Bad TOS for host"),                 /* 12 */
      __LANG("Admin prohibited"),
      __LANG("Host precedence violation"),        /* 14 */
      __LANG("Precedence cutoff")
    };

/* code-strings for type ICMP_REDIRECT
 */
const char *icmp_redirect_str [4] = {
      __LANG("Redirect for Network"),
      __LANG("Redirect for Host"),
      __LANG("Redirect for TOS and Network"),
      __LANG("Redirect for TOS and Host")
    };

/* code-strings for type ICMP_TIMXCEED
 */
const char *icmp_exceed_str [2] = {
      __LANG("TTL exceeded in transit"),
      __LANG("Fragment reassembly time exceeded")
    };

W32_CLANG_PACK_WARN_OFF()

#include <sys/pack_on.h>

/*!\struct _pkt
 */
struct _pkt {
       in_Header  ip;
       ICMP_PKT   icmp;
       in_Header  data;
     };

#include <sys/pack_off.h>

W32_CLANG_PACK_WARN_DEF()

/*!\struct do_redirect
 */
static struct {
       char  icmp;
       char  igmp;
       char  udp;
       char  tcp;
     } do_redirect = { 1,1,1,1 };


static void icmp_print (int dbg_lvl, const char *msg, DWORD src)
{
  if (debug_on < dbg_lvl)
     return;

  outs ("\nICMP: ");
  if (src)
  {
    outs ("(");
    outs (_inet_ntoa(NULL,intel(src)));
    outs ("): ");
  }
  outsnl (_LANG(msg));
}

/**
 * Print info about bogus and possibly dangerous ICMP messages.
 * e.g. "ICMP: (144.133.122.111) Bogus Redirect; GW = 111.110.109.108"
 */
static void icmp_bogus (const in_Header *ip, int type, const char *msg)
{
  char buf[100];

  strcpy (buf, _LANG("Bogus "));
  strcat (buf, _LANG(icmp_type_str[type]));
  if (msg)
     strcat (buf, msg);
  icmp_print (1, buf, ip->source);
}

/**
 * Check if ip-source is a (directed) broadcast address.
 * Some hacker may try to create a broadcast storm.
 * Also check for null source address (0.0.0.0).
 * Broadcast destination is already filtered out by icmp_handler().
 */
static BOOL icmp_check (const in_Header *ip, int type)
{
  DWORD src, dst;
  BOOL  bcast;

  src   = intel (ip->source);
  dst   = intel (ip->destination);
  bcast = (~src & ~sin_mask) == 0;

  if (bcast)
  {
    icmp_bogus (ip, type, _LANG(" (broadcast)"));
    return (FALSE);
  }
  if (ip->source == 0UL)
  {
    icmp_bogus (ip, type, _LANG(" (network)"));
    return (FALSE);
  }
  if (IN_MULTICAST(dst))
  {
    icmp_bogus (ip, type, _LANG(" (multicast)"));
    return (FALSE);
  }
  if (IN_EXPERIMENTAL(dst))
  {
    icmp_bogus (ip, type, _LANG(" (experimental)"));
    return (FALSE);
  }
  return (TRUE);
}

/**
 * Format and send a ICMP packet.
 * \note: src and dest are on network order.
 */
static int icmp_send (struct _pkt *pkt, DWORD src, DWORD dest, int length)
{
  in_Header *ip   = &pkt->ip;
  ICMP_PKT  *icmp = &pkt->icmp;

  icmp->unused.checksum = 0;
  icmp->unused.checksum = ~CHECKSUM (icmp, length);

  return IP4_OUTPUT (ip, src, dest, ICMP_PROTO, 0, 0, 0, length, NULL);
}

/**
 * Send an ICMP destination/protocol unreachable back to 'ip->source'.
 * Limit the rate of these to 20 per second. Ref. RFC-1812.
 */
int icmp_send_unreach (const in_Header *ip, int code)
{
  static DWORD next_time = 0UL;
  struct _pkt    *pkt;
  union ICMP_PKT *unr;
  unsigned        len;

  if (!icmp_check(ip,ICMP_UNREACH))
     return (0);

  if (next_time && !chk_timeout(next_time))
     return (0);

  next_time = set_timeout (50);

  pkt = (struct _pkt*) _eth_formatpacket (MAC_SRC(ip), IP4_TYPE);
  unr = &pkt->icmp;
  len = intel16 (ip->length) - in_GetHdrLen (ip);
  len = min (len, sizeof(*ip)+sizeof(unr->unused.spares));

  icmp_print (1, _LANG(icmp_unreach_str[code]), ip->destination);
  memcpy (&unr->unused.ip, ip, len);
  unr->unused.type = ICMP_UNREACH;
  unr->unused.code = (BYTE) code;

  return icmp_send (pkt, ip->destination, ip->source, sizeof(unr->unused));
}

/**
 * Send an "ICMP Time Exceeded" (reassebly timeout) back to 'ip->source'
 */
int icmp_send_timexceed (const in_Header *ip, const void *mac_dest)
{
  struct _pkt    *pkt;
  union ICMP_PKT *icmp;

  if (!icmp_check(ip,ICMP_TIMXCEED))
     return (0);

  pkt  = (struct _pkt*) _eth_formatpacket (mac_dest, IP4_TYPE);
  icmp = &pkt->icmp;

  icmp_print (1, icmp_exceed_str[1], ip->destination);

  memset (&icmp->unused, 0, sizeof(icmp->unused));

  icmp->unused.type = ICMP_TIMXCEED;
  icmp->unused.code = 1;

  icmp->unused.ip.hdrlen         = sizeof(in_Header) / 4;
  icmp->unused.ip.ver            = 4;
  icmp->unused.ip.ttl            = _default_ttl;
  icmp->unused.ip.identification = ip->identification;
  icmp->unused.ip.length         = intel16 (sizeof(in_Header));
  icmp->unused.ip.proto          = ip->proto;
  icmp->unused.ip.checksum       = ~CHECKSUM (&icmp->unused.ip, sizeof(in_Header));

  return icmp_send (pkt, ip->destination, ip->source, sizeof(icmp->unused));
}

/**
 * Send an ICMP Address Mask Request as link-layer + IP broadcast.
 * Even if we know our address, we send with src = 0.0.0.0.
 */
static WORD addr_mask_id  = 0;
static WORD addr_mask_seq = 0;

int icmp_send_mask_req (void)
{
  mac_address    *dst  = (_pktserial ? NULL : &_eth_brdcast);
  struct _pkt    *pkt  = (struct _pkt*) _eth_formatpacket (dst, IP4_TYPE);
  union ICMP_PKT *icmp = &pkt->icmp;

  addr_mask_id = (WORD) set_timeout (0);   /* get a random ID */
  icmp->mask.type       = ICMP_MASKREQ;
  icmp->mask.code       = 0;
  icmp->mask.identifier = addr_mask_id;
  icmp->mask.sequence   = addr_mask_seq++;
  icmp->mask.mask       = 0;
  return icmp_send (pkt, 0, (DWORD)INADDR_BROADCAST, sizeof(icmp->mask));
}

static int icmp_echo_reply (const in_Header *ip, const union ICMP_PKT *req,
                            unsigned len)
{
  struct _pkt     *pkt;
  union  ICMP_PKT *icmp;

  if (!icmp_check(ip,ICMP_ECHO))
     return (0);

  icmp_print (2, _LANG("PING reply sent"), 0);

#if defined(USE_FRAGMENTS)
  if (len > _mtu - sizeof(*ip))
  {
    icmp = (union ICMP_PKT*) req;        /* reuse input for output */
    icmp->echo.type     = ICMP_ECHOREPLY;
    icmp->echo.checksum = 0;
    icmp->echo.checksum = ~CHECKSUM (icmp, len);
    return _IP4_SEND_FRAGMENTS (NULL, ICMP_PROTO, ip->source, icmp, len);
  }
#endif

  pkt  = (struct _pkt*) _eth_formatpacket (MAC_SRC(ip), IP4_TYPE);
  icmp = &pkt->icmp;

  len = min (len, _mtu - sizeof(*ip));
  memcpy (icmp, req, len);
  icmp->echo.type = ICMP_ECHOREPLY;
  icmp->echo.code = req->echo.code; /* Win uses 0, Unix !0 */

  /* Use supplied ip values in case we ever multi-home.
   * Note that ip values are still in network order.
   */
  return icmp_send (pkt, ip->destination, ip->source, len);
}

/**
 * Handle ICMP_REDIRECT messages.
 */
static void icmp_redirect (const union ICMP_PKT *icmp, const in_Header *ip,
                           const in_Header *orig_ip, int code)
{
  DWORD new_ip = intel (icmp->ip.ipaddr);
  DWORD old_ip = intel (orig_ip->destination);
  const char *msg;

  if (new_ip == old_ip)
  {
    /* Possibly because we and router use different netmasks
     */
  }
  else if ((new_ip ^ my_ip_addr) & sin_mask) /* new host not on subnet */
  {
    char buf[100];

    strcpy (buf, ", GW = ");
    strcat (buf, _inet_ntoa(NULL,new_ip));
    icmp_bogus (ip, ICMP_REDIRECT, buf);
    return;
  }

  msg = icmp_redirect_str[code];
  icmp_print (1, msg, ip->source);

  switch (orig_ip->proto)
  {
#if !defined(USE_UDP_ONLY)
    case TCP_PROTO:
         if (do_redirect.tcp)  /* do it to some socket */
            _tcp_cancel (orig_ip, ICMP_REDIRECT, code, msg, &new_ip);
         break;
#endif
    case UDP_PROTO:
         if (do_redirect.udp)
            _udp_cancel (orig_ip, ICMP_REDIRECT, code, msg, &new_ip);
         break;

    case ICMP_PROTO:
         if (do_redirect.icmp)
         {
        /* _ip_recursion = TRUE; !! */
           _arp_register (new_ip, old_ip);
        /* _ip_recursion = FALSE; !! */
         }
         break;

    case IGMP_PROTO:
         if (do_redirect.igmp)
         {
        /* _ip_recursion = TRUE; !! */
           _arp_register (new_ip, old_ip);
        /* _ip_recursion = FALSE; !! */
         }
         break;
  }
}

/**
 * Handler for incoming ICMP packets.
 */
void icmp_handler (const in_Header *ip, BOOL broadcast)
{
  union ICMP_PKT  *icmp;
  const in_Header *orig_ip;
  int              type, code;
  unsigned         len;
  DWORD            delta_time;
  BOOL             for_me, i_orig;  /* is it for me, did I originate it */
  const char      *msg;

  DEBUG_RX (NULL, ip);

  if (block_icmp)   /* application is handling ICMP on it's own; exit now */
     return;

  len    = in_GetHdrLen (ip);
  icmp   = (union ICMP_PKT*) ((BYTE*)ip + len);
  len    = intel16 (ip->length) - len;
  for_me = _ip4_is_multihome_addr (intel(ip->destination));

  if (!for_me || broadcast)  /* drop broadcast pings.. */
     return;

  if (len < sizeof(icmp->info))
  {
    STAT (icmpstats.icps_tooshort++);
    return;
  }

  if (CHECKSUM(icmp,len) != 0xFFFF)
  {
    STAT (icmpstats.icps_checksum++);
    icmp_print (1, _LANG("bad checksum"), ip->source);
    return;
  }

  type    = icmp->unused.type;
  code    = icmp->unused.code;
  orig_ip = &icmp->ip.ip;
  i_orig  = _ip4_is_local_addr (intel(orig_ip->source));

  if (type == ICMP_MASKREPLY)
  {
    if (!_do_mask_req)
       return;
    i_orig = TRUE;
  }

  /* !! this needs work
   */
  if (!i_orig &&
      (type != ICMP_ECHOREPLY && type != ICMP_ECHO &&
       type != ICMP_IREQREPLY && type != ICMP_TSTAMP))
  {
    icmp_bogus (ip, type, NULL);
    return;
  }

  switch (type)
  {
    case ICMP_ECHOREPLY:  /* check if we were waiting for it */
         delta_time = set_timeout(0) - icmp->echo.identifier;
         add_ping (intel(ip->source), delta_time, icmp->echo.index);
         return;

    case ICMP_UNREACH:
         if (code < DIM(icmp_unreach_str))
         {
           UINT len_needed = 8 + in_GetHdrLen (orig_ip);
           WORD next_mtu   = 0;

           msg = _LANG (icmp_unreach_str[code]);
           icmp_print (1, msg, ip->source);

           if (orig_ip->proto == TCP_PROTO ||
               orig_ip->proto == UDP_PROTO)
              len_needed += 4;  /* Need the src/dest port numbers */

           if (len >= len_needed)
           {
             if (code == ICMP_UNREACH_NEEDFRAG)
                next_mtu = intel16 (icmp->needfrag.next_mtu);

#if !defined(USE_UDP_ONLY)
             if (orig_ip->proto == TCP_PROTO)
                _tcp_cancel (orig_ip, ICMP_UNREACH, code, msg, &next_mtu);
             else
#endif
             if (orig_ip->proto == UDP_PROTO)
                _udp_cancel (orig_ip, ICMP_UNREACH, code, msg, &next_mtu);

             /** \todo Handle cancelling raw sockets */
#if defined(USE_BSD_API) && 0
             else
              _raw_cancel (orig_ip, ICMP_UNREACH, code, msg);
#endif
           }
           else
             STAT (icmpstats.icps_tooshort++);
         }
         else
           STAT (icmpstats.icps_badcode++);
         return;

    case ICMP_SOURCEQUENCH:
#if !defined(USE_UDP_ONLY)
         if (orig_ip->proto == TCP_PROTO)
         {
           msg = _LANG (icmp_type_str[type]);
           icmp_print (1, msg, ip->source);
           _tcp_cancel (orig_ip, ICMP_SOURCEQUENCH, code, msg, NULL);
         }
#endif
         return;

    case ICMP_REDIRECT:
         if (code < DIM(icmp_redirect_str))
              icmp_redirect (icmp, ip, orig_ip, code);
         else STAT (icmpstats.icps_badcode++);
         return;

    case ICMP_ECHO:
         icmp_print (2, _LANG("PING requested of us"), ip->source);
         icmp_echo_reply (ip, icmp, len);
         return;

    case ICMP_TIMXCEED:
         if (code >= DIM(icmp_exceed_str))
         {
           STAT (icmpstats.icps_badcode++);
           return;
         }
         if (code == 0)  /* "TTL exceeded in transit" */
            switch (orig_ip->proto)
            {
#if !defined(USE_UDP_ONLY)
              case TCP_PROTO:
                   msg = _LANG (icmp_exceed_str[0]);
                   icmp_print (1, msg, ip->source);
                   _tcp_cancel (orig_ip, ICMP_TIMXCEED, code, msg, NULL);
                   break;
#endif
              case UDP_PROTO:
                   msg = _LANG (icmp_exceed_str[0]);
                   icmp_print (1, msg, ip->source);
                   _udp_cancel (orig_ip, ICMP_TIMXCEED, code, msg, NULL);
                   break;
            }
         return;

    case ICMP_PARAMPROB:
         msg = _LANG (icmp_type_str[ICMP_PARAMPROB]);
         switch (orig_ip->proto)
         {
#if !defined(USE_UDP_ONLY)
           case TCP_PROTO:
                icmp_print (0, msg, ip->source);
                _tcp_cancel (orig_ip, ICMP_PARAMPROB, code, msg, NULL);
                break;
#endif
           case UDP_PROTO:
                icmp_print (0, msg, ip->source);
                _udp_cancel (orig_ip, ICMP_PARAMPROB, code, msg, NULL);
                break;
         }
         return;

    case ICMP_ROUTERADVERT:  /** \todo !! */
         msg = _LANG (icmp_type_str[ICMP_ROUTERADVERT]);
         icmp_print (1, msg, ip->source);
         return;

    case ICMP_ROUTERSOLICIT: /** \todo !! */
         msg = _LANG (icmp_type_str[ICMP_ROUTERSOLICIT]);
         icmp_print (1, msg, ip->source);
         return;

    case ICMP_TSTAMP:
         msg = _LANG (icmp_type_str[ICMP_TSTAMP]);
         icmp_print (1, msg, ip->source);
         /**< \todo send reply? */
         return;

    case ICMP_TSTAMPREPLY:
         msg = _LANG (icmp_type_str[ICMP_TSTAMPREPLY]);
         icmp_print (1, msg, ip->source);
         /**< \todo should store */
         return;

    case ICMP_IREQ:
         msg = _LANG (icmp_type_str[ICMP_IREQ]);
         icmp_print (1, msg, ip->source);
         /**< \todo send reply */
         return;

    case ICMP_IREQREPLY:
         msg = _LANG (icmp_type_str[ICMP_IREQREPLY]);
         icmp_print (1, msg, ip->source);
         /**< \todo send reply upwards */
         return;

    case ICMP_MASKREQ:
         /* might be sent by us, never answer */
         break;

    case ICMP_MASKREPLY:
         msg = _LANG (icmp_type_str[ICMP_MASKREPLY]);
         icmp_print (0, msg, ip->source);
         if ((icmp->mask.identifier == addr_mask_id)    &&
             (icmp->mask.sequence   == addr_mask_seq-1) &&
             sin_mask != intel(icmp->mask.mask))
            outsnl ("Conflicting net-mask from \"ICMP Addr Mask Reply\"\7");
         addr_mask_id = 0;
         return;
  }
}

/**
 * Determine which protocols we shall act upon when
 * ICMP redirect is received.
 */
void icmp_doredirect (const char *value)
{
  char *val = strdup (value);

  if (val)
  {
    strupr (val);
    do_redirect.icmp = (strstr(val,"ICMP") != NULL);
    do_redirect.igmp = (strstr(val,"IGMP") != NULL);
    do_redirect.udp  = (strstr(val,"UDP")  != NULL);
    do_redirect.tcp  = (strstr(val,"TCP")  != NULL);
    free (val);
  }
}
