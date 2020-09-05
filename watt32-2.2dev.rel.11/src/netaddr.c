/*!\file netaddr.c
 *
 *  Provide some more BSD address functions.
 *  Gisle Vanem, Oct 12, 1995
 *
 *  inet_network() is Copyright (c) 1983, 1993
 *  The Regents of the University of California.  All rights reserved.
 *
 *  ascii2address() and addr2ascii() are Copyright 1996
 *  Massachusetts Institute of Technology
 *
 *  \version v0.0
 *    \date Jan 11, 1991 \author E. Engelke
 *
 *  \version v0.2
 *    \date Apr 10, 1995 \author G. Vanem, function priv_addr()
 *
 *  \version v0.3
 *    \date Nov 12, 2003 \author G. Vanem, some functions moved from now
 *                       obsolete udp_nds.c.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "wattcp.h"
#include "misc.h"
#include "strings.h"
#include "netaddr.h"
#include "cpumodel.h"

#if defined(USE_BSD_API)
#include "socket.h"
#endif

/**
 * Convert an IP-address 'ip' into a string.
 *  \retval 'p'.
 *  \note 'ip' is on \b host order.
 * Use 4 buffers in round-robin.
 */
char * W32_CALL _inet_ntoa (char *s, DWORD ip)
{
  static char buf[4][20];
  static int idx = 0;
  char *p = s;

  if (!p)
  {
    p = buf [idx++];
    idx &= 3;
  }
  itoa ((int)(ip >> 24), p, 10);
  strcat (p, ".");
  itoa ((int)(ip >> 16) & 0xFF, strchr(p,'\0'), 10);
  strcat (p, ".");
  itoa ((int)(ip >> 8) & 0xFF, strchr(p,'\0'), 10);
  strcat (p, ".");
  itoa ((int)(ip & 0xFF), strchr(p,'\0'), 10);
  return (p);
}

/**
 * Convert a string into an IP-address.
 *  \retval address on \b host order.
 *  \retval 0 string isn't a (dotless) address.
 */
DWORD W32_CALL _inet_addr (const char *s)
{
  DWORD addr = 0;

  if (isaddr(s))
     return aton (s);
  if (isaddr_dotless(s,&addr))
     return (addr);
  return (0);
}


/**
 * Converts [a.b.c.d] or a.b.c.d to 32 bit IPv4 address.
 *  \retval 0 on error (safer than -1)
 *  \retval IPv4 address (host order)
 */
DWORD W32_CALL aton (const char *str)
{
  DWORD ip = 0;
  int   i;
  char *s = (char*)str;

  if (*s == '[')
     ++s;

  for (i = 24; i >= 0; i -= 8)
  {
    int cur = ATOI (s);

    ip |= (DWORD)(cur & 0xFF) << i;
    if (!i)
       return (ip);

    s = strchr (s, '.');
    if (!s)
       return (0);      /* return 0 on error */
    s++;
  }
  return (0);           /* cannot happen ??  */
}

/**
 * Converts `[dotless]' or `dotless' to 32 bit long (host order)
 *  \retval 0 on error (safer than -1)
 */
DWORD W32_CALL aton_dotless (const char *str)
{
  DWORD ip = 0UL;

  isaddr_dotless (str, &ip);
  return (ip);
}

/**
 * Check if 'str' is simply an ip address.
 * Accepts 'a.b.c.d' or '[a.b.c.d]' forms.
 *  \retval TRUE if 'str' is an IPv4 address.
 */
BOOL W32_CALL isaddr (const char *str)
{
  int ch;

  while ((ch = *str++) != 0)
  {
    if (isdigit(ch))
       continue;
    if (ch == '.' || ch == ' ' || ch == '[' || ch == ']')
       continue;
    return (FALSE);
  }
  return (TRUE);
}

/**
 * Check if 'str' is a dotless ip address.
 * Accept "[ 0-9\[\]]+". Doesn't accept octal base.
 *  \retval TRUE if 'str' is a dotless address.
 */
BOOL W32_CALL isaddr_dotless (const char *str, DWORD *ip)
{
  char  buf[10] = { 0 };
  int   ch, i = 0;
  DWORD addr;

  while ((ch = *str++) != '\0')
  {
    if (ch == '.' || i == SIZEOF(buf))
       return (FALSE);

    if (isdigit(ch))
    {
      buf[i++] = ch;
      continue;
    }
    if (ch == ' ' || ch == '[' || ch == ']')
       continue;
    return (FALSE);
  }

  buf[i] = '\0';
  addr = atol (buf);
  if (addr == 0)
     return (FALSE);

  if ((addr % 256) == 0)         /* LSB must be non-zero */
     return (FALSE);

  if (((addr >> 24) % 256) == 0) /* MSB must be non-zero */
     return (FALSE);

  if (ip)
     *ip = addr;
  return (TRUE);
}

char * W32_CALL inet_ntoa (struct in_addr addr)
{
  static char buf [4][20];   /* use max 4 at a time */
  static int  idx = 0;
  char       *rc  = buf [idx++];

  idx &= 3;
  return _inet_ntoa (rc, ntohl(addr.s_addr));
}

/**
 * Parse string "ether-addr, ip-addr".
 *
 * DOSX == 0: Assumes 'src' contain only legal hex-codes
 *   and ':' separators (or '-' too for DOSX). Doesn't waste
 *   precious memory using sscanf() if DOSX==0.
 *
 * Read 'src', dump to ethernet buffer ('*p_eth')
 * and return pointer to "ip-addr".
 */
const char *_inet_atoeth (const char *src, eth_address *p_eth)
{
  BYTE *eth = (BYTE*)p_eth;

#if (DOSX)
  int  tmp [sizeof(eth_address)];
  BOOL ok, colon = (src[2] == ':');

  ok = colon ? (sscanf(src,"%02x:%02x:%02x:%02x:%02x:%02x",
                       &tmp[0], &tmp[1], &tmp[2],
                       &tmp[3], &tmp[4], &tmp[5]) == DIM(tmp)) :
               (sscanf(src,"%02x-%02x-%02x-%02x-%02x-%02x",
                       &tmp[0], &tmp[1], &tmp[2],
                       &tmp[3], &tmp[4], &tmp[5]) == DIM(tmp));
  if (!ok)
     return (NULL);

  eth [0] = tmp [0];
  eth [1] = tmp [1];
  eth [2] = tmp [2];
  eth [3] = tmp [3];
  eth [4] = tmp [4];
  eth [5] = tmp [5];
  src = strrchr(src, colon ? ':' : '-') + 3;
#else
  eth [0] = atox (src-2);      /*   xx:xx:xx:xx:xx:xx */
  eth [1] = atox (src+1);      /* ^src-2              */
  eth [2] = atox (src+4);      /*    ^src+1           */
  eth [3] = atox (src+7);
  eth [4] = atox (src+10);
  eth [5] = atox (src+13);

  src += strlen ("xx:xx:xx:xx:xx:xx");
#endif

  if (*src == ',')
     ++src;
  return strltrim (src);
}

#if defined(USE_BSD_API)

char * W32_CALL inet_ntoa_r (struct in_addr addr, char *buf, int buflen)
{
  if (buf && buflen >= SIZEOF("255.255.255.255"))
     return _inet_ntoa (buf, ntohl(addr.s_addr));
  return (NULL);
}

int W32_CALL inet_aton (const char *name, struct in_addr *addr)
{
  u_long ip = inet_addr (name);

  if (ip == INADDR_NONE && strcmp(name,"255.255.255.255"))
     return (0);
  addr->s_addr = ip;
  return (1);
}

u_long W32_CALL inet_addr (const char *addr)
{
  DWORD ip = htonl (_inet_addr(addr));

  if (ip)
     return (ip);
  return (INADDR_NONE);
}

u_long W32_CALL inet_network (const char *name)
{
  u_long  val, base;
  u_long  parts[4];
  u_long *pp = parts;
  int     i, c, n;

again:
  /* Collect number up to ``.''. Values are specified as for C:
   * 0x=hex, 0=octal, other=decimal.
   */
  val  = 0;
  base = 10;

  /* The 4.4BSD version of this file also accepts 'x__' as a hexa
   * number.  I don't think this is correct.  -- Uli
   */
  if (*name == '0')
  {
    if (*++name == 'x' || *name == 'X')
         base = 16, name++;
    else base = 8;
  }
  while ((c = *name) != '\0')
  {
    if (isdigit(c))
    {
      val = (val * base) + (c - '0');
      name++;
      continue;
    }
    if (base == 16 && isxdigit(c))
    {
      val = (val << 4) + (c + 10 - (islower(c) ? 'a' : 'A'));
      name++;
      continue;
    }
    break;
  }
  if (*name == '.')
  {
    if (pp >= parts + 4)
       return (INADDR_NONE);

    *pp++ = val;
    name++;
    goto again;
  }
  if (*name && !isspace((int)*name))
     return (INADDR_NONE);

  *pp++ = val;
  n = (int) (pp - parts);
  if (n > 4)
     return (INADDR_NONE);

  for (val = 0, i = 0; i < n; i++)
  {
    val <<= 8;
    val |= parts[i] & 0xff;
  }
  return (val);
}

/**
 * Return the network number from an internet
 * address; handles class A/B/C network #'s.
 */
u_long W32_CALL inet_netof (struct in_addr addr)
{
  u_long a = ntohl (addr.s_addr);

  if (IN_CLASSA(a))
     return ((a & IN_CLASSA_NET) >> IN_CLASSA_NSHIFT);

  if (IN_CLASSB(a))
     return ((a & IN_CLASSB_NET) >> IN_CLASSB_NSHIFT);

  return ((a & IN_CLASSC_NET) >> IN_CLASSC_NSHIFT);
}

/**
 * Return the local network address portion of an
 * internet address; handles class A/B/C network
 * number formats only.
 * \note return value is host-order.
 */
u_long W32_CALL inet_lnaof (struct in_addr addr)
{
  u_long a = ntohl (addr.s_addr);

  if (IN_CLASSA(a))
     return (a & IN_CLASSA_HOST);

  if (IN_CLASSB(a))
     return (a & IN_CLASSB_HOST);

  return (a & IN_CLASSC_HOST);
}

/**
 * Formulate an Internet address from network + host (with subnet address).
 *  \note 'net' is on host-order.
 */
struct in_addr W32_CALL inet_makeaddr (u_long net, u_long host)
{
  u_long addr;

  if (net < 128)
       addr = (net << IN_CLASSA_NSHIFT) | (host & IN_CLASSA_HOST);

  else if (net < 65536)
       addr = (net << IN_CLASSB_NSHIFT) | (host & IN_CLASSB_HOST);

  else if (net < 16777216L)
       addr = (net << IN_CLASSC_NSHIFT) | (host & IN_CLASSC_HOST);

  else addr = (net | host);

  addr = htonl (addr);
  return (*(struct in_addr*) &addr);
}

/**
 * Convert an IPv6-address 'ip' into a string.
 *  \retval  static string.
 *  \warning Not reentrant.
 */
const char *_inet6_ntoa (const void *ip)
{
  static char buf [4][50];   /* use max 4 at a time */
  static int  idx = 0;
  char  *rc = buf [idx++];

  /* inet_ntop() should never return NULL.
   */
  idx &= 3;
  rc = (char*) inet_ntop (AF_INET6, ip, rc, sizeof(buf[0]));
  return (const char*)rc;
}

/**
 * Convert a string 'str' into an IPv6-address.
 *  \retval static buffer for address.
 *  \retval NULL 'str' isn't a valid IPv6 address.
 */
const ip6_address *_inet6_addr (const char *str)
{
  static ip6_address ip6;

  if (!inet_pton(AF_INET6, str, &ip6))
     return (NULL);
  return (const ip6_address*)ip6;
}

/*
 * Convert ASCII to network addresses.
 */
int W32_CALL ascii2addr (int af, const char *ascii, void *result)
{
  struct in_addr *ina;
  char   strbuf [4*sizeof("123")];  /* long enough for V4 only */

  switch (af)
  {
    case AF_INET:
         ina = (struct in_addr*) result;
         strbuf[0] = '\0';
         strncat (strbuf, ascii, sizeof(strbuf)-1);
         if (inet_aton (strbuf, ina))
            return sizeof (*ina);
         SOCK_ERRNO (EINVAL);
         break;

    case AF_LINK:
         link_addr (ascii, (struct sockaddr_dl*)result);
         /* oops... no way to detect failure */
         return sizeof (struct sockaddr_dl);

#if defined(USE_IPV6)
    case AF_INET6:
         return inet_pton (AF_INET6, ascii, result);
#endif

    default:
         SOCK_ERRNO (EPROTONOSUPPORT);
         break;
  }
  return (-1);
}

/*-
 * Convert a network address from binary to printable numeric format.
 * This API is copied from INRIA's IPv6 implementation, but it is a
 * bit bogus in two ways:
 *
 *      1) There is no value in passing both an address family and
 *         an address length; either one should imply the other,
 *         or we should be passing sockaddrs instead.
 *      2) There should by contrast be /added/ a length for the buffer
 *         that we pass in, so that programmers are spared the need to
 *         manually calculate (read: ``guess'') the maximum length.
 *
 * Flash: the API is also the same in the NRL implementation, and seems to
 * be some sort of standard, so we appear to be stuck with both the bad
 * naming and the poor choice of arguments.
 */
char * W32_CALL addr2ascii (int af, const void *addrp, int len, char *buf)
{
  static char staticbuf[64];    /* 64 for AF_LINK > 16 for AF_INET */

  if (!buf)
     buf = staticbuf;

  switch (af)
  {
    case AF_INET:
         if (len != sizeof (struct in_addr))
         {
           SOCK_ERRNO (ENAMETOOLONG);
           return (NULL);
         }
         strcpy (buf, inet_ntoa (*(const struct in_addr *) addrp));
         break;

    case AF_LINK:
         if (len != sizeof (struct sockaddr_dl))
         {
           SOCK_ERRNO (ENAMETOOLONG);
           return (NULL);
         }
         strcpy (buf, link_ntoa ((const struct sockaddr_dl *) addrp));
         break;

    default:
         SOCK_ERRNO (EPROTONOSUPPORT);
         return (NULL);
  }
  return (buf);
}
#endif  /* USE_BSD_API */


BOOL W32_CALL priv_addr (DWORD ip)
{
  /**
   * \brief Check if 'ip' is a private addresses in the
   *        "black-hole range" (ref. RFC-1918):
   *
   *   10.0.0.0    - 10.255.255.255
   *   172.16.0.0  - 172.31.255.255
   *   192.168.0.0 - 192.168.255.255
   *
   *  \todo: rewrite using mask_len().
   */
  if (ip >= aton("10.0.0.0") && ip <= aton("10.255.255.255"))
     return (TRUE);

  if (ip >= aton("172.16.0.0") && ip <= aton("172.31.255.255"))
     return (TRUE);

  if (ip >= aton("192.168.0.0") && ip <= aton("192.168.255.255"))
     return (TRUE);

  return (FALSE);
}

/*
 * Lev Walkin <vlm@lionet.info> provided these handy mask-checking
 * macros/routines.
 *
 * Masklen to mask convertor.
 */
#define MLEN2MASK(mlen) ((mlen) ? (((DWORD)-1) << (32 - (mlen))) : 0)

/*
 * Get the number of 0-bits of a mask.
 * E.g. 255.255.255.0 -> 8
 */
int mask_len (DWORD mask)
{
  int len = 0;

  mask = ~mask;
  while (mask & 1)
  {
    len++;
    mask >>= 1;
  }
  return (len);
}

#if defined(HAVE_POPCOUNT)
  #if defined(__GNUC__)
    /*
     * gcc 4.4.1 (?) have the '__builtin_popcount' instruction.
     */
    #define POPCOUNT(x)  __builtin_popcount (x)
  #elif defined(_MSC_VER)
    /*
     * MSVC >= 1500 have the '_mm_popcnt_u32' inline instruction.
     */
    #define POPCOUNT(x)  _mm_popcnt_u32 (x)
  #else
    #error Help!!
  #endif
#endif

/*
 * Determine number of bits set in an IPv4 mask.
 */
static DWORD bit_set (DWORD v)
{
  DWORD m;
#if defined(HAVE_POPCOUNT)
  static int have_popcnt = -1;

  if (have_popcnt == -1)
     have_popcnt = (x86_have_cpuid && (x86_capability & X86_CAPA_MMX));
  if (have_popcnt)
     return  POPCOUNT(v);

  /* Fall through */
#endif

  m = v;
  m = (m & 0x55555555) + ((m & 0xAAAAAAAA) >> 1);
  m = (m & 0x33333333) + ((m & 0xCCCCCCCC) >> 2);
  m = (m & 0x0F0F0F0F) + ((m & 0xF0F0F0F0) >> 4);
  m = (m & 0x00FF00FF) + ((m & 0xFF00FF00) >> 8);
  m = (m & 0x0000FFFF) + ((m & 0xFFFF0000) >> 16);
  return (m);
}

/*
 * Check the mask for correctness.
 */
int check_mask (DWORD mask)
{
  DWORD v = bit_set (mask);
  return (MLEN2MASK(v) == mask);
}

#if defined(USE_BSD_API)
/*
 * The string-based version of above function.
 * Inside 'USE_BSD_API' since inet_aton() is needed.
 */
int check_mask2 (const char *mask)
{
  struct in_addr imask;

  if (inet_aton(mask,&imask) != 1)
     return (-1);
  return check_mask (ntohl(imask.s_addr));
}
#endif  /* USE_BSD_API */


