
/******************************************************************************
 DAYSRV - Example of a UDP daytime server.
 Other machines can access this machine's time of day by running the daytime
 program.
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <time.h>
#include <tcp.h>
#include <netdb.h>

#define BROADCAST_ADR 0xFFFFFFFF

#if (defined(__SMALL__) && !defined(DOS386)) || \
    defined(__LARGE__)
  #define SAVE_SPACE
#endif

struct servent *serv;
WORD   ourPort = 13;      /* Default DAYTIME port */
char   debug   = 0;
DWORD  reply_host;
WORD   reply_port;

int dayreply (DWORD host, WORD port)
{
  static udp_Socket telsock;
  udp_Socket *s = &telsock;
  time_t now;
  char   *now_s, *nl;
  char   buf [256];

  if (debug)
     printf ("dayreply: host (%d.%d.%d.%d), port %d\n",
             ((BYTE*)&host)[0],
             ((BYTE*)&host)[1],
             ((BYTE*)&host)[2],
             ((BYTE*)&host)[3], port);

  if (!udp_open(s,port,htonl(host), htons(port),NULL))
  {
    puts("Sorry, unable to connect to that machine right now!");
    return 3;
  }

  time(&now);
  now_s = asctime(localtime(&now));

  if ((nl = strchr(now_s,'\n')) != NULL)
     *nl = 0;
  sprintf (buf, "%s\r\n", now_s);
  sock_write (s, (const BYTE*)buf, strlen(buf)); /* Probably not quite in RFC format */

  printf ("dayreply: sock_write(s,%s,%d)\n", buf, (int)strlen(buf));
  sock_close (s);
  return 0;
}

/*-------------------------------------------------------------------------
 Function called whenever we receive a packet on the listening socket.
-------------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

int dataHandler (void *s, const BYTE *dp, size_t len, DWORD *phSrc, WORD *upSrcPort)
{
  if (debug)
     printf ("dataHandler: src %p, %u.%u.%u.%u, len %u\n",
             phSrc, ((BYTE*)&phSrc)[0], ((BYTE*)&phSrc)[1],
                    ((BYTE*)&phSrc)[2], ((BYTE*)&phSrc)[3], (unsigned)len);
  reply_host = *phSrc;
  reply_port = *upSrcPort;
  (void)s;
  (void)dp;
  return 0;
}

int daysrv (void)
{
  static udp_Socket telsock;
  udp_Socket *s = &telsock;

  s = &telsock;
  if (!udp_open(s,ourPort,BROADCAST_ADR,0, (ProtoHandler)dataHandler))
  {
    puts ("Sorry, unable to open listener port.\n");
    return 3;
  }

  reply_host = 0;
  while (!watt_kbhit())
  {
    tcp_tick(s);
    /* If a packet comes in, tcp_tick will call dataHandler,
     * which will record who sent the packet in reply_*.
     */
    if (reply_host)
    {
      dayreply (reply_host, reply_port);
      reply_host = 0;
    }
  }
  sock_close(s);
  return 0;
}


int MS_CDECL main (int argc, char **argv)
{
  if (argc > 1 && argv[1][1] == '?')
  {
    puts("DAYSRV [-d]");
    puts("Listen for connections on the daytime UDP port, "\
         "and send them our time.");
    return 0;
  }
  else if (argc > 1 && argv[1][1] == 'd')
  {
    debug = 1;
    tcp_set_debug_state(debug);
    dbug_init();
  }
  sock_init();

#ifndef SAVE_SPACE
  if ((serv = getservbyname("daytime",NULL)) != NULL)
     ourPort = intel16 (serv->s_port);
#endif

  return daysrv();
}
