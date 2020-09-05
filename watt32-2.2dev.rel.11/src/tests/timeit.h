#ifndef __TIMEIT_H
#define __TIMEIT_H

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>

#if defined(__GNUC__)
  #define uint64 unsigned long long
  #define GCC_INLINE extern __inline__ __attribute__ ((__gnu_inline__))

#elif defined(_MSC_VER)
  #define uint64 unsigned __int64
#endif

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
    __wbinvd();
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
          modify [eax edx];

  extern void _invd_cache (void);
  #pragma aux _invd_cache = \
            "db 0Fh, 08h";

#elif defined(__BORLANDC__) && defined(__FLAT__) /* bcc32 */
  static __inline uint64 _get_rdtsc (void)
  {
    __asm rdtsc
    __asm ret
  }

  static __inline void _invd_cache (void)
  {
    __asm db 0Fh, 08h
    __asm ret
  }

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

  static void cdecl sigill_handler (int sig)
  {
 /* printf ("SIGILL caught"); */
    do_invd = 0;
    signal (sig, SIG_IGN);
    longjmp (sig_jmp, 1);
  }

  static void check_invd (void)
  {
    void (cdecl *old)(int);

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

#if !defined(S64_FMT)
  #if defined(__GNUC__)
    #if defined(__DJGPP__) || defined(__CYGWIN__)
      #define S64_FMT   "lld"

    #elif defined(__MINGW32__) || defined(__MINGW64__)
      #define S64_FMT   "I64d"
    #endif

  #elif defined(_MSC_VER) || defined(_MSC_EXTENSIONS) || \
        defined(__WATCOMC__) || defined(__LCC__) || defined(__BORLANDC__)
    #define S64_FMT     "I64d"

  #else
    #define S64_FMT     "Ld"
  #endif
#endif

#define TIME_IT(func, args, loops)                      \
    do {                                                \
      uint64 x, T = _get_rdtsc();                       \
      int    i, flen = strlen (#func);                  \
                                                        \
      printf ("Timing %s()", #func);                    \
      for (i = 0; i < 27-flen; i++)                     \
         putchar ('.');                                 \
      fflush (stdout);                                  \
      check_invd();                                     \
      for (i = 0; i < (int)loops; i++)                  \
          func args;                                    \
      x = (_get_rdtsc() - T) / loops;                   \
      if (x > 1000ULL)                                  \
           printf (" %8" S64_FMT ".%03u clocks/loop\n", \
                   x/1000ULL, (unsigned)(x % 1000ULL)); \
      else printf (" %12" S64_FMT " clocks/loop\n", x); \
    }                                                   \
    while (0)

#endif  /* __TIMEIT_H */
