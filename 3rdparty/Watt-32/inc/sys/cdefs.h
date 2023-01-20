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
   *    "w "../../inc/sys/cdefs.h",L59/C23(#491):  Unrecognized preprocessor directive:"
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

/*
 * Gets the 'gcc' version as 'MmmP' where:
 *   'M'  = major: >= 3.
 *   'mm' = minor: 00 - 99.
 *   'P'  = patch-level.
 *
 * gcc >= 3.1 is assumed throughout the Watt-32 library.
 */
#if defined(__GNUC__)
  #define W32_GCC_VERSION  (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + \
                            __GNUC_PATCHLEVEL__)
#else
  #define W32_GCC_VERSION  0
#endif

/*
 * Macros for 'gcc / clang-cl' features.
 */
#if defined(__GNUC__) || defined(__clang__)
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
  #define W32_ATTR_PRINTF(_1,_2)
  #define W32_ATTR_SCANF(_1,_2)
  #define W32_ATTR_NOINLINE()
  #define W32_ATTR_REGPARM(_n)
  #define W32_ATTR_NORETURN()
  #define W32_ARG_NONNULL(_1)
#endif

/* Warn on deprecated type/func.
 */
#if defined(__GNUC__) || defined(__clang__)
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
  #define W32_CDECL _cdecl

#elif defined(__MINGW32__) || defined(__CYGWIN__)
  #if defined(cdecl)
    #define W32_CDECL cdecl
  #else
    #define W32_CDECL __attribute__((__cdecl__))
  #endif
#endif

#if defined(_MSC_VER) || defined(__POCC__)
  #if (_MSC_VER <= 600) && !defined(__POCC__)
    #define W32_CDECL _cdecl
    #undef  __STDC__
    #define __STDC__ 1
  #else
    #define W32_CDECL __cdecl
  #endif
  #define NO_ANSI_KEYWORDS

#elif defined(__DMC__)    /* Digital Mars compiler */
  #define NO_UNDERSCORE __syscall
  /* e.g. int NO_UNDERSCORE foo (void); */
#endif

#ifndef W32_CDECL
#define W32_CDECL
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
 */
#if defined(__MINGW32__)
  #include <_mingw.h>
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
   *
   * See <sys/w32api.h> for the rationale behind 'W32_CALL'. It's
   * currently forced to 'cdecl' on Windows. Thus 'MS_CDECL' should be
   * important for MSVC users on Windows only. But I'm not sure....
   */
  #undef  MS_CDECL
  #define MS_CDECL   W32_CDECL
#else
  #define MS_CDECL
#endif

#if !defined(__AVR__)
/*
 * The __CONCAT macro is used to concatenate parts of symbol names, e.g.
 * with "#define OLD(foo) __CONCAT(old,foo)", OLD(foo) produces oldfoo.
 * The __CONCAT macro is a bit tricky -- make sure you don't put spaces
 * in between its arguments.  __CONCAT can also concatenate double-quoted
 * strings produced by the __STRING macro, but this only works with ANSI C.
 */
#if ((defined(__STDC__) && __STDC__) || defined(__cplusplus) || \
    defined(__TURBOC__)) && !defined(__CYGWIN__)
  #define __CONCAT(x,y)   x ## y
  #define __STRING(x)     #x

  #if defined(__cplusplus)
    #define __inline      inline        /* convert to C++ keyword */
  #elif !defined(__GNUC__) && !defined(_MSC_VER) && !defined(__WATCOMC__)
    #define __inline                    /* delete GCC/MSC/Watcom keyword */
  #endif

#else
  #define __CONCAT(x,y)   x/**/y
  #define __STRING(x)     "x"

  #if !defined(__GNUC__) && !defined(_MSC_VER) && !defined(__WATCOMC__)
    #define __inline
  #endif

  #if !defined(__GNUC__)
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
#endif    /* !__AVR__ */


/*
 * Macros for gcc 4.6+ Pragmas. Ref.:
 * http://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html
 */
#if (W32_GCC_VERSION >= 40600) || defined(__clang__) || defined(__ORANGEC__)
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
    #define W32_CLANG_PACK_WARN_DEF() W32_GCC_PRAGMA (clang diagnostic pop)
  #endif

#else
  #define W32_CLANG_NONNULL(x) 0
#endif

/*
 * A simple compile-time assert() macro.
 * Use it like 'W32_COMPILE_TIME_ASSERT (some_typedef, sizeof(some_typedef) == 10);'
 *
 * If 'sizeof(some_typedef) != 10' (i.e. 'x == 0'), it expands to
 * 'typedef int w32_compile_time_assert_some_typedef [-1];' which will not compile.
 */
#define W32_COMPILE_TIME_ASSERT(name, x) \
        typedef int w32_compile_time_assert_ ## name [(x) * 2 - 1]


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
    #define min(a, b) _min(a, b)  /* intrinsic functions */
    #define max(a, b) _max(a, b)
  #endif

  #ifndef min
  #define min(a, b) (((a) < (b)) ? (a) : (b))
  #endif

  #ifndef max
  #define max(a, b) (((a) > (b)) ? (a) : (b))
  #endif
#endif

#endif
