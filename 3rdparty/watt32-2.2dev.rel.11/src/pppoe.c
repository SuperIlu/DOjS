/*!\file pppoe.c
 *
 *  PPPoE (PPP-over-Ethernet) for Watt-32. Refer RFC-2516 for spec.
 *
 *  Copyright (c) 1997-2002 Gisle Vanem <gvanem@yahoo.no>
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. All advertising materials mentioning features or use of this software
 *     must display the following acknowledgement:
 *       This product includes software developed by Gisle Vanem
 *       Bergen, Norway.
 *
 *  THIS SOFTWARE IS PROVIDED BY ME (Gisle Vanem) AND CONTRIBUTORS ``AS IS''
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL I OR CONTRIBUTORS BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  Version
 *
 *  0.5 : Aug 07, 2001 : G. Vanem - created
 *  0.6 : Aug 08, 2001 : Added extensions from
 *                       draft-carrel-info-pppoe-ext-00.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "wattcp.h"
#include "misc.h"
#include "timer.h"
#include "strings.h"
#include "language.h"
#include "run.h"
#include "pcconfig.h"
#include "pcdbug.h"
#include "pcqueue.h"
#include "pctcp.h"
#include "pcsed.h"
#include "pcpkt.h"
#include "pppoe.h"
#include "ppp.h"

#if defined(USE_PPPOE)

enum States {
     STATE_NONE = 0,
     STATE_DISCOVERY,
     STATE_SESSION
   };

static void (W32_CALL *prev_hook) (const char*, const char*) = NULL;

static BOOL        got_PADO;
static BOOL        got_PADS;
static BOOL        got_PADT;
static BOOL        got_PADM;
static WORD        session = 0;
static enum States state   = STATE_NONE;
static mac_address ac_mac_addr;  /* Access Consentrator's (AC) MAC address */

static struct {
       int  enable;
       int  trace;
       int  timeout;
       int  retries;
       int  lcp_echo_intv;                /**< Not used */
       char service_name [MAX_VALUELEN+1];
       char user_name [MAX_VALUELEN+1];   /**< \todo PPP user/passwd not handled */
       char password  [MAX_VALUELEN+1];
     } cfg = { 0, 0, 1, 3, 60, "", "", "" };

static int pppoe_send_disc (int code);

static void W32_CALL pppoe_config (const char *name, const char *value)
{
  static const struct config_table pppoe_cfg[] = {
       { "ENABLE",      ARG_ATOI,   (void*)&cfg.enable        },
       { "TRACE",       ARG_ATOI,   (void*)&cfg.trace         },
       { "TIMEOUT",     ARG_ATOI,   (void*)&cfg.timeout       },
       { "RETRIES",     ARG_ATOI,   (void*)&cfg.retries       },
       { "LCP_ECHO",    ARG_ATOI,   (void*)&cfg.lcp_echo_intv },
       { "SERVICENAME", ARG_STRCPY, (void*)&cfg.service_name  },
       { "USER",        ARG_STRCPY, (void*)&cfg.user_name     },
       { "PASSWD",      ARG_STRCPY, (void*)&cfg.password      },
       { NULL,          0,          NULL                      }
     };
  if (!parse_config_table(&pppoe_cfg[0], "PPPOE.", name, value) && prev_hook)
     (*prev_hook) (name, value);
}

/**
 * Loop waiting for timeout or a PAD response.
 * Blocks until some got_X flags are set in by pppoe_handler().
 */
static BOOL pppoe_wait (int wait_code)
{
  DWORD timer = set_timeout (1000 * cfg.timeout);

  while (1)
  {
    tcp_tick (NULL);

    if ((wait_code == PPPOE_CODE_PADO && got_PADO) ||
        (wait_code == PPPOE_CODE_PADS && got_PADS))
    {
#if defined(USE_DEBUG)
      if (cfg.trace)
         (*_printf) ("PPPoE: got %s\n", pppoe_get_code(wait_code));
#endif
      return (TRUE);
    }
    if (chk_timeout(timer))
       break;
  }
  return (FALSE);
}

/**
 * The PPPoE Discovery handler. Send PAD Init, wait for PAD offer,
 * send PAD Request and wait for PAD Session confirmation.
 */
static BOOL pppoe_discovery (void)
{
  int loop;

  for (loop = 1; loop <= cfg.retries; loop++)
  {
    if (!pppoe_send_disc(PPPOE_CODE_PADI))  /* send Init packet */
       return (FALSE);

    if (pppoe_wait(PPPOE_CODE_PADO))        /* wait for Offer */
       break;
  }
  if (loop > cfg.retries)
     return (FALSE);

  for (loop = 1; loop <= cfg.retries; loop++)
  {
    if (!pppoe_send_disc(PPPOE_CODE_PADR))  /* send Request */
       return (FALSE);

    if (pppoe_wait(PPPOE_CODE_PADS))        /* wait for Session-confirm */
       break;
  }
  return (loop <= cfg.retries);
}

/**
 * Set config-parser hook and initial values.
 */
void pppoe_init (void)
{
  prev_hook = usr_init;
  usr_init  = pppoe_config;
  state     = STATE_DISCOVERY;
  got_PADO  = got_PADS = got_PADT = FALSE;
  memcpy (ac_mac_addr, _eth_brdcast, sizeof(ac_mac_addr));
}


/**
 * Start PPPoE by doing a Discovery. Adjust MSS/MTU for PPPoE
 * packets (8 byte less than normal DIX ethernet).
 * Enter Session state and kick-start PPP (LCP/IPCP).
 *
 * \todo Fix-me: The new MTU/MSS affects all connections (also those
 *       which doesn't use PPPoE framing).
 */
int pppoe_start (void)
{
  if (!cfg.enable || _pktdevclass != PDCLASS_ETHER)
     return (1);

  if (!pppoe_discovery())
     return (0);

#undef  MSS_MAX
#define MSS_MAX (PPPOE_MAX_DATA - TCP_OVERHEAD)
  _mss = min (_mss, MSS_MAX);

  /* If not already clamped by user/pktdrvr, set MTU == 1492
   */
  if (_mtu > PPPOE_MAX_DATA)
      _mtu = PPPOE_MAX_DATA;

  state = STATE_SESSION;
  ppp_start (cfg.trace);
  return (1);
}

/**
 * Close down PPPoE by sending a PADT.
 */
void pppoe_exit (void)
{
  if (!cfg.enable || state == STATE_NONE)
     return;

  pppoe_send_disc (PPPOE_CODE_PADT);
  session = 0;
  state   = STATE_NONE;
  memcpy (ac_mac_addr, _eth_brdcast, sizeof(ac_mac_addr));
}

/**
 * Handle incoming PPPoE packets.
 */
int pppoe_handler (const pppoe_Packet *pkt)
{
  const BYTE *buf;
  const void *src;
  const void *dst;
  WORD  proto, len;
  BOOL  bcast, delivered = FALSE;

  if (pkt->type != 1 || pkt->ver != 1)
     return (0);

  src   = MAC_SRC (pkt);
  dst   = MAC_DST (pkt);
  proto = MAC_TYP (pkt);
  bcast = !memcmp (dst, _eth_brdcast, _eth_mac_len);

  if (proto == PPPOE_SESS_TYPE && state == STATE_SESSION)
  {
    if (pkt->code    == PPPOE_CODE_SESS &&
        pkt->session == session         &&
        !bcast                          &&
        !memcmp(dst, _eth_addr, _eth_mac_len) &&  /* to us? */
        !memcmp(src, ac_mac_addr, _eth_mac_len))
    {
      len = intel16 (pkt->length);
      buf = &pkt->data[0];
      ppp_input (buf, len);    /* assume ppp_input() traces it */
      delivered = TRUE;
    }
  }
  else if (!bcast && proto == PPPOE_DISC_TYPE && state == STATE_DISCOVERY)
  {
    if (pkt->code == PPPOE_CODE_PADO)          /* Offer (can this be bcast?) */
    {
      got_PADO = TRUE;
      memcpy (ac_mac_addr, src, _eth_mac_len);
    }
    else if (pkt->code == PPPOE_CODE_PADT &&  /* Terminate */
             pkt->session == session)
    {
      if (cfg.trace)
         outsnl (_LANG("PPPoE: session terminated"));
      got_PADT = TRUE;
      session  = 0;
    }
    else if (pkt->code == PPPOE_CODE_PADS)    /* Session-confirmation */
    {
      got_PADS = TRUE;
      session  = pkt->session;
    }
    else if (pkt->code == PPPOE_CODE_PADM)    /* Message (what to do?) */
    {
      got_PADM = TRUE;
    }
  }
  if (!delivered)
     DEBUG_RX (NULL, pkt);
  return (1);
}

/**
 * Determine if we have a PPPoE session with the machine given
 * by 'dest'. Can only have a session with the AC.
 */
BOOL pppoe_is_up (const void *dest)
{
#if 1
  if (!cfg.enable || memcmp(dest,ac_mac_addr,_eth_mac_len))
     return (FALSE);
  return (session != 0 && !got_PADT);
#else
  session = 0xAA55;
  return (TRUE);       /* test */
#endif
}

/**
 * Build a PADx packet.
 */
static WORD build_pad (struct pppoe_Packet *pkt, WORD code)
{
  BYTE *tags = &pkt->data[0];

  if (code == PPPOE_CODE_PADI || code == PPPOE_CODE_PADR)
  {
    WORD srv_len = 0;

    *(WORD*) tags = PPPOE_TAG_SERVICE_NAME;
    tags += 2;
    if (cfg.service_name[0])
    {
      srv_len = strlen (cfg.service_name);
      srv_len = min (srv_len, sizeof(pkt->data)-2-PPPOE_TAG_HDR_SIZE);
      memcpy (tags+2, cfg.service_name, srv_len);
    }
    *(WORD*) tags = intel16 (srv_len);

    /* Insert PPPOE_TAG_END_LIST ?
     */
    return (srv_len + PPPOE_TAG_HDR_SIZE);
  }
  *(WORD*)tags = 0;  /* No tags */
  return (0);
}

/**
 * Build and send a PADx (PPPoE Active Discovery) packet as
 * link-layer broadcast or unicast.
 */
static int pppoe_send_disc (int code)
{
  pppoe_Packet *pkt;
  WORD          len;

  pkt = (pppoe_Packet*) _eth_formatpacket (&ac_mac_addr[0], PPPOE_DISC_TYPE);
  len = build_pad (pkt, code);

  pkt->ver     = 1;
  pkt->type    = 1;
  pkt->code    = code;
  pkt->session = session;
  pkt->length  = intel16 (len);

#if defined(USE_DEBUG)
  if (cfg.trace)
     (*_printf) ("PPPOE: sending %s\n", pppoe_get_code(code));
#endif

  return _eth_send (len + PPPOE_HDR_SIZE, NULL, __FILE__, __LINE__);
}

/**
 * Build and send a PPPoE Session packet (IPCP or LCP packet).
 */
int pppoe_send_sess (const void *sock, const BYTE *buf, WORD len)
{
  pppoe_Packet *pkt;

  pkt = (pppoe_Packet*) _eth_formatpacket (&ac_mac_addr[0], PPPOE_SESS_TYPE);

  pkt->ver     = 1;
  pkt->type    = 1;
  pkt->code    = PPPOE_CODE_SESS;
  pkt->session = session;
  pkt->length  = intel16 (len);

  memcpy (pkt->data, buf, len);
  return _eth_send (len + PPPOE_HDR_SIZE, sock, __FILE__, __LINE__);
}

/**
 * Build a PPPoE session packet header.
 * Only called for IPv4 packets.
 */
void *pppoe_mac_format (union link_Packet *tx)
{
  pppoe_Packet *pppoe = (pppoe_Packet*) tx->eth.data;

  memcpy (&tx->eth.head.destination[0], ac_mac_addr, sizeof(mac_address));
  memcpy (&tx->eth.head.source[0], _eth_addr, sizeof(mac_address));

  tx->eth.head.type = PPPOE_SESS_TYPE;
  pppoe->ver     = 1;
  pppoe->type    = 1;
  pppoe->code    = PPPOE_CODE_SESS;
  pppoe->session = session;
  _pkt_ip_ofs    = sizeof(eth_Header) + PPPOE_HDR_SIZE + 2; /* !! see _eth_send() */
  *(WORD*) &pppoe->data[0] = intel16 (PPP_IP);
  return (void*) &pppoe->data[2];
}
#endif /* USE_PPPOE */

