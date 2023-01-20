/*!\file ip6_in.c
 * IPv6 input routines.
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
#include <arpa/inet.h>

#include "wattcp.h"
#include "misc.h"
#include "misc_str.h"
#include "chksum.h"
#include "language.h"
#include "sock_ini.h"
#include "split.h"
#include "pctcp.h"
#include "pcsed.h"
#include "pcstat.h"
#include "pcdbug.h"
#include "pcpkt.h"
#include "pcconfig.h"
#include "pcicmp6.h"
#include "bsddbug.h"
#include "netaddr.h"
#include "get_xby.h"
#include "ip6_out.h"
#include "ip6_in.h"

/* \if USE_IPV6 */
#if defined(USE_IPV6)

/*
 * _ip6_handler - do a simple check on IPv6 header
 *              - Demultiplex packet to correct protocol handler
 */
int _ip6_handler (const in6_Header *ip, BOOL broadcast)
{
  const struct pkt_split *ps;

  if (block_ip)
     return (0);

  if (ip->ver != 6)
  {
    DEBUG_RX (NULL, ip);
    STAT (ip6stats.ip6s_badvers++);
    return (0);
  }

  /* Match 'ip' against all SOCK_RAW sockets before doing normal
   * protocol multiplexing below.
   */
  if (_bsd_socket_hook)
    (*_bsd_socket_hook) (BSO_IP6_RAW, ip);

  for (ps = pkt_get_split_in(); ps->data; ps++)
      switch (ps->type)
      {
        case TYPE_TOKEN_HEAD:
        case TYPE_FDDI_HEAD:
        case TYPE_ETHER_HEAD:
        case TYPE_IP6:
             break;

        case TYPE_TCP_HEAD:
             _tcp_handler ((const in_Header*)ip, broadcast);
             break;

        case TYPE_UDP_HEAD:
             _udp_handler ((const in_Header*)ip, broadcast);
             break;

        case TYPE_IP6_ICMP:
             icmp6_handler (ip);
             break;

        case TYPE_IP6_NONE:
             break;

        case TYPE_IP6_UNSUPP:
#if defined(USE_DEBUG)
             dbug_printf ("Received unsupported IPv6 message, next-hdr %d,"
                          " broadcast %d\n", ip->next_hdr, broadcast);
#endif
             break;

        default:
             DEBUG_RX (NULL, ip);
             if ((ps->type >= TYPE_IP6_HOP && ps->type <= TYPE_IP6_DEST) &&
                 !broadcast)
             {
               if (IN6_IS_ADDR_LINKLOCAL(&ip->destination))
                  icmp6_unreach (ip, 2);  /* protocol unreachable */
               STAT (ip6stats.ip6s_cantforward++);
             }
             break;
      }

#if 0
  if (ip->next_hdr != IP6_NEXT_ICMP)  /* !! bug hunt */
     dbug_printf ("IP6 packet, next-hdr %d, brdcast %d\n",
                  ip->next_hdr, broadcast);
#endif

  STAT (ip6stats.ip6s_delivered++);
  return (1);
}

/*
 * Return TRUE if IPv6 address is a local address or on the
 * loopback network (::1).
 */
int _ip6_is_local_addr (const void *ip)
{
  if (!memcmp(&in6addr_my_ip,ip,sizeof(in6addr_my_ip)) ||
      !memcmp(&in6addr_loopback,ip,sizeof(in6addr_loopback)))
    return (1);
  return (0);
}

#ifdef NOT_USED  /* use IN6_xx macros in <netinet/in.h> instead */
/*
 * Return true if address has a IPv4 prefix (0:0:0:0:0:FFFF)
 */
static WORD v4_prefix[6] = { 0,0,0,0,0,0xFFFF };

int _ip6_is_addr_v4mapped (const void *ip)
{
  if (!memcmp(ip,&v4_prefix,sizeof(v4_prefix)))
     return (1);
  return (0);
}

int _ip6_is_addr_loopback (const void *ip)
{
  if (!memcmp(ip,&in6addr_loopback,sizeof(in6addr_loopback)))
     return (1);
  return (0);
}
#endif  /* NOT_USED */


static void (W32_CALL *prev_hook) (const char*, const char*) = NULL;

static void set_my_ip6 (const char *value)
{
  if (!stricmp(value,"dhcp") || !stricmp(value,"dhcp6"))
     _dhcp6_on = 1;
  else if (!inet_pton(AF_INET6, value, (void*)&in6addr_my_ip))
     printf (_LANG("\"IP6.MY_IP\": Invalid IPv6 address \"%s\"\n"), value);
}

static void W32_CALL ip6_config (const char *name, const char *value)
{
  static const struct config_table ip6_cfg[] = {
            { "MY_IP",        ARG_FUNC,  (void*)set_my_ip6          },
            { "6TO4_GATEWAY", ARG_ATOIP, (void*)&icmp6_6to4_gateway },
            { "DEF_TTL",      ARG_ATOI,  (void*)&_default6_ttl      },
            { NULL,           0,         NULL                       }
          };
  if (!parse_config_table(&ip6_cfg[0], "IP6.", name, value) && prev_hook)
     (*prev_hook) (name, value);
}

int _ip6_init (void)
{
  prev_hook = usr_init;
  usr_init  = ip6_config;
  return (1);
}

/*
 * Sets default (preferred link-local) IPv6 address to
 * FE80::201:80FF:FE <last 3 bytes of MAC-address>
 */
static void set_default_ip (void)
{
  BYTE *a = (BYTE*) &in6addr_my_ip.s6_addr[0];

  memset (a, 0, sizeof(in6addr_my_ip));
  a[0]  = 0xFE;
  a[1]  = 0x80;
  a[8]  = 0x20;
  a[9]  = 0x01;
  a[10] = 0x80;
  a[11] = 0xFF;
  a[12] = 0xFE;
  a[13] = _eth_real_addr[3];
  a[14] = _eth_real_addr[4];
  a[15] = _eth_real_addr[5];
}

/*
 * Called from socket.c only (I.e. IPv6 requires BSD-sockets. Core
 * API cannot be used for IPv6).
 */
int _ip6_pkt_init (void)
{
  if (!memcmp(&in6addr_my_ip,&in6addr_any,sizeof(in6addr_my_ip)))
     set_default_ip();

  if (_pktserial)  /* IPv6 cannot work on any known serial drivers */
     return (0);

  /* Ignore result, not much we can do if it fails anyway.
   */
  if (_pkt_rxmode < RXMODE_MULTICAST2) /* if not already set */
     return pkt_set_rcv_mode (RXMODE_MULTICAST2);
  return (1);     /* promiscous mode is okay */
}

/*
 * Called from sock_ini.c after config read
 */
void _ip6_post_init (void)
{
  if (icmp6_6to4_gateway)
     icmp6_add_gateway4();
}
#endif  /* USE_IPV6 */
/* \endif */
