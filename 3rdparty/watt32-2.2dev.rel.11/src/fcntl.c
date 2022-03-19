/*!\file fcntl.c
 * BSD fcntlsocket().
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
 *  0.5 : 19 Aug 1999 : by Claus Oberste-Brandenburg
 *                        <Claus.OBERSTE-BRANDENBURG@jacala.ensmp.fr>
 *
 *  0.6 : 19 Aug 1999 : Changed function prototype, added fd_duped (GV)
 */

#include "socket.h"

#if defined(USE_BSD_API)

#ifndef O_NONBLOCK
#define O_NONBLOCK 0x2000
#endif

#if defined(USE_DEBUG)
/*
 * Return string for fcntlsocket() command
 */
static const struct search_list commands[] = {
           { F_DUPFD,  "F_DUPFD"  },
           { F_GETFL,  "F_GETFL"  },
           { F_SETFL,  "F_SETFL"  },
           { F_GETFD,  "F_GETFD"  },
           { F_SETFD,  "F_SETFD"  },
           { F_GETLK,  "F_GETLK"  },
           { F_SETLK,  "F_SETLK"  },
           { F_SETLKW, "F_SETLKW" },
           { F_GETOWN, "F_GETOWN" },
           { F_SETOWN, "F_SETOWN" }
         };
#endif  /* USE_DEBUG */


int MS_CDECL fcntlsocket (int s, int cmd, ...)
{
  Socket *socket = _socklist_find (s);
  size_t  arg;   /* same size as a pointer */
  int     rc = 0;
  va_list va;

  va_start (va, cmd);
  arg = va_arg (va, size_t);

  SOCK_PROLOGUE (socket, "\nfcntlsocket:%d, ", s);

  SOCK_DEBUGF ((", %s: ", list_lookup(cmd,commands,DIM(commands))));

  /* fcntl is always file-ioctrl
   */
  switch (cmd)
  {
    case F_DUPFD:
         socket->fd_duped++;
         SOCK_DEBUGF (("dup %d", socket->fd_duped));
         return (0);

    case F_GETFL:
         /** \todo handle \b O_TEXT and \b O_BINARY
          */
         if (socket->so_state & SS_NBIO)
         {
           SOCK_DEBUGF (("non-blocking"));
           return (O_NONBLOCK);
         }
         SOCK_DEBUGF (("blocking"));
         return (0);

    case F_SETFL:
         if (arg & O_NONBLOCK)  /* O_APPEND will not be served */
         {
           SOCK_DEBUGF (("non-blocking"));
           socket->so_state |= SS_NBIO;
           socket->timeout = 0;
           if (socket->tcp_sock)
              socket->tcp_sock->timeout = 0;
         }
         else
         {
           SOCK_DEBUGF (("blocking"));
           socket->so_state &= ~SS_NBIO;
           socket->timeout = sock_delay;
         }
         return (0);

    case F_GETFD:
         return (0); /* socket remains open across exec() */

    case F_SETFD:
    case F_GETLK:
    case F_SETLK:
    case F_SETLKW:
    case F_GETOWN:
    case F_SETOWN:
         SOCK_DEBUGF (("not supported"));
         SOCK_ERRNO (ESOCKTNOSUPPORT);
         rc = -1;
         break;

    default:
         SOCK_ENTER_SCOPE();
         if (ioctlsocket(s, cmd, (char*)arg) < 0)
         {
           SOCK_DEBUGF ((", unknown cmd %d", cmd));
           SOCK_ERRNO (ESOCKTNOSUPPORT);
           rc = -1;
         }
         SOCK_LEAVE_SCOPE();
  }
  return (rc);
}
#endif  /* USE_BSD_API */

