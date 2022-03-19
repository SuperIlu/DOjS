/*!\file transmit.c
 * BSD send(), sendto(), write().
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
 */

#include "socket.h"
#include "pcicmp6.h"

#if defined(USE_BSD_API)

static int raw_transmit (Socket *socket, const void *buf, unsigned len);
static int udp_transmit (Socket *socket, const void *buf, unsigned len);
static int tcp_transmit (Socket *socket, const void *buf, unsigned len, int flags);
static int setup_udp_raw (Socket *socket, const struct sockaddr *to, int tolen);

static int transmit (const char *func, int s, const void *buf, unsigned len,
                     int flags, const struct sockaddr *to, int tolen,
                     BOOL have_remote_addr);

int W32_CALL sendto (int s, const void *buf, int len, int flags,
                     const struct sockaddr *to, socklen_t tolen)
{
  return transmit ("sendto", s, buf, len, flags, to, tolen, TRUE);
}

int W32_CALL send (int s, const void *buf, int len, int flags)
{
  return transmit ("send", s, buf, len, flags, NULL, 0, FALSE);
}

int W32_CALL write_s (int s, const char *buf, int nbyte)
{
  return transmit ("write_s", s, buf, nbyte, 0, NULL, 0, FALSE);
}

int W32_CALL writev_s (int s, const struct iovec *vector, size_t count)
{
  int i, len, bytes = 0;

  SOCK_DEBUGF (("\nwritev_s:%d, iovecs=%lu", s, (u_long)count));

  for (i = 0; i < (int)count; i++)
  {
#if (DOSX)
    if (!valid_addr(vector[i].iov_base, vector[i].iov_len))
    {
      SOCK_DEBUGF ((", EFAULT (iovec[%d] = %p, len %d)",
                    i, vector[i].iov_base, vector[i].iov_len));
      SOCK_ERRNO (EFAULT);
      return (-1);
    }
#endif

    len = transmit (NULL, s, vector[i].iov_base, vector[i].iov_len,
                    0, NULL, 0, FALSE);
    if (len < 0)
    {
      bytes = -1;
      break;
    }
    bytes += len;
  }

  SOCK_DEBUGF ((", total %d", bytes));  /* writing 0 byte is not an error */
  return (bytes);
}

/*
 * sendmsg():
 */
int W32_CALL sendmsg (int s, const struct msghdr *msg, int flags)
{
  const struct iovec *iov;
  int   count = msg->msg_iovlen;
  int   i, bytes, len;

  SOCK_DEBUGF (("\nsendmsg:%d, iovecs=%d", s, count));

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

    len = transmit (NULL, s, iov[i].iov_base, iov[i].iov_len,
                    flags, (struct sockaddr*)msg->msg_name,
                    msg->msg_namelen, TRUE);
    if (len < 0)
    {
      bytes = -1;
      break;
    }
    bytes += len;
  }
  SOCK_DEBUGF ((", total %d", bytes));
  return (bytes);
}

/*
 * Close socket if MSG_EOR specified in flags.
 */
static __inline void msg_eor_close (Socket *socket)
{
  switch (socket->so_type)
  {
    case SOCK_STREAM:
         socket->so_state |= SS_CANTSENDMORE;
         sock_close ((sock_type*)socket->tcp_sock);
         break;
    case SOCK_DGRAM:
         socket->so_state |= SS_CANTSENDMORE;
         sock_close ((sock_type*)socket->udp_sock);
         break;
    case SOCK_RAW:
         socket->so_state |= SS_CANTSENDMORE;
         break;
  }
}

/*
 * transmit() flags:
 *   MSG_DONTROUTE                                     (not supported)
 *   MSG_EOR       Close sending side after data sent
 *   MSG_TRUNC                                         (not supported)
 *   MSG_CTRUNC                                        (not supported)
 *   MSG_OOB                                           (not supported)
 *   MSG_WAITALL   Wait till room in tx-buffer         (not supported)
 *   MSG_NOSIGNAL  ??                                  (not supported)
 */
static int transmit (const char *func, int s, const void *buf, unsigned len,
                     int flags, const struct sockaddr *to, int tolen,
                     BOOL have_remote_addr)  /* for sendto() and sendmsg() */
{
  Socket *socket = _socklist_find (s);
  int     rc;

  if (func)
  {
    SOCK_DEBUGF (("\n%s:%d, len=%d", func, s, len));
    if (flags)
       SOCK_DEBUGF ((", flags 0x%X", flags));
  }

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

  if (flags & MSG_NOSIGNAL)   /* Don't do 'raise(SIGPIPE)' */
     socket->msg_nosig = TRUE;

  if (socket->so_type == SOCK_STREAM ||      /* TCP-socket or */
      (socket->so_state & SS_ISCONNECTED))   /* "connected" udp/raw */
  {
    /* Note: SOCK_RAW doesn't really need a local address/port, but
     * makes the code more similar for all socket-types.
     * Disadvantage is that SOCK_RAW ties up a local port and a bit
     * more memory.
     */

    if (!socket->local_addr)
    {
      SOCK_DEBUGF ((", no local_addr"));
      SOCK_ERRNO (ENOTCONN);
      return (-1);
    }

    if (!socket->remote_addr)
    {
      SOCK_DEBUGF ((", no remote_addr"));
      SOCK_ERRNO (ENOTCONN);
      return (-1);
    }

    if (socket->so_state & SS_CONN_REFUSED)
    {
      if (socket->so_error == ECONNRESET)  /* set in tcp_sockreset() */
      {
        SOCK_DEBUGF ((", ECONNRESET"));
        SOCK_ERRNO (ECONNRESET);
      }
      else
      {
        SOCK_DEBUGF ((", ECONNREFUSED"));
        SOCK_ERRNO (ECONNREFUSED);
      }
      return (-1);
    }
  }

  /* connectionless protocol setup.
   * SOCK_PACKET sockets go pretty much unchecked.
   */
  if (socket->so_type == SOCK_DGRAM ||
      socket->so_type == SOCK_RAW)
  {
    size_t sa_len = (socket->so_family == AF_INET6)  ?
                     sizeof(struct sockaddr_in6)     :
                    (socket->so_family == AF_PACKET) ?
                     sizeof(struct sockaddr_ll)      :
                     sizeof(struct sockaddr_in);

    if (!have_remote_addr)
    {
      to = (const struct sockaddr*) socket->remote_addr;
      tolen = sa_len;
    }
    if (!to || tolen < (int)sa_len)
    {
      SOCK_DEBUGF ((", illegal to-addr (tolen = %d, sa_len %d)",
                    tolen, (int)sa_len));
      SOCK_ERRNO (EINVAL);
      return (-1);
    }
    if (socket->so_type != SOCK_PACKET &&
        setup_udp_raw(socket,to,tolen) < 0)
       return (-1);
  }

  if (len > 0)
     VERIFY_RW (buf, len);

  if (socket->so_type != SOCK_DGRAM && (!buf || len == 0))
  {
    SOCK_DEBUGF ((", EINVAL"));
    SOCK_ERRNO (EINVAL);
    return (-1);
  }

  if (_sock_sig_setup() < 0)
  {
    SOCK_ERRNO (EINTR);
    return (-1);
  }

  switch (socket->so_type)
  {
    case SOCK_DGRAM:
         rc = udp_transmit (socket, buf, len);
         break;

    case SOCK_STREAM:
         rc = tcp_transmit (socket, buf, len, flags);
         break;

    case SOCK_RAW:
         rc = raw_transmit (socket, buf, len);
         break;

    case SOCK_PACKET:
         rc = sock_packet_transmit (socket, buf, len, to, tolen);
         break;

    default:
         SOCK_DEBUGF ((", EPROTONOSUPPORT"));
         SOCK_ERRNO (EPROTONOSUPPORT);
         rc = -1;
         break;
  }

  if (rc >= 0 && (flags & MSG_EOR))
     msg_eor_close (socket);

  _sock_sig_restore();
  return (rc);
}


/*
 * Setup remote_addr for SOCK_RAW/SOCK_DGRAM (connectionless) protocols.
 * Must "reconnect" socket if 'remote_addr' or 'to' address are different.
 * I.e we're sending to another host/port than last time.
 */
static int setup_udp_raw (Socket *socket, const struct sockaddr *to, int tolen)
{
  const struct sockaddr_in *peer = (const struct sockaddr_in*) to;
  DWORD keepalive = socket->keepalive;
  WORD  lport     = 0;
  BOOL  is_ip6    = (socket->so_family == AF_INET6);
  BYTE *rdata     = NULL;
  int   rc;

  if (socket->so_state & SS_ISCONNECTED)
  {
    if (!socket->remote_addr)
    {
      SOCK_FATAL (("setup_udp_raw(): no remote_addr\n"));
      return (-1);
    }

    /* No need to reconnect if same peer address/port.
     */
    if (!is_ip6)
    {
      const struct sockaddr_in *ra = (const struct sockaddr_in*)socket->remote_addr;

      if (peer->sin_addr.s_addr == ra->sin_addr.s_addr &&
          peer->sin_port        == ra->sin_port)
        return (1);
    }
#if defined(USE_IPV6)
    else
    {
      const struct sockaddr_in6 *ra    = (const struct sockaddr_in6*)socket->remote_addr;
      const struct sockaddr_in6 *peer6 = (const struct sockaddr_in6*)to;

      if (!memcmp(&peer6->sin6_addr, &ra->sin6_addr, sizeof(peer6->sin6_addr)) &&
          peer6->sin6_port == ra->sin6_port)
        return (1);
    }
#endif

    SOCK_DEBUGF ((", reconnecting"));

    free (socket->remote_addr);
    socket->remote_addr = NULL;

    /* Clear any effect of previous ICMP errors etc.
     */
    socket->so_state &= ~(SS_CONN_REFUSED | SS_CANTSENDMORE | SS_CANTRCVMORE);
    socket->so_error  = 0;

    if (socket->so_type == SOCK_DGRAM)
    {
      lport = socket->udp_sock->myport;
      rdata = socket->udp_sock->rx_data;  /* preserve current data */
    }
  }

  /* For SOCK_DGRAM, udp_close() will be called when (re)opening socket.
   */
  SOCK_ENTER_SCOPE();
  rc = connect (socket->fd, to, tolen);
  SOCK_LEAVE_SCOPE();

  if (rc < 0)
     return (-1);

#if 0
  if ((socket->so_state & SS_PRIV) && socket->so_type == SOCK_DGRAM)
  {
    SOCK_DEBUGF ((", SS_PRIV"));

    /* Clear any effect of previous ICMP errors etc.
     */
    socket->so_state &= ~(SS_CONN_REFUSED | SS_CANTSENDMORE | SS_CANTRCVMORE);
    socket->so_error  = 0;

    lport = socket->udp_sock->myport;
    grab_localport (lport);
  }
#endif

  if (rdata)  /* Must be SOCK_DGRAM */
  {
    _udp_Socket *udp = socket->udp_sock;

    /* free new rx-buffer set in connect() / _UDP_open().
     */
    DISABLE();
    _sock_free_rcv_buf ((sock_type*)udp);
    udp->rx_data = rdata;      /* reuse previous data buffer */
    ENABLE();

    grab_localport (lport);    /* Restore free'd localport */
  }

  /* restore keepalive timer changed in connect()
   */
  socket->keepalive = keepalive;
  return (1);
}

/*
 * Check for enough room in Tx-buffer for a non-blocking socket
 * to transmit without waiting. Only called for SOCK_DGRAM/SOCK_STREAM
 * sockets.
 *
 * If '*len > room', modify '*len' on output to 'room' (the size of
 * bytes left in Tx-buf).
 */
static __inline BOOL check_non_block_tx (Socket *socket, unsigned *len)
{
  sock_type *sk;
  unsigned   room;

  if (socket->so_type == SOCK_DGRAM)
       sk = (sock_type*) socket->udp_sock;
  else sk = (sock_type*) socket->tcp_sock;

  room = sock_tbleft (sk);
  if (*len <= room)
     return (TRUE);   /* okay, enough room, '*len' unmodified */

#if 0
  WATT_YIELD();       /* a small delay to clear up things */
  tcp_tick (sk);

  room = sock_tbleft (sk);
  if (*len <= room)
     return (TRUE);
#endif

  /* Still no room, but cannot split up datagrams (only in IP-fragments)
   */
  if (socket->so_type == SOCK_DGRAM)
     return (FALSE);

  /* Stream: Tx room below (or equal) low-water mark is failure.
   */
  if (*len > 0 && room <= socket->send_lowat)
     return (FALSE);

  /* Streams may be split up, modify '*len'
   */
  *len = room;
  return (TRUE);
}

/*
 * TCP transmitter.
 */
static int tcp_transmit (Socket *socket, const void *buf, unsigned len,
                         int flags)
{
  sock_type *sk = (sock_type*)socket->tcp_sock;
  int        rc;

  /* Don't timeout BSD sockets on inactivity (not sending)
   */
  sk->tcp.datatimer = 0;

  tcp_tick (sk);
  tcp_Retransmitter (TRUE);

  /** \todo Allow non-blocking sockets to send in SYNSENT state
   */
  if (sk->tcp.state < tcp_StateESTAB || sk->tcp.state >= tcp_StateLASTACK)
  {
    socket->so_state |= SS_CANTSENDMORE;
    SOCK_DEBUGF ((", EPIPE"));
    SOCK_ERRNO (EPIPE);       /* !! was ENOTCONN */
    return (-1);
  }

  if (socket->so_state & SS_NBIO)
  {
    unsigned in_len = len;

    if (!check_non_block_tx(socket,&len))
    {
      SOCK_DEBUGF ((", EWOULDBLOCK"));
      SOCK_ERRNO (EWOULDBLOCK);
      return (-1);
    }
    if (in_len != len)
       SOCK_DEBUGF ((" [%u]", len)); /* trace "len=x [y]" */
  }

#if defined(USE_IPV6)
  if (socket->so_family == AF_INET6)
  {
    struct sockaddr_in6 *ra = (struct sockaddr_in6*) socket->remote_addr;

    SOCK_DEBUGF ((", %s (%d) / TCP",
                  _inet6_ntoa(&ra->sin6_addr),
                  ntohs(socket->remote_addr->sin_port)));
    ARGSUSED (ra);
  }
  else
#endif
     SOCK_DEBUGF ((", %s (%d) / TCP",
                   inet_ntoa(socket->remote_addr->sin_addr),
                   ntohs(socket->remote_addr->sin_port)));

#if 0
  if (len > sizof(sk->tcp.max_tx_data) - 1)
  {
    unsigned total  = len;
    BYTE    *buffer = (BYTE*)buf;

    while (1)
    {
      if (sock_tbused(sk) == 0)  /* Tx buffer empty */
      {
        unsigned bytes = min (sk->tcp.max_tx_data, total);

        if (bytes > 0)
        {
          buffer += sock_enqueue (sk, buffer, bytes);
          total  -= bytes;
        }
        else
          break;
      }
      if (!tcp_tick(sk))
         break;
    }
    rc = buffer - (BYTE*)buf;
  }
  else
#endif
    rc = sock_write (sk, (const BYTE*)buf, len);

  socket->keepalive = 0UL;

  if (rc <= 0)    /* error in tcp_write() */
  {
    if (sk->tcp.locflags & LF_GOT_ICMP) /* got ICMP host/port unreachable */
    {
      SOCK_DEBUGF ((", ECONNREFUSED")); /* !! a better code? */
      SOCK_ERRNO (ECONNREFUSED);
    }
    else if (sk->tcp.state != tcp_StateESTAB)
    {
      SOCK_DEBUGF ((", EPIPE"));
      SOCK_ERRNO (EPIPE);     /* !! was ENOTCONN */
    }
    else
    {
      SOCK_DEBUGF ((", ENETDOWN"));
      SOCK_ERRNO (ENETDOWN);
    }
    return (-1);
  }
  ARGSUSED (flags);
  return (rc);
}


/*
 * UDP transmitter
 */
static int udp_transmit (Socket *socket, const void *buf, unsigned len)
{
  sock_type  *sk     = (sock_type*) socket->udp_sock;
  BOOL        is_ip6 = (socket->so_family == AF_INET6);
  BOOL        is_bcast, is_mcast;
  unsigned    tx_room;
  int         rc;
  const void *dest;

  if (!tcp_tick(sk))
  {
    socket->so_state |= SS_CANTSENDMORE;
    SOCK_DEBUGF ((", EPIPE (can't send)"));
    SOCK_ERRNO (EPIPE);     /* !! was ENOTCONN */
    return (-1);
  }

  tcp_Retransmitter (TRUE);

  if ((socket->so_state & SS_NBIO) &&
      !check_non_block_tx(socket,&len))
  {
    SOCK_DEBUGF ((", EWOULDBLOCK"));
    SOCK_ERRNO (EWOULDBLOCK);
    return (-1);
  }

#if defined(USE_IPV6)
  if (is_ip6)
  {
    const struct sockaddr_in6 *ra = (const struct sockaddr_in6*)socket->remote_addr;

    dest = &ra->sin6_addr.s6_addr[0];
    is_bcast = IN6_IS_ADDR_MC_GLOBAL (dest);
    is_mcast = IN6_IS_ADDR_MULTICAST (dest);

    SOCK_DEBUGF ((", %s (%d) / UDP %s", _inet6_ntoa(dest),
                  ntohs(socket->remote_addr->sin_port),
                  is_mcast ? "(mcast)" : ""));
  }
  else
#endif
  {
    dest     = &socket->remote_addr->sin_addr.s_addr;
    is_bcast = (*(DWORD*)dest == INADDR_BROADCAST ||
                *(DWORD*)dest == INADDR_ANY);
    is_mcast = IN_MULTICAST (ntohl(*(DWORD*)dest));

    SOCK_DEBUGF ((", %s (%d) / UDP %s",
                  inet_ntoa(*(struct in_addr*)dest),
                  ntohs(socket->remote_addr->sin_port),
                  is_mcast ? "(mcast)" : ""));
  }

  if (len == 0)   /* 0-byte probe packet */
     return raw_transmit (socket, NULL, 0);

  tx_room = sock_tbleft (sk);  /* always MTU-28 */

  /* Special tests for broadcast messages
   */
  if (is_bcast)
  {
    if (len > tx_room)   /* no room, fragmented broadcasts not allowed */
    {
      SOCK_DEBUGF ((", EMSGSIZE"));
      SOCK_ERRNO (EMSGSIZE);
      goto drop;
    }
    if (_pktserial)           /* Link-layer doesn't allow broadcast */
    {
      SOCK_DEBUGF ((", EADDRNOTAVAIL"));
      SOCK_ERRNO (EADDRNOTAVAIL);
      goto drop;
    }
  }

  /* set new TTL if setsockopt() used before sending to Class-D socket
   */
  if (is_mcast)
     udp_SetTTL (socket->udp_sock, socket->ip_ttl);

#if defined(USE_FRAGMENTS)
  if (len > USHRT_MAX - sizeof(udp_Header))
  {
    SOCK_DEBUGF ((", EMSGSIZE"));
    SOCK_ERRNO (EMSGSIZE);
    if (!is_ip6)
       STAT (ip4stats.ips_toolong++);
    return (-1);
  }

  if (!is_ip6 && len > tx_room)
     return _IP4_SEND_FRAGMENTS (sk, UDP_PROTO, *(DWORD*)dest, buf, len);
#endif

  sk->udp.hisaddr = ntohl (socket->remote_addr->sin_addr.s_addr);
  sk->udp.hisport = ntohs (socket->remote_addr->sin_port);

  rc = sock_write (sk, (BYTE*)buf, len);

  /* Patch hisaddr/hisport so that udp_demux() will handle further
   * traffic as broadcast.
   */
  if (socket->so_state & SS_PRIV)
  {
    sk->udp.hisaddr = INADDR_BROADCAST;
    sk->udp.hisport = IPPORT_ANY;
  }

  if (rc <= 0)    /* error in udp_write() */
  {
    if (sk->udp.locflags & LF_GOT_ICMP)
    {
      SOCK_DEBUGF ((", ECONNREFUSED"));
      SOCK_ERRNO (ECONNREFUSED);
    }
    else
    {
      SOCK_DEBUGF ((", ENETDOWN"));
      SOCK_ERRNO (ENETDOWN);
    }
    return (-1);
  }
  return (rc);

drop:
  if (is_ip6)
       STAT (ip6stats.ip6s_odropped++);
  else STAT (ip4stats.ips_odropped++);
  return (-1);
}

/**
 * Raw IPv4 transmitter.
 * \note
 *   'tx' is always non-NULL and 'len' is always > 0.
 *   Except for SOCK_DGRAM probe packets (tx=NULL and len==0).
 */
static int ip4_transmit (Socket *socket, const void *tx, unsigned len)
{
  eth_address  eth;
  u_long       dest;
  unsigned     tx_len, tx_room;
  sock_type   *sk  = (sock_type*) socket->raw_sock;
  struct ip   *ip  = (struct ip*) tx;
  const  BYTE *buf = (const BYTE*) tx;
  UINT   h_len, o_len;

  if (ip)            /* NULL if called from udp_transmit() */
  {
    if ((socket->so_state & SS_NBIO) &&
        sock_tbleft(sk) < (len + socket->send_lowat))
    {
      SOCK_DEBUGF ((", EWOULDBLOCK"));
      SOCK_ERRNO (EWOULDBLOCK);
      return (-1);
    }
  }

  SOCK_DEBUGF ((", %s / Raw", inet_ntoa(socket->remote_addr->sin_addr)));

  if (ip && (socket->inp_flags & INP_HDRINCL))
  {
    dest    = ip->ip_dst.s_addr;
    tx_len  = len;
    tx_room = _mtu;
  }
  else
  {
    dest    = socket->remote_addr->sin_addr.s_addr;
    tx_len  = len + sizeof (*ip);
    tx_room = _mtu + sizeof (*ip);
  }

  if (socket->ip_opt &&
      socket->ip_opt->ip_dst.s_addr)   /* using source routing */
     dest = socket->ip_opt->ip_dst.s_addr;

  if (!dest)
  {
    SOCK_DEBUGF ((", no dest"));
    SOCK_ERRNO (EHOSTUNREACH);
    STAT (ip4stats.ips_noroute++);
    return (-1);
  }

  if (!_arp_resolve(ntohl(dest),&eth))
  {
    SOCK_DEBUGF ((", no route"));
    SOCK_ERRNO (EHOSTUNREACH);
    return (-1);
  }

  if (socket->inp_flags & INP_HDRINCL)   /* IP-header included */
  {
    DWORD offset = ntohs (ip->ip_off);
    WORD  flags  = (WORD) (offset & ~IP_OFFMASK);

    offset = (offset & IP_OFFMASK) << 3; /* 0 <= ip_ofs <= 65536-8 */

    if ((flags & IP_DF) &&               /* DF requested */
        tx_len > tx_room)                /* tx-size above MTU */
    {
      SOCK_DEBUGF ((", EMSGSIZE"));
      SOCK_ERRNO (EMSGSIZE);
      STAT (ip4stats.ips_toolong++);
      return (-1);
    }
  }
  else if (tx_len + socket->ip_opt_len > tx_room)  /* tx-size above MTU */
#if defined(USE_FRAGMENTS)
  {
    if (socket->ip_opt_len > 0)
       ((void)0);  /** \todo Handle sending fragments with IP optons */
    return _IP4_SEND_FRAGMENTS (sk, socket->so_proto, dest, buf, len);
  }
#else
  {
    SOCK_DEBUGF ((", EMSGSIZE"));
    SOCK_ERRNO (EMSGSIZE);
    STAT (ip4stats.ips_toolong++);
    return (-1);
  }
#endif


  /* "Normal" small (tx_len < MTU) IPv4 packets are sent below
   */
  ip = (struct ip*) _eth_formatpacket (&eth, IP4_TYPE);

  if (socket->inp_flags & INP_HDRINCL)  /* caller provided IP-header */
  {
    memcpy (ip, buf, len);   /* SOCK_RAW can never have 0 length */
    if (ip->ip_src.s_addr == 0)
    {
      ip->ip_src.s_addr = gethostid();
      ip->ip_sum = 0;
      ip->ip_sum = ~CHECKSUM ((void*)ip, ip->ip_hl << 2);
    }
    if (ip->ip_sum == 0) /* add header checksum if needed */
        ip->ip_sum = ~CHECKSUM ((void*)ip, ip->ip_hl << 2);
  }
  else
  {
    if (socket->ip_opt && socket->ip_opt_len > 0)
    {
      BYTE *data;

      o_len = min (socket->ip_opt_len, sizeof(socket->ip_opt->IP_opts));
      h_len = sizeof(*ip) + o_len;
      data  = (BYTE*)ip + h_len;
      memcpy (ip+1, &socket->ip_opt->IP_opts[0], o_len);
      if (buf && len > 0)
         memcpy (data, buf, len);
      tx_len += o_len;
    }
    else
    {
      if (buf && len > 0)
         memcpy (ip+1, buf, len);
      h_len = sizeof (*ip);
    }

    ip->ip_v   = IPVERSION;
    ip->ip_hl  = h_len >> 2;
    ip->ip_tos = socket->ip_tos;
    ip->ip_len = htons (tx_len);
    ip->ip_id  = _get_ip4_id();
    ip->ip_off = 0;
    ip->ip_ttl = socket->ip_ttl;
    ip->ip_p   = socket->so_proto;

    ip->ip_src.s_addr = gethostid();
    ip->ip_dst.s_addr = dest;

    ip->ip_sum = 0;
    ip->ip_sum = ~CHECKSUM (ip, h_len);
  }

  if (!_eth_send (tx_len, NULL, __FILE__, __LINE__))
  {
    SOCK_DEBUGF ((", ENETDOWN"));
    SOCK_ERRNO (ENETDOWN);
    return (-1);
  }
  return (len);
}

#if defined(USE_IPV6)
/**
 * Raw IPv6 transmitter.
 * \note
 *   'tx' is always non-NULL and 'len' is always > 0.
 *   Except for SOCK_DGRAM probe packets (tx=NULL and len==0).
 */
static int ip6_transmit (Socket *socket, const void *tx, unsigned len)
{
  eth_address eth;
  ip6_address dest;
  unsigned    tx_len, tx_room;

  sock_type           *sk  = (sock_type*) socket->raw_sock;
  struct in6_Header   *ip6 = (struct in6_Header*) tx;
  struct sockaddr_in6 *ra  = (struct sockaddr_in6*) socket->remote_addr;
  const  BYTE         *buf = (const BYTE*) tx;

  if ((socket->so_state & SS_NBIO) &&
      sock_tbleft(sk) < (len + socket->send_lowat))
  {
    SOCK_DEBUGF ((", EWOULDBLOCK"));
    SOCK_ERRNO (EWOULDBLOCK);
    return (-1);
  }

  SOCK_DEBUGF ((", %s / Raw", _inet6_ntoa(&ra->sin6_addr)));

  if (ip6 && (socket->inp_flags & INP_HDRINCL))
  {
    memcpy (dest, &ip6->destination[0], sizeof(dest));
    tx_len  = len;
    tx_room = _mtu;
  }
  else
  {
    memcpy (dest, &ra->sin6_addr, sizeof(dest));
    tx_len  = len + sizeof (*ip6);
    tx_room = _mtu + sizeof (*ip6);
  }

  if (IN6_IS_ADDR_UNSPECIFIED(&dest) || !icmp6_neigh_solic(&dest,&eth))
  {
    SOCK_DEBUGF ((", no route"));
    SOCK_ERRNO (EHOSTUNREACH);
    STAT (ip6stats.ip6s_noroute++);
    return (-1);
  }

  if (!(socket->inp_flags & INP_HDRINCL) &&
      tx_len + socket->ip_opt_len > tx_room)
  {
    SOCK_DEBUGF ((", EMSGSIZE"));    /** \todo support fragmentation */
    SOCK_ERRNO (EMSGSIZE);
    STAT (ip6stats.ip6s_odropped++);
    return (-1);
  }

  ip6 = (struct in6_Header*) _eth_formatpacket (&eth, IP6_TYPE);

  if (socket->inp_flags & INP_HDRINCL)
  {
    if (buf && len > 0)
       memcpy (ip6, buf, len);
    if (IN6_IS_ADDR_UNSPECIFIED(&ip6->source))
       memcpy (&ip6->source[0], _gethostid6(), sizeof(ip6->source));
  }
  else
  {
#if 0  /* option header not yet supported */
    if (socket->ip_opt && socket->ip_opt_len > 0)
    {
      BYTE *data;
      int   h_len;

      o_len = min (socket->ip_opt_len, sizeof(socket->ip_opt->ip_opts));
      h_len = sizeof(*ip) + o_len;
      data  = (BYTE*)ip + h_len;
      memcpy (ip+1, &socket->ip_opt->ip_opts, o_len);
      if (buf && len > 0)
         memcpy (data, buf, len);
      tx_len += o_len;
      if (socket->ip_opt->ip_dst.s_addr)   /* using source routing */
         dest = socket->ip_opt->ip_dst.s_addr;
    }
    else
#endif
    {
      if (buf && len > 0)
         memcpy (ip6+1, buf, len);
    }
    ip6->pri       = 0;
    ip6->ver       = 6;
    ip6->len       = htons (len);
    ip6->next_hdr  = socket->so_proto;
    ip6->hop_limit = _default_ttl;

    memset (&ip6->flow_lbl[0], 0, sizeof(ip6->flow_lbl));
    memcpy (&ip6->source[0], _gethostid6(), sizeof(ip6->source));
    memcpy (&ip6->destination[0], dest, sizeof(ip6->destination));
  }

  if (!_eth_send (tx_len, NULL, __FILE__, __LINE__))
  {
    SOCK_DEBUGF ((", ENETDOWN"));
    SOCK_ERRNO (ENETDOWN);
    return (-1);
  }
  return (len);
}
#endif  /* USE_IPV6 */


static int raw_transmit (Socket *socket, const void *buf, unsigned len)
{
#if 0
  SOCK_ENTER_SCOPE();
  tcp_tick (NULL);       /* process other TCBs too */
  tcp_Retransmitter (TRUE);
  SOCK_LEAVE_SCOPE();
#endif

#if defined(USE_IPV6)
  if (socket->so_family == AF_INET6)
     return ip6_transmit (socket, buf, len);
#endif

  if (socket->so_family == AF_INET)
     return ip4_transmit (socket, buf, len);

  SOCK_DEBUGF ((", EAFNOSUPPORT "));
  SOCK_ERRNO (EAFNOSUPPORT );
  return (-1);
}
#endif  /* USE_BSD_API */
