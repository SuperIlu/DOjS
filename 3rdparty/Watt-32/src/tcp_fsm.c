/*!\file tcp_fsm.c
 *
 * State machine for TCP processing.
 * Previously this was in pctcp.c and tcp_handler().
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "copyrigh.h"
#include "wattcp.h"
#include "chksum.h"
#include "misc.h"
#include "misc_str.h"
#include "timer.h"
#include "sock_ini.h"
#include "language.h"
#include "pcconfig.h"
#include "pcqueue.h"
#include "pcsed.h"
#include "pcpkt.h"
#include "ip4_out.h"
#include "ip6_out.h"
#include "ip6_in.h"
#include "split.h"
#include "pcdbug.h"
#include "pcstat.h"
#include "pctcp.h"

#if !defined(USE_UDP_ONLY)

#define flag_SYN_ACK  (tcp_FlagSYN | tcp_FlagACK)
#define flag_FIN_ACK  (tcp_FlagFIN | tcp_FlagACK)

typedef int (*tcp_StateProc)  (_tcp_Socket**,    /* in/out: TCP socket, can change on output */
                               const in_Header*, /* in: IP header */
                               tcp_Header*,      /* in: TCP header */
                               int);             /* in: TCP flags */

static int  tcp_listen_state  (_tcp_Socket**, const in_Header*, tcp_Header*, int);
static int  tcp_resolve_state (_tcp_Socket**, const in_Header*, tcp_Header*, int);
static int  tcp_synsent_state (_tcp_Socket**, const in_Header*, tcp_Header*, int);
static int  tcp_synrec_state  (_tcp_Socket**, const in_Header*, tcp_Header*, int);
static int  tcp_estab_state   (_tcp_Socket**, const in_Header*, tcp_Header*, int);
static int  tcp_estcl_state   (_tcp_Socket**, const in_Header*, tcp_Header*, int);
static int  tcp_finwt1_state  (_tcp_Socket**, const in_Header*, tcp_Header*, int);
static int  tcp_finwt2_state  (_tcp_Socket**, const in_Header*, tcp_Header*, int);
static int  tcp_closewt_state (_tcp_Socket**, const in_Header*, tcp_Header*, int);
static int  tcp_closing_state (_tcp_Socket**, const in_Header*, tcp_Header*, int);
static int  tcp_lastack_state (_tcp_Socket**, const in_Header*, tcp_Header*, int);
static int  tcp_timewt_state  (_tcp_Socket**, const in_Header*, tcp_Header*, int);

static int  tcp_process_data (_tcp_Socket *s, const tcp_Header *tcp, int len, int *flags);
static void tcp_set_window   (_tcp_Socket *s, const tcp_Header *tcp);
static int  tcp_process_ACK  (_tcp_Socket *s, long *unack);

static tcp_StateProc tcp_state_tab [] = {
       tcp_listen_state,   /* tcp_StateLISTEN  : listening for connection */
       tcp_resolve_state,  /* tcp_StateRESOLVE : resolving IP, waiting on ARP reply */
       tcp_synsent_state,  /* tcp_StateSYNSENT : SYN sent, active open */
       tcp_synrec_state,   /* tcp_StateSYNREC  : SYN received, SYN+ACK sent (listen TCB) */
       tcp_estab_state,    /* tcp_StateESTAB   : established */
       tcp_estcl_state,    /* tcp_StateESTCL   : established, but will FIN */
       tcp_finwt1_state,   /* tcp_StateFINWT1  : sent FIN */
       tcp_finwt2_state,   /* tcp_StateFINWT2  : sent FIN, received FIN+ACK */
       tcp_closewt_state,  /* tcp_StateCLOSWT  : received FIN waiting for close */
       tcp_closing_state,  /* tcp_StateCLOSING : sent FIN, received FIN (waiting for FIN+ACK) */
       tcp_lastack_state,  /* tcp_StateLASTACK : FIN received, ACK+FIN sent */
       tcp_timewt_state,   /* tcp_StateTIMEWT  : dally after sending final FIN+ACK */
     };

static BOOL  is_ip4;         /* TRUE: input packet is IPv4, else IPv6 */
static DWORD acknum, seqnum; /* ACK/SEQ of current segment */

#if defined(USE_NEW_TCP_REASM)  /* not yet */

#define MAX_REASM_SEG 10

static int tcp_reassemble (_tcp_Socket *s, const tcp_Header *tcp,
                           UINT len, int flags);
struct tcp_reasm {
       struct {
         DWORD left;
         DWORD right;
       } edge [MAX_REASM_SEG];
       int idx;
     };
#endif


/*
 * _tcp_fsm - Our TCP-input state-machine.
 *
 *   Returns 1 to tcp_handler() if a retransmission is required
 *   immediately or when RTO expires.
 */
int _tcp_fsm (_tcp_Socket **sp, const in_Header *ip)
{
  tcp_Header  *tcp;
  _tcp_Socket *s = *sp;
  BYTE         flags;
  int          rc;
  UINT         in_state = s->state;

  if (s->state >= tcp_StateCLOSED)
     return (0);

#if defined(NEED_SPLIT)
  tcp    = (tcp_Header*) pkt_get_type_in (TYPE_TCP_HEAD)->data;
  is_ip4 = (ip->ver == 4);
#else
  tcp     = (tcp_Header*) ((BYTE*)ip + in_GetHdrLen (ip));
  is_ip4 = TRUE;  /* assume IPv4 */
#endif

  acknum = intel (tcp->acknum);
  seqnum = intel (tcp->seqnum);

  flags = tcp->flags & tcp_FlagMASK;
  rc = (*tcp_state_tab[s->state]) (sp, ip, tcp, flags);
  s  = *sp;

#if defined(USE_DEBUG)
  if (debug_on >= 3 && in_state != s->state)
  {
    const char *st_in  = tcpStateName (in_state);
    const char *st_out = tcpStateName (s->state);

    TRACE_CONSOLE (3, "tcp-state: %s -> %s\n", st_in, st_out);
  }
#endif

#if defined(USE_BSD_API)
  if (_bsd_socket_hook)
    (*_bsd_socket_hook) (BSO_DEBUG, s, in_state, s->state);
#endif

  ARGSUSED (in_state);
  return (rc);
}

/*
 * LISTEN state
 */
static int tcp_listen_state (_tcp_Socket **sp, const in_Header *ip,
                             tcp_Header *tcp, int flags)
{
  _tcp_Socket *s = *sp;

  SET_PEER_MAC_ADDR (s, ip);  /* save his ethernet address */

  if ((flags & tcp_FlagSYN) == tcp_FlagSYN)
  {
#if defined(USE_BSD_API)
    /*
     * Append the TCB `s' to the listen-queue. The new TCB on output
     * from `_bsd_socket_hook' is the clone of `s' on input unless the
     * listen-queue is full.
     */
    if (_bsd_socket_hook && !(*_bsd_socket_hook)(BSO_SYN_CALLBACK,&s))
    {
      /* Append failed due to queue full or (temporary) memory shortage.
       * Silently discard SYN. TCB `s' is unchanged.
       */
      CLR_PEER_MAC_ADDR (s);
      return (0);
    }
#endif

    if (is_ip4 && ip->tos > s->tos)
       s->tos = ip->tos;

    s->recv_next = seqnum + 1;
    s->flags     = flag_SYN_ACK;
    s->state     = tcp_StateSYNREC;
    s->unhappy   = TRUE;
    TCP_SEND (s);            /* respond immediately */

    s->timeout = set_timeout (tcp_TIMEOUT);
    STAT (tcpstats.tcps_accepts++);
  }
  else
  {
    TCP_SEND_RESET (s, ip, tcp);
    STAT (tcpstats.tcps_conndrops++);
    CLR_PEER_MAC_ADDR (s);
  }
  return (0);
}

/*
 * RESOLVE state, GvB 2002-09 'inserted'
 * Nothing done here, the action is in pctcp.c / tcp_Retransmitter()
 */
static int tcp_resolve_state (_tcp_Socket **sp, const in_Header *ip,
                              tcp_Header *tcp, int flags)
{
  ARGSUSED (sp);
  ARGSUSED (ip);
  ARGSUSED (tcp);
  ARGSUSED (flags);
  return (1);
}

/*
 * SYNSENT state
 */
static int tcp_synsent_state (_tcp_Socket **sp, const in_Header *ip,
                              tcp_Header *tcp, int flags)
{
  _tcp_Socket *s = *sp;

  if (flags & tcp_FlagSYN)
  {
    if (is_ip4 && ip->tos > s->tos)
       s->tos = ip->tos;

    s->flags   = tcp_FlagACK;
    s->timeout = set_timeout (tcp_TIMEOUT);

    /* SYN+ACK means connection established, else SYNREC
     */
    if (flags & tcp_FlagACK)
    {
      /* Check the ACK value
       */
      if (acknum == s->send_next + 1)
      {
        int   len;
        const in6_Header *ip6 = (const in6_Header*) ip;

        if (is_ip4)
             len = intel16 (ip->length) - in_GetHdrLen (ip);
        else len = intel16 (ip6->len);

        s->state = tcp_StateESTAB;
        s->send_next++;
        s->recv_next = seqnum + 1;

        /* Should be no data in SYN+ACK, but..
         */
        tcp_process_data (s, tcp, len, &flags);

        /* Prevent retrans on no tx-data
         */
        s->unhappy = s->tx_datalen > 0 ? TRUE : FALSE;

       /* !! Maybe use TCP_SENDSOON() to merge application data into ACK
        */
        TCP_SEND (s);
      }
      else
      {
        /* Wrong ACK, force a RST and resend SYN now
         */
        s->flags   = tcp_FlagRST;
        s->unhappy = TRUE;
        TCP_SEND (s);

        s->send_next = INIT_SEQ();  /* !! should we set a new seq-num? */
        s->flags     = tcp_FlagSYN;

#if defined(USE_DEBUG)
        s->last_seqnum[0] = 0UL;
        s->last_seqnum[1] = 0UL;
#endif
        TCP_SENDSOON (s);
      }

      /** \todo recalculate RTT-timer
       */
    }
    else
    {
      s->recv_next++;
      s->state = tcp_StateSYNREC;  /* resend SYN in tcp_Retransmitter() */
      return (1);
    }
  }
  else   /* didn't get a SYN back. Reset it */
  {
    TCP_SEND_RESET (s, ip, tcp);
    return (0);
  }
  return (1);
}

/*
 * SYNREC state (intermediate state for LISTEN to ESTABLISHED state).
 */
static int tcp_synrec_state (_tcp_Socket **sp, const in_Header *ip,
                             tcp_Header *tcp, int flags)
{
  _tcp_Socket *s = *sp;

  if (flags & tcp_FlagSYN)  /* retransmitted SYN */
  {
    s->flags   = flag_SYN_ACK;
    s->unhappy = TRUE;
    TCP_SEND (s);
    s->timeout = set_timeout (tcp_TIMEOUT);
    return (0);
  }

  if ((flags & tcp_FlagACK) && acknum == (s->send_next + 1))
  {
    tcp_set_window (s, tcp);      /* Allocate Tx-buffer based on peer's window */
    STAT (tcpstats.tcps_connects++);
    s->send_next++;
    s->flags   = tcp_FlagACK;
    s->state   = tcp_StateESTAB;
    s->timeout = 0UL;            /* never timeout now */
    s->unhappy = FALSE;
    return (0);
  }
  ARGSUSED (ip);
  return (1);
}

/*
 * ESTABLISHED state
 */
static int tcp_estab_state (_tcp_Socket **sp, const in_Header *ip,
                            tcp_Header *tcp, int flags)
{
  const in6_Header *ip6 = (const in6_Header*) ip;
  _tcp_Socket *s   = *sp;
  int   len;
  long  ldiff;      /* how much still ACK'ed */
  BOOL  did_tx;

  /* handle lost SYN
   */
  if ((flags & tcp_FlagSYN) && (flags & tcp_FlagACK))
  {
    /* !! should check if data and process it */
    TCP_SEND (s);
    return (0);
  }

  if (!(flags & tcp_FlagACK))   /* must ACK something */
     return (0);

  s->timeout = 0UL;             /* we do not timeout at this point */

  if (!tcp_process_ACK(s,&ldiff))
  {
    TRACE_CONSOLE (2, "_tcp_fsm() confused so set unacked "
                      "back to 0 from %ld\n", s->send_una);
    STAT (tcpstats.tcps_persistdrop++); /* !! a better counter? */
    s->send_una = 0;
  }

  if (s->send_una < 0)
      s->send_una = 0;

  s->flags = tcp_FlagACK;

  if (is_ip4)
       len = intel16 (ip->length) - in_GetHdrLen (ip);
  else len = intel16 (ip6->len);

  if (tcp_process_data (s, tcp, len, &flags) < 0)
  {
    TCP_SEND (s);  /* An out-of-order or missing segment; do fast ACK */
    return (1);
  }

  did_tx = FALSE;

  if (s->state != tcp_StateCLOSWT  &&
      (flags & tcp_FlagFIN)        &&
      SEQ_GEQ(s->recv_next,seqnum) &&
      SEQ_LT(s->recv_next,seqnum+s->adv_win))
  {
    if (s->missed_seq[0] == s->missed_seq[1])
    {
      s->recv_next++;
      SET_ERR_MSG (s, _LANG("Connection closed by peer"));

      /* Implied CLOSE-WAIT -> LAST-ACK transition here
       */
      TCP_SEND (s);
      did_tx = TRUE;

      TRACE_CONSOLE (2, "tcp_estab_state(): got FIN\n");

      s->locflags |= LF_GOT_FIN;
      s->flags    |= tcp_FlagFIN;    /* for tcp_Retransmitter() */
      s->unhappy   = TRUE;
      s->timeout   = set_timeout (tcp_LASTACK_TIME); /* Added AGW 6 Jan 2001 */
      s->state     = tcp_StateLASTACK;
    }
    else
    {
      s->unhappy = TRUE;
      TCP_SEND (s);  /* force a retransmit, no state change */
      did_tx = TRUE;
    }
  }

  /*
   * Eliminate the spurious ACK messages bug.
   * For the window update, the length should be the
   * data length only, so exclude the TCP header size
   *  -- Joe <jdhagen@itis.com>
   */
  len -= (tcp->offset << 2);
  if ((ldiff > 0 && s->tx_datalen > 0) || len > 0)
  {
    /* Need to ACK and update window, but how urgent ??
     * We need a better criteria for doing Fast-ACK.
     */
    if (ldiff > 0 || s->adv_win < s->max_seg)
    {
      TRACE_FILE ("tcp_estab_state (%u): FastACK: ldiff %ld, "
                  "UNA %ld, MS-right %ld\n",
                  __LINE__, ldiff, s->send_una,
                  s->missed_seq[0] != s->missed_seq[1] ?
                  (u_long)(s->missed_seq[0] - s->recv_next) : 0);
      s->karn_count = 0;
      s->flags |= tcp_FlagPUSH;
      TCP_SEND (s);
      did_tx = TRUE;

      if (s->adv_win == 0)  /* need to open closed window in retransmitter */
      {
        s->locflags |= LF_WINUPDATE;
        s->unhappy = TRUE;
      }
    }
    else
    {
      TCP_SENDSOON (s);          /* delayed ACK */
      did_tx = TRUE;
    }
  }

  /* Check if we need to reply to keep-alive segment
   */
  if (!did_tx && (len == 0) && (seqnum == s->recv_next-1) &&
      ((flags & tcp_FlagACK) == tcp_FlagACK) &&  /* ACK only */
      (s->state == tcp_StateESTAB))
  {
    s->locflags |= LF_KEEPALIVE;
    TRACE_FILE ("tcp_process_ACK(): Got keepalive ACK\n");
    TCP_SEND (s);
  }
  return (0);
}

/*
 * ESTAB_CLOSE state
 */
static int tcp_estcl_state (_tcp_Socket **sp, const in_Header *ip,
                            tcp_Header *tcp, int flags)
{
  tcp_estab_state (sp, ip, tcp, flags);
  _tcp_close (*sp);
  return (0);
}

/*
 * CLOSE_WAIT state
 */
static int tcp_closewt_state (_tcp_Socket **sp, const in_Header *ip,
                              tcp_Header *tcp, int flags)
{
  return tcp_estab_state (sp, ip, tcp, flags);
}

/*
 * FIN_WAIT1 state
 */
static int tcp_finwt1_state (_tcp_Socket **sp, const in_Header *ip,
                             tcp_Header *tcp, int flags)
{
  const in6_Header *ip6 = (const in6_Header*) ip;
  _tcp_Socket *s   = *sp;
  long  ldiff;
  int   len;

  if (is_ip4)
       len = intel16 (ip->length) - in_GetHdrLen (ip);
  else len = intel16 (ip6->len);

 /* Peer may not have read all the data yet, we
  * must still supply it as requested
  */
  if (tcp_process_ACK(s,&ldiff))
  {
    if (ldiff == 0 || s->send_una < 0)
       s->send_una = 0;
  }

  /* They may still be transmitting data, we must read it
   */
  tcp_process_data (s, tcp, len, &flags);

  /* Check if peer TCP has ACK'ed all sent data and
   * is ready to change states. No missing segment and got FIN+ACK.
   */
  if (s->missed_seq[0] == s->missed_seq[1] &&
      (flags & flag_FIN_ACK) == flag_FIN_ACK)
  {
    if (seqnum == s->recv_next)
    {
      s->recv_next++;               /* we must ACK their FIN! */

      if (SEQ_GEQ(acknum, s->send_next+1))
      {
        /* Not simultaneous close (they've ACKed our FIN)
         * We need to ACK their FIN and move to TIME_WAIT
         */
        s->send_next++;
        s->timeout = set_timeout (tcp_TIMEWT_TO);
        s->state   = tcp_StateTIMEWT;
      }
      else
      {
        /* Simultaneous close (haven't ACKed our FIN yet)
         * We need to ACK their FIN and move to CLOSING
         */
        s->timeout = set_timeout (tcp_TIMEOUT); /* !! S. Lawson, added 12.Nov 1999 */
        s->state   = tcp_StateCLOSING;
      }
      s->flags   = tcp_FlagACK;
      s->unhappy = FALSE;
      TCP_SEND (s);
    }
  }
  else if (flags & tcp_FlagACK)
  {
    /* other side is legitimately ACKing our FIN
     */
    if ((acknum == s->send_next + 1) &&
        (seqnum == s->recv_next)  &&
        (s->tx_datalen == 0))
    {
      if (!(s->locflags & LF_LINGER))
      {
        _tcp_unthread (s, TRUE);  /* enters tcp_StateCLOSED */
        return (0);
      }
      s->send_next++;
      s->state   = tcp_StateFINWT2;
      s->unhappy = FALSE;             /* we don't send anything */
      s->timeout = set_timeout (tcp_TIMEOUT);
    }
    else if ((acknum == s->send_next + 1) && (seqnum == s->recv_next + 1))
    {
     /* !! added 30-Aug 1999 GV
      * Try to stop that annoying retransmission bug/feature(?)
      * from FreeBSD 4.x which increments both SEQ and ACK.
      */
      s->send_next++;
      s->recv_next++;
      s->flags      = tcp_FlagRST;
      s->unhappy    = FALSE;
      s->karn_count = 0;
      s->tx_datalen = 0;
      TCP_SEND (s);
      _tcp_unthread (s, TRUE);
      return (0);
    }
  }
  return (1);
}

/*
 * FIN_WAIT2 state
 */
static int tcp_finwt2_state (_tcp_Socket **sp, const in_Header *ip,
                             tcp_Header *tcp, int flags)
{
  const in6_Header *ip6 = (const in6_Header*) ip;
  _tcp_Socket      *s   = *sp;
  int   len;

  if (is_ip4)
       len = intel16 (ip->length) - in_GetHdrLen (ip);
  else len = intel16 (ip6->len);

  /* They may still be transmitting data, we must read it
   */
  tcp_process_data (s, tcp, len, &flags);

  if (s->missed_seq[0] != s->missed_seq[1])
  {
    /* peer must retransmit to get all data */
    return (1);
  }

  if (flags & tcp_FlagFIN)
  {
    TRACE_CONSOLE (2, "tcp_finwt2_state(): got FIN\n");
    s->locflags |= LF_GOT_FIN;
  }

  if ((flags & tcp_FlagACK)  &&
      acknum == s->send_next &&
      seqnum == s->recv_next)
  {
    s->recv_next++;
    s->flags   = tcp_FlagACK;
    s->unhappy = FALSE;
    s->state   = tcp_StateTIMEWT;
    s->timeout = set_timeout (tcp_TIMEWT_TO);
    TCP_SEND (s);
    return (0);
  }
  return (1);
}


/*
 * CLOSING state
 */
static int tcp_closing_state (_tcp_Socket **sp, const in_Header *ip,
                              tcp_Header *tcp, int flags)
{
  _tcp_Socket *s = *sp;

  if ((flags & (tcp_FlagACK|tcp_FlagFIN)) == tcp_FlagACK)  /* ACK, no FIN */
  {
    /* Per FINWT1 above, acknum should be 's->send_next+1',
     * which should cause us to bump 's->send_next' to match.
     */
    if (SEQ_GT(acknum,s->send_next) && /* AGW - 6th Jan 2001 */
        seqnum == s->recv_next)
    {
      s->send_next++;
      s->state   = tcp_StateTIMEWT;
      s->unhappy = FALSE;
      s->timeout = set_timeout (tcp_TIMEWT_TO);
    }
  }
  ARGSUSED (ip);
  ARGSUSED (tcp);
  return (1);
}

/*
 * LASTACK state
 */
static int tcp_lastack_state (_tcp_Socket **sp, const in_Header *ip,
                              tcp_Header *tcp, int flags)
{
  _tcp_Socket *s = *sp;

  if (flags & tcp_FlagFIN)
  {
    /* they lost our two packets, back up
     */
    TRACE_CONSOLE (2, "tcp_lastack_state(): got FIN\n");

    s->locflags |= LF_GOT_FIN;
    s->flags     = flag_FIN_ACK;
    TCP_SEND (s);
    s->unhappy = TRUE;
    return (0);
  }

  if (SEQ_GT(acknum,s->send_next) && /* AGW allow for any later acks 6th Jan 2001 */
      seqnum == s->recv_next)
  {
    s->state   = tcp_StateCLOSED;   /* no 2*MSL necessary */
    s->unhappy = FALSE;             /* we're done         */
    return (0);
  }
  ARGSUSED (ip);
  ARGSUSED (tcp);
  return (1);
}

/*
 * TIMEWAIT state
 */
static int tcp_timewt_state (_tcp_Socket **sp, const in_Header *ip,
                             tcp_Header *tcp, int flags)
{
  _tcp_Socket *s = *sp;

  if (flags & tcp_FlagACK)
  {
    /* our peer needs an FIN-ACK which is sent by tcp_Retransmitter().
     */
    s->flags   = tcp_FlagACK;
    s->unhappy = FALSE;
    s->state   = tcp_StateCLOSED;  /* support 2 MSL in RST code */
    TCP_SEND (s);
  }
  ARGSUSED (ip);
  ARGSUSED (tcp);
  return (1);
}


/**
 * Process TCP options in segment.
 */
static void tcp_process_options (_tcp_Socket *s, const tcp_Header *tcp,
                                 const BYTE *tcp_data, int flags)
{
  const BYTE *opt = (const BYTE*)(tcp+1);
  WORD  max_seg;
  int   num = 0;

  /* Default to not sending timestamp
   */
  s->locflags &= ~LF_USE_TSTAMP;

  /* Clamp # of options to prevent DoS-attack from 0-sized
   * unknown options.
   */
  while (opt < tcp_data && num++ < 10)
  {
    switch (*opt)
    {
      case TCPOPT_EOL:
           return;

      case TCPOPT_NOP:
           opt++;
           break;

      case TCPOPT_MAXSEG:   /* we are very liberal on MSS stuff */
           if (flags & tcp_FlagSYN)
           {
             max_seg = intel16 (*(WORD*)(opt+2));
             if (!s->max_seg || max_seg < s->max_seg)
             {
               TRACE_CONSOLE (2, "Setting MSS %u\n", max_seg);
               s->max_seg = max_seg;
             }
           }
           opt += 4;
           break;

      case TCPOPT_TIMESTAMP:
           /* Use TS values only if SEQ is for new data.
            */
           if ((flags & tcp_FlagSYN) ||
               ((flags & tcp_FlagACK) &&
               (long)(seqnum - s->recv_next)))
           {
             s->ts_recent = intel (*(DWORD*)(opt+2));  /* echo this back */
             s->ts_echo   = intel (*(DWORD*)(opt+6));  /* use in RTT calc */
             s->locflags |= LF_USE_TSTAMP;             /* peer has this */
           }
           opt += 10;
           break;

      case TCPOPT_WINDOW:
           if (flags & tcp_FlagSYN)
           {
             s->rx_wscale = min (TCP_MAX_WINSHIFT, *(opt+2));
             s->locflags |= LF_RCVD_SCALE;
           }
           opt += 3;
           break;

      case TCPOPT_SACK_PERM:
           if (flags & tcp_FlagSYN)
              s->locflags |= LF_SACK_PERMIT;  /* no effect yet */
           opt += 2;
           break;

      case TCPOPT_CHKSUM_REQ:
           opt += 3;
           break;

      case TCPOPT_CHKSUM_DAT:
           opt += *(opt+1);
           break;

      default:              /* unknown options; type,length,... */
           opt += *(opt+1);
           break;
    }
  }
}


/*
 * TCP segment processsing:
 *
 * In the tcp_Socket structure:
 *
 *   - 'rx_datalen' is the index in the receive buffer where the next
 *     in-order data should be written.
 *
 *   - 'recv_next' is the TCP sequence number of the start of the next
 *     in-order data.
 *
 *   - 'missed_seq[0]' is the TCP sequence number of the first octet of
 *     the buffered out-of-order data.
 *
 *   - 'missed_seq[1]' is the TCP sequence number of the first octet
 *     following the buffered out-of-order data.  If missed_seq[0] and
 *     missed_seq[1] are equal, there is no buffered out-of-order data.
 *
 * 'ldiff' is the difference between the received sequence number and
 * the expected sequence number.  If 'ldiff' is zero or positive, and
 * 'ldiff' is less than the length of the packet, data can be appended
 * to the receive buffer.  If 'ldiff' is negative, data may be prepended
 * and/or appended to the buffered out-of-order data.
 *
 * Packets are discarded without processing if the *end* of the packet
 * is before 'recv_next', in which case we've already processed the
 * data, or after the advertised receive window ('s->adv_win').
 *
 * For 'ldiff >= 0', there are four cases to handle:
 *
 *  1) No out-of-order buffer: append the packet data and add the
 *     length to 'recv_next'.
 *
 *  2) No overlap with out-of-order buffer: same as (1).
 *
 *  3) New data reaches but doesn't extend out-of-order buffer:
 *     copy up to buffer, set 'recv_next' to 'missed_seq[1]', and
 *     clear 'missed_seq[0]' and 'missed_seq[1]'.
 *
 *  4) New data extends past out-of-order buffer: same as (1),
 *     also clearing 'missed_seq[0]' and 'missed_seq[1]'.
 *
 * For 'ldiff < 0', there are four further cases to handle:
 *
 *  5) No out-of-order buffer: copy the packet data and set
 *     'missed_seq[0]' and 'missed_seq[1]'.
 *
 *  6) Data starts at 'missed_seq[1]': append to out-of-order buffer
 *     and update 'missed_seq[1]'.
 *
 *  7) Data extends start: copy new data and update 'missed_seq[0]'.
 *
 *  8) Data extends end: copy new data and updated 'missed_seq[1]'.
 *
 * Case 7 and case 8 are not exclusive.
 *
 * When an out-of-order packet is received, -1 is returned so that a
 * duplicate acknowledgement will be sent immediately, signalling the
 * peer to use fast retransmit to resend the missing data.
 */

/*
 * Add data at 'rx_datalen', updating 'rx_datalen' and 'recv_next'.
 */
static void
copy_in_order (_tcp_Socket *s, const BYTE *data, unsigned len)
{
  TRACE_FILE ("copy_in_order (%u): Append %u bytes at %u-%u\n",
              __LINE__, len, s->rx_datalen, s->rx_datalen + len);
  memcpy (s->rx_data + s->rx_datalen, data, len);
  s->recv_next  += len;
  s->rx_datalen += len;
}

/*
 * Handle any new data that increments the 'recv_next' index.
 * 'ldiff >= 0'.
 */
static void
data_in_order (_tcp_Socket *s, const BYTE *data, unsigned len, unsigned diff)
{
  /* Skip data before recv_next. We must be left with some data or
   * we wouldn't have been called.
   */
  data += diff;
  len  -= diff;

  if (s->protoHandler)
  {
    s->recv_next += (*s->protoHandler) (s, data, len, NULL, NULL);
  }
  else if (s->missed_seq[0] == s->missed_seq[1] ||
           s->recv_next + len < s->missed_seq[0])
  {
    /* (1) Normal case: just copy all the data to the buffer.
     * (2) Received in-order doesn't catch up to buffered out-of-order.
     */
    copy_in_order (s, data, len);
  }
  else
  {
    /* (3) Received data catches up to saved out-of-order data.
     * (4) Received data extends beyond saved out-of-order data.
     */
    unsigned ms_end = s->missed_seq[1] - s->recv_next;

    copy_in_order (s, data, s->missed_seq[0] - s->recv_next);

    /* Update offset and length to incorporate out-of-order data.
     */
    TRACE_FILE ("data_in_order (%u): Use %lu out-of-order bytes\n",
                __LINE__, (u_long)(s->missed_seq[1] - s->missed_seq[0]));
    s->rx_datalen   += (s->missed_seq[1] - s->missed_seq[0]);
    s->recv_next     = s->missed_seq[1];
    s->missed_seq[0] = s->missed_seq[1] = 0;

    if (len > ms_end)
    {
      /* (4) Extend out-of-order data, if received past end.
       */
      copy_in_order (s, data + ms_end, len - ms_end);
    }
  }

  TRACE_FILE ("data_in_order (%u): edges %lu/%lu, recv.next %lu\n",
              __LINE__, (u_long)s->missed_seq[0], (u_long)s->missed_seq[1],
              (u_long)s->recv_next);

  TRACE_FILE ("data_in_order (%u): new data now ends at %u\n",
              __LINE__, s->rx_datalen);
}

/*
 * Add data before 'missed_seq[0]', updating its value.
 */
static void
prepend_out_of_order (_tcp_Socket *s, const BYTE *data, unsigned len)
{
  unsigned start = s->missed_seq[0] - s->recv_next + s->rx_datalen - len;

  TRACE_FILE ("prepend_out_of_order (%u): Prepend %u bytes at %u-%u\n",
              __LINE__, len, start, start + len);
  memcpy (s->rx_data + start, data, len);
  s->missed_seq[0] -= len;
}

/*
 * Add data after 'missed_seq[1]', updating its value.
 */
static void
append_out_of_order (_tcp_Socket *s, const BYTE *data, unsigned len)
{
  unsigned start = s->missed_seq[1] - s->recv_next + s->rx_datalen;

  TRACE_FILE ("append_out_of_order (%u): Append %u bytes at %u-%u\n",
              __LINE__, len, start, start + len);
  memcpy (s->rx_data + start, data, len);
  s->missed_seq[1] += len;
}

/*
 * Handle one out-of-segment packet (ldiff < 0)
 */
static void
data_out_of_order (_tcp_Socket *s, const BYTE *data, unsigned len, unsigned diff)
{
  if (s->missed_seq[0] == s->missed_seq[1])
  {
    /* (5) First out-of-order data.
     */
    s->missed_seq[0] = s->missed_seq[1] = seqnum;
    append_out_of_order (s, data, len);
  }
  else if (seqnum == s->missed_seq[1])
  {
    /* (6) Common case: immediately following last out-of-order packet.
     */
    append_out_of_order (s, data, len);
  }
  else
  {
    /* Offsets from recv_next:
     *  - diff is offset to start of received data;
     *  - diff + len is offset to after received data;
     *  - left is offset to start of saved out-of-order data;
     *  - right is offset to after saved out-of-order data.
     */
    unsigned left  = s->missed_seq[0] - s->recv_next;
    unsigned right = s->missed_seq[1] - s->recv_next;

    if (diff < left && left <= diff + len)
    {
      /* (7) Data extending start. */
      prepend_out_of_order (s, data, left - diff);
    }

    if (diff <= right && right < diff + len)
    {
      /* (8) Data extending end. */
      append_out_of_order (s, data - diff + right, diff + len - right);
    }
  }

  TRACE_FILE ("data_out_of_order (%u): edges %lu/%lu, recv.next %lu\n",
              __LINE__, (u_long)s->missed_seq[0], (u_long)s->missed_seq[1], (u_long)s->recv_next);
}

/**
 * Process the data in an incoming segment.
 * Called from all states where incoming data can be received:
 * SYNSENT, ESTAB, ESTCL, CLOSWT, FIN-WAIT-1 and FIN-WAIT-2.
 */
static int tcp_process_data (_tcp_Socket *s, const tcp_Header *tcp,
                             int len, int *flags)
{
  long  ldiff;
  int   data_ofs;
  const BYTE *data;

  if (s->stress > 0)
      s->stress--;

  tcp_set_window (s, tcp);  /* this one should be redundant */

  ldiff = (long) (s->recv_next - seqnum);

  if (*flags & tcp_FlagSYN)
     ldiff--;                         /* back up to 0 */

  /* find the data portion
   */
  data_ofs = tcp->offset << 2;        /* dword to byte offset */
  data     = (const BYTE*)tcp + data_ofs;

  if (data_ofs - sizeof(*tcp) > 0)
     tcp_process_options (s, tcp, data, *flags);

  if (len - data_ofs < 0)
  {
    STAT (tcpstats.tcps_rcvbadoff++);
    len = 0;
  }
  else
    len -= data_ofs;    /* remove the header length */

  TRACE_FILE ("tcp_process_data (%u): len %u, ldiff %ld\n", __LINE__, len, ldiff);

  /** \todo Handle Out-of-Order urgent data. Raise SIGURG.
   */
#if 0
  if ((*flags & tcp_FlagURG) && tcp->urgent && intel16(tcp->urgent) < len)
  {
    intel16 (tcp->urgent) + seq;
  }
#endif

  /*
   * SYN/RST segments shouldn't carry any data.
   * But SYN-ACK can (?)
   */
  if (*flags & (tcp_FlagSYN | tcp_FlagRST))
     return (0);

  if (ldiff)   /* Out-of-Sequence data */
  {
    STAT (tcpstats.tcps_rcvoopack++);
    STAT (tcpstats.tcps_rcvoobyte += len);
  }
  else
  {
    STAT (tcpstats.tcps_rcvpack++);
    STAT (tcpstats.tcps_rcvbyte += len);
  }

  /* No TCP data, so nothing more to do
   */
  if (len == 0)
     return (0);

  /* Check that *end* of packet is valid, i.e. will fit in advertised receive
   * window ('adv_win'). If it's before 'recv_next', we've seen it all before;
   * if it's after then the peer (or someone else) sent more than we said we
   * could take.
   */

  /*
   * Thanks to Mikulas Patocka <mikulas@twibright.com> for finding a problem
   * with the previous 'if-test':
   *   if we receive a sequence of packets, all the packets are checked
   *   against 's->adv_win', but 's->adv_win' is not decreased as the packets are
   *   received, it remains the same. Consequently a packet that is out of window
   *   is errorneously accepted and corrupts memory.
   *
   * Hence this 'if-test' is used instead:
   */

  /* First patch by Mikulas Patocka:
   * checks that end of packet does not land above the TCP window
   */
  if ( ((unsigned)len - ldiff > s->max_rx_data - s->rx_datalen) ||
      /*
       * Second patch by Mateusz Viste:
       * checks that end of packet does not land below the TCP window
       * (legitimate case when remote peer retransmits an already-received
       * segment that Watt-32 did not acquit due to delayed ack).
       *
       * Details at https://github.com/gvanem/Watt-32/issues/2
       */
      (ldiff > len) )
  {
    TRACE_FILE ("tcp_process_data (%u): packet ends outside %lu/%lu\n",
                __LINE__, (u_long)s->recv_next, (u_long)(s->recv_next + s->adv_win));
    STAT (tcpstats.tcps_rcvpackafterwin++);
    return (0);
  }

  /* Handle any new data that increments the 'recv_next' index.
   */

  if (ldiff >= 0)
  {
    data_in_order (s, data, len, ldiff);
    s->unhappy = (s->tx_datalen > 0);
    return (0);
  }

  STAT (tcpstats.tcps_rcvduppack++);      /* increment dup-ACK count */
  STAT (tcpstats.tcps_rcvdupbyte += len);

  /* No out-of-sequence processing of FIN flag - S. Lawson
   */
  if (*flags & tcp_FlagFIN)
  {
    TRACE_FILE ("tcp_process_data (%u): clearing FIN\n", __LINE__);
    *flags &= ~tcp_FlagFIN;
  }

  data_out_of_order (s, data, len, -ldiff);
  s->unhappy = TRUE;
  return (-1);
}


/*
 * Process the ACK value in received packet, but only if it falls within
 * current window. Discard queued Tx-data that has been acknowledged.
 */
static int tcp_process_ACK (_tcp_Socket *s, long *unacked)
{
  long ldiff = (long) (acknum - s->send_next);
  int  diff  = (int) ldiff;

  if (unacked)
     *unacked = ldiff;

#if 0
  if (ldiff > s->window)  /* peer ACK'd data not sent yet */
  {
    TCP_SEND (s);
    STAT (tcpstats.tcps_rcvacktoomuch++);
    return (1);
  }
#endif

  if (ldiff >= 0 && (unsigned)diff <= s->tx_datalen)
  {
    if (s->tx_queuelen)
    {
      s->tx_queue    += diff;
      s->tx_queuelen -= diff;
    }
    else if ((unsigned)diff < s->tx_datalen && diff > 0)
    {
      memmove (s->tx_data, s->tx_data+diff, s->tx_datalen-diff);
    }

    diff = min (diff, (long)s->tx_datalen); /* protect against "nastygrams" */
    s->tx_datalen -= diff;
    s->send_una   -= diff;
    s->send_next  += ldiff;

    /* If peer ACK'ed everything and all queued data sent,
     * stop RTT-timer
     */
    if (s->send_una == 0 && s->tx_datalen == 0)
       s->rtt_time = 0UL;

    STAT (tcpstats.tcps_rcvackpack++);
    STAT (tcpstats.tcps_rcvackbyte += ldiff);
    return (1);
  }
  return (0);
}

/**
 * Allocate a Tx-buffer based on peer's advertised window.
 * Note: our advertised window (s->adv_win) is controlled by
 *       sock_setbuf().
 */
static void tcp_set_window (_tcp_Socket *s, const tcp_Header *tcp)
{
  s->window = intel16 (tcp->window);
  if (s->window > MAX_WINDOW)
      s->window = MAX_WINDOW;

#if 0  /** \todo Set slow-start threshold */
  if (s->send_ssthresh == 0)
     s->send_ssthresh = s->window;
#endif

  if (s->tx_data == &s->tx_buf[0] &&  /* Tx-data in _tcp_Socket */
      s->window > s->max_tx_data)     /* His window > our Tx-size */
  {
    WORD   window = s->window;
    size_t size;
    BYTE  *buf;

#if defined(__MSDOS__)
    if (_eth_ndis3pkt)       /* limit NDIS3PKT's in-transit bytes */
       window = min (window, 6*_mss);
#endif

    size = window + 8;       /* add size for markers */
    buf  = malloc (size);

    TRACE_FILE ("tcp_set_window (%u): buf %p, size %lu, datalen %u\n",
                __LINE__, buf, (u_long)size, s->tx_datalen);
    if (!buf)
       return;

    *(DWORD*)buf          = SAFETY_TCP;
    *(DWORD*)(buf+size-4) = SAFETY_TCP;
    if (s->tx_datalen)
       memcpy (buf+4, s->tx_data, s->tx_datalen);  /* copy to new buf */
    s->tx_data     = buf + 4;
    s->max_tx_data = window - 1;
  }
}

/**
 * \todo Create a better TCP reassembler some day. Current logic only
 *       handles 1 missed segment. We should handle at least 2.
 */
#if defined(USE_NEW_TCP_REASM)

static int tcp_reassemble (_tcp_Socket *s, const tcp_Header *tcp,
                           const BYTE *data, UINT len, int *flags)
{
  DWORD left_edge, right_edge;
  long  size, ofs;

  if (len == 0)
     return (0);

  if (SEQ_EQ(seqnum, s->recv_next))
  {
    /* normal enqueue */
    return (0);
  }

  left_edge  = s->recv_next - s->rx_datalen;
#if 0
  right_edge = s->recv_next + s->adv_win;
#else
  right_edge = s->recv_next + s->max_rx_data - s->rx_datalen;
#endif

  /* segment is left of expected recv-window
   */
  if (SEQ_LEQ(seqnum, left_edge) &&
      SEQ_LEQ(seqnum + len, left_edge))
  {
    return (0);
  }

  /* Some of the segment is left of expected recv-window
   */
  if (SEQ_LEQ(seqnum, left_edge) &&
      SEQ_BETWEEN(seqnum + len, left_edge+1, right_edge))
  {
    size = seqnum + len - left_edge;
    ofs  = left_edge - seqnum;
    memcpy (s->rx_data + s->rx_datalen, data + ofs, size);
    return (0);
  }

  /* The normal case
   */
  if (SEQ_BETWEEN(seqnum,left_edge,right_edge-1) &&
      SEQ_GT(seqnum+len,right_edge))
  {
    size = len;
    ofs  = seqnum - left_edge;
    memcpy (s->rx_data + s->rx_datalen + ofs, data, size);
    s->rx_datalen += len;
    s->recv_next = seqnum + len;
    return (0);
  }

  if (SEQ_GT(seqnum,right_edge))
  {
    return (0);
  }
  return (-1);   /* unexpected case */
}
#endif /* USE_NEW_TCP_REASM */

#endif /* !USE_UDP_ONLY */

