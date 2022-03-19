/*!\file presaddr.c
 * Convert network addresses to printable format.
 */

/* Copyright (c) 1996 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#include "socket.h"

#if defined(USE_BSD_API)

static const char *inet_ntop4 (const u_char *src, char *dst, size_t size);
static int         inet_pton4 (const char *src, u_char *dst);

#if defined(USE_IPV6)
static const char *inet_ntop6 (const u_char *src, char *dst, size_t size);
static int         inet_pton6 (const char *src, u_char *dst);
#endif

/**
 * Convert a network format address to presentation format.
 *
 * \retval pointer to presentation format address (`dst'),
 * \retval NULL on error (see errno).
 * \author Paul Vixie, 1996.
 */
const char * W32_CALL inet_ntop (int af, const void *src, char *dst, size_t size)
{
  switch (af)
  {
    case AF_INET:
         return inet_ntop4 ((const u_char*)src, dst, size);
#if defined(USE_IPV6)
    case AF_INET6:
         return inet_ntop6 ((const u_char*)src, dst, size);
#endif
    default:
         SOCK_ERRNO (EAFNOSUPPORT);
         return (NULL);
  }
}

const char * W32_CALL _w32_inet_ntop (int af, const void *src, char *dst, size_t size)
{
  return inet_ntop (af, src, dst, size);
}

/**
 * Convert from presentation format (which usually means ASCII printable)
 * to network format (which is usually some kind of binary format).
 *
 * \retval 1  the address was valid for the specified address family.
 * \retval 0  the address wasn't valid (`dst' is untouched in this case).
 * \retval -1 some other error occurred (`dst' is untouched in this case, too).
 * \author Paul Vixie, 1996.
 */
int W32_CALL inet_pton (int af, const char *src, void *dst)
{
  switch (af)
  {
    case AF_INET:
         return inet_pton4 (src, (u_char*)dst);
#if defined(USE_IPV6)
    case AF_INET6:
         return inet_pton6 (src, (u_char*)dst);
#endif
    default:
         SOCK_ERRNO (EAFNOSUPPORT);
         return (-1);
  }
}

int W32_CALL _w32_inet_pton (int af, const char *src, void *dst)
{
  return inet_pton (af, src, dst);
}

/**
 * Format an IPv4 address, more or less like inet_ntoa().
 *
 * \retval `dst' (as a const)
 * \note
 *  - uses no statics
 *  - takes a u_char* not an in_addr as input
 * \author Paul Vixie, 1996.
 */
static const char *inet_ntop4 (const u_char *src, char *dst, size_t size)
{
  char tmp [sizeof("255.255.255.255")];

  if ((size_t)sprintf(tmp,"%u.%u.%u.%u",src[0],src[1],src[2],src[3]) > size)
  {
    SOCK_ERRNO (ENOSPC);
    return (NULL);
  }
  return strcpy (dst, tmp);
}

#if defined(USE_IPV6)
/**
 * Convert IPv6 binary address into presentation (printable) format
 * \author
 *  Paul Vixie, 1996.
 */
static const char *inet_ntop6 (const u_char *src, char *dst, size_t size)
{
  /*
   * Note that int32_t and int16_t need only be "at least" large enough
   * to contain a value of the specified size.  On some systems, like
   * Crays, there is no such thing as an integer variable with 16 bits.
   * Keep this in mind if you think this function should have been coded
   * to use pointer overlays.  All the world's not a VAX.
   */
  char  tmp [sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
  char *tp;
  int   i;
  struct {
    long base;
    long len;
  } best, cur;
  u_long words [IN6ADDRSZ / INT16SZ];

  /* Preprocess:
   *  Copy the input (bytewise) array into a wordwise array.
   *  Find the longest run of 0x00's in src[] for :: shorthanding.
   */
  memset (words, '\0', sizeof(words));
  for (i = 0; i < IN6ADDRSZ; i++)
      words[i/2] |= (src[i] << ((1 - (i % 2)) << 3));

  best.base = -1;
  cur.base  = -1;
  best.len  = 0;
  cur.len   = 0;
  for (i = 0; i < (IN6ADDRSZ / INT16SZ); i++)
  {
    if (words[i] == 0)
    {
      if (cur.base == -1)
           cur.base = i, cur.len = 1;
      else cur.len++;
    }
    else if (cur.base != -1)
    {
      if (best.base == -1 || cur.len > best.len)
         best = cur;
      cur.base = -1;
    }
  }

  if ((cur.base != -1) && (best.base == -1 || cur.len > best.len))
     best = cur;

  if (best.base != -1 && best.len < 2)
     best.base = -1;

  /* Format the result.
   */
  tp = tmp;
  for (i = 0; i < (IN6ADDRSZ / INT16SZ); i++)
  {
    /* Are we inside the best run of 0x00's?
     */
    if (best.base != -1 && i >= best.base && i < (best.base + best.len))
    {
      if (i == best.base)
         *tp++ = ':';
      continue;
    }

    /* Are we following an initial run of 0x00s or any real hex?
     */
    if (i != 0)
       *tp++ = ':';

    /* Is this address an encapsulated IPv4?
     */
    if (i == 6 && best.base == 0 &&
        (best.len == 6 || (best.len == 5 && words[5] == 0xffff)))
    {
      if (!inet_ntop4(src+12, tp, sizeof(tmp) - (tp - tmp)))
      {
        SOCK_ERRNO (ENOSPC);
        return (NULL);
      }
      tp += strlen (tp);
      break;
    }
    tp += sprintf (tp, "%lX", words[i]);
  }

  /* Was it a trailing run of 0x00's?
   */
  if (best.base != -1 && (best.base + best.len) == (IN6ADDRSZ / INT16SZ))
     *tp++ = ':';
  *tp++ = '\0';

  /* Check for overflow, copy, and we're done.
   */
  if ((size_t)(tp - tmp) > size)
  {
    SOCK_ERRNO (ENOSPC);
    return (NULL);
  }
  return strcpy (dst, tmp);
}
#endif      /* USE_IPV6 */

/**
 * Like inet_aton() but without all the hexadecimal and shorthand.
 * \retval 1 if `src' is a valid dotted quad
 * \retval 0 if `src' is not a valid dotted quad.
 * \note
 *   does not touch `dst' unless it's returning 1.
 * \author
 *   Paul Vixie, 1996.
 */
static int inet_pton4 (const char *src, u_char *dst)
{
  static const char digits[] = "0123456789";
  int    saw_digit, octets, ch;
  u_char tmp[INADDRSZ];
  u_char *tp;

  saw_digit = 0;
  octets = 0;
  *(tp = tmp) = '\0';
  while ((ch = *src++) != '\0')
  {
    const char *pch;

    if ((pch = strchr(digits, ch)) != NULL)
    {
      u_int New = *tp * 10 + (pch - digits);

      if (New > 255)
         return (0);
      *tp = New;
      if (! saw_digit)
      {
        if (++octets > 4)
           return (0);
        saw_digit = 1;
      }
    }
    else if (ch == '.' && saw_digit)
    {
      if (octets == 4)
         return (0);
      *++tp = '\0';
      saw_digit = 0;
    }
    else
      return (0);
  }
  if (octets < 4)
     return (0);

  memcpy (dst, tmp, INADDRSZ);
  return (1);
}

#if defined(USE_IPV6)
/**
 * Convert presentation level address to network order binary form.
 *
 * \retval 1 if `src' is a valid [RFC1884 2.2] address.
 * \retval 0 otherwise.
 * \note
 *   - does not touch `dst' unless it's returning 1.
 *   - `::' in a full address is silently ignored.
 *
 * \author Paul Vixie, 1996.
 * \n\b credit: inspired by Mark Andrews.
 */
static int inet_pton6 (const char *src, u_char *dst)
{
  u_char  tmp [IN6ADDRSZ];
  u_char *endp, *colonp, *tp = tmp;
  const   char *curtok;
  int     ch, saw_xdigit;
  u_int   val;

  memset (tmp, 0, sizeof(tmp));
  endp   = tmp + sizeof(tmp);
  colonp = NULL;

  /* Leading :: requires some special handling.
   */
  if (*src == ':' && *++src != ':')
     return (0);

  curtok = src;
  saw_xdigit = 0;
  val = 0;

  while ((ch = *src++) != '\0')
  {
    const char *pch;

    ch = tolower (ch);
    pch = strchr (hex_chars_lower, ch);
    if (pch)
    {
      val <<= 4;
      val |= (pch - hex_chars_lower);
      if (val > 0xffff)
         return (0);
      saw_xdigit = 1;
      continue;
    }
    if (ch == ':')
    {
      curtok = src;
      if (!saw_xdigit)
      {
        if (colonp)
           return (0);
        colonp = tp;
        continue;
      }
      if (tp + INT16SZ > endp)
         return (0);

      *tp++ = (u_char) (val >> 8) & 0xff;
      *tp++ = (u_char) (val & 0xff);
      saw_xdigit = 0;
      val = 0;
      continue;
    }
    if (ch == '.' && ((tp + INADDRSZ) <= endp) && inet_pton4(curtok,tp) > 0)
    {
      tp += INADDRSZ;
      saw_xdigit = 0;
      break;  /* '\0' was seen by inet_pton4(). */
    }
    return (0);
  }
  if (saw_xdigit)
  {
    if (tp + INT16SZ > endp)
       return (0);
    *tp++ = (u_char) (val >> 8) & 0xff;
    *tp++ = (u_char) val & 0xff;
  }
  if (colonp)
  {
    /*
     * Since some memmove()'s erroneously fail to handle
     * overlapping regions, we'll do the shift by hand.
     */
    const int n = tp - colonp;
    int   i;

    for (i = 1; i <= n; i++)
    {
      endp[-i] = colonp[n-i];
      colonp[n-i] = '\0';
    }
    tp = endp;
  }
  if (tp != endp)
     return (0);

  memcpy (dst, tmp, IN6ADDRSZ);
  return (1);
}
#endif    /* USE_IPV6 */


#if defined(TEST_PROG)

int main (int argc, char **argv)
{
  const char *ip6_str;
  ip6_address ip6_addr;

  if (argc != 2)
  {
    printf ("Usage: %s ip6-address\n", argv[0]);
    return (-1);
  }

  ip6_str = argv[1];
  memset (&ip6_addr, 0, sizeof(ip6_addr));
  if (!inet_pton(AF_INET6, ip6_str, &ip6_addr))
  {
    printf ("Invalid IPv6 address `%s'\n", ip6_str);
    return (-1);
  }

  printf ("inet_pton(AF_INET6): %s -> %s\n",
          ip6_str, _inet6_ntoa(&ip6_addr));
  return (0);
}
#endif  /* TEST_PROG */
#endif  /* USE_BSD_API */


