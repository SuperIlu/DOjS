/*!\file accept.c
 * BSD accept().
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
 *  0.6 : Sep 16, 1999 : fixes by Claus Oberste-Brandenburg
 *  0.7 : Nov 22, 1999 : G. Vanem - simplified the socket 'dup' action.
 *                       Simply allocate a new 'remote_addr'.
 *                       Poll backlogged listen-queue in accept-loop.
 *  0.8 : Dec 15, 1999 : Listen-queue filled in _sock_append() and TCB
 *                       is cloned when SYN is received in tcp_listen_state().
 *  0.9 : Mar 09, 2000 : Plugged a memory leak in dup_bind() where 'tcp_sock'
 *                       memory from socket() wasn't free'ed. Thanks to
 *                       Francisco Pastor <fpastor.etra-id@etra.es> for
 *                       finding this.
 *  0.91: Jun 01, 2000 : Rearranged accept-loop for EWOULDBLOCK with no
 *                       listen_queue element (dropped 'continue' construct).
 *  0.92: Aug 07, 2002 : G. Vanem - added AF_INET6 support
 */

#include "socket.h"

#if defined(USE_BSD_API)

static int  dup_bind   (Socket *socket, Socket **clone, int idx);
static int  alloc_addr (Socket *socket, Socket  *clone);
static void listen_free(Socket *socket, int idx);

int W32_CALL accept (int s, struct sockaddr *addr, socklen_t *addrlen)
{
  Socket  *socket, *clone = NULL;
  volatile DWORD   timeout;
  volatile int     newsock = -1;
  volatile int     que_idx;
  volatile int     maxconn;
  volatile BOOL    is_ip6;

  socket = _socklist_find (s);

  SOCK_PROLOGUE (socket, "\naccept:%d", s);

  is_ip6 = (socket->so_family == AF_INET6);

  if (socket->so_type != SOCK_STREAM)
  {
    SOCK_DEBUGF ((", EOPNOTSUPP"));
    SOCK_ERRNO (EOPNOTSUPP);
    return (-1);
  }

  if (!socket->local_addr)
  {
    SOCK_DEBUGF ((", not bound"));
    SOCK_ERRNO (ENOTCONN);
    return (-1);
  }

  if (!(socket->so_options & SO_ACCEPTCONN)) /* listen() not called */
  {
    SOCK_DEBUGF ((", not SO_ACCEPTCONN"));
    SOCK_ERRNO (EINVAL);
    return (-1);
  }

  if (!(socket->so_state & (SS_ISLISTENING | SS_ISCONNECTING)))
  {
    SOCK_DEBUGF ((", not listening"));
    SOCK_ERRNO (ENOTCONN);
    return (-1);
  }

  if (addr && addrlen)
  {
    socklen_t sa_len = is_ip6 ? sizeof(struct sockaddr_in6) :
                                sizeof(struct sockaddr_in);
    if (*addrlen < sa_len)
    {
      SOCK_DEBUGF ((", EFAULT"));
      SOCK_ERRNO (EFAULT);
      return (-1);
    }
    VERIFY_RW (addr, sa_len);
  }

  /* Get max possible TCBs on listen-queue.
   * Some (or all) may be NULL until a SYN comes in.
   */
  maxconn = socket->backlog;
  if (maxconn < 1 || maxconn > SOMAXCONN)
  {
    SOCK_FATAL (("%s(%d): Illegal socket backlog %d\n",
                __FILE__, __LINE__, maxconn));
    SOCK_ERRNO (EINVAL);
    return (-1);
  }

  /* A listening socket should have infinite timeout unless
   * option SO_RCVTIMEO used. I.e. socket->timeout = 0.
   * Ref. listen.c.
   */
  if (socket->timeout)
       timeout = set_timeout (1000 * socket->timeout);
  else timeout = 0UL;

  if (_sock_sig_setup() < 0)
  {
    SOCK_ERRNO (EINTR);
    goto accept_fail;
  }

  _sock_crit_start();

  /* Loop over all queue-slots and accept first connected TCB
   */
  for (que_idx = 0; ; que_idx = (que_idx+1) % maxconn)
  {
    _tcp_Socket *sk = socket->listen_queue [que_idx];

    tcp_tick (NULL);

    WATT_YIELD();

    if (_sock_sig_pending())
    {
      SOCK_DEBUGF ((", EINTR"));
      SOCK_ERRNO (EINTR);
      goto accept_fail;
    }

    /* No SYNs received yet. This shouldn't happen if we called 'accept()'
     * after 'select_s()' said that socket was readable. (At least one
     * connection on the listen-queue).
     */
    if (sk)
    {
      /* This could happen if 'accept()' was called too long after connection
       * was established and then closed by peer. This could also happen if
       * someone did a portscan on us. I.e. he sent 'SYN', we replied with
       * 'SYN+ACK' and he never sent an 'ACK'. Thus we timeout in
       * 'tcp_Retransmitter()' and abort the TCB.
       *
       * Queue slot is in any case ready for another 'SYN' to come and be
       * handled by '_sock_append()'.
       */
      if (sk->state >= tcp_StateESTCL && sk->ip_type == 0)
      {
        SOCK_DEBUGF ((", aborted TCB (idx %d)", que_idx));
        listen_free (socket, que_idx);
        continue;
      }

      /** \todo Should maybe loop over all maxconn TCBs and accept the
       *        one with oldest 'syn_timestamp'.
       */
      if (sk->state >= tcp_StateESTAB && sk->state < tcp_StateCLOSED)
      {
        SOCK_DEBUGF ((", connected! (idx %d)", que_idx));
        break;
      }
    }

    /* We've polled all listen-queue slots and none are connected.
     * Return fail if socket is non-blocking.
     */
    if (que_idx == maxconn-1 && (socket->so_state & SS_NBIO))
    {
      SOCK_DEBUGF ((", would block"));
      SOCK_ERRNO (EWOULDBLOCK);
      goto accept_fail;
    }

    if (chk_timeout(timeout))
    {
      SOCK_DEBUGF ((", ETIMEDOUT"));
      SOCK_ERRNO (ETIMEDOUT);
      goto accept_fail;
    }
  }

  /* We're here only when above 'tcp_established()' succeeded.
   * Now duplicate 'socket' into a new listening socket 'clone'
   * with handle 'newsock'.
   */
  SOCK_ENTER_SCOPE();
  newsock = dup_bind (socket, &clone, que_idx);
  if (newsock < 0)
     goto accept_fail;

  if (!alloc_addr(socket, clone))
  {
    SOCK_DEL_FD (newsock);
    goto accept_fail;
  }

  /* Clone is connected, but *not* listening/accepting.
   * Note: other 'so_state' bits from parent is unchanged.
   *       e.g. clone may be non-blocking.
   */
  clone->so_state   |=  SS_ISCONNECTED;
  clone->so_state   &= ~(SS_ISLISTENING | SS_ISCONNECTING);
  clone->so_options &= ~SO_ACCEPTCONN;

#if 1
  /* Prevent a PUSH on first segment sent.
   */
  sock_noflush ((sock_type*)clone->tcp_sock);
#endif

#if defined(USE_IPV6)
  if (is_ip6)
  {
    struct sockaddr_in6 *ra = (struct sockaddr_in6*)clone->remote_addr;

    SOCK_DEBUGF (("\nremote %s (%u)",
                  _inet6_ntoa (&ra->sin6_addr), ntohs(ra->sin6_port)));
    ARGSUSED (ra);
  }
  else
#endif
    SOCK_DEBUGF (("\nremote %s (%u)",
                  inet_ntoa (clone->remote_addr->sin_addr),
                  ntohs (clone->remote_addr->sin_port)));

  if (addr && addrlen)
  {
#if defined(USE_IPV6)
    if (socket->so_family == AF_INET6)
    {
      struct sockaddr_in6 *sa = (struct sockaddr_in6*)addr;

      memset (sa, 0, sizeof(*sa));
      memcpy (&sa->sin6_addr, &clone->remote_addr->sin_addr, sizeof(sa->sin6_addr));
      sa->sin6_family   = AF_INET6;
      sa->sin6_port     = clone->remote_addr->sin_port;
      sa->sin6_flowinfo = sa->sin6_scope_id = 0;  /* !! */
      *addrlen = sizeof(*sa);
    }
    else
#endif
    {
      struct sockaddr_in *sa = (struct sockaddr_in*)addr;

      sa->sin_family = AF_INET;
      sa->sin_port   = clone->remote_addr->sin_port;
      sa->sin_addr   = clone->remote_addr->sin_addr;
      memset (sa->sin_zero, 0, sizeof(sa->sin_zero));
      *addrlen = sizeof(*sa);
    }
  }

  SOCK_LEAVE_SCOPE();
  _sock_crit_stop();
  _sock_sig_restore();
  return (newsock);

accept_fail:
  SOCK_LEAVE_SCOPE();
  _sock_crit_stop();
  _sock_sig_restore();
  return (-1);
}


/*
 * Duplicate a SOCK_STREAM 'sock' to '*newconn'. Doesn't set
 * local/remote addresses. Transfer TCB from listen-queue[idx] of
 * 'sock' to TCB of 'clone'.
 */
static int dup_bind (Socket *sock, Socket **newconn, int idx)
{
  Socket *clone;
  int     fd = socket (sock->so_family, SOCK_STREAM, IPPROTO_TCP);

  if (fd < 0)
     return (fd);

  clone = _socklist_find (fd);  /* cannot fail */

  /* child gets state from parent
   */
  clone->timeout    = sock->timeout;
  clone->close_time = sock->close_time;
  clone->keepalive  = sock->keepalive;
  clone->ip_tos     = sock->ip_tos;
  clone->ip_ttl     = sock->ip_ttl;
  clone->so_state   = sock->so_state;
  clone->so_options = sock->so_options;

  /* TCB for clone is from listen-queue[idx]; free tcp_sock from
   * socket(). Reuse listen-queue slot for another SYN.
   */
  free (clone->tcp_sock);
  clone->tcp_sock = sock->listen_queue[idx];
  sock->listen_queue [idx] = NULL;
  sock->syn_timestamp[idx] = 0UL;
  *newconn = clone;
  return (fd);
}

/*
 * Allocate and fill local/remote addresses for 'clone'.
 * Take local address from 'socket', and remote address from
 * TCB of clone.
 */
static int alloc_addr (Socket *socket, Socket *clone)
{
  BOOL is_ip6 = (socket->so_family == AF_INET6);
  int  sa_len = is_ip6 ? sizeof(struct sockaddr_in6) :
                         sizeof(struct sockaddr_in);

  clone->local_addr = SOCK_CALLOC (sa_len);
  if (!clone->local_addr)
  {
    SOCK_DEBUGF ((", ENOMEM"));
    SOCK_ERRNO (ENOMEM);
    return (0);
  }

  clone->remote_addr = SOCK_CALLOC (sa_len);
  if (!clone->remote_addr)
  {
    SOCK_DEBUGF ((", ENOMEM"));
    SOCK_ERRNO (ENOMEM);
    free (clone->local_addr);
    clone->local_addr = NULL;
    return (0);
  }

#if defined(USE_IPV6)
  if (is_ip6)
  {
    struct sockaddr_in6 *la = (struct sockaddr_in6*) clone->local_addr;
    struct sockaddr_in6 *ra = (struct sockaddr_in6*) clone->remote_addr;
    struct sockaddr_in6 *sa = (struct sockaddr_in6*) socket->local_addr;

    la->sin6_family = AF_INET6;
    la->sin6_port   = sa->sin6_port;
    memcpy (&la->sin6_addr, &sa->sin6_addr, sizeof(la->sin6_addr));

    ra->sin6_family = AF_INET6;
    ra->sin6_port   = htons (clone->tcp_sock->hisport);
    memcpy (&ra->sin6_addr, &clone->tcp_sock->his6addr, sizeof(ra->sin6_addr));
  }
  else
#endif
  {
    struct in_addr peer;

    peer.s_addr = htonl (clone->tcp_sock->hisaddr);
    clone->local_addr->sin_family  = AF_INET;
    clone->local_addr->sin_port    = socket->local_addr->sin_port;
    clone->local_addr->sin_addr    = socket->local_addr->sin_addr;

    clone->remote_addr->sin_family = AF_INET;
    clone->remote_addr->sin_port   = htons (clone->tcp_sock->hisport);
    clone->remote_addr->sin_addr   = peer;
  }
  ARGSUSED (is_ip6);
  return (1);
}

/*
 * Release a listen-queue slot and associated memory.
 */
static void listen_free (Socket *socket, int idx)
{
  _tcp_Socket *tcb = socket->listen_queue [idx];

  _tcp_unthread (tcb, TRUE);
  _sock_free_rcv_buf ((sock_type*)tcb); /* free large Rx buffer */
  free (tcb);
  socket->listen_queue [idx] = NULL;
}

/**
 * Called from tcp_fsm.c / tcp_listen_state() (via _bsd_socket_hook) to
 * append a new connection to the listen-queue of socket 'sock'.
 *
 * TCB on input ('orig') has received a SYN. Replace TCB on output
 * with a cloned TCB that we append to the listen-queue and eventually
 * is used by accept() to create a new socket.
 *
 * TCB on input ('orig') must still be listening for further connections
 * on the same port as specified in call to _TCP_listen().
 *
 * \todo Implement SYN-cookies. Ref. <http://cr.yp.to/syncookies.html>
 */
int _sock_append (_tcp_Socket **tcp)
{
  _tcp_Socket *clone;
  _tcp_Socket *orig = *tcp;
  Socket      *sock = NULL;   /* associated socket for 'orig' */
  int          i;

  /* Lookup BSD-socket for TCB
   */
  if (!_bsd_socket_hook ||
      (sock = (*_bsd_socket_hook)(BSO_FIND_SOCK,orig)) == NULL)
  {
    /* This could be a native Wattcp socket. Pass it on in
     * tcp_listen_state() for further processing.
     */
    SOCK_DEBUGF (("\n  sock_append: not found!?"));
    return (1);
  }

  SOCK_DEBUGF (("\n  sock_append:%d", sock->fd));

  if (!(sock->so_options & SO_ACCEPTCONN))
  {
    SOCK_DEBUGF ((", not SO_ACCEPTCONN"));
    return (0);  /* How could this happen (SYN attack)? */
  }

  /* Find the first vacant slot for this clone
   */
  for (i = 0; i < sock->backlog; i++)
      if (!sock->listen_queue[i])
         break;

  /** \todo Implement SYN-cookies and drop the segment if no match
   */
  if (i >= sock->backlog || i >= SOMAXCONN)
  {
    /** \todo drop the oldest (or a random) slot in the listen-queue.
     */
    SOCK_DEBUGF ((", queue full (idx %d)", i));
    return (0);
  }

  SOCK_DEBUGF ((", idx %d", i));

  clone = (_tcp_Socket*) SOCK_CALLOC (sizeof(*clone));
  if (!clone)
  {
    SOCK_DEBUGF ((", ENOMEM"));
    return (0);
  }

  /* Link in the semi-connected socket (SYN received, ACK will be sent)
   */
  sock->listen_queue[i]  = clone;
  sock->syn_timestamp[i] = set_timeout (0);

  /* Copy the TCB to clone. Tx buffer of clone must not
   * be set to parent's Tx buffer.
   */
  memcpy (clone, orig, sizeof(*clone));
  clone->tx_data     = &clone->tx_buf[0];
  clone->tx_datalen  = 0;
  clone->max_tx_data = sizeof (clone->tx_buf) - 1;

#if defined(USE_DEBUG)          /* !!needs some work */
  clone->last_acknum[0] = orig->last_acknum[0];
  clone->last_acknum[1] = orig->last_acknum[1];
  clone->last_seqnum[0] = orig->last_seqnum[0];
  clone->last_seqnum[1] = orig->last_seqnum[1];
  orig->last_acknum[0]  = orig->last_acknum[1] = 0L;
  orig->last_seqnum[0]  = orig->last_seqnum[1] = 0L;
#endif

  /* Increase the TCP window (to 16kB by default)
   */
  _sock_set_rcv_buf ((sock_type*)clone, tcp_recv_win);

  /* Undo what tcp_handler() and tcp_listen_state() did to
   * this listening socket.
   */
  orig->hisport = 0;
  orig->hisaddr = 0;
  orig->myaddr  = 0;
#if defined(USE_IPV6)
  memset (&orig->my6addr, 0, sizeof(orig->my6addr));
#endif

  orig->send_next = INIT_SEQ();   /* set new ISS */
  orig->unhappy   = FALSE;
  CLR_PEER_MAC_ADDR (orig);

  clone->next  = _tcp_allsocs;
  _tcp_allsocs = clone;         /* prepend clone to TCB-list */
  *tcp = clone;                 /* the new TCB is now the clone */
  return (1);
}

/*
  A handy note from:
    http://help.netscape.com/kb/corporate/960513-73.html

Solaris 2.3, 2.4, and 2.5 have a listen backlog queue for incoming
TCP/IP connections; its maximum length is 5 by default.  If you leave
this set to the default, then you will frequently see connections to
your web server time out under moderate loads, even though there are
enough idle web server listener processes available to handle the
connections and your Solaris system shows no other signs of resource
saturation.

The listen backlog queue holds connections that are "half-open" (in
the process of being opened), as well as connections that have been
fully opened but have not yet been accepted by any local processes.
This has no effect on the total number of open TCP/IP connections that
your Solaris system can deal with at once; it only means that your
system can't juggle more than five loose connections at a time, and
any other connections that come in while Solaris is busy with five
loose connections will be dropped and will time out on the client end.

On Solaris 2.3 and 2.4, you can bring the maximum queue length from 5
up to 32 by using the "ndd" command, which must be run as root:

    /usr/sbin/ndd -set /dev/tcp tcp_conn_req_max 32

    It is theoretically possible to increase this number beyond 32,
    although this is not recommended.  If increasing the maximum length
    from 5 to 32 solved the problem temporarily for you but your web
    server's traffic has now increased to a point where the symptoms
    appear with the maximum queue length set to 32, then you should
    contact Sun for further help with this.

    The Netscape servers on any Unix system will request a listen backlog
    queue length of 128 when they run; the operating system then reduces
    that to something it can handle.

    Solaris 2.5 allows a maximum listen backlog queue length as high as
    1024 (and you can raise it in the same way), but it still defaults to
    a maximum queue length of 5.

*/

#endif /* USE_BSD_API */
