/*!\file pcicmp6.h
 */
#ifndef _w32_PCICMP6_H
#define _w32_PCICMP6_H

W32_CLANG_PACK_WARN_OFF()

#include <sys/pack_on.h>

/*!\struct ICMP6_unused
 */
struct ICMP6_unused {
       BYTE        type;
       BYTE        code;
       WORD        checksum;
       DWORD       unused;
     };

/*!\struct ICMP6_route_sol
 */
struct ICMP6_route_sol {
       BYTE        type;
       BYTE        code;
       WORD        checksum;
       DWORD       reserved;
     };

/*!\struct ICMP6_route_adv
 */
struct ICMP6_route_adv {
       BYTE        type;
       BYTE        code;
       WORD        checksum;
       BYTE        hop_limit;
       BYTE        reserved : 6;
       BYTE        other    : 1;
       BYTE        managed  : 1;
       WORD        lifetime;
       DWORD       reach_time;
       DWORD       retrans_time;
     };

/*!\struct ICMP6_pointer
 */
struct ICMP6_pointer {
       BYTE        type;
       BYTE        code;
       WORD        checksum;
       DWORD       pointer;
     };

/*!\struct ICMP6_MTU
 */
struct ICMP6_MTU {
       BYTE        type;
       BYTE        code;
       WORD        checksum;
       DWORD       MTU;
     };

/*!\struct ICMP6_echo
 */
struct ICMP6_echo {
       BYTE        type;
       BYTE        code;
       WORD        checksum;
       WORD        identifier;
       WORD        sequence;
     };

/*!\struct ICMP6_nd_adv
 */
struct ICMP6_nd_adv {
       BYTE        type;
       BYTE        code;
       WORD        checksum;
       BYTE        reserved1 : 5;
       BYTE        override  : 1;
       BYTE        solicited : 1;
       BYTE        router    : 1;
       BYTE        reserved2;
       WORD        reserved3;
       ip6_address target;
     };

/*!\struct ICMP6_nd_ra
 */
struct ICMP6_nd_ra {
       BYTE        type;
       BYTE        code;
       WORD        checksum;
       BYTE        hop_limit;
       BYTE        reserved : 6;
       BYTE        other    : 1;
       BYTE        managed  : 1;
       WORD        lifetime;
     };

/*!\struct ICMP6_nd_sol
 */
struct ICMP6_nd_sol {
       BYTE        type;
       BYTE        code;
       WORD        checksum;
       DWORD       reserved;
       ip6_address target;
     };

/*!\union ICMP6_pkt
 */
typedef union ICMP6_PKT {
        struct ICMP6_unused    unused;
        struct ICMP6_route_sol rsolic;
        struct ICMP6_route_adv radvert;
        struct ICMP6_pointer   pointer;
        struct ICMP6_MTU       MTU;
        struct ICMP6_echo      echo;
        struct ICMP6_nd_adv    nd_adv;
        struct ICMP6_nd_ra     nd_ra;
        struct ICMP6_nd_sol    nd_solic;
      } ICMP6_PKT;

#include <sys/pack_off.h>

W32_CLANG_PACK_WARN_OFF()

enum ND_cacheState {
     ND_CACHE_UNUSED,      /* entry not used */
     ND_CACHE_INCOMPLETE,  /* lookup incomplete */
     ND_CACHE_REACHABLE,   /* responds to ND solitations */
     ND_CACHE_PROBE,       /* ?? */
     ND_CACHE_MAX
   };

struct icmp6_cache {
       ip6_address        ip;
       eth_address        eth;
       WORD               flags;
       enum ND_cacheState state;
       time_t             expiry;
     };

#define ND_CACHE_SIZE 10

extern DWORD icmp6_6to4_gateway;

extern void icmp6_handler (const in6_Header *ip);
extern void icmp6_unreach (const in6_Header *ip, int code);

extern int  icmp6_neigh_solic (const void *addr, eth_address *eth);
extern int  icmp6_neighbor_advert (const eth_address *eth);
extern int  icmp6_router_solicitation (void);
extern BOOL icmp6_add_gateway4 (void);

struct icmp6_cache *icmp6_ncache_lookup (const void *ip);
struct icmp6_cache *icmp6_ncache_insert (const void *ip, const void *eth);
struct icmp6_cache *icmp6_ncache_insert_fix (const void *ip, const void *eth);

#endif

