
/******************************************************************************

  FINGER - display user/system information

  Copyright (C) 1991, University of Waterloo

  This program is free software; you can redistribute it and/or modify
  it, but you may not sell it.

  This program is distributed in the hope that it will be useful,
  but without any warranty; without even the implied warranty of
  merchantability or fitness for a particular purpose.

      Erick Engelke                   or via E-Mail
      Faculty of Engineering
      University of Waterloo          Erick@development.watstar.uwaterloo.ca
      200 University Ave.,
      Waterloo, Ont., Canada
      N2L 3G1

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <tcp.h>
#include <netdb.h>

#define FINGER_PORT 79

#if (defined(__SMALL__) && !defined(DOS386)) || defined(__LARGE__)
  #define SAVE_SPACE
#endif


static char buffer [513];

void MS_CDECL TraceOff (void)
{
  ctrace_on = 0;
}

/*---------------------------------------------------------------*/

int finger (char *userid, DWORD host, char *hoststring)
{
  static tcp_Socket sock;
  int    len;
  int    status = 0;
  WORD   port   = FINGER_PORT;

#ifndef SAVE_SPACE
  struct servent *serv = getservbyname ("finger", "tcp");

  if (serv != NULL)
     port = intel16 (serv->s_port);
#endif

  if (host - my_ip_addr <= multihomes)
  {
    puts ("Cannot finger ourself!");
    return (2);
  }

  if (!tcp_open(&sock,0,host,port,NULL))
  {
    puts ("Sorry, unable to connect to that machine right now!");
    return (3);
  }

  printf ("waiting...");
  fflush (stdout);
  sock_wait_established (&sock, sock_delay, NULL, &status);
  printf ("\r");

  if (*userid)
     printf ("`%s' is looking for %s...\n\n", hoststring, userid);

  strcpy (buffer, userid);
  rip (buffer);                     /* kill all \n and \r's */
  strcat (buffer, "\r\n");
  sock_puts (&sock, (const BYTE*)buffer);

  while (1)
  {
    sock_wait_input (&sock, sock_delay, NULL, &status);
    len = sock_fastread (&sock, (BYTE*)buffer, sizeof(buffer)-1);
    buffer [len] = 0;
    printf ("%s", buffer);
  }

sock_err:
  switch (status)
  {
    case 1 : /* foreign host closed */
             puts ("\nHost closed");
             break;
    case -1: /* timeout/abort */
             printf ("\nError: %s\n", sockerr(&sock));
             break;
  }
  sock_close (&sock);
  return (status);
}

/*---------------------------------------------------------------*/

void usage (void)
{
  puts ("FINGER [-v] [-d] [userid]@server");
  exit (3);
}

int W32_CDECL main (int argc, char **argv)
{
  char  *user   = NULL;
  char  *server = "@localhost";
  DWORD  host;
  int    ch, rc;
  int    do_trace = 0;

  while ((ch = getopt(argc,argv,"?vdt")) != EOF)
     switch(ch)
     {
       case 'v': puts (wattcpVersion());
                 break;
       case 'd': dbug_init();
                 break;
       case 't': /* start tracing now (High-C only!) */
                 atexit (TraceOff);
                 do_trace = 1;
                 break;
       case '?':
       default : usage();
     }

  argc -= optind;
  argv += optind;

  if (argc < 1 || (user = strdup(*argv)) == NULL)
     usage();

  server = strchr (user, '@');
  if (!server)
     usage();

  sock_init();
  if (do_trace)
     ctrace_on = 1;
  *server++ = '\0';

  host = lookup_host (server, NULL);
  if (!host)
  {
    puts (dom_strerror(dom_errno));
    rc = 3;
  }
  else
    rc = finger (user, host, server);
  free (user);
  return (rc);
}

