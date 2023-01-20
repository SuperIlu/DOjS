/*!\file pcdns.c
 *
 *  Domain Name Server protocol.
 *
 *  This portion of the code needs some major work. I ported it (read STOLE IT)
 *  from NCSA and lost about half the code somewhere in the process.
 *
 *   \version 0.6
 *     \date Mar 19, 2004 - Added ACE encoding/decoding of international
 *                          domain names. Ref. RFC-3490.
 *   \version 0.5
 *     \date Aug 20, 2003 - Added storing of CNAME record (used as alias in
 *                          gethostbyname, gethostbyaddr)
 *
 *   \version 0.4
 *     \date Aug 04, 2002 - Rewritten for IPv6 support (DTYPE_AAAA)
 *
 *   \version 0.3
 *     \date Jun 16, 1997 - calling usr_yield (i.e. system_yield) while resolving
 *
 *   \version 0.2
 *     \date Apr 24, 1991 - use substring portions of domain
 *
 *   \version 0.1
 *     \date Mar 18, 1991 - improved the trailing domain list
 *
 *   \version 0.0
 *     \date Feb 19, 1991 - pirated by Erick Engelke
 *
 *   \version -1.0        - NCSA code
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/werrno.h>

#include "copyrigh.h"
#include "wattcp.h"
#include "misc.h"
#include "misc_str.h"
#include "timer.h"
#include "language.h"
#include "pcconfig.h"
#include "pcdbug.h"
#include "pctcp.h"
#include "netaddr.h"
#include "idna.h"
#include "ip6_out.h"
#include "netaddr.h"
#include "pcdns.h"

/**
 * Hacks to make gethostbyname() and resolve() cooperate.
 */
#if defined(USE_BSD_API)
  #include <netdb.h>

  BOOL called_from_resolve = FALSE;
  BOOL called_from_ghbn    = FALSE;
#endif

#if !defined(WIN32)
  #define TCHAR
#endif

/**
 * The 2 next variables are loaded from WATTCP.CFG file
 */
char  defaultdomain [MAX_HOSTLEN+1] = "your.domain.com";
char *def_domain = defaultdomain;

int (W32_CALL *_resolve_hook)(void);  /* user hook for terminating resolve() */

BOOL  _resolve_exit;                  /* user hook interrupted */
BOOL  _resolve_timeout;               /* (reverse) lookup timeout */
BOOL  from_windns;                    /* the answer came from WinDNS */
DWORD dom_ttl;                        /* Time-to-Live of last response */
int   dom_errno;                      /* domain errno (DNS_SRV_xx/DNS_CLI_xx) */
char  dom_cname [MAX_HOSTLEN+1];      /* CNAME of last response */

DWORD def_nameservers [MAX_NAMESERVERS];
WORD  last_nameserver = 0;
WORD  dns_timeout     = 0;
BOOL  dns_recurse     = TRUE;    /* Recurse over dotless names */
BOOL  dns_do_ipv6     = TRUE;    /* Do try to resolve to IPv6 */
BOOL  dns_do_idna     = FALSE;   /* Convert non-ASCII names to/from ACE */
WORD  dns_windns      = 0;       /* How to read/write the WinDns cache */

static const char *loc_domain;   /* current subname to be used */
static DWORD       res_timeout;

/* Additional addresses from last resolve().
 * dom_a4_list is on network order!
 */
DWORD dom_a4list [MAX_ADDRESSES+1];

#if defined(USE_IPV6)
ip6_address dom_a6list [MAX_ADDRESSES+1];
#endif


/**
 * Initialise the query header and clear the alternate addr-list.
 */
static void query_init (struct DNS_query *q, BOOL is_ip6)
{
  int i;

  memset (&q->head, 0, sizeof(q->head));
  q->head.flags   = intel16 (DRD);  /* Query, Recursion desired */
  q->head.qdcount = intel16 (1);

#if defined(USE_IPV6)
  if (is_ip6)
     memset (&dom_a6list, 0, sizeof(dom_a6list));
  else
#endif
    for (i = 0; i < DIM(dom_a4list); i++)
        dom_a4list[i] = INADDR_NONE;
  ARGSUSED (is_ip6);
}

/**
 * Remember alternate address in dom_a?list[].
 */
static void store_alt_address (const void *data, int idx, BOOL is_ip6)
{
  if (idx < 0 || idx >= MAX_ADDRESSES)
     return;

#if defined(USE_IPV6)
  if (is_ip6)
     memcpy (&dom_a6list[idx], data, sizeof(ip6_address));
  else
#endif
     dom_a4list [idx] = *(DWORD*)data;   /* store on network order */
  ARGSUSED (is_ip6);
}

/**
 * Pack a regular text string into a packed domain name, suitable
 * for the name server.
 *
 * Returns length or -1 if a labellength is > 63.
 */
static int pack_domain (BYTE *dst, const char *src)
{
  const char *p;
  BYTE *q, *savedst;
  BOOL  dotflag, defflag;
  int   i;

  dotflag = defflag = FALSE;
  savedst = dst;
  p = src;

  do                          /* copy whole string */
  {
    *dst = '\0';
    q = dst + 1;
    while (*p && *p != '.')
       *q++ = *p++;

    i = p - src;
    if (i > MAX_LABEL_SIZE)    /* label length exceeded */
       return (-1);

    *dst = i;
    *q   = '\0';

    if (*p)                   /* update pointers */
    {
      dotflag = TRUE;
      src = ++p;
      dst = q;
    }
    else if (!dotflag && !defflag && loc_domain)
    {
      p = loc_domain;         /* continue packing with default */
      defflag = TRUE;
      src = p;
      dst = q;
    }
  }
  while (*p);

  q++;
  return (q - savedst);       /* length of packed string */
}

/**
 * Put together a domain lookup packet and send it.
 * Uses port DOM_DST_PORT (53).
 */
static int send_query (sock_type *sock, struct DNS_query *q,
                       const char *name, DWORD towho, WORD qtype)
{
  WORD  len;
  BYTE *p;
  int   i;

  if (!udp_open(&sock->udp,DOM_SRC_PORT,towho,DOM_DST_PORT,NULL))
  {
    dom_errno = DNS_CLI_SYSTEM;  /* errno ENETDOWN/EHOSTUNREACH etc. */
    if (debug_on)
    {
      outs (_LANG("Domain: "));
      outsnl (sock->udp.err_msg);
    }
    return (0);
  }

  p = &q->body[0];
  i = pack_domain (p, name);
  if (i < 0)
  {
    dom_errno = DNS_CLI_TOOBIG;
    return (0);
  }

  p += i;
  *(WORD*)p = intel16 (qtype);  /* QTYPE is A or AAAA */
  p += sizeof(WORD);
  *(WORD*)p = intel16 (DIN);    /* QCLASS is always internet */
  p += sizeof(WORD);

  len = p - (BYTE*)q;
  if (sock_write (sock, (const BYTE*)q, len) < len)
  {
    dom_errno = DNS_CLI_SYSTEM;
    return (0);
  }
  return (len);
}

static __inline int count_paths (const char *path_str)
{
  int   count = 0;
  const char *p;

  for (p = path_str; *p || *(p+1); p++)
      if (*p == '\0')
         count++;
  return (++count);
}

static __inline const char *get_path (
                const char *path_str,    /* the path list to search */
                int         which_one)   /* which path to get, starts at 1 */
{
  const char *p;

  if (which_one > count_paths(path_str))
     return (NULL);

  which_one--;
  for (p = path_str; which_one; p++)
      if (*p == '\0')
         which_one--;
  return (p);
}

/**
 * Extract the CNAME from a resource record.
 */
static void extract_cname (const struct DNS_query *q, const char *src,
                           char *dest, size_t max)
{
  const char *p     = src;
  const char *start = dest;
  const char *end   = dest + max;

  while (dest < end)
  {
    BYTE count = *p++;
    WORD ofs;

    if ((count & 0xC0) == 0xC0)
    {
      ofs = ((count - 0xC0) << 8) + *p++;  /* new offset */
      p   = (const char*)q + ofs;
      count = *p++;
    }
    if (count == 0)
       break;

    while (count > 0)
    {
      if (dest > end)
         break;
      *dest++ = *p++;
      count--;
    }
    *dest++ = '.';
  }
  if (dest > start && *(dest-1) == '.')   /* remove trailing '.' */
     dest--;
  *dest = '\0';
}


/**
 * Unpack a received compressed domain name. Handles pointers to
 * continuation domain names.
 *
 * - address of query response ('q') is used as the base for the offset
 *   of any pointer which is present.
 * - returns the number of bytes at 'src' which should be skipped over.
 *   Includes the 0-terminator in its length count.
 * - returns -1 if the response is munged.
 */
static int unpack_domain (const struct DNS_query *q,
                          const BYTE *src, size_t src_size,
                          BYTE       *dst, size_t dst_size)
{
  int   retval = 0;
  const BYTE *start   = src;
  const BYTE *src_max = src + src_size;
  const BYTE *dst_max = dst + dst_size;
  const BYTE *base    = (const BYTE*)q;

  while (*src && src < src_max && dst < dst_max)
  {
    int i, j = *src;

    while ((j & 0xC0) == 0xC0)
    {
      if (!retval)
         retval = src - start + 2;
      src++;
      src = &base [((j - 0xC0) << 8) + *src];
      j = *src;
    }

    src++;
    for (i = 0; i < (j & ~0xC0); i++)
    {
      if (dst >= dst_max-1)
         return (-1);
      *dst++ = *src++;
    }
    *dst++ = '.';
  }

  if (src > start)
  {
    *(--dst) = '\0';     /* add terminator */
    src++;               /* account for terminator on src */
  }

  if (!retval)
     retval = src - start;
  return (retval);
}


/**
 * Extract the IPv4/v6 address from a response message (A or AAAA record).
 * Returns 0 on fail and sets appropriate status code in `dom_errno'.
 * Returns 1 if okay and if the ip is available and copied to 'addr'.
 * Read upto 'len' bytes from reply.
 *
 * Note: Our query is echoed in the answer, but 'q' below is not same as
 *       we sent in send_query().
 */
static int read_response (const struct DNS_query *q, int len,
                          WORD qtype, void *addr)
{
  WORD  nans  = intel16 (q->head.ancount);  /* # of answers */
  WORD  flags = intel16 (q->head.flags);
  BYTE  rcode = (DRCODE & flags);
  BYTE  space [sizeof(q->body)];
  const BYTE *p;
  int   i, num_addr;

  if (rcode != DNS_SRV_OK)  /* not okay reply */
  {
    dom_errno = rcode;
    return (0);
  }

  if (nans == 0 ||         /* not at least one answer */
      !(flags & DQR))      /* response flag not set */
  {
    dom_errno = DNS_SRV_NAME;
    return (0);
  }

  len -= sizeof(q->head);
  p = &q->body[0];
  i = unpack_domain (q, p, len, space, sizeof(space));
  if (i < 0)
  {
    dom_errno = DNS_CLI_ILL_RESP;
    return (0);
  }

  /* spec defines name then QTYPE + QCLASS = 4 bytes
   */
  p += i + 4;

  /* no canonical name yet
   */
  dom_cname[0] = '\0';

  /* There may be several answers.  We will return the first A or
   * AAAA-record. If there's a CNAME-record, store the name too.
   */
  num_addr = 0;
  while (nans-- > 0 && len > 0)            /* look at each answer */
  {
    const struct DNS_resource *rrp;
    size_t rr_len;
    WORD   rr_type;

    i = unpack_domain (q, p, len, space, sizeof(space));
    if (i < 0)
       break;

    p  += i;                               /* account for string */
    rrp = (struct DNS_resource*) p;        /* resource record here */
    rr_len = intel16 (rrp->rdlength);      /* length of this record */
    rr_type = intel16 (rrp->rtype);

    if (intel16(rrp->rclass) == DIN)       /* Internet (ARPA) class */
    {
      if (rr_type == qtype)                /* Record type A or AAAA */
      {
        BOOL is_ip6 = (qtype == DTYPE_AAAA);

        if (num_addr == 0)   /* prefered address should always be 1st */
        {
          /* save IPv4/IPv6 address */
          if (is_ip6)
               memcpy (addr, &rrp->rdata[0], sizeof(ip6_address));
          else *(DWORD*)addr = intel (*(DWORD*)&rrp->rdata[0]);

          dom_ttl   = intel (rrp->ttl);    /* TTL in seconds */
          dom_errno = DNS_SRV_OK;
        }
        else
          store_alt_address (&rrp->rdata[0], num_addr-1, is_ip6);
        num_addr++;
      }
      else if (rr_type == DTYPE_CNAME && !dom_cname[0])
      {
        size_t max = sizeof(dom_cname) - 1;
        extract_cname (q, (const char*)rrp->rdata, dom_cname, max);
      }
    }
    p   += RESOURCE_HEAD_SIZE + rr_len;    /* length of rest of RR */
    len -= RESOURCE_HEAD_SIZE + rr_len;
  }

  if (num_addr > 0)
     return (1);

  /* Could be a reponse with a CNAME record but no A record, so
   * turn it into a "Hostname not found" error.
   */
  dom_errno = DNS_SRV_NAME;
  return (0);
}

/*
 * Check if the ID in the reply is among our ID(s) sent.
 * If not, ignore the response and keep waiting.
 */
static __inline BOOL legal_ident (WORD id, const WORD *cache, int num)
{
  int i;

  for (i = 0; i < num; i++)
      if (id == cache[i])
         return (TRUE);
  return (FALSE);
}

/**
 * DOMAIN based name lookup.
 * Query a domain name server to get an IP number
 * Returns the machine number of the machine record for future reference.
 * Events generated will have this number tagged with them.
 * Returns various negative numbers on error conditions.
 *
 * if add_dom is nonzero, add default domain.
 */
static int lookup_domain (
       struct DNS_query *q,           /* the UDP/IPv4 query packet */
       const char       *mname,         /* the host-name to query */
       BOOL              add_dom,       /* append domain suffix */
       DWORD             nameserver,    /* nameserver to use */
       BOOL             *timedout,      /* set on timeout */
       BOOL              is_aaaa,       /* Query A or AAAA record */
       void             *addr)          /* return address */
{
  _udp_Socket sock;
  sock_type  *dom_sock = NULL;

  struct DNS_query reply;
  char   namebuf [3*MAX_HOSTLEN]; /* may overflow!! */
  size_t len;
  int    rc;
  WORD   qtype = is_aaaa ? DTYPE_AAAA : DTYPE_A;
  WORD   id_cache [100];        /* should be plenty */
  int    id_index = 0;
  WORD   sec;

  *timedout = TRUE;  /* assume we will timeout */

  if (!nameserver)   /* no nameserver, give up now */
  {
    _resolve_exit = TRUE;
    dom_errno = DNS_CLI_NOSERV;
    outsnl (dom_strerror(dom_errno));
    return (0);
  }

  while (*mname && *mname <= ' ')   /* kill leading cruft */
     mname++;

  if (*mname == '\0')
  {
    dom_errno = DNS_CLI_OTHER;
    return (0);
  }

  strcpy (namebuf, mname);

#if defined(USE_IDNA)
  /*
   * Convert 'namebuf' to ACE form if needed
   */
  len = sizeof (namebuf);
  if (dns_do_idna && !IDNA_convert_to_ACE(namebuf,&len))
  {
    dom_errno = DNS_CLI_ILL_IDNA;
    return (0);
  }
#endif

  if (add_dom)
  {
    int dot = strlen (namebuf) - 1;

    if (namebuf[dot] != '.')     /* if no trailing dot */
    {
      if (loc_domain)            /* there is a search list */
      {
        strcat (namebuf, ".");
        strcat (namebuf, get_path(loc_domain,1));
      }
    }
    else
      namebuf [dot] = '\0';      /* kill trailing dot */
  }

  /*
   * This is not terribly good, but it attempts to use a binary
   * exponentially increasing delay.
   */

  for (sec = 2; sec < dns_timeout-1 && !_resolve_exit; sec *= 2)
  {
    q->head.ident = Random (1, USHRT_MAX);

    /* We might get a reply for a previous request with different ID.
     * We therefore use a small cache of IDs sent.
     */
    if (id_index < DIM(id_cache))
         id_cache [id_index++] = q->head.ident;
    else outsnl (_LANG("udp_dom(): ID cache full"));

    dom_sock = (sock_type*) &sock;

    if (!send_query(dom_sock,q,namebuf,nameserver,qtype))
    {
      _resolve_timeout = TRUE;
      rc = 0;
      goto quit; /* doesn't hurt to do sock_close() if udp_open() fails */
    }

again:
    ip_timer_init (dom_sock, sec);
    do
    {
      if (!tcp_tick(dom_sock))   /* ICMP Port unreachable etc. */
      {
        rc = 0;
        dom_errno = DNS_CLI_REFUSE;
        goto quit;
      }

      if (ip_timer_expired(dom_sock) || chk_timeout(res_timeout))
      {
        /* continue the for-loop with a new request */
        break;
      }

      if (_watt_cbroke || (_resolve_hook && (*_resolve_hook)() == 0))
      {
        dom_errno = DNS_CLI_USERQUIT;
        _resolve_exit = TRUE;
        rc = 0;
        goto quit;
      }

      if (dom_sock->udp.usr_yield)
          (*dom_sock->udp.usr_yield)();
      else WATT_YIELD();

      if (sock_dataready(dom_sock))
         *timedout = FALSE;
    }
    while (*timedout);

    if (!*timedout)           /* got an answer */
       break;
  }

  if (*timedout || !dom_sock)
  {
    _resolve_timeout = TRUE;
    rc = 0;
    dom_errno = DNS_CLI_TIMEOUT;
  }
  else
  {
    memset (&reply, 0, sizeof(reply));
    len = sock_fastread (dom_sock, (BYTE*)&reply, sizeof(reply));
    if (len < SIZEOF(reply.head))
    {
      dom_errno = DNS_CLI_ILL_RESP;
      rc = 0;
    }
    else if (!legal_ident(reply.head.ident,id_cache,id_index))
    {
      *timedout = TRUE;
      goto again;
    }
    else
    {
      rc = read_response (&reply, len, qtype, addr);
#if defined(USE_IDNA)
      if (dns_do_idna && dom_cname[0])
      {
        len = sizeof(dom_cname);
        IDNA_convert_from_ACE (dom_cname, &len);
        dom_remove_dot (dom_cname);
      }
#endif
    }
  }

quit:
  if (dom_sock)  /* if we ran the above for-loop */
     sock_close (dom_sock);
  return (rc);
}

/**
 * Given domain and count = 0,1,2,..., return next larger
 * domain or NULL when no more are available.
 */
static const char *next_domain (const char *domain, int count)
{
  const char *p = domain;
  int   i;

  for (i = 0; i < count; i++)
  {
    p = strchr (p, '.');
    if (!p)
       return (NULL);
    p++;
  }
  return (p);
}

/**
 * Return text for server-side errors.
 */
static const char *server_error (enum DNS_serv_resp err)
{
  switch (err)
  {
    case DNS_SRV_OK:
         return ("Success");
    case DNS_SRV_FORM:
         return ("Format error");
    case DNS_SRV_FAIL:
         return ("Error in response");
    case DNS_SRV_NAME:
         return ("Hostname not found");
    case DNS_SRV_NOTIMPL:
         return ("Not implemented");
    case DNS_SRV_REFUSE:
         return ("Service refused");
    case DNS_SRV_MAX:
         break;
  }
  return (NULL);
}

/**
 * Return text for client-side errors.
 */
static const char *client_error (enum DNS_client_code err)
{
  switch (err)
  {
    case DNS_CLI_SYSTEM:
         return (char*) strerror_s_ (_w32_errno);
    case DNS_CLI_USERQUIT:
         return ("User terminated");
    case DNS_CLI_NOSERV:
         return ("No nameserver defined");
    case DNS_CLI_TIMEOUT:
         return ("Request timeout");
    case DNS_CLI_ILL_RESP:
         return ("Illegal short response");
    case DNS_CLI_ILL_IDNA:
         return ("IDNA convertion failed");
    case DNS_CLI_TOOBIG:
         return ("Name too large");
    case DNS_CLI_REFUSE:
         return ("Service refused");
    case DNS_CLI_NOIP:
         return ("Cannot resolve without IP");
    case DNS_CLI_NOIPV6:
         return ("IPv6 DNS disabled");
    case DNS_CLI_OTHER:
         return ("Other error");
    case DNS_CLI_MAX:
         break;
  }
  return (NULL);
}


/**
 * Return text for error code (dom_errno).
 */
const char *W32_CALL dom_strerror (int err)
{
  static char buf[80];
  const char *rc = err < DNS_SRV_MAX ? server_error (err) :
                   err < DNS_CLI_MAX ? client_error (err) : NULL;
  if (rc)
     return (rc);
  strcpy (buf, "Unknown error ");
  itoa (err, buf+strlen(buf), 10);
  return (buf);
}

/*
 *
 */
char *W32_CALL dom_remove_dot (char *name)
{
  char *dot = strrchr (name, '.');

  if (dot && dot[1] == '\0' && dot > name+1)
     *dot = '\0';
  return (name);
}

/**
 * Convert host name to an address.
 * Returns 0 if name is unresolvable right now.
 * Return value is host-order.
 */
DWORD W32_CALL resolve (const char *name)
{
  DWORD       ip4_addr = 0UL;
  int         count;
  char        namebuf [MAX_HOSTLEN], *dot;
  BOOL        timeout [MAX_NAMESERVERS];
  BOOL        got_ip  = FALSE;
  BOOL        recurse = TRUE;
  unsigned    len;
  static WORD brk_mode;
  struct DNS_query request;

  if (!name || *name == '\0')
  {
    dom_errno = DNS_CLI_OTHER;
    return (0);
  }

  len = strlen (name);
  if (len >= sizeof(namebuf)-1)
  {
    dom_errno = DNS_CLI_TOOBIG;
    return (0);
  }

  memcpy (namebuf, name, len+1); /* make a copy of 'name' */
  rip (namebuf);                 /* strip off '\r' and/or '\n' */

  if (isaddr(namebuf))
  {
    dom_errno = 0;
    return aton (namebuf);
  }

  /* Names with trailing '.' shall not perform a DNS recursion
   */
  dot = strrchr (namebuf, '.');
  if (dot && dot[1] == '\0')
  {
    *dot = '\0';
    recurse = FALSE;
  }

#if defined(USE_BSD_API)
  if (!called_from_ghbn)         /* This hack avoids reentrancy */
  {                              /* from gethostbyname() */
    const struct hostent *h;

    called_from_resolve = TRUE;  /* ditto hack ! (vice versa) */
    h = gethostbyname (namebuf);
    called_from_resolve = FALSE;
    if (h)                       /* IP from host-file or cache */
    {
      /*
       * NOTE: gethostbyname() returns network order
       *       We assume IPv4 (32-bit)
       */
      dom_errno = 0;
      return intel (*(DWORD*)h->h_addr_list[0]);
    }

    /* not found in host-file or cache, ask the DNS server(s)
     */
  }
#endif

#if defined(WIN32)
  /*
   * Try the WinDns cache first
   */
  if (WinDnsQuery_A4(namebuf, &ip4_addr))
     return (ip4_addr);
#endif

  /* If no nameserver, give up now
   */
  if (last_nameserver == 0)
  {
    dom_errno = DNS_CLI_NOSERV;
    outsnl (dom_strerror(dom_errno));
    return (0);
  }

  /* We have no IP-address, give up now
   */
  if (my_ip_addr == 0)
  {
    dom_errno = DNS_CLI_NOIP;
    outsnl (dom_strerror(dom_errno));
    return (0);
  }

  if (dns_timeout == 0)
      dns_timeout = sock_delay << 2;
  res_timeout = set_timeout (1000 * dns_timeout);

  count = 0;
  memset (&timeout, 0, sizeof(timeout));
  query_init (&request, FALSE);

  NEW_BREAK_MODE (brk_mode, 1);

  _resolve_exit = _resolve_timeout = FALSE;

  do
  {
    int i;

    if (!recurse || strchr(namebuf,'.'))
    {
      loc_domain = NULL;
      count = -1;
    }
    else if (!dns_recurse && count == 0)
    {
      loc_domain = NULL;
      count = -1;
    }
    else
    {
      loc_domain = next_domain (def_domain, count);
      if (!loc_domain)
         count = -1;     /* use default name */
    }

    for (i = 0; i < last_nameserver; i++)
    {
      if (!timeout[i])
      {
        int rc = lookup_domain (&request, namebuf, count != -1,
                                def_nameservers[i], timeout+i,
                                FALSE, &ip4_addr);
        if (rc)
        {
          got_ip = TRUE;
          break;            /* got address, bail out of for loop */
        }
        if (_resolve_exit)
           break;           /* an error occured, return to caller */
      }

      /*
       * Should we really try the other nameservers if the first
       * says the host doesn't exist? Maybe a trusting mechanism
       * is needed (?)
       */
    }
    if (count == -1)
       break;
    count++;
  }
  while (!got_ip && !_resolve_exit);

#if defined(WIN32)
  /* Put the result in the WinDns cache.
   */
  if (got_ip)
     WinDnsCachePut_A4 (namebuf, ip4_addr);
#endif

  OLD_BREAK_MODE (brk_mode);

  return (ip4_addr);
}

#if defined(USE_IPV6)
int W32_CALL resolve_ip6 (const char *name, void *addr)
{
  struct DNS_query request;
  ip6_address      ip6_addr;

  int      count;
  unsigned len;
  char     namebuf [MAX_HOSTLEN];
  BOOL     timeout [MAX_NAMESERVERS];
  WORD     brk_mode;
  BOOL     got_ip = FALSE;

  if (!dns_do_ipv6)
  {
    dom_errno = DNS_CLI_NOIPV6;
    TRACE_CONSOLE (1, "%s\n", dom_strerror(dom_errno));
    return (0);
  }

  len = strlen (name);
  if (len >= sizeof(namebuf)-1)
  {
    dom_errno = DNS_CLI_TOOBIG;
    TRACE_CONSOLE (1, "%s\n", dom_strerror(dom_errno));
    return (0);
  }

  memcpy (namebuf, name, len+1);
  rip (namebuf);

#if defined(WIN32)
  /*
   * Try the WinDns cache first
   */
  if (WinDnsQuery_A6(namebuf, &ip6_addr))
  {
    memcpy (addr, &ip6_addr, sizeof(ip6_addr));
    return (1);
  }
#endif

  if (last_nameserver == 0)
  {
    dom_errno = DNS_CLI_NOSERV;
    outsnl (dom_strerror(dom_errno));
    return (0);
  }

  if (my_ip_addr == 0)
  {
    dom_errno = DNS_CLI_NOIP;
    outsnl (dom_strerror(dom_errno));
    return (0);
  }

  if (dns_timeout == 0)
      dns_timeout = sock_delay << 2;
  res_timeout = set_timeout (1000 * dns_timeout);

  count = 0;
  memset (&timeout, 0, sizeof(timeout));
  query_init (&request, TRUE);

  NEW_BREAK_MODE (brk_mode, 1);
  _resolve_exit = _resolve_timeout = FALSE;

  do
  {
    int i;

    if (strchr(namebuf,'.'))
    {
      loc_domain = NULL;
      count = -1;
    }
    else if (!dns_recurse && count == 0)
    {
      loc_domain = NULL;
      count = -1;
    }
    else
    {
      loc_domain = next_domain (def_domain, count);
      if (!loc_domain)
         count = -1;
    }

    for (i = 0; i < last_nameserver; i++)
    {
      if (!timeout[i])
      {
        int rc = lookup_domain (&request, namebuf, count != -1,
                                def_nameservers[i], timeout+i,
                                TRUE, &ip6_addr);
        if (rc)
        {
          got_ip = TRUE;
          break;            /* got name, bail out of for loop */
        }
        if (_resolve_exit)
           break;           /* an error occured, return to caller */
      }

      /*
       * Should we really try the other nameservers if the first
       * says the host doesn't exist? Maybe a trusting mechanism
       * is needed (?)
       */
    }
    if (count == -1)
       break;
    count++;
  }
  while (!got_ip && !_resolve_exit);

  if (got_ip)
  {
    memcpy (addr, &ip6_addr, sizeof(ip6_addr));

#if defined(WIN32)
    /* Put the result in the WinDns cache.
     */
    WinDnsCachePut_A6 (namebuf, &ip6_addr);
#endif
  }

  OLD_BREAK_MODE (brk_mode);
  return (got_ip ? 1 : 0);
}
#endif /* USE_IPV6 */

