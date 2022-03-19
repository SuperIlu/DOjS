/*!\file chksum.c
 *
 * Compute internet checksum on data buffer.
 * 1's complement (~) is done by caller.
 */

#include <stdio.h>
#include <stdlib.h>

#include "wattcp.h"
#include "pcicmp.h"
#include "pcdbug.h"
#include "pcstat.h"
#include "language.h"
#include "strings.h"
#include "misc.h"
#include "ip6_in.h"
#include "chksum.h"

/**
 * This checksum routine is only used by 16-bit targets (and files
 * outside the library). 32-bit targets use in_checksum_fast() in
 * chksum0.s + chksum0.asm.
 *
 * \note `in_check_sum()' and `in_checksum_fast()' are equally fast
 *       on a Pentium 4 CPU. But on a Pentium 2 the ASM version is
 *       approx. 10 times faster.
 */
WORD W32_CALL in_checksum (const void *ptr, unsigned len)
{
  register long  sum       = 0;
  register long  slen      = (long) len;   /* must be signed */
  register const WORD *wrd = (const WORD*) ptr;

  #if defined(WIN32) || defined(__DJGPP__)
  // WATT_ASSERT (0);  /* Why should we use this slow checksum routine? */
  #endif

  while (slen > 1)
  {
    sum  += *wrd++;
    slen -= 2;
  }
  if (slen > 0)
     sum += *(const BYTE*)wrd;

  while (sum >> 16)
      sum = (sum & 0xFFFF) + (sum >> 16);

  return (WORD)sum;
}

#if defined(USE_IPV6)
/**
 * Generic IPv6 checksum function.
 */
WORD _ip6_checksum (const in6_Header *ip, WORD proto,
                    const void *payload, unsigned payloadlen)
{
  tcp_PseudoHeader6 ph;
  DWORD chksum;

  memset (&ph, 0, sizeof(ph));
  memcpy (&ph.src, &ip->source, sizeof(ph.src));
  memcpy (&ph.dst, &ip->destination, sizeof(ph.dst));
  ph.length   = intel16 (payloadlen);
  ph.next_hdr = (BYTE)proto;

  chksum  = CHECKSUM (&ph, sizeof(ph));
  chksum += CHECKSUM (payload, payloadlen);

  /* Wrap in the carries to reduce chksum to 16 bits.
   */
  chksum  = (chksum >> 16) + (chksum & 0xFFFF);
  chksum += (chksum >> 16);

  /* Take ones-complement and replace 0 with 0xFFFF.
   */
  chksum = (WORD)~chksum;
  if (chksum == 0UL)
     chksum = 0xFFFFUL;

  return (WORD)chksum;
}

/**
 * Check tcp header checksum of an IPv6 packet.
 */
int _ip6_tcp_checksum (const in6_Header *ip, const tcp_Header *tcp, unsigned len)
{
  if (_ip6_checksum(ip, IP6_NEXT_TCP, tcp, len) != 0xFFFF)
  {
    STAT (tcpstats.tcps_rcvbadsum++);
    if (debug_on)
       outsnl (_LANG("bad IPv6/TCP checksum"));
    return (0);
  }
  return (1);
}

/**
 * Check udp header checksum of an IPv6 packet
 */
int _ip6_udp_checksum (const in6_Header *ip, const udp_Header *udp, unsigned len)
{
  if (_ip6_checksum(ip, IP6_NEXT_UDP, udp, len) != 0xFFFF)
  {
    STAT (udpstats.udps_badsum++);
    if (debug_on)
       outsnl (_LANG("bad IPv6/UDP checksum"));
    return (0);
  }
  return (1);
}

/*
 * Check ICMP header checksum of an IPv6 packet.
 * Caller upcates stats.
 */
int _ip6_icmp_checksum (const in6_Header *ip, const void *icmp, unsigned len)
{
  return (_ip6_checksum(ip, IP6_NEXT_ICMP, icmp, len) == 0xFFFF);
}
#endif /* USE_IPV6 */


#if defined(WIN32) && 0
/**
 * Do the IP checksum in NIC hardware.
 */
int ndis_in_checksum_offload (const void *ptr, unsigned len)
{
}
#endif  /* WIN32 */

#if defined(NOT_USED)

#define CKSUM_CARRY(x) (x = (x >> 16) + (x & 0xffff), \
                        (~(x + (x >> 16)) & 0xffff) )

#ifndef IP_PROTO
#define IP_PROTO 0  /* dummy for IP */
#endif

/*
 * Set header checksum on outgoing packets.
 * Dug Song came up with this very cool checksuming implementation
 * eliminating the need for explicit psuedoheader use. Check it out.
 *
 * Ripped from libnet 1.0.1
 */
int do_checksum (const BYTE *buf, BYTE protocol, unsigned len)
{
  struct in_Header     *ip = (struct in_Header*) buf;
  struct tcp_Header    *tcp;
  struct udp_Header    *udp;
  struct IGMPv1_packet *igmp;
  union  icmp_pkt      *icmp;
  WORD  sum = 0;

  unsigned ip_hlen = in_GetHdrLen (ip);

  switch (protocol)
  {
    case TCP_PROTO:
         tcp = (struct tcp_Header*) (buf + ip_hlen);
         tcp->checksum = 0;
         sum = CHECKSUM (&ip->source, 8);
         sum += intel16 (TCP_PROTO + len);
         sum += CHECKSUM (tcp, len);
         tcp->checksum = CKSUM_CARRY (sum);
         break;

    case UDP_PROTO:
         udp = (struct udp_Header*) (buf + ip_hlen);
         udp->checksum = 0;
         sum = CHECKSUM (&ip->source, 8);
         sum += intel16 (UDP_PROTO + len);
         sum += CHECKSUM (udp, len);
         udp->checksum = CKSUM_CARRY (sum);
         break;

    case ICMP_PROTO:
         icmp = (union icmp_pkt*) (buf + ip_hlen);
         icmp->unused.checksum = 0;
         sum = CHECKSUM (icmp, len);
         icmp->unused.checksum = CKSUM_CARRY (sum);
         break;

    case IGMP_PROTO:
         igmp = (struct IGMPv1_packet*) (buf + ip_hlen);
         igmp->checksum = 0;
         sum += CHECKSUM (igmp, len);
         igmp->checksum = CKSUM_CARRY (sum);
         break;

    case IP_PROTO:
         ip->checksum = 0;
         sum = CHECKSUM (ip, len);
         ip->checksum = CKSUM_CARRY (sum);
         break;

    default:
        TCP_CONSOLE_MSG (2, ("do_checksum: unknown protocol %d\n", protocol));
        return (-1);
  }
  return (1);
}
#endif  /* NOT_USED */

