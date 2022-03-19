#ifndef _w32_TARGET_H
#define _w32_TARGET_H

/*!\file target.h
 *
 * Definitions of targets and macros for Waterloo tcp/ip.
 *
 * by G. Vanem <gvanem@yahoo.no> 1995 - 2015.
 */

#ifndef _w32_WATTCP_H
#error TARGET.H must be included inside or after WATTCP.H
#endif

#define DOS4GW       1             /**< Tenberry's DOS extender        (1+2)  */
#define DJGPP        2             /**< GNU C/C++ and djgpp 2.0 target        */
#define PHARLAP      4             /**< PharLap 386|DosX extender target (1)  */
#define PHAR286      8             /**< PharLap 286|DosX extender target      */
#define POWERPAK     16            /**< Borland's PowerPak DOS extender       */
#define X32VM        32            /**< FlashTek X-32/X-32VM extender         */
#define WINWATT      64            /**< Windows (32/64-bit) using WinPcap     */
#define PHARLAP_DLL (0x80|PHARLAP) /**< PharLap DLL version target (not yet)  */
#define DOS4GW_DLL  (0x80|DOS4GW)  /**< DOS4GW DLL version target (possible?) */
#define DJGPP_DXE   (0x80|DJGPP)   /**< djgpp DXE target (not yet)            */

/*
 * Notes:
 *
 * (1) Most 32-bit DOS compilers (Borland/Watcom/Microsoft/HighC/DMC)
 *     will work with this DOS extender. Some compilers support far
 *     pointers (48-bits), some don't. And even worse, some of those who
 *     do, have bugs in their segment register handling!
 *     Add `-DBUGGY_FARPTR=1' to your makefile's CFLAGS if you experience
 *     this or crashes in weird places (generate .asm listing to find out).
 *
 *     The problem is present in:
 *       - Metaware's HighC v3.1 at -O3 or above (for sure).
 *       - BCC32 v4, Some rumours say far-ptrs in combination with FPU-ops.
 *
 * (2) Several DOS-extenders supports Watcom-386. DOS4GW (from Tenberry)
 *     is a DPMI 0.9 host with limited API. Other compatible DOS-extenders
 *     can also be used without modifying Watt-32. These are:
 *     DOS4GW Pro, DOS4G, Pmode/W, CauseWay, DOS32A, EDOS, WDOSX, HX-DOS and
 *     Zurenava. Watcom-386 with FlashTek's X-32 hasn't been tested.
 *
 */

#ifndef BUGGY_FARPTR
#define BUGGY_FARPTR 0      /* Assume no compilers have fp-bugs, duh! */
#endif

#if defined(_MSC_VER) && defined(M_I86SM)   /* Microsoft doesn't have */
  #define __SMALL__                         /* __SMALL__,  __LARGE__  */
#endif

#if defined(_MSC_VER) && defined(M_I86LM)
  #define __LARGE__
#endif

#if defined(__TINY__) || defined(__COMPACT__) || defined(__MEDIUM__) || defined(__HUGE__)
  #error Unsupported memory model (tiny/compact/medium/huge)
#endif

#if defined(M_I86TM) || defined(M_I86CM) || defined(M_I86MM) || defined(M_I86HM)
  #error Unsupported memory model (tiny/compact/medium/huge)
#endif

#if defined(_M_I86MM) || defined(_M_I86MH)
  #error Unsupported memory model (medium/huge)
#endif

/* 32-bit Digital Mars Compiler defines __SMALL__. So take care when testing
 * for real __SMALL__.
 */
#if defined(__DMC__) && (__INTSIZE==4) && defined(__SMALL__)
  #define __SMALL32__
  #define DMC386
#endif

#if (defined(__SMALL__) || defined(__LARGE__)) && !defined(__SMALL32__)
  #undef  DOSX
  #define DOSX 0
#endif

/*
 * djgpp 2.x with GNU C 2.7 or later.
 */
#if defined(__DJGPP__) && defined(__GNUC__)
  #undef  DOSX
  #define DOSX      DJGPP
#endif

/*
 * Watcom 11.x or OpenWatcom 1.x.
 */
#if defined(__WATCOMC__) && defined(__386__)
  #ifndef DOSX              /* If not set in watcom_*.mak file */
    #undef  DOSX
    #define DOSX    DOS4GW  /* may be DOS4GW, X32VM, PHARLAP or WINWATT */
  #endif
  #define WATCOM386 1
#endif

/*
 * Digital Mars Compiler 8.30+
 */
#if defined(__DMC__) && (__INTSIZE==4)
  #ifndef DOSX              /* If not set in dmars_*.mak file */
    #undef  DOSX
    #define DOSX   PHARLAP  /* may be X32VM, DOS4GW or PHARLAP */
  #endif
#endif

/*
 * PellesC works with Win32 only.
 * Note: "pocc -Ze" sets _MSC_VER. Hence the #undef (would cause
 *       troubles otherwise).
 */
#if defined(__POCC__)
  #if !defined(__POCC__OLDNAMES)
    #error Use the "pocc -Go" option.
  #endif

  /*
   * Lots of these:
   *  warning #2116: Local '(no name)' is used without being initialized.
   */
  #pragma warn (disable: 2116)

  #undef  DOSX
  #define DOSX      WINWATT
  #undef _MSC_VER
#endif

/*
 * Microsoft Visual-C 32-bit. Really not supported with PharLap,
 * but works with Win32 as target.
 */
#if defined(_MSC_VER) && (_M_IX86 >= 300)
  #undef  DOSX
  #define DOSX      WINWATT  /* VC 2.0 can use PHARLAP too */
  #define MSC386
#endif

/*
 * Metaware's High-C 3.x.
 */
#if defined(__HIGHC__)
  #if !defined(_I386) || (_I386 == 0)
  #error What CPU are you for?
  #endif
  #if !defined(__i386__)
  #define __i386__
  #endif

  #undef  DOSX
  #define DOSX      PHARLAP   /* Is DOS4GW possible? */
#endif

/*
 * Borland 32-bit PowerPak compiler (v4.x, v5.x untested)
 */
#if defined(__BORLANDC__) && defined(__FLAT__) && defined(__DPMI32__)
  #define BORLAND386
  #undef  DOSX
  #define DOSX      POWERPAK  /* may also be DOS4GW (for WDOSX targets) */
                              /* or PHARLAP (not yet) */
#endif

#if defined(__BORLANDC__) && defined(WIN32)
  #undef  DOSX
  #define DOSX      WINWATT
#endif

/*
 * LadSoft (cc386 is rather buggy, so it's not really supported)
 */
#if defined(__CCDL__) && defined(__386__)
  #undef  DOSX
  #define DOSX      DOS4GW
#endif

/*
 * Build for Windows (dll and static lib).
 */
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(__CYGWIN__)
  #undef  DOSX
  #define DOSX      WINWATT
#endif

#if defined(__CYGWIN__) && defined(_REENT_ONLY) && defined(WATT32_BUILD)
  #error "CygWin with _REENT_ONLY is not supported."
#endif

#if !defined(DOSX)
  #error DOSX target not defined
#endif

#if (defined(MSDOS) || defined(_MSDOS)) && !defined(__MSDOS__)
#define __MSDOS__   /* dmc etc. defines only MSDOS+_MSDOS */
#endif

#if (DOSX == 0) && !defined(__MSDOS__)
  #error __MSDOS__ not defined for a real-mode DOS compiler!?
#endif

#if (DOSX & WINWATT) == 0 && !defined(__MSDOS__)
  #define __MSDOS__
#endif


/*
 * Macros and hacks depending on target (i.e. DOS-extender or Windows).
 */

#if (DOSX & PHARLAP)
  #include <stdio.h>
  #include <pharlap.h>
  #undef FP_OFF
  #include <dos.h>

  #ifdef WATCOM386
  #include <i86.h>
  #endif

  extern REALPTR _watt_dosTbr;    /* Location and size of DOS transfer buffer */
  extern DWORD   _watt_dosTbSize;

  #if !defined(DMC386) &&         /* It supports far-ptr, but don't use them */ \
      (!BUGGY_FARPTR)  &&         /* Trust the compiler to handle far-ptr ? */  \
      (__CMPLR_FEATURES__ & __FEATURE_FARPTR__)  /* compilers with far-ptrs */
    #define HAVE_FARPPTR48                       /* i.e. HighC, Watcom386   */
    extern FARPTR _watt_dosFp;

    #define DOSMEM(s,o,t) *(t _far*)(_watt_dosFp + (DWORD)((o)|(s)<<4))
    #define PEEKB(s,o)    DOSMEM(s,o,BYTE)
    #define PEEKW(s,o)    DOSMEM(s,o,WORD)
    #define PEEKL(s,o)    DOSMEM(s,o,DWORD)
    #define POKEB(s,o,x)  DOSMEM(s,o,BYTE)  = (BYTE)(x)
    #define POKEW(s,o,x)  DOSMEM(s,o,WORD)  = (WORD)(x)
    #define POKEL(s,o,x)  DOSMEM(s,o,DWORD) = (DWORD)(x)
  #else
    #define PEEKB(s,o)    PeekRealByte (((s) << 16) + (o))
    #define PEEKW(s,o)    PeekRealWord (((s) << 16) + (o))
    #define PEEKL(s,o)    PeekRealDWord(((s) << 16) + (o))
    #define POKEB(s,o,x)  PokeRealByte (((s) << 16) + (o), (x))
    #define POKEW(s,o,x)  PokeRealWord (((s) << 16) + (o), (x))
    #define POKEL(s,o,x)  PokeRealDWord(((s) << 16) + (o), (x))
  #endif

#elif (DOSX & PHAR286)   /* 16-bit protected mode */
  #include <stdio.h>
  #include <phapi.h>

  #error Pharlap Lite not supported yet

#elif (DOSX & DJGPP)
  #include <dos.h>
  #include <dpmi.h>
  #include <go32.h>
  #include <sys/farptr.h>

  #define PEEKB(s,o)      _farpeekb (_dos_ds, (o)+((s)<<4))    /**< peek at a BYTE in DOS memory */
  #define PEEKW(s,o)      _farpeekw (_dos_ds, (o)+((s)<<4))    /**< peek at a WORD in DOS memory */
  #define PEEKL(s,o)      _farpeekl (_dos_ds, (o)+((s)<<4))    /**< peek at a DWORD in DOS memory */
  #define POKEB(s,o,x)    _farpokeb (_dos_ds, (o)+((s)<<4), x) /**< poke a BYTE to DOS memory */
  #define POKEW(s,o,x)    _farpokew (_dos_ds, (o)+((s)<<4), x) /**< poke a WORD to DOS memory */
  #define POKEL(s,o,x)    _farpokel (_dos_ds, (o)+((s)<<4), x) /**< poke a DWORD to DOS memory */
  #define BOOL            int

#elif (DOSX & DOS4GW)      /* Watcom targets normally uses this section */
  #if defined(__DJGPP__)
    #include <dpmi.h>
    #include <go32.h>

  #elif defined(__CCDL__)
    #include <dpmi.h>
    #include <i86.h>
  #endif

  #include <dos.h>

  extern WORD  _watt_dosTbSeg;
  extern DWORD _watt_dosTbSize;

  #define DOSMEM(s,o,t)   *(volatile t *) (((s) << 4) | (o))
  #define PEEKB(s,o)      DOSMEM(s,o,BYTE)
  #define PEEKW(s,o)      DOSMEM(s,o,WORD)
  #define PEEKL(s,o)      DOSMEM(s,o,DWORD)
  #define POKEB(s,o,x)    DOSMEM(s,o,BYTE)  = (BYTE)(x)
  #define POKEW(s,o,x)    DOSMEM(s,o,WORD)  = (WORD)(x)
  #define POKEL(s,o,x)    DOSMEM(s,o,DWORD) = (DWORD)(x)
  #undef  BOOL
  #define BOOL int

#elif (DOSX & X32VM)
  #include <x32.h>

  #if defined(DMC386) || defined(WATCOM386)
    #define HAVE_FARPTR48
    typedef BYTE _far *FARPTR;
    extern FARPTR _watt_dosFp;
  #endif

  extern DWORD _watt_dosTbr, _watt_dosTbSize;

  #ifdef DMC386
    #define DOSMEM(s,o,t) (t*)((DWORD)_x386_zero_base_ptr + (DWORD)((o)|(s)<<4))
    #define PEEKB(s,o)    *DOSMEM(s,o,BYTE)
    #define PEEKW(s,o)    *DOSMEM(s,o,WORD)
    #define PEEKL(s,o)    *DOSMEM(s,o,DWORD)
    #define POKEB(s,o,x)  *DOSMEM(s,o,BYTE)  = (BYTE)(x)
    #define POKEW(s,o,x)  *DOSMEM(s,o,WORD)  = (WORD)(x)
    #define POKEL(s,o,x)  *DOSMEM(s,o,DWORD) = (DWORD)(x)
  #else
    #define PEEKB(s,o)    PeekRealByte (((s) << 16) + (o))
    #define PEEKW(s,o)    PeekRealWord (((s) << 16) + (o))
    #define PEEKL(s,o)    PeekRealDWord(((s) << 16) + (o))
    #define POKEB(s,o,x)  PokeRealByte (((s) << 16) + (o), x)
    #define POKEW(s,o,x)  PokeRealWord (((s) << 16) + (o), x)
    #define POKEL(s,o,x)  PokeRealDWord(((s) << 16) + (o), x)
  #endif

  #undef  BOOL
  #define BOOL int

#elif (DOSX & POWERPAK)   /* Borland 32-bit DOS normally uses this section */
  #include <dos.h>

  extern WORD  _watt_dosTbSeg, _watt_dos_ds;
  extern DWORD _watt_dosTbr, _watt_dosTbSize;

  #define PEEKB(s,o)      _farpeekb (_watt_dos_ds, (o)+((s)<<4))
  #define PEEKW(s,o)      _farpeekw (_watt_dos_ds, (o)+((s)<<4))
  #define PEEKL(s,o)      _farpeekl (_watt_dos_ds, (o)+((s)<<4))
  #define POKEB(s,o,x)    _farpokeb (_watt_dos_ds, (o)+((s)<<4), x)
  #define POKEW(s,o,x)    _farpokew (_watt_dos_ds, (o)+((s)<<4), x)
  #define POKEL(s,o,x)    _farpokel (_watt_dos_ds, (o)+((s)<<4), x)
  #undef  BOOL
  #define BOOL int

#elif !(DOSX & WINWATT)   /* All real-mode and non-Windows targets */
  #include <dos.h>
  #define  BOOL           int

  #if defined(__WATCOMC__) || defined(_MSC_VER)
    #define PEEKB(s,o)    (*((BYTE  __far*) MK_FP(s,o)))
    #define PEEKW(s,o)    (*((WORD  __far*) MK_FP(s,o)))
    #define PEEKL(s,o)    (*((DWORD __far*) MK_FP(s,o)))
    #define POKEB(s,o,x)  (*((BYTE  __far*) MK_FP(s,o)) = (BYTE)(x))
    #define POKEW(s,o,x)  (*((WORD  __far*) MK_FP(s,o)) = (WORD)(x))
    #define POKEL(s,o,x)  (*((DWORD __far*) MK_FP(s,o)) = (DWORD)(x))
  #else
    #define PEEKB(s,o)    peekb(s,o)
    #define PEEKW(s,o)    (WORD) peek(s,o)
    #define PEEKL(s,o)    (*((DWORD far*) MK_FP(s,o)))
    #define POKEB(s,o,x)  pokeb (s,o,x)
    #define POKEW(s,o,x)  poke (s,o,x)
    #define POKEL(s,o,x)  (*((DWORD far*) MK_FP(s,o)) = (DWORD)(x))
  #endif
#endif


/* Use Pharlap's definition of a DOS-memory address;
 *   segment in upper 16-bits, offset in lower 16-bits.
 */
#if !(DOSX & PHARLAP) && !defined(REALPTR) && defined(__MSDOS__)
  #define REALPTR  DWORD
#endif

#if (DOSX)
  #define FARCODE
  #define FARDATA
#else
  #define FARCODE  far
  #define FARDATA  far
#endif

/*
 * Macros and hacks depending on compiler.
 */

#if defined(__POCC__)
  #include <string.h>
  #include <stdlib.h>

#else
  #include <stdlib.h>
  #include <memory.h>
  #include <malloc.h>
#endif

#if defined(__HIGHC__)
  #include <string.h>

  #define max(a,b)        _max (a,b)          /* intrinsic functions */
  #define min(a,b)        _min (a,b)
  #define ENABLE()        _inline (0xFB)      /* sti */
  #define DISABLE()       _inline (0xFA)      /* cli */
#endif

/*
 * This section also includes support "clang-cl" with some exceptions.
 */
#if defined(_MSC_VER) && !defined(__POCC__)   /* "cl" and not "pocc -Ze" */
  #if (DOSX) && (DOSX != WINWATT) && (DOSX != PHARLAP)
    #error Microsoft-C and non-Win32 or non-Pharlap targets not supported
  #endif

  #if !(defined(_M_IX86) || defined(_M_IA64) || defined(_M_X64) || defined(_M_AMD64))
    #error Unsupported CPU-target
  #endif

  #undef cdecl

  #if (_MSC_VER <= 600)   /* A few exceptions for Quick-C <= 6.0 */
    #define NO_INLINE_ASM /* doesn't have built-in assembler */
    #define ENABLE()      _enable()
    #define DISABLE()     _disable()
    #define cdecl         _cdecl
  #else
 /* #pragma warning (disable:4103 4113 4229) */
 /* #pragma warning (disable:4024 4047 4761 4791) */

   /*
    * 4244: "'function': convertion from '__w64' to 'int', possible
    *        loss of data".
    * 4267: "'=': convertion from 'size_t' to 'int', possible
    *        loss of data".
    * 4312: 'type cast' : conversion from 'int' to 'void *' of greater size.
    * 4996: "The POSIX name for this item is deprecated. Instead, use
    *       the ISO C++ conformant name...".
    */
    #if (_MSC_VER >= 1500)  /* VC9+ */
      #pragma warning (disable:4090 4244 4267 4312 4996)
      #ifndef _USE_32BIT_TIME_T
/* !! #error Need to define "_USE_32BIT_TIME_T" for ABI compatability. No need? */
      #endif
    #endif

    #if !defined(__cplusplus)  /* Included via stkwalk.cpp */
      #define ENABLE()      __asm sti
      #define DISABLE()     __asm cli
      #define asm           __asm
      #define cdecl         __cdecl
    #endif
  #endif

  /* Select what compiler built-in instructions to enable.
   */
  #if (_MSC_VER >= 800)
    #pragma intrinsic (abs, memcmp, memcpy, memset)
    #pragma intrinsic (strcat, strcmp, strcpy, strlen)
    #if (DOSX == 0)
      #pragma intrinsic (_fmemcpy)

    #elif (_MSC_VER >= 1500) && !defined(__clang__)
      #include <intrin.h>

      #pragma intrinsic   (__cpuid)
      #pragma intrinsic   (_mm_popcnt_u32)
      #define HAVE_POPCOUNT

      #if defined(_M_X64)  /* 64-bit compiler */
        #pragma intrinsic (_mm_popcnt_u64)
      #endif
    #endif
  #endif

  #if !defined(__i386__) && defined(_M_IX86)
    #define __i386__
  #endif

  #if (DOSX == 0)
    #define interrupt     _interrupt far
    #define getvect(a)    _dos_getvect (a)
    #define setvect(a,b)  _dos_setvect (a, b)
    #undef  MK_FP
    #define MK_FP(s,o)    (void _far*) (((DWORD)(s) << 16) + (DWORD)(o))
  #endif
#endif  /* _MSC_VER && !__POCC__ */


#if defined(__TURBOC__) || defined(__BORLANDC__)
  #if defined(__DPMI16__)
    #error 16-bit DPMI version not supported
  #endif

  #include <string.h>

  #if !defined(WIN32)
    #define ENABLE()      __emit__(0xFB)
    #define DISABLE()     __emit__(0xFA)
  #endif

  #pragma warn -bbf-     /* "Bitfields must be signed or unsigned int" warning */
  #pragma warn -sig-     /* "Conversion may loose significant digits" warning */
  #pragma warn -cln-     /* "Constant is long" warning */

  #if defined(__DPMI32__)
    #pragma warn -stu-   /* structure undefined */
    #pragma warn -aus-   /* assigned a value that is never used */
  #endif

  /* make string/memory functions inline (but not for DPMI32/WIN32)
   */
  #if defined(__BORLANDC__) && !defined(__DPMI32__) && !defined(WIN32)
    #define strlen        __strlen__
    #define strncpy       __strncpy__
    #define strrchr       __strrchr__
    #define strncmp       __strncmp__
    #define memset        __memset__
    #define memcpy        __memcpy__
    #define memcmp        __memcmp__
    #define memchr        __memchr__
  #endif
#endif    /* __TURBOC__ || __BORLANDC__ */


#if defined(__GNUC__)
  #if (__GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 1))
    #error I need GCC 3.1 or later
  #endif
  #include <string.h>

  #ifndef __fastcall
  #define __fastcall      __attribute__((__fastcall__))   /* unsupported in gcc/djgpp */
  #endif

  #ifndef _fastcall
  #define _fastcall       __attribute__((__fastcall__))
  #endif

  #if !(DOSX & WINWATT)
    #define ENABLE()      __asm__ __volatile__ ("sti")
    #define DISABLE()     __asm__ __volatile__ ("cli")
  #endif
#endif

#if defined(__WATCOMC__)
  #include <dos.h>
  #include <string.h>

  #if !defined(_M_IX86)
  #error Only Watcom x86 supported
  #endif

  /* Code inside "W32_NO_8087" is meant to be excluded on a 8086 PC.
   * Since they don't have a math processor, we should prevent the math
   * emulation library to be linked in. The define "__SW_0" is set by
   * "wcc -0".
   */
  #if defined(__SW_0) && !defined(__SW_FPI87)
    #define W32_NO_8087
  #endif

  #pragma intrinsic (strcmp, memset)
  #pragma warning (disable:120)
  #if (__WATCOMC__ >= 1220)  /* OW 1.2+ */
  /* #pragma warning (disable:H3006 H3007) */
  #endif

  #if defined(__MSDOS__)
    #define getvect(a)      _dos_getvect (a)
    #define setvect(a,b)    _dos_setvect (a, b)
    #define BOOL            int

    extern void ENABLE  (void);
    extern void DISABLE (void);
    #pragma aux ENABLE  = 0xFB;
    #pragma aux DISABLE = 0xFA;
  #endif
#endif

#if defined(WIN32) || defined(WIN64)
  extern CRITICAL_SECTION _watt_crit_sect;

  #if defined(__LCC__)
    #define ENTER_CRIT()  EnterCriticalSection ((struct _CRITICAL_SECTION*)&_watt_crit_sect)
    #define LEAVE_CRIT()  LeaveCriticalSection ((struct _CRITICAL_SECTION*)&_watt_crit_sect)
  #else
    #define ENTER_CRIT()  EnterCriticalSection (&_watt_crit_sect)
    #define LEAVE_CRIT()  LeaveCriticalSection (&_watt_crit_sect)
  #endif

  #undef  DISABLE
  #undef  ENABLE
  #define DISABLE()  ENTER_CRIT()
  #define ENABLE()   LEAVE_CRIT()

#else

  #if defined(__DMC__)
    #include <dos.h>
    #include <dpmi.h>
    #include <string.h>

    #define ENABLE()        enable()
    #define DISABLE()       disable()

  #elif defined(__CCDL__)
    #include <string.h>
    #include <dos.h>

    #define cdecl           _cdecl
    #define BOOL            int
    #define ENABLE()        asm sti
    #define DISABLE()       asm cli
  #endif

  #define ENTER_CRIT()    ((void)0)
  #define LEAVE_CRIT()    ((void)0)
#endif  /* WIN32 || WIN64 */


#if !defined(cdecl) && defined(__GNUC__) && (defined(__MINGW32__) || defined(__CYGWIN__))
#define cdecl __attribute__((__cdecl__))
#endif

#ifndef cdecl
#define cdecl
#endif

#if (DOSX & (DOS4GW|X32VM))
#define HAVE_DOS_NEAR_PTR   /* Can use 32-ptr to access DOS-mem directly */
#endif

#ifndef max
#define max(a,b)   (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)   (((a) < (b)) ? (a) : (b))
#endif

#if (W32_GCC_VERSION >= 40410)
  #define HAVE_POPCOUNT
#endif

/*
 * C-99 (?) __FUNCTION__
 */
#if defined(__LCC__) || defined(__POCC__)
  #define __FUNCTION__  __func__

#elif (defined(_MSC_VER) && (_MSC_VER >= 1300)) || defined(__DMC__)
  /* __FUNCTION__ is built-in */

#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
  #define __FUNCTION__  __func__

#elif !defined(__GNUC__) && !defined(__WATCOMC__)
  #define __FUNCTION__  "func?"
#endif

/*
 * Some checks for Borland on Windows
 */
#if defined(__DOS_H) && defined(__BORLANDC__) && defined(WIN32)
  #error Do not include <dos.h> for Borland/WIN32
#endif

#endif  /* _w32_TARGET_H */

