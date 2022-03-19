/*!\file sockopt.c
 * BSD setsockopt(), getsockopt().
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
 *  0.6 : Apr 07, 2000 : Added multicast options IP_ADD_MEMBERSHIP and
 *                       IP_DROP_MEMBERSHIP. Vlad Erochine <vlad@paragon.ru>
 *  0.7 : Jun 06, 2000 : Added support for SO_SNDLOWAT and SO_RCVLOWAT
 */

#include "socket.h"

#if defined(USE_BSD_API)

static int set_sol_opt  (Socket *s, int opt, const void *val, unsigned len);
static int set_raw_opt  (Socket *s, int opt, const void *val, int len);
static int get_sol_opt  (Socket *s, int opt, void *val, int *len);
static int get_raw_opt  (Socket *s, int opt, void *val, int *len);

static int set_tcp_opt  (_tcp_Socket *tcp, int opt, const void *val, int len);
static int set_udp_opt  (_udp_Socket *udp, int opt, const void *val, int len);
static int get_tcp_opt  (_tcp_Socket *tcp, int opt, void *val, int *len);
static int get_udp_opt  (_udp_Socket *udp, int opt, void *val, int *len);

static int set_recv_buf (sock_type *s, DWORD size, BOOL is_tcp);
static int set_xmit_buf (sock_type *s, DWORD size, BOOL is_tcp);
static int raw_rx_buf   (_raw_Socket  *raw, DWORD size);
static int raw_tx_buf   (_raw_Socket  *raw, DWORD size);
static int raw6_rx_buf  (_raw6_Socket *raw, DWORD size);
static int raw6_tx_buf  (_raw6_Socket *raw, DWORD size);

static int set_tx_lowat (Socket *s, unsigned size);
static int set_rx_lowat (Socket *s, unsigned size);
static int get_tx_lowat (const Socket *s, unsigned *size);
static int get_rx_lowat (const Socket *s, unsigned *size);

#if defined(USE_DEBUG)
static const char *sockopt_name (int level, int option);
#endif

int W32_CALL setsockopt (int s, int level, int option, const void *optval, socklen_t optlen)
{
  Socket *socket = _socklist_find (s);
  int     rc;

  SOCK_DEBUGF (("\nsetsockopt:%d, %s", s, sockopt_name(level,option)));

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

  VERIFY_RW (optval, optlen);

  if ((WORD)level == SOL_SOCKET)
     rc = set_sol_opt (socket, option, optval, optlen);

#if 0
  else if ((WORD)level == SOL_PACKET)
     rc = set_packet_opt (socket, option, optval, optlen);
#endif

  else if (level == socket->so_proto && level == IPPROTO_TCP)
     rc = set_tcp_opt (socket->tcp_sock, option, optval, optlen);

  else if (level == socket->so_proto && level == IPPROTO_UDP)
     rc = set_udp_opt (socket->udp_sock, option, optval, optlen);

  else if ((level == socket->so_proto && level == IPPROTO_IP)   ||
           (level == socket->so_proto && level == IPPROTO_ICMP) ||
           socket->so_proto == IPPROTO_UDP ||
           socket->so_proto == IPPROTO_ICMP)
     rc = set_raw_opt (socket, option, optval, optlen);

  else
  {
    SOCK_ERRNO (ENOPROTOOPT);
    rc = -1;
  }

  if (rc < 0)
     SOCK_DEBUGF ((", %s", short_strerror(_w32_errno)));

  return (rc);
}

int W32_CALL getsockopt (int s, int level, int option, void *optval, socklen_t *optlen)
{
  Socket *socket = _socklist_find (s);
  int     rc;

  SOCK_DEBUGF (("\ngetsockopt:%d, %s", s, sockopt_name(level,option)));

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

  VERIFY_RW (optlen, sizeof(u_long));
  VERIFY_RW (optval, *optlen);

  if ((WORD)level == SOL_SOCKET)
     rc = get_sol_opt (socket, option, optval, optlen);

#if 0
  else if ((WORD)level == SOL_PACKET)
     rc = get_packet_opt (socket, option, optval, optlen);
#endif

  else if (level == socket->so_proto && level == IPPROTO_TCP)
     rc = get_tcp_opt (socket->tcp_sock, option, optval, optlen);

  else if (level == socket->so_proto && level == IPPROTO_UDP)
     rc = get_udp_opt (socket->udp_sock, option, optval, optlen);

  else if ((level == socket->so_proto && level == IPPROTO_IP)   ||
           (level == socket->so_proto && level == IPPROTO_ICMP) ||
           socket->so_proto == IPPROTO_UDP)
     rc = get_raw_opt (socket, option, optval, optlen);

  else
  {
    SOCK_ERRNO (ENOPROTOOPT);
    rc = -1;
  }

  if (rc < 0)
     SOCK_DEBUGF ((", %s", short_strerror(_w32_errno)));

  return (rc);
}


static int set_sol_opt (Socket *s, int opt, const void *val, unsigned len)
{
  struct timeval *tv   = (struct timeval*) val;
  int             on   = *(int*) val;
  DWORD           size = (len >= 4) ? *(DWORD*)val :
                         (len >= 2) ? *(WORD*)val  :
                         (len >= 1) ? *(BYTE*)val  : 0U;
  switch (opt)
  {
    case SO_DEBUG:
    case SO_ACCEPTCONN:
         if (on)
              s->so_options |=  opt;
         else s->so_options &= ~opt;
         SOCK_DEBUGF ((" %d", s->so_options & opt));
         break;

    case SO_RCVTIMEO:
         if (len != sizeof(*tv) || tv->tv_usec < 0)
         {
           SOCK_ERRNO (EINVAL);
           return (-1);
         }
         if (tv->tv_sec == 0)        /* i.e. use system default */
              s->timeout = sock_delay;
         else s->timeout = (DWORD)tv->tv_sec + tv->tv_usec/1000000UL;
         SOCK_DEBUGF ((" %d", s->timeout));
         break;

    case SO_SNDTIMEO:    /* Don't think we need this */
         break;

    /*
     * SO_REUSEADDR enables local address reuse. E.g. used to bind
     * multiple sockets to the same port but with different ip-addr.
     */
    case SO_REUSEADDR:
         if (s->tcp_sock && s->so_proto == IPPROTO_TCP)
         {
           /* This is meaningless unless a local port is bound.
            * myport is 0 before a connect() is done.
            */
           reuse_localport (s->tcp_sock->myport);
           return (0);
         }
         if (s->udp_sock && s->so_proto == IPPROTO_UDP)
         {
           reuse_localport (s->udp_sock->myport);
           return (0);
         }
         SOCK_ERRNO (ENOPROTOOPT);
         return (-1);

    /*
     * SO_REUSEPORT enables duplicate address and port bindings
     * ie, one can bind multiple socks to the same <ip_addr.port> pair
     */
/*  case SO_REUSEPORT:  missing in BSD? */
    case SO_DONTROUTE:
    case SO_BROADCAST:
    case SO_USELOOPBACK:
    case SO_OOBINLINE:
         break;

    case SO_DONTLINGER:
         s->linger_time = 0;
         s->linger_on   = 0;
         s->tcp_sock->locflags &= ~LF_LINGER;
         break;

    case SO_KEEPALIVE:
         if (on)
              s->so_options |=  SO_KEEPALIVE;
         else s->so_options &= ~SO_KEEPALIVE;
         SOCK_DEBUGF ((" %d", s->so_options & SO_KEEPALIVE));
         break;

    case SO_SNDLOWAT:
         return set_tx_lowat (s, size);

    case SO_RCVLOWAT:
         return set_rx_lowat (s, size);

    case SO_RCVBUF:
         if (size == 0)
         {
           SOCK_ERRNO (EINVAL);
           return (-1);
         }
         if (s->udp_sock && s->so_proto == IPPROTO_UDP)
            return set_recv_buf ((sock_type*)s->udp_sock, size, FALSE);

         if (s->tcp_sock && s->so_proto == IPPROTO_TCP)
            return set_recv_buf ((sock_type*)s->tcp_sock, size, TRUE);

         if (s->raw_sock)
            return raw_rx_buf (s->raw_sock, size);

         if (s->raw6_sock)
            return raw6_rx_buf (s->raw6_sock, size);

         SOCK_ERRNO (ENOPROTOOPT);
         return (-1);

    case SO_SNDBUF:
         if (size == 0)
         {
           SOCK_ERRNO (EINVAL);
           return (-1);
         }
         if (s->udp_sock && s->so_proto == IPPROTO_UDP)
            return set_xmit_buf ((sock_type*)s->udp_sock, size, FALSE);

         if (s->tcp_sock && s->so_proto == IPPROTO_TCP)
            return set_xmit_buf ((sock_type*)s->tcp_sock, size, TRUE);

         if (s->raw_sock)
            return raw_tx_buf (s->raw_sock, size);

         if (s->raw6_sock)
            return raw6_tx_buf (s->raw6_sock, size);

         SOCK_ERRNO (ENOPROTOOPT);
         return (-1);

    case SO_LINGER:
         {
           struct linger *linger = (struct linger*) val;

           if (len < sizeof(*linger))
           {
             SOCK_ERRNO (EINVAL);
             return (-1);
           }

           if (s->so_type != SOCK_STREAM || !s->tcp_sock)
           {
             SOCK_ERRNO (ENOPROTOOPT);
             return (-1);
           }

           if (linger->l_onoff == 0 && linger->l_linger == 0)
           {
             s->linger_time = 0;
             s->linger_on   = 0;
             s->tcp_sock->locflags &= ~LF_LINGER;
           }
           else if (linger->l_onoff /* && linger->l_linger > 0 */ )
           {
             unsigned sec = TCP_LINGERTIME;

             if (linger->l_linger < 100 * TCP_LINGERTIME)
                sec = linger->l_linger / 100;  /* in 10ms units */

             s->linger_time = sec;
             s->linger_on   = 1;
             s->tcp_sock->locflags |= LF_LINGER;
           }
           s->so_options |= SO_LINGER;
         }
         break;

    default:
         SOCK_ERRNO (ENOPROTOOPT);
         return (-1);
  }
  return (0);
}


static int get_sol_opt (Socket *s, int opt, void *val, int *len)
{
  struct timeval *tv;
  unsigned       *size = (unsigned*)val;

  switch (opt)
  {
    case SO_DEBUG:
    case SO_ACCEPTCONN:
         *(int*)val = (s->so_options & opt);
         *len = sizeof(int);
         break;
#if 0
    case SO_OOBINLINE:   /** \todo Handle urgent (OOB) data */
         if (!s->tcp_sock || s->so_proto != IPPROTO_TCP)
         {
           SOCK_ERRNO (ENOPROTOOPT);
           return (-1);
         }
         if (s->so_options & SO_OOBINLINE)
              *(int*)val = urgent_data ((sock_type*)s->tcp_sock);
         else *(int*)val = 0;
         break;
#endif

    /** \todo Handle these SOL_OPTIONS:
     */
    case SO_REUSEADDR:
    case SO_DONTROUTE:
    case SO_BROADCAST:
    case SO_USELOOPBACK:
         break;

    case SO_DONTLINGER:
         *(int*)val = !(s->tcp_sock->locflags & LF_LINGER);
         break;

    case SO_KEEPALIVE:
         if (s->so_options & SO_KEEPALIVE)
         {
           *(int*)val = tcp_keep_idle;
           *len = sizeof(tcp_keep_idle);
         }
         else
         {
           *(int*)val = 0;
           *len = 0;
         }
         break;

    case SO_SNDLOWAT:
         return get_tx_lowat (s, size);

    case SO_RCVLOWAT:
         return get_rx_lowat (s, size);

    case SO_RCVBUF:
         if (s->udp_sock && s->so_proto == IPPROTO_UDP)
         {
           *(int*)val = sock_rbsize ((sock_type*)s->udp_sock);
           return (0);
         }
         if (s->tcp_sock && s->so_proto == IPPROTO_TCP)
         {
           *(int*)val = sock_rbsize ((sock_type*)s->tcp_sock);
           return (0);
         }
         if (s->raw_sock)
         {
           *(size_t*)val = sizeof (s->raw_sock->rx_data);
           return (0);
         }
         SOCK_ERRNO (ENOPROTOOPT);
         return (-1);

    case SO_SNDBUF:
         if (s->udp_sock && s->so_proto == IPPROTO_UDP)
         {
           *(unsigned*)val = 0;  /* UDP doesn't have a Tx-buffer */
           return (0);
         }
         if (s->tcp_sock && s->so_proto == IPPROTO_TCP)
         {
           *(unsigned*)val = sock_tbsize ((sock_type*)s->tcp_sock);
           return (0);
         }
         if (s->raw_sock)
         {
           *(unsigned*)val = _mtu;
           return (0);
         }
         SOCK_ERRNO (ENOPROTOOPT);
         return (-1);

    case SO_LINGER:
         {
           struct linger *linger = (struct linger*) val;

           if (!len || *len < SIZEOF(*linger))
           {
             SOCK_ERRNO (EINVAL);
             return (-1);
           }
           if (s->so_type != SOCK_STREAM || !s->tcp_sock)
           {
             SOCK_ERRNO (ENOPROTOOPT);
             return (-1);
           }
           *len = SIZEOF(*linger);
           linger->l_onoff  = (s->tcp_sock->locflags & LF_LINGER) ? 1 : 0;
           linger->l_linger = 100 * s->linger_time;
           SOCK_DEBUGF ((", linger %d, %s",
                         linger->l_linger, linger->l_onoff ? "on" : "off"));
         }
         break;

    case SO_SNDTIMEO:
         break;

    case SO_RCVTIMEO:
         if (*len < SIZEOF(*tv))
         {
           SOCK_ERRNO (EINVAL);
           return (-1);
         }
         tv = (struct timeval*)val;
         if (s->timeout == 0)
         {
           tv->tv_usec = LONG_MAX;
           tv->tv_sec  = LONG_MAX;
         }
         else
         {
           tv->tv_usec = 0;
           tv->tv_sec  = s->timeout;
         }
         SOCK_DEBUGF ((", timeout %lu.%06ld", (u_long)tv->tv_sec, tv->tv_usec));
         break;

    case SO_ERROR:
         *(int*)val = s->so_error;
         *len = SIZEOF(s->so_error);
         SOCK_DEBUGF ((", val %d=%s",
                      s->so_error, short_strerror(s->so_error)));
         s->so_error = 0;   /* Linux man-page states we should clear this */
         break;

 /* case SO_STYLE: GNU libc name */
    case SO_TYPE:
         *(int*)val = s->so_type;
         *len = SIZEOF(s->so_type);
         SOCK_DEBUGF ((", type %d", s->so_type));
         break;

    default:
         SOCK_ERRNO (ENOPROTOOPT);
         return (-1);
  }
  SOCK_ERRNO (0);
  return (0);
}

/*
 * set/get TCP-layer options
 */
static int set_tcp_opt (_tcp_Socket *tcp, int opt, const void *val, int len)
{
  BOOL on = *(BOOL*)val;
  UINT MSS;

  switch (opt)
  {
    case TCP_NODELAY:
         if (on)   /* disable Nagle's algorithm */
              tcp->sockmode &= ~SOCK_MODE_NAGLE;
         else tcp->sockmode |= SOCK_MODE_NAGLE;
         break;

    case TCP_MAXSEG:
         MSS = *(DWORD*)val;
         MSS = max (MSS, MSS_MIN);
         MSS = min (MSS, MSS_MAX);
         tcp->max_seg = MSS;
         break;

    case TCP_NOPUSH:
         if (on)
              tcp->locflags |=  LF_NOPUSH;
         else tcp->locflags &= ~LF_NOPUSH;
         break;

    case TCP_NOOPT:
         if (on)
              tcp->locflags |=  LF_NOOPT;
         else tcp->locflags &= ~LF_NOOPT;
         break;

    default:
         SOCK_ERRNO (ENOPROTOOPT);
         return (-1);
  }
  ARGSUSED (len);
  return (0);
}

static int get_tcp_opt (_tcp_Socket *tcp, int opt, void *val, int *len)
{
  switch (opt)
  {
    case TCP_NODELAY:
         if (tcp->sockmode & SOCK_MODE_NAGLE)
              *(int*)val = 0;
         else *(int*)val = TCP_NODELAY;
         *len = SIZEOF(int);
         break;

    case TCP_MAXSEG:
         *(int*)val = tcp->max_seg;
         *len = SIZEOF(int);
         break;

    case TCP_NOPUSH:
         *(int*)val = (tcp->locflags & LF_NOPUSH);
         *len = SIZEOF(tcp->locflags);
         break;

    case TCP_NOOPT:
         *(int*)val = (tcp->locflags & LF_NOOPT);
         *len = SIZEOF(tcp->locflags);
         break;

    default:
         SOCK_ERRNO (ENOPROTOOPT);
         return (-1);
  }
  return (0);
}

/*
 * set/get UDP-layer options
 */
static int set_udp_opt (_udp_Socket *udp, int opt, const void *val, int len)
{
  ARGSUSED (udp);      /* no udp option support yet */
  ARGSUSED (opt);
  ARGSUSED (val);
  ARGSUSED (len);
  SOCK_ERRNO (ENOPROTOOPT);
  return (-1);
}


static int get_udp_opt (_udp_Socket *udp, int opt, void *val, int *len)
{
  ARGSUSED (udp);    /* no udp option support yet */
  ARGSUSED (opt);
  ARGSUSED (val);
  ARGSUSED (len);
  SOCK_ERRNO (ENOPROTOOPT);
  return (-1);
}

/*
 * set/get IP/ICMP-layer (raw/multicast) options
 */
static int set_raw_opt (Socket *s, int opt, const void *val, int len)
{
#ifdef USE_MULTICAST
  struct ip_mreq *mc_req;
  DWORD  ip;
#endif
  BOOL on = *(BOOL*)val;

  switch (opt)
  {
    case IP_OPTIONS:
         if (!s->ip_opt &&
             (s->ip_opt = SOCK_CALLOC (sizeof(*s->ip_opt))) == NULL)
         {
           SOCK_ERRNO (ENOMEM);
           return (-1);
         }
         if (len == 0 && s->ip_opt)
         {
           free (s->ip_opt);
           s->ip_opt     = NULL;
           s->ip_opt_len = 0;
         }
         else
         {
           s->ip_opt_len = min ((unsigned)len, sizeof(*s->ip_opt));
           if (len > 0)
              memcpy (&s->ip_opt->IP_opts[0], val, s->ip_opt_len);
         }
         break;

    case IP_HDRINCL:
         if (on)
              s->inp_flags |=  INP_HDRINCL;
         else s->inp_flags &= ~INP_HDRINCL;
         break;

    case IP_TOS:
         s->ip_tos = *(int*)val;
         if (s->tcp_sock)
             s->tcp_sock->tos = s->ip_tos;
         else if (s->raw_sock)
            s->raw_sock->ip.tos = s->ip_tos;
         break;

    case IP_TTL:
    case IP_MULTICAST_TTL:
         s->ip_ttl = *(BYTE*)val;
         s->ip_ttl = max (1, s->ip_ttl);
         if (s->udp_sock)
            s->udp_sock->ttl = s->ip_ttl;
         else if (s->tcp_sock)
            s->tcp_sock->ttl = s->ip_ttl;
         else if (s->raw_sock)
            s->raw_sock->ip.ttl = s->ip_ttl;
         break;

    case IP_ADD_MEMBERSHIP:
    case IP_DROP_MEMBERSHIP:
#ifdef USE_MULTICAST
         _multicast_on = TRUE;
         mc_req = (struct ip_mreq*)val;
         if (!mc_req || len < SIZEOF(*mc_req))
         {
           SOCK_ERRNO (EINVAL);
           return (-1);
         }
         ip = ntohl (mc_req->imr_multiaddr.s_addr);
         if (!_ip4_is_multicast(ip))
         {
           SOCK_ERRNO (EINVAL);
           return (-1);
         }
         if (opt == IP_ADD_MEMBERSHIP && !join_mcast_group(ip))
         {
           SOCK_ERRNO (ENOBUFS);        /* !!correct errno? */
           return (-1);
         }
         if (opt == IP_DROP_MEMBERSHIP && !leave_mcast_group(ip))
         {
           SOCK_ERRNO (EADDRNOTAVAIL);  /* !!correct errno? */
           return (-1);
         }
#endif
         break;

    case IP_MULTICAST_IF:
    case IP_MULTICAST_LOOP:
    case IP_MULTICAST_VIF:
         _multicast_on = TRUE;
         break;

    case IP_RECVOPTS:
    case IP_RECVRETOPTS:
    case IP_RECVDSTADDR:
    case IP_RETOPTS:
    case IP_RSVP_ON:
    case IP_RSVP_OFF:
    case IP_RSVP_VIF_ON:
    case IP_RSVP_VIF_OFF:
    case IP_PORTRANGE:
    case IP_RECVIF:
         break;

    default:
         SOCK_ERRNO (ENOPROTOOPT);
         return (-1);
  }
  return (0);
}

static int get_raw_opt (Socket *s, int opt, void *val, int *len)
{
  switch (opt)
  {
    case IP_OPTIONS:
         if (s->ip_opt)
         {
           *len = (int)s->ip_opt_len;
           memcpy (val, &s->ip_opt->IP_opts[0], *len);
         }
         else
         {
           memset (val, 0, *len);
           *len = 0;
         }
         break;

    case IP_HDRINCL:
         *(int*)val = (s->inp_flags & INP_HDRINCL);
         *len = sizeof(int);
         break;

    case IP_TOS:
         *(int*)val = s->ip_tos;
         *len = sizeof(int);
         break;

    case IP_TTL:
    case IP_MULTICAST_TTL:
         *(BYTE*)val = s->ip_ttl;
         *len = 1;
         break;

    case IP_RECVOPTS:
    case IP_RECVRETOPTS:
    case IP_RECVDSTADDR:
    case IP_RETOPTS:
    case IP_RSVP_ON:
    case IP_RSVP_OFF:
    case IP_RSVP_VIF_ON:
    case IP_RSVP_VIF_OFF:
    case IP_PORTRANGE:
    case IP_RECVIF:
         break;

    case IP_ADD_MEMBERSHIP:
    case IP_DROP_MEMBERSHIP:
    case IP_MULTICAST_IF:
         _multicast_on = TRUE;
         break;

    case IP_MULTICAST_LOOP:
         _multicast_on = TRUE;
         *(BYTE*)val = 0;
         *len = 1;
         break;

    case IP_MULTICAST_VIF:
         _multicast_on = TRUE;
         break;

    default:
         SOCK_ERRNO (ENOPROTOOPT);
         return (-1);
  }
  return (0);
}

/*
 * Set receive buffer size for UDP/TCP. Since sock_setbuf() handles
 * maxiumum 64kB, we do the same here.
 * Maximum size accepted for TCP is '64k * (2 << TCP_MAX_WINSHIFT)' = 1MByte.
 * Minimum size is 1 byte.
 */
static int set_recv_buf (sock_type *s, DWORD size, BOOL is_tcp)
{
  BYTE  *buf;
  DWORD  len;
  DWORD  max = is_tcp ? MAX_TCP_RECV_BUF : MAX_UDP_RECV_BUF;

  if (s->tcp.rx_data == &s->tcp.rx_buf[0])
       buf = NULL;
  else buf = s->tcp.rx_data-4;

  size = min (size, max);  /* 64kB/1MB */
  buf  = (BYTE*) realloc (buf, size+8);
  if (!buf)
  {
    SOCK_ERRNO (ENOMEM);
    return (-1);
  }

  /* Copy the data to new buffer. Data might be overlapping
   * hence using memmove(). Add front/back markers.
   */
  if (s->tcp.rx_datalen > 0)
  {
    len = min ((long)size-8, s->tcp.rx_datalen);
    memmove (buf+4, s->tcp.rx_data, len);
  }
  *(DWORD*)buf          = SAFETY_TCP;
  *(DWORD*)(buf+4+size) = SAFETY_TCP;
  s->tcp.rx_data     = buf + 4;
  s->tcp.max_rx_data = size - 1;

  if (is_tcp && size > USHRT_MAX)
     s->tcp.tx_wscale = (BYTE) (size >> 16);  /* not yet */

  SOCK_DEBUGF ((" %lu", (u_long)size));
  return (0);
}

/*
 * Set transmit buffer size for UDP/TCP.
 * Max size accepted is 'max'.
 * size == 0 is legal, but how should we handle that?
 */
static int set_xmit_buf (sock_type *s, DWORD size, BOOL is_tcp)
{
#if 0 /* not yet */
  BYTE  *buf;
  DWORD  max = is_tcp ? MAX_TCP_SEND_BUF : MAX_UDP_SEND_BUF;
  size_t len;

  size = min (size, max);
  buf  = realloc (s->tcp.tx_data, size);
  if (!buf)
  {
    SOCK_ERRNO (ENOMEM);
    return (-1);
  }

  /* Copy current data to new buffer. Data might be overlapping
   * hence using memmove(). Add front/back markers.
   */
  if (s->tcp.tx_datalen > 0)
  {
    len = min ((long)size-8, s->tcp.tx_datalen);
    memmove (buf+4, s->tcp.tx_data, len);
  }
  *(DWORD*)buf          = SAFETY_TCP;
  *(DWORD*)(buf+size-4) = SAFETY_TCP;
  s->tcp.tx_data    = buf + 4;
  s->tcp.maxdatalen = size - (4+4+1);
  SOCK_DEBUGF ((" %lu", size));
#else
  ARGSUSED (s);
  ARGSUSED (size);
  ARGSUSED (is_tcp);
#endif
  return (0);
}

/*
 * Set receive buffer size for RAW socket
 */
static int raw_rx_buf (_raw_Socket *raw, DWORD size)
{
  /** \todo Support setting Rx-buffer size of raw IPv4 sockets */
  SOCK_DEBUGF ((" %lu unsupported", (u_long)size));
  ARGSUSED (raw);
  ARGSUSED (size);
  return (0);
}

static int raw_tx_buf (_raw_Socket *raw, DWORD size)
{
  /** \todo Support setting Tx-buffer size of raw IPv4 sockets */
  SOCK_DEBUGF ((" %lu unsupported", (u_long)size));
  ARGSUSED (raw);
  ARGSUSED (size);
  return (0);
}

static int raw6_rx_buf (_raw6_Socket *raw, DWORD size)
{
  /** \todo Support setting Rx-buffer size of raw IPv6 sockets */
  SOCK_DEBUGF ((" %lu unsupported", (u_long)size));
  ARGSUSED (raw);
  ARGSUSED (size);
  return (0);
}

static int raw6_tx_buf (_raw6_Socket *raw, DWORD size)
{
  /** \todo Support setting Tx-buffer size of raw IPv6 sockets */
  SOCK_DEBUGF ((" %lu unsupported", (u_long)size));
  ARGSUSED (raw);
  ARGSUSED (size);
  return (0);
}

/*
 * Set send buffer "low water marks"; x >= 0, x < send-size.
 */
static int set_tx_lowat (Socket *sock, unsigned size)
{
  switch (sock->so_type)
  {
    case SOCK_STREAM:
         sock->send_lowat = min (size, MAX_TCP_SEND_BUF-1);
         break;
    case SOCK_DGRAM:
         sock->send_lowat = min (size, MAX_UDP_SEND_BUF-1);
         break;
    case SOCK_RAW:
         sock->send_lowat = min (size, _mtu);
         break;
    default:
         SOCK_ERRNO (ENOPROTOOPT);
         return (-1);
  }
  return (0);
}

/*
 * Set receive buffer "low water marks"; x >= 0, x < recv-size.
 */
static int set_rx_lowat (Socket *sock, unsigned size)
{
  switch (sock->so_type)
  {
    case SOCK_STREAM:
         if (sock->tcp_sock)
            sock->recv_lowat = min (size, sock->tcp_sock->max_rx_data-1);
         break;
    case SOCK_DGRAM:
         if (sock->udp_sock)
            sock->recv_lowat = min (size, sock->udp_sock->max_rx_data-1);
         break;
    case SOCK_RAW:
         sock->recv_lowat = min (size, sizeof(sock->raw_sock->rx_data)-1);
         break;
    default:
         SOCK_ERRNO (ENOPROTOOPT);
         return (-1);
  }
  return (0);
}

/*
 * Get receive/transmit buffer "low water marks"
 */
static int get_tx_lowat (const Socket *sock, unsigned *size)
{
  if (sock->so_type == SOCK_STREAM ||
      sock->so_type == SOCK_DGRAM  ||
      sock->so_type == SOCK_RAW)
  {
    *size = sock->send_lowat;
    return (0);
  }
  SOCK_ERRNO (ENOPROTOOPT);
  return (-1);
}

static int get_rx_lowat (const Socket *sock, unsigned *size)
{
  if (sock->so_type == SOCK_STREAM ||
      sock->so_type == SOCK_DGRAM  ||
      sock->so_type == SOCK_RAW)
  {
    *size = sock->recv_lowat;
    return (0);
  }
  SOCK_ERRNO (ENOPROTOOPT);
  return (-1);
}

#if defined(USE_DEBUG)
/*
 * Handle printing of option names
 */
static const struct search_list sol_options[] = {
                  { SO_DEBUG,       "SO_DEBUG"       },
                  { SO_ACCEPTCONN,  "SO_ACCEPTCONN"  },
                  { SO_REUSEADDR,   "SO_REUSEADDR"   },
                  { SO_KEEPALIVE,   "SO_KEEPALIVE"   },
                  { SO_DONTROUTE,   "SO_DONTROUTE"   },
                  { SO_BROADCAST,   "SO_BROADCAST"   },
                  { SO_USELOOPBACK, "SO_USELOOPBACK" },
                  { SO_LINGER,      "SO_LINGER"      },
                  { SO_OOBINLINE,   "SO_OOBINLINE"   },
                  { SO_SNDBUF,      "SO_SNDBUF"      },
                  { SO_RCVBUF,      "SO_RCVBUF"      },
                  { SO_SNDLOWAT,    "SO_SNDLOWAT"    },
                  { SO_RCVLOWAT,    "SO_RCVLOWAT"    },
                  { SO_SNDTIMEO,    "SO_SNDTIMEO"    },
                  { SO_RCVTIMEO,    "SO_RCVTIMEO"    },
                  { SO_ERROR,       "SO_ERROR"       },
                  { SO_TYPE,        "SO_TYPE"        }
                };

static const struct search_list tcp_options[] = {
                  { TCP_NODELAY, "TCP_NODELAY" },
                  { TCP_MAXSEG,  "TCP_MAXSEG"  },
                  { TCP_NOPUSH,  "TCP_NOPUSH"  },
                  { TCP_NOOPT,   "TCP_NOOPT"   }
                };

static const struct search_list ip_options[] = {
                  { IP_OPTIONS,        "IP_OPTIONS"         },
                  { IP_HDRINCL,        "IP_HDRINCL"         },
                  { IP_TOS,            "IP_TOS"             },
                  { IP_TTL,            "IP_TTL"             },
                  { IP_RECVOPTS,       "IP_RECVOPTS"        },
                  { IP_RECVRETOPTS,    "IP_RECVRETOPTS"     },
                  { IP_RECVDSTADDR,    "IP_RECVDSTADDR"     },
                  { IP_RETOPTS,        "IP_RETOPTS"         },
                  { IP_MULTICAST_IF,   "IP_MULTICAST_IF"    },
                  { IP_MULTICAST_TTL,  "IP_MULTICAST_TTL"   },
                  { IP_MULTICAST_LOOP, "IP_MULTICAST_LOOP"  },
                  { IP_ADD_MEMBERSHIP, "IP_ADD_MEMBERSHIP"  },
                  { IP_DROP_MEMBERSHIP,"IP_DROP_MEMBERSHIP" },
                  { IP_MULTICAST_VIF,  "IP_MULTICAST_VIF"   },
                  { IP_RSVP_ON,        "IP_RSVP_ON"         },
                  { IP_RSVP_OFF,       "IP_RSVP_OFF"        },
                  { IP_RSVP_VIF_ON,    "IP_RSVP_VIF_ON"     },
                  { IP_RSVP_VIF_OFF,   "IP_RSVP_VIF_OFF"    },
                  { IP_PORTRANGE,      "IP_PORTRANGE"       },
                  { IP_RECVIF,         "IP_RECVIF"          },
                  { IP_FW_ADD,         "IP_FW_ADD"          },
                  { IP_FW_DEL,         "IP_FW_DEL"          },
                  { IP_FW_FLUSH,       "IP_FW_FLUSH"        },
                  { IP_FW_ZERO,        "IP_FW_ZERO"         },
                  { IP_FW_GET,         "IP_FW_GET"          },
                  { IP_NAT,            "IP_NAT"             }
                };


static const char *sockopt_name (int level, int option)
{
  switch ((DWORD)level)
  {
    case SOL_SOCKET:
         return list_lookup (option, sol_options, DIM(sol_options));

    case IPPROTO_UDP:
         return ("udp option!?");

    case IPPROTO_TCP:
         return list_lookup (option, tcp_options, DIM(tcp_options));

    case IPPROTO_IP:
    case IPPROTO_ICMP:
         return list_lookup (option, ip_options, DIM(ip_options));

    default:
         return ("invalid level?");
  }
}
#endif  /* USE_DEBUG    */
#endif  /* USE_BSD_API */
