/*!\file pcarp.h
 * ARP handler module.
 */
#ifndef _w32_PCARP_H
#define _w32_PCARP_H

/*!\struct gate_entry
 * Gateway table.
 * \todo Move the gateway/route stuff to route.c
 */
struct gate_entry {
       DWORD  gate_ip;       /**< The IP address of gateway */
       DWORD  subnet;        /**< The subnet mask of gateway */
       DWORD  mask;          /**< The netmask fgor the above entry */
       BOOL   is_dead;       /* Dead Gate detection: */
       BOOL   echo_pending;  /*   echo-reply pending */
       DWORD  chk_timer;     /*   check interval timer */
     };

/*!\struct arp_entry
 * ARP cache table.
 */
struct arp_entry {
       DWORD       ip;            /**< IP address of ARP entry */
       eth_address hardware;      /**< MAC address of ARP entry */
       DWORD       expiry;        /**< 'pending timeout' or 'dynamic expiry' */
       DWORD       retransmit_to; /**< used only for re-requesting MAC while 'pending' */
       WORD        flags;         /**< ARP flags, see below */
     };


/** arp_entry::flags definitions.
 */
#define ARP_FLG_INUSE   0x01      /**< Entry is in use */
#define ARP_FLG_FIXED   0x02      /**< Entry is fixed (never times out) */
#define ARP_FLG_DYNAMIC 0x04      /**< Entry is dynamic (will timeout) */
#define ARP_FLG_PENDING 0x08      /**< Entry is pending a lookup */

/*!\struct route_entry
 * Routing table.
 * \todo: Move to route.c
 */
struct route_entry  {
       DWORD  host_ip;  /* when connection to this host ... */
       DWORD  gate_ip;  /* ... we use this gateway */
       DWORD  mask;     /* mask used to calculate this route */
       DWORD  flags;    /* not used yet */
     };

/** route_entry::flags definitions.
 */
#define ROUTE_FLG_USED     0x01
#define ROUTE_FLG_PENDING  0x02
#define ROUTE_FLG_DYNAMIC  0x04
#define ROUTE_FLG_FIXED    0x08


/** Gateway functions.
 * \todo: Move to route.c and prefix with '_route'.
 *
 * \note: Exported only because ./bin/tcpinfo.c uses some of those.
 *        Therefore all functions use 'W32_CALL' convention too.
 */
W32_FUNC int  W32_CALL _arp_gateways_get (const struct gate_entry **);
W32_FUNC int  W32_CALL _arp_check_gateways (void);
W32_FUNC BOOL W32_CALL _arp_add_gateway  (const char* config_string, DWORD ip);
W32_FUNC BOOL W32_CALL _arp_have_default_gw (void);
W32_FUNC void W32_CALL _arp_kill_gateways (void);

/** Blocking ARP resolve functions.
 *
 * Doesn't return until success or time-out.
 */
W32_FUNC BOOL W32_CALL _arp_resolve      (DWORD ina, eth_address *eth);
W32_FUNC BOOL W32_CALL _arp_check_own_ip (eth_address *eth);

/** New non-blocking functions, GvB 2002-09.
 */
W32_FUNC BOOL W32_CALL _arp_start_lookup   (DWORD ip);
W32_FUNC BOOL W32_CALL _arp_lookup         (DWORD ip, eth_address *eth);
W32_FUNC BOOL W32_CALL _arp_lookup_fixed   (DWORD ip, eth_address *eth);
W32_FUNC BOOL W32_CALL _arp_lookup_pending (DWORD ip);

/** ARP cache functions.
 */
W32_FUNC BOOL W32_CALL _arp_cache_add (DWORD ip, const void *eth, BOOL expire);
W32_FUNC BOOL W32_CALL _arp_cache_del (DWORD ip);
W32_FUNC int  W32_CALL _arp_cache_get (const struct arp_entry **);
W32_FUNC void W32_CALL _arp_cache_dump (void);
W32_FUNC void W32_CALL _arp_gateways_dump (void);
W32_FUNC void W32_CALL _arp_routes_dump (void);
W32_FUNC void W32_CALL _arp_debug_dump (void);

/** 'Internal' interface to pctcp.c & others.
 */
W32_FUNC void W32_CALL _arp_init (void);
W32_FUNC BOOL W32_CALL _arp_reply  (const void *mac_dst, DWORD src_ip, DWORD dst_ip);
W32_FUNC BOOL W32_CALL _arp_handler (const arp_Header *arp, BOOL brdcast);

/** ICMP redirection.
 */
W32_FUNC BOOL W32_CALL _arp_register (DWORD use_this_gateway_ip, DWORD for_this_host_ip);

#endif
