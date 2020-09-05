/*!\file get_ai.c
 * BSD getaddrinfo().
 */

/* Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * "FAITH" part is local hack for supporting IPv4-v6 translator.
 *
 * Issues to be discussed:
 * - Thread safe-ness must be checked.
 * - Return values.  There are nonstandard return values defined and used
 *   in the source code. This is because RFC2553 is silent about which error
 *   code must be returned for which situation.
 * Note:
 * - The code filters out AFs that are not supported by the kernel,
 *   when globbing NULL hostname (to loopback, or wildcard). Is it the right
 *   thing to do? What is the relationship with post-RFC2553 AI_ADDRCONFIG
 *   in ai_flags?
 *
 * Adapted for Watt-32 tcp/ip stack by G. Vanem <gvanem@yahoo.no>, Aug 2002.
 */

#include "socket.h"
#include "pcdns.h"

#if defined(USE_BSD_API)

#define ANY       0
#define YES       1
#define NO        0
#define PTON_MAX  16

/* Addresss family descriptor
 */
struct afd {
       int         a_af;
       int         a_addrlen;
       int         a_socklen;
       int         a_off;
       const char *a_addrany;
       const char *a_loopback;
       int         a_scoped;
     };

struct explore {
       int         e_af;
       int         e_socktype;
       int         e_protocol;
       const char *e_protostr;
       int         e_wild;
#define WILD_AF(ex)        ((ex)->e_wild & 0x01)
#define WILD_SOCKTYPE(ex)  ((ex)->e_wild & 0x02)
#define WILD_PROTOCOL(ex)  ((ex)->e_wild & 0x04)
      };

static struct explore explore[] = {
     { AF_INET6, SOCK_DGRAM,  IPPROTO_UDP, "udp", 7 },
     { AF_INET6, SOCK_STREAM, IPPROTO_TCP, "tcp", 7 },
     { AF_INET6, SOCK_RAW,    ANY,         NULL,  5 },
     { AF_INET,  SOCK_DGRAM,  IPPROTO_UDP, "udp", 7 },
     { AF_INET,  SOCK_STREAM, IPPROTO_TCP, "tcp", 7 },
     { AF_INET,  SOCK_RAW,    ANY,         NULL,  5 },
     { -1,       0,           0,           NULL,  0 }
   };

BOOL called_from_getai = FALSE;

static struct afd afd_list [3];

static int str_isnumber (const char *);
static int explore_fqdn (const struct addrinfo *, const char *, const char *,
                         struct addrinfo **res);

static int explore_null (const struct addrinfo *, const char *,
                         struct addrinfo **);

static int explore_numeric (const struct addrinfo *, const char *, const char *,
                            struct addrinfo **);

static int explore_numeric_scope (const struct addrinfo *, const char *,
                                  const char *, struct addrinfo **);

static int get_name (const char *, const struct afd *, struct addrinfo **,
                     char *, const struct addrinfo *, const char *);

static int get_canonname (const struct addrinfo *, struct addrinfo *,
                          const char *);

static struct addrinfo *get_ai (const struct addrinfo *, const struct afd *,
                                const char *);

static int get_portmatch (const struct addrinfo *, const char *);
static int get_port (struct addrinfo *, const char *, int);
static const struct afd *find_afd (int);

static const char *ai_errlist[] = {
  "Success",
  "Address family for hostname not supported",    /* EAI_ADDRFAMILY */
  "Temporary failure in name resolution",         /* EAI_AGAIN      */
  "Invalid value for ai_flags",                   /* EAI_BADFLAGS   */
  "Non-recoverable failure in name resolution",   /* EAI_FAIL       */
  "ai_family not supported",                      /* EAI_FAMILY     */
  "Memory allocation failure",                    /* EAI_MEMORY     */
  "No address associated with hostname",          /* EAI_NODATA     */
  "hostname nor servname provided, or not known", /* EAI_NONAME     */
  "servname not supported for ai_socktype",       /* EAI_SERVICE    */
  "ai_socktype not supported",                    /* EAI_SOCKTYPE   */
  "System error returned in errno",               /* EAI_SYSTEM     */
  "Invalid value for hints",                      /* EAI_BADHINTS   */
  "Resolved protocol is unknown",                 /* EAI_PROTOCOL   */
  "Unknown error",                                /* EAI_MAX        */
};

/* XXX macros that make external reference is BAD. */

#define GET_AI(ai, afd, addr) do {                        \
        /* external reference: pai, error and free_it */  \
        (ai) = get_ai (pai, (afd), (const char*) (addr)); \
        if ((ai) == NULL) {                               \
           error = EAI_MEMORY;                            \
           goto free_it;                                  \
        }                                                 \
      } while (0)

#define GET_PORT(ai, serv) do {                           \
        /* external reference: error and free_it */       \
        error = get_port ((ai), (serv), 0);               \
        if (error != 0)                                   \
           goto free_it;                                  \
      } while (0)

#define GET_CANONNAME(ai, str) do {                       \
        /* external reference: pai, error and free_it */  \
        error = get_canonname (pai, (ai), (str));         \
        if (error != 0)                                   \
           goto free_it;                                  \
      } while (0)

#define ERR(err) do {                                     \
        /* external reference: error, and label bad */    \
        error = (err);                                    \
        goto bad;                                         \
      } while (0)

#define MATCH_FAMILY(x, y, w) \
        ((x) == (y) || ((w) && ((x) == AF_UNSPEC || (y) == AF_UNSPEC)))

#define MATCH(x, y, w) \
        ((x) == (y) || ((w) && ((x) == ANY || (y) == ANY)))

char * W32_CALL gai_strerror (int ecode)
{
  if (ecode < 0 || ecode > EAI_MAX)
     ecode = EAI_MAX;
  return (char*) ai_errlist[ecode];
}

/*
 * We only have 1 interface.
 * Size of 'if_name' must be >= IF_NAMESIZE.
 * Assume __get_ifname() returns name with trailing digit.
 */
int W32_CALL if_nametoindex (const char *if_name)
{
  char name [IF_NAMESIZE];
  char id;

  __get_ifname (name);
  id = name [strlen(name)] - '0';
  if (!stricmp(if_name,name))
     return (id+1);
  return (0);
}

char * W32_CALL if_indextoname (int if_id, char *if_name)
{
  char name [IF_NAMESIZE];
  int  id;

  __get_ifname (name);
  id = name [strlen(name)] - '0';
  if (if_id != id+1)
  {
    SOCK_ERRNO (ENXIO);
    return (NULL);
  }
  return strcpy (if_name, name);
}

int __scope_ascii_to_id (const char *str)
{
  if (!isdigit((int)*str))
     return (0);
  return (*str - '0');
}

int __scope_id_to_ascii (int scope)
{
  scope += '0';
  if (!isdigit(scope))
     return (0);
  return (scope);
}

static void free_addrinfo (struct addrinfo *ai)
{
  struct addrinfo *next;

  for ( ; ai; ai = next)
  {
    next = ai->ai_next;
    if (ai->ai_canonname)
       free (ai->ai_canonname);
    free (ai);
  }
}

void W32_CALL freeaddrinfo (struct addrinfo *ai)
{
  SOCK_DEBUGF (("\nfreeaddrinfo: 0x%" ADDR_FMT, ADDR_CAST(ai)));
  free_addrinfo (ai);
}

static int str_isnumber (const char *p)
{
  char *q = (char *) p;

  while (*q)
  {
    if (!isdigit((int)*q))
       return (NO);
    q++;
  }
  return (YES);
}

/**
 * Initialise afd_list[].
 * Give IPv6 protocol highest priority if 'ip6_prio' is TRUE.
 */
static void afd_list_init (BOOL ip6_prio)
{
  static const char in4addr_loopback[] = { 127, 0, 0, 1 };
  static const char in4addr_addrany[]  = { 0, 0, 0, 0 };
#if !defined(USE_IPV6)
  static const struct in6_addr in6addr_loopback = {{ 0,0,0,0,0,0,0,0,
                                                     0,0,0,0,0,0,0,1 }};
  static const struct in6_addr in6addr_any      = {{ 0,0,0,0,0,0,0,0,
                                                     0,0,0,0,0,0,0,0 }};
#endif


  int    i = (ip6_prio ? 0 : 1);

  memset (&afd_list, 0, sizeof(afd_list));

  afd_list[i].a_af       = AF_INET6;
  afd_list[i].a_addrlen  = sizeof (struct in6_addr);
  afd_list[i].a_socklen  = sizeof (struct sockaddr_in6);
  afd_list[i].a_off      = offsetof (struct sockaddr_in6, sin6_addr);
  afd_list[i].a_addrany  = (const char*) &in6addr_any;
  afd_list[i].a_loopback = (const char*) &in6addr_loopback;
  afd_list[i].a_scoped   = 1;

  i ^= 1;
  afd_list[i].a_af       = AF_INET;
  afd_list[i].a_addrlen  = sizeof (struct in_addr);
  afd_list[i].a_socklen  = sizeof (struct sockaddr_in);
  afd_list[i].a_off      = offsetof (struct sockaddr_in, sin_addr);
  afd_list[i].a_addrany  = in4addr_addrany;
  afd_list[i].a_loopback = in4addr_loopback;
  afd_list[i].a_scoped   = 0;

  if (!ip6_prio)
  {
    explore[0].e_af = explore[1].e_af = explore[2].e_af = AF_INET;
    explore[3].e_af = explore[4].e_af = explore[5].e_af = AF_INET6;
  }
}

int W32_CALL getaddrinfo (const char *hostname, const char *servname,
                          const struct addrinfo *hints, struct addrinfo **res)
{
  static BOOL init = FALSE;
  struct addrinfo      *cur, *pai, ai, ai0, sentinel;
  const struct afd     *afd;
  const struct explore *ex;
  int   error = 0;

  if (!init)
     afd_list_init (TRUE);  /* Give IPv6 highest priority */
  init = TRUE;

  sentinel.ai_next = NULL;
  cur = &sentinel;
  pai = &ai;
  pai->ai_flags     = 0;
  pai->ai_family    = AF_UNSPEC;
  pai->ai_socktype  = ANY;
  pai->ai_protocol  = ANY;
  pai->ai_addrlen   = 0;
  pai->ai_canonname = NULL;
  pai->ai_addr      = NULL;
  pai->ai_next      = NULL;

  SOCK_DEBUGF (("\ngetaddrinfo: `%s', `%s'",
                hostname, servname));

  if (!hostname && !servname)
     ERR (EAI_NONAME);

  if (hints)
  {
    /* error check for hints */
    if (hints->ai_addrlen || hints->ai_canonname ||
        hints->ai_addr    || hints->ai_next)
       ERR (EAI_BADHINTS);       /* xxx */

    if (hints->ai_flags & ~AI_MASK)
       ERR (EAI_BADFLAGS);

    switch (hints->ai_family)
    {
      case AF_UNSPEC:
      case AF_INET:
      case AF_INET6:
           break;
      default:
           ERR (EAI_FAMILY);
    }
    *pai = *hints;

    /* if both socktype/protocol are specified, check if they
     * are meaningful combination.
     */
    if (pai->ai_socktype != ANY && pai->ai_protocol != ANY)
    {
      for (ex = explore; ex->e_af >= 0; ex++)
      {
        if (pai->ai_family != ex->e_af ||
            ex->e_socktype == ANY ||
            ex->e_protocol == ANY)
           continue;

        if (pai->ai_socktype == ex->e_socktype &&
            pai->ai_protocol != ex->e_protocol)
           ERR (EAI_BADHINTS);
      }
    }
  }

  /* Check for special cases:
   *  (1) numeric servname is disallowed if socktype/protocol are left
   *      unspecified.
   *  (2) servname is disallowed for raw and other inet{,6} sockets.
   */
  if (MATCH_FAMILY(pai->ai_family, AF_INET, 1) ||
      MATCH_FAMILY(pai->ai_family, AF_INET6, 1))
  {
    ai0 = *pai;

    if (pai->ai_family == AF_UNSPEC)
        pai->ai_family = AF_INET6;
    error = get_portmatch (pai, servname);
    if (error)
       ERR (error);
    *pai = ai0;
  }

  ai0 = *pai;

  /* NULL hostname, or numeric hostname
   */
  for (ex = explore; ex->e_af >= 0; ex++)
  {
    *pai = ai0;

    if (!MATCH_FAMILY(pai->ai_family, ex->e_af, WILD_AF(ex)))
       continue;

    if (!MATCH(pai->ai_socktype, ex->e_socktype, WILD_SOCKTYPE(ex)))
       continue;

    if (!MATCH(pai->ai_protocol, ex->e_protocol, WILD_PROTOCOL(ex)))
       continue;

    if (pai->ai_family == AF_UNSPEC)
        pai->ai_family = ex->e_af;
    if (pai->ai_socktype == ANY && ex->e_socktype != ANY)
        pai->ai_socktype = ex->e_socktype;
    if (pai->ai_protocol == ANY && ex->e_protocol != ANY)
        pai->ai_protocol = ex->e_protocol;

    if (!hostname)
         error = explore_null (pai, servname, &cur->ai_next);
    else error = explore_numeric_scope (pai, hostname, servname, &cur->ai_next);

    if (error)
       goto free_it;

    while (cur && cur->ai_next)
      cur = cur->ai_next;
  }

  /* If numeric representation of AF1 can be interpreted as FQDN
   * representation of AF2, we need to think again about the code below.
   */
  if (sentinel.ai_next)
     goto good;

  if (pai->ai_flags & AI_NUMERICHOST)
     ERR (EAI_NONAME);

  if (!hostname)
     ERR (EAI_NONAME);

  /* hostname as alphabetical name.
   * We would like to prefer AF_INET6 than AF_INET, so we'll make a
   * outer loop by AFs.
   */
  for (afd = afd_list; afd->a_af; afd++)
  {
    *pai = ai0;

    if (!MATCH_FAMILY (pai->ai_family, afd->a_af, 1))
       continue;

    for (ex = explore; ex->e_af >= 0; ex++)
    {
      *pai = ai0;

      if (pai->ai_family == AF_UNSPEC)
          pai->ai_family = afd->a_af;

      if (!MATCH_FAMILY (pai->ai_family, ex->e_af, WILD_AF(ex)))
         continue;

      if (!MATCH (pai->ai_socktype, ex->e_socktype, WILD_SOCKTYPE(ex)))
         continue;

      if (!MATCH (pai->ai_protocol, ex->e_protocol, WILD_PROTOCOL(ex)))
         continue;

      if (pai->ai_family == AF_UNSPEC)
          pai->ai_family = ex->e_af;
      if (pai->ai_socktype == ANY && ex->e_socktype != ANY)
          pai->ai_socktype = ex->e_socktype;
      if (pai->ai_protocol == ANY && ex->e_protocol != ANY)
          pai->ai_protocol = ex->e_protocol;

      error = explore_fqdn (pai, hostname, servname, &cur->ai_next);

      if (_resolve_exit)  /* interrupted or other fail */
      {
        error = EAI_MAX;
        goto free_it;
      }
      while (cur && cur->ai_next)
        cur = cur->ai_next;
    }
  }

good:
  if (sentinel.ai_next)
     error = 0;

  if (error == 0)
  {
    if (sentinel.ai_next)
    {
      if (res)
         *res = sentinel.ai_next;
      // dump_addrinfo (*res);
      return (0);   /* success */
    }
    error = EAI_FAIL;
  }

free_it:
bad:
  free_addrinfo (sentinel.ai_next);
  if (res)
     *res = NULL;
  SOCK_DEBUGF ((": %s", ai_errlist[error]));
  return (error);
}

/*
 * FQDN hostname, DNS lookup
 */
static int explore_fqdn (const struct addrinfo *pai, const char *hostname,
                         const char *servname, struct addrinfo **res)
{
  static struct hostent  copy;
  static struct in6_addr addr [MAX_ADDRESSES+1];
  static char           *list [MAX_ADDRESSES+1], *ap;
  struct hostent   *hp = NULL;
  struct addrinfo   sentinel, *cur;
  const struct afd *afd;
  int             af, i, error = 0;

#ifdef TEST_PROG
  SOCK_DEBUGF (("\nexplore_fqdn"));
#endif

  *res = NULL;
  cur  = &sentinel;
  sentinel.ai_next = NULL;

  /* Do not filter unsupported AFs here. We need to honor content of
   * databases (/etc/hosts, DNS and others). Otherwise we cannot
   * replace gethostbyname() by getaddrinfo().
   */

  /* if the servname does not match socktype/protocol, ignore it.
   */
  if (get_portmatch(pai, servname) != 0)
     return (0);

  afd = find_afd (pai->ai_family);

  /* Post-RFC2553: should look at (pai->ai_flags & AI_ADDRCONFIG)
   * rather than hardcoding it. We may need to add AI_ADDRCONFIG
   * handling code by ourselves.
   */
  SOCK_ENTER_SCOPE();
  hp = gethostbyname2 (hostname, pai->ai_family);
  SOCK_LEAVE_SCOPE();

  if (!hp)
  {
    switch (h_errno)
    {
      case HOST_NOT_FOUND:
      case NO_DATA:
           error = EAI_NODATA;
           break;
      case TRY_AGAIN:
           error = EAI_AGAIN;
           break;
      case NO_RECOVERY:
      case NETDB_INTERNAL:
      default:
           error = EAI_FAIL;
           break;
    }

  }
  else if (!hp->h_name || !hp->h_name[0] || !hp->h_addr_list[0])
  {
    hp = NULL;
    error = EAI_FAIL;
  }

  if (!hp)
     goto free_it;

 /* Perform a shallow copy of 'hp'. Since 'hp' is returned from
  * fill_hostent(), the contents will be destroyed in below
  * get_name() otherwise.
  */
  copy = *hp;
  memset (&addr, 0, sizeof(addr));
  memset (&list, 0, sizeof(list));
  for (i = 0; hp->h_addr_list[i]; i++)
  {
    memcpy (&addr[i], hp->h_addr_list[i], hp->h_length);
    list[i] = (char*) &addr[i];
  }
  list[i]          = NULL;
  copy.h_addr_list = list;
  hp = &copy;

  for (i = 0; hp->h_addr_list[i]; i++)
  {
    af = hp->h_addrtype;
    ap = hp->h_addr_list[i];
    if (af == AF_INET6 && IN6_IS_ADDR_V4MAPPED(ap))
    {
      af = AF_INET;
      ap += sizeof(struct in6_addr) - sizeof(struct in_addr);
    }

    if (af != pai->ai_family)
       continue;

    if (!(pai->ai_flags & AI_CANONNAME))
    {
      GET_AI (cur->ai_next, afd, ap);
      GET_PORT (cur->ai_next, servname);
    }
    else
    {
      /* if AI_CANONNAME and if reverse lookup fail, return ai anyway
       * to pacify calling application.
       *
       * XXX getaddrinfo() is a name to address translation function,
       * and it looks strange that we do addr to name translation here.
       */
      get_name (ap, afd, &cur->ai_next, ap, pai, servname);
    }
    while (cur && cur->ai_next)
      cur = cur->ai_next;
  }
  *res = sentinel.ai_next;
  return (0);

free_it:
  free_addrinfo (sentinel.ai_next);
  return (error);
}

/*
 * hostname == NULL.
 * passive socket -> anyaddr (0.0.0.0 or ::)
 * non-passive socket -> localhost (127.0.0.1 or ::1)
 */
static int explore_null (const struct addrinfo *pai, const char *servname,
                         struct addrinfo **res)
{
  const struct afd *afd;
  struct addrinfo  *cur, sentinel;
  int    s, error;

#ifdef TEST_PROG
  SOCK_DEBUGF (("\nexplore_null"));
#endif

  *res = NULL;
  sentinel.ai_next = NULL;
  cur = &sentinel;

  /* filter out AFs that are not supported
   */
  SOCK_ENTER_SCOPE();
  s = socket (pai->ai_family, SOCK_DGRAM, 0);
  if (s >= 0)
     close_s (s);
  SOCK_LEAVE_SCOPE();

  if (s < 0)
     return (0);

  /* if the servname does not match socktype/protocol, ignore it.
   */
  if (get_portmatch (pai, servname) != 0)
     return (0);

  afd = find_afd (pai->ai_family);

  if (pai->ai_flags & AI_PASSIVE)
  {
    GET_AI (cur->ai_next, afd, afd->a_addrany);
    /* xxx meaningless?
     * GET_CANONNAME(cur->ai_next, "anyaddr");
     */
    GET_PORT (cur->ai_next, servname);
  }
  else
  {
    GET_AI (cur->ai_next, afd, afd->a_loopback);
    /* xxx meaningless?
     * GET_CANONNAME(cur->ai_next, "localhost");
     */
    GET_PORT (cur->ai_next, servname);
  }

  *res = sentinel.ai_next;
  return (0);

free_it:
  free_addrinfo (sentinel.ai_next);
  return (error);
}

/*
 * numeric hostname
 */
static int explore_numeric (const struct addrinfo *pai, const char *hostname,
                            const char *servname, struct addrinfo **res)
{
  const struct afd *afd;
  struct addrinfo  *cur, sentinel;
  int    error, flags;
  union {
    struct in_addr a4;
    struct in6_addr a6;
  } pton;

#ifdef TEST_PROG
  SOCK_DEBUGF (("\nexplore_numeric"));
#endif

  *res = NULL;
  sentinel.ai_next = NULL;
  cur = &sentinel;

  /* if the servname does not match socktype/protocol, ignore it.
   */
  if (get_portmatch (pai, servname) != 0)
     return (0);

  afd   = find_afd (pai->ai_family);
  flags = pai->ai_flags;

  if (inet_pton (afd->a_af, hostname, &pton) == 1)
  {
    DWORD v4a;
    BYTE  pfx;

    switch (afd->a_af)
    {
      case AF_INET:
           v4a = (DWORD) ntohl (pton.a4.s_addr);
           if (IN_MULTICAST (v4a) || IN_EXPERIMENTAL(v4a))
              flags &= ~AI_CANONNAME;
           v4a >>= IN_CLASSA_NSHIFT;
           if (v4a == 0 || v4a == IN_LOOPBACKNET)
              flags &= ~AI_CANONNAME;
           break;

      case AF_INET6:
           pfx = pton.a6.s6_addr[0];
           if (pfx == 0 || pfx == 0xfe || pfx == 0xff)
              flags &= ~AI_CANONNAME;
           break;
    }

    if (pai->ai_family == afd->a_af || pai->ai_family == AF_UNSPEC)
    {
      if (!(flags & AI_CANONNAME))
      {
        GET_AI (cur->ai_next, afd, &pton);
        GET_PORT (cur->ai_next, servname);
      }
      else
      {
        /* if AI_CANONNAME and if reverse lookup fail, return ai anyway
         * to pacify calling application.
         *
         * XXX getaddrinfo() is a name to address translation function,
         * and it looks strange that we do addr to name translation here.
         */
        get_name ((const char*)&pton, afd, &cur->ai_next,
                  (char*)&pton, pai, servname);
      }
      while (cur && cur->ai_next)
        cur = cur->ai_next;
    }
    else
      ERR (EAI_FAMILY);         /* xxx */
  }
  *res = sentinel.ai_next;
  return (0);

free_it:
bad:
  free_addrinfo (sentinel.ai_next);
  return (error);
}

/*
 * numeric hostname with scope
 */
static int explore_numeric_scope (const struct addrinfo *pai,
                                  const char *hostname, const char *servname,
                                  struct addrinfo **res)
{
  const struct afd    *afd;
  struct addrinfo     *cur;
  struct sockaddr_in6 *sin6;
  char *cp, hostname2 [MAX_HOSTLEN];
  int   error, scope = 0;

#ifdef TEST_PROG
  SOCK_DEBUGF (("\nexplore_numeric_scope"));
#endif

  /* if the servname does not match socktype/protocol, ignore it.
   */
  if (get_portmatch (pai, servname) != 0)
     return (0);

  afd = find_afd (pai->ai_family);
  if (!afd->a_scoped)
     return explore_numeric (pai, hostname, servname, res);

  cp = strchr (hostname, SCOPE_DELIMITER);
  if (!cp || (cp - hostname) >= SIZEOF(hostname2))
     return explore_numeric (pai, hostname, servname, res);

  /* Handle special case of <scoped_address>%<scope id>
   * 'scope id' is numeric "1..x".
   */
  _strlcpy (hostname2, hostname, sizeof(hostname2));

  /* terminate at the delimiter
   */
  hostname2 [cp-hostname] = '\0';

  switch (pai->ai_family)
  {
    case AF_INET6:
         scope = __scope_ascii_to_id (cp+1);
         if (scope == 0)
            return (EAI_NONAME);
         break;
  }

  error = explore_numeric (pai, hostname2, servname, res);
  if (error == 0)
  {
    for (cur = *res; cur; cur = cur->ai_next)
    {
      if (cur->ai_family != AF_INET6)
         continue;

      sin6 = (struct sockaddr_in6*) cur->ai_addr;
      if (IN6_IS_ADDR_LINKLOCAL(&sin6->sin6_addr) ||
          IN6_IS_ADDR_MC_LINKLOCAL(&sin6->sin6_addr))
        sin6->sin6_scope_id = scope;
    }
  }
  return (error);
}

static int get_name (const char *addr, const struct afd *afd,
                     struct addrinfo **res, char *numaddr,
                     const struct addrinfo *pai, const char *servname)
{
  struct hostent  *hp;
  struct addrinfo *cur = NULL;
  int    error = 0;

  SOCK_ENTER_SCOPE();
  called_from_getai = TRUE;
  hp = gethostbyaddr (addr, afd->a_addrlen, afd->a_af);
  called_from_getai = FALSE;
  SOCK_LEAVE_SCOPE();

  if (hp && hp->h_name && hp->h_name[0] && hp->h_addr_list[0])
  {
    GET_AI (cur, afd, hp->h_addr);
    GET_PORT (cur, servname);
    GET_CANONNAME (cur, hp->h_name);
  }
  else
  {
    GET_AI (cur, afd, numaddr);
    GET_PORT (cur, servname);
  }

  *res = cur;
  return (0);  /* success */

free_it:
  free_addrinfo (cur);
  *res = NULL;
  return (error);
}

static int get_canonname (const struct addrinfo *pai, struct addrinfo *ai,
                          const char *str)
{
  if (pai->ai_flags & AI_CANONNAME)
  {
    ai->ai_canonname = strdup (str);
    if (!ai->ai_canonname)
       return (EAI_MEMORY);
  }
  return (0);
}

static struct addrinfo *get_ai (const struct addrinfo *pai,
                                const struct afd      *afd,
                                const char            *addr)
{
  struct addrinfo *ai;
  char  *p;

  ai = calloc (sizeof(*ai) + afd->a_socklen, 1);
  if (!ai)
     return (NULL);

  *ai = *pai;
  ai->ai_addr            = (struct sockaddr*) (ai + 1);
  ai->ai_addrlen         = afd->a_socklen;
  ai->ai_addr->sa_family = ai->ai_family = afd->a_af;

  p = (char*)ai->ai_addr + afd->a_off;
  memcpy (p, addr, afd->a_addrlen);
  return (ai);
}

static int get_portmatch (const struct addrinfo *ai, const char *servname)
{
  /* get_port does not touch first argument. when matchonly == 1. */
  return get_port ((struct addrinfo*)ai, servname, 1);
}

static int get_port (struct addrinfo *ai, const char *servname, int matchonly)
{
  struct servent *sp;
  const  char    *proto;
  WORD   port;
  BOOL   allownumeric;

  if (!servname ||
      (ai->ai_family != AF_INET && ai->ai_family != AF_INET6))
     return (0);

  switch (ai->ai_socktype)
  {
    case SOCK_RAW:
    case SOCK_PACKET:
         return (EAI_SERVICE);
    case SOCK_DGRAM:
    case SOCK_STREAM:
    case ANY:
         allownumeric = TRUE;
         break;
    default:
         return (EAI_SOCKTYPE);
  }

  if (str_isnumber(servname))
  {
    if (!allownumeric)
       return (EAI_SERVICE);
    port = htons (ATOI(servname));
  }
  else
  {
    switch (ai->ai_socktype)
    {
      case SOCK_DGRAM:
           proto = "udp";
           break;
      case SOCK_STREAM:
           proto = "tcp";
           break;
      default:
           proto = NULL;
           break;
    }

    SOCK_ENTER_SCOPE();
    sp = getservbyname (servname, proto);
    SOCK_LEAVE_SCOPE();

    if (!sp)
       return (EAI_SERVICE);
    port = sp->s_port;
  }

  if (!matchonly)
  {
    switch (ai->ai_family)
    {
      case AF_INET:
           ((struct sockaddr_in*)ai->ai_addr)->sin_port = port;
           break;
      case AF_INET6:
           ((struct sockaddr_in6*)ai->ai_addr)->sin6_port = port;
           break;
    }
  }
  return (0);
}

static const struct afd *find_afd (int af)
{
  const struct afd *afd;

  if (af == AF_UNSPEC)
     return (NULL);

  for (afd = afd_list; afd->a_af; afd++)
      if (afd->a_af == af)
         return (afd);
  return (NULL);
}

#if defined(TEST_PROG)
static void dump_addrinfo (const struct addrinfo *ai)
{
  for (; ai; ai = ai->ai_next)
  {
    const struct sockaddr_in  *sa4 = (const struct sockaddr_in*)ai->ai_addr;
    const struct sockaddr_in6 *sa6 = (const struct sockaddr_in6*)ai->ai_addr;
    int   af = ai->ai_family;
    char  buf [INET6_ADDRSTRLEN];

    printf ("    family %2d, CNAME %s, ",
            af, ai->ai_canonname ? ai->ai_canonname : "<none>");
    printf ("%s\n",
            af == AF_INET  ? inet_ntop(af, &sa4->sin_addr, buf, sizeof(buf)) :
            af == AF_INET6 ? inet_ntop(af, &sa6->sin6_addr, buf, sizeof(buf)) :
            "??");
  }
}

static void test_getaddrinfo (const char *host, const char *serv)
{
  struct addrinfo hints, *res = NULL;
  int rc;

  printf ("Calling: getaddrinfo (\"%s\", \"%s\",...);", host, serv);
  memset (&hints, 0, sizeof(hints));
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_CANONNAME;

  rc = getaddrinfo (host, serv, &hints, &res);
  puts ("");
  if (res)
       dump_addrinfo (res);
  else printf ("fail; %s\n", gai_strerror(rc));

  if (res)
     freeaddrinfo (res);
}

int main (void)
{
  dbug_init();
  sock_init();

  test_getaddrinfo ("www.kame.net", "http");
  test_getaddrinfo ("www.watt-32.net", "http");

  return (0);
}
#endif /* TEST_PROG */
#endif /* USE_BSD_API */

