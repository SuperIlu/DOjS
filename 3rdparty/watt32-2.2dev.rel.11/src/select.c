/*!\file select.c
 * BSD select().
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
 *  0.6 : Nov 05, 1999 : G. Vanem - several changes;
 *                       Protect select-loop as critical region.
 *                       Changed criteria for read/writeability.
 */

#include "socket.h"

#if defined(USE_BSD_API)

#ifdef __HIGHC__     /* set warning for stack-usage */
#pragma stack_size_warn (16000)    /* ~3*MAX_SOCKETS */
#endif

#define ALWAYS_YIELD  0

#ifndef STDIN_FILENO
#define STDIN_FILENO  0
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif


/**
 * The smallest time for \b not yielding or calling tcp_tick().
 * \todo Should be configurable.
 */
static struct timeval sel_min_block = { 0, 500000 }; /* 0.5 sec */

/*
 * Select sockets for read, write or exceptions
 *   Returns 1 if socket is ready or
 *   Returns 0 if socket isn't selectable
 */
static int read_select  (int s, Socket *socket);
static int write_select (int s, Socket *socket);
static int exc_select   (int s, Socket *socket);

/*
 * Setup for read/write/except_select()
 */
static __inline Socket *setup_select (int s, BOOL first_loop)
{
  Socket *socket = _socklist_find (s);

  if (first_loop && !socket)
  {
    if (_sock_dos_fd(s))
    {
      SOCK_DEBUGF ((", ENOTSOCK (%d)", s));
      SOCK_ERRNO (ENOTSOCK);
    }
    else
    {
      SOCK_DEBUGF ((", EBADF (%d)", s));
      SOCK_ERRNO (EBADF);
    }
  }
  return (socket);
}

/**
 * The select_s() function.
 *
 * \note It supports more sockets than would fit in a fd_set.
 * I.e. it handles an array of 'fd_set's on input/output.
 * Thus it is the user's responsibilty to not use a too high
 * 'nfds' value. I.e. If nfs > 512*8 and user fd_set's on input
 * is smaller than 512*, select_s() could trash the fd_set's
 * on output.
 */
int W32_CALL select_s (int nfds, fd_set *readfds, fd_set *writefds,
                       fd_set *exceptfds, struct timeval *timeout)
{
  fd_set tmp_read  [NUM_SOCK_FDSETS];
  fd_set tmp_write [NUM_SOCK_FDSETS];
  fd_set tmp_except[NUM_SOCK_FDSETS];
  struct timeval starttime, expiry, now;

  int  num_fd    = nfds;
  int  ret_count = 0;
  BOOL expired   = FALSE;
  BOOL loop_1st  = TRUE;
  int  s, loops;

#if defined(USE_DEBUG)
  unsigned total_rd = 0;
  unsigned total_wr = 0;
  unsigned total_ex = 0;
#endif

  SOCK_DEBUGF (("\nselect_s: n=0-%d, %c%c%c", num_fd-1,
                readfds   ? 'r' : '-',
                writefds  ? 'w' : '-',
                exceptfds ? 'x' : '-'));


  /* num_fd == 0 is permitted. Often used to perform delays:
   *   select (0,NULL,NULL,NULL,&tv);
   *
   * Some programs uses -1 to mean all possible sockets.
   */
  if (num_fd > MAX_SOCKETS || num_fd < 0)
      num_fd = MAX_SOCKETS;

  if (timeout)
  {
    if ((long)timeout->tv_sec < 0 || timeout->tv_usec < 0)
    {
      SOCK_DEBUGF ((", EINVAL (negative timeout)"));
      SOCK_ERRNO (EINVAL);
      return (-1);
    }

    gettimeofday2 (&starttime, NULL); /* initialize start time */

    expiry.tv_sec  = starttime.tv_sec  + timeout->tv_sec;
    expiry.tv_usec = starttime.tv_usec + timeout->tv_usec;
    while (expiry.tv_usec >= 1000000L)
    {
      expiry.tv_usec -= 1000000L;
      expiry.tv_sec++;
    }

    SOCK_DEBUGF ((", timeout %lu.%06lds",
                  (u_long)timeout->tv_sec, timeout->tv_usec));
  }
  else
    SOCK_DEBUGF ((", timeout undef"));


  /**
   * Clear our "working" fd_sets.
   * \note Don't use FD_ZERO() since our working fd_sets contain
   *       more than FD_SETSIZE sockets.
   */
  memset (tmp_read, 0, sizeof(tmp_read));
  memset (tmp_write, 0, sizeof(tmp_write));
  memset (tmp_except, 0, sizeof(tmp_except));

  /* If application catches same signals we do, we must exit
   * gracefully from the do-while and for loops below.
   */
  if (_sock_sig_setup() < 0)
     goto select_intr;

  /*
   * Loop until specified timeout expires or event(s) satisfied.
   */
  for (loop_1st = TRUE, loops = 1;; loop_1st = FALSE, loops++)
  {
    /* Poll for caught signals (SIGINT/SIGALRM)
     */
    if (_sock_sig_pending())
       goto select_intr;

    for (s = 0; s < num_fd; s++)
    {
      /* read/write/except counters for socket 's'
       */
      int     do_read  = 0, do_write  = 0, do_exc  = 0;
      int     read_cnt = 0, write_cnt = 0, exc_cnt = 0;
      Socket *socket = NULL;

      /* Not safe to run sock_daemon() (or other "tasks") now
       */
      _sock_crit_start();

      if (readfds && FD_ISSET(s,readfds))
         do_read = 1;

      if (writefds && FD_ISSET(s,writefds))
         do_write = 1;

      if (exceptfds && FD_ISSET(s,exceptfds))
         do_exc = 1;

      if (do_read || do_write || do_exc)
      {
        tcp_tick (NULL);      /* must do tcp_tick() here */
        if (s >= SK_FIRST)
        {
          socket = setup_select (s, loop_1st);
          if (!socket)        /* skip this fd */
             do_read = do_write = do_exc = 0;
        }
      }

      /* Check this socket for readability ?
       */
      if (do_read)
      {
        read_cnt = read_select (s, socket);
        if (read_cnt > 0)
           FD_SET (s, &tmp_read[0]);
      }

      /* Check this socket for writeability ?
       */
      if (do_write)
      {
        write_cnt = write_select (s, socket);
        if (write_cnt > 0)
           FD_SET (s, &tmp_write[0]);
      }

      /* Check this socket for exception ?
       */
      if (do_exc)
      {
        exc_cnt = exc_select (s, socket);
        if (exc_cnt > 0)
           FD_SET (s, &tmp_except[0]);
      }

      /* Increment the return and total counters (may increment by 0)
       */
      ret_count += (read_cnt + write_cnt + exc_cnt);

#if defined(USE_DEBUG)
      total_rd += read_cnt;
      total_wr += write_cnt;
      total_ex += exc_cnt;
#endif

      /* Safe to run other "tasks" now.
       */
      _sock_crit_stop();
      SOCK_DBUG_FLUSH();

    } /* end of for loop; all sockets checked at least once */


    /* WATT_YIELD() sometimes hangs for approx 250msec under Win-XP.
     * Don't yield for "small" timeouts.
     */
#if (!ALWAYS_YIELD)
    if (!timeout || timercmp(timeout,&sel_min_block,>))
#endif
    {
      if (ret_count == 0)
      {
        WATT_YIELD();
        tcp_tick (NULL);
      }
    }

    if (timeout)
    {
      gettimeofday2 (&now, NULL);
      if (now.tv_sec > expiry.tv_sec ||
          (now.tv_sec == expiry.tv_sec && now.tv_usec >= expiry.tv_usec))
        expired = TRUE;
    }

    /* If atleast 1 event is set.
     */
    if (ret_count > 0)
    {
      SOCK_DEBUGF ((", cnt=%d (%dr/%dw/%dx)",
                    ret_count, total_rd, total_wr, total_ex));

      /* Copy our working fd_sets to output fd_sets
       */
      for (s = 0; s < num_fd; s++)
      {
        if (readfds)
        {
          if (FD_ISSET(s, &tmp_read[0]))
               FD_SET (s, readfds);
          else FD_CLR (s, readfds);
        }
        if (writefds)
        {
          if (FD_ISSET(s, &tmp_write[0]))
               FD_SET (s, writefds);
          else FD_CLR (s, writefds);
        }
        if (exceptfds)
        {
          if (FD_ISSET(s, &tmp_except[0]))
               FD_SET (s, exceptfds);
          else FD_CLR (s, exceptfds);
        }
      }

      /* Do as Linux and return the time left of the period.
       * NB! The 'tv_sec' can be negative if select_s() took too long.
       */
      if (timeout)
      {
        double remaining = timeval_diff (&now, &expiry);

        timeout->tv_sec  = (long)(remaining / 1E6);
        timeout->tv_usec = (long)remaining % 1000000UL;
      }
      goto select_ok;
    }

    if (expired)
    {
      SOCK_DEBUGF ((", timeout!: %.6fs", timeval_diff(&now, &starttime)/1E6));

      if (readfds)
         for (s = 0; s < num_fd; s++)
             FD_CLR (s, readfds);

      if (writefds)
         for (s = 0; s < num_fd; s++)
             FD_CLR (s, writefds);

      if (exceptfds)
         for (s = 0; s < num_fd; s++)
             FD_CLR (s, exceptfds);

      ret_count = 0;   /* should already be 0 */
      goto select_ok;
    }
  }

select_intr:
  SOCK_DEBUGF ((", EINTR"));
  SOCK_ERRNO (EINTR);
  ret_count = -1;
  _sock_crit_stop();

select_ok:
  _sock_sig_restore();

  return (ret_count);
}

#if defined(WIN32)
int W32_CALL select (int nfds, fd_set *read_fds, fd_set *write_fds,
                     fd_set *except_fds, struct timeval *timeout)
{
  return select_s (nfds, read_fds, write_fds, except_fds, timeout);
}
#endif


#ifdef NOT_YET
int pselect (int nfds, fd_set *readfds, fd_set *writefds,
             fd_set *exceptfds, struct timespec *timeout,
             const sigset_t *sigmask)
{
  struct timeval tv;
  sigset_t old_mask;
  int      rc;

  if (timeout)
  {
    tv.tv_sec  = tv_sec;
    tv_tv_nsec = 1000UL * tv_usec;
  }
  if (sigmask)
     sigprocmask (SIG_BLOCK, sigmask, &old_mask);

  rc = select (nfds, readfs, writefds, exceptfds, timeout ? &tv : NULL);

  if (sigmask)
     sigprocmask (SIG_UNBLOCK, &old_mask, NULL);

  return (rc);
}
#endif

/*
 * Check listen-queue for first connected TCB.
 * Only called for listening (accepting) sockets.
 */
static __inline int listen_queued (Socket *socket)
{
  int i;

  for (i = 0; i < socket->backlog && i < DIM(socket->listen_queue); i++)
  {
    _tcp_Socket *tcb = socket->listen_queue[i];

    if (!tcb)
       continue;

    /* Socket has reached Established state or receive data above
     * low water mark. This means, socket may have reached Closed,
     * but this still counts as a readable event.
     */
    if (tcb->state == tcp_StateESTAB ||
        sock_rbused((sock_type*)tcb) > socket->recv_lowat)
       return (1);
  }
  return (0);
}

/*
 * Return TRUE if socket has a "real" error and not a "pending" error.
 * I.e. EALREADY is not a real error, but a pending condition until a
 * non-blocking socket is actually connected. Or if connection fails,
 * in which case the error is ECONNREFUSED.
 *
 * This signalled read/write state is assumed to persist for the
 * remaining life of the socket.
 */
#define READ_STATE_MASK   (SS_CANTRCVMORE)  /* set in recv() or shutdown() */
#define WRITE_STATE_MASK  (SS_CANTSENDMORE | SS_ISCONNECTED)

static __inline int sock_signalled (Socket *socket, int mask)
{
  if (socket->so_state & mask)
     return (1);

  /* A normal blocking socket. 'so_error' is set in
   * connect(), a "ICMP Unreachable" callback or when RST received
   * in pctcp.c. Otherwise 'so_error' is zero.
   */
  if (!(socket->so_state & SS_NBIO))
     return (socket->so_error);

#if 0
  if (socket->so_options & SO_ACCEPTCONN) /* non-blocking listen sock */
     return (0);

  if (socket->so_error == EALREADY)       /* temporary non-blocking error */
     return (0);
  return (1);
#else
  return (0);
#endif
}

#if defined(__MSDOS__)
/*
 * Check if a standard handle is ready.
 * This should return TRUE if handle is redirected.
 */
static BOOL handle_ready (int hnd)
{
  union REGS regs;

#if defined(__HIGHC__) || defined(__DMC__)
  regs.x.ax = 0x4406;
  regs.x.bx = hnd;
#else
  regs.w.ax = 0x4406;
  regs.w.bx = hnd;
#endif

  intdos (&regs, &regs);
  return (regs.h.al == 0xFF);  /* ready character device */
}
#else
#define handle_ready(hnd)  0
#endif

/*
 * Check if 's' can be read from.
 */
static int read_select (int s, Socket *socket)
{
  if (s == STDIN_FILENO)
  {
#if defined(__DJGPP__) && 0
    struct timeval tv = { 0, 0 };
    fd_set rd;
    int    rc;

    FD_ZERO (&rd);
    FD_SET (STDIN_FILENO, &rd);
    rc = (select) (1, &rd, NULL, NULL, &tv);
    return (rc == 1 ? 1 : 0);
#else
    return (watt_kbhit() || handle_ready(STDIN_FILENO));
#endif
  }

  if (s <= STDERR_FILENO)
     return (0);

  if (socket->so_type == SOCK_PACKET)
     return sock_packet_rbused (socket) ? 1 : 0;

  if (socket->so_type == SOCK_RAW)
  {
#if defined(USE_IPV6)
    if (socket->so_family == AF_INET6)
       return sock_rbused ((sock_type*)socket->raw6_sock) ? 1 : 0;
#endif
    return sock_rbused ((sock_type*)socket->raw_sock) ? 1 : 0;
  }

  if (socket->so_type == SOCK_DGRAM)
  {
    size_t len;

    if (socket->so_state & SS_PRIV)
         len = sock_recv_used ((sock_type*)socket->udp_sock);
    else len = sock_rbused ((sock_type*)socket->udp_sock);

    if (len > socket->recv_lowat ||
        sock_signalled(socket,READ_STATE_MASK))
       return (1);
    return (0);
  }

  if (socket->so_type == SOCK_STREAM)
  {
    sock_type *sk = (sock_type*) socket->tcp_sock;

    if ((socket->so_state & SS_NBIO) &&
        sk->tcp.state == tcp_StateESTAB &&
        socket->so_error == EALREADY)
       socket->so_error = EISCONN;

    if (sock_signalled(socket,READ_STATE_MASK) || /* signalled for read_s() */
        sk->tcp.state >= tcp_StateLASTACK      || /* got FIN from peer */
        sock_rbused(sk) > socket->recv_lowat)     /* Rx-data above low-limit */
       return (1);

    if ((socket->so_options & SO_ACCEPTCONN) &&   /* connection pending */
        listen_queued(socket))
       return (1);
  }
  return (0);
}

/*
 * Check if 's' can be written to.
 */
static int write_select (int s, Socket *socket)
{
#if defined(__DJGPP__) && 0
  if (s == STDOUT_FILENO || s == STDERR_FILENO)
  {
    struct timeval tv = { 0, 0 };
    fd_set wr;
    int    rc;

    FD_ZERO (&wr);
    FD_SET (s, &wr);
    rc = (select) (1, NULL, &wr, NULL, &tv);
    return (rc == 1 ? 1 : 0);
  }
#else
  if (s == STDOUT_FILENO)
     return !ferror (stdout);

  if (s == STDERR_FILENO)
     return !ferror (stderr);
#endif

  if (s == STDIN_FILENO)
     return (0);

  /* SOCK_PACKET, SOCK_RAW and SOCK_DGRAM sockets are always writable
   */
  if (socket->so_type == SOCK_PACKET ||
      socket->so_type == SOCK_RAW    ||
      socket->so_type == SOCK_DGRAM)
     return (1);

  if (socket->so_type == SOCK_STREAM)
  {
    sock_type *sk = (sock_type*) socket->tcp_sock;

    if (sk->tcp.state == tcp_StateESTAB)
    {
      if ((socket->so_state & SS_NBIO) && socket->so_error == EALREADY)
         socket->so_error = EISCONN;

      if (sock_tbleft(sk) > socket->send_lowat)  /* Tx room above low-limit */
         return (1);
    }
    if (sk->tcp.state >= tcp_StateESTCL)
       return (1);

    if (sock_signalled(socket,WRITE_STATE_MASK)) /* signalled for write */
       return (1);
  }
  return (0);
}

/*
 * Check 's' for exception or faulty condition.
 */
static int exc_select (int s, Socket *socket)
{
  if (s < SK_FIRST)
     return (0);

  /** \todo Only arrival of OOB-data should count here
   */
#if 0
  if (sock_signalled(socket, READ_STATE_MASK|WRITE_STATE_MASK))
     return (1);
#endif

  ARGSUSED (socket);
  return (0);
}
#endif /* USE_BSD_API */


/*
 * A small test program (for djgpp/Watcom/HighC/DMC)
 */
#if defined(TEST_PROG)

#include <time.h>

#ifndef __CYGWIN__
#include <conio.h>
#endif

#ifndef _MSC_VER
#include <unistd.h>
#endif

void usage (const char *argv0)
{
  fprintf (stderr,
           "Usage: %s [-s select_size] [-w micro-sec] [-hid]\n"
           "\t\t -s  set the number of fd's to select for (default 1)\n"
           "\t\t -w  time to wait in select (default 0.5)\n"
           "\t\t -h  don't use 8254 hi-resolution timer function\n"
           "\t\t -i  use timer ISR (init_timer_isr)\n"
           "\t\t -d  write debug to wattcp.sk/wattcp.dbg\n", argv0);
  exit (-1);
}

char *fd_set_str (char *buf, size_t len, const fd_set *fd)
{
  char *p   = buf;
  char *end = buf + len;
  int   i, num;

#if 0
  p += sprintf (p, "%d: ", fd->fd_count);
#endif

  for (i = 0, num = 0; i < 8*FD_SETSIZE; i++)
  {
    if (FD_ISSET(i,fd))
    {
      p += sprintf (p, "%d,", i);
      num++;
    }
    if (p > end - 12)
    {
      strcat(p, "<overflow>");
      break;
    }
  }
  if (p < end - 12)
     strcat (p, "<none set>");
  return (buf);
}

void select_dump (int sock_err, int num_fd, const struct timeval *tv,
                  const fd_set *fd_read,
                  const fd_set *fd_write,
                  const fd_set *fd_except)
{
  char buf1[512], buf2[512], buf3[512];

  printf ("select_dump: sock_err %d, num_fd %d, timeout %ld.06%lu\n"
          "\tread-fds:   %s\n"
          "\twrite-fds:  %s\n"
          "\texcept-fds: %s\n",
          sock_err, num_fd, (long)tv->tv_sec, tv->tv_usec,
          fd_set_str(buf1,sizeof(buf1),fd_read),
          fd_set_str(buf2,sizeof(buf2),fd_write),
          fd_set_str(buf3,sizeof(buf3),fd_except));
}

int main (int argc, char **argv)
{
  BOOL   use_8254  = TRUE;
  BOOL   use_isr   = FALSE;
  BOOL   debug     = FALSE;
  double sel_wait  = 0.5;
  int    sel_size  = 1;
  int    ch;

  while ((ch = getopt(argc, argv, "s:w:hid?")) != EOF)
    switch (ch)
    {
      case 'h':
           use_8254 = FALSE;
           break;

      case 'i':
#ifdef MSDOS
           init_timer_isr();
           use_isr = TRUE;
#endif
           break;

      case 's':
           sel_size = atoi (optarg);
           break;

      case 'd':
           dbug_init();
           debug = TRUE;
           break;

      case 'w':
           sel_wait = atof (optarg);
           break;

      case '?':
      default:
           usage (argv[0]);
           break;
    }

  if (sel_size <= 0 || sel_size > MAX_SOCKETS)
  {
    printf ("Illegal select-size %d. Range 0 - %d\n", sel_size, MAX_SOCKETS);
    return (-1);
  }

  sock_init();

  printf ("select-wait %.3f sec, select-size %d, debug %d, ",
          sel_wait, sel_size, debug);

#if defined(MSDOS)
  printf ("has_8254 %d, uses_int8 %d\n", has_8254, use_isr);
  hires_timer (use_8254);
#endif

  puts ("Press 'q' to quit");

  while (1)
  {
    fd_set read [NUM_SOCK_FDSETS];
    struct timeval tv;
    uclock_t start, diff;

    tv.tv_sec  = (time_t)sel_wait;
    tv.tv_usec = 1000000UL * (sel_wait - (double)tv.tv_sec);
    start = uclock();
    memset (read, 0, sizeof(read));
    FD_SET (STDIN_FILENO, &read[0]);

    if (select_s (sel_size, read, NULL, NULL, &tv) < 0)
    {
      perror ("select");
      break;
    }

    diff = uclock() - start;
    if (debug)
       SOCK_DEBUGF ((", diff: %.6fs", (double)diff/(double)UCLOCKS_PER_SEC));

    fputc ('.', stderr);
    usleep (100000UL);

    if (FD_ISSET(0,&read[0]))
    {
      int ch = getch();
      fputc (ch, stderr);
      if (ch == 'q')
         break;
    }
  }
  ARGSUSED (use_8254);
  ARGSUSED (use_isr);
  return (0);
}
#endif  /* TEST_PROG */
