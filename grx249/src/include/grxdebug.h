/**
 ** grxdebug.h ---- GRX library debug support
 **
 ** Copyright (c) 1998 Hartmut Schirmer
 **
 ** This file is part of the GRX graphics library.
 **
 ** The GRX graphics library is free software; you can redistribute it
 ** and/or modify it under some conditions; see the "copying.grx" file
 ** for details.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 **
 **/

#ifndef __GRXDEBUG_H_INCLUDED__
#define __GRXDEBUG_H_INCLUDED__

#define DBG_ENTER          0x8000
#define DBG_LEAVE          DBG_ENTER
#define DBG_FONT           0x4000
#define DBG_SETMD          0x2000
#define DBG_DRIVER         0x1000
#define DBG_COLOR          0x0800
#define DBG_COPYFILL       0x0400

extern char *_GR_debug_file;
extern int   _GR_debug_line;
extern int   _GR_debug_flags;
extern void  _GR_debug_printf(char *fmt,...);

# ifdef __GNUC__
  extern char *_GR_debug_function;
#  define DBGPRINTF(tst,x) do {           \
     if ((tst)&_GR_debug_flags) {         \
       _GR_debug_file = __FILE__;         \
       _GR_debug_line = __LINE__;         \
       _GR_debug_function = __FUNCTION__; \
       _GR_debug_printf x;                \
     }                                    \
   } while (0)
# else
#  define DBGPRINTF(tst,x) do {           \
     if ((tst)&_GR_debug_flags) {         \
       _GR_debug_file = __FILE__;         \
       _GR_debug_line = __LINE__;         \
       _GR_debug_printf x ;               \
     }                                    \
   } while (0)
#endif

#define GRX_ENTER()       DBGPRINTF(DBG_ENTER,("FUNC ENTER\n"))
#define GRX_LEAVE()       DBGPRINTF(DBG_LEAVE,("FUNC EXIT\n"))
#define GRX_RETURN(x)     GRX_LEAVE(); return x;

#endif  /* whole file */

