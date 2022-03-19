/*!\file poll.c
 * BSD poll().
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
 *  1.0 : Sep 3, 1999 : G. Vanem
 *
 *  This function is not in BSD, but Linux-specific (to aid
 *  porting Linux applications to Watt-32).
 *
 *  poll() taken from HTTP-tunnel:
 *
 *  Copyright (C) 1999 Lars Brinkhoff <lars@nocrew.org>.
 *
 */

#include <sys/poll.h>
#include "socket.h"

#if defined(USE_BSD_API)

#undef read
#undef write
#undef except

#ifdef __HIGHC__     /* set warning for stack-usage */
#pragma stack_size_warn (16000)    /* ~3*MAX_SOCKETS */
#endif

int poll (struct pollfd *p, int num, int timeout)
{
  struct timeval tv;
  fd_set read  [NUM_SOCK_FDSETS];
  fd_set write [NUM_SOCK_FDSETS];
  fd_set except[NUM_SOCK_FDSETS];
  int    i, n, ret;

  memset (read, 0, sizeof(read));
  memset (write, 0, sizeof(write));
  memset (except, 0, sizeof(except));

  SOCK_DEBUGF (("\npoll: num %d, to %d", num, timeout));
  SOCK_ENTER_SCOPE();

  if (num > MAX_SOCKETS)
  {
    SOCK_DEBUGF ((", EINVAL"));
    SOCK_ERRNO (EINVAL);
    return (-1);
  }

  n = -1;
  for (i = 0; i < num; i++)
  {
    if (p[i].fd < 0)
       continue;
    if (p[i].events & POLLIN)
       FD_SET (p[i].fd, &read[0]);
    if (p[i].events & POLLOUT)
       FD_SET (p[i].fd, &write[0]);
    if (p[i].events & POLLERR)
       FD_SET (p[i].fd, &except[0]);
    if (p[i].fd > n)
       n = p[i].fd;
  }

  if (n == -1)
     ret = 0;

  else if (timeout < 0)
     ret = select_s (n+1, read, write, except, NULL);

  else
  {
    tv.tv_sec  = timeout / 1000;
    tv.tv_usec = 1000 * (timeout % 1000);
    ret = select_s (n+1, read, write, except, &tv);
  }

  for (i = 0; ret >= 0 && i < num; i++)
  {
    p[i].revents = 0;
    if (FD_ISSET (p[i].fd, &read[0]))
       p[i].revents |= POLLIN;
    if (FD_ISSET (p[i].fd, &write[0]))
       p[i].revents |= POLLOUT;
    if (FD_ISSET (p[i].fd, &except[0]))
       p[i].revents |= POLLERR;
  }

  SOCK_DEBUGF (("\nrc %d", ret));
  SOCK_LEAVE_SCOPE();
  return (ret);
}
#endif /* USE_BSD_API */
