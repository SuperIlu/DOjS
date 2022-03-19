/*!\file pcbootp.c
 *
 * BOOTP - Boot Protocol (RFC 854/951/1048).
 *
 * These extensions get called if _bootp_on is set.
 *
 * \version 0.3: Feb  1, 1992 : J. Dent - patched up various things.
 * \version 0.2: May 22, 1991 : E.J. Sutcliffe - added RFC_1048 vendor fields.
 * \version 0.1: May  9, 1991 : E. Engelke - made part of the library.
 * \version 0.0: May  3, 1991 : E. Engelke - original program as an application.
 */

#include <stdio.h>
#include <stdlib.h>

#include "copyrigh.h"
#include "wattcp.h"
#include "strings.h"
#include "language.h"
#include "pcdns.h"
#include "syslog2.h"
#include "misc.h"
#include "timer.h"
#include "netaddr.h"
#include "pctcp.h"
#include "pcsed.h"
#include "pcarp.h"
#include "pcconfig.h"
#include "pcqueue.h"
#include "pcpkt.h"
#include "pcbootp.h"

#undef udp_Socket

DWORD _bootp_host = IP_BCAST_ADDR;

#if defined(USE_DHCP)
  int _bootp_timeout = 15;  /* give DHCP a chance also (sock_delay=30) */
#else
  int _bootp_timeout = 30;
#endif

#if defined(USE_BOOTP)

static sock_type *boot_sock;

static int  bootp_xmit  (struct bootp *bootp_out);
static BOOL bootp_recv  (struct bootp *bootp_out, struct bootp *bootp_in);
static void bootp_parse (const struct bootp *bootp_in, int len);

/**
 * Main BOOTP initialisation.
 * Checks global variables _bootp_timeout, _bootp_host.
 * If no host specified, use the broadcast address.
 *   \retval 1 success, local IP address set.
 *   \retval 0 something failed.
 *
 * \note Doesn't retry the bootp_xmit() on timeout.
 */
int BOOTP_do_boot (void)
{
  struct bootp      bootp_out, bootp_in;
  struct udp_Socket sock;
  DWORD  save_ip = my_ip_addr;
  int    rc = 0;

  boot_sock = (sock_type*) &sock;

  outs (_LANG("Configuring through BOOTP.."));

  my_ip_addr = 0UL;   /* We must use IP address 0.0.0.0 for sending */

  if (!udp_open(&sock,IPPORT_BOOTPC,_bootp_host,IPPORT_BOOTPS,NULL))
  {
    outsnl (boot_sock->udp.err_msg);
    my_ip_addr = save_ip;
  }
  else
  {
    memset (&bootp_in, 0, sizeof(bootp_in));
    memset (&bootp_out, 0, sizeof(bootp_out));
    bootp_xmit (&bootp_out);
    if (bootp_recv(&bootp_out, &bootp_in))
    {
      my_ip_addr = intel (bootp_in.bp_yiaddr);
      rc = 1;
    }
  }
  sock_close (boot_sock);
  boot_sock = NULL;
  outsnl (rc ? "okay" : "failed");
  return (rc);
}

static int bootp_xmit (struct bootp *bootp_out)
{
  DWORD exchange_id = set_timeout (0);  /* "random" exchange ID */

  _eth_get_hwtype (&bootp_out->bp_htype, &bootp_out->bp_hlen);

  bootp_out->bp_op   = BOOTP_REQUEST;
  bootp_out->bp_xid  = exchange_id;
  bootp_out->bp_secs = intel16 (((WORD)exchange_id & 7) + 7);
  *(DWORD*) &bootp_out->bp_vend = intel (VM_RFC1048);  /* Magic Number */

  memcpy (&bootp_out->bp_chaddr, _eth_addr, _eth_mac_len);
  return sock_fastwrite (boot_sock, (BYTE*)bootp_out, sizeof(*bootp_out));
}

static BOOL bootp_recv (struct bootp *bootp_out, struct bootp *bootp_in)
{
  DWORD exchange_id = bootp_out->bp_xid;
  DWORD boot_timer  = set_timeout (1000 * _bootp_timeout);

  while (1)
  {
    DWORD vendor;
    int   len;

    WATT_YIELD();

    if (chk_timeout(boot_timer) || !tcp_tick(boot_sock))
       return (FALSE);

    if (sock_dataready(boot_sock) < sizeof(*bootp_in))
       continue;

    /* got a response, lets consider it
     */
    memset (bootp_in, 0, sizeof(*bootp_in));
    len = sock_fastread (boot_sock, (BYTE*)bootp_in, sizeof(*bootp_in));
    if (len < BOOTP_MIN_SIZE ||
        bootp_in->bp_op != BOOTP_REPLY)
       continue;

    /* Check if transaction ID and MAC-address matches
     */
    if (bootp_in->bp_xid != exchange_id ||
        memcmp(&bootp_in->bp_chaddr, _eth_addr, _eth_mac_len))
       continue;

    vendor = intel (*(DWORD*)&bootp_in->bp_vend);
    if (vendor == VM_RFC1048)
       bootp_parse (bootp_in, len);
    break;
  }
  return (TRUE);
}

/*
 * Parse RFC1048 compliant BOOTP vendor field.
 * Based heavily on NCSA Telnet BOOTP
 */
static void bootp_parse (const struct bootp *bootp_in, int max)
{
  const BYTE *p   = &bootp_in->bp_vend[4]; /* Point after magic value */
  const BYTE *end = max + (const BYTE*)bootp_in;
  BOOL  got_end = FALSE;

  while (!got_end && p < end)
  {
    DWORD ip;
    int   i, len;

    switch (*p)
    {
      case BOOTP_OPT_PAD:
           p++;
           continue;

      case BOOTP_OPT_SUBNET_MASK:
           sin_mask = intel (*(DWORD*)(p+2));
           break;

      case BOOTP_OPT_ROUTERS_ON_SNET:
           /* only add first */
           ip = intel (*(DWORD*)(p+2));
           _arp_add_gateway (NULL, ip);
           break;

      case BOOTP_OPT_DNS_SRV:
           for (i = 0; i < *(p+1); i += sizeof(ip))
           {
             ip = intel (*(DWORD*)(p+2+i));
             _add_server (&last_nameserver, def_nameservers,
                          DIM(def_nameservers), ip);
           }
           break;

      case BOOTP_OPT_COOKIE_SRV:
           for (i = 0; i < *(p+1) ; i += sizeof(ip))
           {
             ip = intel (*(DWORD*)(p+2+i));
             _add_server (&last_cookie, cookies, DIM(cookies), ip);
           }
           break;

#if defined(USE_BSD_API)
      case BOOTP_OPT_LOG_SRV:
           ip = intel (*(DWORD*)(p+2));  /* select 1st host */
           if (!syslog_host_name[0] &&   /* not in config-file */
               p[1] % 4 == 0)            /* length = n * 4 */
             _strlcpy (syslog_host_name, _inet_ntoa(NULL,ip),
                       sizeof(syslog_host_name));
           break;
#endif

      case BOOTP_OPT_HOST_NAME:
           len = min (p[1], sizeof(hostname));
           memcpy (&hostname[0], p+2, len);
           hostname[len] = '\0';
           break;

      case BOOTP_OPT_NAME_SRV:    /* IEN-116 name server */
      case BOOTP_OPT_LPR_SRV:
      case BOOTP_OPT_IMPRESS_SRV:
      case BOOTP_OPT_RES_LOCATION_SRV:
      case BOOTP_OPT_TIME_SRV:
      case BOOTP_OPT_TIME_OFFSET:
           break;

      case BOOTP_OPT_END:
           got_end = TRUE;
           break;

      default:
           break;
    }
    p += *(p+1) + 2;
  }
}
#endif /* USE_BOOTP */

