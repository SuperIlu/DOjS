/*!\file sys/werrno.h
 *
 * sys_errlist[] and errno's for compilers with limited errnos.
 * For WIN32, we do NOT use the <winsock.h> WSAE* codes.
 *
 * G. Vanem <gvanem@yahoo.no> 1998 - 2017
 */

#ifndef __SYS_WERRNO_H
#define __SYS_WERRNO_H

#ifndef __SYS_W32API_H
#include <sys/w32api.h>
#endif

#ifndef __SYS_CDEFS_H
#include <sys/cdefs.h>
#endif

/* When doing "gcc -MM" with gcc 3.0 we must include <sys/version.h>
 * (via stdio.h) in order for __DJGPP__ to be defined
 */
#include <stdio.h>
#include <errno.h>

#if defined(W32_NEW_ERRNO)
  #include <sys/werrno2.h>

#else  /* rest of file */

/* Hack: fix for compiling with djgpp 2.04, but
 *       ./util/dj_err.exe was compiled with 2.03
 */
#if defined(MIXING_DJGPP_203_AND_204)
#undef ELOOP
#endif

/*
 * Just a C preprocess test using TDM-gcc with djgpp headers.
 */
#if defined(WATT32_DJGPP_MINGW)
  #ifndef __DJGPP__
  #define __DJGPP__ 2
  #endif
  #include <sys/djgpp.err>

  /*
   * '__MINGW32__' is defined by BOTH mingw.org and by the MinGW-w64
   * projects [1,2], because both can target Win32.
   * 'W32_IS_MINGW64' is defined in <sys/cdefs.h> when targeting these.
   *
   * '__MINGW64__' is defined only when targeting Win64 (__x86_64__,
   * or option '-m64').
   *
   * [1] http://mingw-w64.sourceforge.net/
   * [2] http://tdm-gcc.tdragon.net/
   *
   * Hopefully both Win32 and Win64 targets define the same range of
   * errnos. Hence we use the one generated for '__MINGW64__'.
   */
#elif defined(W32_IS_MINGW64) || defined(__MINGW64__)
  #undef EDEADLOCK
  #include <sys/mingw64.err>

#elif defined(__MINGW32__)    /* Original from mingw.org */
  #include <sys/mingw32.err>

#elif defined(__HIGHC__)
  #undef EDEADLK
  #undef EDEADLOCK
  #include <sys/highc.err>

#elif defined(__BORLANDC__)
  #ifdef __FLAT__
  #undef ENAMETOOLONG  /* bcc32 4.0 */
  #endif
  #ifdef _WIN32
  #undef ENOTEMPTY
  #endif
  #include <sys/borlandc.err>

#elif defined(__TURBOC__)
  #include <sys/turboc.err>

#elif defined(__WATCOMC__)
  #include <sys/watcom.err>

#elif defined(__DJGPP__)
  #include <sys/djgpp.err>

#elif defined(__DMC__)                        /* Digital Mars Compiler */
  #include <sys/digmars.err>

#elif defined(__CYGWIN__)
  /*
   * Since CygWin's <sys/errno.h> provides all the errno-values
   * we need, there is no need to use util/errnos.c to create new
   * ones for CygWin. Simply pull in <sys/errno.h>. Done by
   * #include_next <sys/errno.h> in our '<sys/errno.h>'.
   */
#elif defined(__clang__)
  /*
   * MUST be include before '_MSC_VER' section since 'clang-cl'
   * emulates 'cl.
   */
  #undef EDEADLOCK
  #include <sys/clang.err>

#elif defined(_MSC_VER) && (_MSC_VER > 800)   /* Visual C on Windows */
  #undef EDEADLOCK
  #include <sys/visualc.err>

#elif defined(__CCDL__)                       /* LadSoft's cc386.exe */
  #include <sys/ladsoft.err>

#elif defined(__LCC__)
  #include <sys/lcc.err>

#elif defined(__POCC__)
  #include <sys/pellesc.err>

#else
  #error Unknown target in <sys/werrno.h>.
#endif

/*
 * Ugly hack ahead. Someone tell me a better way, but
 * errno and friends are macros on Windows. Redefine them
 * to point to our variables.
 *
 * On Windows, the usual 'errno' is a macro "(*_errno)()" that
 * is problematic to use as a lvalue.
 * On other platforms we modify the global 'errno' variable directly.
 * (see SOCK_ERRNO() in src/misc.h). So no need to redefine it in any way.
 */

W32_DATA int   _w32_errno;
W32_DATA int   _w32_sys_nerr;
W32_DATA char *_w32_sys_errlist[];

#if 0
  #undef  sys_nerr
  #define sys_nerr      _w32_sys_nerr

  #undef  _sys_nerr
  #define _sys_nerr     _w32_sys_nerr

  #undef  __sys_nerr
  #define __sys_nerr    _w32_sys_nerr

  #undef  sys_errlist
  #define sys_errlist   _w32_sys_errlist

  #undef  _sys_errlist
  #define _sys_errlist  _w32_sys_errlist

  #undef  __sys_errlist
  #define __sys_errlist _w32_sys_errlist
#endif

/*
 * Incase you have trouble with duplicate defined symbols,
 * make sure the "*_s()" versions are referenced before normal
 * perror() and strerror() in your C-library.
 *
 * The Windows SDK already have a 'strerror_s()'.
 * Hence the 'strerror_s_()'.
 */
W32_FUNC void  W32_CALL perror_s    (const char *str);
W32_FUNC char *W32_CALL strerror_s_ (int errnum);

#if defined(__WATCOMC__) && (__WATCOMC__ >= 1250)  /* OW 1.5+ */
  #define W32_OPENWATCOM_15
#endif

#if defined(__cplusplus)
  #include <stdlib.h>
  #include <string.h>

#elif !defined(_MSC_VER) && !defined(WIN32) && !defined(W32_OPENWATCOM_15)
  #if !defined(_INC_STDLIB) && !defined(_STDLIB_H_) && !defined(_STDIO_H)
  W32_FUNC void W32_CALL perror (const char *str);
  #endif

  #if !defined(_INC_STRING) && !defined(_STRING_H) && !defined(_STRING_H_)
  W32_FUNC char *W32_CALL strerror (int errnum);
  #endif
#endif

#if defined(WATT32_ON_WINDOWS)
  W32_FUNC int  __stdcall WSAGetLastError (void);
  W32_FUNC void __stdcall WSASetLastError (int err);
#endif

#endif  /* W32_NEW_ERRNO */
#endif  /* __SYS_WERRNO_H */

