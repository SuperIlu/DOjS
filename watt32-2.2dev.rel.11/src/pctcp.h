/*!\file pctcp.h
 */
#ifndef _w32_PCTCP_H
#define _w32_PCTCP_H

/**
 * \def Timer definitions.
 * All timers are in milli-seconds.
 */
#define tcp_TIMEOUT       13000UL            /* timeout during a connection */
#define tcp_LONGTIMEOUT  (1000UL*sock_delay) /* timeout for open */
#define tcp_LASTACK_TIME  10000     /* timeout in the LASTACK state added AGW 5th Jan 2001 */

#define DEF_OPEN_TO       1000UL    /* # of msec in tcp-open (<=3s; RFC1122) */
#define DEF_CLOSE_TO      1000UL    /* # of msec for CLOSEWT state */
#define DEF_RTO_ADD       100UL     /* # of msec delay in RTT increment */
#define DEF_RTO_BASE      10UL      /* # of msec in RTO recalculation */
#define DEF_RTO_SCALE     64        /* RTO scale factor in _tcp_sendsoon() */
#define DEF_RST_TIME      100UL     /* # of msec before sending RST */
#define DEF_RETRAN_TIME   10UL      /* do retransmit logic every 10ms */
#define DEF_RECV_WIN     (16*1024)  /* default receive window, 16kB */


/*
 * S. Lawson - define a short TIME_WAIT timeout. It should be from
 * .5 to 4 minutes (2MSL) but it's not really practical for us.
 * 2 secs will hopefully handle the case where ACK must be retransmitted,
 * but can't protect future connections on the same port from old packets.
 */
#define tcp_TIMEWT_TO 2000UL

#if !defined(__NETINET_TCP_SEQ_H)

  /**
   * \def SEQ_* macros.
   *
   * TCP sequence numbers are 32 bit integers operated
   * on with modular arithmetic.  These macros can be
   * used to compare such integers.
   */
  #define SEQ_LT(a,b)            ((long)((a) - (b)) < 0)
  #define SEQ_LEQ(a,b)           ((long)((a) - (b)) <= 0)
  #define SEQ_GT(a,b)            ((long)((a) - (b)) > 0)
  #define SEQ_GEQ(a,b)           ((long)((a) - (b)) >= 0)
  #define SEQ_BETWEEN(seq,lo,hi) ((seq - lo) <= (hi - lo))
                              /* (SEG_GEQ(seq,lo) && SEG_LEQ(seq,hi)) */
#endif

/**
 * \def INIT_SEQ
 *
 * We use 32-bit from system-timer as initial sequence number
 * (ISN, network order). Maybe not the best choice (easy guessable).
 * The ISN should wrap only once a day.
 */
#define INIT_SEQ()  intel (Random(1,0xFFFFFFFF)) /* intel (set_timeout(1)) */

/**
 * Van Jacobson's Algorithm; max std. average and std. deviation
 */
#define DEF_MAX_VJSA    60000U
#define DEF_MAX_VJSD    20000U
#define INIT_VJSA       220

/**
 * The TCP options.
 */
#define TCPOPT_EOL        0       /**< end-of-option list */
#define TCPOPT_NOP        1       /**< no-operation */
#define TCPOPT_MAXSEG     2       /**< maximum segment size */
#define TCPOPT_WINDOW     3       /**< window scale factor        RFC1072 */
#define TCPOPT_SACK_PERM  4       /**< selective ack permitted    RFC1072 */
#define TCPOPT_SACK       5       /**< selective ack              RFC1072 */
#define TCPOPT_ECHO       6       /**< echo-request               RFC1072 */
#define TCPOPT_ECHOREPLY  7       /**< echo-reply                 RFC1072 */
#define TCPOPT_TIMESTAMP  8       /**< timestamps                 RFC1323 */
#define TCPOPT_CC         11      /**< T/TCP CC options           RFC1644 */
#define TCPOPT_CCNEW      12      /**< T/TCP CC options           RFC1644 */
#define TCPOPT_CCECHO     13      /**< T/TCP CC options           RFC1644 */
#define TCPOPT_CHKSUM_REQ 14      /**< Alternate checksum request RFC1146 */
#define TCPOPT_CHKSUM_DAT 15      /**< Alternate checksum data    RFC1146 */
#define TCPOPT_SIGNATURE  19      /**< MD5 signature              RFC2385 */
#define   TCPOPT_SIGN_LEN 16

#define TCP_MAX_WINSHIFT  14      /**< maximum window shift */


/**
 * MTU defaults to 1500 (ETH_MAX_DATA).
 * TCP_OVERHEAD == 40.
 */
#define MSS_MAX       (_mtu - TCP_OVERHEAD)
#define MSS_MIN       (576 - TCP_OVERHEAD)
#define MSS_REDUCE     20   /* do better than this (exponentially decrease) */

#define my_ip_addr     W32_NAMESPACE (my_ip_addr)
#define sin_mask       W32_NAMESPACE (sin_mask)
#define block_tcp      W32_NAMESPACE (block_tcp)
#define block_udp      W32_NAMESPACE (block_udp)
#define block_ip       W32_NAMESPACE (block_ip)
#define block_icmp     W32_NAMESPACE (block_icmp)
#define use_rand_lport W32_NAMESPACE (use_rand_lport)

#define mtu_discover   W32_NAMESPACE (mtu_discover)
#define mtu_blackhole  W32_NAMESPACE (mtu_blackhole)

#define tcp_nagle      W32_NAMESPACE (tcp_nagle)
#define tcp_keep_idle  W32_NAMESPACE (tcp_keep_idle)
#define tcp_keep_intvl W32_NAMESPACE (tcp_keep_intvl)
#define tcp_max_idle   W32_NAMESPACE (tcp_max_idle)
#define tcp_opt_ts     W32_NAMESPACE (tcp_opt_ts)
#define tcp_opt_sack   W32_NAMESPACE (tcp_opt_sack)
#define tcp_opt_wscale W32_NAMESPACE (tcp_opt_wscale)
#define hostname       W32_NAMESPACE (hostname)

extern BOOL mtu_discover;
extern BOOL mtu_blackhole;
extern BOOL use_rand_lport;
extern BOOL tcp_nagle;
extern BOOL tcp_opt_ts;
extern BOOL tcp_opt_sack;
extern BOOL tcp_opt_wscale;

extern char hostname [MAX_HOSTLEN+1];

extern _tcp_Socket *_tcp_allsocs;
extern _udp_Socket *_udp_allsocs;

extern void _udp_cancel (const in_Header *ip, int icmp_type, int icmp_code,
                         const char *msg, const void *arg);

extern void _tcp_cancel (const in_Header *ip, int icmp_type, int icmp_code,
                         const char *msg, const void *arg);

extern void _tcp_close     (_tcp_Socket *s);
extern void  tcp_rtt_add   (const _tcp_Socket *s, UINT rto, UINT MTU);
extern void  tcp_rtt_clr   (const _tcp_Socket *s);
extern BOOL  tcp_rtt_get   (const _tcp_Socket *s, UINT *rto, UINT *MTU);

extern int  _tcp_send      (_tcp_Socket *s, const char *file, unsigned line);
extern int  _tcp_sendsoon  (_tcp_Socket *s, const char *file, unsigned line);
extern int  _tcp_keepalive (_tcp_Socket *s);

extern void tcp_Retransmitter (BOOL force);

extern _udp_Socket *_udp_handler  (const in_Header *ip, BOOL broadcast);
extern _tcp_Socket *_tcp_handler  (const in_Header *ip, BOOL broadcast);
extern _tcp_Socket *_tcp_unthread (_tcp_Socket *s, BOOL free_tx);
extern _tcp_Socket *_tcp_abort    (_tcp_Socket *s, const char *file, unsigned line);
extern int          _tcp_send_reset (_tcp_Socket *s, const in_Header *ip,
                                     const tcp_Header *tcp, const char *file,
                                     unsigned line);

#define TCP_SEND(s)     _tcp_send     (s, __FILE__, __LINE__)
#define TCP_SENDSOON(s) _tcp_sendsoon (s, __FILE__, __LINE__)
#define TCP_ABORT(s)    _tcp_abort    (s, __FILE__, __LINE__)

#define TCP_SEND_RESET(s, ip, tcp) \
       _tcp_send_reset(s, ip, tcp, __FILE__, __LINE__)

#define SET_ERR_MSG(s, msg) \
        do { \
          if (s && s->err_msg == NULL && msg != NULL) \
             s->err_msg = _strlcpy (s->err_buf, msg, sizeof(s->err_buf)); \
        } while (0)

/*!\struct tcp_rtt
 *
 * A simple RTT cache based on Phil Karn's KA9Q.
 */
struct tcp_rtt {
       DWORD  ip;     /**< IP-address of this entry */
       UINT   rto;    /**< Round-trip timeout for this entry */
       UINT   MTU;    /**< Path-MTU discovered for this entry */
     };

#define RTTCACHE  16  /**< # of TCP round-trip-time cache entries */


#if defined(USE_BSD_API)
  /*
   * Operations for the '_bsd_socket_hook'.
   */
  enum BSD_SOCKET_OPS {
       BSO_FIND_SOCK = 1,   /* return a 'Socket*' from '_tcp_Socket*' */
       BSO_SYN_CALLBACK,    /* called on SYN received */
       BSO_RST_CALLBACK,    /* called on RST received */
       BSO_IP4_RAW,         /* called on IPv4 input. */
       BSO_IP6_RAW,         /* called on IPv6 input. */
       BSO_DEBUG,           /* called to perform SO_DEBUG stuff */
     };

  extern void * (MS_CDECL *_bsd_socket_hook) (enum BSD_SOCKET_OPS op, ...);
#endif

/* In ports.c
 */
extern WORD init_localport  (void);
extern WORD find_free_port  (WORD oldport, BOOL sleep_msl);
extern int  grab_localport  (WORD port);
extern int  reuse_localport (WORD port);
extern int  maybe_reuse_localport (_tcp_Socket *s);

#endif
