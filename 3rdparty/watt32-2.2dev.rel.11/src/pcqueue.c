/*!\file pcqueue.c
 *
 *  Simple ring-buffer queue handler for reception of packets
 *  from network driver.
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
 *  Created Jan-1998
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "wattcp.h"
#include "strings.h"
#include "misc.h"
#include "x32vm.h"
#include "powerpak.h"
#include "pcpkt.h"
#include "pcqueue.h"

#if defined(USE_DEBUG)
int pktq_check (struct pkt_ringbuf *q)
{
  int   i;
  char *buf;

  if (!q || !q->num_buf || !q->buf_start)
     return (0);

  buf = (char*) q->buf_start;

  for (i = 0; i < q->num_buf; i++)
  {
    buf += q->buf_size;
    if (*(DWORD*)(buf - sizeof(DWORD)) != PKTQ_MARKER)
       return (0);
  }
  return (1);
}
#endif  /* USE_DEBUG */


int pktq_init (struct pkt_ringbuf *q, int size, int num, char *buf)
{
  q->buf_size  = size;
  q->num_buf   = num;
  q->buf_start = (DWORD_PTR) buf;
  q->in_index  = 0;
  q->out_index = 0;
#if (DOSX & DOS4GW)
  q->dos_ofs = 0;   /* must be set manually */
#endif

#if defined(USE_DEBUG)
  WATT_ASSERT (size > 0);
  WATT_ASSERT (num > 0);
  WATT_ASSERT (buf != NULL);
  {
    int i;
    for (i = 0; i < q->num_buf; i++)
    {
      buf += q->buf_size;
      *(DWORD*) (buf - sizeof(DWORD)) = PKTQ_MARKER;
    }
  }
#endif
  return (1);
}

/*
 * Increment the queue 'in_index' (head).
 * Check for wraps.
 */
int pktq_inc_in (struct pkt_ringbuf *q)
{
  q->in_index++;
  if (q->in_index >= q->num_buf)
      q->in_index = 0;
  return (q->in_index);
}

/*
 * Increment the queue 'out_index' (tail).
 * Check for wraps.
 */
int pktq_inc_out (struct pkt_ringbuf *q)
{
  q->out_index++;
  if (q->out_index >= q->num_buf)
      q->out_index = 0;
  return (q->out_index);
}

/*
 * Return the queue's next 'in_index' (head).
 * Check for wraps. Caller is responsible for using cli/sti
 * around this function when needed.
 */
int pktq_in_index (struct pkt_ringbuf *q)
{
  int index = q->in_index + 1;

  if (index >= q->num_buf)
      index = 0;
  return (index);
}

/*
 * Return the queue's head-buffer.
 * Should be interruptable because 'in_index' is 'volatile'.
 */
char *pktq_in_buf (struct pkt_ringbuf *q)
{
  return ((char*)q->buf_start + (q->buf_size * q->in_index));
}

/*
 * Return the queue's tail-buffer.
 */
char *pktq_out_buf (struct pkt_ringbuf *q)
{
  return ((char*)q->buf_start + (q->buf_size * q->out_index));
}


/*
 * Clear the queue ring-buffer by setting head = tail.
 */
void pktq_clear (struct pkt_ringbuf *q)
{
#if defined(USE_FAST_PKT) && defined(__MSDOS__)
  WORD out_ofs;

  DISABLE();
  out_ofs = FAR_PEEK_WORD (struct pkt_info, pkt_queue.out_index);
  FAR_POKE_WORD (struct pkt_info, pkt_queue.in_index, out_ofs);
  ENABLE();
  ARGSUSED (q);
#else
  DISABLE();
  q->in_index = q->out_index;
  ENABLE();
#endif
}

/*
 * Return number of buffers waiting in queue. Check for wraps.
 * Should be interruptable because 'in_index' is 'volatile'.
 */
int pktq_queued (struct pkt_ringbuf *q)
{
  register int index = q->out_index;
  register int num   = 0;

  DISABLE();

  while (index != q->in_index)
  {
    num++;
    if (++index >= q->num_buf)
       index = 0;
  }
  ENABLE();
  return (num);
}


#if defined(USE_FAST_PKT) && defined(__MSDOS__)

#if (DOSX == 0)
#error USE_FAST_PKT is not for real-mode targets.
#endif

DWORD asmpkt_rm_base;

#if defined(USE_DEBUG)
int pktq_far_check (struct pkt_ringbuf *q)
{
  DWORD buf_start, buf, marker;
  int   i, size;

  i = FAR_PEEK_WORD (struct pkt_info, pkt_queue.num_buf);
  if (i != q->num_buf)
  {
    (*_printf) ("pktq_far_check(): num = %d\n", i);
    pkt_dump_real_mem();
    return (0);
  }

  buf  = FAR_PEEK_DWORD (struct pkt_info, pkt_queue.buf_start);
  size = FAR_PEEK_WORD (struct pkt_info, pkt_queue.buf_size);
  buf_start = buf;

  for (i = 0; i < q->num_buf; i++)
  {
    buf += size;

    if (buf < asmpkt_rm_base || buf > asmpkt_rm_base + 64*1024)
    {
      (*_printf) ("pktq_far_check(): illegal buf = %08lX, size %d\n",
                  buf, size);
      pkt_dump_real_mem();
      return (0);
    }
    marker = PEEKL (0, buf - sizeof(DWORD));
    if (marker != PKTQ_MARKER)
    {
      (*_printf) ("pktq_far_check(): size = %d, buf_start = %08lX, "
                  "marker[%d] = %08lX\n", size, buf_start, i, marker);
      pkt_dump_real_mem();
      return (0);
    }
  }
  return (1);
}
#endif  /* USE_DEBUG */


int pktq_far_init (int size, int num, DWORD real_base)
{
  struct pkt_ringbuf q;
  DWORD  dos_addr;
  size_t rx_buf_ofs = sizeof(struct pkt_info);  /* rx_buf[] after _pkt_inf */
  int    i;

  memset (&q, 0, sizeof(q));
  q.buf_size  = size;
  q.buf_start = real_base + rx_buf_ofs;
  q.num_buf   = num;
  q.dos_ofs   = rx_buf_ofs;
  asmpkt_rm_base = real_base;

  WATT_ASSERT (size > 0);
  WATT_ASSERT (num > 0);

  for (i = 0; i < q.num_buf; i++)
  {
    real_base += q.buf_size;
    POKEL (0, q.dos_ofs + real_base - sizeof(DWORD), PKTQ_MARKER);
  }

  dos_addr = asmpkt_rm_base + offsetof(struct pkt_info,pkt_queue);

#if (DOSX & (PHARLAP|X32VM|POWERPAK))
  WriteRealMem (dos_addr, &q, sizeof(q));
#elif (DOSX & DOS4GW)
  memcpy ((void*)dos_addr, &q, sizeof(q));
#elif (DOSX & DJGPP)
  dosmemput (&q, sizeof(q), dos_addr);
#else
  #error Help!
#endif
  return (1);
}
#endif /* USE_FAST_PKT && __MSDOS__ */
