/*!\file pcping.c
 *
 * Simple ping client.
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "copyrigh.h"
#include "wattcp.h"
#include "misc.h"
#include "timer.h"
#include "chksum.h"
#include "strings.h"
#include "language.h"
#include "ip4_in.h"
#include "ip4_out.h"
#include "pcsed.h"
#include "pcarp.h"
#include "pcdbug.h"
#include "pctcp.h"
#include "pcicmp.h"

// #undef time

/*!\struct ping_cache
 * Our little ping-cache.
 */
static struct ping_cache {
       DWORD  host;
       DWORD  time;
       DWORD  number;
     } pcache [10];

/**
 * Format and send an ICMP echo request (ping).
 *   \arg    countnum is the ICMP index value.
 *   \arg    pattern optional data that will be sent.
 *   \arg    len length of pattern data.
 *   \retval 0 if ARP fails or error in lower layer.
 *   \note   if \b pattern or \b len is 0, only an 12 byte ICMP echo
 *           header will be sent.
 *   \note   Sends fragments if needed.
 */
int W32_CALL _ping (DWORD host, DWORD countnum, const BYTE *pattern, size_t len)
{
  struct ping_pkt  *pkt;
  struct ICMP_echo *icmp;
  struct in_Header *ip;
  eth_address       dest;

  if (sin_mask != 0xFFFFFFFF && !_ip4_is_loopback_addr(host) &&
      !(~host & ~sin_mask))
  {
    outsnl (_LANG("Cannot ping a network!"));
    return (0);
  }

  if (!_arp_resolve(host,&dest))
  {
    outsnl (_LANG("Cannot resolve host's hardware address"));
    return (0);
  }

  if (debug_on >= 2)
  {
    outs (_LANG("\nDEBUG: destination hardware: "));
    outhexes ((char*)&dest, sizeof(dest));
    outs ("\n");
  }

  host = intel (host);

  /* Jumbo packets must be sent as IP fragments
   */
#if defined(USE_FRAGMENTS)
  if (len > (size_t)(_mtu - sizeof(*ip) - sizeof(*icmp)))
  {
    int rc = 0;

    len  = min (len, USHRT_MAX - sizeof(*ip) - sizeof(*icmp));
    icmp = (struct ICMP_echo*) malloc (len + sizeof(*icmp));
    if (!icmp)
    {
      if (debug_on)
         outsnl (_LANG("_ping(): malloc failed"));
      return (0);
    }

    if (pattern)
       memcpy (icmp+1, pattern, len);
    len += sizeof(*icmp);
    icmp->type       = ICMP_ECHO;
    icmp->code       = 0;
    icmp->index      = countnum;
    icmp->identifier = set_timeout (0) & 0xFFFF;  /* "random" id */
    icmp->sequence   = 0;
    icmp->checksum   = 0;
    icmp->checksum   = ~CHECKSUM (icmp, len);
    rc = _IP4_SEND_FRAGMENTS (NULL, ICMP_PROTO, host, icmp, len);
    free (icmp);
    return (rc < 0 ? 0 : rc);
  }
#else
  len = min (len, _mtu - sizeof(*ip) - sizeof(*icmp)); /* truncate */
#endif

  /* Ordinary small pings; format an Ethernet packet with IPv4 type
   */
  pkt  = (struct ping_pkt*) _eth_formatpacket (&dest, IP4_TYPE);
  ip   = &pkt->in;
  icmp = &pkt->icmp;

  if (pattern && len > 0)
  {
    len = min (len, _mtu-sizeof(struct ping_pkt));
    memcpy (icmp+1, pattern, len);   /* pattern after echo header */
    len += sizeof (*icmp);
  }
  else
    len = sizeof (*icmp);

  icmp->type       = ICMP_ECHO;
  icmp->code       = 0;
  icmp->index      = countnum;
  icmp->identifier = set_timeout (0) & 0xFFFF;  /* "random" id */
  icmp->sequence   = 0;
  icmp->checksum   = 0;
  icmp->checksum   = ~CHECKSUM (icmp, len);

  return IP4_OUTPUT (ip, 0, host, ICMP_PROTO, 0, (BYTE)_default_tos,
                     0, len, NULL);
}

/**
 * Compare two entries in ping-cache.
 */
static int MS_CDECL compare (const struct ping_cache *a,
                             const struct ping_cache *b)
{
  return ((long)a->number - (long)b->number);
}

/**
 * Add an ICMP echo reply to the ping-cache.
 */
void W32_CALL add_ping (DWORD host, DWORD time, DWORD number)
{
  int i;

  for (i = 0; i < DIM(pcache); i++)
      if (host && pcache[i].host == 0)   /* vacant entry */
      {
        if (time > 0x7FFFFFFFL)
            time += 0x1800B0L;
        pcache[i].host   = host;
        pcache[i].time   = time;
        pcache[i].number = number;
        qsort ((void*)&pcache, DIM(pcache), sizeof(pcache[0]), (CmpFunc)compare);
        break;
      }
}

/**
 * Check for ICMP echo reply in ping-cache.
 * \retval  reply time in system ticks.
 * \retval  (DWORD)-1 on failure
 */
DWORD W32_CALL _chk_ping (DWORD host, DWORD *number)
{
  int i;

  WATT_YIELD();

  for (i = 0; i < DIM(pcache); i++)
      if (host && pcache[i].host == host)
      {
        pcache[i].host = 0;    /* clear entry */
        if (number)
           *number = pcache[i].number;
        return (pcache[i].time);
      }
  return (0xFFFFFFFFUL);
}


