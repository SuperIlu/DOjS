#ifndef _SYSDEP_H
#define _SYSDEP_H

/*
 * Timing stuff and portability hacks.
 */
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>

#if !defined(__CYGWIN__) && !defined(__BORLANDC__)
#include <dos.h>
#endif

#if !defined(__CYGWIN__) && !defined(__DJGPP__)
#include <conio.h>
#endif

#if !defined(_MSC_VER) && !defined(__BORLANDC__)
#include <unistd.h>
#endif

/*
 * When compiling pure Winsock versions of various test programs:
 */
#if defined(_Windows) && !defined(WATT32)
  #define __WATT_TCP_H
  #define __NETINET_IN_H
  #define __SYS_SOCKET_H
  #define __SYS_WTIME_H
  #define _WINSOCK_DEPRECATED_NO_WARNINGS

  #if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0501)
    #undef  _WIN32_WINNT
    #define _WIN32_WINNT 0x0501
  #endif

  #if defined(__CYGWIN__) && !defined(__USE_W32_SOCKETS)
    /*
     * Do not call 'WSAStartup()' in test programs.
     */
    #undef _Windows

    #include <sys/select.h>
    #include <sys/ioctl.h>

    #define uint64   unsigned long long
    #define u_int64  uint64
    #define ioctlsocket(s, cmd, val_p)  ioctl(s, cmd, val_p)
  #else
    /*
     * Undo what <sys/w32api.h> did.
     */
    #undef _WINSOCKAPI_
    #undef _WINSOCK2API_
    #undef _WINSOCK_H
    #undef _WINSOCK2_H

    #include <winsock2.h>
    #include <windows.h>

    #define close(s)  closesocket (s)
    #define uint64    unsigned __int64

    static struct WSAData wsa_state;

    static void cleanup (void)
    {
      WSACleanup();
    }
  #endif

  #if defined(__CYGWIN__)
    int kbhit (void);  /* In cyg_kbhit.c */
  #endif

#else
  #include "wattcp.h"
  #include "timer.h"
  #include "misc.h"

  #include <netinet/in.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
#endif

#ifdef WATT32
  #undef _Windows  /* '__BORLANDC__' for Win32 seems to have this as a built-in */

  #undef  kbhit
  #define kbhit()   watt_kbhit()
  #define close(s)  close_s(s)
  #define select(num, rd, wr, exc, tv) select_s(num, rd, wr, exc, tv)
#endif

#if defined(_WIN32)
  #include <windows.h>
  #define sleep(x)  Sleep((x)*1000)
#endif

#if defined(__CYGWIN__)
  #define strnicmp(s1, s2, len)  strncasecmp (s1, s2, len)
#endif

#if defined(__DJGPP__)
  int getch (void);  /* Since <conio.h> can not be included */
#endif

#if defined(__DMC__)
  #define dosdate_t  dos_date_t
  #define dostime_t  dos_time_t
#endif

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif

/*
 * Hacks to shut-up Cygwin x64.
 */
#if !defined(__CYGWIN__) || !defined(__USE_W32_SOCKETS)
  #define __ms_u_long   u_long
  #define __ms_timeval  timeval
#endif

#if (DOSX)
  #define HAVE_CHECK_CPU_TYPE
#endif

#if !defined(__WATCOMC__)
  #define __watcall cdecl
#else

  /*
   * Use this if no version is provider for 'MSDOS' targets in '../timer.c'
   */
  #if defined(W32_NO_8087)
  inline double W32_CALL timeval_diff (const struct timeval *newer, const struct timeval *older)
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
#endif   /* __WATCOMC__ */

#define GCC_INLINE    extern __inline__ __attribute__ ((__gnu_inline__))
#define GCC_FASTCALL  __attribute__((__fastcall__))

#if defined(__GNUC__) && defined(__i386__) && !defined(__NO_INLINE__)  /* -O0 */
  GCC_INLINE uint64 _get_rdtsc (void)
  {
    register uint64 tsc;
    __asm__ __volatile__ (
              ".byte 0x0F, 0x31;"   /* rdtsc opcode */
            : "=A" (tsc) );
    return (tsc);
  }

  GCC_INLINE void _invd_cache (void)
  {
    __asm__ __volatile__ (
              ".byte 0x0F, 0x08;");   /* INVD opcode */
  }

#elif defined(__GNUC__) && defined(__x86_64__) && !defined(__NO_INLINE__)
  GCC_INLINE uint64 _get_rdtsc (void)
  {
    unsigned hi, lo;
    __asm__ __volatile__ (
              "rdtsc" : "=a" (lo), "=d" (hi) );
    return ( (uint64)lo) | ( ((uint64)hi) << 32 );
  }

  GCC_INLINE void _invd_cache (void)
  {
    __asm__ __volatile__ (
              ".byte 0x0F, 0x08;");   /* INVD opcode */
  }

#elif defined(_MSC_VER) && (_MSC_VER >= 1200) && defined(_M_IX86)
  /*
   * MSVC 6+, 32-bit.
   */
  __declspec(naked) static uint64 _get_rdtsc (void)
  {
    __asm rdtsc
    __asm ret
  }

  __declspec(naked) static void _invd_cache (void)
  {
    __asm invd
    __asm ret
  }

#elif defined(_MSC_VER) && (_MSC_VER >= 1200) && defined(_M_X64)
  #include <intrin.h>
  static __inline uint64 _get_rdtsc (void)
  {
    return __rdtsc();
  }

  static __inline void _invd_cache (void)
  {
   #if !defined(__clang__)  /* where is this in clang-cl? */
    __wbinvd();
   #endif
  }

#elif defined(__WATCOMC__) && defined(__386__)
  extern uint64 _get_rdtsc (void);
  #pragma aux _get_rdtsc = \
          ".586"           \
          "db 0Fh, 31h"    \
          "cld"            \
          "nop"            \
          "nop"            \
          "nop"            \
          "nop"            \
          __modify [__eax __edx];

  extern void _invd_cache (void);
  #pragma aux _invd_cache = \
          "db 0Fh, 08h";

#elif defined(__WATCOMC__) && defined(__I86__) && defined(__WATCOM_INT64__)
  extern uint64 _get_rdtsc (void);   /* Is this right? */
  #pragma aux _get_rdtsc = \
          ".586"           \
          "db 0Fh, 31h"    \
          "cld"            \
          "nop"            \
          "nop"            \
          "nop"            \
          "nop"            \
          __modify [__ax __dx];

  #define get_rdtsc() _get_rdtsc() /* since there isn't one in cpumodel.asm */

  extern void _invd_cache (void);
  #pragma aux _invd_cache = \
          "db 0Fh, 08h";

#elif defined(__BORLANDC__) && defined(__FLAT__) /* bcc32 / bcc32c */
  #if (__BORLANDC__ >= 0x0700)
    static __inline uint64 _get_rdtsc (void)
    {
      register uint64 tsc;
      __asm__ __volatile__ (
                ".byte 0x0F, 0x31;"   /* rdtsc opcode */
              : "=A" (tsc) );
      return (tsc);
    }
    static __inline void _invd_cache (void)
    {
      __asm__ __volatile__ (
                ".byte 0x0F, 0x08;");   /* INVD opcode */
    }
  #else
    static __inline uint64 _get_rdtsc (void)
    {
      __asm rdtsc
      __asm ret
    }

    static __inline void _invd_cache (void)
    {
      __asm db 0Fh
      __asm db 08h
      __asm ret
    }
  #endif

#elif defined(__LCC__)
  #include <intrinsics.h>
  #define _get_rdtsc() _rdtsc()

#else
  #error "Unsupported CPU/compiler"
#endif

/*
 * Use the TIME_IT() macro as:
 *
 * void func_to_be_timed (void *arg1, int arg2)
 * {
 *   // whatever to be tested for speed...
 * }
 *
 * int main (void)
 * {
 *   void *buf = input_for_func();
 *   TIMEIT (func_to_be_timed, (buf, 1000), 10000);
 *   return (0);
 * }
 */
#ifdef INVD_CACHE
  static int     do_invd = -1;
  static jmp_buf sig_jmp;

  static void sigill_handler (int sig)
  {
 /* printf ("SIGILL caught"); */
    do_invd = 0;
    signal (sig, SIG_IGN);
    longjmp (sig_jmp, 1);
  }

  static void check_invd (void)
  {
    void (*old)(int);

    if (do_invd == 0)
       return;
    old = signal (SIGILL, sigill_handler);
    if (!setjmp(sig_jmp))
    {
      _invd_cache();
      do_invd = 1;
    }
    signal (SIGILL, old);
  }
#else
  #define check_invd() ((void)0)
#endif

/* If '../misc.h' was not included for this combination,
 * but we need this:
 */
#if !defined(S64_FMT)
  #if defined(__GNUC__)
    #if defined(__DJGPP__) || defined(__CYGWIN__)
      #define S64_FMT   "lld"

    #elif defined(__MINGW32__) || defined(__MINGW64__)
      #define S64_FMT   "I64d"
    #endif

  #elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x0700)
    #define S64_FMT     "lld"

  #elif defined(_MSC_VER) || defined(_MSC_EXTENSIONS) || \
        defined(__WATCOMC__) || defined(__LCC__) || defined(__BORLANDC__)
    #define S64_FMT     "I64d"

  #else
    #define S64_FMT     "Ld"
  #endif
#endif

#define TIME_IT(func, args, loops)                           \
    do {                                                     \
      volatile uint64        clk_per_loop, T = _get_rdtsc(); \
      volatile unsigned long abs, dec;                       \
      volatile long          i, flen;                        \
                                                             \
      flen = printf ("Timing %s()", #func);                  \
      for (i = 0; i < 37-flen; i++)                          \
          putchar ('.');                                     \
      fflush (stdout);                                       \
      check_invd();                                          \
      for (i = 0; i < (long)loops; i++)                      \
          if (!func args)                                    \
             break;                                          \
      if (i == (long)loops) { /* all loops ran okay */       \
        clk_per_loop = (_get_rdtsc() - T) / loops;           \
        abs = (unsigned long) (clk_per_loop/1000ULL);        \
        dec = (unsigned long) (clk_per_loop % 1000ULL);      \
        if (clk_per_loop > 1000ULL)                          \
             printf (" %6lu.%03lu clocks/loop\n", abs, dec); \
        else printf (" %10lu clocks/loop\n", abs);           \
      }                                                      \
    }                                                        \
    while (0)

#endif  /* _SYSDEP_H */
