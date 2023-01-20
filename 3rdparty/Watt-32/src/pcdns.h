/*!\file pcdns.h
 */
#ifndef _w32_UDP_DOM_H
#define _w32_UDP_DOM_H

#define MAX_LABEL_SIZE    63  /* maximum length of a single domain label  */
#define DOMSIZE          512  /* maximum domain message size to mess with */
#define DOM_DST_PORT      53  /* destination port number for DNS protocol */
#define DOM_SRC_PORT    1415  /* local port number for DNS protocol. Old  */
                              /* port 997 didn't work with some firewalls */

W32_CLANG_PACK_WARN_OFF()

#include <sys/pack_on.h>

/*
 * Header format (struct DNS_head), from RFC 1035:
 *                                  1  1  1  1  1  1
 *    0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                      ID                       |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                    QDCOUNT                    |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                    ANCOUNT                    |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                    NSCOUNT                    |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                    ARCOUNT                    |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *
 * AA, TC, RA, and RCODE are only set in responses.
 * Brief description of the remaining fields:
 *      ID      Identifier to match responses with queries
 *      QR      Query (0) or response (1)
 *      Opcode  For our purposes, always QUERY
 *      RD      Recursion desired
 *      Z       Reserved (zero)
 *      QDCOUNT Number of queries
 *      ANCOUNT Number of answers
 *      NSCOUNT Number of name server records
 *      ARCOUNT Number of additional records
 *
 * Question format, from RFC 1035:
 *                                  1  1  1  1  1  1
 *    0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                                               |
 *  /                     QNAME                     /
 *  /                                               /
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                     QTYPE                     |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                     QCLASS                    |
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *
 * The QNAME is encoded as a series of labels, each represented
 * as a one-byte length (maximum 63) followed by the text of the
 * label. The list is terminated by a label of length zero (which can
 * be thought of as the root domain).
 */

/*
 * Header for the DOMAIN queries.
 * ALL OF THESE ARE BYTE SWAPPED QUANTITIES!
 * We are the poor slobs who are incompatible with the world's byte order.
 */
struct DNS_head {
       WORD  ident;           /* Unique identifier */
#if 1
       WORD  flags;           /* See below */
#else
       unsigned rd     : 1;   /* Recursion desired */
       unsigned tc     : 1;   /* Truncated message */
       unsigned aa     : 1;   /* Authoritative answer */
       unsigned opcode : 4;   /* See below */
       unsigned qr     : 1;   /* Response = 1, Query = 0 */
       unsigned rcode  : 4;   /* Response code */
       unsigned cd     : 1;   /* Checking disabled by resolver */
       unsigned ad     : 1;   /* Authentic data from named */
       unsigned unused : 1;   /* Unused bits */
       unsigned ra     : 1;   /* Recursion available */
#endif
       WORD  qdcount;         /* Question section, # of entries */
       WORD  ancount;         /* Answers, how many              */
       WORD  nscount;         /* Count of name server RRs       */
       WORD  arcount;         /* Number of "additional" records */
     };


/** Another definition of 'struct DNS_head'.
 * \todo Rewrite to use only 'struct DNS_Header'.
 */
struct DNS_Header {
       WORD  dns_id;
       WORD  dns_fl_rd    : 1;  /* recursion desired */
       WORD  dns_fl_tc    : 1;  /* truncated message */
       WORD  dns_fl_aa    : 1;  /* authoritative answer */
       WORD  dns_fl_opcode: 4;  /* purpose of message */
       WORD  dns_fl_qr    : 1;  /* response flag */
       WORD  dns_fl_rcode : 4;  /* response code */
       WORD  dns_fl_cd    : 1;  /* checking disabled by resolver */
       WORD  dns_fl_ad    : 1;  /* authentic data from named */
       WORD  dns_fl_unused: 1;  /* unused bits (MBZ as of 4.9.3a3) */
       WORD  dns_fl_ra    : 1;  /* recursion available */
       WORD  dns_num_q;
       WORD  dns_num_ans;
       WORD  dns_num_auth;
       WORD  dns_num_add;
     };


/*
 * DNS_head::flags
 */
#define DQR       0x8000     /* Query = 0, Response = 1 */
#define DOPCODE   0x7100     /* Opcode mask, see below  */
#define DAA       0x0400     /* Authoritative answer    */
#define DTC       0x0200     /* Truncated response      */
#define DRD       0x0100     /* Recursion desired       */
#define DRA       0x0080     /* Recursion available     */
#define DRCODE    0x000F     /* Response code mask, see below */

/*
 * Opcode possible values:
 */
#define DOPQUERY  0          /* a standard query */
#define DOPIQ     1          /* an inverse query */
#define DOPCQM    2          /* a completion query, multiple reply */
#define DOPCQU    3          /* a completion query, single reply   */
#define DUPDATE   5          /* DNS update (RFC-2136) */
/* the rest reserved for future */

/*
 * DNS server response codes; RCODE (stored in dom_errno)
 */
enum DNS_serv_resp {
     DNS_SRV_OK = 0,         /* 0 = okay response */
     DNS_SRV_FORM,           /* 1 = format error  */
     DNS_SRV_FAIL,           /* 2 = server failed */
     DNS_SRV_NAME,           /* 3 = name error (NXDOMAIN) */
     DNS_SRV_NOTIMPL,        /* 4 = function not implemented */
     DNS_SRV_REFUSE,         /* 5 = service refused */
     DNS_SRV_MAX = 15
   };

/*
 * DNS client codes (stored in dom_errno)
 */
enum DNS_client_code {
     DNS_CLI_SYSTEM = DNS_SRV_MAX,  /* See 'errno' */
     DNS_CLI_REFUSE,                /* Name server refused */
     DNS_CLI_USERQUIT,              /* User terminated via hook or ^C */
     DNS_CLI_NOSERV,                /* No nameserver defined */
     DNS_CLI_TIMEOUT,               /* Timeout, no reply */
     DNS_CLI_ILL_RESP,              /* Illegal/short response */
     DNS_CLI_ILL_IDNA,              /* Convert to/from international name failed */
     DNS_CLI_TOOBIG,                /* Name/label too large */
     DNS_CLI_NOIP,                  /* my_ip_addr == 0 */
     DNS_CLI_NOIPV6,                /* IPv6 DNS disabled */
     DNS_CLI_OTHER,                 /* Other general error */
     DNS_CLI_MAX
  };

/*
 * Query types (QTYPE)
 */
#define DTYPE_A     1         /* host address resource record (A) */
#define DTYPE_CNAME 5         /* host's canonical name record (CNAME) */
#define DTYPE_AAAA  28        /* host address resource record (AAAA) */
#define DTYPE_PTR   12        /* domain name ptr (PTR) */
#define DTYPE_SOA   6         /* start of authority zone (SOA) */
#define DTYPE_SRV   33        /* server selection (SRV), RFC-2052 */
#define DTYPE_CAA  257        /* Certification Authority Authorization, RFC-6844 */

/*
 * Query class (QCLASS)
 */
#define DIN         1         /* ARPA internet class */
#define DWILD       255       /* Wildcard for several classifications */

/*
 * DNS Query request/response header.
 */
struct DNS_query {
       struct DNS_head head;
       BYTE   body [DOMSIZE];
     };

struct DNS_Query {
       struct DNS_Header head;
       BYTE   body [DOMSIZE];
     };

/*
 * A resource record is made up of a compressed domain name followed by
 * this structure.  All of these words need to be byteswapped before use.
 * Part of 'struct DNS_query::body'.
 */
struct DNS_resource {
       WORD   rtype;              /* resource record type = DTYPE_A/AAAA */
       WORD   rclass;             /* RR class = DIN                      */
       DWORD  ttl;                /* time-to-live, changed to 32 bits    */
       WORD   rdlength;           /* length of next field                */
       BYTE   rdata [DOMSIZE-10]; /* data field                          */
     };

#define RESOURCE_HEAD_SIZE  10

#include <sys/pack_off.h>

W32_CLANG_PACK_WARN_DEF()

extern char  dom_cname [MAX_HOSTLEN+1];
extern DWORD dom_ttl;
extern BOOL  dns_do_ipv6;
extern BOOL  dns_do_idna;
extern WORD  dns_windns;
extern BOOL  called_from_resolve;
extern BOOL  called_from_ghbn;
extern BOOL  from_windns;

extern DWORD       dom_a4list [MAX_ADDRESSES+1];
extern ip6_address dom_a6list [MAX_ADDRESSES+1];

/* In udp_rev.c
 */
extern int reverse_lookup_myip (void);
extern int reverse_resolve_ip4 (DWORD ipv4, char *result, size_t size);

#if defined(USE_IPV6)
extern int reverse_resolve_ip6 (const void *ipv6, char *result, size_t size);
#endif

#endif
