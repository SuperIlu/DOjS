/*!\file sock_ini.h
 */
#ifndef _w32_SOCK_INI_H
#define _w32_SOCK_INI_H

enum eth_init_result {    /* pass to sock_init_err() */
     WERR_NO_ERROR,
     WERR_ILL_DOSX,       /* Watcom/HighC: illegal DOS-extender */
     WERR_NO_MEM,         /* All: No memory for misc. buffers */
     WERR_NO_DRIVER,      /* All: No network driver found (PKTDRVR/WinPcap) */
     WERR_PKT_ERROR,      /* All: General error in PKTDRVR/WinPcap interface */
     WERR_BOOTP_FAIL,     /* All: BOOTP protocol failed */
     WERR_DHCP_FAIL,      /* All: DHCP protocol failed */
     WERR_RARP_FAIL,      /* All: RARP protocol failed */
     WERR_NO_IPADDR,      /* All: Failed to find an IP-address */
     WERR_PPPOE_DISC      /* All: PPPoE discovery failed (timeout) */
   };

#define _bootp_on      W32_NAMESPACE (_bootp_on)
#define _dhcp_on       W32_NAMESPACE (_dhcp_on)
#define _dhcp6_on      W32_NAMESPACE (_dhcp6_on)
#define _rarp_on       W32_NAMESPACE (_rarp_on)
#define _do_mask_req   W32_NAMESPACE (_do_mask_req)

#define survive_eth    W32_NAMESPACE (survive_eth)
#define survive_bootp  W32_NAMESPACE (survive_bootp)
#define survive_dhcp   W32_NAMESPACE (survive_dhcp)
#define survive_rarp   W32_NAMESPACE (survive_rarp)

extern int  _bootp_on;    /* boot-up through BOOTP and/or DHCP */
extern int  _dhcp_on;
extern int  _dhcp6_on;
extern int  _rarp_on;
extern BOOL _do_mask_req;
extern BOOL _watt_is_init;

W32_DATA BOOL _watt_do_exit;

extern BOOL survive_eth,  survive_bootp;
extern BOOL survive_dhcp, survive_rarp;

extern WattUserConfigFunc _watt_user_config_fn;

#if !defined(sock_init) && defined(TEST_PROG)
  #define sock_init()  watt_sock_init (0, 0, sizeof(time_t))
#endif

#if defined(SWIG)
  #undef sock_init
  static int sock_init (void)
  {
    return watt_sock_init (0, 0, sizeof(time_t));
  }
#endif

#endif
