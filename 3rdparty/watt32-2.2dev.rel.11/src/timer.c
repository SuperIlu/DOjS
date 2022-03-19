/*!\file timer.c
 *
 *  Module for handling timers.
 *
 *  Modifications by GvB 2002-09
 *
 *  \todo To get rid of the timer problem once and for all we need a
 *        timer ISR that increments a 32-bit value no regard to day
 *        roll-over etc. Maybe hook up IRQ 8 to call a real-mode stub
 *        that we opy to DOS-memory (similar to the pkt_receiver stub
 *        code. Then far-peek at that timer variable.
 *
 * 09-Jan-2003: Bernd Omenitch <Omenitsch@cashpoint.at> contributed the
 *    Timer ISR code (init_timer_isr() etc). I rewrote it and added
 *    support for Borland/PowerPak/Pharlap).
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <sys/wtime.h>

#include "wattcp.h"
#include "ioport.h"
#include "strings.h"
#include "gettod.h"
#include "pcdbug.h"
#include "pcqueue.h"
#include "language.h"
#include "cpumodel.h"
#include "powerpak.h"
#include "wdpmi.h"
#include "x32vm.h"
#include "rs232.h"
#include "misc.h"
#include "run.h"
#include "timer.h"

#if (DOSX & PHARLAP)
#include <mw/exc.h>  /* _dx_alloc_rmode_wrapper_iret() */
#endif

DWORD start_time = 0;              /**< Start-time of watt_sock_init() (in msec) */
DWORD start_day  = 0;              /**< Start-day of watt_sock_init() */

#if defined(WIN32)
  #if defined(__WATCOMC__)
    WINBASEAPI VOID WINAPI GetNativeSystemInfo (LPSYSTEM_INFO);
  #endif

  BOOL has_rdtsc = TRUE;           /**< Never set in Watt-32. Some users may need it. */
  BOOL use_rdtsc = TRUE;           /**< Ditto */

#else
  BOOL has_8254   = FALSE;         /**< Do we have a working 8254 timer chip? */
  BOOL has_rdtsc  = FALSE;         /**< Do we have a CPU with RDTSC instruction? */
  BOOL use_rdtsc  = FALSE;         /**< Do not use RDTSC by default */
  BOOL using_int8 = FALSE;         /**< init_timer_isr() called */
#endif

DWORD    clocks_per_usec  = 0L;    /**< Measured CPU speed (MHz) */
time_t   user_tick_base   = 0UL;   /* time() when user-tick started */
DWORD    user_tick_msec   = 0UL;   /* milli-sec user-tick counter */
BOOL     user_tick_active = FALSE;
unsigned num_cpus         = 1;

static DWORD date    = 0;
static DWORD date_ms = 0;

/*
 * Things for the user installable timer ISR.
 */
#if !(DOSX & DJGPP)
  #define TIMER_ISR_PERIOD  10UL  /* 10 msec between interrupts */
  #define TIMER_INTR        8
#endif

#if (DOSX == 0) || (DOSX & DOS4GW)
  #define HAVE_TIMER_ISR

  static W32_IntrHandler old_int_8 = NULL;

#elif (DOSX & (PHARLAP|X32VM))
  #define HAVE_TIMER_ISR

  static REALPTR old_int_8, int8_cback;
/*static FARPTR  old_int_8_pm; */
  static RMC_BLK rmBlock_int8;

#elif (DOSX & DJGPP)
  #define HAVE_TIMER_ISR

  static DWORD            itimer_resolution = 0UL; /* in usec */
  static void           (*old_sigalrm)(int);
  static struct itimerval old_itimer;
#endif

#if defined(HAVE_TIMER_ISR) && !(DOSX & DJGPP)
  static DWORD tick_timer_8254 = 0UL;

  /* 1193181 = 4.77MHz/4 pulses per sec.
   */
  #define TIMER_8254_COUNT ((1193181UL * TIMER_ISR_PERIOD) / 1000UL)
#endif


/**
 * Setup timer stuff and measure CPU speed.
 * Called from init_misc().
 */
void init_timers (void)
{
#if defined(WIN32)
  SYSTEM_INFO sys_info;

  memset (&sys_info, 0, sizeof(sys_info));

#if !defined(WIN64) && (_WIN32_WINNT >= 0x0501)
  if (_watt_is_wow64)
    GetNativeSystemInfo (&sys_info);
  else
#endif
    GetSystemInfo (&sys_info);

  num_cpus = sys_info.dwNumberOfProcessors;
  clocks_per_usec = (DWORD) (get_cpu_speed() / S64_SUFFIX(1000000));

#else
  #if (DOSX)                  /* problems using 64-bit types in small/large models */
    hires_timer (TRUE);      /**< \todo check if 8254 PIT is really working */
  #endif

  #if defined(HAVE_UINT64)
    if (use_rdtsc && has_rdtsc)
    {
      clocks_per_usec = (DWORD) (get_cpu_speed() / S64_SUFFIX(1000000));
      if (clocks_per_usec == 0UL)
      {
        outsnl ("CPU speed is 0?");
        use_rdtsc = FALSE;
        has_rdtsc = FALSE;
      }
    }
    set_utc_offset();  /* don't pull in gettod.c unneccesary */
  #endif
#endif

  chk_timeout (0UL);         /* init 'date' variables */

  start_time = set_timeout (0);
  start_day  = get_day_num();
}

/**
 * init_userSuppliedTimerTick() - setup timer handling for programs that are
 * already running a periodic interrupt or whatever (i.e. ALLEGRO & more).
 * This is called to signify that the 'user task' will periodically call
 * userTimerTick() to supply the elapsed time since the last call.
 * Best called once before sock_init(). There is no way to revert to
 * original timer handling once this is called.
 * By GvB 2002-09
 */
void W32_CALL init_userSuppliedTimerTick (void)
{
  SIO_TRACE (("init_userSuppliedTimerTick"));
  user_tick_active = TRUE;
  user_tick_base = time (NULL);
#ifdef __MSDOS__
  has_8254 = FALSE;
#endif
}

/*
 * Disable stack-checking here
 */
#if defined(__HIGHC__)
  #pragma off(Call_trace)
  #pragma off(Prolog_trace)
  #pragma off(Epilog_trace)
#endif

#include "nochkstk.h"

/**
 * Provide timer-ticks from an application.
 * This should be periodically called from user code to increment the timer
 * count once activated via init_userSuppliedTimerTick().
 * Best called from a periodic interrupt handler (int 8/1Ch/70h) which
 * is the case now with init_timer_isr().
 *
 * by GvB 2002-09
 */
void W32_CALL userTimerTick (DWORD elapsed_time_msec)
{
  user_tick_msec += elapsed_time_msec;
}

#if defined(HAVE_TIMER_ISR)
/**
 * User defined 10 (or 55) milli-sec counter ISR.
 */
#if (DOSX & (DJGPP|PHARLAP|X32VM))
  static void new_int_8 (void)   /* not really an intr-handler */
#else
  static INTR_PROTOTYPE new_int_8 (void)
#endif
{
#if !(DOSX & DJGPP)
  tick_timer_8254 += TIMER_8254_COUNT;

  if (tick_timer_8254 >= 0x10000)
  {
    tick_timer_8254 -= 0x10000;

#if defined(__WATCOMC__)
    _chain_intr (old_int_8);    /* chain now */
#elif (DOSX & (PHARLAP|X32VM))
    _dx_call_real (old_int_8, &rmBlock_int8, 1);
#else
    (*old_int_8)();
#endif
  }
  else
    _outportb (0x20, 0x60);  /* specific EOI for IRQ 0 */

  userTimerTick (TIMER_ISR_PERIOD);

#else
  userTimerTick (itimer_resolution / 1000UL);
#endif   /* !(DOSX & DJGPP) */

#if (DOSX & DJGPP)
  /* BEEP(); */  /* !! test */
#endif
}

void exit_timer_isr (void)
{
  if (!using_int8)
     return;

#if (DOSX & DJGPP)
  signal (SIGALRM, old_sigalrm);
  setitimer (ITIMER_REAL, &old_itimer, NULL);
#else
  if (old_int_8)
  {
    _outportb (0x43, 0x34);  /* restore default timer rate */
    _outportb (0x40, 0);
    _outportb (0x40, 0);

    PUSHF_CLI();

#if (DOSX & (PHARLAP|X32VM))
    _dx_rmiv_set (TIMER_INTR, old_int_8);
    if (int8_cback)
       _dx_free_rmode_wrapper (int8_cback);
    old_int_8  = 0;
    int8_cback = 0;
#else
    _dos_setvect (TIMER_INTR, old_int_8);
    old_int_8 = NULL;
#endif

    POPF();
  }
#endif  /* DJGPP */

  /* This causes timeout calculations for timers started earlier
   * to go wrong.
   */
  user_tick_active = FALSE;
  using_int8 = FALSE;
}

/*
 * DOS/djgpp:  Hook INT 8 and modify rate of timer channel 0 for 55 msec period.
 * DOS/others: Hook INT 8 and modify rate of timer channel 0 for 10 msec period.
 * Win32:      N/A.
 */
void init_timer_isr (void)
{
  /* Caller must do exit_timer_isr() before doing this again
   */
  if (using_int8)
     return;

#if (DOSX & DJGPP)
  {
    struct itimerval tim;

    memset (&tim, 0, sizeof(tim));
    tim.it_interval.tv_usec = 1;
    setitimer (ITIMER_REAL, &tim, NULL);
    setitimer (ITIMER_REAL, NULL, &tim);
    itimer_resolution = 1000000UL * tim.it_interval.tv_sec + /* should become 55000 */
                        tim.it_interval.tv_usec;
    tim.it_value.tv_sec  = tim.it_interval.tv_sec;
    tim.it_value.tv_usec = tim.it_interval.tv_usec;

    memset (&old_itimer, 0, sizeof(old_itimer));
    old_sigalrm = signal (SIGALRM, (W32_IntrHandler)new_int_8);
    setitimer (ITIMER_REAL, &tim, &old_itimer);
  }

#elif (DOSX & (PHARLAP|X32VM))
  #if 1
    int8_cback = _dx_alloc_rmode_wrapper_iret ((pmodeHook)new_int_8, 256);
    if (!int8_cback)
    {
      outsnl (_LANG("Cannot allocate real-mode timer callback"));
      return;
    }
    _dx_rmiv_get (8, &old_int_8);
    _dx_rmiv_set (8, int8_cback);
  #else
  {
    FARPTR fp;
    FP_SET (fp, new_int_8, MY_CS());
    _dx_pmiv_get (8, &old_int_8_pm);
    _dx_apmiv_set (8, fp);
  }
  #endif
#else
  old_int_8 = _dos_getvect (8);
  _dos_setvect (8, new_int_8);
#endif

#if !(DOSX & DJGPP)
  _outportb (0x43, 0x34);
  _outportb (0x40, loBYTE(TIMER_8254_COUNT));
  _outportb (0x40, hiBYTE(TIMER_8254_COUNT));
#endif

  init_userSuppliedTimerTick();
  using_int8 = TRUE;

  /* release it very early */
  RUNDOWN_ADD (exit_timer_isr, -2);
}
#endif  /* HAVE_TIMER_ISR */

/**
 * Compare two timers with expirary at 't1' and 't2'.
 *   \retval \li -1 if t1 expires before t2
 *           \li  0 if t1 == t2
 *           \li +1 if t1 expires after t2
 *
 * \note This logic fails when timers approaches ULONG_MAX
 *       after approx. 50 days.
 */
int W32_CALL cmp_timers (DWORD t1, DWORD t2)
{
  if (t1 == t2)
     return (0);
  return (t1 < t2 ? -1 : +1);
}

#if defined(__MSDOS__)

/**
 * Control use of high-resolution timer.
 * When using Watt-32 together with programs that reprogram the PIT
 * (8254 timer chip), the millisec_clock() function may return wrong
 * values. In such cases it's best to use old-style 55ms timers.
 * Using Watt-32 with Allegro graphics library is one such case where
 * application program should call `hires_timer(0)' after `sock_init()'
 * has been called.
 */
int hires_timer (int on)
{
  int old = has_8254;

  SIO_TRACE (("hires_timer %s", on ? "on" : "off"));
  has_8254 = on;
  chk_timeout (0UL);
  return (old);
}


/*
 * The following 2 routines are modified versions from KA9Q NOS
 * by Phil Karn.
 *
 * clockbits() - Read low order bits of timer 0 (the TOD clock)
 * This works only for the 8254 chips used in ATs and 386+.
 *
 * Old version:
 *   The timer 0 runs in mode 3 (square wave mode), counting down
 *   by 2s, twice for each cycle. So it is necessary to read back the
 *   OUTPUT pin to see which half of the cycle we're in. I.e., the OUTPUT
 *   pin forms the most significant bit of the count. Unfortunately,
 *   the 8253 in the PC/XT lacks a command to read the OUTPUT pin...
 *
 * Now simply:
 *   The timer 0 runs in mode 2 (rate generator), counting down.
 *   We don't compare the OUTPUT pin to see what cycle we're in.
 *   A bit lower precision, but a bit faster.
 *
 * WARNING!: This can cause serious trouble if anything else is also using
 * timer interrupts. In that case use the above "userSuppliedTimerTick()".
 * The reading of I/O registers gives bad precision under Windows.
 */
DWORD clockbits (void)
{
  static VOLATILE DWORD count;  /* static because of PUSHF_CLI() */

#if 1
  /*
   * The following version was suggested by Andrew Paulsen
   * <andrew.paulsen@ndsu.nodak.edu>. Seem to work fine.
   */
  static BOOL started = FALSE;

  PUSHF_CLI();

  if (!started)                  /* Initialize once, it is periodic */
  {
    _outportb (0x43, 0x34);      /* Set timer 0, mode 2 */
    _outportb (0x40, 0x00);      /* Count is decremented, then compared to 0 */
    _outportb (0x40, 0x00);      /* so set count to 0 to get 65536 */
    started = TRUE;
  }

  _outportb (0x43, 0x04);        /* Latch present counter value command */
  count  = _inportb (0x40);      /* LSB of count */
  count |= _inportb (0x40) << 8; /* MSB of count */

  POPF();

  return (count);

#else
  static VOLATILE DWORD stat, count;  /* static because of PUSHF_CLI() */

  do
  {
    PUSHF_CLI();
    _outportb (0x43, 0xC2);        /* Read Back Command (timer 0, 8254 only) */
    stat   = _inportb (0x40);      /* get status of timer 0 */
    count  = _inportb (0x40);      /* LSB of count */
    count |= _inportb (0x40) << 8; /* MSB of count */
    POPF();
  }
  while (stat & 0x40);             /* reread if NULL COUNT bit set */

  stat = (stat & 0x80) << 8;       /* Shift OUTPUT to MSB of 16-bit word */
  count >>= 1;                     /* count = count/2 */
  if (count == 0)
     return (stat ^ 0x8000UL);     /* return complement of OUTPUT bit */
  return (count | stat);           /* Combine OUTPUT with counter */
#endif
}

#if !defined(W32_NO_8087)
/*
 * Return hardware time-of-day in milliseconds. Resolution is improved
 * beyond 55 ms (the clock tick interval) by reading back the instantaneous
 * 8254 counter value and combining it with the clock tick counter.
 *
 * Reading the 8254 is a bit tricky since a tick could occur asynchronously
 * between the two reads in clockbits(). The tick counter is examined before
 * and after the hardware counter is read. If the tick counter changes, try
 * again.
 *
 * \note The hardware counter (lo) counts down from 65536 at a rate of
 *       1.193181 MHz (4.77/4). System tick count (hi) counts up.
 */
DWORD millisec_clock (void)
{
  DWORD  hi;
  WORD   lo;
  double x;

  do
  {
    hi = PEEKL (0, BIOS_CLK);
    lo = clockbits();
  }
  while (hi != PEEKL(0, BIOS_CLK));   /* tick count changed, try again */
  lo = 0 - lo;

  /* Emulating this would be slow. We'd better have a math-processor
   */
  x = ldexp ((double)hi, 16) + (double)lo;  /* x = hi*2^16 + lo */
  return (DWORD) (x * 4.0 / 4770.0);
}
#endif  /* !W32_NO_8087 */
#endif  /* __MSDOS__ */


/**
 * Return time for when given timeout (msec) expires.
 * Make sure it never returns 0 (it will confuse chk_timeout).
 */
DWORD W32_CALL set_timeout (DWORD msec)
{
  DWORD ret;

  if (user_tick_active)  /* using timer ISR or user-timer */
  {
    ret = user_tick_msec + msec;
  }
#if defined(HAVE_UINT64) /* CPU-clock, 8254 PIT or 55msec ticks */ || \
    defined(WIN32)       /* GetSystemTimeAsFileTime() */
  else
  {
    struct timeval now;

    gettimeofday2 (&now, NULL);
    ret = msec + 1000 * (DWORD)now.tv_sec + (DWORD)(now.tv_usec / 1000);
  }
#else

#if !defined(W32_NO_8087)
  else if (has_8254)    /* high-resolution PIT */
  {
    ret = msec + date_ms + millisec_clock();
  }
#endif
  else /* fallback to 55msec ticks */
  {
    DWORD ticks = 1;

    if (msec > 55)
       ticks = msec / 55UL;
 /* ret = ticks + ms_clock();  !!!! try this */
    ret = ticks + date + PEEKL (0,BIOS_CLK);
  }
#endif /* HAVE_UINT64 || WIN32 */

  return (ret == 0 ? 1 : ret);
}

/**
 * Check if milli-sec value has expired:
 *  \retval \li  TRUE  timer expired
 *          \li  FALSE not expired (or value not set)
 */
BOOL W32_CALL chk_timeout (DWORD value)
{
  if (user_tick_active)      /* using timer ISR or user-timer */
  {
    if (value == 0)
       return (0);
    return (user_tick_msec >= value);
  }

#if defined(HAVE_UINT64) || defined(WIN32)
  if (value == 0UL)
     return (0);
  return (set_timeout(0) >= value);

#else
  {
    static char  oldHour = -1;
#if !defined(W32_NO_8087)
    static DWORD now_ms;
#endif
    static DWORD now;
    char   hour;

    now  = (DWORD) PEEKL (0, BIOS_CLK);
#if 1
    hour = (char)(now >> 16U);
#else
    hour = (char)(now/65543.33496093);  /* Jan De Geeter 17.12.01 */
#endif

    if (hour != oldHour)
    {
      if (hour < oldHour)               /* midnight passed */
      {
#define CORR 290UL                      /* experimental correction */
        date    += 1572750UL + CORR;    /* Update date (0x1800B0UL) */
        date_ms += 3600UL*24UL*1000UL;  /* ~2^26 */
      }
      oldHour = hour;
    }

    if (value == 0L)        /* timer not initialised */
       return (0);          /* (or stopped) */

#if !defined(W32_NO_8087)
    if (has_8254)
    {
      now_ms = millisec_clock();
      return (now_ms + date_ms >= value);
    }
#endif
    now += date;            /* date extend current time */
    return (now >= value);  /* return true if expired */
  }
#endif   /* HAVE_UINT64 || WIN32 */
}

/**
 * Must be called by user right before or after a time change occurs.
 * Not used in Watt-32.
 */
int W32_CALL set_timediff (long msec)
{
  date_ms -= msec;
  date    -= msec/55;
  return (0);
}

/**
 * Return time difference between timers 'now' and 'tim'.
 * Check for day rollover. Max timer distance is 24 hours.
 * This function should be called immediately after chk_timeout()
 * is called.
 */
long W32_CALL get_timediff (DWORD now, DWORD tim)
{
#if defined(HAVE_UINT64) || defined(WIN32)
  return (long)(now - tim);
#else
  long dt = (long)(now - tim);

  if (has_8254)
       dt = dt % (3600L*24L*1000L);
  else dt = dt % (1572750L + CORR);
  return (dt);
#endif
}

#if !defined(W32_NO_8087)
/*
 * Return difference (in micro-sec) between timevals `*newer' and `*older'
 */
double W32_CALL timeval_diff (const struct timeval *newer, const struct timeval *older)
{
  long d_sec  = (long)newer->tv_sec - (long)older->tv_sec;
  long d_usec = newer->tv_usec - older->tv_usec;

  while (d_usec < 0)
  {
    d_usec += 1000000L;
    d_sec  -= 1;
  }
  return ((1E6 * (double)d_sec) + (double)d_usec);
}
#endif

struct timeval W32_CALL timeval_diff2 (struct timeval *ta, struct timeval *tb)
{
  struct timeval tv;

  if (tb->tv_usec > ta->tv_usec)
  {
    ta->tv_sec--;
    ta->tv_usec += 1000000L;
  }
  tv.tv_sec  = ta->tv_sec - tb->tv_sec;
  tv.tv_usec = ta->tv_usec - tb->tv_usec;
  if (tv.tv_usec >= 1000000L)
  {
    tv.tv_usec -= 1000000L;
    tv.tv_sec++;
  }
  return (tv);
}

#if defined(USE_DEBUG)
/*
 * Return string "x.xx" for timeout value.
 * 'val' is always in milli-sec units if we're using the 8254 PIT,
 * RDTSC timestamps, timer ISR or user-supplied timer.
 * 'val' is always in milli-sec units on Win32.
 */
const char *time_str (DWORD val)
{
  static char buf[30];
  char   fmt[6];
  double flt_val;

#if defined(WIN32)
  flt_val = (double)val / 1000.0F;
  strcpy (fmt, "%.3f");

#else

  if (has_8254 || has_rdtsc || user_tick_active)
  {
    flt_val = (double)val / 1000.0F;
    strcpy (fmt, "%.3f");
  }
  else
  {
    flt_val = (double)val / 18.2F;
    strcpy (fmt, "%.2f");
  }
#endif

#if defined(SNPRINTF)
  SNPRINTF (buf, sizeof(buf), fmt, flt_val);
#else
  sprintf (buf, fmt, flt_val);
#endif
  return (buf);
}

/*
 * Return string "HH:MM:SS" for a time in seconds.
 */
const char *hms_str (DWORD sec)
{
  static char buf[30];
  WORD   hour = (WORD) (sec / 3600UL);
  WORD   min  = (WORD) (sec / 60) - (60 * hour);

  sprintf (buf, "%02u:%02u:%02u", hour, min, (UINT)(sec % 60UL));
  return (buf);
}

/*
 * Return string "xx:xx" for time elapsed since started.
 * Handles max 24 days. Or max 49 days with user_tick_active.
 */
const char *elapsed_str (DWORD val)
{
  static char buf[30];
  WORD   hour, min, msec;
  DWORD  sec;
  BOOL   msec_clock = 1;

#ifdef __MSDOS__
  msec_clock = (has_8254 || has_rdtsc);
#endif

  if (val == 0UL)
     return ("00:00:00.000");

#if 0
  if (!user_tick_active && val < start_time)   /* wrapped? */
     val = ULONG_MAX - val + start_time;
#endif

  /* If user-ticks is used, 'val' should be correct msec count
   * since init_userSuppliedTimerTick() was called.
   */
  if (user_tick_active)
  {
    sec  = val / 1000UL;
    msec = (WORD) (val % 1000UL);
  }
  else if (msec_clock)
  {
    val -= start_time;
    sec  = val / 1000UL;
    msec = (WORD) (val % 1000UL);
  }
  else
  {
    val -= start_time;
    sec  = val / 18;
    msec = 55 * (WORD)(val % 18UL);
  }

  hour = (WORD) (sec / 3600UL);
  min  = (WORD) (sec / 60UL) - 60 * hour;
  sec  = sec % 60UL;
  sprintf (buf, "%02u:%02u:%02lu.%03u", hour, min, (u_long)sec, msec);
  return (buf);
}
#endif /* USE_DEBUG */


#if defined(__MSDOS__)
/*
 * The following was contributed by
 * "Alain" <alainm@pobox.com>
 */
#define TICKS_DAY 1573067UL   /* Number of ticks in one day */

DWORD ms_clock (void)
{
  static DWORD lastTick, tickOffset;
  static char  firstTime = 1;
  DWORD  tick = PEEKL (0, 0x46C);

  if (firstTime)
  {
    firstTime = 0;
    lastTick  = tickOffset = tick;
    return (0);
  }
  if (lastTick > tick)        /* day changed */
     tickOffset -= TICKS_DAY;
  lastTick = tick;
  return (tick - tickOffset);
}

#if defined(__HIGHC__) || defined(__DMC__)
void delay (unsigned int msec)
{
  for (; msec; msec--)
  {
    DWORD now = ms_clock();
    while (ms_clock() == now)   /* wait 1msec */
       ;
  }
}
#endif

#if (DOSX) && defined(HAVE_UINT64)
/*
 * Wait for BIOS timer-tick to change 'num' times.
 */
static void Wait_N_ticks (int num)
{
  while (num--)
  {
    DWORD time = PEEKL (0, BIOS_CLK);

    while (time == PEEKL(0,BIOS_CLK))
    {
#ifdef __DJGPP__
      ENABLE();    /* CWSDPMI requires this! */
#else
      ((void)0);
#endif
    }
  }
}

/**
 * Return estimated CPU speed in Hz.
 */
uint64 get_cpu_speed (void)
{
  #define NUM_SAMPLES 3
  #define WAIT_TICKS  2

  uint64 speed;
  uint64 sample [NUM_SAMPLES];
  int    i;

  Wait_N_ticks (1);

  for (i = 0; i < NUM_SAMPLES; i++)
  {
    uint64 start = GET_RDTSC();

    Wait_N_ticks (WAIT_TICKS);
    sample[i] = 1000 * (GET_RDTSC() - start) / (WAIT_TICKS*55);
  }

  speed = 0;
  for (i = 0; i < NUM_SAMPLES; i++)
      speed += sample[i];
  return (speed / NUM_SAMPLES);
}
#endif  /* DOSX && HAVE_UINT64 */

#elif defined(WIN32)

/* Another hackery section for 'gcc -O0 -fgnu89-inline':
 */
#if defined(__GNUC__) && defined(__NO_INLINE__)  /* -O0 */
  #if defined(__i386__)
    uint64 _w32_get_rdtsc (void)
    {
      register uint64 tsc;
      __asm__ __volatile__ (
                ".byte 0x0F, 0x31;"   /* rdtsc opcode */
              : "=A" (tsc) );
      return (tsc);
    }

  #elif defined(__x86_64__)
    uint64 _w32_get_rdtsc (void)
    {
      unsigned hi, lo;
      __asm__ __volatile__ (
                "rdtsc" : "=a" (lo), "=d" (hi) );
      return ( (uint64)lo) | ( ((uint64)hi) << 32 );
    }
  #else
    #error What kind of CPU is this?
  #endif
#endif    /* __GNUC__ && __NO_INLINE__ */


#if 0
/*
 * Wait for timer-ticks to change 'num' times.
 */
static void Wait_N_ticks (int num)
{
  DWORD tim;

  while (num--)
  {
    tim = GetTickCount() + 55;
    while (GetTickCount() < tim)
          Sleep (0);
  }
}
#endif

static BOOL get_processor_freq (uint64 *Hz)
{
  BOOL  rc = FALSE;
  DWORD MHz, size = sizeof(MHz);
  HKEY  key = NULL;
  LONG  status = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                               "HARDWARE\\DESCRIPTION\\System\\"
                               "CentralProcessor\\0", 0, KEY_READ, &key);

  if (status != ERROR_SUCCESS)
     goto fail;

  status = RegQueryValueEx (key, "~MHz", NULL, NULL, (BYTE*)&MHz, &size);
  if (status != ERROR_SUCCESS)
     goto fail;

  *Hz =  MHz * U64_SUFFIX(1000000);
  rc = TRUE;

fail:
  if (key)
     RegCloseKey (key);
  return (rc);
}

/**
 * Return estimated CPU speed in Hz.
 * With multiple CPU cores, the QueryPerformanceFrequency() doesn't
 * give the real CPU frequency. Then simply extract from Registry.
 */
uint64 get_cpu_speed (void)
{
  static LARGE_INTEGER qpc;
  static uint64        Hz;
  static BOOL          done = FALSE;
  static uint64        rc = U64_SUFFIX(0);

  if (done)
     return (rc);

  if (num_cpus > 1)
       rc = get_processor_freq(&Hz) ? Hz : U64_SUFFIX(0);
  else if (QueryPerformanceFrequency(&qpc))
       rc = qpc.QuadPart;
  else rc = U64_SUFFIX(0);
  done = TRUE;
  return (rc);
}

uint64 win_get_perf_count (void)
{
  LARGE_INTEGER rc;
  DWORD  am = 0UL;
  HANDLE ct = INVALID_HANDLE_VALUE;

  if (num_cpus > 1)
  {
    ct = GetCurrentThread();
    am = SetThreadAffinityMask (ct, 1);
  }
  QueryPerformanceCounter (&rc);
  if (num_cpus > 1)
     SetThreadAffinityMask (ct, am);
  return (rc.QuadPart);
}

/*
 * Make sure RDTSC is returned from the 1st CPU on a multi (or hyper-threading)
 * CPU system.
 */
uint64 win_get_rdtsc (void)
{
  DWORD  am = 0UL;
  HANDLE ct = INVALID_HANDLE_VALUE;
  uint64 rc;

  if (num_cpus > 1)
  {
    ct = GetCurrentThread();
    am = SetThreadAffinityMask (ct, 1);
  }
  rc = get_rdtsc();
  if (num_cpus > 1)
     SetThreadAffinityMask (ct, am);
  return (rc);
}
#endif  /* __MSDOS__ */

