/*
MIT License

Copyright (c) 2019-2021 Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __GLUE_H__
#define __GLUE_H__

#include "DOjS.h"

#include <stdint.h>
#include <stdbool.h>
#include <allegro.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//////
// DJGPP
extern unsigned short _osmajor, _osminor;
extern unsigned short _os_trueversion;
extern const char *_os_flavor;

//////
// allegro
extern void glue_init(js_State *J);
extern void glue_shutdown(void);
extern FONT *load_grx_font_pf(PACKFILE *f, RGB *pal, void *param);
extern struct BITMAP *load_bitmap_pf(PACKFILE *f, struct RGB *pal, const char *aext);

//////
// watt32
#define _gethostid() gethostid()

typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef bool BOOL;

struct watt_sockaddr { /* for _getpeername, _getsockname */
    WORD s_type;
    WORD s_port;
    DWORD s_ip;
    BYTE s_spares[6]; /* unused */
};

typedef void sock_type;
typedef sock_type tcp_Socket;
typedef sock_type udp_Socket;

/**<\typedef protocol handler callback type.
 */
typedef void *ProtoHandler;
typedef void *UserHandler;

extern int dom_errno;
extern DWORD sin_mask;
extern int _watt_do_exit;

extern const char *wattcpVersion(void);      /* Watt-32 target version/date */
extern const char *wattcpCapabilities(void); /* what features was been compiled in */

extern DWORD resolve(const char *name);
extern int resolve_ip(DWORD ip, char *name, int len);
extern const char *sock_init_err(int rc);
extern int sock_init(void);
extern void dbug_init(void);

extern const char *dom_strerror(int err);
extern char *_inet_ntoa(char *s, DWORD x);
extern const char *sockerr(const sock_type *s); /* UDP / TCP */

extern int sock_puts(sock_type *s, const BYTE *dp);
extern int sock_write(sock_type *s, const BYTE *dp, int len);
extern int sock_read(sock_type *s, BYTE *dp, size_t len);
extern int sock_established(sock_type *s);
extern int sock_close(sock_type *s);
extern WORD sock_gets(sock_type *s, BYTE *dp, int n);
extern WORD sock_mode(sock_type *s, WORD mode);
extern int sock_getc(sock_type *s);
extern void sock_flushnext(sock_type *s);
extern void sock_noflush(sock_type *s);
extern void sock_flush(sock_type *s);
extern WORD tcp_tick(sock_type *s);
extern BYTE sock_putc(sock_type *s, BYTE c);
extern int sock_abort(sock_type *s);
extern int _w32__getpeername(const sock_type *s, void *dest, int *len);
extern int _w32__getsockname(const sock_type *s, void *dest, int *len);
// extern int getdomainname(char *name, size_t len);

extern WORD sock_dataready(sock_type *s);

extern int udp_open(udp_Socket *s, WORD lport, DWORD ina, WORD port, ProtoHandler handler);
extern int tcp_open(tcp_Socket *s, WORD lport, DWORD ina, WORD port, ProtoHandler handler);
extern int udp_listen(udp_Socket *s, WORD lport, DWORD ina, WORD port, ProtoHandler handler);
extern int tcp_listen(tcp_Socket *s, WORD lport, DWORD ina, WORD port, ProtoHandler handler, WORD timeout);

extern size_t sock_tbsize(const sock_type *s);
extern size_t sock_tbused(const sock_type *s);
extern size_t sock_tbleft(const sock_type *s);

extern int _ip_delay0(sock_type *s, int sec, UserHandler fn, int *statusptr);
extern int _ip_delay1(sock_type *s, int sec, UserHandler fn, int *statusptr);
extern int _ip_delay2(sock_type *s, int sec, UserHandler fn, int *statusptr);

#define sock_wait_established(s, seconds, fn, statusptr)          \
    do {                                                          \
        if (_ip_delay0(s, seconds, fn, statusptr)) goto sock_err; \
    } while (0)

#define sock_wait_input(s, seconds, fn, statusptr)                \
    do {                                                          \
        if (_ip_delay1(s, seconds, fn, statusptr)) goto sock_err; \
    } while (0)

#define sock_tick(s, statusptr)             \
    do {                                    \
        if (!tcp_tick(s)) {                 \
            if (statusptr) *statusptr = -1; \
            goto sock_err;                  \
        }                                   \
    } while (0)

#define sock_wait_closed(s, seconds, fn, statusptr)               \
    do {                                                          \
        if (_ip_delay2(s, seconds, fn, statusptr)) goto sock_err; \
    } while (0)

#endif  // __GLUE_H__
