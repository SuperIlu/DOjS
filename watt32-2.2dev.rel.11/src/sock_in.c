/*!\file sock_in.c
 *
 * SOCK_IN: User input functions moved from pctcp.c.
 *
 *  Functions used in these macros:
 *    sock_wait_established()
 *    sock_wait_input()
 *    sock_wait_closed();
 */

#include <stdio.h>

#include "copyrigh.h"
#include "wattcp.h"
#include "language.h"
#include "misc.h"
#include "timer.h"
#include "wdpmi.h"
#include "pctcp.h"

/*
 * ip user level timer stuff
 *   void ip_timer_init (void *s, int delayseconds)
 *   int  ip_timer_expired (void *s)
 *  - 0 if not expired
 */
void W32_CALL ip_timer_init (sock_type *s, unsigned seconds)
{
  if (seconds)
       s->tcp.usertimer = set_timeout (1000UL * seconds);
  else s->tcp.usertimer = 0;
}

int W32_CALL ip_timer_expired (const sock_type *s)
{
  if (!s->tcp.usertimer)
     return (0);
  return chk_timeout (s->tcp.usertimer);
}

/*
 * _ip_delay0 called by macro sock_wait_established()
 *
 */
int W32_CALL _ip_delay0 (sock_type *s, int timeout_sec, UserHandler fn, int *statusptr)
{
  int status = -1;

  ip_timer_init (s, timeout_sec);
  do
  {
#if !defined(USE_UDP_ONLY)
    if (s->tcp.ip_type == TCP_PROTO && tcp_established(&s->tcp))
    {
      status = 0;
      break;
    }
#endif

    if (!tcp_tick(s))
    {
      if (s->tcp.err_msg == NULL)
          s->tcp.err_msg = _LANG("Host refused connection");
      status = -1;       /* get an early reset */
      break;
    }
    if (ip_timer_expired(s))
    {
      if (s->tcp.err_msg == NULL)
          s->tcp.err_msg = _LANG("Open timed out");
      if (!(s->tcp.locflags & LF_NOCLOSE))
         sock_close (s);
      status = -1;
      break;
    }
    if (fn && (status = (*fn)(s)) != 0)
       break;

    if (s->tcp.usr_yield)
         (*s->tcp.usr_yield)();
    else WATT_YIELD();

    if (s->tcp.ip_type == UDP_PROTO)
    {
      status = 0;
      break;
    }
  }
  while (1);

  if (statusptr)
     *statusptr = status;
  return (status);
}

/*
 * _ip_delay1 called by macro sock_wait_input()
 */
int W32_CALL _ip_delay1 (sock_type *s, int timeoutseconds, UserHandler fn, int *statusptr)
{
  int status = -1;

  ip_timer_init (s, timeoutseconds);

#if !defined(USE_UDP_ONLY)
  sock_flush (s);    /* new enhancement */
#endif

  do
  {
    if (sock_dataready(s))
    {
      status = 0;
      break;
    }
    if (s->tcp.locflags & LF_GOT_FIN)
    {
      status = 1;
      break;
    }
    if (!tcp_tick(s))
    {
      status = 1;
      break;
    }

    if (ip_timer_expired(s))
    {
      if (s->tcp.err_msg == NULL)
          s->tcp.err_msg = _LANG("Connection timed out");
      if (!(s->tcp.locflags & LF_NOCLOSE))
         sock_close (s);
      status = -1;
      break;
    }
    if (fn && (status = (*fn)(s)) != 0)
       break;

    if (s->tcp.usr_yield)
        (*s->tcp.usr_yield)();
    else WATT_YIELD();
  }
  while (1);

  if (statusptr)
     *statusptr = status;
  return (status);
}

/*
 * _ip_delay2 called by macro sock_wait_closed();
 */
int W32_CALL _ip_delay2 (sock_type *s, int timeoutseconds, UserHandler fn, int *statusptr)
{
  int status = -1;

  if (s->tcp.ip_type != TCP_PROTO)
  {
    if (statusptr)
       *statusptr = 1;
    return (1);
  }

#if !defined(USE_UDP_ONLY)
  ip_timer_init (s, timeoutseconds);

  do
  {
    /* in this situation we know user is not planning to read rx_data
     */
    s->tcp.rx_datalen = 0;

    if (!tcp_tick(s))
    {
      status = 1;
      break;
    }
    if (ip_timer_expired(s))
    {
      if (s->tcp.err_msg == NULL)
          s->tcp.err_msg = _LANG("Connection timed out");
      TCP_ABORT (&s->tcp);
      status = -1;
      break;
    }
    if (fn && (status = (*fn)(s)) != 0)
       break;

    if (s->tcp.usr_yield)
        (*s->tcp.usr_yield)();
    else WATT_YIELD();
  }
  while (1);

  if (statusptr)
     *statusptr = status;

#else
  ARGSUSED (fn);
  ARGSUSED (timeoutseconds);
#endif
  return (status);
}

int W32_CALL sock_timeout (sock_type *s, int sec)
{
  if (s->tcp.ip_type != TCP_PROTO)
     return (1);

#if !defined(USE_UDP_ONLY)
  if (s->tcp.state != tcp_StateESTAB)
     return (2);

  s->tcp.timeout = set_timeout (1000UL * sec);
#else
  ARGSUSED (sec);
#endif
  return (0);
}

int W32_CALL sock_established (sock_type *s)
{
  switch (s->tcp.ip_type)
  {
    case UDP_PROTO:
         return (1);

#if !defined(USE_UDP_ONLY)
    case TCP_PROTO:
         return (s->tcp.state == tcp_StateESTAB ||
                 s->tcp.state == tcp_StateESTCL ||
                 s->tcp.state == tcp_StateCLOSWT);
#endif
    default:
         return (0);
  }
}

