/*
 * eth-wake.c: Send a magic packet to wake up sleeping machines.
 */

static char version_msg[] = "eth-wake.c: v1.06 1/28/2002 Donald Becker, http://www.scyld.com/";
static char brief_usage_msg[] =
  "usage: ether-wake [-p aa:bb:cc:dd[:ee:ff]] 00:11:22:33:44:55\n"
  "   Use '-u' to see the complete set of options.\n";

static char usage_msg[] =
  "usage: ether-wake [-p aa:bb:cc:dd[:ee:ff]] 00:11:22:33:44:55\n"
  "\n"
  "  This program generates and transmits a Wake-On-LAN (WOL) \"Magic Packet\",\n"
  "  used for restarting machines that have been soft-powered-down\n"
  "  (ACPI D3-warm state).  It currently generates the standard AMD Magic Packet\n"
  "  format, with an optional password appended.\n"
  "\n"
  "  The single required parameter is the Ethernet MAC (station) address\n"
  "  of the machine to wake.  This is typically retrieved with the 'arp'\n"
  "  program while the target machine is awake.\n"
  "\n"
  "  Options:\n"
  "    -b      Send wake-up packet to the broadcast address.\n"
  "    -D      Increase the debug level. Also print to Watt-32 debug files.\n"
  "    -v      Set verbose mode.\n"
  "    -p <pw> Append the four or six byte password PW to the packet.\n"
  "            A password is only required for a few adapter types.\n"
  "            The password may be specified in ethernet hex format\n"
  "            or dotted decimal (Internet address):\n"
  "               -p 00:22:44:66:88:aa\n"
  "               -p 192.168.1.1\n";

/*
 * This program generates and transmits a Wake-On-LAN (WOL) "Magic Packet",
 * used for restarting machines that have been soft-powered-down
 * (ACPI D3-warm state).  It currently generates the standard AMD Magic Packet
 * format, with an optional password appended.
 *
 * This software may be used and distributed according to the terms
 * of the GNU Public License, incorporated herein by reference.
 * Contact the author for use under other terms.
 *
 * This source file is part of the network tricks package.
 * Copyright 1999-2002.
 *
 * The author may be reached as becker@scyld, or C/O
 * Scyld Computing Corporation
 * 410 Severn Ave., Suite 210
 * Annapolis MD 21403
 *
 * The single required parameter is the Ethernet MAC (station) address
 * of the machine to wake.  This is typically retrieved with the 'arp'
 * program while the target machine is awake.
 *
 * Options:
 * -b Send wake-up packet to the broadcast address.
 * -D Increase the debug level.
 * -p <pw>  Append the four or six byte password PW to the packet.
 *    A password is only required for a few adapter types.
 *    The password may be specified in ethernet hex format
 *    or dotted decimal (Internet address)
 *     -p 00:22:44:66:88:aa
 *     -p 192.168.1.1
 *
 * Notes:
 * On some systems dropping root capability allows the process to be
 * dumped, traced or debugged.
 * If someone traces this program, they get control of a raw socket.
 * Linux handles this safely, but beware when porting this program.
 *
 * An alternative to needing 'root' is using a UDP broadcast socket, however
 * doing so only works with adapters configured for unicast+broadcast Rx
 * filter.  That configuration consumes more power.
 */

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <sys/socket.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <tcp.h>

u_char outpack[1000];
int    outpack_sz = 0;
int    debug = 0;
int    wol_passwd_sz = 0;
u_char wol_passwd[6];

static int opt_no_src_addr = 0, opt_broadcast = 0;

static int get_fill (unsigned char *pkt, const char *arg);
static int get_wol_pw (const char *optarg);

int main (int argc, char *argv[])
{
  int one = 1;
  int verbose = 0;
  int s, i, c, pktsize;

  while ((c = getopt (argc, argv, "bDi:p:h?vV")) != EOF)
    switch (c)
    {
      case 'b':
           opt_broadcast++;
           break;
      case 'D':
           debug++;
           if (debug == 1)
              dbug_init();
           break;
      case 'p':
           get_wol_pw (optarg);
           break;
      case 'h':
      case '?':
           puts (usage_msg);
           return (0);
      case 'v':
           verbose++;
           break;
      case 'V':
           printf ("%s\n%s\n", version_msg, wattcpVersion());
           break;
      default:
           puts (brief_usage_msg);
           return (3);
    }

  if (optind == argc)
  {
    fprintf (stderr, "Specify the Ethernet address as 00:11:22:33:44:55.\n");
    return (3);
  }

  /* Note: PF_INET, SOCK_DGRAM, IPPROTO_UDP would allow SIOCGIFHWADDR to
   * work as non-root, but we need SOCK_PACKET to specify the Ethernet
   * destination address.
   */
  if ((s = socket (PF_PACKET, SOCK_PACKET, SOCK_PACKET)) < 0)
  {
    perror ("socket");
    return (2);
  }

  pktsize = get_fill (outpack, argv[optind]);

  /* Fill in the source address, if possible.
   */
  if (!opt_no_src_addr)
  {
    struct ifreq if_hwaddr;
    u_char *hwaddr = (u_char*)&if_hwaddr.ifr_hwaddr.sa_data;

    strcpy (if_hwaddr.ifr_name, "eth0");
    if (ioctlsocket (s, SIOCGIFHWADDR, (char*)&if_hwaddr) < 0)
    {
      fprintf (stderr, "SIOCGIFHWADDR on eth0 failed: %s\n", strerror(errno));
      return (1);
    }
    memcpy (outpack + 6, if_hwaddr.ifr_hwaddr.sa_data, 6);

    if (verbose)
    {
      printf ("The hardware address (SIOCGIFHWADDR) of eth is type %d  "
              "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x.\n",
              if_hwaddr.ifr_hwaddr.sa_family, hwaddr[0],
              hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
    }
  }

  if (wol_passwd_sz > 0)
  {
    memcpy (outpack + pktsize, wol_passwd, wol_passwd_sz);
    pktsize += wol_passwd_sz;
  }

  if (verbose > 1)
  {
    printf ("The final packet is: ");
    for (i = 0; i < pktsize; i++)
        printf (" %02x", outpack[i]);
    printf (".\n");
  }

  if (setsockopt (s, SOL_SOCKET, SO_BROADCAST, (char*)&one, sizeof(one)) < 0)
     perror ("setsockopt: SO_BROADCAST");

  i = send (s, outpack, pktsize, 0);
  if (i < pktsize)
     perror ("send");
  else if (debug)
     printf ("send() worked ! %d.\n", i);

  close_s (s);
  return (0);
}

static int get_fill (unsigned char *pkt, const char *arg)
{
  int    sa[6];
  u_char station_addr[6];
  int    byte_cnt;
  int    offset, i;
  const char *cp;

  for (cp = arg; *cp; cp++)
    if (*cp != ':' && !isxdigit((int)*cp))
    {
      fprintf (stderr, "Patterns must be specified as hex digits.\n");
      exit (2);
    }

  byte_cnt = sscanf (arg, "%2x:%2x:%2x:%2x:%2x:%2x",
                     &sa[0], &sa[1], &sa[2], &sa[3], &sa[4], &sa[5]);
  for (i = 0; i < 6; i++)
      station_addr[i] = sa[i];

  if (debug)
     fprintf (stderr, "Command line stations address is "
              "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x.\n",
              sa[0], sa[1], sa[2], sa[3], sa[4], sa[5]);

  if (byte_cnt != 6)
  {
    fprintf (stderr, "The Magic Packet address must be specified as "
             "00:11:22:33:44:55.\n");
    exit (2);
  }

  if (opt_broadcast)
       memset (pkt, 0xff, 6);
  else memcpy (pkt, station_addr, 6);

  memcpy (pkt + 6, station_addr, 6);
  pkt[12] = 0x08;               /* Or 0x0806 for ARP, 0x8035 for RARP */
  pkt[13] = 0x42;
  offset = 14;

  memset (pkt + offset, 0xff, 6);
  offset += 6;

  for (i = 0; i < 16; i++)
  {
    memcpy (pkt + offset, station_addr, 6);
    offset += 6;
  }
  if (debug)
  {
    fprintf (stderr, "Packet is ");
    for (i = 0; i < offset; i++)
        fprintf (stderr, " %02x", pkt[i]);
    fprintf (stderr, ".\n");
  }
  return (offset);
}

static int get_wol_pw (const char *optarg)
{
  int passwd[6];
  int byte_cnt;
  int i;

  byte_cnt = sscanf (optarg, "%2x:%2x:%2x:%2x:%2x:%2x",
                     &passwd[0], &passwd[1], &passwd[2],
                     &passwd[3], &passwd[4], &passwd[5]);
  if (byte_cnt < 4)
     byte_cnt = sscanf (optarg, "%d.%d.%d.%d",
                        &passwd[0], &passwd[1], &passwd[2], &passwd[3]);
  if (byte_cnt < 4)
  {
    fprintf (stderr, "Unable to read the Wake-On-LAN password.\n");
    return (0);
  }
  printf (" The Magic packet password is %2.2x %2.2x %2.2x %2.2x (%d).\n",
          passwd[0], passwd[1], passwd[2], passwd[3], byte_cnt);
  for (i = 0; i < byte_cnt; i++)
      wol_passwd[i] = passwd[i];
  return wol_passwd_sz = byte_cnt;
}

