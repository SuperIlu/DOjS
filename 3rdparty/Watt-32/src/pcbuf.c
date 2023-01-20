/*!\file pcbuf.c
 *
 * Status of Rx/tx buffers.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "copyrigh.h"
#include "wattcp.h"
#include "language.h"
#include "misc.h"
#include "misc_str.h"
#include "pctcp.h"
#include "pcdbug.h"
#include "pcbuf.h"

#if defined(USE_BSD_API)
static int num_free_raw (const _raw_Socket *raw)
{
  int num;

  for (num = 0; raw; raw = raw->next)
      if (!raw->used)
         num++;
  return (num);
}
#endif

#if defined(USE_IPV6)
static int num_free_raw6 (const _raw6_Socket *raw)
{
  int num;

  for (num = 0; raw; raw = raw->next)
      if (!raw->used)
         num++;
  return (num);
}
#endif


size_t W32_CALL sock_rbsize (const sock_type *s)
{
  switch (_chk_socket(s))
  {
    case VALID_IP4:
         return (sizeof(s->raw.ip) + sizeof(s->raw.rx_data));
#if defined(USE_IPV6)
    case VALID_IP6:
         return (sizeof(s->raw6.ip6) + sizeof(s->raw6.rx_data));
#endif
    case VALID_UDP:
         return (s->udp.max_rx_data);
    case VALID_TCP:
         return (s->tcp.max_rx_data);
  }
  return (0);
}

size_t W32_CALL sock_rbused (const sock_type *s)
{
  switch (_chk_socket(s))
  {
#if defined(USE_BSD_API)
    case VALID_IP4:
         {
           const _raw_Socket *raw = find_oldest_raw (&s->raw);
           return (raw ? intel16 (raw->ip.length) : 0);
         }
#if defined(USE_IPV6)
    case VALID_IP6:
         {
           const _raw6_Socket *raw = find_oldest_raw6 (&s->raw6);
           return (raw ? intel16 (raw->ip6.len) : 0);
         }
#endif
#endif
    case VALID_UDP:
         return (s->udp.rx_datalen);
    case VALID_TCP:
         return (s->tcp.rx_datalen);
  }
  return (0);
}

size_t W32_CALL sock_rbleft (const sock_type *s)
{
  switch (_chk_socket(s))
  {
#if defined(USE_BSD_API)
    case VALID_IP4:
         return num_free_raw (&s->raw) *
                  (sizeof(s->raw.ip) + sizeof(s->raw.rx_data));
#if defined(USE_IPV6)
    case VALID_IP6:
         return num_free_raw6 (&s->raw6) *
                  (sizeof(s->raw6.ip6) + sizeof(s->raw6.rx_data));
#endif
#endif
    case VALID_UDP:
         return (s->tcp.max_rx_data - s->udp.rx_datalen);
    case VALID_TCP:
         return (s->tcp.max_rx_data - s->tcp.rx_datalen);
  }
  return (0);
}

size_t W32_CALL sock_tbsize (const sock_type *s)
{
  switch (_chk_socket(s))
  {
    case VALID_IP4:
    case VALID_IP6:
         return (_mtu);
    case VALID_TCP:
         return (s->tcp.max_tx_data);
    case VALID_UDP:
         return (_mtu - UDP_OVERHEAD);
  }
  return (0);
}

size_t W32_CALL sock_tbleft (const sock_type *s)
{
  switch (_chk_socket(s))
  {
    case VALID_IP4:
    case VALID_IP6:
         return (_mtu);
    case VALID_TCP:
         return (s->tcp.max_tx_data - s->tcp.tx_datalen - 1);
    case VALID_UDP:
         return (_mtu - UDP_OVERHEAD);
  }
  return (0);
}

size_t W32_CALL sock_tbused (const sock_type *s)
{
  if (_chk_socket(s) == VALID_TCP)
     return (s->tcp.tx_datalen);
  return (0);
}

/*
 * Sets new buffer for Rx-data (for udp/tcp).
 * Set debug-marker in front and at end of buffer.
 * \note does not copy the old content over to new buffer.
 */
size_t W32_CALL sock_setbuf (sock_type *s, BYTE *rx_buf, size_t rx_len)
{
  int type = _chk_socket (s);

  /* Raw-sockets use fixed buffer
   */
  if (!type || type == VALID_IP4 || type == VALID_IP6)
     return (0);

  if (rx_len < 8 || !rx_buf)
  {
    s->tcp.rx_data     = &s->tcp.rx_buf[0];
    s->tcp.max_rx_data = sizeof(s->tcp.rx_buf) - 1;
  }
  else
  {
    size_t len = min (rx_len, USHRT_MAX-1) - 8;

    *(DWORD*)rx_buf         = SAFETY_TCP;
    *(DWORD*)(rx_buf+4+len) = SAFETY_TCP;
    s->tcp.rx_data     = rx_buf + 4;
    s->tcp.max_rx_data = (UINT) (len - 1);
  }
  return (s->tcp.max_rx_data);
}

#if defined(USE_DEBUG)
/*
 * Check the Rx-buffer marker signatures of Rx/Tx-buffers.
 * Rx-buffer may have been allocated in above sock_setbuf().
 * Tx-buffer may have been allocated in tcp_set_window().
 * Only usable for TCP sockets.
 */
void _sock_check_tcp_buffers (const _tcp_Socket *tcp)
{
  const BYTE *rx, *tx;

  if (tcp->ip_type != TCP_PROTO)  /* should already be true */
     return;

  rx = tcp->rx_data;
  tx = tcp->tx_data;

  WATT_ASSERT (tcp->safetysig == SAFETY_TCP);
  WATT_ASSERT (tcp->safetytcp == SAFETY_TCP);

#if (DOSX)
  WATT_ASSERT (valid_addr(rx, tcp->max_rx_data));
  WATT_ASSERT (valid_addr(tx, tcp->max_tx_data));
#endif

  if (rx != &tcp->rx_buf[0])
  {
    WATT_ASSERT (*(DWORD*)(rx-4) == SAFETY_TCP);
    WATT_ASSERT (*(DWORD*)(rx+1+tcp->max_rx_data) == SAFETY_TCP);
  }
  if (tx != &tcp->tx_buf[0])
  {
    WATT_ASSERT (*(DWORD*)(tx-4) == SAFETY_TCP);
    WATT_ASSERT (*(DWORD*)(tx+1+tcp->max_tx_data) == SAFETY_TCP);
  }
}

void _sock_check_udp_buffers (const _udp_Socket *udp)
{
  ARGSUSED (udp);   /* \todo */
}
#endif


int W32_CALL sock_preread (const sock_type *s, BYTE *buf, int len)
{
  int count;
  int type = _chk_socket (s);

  /* Raw-sockets use fixed buffer
   */
  if (!type || type == VALID_IP4 || type == VALID_IP6)
     return (0);

  count = s->tcp.rx_datalen;
  if (count < 1)
     return (count);

  if (count > len)
      count = len;
  if (buf)
     memcpy (buf, s->tcp.rx_data, count);
  return (count);
}

/*
 * chk_socket - determine whether a real socket or not
 */
int W32_CALL _chk_socket (const sock_type *s)
{
  if (!s)
     return (0);

  if (s->tcp.ip_type == TCP_PROTO && s->tcp.state <= tcp_StateCLOSED)
     return (VALID_TCP);

  if (s->udp.ip_type == UDP_PROTO)
     return (VALID_UDP);

  if (s->raw.ip_type == IP4_TYPE)
     return (VALID_IP4);

  if (s->raw6.ip_type == IP6_TYPE)
     return (VALID_IP6);

  return (0);
}

const char *W32_CALL sockerr (const sock_type *s)
{
  if (s && s->tcp.err_msg)
     return (s->tcp.err_msg);
  return (NULL);
}

void W32_CALL sockerr_clear (sock_type *s)
{
  if (s)
  {
    s->tcp.err_msg = NULL;
    s->tcp.err_buf[0] = '\0';
  }
}

const char *W32_CALL sockstate (const sock_type *s)
{
  switch (_chk_socket(s))
  {
    case VALID_IP4:
         return (_LANG("Raw IPv4 Socket"));

    case VALID_IP6:
         return (_LANG("Raw IPv6 Socket"));

    case VALID_UDP:
         return (_LANG("UDP Socket"));

    case VALID_TCP:
         return (_LANG(tcpStateName(s->tcp.state)));

    default:
         return (_LANG("Not an active socket"));
  }
}

#if defined(USE_BSD_API)
const _raw_Socket *find_oldest_raw (const _raw_Socket *raw)
{
  const _raw_Socket *s;

  for (s = raw; raw; raw = raw->next)
  {
    if (raw->used && raw->seq_num < s->seq_num)
       s = raw;
  }
  return (s->used ? s : NULL);
}

#if defined(USE_IPV6)
const _raw6_Socket *find_oldest_raw6 (const _raw6_Socket *raw)
{
  const _raw6_Socket *s;

  for (s = raw; raw; raw = raw->next)
  {
    if (raw->used && raw->seq_num < s->seq_num)
       s = raw;
  }
  return (s->used ? s : NULL);
}
#endif
#endif

