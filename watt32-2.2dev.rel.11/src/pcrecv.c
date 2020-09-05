/*!\file pcrecv.c
 *
 * Alternative socket receive handlers (see Waterloo manual).
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "wattcp.h"
#include "strings.h"
#include "language.h"
#include "bsddbug.h"
#include "misc.h"
#include "pcsed.h"
#include "pcbuf.h"
#include "pctcp.h"
#include "pcrecv.h"

#define RECV_USED    0xF7E3D2B1L
#define RECV_UNUSED  0xE6D2C1AFL

/* UDP "sequence number" for finding the oldest packet.
 * Newest received has higher 'p->buf_seqnum' number
 */
static long seq_num = 0;

/*
 * Gets upcalled when data arrives.
 * We MUST set 'p->buf_len = -1' to signal a 0-byte UDP packet
 * not 0
 */
static int W32_CALL sock_recvdaemon (
           sock_type *s, const void *data, unsigned len,
           const tcp_PseudoHeader *ph, const udp_Header *udp)
{
  recv_data *r;
  recv_buf  *p;
  unsigned   i;

  switch (s->udp.ip_type)
  {
    case UDP_PROTO:
         r = (recv_data*) s->udp.rx_data;
         p = (recv_buf*) r->recv_bufs;
         if (r->recv_sig != RECV_USED)
         {
           outsnl (_LANG("ERROR: udp recv data conflict"));
           return (0);
         }
         /* find an unused buffer
          */
         for (i = 0; i < r->recv_bufnum; i++, p++)
             switch (p->buf_sig)
             {
               case RECV_USED:
                    break;
               case RECV_UNUSED:  /* take this one */
                    p->buf_sig     = RECV_USED;
                    p->buf_hisport = udp->srcPort;
                    p->buf_seqnum  = seq_num++;
#if defined(USE_IPV6)
                    if (s->udp.is_ip6)
                       memcpy (&p->buf_hisip6, &((tcp_PseudoHeader6*)ph)->src,
                               sizeof(p->buf_hisip6));

                    else
#endif
                       p->buf_hisip = ph->src;
                    len = min (len, sizeof(p->buf_data));
                    if (len > 0)
                    {
                      memcpy (p->buf_data, data, len);
                      p->buf_len = (short) len;
                    }
                    else
                      p->buf_len = -1;  /* a 0-byte probe */
#if 0
                    SOCK_DEBUGF (("\nsock_recvdaemon(): buffer %d, "
                                  "seq-num %ld, len %d",
                                  i, seq_num-1, p->buf_len));
#endif
                    return (0);
               default:
                    outsnl (_LANG("ERROR: sock_recv_daemon data err"));
                    return (0);
             }
         return (0);

#if !defined(USE_UDP_ONLY)
    case TCP_PROTO:
         {
           _tcp_Socket *t = &s->tcp;

           r = (recv_data*) t->rx_data;

           if (r->recv_sig != RECV_USED)
           {
             outsnl (_LANG("ERROR: tcp recv data conflict"));
             return (0);
           }
           /* stick it on the end if you can
            */
           i = t->max_rx_data - t->rx_datalen;
           if (i > 1)
           {
             /* we can accept some of this */
             if (len > i)
                 len = i;
             if (len > 0)
                memcpy (r->recv_bufs + t->rx_datalen, data, len);
             t->rx_datalen += len;
             return (len);
           }
           return (0);   /* didn't take none */
         }
#endif
  }
  return (0);
}


int W32_CALL sock_recv_used (const sock_type *s)
{
  const recv_data *r;
  const recv_buf  *p;

  int i, len;

  switch (_chk_socket(s))
  {
    case VALID_UDP:
         r = (const recv_data*) s->udp.rx_data;
         p = (const recv_buf*) r->recv_bufs;
         if (r->recv_sig != RECV_USED)
            return (-1);

         for (i = len = 0; i < r->recv_bufnum; i++, p++)
             if (p->buf_sig == RECV_USED)
                len += abs (p->buf_len);   /* -1 -> 1 */
         return (len);

#if !defined(USE_UDP_ONLY)
    case VALID_TCP:
         r = (const recv_data*) s->tcp.rx_data;
         if (r->recv_sig != RECV_USED)
            return (-1);
         return (s->tcp.rx_datalen);
#endif
  }
  return (0);
}


int W32_CALL sock_recv_init (sock_type *s, void *space, unsigned len)
{
  recv_buf  *p = (recv_buf*) space;
  recv_data *r = (recv_data*) s->udp.rx_data;
  int i;

  WATT_ASSERT ((DWORD)len <= USHRT_MAX);
  memset (r, 0, s->udp.max_rx_data);  /* clear Rx-buffer */
  memset (p, 0, len);                 /* clear data area */

  s->udp.protoHandler = (ProtoHandler)sock_recvdaemon;
  r->recv_sig         = RECV_USED;
  r->recv_bufs        = (BYTE*) p;
  r->recv_bufnum      = (WORD) (len / sizeof(recv_buf));

  if (s->udp.ip_type == UDP_PROTO)
     for (i = 0; i < r->recv_bufnum; i++, p++)
         p->buf_sig = RECV_UNUSED;
  return (0);
}

int W32_CALL sock_recv_from (sock_type *s, DWORD *hisip, WORD *hisport,
                             void *buffer, unsigned len, int peek)
{
#if !defined(USE_UDP_ONLY)
  _tcp_Socket *t;
#endif
  recv_buf    *p, *oldest = NULL;
  recv_data   *r = (recv_data*) s->udp.rx_data;
  long seqnum = LONG_MAX;
  int  i;

  if (r->recv_sig != RECV_USED)
  {
    SOCK_ERRNO (EBADF);

    /* To differentiate an error from 0-byte probe (also -1) */
    return (-1);
  }

  switch (s->udp.ip_type)
  {
    case UDP_PROTO:
         p = (recv_buf*) r->recv_bufs;

         /* find the oldest used UDP buffer.
          */
         for (i = 0; i < r->recv_bufnum; i++, p++)
         {
           switch (p->buf_sig)
           {
             case RECV_UNUSED:
                  break;

             case RECV_USED:
#if defined(__MSDOS__)
                  /* Drop looped packets sent by us (running
                   * under Win32 DOS box using NDIS3PKT or SwsVpkt).
                   */
                  if ((_eth_ndis3pkt || _eth_SwsVpkt) &&
                      !s->tcp.is_ip6 && p->buf_hisip == intel(my_ip_addr))
                  {
                    p->buf_sig = RECV_UNUSED;
                    continue;
                  }
#endif
                  if (p->buf_seqnum < seqnum)  /* ignore wraps */
                  {
                    seqnum = p->buf_seqnum;
                    oldest = p;
#if 0
                    SOCK_DEBUGF (("\nsock_recv_from(): buffer %d, "
                                  "seq-num %ld, len %d",
                                  i, seqnum, p->buf_len));
#endif
                  }
                  break;

             default:
                  outsnl (_LANG("ERROR: sock_recv_init data err"));
                  return (0);
           }
         }
         break;

#if !defined(USE_UDP_ONLY)
    case TCP_PROTO:
         t = &s->tcp;
         len = min (len, (unsigned)t->rx_datalen);
         if (len)
             memcpy (buffer, r->recv_bufs, len);
         return (len);
#endif
  }

  if (!oldest)
     return (0);

  /* found the oldest UDP packet */

  p = oldest;
  if (p->buf_len < 0)  /* a 0-byte probe packet */
     len = -1;
  else
  {
    len = min ((unsigned)p->buf_len, len);
    memcpy (buffer, p->buf_data, len);
  }

#if defined(USE_IPV6)
  if (s->tcp.is_ip6)
  {
    if (hisip)
       memcpy (hisip, &p->buf_hisip6, sizeof(ip6_address));
  }
  else
#endif
  if (hisip)
     *hisip = p->buf_hisip;

  if (hisport)
     *hisport = p->buf_hisport;

  if (!peek)
     p->buf_sig = RECV_UNUSED;
  return (len);
}

int W32_CALL sock_recv (sock_type *s, void *buffer, unsigned len)
{
  return sock_recv_from (s, NULL, NULL, buffer, len, 0);
}

