/*!\file close.c
 * BSD close()
 */

/*
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

static int close_dgram  (Socket *socket);
static int close_stream (Socket *socket);

int W32_CALL close_s (int s)
{
  Socket *socket = _socklist_find (s);

  SOCK_PROLOGUE (socket, "\nclose_s:%d", s);

  socket->so_error = 0;

  switch (socket->so_type)
  {
    case SOCK_PACKET:
    case SOCK_RAW:
         SOCK_DEL_FD (s);
         return (0);

    case SOCK_DGRAM:
         return close_dgram (socket);

    case SOCK_STREAM:
         return close_stream (socket);
  }
  SOCK_ERRNO (EOPNOTSUPP);
  return (-1);
}

int W32_CALL closesocket (int s)
{
  return close_s (s);
}

/*
 * Close a SOCK_DGRAM socket
 */
static int close_dgram (Socket *socket)
{
  sock_type *sk = (sock_type*)socket->udp_sock;

  sk->udp.rx_datalen = 0;   /* flush Rx data */
  sock_close (sk);
  socket->so_state |= (SS_ISDISCONNECTING | SS_CANTSENDMORE);

  if (!socket->local_addr)
     SOCK_DEL_FD (socket->fd);
  return (0);
}

/*
 * Close a SOCK_STREAM socket
 */
static int close_stream (Socket *socket)
{
  sock_type *sk       = (sock_type*)socket->tcp_sock;
  BOOL abort_it       = FALSE;
  BOOL blocking_close = FALSE;
  int  i, s;

  if ((socket->so_state & SS_ISDISCONNECTING) && socket->close_time)
  {
    SOCK_DEBUGF ((", close already called"));
    SOCK_ERRNO (EBADF);
    return (-1);
  }

  if (socket->so_options & SO_LINGER)
  {
    if (socket->linger_time == 0)
    {
      abort_it = TRUE;
      sk = NULL;
      SOCK_DEBUGF ((", zero-linger RST"));
    }
    else if (socket->linger_time > 0 && socket->linger_on)
    {
      blocking_close = TRUE;
      SOCK_DEBUGF ((", blocking lingering close"));
    }
  }

  if (!socket->local_addr) /* Not bound or never received anything */
  {
    abort_it = TRUE;
    sk = NULL;
  }

  /* Save memory and abort listen() socket and queue now.
   */
  if (socket->so_options & SO_ACCEPTCONN)
  {
    SOCK_DEBUGF ((", listen abort, backlog %d", socket->backlog));
    abort_it = TRUE;
    sk = NULL;
    for (i = 0; i < socket->backlog && i < SOMAXCONN; i++)
    {
      sock_type *tcb = (sock_type*) socket->listen_queue[i];

      if (!tcb)
         continue;

      tcb->tcp.rx_datalen = 0;   /* flush Rx data */
      TCP_ABORT (&tcb->tcp);
      free (tcb);
      socket->listen_queue[i] = NULL;
    }
  }

  if (sk)
  {
    sk->tcp.rx_datalen = 0;
    sock_flush (sk);
    sock_close (sk);
  }

  s = socket->fd;

  if (abort_it)
  {
    SOCK_DEBUGF ((", fast kill!"));
    SOCK_DEL_FD (s);      /* calls _tcp_abort() */
  }
  else
  {
    /* sock_daemon() will free socket from list and inuse array.
     * Local port is marked for reuse after TCP_LINGERTIME (2min).
     */
    socket->so_state  |= (SS_ISDISCONNECTING | SS_CANTSENDMORE);
    socket->so_state  |= SS_CANTRCVMORE;  /* !! should we do this? */
    socket->close_time = time (NULL);
  }

  if (blocking_close)
  {
    while (_socklist_find(s)) /* while still alive */
    {
      tcp_tick (NULL);        /* poll, sock_daemon() will delete it */
      WATT_YIELD();
    }
  }
  return (0);
}
#endif /* USE_BSD_API */
