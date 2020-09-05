/*!\file sys/wtime.h
 *
 * Watt-32 time functions.
 */

#ifndef __SYS_WTIME_H
#define __SYS_WTIME_H

/*
 * The naming <sys/wtime.h> is required for those compilers that
 * have <sys/time.h> in the usual place but doesn't define
 * the following.
 */

#include <time.h>

#ifdef __BORLANDC__
#undef timezone   /* a macro in bcc 5+ */
#endif

#ifndef __SYS_W32API_H
#include <sys/w32api.h>
#endif

#ifndef __SYS_CDEFS_H
#include <sys/cdefs.h>
#endif

#ifndef __SYS_WHIDE_H
#include <sys/whide.h>
#endif

#if defined(__DJGPP__) || defined(__CYGWIN__)
  #include <sys/time.h>
  #include <sys/times.h>
  #include <sys/types.h>

#elif defined(__MINGW32__)
  #include <sys/time.h>

#else
  struct timeval {
         long tv_sec;
         long tv_usec;
       };
  #define STRUCT_TIMEVAL_DEFINED
#endif

#if !defined(__DJGPP__) && !defined(__CYGWIN__)
  #if !W32_MINGW_VER(3,10)
    struct timezone {         /* Added in MinGW 3.1 */
           int tz_minuteswest;
           int tz_dsttime;
         };
  #endif

  struct tms {
         unsigned long tms_utime;
         unsigned long tms_cstime;
         unsigned long tms_cutime;
         unsigned long tms_stime;
       };
  #define STRUCT_TIMEZONE_DEFINED
  #define STRUCT_TMS_DEFINED

  __BEGIN_DECLS

  #define ITIMER_REAL  0
  #define ITIMER_PROF  1

  struct itimerval {
         struct timeval it_interval;  /* timer interval */
         struct timeval it_value;     /* current value */
       };

  W32_FUNC int W32_CALL getitimer (int, struct itimerval *);
  W32_FUNC int W32_CALL setitimer (int, struct itimerval *, struct itimerval *);

  #if !W32_MINGW_VER(3,10)   /* Added in MinGW 3.1 */
    W32_FUNC int W32_CALL gettimeofday (struct timeval *, struct timezone *);
  #endif

  __END_DECLS

#endif    /* !__DJGPP__ && !__CYGWIN__ */

/* 'struct timespec' hacks
 */
#if defined(_STRUCT_TIMESPEC) || defined(_TIMESPEC_DEFINED)
  #define W32_HAVE_STRUCT_TIMESPEC

#elif (defined(_MSC_VER) && _MSC_VER >= 1900)   /* VC 2015 uclib */
  #define W32_HAVE_STRUCT_TIMESPEC

#elif (defined(__clang__) && _MSC_VER >= 1800)  /* clang-cl 5.0+? */
  #define W32_HAVE_STRUCT_TIMESPEC

#elif defined(__POCC__) && ( __POCC_STDC_VERSION__ >= 201101L) /* PellesC */
  #define W32_HAVE_STRUCT_TIMESPEC

#elif defined(_pthread_signal_h) || defined(__CYGWIN__)
  #define W32_HAVE_STRUCT_TIMESPEC

#elif W32_MINGW_VER(3,21)                     /* MinGW 3.21 + */
  #define W32_HAVE_STRUCT_TIMESPEC
#endif

#if !defined(W32_HAVE_STRUCT_TIMESPEC)
  #define _STRUCT_TIMESPEC
  #define __struct_timespec_defined    /* For MinGW 3.21+ */

  struct timespec {
         long tv_sec;
         long tv_nsec;
       };
#endif

#ifndef HZ
#define HZ 18.2F
#endif

__BEGIN_DECLS

W32_FUNC unsigned long W32_CALL net_times (struct tms *buffer);
W32_FUNC int           W32_CALL gettimeofday2 (struct timeval *, struct timezone *);

__END_DECLS

/*
 * Operations on timevals.
 *
 * NB: timercmp does not work for >= or <=.
 */
#if !defined(timerisset)
  #define timerisset(tvp)         ((tvp)->tv_sec || (tvp)->tv_usec)

  #define timercmp(tvp, uvp, cmp) ((tvp)->tv_sec cmp (uvp)->tv_sec || \
                                   ((tvp)->tv_sec == (uvp)->tv_sec &&  \
                                    (tvp)->tv_usec cmp (uvp)->tv_usec))

  #define timerclear(tvp)         (tvp)->tv_sec = (tvp)->tv_usec = 0
#endif

#endif  /* !__SYS_WTIME_H */
