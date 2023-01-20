/*!\file ip4_out.c
 * IPv4 output routines.
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
 *  Version
 *
 *  1.0 : Apr 18, 1999 : G. Vanem - created
 *  1.1 : Jan 29, 2000 : G. Vanem - added functions for
 *                       handling list of IP-fragments.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "wattcp.h"
#include "misc.h"
#include "chksum.h"
#include "pcdbug.h"
#include "pcsed.h"
#include "pctcp.h"
#include "pcstat.h"
#include "pcicmp.h"
#include "socket.h"

BYTE _default_tos      = 0;      /* Type-Of-Service */
int  _default_ttl      = 254;    /* TTL on outgoing packets */
int  _ip4_id_increment = 1;      /* Some stacks uses 5 */
BOOL _ip4_dont_frag    = FALSE;  /* Don't set DF bit on every packet */

static WORD ip4_id = 0;

/**
 * Increment IPv4-identifier before returning it.
 *
 * Note: Use _get_this_ip4_id() for sending IP fragments.
 */
WORD _get_ip4_id (void)
{
  ip4_id += _ip4_id_increment;
  return intel16 (ip4_id);
}

/**
 * Return IPv4-identifier but don't increment it.
 */
WORD _get_this_ip4_id (void)
{
  return intel16 (ip4_id);
}

/**
 * The IP4 packet transmitter.
 *
 * \retval  'data_len + sizeof(*ip)' on success.
 * \retval  0 on fail.
 * \todo Handle IP-fragments here (call ip4_send_fragments() when needed)
 */
int _ip4_output (in_Header  *ip,        /* ip-structure to fill in */
                 DWORD       src_ip,    /* from address (network order!) */
                 DWORD       dst_ip,    /* dest address (network order!) */
                 BYTE        protocol,  /* IP-protocol number */
                 BYTE        ttl,       /* Time To Live */
                 BYTE        tos,       /* Type Of Service, 0 unspecified */
                 WORD        ip_id,     /* IP identification, normally 0 */
                 int         data_len,  /* length of data after ip header */
                 const void *sock,      /* which socket is this */
                 const char *file,      /* Debug: from what file */
                 unsigned    line)      /*  and line was _ip4_output called */
{
  int len = sizeof(*ip) + data_len;

 /*
  * Note: the 'ip->frag_ofs' field isn't normally set here (it's
  *       cleared in eth_formatpacket() causing IP_DF bit to off).
  *       If sending fragments it's set in _ip4_send_fragments() below.
  */

  if (src_ip == 0)
      src_ip = intel (my_ip_addr);

  if (_ip4_dont_frag)
     ip->frag_ofs |= intel16 (IP_DF);

  ip->ver            = 4;
  ip->hdrlen         = sizeof(*ip) / 4;
  ip->length         = intel16 (len);
  ip->tos            = tos & IP_TOSMASK;
  ip->ttl            = ttl   ? ttl   : _default_ttl;
  ip->identification = ip_id ? ip_id : _get_ip4_id();
  ip->proto          = protocol;
  ip->source         = src_ip;
  ip->destination    = dst_ip;
  ip->checksum       = 0;
  ip->checksum       = ~CHECKSUM (ip, sizeof(*ip));

  return _eth_send (len, sock, file, line);
}

#if defined(USE_FRAGMENTS)
/*
 * Note: host is on network order.
 * No IP options supported here.
 */
static __inline in_Header *make_ip_pkt (DWORD host, BOOL first, char **data)
{
  eth_address dest;
  in_Header  *ip;

  if (first && !_arp_resolve(intel(host),&dest))
  {
    if (debug_on)
       outsnl (_LANG("make_ip_pkt(): ARP failed"));
    SOCK_DEBUGF ((", EHOSTUNREACH"));
    SOCK_ERRNO (EHOSTUNREACH);
    return (NULL);
  }

  ip = (in_Header*) _eth_formatpacket (&dest, IP4_TYPE);
  *data = (char*) (ip+1);
  return (ip);
}

/*
 * \todo Handle IP/TCP options.
 */
static in_Header *make_tcp_pkt (const _tcp_Socket *s,
                                BOOL first, char **data)
{
  in_Header *ip = (in_Header*) _eth_formatpacket (&s->his_ethaddr, IP4_TYPE);

  if (first)
  {
    tcp_Header      *tcp = (tcp_Header*) (ip+1);
    tcp_PseudoHeader ph;
    WORD tcp_len = sizeof(*tcp);

    tcp->srcPort  = intel16 (s->myport);
    tcp->dstPort  = intel16 (s->hisport);
    tcp->seqnum   = intel (s->send_next + s->send_una);
    tcp->acknum   = intel (s->recv_next);
    tcp->window   = intel16 (s->max_rx_data - s->rx_datalen);
    tcp->flags    = (BYTE) s->flags;
    tcp->unused   = 0;
    tcp->checksum = 0;
    tcp->urgent   = 0;
    tcp->offset   = tcp_len / 4;

    memset (&ph, 0, sizeof(ph));
    ph.src      = intel (s->myaddr);
    ph.dst      = intel (s->hisaddr);
    ph.protocol = TCP_PROTO;
    ph.length   = intel16 (tcp_len);
    ph.checksum = CHECKSUM (tcp, tcp_len);
    tcp->checksum = ~CHECKSUM (&ph, sizeof(ph));
    *data = (char*)tcp + sizeof(*tcp);
  }
  else
    *data = (char*)(ip+1);

  return (ip);
}

/*
 * Note: We assume 's->his_ethaddr' is filled by udp_open() or udp_demux().
 *       I.e. socket is active and not a broadcast socket.
 */
static __inline in_Header *make_udp_pkt (const _udp_Socket *s, BOOL first,
                                         unsigned len, char **data)
{
  in_Header *ip = (in_Header*) _eth_formatpacket (&s->his_ethaddr, IP4_TYPE);

  if (first)
  {
    udp_Header      *udp = (udp_Header*) (ip+1);
    tcp_PseudoHeader ph;

    udp->srcPort  = intel16 (s->myport);
    udp->dstPort  = intel16 (s->hisport);
    udp->checksum = 0;
    udp->length   = intel16 ((WORD)(sizeof(*udp) + len));
    memset (&ph, 0, sizeof(ph));
    ph.src = intel (s->myaddr);
    ph.dst = intel (s->hisaddr);

    if (s->sockmode & SOCK_MODE_UDPCHK)
    {
      ph.protocol = UDP_PROTO;
      ph.length   = udp->length;
      ph.checksum = CHECKSUM (udp, sizeof(*udp)) + CHECKSUM (*data, len);
      udp->checksum = ~CHECKSUM (&ph, sizeof(ph));
    }
    *data = (char*)udp + sizeof(*udp);
  }
  else
    *data = (char*)(ip+1);

  return (ip);
}


/*
 * Sends a chain of IPv4 fragments for proto. `buf' is higher-level
 * protocol (ICMP/UDP/TCP).
 * Note: `dest' is on network order. This function is only called
 *       when we're sure fragment are needed; num_frags >= 1.
 */
int _ip4_send_fragments (sock_type *s, BYTE proto, DWORD dest,
                         const void *buf, unsigned len,
                         const char *file, unsigned line)
{
  int  i, num_frags, rc = 0;
  int  frag_size, frag_ofs;
  WORD ip_id;
  WORD ip_max = _mtu - sizeof(in_Header);

  num_frags = len / ip_max;

  WATT_ASSERT (num_frags >= 1);

  if (len % ip_max)
     num_frags++;

  /* Increment IP-identifier. Use same value for all fragments
   */
  ip_id = _get_ip4_id();

  /** \todo Maybe send highest offset fragment first?
   */
  for (frag_ofs = i = 0; i < num_frags; i++)
  {
    in_Header *ip   = NULL;
    char      *data = (char*) buf;

    switch (proto)
    {
#if 0 /** \todo support sending raw IPv4 fragments */
      case IP4_TYPE:       /* raw IP type (from ip4_transmit) */
           proto = ((in_Header*)buf)->proto;
           /* fall through */
#endif
      case ICMP_PROTO:
           ip = make_ip_pkt (dest, frag_ofs == 0, &data);
           if (!ip)
              return (-1);
           break;

      case UDP_PROTO:
           ip = make_udp_pkt (&s->udp, frag_ofs == 0, len, &data);
           break;

      case TCP_PROTO:
           if (s->tcp.state != tcp_StateESTAB)
              return (0);
           ip = make_tcp_pkt (&s->tcp, frag_ofs == 0, &data);
           break;

      default:
           TRACE_CONSOLE (0, "%s (%d): Illegal protocol %04X\n",
                          __FILE__, __LINE__, proto);
           return (0);
    }

    if (i == num_frags-1)    /* last fragment */
    {
      frag_size    = len;
      ip->frag_ofs = intel16 (frag_ofs/8);
   /* memset (data+frag_size-8, 0, len); */  /* padding */
    }
    else
    {
      frag_size    = ip_max;
      ip->frag_ofs = intel16 (frag_ofs/8 | IP_MF);
    }

    memcpy (data, (const char*)buf + frag_ofs, frag_size);

    if (_ip4_output (ip, 0, dest, proto, 0, 0, ip_id, frag_size,
                     s, file, line) <= 0)
    {
      SOCK_DEBUGF ((", ENETDOWN"));
      SOCK_ERRNO (ENETDOWN);
      rc = -1;
      break;
    }
    STAT (ip4stats.ips_ofragments++);

    frag_ofs += frag_size;
    len      -= frag_size;
    rc       += frag_size;
  }

  tcp_tick (NULL);  /* !! for viewing loopback trace */
  return (rc);
}
#endif /* USE_FRAGMENTS */

