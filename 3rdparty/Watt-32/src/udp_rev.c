/*!\file udp_rev.c
 * Reverse DNS lookup
 */

/*
 *  Reverse IPv4/IPv6 lookup functions
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
 *  \version 0.2
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wattcp.h"
#include "misc.h"
#include "misc_str.h"
#include "timer.h"
#include "language.h"
#include "netaddr.h"
#include "pctcp.h"
#include "pcdbug.h"
#include "pcconfig.h"
#include "idna.h"
#include "bsdname.h"
#include "pcdns.h"

static BOOL  read_response (sock_type *s, char *name, size_t size);
static BYTE *dns_labels (BYTE *bp, BYTE *p, BYTE *ep, char *result);
static BYTE *dns_resource (WORD req_qtype, BYTE *bp, BYTE *p,
                           BYTE *ep, char *result);

/**
 * Fill in the reverse lookup question packet.
 * Return total length of request.
 */
static size_t query_init_ip4 (struct DNS_query *q, DWORD ip)
{
  char *c;
  BYTE  i, x;

  q->head.ident   = set_timeout (0UL) & 0xFFFF;  /* Random ID */
  q->head.flags   = intel16 (DRD);               /* Query, Recursion desired */
  q->head.qdcount = intel16 (1);
  q->head.ancount = 0;
  q->head.nscount = 0;
  q->head.arcount = 0;

  c  = (char*) &q->body[0];
  ip = ntohl (ip);
  for (i = 0; i < sizeof(ip); i++)
  {
    x = ip & 255;
    ip >>= 8;
    *c = (x < 10) ? 1 : (x < 100) ? 2 : 3;
    itoa (x, c+1, 10);
    c += *c + 1;
  }
  strcpy (c, "\7in-addr\4arpa");
  c += 14;
  *(WORD*) c = intel16 (DTYPE_PTR);
  c += sizeof(WORD);
  *(WORD*) c = intel16 (DIN);
  c += sizeof(WORD);

  return (c - (char*)q);
}


/**
 * Translate an IPv4/IPv6 address into a host name.
 * Returns 1 on success, 0 on error or timeout.
 */
static BOOL reverse_lookup (const struct DNS_query *q, size_t qlen,
                            char *name, size_t size, DWORD nameserver)
{
  BOOL        ret;
  BOOL        ready = FALSE;
  BOOL        quit  = FALSE;
  WORD        sec;
  DWORD       timer;
  _udp_Socket dom_sock;
  sock_type  *sock = NULL;

  if (!nameserver)      /* no nameserver, give up */
  {
    dom_errno = DNS_CLI_NOSERV;
    outsnl (dom_strerror(dom_errno));
    return (FALSE);
  }

  if (!udp_open(&dom_sock, DOM_SRC_PORT, nameserver, DOM_DST_PORT, NULL))
  {
    dom_errno = DNS_CLI_SYSTEM;
    return (FALSE);
  }

  timer = set_timeout (1000 * dns_timeout);  /* overall expiry (for all servers) */

  for (sec = 2; sec < dns_timeout-1 && !quit && !_resolve_exit; sec *= 2)
  {
    sock = (sock_type*)&dom_sock;
    sock_write (sock, (const BYTE*)q, qlen);
    ip_timer_init (sock, sec);               /* per server expiry */

    do
    {
      tcp_tick (sock);

      if (_watt_cbroke || (_resolve_hook && (*_resolve_hook)() == 0))
      {
        _resolve_exit = 1;       /* terminate do_reverse_resolve() */
        dom_errno = DNS_CLI_USERQUIT;
        quit  = TRUE;
        ready = FALSE;
        break;
      }
      if (sock_dataready(sock))
      {
        quit  = TRUE;
        ready = TRUE;
      }
      else if (ip_timer_expired(sock) || chk_timeout(timer))
      {
        dom_errno = DNS_CLI_TIMEOUT;
        ready = FALSE;
        _resolve_timeout = 1;
        break;  /* retry */
      }
      if (sock->udp.usr_yield)
           (*sock->udp.usr_yield)();
      else WATT_YIELD();
    }
    while (!quit);
  }

  if (ready && sock)
       ret = read_response (sock, name, size);
  else ret = FALSE;

  if (sock)  /* if we ran the above for-loop */
     sock_close (sock);
  return (ret);
}

/**
 * Do a reverse lookup on `my_ip_addr'. If successfull, replace
 * `hostname' and `def_domain' with returned result.
 */
int reverse_lookup_myip (void)
{
  char name [MAX_HOSTLEN];

  if (!reverse_resolve_ip4(htonl(my_ip_addr),name,sizeof(name)))
     return (0);

  if (debug_on >= 1)
  {
    outs (_LANG("My FQDN: "));
    outsnl (name);
  }
  if (sethostname(name,sizeof(name)) < 0)
     return (0);
  return (1);
}

/*------------------------------------------------------------------*/

static int do_reverse_resolve (const struct DNS_query *q, size_t qlen,
                               char *result, size_t size)
{
  WORD brk_mode;
  int  i;

  if (dns_timeout == 0)
      dns_timeout = (UINT)sock_delay << 2;

  NEW_BREAK_MODE (brk_mode, 1);

  _resolve_exit = _resolve_timeout = 0;

  *result = '\0';
  dom_cname[0] = '\0';

  for (i = 0; i < last_nameserver; ++i)
      if (reverse_lookup(q,qlen,result,size,def_nameservers[i]) ||
          _resolve_exit)
         break;

  OLD_BREAK_MODE (brk_mode);
  return (*result != '\0');
}

/*------------------------------------------------------------------*/

int reverse_resolve_ip4 (DWORD ip, char *result, size_t size)
{
  struct DNS_query q;
  size_t len = query_init_ip4 (&q, ip);

  memset (&dom_a4list, 0, sizeof(dom_a4list));

#if defined(WIN32)
  if (WinDnsQuery_PTR4(ip, result, size))
     return (1);
#endif
  return do_reverse_resolve (&q, len, result, size);
}

#if defined(USE_IPV6)

#define USE_IP6_BITSTRING 0     /* RFC-2673 */

/**
 * Fill in the reverse lookup question packet.
 * Return total length of request.
 * \todo Use "ip6.arpa" bitstring format ?
 */
static size_t query_init_ip6 (struct DNS_query *q, const void *addr)
{
  const BYTE *ip = (const BYTE*) addr;
  char *c;
  int   i;

  q->head.ident   = set_timeout (0UL) & 0xFFFF;  /* Random ID */
  q->head.flags   = intel16 (DRD);               /* Query, Recursion desired */
  q->head.qdcount = intel16 (1);                 /* 1 query */
  q->head.ancount = 0;
  q->head.nscount = 0;
  q->head.arcount = 0;

  c = (char*) &q->body[0];

#if USE_IP6_BITSTRING
  strcpy (c, "\3\\[x");
  c += 4;
  for (i = 0; i < SIZEOF(ip6_address); i++)
  {
    int hi = ip[i] >> 4;
    int lo = ip[i] & 15;

    if (i == SIZEOF(ip6_address) - 1)
         *c++ = 5;
    else *c++ = 4;
    *c++ = hex_chars_lower [hi];
    *c++ = hex_chars_lower [lo];
  }
  strcpy (c, "]\3ip6\4arpa");
  c += 11;

#else
  for (i = sizeof(ip6_address)-1; i >= 0; i--)
  {
    int hi = ip[i] >> 4;
    int lo = ip[i] & 15;

    *c++ = 2;
    *c++ = hex_chars_lower [hi];
    *c++ = hex_chars_lower [lo];
  }
  strcpy (c, "\3ip6\4arpa");
  c += 10;
#endif

  *(WORD*) c = intel16 (DTYPE_PTR);
  c += sizeof(WORD);
  *(WORD*) c = intel16 (DIN);
  c += sizeof(WORD);

  return (c - (char*)q);
}

int reverse_resolve_ip6 (const void *addr, char *result, size_t size)
{
  struct DNS_query q;
  size_t len = query_init_ip6 (&q, addr);

  memset (&dom_a6list, 0, sizeof(dom_a6list));

#if defined(WIN32)
  if (WinDnsQuery_PTR6(addr, result, size))
     return (1);
#endif
  return do_reverse_resolve (&q, len, result, size);
}
#endif   /* USE_IPV6 */


static BOOL read_response (sock_type *s, char *name, size_t size)
{
  struct {
     struct DNS_Header head;
     BYTE   body [DOMSIZE];
   } response;

  struct DNS_Header *dns = &response.head;
  int    len, num_q, num_ans;
  BYTE  *bp, *body, *end_p;
  char   result [DOMSIZE];

  #define CHECK_SHORT(p) if ((p) > len+(BYTE*)&response) goto short_resp

  memset (&response, 0, sizeof(response));
  len   = sock_fastread (s, (BYTE*)&response, sizeof(response));
  bp    = (BYTE*) &response;
  body  = (BYTE*) &response.body;
  end_p = (BYTE*) &response + len;

  CHECK_SHORT (body);

  num_q   = intel16 (dns->dns_num_q);
  num_ans = intel16 (dns->dns_num_ans);
  if (dns->dns_fl_rcode != DNS_SRV_OK || num_ans == 0)
     goto no_name;

  /* Go past the question part of the packet.
   */
  while (num_q > 0)
  {
    body = dns_labels (bp, body, end_p, result);
    CHECK_SHORT (body);
    body += 4;        /* skip Qtype and Qclass */
    num_q--;
  }

  /* Parse the resource records for the answers
   */
  while (num_ans > 0)
  {
    memset (&result, '\0', sizeof(result));
    body = dns_resource (DTYPE_PTR, bp, body, end_p, result);
    CHECK_SHORT (body);
    num_ans--;
    if (result[0])
    {
#if defined(USE_IDNA)
      if (dns_do_idna && !IDNA_convert_from_ACE(result,&size))
      {
        dom_errno = DNS_CLI_ILL_IDNA;
        return (FALSE);
      }
#endif
      dom_errno = DNS_SRV_OK;
      _strlcpy (name, dom_remove_dot(result), size);
      return (TRUE);
    }
  }

no_name:
  dom_errno = dns->dns_fl_rcode;
  return (FALSE);

short_resp:
  dom_errno = DNS_CLI_ILL_RESP;
  return (FALSE);
}

/*
 * Recursively parse a label entry in a DNS packet
 * 'bp' points to a beginning of DNS header.
 * 'p'  points to a DNS label.
 */
static BYTE *dns_labels (BYTE *bp, BYTE *p, BYTE *ep, char *result)
{
  while (1)
  {
    BYTE count = *p++;
    WORD offset;

    if (count >= 0xC0)
    {
      /* There's a pointer (rel to 'bp' start) in this label.
       * Let's grab the 14 low-order bits and run with them.
       */
      count -= 0xC0;
      offset = (WORD) (((unsigned)(count) << 8) + *p++);

      dns_labels (bp, bp+offset, ep, result);
      return (p);
    }
    if (count == 0)
       break;

    while (count > 0)
    {
      if (p <= ep)
      {
        *result++ = *p++;
        count--;
      }
      else
        return (p);   /* Packet length exceeded */
    }
    *result++ = '.';
    *result   = '\0';  /* 0-terminate incase this is the last label */
  }
  return (p);
}

/*
 * Return the given resource record.
 */
static BYTE *dns_resource (WORD req_qtype, BYTE *bp, BYTE *p, BYTE *ep, char *result)
{
  DWORD ttl;
  WORD  qtype, qclass, reslen;
  char  dummy [DOMSIZE];

  p = dns_labels (bp, p, ep, dummy);

  /* Do query type, class, ttl and resource length
   */
  qtype  = intel16 (*(WORD*)p);  p += sizeof(qtype);
  qclass = intel16 (*(WORD*)p);  p += sizeof(qclass);
  ttl    = intel  (*(DWORD*)p);  p += sizeof(ttl);
  reslen = intel16 (*(WORD*)p);  p += sizeof(reslen);

  /* Do resource data.
   */
  if (qclass == DIN && qtype == req_qtype)
  {
    p = dns_labels (bp, p, ep, result);
    dom_ttl = ttl;
  }
  else
    p += reslen;

  return (p);
}

