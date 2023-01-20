#ifndef __TELNET_H
#define __TELNET_H

#define TELNET_VERSION  "telnet 0.8"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <arpa/telnet.h>
#include <arpa/nameser.h>
#include <tcp.h>

#if defined(__HIGHC__)
  #include <pharlap.h>
  #include <mw/conio.h>     /* conio extensions */
  #include <mw/exc.h>
  #include <alloca.h>
  #include <unistd.h>
#elif defined(__WATCOMC__)
  #include <mw/conio.h>
  #include <unistd.h>
#else
  #include <conio.h>
  #include <dos.h>
#endif

#ifdef __DJGPP__
  #include <unistd.h>
  #define SLASH     '/'
  #define _MAX_PATH 128
#else
  #define SLASH     '\\'
#endif

#ifndef NDEBUG
  #undef  assert
  #define assert(x) ((x) ? (void)0 : assert_fail(#x,__FILE__,__LINE__))
#endif

#ifndef IPPORT_TELNET
#define IPPORT_TELNET 23
#endif

#ifndef IPPORT_SSH
#define IPPORT_SSH    22
#endif

#ifndef min
#define min(x,y)     ((x) < (y) ? (x) : (y))
#endif

#ifndef max
#define max(x,y)     ((x) > (y) ? (x) : (y))
#endif

#ifndef DIM
#define DIM(array)   (sizeof(array) / sizeof(array[0]))
#endif

#define STATIC
#define ARGSUSED(x)  (void)x

#define MAX_HOSTNAME 64
#define MAX_ARGC     10
#define LIN_SIZE     2048

#define HEATH19      1
#define VT52         2
#define VT100        4
#define VT102        8
#define VT200        16
#define ANSI         32
#define LINUX        64

#if defined(__HIGHC__)
  #define VSPRINTF(buf, fmt, va)   _vbprintf (buf, sizeof(buf), fmt, va)
#elif defined(__WATCOMC__)
  #define VSPRINTF(buf, fmt, va)   _vsnprintf (buf, sizeof(buf), fmt, va)
#elif defined(__DJGPP__) && (DJGPP_MINOR >= 4)
  #define VSPRINTF(buf, fmt, va)   vsnprintf (buf, sizeof(buf), fmt, va)
#else
  #define VSPRINTF(buf, fmt, va)   vsprintf (buf, fmt, va)
#endif

extern char       progPath [_MAX_PATH];
extern char       progName [_MAX_PATH];
extern tcp_Socket sock;

extern int  connected;           /* Are we connected to the other side? */
extern int  dbug_mode;           /* Debug mode; cmdline option -d */
extern int  verbose;             /* Verbosity level: cmdline option -v */
extern int  quit;                /* <Alt-X> pressed; quit */
extern char hostname[];          /* Who are we connected to? */
extern void (*recvState)(void);  /* main receiving function */
extern void (*xmitState)(void);  /* main transmitting function */

extern int  GetCharTN   (int *ch);
extern int  ConnectTN   (const char *host, WORD port);
extern void CloseTN     (void);
extern int  GetKeyTN    (char *str, size_t max);
extern int  PutStringTN (const char *fmt, ...);
extern int  BackGroundTN(void);

extern void TelnetRecv  (void);
extern void TelnetSend  (void);
extern void TelnetExit  (void);

extern void assert_fail (const char *what, const char *file, unsigned line);
extern void fatal       (const char *fmt, ...);

extern void int29_init  (void);
extern void int29_exit  (void);

extern void VT_Init     (void);
extern void VT_Process  (int);
extern int  VT_SetMode  (int);
const char *VT_GetMode  (void);

#endif /* __TELNET_H */
