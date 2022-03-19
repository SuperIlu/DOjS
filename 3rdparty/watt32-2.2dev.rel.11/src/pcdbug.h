/*!\file pcdbug.h
 */
#ifndef _w32_PCDBUG_H
#define _w32_PCDBUG_H

#include "strings.h"  /* _printf */

typedef void (*DebugProc) (const void *sock, const in_Header *ip,
                           const char *file, unsigned line);

#define debug_xmit W32_NAMESPACE (debug_xmit)
#define debug_recv W32_NAMESPACE (debug_recv)
#define debug_on   W32_NAMESPACE (debug_on)

W32_DATA DebugProc debug_xmit, debug_recv;
W32_DATA int       debug_on;

extern const char *tcpStateName (UINT state);

extern BOOL dbg_mode_all, dbg_print_stat, dbg_dns_details;

extern   void dbug_open (void);
extern   BOOL dbug_file (void);
extern   int  dbug_write (const char *);
extern   int  dbug_putc (int c);
extern   void dbug_flush (void);

#if !defined(SWIG)
  #if defined(__POCC__)
    extern _CRTCHK(printf,1,2) int dbug_printf (const char *fmt, ...);
  #else
    extern int MS_CDECL            dbug_printf (const char *fmt, ...) W32_ATTR_PRINTF (1, 2);
  #endif
#endif

#if defined(USE_PPPOE)
  extern const char *pppoe_get_code (WORD code);
#endif

/*
 * Send Rx/Tx packet to debug-file.
 * 'nw_pkt' must point to network layer packet.
 */
#if defined(USE_DEBUG)
  #define DEBUG_RX(sock, nw_pkt)                             \
          do {                                               \
            if (debug_recv)                                  \
              (*debug_recv) (sock, (const in_Header*)nw_pkt, \
                             __FILE__, __LINE__);            \
          } while (0)

  #define DEBUG_TX(sock, nw_pkt)                             \
          do {                                               \
            if (debug_xmit)                                  \
              (*debug_xmit) (sock, (const in_Header*)nw_pkt, \
                             __FILE__, __LINE__);            \
          } while (0)

  /* Generic trace to wattcp.dbg file
   */
  #define TCP_TRACE_MSG(args)    \
          do {                   \
            if (dbug_file())     \
               dbug_printf args; \
          } while (0)

  /* Trace to console
   */
  #define TCP_CONSOLE_MSG(lvl, args) \
          do {                       \
            if (debug_on >= lvl) {   \
              (*_printf) args;       \
              fflush (stdout);       \
            }                        \
          } while (0)
#else
  #define DEBUG_RX(sock, ip)         ((void)0)
  #define DEBUG_TX(sock, ip)         ((void)0)
  #define TCP_TRACE_MSG(args)        ((void)0)
  #define TCP_CONSOLE_MSG(args, lvl) ((void)0)
#endif

#define TRACE_MSG   TCP_TRACE_MSG
#define CONSOLE_MSG TCP_CONSOLE_MSG

#endif

