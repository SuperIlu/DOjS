/*!\file split.h
 */
#ifndef _w32_SPLIT_H
#define _w32_SPLIT_H

#if defined(USE_STATISTICS) || defined(USE_DEBUG) || defined(USE_IPV6)
#define NEED_PKT_SPLIT
#endif

enum Components {
     TYPE_TOKEN_HEAD = 1,
     TYPE_FDDI_HEAD,
     TYPE_ETHER_HEAD,
     TYPE_ARCNET_HEAD,
     TYPE_ARP,
     TYPE_RARP,
     TYPE_PPPOE_DISC,
     TYPE_PPPOE_SESS,
     TYPE_PPP_LCP,
     TYPE_PPP_IPCP,
     TYPE_LLC_HEAD,
     TYPE_IP4,
     TYPE_IP4_OPTIONS,
     TYPE_IP4_FRAG,
     TYPE_ICMP,
     TYPE_IGMP,
     TYPE_UDP_HEAD,
     TYPE_UDP_DATA,
     TYPE_TCP_HEAD,
     TYPE_TCP_OPTIONS,
     TYPE_TCP_DATA,
     TYPE_IP6,
     TYPE_IP6_HOP,
     TYPE_IP6_IPV6,
     TYPE_IP6_ROUTING,
     TYPE_IP6_FRAGMENT,
     TYPE_IP6_ESP,
     TYPE_IP6_AUTH,
     TYPE_IP6_ICMP,
     TYPE_IP6_DEST,
     TYPE_IP6_NONE,
     TYPE_IP6_UNSUPP,
     TYPE_MAX
   };

struct pkt_split {
       enum Components type;
       const void     *data;
       unsigned        len;
     };

extern const struct pkt_split *pkt_split_mac_in  (const void *link_pkt);
extern const struct pkt_split *pkt_split_mac_out (const void *link_pkt);
extern const struct pkt_split *pkt_get_split_in  (void);
extern const struct pkt_split *pkt_get_split_out (void);
extern const struct pkt_split *pkt_get_type_in   (enum Components type);
extern const struct pkt_split *pkt_get_type_out  (enum Components type);

extern void pkt_print_split_in  (void);
extern void pkt_print_split_out (void);

#endif
