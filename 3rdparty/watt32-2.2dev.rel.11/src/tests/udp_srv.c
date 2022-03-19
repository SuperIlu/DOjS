/*
 * listener.c -- a datagram sockets "server" demo
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#ifdef __CYGWIN__
  extern int _w32_kbhit (void);
  #define kbhit() _w32_kbhit()
#else
  #include <conio.h>
#endif

#ifdef WATT32
  #include <tcp.h>
  #define close  close_s
  #define select select_s
#endif

#ifdef _Windows   /* Not Watt-32 on Windows! */
  #define WIN32_LEAN_AND_MEAN
  #include <winsock.h>
  #define close(s) closesocket(s)

  static struct WSAData wsa_state;

  static void cleanup (void)
  {
    WSACleanup();
  }
#else
  #ifndef _MSC_VER
  #include <unistd.h>
  #endif
  #include <netinet/in.h>
  #include <sys/socket.h>
  #include <sys/ioctl.h>
  #include <arpa/inet.h>
#endif

#define MYPORT 6543    /* the port users will be connecting to */

#define MAXBUFLEN 100

void usage (void)
{
  printf ("udp_srv [-dn]\n"
          "  listen for UDP-traffic on port %d\n"
          "  -d  enable Watt-32 debug\n"
          "  -n  use non-blocking socket\n", MYPORT);
  exit (0);
}

int main (int argc, char **argv)
{
  struct sockaddr_in my_addr;    /* my address information */
  struct sockaddr_in their_addr; /* connector's address information */
  int    sockfd;
  int    addr_len, numbytes;
  int    debug = 0;
  int    non_block = 0;
  char   buf[MAXBUFLEN];

  while (argc > 1)
  {
    if (!strcmp(argv[1],"-n"))
       non_block = 1;
    if (!strcmp(argv[1],"-d"))
       debug = 1;
    if (!strcmp(argv[1],"-?"))
       usage();
    argc--;
    argv++;
  }

#if defined(WATT32)
  if (debug)
     dbug_init();
  sock_init();

#elif defined(_Windows)
  memset (&wsa_state, 0, sizeof(wsa_state));
  if (WSAStartup(MAKEWORD(1,1),&wsa_state) != 0)
  {
    printf ("Unable to start WinSock, error code=%d\n", WSAGetLastError());
    return (0);
  }
  atexit (cleanup);
#endif

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
  {
    perror ("socket");
    return (1);
  }

  memset (&my_addr,0,sizeof(my_addr));  /* zero the rest of the struct */
  my_addr.sin_family = AF_INET;         /* host byte order */
  my_addr.sin_port   = htons(MYPORT);   /* short, network byte order */
  my_addr.sin_addr.s_addr = INADDR_ANY; /* automatically fill with my IP */

  if (bind(sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr)) == -1)
  {
    perror ("bind");
    return (1);
  }

#ifdef _Windows
  ioctlsocket (sockfd, FIONBIO, (u_long*)&non_block);
#else
  ioctlsocket (sockfd, FIONBIO, (char*)&non_block);
#endif

  printf ("Waiting for UDP messages at port %d...", MYPORT);
  fflush (stdout);

  while (!kbhit())
  {
    if (non_block)
    {
      fd_set fd;
      struct timeval tv = { 0, 10000 };
      int    n;

      FD_ZERO (&fd);
      FD_SET (sockfd, &fd);
      n = select (sockfd+1, &fd, NULL, NULL, &tv);
      if (n < 0)
      {
        perror ("select");
        return (1);
      }
      if (n == 0)
         continue;
    }

    /*  --- Wait for UDP message */

    addr_len = sizeof (struct sockaddr);
    numbytes = recvfrom (sockfd, buf, MAXBUFLEN, 0,
                         (struct sockaddr *)&their_addr, &addr_len);
    if (numbytes < 0)
    {
      perror ("recvfrom");
      return (1);
    }

    printf ("\007got packet from %s\n",inet_ntoa(their_addr.sin_addr));
    printf ("packet is %d bytes long\n",numbytes);
    buf[numbytes] = '\0';
    printf ("packet contains \"%s\"\n",buf);

    /*  --- Reply to this message (echo) */

    numbytes = sendto (sockfd, buf, numbytes, 0,
                       (struct sockaddr*)&their_addr, sizeof(struct sockaddr));
    if (numbytes < 0)
    {
      perror ("sendto");
      return (1);
    }
    printf ("sent %d bytes to %s\n", numbytes, inet_ntoa(their_addr.sin_addr));
  }

  close (sockfd);
  return (0);
}

