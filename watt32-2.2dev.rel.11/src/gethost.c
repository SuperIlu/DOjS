/*!\file gethost.c
 *
 *  BSD-like host-entry functions.
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
 *
 *  18.aug 1996 (GV)  - Created
 *  02.dec 1997 (GV)  - Integrated with resolve()
 *  05.jan 1998 (GV)  - Added host cache functionality
 *  18.may 1999 (GV)  - Added timeout of cached values
 *
 *  todo: support real host aliases as they come from the name server
 *  todo: accept "rooted FQDN" strings as normal FQDN strings.
 *        Note: "domain_name.com" and "domain_name.com." are equivalent
 *        (both are valid forms of fully qualified domain names (FQDNs);
 *        with the period, it is referred to as a rooted FQDN). Both forms
 *        should work with all mail clients and servers.  However, using the
 *        trailing "." is rarely used (except in DNS maintenance).
 *
 *  Note: if gethostbyname("some.host") returns address a.b.c.d, then
 *        a call to gethostbyaddr(a.b.c.d) will return "some.host"
 *        immediately (from cached info provided it didn't time out).
 *        This is contrary to most Unixes where the calls doesn't share
 *        the same information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "wattcp.h"
#include "strings.h"
#include "misc.h"
#include "run.h"
#include "timer.h"
#include "language.h"
#include "pcconfig.h"
#include "netaddr.h"
#include "pctcp.h"
#include "bsdname.h"
#include "bsddbug.h"
#include "pcdns.h"
#include "get_xby.h"

int h_errno = 0;

#if defined(USE_BSD_API)   /* Rest of file */

unsigned netdbCacheLife = MAX_CACHE_LIFE;

static char            *hostFname  = NULL;
static FILE            *hostFile   = NULL;
static BOOL             hostClose  = FALSE;
static const char      *from_where = NULL;
static struct _hostent *host0      = NULL;

static BOOL did_lookup = FALSE;   /* tried a DNS lookup */
static BOOL is_addr    = FALSE;   /* name is simply an IPv4 address */

static BOOL gethostbyname_internal (const char *name,
                                    const char **alias,
                                    struct _hostent *ret);

static BOOL gethostbyaddr_internal (const char *addr_name, int len, int type,
                                    struct _hostent *ret);

static struct hostent  *fill_hostent (const struct _hostent *h);

static struct _hostent *add_hostent (struct _hostent *h,
                                     const char *name, const char *cname,
                                     DWORD *alist, DWORD addr, DWORD ttl,
                                     BOOL windns);

static void W32_CALL _endhostent (void)
{
  endhostent();
}

void W32_CALL ReadHostsFile (const char *fname)
{
  static BOOL been_here = FALSE;

  if (!fname || !*fname)
     return;

  if (been_here)  /* loading multiple hosts files */
  {
    free (hostFname);
    FCLOSE (hostFile);
    hostFile = NULL;
  }

  hostFname = strdup (fname);
  if (!hostFname)
     return;

  sethostent (1);
  if (!hostFile)
     return;

  been_here = TRUE;

  while (1)
  {
    struct  hostent *h = gethostent();
    struct _hostent *h2;
    int     i;

    if (!h)
       break;

    h2 = calloc (sizeof(*h2), 1);
    if (!h2)
    {
      (*_printf) (_LANG("%s too big!\n"), hostFname);
      break;
    }

    for (i = 0; h->h_aliases[i]; i++)
        h2->h_aliases[i] = strdup (h->h_aliases[i]);
    h2->h_name       = strdup (h->h_name);
    h2->h_address[0] = *(DWORD*) h->h_addr_list[0];
    h2->h_num_addr   = 1;
    if (!h2->h_name)
       break;
    h2->h_next = host0;
    host0      = h2;
  }

#if 0  /* test !! */
  {
    const struct _hostent *h;
    int   i;

    printf ("\n%s entries:\n", hostFname);
    for (h = host0; h; h = h->h_next)
    {
      printf ("address = %-17.17s name = %-30.30s  Aliases:",
              inet_ntoa(*(struct in_addr*)&h->h_address[0]), h->h_name);
      for (i = 0; h->h_aliases[i]; i++)
          printf (" %s,", h->h_aliases[i]);
      puts ("");
    }
    fflush (stdout);
  }
#endif
  rewind (hostFile);
  RUNDOWN_ADD (_endhostent, 254);
}

/**
 * Return name of hosts-file.
 * \note Only returns the last 'hosts' file opened.
 */
const char * W32_CALL GetHostsFile (void)
{
  return (hostFname);
}

/**
 * Close hosts-file.
 * To prevent running out of file-handles, one should close the
 * hosts-file before spawning a new shell.
 */
void W32_CALL CloseHostFile (void)
{
  if (hostFile)
     FCLOSE (hostFile);
  hostFile = NULL;
}

/**
 * Reopen hosts-file.
 * Call this after e.g. system() returns.
 */
void W32_CALL ReopenHostFile (void)
{
  ReadHostsFile (hostFname);
}

/**
 * Return the next (non-commented) line from the host-file.
 * Format is:
 *  ip-address host-name [alias..] {\n | # ..}
 */
struct hostent * W32_CALL gethostent (void)
{
  struct _hostent h;
  char  *tok, *ip, *name, *alias, *tok_buf = NULL;
  char   buf [2*MAX_HOSTLEN];
  int    i;

  if (!netdb_init() || !hostFile)
  {
    h_errno = NO_RECOVERY;
    return (NULL);
  }

  while (1)
  {
    if (!fgets(buf,sizeof(buf),hostFile))
       return (NULL);

    tok = strltrim (buf);
    if (*tok == '#' || *tok == ';' || *tok == '\n')
       continue;

    ip   = strtok_r (tok, " \t", &tok_buf);
    name = strtok_r (NULL, " \t\n", &tok_buf);
    if (ip && name && isaddr(ip))
       break;
  }

  if (hostClose)
     endhostent();

  memset (&h, 0, sizeof(h));
  if (!strcmp(ip,"0.0.0.0"))   /* inet_addr() maps 0 -> INADDR_NONE */
       h.h_address[0] = INADDR_ANY;
  else h.h_address[0] = inet_addr (ip);

  h.h_num_addr = 1;
  h.h_name = name;
  alias = strtok_r (NULL, " \t\n", &tok_buf);

  for (i = 0; alias && i < MAX_HOST_ALIASES; i++)
  {
    static char aliases [MAX_HOST_ALIASES][MAX_HOSTLEN];

    if (*alias == '#' || *alias == ';')
       break;

    h.h_aliases[i] = _strlcpy (aliases[i], alias, sizeof(aliases[i]));
    alias = strtok_r (NULL, " \t\n", &tok_buf);
  }
  return fill_hostent (&h);
}

/*
 * Return a 'struct hostent *' for name.
 */
struct hostent * W32_CALL gethostbyname (const char *name)
{
  struct _hostent h;
  const char *alias;

  SOCK_DEBUGF (("\ngethostbyname: `%s'", name));
  is_addr = FALSE;

  if (gethostbyname_internal(name, &alias, &h))
  {
#if defined(USE_DEBUG)
    int i;
    for (i = 0; i < h.h_num_addr; i++)
        SOCK_DEBUGF ((" %s,", inet_ntoa(*(struct in_addr*)&h.h_address[i])));

    if (!did_lookup)
       SOCK_DEBUGF ((" %s", is_addr     ? "" :
                            h.h_timeout ? "cached" :
                            from_where  ? from_where : "hosts-file"));
    if (alias)
       SOCK_DEBUGF ((" (alias %s)", alias));
#endif
    return fill_hostent (&h);
  }

  if (called_from_resolve)
       SOCK_DEBUGF ((", unknown (called from resolve)"));
  else SOCK_DEBUGF ((", failed (%s)", did_lookup ? dom_strerror(dom_errno) :
                      hstrerror(h_errno)));
  return (NULL);
}

static BOOL gethostbyname_internal (const char *name, const char **alias,
                                    struct _hostent *ret)
{
  static char our_name [MAX_HOSTLEN];
  struct in_addr  addr;
  struct _hostent *h;
  time_t now;
  DWORD  ip;

  h_errno = HOST_NOT_FOUND;
  did_lookup = FALSE;
  from_where = NULL;

  _resolve_exit = _resolve_timeout = 0;
  memset (ret, 0, sizeof(*ret));
  *alias = NULL;

  if (!netdb_init())
  {
    h_errno = NO_RECOVERY;
    return (FALSE);
  }

  if (inet_aton(name,&addr))
  {
    /** \todo should be canonical name */
    ret->h_name       = (char*) name;
    ret->h_address[0] = addr.s_addr;
    ret->h_num_addr   = 1;
    is_addr = TRUE;
    return (TRUE);
  }

  now = time (NULL);

  for (h = host0; h; h = h->h_next)
  {
    int i;

    if (h->h_name && !stricmp(h->h_name,name))
    {
      /* if cached entry expired, do DNS lookup
       */
      if (h->h_timeout && now > h->h_timeout)
         goto expired;

      *ret = *h;
      return (h->h_address[0] != INADDR_NONE ? TRUE : FALSE);
    }
    for (i = 0; i < MAX_HOST_ALIASES && h->h_aliases[i]; i++)
        if (!stricmp(name,h->h_aliases[i]))
        {
          if (h->h_timeout && now > h->h_timeout)
             goto expired;

          *alias = h->h_aliases[i];
          *ret = *h;
          return (h->h_address[0] != INADDR_NONE ? TRUE : FALSE);
        }
  }

  /* Not found in linked list (hosts file or cache). Check name
   * against our own host-name (short-name or FQDN).
   * \todo Should return all our addresses if we're multihomed.
   */
  if (hostname[0] && !stricmp(name,hostname))
  {
    ret->h_num_addr   = 1;
    ret->h_address[0] = gethostid();
    ret->h_name       = hostname;
    from_where        = "gethostname";
    return (TRUE);
  }

  if (!gethostname(our_name,sizeof(our_name)) && !stricmp(name,our_name))
  {
    ret->h_num_addr   = 1;
    ret->h_address[0] = gethostid();
    ret->h_name       = our_name;
    from_where        = "gethostname";
    return (TRUE);
  }

expired:

  if (called_from_resolve) /* prevent recursion */
     return (FALSE);

  /* Do a full DNS lookup
   */
  called_from_ghbn = TRUE;
  ip = resolve (name);       /* do a normal lookup */
  called_from_ghbn = FALSE;
  did_lookup = TRUE;

  if (_resolve_exit ||   /* interrupted or other fail */
      _resolve_timeout)  /* timed out resolving */
     return (FALSE);

  if (ip)                /* successfully resolved */
  {
    h = add_hostent (h, name, dom_cname, dom_a4list, htonl(ip),
                     dom_ttl, from_windns);
    return (h ? *ret = *h, TRUE : FALSE);
  }

  /* Add the name to the list even if we got a negative DNS reply.
   * Thus the next call to gethostbyxx() will return immediately.
   */
  add_hostent (h, name, NULL, NULL, INADDR_NONE, netdbCacheLife, from_windns);
  return (FALSE);
}

struct hostent * W32_CALL gethostbyname2 (const char *name, int family)
{
  if (family == AF_INET)
     return gethostbyname (name);

#if defined(USE_IPV6)
  if (family == AF_INET6)
     return gethostbyname6 (name);
#endif

  h_errno = TRY_AGAIN;
  return (NULL);
}


/*
 * Return a 'struct hostent *' for an address.
 */
struct hostent * W32_CALL gethostbyaddr (const char *addr_name, int len, int type)
{
  struct _hostent h;

  SOCK_DEBUGF (("\ngethostbyaddr: %s",
                (type == AF_INET && addr_name) ?
                inet_ntoa(*(struct in_addr*)addr_name) :
                (type == AF_INET6 && addr_name) ?
                _inet6_ntoa(addr_name) : ""));

#if defined(USE_IPV6)
  if (type == AF_INET6 && len == sizeof(struct in6_addr))
  {
    struct hostent *he;

    SOCK_ENTER_SCOPE();
    he = gethostbyaddr6 (addr_name);
    SOCK_LEAVE_SCOPE();
    return (he);
  }
#endif

  if (gethostbyaddr_internal (addr_name, len, type, &h))
  {
    SOCK_DEBUGF ((" `%s'", h.h_name));
    if (!did_lookup)
       SOCK_DEBUGF ((", %s", h.h_timeout ? "cached" : "hosts-file"));
    return fill_hostent (&h);
  }

  SOCK_DEBUGF ((" failed (%s) ", did_lookup ? dom_strerror(dom_errno) :
                hstrerror(h_errno)));
  return (NULL);
}


void W32_CALL sethostent (int stayopen)
{
  hostClose = (stayopen == 0);
  if (!netdb_init() || !hostFname)
     return;

  if (!hostFile)
       FOPEN_TXT (hostFile, hostFname);
  else rewind (hostFile);
}

void W32_CALL endhostent (void)
{
  struct _hostent *h, *next = NULL;

  if (_watt_fatal_error)
     return;

  if (hostFname)
     free (hostFname);
  if (hostFile)
     FCLOSE (hostFile);
  hostFname = NULL;
  hostFile  = NULL;

  for (h = host0; h; h = next)
  {
    int i;
    for (i = 0; h->h_aliases[i]; i++)
        free (h->h_aliases[i]);
    next = h->h_next;
    free (h->h_name);
    free (h);
  }
  host0 = NULL;
  hostClose = TRUE;
}


static BOOL gethostbyaddr_internal (const char *addr_name, int len, int type,
                                    struct _hostent *ret)
{
  static char name [MAX_HOSTLEN];
  struct _hostent *h = NULL;
  DWORD  addr;
  BOOL   rc;
  time_t now;

  h_errno = HOST_NOT_FOUND;
  did_lookup = FALSE;
  from_where = NULL;
  _resolve_exit = _resolve_timeout = 0;
  memset (ret, 0, sizeof(*ret));

  if (type != AF_INET || len < SIZEOF(addr))
  {
    h_errno = NO_RECOVERY;
    return (FALSE);
  }

  if (!netdb_init())
  {
    h_errno = NO_RECOVERY;
    return (FALSE);
  }

  addr = *(DWORD*) addr_name;

  if ((addr == INADDR_ANY ||           /* 0.0.0.0 -> my_ip_addr */
       addr == gethostid()) &&
      gethostname(name,sizeof(name)) == 0)
  {
    /** \todo Should return all our addresses if we're multihomed.
     */
    ret->h_num_addr   = 1;
    ret->h_address[0] = gethostid();
    ret->h_name       = name;
    from_where        = "gethostname";
    return (TRUE);
  }

  if (addr == INADDR_BROADCAST ||      /* 255.255.255.255    */
      (~ntohl(addr) & ~sin_mask) == 0) /* directed broadcast */
  {
    ret->h_num_addr   = 1;
    ret->h_address[0] = addr;
    ret->h_name       = (char*) "broadcast";
    return (TRUE);
  }

  now = time (NULL);

  /* If called from getnameinfo() with AI_CANONNAME, we
   * should not search the host0 list. But do a full reeverse
   * lookup.
   */
  if (called_from_getai)
     goto expired;

  for (h = host0; h; h = h->h_next)
  {
    int i;

    for (i = 0; i < h->h_num_addr && h->h_address[i] != INADDR_NONE; i++)
    {
      if (addr == h->h_address[i])
      {
        /* if cached entry expired, do a new reverse lookup
         */
        if (h->h_timeout && now > h->h_timeout)
           goto expired;

        *ret = *h;
        if (!strcmp(h->h_name,"*unknown*"))
           return (FALSE);
        return (TRUE);
      }
    }
  }

expired:

  /* do a reverse ip lookup
   */
  did_lookup = TRUE;
  rc = reverse_resolve_ip4 (addr, name, sizeof(name));

  /* interrupted or timedout
   */
  if (!rc && (_resolve_exit || _resolve_timeout))
     return (FALSE);

  if (rc)     /* successfully resolved */
  {
    h = add_hostent (h, name, dom_cname, NULL, addr, dom_ttl, from_windns);
    /** \todo should be the new aliases */
    return (h ? *ret = *h, TRUE : FALSE);
  }

  /* Add the IP to the list even if reverse lookup failed and not
   * interrupted by _resolve_hook(). Thus the next call to gethostbyxx()
   * will return immediately.
   */
  add_hostent (h, "*unknown*", NULL, NULL, addr, 0UL, FALSE);
  return (FALSE);
}


#if defined(NOT_USED)
/*
 * Warn about calling 'getXbyY()' functions before calling
 * 'watt_sock_init()'. Many other functions will fail if we're not
 * initialised, but 'getXbyY()' are often used during application
 * startup.
 */
void uninit_warn (const char *func)
{
  (*_printf) ("Warning: function \"%s()\" called before \"sock_init()\".",
              func);
}
#endif

int * W32_CALL __h_errno_location (void)
{
  return (&h_errno);
}

static struct hostent *fill_hostent (const struct _hostent *h)
{
  static struct hostent ret;
  static struct in_addr addr [MAX_ADDRESSES+1];
  static char  *list [MAX_ADDRESSES+1];
  static char   hostnam [MAX_HOSTLEN+1];
  static char  *aliases [MAX_HOST_ALIASES+1];
  int    i;

  if (!h->h_name)
     return (NULL);

  memset (&addr, 0, sizeof(addr));
  memcpy (&aliases, h->h_aliases, sizeof(aliases));

  for (i = 0; i < h->h_num_addr && i < MAX_ADDRESSES; i++)
  {
    addr[i].s_addr = h->h_address[i];
    list[i]        = (char*) &addr[i];
  }

  list[i]         = NULL;
  ret.h_addr_list = list;
  ret.h_name      = _strlcpy (hostnam, h->h_name, sizeof(hostnam));
  ret.h_aliases   = aliases;
  ret.h_addrtype  = AF_INET;
  ret.h_length    = sizeof (addr[0].s_addr);
  h_errno         = NETDB_SUCCESS;
  return (&ret);
}

/*
 * Modify an expired cached entry or create a new node and
 * add it to the linked list. Not used for entries in hosts-file.
 */
static struct _hostent  *add_hostent
       (struct _hostent *h,
        const char      *name,   /* Host name */
        const char      *cname,  /* Canonical name (CNAME) */
        DWORD           *alist,  /* List of alternate addresses */
        DWORD            addr,   /* Main IP-address */
        DWORD            ttl,    /* Time-to-live (sec) */
        BOOL             windns) /* answer came from WinDNS */
{
  DWORD real_ttl = ttl;
  int   i;

  ttl = min (ttl, netdbCacheLife);  /* clamp the TTL */

  if (h)         /* reuse expired entry */
  {
    if (h->h_name)
       free (h->h_name);
    if (h->h_aliases[0])
       free (h->h_aliases[0]);     /* !! max 1 alias */
    memset (&h->h_address[1], 0,   /* clear old alternates */
            sizeof(h->h_address)-sizeof(u_long));
    h->h_aliases[0] = NULL;
  }
  else           /* create a new node */
  {
    h = calloc (sizeof(*h), 1);
    if (h)
    {
      h->h_next = host0;
      host0 = h;
    }
  }

  if (!cname || !cname[0])
     cname = NULL;

  if (addr != INADDR_NONE)
  {
    SOCK_DEBUGF ((", CNAME %s, TTL %s, ",
                  cname ? cname : "<none>", hms_str(real_ttl)));
#if defined(WIN32)
    SOCK_DEBUGF (("WinDNS %d, ", windns));
#endif
  }

  if (h)
  {
    h->h_timeout    = ttl ? time (NULL) + ttl : 0;
    h->h_real_ttl   = real_ttl;
    h->h_address[0] = addr;
    h->h_num_addr   = 1;

    for (i = 0; alist; i++)
    {
      if (alist[i] == INADDR_NONE || alist[i] == INADDR_ANY)
         break;
      h->h_address[i+1] = alist[i];
      h->h_num_addr++;
    }
    if (cname)    /* swap name with CNAME */
    {
      h->h_name       = strdup (cname);
      h->h_aliases[0] = strdup (name);  /* !! only allow 1 alias */
    }
    else
      h->h_name = strdup (name);
  }
  else
    SOCK_DEBUGF ((" ENOMEM"));

#if defined(TEST_PROG)  /* test updated cache */
  if (h)
  {
    printf ("new entry: name %-30.30s -> address %s ",
            h->h_name, inet_ntoa(*(struct in_addr*)&h->h_address[0]));
    for (i = 1; i < h->h_num_addr; i++)
        printf (", %s",
                inet_ntoa(*(struct in_addr*)&h->h_address[i]));
    puts ("");
  }
  else
    printf ("new entry: ENOMEM\n");
#endif

  ARGSUSED (windns);
  return (h);
}

#if defined(USE_DEBUG)
/*
 * Reverse the 'host0' list before printing it. I.e. on same order
 * the host-names where put in the list by add_hostent().
 */
static BOOL ReverseHostsList (void)
{
  struct _hostent *h, **list;
  int    i, num_total, num_dynamic;

  for (h = host0, num_total = num_dynamic = 0; h; h = h->h_next)
  {
    num_total++;
    if (h->h_timeout > 0)
       num_dynamic++;
  }
  if (num_total == 0 || num_dynamic == 0)
     return (FALSE);

  list = malloc (num_total * sizeof(*list));
  if (!list)
     return (FALSE);

  for (h = host0, i = 0; h; h = h->h_next)
      list[i++] = h;

  for (host0 = NULL, i = 0; i < num_total; i++)
  {
    h         = list[i];
    h->h_next = host0;
    host0     = h;
  }
  free (list);
  return (TRUE);
}

void W32_CALL DumpHostsCache (void)
{
  struct _hostent *h;
  time_t now;

  if (!ReverseHostsList())
     return;

  SOCK_DEBUGF ((" \n\nCached IPv4 hosts:                                   "
                "Address          TTL       Alias\n"));
  now = time (NULL);

  for (h = host0; h; h = h->h_next)
  {
    const char *ip_str, *cache_time;
    int   i;

    if (h->h_real_ttl == 0)   /* skip fixed hosts-file records */
       continue;

    cache_time = ((long)(h->h_timeout - now) < 0) ? "timedout" :
                  hms_str(h->h_real_ttl);
    ip_str     = (h->h_address[0] == INADDR_NONE) ? "<none>" :
                  inet_ntoa(*(struct in_addr*)&h->h_address[0]);

    SOCK_DEBUGF (("  %-50s %-15s %9s  ", h->h_name, ip_str, cache_time));
    for (i = 0; h->h_aliases[i]; i++)
        SOCK_DEBUGF (("%s ", h->h_aliases[i]));
    if (i == 0)
         SOCK_DEBUGF (("<none>\n"));
    else SOCK_DEBUGF ((" \n"));

    for (i = 1; i < h->h_num_addr; i++)
        SOCK_DEBUGF (("%52s %-15s\n", "", inet_ntoa(*(struct in_addr*)&h->h_address[i])));
  }
}
#endif


#if defined(TEST_PROG)

#if !defined(__CYGWIN__)
#include <conio.h>
#endif

#if !defined(_MSC_VER) && !defined(__BORLANDC__)
#include <unistd.h>
#endif

#include "pcdbug.h"
#include "sock_ini.h"

/*
 * Print list of hosts unsorted.
 */
static void print_hosts (void)
{
  const struct _hostent *h;

  for (h = host0; h; h = h->h_next)
  {
    int i;

    printf ("address %-17.17s  name %s;",
            inet_ntoa(*(struct in_addr*)&h->h_address[0]), h->h_name);

    for (i = 0; h->h_aliases[i]; i++)
    {
      if (i == 0)
         printf (" Aliases:");
      printf (" %s,", h->h_aliases[i]);
    }
    puts ("");
  }
  fflush (stdout);
}

void do_sleep (int time)
{
  while (time)
  {
    if (kbhit())
    {
      int ch = getch();
      if (ch == 27 || ch == 'q')
      {
        fputc ('\n', stderr);
        return;
      }
    }
    fprintf (stderr, "%4d", time--);
#ifdef WIN32
    Sleep (1000);
#else
    sleep (1);
#endif
    tcp_tick (NULL);  /* empty rx-buffers */
    fprintf (stderr, "\b\b\b\b");
  }
}

int main (void)
{
  const struct hostent *h;
  const char *host_name = "test-host";  /* This doesn't exist in real life */
  int   wait_time;
  DWORD addr_list [MAX_ADDRESSES+1];

  dbug_init();
  sock_init();
  print_hosts();

  wait_time = netdbCacheLife + 1;
  memset (&addr_list, 0, sizeof(addr_list));
  addr_list[0] = htonl (_inet_addr("80.22.33.45"));
  addr_list[1] = htonl (_inet_addr("222.22.33.46"));
  addr_list[2] = htonl (_inet_addr("217.22.33.47"));
  addr_list[3] = htonl (_inet_addr("81.22.33.48"));
  addr_list[4] = INADDR_NONE;

  SOCK_DEBUGF (("\nadd_hostent: `%s'", host_name));
  add_hostent (NULL, host_name, "some.cname.org", &addr_list[1],
               addr_list[0], netdbCacheLife, FALSE);
  h = gethostbyname (host_name);
  if (!h)
  {
    fprintf (stderr, "gethostbyname() failed!. h_errno = %d\n", h_errno);
    return (1);
  }
  fprintf (stderr, "Waiting for cache-entry to timeout..");
  do_sleep (wait_time);

  fprintf (stderr, "gethostbyname() should do a DNS lookup now.\n");

  /* Since "test-host" doesn't exist in real life, a new lookup should fail.
   * If 'h != NULL', it should mean the cached entry didn't timeout.
   */
  h = gethostbyname (host_name);
  if (h)
     fprintf (stderr, "entry didn't timeout!.\n");

#if defined(USE_FORTIFY)
  Fortify_ListAllMemory();
  Fortify_OutputStatistics();
#endif

  return (0);
}
#endif /* TEST_PROG */
#endif /* USE_BSD_API */

