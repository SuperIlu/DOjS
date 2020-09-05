/*
 * Simple BSD-socket multicast test program (for djgpp only).
 * Must define USE_MULTICAST in ../config.h
 */

/*
 * sender.c -- multicasts "hello, world!" to a multicast group once a second
 *
 * Antony Courtney,     25/11/94
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <tcp.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#define HELLO_PORT  12345
#define HELLO_GROUP "225.0.0.37"
#define MSGBUFSIZE  256

#if defined(__CYGWIN__)
  #define strnicmp(s1, s2, len)  strncasecmp (s1, s2, len)
#endif

int sender_main (int argc, char **argv)
{
  struct sockaddr_in addr;
  char  *message = "Hello, World!";
  int    ttl, fd = socket (AF_INET, SOCK_DGRAM, 0);

  if (fd < 0)
  {
    perror ("socket");
    return (-1);
  }

  /* set up destination address */
  memset (&addr, 0, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = inet_addr (argc >= 1 ? argv[0] : HELLO_GROUP);
  addr.sin_port        = htons (argc >= 2 ? atoi(argv[1]) : HELLO_PORT);

  ttl = 32;  /* same site */
  if (setsockopt (fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0)
  {
    perror ("setsockopt");
    return (-1);
  }

//tcp_cbreak (1);

  /* now just sendto() our destination! */
  while (1)
  {
    if (sendto (fd, message, strlen(message), 0,
                (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
      perror ("sendto");
      return (-1);
    }
    fputc ('.', stderr);
#ifdef WIN32
    Sleep (1000);
#else
    sleep (1);
#endif

#if 0
    if (_watt_cbroke)
       break;
#endif
  }
  return (0);
}

/*
 * listener.c -- joins a multicast group and echoes all data it receives from
 *              the group to its stdout...
 *
 * Antony Courtney,     25/11/94
 */
int listener_main (int argc, char **argv)
{
  struct sockaddr_in addr;
  struct ip_mreq     mreq;
  char   message [MSGBUFSIZE];
  int    nbytes, addrlen, fd = socket (AF_INET, SOCK_DGRAM, 0);

  if (fd < 0)
  {
    perror ("socket");
    return (-1);
  }

  /* set up destination address */
  memset (&addr, 0, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = htonl (INADDR_ANY); /* N.B.: differs from sender */
  addr.sin_port        = htons (argc >= 2 ? atoi(argv[1]) : HELLO_PORT);

  /* bind to receive address */
  if (bind (fd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
  {
    perror ("bind");
    return (-1);
  }

  /* use setsockopt() to request that the kernel join a multicast group */
  mreq.imr_multiaddr.s_addr = inet_addr (argc >= 1 ? argv[0] : HELLO_GROUP);
  mreq.imr_interface.s_addr = htonl (INADDR_ANY);
  if (setsockopt (fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
  {
    perror ("setsockopt");
    return (-1);
  }

//tcp_cbreak (1);

  /* now just enter a read-print loop */
  while (1)
  {
    addrlen = sizeof (addr);
    nbytes  = recvfrom (fd, message, MSGBUFSIZE, 0,
                        (struct sockaddr*)&addr, &addrlen);
    if (nbytes < 0)
    {
      perror ("recvfrom");
      return (-1);
    }
    puts (message);
#if 0
    if (_watt_cbroke)
       break;
#endif
  }
  return (0);
}

int main (int argc, char **argv)
{
  dbug_init();

  if (argc > 1 && !strnicmp(argv[1],"send",4))
     return sender_main (argc-2, argv+2);

  if (argc > 1 && !strnicmp(argv[1],"listen",6))
     return listener_main (argc-2, argv+2);

  printf ("Usage: %s [listen | send] ..\n", argv[0]);
  return (-1);
}
