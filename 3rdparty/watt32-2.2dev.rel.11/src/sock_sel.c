/*!\file sock_sel.c
 * sock_select(), tcp_simple_state()
 */
#include <stdio.h>

#include "copyrigh.h"
#include "wattcp.h"
#include "pcbuf.h"
#include "pctcp.h"

/*
 * sock_sselect - returns one of several constants indicating
 *                SOCKESTABLISHED - tcp connection has been established
 *                SOCKDATAREAY    - tcp/udp data ready for reading
 *                SOCKCLOSED      - socket has been closed
 */
int W32_CALL sock_sselect (const sock_type *s, int waitstate)
{
  /* are we connected ?
   */
  if (waitstate == SOCKDATAREADY && s->tcp.rx_datalen > 0)
     return (SOCKDATAREADY);

  if (s->tcp.ip_type == 0)
     return (SOCKCLOSED);

  if (waitstate == SOCKESTABLISHED)
  {
    if (s->tcp.ip_type == UDP_PROTO      ||
        s->tcp.state   == tcp_StateESTAB ||
        s->tcp.state   == tcp_StateESTCL ||
        s->tcp.state   == tcp_StateCLOSWT)
       return (SOCKESTABLISHED);
  }
  return (0);
}

/*
 * Returns 'simplified' enum telling what state the tcp socket is currently
 * in, GvB 2002-09
 */
enum TCP_SIMPLE_STATE W32_CALL tcp_simple_state (const _tcp_Socket *s)
{
  if (s->ip_type != TCP_PROTO ||
      s->state == tcp_StateCLOSED)
     return (TCP_CLOSED);

  if (s->state == tcp_StateLISTEN)
     return (TCP_LISTENING);

  if (s->state < tcp_StateESTAB)
     return (TCP_OPENING);

  if (s->state == tcp_StateESTAB)
     return (TCP_OPEN);

  return (TCP_CLOSING);
}

