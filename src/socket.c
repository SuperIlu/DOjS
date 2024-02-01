/*
MIT License

Copyright (c) 2019-2022 Andre Seidelt <superilu@yahoo.com>

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

#include "socket.h"

#include "watt.h"
#include "bytearray.h"

/************
** defines **
************/
#define TCP_CLIENT 0
#define TCP_SERVER 1
#define UDP 2

#define MAX_LINE_LENGTH 4096  //!< read at max 4KiB

//! Get socket userdata and tick socket
#define SOCK_USER_DATA(x)                          \
    socket_t *x = js_touserdata(J, 0, TAG_SOCKET); \
    if (!x->socket) {                              \
        js_error(J, "Socket was closed!");         \
        return;                                    \
    }                                              \
    tcp_tick(x->socket);

/************
** structs **
************/
//! socket userdata definition
typedef struct __socket {
    sock_type *socket;  //!< the socket
    bool udp;           //!< indicates if this is a udp socket
    bool server;        //!< indicates if this is a server socket
} socket_t;

static int socket_count = 0;

/*********************
** static functions **
*********************/
/**
 * flush all data.
 *
 * @param s the socket.
 */
static void socket_flush(socket_t *s) {
#if LINUX != 1
    while (!s->udp && (sock_tbused(s->socket) > 0)) {  // drain
        tcp_tick(s->socket);
    }
#endif  // LINUX != 1
}

/**
 * @brief finalize a socket and free resources.
 *
 * @param J VM state.
 */
static void Socket_Finalize(js_State *J, void *data) {
    int status;

    socket_t *s = (socket_t *)data;
    DEBUGF("%s: 0x%8p / 0x%8p\n", __PRETTY_FUNCTION__, s, s->socket);
    if (s->socket) {
        // abort and close the socket
        sock_abort(s->socket);
        sock_close(s->socket);

        sock_wait_closed(s->socket, 0, NULL, &status);
    }

sock_err:
    free(s->socket);
    s->socket = NULL;
    socket_count--;

    free(s);
}

/**
 * @brief open a socket and store it as userdata in JS object.
 * socket = new Socket(type:number, dst:str|number[], rport:number, [lport:number])
 *
 * @param J VM state.
 */
static void new_Socket(js_State *J) {
    int status;

    NEW_OBJECT_PREP(J);

    DEBUGF("socket_t=%ld, sock_type=%ld\n", sizeof(socket_t), sizeof(tcp_Socket));

    int sock_type = js_touint16(J, 1);
    if ((sock_type != TCP_CLIENT) && (sock_type != TCP_SERVER) && (sock_type != UDP)) {
        js_error(J, "Illegal socket type: %d", sock_type);
        return;
    }

    socket_t *s = calloc(1, sizeof(socket_t));
    if (!s) {
        JS_ENOMEM(J);
        return;
    }
    s->server = false;

    s->socket = calloc(1, sizeof(tcp_Socket) > sizeof(udp_Socket) ? sizeof(tcp_Socket) : sizeof(udp_Socket));
    if (!s->socket) {
        free(s);
        JS_ENOMEM(J);
        return;
    }

    DWORD ip;
    if (js_isarray(J, 2)) {
        ip = watt_toipaddr(J, 2);
    } else {
        const char *host = js_tostring(J, 2);
        ip = resolve(host);
        if (!ip) {
            js_error(J, "Can't resolve '%s': %s", host, dom_strerror(dom_errno));
            free(s->socket);
            free(s);
            return;
        }
    }
    uint16_t rport = js_touint16(J, 3);

    // get local port, if none given use 0 for "don't care"
    uint16_t lport = 0;
    if (js_isnumber(J, 4)) {
        lport = js_touint16(J, 4);
    }

    DEBUGF("Socket(%d, %08lX, %d, %d)\n", sock_type, ip, lport, rport);

    int res;
    if (sock_type == TCP_CLIENT) {
        s->udp = false;
        s->server = false;
        res = tcp_open(s->socket, lport, ip, rport, NULL);
    } else if (sock_type == TCP_SERVER) {
        s->udp = false;
        s->server = true;
        res = tcp_listen(s->socket, lport, ip, rport, NULL, 0);
    } else {
        s->udp = true;
        s->server = false;
        res = udp_open(s->socket, lport, ip, rport, NULL);
    }
    if (res == 0) {
        js_error(J, "cannot open tcp socket '%d.%d.%d.%d' (%d->%d): %s", IP1(ip), IP2(ip), IP3(ip), IP4(ip), lport, rport, sockerr(s->socket));
        free(s);
        return;
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_SOCKET, s, Socket_Finalize);

    // add properties
    js_pushboolean(J, s->udp);
    js_defproperty(J, -2, "udp", JS_READONLY | JS_DONTCONF);

    js_pushboolean(J, s->server);
    js_defproperty(J, -2, "server", JS_READONLY | JS_DONTCONF);

    tcp_tick(s->socket);  // this function is fine for UDP-sockets as well
    socket_count++;

    // server sockets are returned unconnected!
    if (sock_type == TCP_CLIENT) {
        DEBUG("Wait for connection\n");
        sock_wait_established(s->socket, 0, NULL, &status);
    }
    return;

sock_err:
    switch (status) {
        case 1: /*foreign host closed*/
            js_error(J, "Socket closed");
            break;
        case -1: /*time-out*/
            js_error(J, "Connection timed out");
            break;
    }
}

/**
 * @brief close the socket.
 * socket.Close([doFlush:boolean = false])
 *
 * @param J VM state.
 */
static void Socket_Close(js_State *J) {
    int status;

    socket_t *s = js_touserdata(J, 0, TAG_SOCKET);
    if (s->socket) {
#if LINUX != 1
        DEBUGF("%s() TXbuf=%ld - %ld = %ld\n", __PRETTY_FUNCTION__, sock_tbsize(s->socket), sock_tbused(s->socket), sock_tbleft(s->socket));
#endif  // LINUX != 1
        if (js_isboolean(J, 1) && js_toboolean(J, 1)) {
            sock_flush(s->socket);
            socket_flush(s);
        }

        // abort and close the socket
        sock_abort(s->socket);
        sock_close(s->socket);

        sock_wait_closed(s->socket, 0, NULL, &status);

        free(s->socket);
        s->socket = NULL;
        socket_count--;
    }
    return;

sock_err:
    DEBUGF("sock_err %s: 0x%d\n", __PRETTY_FUNCTION__, status);
    switch (status) {
        case -1: /*time-out*/
            js_error(J, "Connection timed out");
            break;
        case 1: /*closed*/
            free(s->socket);
            s->socket = NULL;
            socket_count--;
            break;
    }
}

/**
 * @brief wait until all written data is flushed.
 * socket.WaitFlush()
 *
 * @param J VM state.
 */
static void Socket_WaitFlush(js_State *J) {
    SOCK_USER_DATA(s);
#if LINUX != 1
    DEBUGF("%s() TXbuf=%ld - %ld = %ld\n", __PRETTY_FUNCTION__, sock_tbsize(s->socket), sock_tbused(s->socket), sock_tbleft(s->socket));
#endif  // LINUX == 1
    socket_flush(s);
}

/**
 * @brief wait on socket for incoming data with timeout.
 * socket.WaitInput([timeout:number = 1])
 *
 * @param J VM state.
 */
static void Socket_WaitInput(js_State *J) {
    int status;

    SOCK_USER_DATA(s);

    int timeout = 1;
    if (js_isnumber(J, 1)) {
        timeout = js_toint16(J, 1);
    }

    sock_wait_input(s->socket, timeout, NULL, &status);

sock_err:
    switch (status) {
        case 1: /*foreign host closed*/
            js_error(J, "Socket closed");
            break;
        case -1: /*time-out*/
            js_error(J, "Connection timed out");
            break;
    }
}

/**
 * @brief return the next byte from the socket as number.
 * socket.ReadByte():number
 *
 * @param J VM state.
 */
static void Socket_ReadByte(js_State *J) {
    SOCK_USER_DATA(s);

    int ch = sock_getc(s->socket);
    if (ch != EOF) {
        js_pushnumber(J, ch);
    } else {
        js_pushnull(J);
    }
}

/**
 * @brief write a byte to a socket.
 * socket.WriteByte(ch:number)
 *
 * @param J VM state.
 */
static void Socket_WriteByte(js_State *J) {
    SOCK_USER_DATA(s);

    sock_putc(s->socket, (char)js_toint16(J, 1));
}

/**
 * @brief send binary data.
 * socket.WriteBytes(data:number[])
 *
 * @param J VM state.
 */
static void Socket_WriteBytes(js_State *J) {
    SOCK_USER_DATA(s);

    if (js_isarray(J, 1)) {
        int len = js_getlength(J, 1);

        BYTE *data = malloc(len);
        if (!data) {
            JS_ENOMEM(J);
            return;
        }

        for (int i = 0; i < len; i++) {
            js_getindex(J, 1, i);
            data[i] = (BYTE)js_toint16(J, -1);
            js_pop(J, 1);
        }
        sock_write(s->socket, data, len);

        free(data);
    } else {
        JS_ENOARR(J);
    }
}

/**
 * @brief send binary data.
 * socket.WriteInts(data:ByteArray)
 *
 * @param J VM state.
 */
static void Socket_WriteInts(js_State *J) {
    SOCK_USER_DATA(s);
    JS_CHECKTYPE(J, 1, TAG_BYTE_ARRAY);

    if (js_isuserdata(J, 1, TAG_BYTE_ARRAY)) {
        byte_array_t *ba = js_touserdata(J, 1, TAG_BYTE_ARRAY);

        sock_write(s->socket, ba->data, ba->size);
    } else {
        JS_ENOARR(J);
    }
}

/**
 * @brief send string
 * socket.WriteString(data:string)
 *
 * @param J VM state.
 */
static void Socket_WriteString(js_State *J) {
    SOCK_USER_DATA(s);

    const char *line = js_tostring(J, 1);

    if (sock_puts(s->socket, (BYTE *)line) == 0) {
        js_error(J, "Error writing: %s", sockerr(s->socket));
        return;
    }
}

/**
 * @brief Set binary or ascii mode for UDP/TCP sockets.
 * - Effects sock_gets(), sock_dataready().
 * - Enable/disable UDP checksums.
 * socket.Mode(mode:number)
 *
 * @param J VM state.
 */
static void Socket_Mode(js_State *J) {
    SOCK_USER_DATA(s);

#if LINUX == 1
    js_error(J, "Modes not supported on Linux");
#else
    sock_mode(s->socket, js_touint16(J, 1));
#endif
}

/**
 * @brief Send pending TCP data.
 * socket.Flush([flushWait:boolean = false])
 *
 * @param J VM state.
 */
static void Socket_Flush(js_State *J) {
    SOCK_USER_DATA(s);

    sock_flush(s->socket);

    if (js_isboolean(J, 1) && js_toboolean(J, 1)) {
        socket_flush(s);
    }
}

/**
 * @brief Sets non-flush mode on next TCP write.
 * socket.NoFlush()
 *
 * @param J VM state.
 */
static void Socket_NoFlush(js_State *J) {
    SOCK_USER_DATA(s);

    sock_noflush(s->socket);
}

/**
 * @brief Causes next transmission to have a flush (PUSH bit set).
 * socket.FlushNext()
 *
 * @param J VM state.
 */
static void Socket_FlushNext(js_State *J) {
    SOCK_USER_DATA(s);

    sock_flushnext(s->socket);
}

/**
 * @brief returns number of bytes waiting to be read.
 * socket.DataReady():number
 *
 * @param J VM state.
 */
static void Socket_DataReady(js_State *J) {
    SOCK_USER_DATA(s);

    js_pushnumber(J, sock_dataready(s->socket));
}

/**
 * @brief check if the socket connection is established.
 * socket.Established():bool
 *
 * @param J VM state.
 */
static void Socket_Established(js_State *J) {
    SOCK_USER_DATA(s);

    js_pushboolean(J, sock_established(s->socket));
}

/**
 * @brief get the remote host ip
 * socket.GetRemoteHost():number[]
 *
 * @param J VM state.
 */
static void Socket_GetRemoteHost(js_State *J) {
    SOCK_USER_DATA(s);
    struct watt_sockaddr waddr;
    int len = sizeof(struct watt_sockaddr);
    if (_w32__getpeername(s->socket, &waddr, &len) == 0) {
        watt_pushipaddr(J, waddr.s_ip);
    } else {
        js_error(J, "Could not get remote address");
    }
}

/**
 * @brief get the local port number
 * socket.GetLocalPort():number
 *
 * @param J VM state.
 */
static void Socket_GetLocalPort(js_State *J) {
    SOCK_USER_DATA(s);

    struct watt_sockaddr waddr;
    int len = sizeof(struct watt_sockaddr);
    if (_w32__getsockname(s->socket, &waddr, &len) == 0) {
        js_pushnumber(J, waddr.s_port);
    } else {
        js_error(J, "Could not get local port");
    }
}

/**
 * @brief get the remote port number
 * socket.GetRemotePort():number
 *
 * @param J VM state.
 */
static void Socket_GetRemotePort(js_State *J) {
    SOCK_USER_DATA(s);

    struct watt_sockaddr waddr;
    int len = sizeof(struct watt_sockaddr);
    if (_w32__getpeername(s->socket, &waddr, &len) == 0) {
        js_pushnumber(J, waddr.s_port);
    } else {
        js_error(J, "Could not get remote port");
    }
}

/**
 * @brief return the next line from the socket as string.
 * socket.ReadLine():string
 *
 * @param J VM state.
 */
static void Socket_ReadLine(js_State *J) {
    SOCK_USER_DATA(s);

    char line[MAX_LINE_LENGTH + 1];
    WORD size = sock_gets(s->socket, (BYTE *)line, sizeof(line));
    if (size > 0) {
        js_pushstring(J, line);
    } else {
        js_pushnull(J);
    }
}

/**
 * @brief return data as string.
 * socket.ReadString(len:number):string
 *
 * @param J VM state.
 */
static void Socket_ReadString(js_State *J) {
    SOCK_USER_DATA(s);

    int32_t len = js_toint32(J, 1);
    if (len <= 0) {
        js_error(J, "Socket read length must be >= 0");
        return;
    }

    char *buff = malloc(len + 1);
    if (!buff) {
        JS_ENOMEM(J);
        return;
    }

    int read = sock_read(s->socket, (BYTE *)buff, len);
    buff[read] = 0;
    if (read) {
        js_pushstring(J, buff);
    } else {
        js_pushnull(J);
    }
    free(buff);
}

/**
 * @brief return data as array.
 * socket.ReadBytes(len:number):number[]
 *
 * @param J VM state.
 */
static void Socket_ReadBytes(js_State *J) {
    SOCK_USER_DATA(s);

    int32_t len = js_toint32(J, 1);
    if (len <= 0) {
        js_error(J, "Socket read length must be >= 0");
        return;
    }

    char *buff = malloc(len + 1);
    if (!buff) {
        JS_ENOMEM(J);
        return;
    }

    int read = sock_read(s->socket, (BYTE *)buff, len);
    if (read) {
        js_newarray(J);

        for (int i = 0; i < read; i++) {
            js_pushnumber(J, buff[i]);
            js_setindex(J, -2, i);
        }
    } else {
        js_pushnull(J);
    }
    free(buff);
}

/**
 * @brief return data as ByteArray
 * socket.ReadInts(len:number):ByteArray
 *
 * @param J VM state.
 */
static void Socket_ReadInts(js_State *J) {
    SOCK_USER_DATA(s);

    int32_t len = js_toint32(J, 1);
    if (len <= 0) {
        js_error(J, "Socket read length must be >= 0");
        return;
    }

    char *buff = malloc(len + 1);
    if (!buff) {
        JS_ENOMEM(J);
        return;
    }

    int read = sock_read(s->socket, (BYTE *)buff, len);
    if (read) {
        ByteArray_fromBytes(J, (uint8_t *)buff, read);
    } else {
        js_pushnull(J);
    }
    free(buff);
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize socket subsystem.
 *
 * @param J VM state.
 */
void init_socket(js_State *J) {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    if (!DOjS.params.no_tcpip) {
        js_newobject(J);
        {
            NPROTDEF(J, Socket, Close, 0);
            NPROTDEF(J, Socket, Mode, 0);
            NPROTDEF(J, Socket, WaitFlush, 0);
            NPROTDEF(J, Socket, Flush, 0);
            NPROTDEF(J, Socket, NoFlush, 0);
            NPROTDEF(J, Socket, FlushNext, 0);
            NPROTDEF(J, Socket, DataReady, 0);
            NPROTDEF(J, Socket, Established, 0);
            NPROTDEF(J, Socket, ReadByte, 0);
            NPROTDEF(J, Socket, ReadBytes, 0);
            NPROTDEF(J, Socket, ReadInts, 0);
            NPROTDEF(J, Socket, ReadLine, 0);
            NPROTDEF(J, Socket, GetLocalPort, 0);
            NPROTDEF(J, Socket, GetRemotePort, 0);
            NPROTDEF(J, Socket, GetRemoteHost, 0);
            NPROTDEF(J, Socket, WriteByte, 1);
            NPROTDEF(J, Socket, WriteBytes, 1);
            NPROTDEF(J, Socket, WriteInts, 1);
            NPROTDEF(J, Socket, WaitInput, 1);
            NPROTDEF(J, Socket, ReadString, 1);
            NPROTDEF(J, Socket, WriteString, 1);
        }
        CTORDEF(J, new_Socket, TAG_SOCKET, 3);
    }

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}

void tick_socket() {
    if (socket_count > 0) {
        tcp_tick(NULL);
    }
}
