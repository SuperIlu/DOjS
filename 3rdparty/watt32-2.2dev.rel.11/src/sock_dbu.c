/*!\file sock_dbu.c
 * sock_type debug dump.
 */
#include <stdio.h>

#include "copyrigh.h"
#include "wattcp.h"
#include "strings.h"
#include "misc.h"
#include "timer.h"
#include "pcdbug.h"
#include "pcbuf.h"

void W32_CALL sock_debugdump (const sock_type *s)
{
#if defined(USE_DEBUG)
  const _tcp_Socket *tcp = &s->tcp;

  if (s->raw.ip_type == IP4_TYPE)
     return;

#if defined(__SMALL__) && !defined(__SMALL32__)
  (*_printf) ("next       %04X\n",      s->tcp.next);
#elif defined(__LARGE__)
  (*_printf) ("next       %04X:%04X\n", FP_SEG(s->tcp.next), FP_OFF(s->tcp.next));
#else
  (*_printf) ("next       %" ADDR_FMT "\n", ADDR_CAST(s->tcp.next));
#endif

  (*_printf) ("type       %d\n", s->tcp.ip_type);

  (*_printf) ("stat/error %s\n", s->tcp.err_msg ? s->tcp.err_msg : "(none)");
  (*_printf) ("usr-timer  %08lX (%sexpired)\n",
              (u_long)s->tcp.usertimer, chk_timeout(s->tcp.usertimer) ? "" : "not ");

  switch (s->tcp.ip_type)
  {
    case UDP_PROTO:
         (*_printf) ("udp rxdata  %u `%.*s'\n",
                     s->tcp.rx_datalen, s->tcp.rx_datalen, s->tcp.rx_data);
         break;
    case TCP_PROTO:
         (*_printf) ("tcp rxdata  %u `%.*s'\n",
                     tcp->rx_datalen, tcp->rx_datalen, tcp->rx_data);
         (*_printf) ("tcp state  %u (%s)\n",
                     tcp->state, tcpStateName(tcp->state));
         break;
  }
#else
  ARGSUSED (s);
#endif
}
