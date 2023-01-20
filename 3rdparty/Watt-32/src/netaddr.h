/*!\file netaddr.h
 */
#ifndef _w32_NETADDR_H
#define _w32_NETADDR_H

extern int mask_len  (DWORD mask);
extern int check_mask (DWORD mask);
extern int check_mask2 (const char *mask);

extern const char *_inet_atoeth (const char *src, eth_address *eth);

extern const char        *_inet6_ntoa (const void *ip);
extern const ip6_address *_inet6_addr (const char *str);

#endif
