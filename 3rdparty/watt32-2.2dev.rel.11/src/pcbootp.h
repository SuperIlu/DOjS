/*!\file pcbootp.h
 *
 * Bootstrap Protocol (BOOTP).
 * Based on RFC 951.
 */

#ifndef _w32_PCBOOTP_H
#define _w32_PCBOOTP_H

extern DWORD _bootp_host;
extern int   _bootp_timeout;

extern int BOOTP_do_boot (void);

#define BOOTP_REQUEST  1    /**< bootp::bp_op */
#define BOOTP_REPLY    2

/*
 * BOOTP options and values.
 */
enum BOOTP_Options {
     BOOTP_OPT_PAD                      =  0,
     BOOTP_OPT_SUBNET_MASK              =  1,
     BOOTP_OPT_TIME_OFFSET              =  2,
     BOOTP_OPT_ROUTERS_ON_SNET          =  3,
     BOOTP_OPT_TIME_SRV                 =  4,
     BOOTP_OPT_NAME_SRV                 =  5,
     BOOTP_OPT_DNS_SRV                  =  6,
     BOOTP_OPT_LOG_SRV                  =  7,
     BOOTP_OPT_COOKIE_SRV               =  8,
     BOOTP_OPT_LPR_SRV                  =  9,
     BOOTP_OPT_IMPRESS_SRV              = 10,
     BOOTP_OPT_RES_LOCATION_SRV         = 11,
     BOOTP_OPT_HOST_NAME                = 12,
     BOOTP_OPT_BOOT_FSIZE               = 13,
     BOOTP_OPT_DOMAIN_NAME              = 15,
     BOOTP_OPT_NON_LOCAL_SRC_ROUTE      = 20,
     BOOTP_OPT_POLICY_FILTER            = 21,
     BOOTP_OPT_MAX_DGRAM_REASM_SIZE     = 22,
     BOOTP_OPT_IP_DEFAULT_TTL           = 23,
     BOOTP_OPT_PATH_MTU_AGING_TIMEOUT   = 24,
     BOOTP_OPT_PATH_MTU_PLATEAU_TABLE   = 25,
     BOOTP_OPT_IF_MTU                   = 26,
     BOOTP_OPT_ALL_SUBNETS_LOCAL        = 27,
     BOOTP_OPT_BROADCAST_ADDR           = 28,
     BOOTP_OPT_PERFORM_MASK_DISCOVERY   = 29,
     BOOTP_OPT_MASK_SUPPLIER            = 30,
     BOOTP_OPT_PERFORM_ROUTER_DISCOVERY = 31,
     BOOTP_OPT_ROUTER_SOLICITATION_ADDR = 32,
     BOOTP_OPT_STATIC_ROUTE             = 33,
     BOOTP_OPT_TRAILER_ENCAPSULATION    = 34,
     BOOTP_OPT_ARP_CACHE_TIMEOUT        = 35,
     BOOTP_OPT_ETHERNET_ENCAPSULATION   = 36,
     BOOTP_OPT_END                      = 255,
   };

W32_CLANG_PACK_WARN_OFF()

#include <sys/pack_on.h>

/*!\struct bootp
 *
 * Structure for send and receive.
 */
struct bootp {
       BYTE   bp_op;         /* packet op code / message type.            */
       BYTE   bp_htype;      /* hardware address type, 1 = 10 mb ethernet */
       BYTE   bp_hlen;       /* hardware address len, eg '6' for 10mb eth */
       BYTE   bp_hops;       /* client sets to zero, optionally used by   */
                             /* gateways in cross-gateway booting.        */
       DWORD  bp_xid;        /* transaction ID, a random number           */
       WORD   bp_secs;       /* filled in by client, seconds elapsed      */
                             /* since client started trying to boot.      */
       WORD   bp_spare;
       DWORD  bp_ciaddr;     /* client IP address filled by client if known */
       DWORD  bp_yiaddr;     /* 'your' (client) IP address                */
                             /* filled by server if client doesn't know   */
       DWORD  bp_siaddr;     /* server IP address returned in bootreply   */
       DWORD  bp_giaddr;     /* gateway IP address,                       */
                             /* used in optional cross-gateway booting.   */
       BYTE   bp_chaddr[16]; /* client hardware address, filled by client */
       BYTE   bp_sname[64];  /* optional server host name, null terminated*/

       BYTE   bp_file[128];  /* boot file name, null terminated string    */
                             /* 'generic' name or null in bootrequest,    */
                             /* fully qualified directory-path            */
                             /* name in bootreply.                        */
       BYTE   bp_vend[64];   /* optional vendor-specific area             */
     };

#define BOOTP_MIN_SIZE  44

/*
 * UDP port numbers, server and client.
 */
#define IPPORT_BOOTPS   67
#define IPPORT_BOOTPC   68


/******** the following is stolen from NCSA which came from CUTCP *********/
/* I have not implemented these, but someone may wish to in the future so */
/* I kept them around.                                                    */
/**************************************************************************/

/*!\struct bootp_vend
 *
 * "vendor" data permitted for Stanford boot clients.
 */
struct bootp_vend {
       BYTE  v_magic[4];    /* magic number */
       DWORD v_flags;       /* flags/opcodes, etc. */
       BYTE  v_unused[56];  /* currently unused */
     };

#include <sys/pack_off.h>

W32_CLANG_PACK_WARN_DEF()

#define VM_STANFORD  0x5354414EUL  /* v_magic for Stanford ("STAN") */
#define VM_RFC1048   0x63825363UL  /* v_magic for RFC client/servers */

/*
 * 'v_flags' values.
 */
#define VF_PCBOOT          1   /* an IBMPC or Mac wants environment info */
#define VF_HELP            2   /* help me, I'm not registered */
#define TAG_BOOTFILE_SIZE  13  /* tag used by bootp_vend fields, RFC 1048 */

#endif
