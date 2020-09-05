/*!\file ppp.c
 *
 * PPP tunneled over Ethernet (PPPOE_SESS_TYPE frames)
 *
 * This implements just LCP and IPCP, and then only a subset of each
 * of those.  This is not supported and probably shouldn't be used for
 * much more than simple experiments with PPP.  If you need a real
 * version of PPP, see Paul Mackerras' ppp-2.3. If you do try to use
 * this, don't say I didn't warn you.
 *
 * This code may be used for any purpose as long as the author's
 * copyright is cited in any source code distributed.  This code
 * is also available electronically from the author's web site.
 *
 * http://people.ne.mediaone.net/carlson/ppp
 *
 * http://www.workingcode.com/ppp
 *
 * Copyright 1997 by James Carlson and Working Code
 *
 * Adapted for Watt-32's PPPoE extension by
 * G. Vanem Aug-2000
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>
#include <netinet/in.h>

#include "wattcp.h"
#include "misc.h"
#include "strings.h"
#include "pcdbug.h"
#include "pcqueue.h"
#include "pcconfig.h"
#include "pcarp.h"
#include "pcsed.h"
#include "pcpkt.h"
#include "pctcp.h"
#include "run.h"
#include "ip4_in.h"
#include "netaddr.h"
#include "pppoe.h"
#include "ppp.h"

#if defined(USE_PPPOE)

#define INITIAL_TIMEOUT 2
#define MAX_TERMINATE   2
#define MAX_CONFIGURE   10
#define MAX_FAILURE     5
#define MAX_TIMEOUT     8


static DWORD remote_ip_base = 0x0A000000;
static DWORD remote_ip_mask = 0xFF000000;
static int   debug = 0;


static struct ppp_state mystate;

static void ipcp_addr_req (struct ppp_state *state,
                           struct ppp_xcp *xcp, const struct xcp_type *tp,
                           const BYTE *buf, int len);

static void ipcp_addr_nak (struct ppp_state *state,
                           struct ppp_xcp *xcp, const struct xcp_type *tp,
                           const BYTE *buf, int len);

static void ipcp_addr_show (struct ppp_state *state,
                            struct ppp_xcp *xcp, const struct xcp_type *tp,
                            const BYTE *buf, int len);

static void send_event (struct ppp_state *state, struct ppp_xcp *xcp,
                        enum ppp_event event);

static void change_phase (struct ppp_state *state, enum ppp_phase phase);


static const struct xcp_type lcp_types[] = {
                             { NULL, -1, 0, 0, 0, 0, NULL, NULL, NULL }
                           };

static const struct xcp_type ipcp_types[] = {
                             { "IP-Address", 3, 1, 6, 6, 0, ipcp_addr_req,
                               ipcp_addr_nak, ipcp_addr_show },
                             { NULL, -1, 0, 0, 0, 0, NULL, NULL, NULL }
                           };


/* Dispatch table from RFC 1661
 */
static const struct ppp_dispatch {
       enum xcp_state   next;
       enum xcp_action  act[3];
     } ppp_dispatch[10][16] = {

    { /* stInitial */
      /* evUp */    { stClosed,   { acNull, acNull, acNull } },
      /* evDown */  { stNoChange, { acNull, acNull, acNull } },
      /* evOpen */  { stStarting, { acTls,  acNull, acNull } },
      /* evClose */ { stClosed,   { acNull, acNull, acNull } },
      /* evTOp */   { stNoChange, { acNull, acNull, acNull } },
      /* evTOm */   { stNoChange, { acNull, acNull, acNull } },
      /* evRCRp */  { stNoChange, { acNull, acNull, acNull } },
      /* evRCRm */  { stNoChange, { acNull, acNull, acNull } },
      /* evRCA */   { stNoChange, { acNull, acNull, acNull } },
      /* evRCN */   { stNoChange, { acNull, acNull, acNull } },
      /* evRTR */   { stNoChange, { acNull, acNull, acNull } },
      /* evRTA */   { stNoChange, { acNull, acNull, acNull } },
      /* evRUC */   { stNoChange, { acNull, acNull, acNull } },
      /* evRXJp */  { stNoChange, { acNull, acNull, acNull } },
      /* evRXJm */  { stNoChange, { acNull, acNull, acNull } },
      /* evRXR */   { stNoChange, { acNull, acNull, acNull } },
    },
    { /* stStarting */
      /* evUp */    { stReqSent,  { acIrc,  acScr,  acNull } },
      /* evDown */  { stNoChange, { acNull, acNull, acNull } },
      /* evOpen */  { stStarting, { acNull, acNull, acNull } },
      /* evClose */ { stInitial,  { acTlf,  acNull, acNull } },
      /* evTOp */   { stNoChange, { acNull, acNull, acNull } },
      /* evTOm */   { stNoChange, { acNull, acNull, acNull } },
      /* evRCRp */  { stNoChange, { acNull, acNull, acNull } },
      /* evRCRm */  { stNoChange, { acNull, acNull, acNull } },
      /* evRCA */   { stNoChange, { acNull, acNull, acNull } },
      /* evRCN */   { stNoChange, { acNull, acNull, acNull } },
      /* evRTR */   { stNoChange, { acNull, acNull, acNull } },
      /* evRTA */   { stNoChange, { acNull, acNull, acNull } },
      /* evRUC */   { stNoChange, { acNull, acNull, acNull } },
      /* evRXJp */  { stNoChange, { acNull, acNull, acNull } },
      /* evRXJm */  { stNoChange, { acNull, acNull, acNull } },
      /* evRXR */   { stNoChange, { acNull, acNull, acNull } },
    },
    { /* stClosed */
      /* evUp */    { stNoChange, { acNull, acNull, acNull } },
      /* evDown */  { stInitial,  { acNull, acNull, acNull } },
      /* evOpen */  { stReqSent,  { acIrc,  acScr,  acNull } },
      /* evClose */ { stClosed,   { acNull, acNull, acNull } },
      /* evTOp */   { stNoChange, { acNull, acNull, acNull } },
      /* evTOm */   { stNoChange, { acNull, acNull, acNull } },
      /* evRCRp */  { stClosed,   { acSta,  acNull, acNull } },
      /* evRCRm */  { stClosed,   { acSta,  acNull, acNull } },
      /* evRCA */   { stClosed,   { acSta,  acNull, acNull } },
      /* evRCN */   { stClosed,   { acSta,  acNull, acNull } },
      /* evRTR */   { stClosed,   { acSta,  acNull, acNull } },
      /* evRTA */   { stClosed,   { acNull, acNull, acNull } },
      /* evRUC */   { stClosed,   { acScj,  acNull, acNull } },
      /* evRXJp */  { stClosed,   { acNull, acNull, acNull } },
      /* evRXJm */  { stClosed,   { acTlf,  acNull, acNull } },
      /* evRXR */   { stClosed,   { acNull, acNull, acNull } },
    },
    { /* stStopped */
      /* evUp */    { stNoChange, { acNull, acNull, acNull } },
      /* evDown */  { stStarting, { acTls,  acNull, acNull } },
      /* evOpen */  { stStopped,  { acNull, acNull, acNull } },
      /* evClose */ { stClosed,   { acNull, acNull, acNull } },
      /* evTOp */   { stNoChange, { acNull, acNull, acNull } },
      /* evTOm */   { stNoChange, { acNull, acNull, acNull } },
      /* evRCRp */  { stAckSent,  { acIrc,  acScr,  acSca  } },
      /* evRCRm */  { stReqSent,  { acIrc,  acScr,  acScn  } },
      /* evRCA */   { stStopped,  { acSta,  acNull, acNull } },
      /* evRCN */   { stStopped,  { acSta,  acNull, acNull } },
      /* evRTR */   { stStopped,  { acSta,  acNull, acNull } },
      /* evRTA */   { stStopped,  { acNull, acNull, acNull } },
      /* evRUC */   { stStopped,  { acScj,  acNull, acNull } },
      /* evRXJp */  { stStopped,  { acNull, acNull, acNull } },
      /* evRXJm */  { stStopped,  { acTlf,  acNull, acNull } },
      /* evRXR */   { stStopped,  { acNull, acNull, acNull } },
    },
    { /* stClosing */
      /* evUp */    { stNoChange, { acNull, acNull, acNull } },
      /* evDown */  { stInitial,  { acNull, acNull, acNull } },
      /* evOpen */  { stStopping, { acNull, acNull, acNull } },
      /* evClose */ { stClosing,  { acNull, acNull, acNull } },
      /* evTOp */   { stClosing,  { acStr,  acNull, acNull } },
      /* evTOm */   { stClosed,   { acTlf,  acNull, acNull } },
      /* evRCRp */  { stClosing,  { acNull, acNull, acNull } },
      /* evRCRm */  { stClosing,  { acNull, acNull, acNull } },
      /* evRCA */   { stClosing,  { acNull, acNull, acNull } },
      /* evRCN */   { stClosing,  { acNull, acNull, acNull } },
      /* evRTR */   { stClosing,  { acSta,  acNull, acNull } },
      /* evRTA */   { stClosed,   { acTlf,  acNull, acNull } },
      /* evRUC */   { stClosing,  { acScj,  acNull, acNull } },
      /* evRXJp */  { stClosing,  { acNull, acNull, acNull } },
      /* evRXJm */  { stClosed,   { acTlf,  acNull, acNull } },
      /* evRXR */   { stClosing,  { acNull, acNull, acNull } },
    },
    { /* stStopping */
      /* evUp */    { stNoChange, { acNull, acNull, acNull } },
      /* evDown */  { stInitial,  { acNull, acNull, acNull } },
      /* evOpen */  { stStopping, { acNull, acNull, acNull } },
      /* evClose */ { stClosing,  { acNull, acNull, acNull } },
      /* evTOp */   { stStopping, { acStr,  acNull, acNull } },
      /* evTOm */   { stStopped,  { acTlf,  acNull, acNull } },
      /* evRCRp */  { stStopping, { acNull, acNull, acNull } },
      /* evRCRm */  { stStopping, { acNull, acNull, acNull } },
      /* evRCA */   { stStopping, { acNull, acNull, acNull } },
      /* evRCN */   { stStopping, { acNull, acNull, acNull } },
      /* evRTR */   { stStopping, { acSta,  acNull, acNull } },
      /* evRTA */   { stStopped,  { acTlf,  acNull, acNull } },
      /* evRUC */   { stStopping, { acScj,  acNull, acNull } },
      /* evRXJp */  { stStopping, { acNull, acNull, acNull } },
      /* evRXJm */  { stStopped,  { acTlf,  acNull, acNull } },
      /* evRXR */   { stStopping, { acNull, acNull, acNull } },
    },
    { /* stReqSent */
      /* evUp */    { stNoChange, { acNull, acNull, acNull } },
      /* evDown */  { stStarting, { acNull, acNull, acNull } },
      /* evOpen */  { stReqSent,  { acNull, acNull, acNull } },
      /* evClose */ { stClosing,  { acIrc,  acStr,  acNull } },
      /* evTOp */   { stReqSent,  { acScr,  acNull, acNull } },
      /* evTOm */   { stStopped,  { acTlf,  acNull, acNull } },
      /* evRCRp */  { stAckSent,  { acSca,  acNull, acNull } },
      /* evRCRm */  { stReqSent,  { acScn,  acNull, acNull } },
      /* evRCA */   { stAckRcvd,  { acIrc,  acNull, acNull } },
      /* evRCN */   { stReqSent,  { acIrc,  acScr,  acNull } },
      /* evRTR */   { stReqSent,  { acSta,  acNull, acNull } },
      /* evRTA */   { stReqSent,  { acNull, acNull, acNull } },
      /* evRUC */   { stReqSent,  { acScj,  acNull, acNull } },
      /* evRXJp */  { stReqSent,  { acNull, acNull, acNull } },
      /* evRXJm */  { stStopped,  { acTlf,  acNull, acNull } },
      /* evRXR */   { stReqSent,  { acNull, acNull, acNull } },
    },
    { /* stAckRcvd */
      /* evUp */    { stNoChange, { acNull, acNull, acNull } },
      /* evDown */  { stStarting, { acNull, acNull, acNull } },
      /* evOpen */  { stAckRcvd,  { acNull, acNull, acNull } },
      /* evClose */ { stClosing,  { acIrc,  acStr,  acNull } },
      /* evTOp */   { stReqSent,  { acScr,  acNull, acNull } },
      /* evTOm */   { stStopped,  { acTlf,  acNull, acNull } },
      /* evRCRp */  { stOpened,   { acSca,  acTlu,  acNull } },
      /* evRCRm */  { stAckRcvd,  { acScn,  acNull, acNull } },
      /* evRCA */   { stReqSent,  { acScr,  acNull, acNull } },
      /* evRCN */   { stReqSent,  { acScr,  acNull, acNull } },
      /* evRTR */   { stReqSent,  { acSta,  acNull, acNull } },
      /* evRTA */   { stReqSent,  { acNull, acNull, acNull } },
      /* evRUC */   { stAckRcvd,  { acScj,  acNull, acNull } },
      /* evRXJp */  { stReqSent,  { acNull, acNull, acNull } },
      /* evRXJm */  { stStopped,  { acTlf,  acNull, acNull } },
      /* evRXR */   { stAckRcvd,  { acNull, acNull, acNull } },
    },
    { /* stAckSent */
      /* evUp */    { stNoChange, { acNull, acNull, acNull } },
      /* evDown */  { stStarting, { acNull, acNull, acNull } },
      /* evOpen */  { stAckSent,  { acNull, acNull, acNull } },
      /* evClose */ { stClosing,  { acIrc,  acStr,  acNull } },
      /* evTOp */   { stAckSent,  { acScr,  acNull, acNull } },
      /* evTOm */   { stStopped,  { acTlf,  acNull, acNull } },
      /* evRCRp */  { stAckSent,  { acSca,  acNull, acNull } },
      /* evRCRm */  { stReqSent,  { acScn,  acNull, acNull } },
      /* evRCA */   { stOpened,   { acIrc,  acTlu,  acNull } },
      /* evRCN */   { stAckSent,  { acIrc,  acScr,  acNull } },
      /* evRTR */   { stReqSent,  { acSta,  acNull, acNull } },
      /* evRTA */   { stAckSent,  { acNull, acNull, acNull } },
      /* evRUC */   { stAckSent,  { acScj,  acNull, acNull } },
      /* evRXJp */  { stAckSent,  { acNull, acNull, acNull } },
      /* evRXJm */  { stStopped,  { acTlf,  acNull, acNull } },
      /* evRXR */   { stAckSent,  { acNull, acNull, acNull } },
    },
    { /* stOpened */
      /* evUp */    { stNoChange, { acNull, acNull, acNull } },
      /* evDown */  { stStarting, { acTld,  acNull, acNull } },
      /* evOpen */  { stOpened,   { acNull, acNull, acNull } },
      /* evClose */ { stClosing,  { acTld,  acIrc,  acStr  } },
      /* evTOp */   { stNoChange, { acNull, acNull, acNull } },
      /* evTOm */   { stNoChange, { acNull, acNull, acNull } },
      /* evRCRp */  { stAckSent,  { acTld,  acScr,  acSca  } },
      /* evRCRm */  { stReqSent,  { acTld,  acScr,  acScn  } },
      /* evRCA */   { stReqSent,  { acTld,  acScr,  acNull } },
      /* evRCN */   { stReqSent,  { acTld,  acScr,  acNull } },
      /* evRTR */   { stStopping, { acTld,  acZrc,  acSta  } },
      /* evRTA */   { stReqSent,  { acTld,  acScr,  acNull } },
      /* evRUC */   { stOpened,   { acScj,  acNull, acNull } },
      /* evRXJp */  { stOpened,   { acNull, acNull, acNull } },
      /* evRXJm */  { stStopping, { acTld,  acIrc,  acStr  } },
      /* evRXR */   { stOpened,   { acSer,  acNull, acNull } },
    }
  };


#if defined(USE_DEBUG)

static const char *ph_str[] = {
                  "Dead", "Establish", "Authenticate",
                  "Network", "Terminate"
                };

static const char *ev_str[] = {
                  "Up", "Down", "Open", "Close", "TO+", "TO-",
                  "RCR+", "RCR-", "RCA", "RCN", "RTR", "RTA",
                  "RUC", "RXJ+", "RXJ-", "RXR"
                };

static const char *st_str[] = {
                  "Initial", "Starting", "Closed", "Stopped",
                  "Closing", "Stopping", "Req-Sent", "Ack-Rcvd",
                  "Ack-Sent", "Opened", "No-Change"
                };

static const char *ac_str[] = {
                  "Null", "irc", "scr", "tls", "tlf", "str", "sta",
                  "sca", "scn", "scj", "tld", "tlu", "zrc", "ser"
                };

/*
 * Display contents of buffer
 */
static void buffer_print (const char *verb, const BYTE *buf, int len)
{
  int i, j;

  (*_printf) ("%s %d bytes:\n", verb, len);

  for (i = 0; i < len; i += 16)
  {
    (*_printf) ("  %04X: ", i);
    for (j = i; j < len && j < i + 16; j++)
        (*_printf) (" %02X", buf[j]);
    (*_printf) ("\n");
  }
}

/*
 * Print text string safely.
 */
static void safe_string (const BYTE *bp, int len)
{
  (*_printf) ("\"");

  while (--len >= 0)
  {
    char buf[2];
    int  chr = *bp++;

    buf[0] = chr;
    buf[1] ='\0';
    if (isascii(chr) && isprint(chr))
         (*_printf) (buf);
    else (*_printf) ("\\%03o", chr);
  }

  (*_printf) ("\"");
}

/*
 * Show all options in negotiation message.
 */
static void option_show (const struct ppp_state *state,
                         const struct ppp_xcp *xcp,
                         const BYTE *bp, int rlen)
{
  while (rlen > 0)
  {
    const struct xcp_type *tp;
    int   i;

    (*_printf) (" ");
    if (rlen < 2)
    {
      (*_printf) ("[trail %02X]\n", bp[0]);
      break;
    }
    if (bp[1] > rlen)
    {
      (*_printf) ("[trunc %02X %d>%d]\n", bp[0], bp[1], rlen);
      break;
    }
    for (tp = xcp->types; tp->type != -1; tp++)
        if (tp->type == bp[0])
          break;

    if (tp->show)
    {
      /* Valid option; use defined printing routine
       */
      (*tp->show) ((struct ppp_state*)state, (struct ppp_xcp*)xcp,
                   tp, bp + 2, bp[1]);
    }
    else
    {
      /* Undefined option; just dump contents.
       */
      if (tp->name == NULL)
           (*_printf) ("%02X:", bp[0]);
      else (*_printf) ("%s:", tp->name);
      for (i = 2; i < bp[1]; i++)
          (*_printf) ("%02X", bp[i]);
    }
    i = bp[1];
    if (i < 2)
        i = 2;
    rlen -= i;
    bp   += i;
  }
}

/*
 * Show a negotiation packet.
 * Assumes 'bp' points to the code number (after PPP Protocol ID).
 */
static void show_packet (const char *inout, const struct ppp_state *state,
                         const struct ppp_xcp *xcp, const BYTE *bp, int len)
{
  static const char *code_str[] = {
                    "Vendor", "Configure-Request", "Configure-Ack",
                    "Configure-Nak", "Configure-Reject", "Terminate-Request",
                    "Terminate-Ack", "Code-Reject", "Protocol-Reject",
                    "Echo-Request", "Echo-Reply", "Discard-Request"
                  };
  int code   = *bp++;
  int id     = *bp++;
  int length = intel16 (*(WORD*)bp);

  bp += 2;
  if (len > length)
      len = length;
  len -= PPP_HDRLEN;

  (*_printf) ("%s %s ", inout, xcp->name);
  if (code < DIM(code_str) && code_str[code])
       (*_printf) ("%s", code_str[code]);
  else (*_printf) ("code:%d", code);

  (*_printf) (" ID:%d", id);
  if ((code == CODE_CONF_ACK || code == CODE_CONF_NAK ||
       code == CODE_CONF_REJ || code == CODE_TERM_ACK ||
       code == CODE_ECHO_REP) && id != xcp->ident)
     (*_printf) (" **ERR**");
  (*_printf) (" len:%d", length);

  switch (code)
  {
    case CODE_CONF_REQ:
    case CODE_CONF_ACK:
    case CODE_CONF_NAK:
    case CODE_CONF_REJ:
         option_show (state, xcp, bp, len);
         break;
    case CODE_TERM_REQ:
    case CODE_TERM_ACK:
         if (len > 0)
         {
           (*_printf) (" ");
           safe_string (bp, len);
         }
         break;
    case CODE_CODE_REJ:
         (*_printf) (" code %d", *bp);
         break;
    case CODE_PROTO_REJ:
         (*_printf) (" protocol %02X%02X", bp[0], bp[1]);
         break;
    case CODE_ECHO_REQ:
    case CODE_ECHO_REP:
    case CODE_DISCARD_REQ:
         if (len >= 4)
            (*_printf) (" magic %02X%02X%02X%02X", bp[0], bp[1], bp[2], bp[3]);
         if (len > 4)
         {
           (*_printf) (" ");
           safe_string (bp+4, len-4);
         }
         break;
  }
  (*_printf) ("\n");
}

#undef  PRINTF
#define PRINTF(lvl,arg)    do {                 \
                             if (debug >= lvl)  \
                                (*_printf) arg; \
                           } while (0)
#else
  #undef  PRINTF
  #define PRINTF(lvl,arg)  ((void)0)
#endif  /* USE_DEBUG */


/*
 * Write packet to PPPoE session. Display contents if debugging.
 */
static void ppp_write (const struct ppp_state *state,
                       const struct ppp_xcp *xcp,
                       const BYTE *buf, int len)
{
  WATT_ASSERT (state);
  WATT_ASSERT (xcp);

#if defined(USE_DEBUG)
  if (debug > 3)
     buffer_print ("Transmitting", buf, len);

  if (debug > 2)
  {
    int retv = buf[0] == 0xFF ? 2 : 0;
    retv += buf[retv] & 1 ? 1 : 2;
    show_packet ("SENT", state, xcp, buf + retv, len - retv);
  }
#endif
  pppoe_send_sess (NULL /*\todo state->sock */, buf, len);

  ARGSUSED (state);
  ARGSUSED (xcp);
}

/*
 * Insert PPP protocol field.
 */
static __inline BYTE *setup_acpf (BYTE *buf, WORD proto)
{
  *(WORD*)buf = intel16 (proto);
  return (buf + 2);
}

/*
 * Set up buffer for a new message (Configure-Request, Protocol-Reject,
 * Echo-Request, or Discard-Request).
 */
static BYTE *code_id (struct ppp_state *state, struct ppp_xcp *xcp, int code)
{
  BYTE *buf = setup_acpf (state->outbuffer, xcp->proto);

  *buf++ = code;
  *buf++ = ++xcp->ident;
  *buf++ = 0;
  *buf++ = 4;
  return (buf);
}

/*
 * Set up buffer for a reply to a previous message (Configure-Ack,
 * Configure-Nak, Configure-Reject, Echo-Reply).
 */
static BYTE *code_reply (struct ppp_state *state, struct ppp_xcp *xcp, int code)
{
  BYTE *buf = setup_acpf (state->outbuffer, xcp->proto);

  *buf++ = code;
  *buf++ = state->inbuffer[3]; /* ident */
  *buf++ = 0;
  *buf++ = 4;
  return (buf);
}

/*
 * Find a given option in the list for an XCP.
 */
static struct option_state *find_option (const struct ppp_xcp *xcp, int type)
{
  const struct xcp_type *tp;

  for (tp = xcp->types; tp->type != -1; tp++)
      if (tp->type == type)
         return (xcp->opts + (tp - xcp->types));
  return (NULL);
}

/*
 * Loop over all known options and insert into a Configure-Request
 * being built.
 */
static void create_request (struct ppp_state *state, struct ppp_xcp *xcp)
{
  struct xcp_type     *tp;
  struct option_state *os;
  BYTE  *bp, *obp;

  obp = bp = code_id (state, xcp, CODE_CONF_REQ);
  for (tp = xcp->types, os = xcp->opts; tp->type != -1; tp++, os++)
  {
    if (os->state == osUnusable)
       continue;

    if (!tp->flag && os->my_value == tp->default_value)
       continue;

    bp[0] = (BYTE) tp->type;
    bp[1] = tp->minlen;
    if (tp->minlen > 2)
       bp[2] = (BYTE) (os->my_value >> (8 * (tp->minlen - 3))) & 0xFF;
    if (tp->minlen > 3)
       bp[3] = (BYTE) (os->my_value >> (8 * (tp->minlen - 4))) & 0xFF;
    if (tp->minlen > 4)
       bp[4] = (BYTE) (os->my_value >> (8 * (tp->minlen - 5))) & 0xFF;
    if (tp->minlen > 5)
       bp[5] = (BYTE) os->my_value;
    bp += bp[1];
  }
  *(WORD*) (obp-2) = intel16 (bp-obp+4);
  state->up = bp;
}

/*
 * We've gotten a Configure-Request we like (RCR+), so we're agreeing
 * with the peer.  Set up to send Configure-Ack.
 */
static void copy_peer_values (const struct ppp_state *state, struct ppp_xcp *xcp)
{
  struct option_state *os;
  const struct xcp_type *tp;
  const BYTE  *bp;
  int   rlen;
  DWORD val;

  for (tp = xcp->types, os = xcp->opts; tp->type != -1; tp++, os++)
      os->peer_value = tp->default_value;
  bp   = state->bp;
  rlen = state->mlen;
  while (rlen > 0)
  {
    for (tp = xcp->types; tp->type != -1; tp++)
        if (tp->type == bp[0])
           break;

    os  = xcp->opts + (tp - xcp->types);
    val = bp[2];
    if (bp[1] > 3)
       val = (val << 8) + bp[3];
    if (bp[1] > 4)
       val = (val << 8) + bp[4];
    if (bp[1] > 5)
       val = (val << 8) + bp[5];
    os->peer_value = val;
    rlen -= bp[1];
    bp   += bp[1];
  }
}

/*
 * Perform RFC 1661 actions as indicated by state machine.
 */
static void dispatch_action (struct ppp_state *state, struct ppp_xcp *xcp,
                             enum xcp_action act)
{
  BYTE *bp;

  if (act == acNull)
     return;

  PRINTF (2, ("%s action %s (%d)\n", xcp->name, ac_str[act], act));

  switch (act)
  {
    case acNull:
         break;

    case acIrc:
         xcp->restart = state->phase == phTerminate ? MAX_TERMINATE : MAX_CONFIGURE;
         state->timeout_period = INITIAL_TIMEOUT;
         break;

    case acScr:
         if (xcp->restart > 0)
         {
           xcp->restart--;
           create_request (state, xcp);
           ppp_write (state, xcp, state->outbuffer, state->up - state->outbuffer);
         }
         xcp->timeout = time (NULL) + state->timeout_period;
         break;

    case acTls:
         if (xcp == &state->xcps[XCP_IPCP])
              send_event (state, &state->xcps[XCP_LCP], evOpen);
         else change_phase (state, phEstablish);
         break;

    case acTlf:
         if (xcp == &state->xcps[XCP_IPCP])
              send_event (state, &state->xcps[XCP_LCP], evClose);
         else change_phase (state, phTerminate);
         break;

    case acStr:
         if (xcp->restart > 0)
         {
           xcp->restart--;
           bp = code_id (state, xcp, CODE_TERM_REQ);
           ppp_write (state, xcp, state->outbuffer, bp - state->outbuffer);
         }
         xcp->timeout = time (NULL) + state->timeout_period;
         break;

    case acSta:
         bp = code_reply (state, xcp, CODE_TERM_ACK);
         ppp_write (state, xcp, state->outbuffer, bp - state->outbuffer);
         break;

    case acSca:
         copy_peer_values (state, xcp);
         state->inbuffer[2] = CODE_CONF_ACK;
         ppp_write (state, xcp, state->inbuffer, state->mlen +
                    (state->bp - state->inbuffer));
         break;

    case acScn:
         if (xcp->naks_sent++ >= MAX_FAILURE)
            state->outbuffer[4] = CODE_CONF_REJ;
         /* fall-through */

    case acScj:
         ppp_write (state, xcp, state->outbuffer, state->up - state->outbuffer);
         break;

    case acTld:
         if (xcp == &state->xcps[XCP_LCP])
              change_phase (state, phTerminate);
         else PRINTF (0, ("IPCP down!\n"));
         break;

    case acTlu:
         if (xcp == &state->xcps[XCP_LCP])
            change_phase (state, phAuthenticate);
         else
         {
           struct option_state *os = find_option (xcp, 3);
           DWORD  addr = intel (os->my_value);

           my_ip_addr = addr;
           PRINTF (0, ("IPCP up!  Local address %s, ",
                   _inet_ntoa(NULL,addr)));
           addr = intel (os->peer_value);
           _arp_kill_gateways();
           _arp_add_gateway (NULL, addr);
           PRINTF (0, ("remote %s\n", _inet_ntoa(NULL,addr)));
         }
         xcp->restart = MAX_CONFIGURE;
         xcp->timeout = 0;
         break;

    case acZrc:
         xcp->restart = 0;
         state->timeout_period = INITIAL_TIMEOUT;
         break;

    case acSer:
         state->inbuffer[2] = CODE_ECHO_REP;
         *(long*) (state->inbuffer + 6) = intel (state->mymagic);
         ppp_write (state, xcp, state->inbuffer, state->mlen +
                    (state->bp - state->inbuffer));
         break;
  }
}

/*
 * Issue event into XCP state machine -- go to next state and
 * invoke associated actions.
 */
static void send_event (struct ppp_state *state, struct ppp_xcp *xcp,
                        enum ppp_event event)
{
  const struct ppp_dispatch *dp = &ppp_dispatch [xcp->state][event];

  if (dp->next == stNoChange)
  {
    PRINTF (2, ("%s got illegal %s event (%d) in %s state (%d)\n"
                "\t(RFC 1661 section 4.4)\n",
                xcp->name, ev_str[event], event, st_str[xcp->state],
                xcp->state));
  }
  else
  {
    PRINTF (2, ("%s got %s event (%d) in %s state (%d), next is %s state (%d)\n",
                xcp->name, ev_str[event], event, st_str[xcp->state],
                xcp->state, st_str[dp->next], dp->next));
    xcp->state = dp->next;
  }
  dispatch_action (state, xcp, dp->act[0]);
  dispatch_action (state, xcp, dp->act[1]);
  dispatch_action (state, xcp, dp->act[2]);
}

/*
 * Change overall link phase.  Central routine helps with
 * debugging.
 */
static void change_phase (struct ppp_state *state, enum ppp_phase phase)
{
  PRINTF (2, ("Current PPP phase is %s, switching to %s\n",
              ph_str[state->phase], ph_str[phase]));

  switch (phase)
  {
    case phEstablish:
         if (state->phase == phDead)
            send_event (state, &state->xcps[XCP_LCP], evUp);
         break;
    case phAuthenticate:
         break;
    case phNetwork:
         if (state->phase == phAuthenticate)
            send_event (state, &state->xcps[XCP_IPCP], evUp);
         break;
    case phTerminate:
         send_event (state, &state->xcps[XCP_IPCP], evDown);
         break;
    case phDead:
         break;
  }
  state->phase = phase;
  if (phase == phAuthenticate)
     change_phase (state, phNetwork); /* XXX - no auth yet */
}

/*
 * Check for time-outs
 */
static void check_timeouts (void)
{
  time_t now;
  int    i;

  if (mystate.timeout_period < MAX_TIMEOUT)
      mystate.timeout_period <<= 1;
  now = time (NULL);

  for (i = 0; i < DIM(mystate.xcps); i++)
  {
    if (mystate.xcps[i].timeout && now >= mystate.xcps[i].timeout)
    {
      mystate.xcps[i].timeout = 0;

      /* Hack -- send with same ID on timeout.
       */
      if (mystate.xcps[i].restart > 0)
          mystate.xcps[i].ident--;

      PRINTF (4, ("Sending TO%c to %s\n",
                  mystate.xcps[i].restart > 0 ? '+' : '-',
                  mystate.xcps[i].name));

      send_event (&mystate, mystate.xcps+i,
                  mystate.xcps[i].restart > 0 ? evTOp : evTOm);
    }
  }
}

/**
 * The PPP input handler.
 */
void ppp_input (const BYTE *inbuffer, WORD len)
{
  struct ppp_state *state = &mystate;
  int    i;
  WORD   proto;

  state->inbuffer = (BYTE*) inbuffer;

#if defined(USE_DEBUG)
  if (debug > 3)
     buffer_print ("Received", state->inbuffer, len);
#endif

#if 0   /* no proto-compression used in PPPoE */
  if (state->inbuffer[0] & 0x1)
  {
    proto = state->inbuffer[0];
    state->inbuffer++;
    len--;
  }
  else if (state->inbuffer[1] & 0x1)
  {
    proto = intel16 (*(WORD*)state->inbuffer);
    state->inbuffer += 2;
    len -=2;
  }
  else
  {
    proto = intel16 (*(WORD*)state->inbuffer);
    PRINTF (0, ("Bad protocol field %04X\n", proto));
    return;
  }
#else
  proto = intel16 (*(WORD*)state->inbuffer);
  state->inbuffer += 2;
  len -= 2;
#endif

  if (proto == PPP_IP)
  {
    _ip4_handler ((const in_Header*)state->inbuffer, FALSE);
    return;
  }

  /* Find XCP (IPCP/LCP) for indicated protocol
   */
  for (i = 0; i < DIM(state->xcps); i++)
      if (state->xcps[i].proto == proto)
         break;

  if (i >= DIM(state->xcps))
  {
    /* Generate LCP Protocol-Reject for unknown things.
     */
    PRINTF (0, ("Unknown protocol 0x%04X\n", proto));

    if (state->xcps[XCP_LCP].state == stOpened)
    {
      BYTE *bp = code_id (state, &state->xcps[XCP_LCP], CODE_PROTO_REJ);

      i = sizeof(state->outbuffer) - (bp - state->outbuffer);
      if (len > i)
          len = i;
      memcpy (bp, state->inbuffer-2, len);
      len += bp - state->outbuffer;
      *(WORD*) (state->outbuffer+2) = intel16 (len-4);
      ppp_write (state, &state->xcps[XCP_LCP], state->outbuffer, len);
    }
    return;
  }

  state->bp   = state->inbuffer;
  state->mlen = len;
  (*state->xcps[i].deliver) (state, state->xcps + i);
}

/*
 * Add current option to list of options to be sent in Configure-Reject.
 */
static void set_reject (struct ppp_state *state, struct ppp_xcp *xcp,
                        const BYTE *bp)
{
  int i;

  if (state->parse_state == psBad)
     return;

  /* If this is the first reject for this packet, then set up.
   */
  if (state->parse_state != psRej)
  {
    state->up = code_reply (state, xcp, CODE_CONF_REJ);
    state->parse_state = psRej;
  }

  /* Handle malformed options; make sure we don't send anything illegal
   * to the peer (even if he's sending us junk).
   */
  i = bp[1];
  if (i < 2)
      i = 2;
  memcpy (state->up, bp, i);
  state->up[1] = i;
  state->up += i;
}

/*
 * Add current option to list of options to be sent in Configure-Nak.
 */
static void set_nak (struct ppp_state *state, struct ppp_xcp *xcp, int type,
                     int len, const BYTE *bp)
{
  /* If we're rejecting, then no point in doing naks.
   */
  if (state->parse_state == psBad || state->parse_state == psRej)
     return;

  /* If this is the first nak for this packet, then set up.
   */
  if (state->parse_state != psNak)
  {
    state->up = code_reply (state, xcp, CODE_CONF_NAK);
    state->parse_state = psNak;
  }
  *state->up++ = type;
  *state->up++ = len;
  while (--len > 1)
    *state->up++ = *bp++;
}

/*
 * Check Configure-Request options from peer against list of
 * known, valid, and desired options.
 */
static int validate_request (struct ppp_state *state, struct ppp_xcp *xcp)
{
  const struct xcp_type *tp;
  const BYTE  *bp;
  int   rlen, i;

  state->parse_state = psOK;
  rlen = state->mlen;
  bp = state->bp;

  while (rlen > 0)
  {
    if (rlen < 2 || bp[1] > rlen)
    {
      state->parse_state = psBad;
      break;
    }
    for (tp = xcp->types; tp->type != -1; tp++)
        if (tp->type == bp[0])
           break;

    if (tp->type == -1)
       set_reject (state, xcp, bp);
    else if (bp[1] < tp->minlen)
       set_nak (state, xcp, tp->type, tp->minlen, bp + 2);
    else if (bp[1] > tp->maxlen)
       set_nak (state, xcp, tp->type, tp->maxlen, bp + 2);
    else if (tp->validate_req)
       (*tp->validate_req) (state, xcp, tp, bp + 2, bp[1]);

    i = bp[1];
    if (i < 2)
        i = 2;
    rlen -= i;
    bp  += i;
  }
  if (state->parse_state == psNak || state->parse_state == psRej)
  {
    i = (state->up - state->outbuffer) - 4;
    state->outbuffer[6] = (i >> 8) & 0xFF;
    state->outbuffer[7] = i & 0xFF;
  }
  return (state->parse_state == psOK);
}

/*
 * Process options in a Configure-{Ack,Nak,Reject}.
 */
static void process_rcx (struct ppp_state *state, struct ppp_xcp *xcp, BYTE code)
{
  struct option_state *os;
  const struct xcp_type *tp;
  int    rlen, i, val;
  BYTE  *bp;

  rlen = state->mlen;
  bp   = state->bp;

  while (rlen > 0)
  {
    if (rlen < 2 || bp[1] > rlen)
       break;

    for (tp = xcp->types; tp->type != -1; tp++)
        if (tp->type == bp[0])
           break;

    if (tp->type != -1)
    {
      os = xcp->opts + (tp - xcp->types);
      if (code == CODE_CONF_REJ)
         os->state = osUnusable;
      else if (code == CODE_CONF_ACK || tp->validate_nak == NULL)
      {
        if (bp[1] > 6 || bp[1] <= 2)
           os->my_value = tp->default_value;
        else
        {
          val = bp[2];
          if (bp[1] > 3)
             val = (val << 8) + bp[3];
          if (bp[1] > 4)
             val = (val << 8) + bp[4];
          if (bp[1] > 5)
             val = (val << 8) + bp[5];
          os->my_value = val;
        }
      }
      else
        (*tp->validate_nak) (state, xcp, tp, bp + 2, bp[1]);
    }
    if ((i = bp[1]) < 2)
       i = 2;
    rlen -= i;
    bp += i;
  }
}

/*
 * Standard negotiation entry point.  This is assigned as the
 * 'delivery' function for control protocols, like LCP and IPCP.
 */
static void std_negotiation (struct ppp_state *state, struct ppp_xcp *xcp)
{
  BYTE code, id;
  WORD proto;
  int  i, length;

  /* Validate overall message length
   */
  if (state->mlen < 4)
  {
    PRINTF (0, ("Malformed negotiation message; %d < 4\n", state->mlen));
    return;
  }

  code   = *state->bp++;
  id     = *state->bp++;
  length = intel16 (*(WORD*)state->bp);  /* !! */
  if (length > state->mlen)
  {
    PRINTF (0, ("Truncated negotiation message; %d > %d\n",
                length, state->mlen));
    return;
  }
  if (length < state->mlen)
  {
    PRINTF (0, ("Trimmed negotiation message; %d < %d\n",
                length, state->mlen));
    state->mlen = length;
  }
  state->bp   += 2;
  state->mlen -= 4;

#if defined(USE_DEBUG)
  if (debug > 2)
     show_packet ("RCVD", state, xcp, state->bp-4, state->mlen+4);
#endif

  /* Now switch out on received code number
   */
  switch (code)
  {
    case CODE_CONF_REQ:
         /* If request is good, then RCR+, if bad, then RCR-
          */
         if (validate_request (state, xcp))
              send_event (state, xcp, evRCRp);
         else send_event (state, xcp, evRCRm);
         break;

    case CODE_CONF_ACK:
         /* Configure-Ack ID number must match last sent.
          */
         if (id == xcp->ident)
         {
           /* Should validate contents against last req sent, but we don't.
            */
           process_rcx (state, xcp, code);
           send_event (state, xcp, evRCA);
         }
         /** \todo extract LCP_MRU option */
         break;

    case CODE_CONF_NAK:
    case CODE_CONF_REJ:
         /* Configure-Nak/Reject ID number must match last sent.
          */
         if (id == xcp->ident)
         {
           process_rcx (state, xcp, code);
           send_event (state, xcp, evRCN);
         }
         break;

    case CODE_TERM_REQ:
         send_event (state, xcp, evRTR);
         break;

    case CODE_TERM_ACK:
         if (id == xcp->ident)
            send_event (state, xcp, evRTA);
         break;

    case CODE_CODE_REJ:
         code = *state->bp++;

         /* Peer cannot reject well-known code numbers.
          */
         if (code && code < CODE_ECHO_REQ)
         {
           PRINTF (0, ("Invalid code reject for %d\n", code));
           send_event (state, xcp, evRXJm);
         }
         else
           send_event (state, xcp, evRXJp);
         break;

    case CODE_PROTO_REJ:
         proto = intel16 (*(WORD*)state->bp);  /* !! */

         /* Peer cannot reject LCP!
          */
         if (proto == state->xcps[XCP_LCP].proto)
         {
           PRINTF (0, ("Peer protocol-rejected LCP itself!\n"));
           send_event (state, &state->xcps[XCP_LCP], evRXJm);
         }
         else
         {
           send_event (state, xcp, evRXJp);
           for (i = 0; i < DIM(state->xcps); i++)
               if (state->xcps[i].proto == proto)
                  state->xcps[i].state = stInitial;
         }
         break;

    case CODE_ECHO_REQ:
         /* send a reply here. */
         send_event (state, xcp, evRXR);
         break;

    case CODE_DISCARD_REQ:
         send_event (state, xcp, evRXR);
         break;

    case CODE_ECHO_REP:
         /* ID number in Echo-Reply must match last echo sent.
          */
         if (id == xcp->ident)
            send_event (state, xcp, evRXR);
         break;

    default:
         state->bp -= 4;
         send_event (state, xcp, evRUC);
         break;
  }
}

/*
 * Initialize an XCP (LCP or NCP).
 */
static void init_xcp (const char *name, struct ppp_xcp *xcp,
                      void (*deliver) (struct ppp_state *state,
                                       struct ppp_xcp *xcp),
                      const struct xcp_type *types, int ntypes, WORD proto)
{
  struct option_state *os;

  xcp->name    = name;
  xcp->state   = stInitial;
  xcp->restart = 0;
  xcp->timeout = 0;
  xcp->deliver = deliver;
  xcp->types   = (struct xcp_type*)types;
  xcp->proto   = proto;

  /* If no options, then no vector to store negotiated values.
   */
  if (ntypes <= 0)
  {
    xcp->opts = NULL;
    return;
  }
  /* Otherwise, allocate vector and initialize options
   */
  os = xcp->opts = (struct option_state*) calloc (ntypes, sizeof(*os));

  while (--ntypes >= 0)
  {
    os->my_value = os->peer_value = types->default_value;
    os->state    = osUsable;
    os++;
    types++;
  }
}

/*
 * Handle Configure-Request from peer.  If it's in the range we think
 * it should be, then do nothing (allow Configure-Ack).  Otherwise,
 * send Configure-Nak with something more appropriate.
 */
static void ipcp_addr_req (struct ppp_state *state,
                           struct ppp_xcp *xcp,
                           const struct xcp_type *tp,
                           const BYTE *buf, int len)
{
  struct option_state *os;
  BYTE   lbuf[4];
  DWORD  addr = intel (*(DWORD*)buf);

  os = xcp->opts + (tp - xcp->types);
  if (addr != os->my_value && (addr & remote_ip_mask) == remote_ip_base)
     return;

  addr = xcp->opts [tp-xcp->types].peer_value;
  *(DWORD*) &lbuf = intel (addr);
  set_nak (state, xcp, tp->type, tp->minlen, lbuf);
  ARGSUSED (len);
}

/*
 * Got a Configure-Nak of our address.  Just change; we're flexible.
 */
static void ipcp_addr_nak (struct ppp_state *state,
                           struct ppp_xcp *xcp,
                           const struct xcp_type *tp,
                           const BYTE *buf, int len)
{
  xcp->opts [tp-xcp->types].my_value = intel (*(DWORD*)buf);
  ARGSUSED (state);
  ARGSUSED (len);
}

/*
 * Show IP address.
 */
static void ipcp_addr_show (struct ppp_state *state,
                            struct ppp_xcp *xcp, const struct xcp_type *tp,
                            const BYTE *buf, int len)
{
  PRINTF (0, ("%s", _inet_ntoa(NULL, intel(*(DWORD*)buf))));
  ARGSUSED (state);
  ARGSUSED (xcp);
  ARGSUSED (tp);
  ARGSUSED (buf);
  ARGSUSED (len);
}

/*
 * Set local value (for Configure-Request).
 */
static void set_option_value_lcl (struct ppp_xcp *xcp, int type, int value)
{
  struct option_state *os = find_option (xcp, type);

  if (os)
     os->my_value = value;
}

/*
 * Set desired value for peer (we'll Configure-Nak with this).
 */
static void set_option_value_rem (struct ppp_xcp *xcp, int type, int value)
{
  struct option_state *os = find_option (xcp, type);

  if (os)
     os->peer_value = value;
}

void ppp_start (int dbg_level)
{
  struct ppp_state *state = &mystate;

  debug = dbg_level;

  /* Initialize PPP state machine and add LCP and IPCP.
   */
  state->phase          = phDead;
  state->timeout_period = INITIAL_TIMEOUT;
  state->mymagic        = Random (0, UINT_MAX);

  init_xcp ("LCP", &state->xcps[XCP_LCP], std_negotiation,
            lcp_types, DIM(lcp_types) - 1, PPP_LCP);

  init_xcp ("IPCP", &state->xcps[XCP_IPCP], std_negotiation,
            ipcp_types, DIM(ipcp_types) - 1, PPP_IPCP);

  set_option_value_lcl (&state->xcps[XCP_IPCP], 3, 0x0A000000 + (rand() % 1000));
  set_option_value_rem (&state->xcps[XCP_IPCP], 3, 0x0A00000A + (rand() % 1000));

  /* Tell LCP to open
   */
  send_event (state, &state->xcps[XCP_IPCP], evOpen);

  /* Add handler for checking timeouts (via tcp_tick)
   */
  DAEMON_ADD (check_timeouts);
}

#endif /* USE_PPPOE */

