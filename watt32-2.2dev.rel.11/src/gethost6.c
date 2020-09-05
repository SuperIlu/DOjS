/*!\file gethost6.c
 *
 *  gethostbyname6() for IPv6.
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
 *  03.Aug 2002 (GV)  - Created
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <netdb.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <resolv.h>

#include "wattcp.h"
#include "strings.h"
#include "misc.h"
#include "run.h"
#include "timer.h"
#include "language.h"
#include "ip6_out.h"
#include "pcconfig.h"
#include "pctcp.h"
#include "netaddr.h"
#include "bsdname.h"
#include "bsddbug.h"
#include "pcdns.h"
#include "get_xby.h"

/* \if USE_IPV6 */
#if defined(USE_BSD_API) && defined(USE_IPV6) /* whole file */

static char             *host6Fname = NULL;
static FILE             *host6file  = NULL;
static BOOL              hostClose  = FALSE;
static struct _hostent6 *host0      = NULL;

static BOOL did_lookup = FALSE;   /* tried a DNS lookup */
static BOOL is_addr    = FALSE;   /* name is simply an IPv6 address */

static BOOL gethostbyname6_internal (const char *name,
                                     const char **alias,
                                     struct _hostent6 *ret);

static BOOL gethostbyaddr6_internal (const char *addr,
                                     struct _hostent6 *ret);

static struct hostent *fill_hostent6 (const struct _hostent6 *h);

static struct _hostent6 *add_hostent6 (struct _hostent6 *h,
                                       const char       *name,
                                       const char       *cname,
                                       const void       *addr_list,
                                       const void       *addr,
                                       DWORD             ttl);

static void W32_CALL _endhostent6 (void)
{
  endhostent6();
}

void W32_CALL ReadHosts6File (const char *fname)
{
  static BOOL been_here = FALSE;

  if (!fname || !*fname)
     return;

  if (been_here)  /* loading multiple hosts6 files */
  {
    free (host6Fname);
    CloseHost6File();
  }

  host6Fname = strdup (fname);
  if (!host6Fname)
     return;

  sethostent6 (1);
  if (!host6file)
     return;

  been_here = TRUE;

  while (1)
  {
    struct  hostent  *h = gethostent6();
    struct _hostent6 *h2;
    int     i;

    if (!h)
       break;

    h2 = calloc (sizeof(*h2), 1);
    if (!h2)
    {
      (*_printf) (_LANG("%s too big!\n"), host6Fname);
      break;
    }
    for (i = 0; h->h_aliases[i]; i++)
        h2->h_aliases[i] = strdup (h->h_aliases[i]);
    memcpy (&h2->h_address[0], h->h_addr_list[0], sizeof(h2->h_address[0]));
    h2->h_num_addr = 1;
    h2->h_name     = strdup (h->h_name);
    h2->h_timeout  = 0;      /* don't expire */
    h2->h_real_ttl = 0;
    if (!h2->h_name)
       break;

    h2->h_next = host0;
    host0      = h2;
  }

#if 0  /* test !! */
  {
    const struct _hostent6 *h;
    int   i;

    printf ("\n%s entries:\n", host6Fname);
    for (h = host0; h; h = h->h_next)
    {
      printf ("address %-40.40s  name %-30.30s  Aliases:",
              _inet6_ntoa(&h->h_address[0]), h->h_name);
      for (i = 0; h->h_aliases[i]; i++)
          printf (" %s,", h->h_aliases[i]);
      puts ("");
    }
    fflush (stdout);
  }
#endif

  rewind (host6file);
  RUNDOWN_ADD (_endhostent6, 256);
}

const char * W32_CALL GetHosts6File (void)
{
  return (host6Fname);
}

/*
 * To prevent running out of file-handles, one should close the
 * 'hosts' file before spawning a new shell.
 */
void W32_CALL CloseHost6File (void)
{
  if (!host6file)
     return;
  FCLOSE (host6file);
  host6file = NULL;
}

void W32_CALL ReopenHost6File (void)
{
  ReadHosts6File (host6Fname);
}

/*
 * Return the next (non-commented) line from the host6-file
 * Format is:
 *  IPv6-address [=] host-name [alias..] {\n | # ..}
 */
struct hostent * W32_CALL gethostent6 (void)
{
  struct _hostent6 h;
  char *tok, *ip, *name, *alias, *tok_buf = NULL;
  char  buf [2*MAX_HOSTLEN];
  int   i;

  if (!netdb_init() || !host6file)
  {
    h_errno = NO_RECOVERY;
    return (NULL);
  }

  memset (&h, 0, sizeof(h));

  while (1)
  {
    if (!fgets(buf,sizeof(buf),host6file))
       return (NULL);

    tok = strltrim (buf);
    if (*tok == '#' || *tok == ';' || *tok == '\n')
       continue;

    ip   = strtok_r (tok, " \t", &tok_buf);
    name = strtok_r (NULL, " \t\n", &tok_buf);
    if (ip && name && inet_pton(AF_INET6, ip, &h.h_address[0]) == 1)
       break;
  }

  if (hostClose)
     endhostent6();

  h.h_num_addr = 1;
  h.h_name = name;
  alias    = strtok_r (NULL, " \t\n", &tok_buf);

  for (i = 0; alias && i < MAX_HOST_ALIASES; i++)
  {
    static char aliases [MAX_HOST_ALIASES][MAX_HOSTLEN];

    if (*alias == '#' || *alias == ';')
       break;

    h.h_aliases[i] = _strlcpy (aliases[i], alias, sizeof(aliases[i]));
    alias = strtok_r (NULL, " \t\n", &tok_buf);
  }
  return fill_hostent6 (&h);
}

/*------------------------------------------------------------------*/

struct hostent * W32_CALL gethostbyname6 (const char *name)
{
  struct _hostent6 h;
  const char *alias;

  SOCK_DEBUGF (("\ngethostbyname6: `%s'", name));
  is_addr = FALSE;

  if (gethostbyname6_internal(name, &alias, &h))
  {
#if defined(USE_DEBUG)
    int i;
    for (i = 0; i < h.h_num_addr; i++)
        SOCK_DEBUGF ((" %s,", _inet6_ntoa(&h.h_address[i])));

    if (!did_lookup)
       SOCK_DEBUGF ((" %s", is_addr     ? "" :
                            h.h_timeout ? "cached" :
                            "hosts6-file"));
    if (alias)
       SOCK_DEBUGF ((" (alias %s)", alias));
#endif
    return fill_hostent6 (&h);
  }

  SOCK_DEBUGF ((", failed (%s)", did_lookup ? dom_strerror(dom_errno) :
                hstrerror(h_errno)));
  return (NULL);
}

static BOOL gethostbyname6_internal (const char *name, const char **alias,
                                     struct _hostent6 *ret)
{
  static char our_name[MAX_HOSTLEN];
  struct in6_addr  addr;
  struct _hostent6 *h;
  time_t now;
  int    rc;

  h_errno = HOST_NOT_FOUND;
  did_lookup = FALSE;

  _resolve_exit = _resolve_timeout = 0;
  memset (ret, 0, sizeof(*ret));
  *alias = NULL;

  if (!netdb_init() || !name)
  {
    h_errno = NO_RECOVERY;
    return (FALSE);
  }

  if (inet_pton(AF_INET6,name,&addr) == 1)
  {
    ret->h_name = (char*) name;
    memcpy (&ret->h_address[0], &addr, sizeof(ret->h_address[0]));
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

      if (IN6_ARE_ADDR_EQUAL(&h->h_address[0], &in6addr_all_1))
         return (FALSE);

      *ret = *h;
      return (TRUE);
    }
    for (i = 0; i < MAX_HOST_ALIASES && h->h_aliases[i]; i++)
        if (!stricmp(name,h->h_aliases[i]))
        {
          if (h->h_timeout && now > h->h_timeout)
             goto expired;

          if (IN6_ARE_ADDR_EQUAL(&h->h_address[0], &in6addr_all_1))
             return (FALSE);
          *alias = h->h_aliases[i];
          *ret  = *h;
          return (TRUE);
        }
  }

  /* Not found in linked list (hosts6 file or cache). Check name
   * against our own host-name (short-name or FQDN)
   */
  if (hostname[0] && !stricmp(name,hostname))
  {
    memcpy (&ret->h_address[0], &in6addr_my_ip, sizeof(ret->h_address[0]));
    ret->h_num_addr = 1;
    ret->h_name     = hostname;
    return (TRUE);
  }

  if (!gethostname(our_name,sizeof(our_name)) && !stricmp(name,our_name))
  {
    memcpy (&ret->h_address[0], &in6addr_my_ip, sizeof(ret->h_address[0]));
    ret->h_num_addr = 1;
    ret->h_name     = our_name;
    return (TRUE);
  }

expired:

  /* do a full DNS lookup
   */
  rc = resolve_ip6 (name, &addr);
  did_lookup = TRUE;

  if (_resolve_exit ||   /* interrupted by _resolve_hook() */
      _resolve_timeout)  /* timed out resolving */
     return (FALSE);

  if (rc)                /* successfully resolved */
  {
    h = add_hostent6 (h, name, dom_cname, &dom_a6list, &addr, dom_ttl);
    return (h ? *ret = *h, TRUE : FALSE);
  }

  /* Add the IP to the list even if DNS failed (but not interrupted by
   * _resolve_hook() or timedout). Thus the next call to gethostbyxxx6()
   * will return immediately.
   */
  add_hostent6 (h, name, NULL, NULL, &in6addr_all_1, netdbCacheLife);
  return (FALSE);
}

/*------------------------------------------------------------------*/

void W32_CALL sethostent6 (int stayopen)
{
  hostClose = (stayopen == 0);
  if (!netdb_init() || !host6Fname)
     return;

  if (!host6file)
       FOPEN_TXT (host6file, host6Fname);
  else rewind (host6file);
}

/*------------------------------------------------------------------*/

void W32_CALL endhostent6 (void)
{
  struct _hostent6 *h, *next;

  if (_watt_fatal_error)
     return;

  if (host6Fname)
     free (host6Fname);
  host6Fname = NULL;
  CloseHost6File();

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

/*
 * Return a 'struct hostent *' for an IPv6 address.
 */
struct hostent * W32_CALL gethostbyaddr6 (const void *addr)
{
  struct _hostent6 h;

  SOCK_DEBUGF (("\ngethostbyaddr6: %s", _inet6_ntoa(addr)));

  if (gethostbyaddr6_internal (addr, &h))
  {
    SOCK_DEBUGF ((" `%s'", h.h_name));
    if (!did_lookup)
       SOCK_DEBUGF ((", %s", h.h_timeout ? "cached" : "hosts-file"));
    return fill_hostent6 (&h);
  }

  SOCK_DEBUGF ((", failed (%s) ", did_lookup ? dom_strerror(dom_errno) :
                hstrerror(h_errno)));
  return (NULL);
}

static BOOL gethostbyaddr6_internal (const char *addr, struct _hostent6 *ret)
{
  static char name [MAX_HOSTLEN];
  struct _hostent6 *h;
  ip6_address ip;
  time_t      now;
  int         rc;

  h_errno = HOST_NOT_FOUND;
  did_lookup = FALSE;
  _resolve_exit = _resolve_timeout = 0;
  memset (ret, 0, sizeof(*ret));
  memcpy (&ip, addr, sizeof(ip));

  if (!netdb_init())
  {
    h_errno = NO_RECOVERY;
    return (FALSE);
  }

  if ((IN6_ARE_ADDR_EQUAL(&ip, &in6addr_any) ||  /* :: -> in6addr_myip */
       IN6_IS_ADDR_LINKLOCAL(&ip)) &&            /* FE8/119 */
      gethostname(name,sizeof(name)) == 0)
  {
    memcpy (&ret->h_address[0], &in6addr_my_ip, sizeof(ret->h_address[0]));
    ret->h_num_addr = 1;
    ret->h_name     = name;
    return (TRUE);
  }

  now = time (NULL);

  for (h = host0; h; h = h->h_next)
      /** \todo should check all addresses
       */
      if (IN6_ARE_ADDR_EQUAL(&h->h_address[0],&ip))
      {
        /* if cached entry expired, do a new reverse lookup
         */
        if (h->h_timeout && now > h->h_timeout)
           break;

        *ret = *h;
        return (TRUE);
      }

  /* do a reverse IPv6 lookup
   */
  did_lookup = TRUE;
  rc = reverse_resolve_ip6 (&ip, name, sizeof(name));

  /* interrupted or timedout
   */
  if (!rc && (_resolve_exit || _resolve_timeout))
     return (FALSE);

  if (rc)        /* successfully resolved */
  {
    h = add_hostent6 (h, name, NULL, NULL, &ip, dom_ttl);
    /** \todo should be the new aliases */
    return (h ? *ret = *h, TRUE : FALSE);
  }

  /* Add the IP to the list even if reverse lookup failed and not
   * interrupted by _resolve_hook(). Thus the next call to gethostbyxx()
   * will return immediately.
   */
  add_hostent6 (h, "*unknown*", NULL, NULL, &ip, 0UL);
  return (FALSE);
}

/*
 * Return a 'struct hostent*' from an internal 'struct _hostent6*'
 */
static struct hostent *fill_hostent6 (const struct _hostent6 *h)
{
  static struct hostent  ret;
  static struct in6_addr addr [MAX_ADDRESSES+1];
  static char  *list [MAX_ADDRESSES+1];
  static char   hostname [MAX_HOSTLEN];
  static char  *aliases [MAX_HOST_ALIASES+1];
  int    i;

  if (!h->h_name)
     return (NULL);

  memset (&addr, 0, sizeof(addr));
  memcpy (&aliases, h->h_aliases, sizeof(aliases));

  for (i = 0; i < h->h_num_addr && i < MAX_ADDRESSES; i++)
  {
    memcpy (&addr[i].s6_addr, &h->h_address[i], sizeof(addr[i].s6_addr));
    list[i] = (char*) &addr[i];
  }

  list[i]         = NULL;
  ret.h_addr_list = list;
  ret.h_name      = _strlcpy (hostname, h->h_name, sizeof(hostname));
  ret.h_aliases   = aliases;
  ret.h_addrtype  = AF_INET6;
  ret.h_length    = sizeof (addr[0].s6_addr);
  h_errno         = NETDB_SUCCESS;
  return (&ret);
}

/*
 * Modify an expired cached entry or add a new node to
 * the linked list. Not used for entries in hosts-file.
 */
static struct _hostent6 *add_hostent6 (
       struct _hostent6 *h,
       const char       *name,
       const char       *cname,  /* Canonical name (CNAME) */
       const void       *alist,  /* List of alternate addr */
       const void       *addr,   /* Main IP6-address */
       DWORD             ttl)    /* Time-to-live (sec) */
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
    memset (&h->h_address[1], 0,   /* clear old addresses */
            sizeof(h->h_address)-sizeof(ip6_address));
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

  if (addr != &in6addr_all_1)
     SOCK_DEBUGF ((", CNAME %s, ttl %lus,",
                   cname ? cname : "<none>", (long unsigned int)real_ttl));
  if (h)
  {
    const ip6_address *a6list = (const ip6_address*) alist;

    h->h_timeout  = ttl ? time (NULL) + ttl : 0;
    h->h_real_ttl = real_ttl;
    h->h_num_addr = 1;
    memcpy (&h->h_address[0], addr, sizeof(h->h_address[0]));
    for (i = 0; a6list && !IN6_IS_ADDR_UNSPECIFIED(a6list[i]); i++)
    {
      memcpy (&h->h_address[i+1], a6list[i], sizeof(ip6_address));
      h->h_num_addr++;
    }
    if (cname)        /* swap name with CNAME */
    {
      h->h_name = strdup (cname);
      h->h_aliases[0] = strdup (name);  /* !! only allow 1 alias */
    }
    else
      h->h_name = strdup (name);
  }
  else
    SOCK_DEBUGF ((" ENOMEM"));

  return (h);
}

#if defined(USE_DEBUG)
/*
 * Reverse the 'host0' list before printing it (a bit nicer and intuitive).
 * I.e. on same order the host-names where put in the list by add_hostent().
 */
static BOOL ReverseHosts6List (void)
{
  struct _hostent6 *h, **list;
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

void W32_CALL DumpHosts6Cache (void)
{
  struct _hostent6 *h;
  time_t now;

  if (!ReverseHosts6List())
     return;

  SOCK_DEBUGF ((" \n\nCached IPv6 hosts:                                   "
                "Address                                  TTL       Alias\n"));
  now = time (NULL);

  for (h = host0; h; h = h->h_next)
  {
    const char *addr;
    int   i;
    long  dT;

    if (h->h_real_ttl == 0) /* skip static records */
       continue;

    dT = (long)(h->h_timeout - now);
    if (IN6_ARE_ADDR_EQUAL(&h->h_address[0], &in6addr_all_1))
         addr = "::";
    else addr = _inet6_ntoa (&h->h_address[0]);

    SOCK_DEBUGF (("  %-50s %-40s %8s  ", h->h_name, addr,
                  dT < 0 ? "timedout" : hms_str(h->h_real_ttl)));

    for (i = 0; h->h_aliases[i]; i++)
        SOCK_DEBUGF (("%s ", h->h_aliases[i]));
    if (i == 0)
         SOCK_DEBUGF (("<none>\n"));
    else SOCK_DEBUGF ((" \n"));

    for (i = 1; i < h->h_num_addr; i++)
        SOCK_DEBUGF (("%62s %s\n", "", _inet6_ntoa(&h->h_address[i])));

  }
}
#endif  /* USE_DEBUG */


/*------------------------------------------------------------------*/

#ifdef TEST_PROG

#include "pcdbug.h"
#include "sock_ini.h"

/*
 * Print list of hosts unsorted.
 */
static void print_hosts (void)
{
  const struct _hostent6 *h;

  for (h = host0; h; h = h->h_next)
  {
    int i;
    printf ("%-40s -> %s; Aliases:", _inet6_ntoa(&h->h_address), h->h_name);

    for (i = 0; h->h_aliases[i]; i++)
        printf (" %s,", h->h_aliases[i]);
    puts ("");
  }
  fflush (stdout);
}

int main (void)
{
  dbug_init();
  sock_init();
  print_hosts();
  return (0);
}
#endif /* TEST_PROG */
#endif /* USE_BSD_API && USE_IPV6 */
/* \endif */

