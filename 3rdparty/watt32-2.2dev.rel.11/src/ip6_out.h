/*!\file ip6_out.h
 */
#ifndef _w32_IP6_OUTPUT_H
#define _w32_IP6_OUTPUT_H

#ifndef __NETINET_IN_H
#include <netinet/in.h> /* struct in6_addr */
#endif

#if defined(USE_IPV6)

W32_DATA struct in6_addr in6addr_my_ip;

extern const struct in6_addr in6addr_all_1;
extern const struct in6_addr in6addr_alln_mc;
extern const struct in6_addr in6addr_allr_mc;
extern const BYTE            in6addr_mapped[12];

extern int _default6_ttl;

extern int _ip6_output (in6_Header *ip, ip6_address *src_ip,
                        ip6_address *dst_ip, BYTE next_hdr,
                        unsigned data_len, int hop_lim, const void *sock,
                        const char *file, unsigned line);

#define IP6_OUTPUT(ip, src, dst, next_hdr, data_len, hop_lim, sock) \
       _ip6_output(ip, src, dst, next_hdr, data_len, hop_lim, sock, \
                   __FILE__, __LINE__)

#endif  /* USE_IPV6 */
#endif

