/*!\file bsddbug.h
 */
#ifndef _w32_BSDDBUG_H
#define _w32_BSDDBUG_H

/*
 * Debugging of BSD-socket API. Writes to "sk_debug.device"
 * when dbug_init() is called or `SO_DEBUG' is set on socket.
 */
#if defined(USE_DEBUG) && defined(USE_BSD_API)
  #if defined(__POCC__)
    extern _CRTCHK(printf,2,3) int _sock_debugf (const char *fmt, ...);
  #else
    extern int MS_CDECL            _sock_debugf (const char *fmt, ...) W32_ATTR_PRINTF (1,2);
  #endif

  extern void  _sock_dbug_flush  (void);
  extern void  _sock_dbug_init   (void);
  extern void  _sock_dbug_open   (void);
  extern BOOL  _sock_dbug_active (void);
  extern void  _sock_enter_scope (void);
  extern void  _sock_leave_scope (void);

  extern void bsd_fortify_print (const char *buf);

  #define SOCK_DEBUGF(x)      _sock_debugf x
  #define SOCK_DBUG_FLUSH()   _sock_dbug_flush()
  #define SOCK_ENTER_SCOPE()  _sock_enter_scope()
  #define SOCK_LEAVE_SCOPE()  _sock_leave_scope()
#else
  #define SOCK_DEBUGF(x)      ((void)0)
  #define SOCK_DBUG_FLUSH()   ((void)0)
  #define SOCK_ENTER_SCOPE()  ((void)0)
  #define SOCK_LEAVE_SCOPE()  ((void)0)
#endif

#endif
