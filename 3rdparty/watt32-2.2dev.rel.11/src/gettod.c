/*!\file gettod.c
 *
 *  gettimeofday() for non-djgpp targets and \n
 *  gettimeofday2() for higher accuracy timing.
 */

/*  Copyright (c) 1997-2002 Gisle Vanem <gvanem@yahoo.no>
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
 *  10.aug 1996 - Created
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <errno.h>

#include "wattcp.h"
#include "misc.h"
#include "timer.h"
#include "x32vm.h"
#include "cpumodel.h"
#include "strings.h"
#include "win_dll.h"
#include "gettod.h"

#define STEVES_PATCHES 1

#if defined(WIN32)
  #include <sys/timeb.h>
#else
  static long utc_offset = 0;  /* UTC to local offset in seconds */
#endif

/*
 * Timezone defines.
 */
#if defined(__WATCOMC__)
  #define _timezone timezone
#elif defined(__TURBOC__) && (__TURBOC__ <= 0x410)  /* TCC/BCC <= 3.1 */
  #define _timezone timezone
#elif defined(__POCC__)
  #define _timezone 0
#elif defined(_MSC_VER) && (_MSC_VER <= 600)
  #define _timezone timezone
#endif

#if defined(__CCDL__)
#define int386 _int386
#endif

#ifndef __inline
#define __inline
#endif

static void get_zone (struct timezone *tz, time_t now)
{
  struct tm  res;
  struct tm *tm = localtime_r (&now, &res);

  if (tm && tz)
  {
#ifdef __DJGPP__
    tz->tz_minuteswest = - res.__tm_gmtoff / 60;
#else
    tz->tz_minuteswest = - (int)_timezone;
#endif
    tz->tz_dsttime = res.tm_isdst;
  }
}

#if defined(WIN32)
/*
 * Stolen and modified from APR (Apache Portable Runtime):
 * Number of micro-seconds between the beginning of the Windows epoch
 * (Jan. 1, 1601) and the Unix epoch (Jan. 1, 1970).
 *
 * This assumes all Win32 compilers have 64-bit support.
 */
#define DELTA_EPOCH_IN_USEC  U64_SUFFIX (11644473600000000)

uint64 FILETIME_to_unix_epoch (const FILETIME *ft)
{
  uint64 res = (uint64) ft->dwHighDateTime << 32;

  res |= ft->dwLowDateTime;
  res /= 10;                   /* from 100 nano-sec periods to usec */
  res -= DELTA_EPOCH_IN_USEC;  /* from Win epoch to Unix epoch */
  return (res);
}

const char *ULONGLONG_to_ctime (ULONGLONG ts)
{
  LARGE_INTEGER ft;
  time_t        t;
  const struct  tm *tm;
  static char   buf[30];
  static        BOOL tz_set = FALSE;

  if (!tz_set)
     tzset();
  tz_set = TRUE;

  ft.QuadPart = ts;
  t = FILETIME_to_unix_epoch ((const FILETIME*)&ft) / U64_SUFFIX(1000000);
  tm = localtime (&t);
  if (tm)
       strftime (buf, sizeof(buf), "%Y-%m-%d/%H:%M:%S", tm);
  else snprintf (buf, sizeof(buf), "%lu/%" U64_FMT, (u_long)t, ts);
  return (buf);
}

W32_FUNC int W32_CALL W32_NAMESPACE (gettimeofday) (
         struct timeval  *tv,
         struct timezone *tz)
{
  if (!tv && !tz)
  {
    SOCK_ERRNO (EINVAL);
    return (-1);
  }

  if (tv)
  {
    FILETIME ft;
    uint64   tim;

    if (p_GetSystemTimePreciseAsFileTime)
         (*p_GetSystemTimePreciseAsFileTime) (&ft);
    else GetSystemTimeAsFileTime (&ft);

    tim = FILETIME_to_unix_epoch (&ft);
    tv->tv_sec  = (long) (tim / 1000000L);
    tv->tv_usec = (long) (tim % 1000000L);
  }
  if (tz)
     get_zone (tz, tv->tv_sec);
  return (0);
}
#else

/**
 * Called from init_timers() once.
 */
void set_utc_offset (void)
{
  struct timezone tz = { 0, 0 };

#if !defined(__POCC__)
  tzset();
#endif
  get_zone (&tz, time(NULL));
  utc_offset = 60 * tz.tz_minuteswest;

#ifdef TEST_PROG
  printf ("Minutes west %d, DST %d\n", tz.tz_minuteswest, tz.tz_dsttime);
#endif
}
#endif   /* WIN32 */


#if (DOSX == 0)
int W32_CALL W32_NAMESPACE (gettimeofday) (struct timeval *tv, struct timezone *tz)
{
  union  REGS reg;
  struct tm   tm;

  if (!tv)
  {
    SOCK_ERRNO (EINVAL);
    return (-1);
  }

  reg.h.ah = 0x2C;
  int86 (0x21, &reg, &reg);

  tv->tv_usec = reg.h.dl * 10000UL;
  tm.tm_sec   = reg.h.dh;
  tm.tm_min   = reg.h.cl;
  tm.tm_hour  = reg.h.ch;

  reg.h.ah = 0x2A;
  int86 (0x21, &reg, &reg);

  tm.tm_mday  = reg.h.dl;
  tm.tm_mon   = reg.h.dh - 1;
  tm.tm_year  = (reg.x.cx & 0x7FF) - 1900;
  tm.tm_wday  = tm.tm_yday = 0;
  tm.tm_isdst = -1;

  tv->tv_sec = mktime (&tm);

  if (tz)
     get_zone (tz, tv->tv_sec);
  return (0);
}

#elif (DOSX & (PHARLAP|X32VM))
int W32_CALL W32_NAMESPACE (gettimeofday) (struct timeval *tv, struct timezone *tz)
{
  SWI_REGS  reg;
  struct tm tm;

  if (!tv)
  {
    SOCK_ERRNO (EINVAL);
    return (-1);
  }

  reg.r_ax = 0x2C00;
  _dx_real_int (0x21, &reg);

  tv->tv_usec = loBYTE (reg.r_dx) * 10000UL;
  tm.tm_sec   = hiBYTE (reg.r_dx);
  tm.tm_min   = loBYTE (reg.r_cx);
  tm.tm_hour  = hiBYTE (reg.r_cx);

  reg.r_ax = 0x2A00;
  _dx_real_int (0x21, &reg);

  tm.tm_mday  = loBYTE (reg.r_dx);
  tm.tm_mon   = hiBYTE (reg.r_dx) - 1;
  tm.tm_year  = (reg.r_cx & 0x7FF) - 1900;
  tm.tm_wday  = tm.tm_yday = 0;
  tm.tm_isdst = -1;

  tv->tv_sec = mktime (&tm);
  if (tz)
     get_zone (tz, tv->tv_sec);
  return (0);
}

#elif defined(__MSDOS__) && (defined(WATCOM386) || defined(BORLAND386) || defined(__CCDL__))
int W32_CALL W32_NAMESPACE (gettimeofday) (struct timeval *tv, struct timezone *tz)
{
  union  REGS reg;
  struct tm   tm;

  if (!tv)
  {
    SOCK_ERRNO (EINVAL);
    return (-1);
  }

  reg.x.eax = 0x2C00;
  int386 (0x21, &reg, &reg);

  tv->tv_usec = reg.h.dl * 10000UL;
  tm.tm_sec   = reg.h.dh;
  tm.tm_min   = reg.h.cl;
  tm.tm_hour  = reg.h.ch;

  reg.x.eax = 0x2A00;
  int386 (0x21, &reg, &reg);

  tm.tm_mday  = reg.h.dl;
  tm.tm_mon   = reg.h.dh - 1;
  tm.tm_year  = (reg.x.ecx & 0x7FF) - 1900;
  tm.tm_wday  = tm.tm_yday = 0;
  tm.tm_isdst = -1;

  tv->tv_sec = mktime (&tm);
  if (tz)
     get_zone (tz, tv->tv_sec);
  return (0);
}
#endif   /* DOSX == 0 */


#if defined(HAVE_UINT64) && defined(__MSDOS__)
/*
 * Return hardware time-of-day in microseconds.
 * NOTE: hardware counter (lo) counts down from 65536 at a rate of
 *       1.19318 MHz (4.77/4).
 */
static __inline uint64 microsec_clock (void)
{
  DWORD  hi;
  long   lo;
  uint64 rc;

  do
  {
    hi = PEEKL (0, BIOS_CLK);
    lo = clockbits();
  }
  while (hi != PEEKL(0, BIOS_CLK));   /* tick count changed, try again */
  lo = 0 - lo;
  rc = ((uint64)hi << 16) + lo;
  return (rc * U64_SUFFIX(4000000) / U64_SUFFIX(4770000));
}

#if (DOSX)
/*
 * Return CPU time-stamp counter in microseconds since epoch
 * (1 Jan 1970 UTC) or from '*base'.
 */
static uint64 tsc_microsec (const uint64 *base)
{
  static uint64 time_ofs = U64_SUFFIX(-1);
  static time_t start    = 0;
  uint64 rc, now;

  if (clocks_per_usec == 0)
     return (0);

  if (time_ofs == U64_SUFFIX(-1))
  {
    start = time (NULL);
    while (time(NULL) == start)
          ENABLE();
    time_ofs = GET_RDTSC();
  }
  if (base)
       now = *base;
  else now = GET_RDTSC();
  rc  = (now - time_ofs) / clocks_per_usec;
  rc += U64_SUFFIX(1000000) * (start+1);
  return (rc);
}

static void adjust_cpu_clock (const struct timeval *tv)
{
  static DWORD last = 0;
  DWORD  tick = PEEKL (0, BIOS_CLK);

  if (((tick % 5) == 0) && last != tick) /* 260msec passed */
  {
    static double sample[4];
    static int    idx = 0;

    if (idx < DIM(sample))
    {
      struct timeval tv0;

      gettimeofday (&tv0, NULL);
      sample[idx++] = timeval_diff (&tv0, tv);
    }
    else
    {
      double diff = 0.0;
      int    i, corr;

      for (i = 0; i < DIM(sample); i++)
          diff += sample[i];
      idx  = 0;
      diff = diff / DIM(sample);
      corr = abs ((int)last-(int)tick) / 8;
      corr = diff < 0.0 ? -corr : +corr;
      clocks_per_usec += corr;

#ifdef TEST_PROG
      printf ("diff %.6f, corr %d\n", diff/1E6, corr);
#endif
    }
    last = tick;
  }
}

/*
 * Return a 'timeval' from a CPU timestamp.
 */
BOOL get_tv_from_tsc (const struct ulong_long *tsc, struct timeval *tv)
{
  uint64 usecs;

  if (!tsc || !tv || !has_rdtsc)
     return (FALSE);

  usecs = tsc_microsec ((const uint64*)tsc);
  if (!usecs)
     return (FALSE);

  tv->tv_usec = (long) (usecs % U64_SUFFIX(1000000));
  tv->tv_sec  = (time_t) ((usecs - tv->tv_usec) / U64_SUFFIX(1000000));
  return (TRUE);
}
#endif /* DOSX */
#endif /* HAVE_UINT64 && __MSDOS__ */


/*
 * A high-resolution [1us] version of gettimeofday() needed in
 * select_s() etc. Should return 'tv_sec' in UTC.
 */
#if defined(WIN32)
int W32_CALL gettimeofday2 (struct timeval *tv, struct timezone *tz)
{
  init_misc();
  return gettimeofday (tv, tz);
}

#else

int W32_CALL gettimeofday2 (struct timeval *tv, struct timezone *tz)
{
  init_misc();

  if (user_tick_active)
  {
    tv->tv_usec = (long) (1000UL * (user_tick_msec % 1000UL));
    tv->tv_sec  = (DWORD) user_tick_base + (user_tick_msec / 1000UL);

    if (tz)
       get_zone (tz, tv->tv_sec);
    return (0);
  }

#if defined(HAVE_UINT64)
#if (DOSX)
  if (has_rdtsc && use_rdtsc)
  {
    uint64 usecs = tsc_microsec (NULL);

    if (!usecs)
       return (-1);

    tv->tv_usec = (long) (usecs % U64_SUFFIX(1000000));
    tv->tv_sec  = (time_t) ((usecs - tv->tv_usec) / U64_SUFFIX(1000000));
    if (tz)
       get_zone (tz, tv->tv_sec);
    adjust_cpu_clock (tv);
    return (0);
  }
#endif

  if (has_8254)
  {
    static time_t secs = 0;           /* seconds since midnight */
    uint64 usecs = microsec_clock();  /* usec day-clock */

#if STEVES_PATCHES
    secs = time (NULL);
#else
    static uint64 last = 0;
    if (secs == 0 || usecs < last)    /* not init or wrapped */
    {
      secs = time (NULL);
      secs -= (secs % (24*3600));
    }
    last = usecs;
#endif
    tv->tv_usec = (long) (usecs % U64_SUFFIX(1000000));
#if STEVES_PATCHES
    tv->tv_sec = (time_t) secs;
#else
    tv->tv_sec  = (time_t) ((usecs - tv->tv_usec) / U64_SUFFIX(1000000) + (uint64)secs);
#endif
    tv->tv_sec += utc_offset;

    if (tz)
       get_zone (tz, tv->tv_sec);
    return (0);
  }
#endif  /* HAVE_UINT64 */

  return gettimeofday (tv, tz);
}
#endif


#if defined(TEST_PROG)

#include "printk.h"

#ifdef __MSDOS__
  static __inline BOOL day_rollover (void)
  {
    return PEEKB (0, 0x470);
  }
#else
  #define day_rollover()  0
  #undef  hires_timer
  #define hires_timer(x)  ((void)0)
#endif

#if !defined(__DJGPP__) && 0
static void usleep (DWORD usec)
{
  struct timeval now, start, expiry;

  gettimeofday2 (&start, NULL);
  expiry.tv_sec  = start.tv_sec  + (usec / 1000000UL);
  expiry.tv_usec = start.tv_usec + (usec % 1000000UL);
  while (expiry.tv_usec >= 1000000UL)
  {
    expiry.tv_usec -= 1000000UL;
    expiry.tv_sec++;
  }
  while (1)
  {
    gettimeofday2 (&now, NULL);
    if (now.tv_sec > expiry.tv_sec ||
        (now.tv_sec == expiry.tv_sec && now.tv_usec > expiry.tv_usec))
       break;
  }
}
#endif

int main (int argc, char **argv)
{
  DWORD loops   = 0;
  BOOL  do_cmp  = FALSE;
  BOOL  use_isr = FALSE;
  int   ch;

  putenv ("USE_RDTSC=0");

  init_misc();

  while ((ch = getopt(argc, argv, "?hcHir")) != EOF)
        switch (ch)
        {
          case 'c':
               do_cmp = TRUE;
               break;
          case 'H':
               hires_timer (0);
               break;
          case 'i':
               init_timer_isr();
               use_isr = TRUE;
               break;
          case 'r':
               if (use_isr)
               {
                 puts ("Cannot use both `-i' and `-r'");
                 return (-1);
               }
               putenv ("USE_RDTSC=1");
               break;
          case '?':
          case 'h':
          default:
               printf ("Usage: %s [-chir]\n"
                       "  -c compare gettimeofday2() and gettimeofday()\n"
                       "  -H don't use hi-res 8254 PIT (DOS only)\n"
                       "  -i use timer ISR             (DOS only)\n"
                       "  -r use RDTSC for gettimeofday2()\n",
                       argv[0]);
               return (0);
        }

  if (do_cmp)
       puts ("gettimeofday2(),   gettimeofday(),    tv2-tv        ctime     rollover");
  else puts ("gettimeofday2(),   sleep,    (ctime)");

  puts ("------------------------------------------------------------------------");

  while (!kbhit())
  {
    struct timeval tv, tv2;
    struct timeval last;
    time_t t;
    double delta;
    const char *ct;

    gettimeofday (&tv, NULL);
    gettimeofday2 (&tv2, NULL);

    if (loops > 0)
         delta = timeval_diff (&tv2, &last);
    else delta = 0.0;
    last.tv_sec  = tv2.tv_sec;
    last.tv_usec = tv2.tv_usec;

    if (do_cmp)
    {
      printf ("%10ld.%06lu, %10ld.%06lu, %12.6f",
              (long)tv2.tv_sec, tv2.tv_usec, (long)tv.tv_sec, tv.tv_usec,
              timeval_diff(&tv2,&tv)/1E6);
    }
    else
      printf ("%10ld.%06lu, %.6f", (long)tv2.tv_sec, tv2.tv_usec, delta/1E6);

    t = tv2.tv_sec;
    ct = ctime (&t);  /*  Cannot do 'ctime (&tv2.tv_sec)' directly since 'tv2.tv_sec' is a 'long' */
    printf (", %.8s, %d\n", ct ? ct+11 : "??", day_rollover());

#if defined(__DJGPP__)   /* don't use delay(). It doesn't work under Win2K/XP */
    usleep (1000000);
#elif defined(_WIN32)
    Sleep (1000);
#else
    sleep (1);
#endif
    loops++;
  }
  return (0);
}
#endif  /* TEST_PROG */

