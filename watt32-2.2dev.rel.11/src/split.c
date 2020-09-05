/*!\file split.c
 *
 *  Splitting of link-layer packets into components.
 */

/*  Copyright (c) 1997-2002 Gisle Vanem <gvanem@yahoo.no>
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
 *  G. Vanem <gvanem@yahoo.no> 2002
 */

#include <stdio.h>
#include <stdlib.h>

#include "wattcp.h"
#include "pcsed.h"
#include "pcpkt.h"
#include "pppoe.h"
#include "ppp.h"
#include "misc.h"
#include "strings.h"
#include "bsddbug.h"
#include "ip6_in.h"
#include "split.h"

#if defined(NEED_PKT_SPLIT)

#define MARKER     0xDEAFABBA
#define MAX_SPLITS 10

static struct pkt_split split_list_in [MAX_SPLITS];
static struct pkt_split split_list_out[MAX_SPLITS];

static void split_mac (struct pkt_split *, const void *);
static void split_ip4 (struct pkt_split *, const in_Header *);
static void split_ip6 (struct pkt_split *, const in6_Header *);

const struct pkt_split *pkt_get_split_in (void)
{
  WATT_ASSERT (*(DWORD*)&split_list_in[MAX_SPLITS-1] == MARKER);
  return (&split_list_in[0]);
}

const struct pkt_split *pkt_get_split_out (void)
{
  WATT_ASSERT (*(DWORD*)&split_list_out[MAX_SPLITS-1] == MARKER);
  return (&split_list_out[0]);
}

const struct pkt_split *pkt_get_type_in (enum Components type)
{
  const struct pkt_split *ps;

  for (ps = pkt_get_split_in(); ps->data; ps++)
      if (type == ps->type)
         return (ps);
  return (NULL);
}

const struct pkt_split *pkt_get_type_out (enum Components type)
{
  const struct pkt_split *ps;

  for (ps = pkt_get_split_out(); ps->data; ps++)
      if (type == ps->type)
         return (ps);
  return (NULL);
}

const struct pkt_split *pkt_split_mac_in (const void *ptr)
{
  *(DWORD*) &split_list_in[MAX_SPLITS-1] = MARKER;
  split_mac (&split_list_in[0], ptr);
  return (&split_list_in[0]);
}

const struct pkt_split *pkt_split_mac_out (const void *ptr)
{
  *(DWORD*) &split_list_out[MAX_SPLITS-1] = MARKER;
  split_mac (&split_list_out[0], ptr);
  return (&split_list_out[0]);
}

static void split_mac (struct pkt_split *ps, const void *ptr)
{
  const link_Packet *pkt = (const link_Packet*) ptr;
  const in_Header   *ip  = NULL;
  WORD  type = 0;

  if (_pktserial)    /* SLIP/PPP/AX25 doesn't have MAC header */
  {
    ip = &pkt->ip.head;
    if (ip->ver == 6)
         split_ip6 (ps, (const in6_Header*)ip);
    else split_ip4 (ps, ip);
    return;
  }

  if (_pktdevclass == PDCLASS_TOKEN)
  {
    const tok_Packet *tr = &pkt->tok;

    ps->type = TYPE_TOKEN_HEAD;
    ps->data = pkt;
    ps->len  = sizeof(tok_Header);

    ip   = (const in_Header*) &tr->data[0];
    type = tr->head.type;
  }
  else if (_pktdevclass == PDCLASS_FDDI)
  {
    const fddi_Packet *fddi = &pkt->fddi;

    ps->type = TYPE_FDDI_HEAD;
    ps->data = pkt;
    ps->len  = sizeof(fddi_Header);

    ip   = (const in_Header*) &fddi->data[0];
    type = fddi->head.type;
  }
  else if (_pktdevclass == PDCLASS_ARCNET)
  {
    const arcnet_Packet *arc = &pkt->arc;

    ps->type = TYPE_ARCNET_HEAD;
    ps->data = pkt;
    ps->len  = ARC_HDRLEN;

    ip   = (const in_Header*) ((BYTE*)arc + ARC_HDRLEN);
    type = arc->head.type;
  }
  else if (_pktdevclass == PDCLASS_ETHER)
  {
    ps->type = TYPE_ETHER_HEAD;
    ps->data = pkt;
    ps->len  = sizeof(eth_Header);

    ip   = (const in_Header*) &pkt->eth.data[0];
    type = pkt->eth.head.type;
  }

  if (type == IP4_TYPE)
  {
    ps++;
    split_ip4 (ps, ip);
    return;
  }

  if (type == IP6_TYPE)
  {
    ps++;
    split_ip6 (ps, (const in6_Header*)ip);
    return;
  }

  if (type == ARP_TYPE)
  {
    ps++;
    ps->type = TYPE_ARP;
    ps->data = &pkt->eth.data[0];
    ps->len  = sizeof(arp_Header);
  }
  else if (type == RARP_TYPE)
  {
    ps++;
    ps->type = TYPE_RARP;
    ps->data = &pkt->eth.data[0];
    ps->len  = sizeof(rarp_Header);
  }
  else if (type == PPPOE_DISC_TYPE)
  {
    pppoe_Packet *pppoe = (pppoe_Packet*)&pkt->eth.data[0];

    ps++;
    ps->type = TYPE_PPPOE_DISC;
    ps->data = pppoe;
    ps->len  = intel16 (pppoe->length);
  }
  else if (type == PPPOE_SESS_TYPE)
  {
    const pppoe_Packet *pppoe = (const pppoe_Packet*)&pkt->eth.data[0];
    WORD                proto = intel16 (*(WORD*)&pppoe->data[0]);

    ps++;
    ps->type = TYPE_PPPOE_SESS;
    ps->data = pppoe;
    ps->len  = PPPOE_HDR_SIZE;
    if (proto == intel16(PPP_IP))
    {
      ip = (const in_Header*)&pppoe->data[2];
      split_ip4 (++ps, ip);
      return;
    }
    if (proto == PPP_LCP)
    {
      const char *bp = (const char*) &pppoe->data[2];
      ps++;
      ps->type = TYPE_PPP_LCP;
      ps->data = bp;
      ps->len  = intel16 (*(WORD*)(bp+2));
    }
    else if (proto == PPP_IPCP)
    {
      const char *bp = (const char*) &pppoe->data[2];
      ps++;
      ps->type = TYPE_PPP_IPCP;
      ps->data = bp;
      ps->len  = intel16 (*(WORD*)(bp+2));
    }
  }
  else if (intel16(type) < ETH_MAX_DATA)   /* LLC unsupported */
  {
    ps++;
    ps->type = TYPE_LLC_HEAD;
    ps->len  = intel16 (type);
    ps->data = &pkt->eth.data[0];
  }

  ps++;
  ps->data = NULL;
}

static void split_ip4 (struct pkt_split *ps, const in_Header *ip4)
{
  unsigned ip_len, ip_ofs, head_len;
  int      opt_len;

  head_len = in_GetHdrLen (ip4);
  opt_len  = head_len - sizeof(*ip4);
  ip_ofs   = intel16 (ip4->frag_ofs);
  ip_ofs   = (ip_ofs & IP_OFFMASK) << 3;  /* 0 <= ip_ofs <= 65536-8 */

  if (head_len < sizeof(*ip4))
     goto quit;

  ps->type = TYPE_IP4;
  ps->len  = head_len;
  ps->data = ip4;

  if (opt_len > 0)
  {
    ps++;
    ps->type = TYPE_IP4_OPTIONS;
    ps->data = (ip4 + 1);
    ps->len  = opt_len;
  }

  if (ip_ofs)
  {
    ps++;
    ps->type = TYPE_IP4_FRAG;
    ps->data = (BYTE*)ip4 + head_len;
    ps->len  = intel16 (ip4->length) - head_len;
    goto quit;
  }

  if (ip4->proto == ICMP_PROTO)
  {
    ps++;
    ps->type = TYPE_ICMP;
    ps->data = (BYTE*)ip4 + head_len;
    ps->len  = intel16 (ip4->length) - head_len;
  }

  else if (ip4->proto == IGMP_PROTO)
  {
    ps++;
    ps->type = TYPE_IGMP;
    ps->data = (BYTE*)ip4 + head_len;
    ps->len  = intel16 (ip4->length) - head_len;
  }

  else if (ip4->proto == UDP_PROTO)
  {
    const udp_Header *udp = (const udp_Header*) ((BYTE*)ip4 + head_len);
    unsigned          len = intel16 (udp->length) - sizeof(*udp);

    ps++;
    ps->type = TYPE_UDP_HEAD;
    ps->data = (BYTE*)ip4 + head_len;
    ps->len  = sizeof (*udp);
    ps++;
    ps->type = TYPE_UDP_DATA;
    ps->data = (BYTE*)ip4 + head_len + sizeof(*udp);
    ps->len  = min (len, intel16(ip4->length) - head_len - sizeof(*udp));
  }

  else if (ip4->proto == TCP_PROTO)
  {
    const tcp_Header *tcp = (const tcp_Header*) ((BYTE*)ip4 + head_len);
    unsigned          ofs = tcp->offset << 2;
    unsigned          dlen;

    ip_len = intel16 (ip4->length) - sizeof(*ip4);
    ps++;
    ps->type = TYPE_TCP_HEAD;
    ps->data = tcp;
    ps->len  = sizeof (*tcp);
    if (ofs - sizeof(*tcp) > 0)
    {
      ps++;
      ps->type = TYPE_TCP_OPTIONS;
      ps->data = tcp + 1;
      ps->len  = ofs - sizeof(*tcp);
    }
    dlen = intel16 (ip4->length) - head_len - ofs;
    dlen = min (dlen, ip_len);
    ps++;
    ps->type = TYPE_TCP_DATA;
    ps->data = (const char*)tcp + ofs;
    ps->len  = dlen;   /* may be 0 */
  }

quit:
  ps++;
  ps->data = NULL;
}

static void split_ip6 (struct pkt_split *ps, const in6_Header *ip6)
{
#if defined(USE_IPV6)
  const tcp_Header   *tcp;
  const ip6_RouteHdr *rh;
  const BYTE         *bp = (const BYTE*)ip6;
  WORD  ip_len = intel16 (ip6->len);
  BYTE  nxt_hd = ip6->next_hdr;
  int   ofs, len;

  ps->type = TYPE_IP6;
  ps->len  = sizeof(*ip6);
  ps->data = ip6;
  ps++;

  while (nxt_hd && (bp < (const BYTE*)ip6 + ip_len))
  {
    switch (nxt_hd)
    {
      case IP6_NEXT_TCP:
           tcp = (const tcp_Header*) (ip6 + 1);
           ofs = tcp->offset << 2;
           ps->type = TYPE_TCP_HEAD;
           ps->data = tcp;
           ps->len  = sizeof (*tcp);
           if (ofs - sizeof(*tcp) > 0)
           {
             ps++;
             ps->type = TYPE_TCP_OPTIONS;
             ps->data = tcp + 1;
             ps->len  = ofs - sizeof(*tcp);
           }
           nxt_hd = 0;   /* no next headers allowed */
           break;

      case IP6_NEXT_UDP:
           ps->type = TYPE_UDP_HEAD;
           ps->data = ip6 + 1;
           ps->len  = sizeof (udp_Header);
           nxt_hd   = 0;   /* no next headers allowed */
           break;

      case IP6_NEXT_HOP:
           rh  = (struct ip6_RouteHdr*) (ip6 + 1);
           len = (rh->hdrlen + 1) << 3;
           ps->type = TYPE_IP6_HOP;
           ps->data = rh;
           ps->len  = len;
           bp      += len;
           nxt_hd   = *bp;
           break;

      case IP6_NEXT_ROUTING:
           rh  = (struct ip6_RouteHdr*) (ip6 + 1);
           len = (rh->hdrlen + 1) << 3;
           ps->type = TYPE_IP6_ROUTING;
           ps->data = rh;
           ps->len  = len;
           bp      += len;
           nxt_hd   = *bp;
           break;

      case IP6_NEXT_DEST:
           rh  = (struct ip6_RouteHdr*) (ip6 + 1);
           len = (rh->hdrlen + 1) << 3;
           ps->type = TYPE_IP6_DEST;
           ps->data = rh;
           ps->len  = len;
           bp      += len;
           nxt_hd   = *bp;
           break;

      case IP6_NEXT_ICMP:
           rh  = (struct ip6_RouteHdr*) (ip6 + 1);
           len = (rh->hdrlen + 1) << 3;
           ps->type = TYPE_IP6_ICMP;
           ps->data = rh;
           ps->len  = len;
           bp      += len;
           nxt_hd   = 0;
           break;

      case IP6_NEXT_NONE:
           ps->type = TYPE_IP6_NONE;
           ps->data = NULL;
           ps->len  = 0;
           nxt_hd   = 0;
           break;

      /** \todo Handle Fragment, ESP, AUTH, IPv6 encap */
      case IP6_NEXT_FRAGMENT:
      case IP6_NEXT_ESP:
      case IP6_NEXT_AUTH:
      case IP6_NEXT_IPV6:
           rh  = (struct ip6_RouteHdr*) (ip6 + 1);
           len = (rh->hdrlen + 1) << 3;
           ps->type = TYPE_IP6_UNSUPP;
           ps->data = NULL;
           ps->len  = 0;
           bp      += len;
           nxt_hd   = *bp;
           break;

      case IP6_NEXT_COMP:
      default:
           ps->type = TYPE_IP6_UNSUPP;
           ps->data = NULL;
           ps->len  = 0;
           nxt_hd   = 0;
           break;
    }
    ps++;
  }
#else
  ARGSUSED (ip6);
#endif        /* USE_IPV6 */

  ps->data = NULL;
}

#if defined(USE_DEBUG)
#if defined(USE_BSD_API)

static const struct search_list names[] = {
     { TYPE_TOKEN_HEAD,  "token"      },
     { TYPE_FDDI_HEAD,   "fddi"       },
     { TYPE_ETHER_HEAD,  "ether"      },
     { TYPE_ARP,         "arp"        },
     { TYPE_RARP,        "rarp"       },
     { TYPE_PPPOE_DISC,  "pppoe-disc" },
     { TYPE_PPPOE_SESS,  "pppoe-sess" },
     { TYPE_PPP_LCP,     "lcp"        },
     { TYPE_PPP_IPCP,    "ipcp"       },
     { TYPE_LLC_HEAD,    "llc"        },
     { TYPE_IP4,         "ip4"        },
     { TYPE_IP4_OPTIONS, "ip4-opt"    },
     { TYPE_IP4_FRAG,    "ip4-frag"   },
     { TYPE_ICMP,        "icmp"       },
     { TYPE_IGMP,        "igmp"       },
     { TYPE_UDP_HEAD,    "udp"        },
     { TYPE_UDP_DATA,    "udp-data"   },
     { TYPE_TCP_HEAD,    "tcp"        },
     { TYPE_TCP_OPTIONS, "tcp-opt"    },
     { TYPE_TCP_DATA,    "tcp-data"   },
     { TYPE_IP6,         "ip6"        },
     { TYPE_IP6_HOP,     "ip6-hop"    },
     { TYPE_IP6_IPV6,    "ip6-ip6"    },
     { TYPE_IP6_ROUTING, "ip6-route"  },
     { TYPE_IP6_FRAGMENT,"ip6-frag"   },
     { TYPE_IP6_ESP,     "ip6-esp"    },
     { TYPE_IP6_AUTH,    "ip6-auth"   },
     { TYPE_IP6_ICMP,    "icmp6"      },
     { TYPE_IP6_DEST,    "ip6-dest"   },
     { TYPE_IP6_NONE,    "ip6-none"   },
     { TYPE_IP6_UNSUPP,  "ip6-unsupp" }
   };
#endif

void pkt_print_split_in (void)
{
  const struct pkt_split *ps;

  SOCK_DEBUGF (("\nRx:\n"));
  for (ps = pkt_get_split_in(); ps->data; ps++)
      SOCK_DEBUGF (("%10s: %5u bytes\n",
                    list_lookup(ps->type, names, DIM(names)), ps->len));
}

void pkt_print_split_out (void)
{
  const struct pkt_split *ps;

  SOCK_DEBUGF (("\nTx:\n"));
  for (ps = pkt_get_split_out(); ps->data; ps++)
      SOCK_DEBUGF (("%10s: %5u bytes\n",
                    list_lookup(ps->type, names, DIM(names)), ps->len));
}
#endif /* USE_DEBUG */
#endif /* NEED_PKT_SPLIT */
