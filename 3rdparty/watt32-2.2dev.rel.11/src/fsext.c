/*!\file fsext.c
 * File System extensions for djgpp.
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
 *  0.5 : Jan 21, 1998 : G. Vanem - created
 *  0.6 : Apr 28, 2003 : G. Vanem - updates for djgpp 2.04
 */

#include "socket.h"

#if defined(__DJGPP__) && defined(USE_BSD_API) && defined(USE_FSEXT)

#include <sys/fsext.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef S_IFSOCK
#define S_IFSOCK 0x10000
#endif

#if (DJGPP_MINOR < 4)
#define blksize_t off_t
#endif

struct wstat {
       time_t    st_atime;
       time_t    st_ctime;
       dev_t     st_dev;
       gid_t     st_gid;
       ino_t     st_ino;
       mode_t    st_mode;
       time_t    st_mtime;
       nlink_t   st_nlink;
       off_t     st_size;
       blksize_t st_blksize;
       uid_t     st_uid;
       dev_t     st_rdev;
     };


#if defined(USE_DEBUG)
/*
 * Handle printing of function names
 */
static const struct search_list fs_func[] = {
                      { __FSEXT_nop,   "nop"    },
                      { __FSEXT_open,  "open"   },
                      { __FSEXT_creat, "creat"  },
                      { __FSEXT_read,  "read"   },
                      { __FSEXT_write, "write"  },
                      { __FSEXT_ready, "ready"  },
                      { __FSEXT_close, "close"  },
                      { __FSEXT_fcntl, "fcntl"  },
                      { __FSEXT_ioctl, "ioctl"  },
                      { __FSEXT_lseek, "lseek"  },
                      { __FSEXT_link,  "link"   },
                      { __FSEXT_unlink,"unlink" },
#if (DJGPP_MINOR >= 2)
                      { __FSEXT_dup,   "dup"    },
                      { __FSEXT_dup2,  "dup2"   },
                      { __FSEXT_fstat, "fstat"  },
                      { __FSEXT_stat,  "stat"   },
#endif
#if (DJGPP_MINOR >= 4)
                      { __FSEXT_llseek,  "llseek"   },
                      { __FSEXT_readlink,"readlink" },
                      { __FSEXT_symlink, "symlink"  },
                      { __FSEXT_fchown,  "fchown"   },
                      { __FSEXT_chmod,   "chmod",   },
                      { __FSEXT_chown,   "chown",   },
                      { __FSEXT_fchmod,  "fchmod"   }
#endif
                    };
#endif /* USE_DEBUG */

/*
 * Filesystem extensions for BSD-sockets and djgpp 2.x
 */
int _fsext_demux (__FSEXT_Fnumber func, int *rv, va_list _args)
{
  int     fd   = va_arg (_args, int);
  int     cmd  = va_arg (_args, int);
  int     arg  = va_arg (_args, int);
  Socket *sock = __FSEXT_get_data (fd); /* same as _socklist_find(fd) */

  SOCK_DEBUGF (("\n_fsext_demux: fd %d%c, func %d = \"%s\", cmd %08lX, arg %08lX",
                fd, sock ? 's' : 'f', func,
                list_lookup(func, fs_func, DIM(fs_func)),
                (DWORD)cmd, (DWORD)arg));

  /* fd is not a valid socket, pass on to lower layer
   */
  if (!sock)
     return (0);

  switch (func)
  {
    case __FSEXT_read:                        /* read (fd,&buf,len) */
         *rv = read_s (fd, (char*)cmd, arg);
         return (1);

    case __FSEXT_write:                       /* write (fd,&buf,len) */
         *rv = write_s (fd, (char*)cmd, arg);
         return (1);

    case __FSEXT_close:                       /* close (fd) */
         if (sock->fd_duped == 0)
            *rv = close_s (fd);
         else
         {
           sock->fd_duped--;
           *rv = 0;
         }
         return (1);

    case __FSEXT_ioctl:   /* ioctl (fd,cmd,...) */
         *rv = ioctlsocket (fd, cmd, (char*)arg);
         return (1);

    case __FSEXT_fcntl:   /* 'fcntl (fd,cmd)' or 'fcntl (fd,cmd,arg)' */
         *rv = fcntlsocket (fd, cmd, (long)arg);
         return (1);

    case __FSEXT_ready:   /* ready (fd) called from select() */
         {
           struct  timeval tv;
           fd_set  fd_read, fd_write, fd_err;

           FD_ZERO (&fd_read);
           FD_ZERO (&fd_write);
           FD_ZERO (&fd_err);
           FD_SET (fd, &fd_read);
           FD_SET (fd, &fd_write);
           FD_SET (fd, &fd_err);
           tv.tv_sec  = 0;
           tv.tv_usec = 0L;
           *rv = 0;

           if (select_s (fd+1, &fd_read, &fd_write, &fd_err, &tv) >= 0)
           {
             if (FD_ISSET(fd,&fd_read))
                *rv |= __FSEXT_ready_read;
             if (FD_ISSET(fd,&fd_write))
                *rv |= __FSEXT_ready_write;
             if (FD_ISSET(fd,&fd_err))
                *rv |= __FSEXT_ready_error;
           }
           else
             *rv = -1;
         }
         return (1);

    case __FSEXT_creat:
         return (0);

#if (DJGPP_MINOR >= 2)    /* functions added in v2.02 */
    case __FSEXT_dup:     /* dup (fd) */
         *rv = fd;
         if (sock->fd_duped < UINT_MAX)
            sock->fd_duped++;
         return (1);

    case __FSEXT_dup2:    /* dup2 (oldfd,newfd) */
         close (cmd);     /* close (newd) */
         *rv = socket (sock->so_family, sock->so_type, sock->so_proto);
         return (1);

    case __FSEXT_fstat:
#if defined(USE_STATISTICS)
         {
           struct wstat *stat = (struct wstat*) cmd;
           int    size;

           *rv = 0;
           stat->st_mode = S_IFSOCK;

           /* Hmm. If user really want this, lets return it */
           switch (sock->so_proto)
           {
             case IPPROTO_TCP:
                  size = min (sizeof(tcpstats), sizeof(struct stat));
                  memcpy (rv, &tcpstats, size);
                  break;
             case IPPROTO_UDP:
                  size = min (sizeof(udpstats), sizeof(struct stat));
                  memcpy (rv, &udpstats, size);
                  break;
             case IPPROTO_ICMP:
                  size = min (sizeof(icmpstats), sizeof(struct stat));
                  memcpy (rv, &icmpstats, size);
                  break;
             case IPPROTO_IGMP:
                  size = min (sizeof(igmpstats), sizeof(struct stat));
                  memcpy (rv, &igmpstats, size);
                  break;
             case IPPROTO_IP:
                  size = min (sizeof(ip4stats), sizeof(struct stat));
                  memcpy (rv, &ip4stats, size);
                  break;
#if defined(IPPROTO_IP6)
             case IPPROTO_IP6:
                  size = min (sizeof(ip6stats), sizeof(struct stat));
                  memcpy (rv, &ip6stats, size);
                  break;
#endif
             default:
                 *rv = -1;
           }
         }
         return (1);
#else
         return (0);
#endif  /* USE_STATISTICS */

#endif  /* DJGPP_MINOR >= 2 */

    default:
         SOCK_DEBUGF ((" unhandled FSext function"));
         break;
  }
  return (0);    /* unhandled, pass on to lower layer */
}

#endif /* __DJGPP__ && USE_BSD_API && USE_FSEXT */

