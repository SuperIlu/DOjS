/*
 * DAYTIME - read and print time of day string from internet
 *
 * Copyright (C) 1991, University of Waterloo
 * Portions Copyright (C) 1990, National Center for Supercomputer Applications
 *
 * This program is free software; you can redistribute it and/or modify
 * it, but you may not sell it.
 *
 * This program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * merchantability or fitness for a particular purpose.
 *
 *     Erick Engelke                   or via E-Mail
 *     Faculty of Engineering
 *     University of Waterloo          Erick@development.watstar.uwaterloo.ca
 *     200 University Ave.,
 *     Waterloo, Ont., Canada
 *     N2L 3G1
 *
 * Returns:
 *     0 - success
 *     2 - some failure in the connection (port unavailable,
 *         no response, etc.)
 *     3 - unable to reach it - local host or first router is down
 *
 * This works as of March 2020:
 *  e:\net\watt\bin> daytime.exe -d utcnist2.colorado.edu
 *  Resolving utcnist2.colorado.edu...128.138.141.172
 *  connected
 *
 * 58938 20-03-30 12:49:57 50 0 0   0.0 UTC(NIST) *
 *
 * A list of servers:
 *   https://tf.nist.gov/tf-cgi/servers.cgi
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tcp.h>

#define DAYTIME_PORT 13

#if !defined(__SMALL__) || defined(DOS386)  /* 32-bit dmc defines __SMALL__ */
  #define TCP_DAYTIME                       /* no choice; use UDP */
#endif

#if defined(__GNUC__)
  #pragma GCC diagnostic ignored "-Waddress"
#endif

int daytime (DWORD host, int use_udp)
{
  static tcp_Socket sock;
  void *s = &sock;
  char  buffer [513];
  int   status = 0;
  int   total  = 0;
  int   udp_retries = 3;
  DWORD udp_retrytime = 0UL;

  if (use_udp)
  {
    udp_open (s, 0, host, DAYTIME_PORT, NULL);
    sock_putc (s, '\n');
    udp_retrytime = set_timeout (2000);
  }
#ifdef TCP_DAYTIME
  else
  {
    if (!tcp_open(s, 0, host, DAYTIME_PORT, NULL))
    {
      outs ("Sorry, unable to connect to that machine right now!\n");
      return (3);
    }
    outs ("waiting...\r");
    sock_wait_established (s, sock_delay, NULL, &status);
    outsnl ("connected ");
  }
#endif

  while (1)
  {
    sock_tick (s, &status);

    if (use_udp && chk_timeout(udp_retrytime))
    {
      if (udp_retries-- == 0)
         break;
      udp_retrytime = set_timeout (2000);
      sock_putc (s, '\n');
    }
    while (sock_dataready(s))
    {
      int len = sock_gets (s, (BYTE*)&buffer, sizeof(buffer));

      outsnl (buffer);
      total += len;
      break;
    }
    if (total > 0)
       break;
  }
  sock_close (s);
  sock_wait_closed (s, sock_delay, NULL, &status);

sock_err:
  switch (status)
  {
    case 1: /* foreign host closed normally */
         return (total > 0 ? 0 : 2);
    case -1: /* timeout/abort */
         outsnl (sockerr(s));
         return (2);
    default:
         outsnl ("\nAborting");
         return (2);
  }
}

void Usage (void)
{
  outsnl ("DAYTIME [-dvu] server");
  exit (1);
}

int MS_CDECL main (int argc, char **argv)
{
  DWORD host;
  int   ch;
  int   use_udp = 0;
  const char *name = NULL;

  while ((ch = getopt(argc, argv, "?vdu")) != EOF)
     switch (ch)
     {
       case 'v':
            puts (wattcpVersion());
            break;
       case 'd':
            dbug_init();
            break;
       case 'u':
            use_udp = 1;
            break;
       case '?':
       default:
            Usage();
     }

  argc -= optind;
  argv += optind;

  if (argc-- < 1 || (name = *argv++) == NULL)
     Usage();

  sock_init();

  host = lookup_host (name, NULL);
  if (!host)
  {
    printf ("%s", dom_strerror(dom_errno));
    return (3);
  }

#ifdef TCP_DAYTIME
  return daytime (host, use_udp);
#else
  return daytime (host, 1);
#endif
}
