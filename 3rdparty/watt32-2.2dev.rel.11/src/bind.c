/*!\file bind.c
 * BSD bind().
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
 *  0.6 : Aug 06, 2002 : G. Vanem - added AF_INET6 support
 */

#include "socket.h"

#if defined(USE_BSD_API)

/*!
 * \brief bind() - bind name to socket.
 *
 *  The purpose of bind is to fill in the information for the local IP
 *  address and local port (local_addr). In this respect, it is the
 *  opposite of connect(). connect() fills in the destination IP address
 *  and destination port (remote_addr).
 *  In most situations, one does not need to call bind().
 *
 *  Calling bind() on a \pre SOCK_DGRAM socket and for multiple addresses
 *  (\b 'myaddr->sin_addr.s_addr == INADDR_ANY' \b) requires special attention:
 *  Wattcp's 'udp_handler()' doesn't store address information for
 *  broadcast.
 *
 *  We therefore install a 's->dataHandler' pointing to '_recvdaemon()'
 *  which queues up SOCK_DGRAM messages. This queue is polled (by
 *  'sock_recv_from()') when we call 'receive()' on such a socket.
 *  Thus the '*from' address in 'receive()' will be correctly set to
 *  source address/port of peer.
 */
int W32_CALL bind (int s, const struct sockaddr *myaddr, socklen_t namelen)
{
  Socket              *socket = _socklist_find (s);
  struct sockaddr_in  *addr   = (struct sockaddr_in*) myaddr;
  struct sockaddr_in6 *addr6  = (struct sockaddr_in6*) myaddr;
  BOOL   bind_any, is_bcast, is_mcast, our_addr;
  WORD   local_port;
  BOOL   is_ip6;
  int    sa_len;

  SOCK_PROLOGUE (socket, "\nbind:%d", s);

  is_ip6 = (socket->so_family == AF_INET6);
  sa_len = is_ip6 ? sizeof(*addr6) : sizeof(*addr);

  if (_sock_chk_sockaddr(socket, myaddr, namelen) < 0)
     return (-1);

  if (socket->so_type == SOCK_STREAM)
  {
    if (is_ip6)
         is_mcast = IN6_IS_ADDR_MULTICAST (&addr6->sin6_addr);
    else is_mcast = IN_MULTICAST (ntohl(addr->sin_addr.s_addr));
    if (is_mcast)
    {
      SOCK_DEBUGF ((", EINVAL (mcast)"));
      SOCK_ERRNO (EINVAL);
      return (-1);
    }
  }

  /* binding to any address/port on local machine?
   */
#if defined(USE_IPV6)
  if (is_ip6)
  {
    our_addr = _ip6_is_local_addr (&addr6->sin6_addr);
    bind_any = IN6_ARE_ADDR_EQUAL (&addr6->sin6_addr, &in6addr_any);
    is_bcast = IN6_IS_ADDR_MC_GLOBAL (&addr6->sin6_addr);   /* ?? */
  }
  else
#endif
  {
    our_addr = _ip4_is_local_addr (ntohl(addr->sin_addr.s_addr));
    bind_any = (addr->sin_addr.s_addr == INADDR_ANY);
    is_bcast = ((~addr->sin_addr.s_addr & ~sin_mask) == 0);
  }

  if (socket->so_type == SOCK_STREAM &&
      !bind_any && !our_addr && !is_bcast)
  {
    SOCK_DEBUGF ((", EADDRNOTAVAIL"));
    SOCK_ERRNO (EADDRNOTAVAIL);
    return (-1);
  }

  if (socket->local_addr)
  {
    struct in6_addr *ip6 = &((struct sockaddr_in6*) socket->local_addr)->sin6_addr;
    u_short port  = socket->local_addr->sin_port;
    u_long  ip4   = socket->local_addr->sin_addr.s_addr;
    BOOL    equal = (!is_ip6 && ip4 == addr->sin_addr.s_addr) ||
                   (is_ip6 && IN6_ARE_ADDR_EQUAL(ip6,&addr6->sin6_addr));

    if ((addr->sin_port != IPPORT_ANY && addr->sin_port == port) ||
        (!bind_any && equal))
    {
      SOCK_DEBUGF ((", EADDRINUSE, local port %d, bind_any %d",
                     ntohs(port), bind_any));
      SOCK_ERRNO (EADDRINUSE);
      return (-1);
    }

    /** \todo check for "sleeping" ports (lport_inuse in pctcp.c)
     *        also. And return EADDRINUSE if local port not free.
     */
  }
  else
  {
    socket->local_addr = (struct sockaddr_in*) SOCK_CALLOC (sa_len);
    if (!socket->local_addr)
    {
      SOCK_DEBUGF ((", ENOMEM"));
      SOCK_ERRNO (ENOMEM);
      return (-1);
    }
  }

#if defined(USE_IPV6)
  if (is_ip6 && addr6->sin6_port == IPPORT_ANY)
  {
    local_port = find_free_port (0, TRUE);
    addr6->sin6_port = htons (local_port);
  }
  else
#endif
  if (!is_ip6 && addr->sin_port == IPPORT_ANY)
  {
    local_port = find_free_port (0, TRUE);
    addr->sin_port = htons (local_port);
  }
  else  /* check if requested port is vacant */
  {
    local_port = ntohs (addr->sin_port);
    if (grab_localport(local_port) < 0)
    {
      SOCK_DEBUGF ((", EADDRNOTAVAIL, local port %d", local_port));
      SOCK_ERRNO (EADDRNOTAVAIL);
      return (-1);
    }
  }

#if defined(USE_IPV6)
  if (is_ip6)
  {
    struct sockaddr_in6 *la = (struct sockaddr_in6*)socket->local_addr;

    la->sin6_family = AF_INET6;
    la->sin6_port   = addr6->sin6_port;
    memcpy (&la->sin6_addr, &addr6->sin6_addr, sizeof(la->sin6_addr));

    SOCK_DEBUGF ((", %s, (%d)",
                  bind_any ? "INADDR_ANY" : _inet6_ntoa(&addr6->sin6_addr),
                  ntohs(addr->sin_port)));
  }
  else
#endif
  {
    socket->local_addr->sin_port   = addr->sin_port;
    socket->local_addr->sin_addr   = addr->sin_addr;
    socket->local_addr->sin_family = AF_INET;

    SOCK_DEBUGF ((", %s, (%d)",
                  bind_any ? "INADDR_ANY" : inet_ntoa(addr->sin_addr),
                  ntohs(addr->sin_port)));
  }

  /* Since SOCK_DGRAM sockets are connectionless, the application need
   * not use connect() or accept(). Hence we need to use _UDP_listen().
   */
  if (socket->so_type == SOCK_DGRAM)
  {
    if (bind_any)
       socket->so_state |= SS_PRIV; /* privileged for broadcast reception */

#if defined(USE_IPV6)
    if (is_ip6)
    {
      if (_UDP6_listen (socket, &addr6->sin6_addr, addr6->sin6_port) < 0)
         return (-1);
    }
    else
#endif
    if (_UDP_listen (socket, addr->sin_addr, addr->sin_port) < 0)
       return (-1);
  }
  return (0);
}


/*
 * A small test djgpp program
 */
#if defined(TEST_PROG)

#define MY_PORT_ID  6060
#undef  close

#if !defined(__CYGWIN__)
#include <conio.h>
#endif

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif

#include "pcdbug.h"

int main (int argc, char **argv)
{
  struct sockaddr_in addr;
  int    sock, quit;

  dbug_init();

  sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock < 0)
  {
    perror ("socket");
    return (-1);
  }

  memset (&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;

  if (argc >= 2 && !strcmp(argv[1],"-frag")) /* test Tx of large datagrams */
  {
    #define CHUNK_SIZE 500
    char    data [3*CHUNK_SIZE];
    size_t  i;

    for (i = 0; i < sizeof(data);  i++)
        data[i] = i;

    addr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
    addr.sin_port        = IPPORT_ANY;

    if (sendto (sock, &data, sizeof(data), 0, (struct sockaddr*)&addr, sizeof(addr)) < 0)
       perror ("sendto");

    close (sock);
    return (-1);
  }

  /* INADDR_ANY will take all the address of the system
   */
  addr.sin_addr.s_addr = htonl (INADDR_ANY);
  addr.sin_port        = htons (MY_PORT_ID);

  if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
  {
    perror ("bind");
    close (sock);
    return (-1);
  }

#if 0
  if (listen (sock, 5) < 0)
  {
    perror ("listen");
    close (sock);
    return (-1);
  }
#endif

  quit = 0;

  while (!quit)
  {
    struct timeval  tv;
    struct sockaddr from;
    fd_set fd_read, fd_write, fd_exc;
    int    num, from_len = sizeof(from);

    FD_ZERO (&fd_read);
    FD_ZERO (&fd_write);
    FD_ZERO (&fd_exc);
    FD_SET (STDIN_FILENO, &fd_read);
    FD_SET (sock, &fd_read);
    FD_SET (sock, &fd_write);
    FD_SET (sock, &fd_exc);
    tv.tv_usec = 0;
    tv.tv_sec  = 1;

    num = select (sock+1, &fd_read, &fd_write, &fd_exc, &tv);

    if (FD_ISSET(sock, &fd_read))  fputc ('r', stderr);
    if (FD_ISSET(sock, &fd_write)) fputc ('w', stderr);
    if (FD_ISSET(sock, &fd_exc))   fputc ('x', stderr);

    if (FD_ISSET(STDIN_FILENO, &fd_read))
    {
      int ch = getch();
      quit = (ch == 27);
      fputc (ch, stderr);
    }

    if (FD_ISSET(sock, &fd_read) &&
        accept (sock, &from, &from_len) < 0)
    {
      perror ("accept");
      break;
    }
    if (num < 0)
    {
      perror ("select");
      break;
    }
    fputc ('.', stderr);
    usleep (300000);   /* 300ms */
  }

  close (sock);
  return (0);
}
#endif  /* TEST_PROG */

#endif /* USE_BSD_API */

