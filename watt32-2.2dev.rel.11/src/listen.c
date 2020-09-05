/*!\file listen.c
 * BSD listen().
 */

/*  BSD sockets functionality for Watt-32 TCP/IP
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
 *  0.6 : Aug 08, 2002 : G. Vanem - added AF_INET6 support
 */

#include "socket.h"

#if defined(USE_BSD_API)

int W32_CALL listen (int s, int backlog)
{
  Socket *socket = _socklist_find (s);

  SOCK_PROLOGUE (socket, "\nlisten:%d", s);

  if (!socket->local_addr)              /* bind() not called */
  {
    SOCK_DEBUGF ((", EINVAL"));
    SOCK_ERRNO (EINVAL);
    return (-1);
  }

  if (socket->so_type != SOCK_STREAM)
  {
    SOCK_DEBUGF ((", EOPNOTSUPP"));
    SOCK_ERRNO (EOPNOTSUPP);
    return (-1);
  }

  _sock_set_syn_hook (_sock_append);

#if defined(USE_IPV6)
  if (socket->so_family == AF_INET6)
  {
    struct in6_addr addr;

    memset (&addr, 0, sizeof(addr));  /* unspecified addr */
    _TCP6_listen (socket, &addr, socket->local_addr->sin_port);
  }
  else
#endif
  {
    struct in_addr addr;

    addr.s_addr = INADDR_ANY;
    _TCP_listen (socket, addr, socket->local_addr->sin_port);
  }

  SOCK_DEBUGF ((", port %d", ntohs(socket->local_addr->sin_port)));

  /* A listen socket should never time out unless
   * setsockopt(SO_RCVTIMEOUT) is called prior to
   * accept().
   */
  socket->timeout = 0;
  socket->so_state   |= SS_ISLISTENING;
  socket->so_options |= SO_ACCEPTCONN;

  /* legal backlog range [1..SOMAXCONN>
   */
  socket->backlog = min (backlog, SOMAXCONN-1);
  socket->backlog = max (socket->backlog, 1);
  SOCK_DEBUGF ((", backlog %d ", socket->backlog));
  return (0);
}

#if defined(TEST_PROG)  /* A simple finger server */

#ifndef __CYGWIN__
#include <conio.h>
#endif

#define FINGER_PORT 79

int main (void)
{
  struct sockaddr_in addr;
  int    serv_sock, cli_sock = -1;
  BOOL   quit = FALSE;

  dbug_init();

  serv_sock = socket (AF_INET, SOCK_STREAM, 0);
  if (serv_sock < 0)
  {
    perror ("socket");
    return (-1);
  }

  memset (&addr, 0, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_port        = htons (FINGER_PORT);
  addr.sin_addr.s_addr = htonl (INADDR_ANY);

  if (bind(serv_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
  {
    perror ("bind");
    close_s (serv_sock);
    return (-1);
  }

  if (listen(serv_sock, 5) == -1)
  {
    perror ("listen");
    close_s (serv_sock);
    return (-1);
  }

  printf ("Watt-32 finger server.\nPress ESC to quit\n");

  while (!quit)
  {
    struct timeval tv = { 1, 0 };
    struct sockaddr_in peer;
    const char *who = "Login   Name  Tty  Idle   Login  Time\r\n"
                      "root    root  *1          Aug 22 10:15\r\n"
                      "guest   guest *2          Aug 22 11:22\r\n";
    fd_set rd;
    int    len = sizeof(peer);
    int    num;

    if (kbhit() && getch() == 27)
       quit = TRUE;

    FD_ZERO (&rd);
    FD_SET (serv_sock, &rd);

    num = select_s (serv_sock+1, &rd, NULL, NULL, &tv);
    if (num < 0)
    {
      perror ("select_s");
      break;
    }
    if (num == 0)
    {
      putchar ('.');
      fflush (stdout);
      continue;
    }

    if (!FD_ISSET(serv_sock,&rd))
       continue;

    cli_sock = accept (serv_sock, (struct sockaddr*)&peer, &len);
    if (cli_sock < 0)
    {
      perror ("accept");
      break;
    }

    printf ("Finger request from %s (%u)\n",
            inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));

    if (send(cli_sock, who, strlen(who), 0) < 0)
    {
      perror ("write_s");
      break;
    }
    close_s (cli_sock);   /* ready for next client */
    cli_sock = -1;
  }

  if (cli_sock != -1)
     close_s (cli_sock);
  close_s (serv_sock);
  return (0);
}
#endif  /* TEST_PROG */
#endif  /* USE_BSD_API */
