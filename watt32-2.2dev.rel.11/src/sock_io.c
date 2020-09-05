/*!\file sock_io.c
 *
 * some sock_xx() functions moved from pctcp.c due
 * to memory contraints under Turbo-C
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "copyrigh.h"
#include "wattcp.h"
#include "strings.h"
#include "misc.h"
#include "pcconfig.h"
#include "pctcp.h"

/*
 * sock_putc - put a character
 *    - no expansion but flushes on CR/LF
 *    - returns character
 */
BYTE W32_CALL sock_putc (sock_type *s, BYTE c)
{
#if !defined(USE_UDP_ONLY)
  if (c == '\n' || c == '\r')
     sock_flushnext (s);
#endif
  sock_write (s, &c, 1);
  return (c);
}


/*
 * sock_puts - does not append carriage return in binary mode
 *           - returns length.
 *           - only accepts ASCII strings.
 */
int W32_CALL sock_puts (sock_type *s, const BYTE *data)
{
  int len;

#if defined(USE_BSD_API)
  if (s->raw.ip_type == IP4_TYPE)
     return (0);   /* not supported yet */
#endif

  len = strlen ((const char*)data);

  if (!(s->tcp.sockmode & SOCK_MODE_BINARY)) /* udp/tcp ASCII mode */
  {
    BYTE tmp [MAX_IP4_DATA];  /**< \todo suppport UDP frags */

    if (len > 0)  /* merge in a newline */
    {
      len = min (len, SIZEOF(tmp)-2);  /* May truncate, tough luck */
      memcpy (&tmp, data, len);
      tmp [len++] = '\r';
      tmp [len++] = '\n';
      data = tmp;
    }
    else
    {
      data = (const BYTE*) "\r\n";
      len  = 2;
    }
    len = sock_write (s, data, len);
  }
  else
  {
#if !defined(USE_UDP_ONLY)
    sock_flushnext (s);
#endif
    len = sock_write (s, data, len);
  }
  return (len);
}


/*
 * sock_gets - read a {\r|\n} terminated string from a UDP/TCP socket.
 *    - return length of returned string
 *    - removes end of line terminator(s)
 *    - Quentin Smart and Mark Phillips fixed some problems
 *
 * BIG WARNING: Don't use sock_gets() for packetised protocols like
 *              SSH. Only suitable for ASCII orientented protocols
 *              like POP3/SMTP/NNTP etc.
 */
WORD W32_CALL sock_gets (sock_type *s, BYTE *data, int bmax)
{
  int   len = 0, frag = 0;
  BYTE *nl_p, *cr_p, eol;

#if defined(USE_BSD_API)
  if (s->raw.ip_type == IP4_TYPE)
     return (0);   /* raw sockets not supported */
#endif

#if !defined(USE_UDP_ONLY)
  if (s->udp.ip_type == TCP_PROTO &&
      s->tcp.missed_seq[0] != s->tcp.missed_seq[1])
     frag = s->tcp.missed_seq[1] - s->tcp.recv_next;
#endif

  if (s->tcp.sockmode & SOCK_MODE_SAWCR)
  {
    s->tcp.sockmode &= ~SOCK_MODE_SAWCR;
    if (s->tcp.rx_datalen > 0 &&
        (*s->tcp.rx_data == '\n' || *s->tcp.rx_data == '\0'))
    {
      memmove (s->tcp.rx_data, s->tcp.rx_data + 1, frag + s->tcp.rx_datalen);
      s->tcp.rx_datalen--;
    }
  }

  /* Return if there is nothing in the buffer.
   */
  if (s->tcp.rx_datalen == 0)
     return (0);

  /* If there is space for all the data, then copy all of it,
   * otherwise, only copy what the space will allow (taking
   * care to reserve space for the null terminator.
   */
  --bmax;
  if (bmax > s->tcp.rx_datalen)
      bmax = s->tcp.rx_datalen;
  memcpy (data, s->tcp.rx_data, bmax);
  data [bmax] = '\0';

  /* At this point, data is a null-terminated string,
   * containing as much of the data as is possible.
   */
  len = bmax;

  /* Because we are in ASCII mode, we assume that the
   * sender will be consistent in which kind of CRLF is
   * sent (e.g. one and only one of \r\n, \r0, \r, or \n).
   * So if both \r and \n are found, we assume that they
   * are always next to each other, and \n\r is invalid.
   */

  /* Zero the first occurance of \r and \n in data.
   */
  cr_p = (BYTE*) memchr (data, '\r', bmax);
  if (cr_p)
     *cr_p = '\0';

  nl_p = (BYTE*) memchr (data, '\n', bmax);
  if (nl_p)
     *nl_p = '\0';

#if !defined(USE_UDP_ONLY)
  /*
   * Handle flushed end of strings EE 02.02.28
   */
  if (s->tcp.ip_type == TCP_PROTO)
  {
    if ((s->tcp.locflags & LF_GOT_PUSH) && !cr_p)
    {
      cr_p = data + bmax;
      s->tcp.locflags &= ~LF_GOT_PUSH;
    }

    /* Return if we did not find '\r' or '\n' yet, but still had room,
     * *and* the connection can get more data!
     */
    if (!cr_p && !nl_p &&
        (bmax > s->tcp.rx_datalen) &&
        ((UINT)s->tcp.rx_datalen < s->tcp.max_rx_data) &&
        (s->tcp.state != tcp_StateLASTACK) &&
        (s->tcp.state != tcp_StateCLOSED))
    {
      *data = '\0';
      return (0);
    }
  }
#endif

  /* If we did find a terminator, then stop there.
   */
  if (cr_p || nl_p)
  {
    /* Find the length of the first line of data in 'data'.
     */
    len = (int) (nl_p == NULL ? (char*)cr_p - (char*)data :
                 cr_p == NULL ? (char*)nl_p - (char*)data :
                 nl_p < cr_p  ? (char*)nl_p - (char*)data :
                                (char*)cr_p - (char*)data);

    /* We found a terminator character ...
     */
    bmax = len + 1;

    /* If '\r' at end of data, might get a '\0' or '\n' in next packet
     */
    if (cr_p && s->tcp.rx_datalen == bmax)
       s->tcp.sockmode |= SOCK_MODE_SAWCR;

    /* ... and it could have been "\r\0" or "\r\n".
     */
    eol = s->tcp.rx_data [bmax];
    if ((s->tcp.rx_datalen > bmax) &&
        (eol == '\0' || (cr_p && eol == '\n')))
       bmax++;
  }

  /* Remove the first line from the buffer.
   */
  s->tcp.rx_datalen -= bmax;
  if (frag || s->tcp.rx_datalen > 0)
     memmove (s->tcp.rx_data, s->tcp.rx_data + bmax, frag + s->tcp.rx_datalen);

#if !defined(USE_UDP_ONLY)
  /*
   * Update window if less than MSS/2 free in receive buffer
   */
  if (s->tcp.ip_type == TCP_PROTO &&
      s->tcp.state != tcp_StateCLOSED &&
      (s->tcp.max_rx_data - s->tcp.rx_datalen) < s->tcp.max_seg/2)
    TCP_SENDSOON (&s->tcp);
#endif

  return (len);
}

/*
 * Return a single character.
 * Returns EOF on fail (no data or connection closed).
 */
int W32_CALL sock_getc (sock_type *s)
{
  BYTE ch = 0;
  return (sock_read (s, &ch, 1) < 1 ? EOF : ch);
}

/**
 * sock_dataready - returns number of bytes waiting to be read.
 *    - if in ASCII mode, return >0 if a terminated line is present or
 *      the buffer is full or state is LASTACK (tcp only).
 *    - if in binary mode (default), simply return number of bytes in
 *      receive buffer.
 *    - For UDP or TCP sockets only.
 */
WORD W32_CALL sock_dataready (sock_type *s)
{
  char *p;
  int   len = s->tcp.rx_datalen;

#if defined(USE_BSD_API)
  if (s->raw.ip_type == IP4_TYPE)
     return (0);   /* not supported yet */
#endif

  if (len && !(s->tcp.sockmode & SOCK_MODE_BINARY))  /* ASCII mode */
  {
    p = (char*)s->tcp.rx_data;

    if (s->tcp.sockmode & SOCK_MODE_SAWCR)  /* !! S. Lawson */
    {
      s->tcp.sockmode &= ~SOCK_MODE_SAWCR;
      if (*p == '\n' || *p == '\0')
      {
        s->tcp.rx_datalen = --len;
        memmove (p, p+1, s->tcp.rx_datalen);
        if (!len)
           return (0);
      }
    }

    if ((UINT)len == s->tcp.max_rx_data)
       return (len);

    if (s->tcp.ip_type == TCP_PROTO)         /* GV 99.12.02 */
    {
      if (s->tcp.state == tcp_StateLASTACK)  /* EE 99.07.02 */
         return (len);

      /* S. Lawson - happens if final ACK arrives before app reads data
       */
      if (s->tcp.state == tcp_StateCLOSED)
         return (len);
    }

    /* check for terminating `\r' and/or `\n'
     */
    if (memchr(p, '\r', len))
       return (len);
    if (memchr(p, '\n', len))              /* EE added 99/04/30 */
       return (len);
    return (0);
  }
  return (len);
}

#ifdef NOT_YET
/**
 * \todo Wide character versions of above functions.
 */
int W32_CALL sock_putcw (sock_type *s, wchar_t ch)
{
}

/**
 * \todo Wide character version of sock_puts()
 */
int W32_CALL sock_putsw (sock_type *s, const wchar_t *data)
{
}

/**
 * \todo Wide character version of sock_gets()
 */
int W32_CALL sock_getsw (sock_type *s, wchar_t *data, int n)
{
}

/**
 * \todo Wide character version of sock_getc()
 */
wchar_t W32_CALL sock_getcw (sock_type *s)
{
}
#endif

