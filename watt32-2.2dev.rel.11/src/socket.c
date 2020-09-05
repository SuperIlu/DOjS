/*!\file socket.c
 * BSD socket().
 */

/*  BSD sockets functionality for Watt-32 TCP/IP.
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
 *  0.6 : Aug 06, 2002 : G. Vanem - added AF_INET6 support
 */

#include "socket.h"
#include "pcdbug.h"
#include "pcicmp6.h"
#include "pcdns.h"

#if defined(USE_BSD_API)

#if defined(__DJGPP__)
  #include <sys/resource.h>
  #include <sys/fsext.h>
  #include <unistd.h>
  #if (DJGPP_MINOR >= 4)
  #include <libc/fd_props.h>
  #endif

  /* in fsext.c */
  extern int _fsext_demux (__FSEXT_Fnumber func, int *rv, va_list _args);

#elif defined(__CYGWIN__)
  #include <cygwin/version.h>

  extern long _get_osfhandle (int);   /* in cygwin1.dll (no prototype) */
#endif

static int     sk_block = 0;          /* sock_daemon() semaphore */
static int     sk_last  = SK_FIRST+1; /* highest socket number + 1 */
static Socket *sk_list  = NULL;
static DWORD   raw_seq  = 0;

static int (*tcp_syn_hook) (_tcp_Socket **tcp)    = NULL;
static int (*ip4_raw_hook) (const in_Header *ip)  = NULL;

#if defined(USE_IPV6)
static int (*ip6_raw_hook) (const in6_Header *ip) = NULL;
#endif

/** \todo
 * Use socket hash bucket to speedup _socklist_find() etc.
 */
#define SOCK_HASH_SIZE 64  /* must be 2^n */

static Socket *sk_hashes [SOCK_HASH_SIZE];

#if 0
  static __inline Socket *sock_hash_get (int fd)
  {
    return sk_hashes [fd & (SOCK_HASH_SIZE-1)];
  }
  static __inline void sock_hash_add (int fd, Socket *sock)
  {
    sk_hashes [fd & (SOCK_HASH_SIZE-1)] = sock;
  }
#else
  #define sock_hash_get(fd)       sk_list
  #define sock_hash_add(fd,sock)  ((void)0)
#endif

/**
 * Memory allocation; print some info if allocation fails.
 */
void *_sock_calloc (size_t size, const char *file, unsigned line)
{
  void *ptr;

#if defined(WATCOM386) && 0  /* find DOS4GW bug! */
  int rc = _heapset (0xCC);

  if (rc != _HEAPOK && rc != _HEAPEMPTY)
     SOCK_FATAL (("%s (%u) Fatal: heap corrupt\n", file, line));
#endif

#if defined(USE_FORTIFY)
  ptr = Fortify_calloc (size, 1, file, line);
#elif defined(_CRTDBG_MAP_ALLOC)  /* MSVC _DEBUG */
  ptr = _calloc_dbg (size, 1, _NORMAL_BLOCK, file, (int)line);
#else
  ptr = calloc (size, 1);
#endif

  if (!ptr)
  {
#if defined(WATCOM386) && 0  /* find DOS4GW bug! */
    struct _heapinfo hi;
    _heapwalk (&hi);
#endif
    SOCK_FATAL (("%s (%u) Fatal: Allocation failed\n", file, line));
  }

  ARGSUSED (file);
  ARGSUSED (line);
  return (ptr);
}


/**
 * `inuse[]' has a non-zero bit for each socket-descriptor in use.
 * There can be max `MAX_SOCKETS' allocated at any time.  Dead stream
 * sockets will be unlinked by `sock_daemon()' in due time.
 *
 * Non-djgpp targets:
 *   Allocate a new descriptor (handle) by searching through `inuse' for
 *   the first zero bit. Update `sk_last' as needed.
 *
 * djgpp target:
 *   Allocate a descriptior from the "File System Extension" layer.
 *   `sk_last' is not used (initialised to MAX_SOCKETS). Thus max # of
 *   sockets are limited by max # of DOS handles that can be created.
 */

static fd_set inuse [NUM_SOCK_FDSETS];  /* MAX_SOCKETS/8 = 625 bytes */

static int sock_alloc_fd (char **err)
{
  int fd;

  *err = NULL;

#if defined(__DJGPP__) && defined(USE_FSEXT)
  {
    static char err_buf[100];

    err_buf[0] = '\0';
    fd = __FSEXT_alloc_fd (_fsext_demux);

    if (fd < 0)
    {
      sprintf (err_buf, "FSEXT_alloc_fd() failed; %s", strerror(errno));
      *err = err_buf;
      return (-1);
    }

    if (FD_ISSET(fd,&inuse[0]))
    {
      sprintf (err_buf, "Reusing existing socket %d", fd);
      *err = err_buf;
      return (-1);
    }
  }

#if (DJGPP_MINOR >= 4) && 0
  {
    char dev_name[20];

    snprintf (dev_name, sizeof(dev_name), "/dev/socket/%d", fd);
    __set_fd_properties (fd, dev_name, 0);     /* last arg? */
    __set_fd_flags (fd, FILE_DESC_TEMPORARY);  /* auto close socket */
  }
#endif

#else  /* __DJGPP__ && USE_FSEXT */

  for (fd = SK_FIRST; fd < sk_last; fd++)
      if (!FD_ISSET(fd,&inuse[0]) &&  /* not marked as in-use */
          !_socklist_find(fd))        /* don't use a dying socket */
         break;

#endif /* __DJGPP__ && USE_FSEXT */

  if (fd < MAX_SOCKETS)
  {
    if (fd == sk_last)
       sk_last++;

    FD_SET (fd, &inuse[0]);
    return (fd);
  }

  /* No vacant bits in 'inuse' array. djgpp (and DOS) could theoretically
   * return a file-handle > 'MAX_SOCKETS-1'.
   */
  *err = (char*) "No more sockets";
  return (-1);
}

/**
 * _sock_dos_fd -
 *   Return TRUE if `s' is a valid DOS/Win32 handle.
 *   Used to differentiate EBADF from ENOTSOCK.
 *
 *   Note: for non-djgpp targets `s' may have same value as a
 *         DOS-handle. This function should only be used when `s'
 *         isn't found in `sk_list'.
 */
BOOL _sock_dos_fd (int s)
{
  if (s >= fileno(stdin) && s <= fileno(stderr)) /* 0..2 (redirected) */
     return (TRUE);

  if (s > fileno(stderr) && isatty(s))
     return (TRUE);

#if defined(WIN32)
  {
    HANDLE hnd = (HANDLE) _get_osfhandle (s);
    if (hnd != INVALID_HANDLE_VALUE)
    {
      CloseHandle (hnd);
      return (TRUE);
    }
  }
#endif

  return (FALSE);
}

/**
 * Setup a bigger receive buffer, the default in Wattcp
 * is only 2k.
 * \note If calloc() fails, sock_setbuf() reverts to default
 *       2kB socket buffer.
 * \todo allow user to define size using SO_RCVBUF/SO_SNDBUF
 *       before calling connect().
 */
int _sock_set_rcv_buf (sock_type *s, size_t len)
{
  len = min (len+8,USHRT_MAX);  /* add room for head/tail markers */
  return sock_setbuf (s, (BYTE*)malloc(len), len);

  /* Note: the ret-val from above malloc() is freed below.
   * Normally via tcp_daemon(). The freeing can take some time.
   * Hence reports about memory leaks (from e.g. MSVC debug-builds)
   * are not so important.
   */
}

/**
 * Free receive buffer associated with udp/tcp sockets.
 */
void _sock_free_rcv_buf (sock_type *s)
{
  if (s->tcp.rx_data &&   /* if xx_open() called */
      s->tcp.rx_data != &s->tcp.rx_buf[0])
  {
    *(DWORD*)(s->tcp.rx_data-4) = 0;  /* clear marker */
    free (s->tcp.rx_data-4);
    s->tcp.rx_data    = &s->tcp.rx_buf[0];
    s->tcp.rx_datalen = 0;
  }
}

/**
 * Deletes the list element associated with a socket.
 * Return pointer to next node.
 * Return NULL if no next or socket not found.
 */
static __inline Socket *sk_list_del (int s)
{
  Socket *sock, *last, *next;

  for (sock = last = sock_hash_get(s); sock; last = sock, sock = next)
  {
    next = sock->next;
    if (sock->fd != s)
       continue;

    if (sock == sk_list)
         sk_list    = next;
    else last->next = next;
    sock_hash_add (s, NULL);
    free (sock);
    return (next);
  }
  return (NULL);
}

/**
 * Hack function if user application needs to use Wattcp core functions
 * for BSD sockets. Must *not* modify return value in any way.
 */
const sock_type *__get_sock_from_s (int s, int proto)
{
  Socket *sock = _socklist_find (s);

  if (!sock)
     return (NULL);

  if (proto == UDP_PROTO)
     return (sock_type*) sock->udp_sock;

  if (proto == TCP_PROTO)
     return (sock_type*) sock->tcp_sock;

  if (proto == IPPROTO_IP)
     return (sock_type*) sock->raw_sock;

  return (NULL);
}

/**
 * Traverse socket-list to find other SOCK_STREAM sockets
 * besides '_this' which are also listening.
 */
static __inline BOOL other_tcp_listeners (const Socket *_this)
{
  const Socket *sock;

  for (sock = sk_list; sock; sock = sock->next)
      if (sock->so_type == SOCK_STREAM       &&
          (sock->so_options & SO_ACCEPTCONN) &&
          sock != _this)
        return (TRUE);
  return (FALSE);
}

void _sock_set_syn_hook (int (*func)(_tcp_Socket**))
{
  tcp_syn_hook = func;
}

/**
 * Traverse socket-list to find other SOCK_RAW sockets
 * besides '_this'. Unhook '_raw_ip?_hook' if none found.
 */
static __inline BOOL other_raw_socks (const Socket *_this)
{
  const Socket *sock;

  for (sock = sk_list; sock; sock = sock->next)
      if (sock->so_type == SOCK_RAW && sock != _this)
         return (TRUE);
  return (FALSE);
}

/**
 * Switch back to normal rx-mode (RXMODE_BROADCAST) if no multicasts
 * active and no more IPv6/SOCK_PACKET sockets besides '_this'.
 */
static BOOL mcast_set  = FALSE;
static BOOL promis_set = FALSE;

BOOL _sock_set_normal_rx_mode (const Socket *_this)
{
  const Socket *sock;
  BOOL  rc = TRUE;

#if defined(USE_MULTICAST)
  if (num_mcast_active() > 0)
     return (rc);
#endif

  for (sock = sk_list; sock; sock = sock->next)
      if (sock != _this &&
          (sock->so_family == AF_INET6 || sock->so_type == SOCK_PACKET))
         break;

  if (!sock)
  {
    rc = pkt_set_rcv_mode (RXMODE_BROADCAST);
    if (!rc)
       SOCK_DEBUGF (("\n  setting normal rx-mode failed; %s",
                     pkt_strerror(_pkt_errno)));
    _pkt_forced_rxmode = RXMODE_BROADCAST;
    mcast_set = FALSE;
  }
  return (rc);
}

BOOL _sock_set_mcast_rx_mode (void)
{
  BOOL rc = TRUE;

  if (!mcast_set)
#if defined(USE_IPV6)
     rc = _ip6_pkt_init();
#else
     rc = pkt_set_rcv_mode (RXMODE_MULTICAST2);
#endif

  mcast_set = TRUE;
  return (rc);
}

BOOL _sock_set_promisc_rx_mode (void)
{
  BOOL rc = TRUE;

  if (!promis_set)
     rc = pkt_set_rcv_mode (RXMODE_PROMISCOUS);
  promis_set = TRUE;
  return (rc);
}

/**
 * Free buffers and hooks used by a SOCK_RAW socket.
 */
static void sock_raw_del (Socket *sock)
{
#if defined(USE_IPV6)
  if (sock->so_family == AF_INET6)
  {
    _raw6_Socket *raw, *next;

    if (!other_raw_socks(sock))
       ip6_raw_hook = NULL;

    for (raw = sock->raw6_sock; raw; raw = next)
    {
      raw->ip_type = 0;
      next = raw->next;
      free (raw);
    }
    sock->raw6_sock = NULL;
  }
  else
#endif
  {
    _raw_Socket *raw, *next;

    if (!other_raw_socks(sock))
       ip4_raw_hook = NULL;

    for (raw = sock->raw_sock; raw; raw = next)
    {
      raw->ip_type = 0;
      next = raw->next;
      free (raw);
    }
    sock->raw_sock = NULL;
  }
}

/**
 * Allocate ringbuffer and rx-buffers for a SOCK_PACKET socket.
 */
static BOOL sock_packet_add (Socket *sock)
{
  sock_packet_pool *p = SOCK_CALLOC (sizeof(*p));

  sock->packet_pool = p;
  if (!sock->packet_pool)
     return (FALSE);
  pktq_init (&p->queue, sizeof(p->buf[0]), DIM(p->buf), (char*)p->buf);
  return (TRUE);
}

static void sock_packet_del (Socket *sock)
{
  if (sock->packet_pool)
  {
 /* SOCK_DEBUGF ((", num-drops %lu", sock->packet_pool->queue.num_drop); */
    free (sock->packet_pool);
  }
  sock->packet_pool = NULL;
  _eth_recv_peek = sock->old_eth_peek;
}

/*
 * Receiver for SOCK_PACKET sockets
 */
static int W32_CALL sock_packet_peek (const union link_Packet *pkt)
{
  Socket *sock, *last = NULL;

  for (sock = sk_list; sock; sock = sock->next)
      if (sock->so_type == SOCK_PACKET && sock->packet_pool)
      {
        struct pkt_ringbuf     *q = &sock->packet_pool->queue;
        struct sock_packet_buf *rx;
        size_t len;

#if defined(USE_DEBUG)
        if (!pktq_check(q))
        {
          TCP_CONSOLE_MSG (0, ("%s(%u): SOCK_PACKET queue munged, "
                           "fd %d\n", __FILE__, __LINE__, sock->fd));
          pktq_clear (q);
          continue;
        }
#endif
        if (pktq_in_index(q) == q->out_index)   /* no room */
        {
          q->num_drop++;
          TCP_CONSOLE_MSG (0, ("SOCK_PACKET drops %lu\n", (u_long)q->num_drop));
          continue;
        }

        rx = (struct sock_packet_buf*) pktq_in_buf (q);

        if (_eth_last.rx.size > 0)   /* Only for Win32 or USE_FAST_PKT */
             len = _eth_last.rx.size;
        else len = sizeof(rx->rx_buf) - 4;

        memcpy (rx->rx_buf, pkt, len);
        rx->rx_len = len;
        pktq_inc_in (q);
        last = sock;
      }

  if (last && last->old_eth_peek && last->old_eth_peek != _eth_recv_peek)
     return (*last->old_eth_peek) ((void*)pkt);
  return (1);
}

/**
 * Return correct PACKET_xx type.
 */
static char get_pkt_type_eth (const eth_Header *eth)
{
  if (!memcmp(&_eth_addr,&eth->destination,_eth_mac_len))
     return (PACKET_HOST);
  if (!memcmp(&_eth_addr,&eth->source,_eth_mac_len))
     return (PACKET_OUTGOING);
  if (!memcmp(&eth->destination,&_eth_brdcast,_eth_mac_len))
     return (PACKET_BROADCAST);
  if ((eth->destination[0] & 1) == 1)
     return (PACKET_MULTICAST);
  return (PACKET_OTHERHOST);     /* To someone else */
}

static char get_pkt_type_fddi (const fddi_Header *fddi)
{
  if (!memcmp(&_eth_addr,&fddi->destination,_eth_mac_len))
     return (PACKET_HOST);
  if (!memcmp(&_eth_addr,&fddi->source,_eth_mac_len))
     return (PACKET_OUTGOING);
  if (!memcmp(&fddi->destination,&_eth_brdcast,_eth_mac_len))
     return (PACKET_BROADCAST);
  if ((fddi->destination[0] & 1) == 1)
     return (PACKET_MULTICAST);
  return (PACKET_OTHERHOST);     /* To someone else */
}

static char get_pkt_type_tok (const tok_Header *tok)
{
  if (!memcmp(&_eth_addr,&tok->destination,_eth_mac_len))
     return (PACKET_HOST);
  if (!memcmp(&_eth_addr,&tok->source,_eth_mac_len))
     return (PACKET_OUTGOING);
  if (!memcmp(&tok->destination,&_eth_brdcast,_eth_mac_len))
     return (PACKET_BROADCAST);
  if ((tok->destination[0] & 1) == 1)
     return (PACKET_MULTICAST);
  return (PACKET_OTHERHOST);     /* To someone else */
}

static char get_pkt_type_arc (const arcnet_Header *arc)
{
  ARGSUSED (arc);
  return (PACKET_HOST);
}

static char get_pkt_type (const union link_Packet *pkt)
{
 switch (_pktdevclass)
 {
   case PDCLASS_TOKEN:
        return get_pkt_type_tok (&pkt->tok.head);
   case PDCLASS_FDDI:
        return get_pkt_type_fddi (&pkt->fddi.head);
   case PDCLASS_ARCNET:
        return get_pkt_type_arc (&pkt->arc.head);
   case PDCLASS_ETHER:
        return get_pkt_type_eth (&pkt->eth.head);
   default:
        return (PACKET_HOST);  /* to us (assuming PPP/SLIP) */
  }
}

/**
 * Called from receive.c for AF_PACKET sockets.
 * \todo This should loop until some packet is received.
 */
unsigned sock_packet_receive (Socket *sock, void *buf, unsigned len,
                              struct sockaddr *from, size_t *fromlen)
{
  struct pkt_ringbuf     *q;
  struct sock_packet_buf *rx;

  if (!sock->packet_pool)
     return (0);

  q = &sock->packet_pool->queue;
  if (q->in_index == q->out_index)  /* no elements */
     return (0);

  rx  = (struct sock_packet_buf*) pktq_out_buf (q);
  len = min (len, rx->rx_len);
  memcpy (buf, rx->rx_buf, len);

  if (from && fromlen)  /**\todo should check fromlen is large enough */
  {
    struct sockaddr_ll *sa = (struct sockaddr_ll*) from;
    BYTE   hw_type, hw_len;

    _eth_get_hwtype (&hw_type, &hw_len);
    memset (from, 0, sizeof(*sa));
    sa->sll_halen    = hw_len;
    sa->sll_hatype   = hw_type;
    sa->sll_protocol = *(WORD*) ((char*)buf+_pkt_ip_ofs-2);
    sa->sll_ifindex  = 1;
    sa->sll_family   = AF_PACKET;
    sa->sll_pkttype  = get_pkt_type ((const union link_Packet*)buf);
    *fromlen = sizeof (*sa);
  }
  pktq_inc_out (q);
  return (len);
}

/*
 * Needed by select_s().
 */
unsigned sock_packet_rbused (Socket *sock)
{
  if (!sock->packet_pool)
     return (0);
  return pktq_queued (&sock->packet_pool->queue);
}

unsigned sock_packet_transmit (Socket *socket, const void *buf, unsigned len,
                               const struct sockaddr *to, int tolen)
{
  const struct sockaddr_ll *sa = (const struct sockaddr_ll*) to;
  BYTE *tx;
  int   rc;

  if (len <= _pkt_ip_ofs)
  {
    SOCK_ERRNO (EINVAL);
    return (-1);
  }
  tx = _eth_formatpacket (sa ? &sa->sll_addr : NULL,
                          sa ? sa->sll_protocol : 0);
  tx -= _pkt_ip_ofs;
  len = min (len, sizeof(union link_Packet));
  memcpy (tx, buf, len);
  rc = _eth_send (len - _pkt_ip_ofs, NULL, __FILE__, __LINE__);
  if (rc <= 0)
     return (-1);

  ARGSUSED (socket);
  ARGSUSED (tolen);   /* already checked by caller */
  return (rc + _pkt_ip_ofs);
}


/*
 * Clear 'fd' from the inuse array. For djgpp only:
 *  Free the socket from File-System Extension system.
 *  Free the duplicated handle from DOS's System File Table.
 */
static void sock_close_fd (int fd)
{
  if (fd >= SK_FIRST && fd < MAX_SOCKETS)
  {
    FD_CLR (fd, &inuse[0]);
    if (fd == sk_last-1)
       sk_last--;
  }

#if defined(__DJGPP__) && defined(USE_FSEXT)
  __FSEXT_set_function (fd, NULL);
  if (fd >= SK_FIRST && _close(fd) < 0)
     SOCK_DEBUGF (("  _close() failed; %s", strerror(errno)));
#endif
}

/**
 * Delete the socket from `inuse' array and all memory associated
 * with it. Also unlink it from the socket list (sk_list).
 * Clear _tcp_syn_hook is no more SOCK_STREAM listeners.
 * Return pointer to next node in list or NULL if none/error.
 */
Socket *_sock_del_fd (int fd, const char *file, unsigned line)
{
  Socket    *sock, *next = NULL;
  sock_type *sk;

  SOCK_DEBUGF (("\n  _sock_del_fd:%d", fd));

  if (fd < SK_FIRST || fd >= sk_last || !FD_ISSET(fd,&inuse[0]))
  {
    SOCK_FATAL (("%s (%u) Fatal: socket %d not inuse\n", file, line, fd));
    return (NULL);
  }

  sock = _socklist_find (fd);
  if (!sock)
  {
    SOCK_FATAL (("%s (%u) Fatal: socket %d not in list\n", file, line, fd));
    goto not_inuse;
  }

  if (sock->cookie != SAFETY_TCP)  /* Aaarg! marker destroyed */
  {
    SOCK_FATAL (("%s (%u) fatal: socket %d (%p) overwritten\n",
                 file, line, fd, sock));
    goto not_inuse;
  }

  switch (sock->so_type)
  {
    case SOCK_STREAM:
         sk = (sock_type*) sock->tcp_sock;
         if (sk)
         {
           reuse_localport (sk->tcp.myport); /* clear 'lport_inuse' bit now */
           TCP_ABORT (&sk->tcp);
           _sock_free_rcv_buf (sk);
         }
         DO_FREE (sock->tcp_sock);
         if (!other_tcp_listeners(sock))
            _sock_set_syn_hook (NULL);
         break;

    case SOCK_DGRAM:
         sk = (sock_type*) sock->udp_sock;
         if (sk)
         {
           reuse_localport (sk->udp.myport);  /* redundant? */
           sock_abort (sk);
           _sock_free_rcv_buf (sk);
         }
         DO_FREE (sock->udp_sock);
         break;

    case SOCK_RAW:
         sock_raw_del (sock);
         break;

    case SOCK_PACKET:
         sock_packet_del (sock);
         break;

    default:
         SOCK_DEBUGF (("\n  _sock_del_fd:%d: unknown type %d",
                       fd, sock->so_type));
         break;
  }

  DO_FREE (sock->local_addr);
  DO_FREE (sock->remote_addr);
  DO_FREE (sock->ip_opt);
  DO_FREE (sock->bcast_pool);
  DO_FREE (sock->packet_pool);

  /* Check for any remaining IPv6/PACKET sockets.
   */
  if ((sock->so_family == AF_INET6 || sock->so_type == SOCK_PACKET) &&
      _pkt_rxmode > RXMODE_BROADCAST && /* current Rx-mode not broadcast */
      _pkt_forced_rxmode < 0)           /* we didn't force a specific mode */
     _sock_set_normal_rx_mode (sock);   /* Then set normal Rx-mode */

  next = sk_list_del (fd);  /* delete socket from linked list */

  if (sk_list == NULL)      /* no more sockets, hook not needed */
     _bsd_socket_hook = NULL;

not_inuse:
  sock_close_fd (fd);
  ARGSUSED (file);
  ARGSUSED (line);
  return (next);
}

#ifdef NOT_USED
/**
 * Finds the 'fd' associated with pointer 'socket'.
 * Return -1 if not found.
 */
static int sock_find_fd (const Socket *socket)
{
  const Socket *sock;

  for (sock = sock_hash_get(socket->fd); sock; sock = sock->next)
      if (sock == socket)
         return (sock->fd);
  return (-1);
}

/**
 * Finds the 'Socket' associated with udp-socket 'udp'.
 * Return NULL if not found.
 */
static Socket *sock_find_udp (const _udp_Socket *udp)
{
  Socket *sock;

  for (sock = sk_list; sock; sock = sock->next)
      if (sock->udp_sock == udp)
         return (sock);
  return (NULL);
}
#endif  /* NOT_USED */

/**
 * Finds the 'Socket*' associated with tcp-socket '*tcp'.
 */
static void *sock_find_tcp (const _tcp_Socket *tcp)
{
  Socket *sock;

  for (sock = sk_list; sock; sock = sock->next)
      if (sock->tcp_sock == tcp)
         return (void*)sock;
  return (NULL);
}

/**
 * Check `sockaddr*' passed to bind/connect.
 */
int _sock_chk_sockaddr (Socket *socket, const struct sockaddr *sa, socklen_t len)
{
  socklen_t sa_len = (socket->so_family == AF_INET6) ? sizeof(struct sockaddr_in6) :
                                                       sizeof(struct sockaddr_in);
  if (!sa || len < sa_len)
  {
    SOCK_DEBUGF ((", EINVAL"));
    SOCK_ERRNO (EINVAL);
    return (-1);
  }

  if (socket->so_type == SOCK_PACKET)
  {
    SOCK_DEBUGF ((", SOCK_PACKET, EAFNOSUPPORT"));
    SOCK_ERRNO (EAFNOSUPPORT);
    return (-1);
  }

  /* Easy way to reconnect a UDP socket.
   */
  if (sa->sa_family == AF_UNSPEC && socket->so_type == SOCK_DGRAM)
  {
    socket->so_state &= ~(SS_ISDISCONNECTING | SS_CANTSENDMORE);
    socket->udp_sock->rx_datalen = 0;  /* flush recv buf */
    if (socket->remote_addr)
       free (socket->remote_addr);
    socket->remote_addr = NULL;

    SOCK_DEBUGF ((", AF_UNSPEC, EAFNOSUPPORT"));
    SOCK_ERRNO (EAFNOSUPPORT);
    return (-1);
  }

  if (sa->sa_family != socket->so_family)
  {
    SOCK_DEBUGF ((", different families"));
    SOCK_ERRNO (EAFNOSUPPORT);
    return (-1);
  }

  VERIFY_RW (sa, len);
  return (0);
}

/**
 * Search in sock->raw?_sock list for an unused raw-buffer.
 */
static struct _raw_Socket *find_free_raw_sock (Socket *sock)
{
  struct _raw_Socket *raw;

  if (!sock->raw_sock)
     SOCK_FATAL (("find_raw_sock() called for non-raw sock, so_type %d\n",
                 sock->so_type));

  for (raw = sock->raw_sock; raw; raw = raw->next)
      if (!raw->used)
         return (raw);
  return (NULL);
}

#if defined(USE_IPV6)
static struct _raw6_Socket *find_free_raw6_sock (Socket *sock)
{
  struct _raw6_Socket *raw;

  if (!sock->raw6_sock)
     SOCK_FATAL (("find_raw6_sock() called for non-raw sock, so_type %d\n",
                 sock->so_type));

  for (raw = sock->raw6_sock; raw; raw = raw->next)
      if (!raw->used)
         return (raw);
  return (NULL);
}
#endif

/**
 * Called from _ip4_handler() via `_bsd_socket_hook'.
 * IP-header is already checked in `_ip4_handler()'.
 * Finds all 'Socket' associated with raw IP-packet 'ip'.
 * Enqueue to 'sock->raw_sock'.
 * Return >=1 if 'ip' is consumed, 0 otherwise.
 *
 * \todo Handle receiving bad IP-packets for SOCK_RAW sockets.
 */
static int sock_raw4_recv (const in_Header *ip)
{
  Socket      *sock;
  _raw_Socket *raw;
  int          num_enqueued = 0;
  size_t       len = ntohs (ip->length);

  /* Early drop; jumbo packets won't match any raw-sockets
   */
  if (len > sizeof(sock->raw_sock->rx_data))
     return (0);

  /* Should SOCK_RAW allow packets not destined to us *and* not
   * broadcast? Yes for now.
   */
#if 0
  {
    DWORD ip_dst = ntohl (ip->destination);

    /* Not addressed to us or not (limited) broadcast
     */
    if (!_ip4_is_local_addr(ip_dst) && !_ip4_is_ip_brdcast(ip))
       return (0);
  }
#endif

  for (sock = sk_list; sock; sock = sock->next)
  {
    if (sock->so_type != SOCK_RAW)
       continue;

    if (sock->so_proto == IPPROTO_IP)
       goto enqueue;   /* socket matches every IP-protocol, enqueue */

    if (ip->proto != sock->so_proto)  /* isn't what we want */
       continue;

  enqueue:
    raw = find_free_raw_sock (sock);
    if (!raw)
    {
#if 1 /* Maybe user didn't bother to read it. Ignore it. */
      SOCK_DEBUGF (("\n  socket:%d, dropped raw (IP4), proto %d, id %04X",
                    sock->fd, ip->proto, ntohs(ip->identification)));
#endif
      continue;
    }

    /* Copy IP-header to raw_sock.ip
     */
    memcpy (&raw->ip, ip, sizeof(*ip));

    /* Copy any options and rest of IP-packet
     */
    memcpy (&raw->rx_data[0], ip+1, min(len,sizeof(raw->rx_data)));
    raw->used    = TRUE;
    raw->seq_num = ++raw_seq;
    num_enqueued++;

    if (sock->so_options & SO_DEBUG)
    {
      /*\todo Add SOCK_RAW debug */
    }
  }
  return (num_enqueued);
}

#if defined(USE_IPV6)
static int sock_raw6_recv (const in6_Header *ip)
{
  Socket       *sock;
  _raw6_Socket *raw;
  int           num_enqueued = 0;
  int           num_dropped  = 0;
  const void   *dest         = &ip->destination[0];
  size_t        len          = ntohs (ip->len);

  /* Jumbo packets won't match any raw-sockets
   */
  if (len > sizeof(sock->raw6_sock->rx_data))
     return (0);

  /* Not addressed to us or not multicast
   */
  if (memcmp(&in6addr_my_ip,dest,sizeof(in6addr_my_ip)) &&
      !IN6_IS_ADDR_MULTICAST(dest))
     return (0);

  for (sock = sk_list; sock; sock = sock->next)
  {
    if (ip->next_hdr == IPPROTO_TCP && sock->so_type == SOCK_STREAM)
       continue;

    if (ip->next_hdr == IPPROTO_UDP && sock->so_type == SOCK_DGRAM)
       continue;

    if (sock->so_type != SOCK_RAW ||
        (ip->next_hdr != sock->so_proto && sock->so_proto != IPPROTO_IP))
       continue;

    raw = find_free_raw6_sock (sock);
    if (!raw)
    {
      num_dropped++;
      SOCK_DEBUGF (("\n  socket:%d, dropped raw (IP6), proto %d",
                    sock->fd, ip->next_hdr));
    }
    else
    {
      /* Copy IP-header to raw6_sock.ip
       */
      memcpy (&raw->ip6, ip, sizeof(*ip));

      /* Copy rest of IP-packet (no optios)
       */
      memcpy (&raw->rx_data[0], ip+1, min(len,sizeof(raw->rx_data)));
      raw->used    = TRUE;
      raw->seq_num = ++raw_seq;
      num_enqueued++;

      if (sock->so_options & SO_DEBUG)
      {
        /*\todo Add SOCK_RAW debug */
      }
    }
  }

  if (num_dropped > 0)
     STAT (ip4stats.ips_idropped++);

  return (num_enqueued);
}
#endif  /* USE_IPV6 */

/**
 * Handle keepalive on STREAM socket.
 * Not complete; doesn't consider tcp_max_idle.
 */
static void do_keepalive (Socket *sock)
{
  if (sock->keepalive == 0)
  {
    if (tcp_keep_idle)
       sock->keepalive = set_timeout (1000 * tcp_keep_idle);
  }
  else if (sock->tcp_sock->locflags & LF_KEEPALIVE)
  {
    sock->keepalive = 0UL;
  }
  else if (chk_timeout(sock->keepalive))
  {
    _tcp_keepalive (sock->tcp_sock);
    SOCK_DEBUGF (("\n  sock_keepalive:%d", sock->fd));
    sock->keepalive = 0UL;
  }
}

/**
 * The demultiplexer for callback operations called via '_bsd_socket_hook'
 * from functions in the core Wattcp API.
 */
static void * MS_CDECL socket_op_demux (enum BSD_SOCKET_OPS op, ...)
{
  const _tcp_Socket *tcp;
  void   *rc   = NULL;
  Socket *sock;
  va_list args;

  va_start (args, op);

  switch (op)
  {
    case BSO_FIND_SOCK:  /* return a 'Socket*' from '_tcp_Socket*' */
         tcp = va_arg (args, const _tcp_Socket*);
         if (tcp->ip_type == TCP_PROTO)
              rc = sock_find_tcp (tcp);
         else rc = NULL;
         break;

    case BSO_RST_CALLBACK:  /* RST received in _tcp_handler() */
         tcp  = va_arg (args, const _tcp_Socket*);
         sock = sock_find_tcp (tcp);
         if (sock)
         {
           sock->so_error = ECONNRESET;
           SOCK_DEBUGF (("\n  TCP-reset:%d", sock->fd));
           if (tcp->rx_datalen == 0)
           {
             sock->so_state |= (SS_CONN_REFUSED | SS_CANTSENDMORE | SS_CANTRCVMORE);
             sock->close_time  = time (NULL);
             sock->linger_time = TCP_LINGERTIME/40;  /* 3 sec */
           }
         }
         else
           SOCK_DEBUGF (("\n  TCP-reset for unknown socket??"));
         break;

    case BSO_SYN_CALLBACK:     /* filtering SYNs in _sock_append() */
         if (tcp_syn_hook)
              rc = IntToPtr ((*tcp_syn_hook) (va_arg(args,_tcp_Socket**)));
         else rc = IntToPtr (1);        /* ret-val doesn't matter */
         break;

    case BSO_IP4_RAW:
         if (ip4_raw_hook)
            rc = IntToPtr ((*ip4_raw_hook) (va_arg(args,const in_Header*)));
         break;

    case BSO_IP6_RAW:
#if defined(USE_IPV6)
         if (ip6_raw_hook)
            rc = IntToPtr ((*ip6_raw_hook) (va_arg(args,const in6_Header*)));
#endif
         break;

    case BSO_DEBUG:       /* \todo trace the in/out state etc. */
#if 0
         sk = va_arg (args, const sock_type*);
         if (sk->tcp.ip_type == TCP_PROTO)
         {
           UINT st_in  = va_arg (args, UINT);
           UINT st_out = va_arg (args, UINT);
           (*_printf) ("TCP: %s -> %s\n", st_in, st_out);
         }
         socket_debugdump (sk);
#endif
         break;

    default:
         SOCK_FATAL (("%s (%u) Fatal: Unknown op-code %d\n",
                     __FILE__, __LINE__, op));
         break;
  }
  va_end (args);
  return (rc);
}

/**
 * Called by sock_daemon() to handle SOCK_STREAM sockets.
 *
 * Unlink the socket from the linked list if application has
 * read all data and tcp_state has become CLOSED and the linger
 * period has expired.
 */
static Socket *tcp_sock_daemon (Socket *sock)
{
  Socket      *next = sock->next;
  _tcp_Socket *tcp  = sock->tcp_sock;
  int  s     = sock->fd;
  int  state = tcp->state;

  if (sock->so_options & SO_KEEPALIVE)
     do_keepalive (sock);

  if (state == tcp_StateRESOLVE ||
      state == tcp_StateSYNSENT)        /* opening active tcp session */
  {
    sock->so_state |= SS_ISCONNECTING;
  }
  else if (state == tcp_StateESTAB)        /* established tcp session */
  {
    sock->so_state |=  SS_ISCONNECTED;
    sock->so_state &= ~(SS_ISCONNECTING | SS_ISDISCONNECTING);
  }
  else if (state >= tcp_StateTIMEWT)             /* dying tcp session */
  {
    sock_type *sk = (sock_type*)tcp;
    BOOL closing  = (sock->so_state & (SS_ISDISCONNECTING | SS_CANTSENDMORE));

    sock->so_state &= ~(SS_ISCONNECTED | SS_ISCONNECTING);
    DO_FREE (sock->ip_opt);

    if (sock->close_time && (sock->so_state & SS_CANTRCVMORE))
    {
      /* Flush any remaining Rx data received after shutdown(0) called.
       */
      sock_fastread (sk, NULL, -1);
    }

    if (closing && sk->tcp.ip_type == 0) /* fully closed, refused or aborted */
    {
      BOOL expired = FALSE;

      if (!sock_rbused(sk))
         _sock_free_rcv_buf (sk);   /* free memory not needed anymore */

      if (sock->close_time)         /* close_s() called */
         expired = (time(NULL) - sock->close_time >= (time_t)sock->linger_time);

      /* If linger-period expired and fully closed, delete the TCB
       */
      if (expired && state == tcp_StateCLOSED)
      {
        SOCK_DEBUGF (("\n  tcp_sock_daemon del:%d, lport %d", s, tcp->myport));
        next = SOCK_DEL_FD (s);
      }
    }
  }
  return (next);
}


/**
 * Called by sock_daemon() for SOCK_DGRAM sockets.
 *
 * Unlink the socket from the linked list if application has read
 * all data and "state" is disconnecting.
 *
 * \note Setting 'SS_ISDISCONNECTING' is really a mis-nomer, but
 *       should indicate socket is closed/aborted with Rx-data remaining.
 */
static Socket *udp_sock_daemon (Socket *sock)
{
  Socket      *next = sock->next;
  _udp_Socket *udp  = sock->udp_sock;

  if ((sock->so_state & (SS_ISDISCONNECTING | SS_CANTSENDMORE)) &&
      (udp->rx_datalen == 0 || udp->ip_type == 0))
  {
    SOCK_DEBUGF (("\n  udp_sock_daemon del:%d", sock->fd));
    next = SOCK_DEL_FD (sock->fd);
  }
  return (next);
}

/**
 * A daemon called from tcp_tick().
 *
 * Called every DAEMON_PERIOD (500 msec) to perform cleanup of
 * bound SOCK_STREAM and SOCK_DGRAM sockets still in use.
 */
static void W32_CALL sock_daemon (void)
{
  Socket *sock, *next = NULL;

  /* If we're in a critical region (e.g. connect() or select_s()) where
   * we don't want our socket-list to change, do this later.
   */
  if (sk_block)
     return;

  for (sock = sk_list; sock; sock = next)
  {
    next = sock->next;

    if (!FD_ISSET(sock->fd,&inuse[0]))
       continue;

    if (sock->local_addr == NULL)  /* not bound to anything yet */
       continue;

    switch (sock->so_type)
    {
      case SOCK_STREAM:
           if (sock->tcp_sock)
              next = tcp_sock_daemon (sock);
           break;
      case SOCK_DGRAM:
           if (sock->udp_sock)
              next = udp_sock_daemon (sock);
           break;
    }
  }
}


/**
 * Start a critical region.
 * Prevent `sk_list' being destroyed (e.g. in sock_daemon) and thus
 * confusing select_s(), connect() etc.
 */
void _sock_crit_start (void)
{
  if (sk_block < INT_MAX)
    ++sk_block;
}

/**
 * Mark the end of a critical region.
 * If blocking-level reached zero, we run our socket-daemon.
 */
void _sock_crit_stop (void)
{
  if (sk_block > 0)
  {
    --sk_block;

#ifdef SIGALRM  /** \todo handle SIGALRM raised in a critical-section */
#endif

    if (sk_block == 0)
       sock_daemon();  /* run blocked sock_daemon() */
  }
}


#if defined(USE_FORTIFY) && defined(USE_DEBUG)
/**
 * Exit handler for Fortify.
 * Traverse all sockets and print some state and memory statistics.
 */
static void sock_fortify_exit (void)
{
  const Socket *sock;

  if (!_sock_dbug_active())
     return;

  for (sock = sk_list; sock; sock = sock->next)
  {
    const char *type  = "<?>";
    const void *wsock = NULL;
    const _tcp_Socket *tcp;

    if (sock->cookie != SAFETY_TCP ||
        sock->fd < SK_FIRST        ||
        sock->fd >= sk_last)
    {
      SOCK_DEBUGF (("\nsk_list munged; sock->fd = %d", sock->fd));
      return;
    }

    switch (sock->so_type)
    {
      case SOCK_STREAM:
           type  = "TCP";
           wsock = sock->tcp_sock;
           break;

      case SOCK_DGRAM:
           type  = "UDP";
           wsock = sock->udp_sock;
           break;

      case SOCK_PACKET:
           type  = "Packet";
           wsock = sock->packet_pool;
           break;

      case SOCK_RAW:
#if defined(USE_IPV6)
           if (sock->so_family == AF_INET6)
           {
             type  = "Raw6";
             wsock = sock->raw6_sock;
           }
           else
#endif
           {
             type  = "Raw";
             wsock = sock->raw_sock;
           }
           break;
    }

    SOCK_DEBUGF (("\n%2d: inuse %d, type %s, watt-sock %08lX",
                  sock->fd, FD_ISSET(sock->fd,&inuse[0]) ? 1 : 0,
                  type, (DWORD_PTR)wsock));

    tcp = sock->tcp_sock;
    if (tcp)
    {
      if (sock->so_state & SS_ISDISCONNECTING)
           SOCK_DEBUGF ((" (closed"));
      else if (sock->so_state & SS_ISLISTENING)
           SOCK_DEBUGF ((" (listening"));
      else SOCK_DEBUGF ((" (aborted?"));
      SOCK_DEBUGF ((", %s)", tcpStateName(tcp->state)));
    }
  }

  SOCK_DEBUGF ((" \n"));
  Fortify_OutputStatistics();
  Fortify_LeaveScope();
}
#endif /* USE_FORTIFY && USE_DEBUG */


#if defined(USE_DEBUG) && defined(ERRNO_VENDOR_VERSION)
/**
 * Return version string of current compiler.
 */
static const char *vendor_version (const char **vendor)
{
  static char buf[10];
  const char *v = NULL;

#if defined(__DJGPP__)
  sprintf (buf, "%d.%02d", __DJGPP__, __DJGPP_MINOR__);
  v = "__DJGPP__";

#elif defined(__DMC__)
  sprintf (buf, "%X.%X", __DMC__ >> 8, __DMC__ & 0xFF);
  v = "__DMC__";

#elif defined(__WATCOMC__)
  sprintf (buf, "%d.%d", __WATCOMC__/100, __WATCOMC__ % 100);
  v = "__WATCOMC__";

#elif defined(__BORLANDC__)
  sprintf (buf, "%X.%X", __BORLANDC__ >> 8, __BORLANDC__ & 0xFF);
  v = "__BORLANDC__";

#elif defined(__TURBOC__)
  sprintf (buf, "%X.%X", (__TURBOC__ >> 8) - 1, __TURBOC__ & 0xFF);
  v = "__TURBOC__";

#elif defined(__clang__)
  sprintf (buf, "%d.%d", __clang_major__, __clang_minor__);
  v = "__clang__";

#elif defined(_MSC_VER)
  sprintf (buf, "%d.%d", _MSC_VER/100, _MSC_VER % 100);
  v = "_MSC_VER";

#elif defined(__MINGW32__) && defined(__MINGW64_VERSION_MAJOR)
  /* __MINGW64_VERSION_[MAJOR|MINOR] is defined through _mingw.h in MinGW-W64 */
  sprintf (buf, "%d.%d", __MINGW64_VERSION_MAJOR, __MINGW64_VERSION_MINOR);
  v = "__MINGW64__";

#elif defined(__MINGW32__)       /* mingw.org MinGW. MingW-RT-4+ defines '__MINGW_MAJOR_VERSION' */
  #if defined(__MINGW_MAJOR_VERSION)
    sprintf (buf, "%d.%d", __MINGW_MAJOR_VERSION, __MINGW_MINOR_VERSION);
  #else
    sprintf (buf, "%d.%d", __MINGW32_MAJOR_VERSION, __MINGW32_MINOR_VERSION);
  #endif
  v = "__MINGW32__";

#elif defined(__CYGWIN__)   /* this should be dead code for CygWin */
  sprintf (buf, "%d.%d", CYGWIN_VERSION_DLL_MAJOR, CYGWIN_VERSION_DLL_MINOR);
  v = "__CYGWIN__";

#elif defined(__CCDL__)
  sprintf (buf, "%d.%d", __CCDL__/100, __CCDL__ % 100);
  v = "__CCDL__";

#else
  buf[0] = '\0';
#endif

  if (vendor)
     *vendor = v;
  return (buf);
}

/**
 * Check that ./util/xx_err.exe is built with same version
 * we're using here. Print a warning if there's a mismatch.
 */
static void check_errno_version (void)
{
  const char *ven, *ver = vendor_version (&ven);

  if (*ver && strcmp(ver,ERRNO_VENDOR_VERSION))
  {
    (*_printf) ("\nWarning: %%WATT_ROOT%%\\inc\\sys\\*.err was created "
                "with a different compiler\nversion (%s vs %s, %s). "
                "Rebuilt %%WATT_ROOT%%\\util\\*_err.exe and "
                "Watt-32 library to avoid subtle bugs.\n",
                ERRNO_VENDOR_VERSION, ver, ven);
    BEEP();
  }
}
#endif  /* USE_DEBUG && ERRNO_VENDOR_VERSION */


/**
 * Main initialisation routine for the BSD socket API.
 * Called only once to do:
 *  - Calls watt_sock_init() w/o exiting if it fails.
 *  - Checks the generated errno list.
 *  - Set the limit on max number of sockets.
 *  - Add our sock_daemon().
 *  - Initialise Fortify malloc debugger.
 */
static BOOL init_sockets (void)
{
  static int sk_result = -1;

  if (sk_result >= 0)
     return (sk_result);

  /* Hack to make linker pull in our strerror().
   */
  pull_neterr_module = 0;

  _watt_do_exit = 0;
  sk_result = (watt_sock_init(0,0,0) == 0);
  if (!sk_result)
     return (FALSE);

#if defined(USE_DEBUG) && defined(ERRNO_VENDOR_VERSION)
  check_errno_version();
#endif

#ifdef __DJGPP__
  {
    struct rlimit r;

    getrlimit (RLIMIT_NOFILE, &r);
    r.rlim_max = MAX_SOCKETS;     /* We don't know this before we try it */
    setrlimit (RLIMIT_NOFILE, &r);
  }
  sk_last = MAX_SOCKETS;
#else
  sk_last = SK_FIRST+1;
#endif  /* __DJGPP__ */

  sk_list = NULL;
  memset (&inuse[0], 0, sizeof(inuse));
  memset (&sk_hashes, 0, sizeof(sk_hashes));

  DAEMON_ADD (sock_daemon);

#if defined(USE_FORTIFY) && defined(USE_DEBUG)
  Fortify_EnterScope();
  Fortify_SetOutputFunc (bsd_fortify_print);
  RUNDOWN_ADD (sock_fortify_exit, 140);
#endif
  return (TRUE);
}

/**
 * Returns a pointer to the Socket structure associated with socket 's'.
 * If socket `s' was not found, NULL is returned.
 */
Socket *_socklist_find (int s)
{
  Socket *sock;

  if (!init_sockets())
     return (NULL);

  for (sock = sock_hash_get(s); sock; sock = sock->next)
      if (sock->fd == s)
         return (sock);
  return (NULL);
}

/**
 * Allocate raw?_Socket buffers for a SOCK_RAW socket.
 */
static BOOL sock_raw_add (Socket *sock, int family)
{
  int i;

#if defined(USE_IPV6)
  if (family == AF_INET6)
  {
    for (i = 0; i < MAX_RAW6_BUFS; i++)
    {
      _raw6_Socket *raw6 = SOCK_CALLOC (sizeof(*raw6));

      if (!raw6)
      {
        if (i == 0)        /* Failing to allocate 1 buffer is fatal */
           return (FALSE);

        if (i > 0)
           break;
      }
      raw6->ip_type   = IP6_TYPE;
      raw6->next      = sock->raw6_sock;
      sock->raw6_sock = raw6;
    }
    return (TRUE);
  }
#endif
  if (family == AF_INET)
  {
    for (i = 0; i < MAX_RAW_BUFS; i++)
    {
      _raw_Socket *raw = SOCK_CALLOC (sizeof(*raw));

      if (!raw)
      {
        if (i == 0)
           return (FALSE);

        if (i > 0)
           break;
      }
      raw->ip_type   = IP4_TYPE;
      raw->next      = sock->raw_sock;
      sock->raw_sock = raw;
    }
    return (TRUE);
  }
  return (FALSE);
}

/**
 * Adds a new socket to the sk_list.
 */
static Socket *socklist_add (int s, int type, int proto, int family)
{
  Socket *sock = SOCK_CALLOC (sizeof(*sock));
  void   *proto_sk;
  BOOL    okay;

  if (!sock)
     return (NULL);

  switch (proto)
  {
    case IPPROTO_TCP:
         /* Only tcp times out on inactivity
          */
         sock->timeout     = sock_delay;
         sock->linger_time = TCP_LINGERTIME;
         sock->tcp_sock    = SOCK_CALLOC (sizeof(*sock->tcp_sock));
         proto_sk          = sock->tcp_sock;
         if (!sock->tcp_sock)
            goto fail;
         break;

    case IPPROTO_UDP:
         sock->udp_sock = SOCK_CALLOC (sizeof(*sock->udp_sock));
         proto_sk       = sock->udp_sock;
         if (!sock->udp_sock)
            goto fail;
         break;

    default:  /* type == SOCK_RAW/SOCK_PACKET */
         proto_sk = NULL;
         if (type == SOCK_PACKET)
              okay = sock_packet_add (sock);
         else okay = sock_raw_add (sock, family);
         if (!okay)
            goto fail;
         break;
  }

#if defined(__DJGPP__) && defined(USE_FSEXT)
  if (!__FSEXT_set_data(s,sock))
  {
    SOCK_FATAL (("%s (%d) Fatal: cannot grow FSEXT table\n",
                __FILE__, __LINE__));
    goto fail;
  }
#else
  ARGSUSED (proto_sk);
#endif

  /* Link 'sock' into the 'sk_list'
   */
  sock->next = sk_list;
  sk_list    = sock;
  sock_hash_add (s, sock);

  sock->fd         = s;
  sock->so_type    = type;
  sock->so_proto   = proto;
  sock->so_family  = family;
  sock->so_state   = SS_UNCONNECTED;
  sock->send_lowat = DEFAULT_SEND_LOWAT;
  sock->recv_lowat = DEFAULT_RECV_LOWAT;
  sock->ip_ttl     = IPDEFTTL;
  sock->ip_tos     = 0;
  sock->cookie     = SAFETY_TCP;
  return (sock);

fail:
  if (proto_sk)
     free (proto_sk);
  else if (type == SOCK_PACKET)
    sock_packet_del (sock);
  else
    sock_raw_del (sock);

  free (sock);
  return (NULL);
}

/**
 * Select (and check) a suitable protocol for socket-type.
 */
static int set_proto (int type, BOOL is_ip6, int *proto)
{
#if defined(USE_IPV6)
  if (is_ip6 && !_sock_set_mcast_rx_mode())
  {
    SOCK_ERRNO (ENETDOWN);
    SOCK_DEBUGF (("\nsocket: ENETDOWN; %s", pkt_strerror(_pkt_errno)));
    return (-1);
  }
#endif

  if (type == SOCK_STREAM)
  {
    if (*proto == 0)
        *proto = IPPROTO_TCP;

    else if (*proto != IPPROTO_TCP)  /**< \todo IPPROTO_XTP */
    {
      SOCK_ERRNO (EPROTONOSUPPORT);
      SOCK_DEBUGF (("\nsocket: invalid STREAM protocol (%d)", *proto));
      return (-1);
    }
  }
  else if (type == SOCK_DGRAM)
  {
    if (*proto == 0)
        *proto = IPPROTO_UDP;

    else if (*proto != IPPROTO_UDP)
    {
      SOCK_ERRNO (EPROTONOSUPPORT);
      SOCK_DEBUGF (("\nsocket: invalid DGRAM protocol (%d)", *proto));
      return (-1);
    }
  }
  else if (type == SOCK_RAW)
  {
    if (*proto == IPPROTO_RAW)          /* match all IP-protocols */
        *proto = IPPROTO_IP;

#if defined(USE_IPV6)
    if (is_ip6)
        ip6_raw_hook = sock_raw6_recv; /* hook for _ip6_handler() */
    else
#endif
        ip4_raw_hook = sock_raw4_recv; /* hook for _ip4_handler() */
  }
  else if (type == SOCK_PACKET)
  {
    *proto = 0;
    if (!_sock_set_promisc_rx_mode())
    {
      SOCK_ERRNO (ENETDOWN);
      SOCK_DEBUGF (("\nsocket: ENETDOWN; %s", pkt_strerror(_pkt_errno)));
      return (-1);
    }
  }
  ARGSUSED (is_ip6);
  return (0);
}

/*
 * Return a friendly name for 'proto'.
 */
#if defined(USE_DEBUG)
static const char *proto_name (int proto)
{
  static const struct search_list proto_names[] = {
                  { IPPROTO_IP,     "IPPROTO_IP"     },
                  { IPPROTO_ICMP,   "IPPROTO_ICMP"   },
                  { IPPROTO_IGMP,   "IPPROTO_IGMP"   },
                  { IPPROTO_GGP,    "IPPROTO_GGP"    },
                  { IPPROTO_IPIP,   "IPPROTO_IPIP"   },
                  { IPPROTO_TCP,    "IPPROTO_TCP"    },
                  { IPPROTO_EGP,    "IPPROTO_EGP"    },
                  { IPPROTO_IGRP,   "IPPROTO_IGRP"   },
                  { IPPROTO_PUP,    "IPPROTO_PUP"    },
                  { IPPROTO_UDP,    "IPPROTO_UDP"    },
                  { IPPROTO_IDP,    "IPPROTO_IDP"    },
                  { IPPROTO_TP,     "IPPROTO_TP"     },
                  { IPPROTO_XTP,    "IPPROTO_XTP"    },
                  { IPPROTO_RSVP,   "IPPROTO_RSVP"   },
                  { IPPROTO_ESP,    "IPPROTO_ESP"    },
                  { IPPROTO_AH,     "IPPROTO_AH"     },
                  { IPPROTO_ICMPV6, "IPPROTO_ICMPV6" },
                  { IPPROTO_NONE,   "IPPROTO_NONE"   },
                  { IPPROTO_EON,    "IPPROTO_EON"    },
                  { IPPROTO_ENCAP,  "IPPROTO_ENCAP"  },
                  { IPPROTO_PIM,    "IPPROTO_PIM"    },
                  { IPPROTO_VRRP,   "IPPROTO_VRRP"   },
                  { IPPROTO_SCTP,   "IPPROTO_SCTP"   },
                  { IPPROTO_DIVERT, "IPPROTO_DIVERT" },
                  { IPPROTO_RAW,    "IPPROTO_RAW"    },
                  { IPPROTO_MAX,    "IPPROTO_MAX"    },
                };
  return list_lookup (proto, proto_names, DIM(proto_names));
}
#endif

/**
 * socket().
 *  \arg family   The protocol family. Supports the AF_INET, AF_INET6,
 *                AF_PACKET families.
 *  \arg type     SOCK_STREAM (tcp), SOCK_DGRAM (udp), SOCK_RAW (ip) or
 *                SOCK_PACKET (link).
 *  \arg protocol IPPROTO_TCP, IPPROTO_UDP or 0/IPPROTO_IP.
 *
 *  \retval The socket ID number
 */
int W32_CALL socket (int family, int type, int protocol)
{
  Socket     *sock = NULL;
  char       *err  = NULL;
  const char *fam;
  int         s;
  BOOL        is_ip6;

  if (!init_sockets())
  {
    SOCK_ERRNO (ENETDOWN);
    return (-1);
  }

  if (family != AF_UNSPEC &&
      family != AF_INET   &&
      family != AF_PACKET
#if defined(USE_IPV6)
   && family != AF_INET6
#endif
     )
  {
    SOCK_DEBUGF (("\nsocket: invalid family (%d)", family));
    SOCK_ERRNO (EAFNOSUPPORT);
    return (-1);
  }

  if (type != SOCK_STREAM &&
      type != SOCK_DGRAM  &&
      type != SOCK_PACKET &&
      type != SOCK_RAW)
  {
    SOCK_DEBUGF (("\nsocket: invalid type (%d)", type));
    SOCK_ERRNO (ESOCKTNOSUPPORT);
    return (-1);
  }

  if (family == AF_PACKET)
  {
    type   = SOCK_PACKET;
    family = AF_UNSPEC;   /* !! is this correct? */
    /* protocol ignored; could be ETH_P_ALL */
  }

  if (type == SOCK_RAW && (protocol < IPPROTO_IP || protocol >= IPPROTO_MAX))
  {
    SOCK_DEBUGF (("\nsocket: invalid SOCK_RAW proto (%d)", protocol));
    SOCK_ERRNO (EINVAL);
    return (-1);
  }

  is_ip6 = (family == AF_INET6);
  fam    = (is_ip6 ? "6" : "");

  if (set_proto(type, is_ip6, &protocol) < 0)
     return (-1);

  s = sock_alloc_fd (&err);
  if (s >= 0)
     sock = socklist_add (s, type, protocol, family);

  switch (type)
  {
    case SOCK_STREAM:
         SOCK_DEBUGF (("\nsocket: fam:AF_INET%s type:STREAM, proto %s, %d",
                       fam, proto_name(protocol), s));
         break;
    case SOCK_DGRAM:
         SOCK_DEBUGF (("\nsocket: fam:AF_INET%s type:DGRAM, proto %s, %d",
                       fam, proto_name(protocol), s));
         break;
    case SOCK_RAW:
         SOCK_DEBUGF (("\nsocket: fam:AF_INET%s type:RAW, proto %s, %d",
                       fam, proto_name(protocol), s));
         break;
    case SOCK_PACKET:
         SOCK_DEBUGF (("\nsocket: fam:AF_UNSPEC type:PACKET, %d", s));
         break;
  }

  if (err)     /* sock_alloc_fd() failed */
  {
    SOCK_DEBUGF ((", EMFILE; %s", err));
    SOCK_ERRNO (EMFILE);
    return (-1);
  }

  if (!sock)   /* socklist_add() failed */
  {
    if (s >= 0)
       sock_close_fd (s);
    SOCK_DEBUGF ((", ENOMEM"));
    SOCK_ERRNO (ENOMEM);
    return (-1);
  }

  if (type == SOCK_PACKET)
  {
    sock->old_eth_peek = _eth_recv_peek;
    _eth_recv_peek = (int (W32_CALL*)(void*)) sock_packet_peek;
  }

  _bsd_socket_hook = socket_op_demux;
  ARGSUSED (fam);
  return (s);
}

/**
 * Callback handlers for "ICMP Port/Host/Network Unreachable" or
 * "ICMP Parameter Problem" issued by lower layer (udp_cancel()
 * and tcp_cancel() in pctcp.c)
 *
 * \note a single ICMP message may apply to several UDP sockets.
 *
 * We should drop (i.e. set ECONNREFUSED in lack of a better errno) on
 * receiving "ICMP-param problem" or "ICMP-unreachable" reached
 * stress-level.
 */
static int stream_cancel (const _tcp_Socket *tcp, BYTE icmp_type, int error)
{
  Socket *socket;
  BOOL    should_drop = (icmp_type == ICMP_UNREACH &&
                         tcp->stress >= tcp->rigid) ||
                        (tcp->ip_type == 0);   /* already aborted */

  for (socket = sk_list; socket; socket = socket->next)
      if (socket->so_type == SOCK_STREAM &&
          tcp == socket->tcp_sock && should_drop)
      {
        socket->so_state |= SS_CONN_REFUSED;
        socket->so_error  = error;
        return (1);
      }
  return (0);
}

static int dgram_cancel (const _udp_Socket *udp, BYTE icmp_type, int error)
{
  Socket *socket;
  int     num = 0;

  for (socket = sk_list; socket; socket = socket->next)
      if (socket->so_type == SOCK_DGRAM && udp == socket->udp_sock)
      {
        socket->so_state |= SS_CONN_REFUSED;
        socket->so_error  = error;
        num++;
      }
  ARGSUSED (icmp_type);
  return (num);
}

#if defined(USE_DEBUG)
static __inline const char *ip_proto_str (WORD prot)
{
  switch (prot)
  {
    case UDP_PROTO:
         return ("UDP");
    case TCP_PROTO:
         return ("TCP");
    case ICMP_PROTO:
         return ("ICMP");
    case IGMP_PROTO:
         return ("IGMP");
    default:
         return ("unknown");
  }
}
#endif

/*
 * Handler for lower layer ICMP messages/errors.
 * Return number of BSD-sockets the ICMP-event matched
 * (retval not used yet).
 */
static int icmp_callback (sock_type *s, BYTE icmp_type, BYTE icmp_code)
{
  const char *type_str = "??";
  int   error;

  if (icmp_type == ICMP_UNREACH)
       error = (icmp_code == ICMP_UNREACH_NET ||
                icmp_code == ICMP_UNREACH_NET_UNKNOWN ||
                icmp_code == ICMP_UNREACH_NET_PROHIB) ?
               ENETUNREACH : EHOSTUNREACH;
  else error = ECONNREFUSED;

  if (icmp_type < DIM(icmp_type_str))
     type_str = icmp_type_str [icmp_type];

  SOCK_DEBUGF (("\n  icmp_callback (s=%" ADDR_FMT ", IP-proto %s, %s, code %d)",
                ADDR_CAST(s), ip_proto_str(s->udp.ip_type), type_str, icmp_code));

  if (icmp_type == ICMP_UNREACH || icmp_type == ICMP_PARAMPROB)
  {
    if (s->udp.ip_type == UDP_PROTO)
       return dgram_cancel (&s->udp, icmp_type, error);

    if (s->udp.ip_type == TCP_PROTO)
       return stream_cancel (&s->tcp, icmp_type, error);
  }
  ARGSUSED (type_str);
  return (0);
}

/**
 * Open and listen routines for SOCK_DGRAM at the socket-level
 */
int _UDP_open (Socket *socket, struct in_addr host, WORD loc_port, WORD rem_port)
{
  DWORD ip = ntohl (host.s_addr);

  loc_port = ntohs (loc_port);
  rem_port = ntohs (rem_port);

  if (!udp_open (socket->udp_sock, loc_port, ip, rem_port, NULL))
     return (0);

  _sock_set_rcv_buf ((sock_type*)socket->udp_sock, DEFAULT_UDP_SIZE);
  socket->udp_sock->icmp_callb = (icmp_upcall) icmp_callback;
  return (1);
}

int _UDP_listen (Socket *socket, struct in_addr host, WORD port)
{
  _udp_Socket *udp = socket->udp_sock;
  DWORD        addr;

  port = ntohs (port);

  if (socket->so_state & SS_PRIV)
  {
    int   size = sizeof(recv_buf) * MAX_DGRAMS;
    void *pool = malloc (size);

    if (!pool)
    {
      SOCK_FATAL (("%s (%d) Fatal: Allocation failed\n",
                  __FILE__, __LINE__));
      SOCK_ERRNO (ENOMEM);
      return (-1);
    }
    socket->bcast_pool = (recv_buf**) pool;
    socket->pool_size  = size;

    /* Mapping `INADDR_ANY' to `INADDR_BROADCAST' causes udp_demux()
     * to demux to the correct watt-socket; s->hisaddr = IP_BCAST_ADDR in
     * passive socket demux loop.
     */
    if (host.s_addr == INADDR_ANY)
         addr = INADDR_BROADCAST;
    else addr = ntohl (host.s_addr);

    udp_listen (udp, port, addr, 0, NULL);

    /* Setup _recvdaemon() to enqueue broadcast/"unconnected" messages
     */
    sock_recv_init ((sock_type*)udp, pool, size);
  }
  else
  {
    addr = ntohl (host.s_addr);
    udp_listen (udp, port, addr, 0, NULL);
  }
  udp->icmp_callb = (icmp_upcall) icmp_callback;
  return (1);
}


/**
 * Open and listen routines for SOCK_STREAM at the socket-level
 */
int _TCP_open (Socket *socket, struct in_addr host, WORD loc_port, WORD rem_port)
{
  DWORD dest = ntohl (host.s_addr);

  loc_port = ntohs (loc_port);
  rem_port = ntohs (rem_port);

  if (!tcp_open (socket->tcp_sock, loc_port, dest, rem_port, NULL))
     return (0);

  if (socket->so_state & SS_NBIO)
     socket->tcp_sock->locflags |= LF_NOCLOSE;

  /**
   * The parameters to tcp_open() is a bit tricky, but the internal Wattcp
   * socket 's' contains the following elements that must match in the
   * first 'for-loop' of _tcp_handler().
   *
   * s->hisport != 0                i.e. active (non-listening) port
   * s->myaddr  == ip->destination, our IP-address
   * s->hisaddr == ip->source,      above 'dest' address
   * s->myport  == tcp->dstPort,    above 'loc_port'
   * s->hisport == tcp->srcPort,    above 'rem_port'
   */

  /* Advertise a large rcv-win from the next ACK on.
   */
  _sock_set_rcv_buf ((sock_type*)socket->tcp_sock, tcp_recv_win);
  socket->tcp_sock->icmp_callb = (icmp_upcall) icmp_callback;
  return (1);
}

int _TCP_listen (Socket *socket, struct in_addr host, WORD port)
{
  DWORD addr     = ntohl (host.s_addr);
  WORD  loc_port = ntohs (port);

  tcp_listen (socket->tcp_sock, loc_port, addr, 0, NULL, 0);
  socket->tcp_sock->icmp_callb = (icmp_upcall) icmp_callback;
  return (1);
}

#if defined(USE_IPV6)
int _TCP6_open (Socket *socket, const void *dst, WORD loc_port, WORD rem_port)
{
  _tcp_Socket *tcp = socket->tcp_sock;
  UINT         rtt;

  SIO_TRACE (("_TCP6_open"));

  WATT_LARGE_CHECK (tcp, sizeof(*tcp));
  _tcp_unthread (tcp, FALSE);    /* just in case not totally closed */

  memset (tcp, 0, sizeof(*tcp));
  tcp->state = tcp_StateCLOSED;  /* otherwise is tcp_StateLISTEN on failure */

  if (!_eth_is_init) /* GvB 2002-09, Lets us survive without a working eth */
  {
    SOCK_ERRNO (ENETDOWN);
    tcp->err_msg = _LANG (_eth_not_init);
    return (0);
  }

  if (IN6_ARE_ADDR_EQUAL(dst,&in6addr_any) || IN6_IS_ADDR_MULTICAST(dst))
  {
    SOCK_ERRNO (EINVAL);
    sprintf (tcp->err_buf, _LANG("Illegal destination (%s)"), _inet6_ntoa(dst));
    tcp->err_msg = tcp->err_buf;
    return (0);
  }

  if (!icmp6_neigh_solic (dst, &tcp->his_ethaddr))  /* This blocks! */
  {
    SOCK_ERRNO (EHOSTUNREACH);
    strcpy (tcp->err_buf, _LANG("Neighbor Solic failed "));
    strcat (tcp->err_buf, _inet6_ntoa(dst));
    tcp->err_msg = tcp->err_buf;
    STAT (ip6stats.ip6s_noroute++);
    return (0);
  }

  _sock_set_rcv_buf ((sock_type*)tcp, tcp_recv_win);

  memcpy (&tcp->my6addr, &in6addr_my_ip, sizeof(tcp->my6addr));
  memcpy (&tcp->his6addr, dst, sizeof(tcp->his6addr));

  loc_port = ntohs (loc_port);
  rem_port = ntohs (rem_port);

  tcp->is_ip6       = TRUE;
  tcp->icmp_callb   = (icmp_upcall) icmp_callback;
  tcp->state        = tcp_StateSYNSENT;
  tcp->ip_type      = TCP_PROTO;
  tcp->max_seg      = _mss;        /** \todo use mss from setsockopt() */
  tcp->cwindow      = 1;
  tcp->wwindow      = 0;                           /* slow start VJ algorithm */
  tcp->vj_sa        = INIT_VJSA;
  tcp->rto          = tcp_OPEN_TO;                 /* added 14-Dec 1999, GV */
  tcp->myaddr       = my_ip_addr;
  tcp->myport       = find_free_port (loc_port,1); /* get a nonzero port val */
  tcp->locflags     = LF_LINGER;                   /* close via TIMEWT state */
  tcp->ttl          = _default_ttl;
  tcp->hisport      = rem_port;
  tcp->send_next    = INIT_SEQ();
  tcp->flags        = tcp_FlagSYN;
  tcp->unhappy      = TRUE;
  tcp->protoHandler = NULL;
  tcp->usr_yield    = NULL;

  tcp->safetysig    = SAFETY_TCP;                /* marker signatures */
  tcp->safetytcp    = SAFETY_TCP;
  tcp->next         = _tcp_allsocs;              /* insert into chain */

/* sock_yield (tcp, tcp6_yield); */ /**< \todo Yield for IPv6 sockets */

  _tcp_allsocs    = tcp;

  /** \todo use TCP_NODELAY set in setsockopt()
   */
  if (tcp_nagle)
     tcp->sockmode = SOCK_MODE_NAGLE;
  tcp->sockmode |= SOCK_MODE_BINARY;

  tcp->timeout = set_timeout (tcp_LONGTIMEOUT);

  TCP_SEND (tcp);  /* send opening SYN */

  /* find previous RTT replacing RTT set in tcp_send() above
   */
  if (tcp_rtt_get(tcp, &rtt, NULL))
       tcp->rtt_time = set_timeout (rtt);
  else tcp->rtt_time = set_timeout (tcp_OPEN_TO);
  return (1);
}

int _TCP6_listen (Socket *socket, const void *host, WORD loc_port)
{
  /** \todo Support TCP listen() for AF_INET6 */
  ARGSUSED (socket);
  ARGSUSED (host);
  ARGSUSED (loc_port);
  return (0);
}

int _UDP6_open (Socket *socket, const void *host, WORD loc_port, WORD rem_port)
{
  /** \todo Support UDP connect() for AF_INET6 */
  _sock_set_rcv_buf ((sock_type*)socket->udp_sock, DEFAULT_UDP_SIZE);
  socket->udp_sock->icmp_callb = (icmp_upcall) icmp_callback;
  ARGSUSED (host);
  ARGSUSED (loc_port);
  ARGSUSED (rem_port);
  return (0);
}

int _UDP6_listen (Socket *socket, const void *host, WORD port)
{
  /** \todo Support UDP listen() for AF_INET6 */
  ARGSUSED (socket);
  ARGSUSED (host);
  ARGSUSED (port);
  return (0);
}
#endif  /* USE_IPV6 */


#ifdef NOT_USED
/**
 * _sock_half_open -
 *   Return true if peer closed his side.
 *   There might still be data to read
 */
int _sock_half_open (const _tcp_Socket *s)
{
  if (!s || s->ip_type != TCP_PROTO)
     return (0);

  return (s->state >= tcp_StateFINWT1 &&
          s->state <= tcp_StateCLOSED);
}
#endif

#if 0  /* not finished and seldom needed */
/**
 * socketpair() - Create a pair of connected sockets.
 * Modified version based on Linux's version.
 */
int W32_CALL socketpair (int family, int type, int protocol, int usockvec[2])
{
  Socket *sock1, *sock2;
  int     s1, s2;

  if ((s1 = socket (family, type, protocol)) < 0)
     return (-errno);

  sock1 = socklist_find (s1);

  /* Now grab another socket and try to connect the two together.
   */
  if ((s2 = socket (family, type, protocol)) < 0)
  {
    int err = -errno;

    close_s (s1);
    return (err);
  }

  sock2 = socklist_find (s2);

  sock1->conn = sock2;
  sock2->conn = sock1;
  sock1->so_state = SS_CONNECTED;
  sock2->so_state = SS_CONNECTED;

  usockvec[0] = s1;
  usockvec[1] = s2;
  return (0);
}
#endif

#endif /* USE_BSD_API */
