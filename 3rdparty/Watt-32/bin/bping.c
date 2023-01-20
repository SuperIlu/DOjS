/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Mike Muuss.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *  This product includes software developed by the University of
 *  California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 *      (B S D) P I N G . C
 *
 * Using the InterNet Control Message Protocol (ICMP) "ECHO" facility,
 * measure round-trip-delays and packet loss across network paths.
 *
 * Author -
 *  Mike Muuss
 *  U. S. Army Ballistic Research Laboratory
 *  December, 1983
 *
 * Status -
 *  Public Domain.  Distribution Unlimited.
 * Bugs -
 *  More statistics could always be gathered.
 *  This program has to run SUID to ROOT to access the ICMP socket.
 *
 * Rewritten/adapted for Waterloo/TCP by
 *  G. Vanem 1997
 *
 * Added support for AF_INET6 (IPv6) sockets
 *  G. Vanem 2002
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <malloc.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
#include <process.h>
#include <dos.h>
#include <io.h>

#include <sys/socket.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip_var.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <netdb.h>
#include <tcp.h>

#if defined(__GNUC__) || defined(__HIGHC__)
#include <unistd.h>
#endif

#if defined(__HIGHC__)
#pragma Offwarn (39)
#pragma Offwarn (572)
#endif

#define  DEFDATALEN  (64 - ICMP_MINLEN)  /* default data length */
#define  MAXIPLEN    60                  /* IP-header with max # of options */
#define  MAXICMPLEN  76
#define  MAXPACKET   (USHRT_MAX - 60 - ICMP_MINLEN)

#define  MAXWAIT     10                  /* max seconds to wait for response */
#define  NROUTES     9                   /* number of record route slots */

#define  A(bit)    rcvd_tbl[(bit)>>3]    /* identify byte in array */
#define  B(bit)    (1 << ((bit) & 0x07)) /* identify bit in byte */
#define  SET(bit)  (A(bit) |= B(bit))
#define  CLR(bit)  (A(bit) &= (~B(bit)))
#define  TST(bit)  (A(bit) & B(bit))

union ICMP_pkt {
      struct icmp      ip4;
      struct icmp6_hdr ip6;
    };

/* various options
 */
int options;
#define  F_FLOOD         0x001
#define  F_INTERVAL      0x002
#define  F_NUMERIC       0x004
#define  F_PINGFILLED    0x008
#define  F_QUIET         0x010
#define  F_RROUTE        0x020
#define  F_SO_DEBUG      0x040
#define  F_SO_DONTROUTE  0x080
#define  F_VERBOSE       0x100

/*
 * MAX_DUP_CHK is the number of bits in received table, i.e. the maximum
 * number of received sequence numbers we can keep track of.  Change 128
 * to 8192 for complete accuracy...
 */
#define  MAX_DUP_CHK  (8 * 128)
int  mx_dup_ck = MAX_DUP_CHK;
char rcvd_tbl [MAX_DUP_CHK / 8];

struct sockaddr_in  whereto4; /* who to ping (IPv4 address) */
struct sockaddr_in6 whereto6; /* who to ping (IPv6 address) */

unsigned datalen = DEFDATALEN;
int      s;                     /* socket file descriptor */
int      ident;                 /* id to identify our packets */
int      use_ip6 = 0;           /* use IPv6 address */
u_char   outpack [MAXPACKET];
char     BSPACE = '\b';         /* characters written for flood */
char     DOT    = '.';
char    *hostname;

/*
 * counters
 */
long npackets;           /* max packets to transmit */
long nreceived;          /* # of packets we got back */
long nrepeats;           /* number of duplicates */
long ntransmitted;       /* sequence # for outbound packets = #sent */
int  interval = 1;       /* interval between packets */

/*
 * timing
 */
int    timing;           /* flag to do timing */
long   tmin = LONG_MAX;  /* minimum round trip time */
long   tmax;             /* maximum round trip time */
u_long tsum;             /* sum of all times, for doing average */

/*
 * Prototypes
 */
char   *pr_addr4 (u_long);
char   *pr_addr6 (const void *);
void    catcher  (int);
void    finish   (int);
void    pinger   (void);
void    beep     (void);
void    tvsub    (struct timeval *out, struct timeval *in);
void    pr_icmp4 (struct icmp *icp);
void    pr_icmp6 (struct icmp6_hdr *icp);
void    pr_retip (struct ip *ip);
void    pr_pack  (u_char *buf, int cc, struct sockaddr_in*, struct sockaddr_in6*);
void    fill     (char *bp, char *patp);
u_short in4_cksum (u_short *addr, int len);
u_short in6_cksum (u_short *addr, int len);

extern const void *_gethostid6 (void);

char *usage = "usage: ping [-Rdfnqrv6] [-c count] [-i wait] "
              "[-l preload]\n\t[-p pattern] [-s packetsize] host\n";

void Exit (const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  va_end (args);
  exit (1);
}

void dump_data (const char *packet, int len)
{
  while (len--)
    printf ("%02X ", (*packet++) & 0xFF);
}

int main (int argc, char **argv)
{
  struct  timeval       timeout;
  struct  hostent      *hp;
  struct  sockaddr_in  *to_ip4;
  struct  sockaddr_in6 *to_ip6;
  struct  protoent     *proto;

  int     ch, hold, packlen;
  int     preload = 0;
  u_char *datap   = &outpack [ICMP_MINLEN + sizeof(struct timeval)];
  u_char *packet;
  char   *target;
  char    rspace  [3 + 4*NROUTES + 1];  /* record route space */

  while ((ch = getopt(argc, argv, "Rc:dfh:i:l:np:qrs:6v?")) != EOF)
    switch (ch)
    {
      case 'c':
           npackets = atoi (optarg);
           if (npackets <= 0)
              Exit ("ping: bad number of packets to transmit.\n");
           break;
      case 'd':
           options |= F_SO_DEBUG;
           dbug_init();
           break;
      case 'f':
           options |= F_FLOOD;
           setbuf (stdout,NULL);
           break;
      case 'i':    /* wait between sending packets */
           interval = atoi (optarg);
           if (interval <= 0)
              Exit ("ping: bad timing interval.\n");
           options |= F_INTERVAL;
           break;
      case 'l':
           preload = atoi (optarg);
           if (preload < 0)
              Exit ("ping: bad preload value.\n");
           break;
      case 'n':
           options |= F_NUMERIC;
           break;
      case 'p':    /* fill buffer with user pattern */
           options |= F_PINGFILLED;
           fill ((char*)datap, optarg);
           break;
      case 'q':
           options |= F_QUIET;
           break;
      case 'R':
           options |= F_RROUTE;
           break;
      case 'r':
           options |= F_SO_DONTROUTE;
           break;
      case 's':    /* size of packet to send */
           datalen = atoi (optarg);
           if (datalen > MAXPACKET)
              Exit ("ping: packet size too large.\n");
           if (datalen <= 0)
              Exit ("ping: illegal packet size.\n");
           break;
      case '6':
           use_ip6 = 1;
           break;
      case 'v':
           options |= F_VERBOSE;
           break;
      case '?':
      default:
           Exit (usage);
    }

  argc -= optind;
  argv += optind;

  if (argc != 1)
     Exit (usage);

  target = *argv;
  sock_init();

  memset (&whereto4, 0, sizeof(whereto4));
  memset (&whereto6, 0, sizeof(whereto6));
  to_ip4 = &whereto4;
  to_ip6 = &whereto6;

  if (options | F_VERBOSE)
  {
    printf ("Resolving %s...", target);
    fflush (stdout);
  }

  if (use_ip6)
  {
    to_ip6->sin6_family = AF_INET6;
    if (inet_pton(AF_INET6, target, &to_ip6->sin6_addr) == 1)
       hostname = target;
    else
    {
      char hnamebuf[MAXHOSTNAMELEN];

      hp = gethostbyname2 (target, AF_INET6);
      if (!hp)
         Exit ("ping: unknown host %s\n", target);

      to_ip6->sin6_family = hp->h_addrtype;
      memcpy (&to_ip6->sin6_addr, hp->h_addr, hp->h_length);
      strncpy (hnamebuf, hp->h_name, sizeof(hnamebuf)-1);
      hnamebuf [sizeof(hnamebuf)-1] = '\0';
      hostname = hnamebuf;
    }
  }
  else
  {
    to_ip4->sin_family      = AF_INET;
    to_ip4->sin_addr.s_addr = inet_addr (target);
    if (to_ip4->sin_addr.s_addr != INADDR_NONE)
       hostname = target;
    else
    {
      char hnamebuf[MAXHOSTNAMELEN];

      hp = gethostbyname (target);
      if (!hp)
         Exit ("ping: unknown host %s\n", target);

      to_ip4->sin_family = hp->h_addrtype;
      memcpy (&to_ip4->sin_addr, hp->h_addr, hp->h_length);
      strncpy (hnamebuf, hp->h_name, sizeof(hnamebuf)-1);
      hnamebuf [sizeof(hnamebuf)-1] = '\0';
      hostname = hnamebuf;
    }
  }

  if ((options & F_FLOOD) && (options & F_INTERVAL))
     Exit ("ping: -f and -i incompatible options.\n");

  if (datalen >= sizeof(struct timeval))  /* can we time transfer */
     timing = 1;

  packlen = datalen + MAXIPLEN + MAXICMPLEN;
  packet  = malloc (packlen);
  if (!packet)
     Exit ("ping: out of memory.\n");

  if (!(options & F_PINGFILLED))
  {
    unsigned i;
    for (i = ICMP_MINLEN; i < datalen; i++)
        *datap++ = i;
  }

  ident = getpid() & 0xFFFF;
  proto = use_ip6 ? getprotobyname ("icmpv6") : getprotobyname ("icmp");
  if (!proto)
     Exit ("ping: unknown protocol icmp.\n");

  if (use_ip6)
       s = socket (AF_INET6, SOCK_RAW, proto->p_proto);
  else s = socket (AF_INET, SOCK_RAW, proto->p_proto);
  if (s < 0)
  {
    perror ("ping: socket");
    return (1);
  }

  hold = 1;
  if (options & F_SO_DEBUG)
     setsockopt (s, SOL_SOCKET, SO_DEBUG, (char*)&hold, sizeof(hold));

  if (options & F_SO_DONTROUTE)
     setsockopt (s, SOL_SOCKET, SO_DONTROUTE, (char*)&hold, sizeof(hold));

  /* record route option
   */
  if (options & F_RROUTE)
  {
    rspace[IPOPT_OPTVAL] = IPOPT_RR;
    rspace[IPOPT_OLEN]   = sizeof(rspace)-1;
    rspace[IPOPT_OFFSET] = IPOPT_MINOFF;
    if (setsockopt(s, IPPROTO_IP, IP_OPTIONS, rspace, sizeof(rspace)) < 0)
    {
      perror ("ping: record route");
      return (1);
    }
  }

  /*
   * When pinging the broadcast address, you can get a lot of answers.
   * Doing something so evil is useful if you are trying to stress the
   * ethernet, or just want to fill the arp cache to get some stuff for
   * /etc/ethers.
   */
  hold = 48 * 1024;
  setsockopt (s, SOL_SOCKET, SO_RCVBUF, (char*)&hold, sizeof(hold));

  if (!use_ip6)
  {
    if (to_ip4->sin_family == AF_INET)
         printf ("ping %s (%s): %u data bytes\n", hostname,
                 inet_ntoa(to_ip4->sin_addr), datalen);
    else printf ("ping %s: %u data bytes\n", hostname, datalen);
  }
  else
  {
    char buf[50];
    if (inet_ntop (AF_INET6, &to_ip6->sin6_addr, buf, sizeof(buf)))
         printf ("ping %s (%s): %u data bytes\n", hostname, buf, datalen);
    else printf ("ping %s: %u data bytes\n", hostname, datalen);
  }

  signal (SIGINT, finish);

  while (preload--)    /* fire off them quickies */
     pinger();

  if ((options & F_FLOOD) == 0)
     catcher (0);    /* start things going */

  for (;;)
  {
    struct sockaddr_in  from4;
    struct sockaddr_in6 from6;
    int    cc, fromlen;

    if (options & F_FLOOD)
    {
      fd_set fdmask;

      pinger();
      timeout.tv_sec  = 0;
      timeout.tv_usec = 10000;
      FD_ZERO (&fdmask);
      FD_SET (s, &fdmask);
      if (select_s(s+1,&fdmask,NULL,NULL,&timeout) < 1)
         continue;
    }
    if (use_ip6)
    {
      fromlen = sizeof(from6);
      cc = recvfrom (s, packet, packlen, 0, (struct sockaddr*)&from6, &fromlen);
    }
    else
    {
      fromlen = sizeof(from4);
      cc = recvfrom (s, packet, packlen, 0, (struct sockaddr*)&from4, &fromlen);
    }
    if (cc < 0)
    {
      if (errno == EINTR)
         break;
      perror ("ping: recvfrom");
      continue;
    }
    if (use_ip6)
         pr_pack (packet, cc, NULL, &from6);
    else pr_pack (packet, cc, &from4, NULL);
    if (npackets && nreceived >= npackets)
       break;
  }
  finish (0);
  return (0);
}

/*
 * catcher --
 *  This routine causes another PING to be transmitted, and then
 *  schedules another SIGALRM for 1 second from now.
 * 
 * bug --
 *  Our sense of time will slowly skew (i.e., packets will not be
 *  launched exactly at 1-second intervals).  This does not affect the
 *  quality of the delay and loss statistics.
 */
void catcher (int sig)
{
  int waittime;

  if (sig == SIGALRM)
     beep();

  pinger();

  if (!npackets || ntransmitted < npackets)
  {
    signal (SIGALRM, catcher);
    alarm (interval);
  }
  else
  {
    if (nreceived)
    {
      waittime = 2 * tmax / 1000;
      if (waittime == 0)
          waittime = 1;
    }
    else
      waittime = MAXWAIT;
    signal (SIGALRM, finish);
    alarm (waittime);
  }
}

/*
 * pinger --
 *   Compose and transmit an ICMP ECHO REQUEST packet.  The IP packet
 * will be added on by the kernel.  The ID field is our UNIX process ID,
 * and the sequence number is an ascending integer.  The first 8 bytes
 * of the data portion are used to hold a UNIX "timeval" struct in VAX
 * byte-order, to compute the round-trip time.
 */
void pinger (void)
{
  volatile union ICMP_pkt *icp;
  volatile int    cc, len, i;
  const struct sockaddr *where;

  icp = (union ICMP_pkt*)outpack;
  if (use_ip6)
  {
    icp->ip6.icmp6_type  = ICMP6_ECHO_REQUEST;
    icp->ip6.icmp6_code  = 0;
    icp->ip6.icmp6_cksum = 0;
    icp->ip6.icmp6_seq   = ntransmitted++;
    icp->ip6.icmp6_id    = ident;      /* ID */
    CLR (icp->ip6.icmp6_seq % mx_dup_ck);
    where = (const struct sockaddr*) &whereto6;
    len = sizeof (whereto6);
  }
  else
  {
    icp->ip4.icmp_type  = ICMP_ECHO;
    icp->ip4.icmp_code  = 0;
    icp->ip4.icmp_cksum = 0;
    icp->ip4.icmp_seq   = ntransmitted++;
    icp->ip4.icmp_id    = ident;      /* ID */
    CLR (icp->ip4.icmp_seq % mx_dup_ck);
    where = (const struct sockaddr*) &whereto4;
    len = sizeof (whereto4);
  }

  if (timing)
     gettimeofday ((struct timeval*)&outpack[ICMP_MINLEN], NULL);

  cc = datalen + ICMP_MINLEN;      /* skips ICMP portion */

  /* compute ICMP checksum here
   */
  if (use_ip6)
  {
    icp->ip6.icmp6_cksum = 0;
    icp->ip6.icmp6_cksum = in6_cksum ((u_short*)icp, cc);
  }
  else
  {
    icp->ip4.icmp_cksum = 0;
    icp->ip4.icmp_cksum = in4_cksum ((u_short*)icp, cc);

  }

  i = sendto (s, (char*)outpack, cc, 0, where, len);

  if (i < 0 || i != cc)
  {
    if (i < 0)
       perror ("ping: sendto");
    printf ("ping: wrote %s %d chars, ret=%d\n", hostname, cc, i);
  }
  if (!(options & F_QUIET) && (options & F_FLOOD))
     write (fileno(stdout), &DOT, 1);
}

/*
 * pr_pack --
 *  Print out the packet, if it came from us.  This logic is necessary
 * because ALL readers of the ICMP socket get a copy of ALL ICMP packets
 * which arrive ('tis only fair).  This permits multiple copies of this
 * program to be run without having intermingled output (or statistics!).
 */
void pr_pack (u_char *buf, int cc,
              struct sockaddr_in *from4, struct sockaddr_in6 *from6)
{
  static u_long old_rrlen;
  static char   old_rr[MAX_IPOPTLEN];
  struct icmp      *icp4;
  struct icmp6_hdr *icp6;
  struct ip        *ip  = NULL;
  struct ip6_hdr   *ip6 = NULL;
  struct timeval    tv, *tp;
  u_long l, i, j;
  u_char *cp, *dp;
  u_short seq;
  long    triptime = 0;
  int     hlen, dupflag;
  char    addr[50] = "?";

  gettimeofday (&tv, NULL);
 
  if (from4)   /* IPv4 echo-response */
  {
    ip   = (struct ip*)buf;
    hlen = ip->ip_hl << 2;
    inet_ntop (AF_INET, &from4->sin_addr, addr, sizeof(addr));
    icp4 = (struct icmp*) (buf + hlen);
    icp6 = NULL;
  }
  else
  {
    ip6  = (struct ip6_hdr*)buf;
    hlen = sizeof(*ip6);
    inet_ntop (AF_INET6, &from6->sin6_addr, addr, sizeof(addr));
    icp6 = (struct icmp6_hdr*) (buf + hlen);
    icp4 = NULL;
  }

  /* Check the IP header
   */
  if (cc < hlen + ICMP_MINLEN)
  {
    if (options & F_VERBOSE)
       fprintf (stderr, "ping: packet too short (%d bytes) from %s\n",
                cc, addr);
    return;
  }

  /* Now the ICMP part
   */
  cc -= hlen;
  if ((icp4 && icp4->icmp_type == ICMP_ECHOREPLY) ||
      (icp6 && icp6->icmp6_type == ICMP6_ECHO_REPLY))
  {
    if ((icp4 && icp4->icmp_id != ident) ||
        (icp6 && icp6->icmp6_id != ident))
       return;      /* That was not our ECHO */

    ++nreceived;
    if (timing)
    {
      tp = (struct timeval*) icp4->icmp_data;
      tvsub (&tv, tp);
      triptime = tv.tv_sec * 1000 + (tv.tv_usec / 1000);
      tsum += triptime;
      if (triptime < tmin)
          tmin = triptime;
      if (triptime > tmax)
          tmax = triptime;
    }
    seq = icp4 ? icp4->icmp_seq : icp6->icmp6_seq;

    if (TST(seq % mx_dup_ck))
    {
      ++nrepeats;
      --nreceived;
      dupflag = 1;
    }
    else
    {
      SET (seq % mx_dup_ck);
      dupflag = 0;
    }

    if (options & F_QUIET)
       return;

    if (options & F_FLOOD)
       write (fileno(stdout), &BSPACE, 1);
    else
    {
      if (icp4)
           printf ("%d bytes from %s: icmp_seq=%u ttl=%d", cc,
                   addr, seq, ip->ip_ttl);
      else printf ("%d bytes from %s: icmp_seq=%u hop=%d", cc,
                   addr, seq, ip6->ip6_hops);
      if (timing)
         printf (" time=%ld ms", triptime);
      if (dupflag)
         printf (" (DUP!)");

      /* check the data
       */
      cp = icp4 ? (u_char*) (icp4->icmp_data + ICMP_MINLEN) :
                  (u_char*) (icp6+1);
      dp = &outpack [ICMP_MINLEN + sizeof(struct timeval)];
      for (i = ICMP_MINLEN; i < datalen; ++i, ++cp, ++dp)
      {
        if (*cp != *dp)
        {
          printf ("\nwrong data byte #%lu should be 0x%x but was 0x%x",
                  i, *dp, *cp);
          cp = (u_char*) icp4->icmp_data;
          for (i = ICMP_MINLEN; i < datalen; ++i, ++cp)
          {
            if ((i % 32) == 8)
               printf ("\n\t");
            printf ("%x ", *cp);
          }
          break;
        }
      }
    }
  }
  else
  {
    /* We've got something other than an ECHOREPLY
     */
    if (!(options & F_VERBOSE))
       return;

    if (icp6)
    {
      printf ("%d bytes from %s: ", cc, pr_addr6(&from6->sin6_addr));
      pr_icmp6 (icp6);
    }
    else
    {
      printf ("%d bytes from %s: ", cc, pr_addr4(from4->sin_addr.s_addr));
      pr_icmp4 (icp4);
    }
  }

  /* Display any IPv4 options (IPv6 options not supported)
   */
  if (from4)
  {
    cp = buf + sizeof(struct ip);

    for ( ; hlen > (int)sizeof(struct ip); --hlen, ++cp)
      switch (*cp)
      {
        case IPOPT_EOL:
             hlen = 0;
             break;
        case IPOPT_LSRR:
             printf ("\nLSRR: ");
             hlen -= 2;
             j = *++cp;
             ++cp;
             if (j > IPOPT_MINOFF)
               for (;;)
               {
                 GETLONG (l,cp);
                 if (l == 0)
                      printf ("\t0.0.0.0");
                 else printf ("\t%s", pr_addr4(l));
                 hlen -= 4;
                 j    -= 4;
                 cp   += 4;
                 if (j <= IPOPT_MINOFF)
                    break;
                 putchar ('\n');
               }
             break;
        case IPOPT_RR:
             j = *++cp;    /* get length */
             i = *++cp;    /* and pointer */
             hlen -= 2;
             if (i > j)
                 i = j;
             i -= IPOPT_MINOFF;
             if (i <= 0)
                continue;
             if (i == old_rrlen &&
                 cp == (u_char *)buf + sizeof(struct ip) + 2 &&
                 !memcmp(cp, old_rr, i) && !(options & F_FLOOD))
             {
               printf ("\t(same route)");
               i = ((i + 3) / 4) * 4;
               hlen -= i;
               cp   += i;
               break;
             }
             old_rrlen = i;
             memcpy (old_rr, cp, i);
             printf ("\nRR: ");
             for (;;)
             {
               GETLONG (l,cp);
               if (l == 0)
                    printf ("\t0.0.0.0");
               else printf ("\t%s", pr_addr4(l));
               hlen -= 4;
               i    -= 4;
               cp   += 4;
               if (i <= 0)
                  break;
               putchar ('\n');
             }
             break;
        case IPOPT_NOP:
             printf ("\nNOP");
             break;
        default:
             printf ("\nunknown option %x", *cp);
             break;
      }
  }

  if (!(options & F_FLOOD))
  {
    putchar ('\n');
    fflush (stdout);
  }
}

/*
 * in4_cksum --
 *  Checksum routine for Internet Protocol family headers (C Version)
 */
u_short in4_cksum (u_short *addr, int len)
{
  int     nleft  = len;
  u_short *w     = addr;
  int     sum    = 0;
  u_short answer = 0;

  /*
   * Our algorithm is simple, using a 32 bit accumulator (sum), we add
   * sequential 16 bit words to it, and at the end, fold back all the
   * carry bits from the top 16 bits into the lower 16 bits.
   */
  while (nleft > 1)
  {
    sum += *w++;
    nleft -= 2;
  }

  /* mop up an odd byte, if necessary
   */
  if (nleft == 1)
  {
    *(u_char*)&answer = *(u_char*)w;
    sum += answer;
  }

  /* add back carry outs from top 16 bits to low 16 bits
   */
  sum  = (sum >> 16) + (sum & 0xFFFF);  /* add hi 16 to low 16 */
  sum += (sum >> 16);                   /* add carry */
  answer = ~sum;                        /* truncate to 16 bits */
  return (answer);
}

#include <sys/pack_on.h>

struct tcp_PseudoHeader6 {
       u_char  src[16];
       u_char  dst[16];
       u_short length;
       u_char  zero[3];
       u_char  next_hdr;
     };

#include <sys/pack_off.h>

u_short in6_cksum (u_short *addr, int len)
{
  struct tcp_PseudoHeader6 ph;
  u_long chksum;

  memset (&ph, 0, sizeof(ph));
  memcpy (&ph.src, _gethostid6(), sizeof(ph.src));
  memcpy (&ph.dst, &whereto6.sin6_addr, sizeof(ph.dst));
  ph.length   = ntohs (len);
  ph.next_hdr = IPPROTO_ICMPV6;

  chksum  = in_checksum ((u_short*)&ph, sizeof(ph));
  chksum += in_checksum (addr, len);

  /* Wrap in the carries to reduce chksum to 16 bits.
   */
  chksum  = (chksum >> 16) + (chksum & 0xFFFF);
  chksum += (chksum >> 16);

  /* Take ones-complement and replace 0 with 0xFFFF.
   */
  chksum = (u_short)~chksum;
  if (chksum == 0UL)
     chksum = 0xFFFFUL;
  return (u_short)chksum;
}

/*
 * tvsub --
 *  Subtract 2 timeval structs:  out = out - in.  Out is assumed to
 * be >= in.
 */
void tvsub (struct timeval *out, struct timeval *in)
{
  if ((out->tv_usec -= in->tv_usec) < 0)
  {
    out->tv_sec--;
    out->tv_usec += 1000000;
  }
  out->tv_sec -= in->tv_sec;
}

/*
 * finish --
 *  Print out statistics, and give up.
 */
void finish (int sig)
{
  if (sig == SIGALRM)
     beep();

  signal (SIGINT, SIG_IGN);
  putchar ('\n');
  fflush (stdout);
  printf ("--- %s ping statistics ---\n", hostname);
  printf ("%ld packets transmitted, ", ntransmitted);
  printf ("%ld packets received, ", nreceived);
  if (nrepeats)
     printf ("+%ld duplicates, ", nrepeats);

  if (ntransmitted)
  {
    double loss = (((double)(ntransmitted - nreceived) * 100.0) / (double)ntransmitted);

    if (nreceived > ntransmitted)
         printf ("-- somebody's printing up packets!");
    else printf ("%.2f%% packet loss", loss);
  }
  putchar ('\n');
  if (nreceived && timing)
     printf ("round-trip min/avg/max = %ld/%lu/%ld ms\n",
             tmin, tsum / (nreceived + nrepeats), tmax);

  alarm (0);
  close_s (s);
  exit (0);
}


/*
 * pr_icmp4 --
 *  Print a descriptive string about an ICMPv4 header.
 */
void pr_icmp4 (struct icmp *icp)
{
  switch (icp->icmp_type)
  {
    case ICMP_ECHOREPLY:
         printf ("Echo Reply\n");
         /* XXX ID + Seq + Data */
         break;

    case ICMP_UNREACH:
         switch (icp->icmp_code)
         {
           case ICMP_UNREACH_NET:
                printf ("Destination Net Unreachable\n");
                break;
           case ICMP_UNREACH_HOST:
                printf ("Destination Host Unreachable\n");
                break;
           case ICMP_UNREACH_PROTOCOL:
                printf ("Destination Protocol Unreachable\n");
                break;
           case ICMP_UNREACH_PORT:
                printf ("Destination Port Unreachable\n");
                break;
           case ICMP_UNREACH_NEEDFRAG:
                printf ("frag needed and DF set\n");
                break;
           case ICMP_UNREACH_SRCFAIL:
                printf ("Source Route Failed\n");
                break;
           default:
                printf ("Dest Unreachable, Bad Code: %d\n", icp->icmp_code);
                break;
         }
         /* Print returned IP header information */
         pr_retip ((struct ip *)icp->icmp_data);
         break;

    case ICMP_SOURCEQUENCH:
         printf ("Source Quench\n");
         pr_retip ((struct ip *)icp->icmp_data);
         break;

    case ICMP_REDIRECT:
         switch (icp->icmp_code)
         {
           case ICMP_REDIRECT_NET:
                printf ("Redirect Network");
                break;
           case ICMP_REDIRECT_HOST:
                printf ("Redirect Host");
                break;
           case ICMP_REDIRECT_TOSNET:
                printf ("Redirect Type of Service and Network");
                break;
           case ICMP_REDIRECT_TOSHOST:
                printf ("Redirect Type of Service and Host");
                break;
           default:
                printf ("Redirect, Bad Code: %d", icp->icmp_code);
                break;
         }
         printf ("(New addr: 0x%08lx)\n", icp->icmp_gwaddr.s_addr);
         pr_retip ((struct ip *)icp->icmp_data);
         break;

    case ICMP_ECHO:
         printf ("Echo Request\n");
         /* XXX ID + Seq + Data */
         break;

    case ICMP_TIMXCEED:
         switch (icp->icmp_code)
         {
           case ICMP_TIMXCEED_INTRANS:
                printf ("Time to live exceeded\n");
                break;
           case ICMP_TIMXCEED_REASS:
                printf ("Frag reassembly time exceeded\n");
                break;
           default:
                printf ("Time exceeded, Bad Code: %d\n", icp->icmp_code);
                break;
         }
         pr_retip ((struct ip *)icp->icmp_data);
         break;

    case ICMP_PARAMPROB:
         printf ("Parameter problem: pointer = 0x%02x\n", icp->icmp_pptr);
         pr_retip ((struct ip *)icp->icmp_data);
         break;

    case ICMP_TSTAMP:
         printf ("Timestamp\n");
         /* XXX ID + Seq + 3 timestamps */
         break;

    case ICMP_TSTAMPREPLY:
         printf ("Timestamp Reply\n");
         /* XXX ID + Seq + 3 timestamps */
         break;

    case ICMP_IREQ:
         printf ("Information Request\n");
         /* XXX ID + Seq */
         break;

    case ICMP_IREQREPLY:
         printf ("Information Reply\n");
         /* XXX ID + Seq */
         break;

    case ICMP_MASKREQ:
         printf ("Address Mask Request\n");
         break;

    case ICMP_MASKREPLY:
         printf ("Address Mask Reply\n");
         break;

    default:
         printf ("Bad ICMP type: %d\n", icp->icmp_type);
  }
}

/*
 * pr_icmp6 --
 *  Print a descriptive string about an ICMPv6 header.
 */
void pr_icmp6 (struct icmp6_hdr *icp)
{
  switch (icp->icmp6_type)
  {
    case ICMP6_ECHO_REPLY:
         printf ("Echo Reply\n");
         /* XXX ID + Seq + Data */
         break;
    /* to do .. */
  }
}

/*
 * pr_iph --
 *  Print an IP header with options.
 */
void pr_iph (struct ip *ip)
{
  int    hlen = ip->ip_hl << 2;
  u_char *cp  = (u_char *)ip + 20;    /* point to options */

  printf ("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src      Dst Data\n");
  printf (" %1x  %1x  %02x %04x %04x", ip->ip_v, ip->ip_hl, ip->ip_tos, ip->ip_len, ip->ip_id);
  printf ("   %1x %04x", (ip->ip_off & 0xE000) >> 13, ip->ip_off & 0x1FFF);
  printf ("  %02x  %02x %04x", ip->ip_ttl, ip->ip_p, ip->ip_sum);
  printf (" %s ", inet_ntoa(ip->ip_src));
  printf (" %s ", inet_ntoa(ip->ip_dst));

  /* dump and option bytes */
  while (hlen-- > 20)
        printf ("%02x", *cp++);
  putchar ('\n');
}

/*
 * pr_addr4 --
 *  Return an ascii host address as a dotted quad and optionally with
 * a hostname.
 */
char *pr_addr4 (u_long l)
{
  struct hostent *hp;
  static char     buf[80];
  char  *adr = inet_ntoa (*(struct in_addr*)&l);

  if ((options & F_NUMERIC) ||
      (hp = gethostbyaddr((char*)&l, 4, AF_INET)) == NULL)
       sprintf (buf, "%s", adr);
  else sprintf (buf, "%s (%s)", hp->h_name, adr);
  return (buf);
}

char *pr_addr6 (const void *l)
{
  struct hostent *hp;
  static char     buf[80];
  const  char    *adr = inet_ntop (AF_INET6, l, buf, sizeof(buf));

  if ((options & F_NUMERIC) ||
      (hp = gethostbyaddr((const char*)l, sizeof(struct in6_addr), AF_INET6)) == NULL)
       sprintf (buf, "%s", adr);
  else sprintf (buf, "%s (%s)", hp->h_name, adr);
  return (buf);
}

/*
 * pr_retip --
 *  Dump some info on a returned (via ICMP) IP packet.
 */
void pr_retip (struct ip *ip)
{
  int     hlen  = ip->ip_hl << 2;
  u_char *cp    = (u_char *)ip + hlen;
  u_short sport = _getshort (cp);
  u_short dport = _getshort (cp+2);

  pr_iph (ip);

  if (ip->ip_p == IPPROTO_TCP)
     printf ("TCP: from port %u, to port %u (decimal)\n", sport, dport);

  if (ip->ip_p == IPPROTO_UDP)
     printf ("UDP: from port %u, to port %u (decimal)\n", sport, dport);
}

void fill (char *bp, char *patp)
{
  int   ii, jj, kk;
  int   pat[16];
  char *cp;

  for (cp = patp; *cp; cp++)
    if (!isxdigit(*cp))
       Exit ("ping: patterns must be specified as hex digits.\n");

  ii = sscanf (patp, "%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x",
               &pat[0], &pat[1], &pat[2], &pat[3], &pat[4], &pat[5], &pat[6],
               &pat[7], &pat[8], &pat[9], &pat[10],&pat[11],&pat[12],
               &pat[13],&pat[14],&pat[15]);

  if (ii > 0)
  {
    for (kk = 0; kk <= MAXPACKET - (8 + ii); kk += ii)
        for (jj = 0; jj < ii; ++jj)
            bp[jj + kk] = pat[jj];
  }

  if (!(options & F_QUIET))
  {
    printf ("PATTERN: 0x");
    for (jj = 0; jj < ii; ++jj)
        printf ("%02x", bp[jj] & 0xFF);
    printf ("\n");
  }
}

void beep (void)
{
 if (!(options & F_SO_DEBUG))
    return;

#if 0
  sound (2000);
  usleep (10000);
  nosound();
#else
  fputc ('\7', stderr);
#endif
}
