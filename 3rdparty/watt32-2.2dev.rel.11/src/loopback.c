/*!\file loopback.c
 *  A simple loopback device.
 *
 *  Copyright (c) 1997-2002 Gisle Vanem <gvanem@yahoo.no>
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. All advertising materials mentioning features or use of this software
 *     must display the following acknowledgement:
 *       This product includes software developed by Gisle Vanem
 *       Bergen, Norway.
 *
 *  THIS SOFTWARE IS PROVIDED BY ME (Gisle Vanem) AND CONTRIBUTORS ``AS IS''
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL I OR CONTRIBUTORS BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *----------------------------------------------------------------------------
 *
 *  A simple loopback device.
 *  At the moment it handles ICMP Echo-request and UDP sink/discard packets.
 *  It pased up the rest.
 *
 *  Add protocol handlers to `loopback_handler' pointer as required.
 *
 *  The modified (src/dst IP swapped) ip packet is enqueued to the IP
 *  receive queue after loopback_device() returns.
 *
 *  The return value is:
 *    -1  if IP-packet should not be enqueued to IP input queue
 *    >0  length of (modified) IP-packet
 *
 *  Gisle Vanem  10 Dec. 1997
 */

#include <stdio.h>
#include <stdlib.h>

#include "wattcp.h"
#include "misc.h"
#include "chksum.h"
#include "ip4_in.h"
#include "ip6_in.h"
#include "pcsed.h"
#include "pcstat.h"
#include "pcicmp.h"
#include "strings.h"
#include "loopback.h"

WORD loopback_mode = LBACK_MODE_ENABLE;

#if defined(USE_LOOPBACK)

#if 0

  A sketch of calls involved in sending/receiving an IP-packet


  Sending an IP-packet            |             Receiving an IP-packet
  --------------------------------|-----------------------------------
      |                           |                     |
      |                           |               tcp_tick()
      |                           |                     |
   _eth_formatpacket() ->buffer X |               _eth_arrived()
      |                           |                     |
   _ip4_output()                  |               poll_recv_queue()
      |                           |                     |
      |                           |                     |
   _eth_send()                    enqueue copy of       |
      |                           X to IP-queue         |
      *----->loopback_device()--*-->>>>>>-<-------pkt_poll_recv()
      |      (1)                | |   |            |    |
      |                         | |   |            |    |
      |                         | |   |            |    |
      |                         | |   |            |    |
   _pkt_send()    drop buffer X \ |   |          none _ip4_handler()
      |                           |   |            |    |
      |                           |   |            \  ip4_defragment()
      |                           |   |                 |
      |                           |   |               protocol handlers
      |                           |   |                 |
      |                           |   |               _eth_free()
      v                           |   |                 |
  packet-driver                   |   `-----------<-- pkt_free_pkt()
      |                           |                     |
  Ethernet/PPP                                          \

  From the above it evident the current design cannot easily send
  IP-fragments. _ip4_output() must be rewritten to handle max. 64kB
  packets, split them in make_fragments(), format a link-layer packet
  using _eth_formatpacket() and repetively call _eth_send().

  (1): A future Win32 version could try enqueueing to the Winsock
       loopback handler. A TDI provider??
#endif


int (W32_CALL *loopback_handler) (in_Header*) = NULL;

static int icmp_loopback (ICMP_PKT   *icmp, unsigned icmp_len);
static int udp_loopback  (udp_Header *udp,  unsigned udp_len);

/**
 * \def loopback_device().
 *
 * We have been called with an IP-packet located on stack by `send_loopback()'
 * (in pcsed.c). Hence, it's safe to call _eth_send() again here.
 * MAC-header is in front of 'ip'.
 */
int loopback_device (in_Header *ip)
{
  int   ip_len = 0;
  DWORD ip_dst;
  DWORD ip_ofs;
  WORD  ip_flg;

  if (!(loopback_mode & LBACK_MODE_ENABLE))
     return (-1);

  if (ip->ver == 4)
  {
    int ip_hlen;

    if (!_chk_ip4_header(ip))       /* silently discard */
       return (-1);

    ip_hlen = in_GetHdrLen (ip);    /* length of IP-header (w/options) */
    ip_len  = intel16 (ip->length); /* total length of IP-packet */
    ip_dst  = ip->destination;

    ip->destination = ip->source;   /* swap source and destination */
    ip->source      = ip_dst;
    ip->checksum    = 0;
    ip->checksum    = ~CHECKSUM (ip, ip_hlen);   /* redo check-sum */

    ip_ofs = intel16 (ip->frag_ofs);
    ip_flg = (WORD) (ip_ofs & ~IP_OFFMASK);
    ip_ofs = (ip_ofs & IP_OFFMASK) << 3; /* 0 <= ip_ofs <= 65536-8 */

    if (ip_ofs || (ip_flg & IP_MF)) /* fragment; let ip4_defragment() */
       return (ip_len);             /* handle it on next poll */

    if (ip->proto == ICMP_PROTO)
    {
      ICMP_PKT *icmp = (ICMP_PKT*) ((BYTE*)ip + ip_hlen);
      int       len  = icmp_loopback (icmp, ip_len - ip_hlen);

      if (len > 0)
         return (ip_hlen+len);
    }
    else if (ip->proto == UDP_PROTO)
    {
      udp_Header *udp = (udp_Header*) ((BYTE*)ip + ip_hlen);
      int         len = udp_loopback (udp, ip_len-ip_hlen);

      if (len > 0)
         return (ip_hlen+len);
    }
  }
#if defined(USE_IPV6)
  else if (ip->ver == 6)
  {
    in6_Header *ip6 = (in6_Header*)ip;
    ip6_address ip6_dst;

    ip_len = intel16 (ip6->len);
    memcpy (&ip6_dst, &ip6->destination, sizeof(ip6_dst));
    memcpy (&ip6->destination, &ip6->source, sizeof(ip6->destination));
    memcpy (&ip6->source, &ip6_dst, sizeof(ip6->source));

    if (ip6->next_hdr == IP6_NEXT_ICMP)
    {
      ICMP_PKT *icmp = (ICMP_PKT*) (ip6 + 1);
      int       len  = icmp_loopback (icmp, ip_len);

      if (len > 0)
         return (len + sizeof(*ip6));
    }
    else if (ip6->next_hdr == UDP_PROTO)
    {
      udp_Header *udp = (udp_Header*) (ip6 + 1);
      int         len = udp_loopback (udp, ip_len);

      if (len > 0)
         return (len + sizeof(*ip6));
    }
  }
  else
  {
    (*_printf) ("%s: Illegal IP-packet (ver %d) for loopback device\n",
                __FILE__, ip->ver);
    return (-1);
  }
#endif

  if (loopback_handler)
     ip_len = (*loopback_handler) (ip);
  return (ip_len);
}

static int icmp_loopback (ICMP_PKT *icmp, unsigned icmp_len)
{
  if (icmp_len >= sizeof(icmp->echo)    &&
      CHECKSUM(icmp,icmp_len) == 0xFFFF &&
      icmp->echo.type == ICMP_ECHO)
  {
    static WORD echo_seq_num = 0;

    icmp->echo.type     = ICMP_ECHOREPLY;
    icmp->echo.sequence = echo_seq_num++;
    icmp->echo.checksum = 0;
    icmp->echo.checksum = ~CHECKSUM (icmp, icmp_len);
    STAT (icmpstats.icps_reflect++);
    STAT (icmpstats.icps_outhist[ICMP_ECHOREPLY]++);
    return (icmp_len);
  }
  /** \todo fall-through to another ICMP loopback handler */

  return (0);
}

static int udp_loopback (udp_Header *udp, unsigned udp_len)
{
  if (intel16(udp->dstPort) == IPPORT_ECHO)
  {
    /** \todo UDP loopback handler should handle ECHO protocol */
  }
  else if (intel16(udp->dstPort) == IPPORT_DISCARD)
  {
    /** \todo UDP loopback handler should handle DISCARD protocol */
  }
  ARGSUSED (udp_len);
  return (0);
}

/*
 *  An example loopback handler for RPC Portmapper (udp port 111)
 *  (Maybe an contradiction in terms to do RPC locally :-)
 *
 *  int (W32_CALL *old_loopback) (const in_Header*) = NULL;
 *
 *  static W32_CALL int pmap_loopback (const in_Header *ip)
 *  {
 *    WORD  hlen = in_GetHdrLen (ip);
 *    const udp_Header *udp = (const udp_Header*) ((BYTE*)ip + hlen);
 *
 *    if (ip->proto == UDP_PROTO && intel16(udp->dstPort) == 111)
 *       return do_portmapper (udp); // return length of IP reply packet
 *
 *    if (old_loopback)
 *       return (*old_loopback) (ip);
 *    return intel16 (ip->length);
 *  }
 *
 *  void init_pmap_loopback (void)
 *  {
 *    old_loopback = loopback_handler;
 *    loopback_handler = pmap_loopback;
 *  }
 */

#endif /* USE_LOOPBACK */
