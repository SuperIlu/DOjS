/*!\file inc/tcp.h
 * Watt-32 public API.
 */

/*
 * Waterloo TCP
 *
 * Copyright (c) 1990-1993 Erick Engelke
 *
 * Portions copyright others, see copyright.h for details.
 *
 * This library is free software; you can use it or redistribute under
 * the terms of the license included in LICENSE.H.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * file LICENSE.H for more details.
 *
 */

#ifndef __WATT_TCP_H
#define __WATT_TCP_H

#define WATT32_NO_OLDIES

/*
 * Version (major.minor.dev-rel), 8-bit each.
 */
#define WATTCP_MAJOR_VER  2
#define WATTCP_MINOR_VER  2
#define WATTCP_DEVEL_REL  11

#define WATTCP_VER  ((WATTCP_MAJOR_VER << 16) + \
                     (WATTCP_MINOR_VER << 8) +  \
                     WATTCP_DEVEL_REL)

#define WATTCP_VER_STRING  "2.2.11"

#if !defined(RC_INVOKED) /* rest of file */

#include <stdio.h>
#include <sys/w32api.h>  /* W32_FUNC, W32_DATA etc. */
#include <sys/wtypes.h>  /* fd_set, iovec */
#include <sys/wtime.h>   /* <time.h>, struct timeval */
#include <sys/whide.h>   /* hide publics inside W32_NAMESPACE() */
#include <sys/cdefs.h>   /* MS_CDECL etc. */
#include <sys/swap.h>    /* intel(), intel16() */

#ifdef __WATCOMC__
#pragma read_only_directory;
#endif

#ifdef __cplusplus
extern "C" {
#endif

W32_DATA const char *wattcpCopyright;                    /* "See COPYRIGHT.H for details" */

W32_FUNC const char *W32_CALL wattcpVersion (void);      /* Watt-32 target version/date */
W32_FUNC const char *W32_CALL wattcpCapabilities (void); /* what features was been compiled in */
W32_FUNC const char *W32_CALL wattcpBuildCC (void);      /* what is the compiler __VENDOR__ nane */
W32_FUNC const char *W32_CALL wattcpBuildCCexe (void);   /* what is the compiler name */
W32_FUNC const char *W32_CALL wattcpBuildCflags (void);  /* what CFLAGS were used */

#if !defined(WATT32_BUILD)
 /*
  * Hide the details of these structures when using Watt-32.
  * In 64-bit mode, these structures are a bit higher.
  */
  #if defined(_M_X64) || defined(_M_IA64) || defined(_M_AMD64) || defined(__x86_64__)
    #define W32_UNDOC_TCP_SOCKET_SIZE  4520
    #define W32_UNDOC_UDP_SOCKET_SIZE  1780
  #else
    #define W32_UNDOC_TCP_SOCKET_SIZE  4470
    #define W32_UNDOC_UDP_SOCKET_SIZE  4470
  #endif

  typedef void sock_type;
  typedef void in_Header;
  typedef void udp_Header;
  typedef void tcp_Header;

  typedef struct {
          BYTE   undoc [W32_UNDOC_TCP_SOCKET_SIZE];
        } tcp_Socket;

  typedef struct {
          BYTE   undoc [W32_UNDOC_UDP_SOCKET_SIZE];
        } udp_Socket;

#else

  #if !defined(_w32_WATTCP_H)
    #error "<wattcp.h> MUST be included before <tcp.h>"
  #endif

  #define tcp_Socket  struct tcp_Socket  /* in wattcp.h */
  #define udp_Socket  struct udp_Socket  /* in wattcp.h */
  #define sock_type   union  sock_type   /* in wattcp.h */

#endif /* WATT32_BUILD */

#define MAX_COOKIES      10
#define MAX_NAMESERVERS  10
#define MAX_HOSTLEN      80

/* Modes for sock_mode()
 */
#define TCP_MODE_BINARY  0x01   /* deprecated */
#define TCP_MODE_ASCII   0x02   /* deprecated */
#define SOCK_MODE_BINARY 0x01   /* new name */
#define SOCK_MODE_ASCII  0x02   /* new name */
#define UDP_MODE_CHK     0x04   /* defaults to checksum */
#define UDP_MODE_NOCHK   0x08
#define TCP_MODE_NAGLE   0x10   /* Nagle's algorithm */
#define TCP_MODE_NONAGLE 0x20

/* wait-states for sock_sselect()
 */
#define SOCKESTABLISHED  1
#define SOCKDATAREADY    2
#define SOCKCLOSED       4

#undef  sock_init
#define sock_init()  watt_sock_init (sizeof(tcp_Socket), sizeof(udp_Socket), sizeof(time_t))

W32_FUNC int         W32_CALL watt_sock_init (size_t tcp_Sock_size,
                                              size_t udp_Sock_size,
                                              size_t time_t_size);

W32_FUNC const char *W32_CALL sock_init_err (int rc);

W32_FUNC void MS_CDECL sock_exit (void);
W32_FUNC void W32_CALL dbug_init (void);  /* effective if compiled with `USE_DEBUG' */
W32_FUNC void W32_CALL init_misc (void);  /* may be called before sock_init() */
W32_FUNC void W32_CALL sock_sig_exit (const char *msg, int sigint);

W32_FUNC void W32_CALL assert_fail_test (void);
W32_FUNC void W32_CALL abort_test  (void);
W32_FUNC void W32_CALL leak_test   (void);
W32_FUNC void W32_CALL except_test (void);

/*
 * `s' is the pointer to a udp or tcp socket
 */
W32_FUNC int    W32_CALL sock_read       (sock_type *s, BYTE *dp, size_t len);
W32_FUNC int    W32_CALL sock_preread    (const sock_type *s, BYTE *dp, int len);
W32_FUNC int    W32_CALL sock_fastread   (sock_type *s, BYTE *dp, int len);
W32_FUNC int    W32_CALL sock_write      (sock_type *s, const BYTE *dp, int len);
W32_FUNC int    W32_CALL sock_enqueue    (sock_type *s, const BYTE *dp, int len);
W32_FUNC int    W32_CALL sock_fastwrite  (sock_type *s, const BYTE *dp, int len);
W32_FUNC size_t W32_CALL sock_setbuf     (sock_type *s, BYTE *buf, size_t len);
W32_FUNC void   W32_CALL sock_flush      (sock_type *s);
W32_FUNC void   W32_CALL sock_noflush    (sock_type *s);
W32_FUNC void   W32_CALL sock_flushnext  (sock_type *s);
W32_FUNC int    W32_CALL sock_puts       (sock_type *s, const BYTE *dp);
W32_FUNC WORD   W32_CALL sock_gets       (sock_type *s, BYTE *dp, int n);
W32_FUNC BYTE   W32_CALL sock_putc       (sock_type *s, BYTE c);
W32_FUNC int    W32_CALL sock_getc       (sock_type *s);
W32_FUNC WORD   W32_CALL sock_dataready  (sock_type *s);
W32_FUNC int    W32_CALL sock_established(sock_type *s);
W32_FUNC int    W32_CALL sock_close      (sock_type *s);
W32_FUNC int    W32_CALL sock_abort      (sock_type *s);

W32_FUNC WORD   W32_CALL sock_mode       (sock_type *s, WORD mode);
W32_FUNC int    W32_CALL sock_sselect    (const sock_type *s, int waitstate);
W32_FUNC int    W32_CALL sock_timeout    (sock_type *s, int seconds);
W32_FUNC int    W32_CALL sock_recv       (sock_type *s, void *buf, unsigned len);
W32_FUNC int    W32_CALL sock_recv_init  (sock_type *s, void *buf, unsigned len);
W32_FUNC int    W32_CALL sock_recv_from  (sock_type *s, DWORD *ip, WORD *port, void *buf, unsigned len, int peek);
W32_FUNC int    W32_CALL sock_recv_used  (const sock_type *s);
W32_FUNC int    W32_CALL sock_keepalive  (sock_type *s);

W32_FUNC size_t W32_CALL sock_rbsize     (const sock_type *s);
W32_FUNC size_t W32_CALL sock_rbused     (const sock_type *s);
W32_FUNC size_t W32_CALL sock_rbleft     (const sock_type *s);
W32_FUNC size_t W32_CALL sock_tbsize     (const sock_type *s);
W32_FUNC size_t W32_CALL sock_tbused     (const sock_type *s);
W32_FUNC size_t W32_CALL sock_tbleft     (const sock_type *s);

W32_FUNC VoidProc W32_CALL sock_yield    (tcp_Socket *s, VoidProc fn);

W32_FUNC int MS_CDECL sock_printf (sock_type *s, const char *fmt, ...)  W32_ATTR_PRINTF(2,3);

W32_FUNC int MS_CDECL sock_scanf (sock_type *s, const char *fmt, ...)   W32_ATTR_SCANF(2,3);

// added functions to get port information
W32_FUNC DWORD W32_CALL sock_rhost(const sock_type *s);
W32_FUNC WORD  W32_CALL sock_rport(const sock_type *s);
W32_FUNC WORD  W32_CALL sock_lport(const sock_type *s);

/*
 * TCP or UDP specific stuff, must be used for open's and listens, but
 * sock stuff above is used for everything else
 */
W32_FUNC int W32_CALL udp_open   (udp_Socket *s, WORD lport, DWORD ina, WORD port, ProtoHandler handler);
W32_FUNC int W32_CALL tcp_open   (tcp_Socket *s, WORD lport, DWORD ina, WORD port, ProtoHandler handler);
W32_FUNC int W32_CALL udp_listen (udp_Socket *s, WORD lport, DWORD ina, WORD port, ProtoHandler handler);
W32_FUNC int W32_CALL tcp_listen (tcp_Socket *s, WORD lport, DWORD ina, WORD port, ProtoHandler handler, WORD timeout);
W32_FUNC int W32_CALL tcp_established (const tcp_Socket *s);


/*
 * less general functions
 */
W32_FUNC int   W32_CALL tcp_cbreak (int mode);
W32_FUNC char *W32_CALL rip        (char *s);
W32_FUNC int   W32_CALL watt_kbhit (void);
W32_FUNC int   W32_CALL priv_addr  (DWORD ip);

#define tcp_cbrk(mode)  tcp_cbreak(mode) /* old name */

/*
 * Set MD5 secret for TCP option 19 (RFC-2385).
 * Only if built with USE_TCP_MD5.
 */
W32_FUNC const char *W32_CALL tcp_md5_secret (tcp_Socket *s, const char *secret);

W32_FUNC void W32_CALL make_md5_signature (const in_Header  *ip,
                                           const tcp_Header *tcp,
                                           WORD              datalen,
                                           const char       *secret,
                                           BYTE             *buf);

W32_FUNC BOOL W32_CALL check_md5_signature (const tcp_Socket *s,
                                            const in_Header *ip);

/*
 * timers
 */
W32_FUNC DWORD  W32_CALL set_timeout  (DWORD msec);
W32_FUNC int    W32_CALL chk_timeout  (DWORD value);
W32_FUNC int    W32_CALL cmp_timers   (DWORD t1, DWORD t2);
W32_FUNC int    W32_CALL set_timediff (long msec);
W32_FUNC long   W32_CALL get_timediff (DWORD now, DWORD t);
W32_FUNC int    W32_CALL hires_timer  (int on);

W32_FUNC double         W32_CALL timeval_diff  (const struct timeval *a, const struct timeval *b);
W32_FUNC struct timeval W32_CALL timeval_diff2 (struct timeval *a, struct timeval *b);

#if defined(__MSDOS__)
  W32_FUNC void W32_CALL init_timer_isr (void);
  W32_FUNC void W32_CALL exit_timer_isr (void);
#endif

W32_FUNC void   W32_CALL init_userSuppliedTimerTick (void);
W32_FUNC void   W32_CALL userTimerTick (DWORD);

W32_FUNC void   W32_CALL ip_timer_init (sock_type *s, unsigned delayseconds);
W32_FUNC int    W32_CALL ip_timer_expired (const sock_type *s);

/*
 * TCP/IP system variables
 */
W32_DATA DWORD     my_ip_addr;
W32_DATA DWORD     sin_mask;       /* eg.  0xFFFFFE00L */
W32_DATA int       sock_delay;
W32_DATA int       sock_inactive;
W32_DATA int       sock_data_timeout;
W32_DATA WORD      multihomes;

W32_DATA int       block_tcp;
W32_DATA int       block_udp;
W32_DATA int       block_icmp;
W32_DATA int       block_ip;
W32_DATA WORD      last_cookie;
W32_DATA DWORD     cookies [MAX_COOKIES];

W32_DATA BOOL      survive_eth;
W32_DATA BOOL      survive_bootp;
W32_DATA BOOL      survive_dhcp;
W32_DATA BOOL      survive_rarp;

W32_DATA unsigned  tcp_OPEN_TO;
W32_DATA unsigned  tcp_CLOSE_TO;
W32_DATA unsigned  tcp_RTO_ADD;
W32_DATA unsigned  tcp_RTO_BASE;
W32_DATA unsigned  tcp_RTO_SCALE;
W32_DATA unsigned  tcp_RST_TIME;
W32_DATA unsigned  tcp_RETRAN_TIME;
W32_DATA unsigned  tcp_MAX_VJSA;
W32_DATA unsigned  tcp_MAX_VJSD;
W32_DATA DWORD     tcp_recv_win;

W32_DATA int (W32_CALL *loopback_handler) (in_Header *ip);

/*
 * things you probably won't need to know about
 */

/*
 * sock_debugdump
 *      - dump some socket control block parameters
 * used for testing the kernal, not recommended
 */
W32_FUNC void W32_CALL sock_debugdump (const sock_type *s);

/*
 * tcp_config - read a configuration file
 *            - if special path desired, call after sock_init()
 *            - NULL reads path WATTCP.CFG env-var or from program's path
 * see sock_init();
 */
W32_FUNC long W32_CALL tcp_config (const char *path);

/*
 * tcp_config_name - return name of configuration file.
 */
W32_FUNC int W32_CALL tcp_config_name (char *name, int max);

/*
 * tcp_tick - must be called periodically by user application.
 *          - returns NULL when our socket closes
 */
W32_FUNC WORD W32_CALL tcp_tick (sock_type *s);

/*
 * tcp_set_debug_state - set to 1,2 or reset 0
 */
W32_FUNC void W32_CALL tcp_set_debug_state (WORD x);


/*
 * name domain constants, etc.
 */
W32_DATA WORD          dns_timeout;
W32_DATA BOOL          dns_recurse;
W32_DATA WORD         _watt_handle_cbreak;      /* ^C/^Break handle mode */
W32_DATA volatile int _watt_cbroke;             /* ^C/^Break occured */
W32_DATA unsigned     _mtu, _mss;
W32_DATA int           ctrace_on;
W32_DATA int           debug_on;
W32_DATA int           has_rdtsc;
W32_DATA DWORD         clocks_per_usec;
W32_DATA unsigned      tcp_keep_idle;
W32_DATA unsigned      tcp_keep_intvl;
W32_DATA unsigned      tcp_max_idle;

#if !defined(WATT32_NO_OLDIES)
  /*
   * Old compatibility
   */
  #define wathndlcbrk  _watt_handle_cbreak
  #define watcbroke    _watt_cbroke
#endif

/*
 * DNS stuff
 */
W32_DATA int (W32_CALL *_resolve_hook)(void);
W32_DATA BOOL           _resolve_exit;
W32_DATA BOOL           _resolve_timeout;

W32_DATA char            defaultdomain [MAX_HOSTLEN+1];
W32_DATA char           *def_domain;
W32_DATA DWORD           def_nameservers [MAX_NAMESERVERS];
W32_DATA WORD            last_nameserver;
W32_DATA int             dom_errno;

W32_FUNC DWORD       W32_CALL resolve       (const char *name);
W32_FUNC int         W32_CALL resolve_ip    (DWORD ip, char *name, int len);
W32_FUNC int         W32_CALL resolve_ip6   (const char *name, void *addr);
W32_FUNC DWORD       W32_CALL lookup_host   (const char *host, char *ip_str);
W32_FUNC const char *W32_CALL dom_strerror  (int err);
W32_FUNC char       *W32_CALL dom_remove_dot(char *name);

/*
 * sock_wait_ .. macros
 */

/*
 * sock_wait_established()
 *      - Waits until connected or aborts if timeout etc.
 *
 * sock_wait_input()
 *      - Waits for received input on socket 's'.
 *      - May not be valid input for sock_gets()..  check returned length.
 *
 * sock_tick()
 *      - Do tick and jump on abort.
 *
 * sock_wait_closed()
 *      - Close socket and wait until fully closed.
 *        Discards all received data.
 *
 * All these macros jump to label sock_err with contents of *statusptr
 * set to
 *       1 on closed normally.
 *      -1 on error, call sockerr(s) for cause.
 *
 */
W32_FUNC int W32_CALL _ip_delay0 (sock_type *s, int sec, UserHandler fn, int *statusptr);
W32_FUNC int W32_CALL _ip_delay1 (sock_type *s, int sec, UserHandler fn, int *statusptr);
W32_FUNC int W32_CALL _ip_delay2 (sock_type *s, int sec, UserHandler fn, int *statusptr);


#define sock_wait_established(s,seconds,fn,statusptr) \
        do {                                          \
           if (_ip_delay0 (s,seconds,fn,statusptr))   \
              goto sock_err;                          \
        } while (0)

#define sock_wait_input(s,seconds,fn,statusptr)       \
        do {                                          \
           if (_ip_delay1 (s,seconds,fn,statusptr))   \
              goto sock_err;                          \
        } while (0)

#define sock_tick(s, statusptr)                       \
        do {                                          \
           if (!tcp_tick(s)) {                        \
              if (statusptr) *statusptr = -1;         \
              goto sock_err;                          \
           }                                          \
        } while (0)

#define sock_wait_closed(s,seconds,fn,statusptr)      \
        do {                                          \
           if (_ip_delay2(s,seconds,fn,statusptr))    \
              goto sock_err;                          \
        } while (0)

/*
 * User hook for WATTCP.CFG initialisation file.
 */
W32_DATA void (W32_CALL *usr_init) (const char *keyword, const char *value);
W32_DATA void (W32_CALL *usr_post_init) (void);

/*!
 * Convert 'arg_func' below to this type.
 */
enum config_tab_types {
     ARG_ATOI,            /**< convert to int */
     ARG_ATOB,            /**< convert to 8-bit byte */
     ARG_ATOW,            /**< convert to 16-bit word */
     ARG_ATOIP,           /**< convert to ip-address (host order) */
     ARG_ATOX_B,          /**< convert to hex-byte */
     ARG_ATOX_W,          /**< convert to hex-word */
     ARG_ATOX_D,          /**< convert to hex-dword */
     ARG_STRDUP,          /**< duplicate string value */
     ARG_STRCPY,          /**< copy string value */
     ARG_RESOLVE,         /**< resolve host to IPv4-address */
     ARG_FUNC             /**< call convertion function */
   };

struct config_table {
       const char            *keyword;
       enum config_tab_types  type;
       void                  *arg_func;
     };


W32_FUNC int W32_CALL parse_config_table (
         const struct config_table *tab,
         const char                *section,
         const char                *name,
         const char                *value);

/*
 * Run with no config file (embedded/diskless)
 */
W32_DATA int _watt_no_config;

W32_FUNC void W32_CALL tcp_inject_config (
              const struct config_table *cfg,
              const char                *key,
              const char                *value);

typedef long (W32_CALL * WattUserConfigFunc) (int pass, const struct config_table *cfg);

W32_FUNC WattUserConfigFunc W32_CALL _watt_user_config (WattUserConfigFunc fn);


/*
 * Bypass standard handling of DHCP transient configuration
 */
W32_CLANG_PACK_WARN_OFF()

#include <sys/pack_on.h>

struct DHCP_config {
       DWORD  my_ip;
       DWORD  netmask;
       DWORD  gateway;
       DWORD  nameserver;
       DWORD  server;
       DWORD  iplease;
       DWORD  renewal;
       DWORD  rebind;
       DWORD  tcp_keep_intvl;
       BYTE   default_ttl;
       char   _hostname [MAX_HOSTLEN+1];
       char   domain [MAX_HOSTLEN+1];
       char   loghost [MAX_HOSTLEN+1]; /* Only used if USE_BSD_FUNC defined */
     };

#include <sys/pack_off.h>

W32_CLANG_PACK_WARN_DEF()

enum DHCP_config_op {
     DHCP_OP_READ  = 0,
     DHCP_OP_WRITE = 1,
     DHCP_OP_ERASE = 2
   };

W32_DATA BOOL DHCP_did_gratuitous_arp;

typedef int (W32_CALL * WattDHCPConfigFunc) (enum DHCP_config_op op,
                                             struct DHCP_config *cfg);

W32_FUNC WattDHCPConfigFunc W32_CALL DHCP_set_config_func (WattDHCPConfigFunc fn);

W32_FUNC void  W32_CALL DHCP_release (BOOL force);
W32_FUNC int   W32_CALL DHCP_read_config (void);
W32_FUNC DWORD W32_CALL DHCP_get_server (void);

/*
 * Various function-pointer hooks etc.
 */
W32_DATA int (MS_CDECL *_printf) (const char *, ...)  W32_ATTR_PRINTF(1,2);

W32_DATA int  (W32_CALL *_outch) (char c);
W32_DATA void (W32_CALL *wintr_chain) (void);
W32_DATA int  (W32_CALL *_tftp_write) (const void *buf, size_t length);
W32_DATA int  (W32_CALL *_tftp_close) (void);

W32_FUNC void  W32_CALL outs    (const char *s);
W32_FUNC void  W32_CALL outsnl  (const char *s);
W32_FUNC void  W32_CALL outsn   (const char *s, int n);
W32_FUNC void  W32_CALL outhexes(const char *s, int n);
W32_FUNC void  W32_CALL outhex  (char ch);

W32_FUNC int   W32_CALL wintr_enable (void);
W32_FUNC void  W32_CALL wintr_disable (void);
W32_FUNC void  W32_CALL wintr_shutdown (void);
W32_FUNC void  W32_CALL wintr_init (void);

W32_FUNC int   W32_CALL _ping     (DWORD host, DWORD num, const BYTE *pattern, size_t len);
W32_FUNC DWORD W32_CALL _chk_ping (DWORD host, DWORD *ping_num);
W32_FUNC void  W32_CALL add_ping  (DWORD host, DWORD tim, DWORD number);

W32_FUNC int   W32_CALL _eth_init         (void);
W32_FUNC void  W32_CALL _eth_release      (void);
W32_FUNC void *W32_CALL _eth_formatpacket (const void *eth_dest, WORD eth_type);
W32_FUNC void  W32_CALL _eth_free         (const void *buf);
W32_FUNC void *W32_CALL _eth_arrived      (WORD *type, BOOL *brdcast);
W32_FUNC int   W32_CALL _eth_send         (WORD len, const void *sock, const char *file, unsigned line);
W32_FUNC int   W32_CALL _eth_set_addr     (const void *addr);
W32_FUNC BYTE  W32_CALL _eth_get_hwtype   (BYTE *hwtype, BYTE *hwlen);

W32_DATA void *(W32_CALL *_eth_recv_hook) (WORD *type);
W32_DATA int   (W32_CALL *_eth_recv_peek) (void *mac_buf);
W32_DATA int   (W32_CALL *_eth_xmit_hook) (const void *buf, unsigned len);

W32_FUNC WORD W32_CALL in_checksum (const void *buf, unsigned len);

#define inchksum(buf,len)  in_checksum(buf, len)


/*
 * Some applications (tracert.c) may need pcarp.c functions.
 */
W32_FUNC BOOL W32_CALL _arp_resolve (DWORD ina, eth_address *eth);

/*
 * Stuff for Windows only
 */
#if defined(WATT32_ON_WINDOWS)
  W32_FUNC int    MS_CDECL gui_printf (const char *fmt, ...);
  W32_FUNC char * W32_CALL win_strerror (DWORD err);

  W32_FUNC int W32_CALL pkt_win_print_GetIfTable (void);
  W32_FUNC int W32_CALL pkt_win_print_GetIfTable2 (void);
  W32_FUNC int W32_CALL pkt_win_print_GetIfTable2Ex (void);
  W32_FUNC int W32_CALL pkt_win_print_GetIpNetTable (void);
  W32_FUNC int W32_CALL pkt_win_print_GetIpNetTable2 (void);
  W32_FUNC int W32_CALL pkt_win_print_GetIpAddrTable (void);
  W32_FUNC int W32_CALL pkt_win_print_GetAdaptersAddresses (void);
  W32_FUNC int W32_CALL pkt_win_print_GetAdapterOrderMap (void);

  W32_FUNC int W32_CALL pkt_win_print_RasEnumConnections (void);
  W32_FUNC int W32_CALL pkt_win_print_WlanEnumInterfaces (void);
#endif

/*
 * BSD-socket similarities.
 * Refer <sys/socket.h> for the real thing.
 */

struct watt_sockaddr {     /* for _getpeername, _getsockname */
       WORD   s_type;
       WORD   s_port;
       DWORD  s_ip;
       BYTE   s_spares[6]; /* unused */
     };

enum CHK_SOCKET_VAL {   /* ret-vals from _chk_socket() */
     VALID_UDP = 1,
     VALID_TCP,
     VALID_IP4,
     VALID_IP6
   };

W32_FUNC DWORD        W32_CALL _gethostid   (void);
W32_FUNC DWORD        W32_CALL _sethostid   (DWORD ip);
W32_FUNC int          W32_CALL _getsockname (const sock_type *s, void *dest, int *len);
W32_FUNC int          W32_CALL _getpeername (const sock_type *s, void *dest, int *len);

W32_FUNC int          W32_CALL _chk_socket  (const sock_type *s);
W32_FUNC void         W32_CALL psocket      (const sock_type *s);
W32_FUNC void         W32_CALL sock_sturdy  (sock_type *s, int level);
W32_FUNC const char * W32_CALL sockerr      (const sock_type *s);   /* UDP / TCP */
W32_FUNC void         W32_CALL sockerr_clear(sock_type *s);         /* UDP / TCP */
W32_FUNC const char * W32_CALL sockstate    (const sock_type *s);   /* UDP / TCP / Raw */

/*
 * Reduce internal states to "user-easy" states, GvB 2002-09
 */
enum TCP_SIMPLE_STATE {
     TCP_CLOSED,
     TCP_LISTENING,
     TCP_OPENING,
     TCP_OPEN,
     TCP_CLOSING
   };

W32_FUNC enum TCP_SIMPLE_STATE W32_CALL tcp_simple_state (const tcp_Socket *s);

W32_FUNC char *W32_CALL _inet_ntoa     (char *s, DWORD x);
W32_FUNC DWORD W32_CALL _inet_addr     (const char *name);
W32_FUNC DWORD W32_CALL aton           (const char *name);
W32_FUNC int   W32_CALL isaddr         (const char *name);
W32_FUNC DWORD W32_CALL aton_dotless   (const char *str);
W32_FUNC BOOL  W32_CALL isaddr_dotless (const char *str, DWORD *ip);

W32_FUNC int   W32_CALL addwattcpd  (VoidProc p);
W32_FUNC int   W32_CALL delwattcpd  (VoidProc p);
W32_FUNC void  W32_CALL stopwattcpd (void);


/*
 * BSD functions for read/write/select
 */
W32_FUNC int W32_CALL close_s  (int s);
W32_FUNC int W32_CALL write_s  (int s, const char *buf, int nbyte);
W32_FUNC int W32_CALL read_s   (int s,       char *buf, int nbyte);
W32_FUNC int W32_CALL writev_s (int s, const struct iovec *vector, size_t count);
W32_FUNC int W32_CALL readv_s  (int s, const struct iovec *vector, size_t count);

W32_FUNC int W32_CALL select_s (int num_sockets,
                                fd_set *read_events,
                                fd_set *write_events,
                                fd_set *except_events,
                                struct timeval *timeout);

/* Some BSD/Winsock replacements.
 * Normally belongs in djgpp's <unistd.h>.
 *
 * When compiling ../src/winadif.c, these prototypes conflicts with the
 * one in <winsock*.h>.
 *
 * Note: gethostname() is also in <sys/socket.h>
 */
#if !defined(__DJGPP__) && !defined(WATT32_COMPILING_WINADINF_C)
  W32_FUNC int W32_CALL select (int num_sockets,
                                fd_set *read_events,
                                fd_set *write_events,
                                fd_set *except_events,
                                struct timeval *timeout);
#endif

#if defined(__DJGPP__)
  W32_FUNC int W32_CALL gethostname (char *name, int len);

#elif !defined(WATT32_COMPILING_WINADINF_C)
  W32_FUNC int W32_CALL gethostname (char *name, size_t len);
#endif

#if defined(__CYGWIN__)
  /* CygWin's <unistd.h> doesn't agree with other vendors here.
   */
  W32_FUNC long   W32_CALL gethostid (void);
  W32_FUNC long   W32_CALL sethostid (u_long ip);
  W32_FUNC int    W32_CALL sethostname (const char *name, size_t len);
#else
  W32_FUNC u_long W32_CALL gethostid (void);
  W32_FUNC u_long W32_CALL sethostid (u_long ip);
  W32_FUNC int    W32_CALL sethostname (const char *name, int len);
#endif

/*
 * Multicast stuff (if built with `USE_MULTICAST')
 */
W32_DATA int _multicast_on, _multicast_intvl;

W32_FUNC int W32_CALL join_mcast_group  (DWORD ip);
W32_FUNC int W32_CALL leave_mcast_group (DWORD ip);
W32_FUNC int W32_CALL multi_to_eth      (DWORD ip, eth_address *eth);
W32_FUNC int W32_CALL udp_SetTTL        (udp_Socket *s, BYTE ttl);
W32_FUNC int W32_CALL num_mcast_active  (void);

/*
 * Commandline parsing.
 *
 * The tricky part is that all the below headers declare getopt().
 * So we fall back to the watt_getopt() in Watt-32 for others who doesn't
 * have it. But we must allways export / define getopt.c functions and
 * data.
 */
#if defined(__DJGPP__)
  #include <unistd.h>
  #define WATT32_NO_GETOPT

#elif defined(__WATCOMC__) && (__WATCOMC__ >= 1250) /* OW 1.5+ */
  #include <unistd.h>
  #define WATT32_NO_GETOPT

#elif defined(__MINGW32__) || defined(__CYGWIN__)
  #include <unistd.h>
  #include <getopt.h>
  #define WATT32_NO_GETOPT
#endif

enum _watt_optmode {
      GETOPT_UNIX,        /* options at start of argument list (default)   */
      GETOPT_ANY,         /* move non-options to the end                   */
      GETOPT_KEEP         /* return options in order                       */
    };

W32_DATA char *watt_optarg;       /* argument of current option                    */
W32_DATA int   watt_optind;       /* index of next argument; default=0: initialize */
W32_DATA int   watt_opterr;       /* 0=disable error messages; default=1: enable   */
W32_DATA int   watt_optopt;       /* option char returned from getopt()            */
W32_DATA char *watt_optswchar;    /* characters introducing options; default="-"   */

W32_FUNC int W32_CALL watt_getopt (int argc, char *const *argv, const char *opt_str);

#if !defined(WATT32_NO_GETOPT)
  #define optarg    watt_optarg
  #define optind    watt_optind
  #define opterr    watt_opterr
  #define optopt    watt_optopt
  #define optswchar watt_optswchar
  #define getopt    watt_getopt
#endif

#if defined(WATT32_ON_WINDOWS)
 /* to-do */
 W32_DATA wchar_t *    _w_watt_optarg;
 W32_DATA wchar_t *    _w_watt_optswchar;
 W32_FUNC int W32_CALL _w_watt_getopt (int argc, wchar_t *const *argv, const wchar_t *opt_str);

 #if (defined(UNICODE) || defined(_UNICODE)) && !defined(_tgetopt)
    #define _toptarg          _w_watt_optarg
    #define _toptswchar       _w_watt_optswchar
    #define _tgetopt(c,a,o)   _w_watt_getopt (c,a,o)
  #else
    #define _toptarg          watt_optarg
    #define _toptswchar       watt_optswchar
    #define _tgetopt(c,a,o)   watt_getopt(c,a,o)
  #endif
#endif   /* WATT32_ON_WINDOWS */

/*
 * Statistics printing
 */
W32_FUNC void W32_CALL print_mac_stats  (void);
W32_FUNC void W32_CALL print_pkt_stats  (void);
W32_FUNC void W32_CALL print_vjc_stats  (void);
W32_FUNC void W32_CALL print_arp_stats  (void);
W32_FUNC void W32_CALL print_pppoe_stats(void);
W32_FUNC void W32_CALL print_ip4_stats  (void);
W32_FUNC void W32_CALL print_ip6_stats  (void);
W32_FUNC void W32_CALL print_icmp_stats (void);
W32_FUNC void W32_CALL print_igmp_stats (void);
W32_FUNC void W32_CALL print_udp_stats  (void);
W32_FUNC void W32_CALL print_tcp_stats  (void);
W32_FUNC void W32_CALL print_all_stats  (void);
W32_FUNC void W32_CALL reset_stats      (void);

W32_FUNC int W32_CALL sock_stats (sock_type *s, DWORD *days, WORD *inactive,
                                  WORD *cwindow, DWORD *avg,  DWORD *sd);

/*
 * PktDrvr/WinPcap interface
 */

/*\struct PktStats
 *
 * Driver statistics.
 */
struct PktStats {
       DWORD  in_packets;       /* # of packets received */
       DWORD  out_packets;      /* # of packets transmitted */
       DWORD  in_bytes;         /* # of bytes received */
       DWORD  out_bytes;        /* # of bytes transmitted */
       DWORD  in_errors;        /* # of reception errors */
       DWORD  out_errors;       /* # of transmission errors */
       DWORD  lost;             /* # of packets lost (RX) */
     };

#if defined(WATT32_ON_WINDOWS)
  /**
   * Driver classes. Use same values as in <ntddndis.h>.
   */
  #define PDCLASS_ETHER     0       /**< NdisMedium802_3 */
  #define PDCLASS_TOKEN     1       /**< NdisMedium802_5 */
  #define PDCLASS_FDDI      2       /**< NdisMediumFddi */
  #define PDCLASS_ARCNET    6       /**< NdisMediumArcnetRaw ? */
  #define PDCLASS_SLIP      0xFF01  /**< Serial Line IP (unsupported) */
  #define PDCLASS_AX25      0xFF02  /**< Amateur X.25 (unsupported) */
  #define PDCLASS_TOKEN_RIF 0xFF03  /**< IEEE 802.5 w/expanded RIFs (unsupported) */
  #define PDCLASS_PPP       0xFF04  /**< PPP/Wan (unsupported) */
  #define PDCLASS_UNKNOWN   0xFFFF

  struct PktParameters {
         int dummy;   /* To be defined */
       };
#else

  /**
   * DOS PktDrvr classes.
   */
  #define PDCLASS_ETHER     1       /**< IEEE 802.2 */
  #define PDCLASS_TOKEN     3       /**< IEEE 802.5 */
  #define PDCLASS_SLIP      6       /**< Serial Line IP */
  #define PDCLASS_ARCNET    8       /**< ARC-net (2.5 MBit/s) */
  #define PDCLASS_AX25      9       /**< Amateur X.25 (packet radio) */
  #define PDCLASS_FDDI      12      /**< FDDI w/802.2 headers */
  #define PDCLASS_TOKEN_RIF 17      /**< IEEE 802.5 w/expanded RIFs */
  #define PDCLASS_PPP       18
  #define PDCLASS_UNKNOWN   0xFFFF

  #include <sys/pack_on.h>      /* cstate, slcompress etc. must be packed */

  /*\struct PktParameters
   *
   * PKTDRVR parameters.
   */
  struct PktParameters {
         BYTE  major_rev;       /* Revision of Packet Driver spec */
         BYTE  minor_rev;       /*  this driver conforms to. */
         BYTE  length;          /* Length of structure in bytes */
         BYTE  addr_len;        /* Length of a MAC-layer address */
         WORD  mtu;             /* MTU, including MAC headers */
         WORD  multicast_avail; /* Buffer size for multicast addr */
         WORD  rcv_bufs;        /* (# of back-to-back MTU rcvs) - 1 */
         WORD  xmt_bufs;        /* (# of successive xmits) - 1 */
         WORD  int_num;         /* interrupt for post-EOI processing */
       };

  #include <sys/pack_off.h>
#endif

W32_FUNC int          W32_CALL pkt_eth_init      (mac_address *eth);
W32_FUNC int          W32_CALL pkt_release       (void);
W32_FUNC int          W32_CALL pkt_reset_handle  (WORD handle);
W32_FUNC int          W32_CALL pkt_send          (const void *tx, int length);
W32_FUNC int          W32_CALL pkt_buf_wipe      (void);
W32_FUNC void         W32_CALL pkt_free_pkt      (const void *pkt);
W32_FUNC int          W32_CALL pkt_waiting       (void);
W32_FUNC int          W32_CALL pkt_set_addr      (const void *eth);
W32_FUNC int          W32_CALL pkt_get_addr      (mac_address *eth);
W32_FUNC int          W32_CALL pkt_get_mtu       (void);
W32_FUNC int          W32_CALL pkt_get_drvr_ver  (WORD *major, WORD *minor, WORD *unused, WORD *build);
W32_FUNC int          W32_CALL pkt_get_api_ver   (WORD *ver);
W32_FUNC int          W32_CALL pkt_set_rcv_mode  (int mode);
W32_FUNC int          W32_CALL pkt_get_rcv_mode  (void);
W32_FUNC BOOL         W32_CALL pkt_check_address (DWORD my_ip);
W32_FUNC BOOL         W32_CALL pkt_is_init       (void);
W32_FUNC int          W32_CALL pkt_get_params    (struct PktParameters *params);
W32_FUNC int          W32_CALL pkt_get_stats     (struct PktStats *stats, struct PktStats *total);
W32_FUNC DWORD        W32_CALL pkt_dropped       (void);
W32_FUNC int          W32_CALL pkt_get_vector    (void);

W32_FUNC const char * W32_CALL pkt_strerror (int code);
W32_FUNC const char * W32_CALL pkt_get_device_name (void);  /* "\Device\NPF_{..." */
W32_FUNC const char * W32_CALL pkt_get_drvr_name (void);    /* NPF.SYS/SwsVpkt.sys/airpcap.sys etc. */
W32_FUNC const char * W32_CALL pkt_get_drvr_descr (void);   /* Driver description */
W32_FUNC WORD         W32_CALL pkt_get_drvr_class (void);   /* Driver class */


/*
 * Controlling timer interrupt handler for background processing.
 * Not recommended, little tested
 */
#if !defined(WATT32_ON_WINDOWS)
  W32_FUNC void W32_CALL backgroundon  (void);
  W32_FUNC void W32_CALL backgroundoff (void);
  W32_FUNC void W32_CALL backgroundfn  (VoidProc func);
#endif

/*
 * Misc functions
 */
#if !defined(__DJGPP__) && !defined(__CYGWIN__) && \
    !(defined(__WATCOMC__) && (__WATCOMC__ >= 1240))
  W32_FUNC int W32_CALL ffs (int mask);
#endif

#if defined(__HIGHC__)
  W32_FUNC int W32_CALL system (const char *cmd);
  pragma Alias (system, "_mw_watt_system");
#endif

#if defined(__HIGHC__) || (defined(__DMC__) && defined(__MSDOS__))
  W32_FUNC void W32_CALL delay (unsigned int msec);
#endif

W32_FUNC unsigned W32_CALL Random (unsigned a, unsigned b);
W32_FUNC void     W32_CALL RandomWait (unsigned a, unsigned b);


/*
 * Tracing to RS-232 serial port, by Gundolf von Bachhaus <GBachhaus@gmx.net>
 * Watt-32 library must be compiled with `USE_RS232_DBG' (see .\src\config.h)
 */
W32_FUNC int W32_CALL   trace2com_init (WORD portAddress, DWORD baudRate);
W32_FUNC int MS_CDECL __trace2com      (const char *fmt, ...)  W32_ATTR_PRINTF(1,2);

#ifdef __cplusplus
}
#endif

#endif  /* RC_INVOKED */
#endif  /* __WATT_TCP_H */
