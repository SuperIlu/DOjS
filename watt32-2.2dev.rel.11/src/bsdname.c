/*!\file bsdname.c
*
 * BSD and core-style name function.
 * \deprecated Oldstyle BSD similar functions are prefixed with '_'.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>

#include "copyrigh.h"
#include "wattcp.h"
#include "pcdns.h"
#include "pcbuf.h"
#include "pctcp.h"
#include "pcdbug.h"
#include "pcconfig.h"
#include "ip6_out.h"
#include "misc.h"
#include "wdpmi.h"
#include "x32vm.h"
#include "win_dll.h"
#include "powerpak.h"
#include "strings.h"
#include "bsdname.h"

#ifdef __CYGWIN__
  #define XHOSTID_RETV      long
  #define SETHOSTNAME_ARG2  size_t
#else
  #define XHOSTID_RETV      u_long
  #define SETHOSTNAME_ARG2  int
#endif

#ifdef __DJGPP__
  #define GETHOSTNAME_ARG2 int
#else
  #define GETHOSTNAME_ARG2 size_t
#endif



/**
 * Core style: Returns local IPv4-address.
 *  \retval address on \b host order.
 */
DWORD W32_CALL _gethostid (void)
{
  return (my_ip_addr);
}

/**
 * Core-style: Sets local IPv4-address.
 *  \retval address on \b host order.
 */
DWORD W32_CALL _sethostid (DWORD ip)
{
  return (my_ip_addr = ip);
}

#if defined(USE_BSD_API)
/**
 * BSD style: returns local IPv4-address.
 *  \retval  address on \b network order.
 */
XHOSTID_RETV W32_CALL gethostid (void)
{
  if (!netdb_init())
     return (INADDR_NONE);
  return htonl (my_ip_addr);
}

/**
 * BSD-style: Sets local IPv4-address.
 *  \retval address on \b host order.
 */
XHOSTID_RETV W32_CALL sethostid (u_long ip)
{
  return (my_ip_addr = ntohl(ip));
}
#endif  /* USE_BSD_API */

#if defined(USE_IPV6)
/**
 * Core-style: Returns local IPv6-address.
 * \note   no BSD equivalent for this.
 * \retval pointer to current local IPv6 address.
 */
const void *_gethostid6 (void)
{
  if (!netdb_init())
     return (&in6addr_any);
  return (&in6addr_my_ip);
}

/**
 * Core-style: Sets local IPv6-address.
 * \note   no BSD equivalent for this.
 */
void _sethostid6 (const void *addr)
{
  memcpy (&in6addr_my_ip, addr, sizeof(in6addr_my_ip));
}
#endif  /* USE_IPV6 */

/**
 * Core-style: \n
 * Return 'watt_sockaddr' for peer in a UDP/TCP connection.
 */
int W32_CALL _getpeername (const sock_type *s, void *dest, int *len)
{
  struct watt_sockaddr temp;
  size_t ltemp;
  int    chk = _chk_socket (s);

  if (!s->tcp.hisaddr || !s->tcp.hisport ||
      (chk != VALID_UDP && chk != VALID_TCP))
  {
    if (len)
       *len = 0;
    return (-1);
  }

  memset (&temp, 0, sizeof(temp));
  temp.s_ip   = s->tcp.hisaddr;
  temp.s_port = s->tcp.hisport;
  temp.s_type = s->tcp.ip_type;

  /* how much do we move?
   */
  ltemp = (len ? *(size_t*)len : sizeof(struct watt_sockaddr));
  if (ltemp > sizeof (struct watt_sockaddr))
      ltemp = sizeof (struct watt_sockaddr);
  memcpy (dest, &temp, ltemp);

  if (len)
     *len = (int)ltemp;
  return (0);
}

/**
 * Core-style: \n
 * Return 'watt_sockaddr' for our side of a UDP/TCP connection.
 */
int W32_CALL _getsockname (const sock_type *s, void *dest, int *len)
{
  struct watt_sockaddr temp;
  size_t ltemp;
  int    chk = _chk_socket (s);

  if (!s->tcp.myaddr || !s->tcp.myport ||
      (chk != VALID_UDP && chk != VALID_TCP))
  {
    if (len)
       *len = 0;
    return (-1);
  }

  memset (&temp, 0, sizeof(temp));
  temp.s_ip   = s->tcp.myaddr;
  temp.s_port = s->tcp.myport;
  temp.s_type = s->tcp.ip_type;

  /* how much do we move
   */
  ltemp = (len ? *(size_t*)len : sizeof (struct watt_sockaddr));
  if (ltemp > sizeof (struct watt_sockaddr))
      ltemp = sizeof (struct watt_sockaddr);
  memcpy (dest, &temp, ltemp);

  if (len)
     *len = (int)ltemp;
  return (0);
}

/**
 * BSD-style: \n
 * Return domain name of this host.
 * As per BSD spec, the resultant buffer ...
 * ... "will be null-terminated _unless_there_is_insufficient_space_";
 *
 * Set errno on failure and return -1.
 */
int W32_CALL getdomainname (char *buffer, size_t buflen)
{
  const char *my_domainname = def_domain ? def_domain : "";

  if (!buffer || buflen < strlen(my_domainname))
  {
    SOCK_ERRNO (EINVAL);
    return (-1);
  }
#if (DOSX)
  if (!valid_addr(buffer, buflen))
  {
    SOCK_ERRNO (EFAULT);
    return (-1);
  }
#endif

  strncpy (buffer, my_domainname, buflen);
  /* no terminating '\0' needs to be forced here per BSD spec */
  return (0);
}

/**
 * BSD-style: \n
 * Set the host's domain name.
 * Set errno on failure and return -1.
 */
int W32_CALL setdomainname (const char *name, size_t len)
{
  if (!name || len > sizeof(defaultdomain)-1)
  {
    SOCK_ERRNO (EINVAL);
    return (-1);
  }

#if (DOSX)
  if (!valid_addr(name, len))
  {
    SOCK_ERRNO (EFAULT);
    return (-1);
  }
#endif

  def_domain = _strlcpy (defaultdomain, name, len);
  return (0);
}

/*
 * BSD-style: \n
 * Make a FQDN from `hostname' & `def_domain'.
 * Set errno on failure and return -1.
 */
int W32_CALL gethostname (char *buffer, GETHOSTNAME_ARG2 buflen)
{
  /* the FQDN when no hostname has been set is "localhost.localdomain".
   * the FQDN when a hostname has been set, but no my_domname is set, is 'my_hostname'
   * the FQDN when both are set is 'my_hostname.my_domname'
   */
  const char *my_hostname = "localhost";
  const char *my_domname  = "localdomain";
  GETHOSTNAME_ARG2 pos;

  if (!buffer)
  {
    SOCK_ERRNO (EINVAL);
    return (-1);
  }
  if (buflen == 0)
     return (0);

#if (DOSX)
  if (!valid_addr(buffer, buflen))
  {
    SOCK_ERRNO (EFAULT);
    return (-1);
  }
#endif

  if (*hostname) /* otherwise use localhost.localdomain */
  {
    my_hostname = hostname;
    my_domname  = NULL;         /* have hostname but no domain name */
    if (def_domain && *def_domain)
       my_domname = def_domain;
  }
  pos = 0;
  while (pos < buflen && *my_hostname)
      buffer[pos++] = *my_hostname++;

  if (pos < buflen && my_domname)
  {
    buffer[pos++] = '.';
    while (pos < buflen && *my_domname)
        buffer[pos++] = *my_domname++;
  }
  if (pos < buflen)
     buffer[pos] = '\0';
  return (0);
}

/**
 * BSD-style: \n
 * Expects a "Fully Qualified Domain Name" in `fqdn'.
 * Split at first `.' and extract `hostname' and `def_domain'.
 * Set errno on failure and return -1.
 */
int W32_CALL sethostname (const char *fqdn, SETHOSTNAME_ARG2 len)
{
  SETHOSTNAME_ARG2 pos;

  if (!fqdn || !*fqdn || len == 0 || len > MAX_HOSTLEN)
  {
    SOCK_ERRNO (EINVAL);
    return (-1);
  }

#if (DOSX)
  if (!valid_addr(fqdn, len))
  {
    SOCK_ERRNO (EFAULT);
    return (-1);
  }
#endif

  pos = 0;
  while (pos < len && fqdn[pos] != '.')
  {
    if (!fqdn[pos]) /** \todo: should do complete alpha/digit/underscore check here */
    {
      pos = 0;
      break;
    }
    pos++;
  }
  if (pos == 0) /* leading '.' or invalid char in name */
  {
    SOCK_ERRNO (EINVAL);
    return (-1);
  }
  if (pos >= SIZEOF(hostname))
  {
    SOCK_ERRNO (ERANGE);
    return (-1);
  }
  if (fqdn[pos] == '.') /* a trailing '.' is ok too */
  {
    if (setdomainname(&fqdn[pos+1], len-pos) != 0)
       return (-1);
  }
  memcpy (&hostname[0], fqdn, pos);
  hostname[pos] = '\0';
  return (0);
}


#if defined(WIN32)

#ifndef ComputerNameDnsHostname
#define ComputerNameDnsHostname 1
#endif

/*
 * Get hostname excluding domain-name
 */
int _get_machine_name (char *buf, int size)
{
  DWORD sz = size;
  int   rc;

  if (p_GetComputerNameExA)
       rc = (*p_GetComputerNameExA) (ComputerNameDnsHostname, buf, &sz);
  else rc = GetComputerNameA (buf, &sz);

  rc = rc ? 0 : -1;
  TCP_CONSOLE_MSG (2, ("_get_machine_name(): \"%s\", rc %d\n", buf, rc));
  return (rc);
}

#else

/*
 * From Ralf Brown's interrupt list
 *
 * --------D-215E00-----------------------------
 * INT 21 - DOS 3.1+ network - GET MACHINE NAME
 *         AX = 5E00h
 *         DS:DX -> 16-byte buffer for ASCII machine name
 * Return: CF clear if successful
 *             CH = validity
 *                 00h name invalid
 *                 nonzero valid
 *                     CL = NetBIOS number for machine name
 *                     DS:DX buffer filled with blank-paded name
 *         CF set on error
 *             AX = error code (01h) (see #1366 at AH=59h)
 * Note:   supported by OS/2 v1.3+ compatibility box, PC-NFS
 */

/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */

/**
 * Try asking a LAN extension of DOS for a host-name.
 * Work for a DOS-box under Windows-XP.
 */
int _get_machine_name (char *buf, int size)
{
  IREGS reg;
  char *h;
  char  dosBuf[16];
  int   len;

  memset (&reg, 0, sizeof(reg));
  reg.r_ax = 0x5E00;

#if (DOSX & DJGPP)
  if (_go32_info_block.size_of_transfer_buffer < sizeof(dosBuf))
     return (-1);
  reg.r_ds = __tb / 16;
  reg.r_dx = __tb & 15;

#elif (DOSX & (PHARLAP|X32VM|POWERPAK))
  if (_watt_dosTbSize < sizeof(dosBuf) || !_watt_dosTbr)
     return (-1);
  reg.r_ds = RP_SEG (_watt_dosTbr);
  reg.r_dx = RP_OFF (_watt_dosTbr);

#elif (DOSX & DOS4GW)
  if (_watt_dosTbSize < sizeof(dosBuf) || !_watt_dosTbSeg)
     return (-1);
  reg.r_ds = _watt_dosTbSeg;
  reg.r_dx = 0;

#elif (DOSX == 0)
  reg.r_ds = FP_SEG (dosBuf);
  reg.r_dx = FP_OFF (dosBuf);

#else
  #error Help me!
#endif

  GEN_INTERRUPT (0x21, &reg);
  if ((reg.r_flags & CARRY_BIT) || hiBYTE(reg.r_cx) == 0)
     return (-1);

#if (DOSX & DJGPP)
  dosmemget (__tb, sizeof(dosBuf), dosBuf);

#elif (DOSX & (PHARLAP|X32VM|POWERPAK))
  ReadRealMem ((void*)&dosBuf, _watt_dosTbr, sizeof(dosBuf));

#elif (DOSX & DOS4GW)
  memcpy (dosBuf, SEG_OFS_TO_LIN(_watt_dosTbSeg,0), sizeof(dosBuf));
#endif

  h = strrtrim (dosBuf);
  len = strlen (h);
  if (len + 1 > size)
  {
    SOCK_ERRNO (ERANGE);
    return (-1);
  }
  strcpy (buf, h);
  strlwr (buf);
  return (0);
}
#endif  /*  WIN32 */

