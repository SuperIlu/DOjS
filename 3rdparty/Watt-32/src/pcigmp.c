/*!\file pcigmp.c
 *
 * IP Multicasting extensions as per RFC 1112 (IGMP v1).
 * Ref.:
 *   http://www.networksorcery.com/enp/protocol/igmp.htm
 *
 * These extensions include routines to detect multicast addresses,
 * transform Multicast IP addresses to Multicast Ethernet addresses, as
 * well as a mechanism to join and leave multicast groups.
 *
 * Jim Martin
 * Rutgers University - RUCS-TD/NS
 * jim@noc.rutgers.edu
 * 6/6/93
 *
 * \todo Support IGMP v3 (draft-ietf-idmr-igmp-v3-??.txt)
 */

#include <stdio.h>

#include "copyrigh.h"
#include "wattcp.h"
#include "misc.h"
#include "misc_str.h"
#include "run.h"
#include "timer.h"
#include "chksum.h"
#include "ip4_in.h"
#include "ip4_out.h"
#include "netaddr.h"
#include "pcconfig.h"
#include "pcpkt.h"
#include "pcsed.h"
#include "pctcp.h"
#include "pcdbug.h"
#include "pcstat.h"
#include "pcigmp.h"

/*
 * Application must set this to non-zero before calling watt_sock_init().
 * Not needed if using the BSD-socket API. Set for mcast options in
 * setsockopt().
 */
int _multicast_on = 0;
int _multicast_intvl = 0;   /* Send a report every x sec when idling */

#if defined(USE_MULTICAST)

W32_CLANG_PACK_WARN_OFF()

#include <sys/pack_on.h>

struct IGMP_PKT {
       in_Header     in;
       IGMPv1_packet igmp;
     };

#include <sys/pack_off.h>

W32_CLANG_PACK_WARN_DEF()

#define ETH_MULTI     0x01005EUL    /**< high order bits of multi eth addr */
#define CLASS_D_MASK  0xE0000000UL  /**< the mask that defines IP Class D  */
#define IPMULTI_MASK  0x007FFFFFUL  /**< to get the low-order 23 bits      */

static struct MultiCast mcast_list [IPMULTI_SIZE];
static BOOL             daemon_on = FALSE;

static void W32_CALL check_mcast_reports (void);

/*
 * Check the LSB bit 0 of ether destination address.
 */
static __inline BOOL is_eth_multicast (const in_Header *ip)
{
  const BYTE *eth_dst;

  /* How could this happen? Assume yes anyway
   */
  if (_pktdevclass != PDCLASS_ETHER)
     return (TRUE);
  eth_dst = (const BYTE*) MAC_DST (ip);
  return (eth_dst[0] & 1);  /* LSB=1 is mcast */
}

/*
 * Calculates the proper ethernet address for a given
 * IP Multicast address.
 *
 * Where:
 *   ip:  IP address to be converted
 *   eth: pointer to ethernet MAC address
 */
int W32_CALL multi_to_eth (DWORD ip, eth_address *mac)
{
  DWORD top = ETH_MULTI;
  BYTE *eth = (BYTE*)mac;

  ip &= IPMULTI_MASK;
  eth[0] = (BYTE) (top >> 16);
  eth[1] = (BYTE) ((top >> 8) & 0xFF);
  eth[2] = (BYTE) (top & 0xFF);
  eth[3] = (BYTE) (ip >> 16);
  eth[4] = (BYTE) ((ip >> 8) & 0xFF);
  eth[5] = (BYTE) (ip & 0xFF);
  return (1);
}

/*
 * Joins a multicast group
 *
 * Where:
 *   ip - address of the group to be joined (host order)
 * Returns:
 *   1 - if the group was joined successfully
 *   0 - if attempt failed
 */
int W32_CALL join_mcast_group (DWORD ip)
{
  struct MultiCast *mc;
  int    i;

  if (!_multicast_on)
     return (0);

  /* first verify that it's a valid mcast address
   */
  if (!_ip4_is_multicast(ip))
  {
    TRACE_CONSOLE (1, "%s isn't a multicast address\n", _inet_ntoa(NULL,ip));
    return (0);
  }

  if (!daemon_on)
     DAEMON_ADD (check_mcast_reports);
  daemon_on = TRUE;

  /* Determine if the group has already been joined.
   * As well as what the first free slot is.
   */
  for (i = 0, mc = mcast_list; i < DIM(mcast_list); i++, mc++)
  {
    if (mc->active && mc->ip == ip)
    {
      if (mc->processes < 255)
          mc->processes++;
      return (1);
    }
    if (!mc->active)
       break;
  }

  /* alas, no...we need to join it
   */
  if (i >= DIM(mcast_list))     /* out of slots! */
     return (0);

  /* Fill in the hardware address
   */
  multi_to_eth (ip, &mc->ethaddr);

  if (!_eth_join_mcast_group(mc))
     return (0);

  mc->active      = TRUE;
  mc->ip          = ip;
  mc->reply_timer = set_timeout (0); /* report ASAP */
  mc->processes   = 1;
  return (1);
}

/*
 * Leaves a multicast group
 *
 * Where:
 *   ip - address of the group to be joined
 * Returns:
 *   1 - if the group was left successfully
 *   0 - if attempt failed
 */
int W32_CALL leave_mcast_group (DWORD ip)
{
  struct MultiCast *mc;
  int    i, rc, num_total = 0;

  if (!_multicast_on)
     return (0);

  /* first verify that it's a valid mcast address
   */
  if (!_ip4_is_multicast(ip))
  {
    TRACE_CONSOLE (1, "%s isn't a multicast address\n", _inet_ntoa(NULL,ip));
    return (0);
  }

  /* Determine if the group has more than one interested
   * process. If so, then just decrement ref-count and return
   */
  for (i = 0, mc = mcast_list; i < DIM(mcast_list); i++, mc++)
  {
    if (!mc->active)
       continue;

    num_total++;
    if (mc->ip != ip)
       continue;

    if (mc->processes > 1)
    {
      mc->processes--;
      return (1);
    }
    break;
  }

  /* did the IP-addr they gave match anything in mcast_list ??
   */
  if (i >= DIM(mcast_list))
     return (0);

  /* alas...we need to physically leave it
   */
  rc = _eth_leave_mcast_group (mc);
  mc->active = FALSE;

  /* Remove daemon if no longer needed
   */
  if (num_total <= 1)
  {
    daemon_on = FALSE;
    DAEMON_DEL (check_mcast_reports);
  }
  return (rc);
}

/*
 * Count number of active multicast groups
 */
int W32_CALL num_mcast_active (void)
{
  int i, num = 0;

  for (i = 0; i < DIM(mcast_list); i++)
      if (mcast_list[i].active)
         num++;
  return (num);
}

/*
 * Handles the incoming IGMP packets
 *
 * Where:
 *   ip - the IP packet in question
 */
void igmp_handler (const in_Header *ip, BOOL broadcast)
{
  BYTE  i;
  DWORD src_ip, host;
  BOOL  found = FALSE;
  WORD  len   = in_GetHdrLen (ip);
  const IGMPv1_packet *igmp = (const IGMPv1_packet*) ((BYTE*)ip + len);
  struct MultiCast    *mc;

  DEBUG_RX (NULL, ip);

  if (igmp->version != IGMP_VERSION_1)  /* We only speak v1 */
     return;

  len = intel16 (ip->length) - len;
  if (len < sizeof(*igmp))
  {
    STAT (igmpstats.igps_rcv_tooshort++);
    return;
  }

  if (CHECKSUM(igmp,sizeof(*igmp)) != 0xFFFF)
  {
    STAT (igmpstats.igps_rcv_badsum++);
    return;
  }

  host   = intel (igmp->address);
  src_ip = intel (ip->source);

  /* drop our own looped packets
   */
  if (_ip4_is_local_addr(src_ip))
     return;

  /* Determine whether this is a report or a query
   */
  switch (igmp->type)
  {
    case IGMPv1_QUERY:
         STAT (igmpstats.igps_rcv_queries++);
         for (i = 0, mc = mcast_list; i < DIM(mcast_list); i++, mc++)
             if (mc->active && mc->reply_timer == 0UL &&
                 mc->ip != MCAST_ALL_SYST)
             {
               mc->reply_timer = set_timeout (Random(500,1000));
               found = TRUE;
             }
         if (!found && !broadcast && !is_eth_multicast(ip))
            STAT (igmpstats.igps_rcv_badqueries++);
         break;

    case IGMPv1_REPORT:
         STAT (igmpstats.igps_rcv_reports++);
         for (i = 0, mc = mcast_list; i < DIM(mcast_list); i++, mc++)
             if (mc->active && mc->ip == host &&
                 host != MCAST_ALL_SYST)
             {
               mc->reply_timer = 0UL;
               found = TRUE;
               STAT (igmpstats.igps_rcv_ourreports++);
               break;
             }
         if (!found && !broadcast && !is_eth_multicast(ip))
            STAT (igmpstats.igps_rcv_badreports++);
         break;

     default:
         break;
  }
}

/*
 * Send a IGMPv1 Report packet
 *
 * Where:
 *   ip is the IP address to report (on host order).
 *
 * Returns:
 *   0 - if unable to send report
 *   1 - report was sent successfully
 */
static int igmp_report (DWORD ip)
{
  struct IGMP_PKT      *pkt;
  struct IGMPv1_packet *igmp;
  eth_address           eth;

  /* get the ethernet addr of the destination
   */
  multi_to_eth (MCAST_ALL_SYST, &eth);

  /* format the packet with the request's hardware address
   */
  pkt  = (struct IGMP_PKT*) _eth_formatpacket (eth, IP4_TYPE);
  igmp = &pkt->igmp;
  ip   = intel (ip);

  /* fill in the igmp packet
   */
  igmp->type     = IGMPv1_REPORT;
  igmp->version  = IGMP_VERSION_1;
  igmp->unused   = 0;
  igmp->address  = ip;
  igmp->checksum = 0;
  igmp->checksum = ~CHECKSUM (igmp, sizeof(*igmp));

  return IP4_OUTPUT (&pkt->in, 0, ip, IGMP_PROTO,
                     0, 0, 0, sizeof(*igmp), NULL);
}

/*
 * Check to see if we owe any IGMP Reports
 * Called as a daemon from tcp_tick()
 */
static void W32_CALL check_mcast_reports (void)
{
  struct MultiCast *mc;
  int    i;

  if (!_multicast_on)
     return;

  for (i = 0, mc = mcast_list; i < DIM(mcast_list); i++, mc++)
  {
    if (!mc->active || mc->ip == MCAST_ALL_SYST || !chk_timeout(mc->reply_timer))
       continue;

    mc->reply_timer = _multicast_intvl > 0 ?
                      set_timeout (1000 * _multicast_intvl) : 0UL;
    igmp_report (mc->ip);
  }
}
#endif /* USE_MULTICAST */

