/*!\file sys/swap.h
 *
 * Swapping of short/long values.
 */

#ifndef __SYS_SWAP_BYTES_H
#define __SYS_SWAP_BYTES_H

#ifndef __SYS_W32API_H
#include <sys/w32api.h>
#endif

#ifndef __SYS_CDEFS_H
#include <sys/cdefs.h>
#endif

#ifndef __NETINET_IN_H
#include <netinet/in.h>
#endif

#ifndef __SYS_WHIDE_H
#include <sys/whide.h>   /* hide publics inside W32_NAMESPACE() */
#endif

#if defined(__dj_include_netinet_in_h_)
#error "You are using the wrong version of <netinet/in.h>. Ref. Note in point 9 of the INSTALL file"
#endif

__BEGIN_DECLS

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
  /*
   * Provide some real versions too
   */
  W32_FUNC unsigned short W32_CALL ntohs (unsigned short);
  W32_FUNC unsigned short W32_CALL htons (unsigned short);
  W32_FUNC unsigned long  W32_CALL ntohl (unsigned long);
  W32_FUNC unsigned long  W32_CALL htonl (unsigned long);
#endif

W32_FUNC unsigned long  W32_CDECL _w32_intel   (unsigned long x);
W32_FUNC unsigned short W32_CDECL _w32_intel16 (unsigned short x);

#undef  ntohs
#undef  htons
#undef  ntohl
#undef  htonl
#define ntohs(x)  intel16(x)
#define htons(x)  intel16(x)
#define ntohl(x)  intel(x)
#define htonl(x)  intel(x)

#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))
 /*
  * The below AT&T syntax breaks any code compiled with 'gcc -masm=intel'
  * Ref: https://github.com/gvanem/Watt-32/issues/61
  */
  #define intel(x)   __builtin_bswap32 (x)
  #define intel16(x) __builtin_bswap16 (x)
  #define W32_GCC_USING_BSWAP  /* for 'src/oldstuff.c' */

#elif defined(__GNUC__) && defined(__i386__) && !defined(__NO_INLINE__)  /* gcc -O0 on 'x86' */
  #define intel(x)   __NtoHL(x)
  #define intel16(x) __NtoHS(x)

  /*
   * Ripped (and adapted) from <linux/include/asm-386/byteorder.h>
   */
  /*@unused@*/ W32_GCC_INLINE unsigned long __NtoHL (unsigned long x)
  {
    __asm__ __volatile (
             "xchgb %b0, %h0\n\t"   /* swap lower bytes  */
             "rorl  $16, %0\n\t"    /* swap words        */
             "xchgb %b0, %h0"       /* swap higher bytes */
            : "=q" (x) : "0" (x));
    return (x);
  }

  /*@unused@*/ W32_GCC_INLINE unsigned short __NtoHS (unsigned short x)
  {
    __asm__ __volatile__ (
              "xchgb %b0, %h0"       /* swap bytes */
            : "=q" (x) : "0" (x));
    return (x);
  }

#elif defined(__GNUC__) && defined(__x86_64__) && !defined(__NO_INLINE__)  /* gcc -O0 on 'x64' */
  #define intel(x)   __NtoHL(x)
  #define intel16(x) __NtoHS(x)

  /*@unused@*/ W32_GCC_INLINE unsigned short __NtoHS (unsigned short x)
  {
    x = ((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8);
    return (x);
  }

  /*@unused@*/ W32_GCC_INLINE unsigned long __NtoHL (unsigned long x)
  {
    x = ((x & 0x000000FF) << 24) |
        ((x & 0x0000FF00) <<  8) |
        ((x & 0x00FF0000) >>  8) |
        ((x & 0xFF000000) >> 24);
    return (x);
  }

#elif defined(__POCC__) && 0 && !defined(_M_X64)    /* PellesC 32-bit */
  #define intel(x)   __NtoHL(x)
  #define intel16(x) __NtoHS(x)

  __declspec(naked) static unsigned short __fastcall __NtoHS (unsigned short x)   /* MSVC form */
  {
    __asm xchg  cl, ch    /* 'x' is in ecx */
    __asm movzx eax, cx
    __asm ret
  }

  __declspec(naked) static unsigned long __fastcall __NtoHL (unsigned long x)  /* MSVC form */
  {
    __asm xchg cl, ch    /* 'x' is in ecx */
    __asm ror  ecx, 16
    __asm xchg cl, ch
    __asm mov  eax, ecx
    __asm ret
  }

#elif (defined(_MSC_VER) && (_MSC_VER >= 1200)) &&  /* MSVC 6+ */ \
      !defined(__POCC__) &&        /* "pocc -Ze" sets _MSC_VER */ \
      !defined(_M_X64) &&          /* Not for 64-bit compilers */ \
      !defined(__clang__)          /* Disable this for Clang until problem spotted */

  #define intel(x)   __NtoHL(x)
  #define intel16(x) __NtoHS(x)

  /*
   * Clang has problems with __fastcall, so use __cdecl.
   */
  #if defined(__clang__)
    __declspec(naked) static unsigned short __cdecl __NtoHS (unsigned short x)
    {
      __asm mov  ax, [esp+4]
      __asm xchg al, ah
      __asm ret
    }

    __declspec(naked) static unsigned long __cdecl __NtoHL (unsigned long x)
    {
      __asm mov  eax, [esp+4]
      __asm xchg al, ah
      __asm ror  eax, 16
      __asm xchg al, ah
      __asm ret
    }

  #else
    __declspec(naked) static unsigned short __fastcall __NtoHS (unsigned short x)
    {
      __asm xchg  cl, ch    /* 'x' is in ecx */
      __asm movzx eax, cx
      __asm ret
    }

    __declspec(naked) static unsigned long __fastcall __NtoHL (unsigned long x)
    {
      __asm xchg cl, ch    /* 'x' is in ecx */
      __asm ror  ecx, 16
      __asm xchg cl, ch
      __asm mov  eax, ecx
      __asm ret
    }
  #endif

#elif defined(__WATCOMC__) && defined(__386__)  /* Watcom 32-bit */
  #define intel(x)   __NtoHL(x)
  #define intel16(x) __NtoHS(x)

  extern unsigned long __NtoHL (unsigned long x);
  #pragma aux  __NtoHL =       \
              "xchg al, ah"    \
              "ror  eax, 16"   \
              "xchg al, ah"    \
              __parm   [__eax] \
              __modify [__eax];

  extern unsigned short __NtoHS (unsigned short x);
  #pragma aux __NtoHS =       \
              "xchg al, ah"   \
              __parm   [__ax] \
              __modify [__ax];

#elif defined(__WATCOMC__) && defined(__I86__)  /* Watcom 16-bit */
  #define intel(x)   __NtoHL(x)
  #define intel16(x) __NtoHS(x)

  extern unsigned long __NtoHL (unsigned long x);
  #pragma aux  __NtoHL =           \
              "xchg al, dh"        \
              "xchg ah, dl"        \
              __parm   [__dx __ax] \
              __modify [__dx __ax];

  extern unsigned short __NtoHS (unsigned short x);
  #pragma aux __NtoHS =        \
              "xchg al, ah"    \
              __parm   [__ax]  \
              __modify [__ax];

#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x0700)
  /*
   * The LLVM based bcc32c compiler for Win32 have no
   * pseudo-registers (_EAX) and inline assembly.
   */
  #if !defined(WATT32_NO_NAMESPACE)
    #define intel   W32_NAMESPACE (intel)
    #define intel16 W32_NAMESPACE (intel16)
  #endif

  #define WATT32_NO_INLINE_INTEL

  W32_FUNC unsigned long  W32_CDECL intel   (unsigned long x);
  W32_FUNC unsigned short W32_CDECL intel16 (unsigned short x);

#elif (defined(__BORLANDC__) && defined(__FLAT__)) ||  /* bcc32 (flat/win32) */ \
      (defined(__DMC__) && (__INTSIZE==4))             /* dmc -mx */

  /* From Borland's <dos.h>:
   *   The prototype for __emit__ has been removed from DOS.H.  __emit__ is
   *   still supported, but is now automatically recognized by the compiler ...
   */
  #if !(defined(__BORLANDC__) && defined(WIN32))
    #include <dos.h>
  #endif

  #define intel(x)    __NtoHL(x)
  #define intel16(x)  __NtoHS(x)

  #define __NtoHS(x)  (_AX = x, \
                       __emit__(0x86, 0xC4),       /* xchg ah, al */ \
                       _AX)

  #define __NtoHL(x)  (_EAX = x, \
                       __emit__(0x86, 0xC4),       /* xchg ah, al  */ \
                       __emit__(0xC1, 0xC8, 0x10), /* ror  eax, 16 */ \
                       __emit__(0x86, 0xC4),       /* xchg ah, al  */ \
                       _EAX)

#elif defined(__CCDL__) && defined(__386__)     /* LadSoft 386 */
  #define intel(x)    __NtoHL(x)
  #define intel16(x)  __NtoHS(x)

  static unsigned long __NtoHL (unsigned long x)
  {
    asm { mov  eax, [x]
          xchg al, ah
          ror  eax, 16
          xchg al, ah
        }
    return (_EAX);
  }

  static unsigned short __NtoHS (unsigned short x)
  {
    asm { mov  ax, [x]
          xchg al, ah
        }
    return (unsigned short)_EAX;  /* LadSoft doesn't have _AX */
  }

  /* This crashes mysteriously if we use _bswap()
   */
#elif defined(__LCC__) && 0              /* LCC-Win32 */
  #define intel(x)    __NtoHL(x)
  #define intel16(x)  __NtoHS(x)

  #if 0
    #include <intrinsics.h>
    #define W32_LCC_INTRINSICS_INCLUDED  /* header guard is missing */
    #define __NtoHL(x)   (unsigned long) _bswap(x)
    #define __NtoHS(x)  ((unsigned short) (_bswap(x) >> 16))

  #else
    unsigned long inline __declspec(naked) __NtoHL (unsigned long x)
    {
      _asm ("movl (%esp), %eax");
      _asm ("xchg %ah, %al");
      _asm ("rorl $16, %eax");
      _asm ("xchg %ah, %al");
    }

    unsigned short inline __declspec(naked) __NtoHS (unsigned short x)
    {
      _asm ("movs (%esp), %ax");
      _asm ("xchg %ah, %al");
    }
  #endif

#else  /* no inlining possible (or worth the effort) */

  #if !defined(WATT32_NO_NAMESPACE)
    #define intel   W32_NAMESPACE (intel)
    #define intel16 W32_NAMESPACE (intel16)
  #endif

  #define WATT32_NO_INLINE_INTEL

  W32_FUNC unsigned long  W32_CDECL intel   (unsigned long x);
  W32_FUNC unsigned short W32_CDECL intel16 (unsigned short x);
#endif

__END_DECLS

#endif /* __SYS_SWAP_BYTES_H */
