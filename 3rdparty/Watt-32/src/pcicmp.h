/*!\file pcicmp.h
 */
#ifndef _w32_PCICMP_H
#define _w32_PCICMP_H

W32_CLANG_PACK_WARN_OFF()

#include <sys/pack_on.h>

/* When passing thise file throght swig:
 *   Warning 314: 'in' is a python keyword, renaming to '_in'
 *
 * Triggered by the below "in_Header in;"
 */
#ifdef SWIG
#define in _in
#endif

/*!\struct ICMP_unused
*   For ICMP_TIMXCEED etc.
 */
struct ICMP_unused {
       BYTE      type;
       BYTE      code;
       WORD      checksum;
       DWORD     unused;
       in_Header ip;
       BYTE      spares [8];
     };

/*!\struct ICMP_pointer
 */
struct ICMP_pointer {
       BYTE      type;
       BYTE      code;
       WORD      checksum;
       BYTE      pointer;
       BYTE      unused [3];
       in_Header ip;
     };

/*!\struct ICMP_ip.
 * For ICMP_UNREACH, ICMP_PARAMPROB etc.
 */
struct ICMP_ip {
       BYTE      type;
       BYTE      code;
       WORD      checksum;
       DWORD     ipaddr;         /* redirect gateway */
       in_Header ip;             /* original ip-header */
     };

/*!\struct ICMP_echo
 */
struct ICMP_echo {
       BYTE      type;
       BYTE      code;
       WORD      checksum;
       WORD      identifier;
       WORD      sequence;
       DWORD     index;
     };

/*!\struct ICMP_timestamp
 */
struct ICMP_timestamp {
       BYTE      type;
       BYTE      code;
       WORD      checksum;
       WORD      identifier;
       WORD      sequence;
       DWORD     original;       /* original timestamp */
       DWORD     receive;        /* receive timestamp  */
       DWORD     transmit;       /* transmit timestamp */
     };

/*!\struct ICMP_info
 */
struct ICMP_info {
       BYTE      type;
       BYTE      code;
       WORD      checksum;
       WORD      identifier;
       WORD      sequence;
     };

/*!\struct ICMP_addr_mask
 */
struct ICMP_addr_mask {
       BYTE      type;
       BYTE      code;
       WORD      checksum;
       WORD      identifier;
       WORD      sequence;
       DWORD     mask;
     };

/*!\struct ICMP_traceroute
 */
struct ICMP_traceroute {
       BYTE      type;
       BYTE      code;
       WORD      checksum;
       WORD      identifier;
       WORD      unused;
       WORD      outbound_hop;
       WORD      return_hop;
       DWORD     link_speed;
       DWORD     link_mtu;
     };


/*!\struct ICMP_needfrag
 * RFC-1191 (for type ICMP_UNREACH, code ICMP_UNREACH_NEEDFRAG)
 */
struct ICMP_needfrag {
       BYTE      type;
       BYTE      code;
       WORD      checksum;
       WORD      unused;   /* 32-bit 0 here for router that doesn't support RFC-1191 */
       WORD      next_mtu;
     };

/*!\union ICMP_pkt
 */
typedef union ICMP_PKT {
        struct ICMP_unused     unused;
        struct ICMP_pointer    pointer;
        struct ICMP_ip         ip;
        struct ICMP_echo       echo;
        struct ICMP_timestamp  timestamp;
        struct ICMP_info       info;
        struct ICMP_addr_mask  mask;
        struct ICMP_traceroute tracert;
        struct ICMP_needfrag   needfrag;
      } ICMP_PKT;

/*!\struct ping_pkt
 * The ping packet.
 */
struct ping_pkt {
       in_Header        in;
       struct ICMP_echo icmp;
    /* BYTE             data[]; */
     };

#include <sys/pack_off.h>

W32_CLANG_PACK_WARN_DEF()

/**
 * These are the ICMP messages. Ref. <netinet/ip_icmp.h>.
 */
#define ICMP_ECHOREPLY     0     /**< echo reply */
#define ICMP_UNREACH       3     /**< dest unreachable, see codes below */
#define ICMP_SOURCEQUENCH  4     /**< packet lost, slow down */
#define ICMP_REDIRECT      5     /**< shorter route */
#define ICMP_ECHO          8     /**< echo service */
#define ICMP_ROUTERADVERT  9     /**< router advertisement */
#define ICMP_ROUTERSOLICIT 10    /**< router solicitation */
#define ICMP_TIMXCEED      11    /**< time exceeded */
#define ICMP_PARAMPROB     12    /**< ip header bad */
#define ICMP_TSTAMP        13    /**< timestamp request */
#define ICMP_TSTAMPREPLY   14    /**< timestamp reply */
#define ICMP_IREQ          15    /**< information request */
#define ICMP_IREQREPLY     16    /**< information reply */
#define ICMP_MASKREQ       17    /**< address mask request */
#define ICMP_MASKREPLY     18    /**< address mask reply */

#ifndef ICMP_MAXTYPE
#define ICMP_MAXTYPE 18
#endif


/*
 * ICMP_UNREACH codes. E.g. 'struct ICMP_unused::code'.
 */
#define ICMP_UNREACH_NET                0
#define ICMP_UNREACH_HOST               1
#define ICMP_UNREACH_PROTOCOL           2
#define ICMP_UNREACH_PORT               3
#define ICMP_UNREACH_NEEDFRAG           4
#define ICMP_UNREACH_SRCFAIL            5
#define ICMP_UNREACH_NET_UNKNOWN        6
#define ICMP_UNREACH_HOST_UNKNOWN       7
#define ICMP_UNREACH_ISOLATED           8
#define ICMP_UNREACH_NET_PROHIB         9
#define ICMP_UNREACH_HOST_PROHIB        10
#define ICMP_UNREACH_TOSNET             11
#define ICMP_UNREACH_TOSHOST            12
#define ICMP_UNREACH_FILTER_PROHIB      13
#define ICMP_UNREACH_HOST_PRECEDENCE    14
#define ICMP_UNREACH_PRECEDENCE_CUTOFF  15

extern const char *icmp_type_str [ICMP_MAXTYPE+1];
extern const char *icmp_unreach_str [16];
extern const char *icmp_redirect_str [4];
extern const char *icmp_exceed_str [2];

extern void icmp_handler (const in_Header *ip, BOOL broadcast);
extern void icmp_doredirect (const char *value);
extern int  icmp_send_timexceed (const in_Header *ip, const void *mac_dest);
extern int  icmp_send_unreach (const in_Header *ip, int code);
extern int  icmp_send_mask_req (void);

#endif
