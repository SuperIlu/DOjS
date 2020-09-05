/*!\file timer.h
 */
#ifndef _w32_TIMER_H
#define _w32_TIMER_H

#ifndef __SYS_WTIME_H
#include <sys/wtime.h>      /* struct timeval */
#endif

#define user_tick_active  W32_NAMESPACE (user_tick_active)
#define user_tick_base    W32_NAMESPACE (user_tick_base)
#define user_tick_msec    W32_NAMESPACE (user_tick_msec)
#define clocks_per_usec   W32_NAMESPACE (clocks_per_usec)
#define num_cpus          W32_NAMESPACE (num_cpus)
#define millisec_clock    W32_NAMESPACE (millisec_clock)
#define start_time        W32_NAMESPACE (start_time)
#define start_day         W32_NAMESPACE (start_day)
#define init_timers       W32_NAMESPACE (init_timers)

#define time_str          W32_NAMESPACE (time_str)
#define hms_str           W32_NAMESPACE (hms_str)
#define elapsed_str       W32_NAMESPACE (elapsed_str)

#define has_rdtsc         W32_NAMESPACE (has_rdtsc)
#define use_rdtsc         W32_NAMESPACE (use_rdtsc)

#if defined(WIN32)
  #define init_timer_isr()   ((void)0)
  #define exit_timer_isr()   ((void)0)
  #define win_get_perf_count W32_NAMESPACE (win_get_perf_count)

  W32_DATA BOOL has_rdtsc, use_rdtsc;
  extern uint64 win_get_perf_count (void);

#else
  /*
   * System clock at BIOS location 40:6C (dword). Counts upwards.
   */
  #define BIOS_CLK 0x46C

  #define init_timer_isr  W32_NAMESPACE (init_timer_isr)
  #define exit_timer_isr  W32_NAMESPACE (exit_timer_isr)
  #define has_8254        W32_NAMESPACE (has_8254)
  #define using_int8      W32_NAMESPACE (using_int8)
  #define clockbits       W32_NAMESPACE (clockbits)
  #define ms_clock        W32_NAMESPACE (ms_clock)
  #define hires_timer     W32_NAMESPACE (hires_timer)

  extern BOOL has_8254, using_int8;
  extern BOOL has_rdtsc, use_rdtsc;

  extern DWORD clockbits      (void);
  extern int   hires_timer    (int on);
  extern DWORD ms_clock       (void);
  extern void  init_timer_isr (void);
  extern void  exit_timer_isr (void);
#endif

W32_DATA BOOL     user_tick_active;
W32_DATA DWORD    clocks_per_usec, user_tick_msec;
W32_DATA time_t   user_tick_base;
W32_DATA DWORD    start_time, start_day;
W32_DATA unsigned num_cpus;

extern void  init_timers      (void);
extern DWORD millisec_clock   (void);

extern const char *time_str   (DWORD val);
extern const char *hms_str    (DWORD sec);
extern const char *elapsed_str(DWORD val);

#if defined(__HIGHC__) || (defined(__DMC__) && defined(__MSDOS__))
  #define delay  W32_NAMESPACE (delay)

  extern void delay (unsigned int msec);
#endif

#if (DOSX) && defined(HAVE_UINT64)
  #define get_cpu_speed  W32_NAMESPACE (get_cpu_speed)

  extern uint64 get_cpu_speed (void);
#endif

#endif

