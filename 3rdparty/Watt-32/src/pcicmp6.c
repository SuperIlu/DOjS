/*!\file pcicmp6.c
 *
 *  ICMP routines for IPv6 (RFC-2461/2463).
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
 *  Version
 *
 *  0.5 : Aug 01, 2002 : G. Vanem - created
 *
 *  Lot of works remains to get routing working.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "wattcp.h"
#include "misc.h"
#include "misc_str.h"
#include "timer.h"
#include "chksum.h"
#include "netaddr.h"
#include "language.h"
#include "ip6_in.h"
#include "ip6_out.h"
#include "ip4_out.h"
#include "split.h"
#include "pctcp.h"
#include "pcsed.h"
#include "pcstat.h"
#include "pcdbug.h"
#include "pcpkt.h"
#include "pcarp.h"
#include "pcconfig.h"
#include "pcicmp6.h"

/** \if USE_IPV6 */
#if defined(USE_IPV6)  /* Rest of file */

struct prefix_table {
       size_t      len;
       ip6_address prefix;
     };

struct prefix_table prefix_list [10];
struct icmp6_cache  neighbor_cache [ND_CACHE_SIZE];
struct icmp6_cache  destin_cache   [ND_CACHE_SIZE];

int         icmp6_prefix_len;
ip6_address icmp6_prefix;
DWORD       icmp6_6to4_gateway = 0;  /* host order */

W32_CLANG_PACK_WARN_OFF()

#include <sys/pack_on.h>

struct _pkt {
       in6_Header  in;
       ICMP6_PKT   icmp;
       BYTE        options[1];
     };

#include <sys/pack_off.h>

W32_CLANG_PACK_WARN_DEF()


#if 0
static const char *icmp6_options_str [] = {
                  "",
                  __LANG ("source link-layer addr"),
                  __LANG ("target link-layer addr"),
                  __LANG ("prefix information"),
                  __LANG ("redirected header"),
                  __LANG ("MTU")
          };
#endif

static WORD route_lifetime;

static void icmp6_print (int dbg_lvl, const char *msg, const void *src);
static int  icmp6_send  (in6_Header *ip, ICMP6_PKT *icmp, UINT len);
static void router_advert (const union ICMP6_PKT *icmp, unsigned len);
static int  neighbor_advert (const eth_address *eth);
static void echo_reply (const in6_Header *ip, const union ICMP6_PKT *icmp);

/*
 * Make a Ethernet destination address from a (solicited node) ip-address.
 * From /etc/manuf: 33:33:0:0:0:0/16, IPv6 neighbor discovery.
 */
static const void *icmp6_mac_addr (const void *ip_addr)
{
  static mac_address mac;
  const  BYTE       *addr = (const BYTE*)ip_addr;

  mac[0] = 0x33;
  mac[1] = 0x33;
  mac[2] = 0xFF;
  mac[3] = addr[13];
  mac[4] = addr[14];
  mac[5] = addr[15];
  return (&mac[0]);
}

int icmp6_router_solicitation (void)
{
  struct _pkt            *pkt;
  struct in6_Header      *ip;
  struct ICMP6_route_sol *icmp;

  pkt  = (struct _pkt*) _eth_formatpacket (_eth_brdcast, IP6_TYPE);
  ip   = &pkt->in;
  icmp = &pkt->icmp.rsolic;
  icmp->type     = ND_ROUTER_SOLICIT;
  icmp->code     = 0;
  icmp->reserved = 0;

  memcpy (&ip->source, &in6addr_my_ip, sizeof(ip->source));
  memcpy (&ip->destination, &in6addr_allr_mc, sizeof(ip->destination));
  return icmp6_send (ip, (ICMP6_PKT*)icmp, sizeof(*icmp));
}

/*
 * Add the IPv4 'icmp6_6to4_gateway' address to the ND cache
 */
BOOL icmp6_add_gateway4 (void)
{
  struct icmp6_cache *ce = NULL;
  ip6_address         ip;
  mac_address         eth;
  BOOL                arp_ok;

  arp_ok = _arp_resolve (icmp6_6to4_gateway, &eth);
  if (arp_ok)
  {
    memcpy (&ip, in6addr_mapped, sizeof(in6addr_mapped));
    *(DWORD*) &ip[12] = intel (icmp6_6to4_gateway);
    ce = icmp6_ncache_insert_fix (&ip, eth);
  }
  TRACE_CONSOLE (1, "Adding %s as 6-to-4 gateway, ARP %s, Insert %s\n",
                 _inet_ntoa(NULL,intel(icmp6_6to4_gateway)),
                 arp_ok ? "OK" : "failed",
                 ce     ? "OK" : "failed");
  return (arp_ok && ce);
}

/*
 * Perform an "ICMPv6 Neighbor Solicitation" on 'addr' to resolve it's
 * MAC address (the role of ARP in IPv4).
 */
int icmp6_neigh_solic (const void *addr, eth_address *eth)
{
  struct _pkt         *pkt;
  struct in6_Header   *ip;
  struct ICMP6_nd_sol *icmp;
  struct icmp6_cache  *cache;
  struct in6_addr      dest;
  BYTE  *options;

  WATT_ASSERT (eth != NULL);

  if (_ip6_is_local_addr(addr))
  {
    memcpy (eth, _eth_addr, sizeof(*eth));
    return (1);
  }

  cache = icmp6_ncache_lookup (addr);
  if (cache)
  {
    memcpy (eth, &cache->eth, sizeof(*eth));
    return (1);
  }

  pkt  = (struct _pkt*) _eth_formatpacket (icmp6_mac_addr(addr), IP6_TYPE);
  ip   = &pkt->in;
  icmp = &pkt->icmp.nd_solic;
  icmp->type     = ND_NEIGHBOR_SOLICIT;
  icmp->code     = 0;
  icmp->reserved = 0;

  memcpy (&dest, &in6addr_alln_mc, sizeof(dest)); /* All nodes MC address */
  dest.s6_addr[11] ^= 1;             /* toggle G-bit */
  dest.s6_addr[12]  = 0xFF;
  dest.s6_addr[13]  = ((BYTE*)addr)[13];
  dest.s6_addr[14]  = ((BYTE*)addr)[14];
  dest.s6_addr[15]  = ((BYTE*)addr)[15];

  memcpy (&icmp->target, addr, sizeof(icmp->target));
  memcpy (&ip->source, &in6addr_my_ip, sizeof(ip->source));
  memcpy (&ip->destination, &dest, sizeof(ip->destination));

  /* Insert "src-mac address" option
   */
  options = (BYTE*) (icmp + 1);
  *options++ = ND_OPT_SOURCE_LINKADDR;
  *options++ = 1;   /* multiple of 8 bytes */
  memcpy (options, _eth_addr, sizeof(_eth_addr));
  options += sizeof(_eth_addr);
  *options++ = 0;   /* End option */
  *options++ = 0;

  icmp6_send (ip, (ICMP6_PKT*)icmp, options - (BYTE*)icmp);

#if 0
  while (1)
  {
    eth = ..
  }
#endif

  icmp6_ncache_insert (addr, eth);

  /** \todo wait for Neighbor Advertisement */
  return (0);
}

void icmp6_handler (const in6_Header *ip)
{
  const struct pkt_split *pkt;
  const union  ICMP6_PKT *icmp;
  int          type, code;
  unsigned     len;
  BOOL         for_me;

  DEBUG_RX (NULL, ip);

  pkt    = pkt_get_type_in (TYPE_IP6_ICMP);
  icmp   = (const union ICMP6_PKT*) pkt->data;
  len    = pkt->len;
  for_me = !memcmp (ip->destination, &in6addr_my_ip, sizeof(ip->destination));

  if (len < sizeof(icmp->unused))
  {
    STAT (icmp6stats.icp6s_tooshort++);
    return;
  }

  if (!_ip6_icmp_checksum(ip,icmp,intel16(ip->len)))
  {
    STAT (icmp6stats.icp6s_checksum++);
    icmp6_print (1, _LANG("bad checksum"), ip->source);
    return;
  }

  type = icmp->unused.type;
  code = icmp->unused.code;

  switch (type)
  {
    case ICMP6_DST_UNREACH:
         break;

    case ICMP6_PACKET_TOO_BIG:
         break;

    case ICMP6_TIME_EXCEEDED:
         break;

    case ICMP6_PARAM_PROB:
         break;

    case ICMP6_ECHO_REQUEST:
         if (for_me)
         {
           icmp6_print (2, _LANG("PING6 requested of us"), ip->source);
           echo_reply (ip, icmp);
         }
         break;

    case ICMP6_ECHO_REPLY:
         break;

    case ICMP6_MEMBERSHIP_QUERY:
         break;

    case ICMP6_MEMBERSHIP_REPORT:
         break;

    case ICMP6_MEMBERSHIP_REDUCTION:
         break;

    case ICMP6_ROUTER_RENUMBERING:
         break;

    case ICMP6_WRUREQUEST:
         break;

    case ICMP6_WRUREPLY:
         break;

    case ND_ROUTER_SOLICIT:   /* silently discard */
         break;

    case ND_ROUTER_ADVERT:
         if (IN6_IS_ADDR_LINKLOCAL(ip->source) && ip->hop_limit == 255)
            router_advert (icmp, len);
         break;

    case ND_NEIGHBOR_SOLICIT:
         if (IN6_ARE_ADDR_EQUAL(icmp->nd_solic.target, &in6addr_my_ip))
            neighbor_advert ((const eth_address*)MAC_SRC(ip));
         break;

    case ND_NEIGHBOR_ADVERT:
         break;

    case ND_REDIRECT:
         break;
  }
  ARGSUSED (code);
}

void icmp6_unreach (const in6_Header *ip, int code)
{
  ARGSUSED (ip);
  ARGSUSED (code);
}

struct icmp6_cache *icmp6_ncache_lookup (const void *ip)
{
  struct icmp6_cache *entry = &neighbor_cache[0];
  time_t now = time (NULL);
  int    i;

  for (i = 0; i < DIM(neighbor_cache); i++, entry++)
  {
#if 0  /* !! test */
    printf ("%2d: ip '%s', entry '%s'\n",
            i, _inet6_ntoa(ip), _inet6_ntoa(entry->ip));
#endif

    if (entry->state != ND_CACHE_REACHABLE)
       continue;

    if (entry->expiry && now >= entry->expiry)
    {
      entry->state  = ND_CACHE_UNUSED;
      entry->expiry = 0;
      continue;
    }

#if 0  /* test !! */
    if (*(DWORD*)&entry->ip[12] == intel(icmp6_6to4_gateway))
       return (entry);
#endif

    if (IN6_IS_ADDR_V4MAPPED(ip) &&
        *(DWORD*)&entry->ip[12] == intel(icmp6_6to4_gateway))
       return (entry);

    if (IN6_ARE_ADDR_EQUAL(ip,entry->ip))
       return (entry);
  }
  return (NULL);
}

struct icmp6_cache *icmp6_ncache_insert (const void *ip, const void *eth)
{
  struct icmp6_cache *found = NULL;
  struct icmp6_cache *entry = &neighbor_cache[0];
  static WORD         index = 0;       /* rotates round-robin */
  int    i;

  for (i = 0; i < DIM(neighbor_cache); i++, entry++)
  {
    if (entry->state == ND_CACHE_PROBE || IN6_IS_ADDR_UNSPECIFIED(entry->ip))
    {
      found = entry;
      break;
    }
  }

  if (!found)   /* not found, grab a "random" slot */
  {
    index++;
    index %= DIM (neighbor_cache);
    found = &neighbor_cache [index];
  }
  memcpy (found->ip, ip, sizeof(found->ip));
  memcpy (found->eth, eth, sizeof(found->eth));
  found->state  = ND_CACHE_INCOMPLETE;
  found->expiry = time (NULL);
  return (found);
}

struct icmp6_cache *icmp6_ncache_insert_fix (const void *ip, const void *eth)
{
  struct icmp6_cache *ce = icmp6_ncache_insert (ip, eth);

  if (ce)
  {
    ce->state  = ND_CACHE_REACHABLE; /* always reachable */
    ce->expiry = 0;                  /* never expire */
  }
  return (ce);
}

static void icmp6_print (int dbg_lvl, const char *msg, const void *src)
{
  if (debug_on < dbg_lvl)
     return;

  outs ("\nICMP6: ");
  if (src)
  {
    const char *addr = _inet6_ntoa (src);
    outs ("(");
    outs (addr ? addr : "(null?)");
    outs ("): ");
  }
  outsnl (_LANG(msg));
}

static int icmp6_send (in6_Header *ip, ICMP6_PKT *icmp, UINT len)
{
  icmp->unused.checksum = 0;
  icmp->unused.checksum = _ip6_checksum (ip, IP6_NEXT_ICMP, icmp, len);
  return IP6_OUTPUT (ip, &ip->source, &ip->destination, IP6_NEXT_ICMP,
                     len, 0, NULL);
}

static void parse_options (const BYTE *opt, unsigned max)
{
  const BYTE *start = opt;

  while (opt < start + max)
  {
    const struct nd_opt_prefix_info *pi = (const struct nd_opt_prefix_info*) opt;
    BYTE  type = opt[0];
    int   len  = opt[1] << 3;

    switch (type)
    {
      case ND_OPT_PREFIX_INFORMATION:
           if (pi->nd_opt_pi_len == 4)
           {
             icmp6_prefix_len = pi->nd_opt_pi_prefix_len;
             memcpy (&icmp6_prefix, &pi->nd_opt_pi_prefix, sizeof(icmp6_prefix));
           }
           break;
      case ND_OPT_SOURCE_LINKADDR:
      case ND_OPT_TARGET_LINKADDR:
      case ND_OPT_REDIRECTED_HEADER:
      case ND_OPT_MTU:
      case 0:          /* EOL */
           break;
      default:
           icmp6_print (1, _LANG("Illegal option"), NULL);
           break;
    }
    opt += len;
  }
}

static void router_advert (const union ICMP6_PKT *icmp, unsigned len)
{
  if (icmp->radvert.hop_limit)
     _default6_ttl = icmp->radvert.hop_limit;
  if (icmp->radvert.lifetime)
     route_lifetime = intel16 (icmp->radvert.lifetime);
  if (len > sizeof(icmp->radvert))
     parse_options ((const BYTE*)icmp + sizeof(icmp->radvert),
                    len - sizeof(icmp->radvert));
}

static int neighbor_advert (const eth_address *eth)
{
  struct _pkt         *pkt;
  struct in6_Header   *ip;
  struct ICMP6_nd_adv *icmp;

  pkt  = (struct _pkt*) _eth_formatpacket (eth, IP6_TYPE);
  ip   = &pkt->in;
  icmp = &pkt->icmp.nd_adv;
  icmp->type      = ND_NEIGHBOR_ADVERT;
  icmp->code      = 0;
  icmp->reserved1 = 0;
  icmp->reserved2 = 0;
  icmp->reserved3 = 0;
  icmp->router    = 0;
  icmp->solicited = 1;
  icmp->override  = 1;

  memcpy (&icmp->target, &in6addr_my_ip, sizeof(icmp->target));
  memcpy (&ip->source, &in6addr_my_ip, sizeof(ip->source));
  memcpy (&ip->destination, &in6addr_allr_mc, sizeof(ip->destination));
  return icmp6_send (ip, (ICMP6_PKT*)icmp, sizeof(*icmp));
}

static void echo_reply (const in6_Header      *orig_ip,
                        const union ICMP6_PKT *orig_icmp)
{
  struct _pkt       *pkt;
  struct in6_Header *ip;
  union  ICMP6_PKT  *icmp;
  WORD   len = intel16 (orig_ip->len);

  pkt  = (struct _pkt*) _eth_formatpacket (MAC_SRC(orig_ip), IP6_TYPE);
  ip   = &pkt->in;
  icmp = &pkt->icmp;

  /* Don't let a huge reassembled ICMP-packet kill us.
   */
  len = min (len, _mtu - sizeof(*ip));
  memcpy (icmp, orig_icmp, len);
  icmp->echo.type = ICMP6_ECHO_REPLY;
  icmp->echo.code = orig_icmp->echo.code;

  /* Use supplied ip values in case we ever multi-home.
   * Note that ip values are still in network order.
   */
  memcpy (&ip->source, &in6addr_my_ip, sizeof(ip->source));
  memcpy (&ip->destination, &orig_ip->source, sizeof(ip->destination));
  ip->hop_limit = orig_ip->hop_limit;

  icmp6_print (2, _LANG("PING reply sent"), NULL);
  icmp6_send (ip, (ICMP6_PKT*)icmp, len);
}
#endif   /* USE_IPV6 */
/** \endif */
