/*
 * Lpq - query printer
 *
 *   Copyright (C) 1991, University of Waterloo
 *
 *   Portions Copyright (C) 1990, National Center for Supercomputer Applications
 *   and portions copyright (c) 1990, Clarkson University
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it, but you may not sell it.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but without any warranty; without even the implied warranty of
 *   merchantability or fitness for a particular purpose.
 *
 *       Erick Engelke                   or via E-Mail
 *       Faculty of Engineering
 *       University of Waterloo          Erick@development.watstar.uwaterloo.ca
 *       200 University Ave.,
 *       Waterloo, Ont., Canada
 *       N2L 3G1
 *
 *  If you want to use this, make sure you are in /etc/host.lpr or better.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <tcp.h>

#define SHORT_LIST 3
#define LONG_LIST  4

#define LPQ_PORT   515
#define LOCAL_PORT 722

static void Usage (void)
{
  puts ("Usage: lpq [-vVd?] [-Pprinter] [-Sserver]");
  exit (1);
}

static void Abort (const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  vfprintf (stdout, fmt, args);
  va_end (args);
  exit (1);
}

int MS_CDECL main (int argc, char **argv)
{
  char  buffer [513];
  char *printer = NULL;
  char *host    = NULL;

  static tcp_Socket socket;
  tcp_Socket *s = &socket;
  DWORD       host_ip;
  int         status,localport,len,ch;
  int         connected = 0;
  int         verbose   = 0;

  while ((ch = getopt(argc,argv,"?vVP:S:d")) != EOF)
     switch (ch)
     {
       case 'v': puts (wattcpVersion());
                 break;
       case 'P': printer = optarg;
                 break;
       case 'S': host = optarg;
                 break;
       case 'V': verbose = 1;
                 break;
       case 'd': tcp_set_debug_state (2);
                 dbug_init();
                 break;
       default : Usage();
     }

  if (!host)
  {
    char *env = getenv ("LPRSRV");
    if (!env)
       Abort ("Environment variable \"LPRSRV\" not set");
    host = strdup (env);
  }

  if (!printer)
  {
    char *env = getenv ("PRINTER");
    if (!env)
       Abort ("Environment variable \"PRINTER\" not set");
    printer = strdup (env);
  }

  sock_init();

  if ((host_ip = lookup_host(host,NULL)) == 0)
     Abort (dom_strerror(dom_errno));

  srand ((unsigned int)time(NULL));
  localport = 255 + (rand() & 255);
  localport = LOCAL_PORT;

  if (!tcp_open (s,localport, host_ip, LPQ_PORT, NULL))
     Abort ("Unable to open socket");

  sock_wait_established (s, sock_delay, NULL, &status);
  connected = 1;

  if (sock_dataready(s))
  {
    sock_fastread (s, (BYTE*)buffer, sizeof(buffer));
    buffer [sizeof(buffer)-1] = 0;
    printf ("Response: %s\n", buffer);
    sock_tick (s,&status);     /* in case above message closed port */
  }

  sock_printf (s,"%c%s\n", verbose ? LONG_LIST : SHORT_LIST, printer);

  while (1)
  {
    sock_wait_input (s, sock_delay, NULL, &status);
    len = sock_read (s, (BYTE*)buffer, sizeof(buffer));
    printf ("%*.*s",len,len,buffer);
  }

sock_err:
  switch (status)
  {
    case  1: status = 0;
             break;
    case -1: puts ("Host closed connection.");
             status = 3;
             break;
  }
  if (!connected)
     puts ("\nCouldn't connect. You may not be in the /etc/hosts.lpd file!");
  return (status);
}

