/*!\file sys/w32api.h
 *
 * Watt-32 API decoration for Win32 targets.
 */
#ifndef __SYS_W32API_H
#define __SYS_W32API_H

#if !defined(_WATT32_FAKE_WINSOCK_H) && (defined(_WINSOCK_H) || defined(_WINSOCKAPI_))
  #error Never include the real <winsock.h> in Watt-32 programs.
  #error Change your include-path so the fake <winsock.h> gets included first.
  #error E.g. add "-I$(WATT_ROOT)/inc -I$(WATT_ROOT)/inc/w32-fakes" to your CFLAGS.
#endif

#if !defined(_WATT32_FAKE_WINSOCK2_H) && (defined(_WINSOCK2_H) || defined(_WINSOCK2API_))
  #error Never include the real <winsock2.h> in Watt-32 programs.
  #error Change your include-path so the fake <winsock2.h> gets included first.
  #error E.g. add "-I$(WATT_ROOT)/inc -I$(WATT_ROOT)/inc/w32-fakes" to your CFLAGS.
#endif

#if !defined(_WATT32_FAKE_WS2TCPIP_H) && defined(_WS2TCPIP_H)
  #error Never include the real <ws2tcpip.h> in Watt-32 programs.
  #error Change your include-path so the fake <ws2tcpip.h> gets included first.
  #error E.g. add "-I$(WATT_ROOT)/inc -I$(WATT_ROOT)/inc/w32-fakes" to your CFLAGS.
#endif

/**
 * Namespace prefix "_w32_".
 *
 * Until C compilers support C++ namespaces, we use this
 * prefix for our namespace.
 */
#if defined(WATT32_NO_NAMESPACE)
  #undef  W32_NAMESPACE
  #define W32_NAMESPACE(x)  x
#else
  #ifndef W32_NAMESPACE
  #define W32_NAMESPACE(x)  _w32_ ## x
  #endif
#endif

/*
 * A C preprocess test using TDM-gcc with djgpp headers.
 */
#if defined(WATT32_DJGPP_MINGW) && 0
  #undef  WIN32
  #undef _WIN32
  #undef  WIN64
  #undef _WIN64

  #undef __MINGW32__
  #undef __MINGW32_VERSION
  #undef __MINGW32_MAJOR_VERSION
  #undef __MINGW32_MINOR_VERSION
  #undef __MINGW64_VERSION_MAJOR
  #undef __MINGW64_VERSION_MINOR
#endif

/*
 * Watt-32 should never assume WIN32/WIN64 (or _WIN32/_WIN64) is set for
 * CygWin. It is not defined unless you include <windows.h> first (which
 * we only do in 1 place. See below).
 */
#if (defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)) || \
    defined(__CYGWIN__)

  #if defined(__DJGPP__)
  // #error WIN32 defined for Djgpp?
  #endif

  /* Don't include the real <winsock*.h>
   */
  #define _WINSOCKAPI_      /* MSVC/DMC/Watcom/Borland/CygWin header guard names */
  #define _WINSOCK2API_
  #define _WINSOCK_H        /* MinGW header guard names */
  #define _WINSOCK2_H

  #ifndef WATT32_ON_WINDOWS
  #define WATT32_ON_WINDOWS
  #endif

  /* Needed here since CygWin's <errno.h> gets included in <windows.h> below.
   * Otherwise the shadow "../inc/errno.h" would cause 'W32_DATA' etc. never
   * to be defined before use in "../inc/werrno.h". A bit of a mess, but
   * needed since CygWin's errnos must be handled specially. I.e. CygWin
   * have all needed errnos we need. Thus there is no need for a special
   * ../util/cygwin_err.exe program or a $(WATT_ROOT)/inc/sys/cygwin.err
   * file.
   */
  #if defined(__CYGWIN__)
    #include_next <sys/errno.h>
  #endif
#endif


/*
 * For all Windows targets, we export functions and data prefixed with
 * 'W32_FUNC' or 'W32_DATA'. Unless we use the static Watt-32 library
 * (./lib/libwatt32.a). Using the static lib requires that the user to
 * add '-DWATT32_STATIC' to the CFLAGS.
 *
 * For non-Win32 targets the .\util\mkimp program (a small C-preprocessor)
 * is meant to search all headers for W32_FUNC/W32_DATA prefixes. All
 * functions with a W32_FUNC prefix will produce an export stub function.
 * See dj_dxe.mak. Very experimental at the moment.
 *
 * \note: only a small subset of the Winsock extensions are implemented in
 *        watt-32.dll (hardly any WSA*() functions yet).
 *
 * \note: 'W32_CALL' is something else. It defines calling-convention
 *        Ref. <sys/cdefs.h>.
 */
#if defined(WATT32_ON_WINDOWS) && !defined(WATT32_STATIC)
  #if defined(WATT32_BUILD)
    #define W32_FUNC  extern __declspec(dllexport)
    #define W32_DATA  extern __declspec(dllexport)
  #else
    #define W32_FUNC  extern __declspec(dllimport)
    #define W32_DATA  extern __declspec(dllimport)
  #endif
#else
  #define W32_FUNC  extern
  #define W32_DATA  extern
#endif

/*
 * W32_CALL is defined to `__cdecl' to ease the interface
 * to OpenWatcom which uses register calls (-3r) as default.
 *
 * So it should be possible to use e.g. a MSVC linked watt-32.dll
 * with a Watcom / MinGW / CygWin compiled program.
 */
#if defined(WATT32_ON_WINDOWS)
  #define W32_CALL __cdecl
#else
  #define W32_CALL
#endif

#if defined(WATT32_ON_WINDOWS)
  /*
   * This should be the only place <windows.h> is included for Windows targets.
   * Due to the organisation of the CygWin headers, <windows.h> must be here.
   */
  #include <windows.h>
#endif

/*
 * CygWin hacks:
 *   Seems no longer a need for this.
 */
#if defined(__CYGWIN__) && defined(WATT32_BUILD) && 0
  #if !defined(WIN32) && defined(__i386__)
    #error "WIN32 should only be defined in wattcp.h included before this point."

  #elif !defined(WIN64) && defined(__x86_64__)
    #error "WIN64 should only be defined in wattcp.h included before this point."
  #endif

  #include <sys/time.h>
#endif

#endif  /* __SYS_W32API_H */

