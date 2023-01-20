/*
 *  NTIME - set dos clock from internet
 *          see RFC 868
 *
 *  Copyright (C) 1991, University of Waterloo
 *  portions Copyright (C) 1990, National Center for Supercomputer Applications
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it, but you may not sell it.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.
 *
 *      Erick Engelke                   or via E-Mail
 *      Faculty of Engineering
 *      University of Waterloo          Erick@development.watstar.uwaterloo.ca
 *      200 University Ave.,
 *      Waterloo, Ont., Canada
 *      N2L 3G1
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <tcp.h>

#if !defined(__CYGWIN__)
#include <dos.h>
#endif

#if !defined(__SMALL__) || defined(DOS386) /* 32-bit dmc defines __SMALL__ */
#define TCP_TIME
#endif


/*
 * Notes:
 * The time is the number of seconds since 00:00 (midnight) 1 January 1900
 * GMT, such that the time 1 is 12:00:01 am on 1 January 1900 GMT; this
 * base will serve until the year 2036.
 *
 * For example:
 *
 *     2,208,988,800L corresponds to 00:00  1 Jan 1970 GMT, start of UNIX time
 *
 */

#define TIME_PORT  37
#define BASE_TIME  2208988800UL

/*
 * ntime() given the host address, returns an Internet based time, not an
 *         UNIX or DOS time.  The UNIX time may be derived by subtracting
 *         BASE_TIME from the returned value.
 */
time_t ntime (DWORD host, time_t *now)
{
  static tcp_Socket sock;
  int    status  = 0;
  time_t host_tm = 0;


#ifdef TCP_TIME
  if (!tcp_open(&sock,0,host,TIME_PORT, NULL))
  {
    puts ("Sorry, unable to connect to that machine right now!");
    return (0);
  }
  printf ("waiting...\r");
  fflush (stdout);
  sock_wait_established (&sock, sock_delay, NULL, &status);
  printf ("connected \n");
#else
  udp_open (&sock, 0, host, TIME_PORT, NULL);
  sock_write (&sock, "\n", 1);
#endif

  while (1)
  {
    sock_tick (&sock, &status);

    if (sock_dataready(&sock) >= sizeof(host_tm))
    {
      time (now);
      sock_read (&sock, (BYTE*)&host_tm, sizeof(host_tm));

      host_tm = ntohl ((DWORD)host_tm);  /* convert to host order */
      sock_close (&sock);
      sock_wait_closed (&sock, sock_delay, NULL, &status);
      return (host_tm);
    }
  }

sock_err:
  switch (status)
  {
    case 1 : /* host closed */
         return (host_tm);
    case -1: /* timeout */
         puts (sockerr(&sock));
         return (0);
    default:
         puts ("Aborting");
         return (0);
  }
}

void usage (void)
{
  puts ("NTIME [-?vd] [-a min] server");
  puts ("  -?  this help");
  puts ("  -v  prints version information");
  puts ("  -d  enable TCP debugging");
  puts ("  -a  minutes to add to local clock");
  exit (0);
}

int main (int argc, char **argv)
{
  DWORD      host;
  char      *name     = NULL;
  DWORD      add_secs = 0L;
  time_t     newtime, loctime;
  struct tm  utime,  ltime;
  double     time_diff;
  int        ch;

  while ((ch = getopt(argc, argv, "?vda:")) != EOF)
     switch (ch)
     {
       case 'v':
            puts (wattcpVersion());
            puts (wattcpCopyright);
            break;
       case 'd':
            dbug_init();
            break;
       case 'a':
            add_secs = 60L * atol (optarg);
            break;
       default:
            usage();
     }

  argc -= optind;
  argv += optind;

  if (argc < 1 || (name = *argv) == NULL)
     usage();

  sock_init();

  host = lookup_host (name, NULL);
  if (host == 0L)
  {
    printf ("Unknown host `%s'\n", name);
    return (3);
  }

  newtime = ntime (host, &loctime);
  if (newtime == 0L)
  {
    printf ("Unable to get the time from that host\n");
    return (1);
  }

  loctime += add_secs;
  newtime -= BASE_TIME - add_secs;    /* time now in UNIX format */
  utime    = *localtime (&newtime);
  ltime    = *localtime (&loctime);

  printf ("Network (Unix) time: %.24s\n", asctime(&utime));
  printf ("Local   (DOS)  time: %.24s\n", asctime(&ltime));

  time_diff = difftime (mktime(&utime), mktime(&ltime));

  printf ("Time set to %.24s.\nCorrected by %.1f sec",
          asctime(&utime), time_diff);
  return (0);
}

