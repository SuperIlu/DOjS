/*!\file printk.h
 */
#ifndef _w32_PRINTK_H
#define _w32_PRINTK_H

#include <stdio.h>
#include <stdarg.h>

#if defined(__DJGPP__) || defined(__HIGHC__) || defined(__WATCOMC__) || defined(__DMC__)
  #undef _WIN32       /* Needed for __DMC__ */
  #include <unistd.h>
#endif

#if defined(__TURBOC__) || defined(_MSC_VER) || defined(__WATCOMC__) || \
    defined(__DMC__) || defined(__MINGW32__) || defined(__LCC__)
  #include <process.h>
#endif

#ifndef __SYS_CDEFS_H
#include <sys/cdefs.h>    /* W32_CDECL */
#endif

#ifdef _w32_WATTCP_H  /* if included after wattcp.h (Watt-32 compile) */
  #define _printk_safe   W32_NAMESPACE (_printk_safe)
  #define _printk_file   W32_NAMESPACE (_printk_file)
  #define _printk_init   W32_NAMESPACE (_printk_init)
  #define _printk_flush  W32_NAMESPACE (_printk_flush)
  #define _printk        W32_NAMESPACE (_printk)
  #define _fputsk        W32_NAMESPACE (_fputsk)
  #define _snprintk      W32_NAMESPACE (_snprintk)
  #define _vsnprintk     W32_NAMESPACE (_vsnprintk)
#endif

extern int   _printk_safe;  /*!< safe to print; we're not in a RMCB/ISR. */
extern FILE *_printk_file;  /*!< what file to print to (or stdout/stderr). */

extern int   _printk_init  (int size, const char *file);
extern void  _printk_flush (void);

extern int   _fputsk    (const char *buf, FILE *stream);

#if defined(__POCC__)
  extern _CRTCHK(printf,3,3) int _vsnprintk (char *buf, int len, const char *fmt, va_list ap);
  extern _CRTCHK(printf,1,2) int _printk (const char *fmt, ...);
  extern _CRTCHK(printf,3,4) int _snprintk (char *buf, int len, const char *fmt, ...);

#else

  extern int _vsnprintk (char *buf, int len, const char *fmt, va_list ap)
  W32_ATTR_PRINTF (3,0);

  extern int MS_CDECL _printk (const char *fmt, ...)
  W32_ATTR_PRINTF (1,2);

  extern int MS_CDECL _snprintk (char *buf, int len, const char *fmt, ...)
  W32_ATTR_PRINTF (3,4);
#endif

#endif  /* _w32_PRINTK_H */

