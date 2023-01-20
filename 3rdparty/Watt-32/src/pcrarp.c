/*!\file pcrarp.c
 *
 *  RARP - Boot Protocol (RFC 903).
 *
 *  These extensions get called if MY_IP is set to RARP in wattcp.cfg
 *
 *  Version
 *
 *  0.0 : Sept 6, 1996 : Copied from E. Engelke's pcbootp.c by Dan Kegel
 *  0.1 : Febr 9, 1997 : Modified, removed debug printing,  G. Vanem
 */

#include <stdio.h>
#include <string.h>

#include "copyrigh.h"
#include "wattcp.h"
#include "language.h"
#include "misc.h"
#include "misc_str.h"
#include "timer.h"
#include "pcsed.h"
#include "pctcp.h"
#include "pcdbug.h"
#include "pcrarp.h"

#if defined(USE_RARP)

WORD _rarptimeout = 5;

static int _rarp_request (void)
{
  rarp_Header *rarp = (rarp_Header*) _eth_formatpacket (&_eth_brdcast[0],
                                                        RARP_TYPE);
  rarp->hwType       = intel16 (_eth_get_hwtype(NULL,NULL));
  rarp->protType     = IP4_TYPE;
  rarp->hwAddrLen    = sizeof (mac_address);
  rarp->protoAddrLen = sizeof (my_ip_addr);
  rarp->opcode       = RARP_REQUEST;
  rarp->srcIPAddr    = 0;
  rarp->dstIPAddr    = 0;
  memcpy (rarp->srcEthAddr, _eth_addr, sizeof(mac_address));
  memcpy (rarp->dstEthAddr, _eth_addr, sizeof(mac_address));

  return _eth_send (sizeof(*rarp), NULL, __FILE__, __LINE__);
}

/**
 * Handle incoming RARP packets.
 */
BOOL _rarp_handler (const rarp_Header *rh, BOOL brdcast)
{
  DEBUG_RX (NULL, rh);

  if (!brdcast && rh->opcode == RARP_REPLY && rh->protType == IP4_TYPE &&
      !memcmp(rh->dstEthAddr,_eth_addr,sizeof(mac_address)))
  {
    my_ip_addr = intel (rh->dstIPAddr);
    return (TRUE);
  }
  return (FALSE);
}

/*
 * _dorarp - Checks global variable _rarptimeout
 *           returns 1 on success and sets ip address
 */
int _dorarp (void)
{
  DWORD rarptimeout    = set_timeout (1000 * _rarptimeout);
  WORD  magictimeout   = Random (7000, 14000);

  outs (_LANG("Configuring through RARP..."));

  while (1)
  {
    DWORD sendtimeout;

    if (!_rarp_request())
       break;

    sendtimeout   = set_timeout (magictimeout);
    magictimeout += Random (1000, 7000);

    while (!chk_timeout(sendtimeout))
    {
      const struct rarp_Header *rarp;
      WORD  eth_type;
      BOOL  bcast;

      if (chk_timeout(rarptimeout))
         return (0);

      WATT_YIELD();

      rarp = (rarp_Header*) _eth_arrived (&eth_type, &bcast);
      if (!rarp)
         continue;

      DEBUG_RX (NULL, rarp);

      if (eth_type == RARP_TYPE && !bcast &&
          rarp->opcode == RARP_REPLY && rarp->protType == IP4_TYPE &&
          !memcmp(rarp->dstEthAddr,_eth_addr,sizeof(mac_address)))
      {
        my_ip_addr = intel (rarp->dstIPAddr);
        _eth_free (rarp);
        return (1);
      }
      _eth_free (rarp);
    }
  }
  return (0);
}
#endif
