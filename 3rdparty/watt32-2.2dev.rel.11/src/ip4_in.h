/*!\file ip4_in.h
 */
#ifndef _w32_IP4_IN_H
#define _w32_IP4_IN_H

/**
 * These are the IPv4 options.
 */
#define IPOPT_EOL       0       /**< end-of-option list */
#define IPOPT_NOP       1       /**< no-operation */
#define IPOPT_RR        7       /**< record packet route */
#define IPOPT_TS        68      /**< timestamp */
#define IPOPT_SECURITY  130     /**< provide s,c,h,tcc */
#define IPOPT_LSRR      131     /**< loose source route */
#define IPOPT_SATID     136     /**< satnet id */
#define IPOPT_SSRR      137     /**< strict source route */
#define IPOPT_RA        148     /**< router alert */

/**
 * Need some macros from <netinet/in.h>
 */
#ifndef IN_MULTICAST
#define IN_MULTICAST(ip)    ((ip & 0xF0000000UL) == 0xE0000000UL)
#endif

#ifndef IN_EXPERIMENTAL
#define IN_EXPERIMENTAL(ip) ((ip & 0xF0000000UL) == 0xF0000000UL)
#endif

extern int _ip4_handler (const in_Header *ip, BOOL broadcast);
extern int _chk_ip4_header (const in_Header *ip);

extern int _ip4_is_local_addr (DWORD ip);
extern int _ip4_is_unique_addr (DWORD ip);
extern int _ip4_is_multihome_addr (DWORD ip);
extern int _ip4_is_ip_brdcast (const in_Header *ip);
extern int _ip4_is_multicast (DWORD ip);
extern int _ip4_is_loopback_addr (DWORD ip);

#endif
