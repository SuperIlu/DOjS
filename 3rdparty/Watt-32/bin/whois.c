/*
 *  whois.c for Baylor College of Medicine.
 *  written by Stan Barber, Director, Networking & Systems Support
 *  Copyright 1991, Baylor College of Medicine
 *  All Rights Reserved.
 *
 *  Modified by G.Vanem 1997 <gvanem@yahoo.no>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <tcp.h>

#define NICHOST     "whois.norid.no"  /* "whois.bcm.tmc.edu" */
#define WHOIS_PORT  43

#if (defined(__SMALL__) && !defined(DOS386)) || defined(__LARGE__)
  #define SAVE_SPACE
#endif

void usage (void)
{
  puts ("usage: whois [-vd] [-h host] name");
  puts ("       try  `whois help'");
  exit (0);
}

int MS_CDECL main (int argc, char **argv)
{
  int    ch;
  char  *host     = NICHOST;
  char  *query    = "help";
  int    status   = 0;
  int    debug    = 0;
  DWORD  ip_addr  = 0;
  WORD   port     = WHOIS_PORT;
  char  *env      = getenv ("WHOIS");
  static tcp_Socket sock;

  if (env)
     host = strdup (env);

  while ((ch = getopt(argc, argv, "?vdh:")) != EOF)
     switch (ch)
     {
       case 'v': puts (wattcpVersion());
                 puts ("WHOIS for Waterloo TCP/IP,  Copyright 1991 Baylor College of Medicine");
                 puts ("For information, send email to `whois-bugs@bcm.tmc.edu'\n");
                 return (0);

       case 'h': host = optarg;
                 break;

       case 'd': debug++;
                 break;

       default:  usage();
     }

  argc -= optind;
  argv += optind;

  if (argc-- < 1 || (query = *argv++) == NULL)
     usage();

  if (debug > 0)
  {
    dbug_init();
    tcp_set_debug_state (debug);
  }
  sock_init();

  if ((ip_addr = lookup_host(host,NULL)) == 0L)
  {
    printf ("%s", dom_strerror(dom_errno));
    return (1);
  }

#ifndef SAVE_SPACE
  {
    struct servent *sp = getservbyname ("whois","tcp");
    if (sp)
       port = intel16 (sp->s_port);
  }
#endif

  if (!tcp_open(&sock,0,ip_addr,port,NULL))
  {
    puts ("Unable to connect to that machine");
    return (3);
  }

  sock_mode (&sock, SOCK_MODE_ASCII);
  printf ("waiting...");
  fflush (stdout);
  sock_wait_established (&sock, sock_delay, NULL, &status);

  printf ("\rquerying ");
  sock_puts (&sock, (const BYTE*)query);

  while (!watt_kbhit())
  {
    char buf[256];

    if (!tcp_tick(&sock))
       break;
    if (sock_gets(&sock,(BYTE*)&buf,sizeof(buf)) > 0)
       puts (buf);
  }
  sock_close (&sock);
  return (0);

sock_err:
  printf ("Connection failed: %s\n", sockerr(&sock));
  return (5);
}
