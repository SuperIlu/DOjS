/*!\file sys/un.h
 *
 * Definitions for UNIX domain sockets.
 *
 * Only added to keep the porting of *some* Posix programs simpler.
 * Watt-32 does *not* implement Unix-sockets at all.
 */
#ifndef __SYS_UN_H
#define __SYS_UN_H

#include <string.h>       /* for strlen */

#ifndef __SYS_SOCKET_H
#include <sys/socket.h>   /* for sa_family_t */
#endif

/* POSIX requires only at least 100 bytes
 */
#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX  108
#endif

struct sockaddr_un {
       sa_family_t sun_family;                 /* address family AF_LOCAL/AF_UNIX */
       char        sun_path [UNIX_PATH_MAX];   /* room for socket address */
     };

/* Evaluates the actual length of "sockaddr_un" structure.
 */
#define SUN_LEN(p)  ( (size_t) (((struct sockaddr_un*) NULL)->sun_path) + strlen((p)->sun_path) )

#endif
