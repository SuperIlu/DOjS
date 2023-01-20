/****************************************************************************

  popdump.c - dump mail from popmail3 into spool subdirectory

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

  Now also for Windows! Actually the first working TCP program
  for Watt-32/Win.  -- G. Vanem 2004

****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__MSDOS__) || defined(MSDOS)
  #include <dos.h>
#elif defined(_WIN32)
  #define WIN32_LEAN_AND_MEAN    /* Do not include <winsock.h> */
  #include <windows.h>
#endif

#include <tcp.h>

#define POP3_PORT 110

#ifdef __MINGW32__
  /*
   * Tell MinGW's CRT to turn off command line globbing by default.
   */
  int _CRT_glob = 0;
#endif

char buffer[513];

long localdiskspace (void)
{
#if defined(_WIN32) || defined(__CYGWIN__)
  ULARGE_INTEGER free;
  GetDiskFreeSpaceEx ("\\", &free, NULL, NULL);
  return (long) free.LowPart;  /* very crude */
#else
  struct diskfree_t d;
  _dos_getdiskfree (0,&d);
  return (DWORD)d.avail_clusters *
         (DWORD)d.bytes_per_sector *
         (DWORD)d.sectors_per_cluster;
#endif
}

/*
 * getnumbers - returns the count of numbers received.
 * Expect 'ascii' on the form "+OK n1 n2".
 */
int getnumbers (const char *ascii, long *d1, long *d2)
{
  char *p = strchr(ascii, ' ');

  /* it must return a number after the white space */
  if (!p)
     return (0);

  /* skip space */
  while (*p == ' ')
      p++;
  *d1 = atol(p);

  if ((p = strchr(p, ' ')) == NULL)
     return (1);

  /* skip space */
  while (*p == ' ') p++;
  *d2 = atol(p);
  return (2);
}

#define RX_BUF_SIZE (16*1014) /* Set a larger Rx-buffer */

int popdump (const char *userid, const char *password, DWORD host)
{
  static tcp_Socket s;
  int    status;
  int    rc      = 0;
  long   process = 0;
  long   count, totallength, locallength, dummy;
  char  *rx_buf = malloc (RX_BUF_SIZE);

  if (!tcp_open(&s,0,host,POP3_PORT,NULL))
  {
    printf ("Sorry, unable to connect to host %s.\n", _inet_ntoa(NULL,host));
    return 1;
  }

  sock_setbuf (&s, (BYTE*)rx_buf, RX_BUF_SIZE);

  printf ("waiting...\r");

  sock_mode (&s, TCP_MODE_ASCII);
  sock_wait_established (&s, sock_delay, NULL, &status);
  sock_wait_input (&s, sock_delay, NULL, &status);
  sock_gets (&s, (BYTE*)&buffer, sizeof(buffer));
  puts (buffer);
  if (*buffer != '+')
     goto quit;

  sock_printf (&s, "USER %s", userid);
  sock_wait_input (&s, sock_delay, NULL, &status);
  sock_gets (&s, (BYTE*)&buffer, sizeof(buffer));
  puts (buffer);
  if (*buffer != '+')
     goto quit;

  sock_printf (&s, "PASS %s", password);
  sock_wait_input (&s, sock_delay, NULL, &status);
  sock_gets (&s, (BYTE*)&buffer, sizeof(buffer));
  puts(buffer);
  if (*buffer != '+')
     goto quit;

  sock_printf (&s, "STAT");
  printf ("STAT\n");
  sock_wait_input (&s, sock_delay, NULL, &status);
  sock_gets (&s, (BYTE*)&buffer, sizeof(buffer));
  puts (buffer);
  if (*buffer != '+')
     goto quit;

  /* it must return two valid numbers */
  if (getnumbers(buffer, &count, &totallength) < 2)
  {
    printf ("protocol error on STAT\n");
    goto quit;
  }
  if (totallength == 0)
     goto quit;

  printf ("Attempting to download %lu messages (%lu bytes)\n",
          count, totallength);

  while (process++ < count)
  {
    printf ("Getting file # %lu\n", process);

    sock_printf (&s, "LIST %lu", process);
    sock_wait_input (&s, sock_delay, NULL, &status);
    sock_gets (&s, (BYTE*)&buffer, sizeof(buffer));
    if (getnumbers(buffer,&dummy,&locallength) < 2)
    {
      printf("protocol error on LIST %lu\n", process);
      break;
    }

#ifndef _WIN32
    if (localdiskspace() < locallength * 2)
    {
      printf("Skipping file # %lu, too big for disk space available\n", process);
      continue;
    }
#endif
    sock_printf (&s, "RETR %lu", process);
    sock_wait_input (&s, sock_delay, NULL, &status);
    sock_gets (&s, (BYTE*)&buffer, sizeof(buffer));
    if (*buffer != '+')
       break;

    do
    {
      sock_wait_input (&s, sock_delay, NULL, &status);
      sock_gets (&s, (BYTE*)&buffer, sizeof(buffer));
      puts (buffer);
    }
    while (buffer[0] != '.' || buffer[1] != 0);

#ifndef _WIN32  /* not while testing this */
    sock_printf (&s, "DELE %lu", process);
    sock_wait_input (&s, sock_delay, NULL, &status);
    sock_gets (&s, (BYTE*)&buffer, sizeof(buffer));
#endif

    puts (buffer);
    if (*buffer != '+')
       break;
  }

quit:
  sock_printf (&s, "QUIT");
  sock_close (&s);
  sock_wait_closed (&s, sock_delay, NULL, &status);

sock_err:
  switch (status)
  {
    case 1: /* foreign host closed */
         rc = 3;
         break;
    case -1: /* timeout */
         printf("ERROR: %s\n", sockerr(&s));
         rc = 4;
         break;
  }
  free (rx_buf);
  return (rc);
}


int MS_CDECL main (int argc, char **argv)
{
  char  user[128], password[64], *server;
  DWORD host;

  if (argc > 1 && !strcmp(argv[1],"-d"))
  {
    argv++;
    argc--;
    dbug_init();
  }

  if (argc < 3)
  {
    puts("Usage: popdump [-d] userid@server password");
    return (3);
  }
  sock_init();

  strncpy (user, argv[1], sizeof(user)-1);
  user [sizeof(user)-1] = '\0';
  strncpy (password, argv[2], sizeof(password)-1);
  password [sizeof(password)-1] = '\0';

  /* username can contain '@', so arg = "guest@somewhere@mail.host.com"
   * is allowed.
   */
  if ((server = strrchr(user, '@')) == NULL)
  {
    printf("missing @server part of userid: %s\n", user);
    return 3;
  }

  *server++ = '\0';
  host = resolve (server);
  if (host)
     return popdump (user, password, host);

  printf ("Could not resolve host `%s'\n", server);
  return (3);
}


