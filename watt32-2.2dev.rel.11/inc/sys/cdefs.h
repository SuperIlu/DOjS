/*!\file sys/cdefs.h
 *
 * C-compiler definitions. Ref for a long list of compiler built-in:
 *   https://sourceforge.net/p/predef/wiki/Compilers/
 */

/*
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Berkeley Software Design, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *   @(#)cdefs.h  8.7 (Berkeley) 1/21/94
 */

#ifndef __SYS_CDEFS_H
#define __SYS_CDEFS_H

#if defined(__HIGHC__)
  /*
   * Note to High-C users:
   *   The below pragma (to suppress warning for '#include_next' line) has no effect
   *   here. The pragma needs to be put in your CFLAGS as
   *   '-Hpragma=Offwarn(491)'.
   *
   *   The warning w/o this pragma is something like:
   *    "w "../../inc/sys/cdefs.h",L60/C23(#491):  Unrecognized preprocessor directive:"
   */
#endif

#if defined(__DJGPP__) || defined(__CYGWIN__) || defined(__MINGW64__)
  #include_next <sys/cdefs.h>

#else
  #if defined(__cplusplus)
    #define __BEGIN_DECLS  extern "C" {
    #define __END_DECLS    }
  #else
    #define __BEGIN_DECLS
    #define __END_DECLS
  #endif
#endif


#if defined(__GNUC__)
  #define W32_GCC_VERSION  (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + \
                            __GNUC_PATCHLEVEL__)
#else
  #define W32_GCC_VERSION  0
#endif

/*
 * Macros for gcc 2.5+ features.
 */
#if (W32_GCC_VERSION >= 20500)
  #if (W32_GCC_VERSION >= 50100)    /* correct? */
    #define W32_ATTR_PRINTF(_1,_2)  __attribute__ ((__format__(gnu_printf,_1,_2)))
    #define W32_ATTR_SCANF(_1,_2)   __attribute__ ((__format__(gnu_scanf,_1,_2)))
  #else
    #define W32_ATTR_PRINTF(_1,_2)  __attribute__ ((format(printf,_1,_2)))
    #define W32_ATTR_SCANF(_1,_2)   __attribute__ ((format(scanf,_1,_2)))
  #endif

  #define W32_ATTR_NOINLINE()       __attribute__ ((__noinline__))     /* do not inline this func */
  #define W32_ATTR_REGPARM(_n)      __attribute__ ((regparm(_n)))      /* parameters are in registers. */
  #define W32_ATTR_NORETURN()       __attribute__ ((noreturn))         /* function never returns */
  #define W32_ARG_NONNULL(_1)       __attribute__ ((nonnull(_1)))      /* argument cannot be NULL */
#else
  #if defined(__clang__)
    #define W32_ATTR_PRINTF(_1,_2)  __attribute__ ((format(printf,_1,_2)))
    #define W32_ATTR_SCANF(_1,_2)   __attribute__ ((format(scanf,_1,_2)))
  #else
    #define W32_ATTR_PRINTF(_1,_2)
    #define W32_ATTR_SCANF(_1,_2)
  #endif

  #define W32_ATTR_NOINLINE()
  #define W32_ATTR_REGPARM(_n)
  #define W32_ATTR_NORETURN()
  #define W32_ARG_NONNULL(_1)
#endif

/* Warn on deprecated type/func
 */
#if (W32_GCC_VERSION >= 30100)
  #define W32_ATTR_DEPRECATED    __attribute__((__deprecated__))
#else
  #if defined(_MSC_VER) && (_MSC_VER >= 1700)  /* Visual Studio 2011+ ? */
    #define W32_ATTR_DEPRECATED  __declspec(deprecated)
  #else
    #define W32_ATTR_DEPRECATED
  #endif
#endif

/* Detection of the various Borland/CodeGearC/CBuilder versions.
 * Not sure about the '0x0700' value.
 *
 * Refs: https://sourceforge.net/p/predef/wiki/Compilers/
 *       https://en.wikipedia.org/wiki/C%2B%2BBuilder
 */
#if defined(__BORLANDC__)
  #if (__BORLANDC__ >= 0x0700)
  //#pragma message ("I am Embarcadero")
    #define W32_IS_EMBARCADERO 1
  #endif

  #if defined(__CODEGEARC__)
  //#pragma message ("I am CodeGearC")
    #define W32_IS_CODEGEARC 1
  #endif
#endif

#if defined(__CCDL__)    /* LadSoft compiler */
  #define cdecl _cdecl

#elif !defined(cdecl) && (defined(__MINGW32__) || defined(__CYGWIN__))
  #define cdecl __attribute__((__cdecl__))
#endif

#if defined(_MSC_VER) || defined(__POCC__)
  #undef cdecl
  #if (_MSC_VER <= 600) && !defined(__POCC__)
    #define cdecl _cdecl
    #undef  __STDC__
    #define __STDC__ 1
  #else
    #define cdecl __cdecl
  #endif
  #define NO_ANSI_KEYWORDS

#elif defined(__DMC__)    /* Digital Mars compiler */
  #define NO_UNDERSCORE __syscall
  /* e.g. int NO_UNDERSCORE foo (void); */
#endif

#ifndef cdecl
#define cdecl
#endif

/*
 * MinGW-w64 defines both __MINGW64__ *and* __MINGW32__, but __MINGW64__
 * is only defined for _WIN64 targets. __MINGW64_VERSION_[MAJOR|MINOR]
 * is defined in MinGW64's <_mingw.h>.
 *
 * MinGW-w64. I.e. "GCC for both x64 & x86 Windows".
 * This is not from the mingw.org folks, but from these:
 *   http://mingw-w64.sourceforge.net/
 *
 * Or to make this even more confusing, the TDM-gcc should act the same
 * as MinGW-w64:
 *   http://tdm-gcc.tdragon.net/
 *
 * It uses it's own versioning.
 *
 * Using the "official" 32-bit MinGW with Watt-32, the W32_MINGW_VER() macro
 * tests for some structs and prototypes added in MinGW 3.1 (?). For newer
 * MinGW-[32|64] versions, we just assume e.g. 'struct timezone' etc. are
 * available.
 */
#if !defined(__MINGW32__)
  #define W32_MINGW_VER(major,minor)  0
#else
  #include <_mingw.h>

  #if defined(__MINGW64_VERSION_MAJOR)
    /*
     * '__MINGW64_VERSION_[MAJOR|MINOR]' is defined in _mingw.h in the
     * MinGW-w64 project.
     * Not a typo; MinGW-64 doesn't define __MINGW64_[MAJOR|MINOR]_VERSION
     */
    #define W32_MINGW_VER(major,minor)  1
    #define W32_IS_MINGW64              1

  #elif defined(__MINGW_MAJOR_VERSION)
    /*
     * '__MINGW_MAJOR_VERSION' is defined through _mingw.h in MinGW-RT v4+.
     */
    #define W32_MINGW_VER(major,minor)  1

  #else /* plain old mingw.org (prior to v4.0): */
    #define W32_MINGW_VER(major,minor) (__MINGW32_MAJOR_VERSION > (major) ||   \
                                        (__MINGW32_MAJOR_VERSION == (major) && \
                                         __MINGW32_MINOR_VERSION >= (minor)))
  #endif
#endif

#if defined(_MSC_VER) && !defined(__POCC__)
  /*
   * All MS compilers (Quick-C/Visual-C for DOS, VC for Windows) insists
   * that signal-handlers, atexit functions and var-arg functions must be
   * defined as cdecl. This is only an issue if a program is using 'fastcall'
   * globally (cl option '-Gr').
   * Ref. the use of 'WINAPIV' in the Windows SDKs.
   *
   * But Watcom's register call doesn't need this. (wcc386 -ecf)
   */
  #undef  MS_CDECL
  #define MS_CDECL   cdecl

  /* See <sys/w32api.h> for the rationale behind 'W32_CALL'. It's
   * currently forced to 'cdecl' on Windows. Thus 'W32_CDECL' and 'MS_CDECL'
   * should be important for MSVC users on Windows only. But I'm not
   * sure....
   */
  #define W32_CDECL  cdecl
#else
  #define MS_CDECL
  #define W32_CDECL
#endif


/*
 * The __CONCAT macro is used to concatenate parts of symbol names, e.g.
 * with "#define OLD(foo) __CONCAT(old,foo)", OLD(foo) produces oldfoo.
 * The __CONCAT macro is a bit tricky -- make sure you don't put spaces
 * in between its arguments.  __CONCAT can also concatenate double-quoted
 * strings produced by the __STRING macro, but this only works with ANSI C.
 */
#if ((defined(__STDC__) && __STDC__) || defined(__cplusplus) || \
    defined(__TURBOC__)) && !defined(__CYGWIN__)
  #undef  __P
  #define __P(protos)     protos        /* full-blown ANSI C */
  #define __CONCAT(x,y)   x ## y
  #define __STRING(x)     #x

  #define __const         const         /* define reserved names to standard */
  #define __signed        signed
/*#define __volatile      volatile */
  #if defined(__cplusplus)
    #define __inline      inline        /* convert to C++ keyword */
  #elif !defined(__GNUC__) && !defined(_MSC_VER) && !defined(__WATCOMC__)
    #define __inline                    /* delete GCC/MSC/Watcom keyword */
  #endif

#else
  #define __P(protos)     ()            /* traditional C preprocessor */
  #define __CONCAT(x,y)   x/**/y
  #define __STRING(x)     "x"

  #if !defined(__GNUC__) && !defined(_MSC_VER) && !defined(__WATCOMC__)
    #define __inline
  #endif

  #if !defined(__GNUC__)
    #define __const                     /* delete pseudo-ANSI C keywords */
    #define __signed
 /* #define __volatile */

    /*
     * In non-ANSI C environments, new programs will want ANSI-only C keywords
     * deleted from the program and old programs will want them left alone.
     * When using a compiler other than gcc, programs using the ANSI C keywords
     * const, inline etc. as normal identifiers should define -DNO_ANSI_KEYWORDS.
     * When using "gcc -traditional", we assume that this is the intent; if
     * __GNUC__ is defined but __STDC__ is not, we leave the new keywords alone.
     */
    #if !defined(NO_ANSI_KEYWORDS)
      #define const                     /* delete ANSI C keywords */
      #define inline
      #define signed
      #define volatile
    #endif
  #endif  /* !__GNUC__ */
#endif    /* (__STDC__ || __cpluplus) && !__CYGWIN__ */

/*
 * GCC1 and some versions of GCC2 declare dead (non-returning) and
 * pure (no side effects) functions using "volatile" and "const";
 * unfortunately, these then cause warnings under "-ansi -pedantic".
 * GCC2 uses a new, peculiar __attribute__((attrs)) style.  All of
 * these work for GNU C++ (modulo a slight glitch in the C++ grammar
 * in the distribution version of 2.5.5).
 */
#if (W32_GCC_VERSION > 0) && (W32_GCC_VERSION < 20500)
  #undef  __attribute__
  #define __attribute__(x)  /* delete __attribute__ if non-gcc */
  #if defined(__GNUC__) && !defined(__STRICT_ANSI__)
    #undef __dead
    #undef __pure
    #define __dead  __volatile
    #define __pure  __const
    #undef  __dead2
    #define __dead2
  #endif
#endif

/*
 * Macros for gcc 4.6+ Pragmas. Ref.:
 * http://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html
 */
#if (W32_GCC_VERSION >= 40600) || defined(__clang__)
  #define W32_GCC_PRAGMA(x)  _Pragma (#x)
#else
  #define W32_GCC_PRAGMA(x)
#endif

/*
 * The above could be used as:
 *   W32_GCC_PRAGMA (message: "Fix this")
 *
 * or:
 *  #if (W32_GCC_VERSION >= 40600)
 *    #pragma message ("Fix this")
 *  #endif
 *
 * But MSVC (and others besides High-C) doesn't handle a pragma in a
 * #define statement. Hence must enclose it in an #ifdef. Like:
 *
 * #if defined(_MSC_VER) && (_MSC_VER >= 1500)
 *   #pragma message ("Fix this")
 * #endif
 */


/*
 * gcc >= 4.2 have an issue with '__inline__'. We must ensure
 * that always traditional GNU extern inline semantics are used
 * (aka -fgnu89-inline) even if ISO C99 semantics have been specified.
 */
#if (W32_GCC_VERSION >= 40200) && !defined(__NO_INLINE__) /* -O0 */
  #define W32_GCC_INLINE extern __inline__ __attribute__ ((__gnu_inline__))
#else
  #define W32_GCC_INLINE extern __inline__
#endif

#if defined(__GNUC__)
  #define W32_INLINE W32_GCC_INLINE
#else
  #define W32_INLINE __inline
#endif

#define W32_CLANG_PACK_WARN_OFF()
#define W32_CLANG_PACK_WARN_DEF()

#if defined(__clang__)
  /*
   * To suppress clang warnings like:
   *   Array access (from variable 'x') results in a null pointer dereference
   *
   * in statements like:
   *   if (!x[0]) {
   *      ...
   *
   * So one could write it like:
   *   if (W32_CLANG_NONNULL(x) || !x[0]) {
   *      ...
   */
  #define W32_CLANG_NONNULL(x) !(x)

  #if (__clang_major__ >= 6)
    /*
     * clang v6.x introdused several new warnings. Especially annoying when including
     * <sys/pack_on.h> and <sys/pack_off.h> is this:
     *   the current #pragma pack aligment value is modified in the included file [-Wpragma-pack]
     *
     * Control these warnings using the below macros.
     * Always used when building *or* using Watt-32. Used like this:
     *
     *  W32_CLANG_PACK_WARN_OFF()  // "push" warning state and turn off "-Wpragma-pack".
     *  #include <sys/pack_on.h>
     *
     *  ... some structures that needs tight 1-byte packing of members.
     *
     *  #include <sys/pack_off.h>  // restore default packing
     *  W32_CLANG_PACK_WARN_DEF()  // "pop" the warning set by 'W32_CLANG_PACK_WARN_OFF()'.
     */
    #undef  W32_CLANG_PACK_WARN_OFF
    #define W32_CLANG_PACK_WARN_OFF() W32_GCC_PRAGMA (clang diagnostic push) \
                                      W32_GCC_PRAGMA (clang diagnostic ignored "-Wpragma-pack")

    #undef  W32_CLANG_PACK_WARN_DEF
    #define W32_CLANG_PACK_WARN_DEF()  W32_GCC_PRAGMA (clang diagnostic pop)
  #endif

#else
  #define W32_CLANG_NONNULL(x) 0
#endif


/*
 * Delete pseudo-keywords wherever they are not available or needed.
 * This seems to break MinGW in mysterious ways, so leave it.
 */
#ifndef __dead
#define __dead
#endif

#ifndef __dead2
#define __dead2
#endif

#ifndef __pure
#define __pure
#endif

#if !defined(__cplusplus)
  /*
   * min() & max() macros
   */
  #if defined(__HIGHC__)
    #undef  min
    #undef  max
    #define min(a,b) _min(a,b)  /* intrinsic functions */
    #define max(a,b) _max(a,b)
  #endif

  #ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
  #endif

  #ifndef max
  #define max(a,b) (((a) > (b)) ? (a) : (b))
  #endif
#endif

/*
 * from NetBSD's <sys/cdefs_aout.h>
 *
 * Written by J.T. Conklin <jtc@wimsey.com> 01/17/95.
 * Public domain.
 */

/* #define _C_LABEL(x) __CONCAT(_,x) */

#if defined(__GNUC__)
  #define __indr_reference(sym,alias)                \
          __asm__(".stabs \"_" #alias "\",11,0,0,0");\
          __asm__(".stabs \"_" #sym "\",1,0,0,0");

  #define __warn_references(sym,msg)                 \
          __asm__(".stabs \"" msg "\",30,0,0,0");    \
          __asm__(".stabs \"_" #sym "\",1,0,0,0");

  #define __IDSTRING(name,string)  \
          static const char name[] __attribute__((__unused__)) = string
#else
  #define __indr_reference(sym,alias)
  #define __warn_references(sym,msg)
  #define __IDSTRING(name,string)  static const char name[] = string
#endif

#define __RCSID(_s)                __IDSTRING(rcsid,_s)
#define __COPYRIGHT(_s)            __IDSTRING(copyright,_s)

#define __KERNEL_RCSID(_n,_s)      __IDSTRING(__CONCAT(rcsid,_n),_s)
#define __KERNEL_COPYRIGHT(_n,_s)  __IDSTRING(__CONCAT(copyright,_n),_s)

#endif
