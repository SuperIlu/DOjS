#ifndef _w32_SYSDEP_H
#define _w32_SYSDEP_H

/* Dirty system-dependent hacks.
 */

#include <string.h>
#include <ctype.h>

#if defined(WIN32) && !defined(_WIN32)
  #define WIN32_LEAN_AND_MEAN

  #include <io.h>
  #include <getopt.h>
  #include <windows.h>

#elif defined(_MSC_VER)
  #define WIN32_LEAN_AND_MEAN

  #include <stdio.h>
  #include <io.h>
  #include <windows.h>
  #include "../src/getopt.c"

  void W32_CALL outsnl (const char *s)
  {
    puts (s);
  }

#elif defined(__MSDOS__)
  #include <dir.h>
  #include <io.h>
  #include <unistd.h>  /* Assumes only djgpp is used for __MSDOS__ */

#elif defined(__MINGW32__) || defined(__CYGWIN__) || defined(__WATCOMC__)
  #include <unistd.h>

#elif defined(__unix__) || defined(__linux__)
  #include <unistd.h>

  /* Cross compiling from Linux->DOS (assume gcc used)
   */
  static __inline char *my_strlwr (char *str)
  {
    char *c = str;
    for ( ; *c; c++)
       *c = tolower (*c);
    return (str);
  }
  static __inline char *my_strupr (char *str)
  {
    char *c = str;
    for ( ; *c; c++)
       *c = toupper (*c);
    return (str);
  }
  #define stricmp   strcasecmp
  #define strnicmp  strncasecmp
  #define strlwr    my_strlwr
  #define strupr    my_strupr

#else
  #error "Unsupported platform or cross-combo"
#endif

/*
 * mkdir() hackery.
 * djgpp do support the mode arg, but I'm not sure the value is important.
 */
#if defined(__DJGPP__)
  #define MKDIR(D) mkdir ((D), 0777)
  #define SLASH    '\\'

#elif defined(WIN32) || defined(__MSDOS__)  /* MinGW32/MinGW64/MSDOS */
  #define MKDIR(D)  mkdir ((D))
  #define SLASH     '\\'

#elif defined(_WIN32)                       /* MSVC */
  #define MKDIR(D) _mkdir ((D))
  #define SLASH    '\\'

#else /* assume unix: */
  #define MKDIR(D)  mkdir ((D),0755)
  #define SLASH     '/'
#endif

#endif  /* _w32_SYSDEP_H */
