/*!\file receive.c
 * BSD recv(), recvfrom().
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

static int tcp_receive (Socket *sock, void *buf, int len, int flags,
                        struct sockaddr *from, size_t *fromlen);

static int udp_receive (Socket *sock, void *buf, int len, int flags,
                        struct sockaddr *from, size_t *fromlen);

static int raw_receive (Socket *sock, void *buf, unsigned len, int flags,
                        struct sockaddr *from, size_t *fromlen);

/*
 * receive() flags:
 *  MSG_PEEK
 *  MSG_WAITALL
 *  MSG_OOB       (not yet supported)
 *  MSG_DONTROUTE (not yet supported)
 *  MSG_EOR       (not yet supported)
 *  MSG_TRUNC     (not yet supported)
 *  MSG_CTRUNC    (not yet supported)
 *
 *  Only one flags bit is handled at a time.
 */

static int receive (const char *func, int s, void *buf, int len, int flags,
                    struct sockaddr *from, size_t *fromlen)
{
  Socket *socket = _socklist_find (s);
  int     ret    = 0;

  if (func)
     SOCK_DEBUGF (("\n%s:%d", func, s));

  if (!socket)
  {
    if (_sock_dos_fd(s))
    {
      SOCK_DEBUGF ((", ENOTSOCK"));
      SOCK_ERRNO (ENOTSOCK);
      return (-1);
    }
    SOCK_DEBUGF ((", EBADF"));
    SOCK_ERRNO (EBADF);
    return (-1);
  }

  VERIFY_RW (buf, len);

  if (from && fromlen)
  {
    size_t sa_len = (socket->so_family == AF_INET6)  ?
                     sizeof(struct sockaddr_in6)     :
                    (socket->so_family == AF_PACKET) ?
                     sizeof(struct sockaddr_ll)      :
                     sizeof(struct sockaddr_in);

    if (*fromlen < sa_len)
    {
      SOCK_DEBUGF ((", EADDRNOTAVAIL (fromlen = %d, sa_len = %d)",
                    (int)*fromlen, (int)sa_len));
      SOCK_ERRNO (EADDRNOTAVAIL);
      return (-1);
    }
    VERIFY_RW (from, sa_len);
  }

  if (socket->so_state & SS_CONN_REFUSED)
  {
    if (socket->so_error == ECONNRESET)  /* from BSO_RST_CALLBACK */
    {
      SOCK_DEBUGF ((", ECONNRESET"));
      SOCK_ERRNO (ECONNRESET);
    }
    else
    {
      SOCK_DEBUGF ((", ECONNREFUSED (1)"));
      SOCK_ERRNO (ECONNREFUSED);
    }
    return (-1);
  }

  if (socket->so_state & SS_CANTRCVMORE)
  {
    SOCK_DEBUGF ((", EPIPE (can't recv more)"));
    SOCK_ERRNO (EPIPE);  /* !! was ENOTCONN */
    return (-1);
  }

  if ((flags & MSG_PEEK) && (flags & MSG_WAITALL))
  {
    SOCK_DEBUGF ((", invalid PEEK+WAITALL flags"));
    SOCK_ERRNO (EINVAL);
    return (-1);
  }

  /* If application installs the same signal handlers we do, we must
   * exit cracefully from below loops.
   */
  if (_sock_sig_setup() < 0)
  {
    SOCK_DEBUGF ((", EINTR"));
    SOCK_ERRNO (EINTR);
    return (-1);
  }

  switch (socket->so_type)
  {
    case SOCK_STREAM:
         ret = tcp_receive (socket, buf, len, flags, from, fromlen);
         SOCK_DEBUGF ((", len=%d", ret));
         break;

    case SOCK_DGRAM:
         ret = udp_receive (socket, buf, len, flags, from, fromlen);
         SOCK_DEBUGF ((", len=%d", ret));
         break;

    case SOCK_RAW:
         ret = raw_receive (socket, buf, len, flags, from, fromlen);
         SOCK_DEBUGF ((", len=%d", ret));
         break;

    case SOCK_PACKET:
         /* give raw packets a chance to be polled first. */
         tcp_tick (NULL);
         ret = sock_packet_receive (socket, buf, len, from, fromlen);
         SOCK_DEBUGF ((", len=%d", ret));
         break;

    default:
         SOCK_DEBUGF ((", EPROTONOSUPPORT"));
         SOCK_ERRNO (EPROTONOSUPPORT);
         ret = -1;
         break;
  }
  _sock_sig_restore();
  return (ret);
}

/*
 * recvfrom(): receive from socket 's'. Address (src-ip/port) is put
 *             in 'from' (if non-NULL)
 */
int W32_CALL recvfrom (int s, void *buf, int len, int flags,
                       struct sockaddr *from, socklen_t *fromlen)
{
  return receive ("recvfrom", s, buf, len, flags, from, (size_t*)fromlen);
}

/*
 * recv(): receive data from socket 's'. Flags may be
 *         MSG_PEEK, MSG_OOB or MSG_WAITALL
 *         Normally used on SOCK_STREAM sockets.
 */
int W32_CALL recv (int s, void *buf, int len, int flags)
{
  return receive ("recv", s, buf, len, flags, NULL, NULL);
}

/*
 * read_s(): As above but no flags.
 */
int W32_CALL read_s (int s, char *buf, int len)
{
  return receive ("read_s", s, buf, len, 0, NULL, NULL);
}

/*
 * recvmsg(): read into a 'msg->msg_iov' vector.
 *            Loop for each element in the vector. If 'msg->msg_name'
 *            is non-NULL, fill in from-addr.
 */
int W32_CALL recvmsg (int s, struct msghdr *msg, int flags)
{
  struct iovec    *iov;
  int    count = msg->msg_iovlen;
  int    i, bytes, len;

  SOCK_DEBUGF (("\nrecvmsg:%d, iovecs=%d", s, count));

  iov = msg->msg_iov;
  if (!iov)
  {
    SOCK_DEBUGF ((", EFAULT"));
    SOCK_ERRNO (EFAULT);
    return (-1);
  }

  for (i = bytes = 0; i < count; i++)
  {
#if (DOSX)
    if (!valid_addr(iov[i].iov_base, iov[i].iov_len))
    {
      SOCK_DEBUGF ((", EFAULT (iovec[%d] = %p/%d)",
                   (int)i, iov[i].iov_base, iov[i].iov_len));
      SOCK_ERRNO (EFAULT);
      return (-1);
    }
#endif

    len = receive (NULL, s, iov[i].iov_base, iov[i].iov_len,
                   flags, (struct sockaddr*)msg->msg_name,
                   (size_t*)&msg->msg_namelen);
    if (len < 0)
    {
      bytes = -1;
      break;
    }
    bytes += len;
    if (len != iov[i].iov_len)  /* nothing more to read */
       break;
  }
  SOCK_DEBUGF ((", total %d", bytes));
  return (bytes);
}

/*
 * readv_s(): Complementary of writev_s().
 */
int W32_CALL readv_s (int s, const struct iovec *vector, size_t count)
{
  int i, len, bytes = 0;

  SOCK_DEBUGF (("\nreadv_s:%d, iovecs=%lu", s, (u_long)count));

  for (i = 0; i < (int)count; i++)
  {
#if (DOSX)
    if (!valid_addr(vector[i].iov_base, vector[i].iov_len))
    {
      SOCK_DEBUGF ((", EFAULT (iovec[%d] = %p/%d)",
                   (int)i, vector[i].iov_base, vector[i].iov_len));
      SOCK_ERRNO (EFAULT);
      return (-1);
    }
#endif

    len = receive (NULL, s, vector[i].iov_base, vector[i].iov_len,
                   0, NULL, NULL);
    if (len < 0)
    {
      bytes = -1;
      break;
    }
    bytes += len;
    if (len != vector[i].iov_len)  /* nothing more to read */
       break;
  }
  SOCK_DEBUGF ((", total %d", bytes));
  return (bytes);
}

/*
 * Fill in packet's address in 'from' and length in 'fromlen'.
 * Only used for UDP & Raw-IP. TCP have peer info in 'socket->remote_addr'.
 */
static void udp_raw_fill_from_ip4 (struct sockaddr *from, size_t *fromlen,
                                   const struct in_addr *peer, WORD port)
{
  struct sockaddr_in *sa = (struct sockaddr_in*) from;

  if (sa && fromlen && *fromlen >= sizeof(*sa))
  {
    memset (sa, 0, sizeof(*sa));
    sa->sin_addr   = *peer;
    sa->sin_family = AF_INET;
    sa->sin_port   = port;
  }
  if (fromlen)
     *fromlen = sizeof (*sa);
}

#if defined(USE_IPV6)
static void udp_raw_fill_from_ip6 (struct sockaddr *from, size_t *fromlen,
                                   const void *peer, WORD port)
{
  struct sockaddr_in6 *sa = (struct sockaddr_in6*) from;

  if (sa && fromlen)
  {
    memset (sa, 0, sizeof(*sa));
    memcpy (&sa->sin6_addr, peer, sizeof(sa->sin6_addr));
    sa->sin6_family = AF_INET6;
    sa->sin6_port   = port;
  }
  if (fromlen)
     *fromlen = sizeof (*sa);
}
#endif

/*
 * TCP receiver
 */
static int tcp_receive (Socket *socket, void *buf, int len, int flags,
                        struct sockaddr *from, size_t *fromlen)
{
  int        ret     = 0;
  BOOL       got_fin = FALSE; /* got FIN from peer */
  DWORD      timer   = 0UL;
  sock_type *sk      = (sock_type*) socket->tcp_sock;

  if (!from && !socket->local_addr)
  {
    SOCK_DEBUGF ((", no local_addr"));
    SOCK_ERRNO (ENOTCONN);
    return (-1);
  }

  if (flags & MSG_OOB)
  {
    SOCK_DEBUGF ((", no OOB-data"));
    SOCK_ERRNO (EWOULDBLOCK);
    return (-1);
  }

  if (socket->timeout && sock_inactive)
     timer = set_timeout (1000 * socket->timeout);

  while (1)
  {
    BOOL ok = (tcp_tick(sk) != 0);

    tcp_Retransmitter (TRUE);

    if (socket->so_state & SS_ISDISCONNECTING)
    {
      got_fin = TRUE;  /* might not have got a FIN, but need to read */
      goto read_it;
    }

    /** \todo Handle receiving Out-of-Band data */
#if 0
    if ((socket-so_options & SO_OOBINLINE) && urgent_data(sk))
    {
      ret = urgent_data_read (sk, (BYTE*)buf, len);
      break;
    }
#endif

    /* Don't do this for a listening socket
     */
    if (!(socket->so_options & SO_ACCEPTCONN))
    {
      if ((sk->tcp.locflags & LF_GOT_FIN) &&     /* got FIN, no unACK data */
          sk->tcp.rx_datalen == 0)
      {
        socket->so_state |=  SS_ISDISCONNECTING; /* We may receive more */
        socket->so_state &= ~SS_ISCONNECTED;     /* no longer ESTAB state */
        got_fin = TRUE;
        SOCK_DEBUGF ((", got FIN"));
        goto read_it;
      }

      if (!ok)
      {
        socket->so_state |= (SS_CANTRCVMORE | SS_ISDISCONNECTING);
        socket->so_state &= ~SS_ISCONNECTED;
        SOCK_DEBUGF ((", EPIPE"));
        SOCK_ERRNO (EPIPE);  /* !! was ENOTCONN */
        return (-1);
      }
    }

    if (sock_rbused(sk) > socket->recv_lowat)
    {
read_it:
      if (flags & MSG_PEEK)
           ret = sock_preread (sk, (BYTE*)buf, len);
      else if (flags & MSG_WAITALL)
           ret = sock_read (sk, (BYTE*)buf, len);
      else ret = sock_fastread (sk, (BYTE*)buf, len);
      break;
    }

    if (socket->so_state & SS_CONN_REFUSED)
    {
      SOCK_DEBUGF ((", ECONNREFUSED (2)"));
      SOCK_ERRNO (ECONNREFUSED);
      return (-1);
    }

    if (socket->so_state & SS_NBIO)
    {
      SOCK_DEBUGF ((", EWOULDBLOCK"));
      SOCK_ERRNO (EWOULDBLOCK);
      return (-1);
    }

    if (chk_timeout(timer))
    {
      SOCK_DEBUGF ((", ETIMEDOUT"));
      SOCK_ERRNO (ETIMEDOUT);
      return (-1);
    }

    if (_sock_sig_pending())
    {
      SOCK_DEBUGF ((", EINTR"));
      SOCK_ERRNO (EINTR);
      return (-1);
    }

    if (sk->tcp.usr_yield)
        (*sk->tcp.usr_yield)();
    else WATT_YIELD();
  }

  if (ret > 0)
  {
    int sa_len = (socket->so_family == AF_INET6) ?
                   sizeof(struct sockaddr_in6) :
                   sizeof(struct sockaddr_in);
    if (from)
       memcpy (from, socket->remote_addr, sa_len);
    if (fromlen)
       *fromlen = sa_len;
    socket->keepalive = 0UL;  /* reset keep-alive timer */
  }
  else if (ret < 0)
  {
    if (got_fin)   /* A FIN and -1 from sock_xread() maps to 0 */
       ret = 0;
    else           /* else some buffer/socket error */
    {
      SOCK_DEBUGF ((", EIO"));
      SOCK_ERRNO (EIO);
    }
  }
  return (ret);
}


/*
 * UDP receiver
 */
static int udp_receive (Socket *socket, void *buf, int len, int flags,
                        struct sockaddr *from, size_t *fromlen)
{
  int   ret   = 0;
  DWORD timer = 0UL;

  if (socket->timeout)
     timer = set_timeout (1000 * socket->timeout);

  /** \todo This needs a redesign */
#if 0
  /* If bind() not called, allow data from anybody on any port !!
   */
  if (from && !socket->local_addr)
  {
    struct sockaddr_in addr = { AF_INET, 0, { INADDR_ANY }};

    socket->so_state |= SS_PRIV;

    if (_UDP_listen (socket, addr.sin_addr, addr.sin_port) < 0)
       return (-1);
  }
#endif

  while (1)
  {
    sock_type *sk = (sock_type*) socket->udp_sock;
    BOOL       ok = (tcp_tick(sk) != 0);

    WATT_YIELD();

    if (socket->so_error == ECONNREFUSED ||
        (sk->udp.locflags & LF_GOT_ICMP))
    {
      socket->so_state |= (SS_CANTRCVMORE | SS_CANTSENDMORE);
      SOCK_DEBUGF ((", ECONNREFUSED"));
      SOCK_ERRNO (ECONNREFUSED);
      return (-1);
    }

    if (!ok)
    {
      socket->so_state |= (SS_CANTRCVMORE | SS_CANTSENDMORE);
      SOCK_DEBUGF ((", EPIPE"));
      SOCK_ERRNO (EPIPE);    /* !! was ENOTCONN */
      return (-1);
    }

    tcp_Retransmitter (TRUE);

    /* If this socket is for broadcast (or is unbound), check the
     * queue setup by sock_recv_init() (in _UDP_listen).
     * Note: it is possible to receive 0-byte probe packets.
     */
    if (socket->so_family == AF_INET && (socket->so_state & SS_PRIV))
    {
      struct in_addr peer = { 0 };
      sock_type     *udp  = (sock_type*)socket->udp_sock;
      WORD           port;

      SOCK_ERRNO (0);
      ret = sock_recv_from (udp, (DWORD*)&peer.s_addr, &port, buf, len,
                            (flags & MSG_PEEK) ? 1 : 0);

      if (ret && _w32_errno != EBADF && peer.s_addr)
      {
        udp_raw_fill_from_ip4 (from, fromlen, &peer, port);
        SOCK_DEBUGF ((", 1: remote: %s (%d)", inet_ntoa(peer), ntohs(port)));

        if (ret < 0)   /* 0-byte probe */
           return (0);
        return (ret);
      }
    }

#if defined(USE_IPV6)
    else if (socket->so_family == AF_INET6 && (socket->so_state & SS_PRIV))
    {
      struct in6_addr peer;
      sock_type      *udp = (sock_type*) socket->udp_sock;
      WORD            port;

      SOCK_ERRNO (0);
      memcpy (&peer, &in6addr_any, sizeof(peer));
      ret = sock_recv_from (udp, (DWORD*)&peer, &port, buf, len,
                            (flags & MSG_PEEK) ? 1 : 0);

      if (ret && _w32_errno != EBADF && !IN6_IS_ADDR_UNSPECIFIED(&peer))
      {
        udp_raw_fill_from_ip6 (from, fromlen, &peer, port);
        SOCK_DEBUGF ((", 1: remote: %s (%d)", _inet6_ntoa(&peer), ntohs(port)));

        if (ret < 0)   /* 0-byte probe */
           return (0);
        return (ret);
      }
    }
#endif

    else if (sock_rbused(sk) > socket->recv_lowat)
    {
      if (flags & MSG_PEEK)
           ret = sock_preread  (sk, (BYTE*)buf, len);
      else if (flags & MSG_WAITALL)
           ret = sock_read     (sk, (BYTE*)buf, len);
      else ret = sock_fastread (sk, (BYTE*)buf, len);
      break;
    }

    if (socket->so_state & SS_CONN_REFUSED)
    {
      SOCK_DEBUGF ((", ECONNREFUSED (2)"));
      SOCK_ERRNO (ECONNREFUSED);
      return (-1);
    }

    if (socket->so_state & SS_NBIO)
    {
      SOCK_DEBUGF ((", EWOULDBLOCK"));
      SOCK_ERRNO (EWOULDBLOCK);
      return (-1);
    }

    if (chk_timeout(timer))
    {
      SOCK_DEBUGF ((", ETIMEDOUT"));
      SOCK_ERRNO (ETIMEDOUT);
      return (-1);
    }

    if (_sock_sig_pending())
    {
      SOCK_DEBUGF ((", EINTR"));
      SOCK_ERRNO (EINTR);
      return (-1);
    }
  }

  if (ret > 0)
  {
#if defined(USE_IPV6)
    if (socket->so_family == AF_INET6)
    {
      WORD        port = htons (socket->udp_sock->hisport);
      const void *peer = &socket->udp_sock->his6addr[0];

      udp_raw_fill_from_ip6 (from, fromlen, peer, port);

      if (socket->remote_addr)
      {
        struct sockaddr_in6 *ra = (struct sockaddr_in6*)socket->remote_addr;

        ra->sin6_family = AF_INET6;
        ra->sin6_port   = port;
        memcpy (&ra->sin6_addr, peer, sizeof(ra->sin6_addr));
      }
      SOCK_DEBUGF ((", 2: remote: %s (%d)", _inet6_ntoa(peer), ntohs(port)));
    }
    else
#endif
    {
      struct in_addr peer;
      _udp_Socket   *udp = socket->udp_sock;
      WORD           port;

      port = htons (udp->hisport);
      peer.s_addr = htonl (udp->hisaddr);

      udp_raw_fill_from_ip4 (from, fromlen, &peer, port);

      if (socket->remote_addr)
      {
        socket->remote_addr->sin_family = AF_INET;
        socket->remote_addr->sin_addr   = peer;
        socket->remote_addr->sin_port   = port;
      }
      SOCK_DEBUGF ((", 2: remote: %s (%d)", inet_ntoa(peer), ntohs(port)));
    }
  }
  return (ret);
}


/*
 * Raw-IP receiver. Doesn't handle IP-options yet.
 */
static int raw_receive (Socket *socket, void *buf, unsigned len, int flags,
                        struct sockaddr *from, size_t *fromlen)
{
  _raw_Socket *raw = NULL;
  DWORD timer, loop;

#if defined(USE_IPV6)
  _raw6_Socket *raw6 = NULL;

  if (socket->so_family == AF_INET6)
  {
    if (len < sizeof(raw6->ip6))
    {
      SOCK_DEBUGF (("EINVAL"));
      SOCK_ERRNO (EINVAL);
      return (-1);
    }
  }
  else
#endif
  if (len < sizeof(raw->ip))
  {
    SOCK_DEBUGF (("EINVAL"));
    SOCK_ERRNO (EINVAL);
    return (-1);
  }

  if (socket->timeout)
       timer = set_timeout (1000 * socket->timeout);
  else timer = 0;
  loop = 1;

  while (1)
  {
    unsigned ip_len, hdr_len;

    /* give sock_rbused() and memcpy() below a chance to run
     * before sock_raw_recv() (via tcp_tick) drops a packet
     */
    if (loop++ > 1)
    {
      tcp_tick (NULL);
      tcp_Retransmitter (TRUE);
    }

#if defined(USE_IPV6)
    if (socket->so_family == AF_INET6)
    {
      raw6    = (_raw6_Socket*) find_oldest_raw6 (socket->raw6_sock);
      ip_len  = raw6 ? intel16 (raw6->ip6.len) : 0;
      hdr_len = sizeof(in6_Header);
    }
    else
#endif
    {
      raw     = (_raw_Socket*) find_oldest_raw (socket->raw_sock);
      ip_len  = raw ? intel16 (raw->ip.length) : 0; /* includes header length */
      hdr_len = sizeof(in_Header);
    }

#if defined(USE_IPV6)
    if (socket->so_family == AF_INET6)
    {
      if (ip_len >= hdr_len + socket->recv_lowat)
      {
        struct in6_Header *ip6 = (struct in6_Header*) buf;

        /* SOCK_RAW shall always return IP-header and data in 'buf'
         */
        memcpy (ip6, &raw6->ip6, hdr_len);
        len = min (ip_len - hdr_len, len);
        if (len > 0)
           memcpy (++ip6, &raw6->rx_data[0], len);

        raw6->used = FALSE;

        udp_raw_fill_from_ip6 (from, fromlen, &raw6->ip6.source[0], 0);
        SOCK_DEBUGF ((", remote: %s", _inet6_ntoa(&raw6->ip6.source[0])));
        return (len + hdr_len);
      }
    }
    else
#endif
    if (ip_len >= hdr_len + socket->recv_lowat)
    {
      struct in_addr    peer;
      struct in_Header *ip = (struct in_Header*) buf;

      /* SOCK_RAW shall always return IP-header and data in 'buf'
       */
      memcpy (ip, &raw->ip, min(hdr_len,len));  /* first, copy the IP-header */
      len = min (ip_len - hdr_len, len);
      if (len > 0)
         memcpy (++ip, &raw->rx_data[0], len);  /* copy the rest. max 666000 bytes */

      peer.s_addr = raw->ip.source;
      raw->used   = FALSE;

      udp_raw_fill_from_ip4 (from, fromlen, &peer, 0);
      SOCK_DEBUGF ((", remote: %s", inet_ntoa(peer)));
      return (len + hdr_len);
    }

    if (socket->so_state & SS_NBIO)
    {
      SOCK_DEBUGF ((", EWOULDBLOCK"));
      SOCK_ERRNO (EWOULDBLOCK);
      break;
    }

    if (chk_timeout(timer))
    {
      SOCK_DEBUGF ((", ETIMEDOUT"));
      SOCK_ERRNO (ETIMEDOUT);
      break;
    }

    if (_sock_sig_pending())
    {
      SOCK_DEBUGF ((", EINTR"));
      SOCK_ERRNO (EINTR);
      break;
    }

    WATT_YIELD();
  }

  ARGSUSED (flags);
  return (-1);
}
#endif  /* USE_BSD_API */
