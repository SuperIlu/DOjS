#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tcp.h>

#include <sys/socket.h>
#include <net/if.h>
#include <net/if_ether.h>
#include <net/if_packet.h>

#ifdef __CYGWIN__
  W32_FUNC int _w32_getch (void);
  #define getch() _w32_getch()
#else
  #include <conio.h>
#endif

#define TRUE  1
#define FALSE 0

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif

static const char *pkt_type (int type)
{
  switch (type)
  {
    case PACKET_HOST:
         return ("host ");
    case PACKET_BROADCAST:
         return ("bcast");
    case PACKET_MULTICAST:
         return ("mcast");
    case PACKET_OTHERHOST:
         return ("other");
    case PACKET_OUTGOING:
         return ("out  ");
    case PACKET_LOOPBACK:
         return ("loop ");
    case PACKET_FASTROUTE:
         return ("fastr");
  }
  return ("??");
}

static void print_header (void)
{
  puts ("sk proto: type , eth-src");
}

static int packet_recv (int s)
{
  struct sockaddr_ll addr;
  const struct ether_header eth;
  int   addr_len = sizeof(addr);
  int   len = recvfrom (s, (char*)&eth, sizeof(eth), 0,
                        (struct sockaddr*)&addr, &addr_len);

  printf ("%d  %04Xh: %s, %02X:%02X:%02X:%02X:%02X:%02X\n",
          s, ntohs(addr.sll_protocol), pkt_type(addr.sll_pkttype),
          eth.ether_shost[0], eth.ether_shost[1],
          eth.ether_shost[2], eth.ether_shost[3],
          eth.ether_shost[4], eth.ether_shost[5]);
  return (len);
}

int main (int argc, char **argv)
{
  BOOL quit = FALSE;
  int  s1, s2;

  printf ("Linux style AF_PACKET example. Press ESC to quit\n");

  if (argc > 1 && !strcmp(argv[1],"-d"))
     dbug_init();

  s1 = socket (AF_PACKET, SOCK_PACKET, 0);
  if (s1 < 0)
  {
    perror ("socket");
    return (-1);
  }
  s2 = socket (AF_PACKET, SOCK_PACKET, 0);
  if (s2 < 0)
  {
    perror ("socket");
    close_s (s1);
    return (-1);
  }

  print_header();

  while (!quit)
  {
    struct timeval tv = { 1, 0 };
    fd_set rd;
    int    num;

    FD_ZERO (&rd);
    FD_SET (s1, &rd);
    FD_SET (s2, &rd);
    FD_SET (STDIN_FILENO, &rd);
    num = max (s1,s2) + 1;

    if (select_s(num, &rd, NULL, NULL, &tv) < 0)
    {
      perror ("select_s");
      quit = TRUE;
    }

    if (FD_ISSET(STDIN_FILENO,&rd) && getch() == 27)
       quit = TRUE;

    if (FD_ISSET(s1,&rd))
       packet_recv (s1);

    if (FD_ISSET(s2,&rd))
       packet_recv (s2);
  }

  close_s (s1);
  close_s (s2);
  return (0);
}

