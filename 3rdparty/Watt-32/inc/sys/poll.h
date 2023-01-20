/*!\file sys/poll.h
 *
 */
#ifndef __SYS_POLL_H
#define __SYS_POLL_H

#include <sys/w32api.h>  /* W32_FUNC, W32_DATA etc. */

#define POLLIN   0x0001
#define POLLPRI  0x0002   /* not used */
#define POLLOUT  0x0004
#define POLLERR  0x0008
#define POLLHUP  0x0010   /* not used */
#define POLLNVAL 0x0020   /* not used */

struct pollfd {
       int fd;
       int events;     /* in param: what to poll for */
       int revents;    /* out param: what events occured */
     };

W32_FUNC int W32_CALL poll (struct pollfd *p, int num, int timeout);

#endif
