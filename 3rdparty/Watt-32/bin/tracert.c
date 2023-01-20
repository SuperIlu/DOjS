/*
 * traceroute host  - trace the route ip packets follow going to "host".
 *
 * Attempt to trace the route an ip packet would follow to some
 * internet host.  We find out intermediate hops by launching probe
 * packets with a small ttl (time to live) then listening for an
 * icmp "time exceeded" reply from a gateway.  We start our probes
 * with a ttl of one and increase by one until we get an icmp "port
 * unreachable" (which means we got to "host") or hit a max (which
 * defaults to 30 hops & can be changed with the -m flag).  Three
 * probes (change with -q flag) are sent at each ttl setting and a
 * line is printed showing the ttl, address of the gateway and
 * round trip time of each probe.  If the probe answers come from
 * different gateways, the address of each responding system will
 * be printed.  If there is no response within a 5 sec. timeout
 * interval (changed with the -w flag), a "*" is printed for that
 * probe.
 *
 * If probe packets are UDP or TCP format, we don't want the destination
 * host to process them. So the destination port is set to an
 * unlikely value (if some clod on the destination is using that
 * value, it can be changed with the -p flag).
 *
 * A sample use might be:
 *
 *     [yak 71]% traceroute nis.nsf.net.
 *     traceroute to nis.nsf.net (35.1.1.48), 30 hops max, 56 byte packet
 *      1  helios.ee.lbl.gov (128.3.112.1)  19 ms  19 ms  0 ms
 *      2  lilac-dmc.Berkeley.EDU (128.32.216.1)  39 ms  39 ms  19 ms
 *      3  lilac-dmc.Berkeley.EDU (128.32.216.1)  39 ms  39 ms  19 ms
 *      4  ccngw-ner-cc.Berkeley.EDU (128.32.136.23)  39 ms  40 ms  39 ms
 *      5  ccn-nerif22.Berkeley.EDU (128.32.168.22)  39 ms  39 ms  39 ms
 *      6  128.32.197.4 (128.32.197.4)  40 ms  59 ms  59 ms
 *      7  131.119.2.5 (131.119.2.5)  59 ms  59 ms  59 ms
 *      8  129.140.70.13 (129.140.70.13)  99 ms  99 ms  80 ms
 *      9  129.140.71.6 (129.140.71.6)  139 ms  239 ms  319 ms
 *     10  129.140.81.7 (129.140.81.7)  220 ms  199 ms  199 ms
 *     11  nic.merit.edu (35.1.1.48)  239 ms  239 ms  239 ms
 *
 * Note that lines 2 & 3 are the same.  This is due to a buggy
 * kernel on the 2nd hop system -- lbl-csam.arpa -- that forwards
 * packets with a zero ttl.
 *
 * A more interesting example is:
 *
 *     [yak 72]% traceroute allspice.lcs.mit.edu.
 *     traceroute to allspice.lcs.mit.edu (18.26.0.115), 30 hops max
 *      1  helios.ee.lbl.gov (128.3.112.1)  0 ms  0 ms  0 ms
 *      2  lilac-dmc.Berkeley.EDU (128.32.216.1)  19 ms  19 ms  19 ms
 *      3  lilac-dmc.Berkeley.EDU (128.32.216.1)  39 ms  19 ms  19 ms
 *      4  ccngw-ner-cc.Berkeley.EDU (128.32.136.23)  19 ms  39 ms  39 ms
 *      5  ccn-nerif22.Berkeley.EDU (128.32.168.22)  20 ms  39 ms  39 ms
 *      6  128.32.197.4 (128.32.197.4)  59 ms  119 ms  39 ms
 *      7  131.119.2.5 (131.119.2.5)  59 ms  59 ms  39 ms
 *      8  129.140.70.13 (129.140.70.13)  80 ms  79 ms  99 ms
 *      9  129.140.71.6 (129.140.71.6)  139 ms  139 ms  159 ms
 *     10  129.140.81.7 (129.140.81.7)  199 ms  180 ms  300 ms
 *     11  129.140.72.17 (129.140.72.17)  300 ms  239 ms  239 ms
 *     12  * * *
 *     13  128.121.54.72 (128.121.54.72)  259 ms  499 ms  279 ms
 *     14  * * *
 *     15  * * *
 *     16  * * *
 *     17  * * *
 *     18  ALLSPICE.LCS.MIT.EDU (18.26.0.115)  339 ms  279 ms  279 ms
 *
 * (I start to see why I'm having so much trouble with mail to
 * MIT.)  Note that the gateways 12, 14, 15, 16 & 17 hops away
 * either don't send ICMP "time exceeded" messages or send them
 * with a ttl too small to reach us.  14 - 17 are running the
 * MIT C Gateway code that doesn't send "time exceeded"s.  God
 * only knows what's going on with 12.
 *
 * The silent gateway 12 in the above may be the result of a bug in
 * the 4.[23]BSD network code (and its derivatives):  4.x (x <= 3)
 * sends an unreachable message using whatever ttl remains in the
 * original datagram.  Since, for gateways, the remaining ttl is
 * zero, the icmp "time exceeded" is guaranteed to not make it back
 * to us.  The behavior of this bug is slightly more interesting
 * when it appears on the destination system:
 *
 *      1  helios.ee.lbl.gov (128.3.112.1)  0 ms  0 ms  0 ms
 *      2  lilac-dmc.Berkeley.EDU (128.32.216.1)  39 ms  19 ms  39 ms
 *      3  lilac-dmc.Berkeley.EDU (128.32.216.1)  19 ms  39 ms  19 ms
 *      4  ccngw-ner-cc.Berkeley.EDU (128.32.136.23)  39 ms  40 ms  19 ms
 *      5  ccn-nerif35.Berkeley.EDU (128.32.168.35)  39 ms  39 ms  39 ms
 *      6  csgw.Berkeley.EDU (128.32.133.254)  39 ms  59 ms  39 ms
 *      7  * * *
 *      8  * * *
 *      9  * * *
 *     10  * * *
 *     11  * * *
 *     12  * * *
 *     13  rip.Berkeley.EDU (128.32.131.22)  59 ms !  39 ms !  39 ms !
 *
 * Notice that there are 12 "gateways" (13 is the final
 * destination) and exactly the last half of them are "missing".
 * What's really happening is that rip (a Sun-3 running Sun OS3.5)
 * is using the ttl from our arriving datagram as the ttl in its
 * icmp reply.  So, the reply will time out on the return path
 * (with no notice sent to anyone since icmp's aren't sent for
 * icmp's) until we probe with a ttl that's at least twice the path
 * length.  I.e., rip is really only 7 hops away.  A reply that
 * returns with a ttl of 1 is a clue this problem exists.
 * Traceroute prints a "!" after the time if the ttl is <= 1.
 * Since vendors ship a lot of obsolete (DEC's Ultrix, Sun 3.x) or
 * non-standard (HPUX) software, expect to see this problem
 * frequently and/or take care picking the target host of your
 * probes.
 *
 * Other possible annotations after the time are !H, !N, !P (got a host,
 * network or protocol unreachable, respectively), !S or !F (source
 * route failed or fragmentation needed -- neither of these should
 * ever occur and the associated gateway is busted if you see one).  If
 * almost all the probes result in some kind of unreachable, traceroute
 * will give up and exit.
 *
 * Notes
 * -----
 * The udp port usage may appear bizarre (well, ok, it is bizarre).
 * The problem is that an icmp message only contains 8 bytes of
 * data from the original datagram.  8 bytes is the size of a udp
 * header so, if we want to associate replies with the original
 * datagram, the necessary information must be encoded into the
 * udp header (the ip id could be used but there's no way to
 * interlock with the kernel's assignment of ip id's and, anyway,
 * it would have taken a lot more kernel hacking to allow this
 * code to set the ip id).  So, to allow two or more users to
 * use traceroute simultaneously, we use this task's pid as the
 * source port (the high bit is set to move the port number out
 * of the "likely" range).  To keep track of which probe is being
 * replied to (so times and/or hop counts don't get confused by a
 * reply that was delayed in transit), we increment the destination
 * port number before each probe.
 *
 * Don't use this as a coding example.  I was trying to find a
 * routing problem and this code sort-of popped out after 48 hours
 * without sleep.  I was amazed it ever compiled, much less ran.
 *
 * I stole the idea for this program from Steve Deering.  Since
 * the first release, I've learned that had I attended the right
 * IETF working group meetings, I also could have stolen it from Guy
 * Almes or Matt Mathis.  I don't know (or care) who came up with
 * the idea first.  I envy the originators' perspicacity and I'm
 * glad they didn't keep the idea a secret.
 *
 * Tim Seaver, Ken Adelman and C. Philip Wood provided bug fixes and/or
 * enhancements to the original distribution.
 *
 * I've hacked up a round-trip-route version of this that works by
 * sending a loose-source-routed udp datagram through the destination
 * back to yourself.  Unfortunately, SO many gateways botch source
 * routing, the thing is almost worthless.  Maybe one day...
 *
 *  -- Van Jacobson (van@helios.ee.lbl.gov)
 *     Tue Dec 20 03:50:13 PST 1988
 *
 *
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


/*
 * PSC Changes Copyright (c) 1992 Pittsburgh Supercomputing Center.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the modifications to this
 * software were developed by the Pittsburgh Supercomputing Center.
 * The name of the Center may not be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE
 * IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.
 *
 */

/* DNS lookup portions of this code also subject to the following:  */
/*
 * Use the domain system to resolve a name.
 *
 * Copyright (C) 1988,1990,1992 Dan Nydick, Carnegie-Mellon University
 * Anyone may use this code for non-commercial purposes as long
 * as my name and copyright remain attached.
 */

/*
 *  1/7/92:  Modified to include a new option for MTU discovery.
 *           This option will send out packets with the don't
 *           fragment bit set in order to determine the MTU
 *           of the path being traceroute'd.  Only decreases
 *           in MTU will be detected, and the MTU will initially
 *           be set to the interface MTU which is used for
 *           routing.  In the event of an MTU decrease in the
 *           path, the output will include a message of the
 *           form MTU=#### with the new MTU for the latest
 *           hop.  This option is invoked with "-M".
 *                            Jamshid Mahdavi,   PSC.
 */

/*
 * 4/12/93:  Modified to include new option (-Q) that will report on
 *           percentage packet loss* in addition to the usual
 *           min/avg/max delay.  When -Q is invoked, delay per packet
 *           reporting is turned off.  Also ^C aborts further packets
 *           sent on that ttl and goes on to the next -- two
 *           consecutive Ctrl-C will terminate the traceroute entirely.
 *           [modified code rom Matt Mathis' uping.c code]
 *                          Jon Boone,  PSC.
 */

/*
 * 4/23/93:  Added support for a "-a" switch which will support
 *           automatic cutoff after a certain number of dropped
 *           packets in a row
 *                          Jon Boone, PSC.
 */

/*
 * 10/21/93: (JM) Fixed SGI version, changed the packet sizing scheme
 *           to a saner system based on total packet size (which
 *           also fixed several bugs in the old code w.r.t.
 *           size of packets).  Added fast timers.  Added network
 *           owner lookup.  Plan to add AS path lookup eventually...
 */

/*
 * 09/19/96: Rewritten to work with Waterloo TCP/IP
 *           Gisle Vanem <gvanem@yahoo.no>
 *
 * 10/10/2006: Added support for GeoIP to print country information
 *
 * 02/03/2021: Added support for IP2Location to print country and city information
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <sys/stat.h>

#ifdef __CYGWIN__
  #include <unistd.h>
#else
  #include <conio.h>
  #include <direct.h>  /* getcwd() */
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/if_dl.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <netdb.h>
#include <tcp.h>

#if defined(__MINGW32__) || defined(_MSC_VER)
  /*
   * MinGW's libmingex.a messes up the printing of GetAddress:
   *
   * snprintf (buf, sizeof(buf), " %-*s", 30-indent, GetAddress(src));
   *
   * MSVC does it right. So use that.
   */
  #define snprintf _snprintf

#elif defined(__DMC__)
  #define snprintf _snprintf
#endif

#if defined(__GNUC__)
  #pragma GCC diagnostic ignored "-Wpointer-sign"
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

#if defined(USE_GEOIP)
  #include "geoip.h"

  extern void _GeoIP_setup_dbfilename (void);
  static int init_geoip (const char *argv0);
  static int get_country_from_ip (struct in_addr addr,
                                  const char **country_code,
                                  const char **country_name);
  static int get_city_from_ip (struct in_addr ip, char *city, size_t city_size);

#elif defined(USE_IP2LOCATION)
  #include "IP2Location.h"

  static int init_geoip (const char *argv0);
  static int get_country_from_ip (struct in_addr addr,
                                  const char **country_code,
                                  const char **country_name);
  static int get_city_from_ip (struct in_addr ip, char *city, size_t city_size);
#endif

#ifndef S_ISREG
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif

#define VERSION          "Waterloo TCP/IP traceroute 1.1"

#ifndef PROBE_PROTOCOL
#define PROBE_PROTOCOL   IPPROTO_ICMP  /* or IPPROTO_UDP or IPPROTO_TCP. \todo: make this a run-time option */
#endif

#ifndef INADDR_LOOPBACK
#define INADDR_LOOPBACK  0x7F000001UL
#endif

#ifndef PATH_MAX
#define PATH_MAX  512
#endif

#ifdef __BORLANDC__
#pragma warn -stu-
#endif

#define MAXPACKET       1500    /* max ip packet size */
#define MINPACKET       (sizeof(struct opacket) - sizeof(struct ether_header))
#define SRC_PORT        1030
#define MAX_IPOPTLEN    40      /* 15*4 - 20 */
#define MAX_SKIP        50
#define MAX_GW         ((MAX_IPOPTLEN-IPOPT_MINOFF)/sizeof(DWORD)-1)

#define ETHPROTO_IP     8       /* network order */
#define IPPROTO_ICMP    1
#define IPPROTO_UDP     17
#define IP_VERSION      4

#define DIM(x)          (int) (sizeof((x)) / sizeof((x)[0]))

W32_CLANG_PACK_WARN_OFF()

#include <sys/pack_on.h>

struct data {
       BYTE   seq;             /* sequence number of this packet */
       BYTE   ttl;             /* ttl packet left with */
       time_t tv;              /* time packet left */
     };

struct ping {
       BYTE  type;
       BYTE  code;
       WORD  chksum;
       WORD  ident;
       WORD  seq;
       DWORD index;
     };

struct opacket {              /* format of a probe packet */
       struct  ether_header eth;
       struct  ip           ip;
#if (PROBE_PROTOCOL == IPPROTO_UDP)
       struct  udphdr       udp;
#elif (PROBE_PROTOCOL == IPPROTO_TCP)
       struct  tcphdr       tcp;
#else
       struct  ping         echo;
#endif
       struct  data         data;
     };

struct pseudoHeader {
       DWORD  src;
       DWORD  dst;
       BYTE   mbz;
       BYTE   protocol;
       WORD   length;
       WORD   checksum;
    };

#include <sys/pack_off.h>

W32_CLANG_PACK_WARN_DEF()

#define NET_ADDR(ip)  inet_ntoa (*(struct in_addr*)&(ip))

/*
 * Prototypes
 */
typedef int (*DebugProc) (void *sock, const struct ip *ip,
                          const char *fname, unsigned line);

W32_DATA DebugProc W32_NAMESPACE (debug_recv);

int         Check_ICMP (const struct ip *ip, int seq, int *type, int *code);
const char *GetAddress (struct in_addr adr);
int         trace_this_ttl (int ttl, int seq);

/*
 * Local data
 */
char   outbuf [sizeof(struct opacket) + MAX_IPOPTLEN];
struct opacket  *outpacket;      /* last output (udp) packet */
DWORD  whereto;                  /* Who to reach (net-order) */
DWORD  from_ip;                  /* Our src ip address (net-order) */
BYTE   optlist [MAX_IPOPTLEN];   /* IP options list */
int    optlen      = 0;          /* length of IP options */
size_t data_len    = 0;          /* packetlength - eth header */
void  (W32_CDECL *old_sigint)(int);  /* remember old SIGINT handler */

char  *source      = NULL;
int    nprobes     = 3;
int    tos         = 0;
int    min_ttl     = 1;
int    max_ttl     = 30;         /* default max hop count */
WORD   dport       = 32768+666;  /* udp/tcp destination port # for probe packets */
double wait_time   = 3.0;        /* time to wait for response (in seconds) */
int    mtu_disc    = 0;          /* do MTU discovery in path */
int    nflag       = 0;          /* print addresses numerically */
int    verbose     = 0;          /* print more details */
int    debug_mode  = 0;          /* show debug messages */

int    halt_flag   = 0;          /* signal happened */
int    pp_delay    = 1;          /* we normally want per-packet delay */
int    pp_loss     = 0;          /* we normally don't want packet loss */
int    lost        = 0;          /* how many packets did we not get back */
double throughput  = 0;          /* percentage packets not lost */
int    consecutive = 0;          /* the number of consecutive lost packets */
int    automagic   = 0;          /* automatically quit after 10 lost packets? */
int    drop_domain = 0;          /* drop domain name from localnet names */

int    as_lookup   = 0;          /* Look up AS path in routing registries (not yet) */
size_t new_mtu     = 0;                   /* reported mtu changed */
size_t min_mtu     = sizeof(struct ping); /* smallest mtu we will handle */

int    num_lsrr = 0;
char  *gw_names  [MAX_GW+1];
DWORD  gw_hosts  [MAX_GW+1];
int    skip_list [MAX_SKIP+1];
int    got_there;
int    unreachable;
int    indent;

double min, max, sum = 0.0, sumsq = 0.0;

const char *get_probe_proto (void)
{
#if (PROBE_PROTOCOL == IPPROTO_UDP)
  return ("UDP");
#elif (PROBE_PROTOCOL == IPPROTO_TCP)
  return ("TCP");
#elif (PROBE_PROTOCOL == IPPROTO_ICMP)
  return ("ICMP");
#else
  return ("??");
#endif
}

struct in_addr last_addr;

/*
 * This needs '-DUSE_GEOIP' or '-DUSE_IP2LOCATION' to be set in makefile.
 * Look up country/city information using one or both of the databases in the directory
 * of 'tracert.exe' and if '-DUSE_GEOIP' is defined:
 *   debug_mode <= 1 use GeoLiteCity.dat only.
 *   debug_mode >= 2 use GeoLiteCountry.dat + GeoLiteCity.dat
 *
 *  and if '-DUSE_IP2LOCATION' is defined:
 *    use only 'IP2Location.bin' in directory of 'tracert.exe' for both
 *    country and city information.
 */
int get_country = 0;

struct timeval *time_now (void)
{
  static struct timeval now;
  gettimeofday (&now, NULL);
  return (&now);
}

static void Exit (const char *fmt, ...)
{
  int i;
  va_list args;

  va_start (args, fmt);
  vfprintf (stdout, fmt, args);
  va_end (args);

  for (i = 0; i < num_lsrr; i++)
     free (gw_names[i]);

#if defined(USE_GEOIP) || defined(USE_IP2LOCATION)
  {
    void W32_CDECL exit_geoip (void);
    exit_geoip();
  }
#endif
  exit (1);
}

void W32_CDECL SigIntHandler (int sig)
{
  halt_flag++;
  signal (sig, old_sigint);
}

void usage (int details)
{
#if defined(USE_GEOIP)
  #define C_OPTION "c"
  #define C_HELP   "         -c  Print country-code from GeoIP database.\n"

#elif defined(USE_IP2LOCATION)
  #define C_OPTION "c"
  #define C_HELP   "         -c  Print country-code from IP2Location database.\n"

#else
  #define C_OPTION ""
  #define C_HELP   ""
#endif

#if (PROBE_PROTOCOL == IPPROTO_ICMP)
  #define P_OPTION  ""
  char *p_help = "";
#else
  #define P_OPTION  "[-p dest-port]"
  char p_help[60];

  snprintf (p_help, sizeof(p_help), "         -p  Dest port number to use (default %u)\n", dport);
#endif


  if (details)
     printf ("%s\nWattcp kernel: %s\nCapabilities: %s\n",
             VERSION, wattcpVersion(), wattcpCapabilities());

  printf ("Usage: tracert [-?adDnVvQuMA%s] [-f min] [-g lsrr] [-h#] [-m max] %s\n"
          "               [-q num] [-s addr] [-w wait] [-t tos] host [datasize]\n\n",
          C_OPTION, P_OPTION);

  if (details)
     printf("Options: -a  Loss Detection\n"
            "         -d  Debug mode (wattcp.dbg)\n"
            "         -D  Drop domain name from local addresses\n"
            "         -n  Print IP-addresses numerically\n"
            "         -v  Verbose printout mode\n"
            "         -V  Print version information\n"
            "         -Q  Statistics Collection\n"
            "         -f  Min # of hops/TTL       (default %u)\n"
            "         -m  Max # of hops/TTL       (default %u)\n"
            "%s"
            "         -q  # of queries per hop    (default %u)\n"
            "         -s  Source IP to use        (default my-IP)\n"
            "         -w  Waittime per hop [s]    (default %.1f)\n\n"
            "Advanced options:\n"
            "%s"
            "         -M  MTU Discovery as per RFC-1191\n"
            "         -A  AS Path Lookup\n"
            "         -g  Loose Source Route\n"
            "         -h  Skip hop number # (max %d)\n"
            "         -t  Type-of-Service: 16 - Lowdelay\n"
            "                               8 - Throughput\n"
            "                               4 - Reliability\n",
            min_ttl, max_ttl, p_help,
            nprobes, wait_time, C_HELP, MAX_SKIP);
  exit(1);
}

static void show_version_info (const char *argv0)
{
  puts (VERSION);
  puts (wattcpVersion());

#if defined(USE_GEOIP) || defined(USE_IP2LOCATION)
  verbose++;
  init_geoip (argv0);
#else
  (void)argv0;
#endif
  exit (0);
}

int main (int argc, char **argv)
{
  char *hostname;
  int   ch, ttl, arg;
  int   seq   = 0;
  int   skip  = 0;

#if defined(__CYGWIN__) && 0
  printf ("sizeof(u_long): %d\n", sizeof(u_long));
  printf ("sizeof(struct in_addr): %d\n", sizeof(struct in_addr));
#endif

  while ((ch = getopt(argc, argv, "?aA" C_OPTION "dDnVvQMf:g:m:p:q:s:w:t:h:")) != EOF)
      switch (ch)
      {
        case '?': usage (1);
                  break;

        case 'a': automagic = 1;
                  break;

        case 'A': as_lookup = 1;
                  break;

        case 'c': get_country = 1;
                  break;

        case 'd': debug_mode++;
                  break;

        case 'D': drop_domain = 1;
                  break;

        case 'n': nflag = 1;
                  break;

        case 'v': verbose++;
                  break;

        case 'V': show_version_info (argv[0]);
                  break;

        case 'Q': pp_loss  = 1;
                  pp_delay = 0;
                  break;

        case 'M': mtu_disc = 1;
                  break;

        case 'f': min_ttl = atoi (optarg);
                  if (min_ttl <= 1)
                     Exit ("min # of hops/ttl must be >1.");
                  break;

        case 'g': if (num_lsrr >= (int)MAX_GW)
                     Exit ("No more than %d gateways\n", MAX_GW);
                  gw_names [num_lsrr++] = strdup (optarg);
                  break;

        case 'm': max_ttl = atoi (optarg);
                  if (max_ttl <= 1)
                     Exit ("max # of hops/ttl must be >1.");
                  break;

        case 'p': dport = atoi (optarg);
#if (PROBE_PROTOCOL == IPPROTO_ICMP)
                  Exit ("Illegal option '-p %s'.", optarg);
#else
                  if (dport < 1)
                     Exit ("port must be >0.");
#endif
                  break;

        case 'q': nprobes = atoi (optarg);
                  if (nprobes < 1)
                     Exit ("nprobes must be >0.");
                  break;

        case 's': source = optarg;
                  if (inet_addr(source) == INADDR_NONE)
                     Exit ("Illegal source address %s", source);
                  break;

        case 'w': wait_time = atof (optarg);
                  if (wait_time <= 0.055)
                     Exit ("wait must be >0.055 sec.");
                  break;

        case 't': tos = atoi (optarg);
                  if (tos < 0 || tos > 255)
                     Exit ("TOS must be 0 - 255.");
                  break;

        case 'h': arg = atoi (optarg);
                  if (skip >= MAX_SKIP)
                     Exit ("No more than %d hop skips\n", MAX_SKIP);
                  if (arg >= max_ttl)
                     Exit ("Cannot skip hop above max TTL");
                  if (arg <= min_ttl)
                     Exit ("Cannot skip hop below min TTL");
                  skip_list [arg] = 1;
                  skip++;
                  break;

        default : usage (0);
      }

#if defined(USE_GEOIP) || defined(USE_IP2LOCATION)
  if (get_country > 0)
     init_geoip (argv[0]);
#endif

  argc -= optind;
  argv += optind;

  if (argc < 1)
     usage (0);

  if (debug_mode > 0)
     dbug_init();

  sock_init();
  block_icmp = 1;  /* make icmp_handler() ignore ICMP packets */

  hostname = *argv++;

  if (*argv)
  {
    data_len = atoi (*argv);
    if (data_len < MINPACKET || data_len > MAXPACKET)
       Exit ("packet size range is %d - %d.", MINPACKET, MAXPACKET);
  }
  else
  {
    if (mtu_disc)
         data_len = MAXPACKET;
    else data_len = MINPACKET;
  }

  whereto = htonl (lookup_host(hostname, NULL));
  if (!whereto)
      Exit (dom_strerror(dom_errno));

  outpacket = (struct opacket*) &outbuf;

  if (!_arp_resolve(ntohl(whereto), &outpacket->eth.ether_dhost))
     Exit ("No route to host");

  if (source)
       from_ip = inet_addr (source);
  else from_ip = htonl (my_ip_addr);

  /* ^C puts you to the next hop. Twice will exit.
   */
  old_sigint = signal (SIGINT, SigIntHandler);

  if (num_lsrr > 0)
  {
    int i;
    for (i = 0; i < num_lsrr; i++)
    {
      gw_hosts[i] = htonl (lookup_host(gw_names[i], NULL));
      if (!gw_hosts[i])
         Exit (dom_strerror(dom_errno));
    }

    optlen = num_lsrr * sizeof(gw_hosts[0]) + 4;  /* min 8 bytes */

    gw_hosts[num_lsrr] = whereto;        /* final hop */
    whereto       = gw_hosts[0];
    optlist [0]   = IPOPT_NOP;           /* force 4 byte alignment */
    optlist [1]   = IPOPT_LSRR;          /* loose source route option */
    optlist [2]   = optlen - 1;          /* LSRR option-list length */
    optlist [3]   = IPOPT_MINOFF;        /* Pointer to LSRR addresses */
    memcpy (&optlist[4], &gw_hosts[1], optlen-4);
  }

  if (data_len < MINPACKET + optlen)
  {
    /* The chosen size is too small to fit everything...make it bigger: */
    data_len = MINPACKET + optlen;
  }

  if (verbose && inet_addr(hostname) != INADDR_NONE)
  {
    struct hostent *hp;
    char   name [MAXHOSTNAMELEN];

    printf ("Reverse lookup (%s)..", hostname);
    fflush (stdout);
    hp = gethostbyaddr ((char*)&whereto, sizeof(whereto), AF_INET);
    if (!hp)
       printf ("<unknown>\n");
    else
    {
      hostname = strcpy (name, hp->h_name);
      printf ("`%s'\n", hostname);
    }
  }

  /*
   *  Finished initialising stuff. Enter main traceroute loop
   */
  printf ("\r%s traceroute to %s (%s)", get_probe_proto(), hostname, NET_ADDR(whereto));

#if (PROBE_PROTOCOL != IPPROTO_ICMP)
  printf (" dest-port %d", dport);
#endif

  if (source)
     printf (" from %s", source);
  printf (", %d hops max, %u byte packets\n", max_ttl, (unsigned)data_len);

  outpacket->ip.ip_src.s_addr = from_ip;
  outpacket->ip.ip_dst.s_addr = whereto;

  for (ttl = min_ttl; ttl <= max_ttl; ttl++)
  {
    printf ("%2d ", ttl);
    if (ttl < DIM(skip_list) && skip_list[ttl])
    {
      puts (" Skipping this hop");
      continue;
    }
    if (!trace_this_ttl(ttl, ++seq))
       break;
  }
  return (0);
}

/*---------------------------------------------------------------------*/

int send_probe (int ttl, int tos)
{
  struct ip   *ip    = &outpacket->ip;
  struct data *data  = &outpacket->data;
  static WORD seqnum = 0x10;
  int    rc;
  size_t len         = data_len - sizeof(*ip);
  size_t hlen        = sizeof (*ip) + optlen;
  BYTE  *buf         = _eth_formatpacket (&outpacket->eth.ether_dhost,
                                          ETHPROTO_IP);
  if (optlen)
  {
    memcpy (ip+1, &optlist, optlen);
    data += optlen;
  }

  ip->ip_off = mtu_disc ? ntohs(IP_DF) : 0; /* don't fragment bit */
  ip->ip_hl  = (u_char) (hlen >> 2);        /* head + options length */
  ip->ip_len = htons ((u_short)data_len);
  ip->ip_tos = tos;
  ip->ip_v   = IP_VERSION;
  ip->ip_ttl = ttl;
  ip->ip_id  = htons (seqnum);
  ip->ip_p   = PROBE_PROTOCOL;
  ip->ip_sum = 0;
  ip->ip_sum = ~inchksum (ip, hlen);

  data->seq = seqnum & 0xFF;
  data->ttl = ttl;
  data->tv  = time (NULL);

#if (PROBE_PROTOCOL == IPPROTO_ICMP)
  {
    struct ping *echo = (struct ping*) ((BYTE*)&outpacket->echo + optlen);

    echo->type    = ICMP_ECHO;
    echo->code    = 0;
    echo->index   = seqnum;
    echo->seq     = seqnum;
    echo->ident   = set_timeout (0) & 0xFFFF;
    echo->chksum  = 0;
    echo->chksum  = ~inchksum (echo, len);
  }
#elif (PROBE_PROTOCOL == IPPROTO_UDP)
  {
    struct udphdr *udp = (struct udphdr *) ((BYTE*)&outpacket->udp + optlen);
    struct pseudoHeader ph;

    udp->uh_sport = htons (SRC_PORT);
    udp->uh_dport = htons (dport);
    udp->uh_ulen  = htons (len);
    udp->uh_sum   = 0;

    ph.src      = ip->ip_src.s_addr;
    ph.dst      = ip->ip_dst.s_addr;
    ph.mbz      = 0;
    ph.protocol = IPPROTO_UDP;
    ph.length   = udp->uh_ulen;
    ph.checksum = inchksum (udp, len);
    udp->uh_sum = ~inchksum (&ph, sizeof(ph));
  }
#elif (PROBE_PROTOCOL == IPPROTO_TCP)
  {
    struct tcphdr *tcp = (struct tcphdr *) ((BYTE*)&outpacket->tcp + optlen);
    struct pseudoHeader ph;
    u_short             th_len = sizeof(*tcp) + optlen;

    tcp->th_sport = htons (SRC_PORT);
    tcp->th_dport = htons (dport);
    tcp->th_flags = TH_SYN;
    tcp->th_off   = th_len / 4;
    tcp->th_win   = htons (MAXPACKET);
    tcp->th_x2    = 0;
    tcp->th_urp   = 0;
    tcp->th_sum   = 0;

    ph.src      = ip->ip_src.s_addr;
    ph.dst      = ip->ip_dst.s_addr;
    ph.mbz      = 0;
    ph.protocol = IPPROTO_TCP;
    ph.length   = htons ((WORD)len);
    ph.checksum = inchksum (tcp, len);
    tcp->th_sum = ~inchksum (&ph, sizeof(ph));
  }
#else
  #error Unknown PROBE_PROTOCOL
#endif

  memcpy ((void*)buf, ip, data_len);
  rc = _eth_send ((WORD) (data_len + sizeof(struct ether_header)),
                  NULL, __FILE__, __LINE__);
  seqnum += 5;
  return (rc);
}

/*---------------------------------------------------------------------*/

size_t Reduce_mtu (size_t value)
{
  /* The following heuristic taken from RFC1191
   */
  static int mtu[] = { 1492, 1006, 508,
                        296,   68, -1
                     };
  int i = 0;

  while (value <= (size_t)mtu[i])
        i++;
  if (mtu[i] <= 0)
     Exit (" Invalid MTU !!\n");
  return (size_t)mtu[i];
}

/*---------------------------------------------------------------------*/

void print_unreach (const struct ip *ip, int icmp_code)
{
  struct icmp *icmp;
  int    hlen;
  size_t mtu;

  switch (icmp_code)
  {
    case ICMP_UNREACH_PORT:
         if (ip->ip_ttl <= 1)
            printf (" !");
         got_there++;
         break;

    case ICMP_UNREACH_NET:
         ++unreachable;
         printf (" !N");
         break;

    case ICMP_UNREACH_HOST:
         ++unreachable;
         printf (" !H");
         break;

    case ICMP_UNREACH_PROTOCOL:
         got_there++;
         printf (" !P");
         break;

    case ICMP_UNREACH_NEEDFRAG:
         if (mtu_disc)
         {
           hlen = ip->ip_hl << 2;
           icmp = (struct icmp *)((BYTE*)ip + hlen);
           mtu  = ntohs (icmp->icmp_nextmtu);

           if (mtu >= data_len)
              printf (" !M>");    /* serious bug somewhere */

           if (mtu <= min_mtu)
              printf (" !M<");
           else if (!mtu)
           {
             printf (" !M");
             new_mtu = Reduce_mtu (data_len);
           }
           else
           {
             new_mtu = mtu;
             printf (" !M=%u", (unsigned)new_mtu);
           }
         }
         else
         {
           ++unreachable;
           printf (" !F");
         }
         break;

    case ICMP_UNREACH_SRCFAIL:
         ++unreachable;
         printf (" !S");
         break;

    case ICMP_UNREACH_FILTER_PROHIB:
         ++unreachable;
         printf (" !X");
         break;
  }
}

/*---------------------------------------------------------------------*/

struct ip *WaitReply (DWORD timeout)
{
  W32_DATA WORD _pkt_ip_ofs;  /* Offset from MAC-header to IP-header */

  while (1)
  {
    static BYTE buf [MAXPACKET];
    int    bcast;
    WORD   packet_type;
    BYTE  *packet = _eth_arrived (&packet_type, &bcast);
    struct ip *ip;

    watt_kbhit();

    if (!packet)
    {
      if (chk_timeout(timeout))
      {
        indent += printf (" *");
        lost++;
        consecutive++;
        return (NULL);
      }
      continue;
    }

    memcpy (buf, packet - _pkt_ip_ofs, sizeof(buf));
    ip = (struct ip*) (buf + _pkt_ip_ofs);
    _eth_free (packet);

    if (W32_NAMESPACE(debug_recv))
      (*W32_NAMESPACE(debug_recv)) (NULL, ip, __FILE__, __LINE__);

    if (bcast ||                              /* link-layer broadcast */
        packet_type       != ETHPROTO_IP  ||  /* not IP-packet */
        ip->ip_p          != IPPROTO_ICMP ||  /* not ICMP-packet */
        ip->ip_dst.s_addr != from_ip)         /* not to us */
      continue;

    return (ip);
  }
  return (NULL);
}

/*---------------------------------------------------------------------*/

void CheckReply (const struct ip *ip, int ttl, int probe, int seq, double delta_t)
{
  struct in_addr src;
  int    icmp_type, icmp_code, different;
  static int last_ttl = 0;

  if (!Check_ICMP(ip, seq, &icmp_type, &icmp_code))
     return;

  src = ip->ip_src;

  if (src.s_addr == whereto)
     got_there = 1;    /* got final destination */

  if (sum == 0)
  {
    sum = min = max = delta_t;
    sumsq = delta_t * delta_t;
  }
  else
  {
    if (delta_t < min)
       min = delta_t;
    if (delta_t > max)
       max = delta_t;
    sum   += delta_t;
    sumsq += delta_t * delta_t;
  }

  different = (src.s_addr != last_addr.s_addr);

  /*
   * It can happen that a 'src.s_addr' is different for the same
   * TTL on the 2nd, 3rd etc. probe. So to not mess up output by
   * printing GetAddress() twice for same TTL, make them equal.
   *
   * Proof by example:
   *   tracert.exe -q2 209.87.252.184
   *   traceroute to 209.87.252.184 (209.87.252.184), 30 hops max, 42 byte packets
   *    1 router                         (10.0.0.1)           0 ms   0 ms
   *    2 1.80-202-225.nextgentel.com    (80.202.225.1)      16 ms  16 ms
   *    3 217-13-1-238.dd.nextgentel.com (217.13.1.238)      31 ms  31 ms
   *    4 94.84-48-3.nextgentel.com      (84.48.3.94)        16 ms  16 ms
   *    5 158.84-48-3.nextgentel.com     (84.48.3.158)       31 ms  31 ms
   *    6 18.84-48-3.nextgentel.com      (84.48.3.18)        31 ms  31 ms
   *    7 oso-b3-link.telia.net          (80.239.193.93)     31 ms  16 ms
   *    8 kbn-bb2-link.telia.net         (80.91.251.48)      63 ms  47 ms
   *    9 hbg-bb2-link.telia.net         (213.155.130.102)   47 ms  47 ms
   *   10-0 ffm-bb2-link.telia.net       (80.91.245.119)     47 ms
   *   10-1 ffm-bb2-link.telia.net       (80.91.249.88)      63 ms
   *
   * Here on TTL = 10, probe = 0, the reply comes from 80.91.245.119, but
   *      on TTL = 10, probe = 1, the reply comes from 80.91.249.88.
   *
   * BTW. the 2 hosts have the same host-name.
   */

  if (probe > 0 && ttl == last_ttl && different)
     different = 0;

  last_ttl = ttl;

  if (different)
  {
    char buf[100];
    char *p = buf;

    p += snprintf (buf, sizeof(buf), " %-*s", 30-indent, GetAddress(src));
    if (!nflag)
       p += snprintf (p, sizeof(buf) - (p - buf), " (%s)", inet_ntoa(src));

    printf (nflag ? "%-30s" : "%-50s", buf);
    last_addr = src;
  }

  if (pp_delay)
     printf (" %3.0f ms", delta_t);

  if (probe == nprobes-1 && priv_addr(ntohl(src.s_addr)))
     printf ("  priv");

  if (icmp_type == ICMP_UNREACH)
     print_unreach (ip, icmp_code);
}

/*---------------------------------------------------------------------*/

char * ICMP_type (BYTE type)
{
  static char buf[30];
  switch (type)
  {
    case ICMP_ECHOREPLY:
         return ("Echoreply");
    case ICMP_UNREACH:
         return ("Unreach");
    case ICMP_SOURCEQUENCH:
         return ("SourceQuench");
    case ICMP_REDIRECT:
         return ("Redirect");
    case ICMP_ECHO:
         return ("Echo");
    case ICMP_TIMXCEED:
         return ("Time exceed");
    case ICMP_PARAMPROB:
          return ("ParamProblem");
    case ICMP_TSTAMP:
          return ("Tstamp");
    case ICMP_TSTAMPREPLY:
         return ("TstampReply");
    case ICMP_IREQ:
         return ("Ireq");
    case ICMP_IREQREPLY:
         return ("IreqReply");
    case ICMP_MASKREQ:
         return ("MaskReq");
    case ICMP_MASKREPLY:
         return ("MaskReply");
    default:
         sprintf (buf,"unknown type %u",type);
         return (buf);
  }
}

/*---------------------------------------------------------------------*/

int Check_ICMP (const struct ip *ip, int seq, int *ret_type, int *ret_code)
{
  int    hlen       = ip->ip_hl << 2;
  struct icmp *icmp = (struct icmp *) ((BYTE*)ip + hlen);
  BYTE   type       = icmp->icmp_type;
  BYTE   code       = icmp->icmp_code;
  char   tbuf[100]  = "";

  *ret_type = type;
  *ret_code = code;

  if (ntohl(ip->ip_src.s_addr) == INADDR_LOOPBACK)
     return (0);

  if (type == ICMP_ECHOREPLY && ip->ip_src.s_addr == whereto)
     return (1);

  if ((type == ICMP_TIMXCEED && code == ICMP_TIMXCEED_INTRANS) ||
      type == ICMP_UNREACH)
  {
    const struct ip *orig_ip;

#if (PROBE_PROTOCOL == IPPROTO_UDP)
    const struct udphdr *orig_udp;
#elif (PROBE_PROTOCOL == IPPROTO_TCP)
    const struct tcphdr *orig_tcp;
#endif

    orig_ip = (const struct ip*) &icmp->icmp_ip;
    hlen    = orig_ip->ip_hl << 2;

#if (PROBE_PROTOCOL == IPPROTO_UDP)
    orig_udp = (const struct udphdr*) ((BYTE*)orig_ip + hlen);
#elif (PROBE_PROTOCOL == IPPROTO_TCP)
    orig_tcp = (const struct tcphdr*) ((BYTE*)orig_ip + hlen);
#endif

    if (orig_ip->ip_p != PROBE_PROTOCOL)
       return (0);

#if (PROBE_PROTOCOL == IPPROTO_UDP || PROBE_PROTOCOL == IPPROTO_TCP)
    sprintf (tbuf, "for %s-ports %u/%u", get_probe_proto(),
#if (PROBE_PROTOCOL == IPPROTO_UDP)
             ntohs(orig_udp->uh_sport), ntohs(orig_udp->uh_dport));
#else
             ntohs(orig_tcp->th_sport), ntohs(orig_tcp->th_dport));
#endif
#endif

#if (PROBE_PROTOCOL == IPPROTO_UDP)
    if (orig_udp->uh_sport != htons(SRC_PORT) ||
        orig_udp->uh_dport != htons(dport))
       return (0);

#elif (PROBE_PROTOCOL == IPPROTO_TCP)
    if (orig_tcp->th_sport != htons(SRC_PORT) ||
        orig_tcp->th_dport != htons(dport))
       return (0);
#endif

    if (verbose > 1)
       printf ("IP encap (%s/%u) from %s %s\n", ICMP_type(type), code,
               NET_ADDR(ip->ip_src.s_addr), tbuf);
    return (1);
  }

  (void) seq;
  return (0);
}

/*
 * Construct an Internet address representation.
 * If the nflag has been supplied, give numeric value, otherwise
 * try for symbolic name.
 * An address in our domain is printed without a domain-name.
 */
const char *GetAddress (struct in_addr addr)
{
  static char name  [MAXHOSTNAMELEN+1];
  static char domain[MAXHOSTNAMELEN+1];
  static int  got_domain = 0;

  if (!got_domain && !nflag)
  {
    got_domain = 1;
    if (getdomainname(domain, sizeof(domain)-1) < 0)
       domain[0] = '\0';
  }

  if (!nflag && addr.s_addr)
  {
    struct hostent *hp = gethostbyaddr ((char*)&addr,sizeof(addr),AF_INET);

    if (hp)
    {
      char *cp;

      strncpy (name, hp->h_name, sizeof(name)-1);
      name [MAXHOSTNAMELEN] = '\0';
      cp = strchr (name,'.');
      if (drop_domain && cp && !strcmp(cp+1,domain))
         *cp = '\0';
      return (name);
    }
  }
  return inet_ntoa (addr);
}

/*
 * this function by default is not included in the
 * small-model library. Include it here if you want
 */
#if defined(__SMALL__) && !defined(DOS386)
int resolve_ip (DWORD ip, char *result)
{
  (void) ip;
  (void) result;
  return (0);
}
#endif

#if defined(USE_GEOIP) || defined(USE_IP2LOCATION)
#if defined(USE_GEOIP)
  static GeoIP *geoip_country_ctx = NULL;
  static GeoIP *geoip_city_ctx    = NULL;
  static int    geoip_country_db  = 0;
  static int    geoip_city_db     = 0;
#else
  static IP2Location *ip2loc_ctx = NULL;
#endif

static char  *geoip_country_info = NULL;
static char  *geoip_city_info    = NULL;

void W32_CDECL exit_geoip (void)
{
  if (geoip_city_info)
     free (geoip_city_info);
  if (geoip_country_info)
     free (geoip_country_info);

#ifdef USE_GEOIP
  if (geoip_city_ctx)
     GeoIP_delete (geoip_city_ctx);
  if (geoip_country_ctx)
     GeoIP_delete (geoip_country_ctx);
  GeoIP_cleanup();
  geoip_city_ctx = geoip_country_ctx = NULL;
#else
  if (ip2loc_ctx)
     IP2Location_close (ip2loc_ctx);
  ip2loc_ctx = NULL;
#endif

  geoip_country_info = NULL;
}

int init_geoip (const char *argv0)
{
  char *p, *home_dir = NULL;
  char  file1 [PATH_MAX];
  char  file2 [PATH_MAX];
  int   file1_found = 1;
  int   file2_found = 1;
  DWORD file1_size = 0;
  DWORD file2_size = 0;

#ifdef _WIN32
  (void) argv0;
  if (GetModuleFileNameA(NULL, file1, sizeof(file1)))
     home_dir = strdup (file1);
#else
  home_dir = strdup (argv0);
#endif

  p = strrchr (home_dir, '\\');
  if (!p)
     p = strrchr (home_dir, '/');

  /* MSVCRT makes argv[0] == tracert.exe when run in CWD.
   * So set home_dir = CWD.
   */
  if (!p)
  {
    char buf [PATH_MAX];

    free (home_dir);
    p = getcwd (buf, sizeof(buf));
    if (!p)
       return (0);
    home_dir = strdup (p);
  }
  else
    *p = '\0';

  if (debug_mode >= 2)
     printf ("home_dir: %s\n", home_dir);

#ifdef USE_GEOIP
  GeoIP_setup_custom_directory (home_dir);
  _GeoIP_setup_dbfilename();

#if defined(__MSDOS__) || defined(MSDOS)  /* Use SFN */
  snprintf (file1, sizeof(file1), "%s\\GEOLIT~2.DAT", home_dir);
  snprintf (file2, sizeof(file2), "%s\\GEOLIT~1.DAT", home_dir);
#else
  snprintf (file1, sizeof(file1), "%s\\GeoLiteCountry.dat", home_dir);
  snprintf (file2, sizeof(file2), "%s\\GeoLiteCity.dat", home_dir);
#endif

#else   /* USE_IP2LOCATION */
  snprintf (file1, sizeof(file1), "%s\\IP2Location.bin", home_dir);
  file2[0] = '\0';
#endif

  free (home_dir);

  if (verbose || debug_mode >= 2)
  {
    struct stat st;

    if (stat(file1, &st) != 0 || !S_ISREG(st.st_mode))
    {
      printf ("Cannot open '%s'\n", file1);
      file1_found = 0;
    }
    else
     file1_size = st.st_size;

    if (!file2[0] || stat(file2, &st) != 0 || !S_ISREG(st.st_mode))
    {
      if (file2[0])
         printf ("Cannot open '%s'\n", file2);
      file2_found = 0;
    }
    else
     file2_size = st.st_size;
  }

#ifdef USE_IP2LOCATION
  ip2loc_ctx = IP2Location_open (file1);

#ifndef MSDOS
  if (ip2loc_ctx && IP2Location_DB_set_shared_memory(ip2loc_ctx->file) == -1)
     puts ("IP2Location SHM failed. Continuing in file I/O mode.");
#endif

  if (verbose || debug_mode >= 2)
  {
    printf ("IP2Location ver:  %s\n", IP2Location_lib_version_string());
    printf ("IP2Location bin:  %s\n", ip2loc_ctx ? IP2Location_bin_version(ip2loc_ctx) : "<none>");
    printf ("IP2Location file: %-20s%s (size: %lu MB).\n",
            file1, file1_found ? "" : " Not found", file1_size/(1024*1024));
  }
#else
  geoip_country_ctx = GeoIP_open (file1, GEOIP_COUNTRY_EDITION);
  geoip_city_ctx    = GeoIP_open (file2, GEOIP_CITY_EDITION_REV0);

  if (!geoip_country_ctx)
  {
    printf ("%s not found.\n", file1);
    GeoIP_cleanup();
    return (0);
  }

  if (!geoip_city_ctx && verbose)
     printf ("%s not found.\n", file2);

  geoip_country_db   = GeoIP_database_edition (geoip_country_ctx);
  geoip_country_info = GeoIP_database_info (geoip_country_ctx);
  geoip_city_db      = GeoIP_database_edition (geoip_city_ctx);
  geoip_city_info    = GeoIP_database_info (geoip_city_ctx);

  if (verbose || debug_mode >= 2)
  {
    printf ("GeoIP description, %s.\n"
            "GeoIP info: %s.\n",
            GeoIPDBDescription[geoip_country_db],
            geoip_country_info ? geoip_country_info : "<none>");

    printf ("GeoIP country-file: %-20s%s (size: %lu MB).\n", file1, file1_found ? "" : " Not found", file1_size/(1024*1024));
    printf ("GeoIP city-file:    %-20s%s (size: %lu MB).\n", file2, file2_found ? "" : " Not found", file2_size/(1024*1024));
  }
#endif

  atexit (exit_geoip);

  (void) file2_found;
  (void) file2_size;
  return (1);
}

int get_country_from_ip (struct in_addr ip, const char **country_code, const char **country_name)
{
  if (country_code)
     *country_code = "-";

  if (country_name)
     *country_name = "-";

#if defined(USE_GEOIP)
  if (!geoip_country_ctx)
     return (0);

  {
    int id;

    if (geoip_country_db != GEOIP_COUNTRY_EDITION)
       return (0);

    id = GeoIP_id_by_ipnum (geoip_country_ctx, ntohl(ip.s_addr));
    if (country_code)
       *country_code = GeoIP_code_by_id (id);
    if (country_name)
       *country_name = GeoIP_name_by_id (id);
  }
#else
  ip_container container;

  if (!ip2loc_ctx)
     return (0);

  if (country_code)
  {
    IP2LocationRecord *rec1;

    container.version = 4;
    container.ipv4 = ntohl (ip.s_addr);
    rec1 = IP2Location_get_ipv4_record (ip2loc_ctx, COUNTRYSHORT, container);
    if (rec1)
       *country_code = rec1->country_short;
 // IP2Location_free_record (rec1);
  }

  if (country_name)
  {
    IP2LocationRecord *rec2;

    container.version = 4;
    container.ipv4 = ntohl (ip.s_addr);
    rec2 = IP2Location_get_ipv4_record (ip2loc_ctx, COUNTRYLONG, container);
    if (rec2)
       *country_name = rec2->country_long;
 // IP2Location_free_record (rec2);
  }
#endif

  return (1);
}

int get_city_from_ip (struct in_addr ip, char *city, size_t city_size)
{
#if 0
  GeoIPRecord *rec;

  city[0] = '-';
  city[1] = '\0';

  /* If these fails, the above init failed.
   */
  if (!geoip_city_ctx ||
      !(geoip_city_db == GEOIP_CITY_EDITION_REV0 ||
        geoip_city_db == GEOIP_CITY_EDITION_REV1))
     return (0);

  rec = GeoIP_record_by_ipnum (geoip_city_ctx, ntohl(ip.s_addr));
  if (!rec)
     return (0);

  strncpy (city, rec->city, city_size-1);
  GeoIPRecord_delete (rec);
  return (1);

#elif defined(USE_IP2LOCATION)
  /*
   * IP2Location uses only one context:
   *   a 'ip2loc_ctx' for both country, region and city-information.
   */
  IP2LocationRecord *rec;
  ip_container       container;

  city[0] = '-';
  city[1] = '\0';

  container.version = 4;
  container.ipv4 = ntohl (ip.s_addr);
  rec = IP2Location_get_ipv4_record (ip2loc_ctx, CITY, container);

  if (!rec)
     return (0);

  strncpy (city, rec->city, city_size-1);
  IP2Location_free_record (rec);
  return (1);
#else
  (void) ip;
  (void) city;
  (void) city_size;
  return (0);
#endif
}

#elif defined(USE_GEOIP)


#endif  /* USE_GEOIP */


int trace_this_ttl (int ttl, int seq)
{
  int probe;

  got_there = unreachable = 0;
  last_addr.s_addr = 0L;
  lost = indent = 0;
  consecutive = 0;
  throughput = 0.0;

  if (new_mtu)
  {
    printf ("MTU=%u ", (unsigned)new_mtu);
    new_mtu = 0;
  }
  fflush (stdout);

  for (probe = 0; probe < nprobes; probe++)
  {
    struct ip *ip;
    struct timeval start;
    DWORD  timeout;
    double delta_t;

    if (send_probe(ttl, tos) == 0)
    {
      halt_flag = 1;
      break;
    }

    timeout = set_timeout ((DWORD)(1000.0*wait_time));
    start = *time_now();
    ip = WaitReply (timeout);
    fflush (stdout);
    if (!ip)
    {
      if (ttl == 1 && probe == nprobes-1)
         Exit ("\n No reply on 1st hop. Giving up");
      if (verbose >= 2)
         printf ("ttl: %d, probe: %d, consecutive: %d, lost: %d\n", ttl, probe, consecutive, lost);
      continue;
    }

    delta_t = timeval_diff (time_now(), &start) / 1000.0;
    CheckReply (ip, ttl, probe, seq, delta_t);
    fflush (stdout);

    /* Reset the ^C action from exit to skip TTL.
     */
    if (halt_flag == 0 && lost == 1)
       signal (SIGINT, SigIntHandler);

    /* We've missed at least one packet, so let's check for
     * the signal to go to the next ttl
     */
    if (halt_flag > 0)
    {
      halt_flag = 0;
      consecutive = 0;
    }

    if (automagic && consecutive > 9)
       break;
  }

  if (pp_loss && lost < probe)
  {
    throughput = 100.0 - lost * 100.0 / probe;
    printf ("  (%1.1f ms/%1.1f ms(+-%1.1f ms)/%1.1f ms)",
            min, sum / (probe - lost), (double)sqrt((double)sumsq)/(probe-lost), max);
    printf (" %d/%d (%#3.2f%%)", probe - lost, probe, throughput);
  }

  if (get_country > 0)
  {
    const char *country_name = "?";  /* the long official country-name, but ignore things after a comma */
    char  city[100] = "?";

#if defined(USE_GEOIP)
    const char *country_code;  /* the short 2 letter name */

    if (get_country_from_ip(last_addr, &country_code, &country_name) && country_code[0] != '-')
    {
      const char *comma = strchr (country_name, ',');
      printf (", CC: %.3s (%.*s)",
              country_code, comma ? (int)(comma-country_name) : (int)strlen(country_name),
              country_name);
    }

    if (get_city_from_ip(last_addr, city, sizeof(city)) && city[0] != '-')
       printf (", City: %s", city);

#elif defined(USE_IP2LOCATION)
    if (get_country_from_ip(last_addr, NULL, &country_name) && country_name[0] != '-')
       printf (", %s", country_name);

    if (get_city_from_ip(last_addr, city, sizeof(city)) && city[0] != '-')
       printf (" / %s", city);
#endif
  }

  puts ("");
  fflush (stdout);
  if (got_there || unreachable > nprobes)
     return (0);

  if (new_mtu)
  {
    ttl--;               /* Redo the same TTL */
    data_len = new_mtu;  /* Set the new data length */
  }
  return (1);
}
