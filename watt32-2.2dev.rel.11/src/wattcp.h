#ifndef _w32_WATTCP_H
#define _w32_WATTCP_H

/**
 * \file wattcp.h
 *
 * \brief Core definitions. Types and structures for protocols are defined here.
 *
 * Problems: - MinGW-w64 defines both WIN32 and WIN64,
 *             but _WIN64 is defined only if actually targeting WIN64.
 *           - MSVC + PellesC defines only _WIN32 for targeting 32-bit Windows.
 *           - MSVC + PellesC defines _WIN32+_WIN64 for targeting 64-bit Windows.
 *           - CygWin's gcc defines none of the above. But it supports
 *             native Win64 if '__x86_64__' is set. (needs 'CygWin64').
 */
#if defined(WIN64) || defined(_WIN64)
  #undef  WIN64
  #undef _WIN64
  #define  WIN64 1
  #define _WIN64 1

#elif defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__)
  #undef  WIN32
  #undef _WIN32
  #define  WIN32 1
  #define _WIN32 1
#endif

#if defined(__CYGWIN__) && defined(__x86_64__)
  #define  WIN64 1
  #define _WIN64 1
#endif

#if defined(WIN32) || defined(WIN64)
  /*
   * This must come before "target.h".
   * Prevent including <winsock*.h> and other seldom used headers.
   */
  #ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN 1
  #endif

  /*
   * Required for bsdname.c, winpkt.c and timer.c
   */
  #if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0501)
    #undef  _WIN32_WINNT
    #define _WIN32_WINNT 0x0501
  #endif

  #define WATT32_ON_WINDOWS
#endif

#ifndef WATT32_BUILD
#define WATT32_BUILD
#endif

#include <sys/w32api.h>      /**< export/import decorations */
#include <sys/wtypes.h>      /**< basic type stuff */
#include <sys/wtime.h>       /**< time types */

/**<\typedef 64-bit types (compiler dependent).
 */
#if defined(__HIGHC__) || defined(__GNUC__) || defined(__CCDL__) || \
    defined(__LCC__)   || defined(__POCC__)
  typedef unsigned long long  uint64;  /**< our unsigned "long long" type */
  typedef long long           int64;   /**< our signed "long long" type */
  #define HAVE_UINT64                  /**< have a compiler with 64-bit ints */

#elif defined(__DMC__) && (__INTSIZE == 4)
  typedef unsigned long long  uint64;
  typedef long long           int64;
  #define HAVE_UINT64

#elif defined(__WATCOMC__) && defined(__WATCOM_INT64__) && !(defined(__SMALL__) || defined(__LARGE__))
  typedef unsigned __int64 uint64;
  typedef __int64          int64;
  #define HAVE_UINT64

#elif defined(_MSC_VER) && (_MSC_VER >= 900)
  typedef unsigned __int64 uint64;
  typedef __int64          int64;
  #define HAVE_UINT64

#elif defined(__BORLANDC__) && defined(WIN32)
  typedef unsigned __int64 uint64;
  typedef __int64          int64;
  #define HAVE_UINT64
#endif

struct ulong_long {
       DWORD lo;
       DWORD hi;
      };

/* On Win32/VC9+: typedef __w64 LONG DWORD_PTR;
 */
#if !defined(WIN32) && !defined(WIN64)
  #define DWORD_PTR     DWORD
  #define IntToPtr(i)   ((void*)(unsigned)(i))
#endif

#if defined(__DMC__)
  #if !defined(ULONG_PTR)
    #ifdef _WIN64
      #define ULONG_PTR  unsigned long long
    #else
      #define ULONG_PTR  unsigned long
    #endif
  #endif

  #if !defined(DWORD_PTR)
    #define DWORD_PTR  ULONG_PTR
  #endif

  #if !defined(ULONG64)
    #define ULONG64  ULONG_PTR
  #endif
#endif


/* Compiling with "cl -RTCc" causes "Run-Time Check Failure" -
 *   A cast to a smaller data type has caused a loss of data.  If this
 *   was intentional, you should mask the source of the cast with the appropriate bitmask.
 *
 * Hence these macros does that. But don't use '-RTCc' in release mode (-MD).
 */

#if defined(_MSC_VER) && defined(_DEBUG)
  #define loBYTE(w)   (BYTE)((w) & 0xFF)
  #define hiBYTE(w)   (BYTE)((WORD)((w) & 0xFF00) >> 8)
  #define loWORD(x)   (WORD)((x) & 0xFFFF)
  #define hiWORD(x)   ((WORD)((x) & 0xFFFF0000) >> 16)

#else
  #define loBYTE(w)   (BYTE)(w)
  #define hiBYTE(w)   (BYTE)((WORD)(w) >> 8)
  #define loWORD(x)   (WORD)(x)
  #define hiWORD(x)   (WORD)((WORD)(w) >> 16)
#endif

#define DIM(x)        (int) (sizeof(x) / sizeof((x)[0]))
#define SIZEOF(x)     (int) sizeof(x)

#if defined(__LCC__)  /* Lcc-win32 is a bit peculiar */
  #define ARGSUSED(foo)  foo = foo
  #define ATOI(x)        atoi ((char*)(x))
  #define ATOL(x)        atol ((char*)(x))
#else
  #define ARGSUSED(foo)  (void)foo
  #define ATOI(x)        atoi (x)
  #define ATOL(x)        atol (x)
#endif

/**
 * Compiler and target definitions.
 */
#include "target.h"          /**< portability macros & defines. */
#include "config.h"          /**< options & features to include. */

#if defined(SWIG) && defined(USE_IPV6)
  #error "Swig and 'USE_IPV6' does not work at the moment. Edit $(WATT_ROOT)/config.h to NOT define USE_IPV6"
#endif

#if (DOSX)
  #define TARGET_IS_32BIT    /**< 32-bit or 64-bit targets. */
#endif

#if defined(USE_CRTDBG)      /**< use CrtDebug in MSVC debug-mode */
  #define _CRTDBG_MAP_ALLOC
  #undef _malloca            /* Avoid MSVC-9 <malloc.h>/<crtdbg.h> name-clash */
  #include <crtdbg.h>

#elif defined(USE_FORTIFY)
  #include "fortify.h"       /**< use Fortify malloc code (internal to watt-32) */

#elif defined(USE_MPATROL)
  #include <mpatrol.h>       /**< use Mpatrol malloc library (externally linked). */

  #if defined(_MSC_VER) && (_MSC_VER >= 1400)
  #pragma comment(lib, "libmpatrol.lib")
  #pragma comment(lib, "imagehlp.lib")
  #endif
#endif

#ifndef FALSE
#define FALSE  0
#endif

#ifndef TRUE
#define TRUE   (!FALSE)
#endif


/**
 * Sizes and protocols we use.
 */
#define ETH_MAX_DATA     1500
#define ETH_MIN          60
#define ETH_MAX          (ETH_MAX_DATA + sizeof(eth_Header))

#define TOK_MAX_DATA     ETH_MAX_DATA  /* could be much larger */
#define TOK_MIN          sizeof(tok_Header)
#define TOK_MAX          (TOK_MAX_DATA + sizeof(tok_Header))

#define FDDI_MAX_DATA    ETH_MAX_DATA  /* really is 4470 */
#define FDDI_MIN         (3 + sizeof(fddi_Header))
#define FDDI_MAX         (FDDI_MAX_DATA + sizeof(fddi_Header))

#define ARCNET_MAX_DATA  512    /* Long frame or Exception frame */
#define ARCNET_MIN       257
#define ARCNET_MAX       (ARCNET_MAX_DATA + sizeof(arcnet_Header))

#define AX25_MAX_DATA    ETH_MAX_DATA
#define AX25_MIN         sizeof(ax25_Header)
#define AX25_MAX         (AX25_MAX_DATA + sizeof(ax25_Header))

#define PPPOE_MAX_DATA   (ETH_MAX_DATA - 8)
#define PPPOE_MIN        sizeof(pppoe_Header)
#define PPPOE_MAX        (PPPOE_MAX_DATA + sizeof(pppoe_Header))

#define VLAN_MAX_DATA    (ETH_MAX_DATA - 4)
#define VLAN_MAX         (VLAN_MAX_DATA + sizeof(vlan_Header))
#define VLAN_MIN         sizeof(vlan_Header)

#define TCP_OVERHEAD     (sizeof(in_Header) + sizeof(tcp_Header))
#define UDP_OVERHEAD     (sizeof(in_Header) + sizeof(udp_Header))


/** Ether-protocol numbers.
 * \note these are on network (big-endian) order.
 */
#define IP4_TYPE         0x0008
#define IP6_TYPE         0xDD86
#define IEEE802_1Q_TYPE  0x0081   /* Virtual LAN */
#define ARP_TYPE         0x0608
#define RARP_TYPE        0x3580
#define PPPOE_DISC_TYPE  0x6388
#define PPPOE_SESS_TYPE  0x6488
#define LLDP_TYPE        0xCC88

/** ARCNET-protocol numbers from EtheReal.
 * \note these are all a single byte on wire.
 */
#define ARCNET_DP_BOOT    0
#define ARCNET_DP_MOUNT   1
#define ARCNET_PL_BEACON  8
#define ARCNET_PL_BEACON2 243

#define ARCNET_DIAG       128
#define ARCNET_IP6        196
#define ARCNET_BACNET     205
#define ARCNET_IP_1201    212
#define ARCNET_ARP_1201   213
#define ARCNET_RARP_1201  214
#define ARCNET_ATALK      221
#define ARCNET_ETHER      232
#define ARCNET_NOVELL     236
#define ARCNET_IP_1051    240
#define ARCNET_ARP_1051   241
#define ARCNET_BANYAN     247
#define ARCNET_IPX        250
#define ARCNET_LANSOFT    251


#if (DOSX)
  #define MAX_FRAGMENTS   45UL
  #define MAX_WINDOW      (64*1024U)   /**< max TCP window */
#else
  #define MAX_FRAGMENTS   30UL
  #define MAX_WINDOW      (32*1024U)
#endif

/** This should really be a function of current MAC-driver.
 */
#define MAX_IP4_DATA      (ETH_MAX_DATA - sizeof(in_Header))
#define MAX_IP6_DATA      (ETH_MAX_DATA - sizeof(in6_Header))
#define MAX_FRAG_SIZE     (MAX_FRAGMENTS * MAX_IP4_DATA)

#define MAX_ADDRESSES     10           /**< # of addresses in resolvers */
#define MAX_NAMELEN       80           /**< max length of a wattcp.cfg keyword */
#define MAX_VALUELEN      80           /**< max length of a wattcp.cfg value */

#if defined(WIN32) || defined(WIN64)
  #define MAX_PATHLEN     MAX_PATH     /* 260 from <windef.h> */
#else
  #define MAX_PATHLEN     256          /**< should be enough for most... */
#endif

#define SAFETY_TCP        0x538F25A3L  /**< marker signatures */
#define SAFETY_UDP        0x3E45E154L

/**
 * The IP protocol numbers we need (see RFC-1700).
 */
#define UDP_PROTO         17
#define TCP_PROTO         6
#define ICMP_PROTO        1
#define IGMP_PROTO        2
#define IPCOMP_PROTO      108
#define SCTP_PROTO        132

/**
 * _udp_Socket/_tcp_Socket 'sockmode' values
 */
#define SOCK_MODE_BINARY  0x01    /**< default mode (ASCII mode if 0) */
#define SOCK_MODE_UDPCHK  0x02    /**< UDP: do checksum checks (default) */
#define SOCK_MODE_NAGLE   0x04    /**< Nagle algorithm (default) */
#define SOCK_MODE_LOCAL   0x08    /**< set by sock_noflush() */
#define SOCK_MODE_SAWCR   0x10    /**< for ASCII sockets; saw a newline */
#define SOCK_MODE_MASK    0x07    /**< mask for sock_mode() */


/**
 * UDP/TCP socket local flags (locflags) bits.
 * Mostly used to support the BSD-socket API.
 */
#define LF_WINUPDATE    0x00001   /**< need to send a window-update */
#define LF_NOPUSH       0x00002   /**< don't push on write */
#define LF_NOOPT        0x00004   /**< don't use tcp options */
#define LF_REUSEADDR    0x00008   /**< \todo Reuse address not supported */
#define LF_KEEPALIVE    0x00010   /**< we got a keepalive ACK */
#define LF_LINGER       0x00020
#define LF_NOCLOSE      0x00040
#define LF_NO_IPFRAGS   0x00080
#define LF_OOBINLINE    0x00100
#define LF_SNDTIMEO     0x00200
#define LF_RCVTIMEO     0x00400
#define LF_GOT_FIN      0x00800
#define LF_GOT_PUSH     0x01000
#define LF_GOT_ICMP     0x02000   /**< got an ICMP port/dest unreachable */
#define LF_USE_TSTAMP   0x04000   /**< send a TS option on next send */
#define LF_RCVD_SCALE   0x08000   /**< a win-scale was received in SYN */
#define LF_IS_SERVER    0x10000   /**< socket is a server (listening)  */
#define LF_SACK_PERMIT  0x20000

/**
 * Socket-states for sock_sselect().
 * Not used by BSD-socket API
 */
#define SOCKESTABLISHED  1
#define SOCKDATAREADY    2
#define SOCKCLOSED       4

W32_CLANG_PACK_WARN_OFF()

#include <sys/pack_on.h>  /**< align structs on byte boundaries */

/*!\struct in_Header
 *
 * The Internet (ip) Header.
 */
typedef struct in_Header {
#if defined(__CCDL__)
        int    hdrlen : 4;
        int    ver    : 4;
#else
        BYTE   hdrlen : 4;     /* Watcom _requires_ BYTE here */
        BYTE   ver    : 4;
#endif
        BYTE   tos;
        WORD   length;
        WORD   identification;
        WORD   frag_ofs;
        BYTE   ttl;
        BYTE   proto;
        WORD   checksum;
        DWORD  source;
        DWORD  destination;

        /* IP options may come here
         */
      } in_Header;

/** `in_Header.frag_ofs' bits.
 */
#define IP_CE      0x8000     /**< Congestion Experienced */
#define IP_DF      0x4000     /**< Don't Fragment */
#define IP_MF      0x2000     /**< More Fragments */
#define IP_OFFMASK 0x1FFF     /**< Offset mask value */

/** `in_Header.tos' bits.
 */
#define IP_MINCOST     0x02
#define IP_RELIABILITY 0x04
#define IP_THROUGHPUT  0x08
#define IP_LOWDELAY    0x10
#define IP_TOSMASK    (IP_MINCOST|IP_RELIABILITY|IP_THROUGHPUT|IP_LOWDELAY)

#define in_GetHdrLen(ip) ((ip)->hdrlen << 2)  /**< # of bytes in IP-header */

#define IP_BCAST_ADDR    0xFFFFFFFFUL
#define CLASS_A_ADDR     0xFF000000UL
#define CLASS_B_ADDR     0xFFFFFE00UL   /* minimal class B */
#define CLASS_C_ADDR     0xFFFFFF00UL


/*!\struct in6_Header
 *
 * IPv6 header.
 */
typedef struct in6_Header {
        BYTE        pri : 4;
        BYTE        ver : 4;
        BYTE        flow_lbl[3];
        WORD        len;
        BYTE        next_hdr;
        BYTE        hop_limit;
        ip6_address source;
        ip6_address destination;
      } in6_Header;

/*!\struct ip_Packet
 *
 * IPv4 packet including header and data.
 */
typedef struct ip_Packet {
        in_Header head;
        BYTE      data [MAX_IP4_DATA];
      } ip_Packet;


/*!\struct udp_Header
 *
 * The UDP header.
 */
typedef struct udp_Header {
        WORD   srcPort;
        WORD   dstPort;
        WORD   length;
        WORD   checksum;
      } udp_Header;


/*!\struct tcp_Header
 *
 * The TCP header.
 */
typedef struct tcp_Header {
        WORD   srcPort;
        WORD   dstPort;
        DWORD  seqnum;
        DWORD  acknum;

#if defined(__CCDL__)
        int    unused : 4;
        int    offset : 4;
#else
        BYTE   unused : 4;   /* Watcom _requires_ BYTE here */
        BYTE   offset : 4;
#endif
        BYTE   flags;
        WORD   window;
        WORD   checksum;
        WORD   urgent;
      } tcp_Header;

/**
 * tcp_Header::flags bits
 */
#define tcp_FlagFIN   0x01
#define tcp_FlagSYN   0x02
#define tcp_FlagRST   0x04
#define tcp_FlagPUSH  0x08
#define tcp_FlagACK   0x10
#define tcp_FlagURG   0x20
#define tcp_FlagECN   0x40    /* ECN-Echo */
#define tcp_FlagCWR   0x80    /* congestion window reduced */
#define tcp_FlagMASK  0x3F    /* ignore ECN/CWR for now */

/*!\struct tcp_PseudoHeader
 *
 * The TCP/UDP Pseudo Header (IPv4).
 */
typedef struct tcp_PseudoHeader {
        DWORD  src;
        DWORD  dst;
        BYTE   mbz;
        BYTE   protocol;
        WORD   length;
        WORD   checksum;
      } tcp_PseudoHeader;


/*!\struct tcp_PseudoHeader6
 *
 * The TCP/UDP Pseudo Header (IPv6).
 */
typedef struct tcp_PseudoHeader6 {
        ip6_address  src;
        ip6_address  dst;
        WORD         length;
        BYTE         zero[3];
        BYTE         next_hdr;
      } tcp_PseudoHeader6;


/*!\struct arp_Header
 *
 * ARP/RARP header.
 */
typedef struct arp_Header {
        WORD        hwType;
        WORD        protType;
        BYTE        hwAddrLen;     /**< MAC addr. length (6) */
        BYTE        protoAddrLen;  /**< IP addr. length  (4) */
        WORD        opcode;
        eth_address srcEthAddr;
        DWORD       srcIPAddr;
        eth_address dstEthAddr;
        DWORD       dstIPAddr;
      } arp_Header;


#include <sys/pack_off.h>          /**< restore default packing */

W32_CLANG_PACK_WARN_DEF()

#define rarp_Header arp_Header

/**
 * ARP definitions.
 */
#define ARP_REQUEST    0x0100      /**< ARP/RARP op codes, Request. */
#define ARP_REPLY      0x0200      /**<                    Reply. */
#define RARP_REQUEST   0x0300
#define RARP_REPLY     0x0400


/**
 * TCP states, from tcp specification RFC-793.
 *
 * \note CLOSE-WAIT state is bypassed by automatically closing a connection
 *       when a FIN is received.  This is easy to undo.
 *       RESOLVE is a pseudo state before SYN is sent in tcp_Retransmitter().
 */
#define tcp_StateLISTEN   0      /* listening for connection */
#define tcp_StateRESOLVE  1      /* resolving IP, waiting on ARP reply */
#define tcp_StateSYNSENT  2      /* SYN sent, active open */
#define tcp_StateSYNREC   3      /* SYN received, ACK+SYN sent. */
#define tcp_StateESTAB    4      /* established */
#define tcp_StateESTCL    5      /* established, but will FIN */
#define tcp_StateFINWT1   6      /* sent FIN */
#define tcp_StateFINWT2   7      /* sent FIN, received FINACK */
#define tcp_StateCLOSWT   8      /* received FIN waiting for close */
#define tcp_StateCLOSING  9      /* sent FIN, received FIN (waiting for FINACK) */
#define tcp_StateLASTACK  10     /* FIN received, FINACK+FIN sent */
#define tcp_StateTIMEWT   11     /* dally after sending final FINACK */
#define tcp_StateCLOSED   12     /* FIN+ACK received */

#define tcp_MaxBufSize    2048   /* maximum bytes to buffer on input */
#define udp_MaxBufSize    1520
#define tcp_MaxTxBufSize  tcp_MaxBufSize  /* and on tcp output */

/**
 * Fields common to UDP & TCP socket definition.
 *
 * Tries to keep members on natural boundaries (words on word-boundary,
 * dwords on dword boundary)
 */

#define UDP_TCP_COMMON                                                           \
        WORD            ip_type;           /* UDP_PROTO,TCP_PROTO or IPx_TYPE */ \
        BYTE            ttl;               /* Time To Live */                    \
        BYTE            fill_1;                                                  \
        const char     *err_msg;           /* NULL when all is okay */           \
        char            err_buf [100];     /* room for error message */          \
        void (W32_CALL *usr_yield) (void); /* yield while waiting */             \
        icmp_upcall     icmp_callb;        /* socket-layer callback (icmp) */    \
        BYTE            rigid;                                                   \
        BYTE            stress;                                                  \
        WORD            sockmode;          /* a logical OR of bits */            \
        WORD            fill_2;                                                  \
        DWORD           usertimer;         /* ip_timer_set, ip_timer_timeout */  \
        ProtoHandler    protoHandler;      /* called with incoming data */       \
        eth_address     his_ethaddr;       /* peer's ethernet address */         \
                                                                                 \
        DWORD           myaddr;            /* my IPv4-address */                 \
        DWORD           hisaddr;           /* peer's IPv4 address */             \
        WORD            hisport;           /* peer's source port */              \
        WORD            myport;            /* my source port */                  \
        DWORD           locflags;          /* local option flags */              \
        BOOL            is_ip6;            /* TRUE if IPv6 socket */             \
        int             rx_datalen;        /* Rx length, must be signed */       \
        UINT            max_rx_data;       /* Last index for rx_data[] */        \
        BYTE           *rx_data            /* Rx data buffer (default rx_buf[]) */

/*!\struct udp_Socket
 *
 * UDP socket definition.
 */
typedef struct udp_Socket {
        struct udp_Socket *next;
        UDP_TCP_COMMON;
        BYTE rx_buf [udp_MaxBufSize+1]; /**< received data buffer */

#if defined(USE_IPV6)
        ip6_address  my6addr;           /**< my ip6-address */
        ip6_address  his6addr;          /**< peer's ip-6 address */
#endif
        DWORD        safetysig;         /**< magic marker */
      } _udp_Socket;



/*!\struct tcp_Socket
 *
 * TCP Socket definition (fields common to _udp_Socket must come first).
 */
typedef struct tcp_Socket {
        struct tcp_Socket *next;        /**< link to next tcp-socket */
        UDP_TCP_COMMON;

        BYTE rx_buf [tcp_MaxBufSize+1]; /**< received data buffer */

#if defined(USE_IPV6)
        ip6_address  my6addr;           /**< our IPv6 address */
        ip6_address  his6addr;          /**< peer's IPv6 address */
#endif
        UINT         state;             /**< tcp connection state */
        DWORD        recv_next;         /**< SEQ number we expect to receive */
        DWORD        send_next;         /**< SEQ we send but not ACK-ed by peer */
        long         send_una;          /**< unacked send data, must be signed */

#if defined(USE_DEBUG)
        DWORD        last_acknum [2];   /**< for pcdbug.c; to follow SEQ/ACK */
        DWORD        last_seqnum [2];   /**< increments */
#endif
        DWORD        timeout;           /**< timer for retrans etc. */
        BYTE         unhappy;           /**< flag, indicates retransmitting segt's */
        BYTE         recent;            /**< 1 if recently transmitted */
        WORD         flags;             /**< TCP flags used in next Tx */

        UINT         window;            /**< other guy's window */
        UINT         adv_win;           /**< our last advertised window */

        BYTE         cwindow;           /**< Congestion window */
        BYTE         wwindow;           /**< Van Jacobson's algorithm */
        WORD         fill_4;

        DWORD        vj_sa;             /**< VJ's alg, standard average   (SRTT) */
        DWORD        vj_sd;             /**< VJ's alg, standard deviation (RTTVAR) */
        DWORD        vj_last;           /**< last transmit time */
        UINT         rto;               /**< retransmission timeout */
        BYTE         karn_count;        /**< count of packets */
        BYTE         tos;               /**< TOS from IP-header */
        WORD         fill_5;

        DWORD        rtt_time;          /**< Round Trip Time value */
        DWORD        rtt_lasttran;      /**< RTT at last transmission */

        DWORD        ts_sent;           /**< last TimeStamp value sent */
        DWORD        ts_recent;         /**< last TimeStamp value received */
        DWORD        ts_echo;           /**< last TimeStamp echo received */

        UINT         max_seg;           /**< MSS for this connection */

        /** S. Lawson - handle one dropped segment.
         * \todo Make a proper re-assembly queue.
         * missed_seq[0] is left edge of missing segment.
         * missed_seq[1] is right edge (in peer's absolute SEQ space)
         */
        DWORD        missed_seq [2];
     /* void        *reasm_buf; */      /**< linked-list of frags; not yet */

#if defined(USE_TCP_MD5)
        char        *secret;            /**< Secret for MD5 finger-print */
#endif
        DWORD        inactive_to;       /**< inactive timer (no Rx data) */
        DWORD        datatimer;         /**< inactive timer (no Tx data) */
     /* int          sock_delay; ?? */

        BYTE         tx_wscale;         /**< \todo window scales shifts, Tx/Rx */
        BYTE         rx_wscale;
        UINT         tx_queuelen;       /**< optional Tx queue length */
        const BYTE  *tx_queue;

        UINT         tx_datalen;        /**< number of bytes of data to send */
        UINT         max_tx_data;       /**< Last index for tx_data[] */
        BYTE        *tx_data;           /**< Tx data buffer (default tx_buf[]) */
        BYTE         tx_buf [tcp_MaxTxBufSize+1]; /**< data for transmission */
        DWORD        safetysig;         /**< magic marker */
        DWORD        safetytcp;         /**< extra magic marker */
      } _tcp_Socket;


/*!\struct _raw_Socket
 *
 * Raw IPv4 socket definition. Only used in BSD-socket API.
 */
typedef struct _raw_Socket {
        struct _raw_Socket *next;
        WORD   ip_type;                  /**< same ofs as for udp/tcp Socket */
        BOOL   used;                     /**< used flag; packet not read yet */
        DWORD  seq_num;                  /**< counter for finding oldest pkt */
        struct in_Header ip;
        BYTE   rx_data [MAX_FRAG_SIZE];  /**< room for 1 jumbo IP packet */
      } _raw_Socket;


/*!\struct _raw6_Socket
 *
 * Raw IPv6 socket definition. Only used in BSD-socket API.
 */
typedef struct _raw6_Socket {
        struct _raw6_Socket *next;
        WORD   ip_type;
        BOOL   used;
        DWORD  seq_num;
        struct in6_Header ip6;
        BYTE   rx_data [MAX_IP6_DATA];
      } _raw6_Socket;

/*!\union sock_type
 *
 * sock_type used for socket I/O.
 */
typedef union sock_type {
        _udp_Socket  udp;
        _tcp_Socket  tcp;
        _raw_Socket  raw;
        _raw6_Socket raw6;
      } sock_type;

#include "../inc/tcp.h"

#endif /* _w32_WATTCP_H */

