/*!\file get_ip.c
 *
 * BSD getipnodebyX().
 */

/*
 * Copyright (C) 1999-2001  Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM
 * DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL
 * INTERNET SOFTWARE CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* $Id: getipnode.c,v 1.30 2001/07/18 02:37:08 mayer Exp $ */

/* Originally from ISC's BIND 9.21; lightweigth resolver library (lwres).
 * Rewritten an simplified for Watt-32 by <gvanem@yahoo.no>  Nov-2003.
 */

#include "socket.h"
#include "get_xby.h"
#include "pcdns.h"

#if defined(USE_BSD_API)

static struct hostent *copyandmerge (const struct hostent *he1,
                                     const struct hostent *he2,
                                     int af, int *err);

static __inline void scan_interface (BOOL *have_v4, BOOL *have_v6)
{
  if (have_v4)
     *have_v4 = TRUE;

  if (have_v6)
#if defined(USE_IPV6)
    *have_v6 = dns_do_ipv6;
#else
    *have_v6 = FALSE;
#endif
}

/*
 * AI_V4MAPPED + AF_INET6
 * If no IPv6 address then a query for IPv4 and map returned values.
 *
 * AI_ALL + AI_V4MAPPED + AF_INET6
 * Return IPv6 and IPv4 mapped.
 *
 * AI_ADDRCONFIG
 * Only return IPv6 / IPv4 address if there is an interface of that
 * type active.
 */
struct hostent * W32_CALL
getipnodebyname (const char *name, int af, int flags, int *error)
{
  struct hostent *he1 = NULL;
  struct hostent *he2 = NULL;
  struct in_addr  in4;
  struct in6_addr in6;
  BOOL   have_v4 = TRUE, have_v6 = TRUE;
  BOOL   v4 = FALSE, v6 = FALSE;
  int    tmp_err;

  SOCK_DEBUGF (("\ngetipnodebyname: %s ", name));

#if !defined(USE_IPV6)
  if (af == AF_INET6)
  {
    *error = HOST_NOT_FOUND;
    return (NULL);
  }
#endif

  /* If we care about active interfaces then check.
   */
  if (flags & AI_ADDRCONFIG)
     scan_interface (&have_v4, &have_v6);

  /* Check for literal address.
   */
  v4 = inet_pton (AF_INET, name, &in4);
  if (!v4)
     v6 = inet_pton (AF_INET6, name, &in6);

  /* Impossible combination?
   */
  if ((af == AF_INET6 && !(flags & AI_V4MAPPED) && v4) ||
      (af == AF_INET && v6) ||
      (!have_v4 && v4) ||
      (!have_v6 && v6) ||
      (!have_v4 && af == AF_INET) ||
      ((!have_v6 && af == AF_INET6) && ((flags & AI_V4MAPPED) && have_v4)) ||
      !(flags & AI_V4MAPPED))
  {
    *error = HOST_NOT_FOUND;
    return (NULL);
  }

  /* Literal address?
   */
  if (v4 || v6)
  {
    struct hostent he;
    char  *addr_list[2];
    char  *aliases[1];

    he.h_name         = (char*) name;
    he.h_addr_list    = addr_list;
    he.h_addr_list[0] = (v4 ? (char*)&in4 : (char*)&in6);
    he.h_addr_list[1] = NULL;
    he.h_aliases      = aliases;
    he.h_aliases[0]   = NULL;
    he.h_length       = (v4 ? INADDRSZ : IN6ADDRSZ);
    he.h_addrtype     = (v4 ? AF_INET  : AF_INET6);
    return copyandmerge (&he, NULL, af, error);
  }

  tmp_err = NO_RECOVERY;
  if (have_v6 && af == AF_INET6)
  {
#if defined(USE_IPV6)
    he1 = gethostbyname6 (name);
#else
    he1 = NULL;
#endif
    if (!he1)
       tmp_err = HOST_NOT_FOUND;
  }

  if (have_v4 &&
      (af == AF_INET ||
       (af == AF_INET6 &&
        (flags & AI_V4MAPPED) && (!he1 || (flags & AI_ALL)))))
  {
    SOCK_ENTER_SCOPE();
    he2 = gethostbyname (name);
    SOCK_LEAVE_SCOPE();
    if (!he2 || !he1)
    {
      *error = HOST_NOT_FOUND;
      return (NULL);
    }
  }
  else
    *error = tmp_err;

  return copyandmerge (he1, he2, af, error);
}

struct hostent * W32_CALL
getipnodebyaddr (const void *src, size_t len, int af, int *error)
{
  struct hostent *he1, *he2 = NULL;
  const  BYTE *cp = (const BYTE*) src;

  SOCK_DEBUGF (("\ngetipnodebyaddr: "));

  if (!src)
  {
    *error = NO_RECOVERY;
    return (NULL);
  }

  switch (af)
  {
    case AF_INET:
         if (len < INADDRSZ)
         {
           *error = NO_RECOVERY;
           return (NULL);
         }
         break;
#if defined(USE_IPV6)
    case AF_INET6:
         if (len < IN6ADDRSZ)
         {
           *error = NO_RECOVERY;
           return (NULL);
         }
         break;
#endif
    default:
         *error = NO_RECOVERY;
         return (NULL);
  }

  /* Look up IPv4 and IPv4 mapped/compatible addresses.
   */
  if ((af == AF_INET6 && IN6_IS_ADDR_V4COMPAT(cp)) ||
      (af == AF_INET6 && IN6_IS_ADDR_V4MAPPED(cp)) ||
      (af == AF_INET))
  {
    if (af == AF_INET6)
       cp += 12;

    SOCK_ENTER_SCOPE();
    he1 = gethostbyaddr ((const char*)cp, 4, AF_INET);
    SOCK_LEAVE_SCOPE();

    if (af == AF_INET)
       goto ret_copy;

#if defined(USE_IPV6)
    /* Convert from AF_INET to AF_INET6.
     */
    he2 = copyandmerge (he1, NULL, af, error);
    if (he2)
    {
      memcpy (he2->h_addr, src, len);  /* Restore original address */
      SOCK_DEBUGF (("%s", af == AF_INET ?
                    inet_ntoa(*(struct in_addr*)&he2->h_addr) :
                    _inet6_ntoa(he2->h_addr)));
    }
#endif
    return (he2);
  }

  he1 = gethostbyaddr (src, len, AF_INET6);   /* Lookup IPv6 address */

ret_copy:
  if (!he1)
  {
    *error = HOST_NOT_FOUND;
    return (NULL);
  }
  return copyandmerge (he1, NULL, af, error);
}

void W32_CALL freehostent (struct hostent *he)
{
  char *p;

  SOCK_DEBUGF (("\nfreehostent: %s ", he->h_name));

  if (!he->h_name)   /* possible double freeing */
     return;

  free (he->h_name);
  he->h_name = NULL;

  for (p = he->h_addr_list[0]; p; p++)
      free (p);

  for (p = he->h_aliases[0]; p; p++)
      free (p);

  free (he->h_aliases);
  free (he->h_addr_list);
  free (he);
}

static struct hostent *copyandmerge (const struct hostent *he1,
                                     const struct hostent *he2,
                                     int af, int *error)
{
  struct hostent *he = NULL;
  int    addresses = 1;      /* NULL terminator */
  int    names = 1;          /* NULL terminator */
  char **cpp, **npp;

#if !defined(USE_IPV6)
  BYTE in6addr_mapped[12];

  WATT_ASSERT (af != AF_INET6);
#endif

  /* Work out array sizes.
   */
  if (he1)
  {
    cpp = he1->h_addr_list;
    while (*cpp)
    {
      addresses++;
      cpp++;
    }
    cpp = he1->h_aliases;
    while (*cpp)
    {
      names++;
      cpp++;
    }
  }

  if (he2)
  {
    cpp = he2->h_addr_list;
    while (*cpp)
    {
      addresses++;
      cpp++;
    }
    if (!he1)
    {
      cpp = he2->h_aliases;
      while (*cpp)
      {
        names++;
        cpp++;
      }
    }
  }

  if (addresses == 1)
  {
    *error = NO_ADDRESS;
    return (NULL);
  }

  he = malloc (sizeof(*he));
  if (!he)
  {
    *error = NO_RECOVERY;
    return (NULL);
  }

  he->h_addr_list = calloc (addresses * sizeof(char*), 1);
  if (!he->h_addr_list)
     goto cleanup0;

  /* Copy addresses.
   */
  npp = he->h_addr_list;
  if (he1)
  {
    cpp = he1->h_addr_list;
    while (*cpp)
    {
      *npp = malloc (af == AF_INET ? INADDRSZ : IN6ADDRSZ);
      if (!*npp)
         goto cleanup1;

      /* Convert to mapped if required.
       */
      if (af == AF_INET6 && he1->h_addrtype == AF_INET)
      {
        memcpy (*npp, in6addr_mapped, sizeof(in6addr_mapped));
        memcpy (*npp + sizeof(in6addr_mapped), *cpp, INADDRSZ);
      }
      else
        memcpy (*npp, *cpp, af == AF_INET ? INADDRSZ : IN6ADDRSZ);
      cpp++;
      npp++;
    }
  }

  if (he2)
  {
    cpp = he2->h_addr_list;
    while (*cpp)
    {
      *npp = malloc (af == AF_INET ? INADDRSZ : IN6ADDRSZ);
      if (!*npp)
         goto cleanup1;

      /* Convert to mapped if required.
       */
      if (af == AF_INET6 && he2->h_addrtype == AF_INET)
      {
        memcpy (*npp, in6addr_mapped, sizeof(in6addr_mapped));
        memcpy (*npp + sizeof(in6addr_mapped), *cpp, INADDRSZ);
      }
      else
        memcpy (*npp, *cpp, af == AF_INET ? INADDRSZ : IN6ADDRSZ);
      cpp++;
      npp++;
    }
  }

  he->h_aliases = calloc (names * sizeof(char*), 1);
  if (!he->h_aliases)
     goto cleanup1;

  /* Copy aliases.
   */
  npp = he->h_aliases;
  cpp = (he1 ? he1->h_aliases : he2->h_aliases);
  while (*cpp)
  {
    *npp = strdup (*cpp);
    if (!*npp)
       goto cleanup2;
    npp++;
    cpp++;
  }

  /* Copy hostname.
   */
  if (he1)
       he->h_name = strdup (he1->h_name);
  else he->h_name = strdup (he2->h_name);

  if (!he->h_name)
     goto cleanup2;

  he->h_addrtype = af;
  he->h_length   = (af == AF_INET ? INADDRSZ : IN6ADDRSZ);
  return (he);

cleanup2:
  for (cpp = he->h_aliases; *cpp; *cpp++ = NULL)
      free (*cpp);
  free (he->h_aliases);

cleanup1:
  for (cpp = he->h_addr_list; *cpp; *cpp++ = NULL)
      free (*cpp);
  free (he->h_addr_list);

cleanup0:
  free (he);
  *error = NO_RECOVERY;
  return (NULL);
}
#endif     /* USE_BSD_API */
