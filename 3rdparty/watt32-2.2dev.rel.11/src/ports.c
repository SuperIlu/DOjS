/*!\file ports.c
 *
 * Handling of local ports used by UDP/TCP.
 *
 * Moved from pctcp.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "copyrigh.h"
#include "wattcp.h"
#include "strings.h"
#include "misc.h"
#include "run.h"
#include "pctcp.h"

#if (DOSX)
  #define MAX_PORT  USHRT_MAX  /* 65535 */
#else
  #define MAX_PORT  5000
#endif

/*
 * Allocated local TCP/UDP-ports start at 1025 (empherical range). We can
 * at max handle MAX_PORT local ports. Use an array of 'fd_set' to check which
 * port is free or not.
 */
#define MIN_PORT    1025
#define NUM_FD_SETS (1 + (MAX_PORT / (8*sizeof(fd_set))))

int    use_rand_lport = TRUE;
static fd_set *lport_inuse;

/**
 * Free allocated memory.
 */
static void W32_CALL exit_localport (void)
{
  if (!_watt_fatal_error)
     DO_FREE (lport_inuse);
}

/**
 * Allocate the "lport_inuse" bit-array from heap.
 * \retval Highest usable port.
 * \todo Make empherical port range configurable.
 * \todo Use some hashing technique to speed up find_free_port().
 */
WORD init_localport (void)
{
  size_t size = sizeof(fd_set) * NUM_FD_SETS;

  lport_inuse = calloc (size, 1);
  if (!lport_inuse)
     return (0);

  RUNDOWN_ADD (exit_localport, 303);
  return (MAX_PORT);
}

/*
 * Return unused local port
 *  - port = 0 -> normal port,
 *  - port = 1 -> special port (513-1023)
 *
 * `linger' is set if port shall be matched against "sleeping" ports.
 * Local tcp-ports should not be reused 60sec (2*MSL) after they have
 * been closed. Note this is NOT related to 'sock_delay'.
 */
WORD find_free_port (WORD port, BOOL linger)
{
  WORD p, lo_port, hi_port;

  if (!lport_inuse)
     return (port);

  if (port > 0 && port < USHRT_MAX)    /* return port as-is */
     return (port);

  if (port == 0)
  {
    if (use_rand_lport)
         lo_port = Random (MIN_PORT, 1500);
    else lo_port = MIN_PORT;
  }
  else
    lo_port = 513;

  hi_port = MAX_PORT;   /* our empherical max-port */

  for (p = lo_port; p <= hi_port; p++)
  {
    if (linger && FD_ISSET(p,lport_inuse)) /* inuse, try next */
       continue;

    if (_udp_allsocs)
    {
      const _udp_Socket *udp = _udp_allsocs;

      while (udp->next && udp->myport != p)
             udp = udp->next;
      if (udp->myport == p)
         continue;
    }
#if !defined(USE_UDP_ONLY)
    if (_tcp_allsocs)
    {
      const _tcp_Socket *tcp = _tcp_allsocs;

      while (tcp->next && tcp->myport != p)
             tcp = tcp->next;
      if (tcp->myport == p)
         continue;
    }
#endif
    break;
  }
  FD_SET (p, lport_inuse);
  return (p);
}

#if defined(USE_BSD_API)
/*
 * Set the "lport_inuse" bit for a local port.
 * 'port' is in host order.
 * Returns original state or -1 if port above our range.
 */
int grab_localport (WORD port)
{
  int rc;

#if (MAX_PORT < USHRT_MAX)
  if (port > MAX_PORT)
     return (-1);
#endif

  if (!lport_inuse)
     return (-1);

  rc = FD_ISSET (port, lport_inuse);
  FD_SET (port, lport_inuse);
  return (rc);
}
#endif

/*
 * Clear the "lport_inuse" bit for a local port.
 * 'port' is in host order. Port 0 is not considered a port.
 * Returns original state or -1 if port above our range.
 */
int reuse_localport (WORD port)
{
  int rc;

#if (MAX_PORT < USHRT_MAX)
  if (port > MAX_PORT)
     return (-1);
#endif

  if (!lport_inuse)
     return (-1);

  rc = FD_ISSET (port, lport_inuse);
  FD_CLR (port, lport_inuse);
  return (rc);
}

#if !defined(USE_UDP_ONLY)
/*
 * Reuse local port now if not owned by a STREAM-socket.
 * Otherwise let socket daemon free local port when linger period
 * expires. We don't care about rapid reuse of local ports associated
 * with DGRAM-sockets.
 */
int maybe_reuse_localport (_tcp_Socket *s)
{
#if defined(USE_BSD_API)
  if (!_bsd_socket_hook || !(*_bsd_socket_hook)(BSO_FIND_SOCK,s))
     return reuse_localport (s->myport);
  return (0);
#else
  return reuse_localport (s->myport);
#endif
}
#endif /* USE_UDP_ONLY */
