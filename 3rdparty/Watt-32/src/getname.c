/*!\file getname.c
 * BSD getsockname(), getpeername().
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
 *  0.6 : Aug 07, 2002 : G. Vanem - added AF_INET6 support
 */

#include "socket.h"

#if defined(USE_BSD_API)

int W32_CALL getsockname (int s, struct sockaddr *name, socklen_t *namelen)
{
  Socket   *socket = _socklist_find (s);
  socklen_t sa_len;

  SOCK_PROLOGUE (socket, "\ngetsockname:%d", s);

  if (!socket)
     goto einval;

  sa_len = (socket->so_family == AF_INET6) ? sizeof(struct sockaddr_in6) :
                                             sizeof(struct sockaddr_in);

  if (!name || !namelen || (*namelen < sa_len))
  {
einval:
    SOCK_DEBUGF ((", EINVAL"));
    SOCK_ERRNO (EINVAL);
    if (namelen)
       *namelen = 0;
    return (-1);
  }

  if (!socket->local_addr)
  {
    SOCK_DEBUGF ((", EINVAL"));
    SOCK_ERRNO (EINVAL);       /* according to HP/UX manpage */
    return (-1);
  }

  VERIFY_RW (name, *namelen);

  *namelen = sa_len;
  memcpy (name, socket->local_addr, sa_len);

#if defined(USE_IPV6)
  if (socket->so_family == AF_INET6)
  {
    const struct sockaddr_in6 *la = (const struct sockaddr_in6*)socket->local_addr;

    SOCK_DEBUGF ((", %s (%d)", _inet6_ntoa(&la->sin6_addr), ntohs(la->sin6_port)));
    ARGSUSED (la);
  }
  else
#endif
  {
    const struct sockaddr_in *la = socket->local_addr;

    SOCK_DEBUGF ((", %s (%d)", inet_ntoa(la->sin_addr), ntohs(la->sin_port)));
    ARGSUSED (la);
  }
  return (0);
}

int W32_CALL getpeername (int s, struct sockaddr *name, socklen_t *namelen)
{
  Socket   *socket = _socklist_find (s);
  socklen_t sa_len;

  SOCK_PROLOGUE (socket, "\ngetpeername:%d", s);

  sa_len = (socket->so_family == AF_INET6) ? sizeof(struct sockaddr_in6) :
                                             sizeof(struct sockaddr_in);

  if (!name || !namelen || (*namelen < sa_len))
  {
    SOCK_DEBUGF ((", EINVAL"));
    SOCK_ERRNO (EINVAL);
    if (namelen)
       *namelen = 0;
    return (-1);
  }

  if (!(socket->so_state & SS_ISCONNECTED))
  {
    SOCK_DEBUGF ((", ENOTCONN"));
    SOCK_ERRNO (ENOTCONN);
    return (-1);
  }

  VERIFY_RW (name, *namelen);

  *namelen = sa_len;
  memcpy (name, socket->remote_addr, *namelen);

#if defined(USE_IPV6)
  if (socket->so_family == AF_INET6)
  {
    const struct sockaddr_in6 *ra = (const struct sockaddr_in6*)socket->remote_addr;

    SOCK_DEBUGF ((", %s (%d)", _inet6_ntoa(&ra->sin6_addr), ntohs(ra->sin6_port)));
    ARGSUSED (ra);
  }
  else
#endif
  {
    const struct sockaddr_in *ra = socket->remote_addr;

    SOCK_DEBUGF ((", %s (%d)", inet_ntoa(ra->sin_addr), ntohs(ra->sin_port)));
    ARGSUSED (ra);
  }
  return (0);
}
#endif
