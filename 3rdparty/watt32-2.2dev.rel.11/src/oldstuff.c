/*!\file oldstuff.c
 *
 * Some old functions are collected here.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "copyrigh.h"
#include "wattcp.h"
#include "pctcp.h"
#include "netaddr.h"
#include "strings.h"
#include "chksum.h"
#include "misc.h"
#include "timer.h"
#include "pcdns.h"

/**
 * \deprecated.
 * Actually simply a macro in <tcp.h>
 */
#undef tcp_cbrk
int tcp_cbrk (int mode)
{
  return tcp_cbreak (mode);
}

/**
 * \deprecated.
 * Not needed in Watt-32. Return timeout from ticks.
 */
unsigned long set_ttimeout (unsigned ticks)
{
#ifdef WIN32
  DWORD now = set_timeout (0);
  return (ticks + now);
#else
  int   tmp = has_8254;
  DWORD now;

  has_8254 = 0;
  now = set_timeout (0);
  has_8254 = tmp;
  return (ticks + now);
#endif
}

/*
 * Not needed in Watt-32. Print remote tcp address and port.
 */
void W32_CALL psocket (const sock_type *s)
{
  char buf[20];

  (*_outch) ('[');
  outs (_inet_ntoa(buf, s->tcp.hisaddr));
  (*_outch) (':');
  itoa (s->tcp.hisport, buf, 10);
  outs (buf);
  (*_outch) (']');
}

/**
 * Keep the socket open inspite of receiving "ICMP Unreachable"
 * For UDP or TCP socket.
 */
void W32_CALL sock_sturdy (sock_type *s, int level)
{
  s->tcp.rigid = level;
  if (s->tcp.rigid < s->tcp.stress)
     sock_abort (s);
}

/**
 * Resolve an IP-address to a name.
 */
int W32_CALL resolve_ip (DWORD ip, char *name, int len)
{
  return reverse_resolve_ip4 (ip, name, len);
}

#undef time

/*
 * Some time/date conversion function only (?) found in Borland libs.
 * Only(?) djgpp/DMC besides Borland have "struct time" etc.
 */
#if defined(__DJGPP__) || defined(__DMC__)
time_t dostounix (struct date *d, struct time *t)
{
  struct tm tm;

  if (!d || !t)
     return (0);

  tm.tm_year  = d->da_year - 1900;
  tm.tm_mon   = d->da_mon - 1;
  tm.tm_mday  = d->da_day;
  tm.tm_isdst = 0;
  tm.tm_hour  = t->ti_hour;
  tm.tm_min   = t->ti_min;
  tm.tm_sec   = t->ti_sec;
  return mktime (&tm);
}

void unixtodos (time_t time, struct date *d, struct time *t)
{
  struct tm *tm = localtime (&time);

  if (d && tm)
  {
    d->da_year = tm->tm_year;
    d->da_mon  = tm->tm_mon;
    d->da_day  = tm->tm_mday;
  }
  if (t && tm)
  {
    t->ti_hour = tm->tm_hour;
    t->ti_min  = tm->tm_min;
    t->ti_sec  = tm->tm_sec;
    t->ti_hund = 0;
  }
}
#endif

#undef inchksum
WORD inchksum (const void *ptr, int len)
{
#ifdef HAVE_IN_CHECKSUM_FAST
  return _w32_in_checksum_fast (ptr, len);
#else
  return _w32_in_checksum (ptr, len);
#endif
}

WORD _w32_inchksum (const void *ptr, int len)
{
#ifdef HAVE_IN_CHECKSUM_FAST
  return _w32_in_checksum_fast (ptr, len);
#else
  return _w32_in_checksum (ptr, len);
#endif
}

#if !defined(WATT32_NO_INLINE_INTEL)
/*
 * If linking old .o-files with new watt*.lib.
 * Or user didn't include <sys/swap.h> (via tcp.h).
 */
#undef intel
#undef intel16
#undef htons
#undef htonl
#undef ntohs
#undef ntohl

DWORD intel (DWORD val)
{
  return __ntohl (val);
}

WORD intel16 (WORD val)
{
  return __ntohs (val);
}
#endif  /* WATT32_NO_INLINE_INTEL */

#if defined(TEST_PROG)

#if !defined(__MSDOS__)
#error For MSDOS only
#endif

int main (void)
{
  struct date d;
  struct time t;
  time_t now = time (NULL);

  getdate (&d);
  gettime (&t);
  printf ("time now is: %s", ctime(&now));
  now = dostounix (&d, &t);
  printf ("dostounix(): %s", ctime(&now));
  printf ("unixtodos(): %02d/%02d/%04d %02d:%02d:%02d\n",
          d.da_day,  d.da_mon, d.da_year,
          t.ti_hour, t.ti_min, t.ti_sec);
  return (0);
}
#endif

