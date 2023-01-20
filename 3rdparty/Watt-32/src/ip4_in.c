/*!\file ip4_in.c
 *
 *  IPv4 input and address classifier functions.
 *
 *  Version
 *
 *  1.0 : Sep 18, 2001 : G. Vanem - moved from pctcp.c & ip_out.c
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "wattcp.h"
#include "misc.h"
#include "misc_str.h"
#include "chksum.h"
#include "pcconfig.h"
#include "pcigmp.h"
#include "pcdbug.h"
#include "pcsed.h"
#include "pctcp.h"
#include "pcstat.h"
#include "pcicmp.h"
#include "ip4_frag.h"
#include "ip4_in.h"

/*
 * _ip4_handler - do a simple check on IPv4 header.
 *              - check the fragment chain queue for a match.
 *              - Demultiplex packet to correct protocol handler.
 */
int _ip4_handler (const in_Header *ip, BOOL broadcast)
{
#if defined(USE_FRAGMENTS)
  const in_Header *in_ip = ip;
#endif
  sock_type *s = NULL;
  DWORD  frag_ofs;
  WORD   frag_flg;
  BOOL   delivered = TRUE;   /* assume yes */

  if (block_ip || !_chk_ip4_header(ip))
     return (0);

  frag_ofs = intel16 (ip->frag_ofs);
  frag_flg = (WORD) (frag_ofs & ~IP_OFFMASK);
  frag_ofs = (frag_ofs & IP_OFFMASK) << 3;    /* 0 <= frag_ofs <= 65536-8 */

#if defined(USE_FRAGMENTS)
  if (!ip4_defragment(&ip,frag_ofs,frag_flg)) /* not defragged yet */
     return (0);
#else
  ARGSUSED (frag_flg);
  if (frag_ofs)
     return (0);
#endif

  /** \todo check for LSRR option and replace ip->source
   *        with actual source-address burried in option
   *        (ref. RFC-1122)
   */

#if defined(USE_BSD_API)
  /*
   * Match 'ip' against all SOCK_RAW sockets before doing normal
   * protocol multiplexing below.
   *
   * Note: _bsd_socket_hook does nothing unless we have allocated at least
   *       one SOCK_RAW socket.
   *
   * Should we return if the hook consumed packet (returns non-NULL) ?
   */
  if (_bsd_socket_hook)
    (*_bsd_socket_hook) (BSO_IP4_RAW, ip);
#endif


  switch (ip->proto)
  {
    case TCP_PROTO:
#if !defined(USE_UDP_ONLY)
         s = (sock_type*) _tcp_handler (ip, broadcast);
#endif
         break;

    case UDP_PROTO:
         s = (sock_type*) _udp_handler (ip, broadcast);
         break;

    case ICMP_PROTO:
         icmp_handler (ip, broadcast);
         break;

    case IGMP_PROTO:
#if defined(USE_MULTICAST)
         igmp_handler (ip, broadcast);
#endif
         break;

    case IPCOMP_PROTO:
         /** \todo handle compressed IP (use zlib) */

    default:
         DEBUG_RX (NULL, ip);
         if (!broadcast)
         {
           if (_ip4_is_local_addr(intel(ip->destination)))
              icmp_send_unreach (ip, ICMP_UNREACH_PROTOCOL);
           STAT (ip4stats.ips_noproto++);
         }
         delivered = FALSE;
         break;
  }

#ifdef NOT_USED
  if (s)         /* Check if peer allows IP-fragments */
  {
    if (intel16(ip->frag_ofs) & IP_DF)
         s->tcp.locflags |=  LF_NO_IPFRAGS;
    else s->tcp.locflags &= ~LF_NO_IPFRAGMS;
  }
#endif

#if defined(USE_FRAGMENTS)
  if (ip != in_ip)
     ip4_free_fragment (ip);  /* free this fragment chain */
#endif

  if (delivered)
     STAT (ip4stats.ips_delivered++);
  ARGSUSED (s);
  return (1);
}

/*
 * Check for correct IP-version, header length and header checksum.
 * If failed, discard the packet. ref. RFC1122: section 3.1.2.2
 */
int _chk_ip4_header (const in_Header *ip)
{
  unsigned hlen = in_GetHdrLen (ip);

  if (ip->ver != 4)    /* We only speak IPv4 */
  {
    DEBUG_RX (NULL, ip);
    STAT (ip4stats.ips_badvers++);
    return (0);
  }
  if (hlen < sizeof(*ip))
  {
    DEBUG_RX (NULL, ip);
    STAT (ip4stats.ips_tooshort++);
    return (0);
  }
  if (CHECKSUM(ip,hlen) != 0xFFFF)
  {
    DEBUG_RX (NULL, ip);
    STAT (ip4stats.ips_badsum++);
    return (0);
  }
  return (1);
}

/*
 * Return TRUE if ip address is a local address or on the
 * loopback network (127.x.x.x). `ip' is on host order.
 * If netmask is 255.255.255.255, 'ip' *could* be remote.
 */
int _ip4_is_local_addr (DWORD ip)
{
  if (sin_mask == 0xFFFFFFFFUL)
     return (0);
  return _ip4_is_loopback_addr(ip) || _ip4_is_multihome_addr(ip);
}

int _ip4_is_loopback_addr (DWORD ip)
{
  return ((ip >> 24) == 127);
}

/*
 * Return TRUE if ip address is a legal address for
 * a unique host. `ip' is on host order.
 */
int _ip4_is_unique_addr (DWORD ip)
{
  return (ip && (ip & ~sin_mask) == 0);
}

/*
 * Return TRUE if ip address is among our interface addresses
 * [my_ip_addr.. (my_ip_addr+multihomes)], `ip' is on host order.
 */
int _ip4_is_multihome_addr (DWORD ip)
{
  if (!my_ip_addr || ip < my_ip_addr)
     return (0);
  if (ip - my_ip_addr <= multihomes)
     return (1);
  return (0);
}

/*
 * return TRUE if ip packet is a (directed) broadcast packet.
 */
int _ip4_is_ip_brdcast (const in_Header *ip)
{
  DWORD dst = intel (ip->destination);
  return ((~dst & ~sin_mask) == 0); /* (directed) ip-broadcast */
}

/*
 * determines if the given IP addr is Class D (Multicast)
 *
 * int _ip4_is_multicast (DWORD ip)
 * Where:
 *    `ip' is on host order.
 * Returns:
 *    1   if ip is Class D
 *    0   if ip is not Class D
 *
 * Note: class-D is 224.0.0.0 - 239.255.255.255, but
 *       range      224.0.0.0 - 224.0.0.255 is reserved for mcast
 *                  routing information.
 */
int _ip4_is_multicast (DWORD ip)
{
  return IN_MULTICAST(ip);
}

