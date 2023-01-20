/*!\file echo.c
 *
 *  A simple echo/discard daemon. Listens for traffic on
 *  udp/tcp ports 7 and 9.
 */

/*
 *  Copyright (c) 1997-2002 Gisle Vanem <gvanem@yahoo.no>
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. All advertising materials mentioning features or use of this software
 *     must display the following acknowledgement:
 *       This product includes software developed by Gisle Vanem
 *       Bergen, Norway.
 *
 *  THIS SOFTWARE IS PROVIDED BY ME (Gisle Vanem) AND CONTRIBUTORS ``AS IS''
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL I OR CONTRIBUTORS BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "wattcp.h"
#include "pcconfig.h"
#include "pctcp.h"
#include "pcdbug.h"
#include "misc.h"
#include "misc_str.h"
#include "run.h"
#include "netaddr.h"
#include "echo.h"

#if defined(USE_ECHO_DISC)

static _udp_Socket *udp_echo_sock, *udp_disc_sock;
static _tcp_Socket *tcp_echo_sock, *tcp_disc_sock;

static BOOL  do_echo   = 0;
static WORD  echo_port = 7;
static DWORD echo_host = 0;

static BOOL  do_disc   = 0;
static WORD  disc_port = 9;
static DWORD disc_host = 0;

static void (W32_CALL *prev_hook) (const char*, const char*) = NULL;

static void W32_CALL echo_discard_daemon (void);

static void W32_CALL udp_handler (sock_type *s, void *data, int len,
                                  const tcp_PseudoHeader *ph,
                                  const void *udp_hdr);

/*
 * Parse and match "echo.daemon = 0/1", "echo.port = <n>" etc.
 */
static void W32_CALL echo_config (const char *name, const char *value)
{
  static const struct config_table echo_cfg[] = {
            { "DAEMON", ARG_ATOI,    (void*)&do_echo   },
            { "HOST",   ARG_RESOLVE, (void*)&echo_host },
            { "PORT",   ARG_ATOI,    (void*)&echo_port },
            { NULL,     0,           NULL              }
          };
  static const struct config_table disc_cfg[] = {
            { "DAEMON", ARG_ATOI,    (void*)&do_disc   },
            { "HOST",   ARG_RESOLVE, (void*)&disc_host },
            { "PORT",   ARG_ATOI,    (void*)&disc_port },
            { NULL,     0,           NULL              }
          };

  if (!parse_config_table(&echo_cfg[0], "ECHO.", name, value) &&
      !parse_config_table(&disc_cfg[0], "DISCARD.", name, value))
  {
    if (prev_hook)
      (*prev_hook) (name, value);
  }
}

/**
 * Don't bother resetting any connections. Just remove daemon and
 * free allocated memory.
 */
static void W32_CALL echo_discard_exit (void)
{
  DAEMON_DEL (echo_discard_daemon);
  if (udp_echo_sock)
     free (udp_echo_sock);
  if (tcp_echo_sock)
     free (tcp_echo_sock);
  if (udp_disc_sock)
     free (udp_disc_sock);
  if (tcp_disc_sock)
     free (tcp_disc_sock);
}

/**
 * Called from watt_sock_init(): Setup config-file parser for "echo..."
 * and "discard.." keywords.
 */
void echo_discard_init (void)
{
  prev_hook = usr_init;
  usr_init  = echo_config;
  udp_echo_sock = udp_disc_sock = NULL;
  tcp_echo_sock = tcp_disc_sock = NULL;
  RUNDOWN_ADD (echo_discard_exit, 258);
}

/**
 * Starts the echo/discard services (udp/tcp).
 */
void echo_discard_start (void)
{
  if (do_echo)
  {
    udp_echo_sock = malloc (sizeof(*udp_echo_sock));
#if !defined(USE_UDP_ONLY)
    tcp_echo_sock = malloc (sizeof(*tcp_echo_sock));
#endif
    if (!udp_echo_sock && !tcp_echo_sock)
       do_echo = FALSE;

    if (udp_echo_sock)
    {
      udp_listen (udp_echo_sock, echo_port, echo_host, 0,
                  (ProtoHandler)udp_handler);
      udp_echo_sock->sockmode &= ~SOCK_MODE_UDPCHK;  /* no checksum testing */
    }
#if !defined(USE_UDP_ONLY)
    if (tcp_echo_sock)
       tcp_listen (tcp_echo_sock, echo_port, echo_host, 0, NULL, 0);
#endif
  }

  if (do_disc)
  {
    udp_disc_sock = malloc (sizeof(*udp_disc_sock));
#if !defined(USE_UDP_ONLY)
    tcp_disc_sock = malloc (sizeof(*tcp_disc_sock));
#endif
    if (!udp_disc_sock && !tcp_disc_sock)
       do_disc = FALSE;

    if (udp_disc_sock)
    {
      udp_listen (udp_disc_sock, disc_port, disc_host, 0,
                  (ProtoHandler)udp_handler);
      udp_disc_sock->sockmode &= ~SOCK_MODE_UDPCHK;
    }
#if !defined(USE_UDP_ONLY)
    if (tcp_disc_sock)
       tcp_listen (tcp_disc_sock, disc_port, disc_host, 0, NULL, 0);
#endif
  }

  if (do_echo || do_disc)
     DAEMON_ADD (echo_discard_daemon);
}

/**
 * "background" process handling echo + discard TCP sockets.
 * \todo handle jumbo packets (IP-fragments)
 */
static void W32_CALL echo_discard_daemon (void)
{
  sock_type *s = (sock_type*) tcp_echo_sock;

  if (!s || !tcp_disc_sock)
     return;

  if (sock_dataready(s))  /* only echo back from echo-socket */
  {
    BYTE buf[MAX_IP4_DATA-sizeof(tcp_Header)];
    int  len = sock_read (s, buf, sizeof(buf));
    sock_write (s, buf, len);
  }
  tcp_disc_sock->rx_datalen = 0;  /* discard recv data */
}

/**
 * Callback handler for echo + discard UDP sockets.
 */
static void W32_CALL udp_handler (sock_type *s, void *data, int len,
                                  const tcp_PseudoHeader *ph,
                                  const void *udp)
{
  if (s == (sock_type*)udp_echo_sock)
  {
    TRACE_CONSOLE (2, "echo (udp): looping %d bytes from %s\n",
                   len, _inet_ntoa(NULL,intel(ph->src)));

    if (!sock_enqueue(s, (const BYTE*)data, len))
       sock_close (s);
  }
  else   /* discard packet */
  {
    TRACE_CONSOLE (2, "discard (udp): dropping %d bytes from %s\n",
                   len, _inet_ntoa(NULL,intel(ph->src)));
  }
  ARGSUSED (ph);
  ARGSUSED (udp);
}
#endif /* USE_ECHO_DISC */

#if defined(TEST_PROG)
int main (int argc, char **argv)
{
  /** \todo Make a \b echo client/server test program */

#if !defined(USE_ECHO_DISC)
  puts ("This program needs '#define USE_ECHO_DISC'");
#endif
  ARGSUSED (argc);
  ARGSUSED (argv);
  return (0);
}
#endif /* TEST_PROG */


