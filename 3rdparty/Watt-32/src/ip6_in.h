/*!\file ip6_in.h
 */
#ifndef _w32_IP6_IN_H
#define _w32_IP6_IN_H

/*
 * `struct in6_Header::next_hdr' field.
 */
#define IP6_NEXT_HOP       0       /* Hop-by-hop option header */
#define IP6_NEXT_TCP       6       /* TCP segment */
#define IP6_NEXT_UDP       17      /* UDP message */
#define IP6_NEXT_IPV6      41      /* IPv6 in IPv6 */
#define IP6_NEXT_ROUTING   43      /* Routing header */
#define IP6_NEXT_FRAGMENT  44      /* Fragmentation/reassembly header */
#define IP6_NEXT_ESP       50      /* Encapsulating security payload */
#define IP6_NEXT_AUTH      51      /* Authentication header */
#define IP6_NEXT_ICMP      58      /* ICMP for IPv6 */
#define IP6_NEXT_NONE      59      /* No next header */
#define IP6_NEXT_DEST      60      /* Destination options header */
#define IP6_NEXT_COMP      108     /* Compression options header */
#define IP6_NEXT_SCTP      132     /* Stream Control Transfer Protocol */

typedef struct ip6_RouteHdr {
               BYTE  next_hdr;
               BYTE  hdrlen;
               BYTE  type;
               BYTE  seg_left;
             } ip6_RouteHdr;

#if defined(USE_IPV6)
  extern int  _ip6_handler (const in6_Header *ip, BOOL broadcast);
  extern int  _ip6_init (void);
  extern int  _ip6_pkt_init (void);
  extern void _ip6_post_init (void);
  extern int  _ip6_is_local_addr (const void *ip);
#endif  /* USE_IPV6 */
#endif
