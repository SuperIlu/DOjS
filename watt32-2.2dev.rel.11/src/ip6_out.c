/*!\file ip6_out.c
 *  IPv6 output routines.
 */

/*
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
 *  Version
 *
 *  0.5 : Jul 29, 2002 : G. Vanem - created
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "wattcp.h"
#include "misc.h"
#include "chksum.h"
#include "pcsed.h"
#include "ip4_out.h"
#include "ip6_out.h"
#include "socket.h"

/* \if USE_IPV6 */
#if defined(USE_IPV6)

struct in6_addr in6addr_my_ip = {{ 0,0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,0 }};

const struct in6_addr in6addr_loopback = {{ 0,0,0,0,0,0,0,0,
                                            0,0,0,0,0,0,0,1 }};
const struct in6_addr in6addr_any      = {{ 0,0,0,0,0,0,0,0,
                                            0,0,0,0,0,0,0,0 }};
const struct in6_addr in6addr_all_1    = {{ 0xFF,0xFF,0xFF,0xFF,
                                            0xFF,0xFF,0xFF,0xFF,
                                            0xFF,0xFF,0xFF,0xFF,
                                            0xFF,0xFF,0xFF,0xFF }};
/* all nodes multicast address */
const struct in6_addr in6addr_alln_mc  = {{ 0xFF,2,0,0,0,0,0,0,
                                            0,0,0,0,0,0,0,1 }};
/* all routers multicast address */
const struct in6_addr in6addr_allr_mc  = {{ 0xFF,2,0,0,0,0,0,0,
                                            0,0,0,0,0,0,0,2 }};

const BYTE in6addr_mapped[12] = { 0,0,0,0, 0,0,0,0, 0,0,0xFF,0xFF };

int  _default6_ttl = 255;    /* TTL on outgoing packets */

int _ip6_output (in6_Header  *ip,       /* IPv6-structure to fill in */
                 ip6_address *src_ip,   /* Source address */
                 ip6_address *dst_ip,   /* Destination address */
                 BYTE         protocol, /* Upper-level protocol */
                 unsigned     data_len, /* Length of data after IPv6 header */
                 int          hop_lim,  /* Hop limit, 0 means default */
                 const void  *sock,     /* Which socket is this */
                 const char  *file,     /* Debug: from what file */
                 unsigned     line)     /*  and line we where called */
{
  unsigned total = data_len + sizeof(*ip);

  if (total > _mtu)
     return (0);

  if (!src_ip)
     src_ip = (ip6_address*)&in6addr_my_ip;

  ip->pri       = 0;
  ip->ver       = 6;
  ip->len       = intel16 ((WORD)data_len);
  ip->hop_limit = hop_lim ? hop_lim : _default6_ttl;

  if (protocol == 0 || data_len == 0)  /* Only an IP6 header */
       ip->next_hdr = IP6_NEXT_NONE;
  else ip->next_hdr = protocol;

  memset (&ip->flow_lbl, 0, sizeof(ip->flow_lbl));

  if (&ip->source != src_ip)   /* assumes ptr set by caller */
     memcpy (&ip->source, src_ip, sizeof(ip->source));

  if (&ip->destination != dst_ip)
     memcpy (&ip->destination, dst_ip, sizeof(ip->destination));

  if (IN6_IS_ADDR_LOOPBACK(&ip->destination) ||
      IN6_IS_ADDR_MULTICAST(&ip->source))
     return (0);

  /** \todo: Support fragments
   */
  return _eth_send (total, sock, file, line);
}
#endif  /* USE_IPV6 */
/* \endif */
