/*!\file stream.c
 *  sock_fgets(), sock_fputs().
 *
 *  BSD sockets functionality for Watt-32 TCP/IP
 *
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
 *  Version
 *
 *  0.5 : Dec 18, 1997 : G. Vanem - created
 */

#include "socket.h"

#if defined(USE_BSD_API)

/*
 * How does this fit in with djgpp's FSextensions ?
 * Does it support fdopen() ?
 */

int sock_fgets (char *buf, int max, FILE *stream)
{
  int        rc, s = fileno (stream);
  Socket    *sock  = _socklist_find (s);
  sock_type *sk    = NULL;

  SOCK_PROLOGUE (sock, "\nsock_fgets:%d", s);

  if (sock->tcp_sock)
     sk = (sock_type*) sock->tcp_sock;
  else if (sock->udp_sock)
     sk = (sock_type*) sock->udp_sock;

  rc = sk ? sock_gets (sk, (BYTE*)buf, max) : -1;

  SOCK_DEBUGF ((", rc %d", rc));
  return (rc);
}

int sock_fputs (const char *text, FILE *stream)
{
  int     s    = fileno (stream);
  Socket *sock = _socklist_find (s);

  SOCK_PROLOGUE (sock, "\nsock_fputs:%d", s);
  return write_s (s, text, strlen(text));
}

#endif  /* USE_BSD_API */
