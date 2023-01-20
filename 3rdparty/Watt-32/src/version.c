/*!\file version.c
 *
 * Return Watt-32 version/capabilities strings.
 */
#include <stdio.h>
#include <string.h>

#ifdef __CYGWIN__
#include <cygwin/version.h>
#endif

#include "wattcp.h"
#include "wdpmi.h"
#include "misc.h"
#include "misc_str.h"

#if defined(_WIN64)
  #if defined(_DEBUG)
    #define DBG_RELEASE  "debug"
  #else
    #define DBG_RELEASE  "release"
  #endif
  #undef  VARIANT
  #define VARIANT        " Win64"

#elif defined(_WIN32) || defined(WIN32)
  #if defined(_DEBUG)
    #define DBG_RELEASE  "debug"       /* Only with MSVC */
  #else
    #define DBG_RELEASE  "release"
  #endif
  #undef  VARIANT
  #define VARIANT        " Win32"

#else
  #define VARIANT   ""
#endif

#undef CC_DEFINE
#undef CFLAGS
#undef CFLAGS_BUF

const char *wattcpCopyright = "See COPYRIGHT.H for details";

#if defined(__GNUC__)
  const char *gcc_get_cpu_tune (void);

#elif defined(_MSC_VER) && (_MSC_VER >= 1000)    /* Visual-C 4+ */
  static const char *msvc_check_fastcall (void);
  static const char *msvc_get_patch_build (void);
#endif

/**
 * Return string for Watt-32 version.
 * Include this:
 *  - Name and version of compiler used to build Watt-32.
 *  - Target environment (if any).
 *  - Build date.
 *
 * E.g. \b
 *   Watt-32 (2.2.11 Win32), GNU-C 4.5.3, CygWin 1.7.16, Aug 31 2012
 *   \b
 */
const char * W32_CALL wattcpVersion (void)
{
  static char buf[100];
  char  *p = buf;

#if defined(WATTCP_DEVEL_REL) && (WATTCP_DEVEL_REL > 0)
  p += sprintf (p, "Watt-32 (%d.%d.%d",
                WATTCP_MAJOR_VER, WATTCP_MINOR_VER, WATTCP_DEVEL_REL);
#else
  p += sprintf (p, "Watt-32 (%d.%d",
                WATTCP_MAJOR_VER, WATTCP_MINOR_VER);
#endif

  p += sprintf (p, "%s), ", VARIANT);

#if defined(__GNUC__)
  p += sprintf (p, "GNU-C %d.%d", __GNUC__, __GNUC_MINOR__);

  #if defined(__GNUC_PATCHLEVEL__) && (__GNUC_PATCHLEVEL__ > 0)
    p += sprintf (p, ".%d", __GNUC_PATCHLEVEL__);
  #endif

  strcpy (p, gcc_get_cpu_tune());

#elif defined(__HIGHC__)
  p += sprintf (p, "Metaware High-C, ");   /* no way to get version */

#elif defined(__POCC__)
  p += sprintf (p, "PellesC %d.%d, ", __POCC__/100, __POCC__ % 100);

#elif defined(__WATCOMC__)
  #if (__WATCOMC__ >= 1200)
    p += sprintf (p, "OpenWatcom %d.%d",
                  (__WATCOMC__/100) - 11, (__WATCOMC__ % 100) / 10);
  #else
    p += sprintf (p, "Watcom C %d.%d", __WATCOMC__/100, __WATCOMC__ % 100);
  #endif

  #if !defined(__FLAT__)
    #if defined(__I86__)
      strcpy (p, " (16-bit");
    #else
      strcpy (p, " (32-bit");
    #endif
    p += 8;
    #if defined(__SMALL__)
      strcpy (p, " small model),");
      p += 14;
    #elif defined(__LARGE__)
      strcpy (p, " large model),");
      p += 14;
    #else
      strcpy (p, "),");
      p += 2;
    #endif
  #endif
  #if (_M_IX86 >= 600)    /* 32-bit can be >= 300 only */
    strcpy (p, " (686");
  #elif (_M_IX86 >= 500)
    strcpy (p, " (586");
  #elif (_M_IX86 >= 400)
    strcpy (p, " (486");
  #elif (_M_IX86 >= 300)
    strcpy (p, " (386");
  #elif (_M_IX86 >= 200)
    strcpy (p, " (286");
  #elif (_M_IX86 >= 100)
    strcpy (p, " (186");
  #else
    strcpy (p, " (8086");
    p++;
  #endif
  p += 5;
  #if defined(__I86__)      /* 16-bit Register based calls only */
    strcpy (p, "), ");
  #elif defined(__SW_3S)    /* 32-bit Stack based calls */
    strcpy (p, "S), ");
  #else
    strcpy (p, "R), ");     /* 32-bit Register based calls is default */
  #endif

#elif defined(__BORLANDC__)
  #if defined(__CODEGEARC_VERSION__)
    /*
     * http://docwiki.embarcadero.com/RADStudio/Tokyo/en/Predefined_Macros
     */
    p += sprintf (p, "C++Builder %X.%X.%d",
                  ((__CODEGEARC_VERSION__ & 0xFF000000) >> 24),
                  ((__CODEGEARC_VERSION__ & 0x00FF0000) >> 16),
                  (__CODEGEARC_VERSION__ & 0x0000FFFF) );

  #elif (__BORLANDC__ >= 0x0700)
    p += sprintf (p, "Embarcadero %X.%X", __BORLANDC__ & >> 8, __BORLANDC__ & 0xFF);

  #else
    p += sprintf (p, "Borland C %X.%X", __BORLANDC__ >> 8, __BORLANDC__ & 0xFF);
  #endif

  #if defined(__SMALL__)
    strcpy (p, " (small model), ");
  #elif defined(__LARGE__)
    strcpy (p, " (large model), ");
  #else
    strcpy (p, ", ");
  #endif

#elif defined(__TURBOC__)
  p += sprintf (p, "Turbo C %X.%X", (__TURBOC__ >> 8) - 1, __TURBOC__ & 0xFF);

  #if defined(__SMALL__)
    strcpy (p, " (small model), ");
  #elif defined(__LARGE__)
    strcpy (p, " (large model), ");
  #else
    strcpy (p, ", ");
  #endif

#elif defined(__clang__)
  p += sprintf (p, "clang %d.%d.%d (%s), ",
                __clang_major__, __clang_minor__, __clang_patchlevel__, DBG_RELEASE);

#elif defined(_MSC_VER)
  /*
   * Really is:
   *
   * #if (_MSC_VER == 1910)
   *   p += sprintf (p, "Microsoft Visual Studio 2017 (%s), ", DBG_RELEASE);
   * #elif (_MSC_VER == 1900)
   *   p += sprintf (p, "Microsoft Visual Studio 2015 (%s), ", DBG_RELEASE);
   * #elif (_MSC_VER == 1800)
   *   p += sprintf (p, "Microsoft Visual Studio 2013 (%s), ", DBG_RELEASE);
   * #elif (_MSC_VER == 1700)
   *   p += sprintf (p, "Microsoft Visual Studio 2012 (%s), ", DBG_RELEASE);
   * #elif (_MSC_VER == 1600)
   *   p += sprintf (p, "Microsoft Visual Studio 2010 (%s), ", DBG_RELEASE);
   * #elif (_MSC_VER == 1500)
   *   p += sprintf (p, "Microsoft Visual Studio 2008 (%s), ", DBG_RELEASE);
   * #elif (_MSC_VER == 1400)
   *   p += sprintf (p, "Microsoft Visual Studio 2005 (%s), ", DBG_RELEASE);
   * #elif (_MSC_VER == 1310)
   *   p += sprintf (p, "Microsoft Visual Studio 2003 (%s), ", DBG_RELEASE);
   * #elif (_MSC_VER == 1300)
   *   p += sprintf (p, "Microsoft Visual Studio 2002 (%s), ", DBG_RELEASE);
   */
  #if (_MSC_VER >= 1000)     /* Visual-C 4+ */
    p += sprintf (p, "Microsoft Visual-C %d.%d%s (%s, %s), ",
                  (_MSC_VER / 100), _MSC_VER % 100, msvc_get_patch_build(),
                  DBG_RELEASE, msvc_check_fastcall());

  #else /* DOS only */
    p += sprintf (p, "Microsoft Quick-C %d.%d",
                  _MSC_VER / 100, _MSC_VER % 100);

    #if defined(__SMALL__)
      strcpy (p, " (small model), ");
    #elif defined(__LARGE__)
      strcpy (p, " (large model), ");
    #else
      strcpy (p, ", ");
    #endif
  #endif

#elif defined(__DMC__)
  p += sprintf (p, "Digital Mars C %X.%X", __DMC__ >> 8, __DMC__ & 0xFF);

  #if defined(DOS386)
    strcpy (p, " (32-bit), ");
  #elif defined(__SMALL__)
    strcpy (p, " (small model), ");
  #elif defined(__LARGE__)
    strcpy (p, " (large model), ");
  #else
    strcpy (p, ", ");
  #endif

#elif defined(__CCDL__)
  p += sprintf (p, "LadSoft C %d.%d, ", __CCDL__/100, __CCDL__ % 100);

#elif defined(__LCC__) && defined(WIN32)
  p += sprintf (p, "lcc-win32, ");

#elif defined(__ORANGEC__)   /* https://orangec.readthedocs.io/en/latest/occ/OCC%20Defining%20Macros/ */
  sprintf (buf, "Orange-C %s", __VERSION__);

#elif defined(__ICC__)
 /*
  * The old ICC Intel compiler is completely untested.
  */
  p += sprintf (p, "Intel C %d.%d, ", __ICC__/100, __ICC__ % 100);

#elif defined(__INTEL_COMPILER__)
 /*
  * The more recent Intel compiler is also completely untested.
  * (costs a lot of money).
  */
  p += sprintf (p, "Intel C %d.%d.%d, ",
                __INTEL_COMPILER / 100,
                __INTEL_COMPILER % 100,
                __INTEL_COMPILER_UPDATE);
#endif

  p = strchr (buf, '\0');

  /* '__MINGW32__' is defined by BOTH mingw.org and by the MinGW-w64
   * project [1], because both can target Win32. '__MINGW64__' is defined
   * only when targeting Win64 (__x86_64__).
   *
   * [1] http://mingw-w64.sourceforge.net/
   */
#if defined(__MINGW32__) && defined(__MINGW64_VERSION_MAJOR)
  p += sprintf (p, "MinGW-w64 %d.%d (%s), ",
                __MINGW64_VERSION_MAJOR, __MINGW64_VERSION_MINOR, __MINGW64_VERSION_STATE);

#elif defined(__MINGW32__)          /* mingw.org MinGW. MinGW-RT-4+ defines '__MINGW_MAJOR_VERSION' */
  #if defined(__MINGW_MAJOR_VERSION)
    p += sprintf (p, "MinGW %d.%d, ", __MINGW_MAJOR_VERSION, __MINGW_MINOR_VERSION);
  #else
    p += sprintf (p, "MinGW %d.%d, ", __MINGW32_MAJOR_VERSION, __MINGW32_MINOR_VERSION);
  #endif

#elif defined(__CYGWIN__)
  p += sprintf (p, "CygWin %d.%d.%d, ", CYGWIN_VERSION_DLL_MAJOR/1000,
                CYGWIN_VERSION_DLL_MAJOR % 1000, CYGWIN_VERSION_DLL_MINOR);

#elif (DOSX == DJGPP)
  p += sprintf (p, "djgpp %d.%02d, ", __DJGPP__, __DJGPP_MINOR__);

#elif (DOSX == DOS4GW)
  {
    const char *extender = dos_extender_name();
    p += sprintf (p, "%s, ", extender ? extender : "unknown");
  }

#elif (DOSX == PHARLAP)
  p += sprintf (p, "Pharlap, ");

#elif (DOSX == X32VM)
  p += sprintf (p, "X32VM, ");

#elif (DOSX == POWERPAK)
  p += sprintf (p, "PowerPak, ");
#endif

#if defined(WIN32)
  if (_watt_is_wow64)
     p += sprintf (p, "WOW64, ");
#endif

  strcpy (p, __DATE__);
  return (buf);
}

/*
 * Since plain DOS is limited to 8+3 files, the 'cflags_buf.h' files
 * were renamed to 'cflagsbf.h'.
 */
#if defined(__DJGPP__)
  #define CC_DEFINE        "__DJGPP__"
  #define CC_PROG          "gcc"
  #define CFLAGS           "build/djgpp/cflags.h"

#elif defined(__BORLANDC__) || defined(__CODEGEARC_VERSION__)
  #define CC_DEFINE        "__BORLANDC__"

  #if defined(WIN32)
    #define CFLAGS         "build/borland/win32/cflags.h"
    #define CFLAGS_BUF     "build/borland/win32/cflagsbf.h"
    #define CC_PROG        "bcc32"

  #elif defined(__FLAT__)
    #define CFLAGS         "build/borland/flat/cflags.h"
    #define CFLAGS_BUF     "build/borland/flat/cflagsbf.h"
    #define CC_PROG        "bcc32"

  #elif defined(__LARGE__)
    #define CFLAGS         "build/borland/large/cflags.h"
    #define CFLAGS_BUF     "build/borland/large/cflagsbf.h"
    #define CC_PROG        "bcc"

  #elif defined(__SMALL__)
    #define CFLAGS         "build/borland/small/cflags.h"
    #define CFLAGS_BUF     "build/borland/small/cflagsbf.h"
    #define CC_PROG        "bcc"
  #endif

#elif defined(__TURBOC__)  /* No longer supported */
  #define CC_DEFINE        "__TURBOC__"
  #define CC_PROG          "tcc"

  #if defined(__SMALL__)
    #define CFLAGS         "build/turboc/small/cflags.h"
  #elif defined(__LARGE__)
    #define CFLAGS         "build/turboc/large/cflags.h"
  #endif

#elif defined(__DMC__)
  #define CC_DEFINE        "__DMC__"
  #define CC_PROG          "dmc"

  #if defined(__SMALL__)
    #define CFLAGS         "build/digmars/small/cflags.h"
    #define CFLAGS_BUF     "build/digmars/small/cflags.h"

  #elif defined(__LARGE__)
    #define CFLAGS         "build/digmars/large/cflags.h"
    #define CFLAGS_BUF     "build/digmars/large/cflagsbf.h"

  #elif defined(WIN32)
    #define CFLAGS         "build/digmars/win32/cflags.h"
    #define CFLAGS_BUF     "build/digmars/win32/cflagsbf.h"

  #elif (DOSX == PHARLAP)
    #define CFLAGS         "build/digmars/phar/cflags.h"
    #define CFLAGS_BUF     "build/digmars/phar/cflagsbf.h"

  #elif (DOSX == X32VM)
    #define CFLAGS         "build/digmars/x32vm/cflags.h"
    #define CFLAGS_BUF     "build/digmars/x32vm/cflagsbf.h"

  #else
    #error What!?
  #endif

#elif defined(__POCC__)    /* Win32 only */
  #define CC_DEFINE        "__POCC__"
  #define CC_PROG          "pocc"

  #if defined(WIN64) || defined(_M_X64)
    #define CFLAGS         "build/pellesc/64bit/cflags.h"
    #define CFLAGS_BUF     "build/pellesc/64bit/cflagsbf.h"
  #else
    #define CFLAGS         "build/pellesc/32bit/cflags.h"
    #define CFLAGS_BUF     "build/pellesc/32bit/cflagsbf.h"
  #endif

#elif defined(__LCC__)     /* Win32 only */
  #define CC_DEFINE        "__LCC__"
  #define CC_PROG          "lcc"
  #define CFLAGS           "build/lcc/cflags.h"
  #define CFLAGS_BUF       "build/lcc/cflagsbf.h"

#elif defined(__CCDL__)    /* DOS4GW only */
  #define CC_DEFINE        "__CCDL__"
  #define CC_PROG          "cc386"
  #define CFLAGS           "build/ladsoft/cflags.h"
  #define CFLAGS_BUF       "build/ladsoft/cflagsbf.h"

#elif defined(__ICC__)     /* Not yet */
  #define CC_DEFINE        "__ICC__"
  #define CC_PROG          "icc"
  #define CFLAGS           "build/intel/cflags.h"

#elif defined(__clang__)
  #define CC_DEFINE        "__clang__"
  #define CC_PROG          "clang-cl"

  #if defined(_DEBUG)
    #if defined(_WIN64)
      #define CFLAGS       "build/clang/64bit/debug/cflags.h"
      #define CFLAGS_BUF   "build/clang/64bit/debug/cflagsbf.h"
    #else
      #define CFLAGS       "build/clang/32bit/debug/cflags.h"
      #define CFLAGS_BUF   "build/clang/32bit/debug/cflagsbf.h"
    #endif
  #else
    #if defined(_WIN64)
      #define CFLAGS       "build/clang/64bit/release/cflags.h"
      #define CFLAGS_BUF   "build/clang/64bit/release/cflagsbf.h"
    #else
      #define CFLAGS       "build/clang/32bit/release/cflags.h"
      #define CFLAGS_BUF   "build/clang/32bit/release/cflagsbf.h"
    #endif
  #endif

#elif defined(__ORANGEC__)  /* Win32 only */
  #define CC_DEFINE        "__ORANGEC__"
  #define CC_PROG          "occ"
  #define CFLAGS           "build/orangec/cflags.h"
  #define CFLAGS_BUF       "build/orangec/cflagsbf.h"

#elif defined(__HIGHC__)   /* Pharlap only */
  #define CC_DEFINE        "__HIGHC__"
  #define CC_PROG          "hc386"
  #define CFLAGS           "build/HighC/cflags.h"

#elif defined(__WATCOMC__)
  #define CC_DEFINE        "__WATCOMC__"

  #if defined(__I86__)
    #define CC_PROG        "wcc"
    #if defined(__SMALL__)
      #define CFLAGS       "build/watcom/small/cflags.h"
      #define CFLAGS_BUF   "build/watcom/small/cflagsbf.h"
    #elif defined(__LARGE__)
      #define CFLAGS       "build/watcom/large/cflags.h"
      #define CFLAGS_BUF   "build/watcom/large/cflagsbf.h"
    #endif

  #elif defined(__386__)
    #define CC_PROG        "wcc386"
    #if defined(WIN32)
      #define CFLAGS       "build/watcom/win32/cflags.h"
      #define CFLAGS_BUF   "build/watcom/win32/cflagsbf.h"
    #elif defined(__FLAT__)
      #if (DOSX == X32VM)
        #define CFLAGS     "build/watcom/x32vm/cflags.h"
        #define CFLAGS_BUF "build/watcom/x32vm/cflagsbf.h"
      #elif (DOSX == PHARLAP)
        #define CFLAGS     "build/watcom/phar/cflags.h"
        #define CFLAGS_BUF "build/watcom/phar/cflagsbf.h"
      #else
        #define CFLAGS     "build/watcom/flat/cflags.h"
        #define CFLAGS_BUF "build/watcom/flat/cflagsbf.h"
      #endif
    #elif defined(__SMALL__)
      #define CFLAGS       "build/watcom/small32/cflags.h"
      #define CFLAGS_BUF   "build/watcom/small32/cflagsbf.h"
    #endif
  #endif

  /* Both these combos can be true for the "MinGW-w64" project [1].
   * Depending on '-m64' or not. '-m64' is targeting Win64 native code
   * and this sets '__MINGW64__' and 'WIN64'.
   */
#elif defined(__MINGW64__) && defined(__MINGW64_VERSION_MAJOR)
  #define CC_DEFINE        "__MINGW64__"
  #define CC_PROG          "gcc"
  #define CFLAGS           "build/MinGW64/64bit/cflags.h"
  #define CFLAGS_BUF       "build/MinGW64/64bit/cflagsbf.h"

#elif defined(__MINGW32__) && defined(__MINGW64_VERSION_MAJOR)
  #define CC_DEFINE        "__MINGW32__"
  #define CC_PROG          "gcc"
  #define CFLAGS           "build/MinGW64/32bit/cflags.h"
  #define CFLAGS_BUF       "build/MinGW64/32bit/cflagsbf.h"

#elif defined(__MINGW32__) /* The old-school MinGW from <mingw.org> */
  #define CC_DEFINE        "__MINGW32__"
  #define CC_PROG          "gcc"
  #define CFLAGS           "build/MinGW32/cflags.h"
  #define CFLAGS_BUF       "build/MinGW32/cflagsbf.h"

#elif defined(__CYGWIN__)
  #define CC_DEFINE        "__CYGWIN__"
  #define CC_PROG          "gcc"

  #if defined(__x86_64__)
    #define CFLAGS         "build/CygWin/64bit/cflags.h"
  #else
    #define CFLAGS         "build/CygWin/32bit/cflags.h"
  #endif

#elif defined(_MSC_VER)
  #define CC_DEFINE        "_MSC_VER"
  #define CC_PROG          "cl"

  #if defined(__SMALL__) || defined(__LARGE__)
    #error "Quick-C small/large model no longer supported."

  #elif (_MSC_VER >= 1000)     /* Visual-C 4+ (debug/release) */
    #if defined(_DEBUG)
      #if defined(_WIN64)
        #define CFLAGS      "build/visualc/64bit/debug/cflags.h"
        #define CFLAGS_BUF  "build/visualc/64bit/debug/cflagsbf.h"
      #else
        #define CFLAGS      "build/visualc/32bit/debug/cflags.h"
        #define CFLAGS_BUF  "build/visualc/32bit/debug/cflagsbf.h"
      #endif
    #else
      #if defined(_WIN64)
        #define CFLAGS      "build/visualc/64bit/release/cflags.h"
        #define CFLAGS_BUF  "build/visualc/64bit/release/cflagsbf.h"
      #else
        #define CFLAGS      "build/visualc/32bit/release/cflags.h"
        #define CFLAGS_BUF  "build/visualc/32bit/release/cflagsbf.h"
      #endif
    #endif
  #endif
#endif

/**
 * Return the the name if the compiler used.
 * E.g. "cl" for MSVC.
 */
const char * W32_CALL wattcpBuildCCexe (void)
{
#ifdef CC_PROG
  return (CC_PROG);
#else
  return ("Unknown");
#endif
}

/**
 * Return the target/compiler.
 * E.g. "_MSC_VER" for MSVC.
 */
const char * W32_CALL wattcpBuildCC (void)
{
#ifdef CC_DEFINE
  return (CC_DEFINE);
#else
  return ("Unknown");
#endif
}

/**
 * Return the $(CFLAGS) used.
 * Since the makefiles contains "-I..\inc" for some targets (except gcc),
 * replace "-I.." with "-I../inc". ("\i" is interpreted as an ESC-sequence).
 *
 * Also removes excessive spaces between args in $(CFLAGS) using 'strtrim()'.
 */
#if defined(CFLAGS_BUF)
  /*
   * Generated array for CFLAGS.
   * Ref: cflagsbf.h rules in respective makefile.
   *
   * Since the makefiles contains "\xx" for some targets (except gcc),
   * the below '#include CFLAGS' is mishandled by the C-preprocessor;
   * The "\xx" are interpreted as an ESC-sequences. But the below array
   * is left as-is.
   */
  char w32_cflags2[] = {
    #include CFLAGS_BUF
      0
    };

  const char * W32_CALL wattcpBuildCflags (void)
  {
    static char buf [sizeof(w32_cflags2)+1];
    static int  done = 0;

    if (!done)
    {
#if 0
      char       *to   = buf;
      const char *from = w32_cflags2;
      size_t      i;

      for (i = 0; i < sizeof(w32_cflags2); i++)
      {
        if (*from == '\r' || *from == '\n' || *from == '\0')
           continue;
        if (*from == '\\')
             *to++ = '/';
        else *to++ = *from;
        from++;
      }
      *to = '\0';
#else
      strreplace ('\n', 0, w32_cflags2);
      strreplace ('\\','/', w32_cflags2);
      strtrim (w32_cflags2, buf, sizeof(buf));
#endif
      done = 1;
    }
    return (buf);
  }

#else

  #if defined(CFLAGS)
  #include CFLAGS
  #endif

  const char * W32_CALL wattcpBuildCflags (void)
  {
  #if defined(CFLAGS)
    static char buf [400];  /* should be enough for High-C */

    _strlcpy (buf, w32_cflags, sizeof(buf));
    return (buf);
  #else
    return ("Unknown");
  #endif  /* CFLAGS */
  }
#endif    /* CFLAGS_BUF */


static const char *capabilities[] = {
#if defined(USE_DEBUG)
           "debug",
#endif
#if defined(USE_MULTICAST)
           "mcast",
#endif
#if defined(USE_BIND)
           "bind",
#endif
#if defined(USE_BSD_API)
           "BSDsock",
#endif
#if defined(USE_BSD_FATAL)
           "BSDfatal",
#endif
#if defined(USE_BOOTP)
           "bootp",
#endif
#if defined(USE_DHCP)
           "dhcp",
#endif
#if defined(USE_RARP)
           "rarp",
#endif
#if defined(USE_LANGUAGE)
           "lang",
#endif
#if defined(USE_FRAGMENTS)
           "frag",
#endif
#if defined(USE_STATISTICS)
           "stat",
#endif
#if defined(USE_FORTIFY)
           "fortify",
#endif
#if defined(USE_CRTDBG)
           "crt-dbg",
#endif
#if defined(USE_FSEXT)
           "fsext",
#endif
#if defined(USE_LOOPBACK)
           "loopback",
#endif
#if defined(USE_EMBEDDED)
           "embedded",
#endif
#if defined(USE_TFTP)
           "tftp",
#endif
#if defined(USE_TCP_SACK)
           "sack",
#endif
#if defined(USE_UDP_ONLY)
           "udp-only",
#endif
#if defined(USE_ECHO_DISC)
           "echo",
#endif
#if defined(USE_PPPOE)
           "PPPoE",
#endif
#if defined(USE_IPV6)
           "IPv6",
#endif
#if defined(USE_DEAD_GWD)
           "dead-gw",
#endif
#if defined(USE_GZIP)
           "gzip",
#endif
#if defined(USE_TCP_MD5)
           "TCP-MD5",
#endif
#if defined(USE_DYNIP_CLI)
           "DynIP",
#endif
#if defined(USE_PROFILER)
           "profiler",
#endif
#if defined(USE_FAST_PKT)
           "fast-pkt",
#elif defined(__MSDOS__)
           "slow-RMCB",
#endif
#if defined(USE_SCTP)
           "SCTP",
#endif
           NULL
         };

/**
 * Return compiled-in capabilities and features.
 * These features are set in "config.h"
 */
const char * W32_CALL wattcpCapabilities (void)
{
  static char buf [240];
  char  *p = buf;
  int    i;

  for (i = 0; capabilities[i]; i++)
  {
    *p++ = '/';
    if (p + strlen(capabilities[i]) - 2 >= buf + sizeof(buf))
    {
      *p++ = '.';
      *p++ = '.';
      break;
    }
    strcpy (p, capabilities[i]);
    p += strlen (capabilities[i]);
  }
  *p = '\0';
  return (buf);
}

#if defined(__GNUC__)
/*
 * The builtin `__tune_xx__' defines were introduced in gcc 2.95.1 (?)
 *
 * 'gcc -v --help'. Look for '=mtune=CPU':
 *
 * -mtune=CPU  optimize for CPU, CPU is one of:
 *             generic32, generic64, i8086, i186, i286, i386, i486,
 *             i586, i686, pentium, pentiumpro, pentiumii,
 *             pentiumiii, pentium4, prescott, nocona, core, core2,
 *             corei7, l1om, k1om, k6, k6_2, athlon, opteron, k8,
 *             amdfam10, bdver1, bdver2, bdver3, btver1, btver2
 */
const char *gcc_get_cpu_tune (void)
{
  static char ret[20];
  const char *p = NULL;

#if defined(__x86_64__)           /* -m64 (MinGW64, AMD) */
  p = "x86-64";
#endif

#if defined(__tune_i386__)        /* -mtune=i386 (default for -m32) */
  p = "386";
#endif

#if defined(__tune_i486__)        /* -mtune=i486 */
  p = "486";
#endif

#if defined(__tune_i586__)        /* -mtune=i586 */
  p = "586";
#endif

#if defined(__tune_i686__)        /* -mtune=pentiumpro */
  p = "686";
#endif

#if defined(__tune_pentium__)     /* -mtune=pentium */
  p = "Pent";
#endif

#if defined(__tune_pentium3__)    /* -mtune=pentium3 */
  p = "Pent3";
#endif

#if defined(__tune_pentium4__)    /* -mtune=pentium4 */
  p = "Pent4";
#endif

#if defined(__tune_pentiumpro__)  /* -mtune=pentiumpro */
  p = "PentPro";
#endif

#if defined(__tune_nocona__)      /* -mtune=nocona,prescott */
  p = "nocona";
#endif

#if defined(__tune_core2__)       /* -mtune=core2 */
  p = "Core2";
#endif

#if defined(__tune_corei7__)      /* -mtune=corei7 */
  p = "CoreI7";
#endif

#if defined(__tune_k6__)          /* -mtune=k6 */
  p = "K6";
#endif

#if defined(__tune_athlon__)      /* -mtune=athlon */
  p = "Athlon";
#endif

#if defined(__tune_k8__)          /* -mtune=k8,opteron */
  p = "K8";
#endif

#if defined(__tune_amdfam10__)    /* -mtune=amdfam10 */
  p = "K8";
#endif

#if defined(__tune_bdver1__)      /* -mtune=bdver1 */
  p = "BDv1";
#endif

#if defined(__tune_bdver2__)      /* -mtune=bdver2 */
  p = "BDv2";
#endif

#if defined(__tune_btver1__)      /* -mtune=btver1 */
  p = "BTv1";
#endif

  if (!p)
     return (", ");

#if defined(SNPRINTF)
  SNPRINTF (ret, sizeof(ret), " (%s), ", p);
#else
  sprintf (ret, " (%s), ", p);
#endif

  return (ret);
}
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1000)  /* Visual-C 4+ */
#if !defined(_M_X64)
  /*
   * cl/x64 doesn't support reg-calls. Does any other 64-bit compilers do that?
   * If compiled as fastcall (-Gr), the 1st argument is in ECX.
   */
  _declspec(naked) static DWORD echo_1st_arg_Gr (DWORD arg)
  {
    __asm mov eax, ecx
    __asm ret
  }
#endif

static const char *msvc_check_fastcall (void)
{
#if !defined(_M_X64)
   if (echo_1st_arg_Gr(0xDEAFBABE) == 0xDEAFBABE)
      return ("fastcall");
#endif
   return ("cdecl");
}

/*
 * Ref. http://msdn.microsoft.com/en-us/library/b0084kay(v=vs.120).aspx
 *      https://sourceforge.net/p/predef/wiki/Compilers/
 *
 * E.g. "cl /?" prints:
 *    Microsoft (R) C/C++ Optimizing Compiler Version 18.00.31101 for x86
 *                       = _MSC_FULL_VER - 180000000  ^----
 *
 * It can also have a trailing build number:
 *   Microsoft (R) C/C++ Optimizing Compiler Version 19.11.25507.1 for x86
 *                                                    _MSC_BUILD ^
 */
static const char *msvc_get_patch_build (void)
{
  static char buf[20]; /* room for "19.11.25507.1" = VV.RR.PPPPP.B\0 */
  char  *end;
  DWORD  patch = 0;

  buf[0] = '\0';

#if defined(_MSC_FULL_VER)
  buf[0] = '.';
  patch = _MSC_FULL_VER % 100000;
#endif  /* _MSC_FULL_VER */

  if (patch)
     _ultoa (patch, buf+1, 10);

#if defined(_MSC_BUILD)
  end = strrchr (buf, '\0');
  *end++ = '.';
  _ultoa (_MSC_BUILD, end, 10);
#endif

  return (buf);
}
#endif    /* _MSC_VER && _MSC_VER >= 1000 */


