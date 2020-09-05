/*!\file pcdhcp.c
 *
 *  Dynamic Host Configuration Protocol (RFC 1541/2131/2132).
 *  These extensions gets called if "MY_IP" is set to "DHCP" or
 *  if no WATTCP.CFG file is found.
 */

/*  Copyright (c) 1997-2007 Gisle Vanem <gvanem@yahoo.no>
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. All advertising materials mentioning features or use of this software
 *     must display the following acknowledgement:
 *       This product includes software developed by Gisle Vanem
 *       Bergen, Norway.
 *
 *  THIS SOFTWARE IS PROVIDED BY ME (Gisle Vanem) AND CONTRIBUTORS ``AS IS''
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL I OR CONTRIBUTORS BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*  \version 0.5: Oct 28, 1996 :
 *    G. Vanem - implemented from RFC1541 with help from pcbootp.c.
 *
 *  \version 0.6: May 18, 1997 :
 *    G. Vanem - added RFC2131 DHCPINFORM message.
 *
 *  \version 0.7: Apr 25, 2002 :
 *     G. Vanem - added RFC3004 User Class option.
 *
 *  \version 0.8: Jul 24, 2002 :
 *    G. Vanem - Rewitten as a non-blocking FSM.
 *             - Removed DHCPINFORM message.
 *
 *  \version 0.9: Nov 08, 2002 :
 *    G. Vanem - added handling of option 60 (Vendor Class)
 *
 *  \version 0.91: Feb 21, 2003 :
 *    Greg Bredthauer found some bugs in DHCP_request(); DHCP_USER_CLASS
 *    and DHCP_CLASS_ID tags had 2 bytes of zeros in them.
 *    Added sending DHCP_CLIENT_ID in DHCP_request() since some DHCP-servers
 *    doesn't reply without it.
 *
 *  \version 0.92: Jul 17, 2003:
 *    Riccardo De Agostini contibuted changes for application hooking of
 *    transient configuration; dhcp_set_config_func() etc.
 *
 *  \version 0.93: Jul 2, 2004:
 *    Fixed logic around reading transient config.
 *
 *  \version 0.94: Oct 22, 2007:
 *    Fixes for Linux dhcp servers: parse dhcp options in ACK msgs.
 *    Changed DHCP_do_boot() to handle renew and rebind as well as init.
 *    Renegotiate lease if past the renew or rebind times.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <io.h>
#include <ctype.h>
#include <errno.h>

#include "wattcp.h"
#include "strings.h"
#include "language.h"
#include "misc.h"
#include "run.h"
#include "timer.h"
#include "pcdns.h"
#include "netaddr.h"
#include "bsdname.h"
#include "ip4_out.h"
#include "syslog2.h"
#include "sock_ini.h"
#include "printk.h"
#include "tftp.h"
#include "pctcp.h"
#include "pcsed.h"
#include "pcarp.h"
#include "pcqueue.h"
#include "pcdbug.h"
#include "pcpkt.h"
#include "pcconfig.h"
#include "pcbootp.h"
#include "pcdhcp.h"

#if defined(USE_DHCP)

#define BROADCAST_FLAG  intel16 (0x8000) /* network order (LSB=1) */

/* This was 284, but D-Link 614+ router ACKs are too small
 */
#define DHCP_MIN_SIZE  280

BOOL DHCP_did_gratuitous_arp = FALSE;

static WattDHCPConfigFunc config_func = NULL;

static DWORD exchange_id;
static DWORD router, nameserver;
static DWORD dhcp_server    = 0;
static DWORD dhcp_renewal   = 0;
static DWORD dhcp_rebind    = 0;
static DWORD dhcp_iplease   = 0;
static DWORD suggest_lease  = 0;
static DWORD old_ip_addr    = 0;

static BOOL  bcast_flag     = TRUE;
static BOOL  got_offer      = FALSE;
static BOOL  configured     = FALSE;
static BOOL  arp_check_ip   = FALSE;
static BOOL  cfg_read       = FALSE;
static BOOL  cfg_saved      = FALSE;

static int   dhcp_timeout   = 10;
static int   max_retries    = 3;
static int   discover_loops = 0;

static struct dhcp dhcp_in, dhcp_out;

static void (*DHCP_state) (int) = NULL;
static time_t renewal_timeout;
static time_t rebind_timeout;
static time_t lease_timeout;
static DWORD  send_timeout;
static BOOL   trace_on = FALSE;

static char       config_file [MAX_VALUELEN+1] = "";
static sock_type *sock = NULL;

/* Default list of DHCP request values
 */
static const BYTE default_request_list[] = {
             DHCP_OPT_SUBNET_MASK,
             DHCP_OPT_ROUTERS_ON_SNET,
             DHCP_OPT_DNS_SRV,
             DHCP_OPT_COOKIE_SRV,
             DHCP_OPT_LPR_SRV,
             DHCP_OPT_HOST_NAME,
             DHCP_OPT_DOMAIN_NAME,
             DHCP_OPT_IP_DEFAULT_TTL,
             DHCP_OPT_IF_MTU,
             DHCP_OPT_ARP_CACHE_TIMEOUT,
             DHCP_OPT_ETHERNET_ENCAPSULATION,
             DHCP_OPT_TCP_DEFAULT_TTL,
#if defined(USE_BSD_API)
             DHCP_OPT_LOG_SRV,         /* links in syslog2.c */
             DHCP_OPT_NBIOS_NAME_SRV,  /* SMB-lib needs these */
             DHCP_OPT_NBIOS_NODE_TYPE,
             DHCP_OPT_NBIOS_SCOPE,
#endif
#if defined(USE_TFTP)
             DHCP_OPT_TFTP_SERVER,
             DHCP_OPT_BOOT_FILENAME
#endif
           };

static struct DHCP_list request_list = {   /**< List of DHCP request values */
                        (BYTE*) &default_request_list,
                        sizeof (default_request_list)
                      };

static struct DHCP_list extra_options = { NULL, 0 };
static struct DHCP_list user_class    = { NULL, 0 };
static struct DHCP_list vend_class    = { (BYTE*)"Watt-32", 7 };

static void  DHCP_state_INIT (int event);
static void  DHCP_state_BOUND (int event);
static void  DHCP_state_RENEWING (int event);
static void  DHCP_state_REQUESTING (int event);
static void  DHCP_state_SELECTING (int event);
static void  DHCP_state_REBINDING (int event);

static void  dhcp_options_add (const BYTE *opt, unsigned max);
static void  dhcp_set_timers  (void);
static void  change_ip_addr   (void);
static BYTE *put_request_list (BYTE *opt, int filled);
static int   write_config     (void);
static void  erase_config     (void);

static void  W32_CALL dhcp_fsm (void);

#define DHCP_SEND(end) sock_fastwrite (sock, (const BYTE*)&dhcp_out,  /* structure to send */ \
                                       end - (BYTE*)&dhcp_out)        /* length of structue */

#if defined(USE_DEBUG)
  #define TRACE(x)      do { if (trace_on) (*_printf) x; } while (0)
  #define INET_NTOA(x)  _inet_ntoa (NULL, x)
#else
  #define TRACE(x)      ((void)0)
  #define INET_NTOA(x)  ((void)0)
#endif

#if defined(USE_DEBUG)
/**
 * Return name of current DHCP state function.
 */
static const char *state_name (void)
{
  return (DHCP_state == DHCP_state_INIT       ? "INIT"       :
          DHCP_state == DHCP_state_BOUND      ? "BOUND"      :
          DHCP_state == DHCP_state_RENEWING   ? "RENEWING"   :
          DHCP_state == DHCP_state_REQUESTING ? "REQUESTING" :
          DHCP_state == DHCP_state_SELECTING  ? "SELECTING"  :
          DHCP_state == DHCP_state_REBINDING  ? "REBINDING"  : "??");
}

/**
 * Return nicely formatted string for a time-period.
 */
static const char *period (DWORD sec)
{
  static char buf[20];
  DWORD  hours = sec / 3600UL;

  if (sec < 60UL)
       sprintf (buf, "%lus", (u_long)sec);
  else if (sec < 3600UL)
       sprintf (buf, "%lu:%02lu", (u_long)(sec/60), (u_long)(sec % 60));
  else sprintf (buf, "%lu:%02lu:%02lu", (u_long)hours, (u_long)(sec/60-hours*60), (u_long)(sec % 60));
  return (buf);
}
#endif

/**
 * Format a BOOTP header.
 */
static BYTE *make_boot_header (void)
{
  DWORD my_ip = 0UL;

  if (DHCP_state == DHCP_state_BOUND    ||
      DHCP_state == DHCP_state_RENEWING ||
      DHCP_state == DHCP_state_REBINDING)
     my_ip = intel (my_ip_addr);

  memset (&dhcp_out, 0, sizeof(dhcp_out));

  _eth_get_hwtype (&dhcp_out.dh_htype, &dhcp_out.dh_hlen);

  dhcp_out.dh_op     = BOOTP_REQUEST;
  dhcp_out.dh_xid    = exchange_id;
  dhcp_out.dh_secs   = 0;
  dhcp_out.dh_flags  = bcast_flag ? BROADCAST_FLAG : 0;
  dhcp_out.dh_yiaddr = 0;
  dhcp_out.dh_ciaddr = my_ip;
  dhcp_out.dh_giaddr = 0;
  memcpy (dhcp_out.dh_chaddr, _eth_addr, _eth_mac_len);
  *(DWORD*) &dhcp_out.dh_opt[0] = DHCP_MAGIC_COOKIE;
  return (&dhcp_out.dh_opt[4]);
}

/**
 * Fill in the hardware type/len in Client ID option tag.
 */
static BYTE *put_hardware_opt (BYTE *opt)
{
  BYTE hw_type, hw_len;

  _eth_get_hwtype (&hw_type, &hw_len);
  *opt++ = DHCP_OPT_CLIENT_ID;
  *opt++ = hw_len + 1;
  *opt++ = hw_type;
  memcpy (opt, _eth_addr, hw_len);
  opt += hw_len;
  return (opt);
}

/**
 * Send a DHCP discover message.
 */
static int DHCP_discover (void)
{
  BYTE *opt, *start;

  exchange_id = set_timeout (0);    /* random exchange ID */
  opt = start = make_boot_header();
  *opt++ = DHCP_OPT_MSG_TYPE;
  *opt++ = 1;
  *opt++ = DHCP_DISCOVER;

  opt = put_hardware_opt (opt);

  *opt++ = DHCP_OPT_MAX_MSG_SIZE;   /* Maximum DHCP message size */
  *opt++ = 2;
  *(WORD*)opt = intel16 (sizeof(struct dhcp));
  opt += 2;

  if (suggest_lease)
  {
    *opt++ = DHCP_OPT_IP_ADDR_LEASE_TIME;
    *opt++ = sizeof (suggest_lease);
    *(DWORD*)opt = intel (suggest_lease);
    opt += sizeof (suggest_lease);
  }
  opt = put_request_list (opt, opt-start);
  *opt++ = DHCP_OPT_END;
  return DHCP_SEND (opt);
}

/**
 * Send a DHCP request message.
 */
static int DHCP_request (BOOL renew)
{
  BYTE *opt = make_boot_header();

  *opt++ = DHCP_OPT_MSG_TYPE;
  *opt++ = 1;
  *opt++ = DHCP_REQUEST;

  if (!renew)
  {
    *opt++ = DHCP_OPT_SRV_IDENTIFIER;
    *opt++ = sizeof (dhcp_server);
    *(DWORD*)opt = intel (dhcp_server);
    opt += sizeof (dhcp_server);
    *opt++ = DHCP_OPT_REQUESTED_IP_ADDR;
    *opt++ = sizeof (my_ip_addr);
    *(DWORD*)opt = intel (my_ip_addr);
    opt += sizeof (my_ip_addr);
  }

  /* Some DHCP-daemons require this tag in a REQUEST.
   */
  opt = put_hardware_opt (opt);

/* ... don't know why we would ever renew a lease
 * for the time remaining on our current lease...
 */
#if 0
  if (dhcp_iplease && dhcp_iplease < (DWORD)-1)
  {
    *opt++ = DHCP_OPT_IP_ADDR_LEASE_TIME;
    *opt++ = sizeof (dhcp_iplease);
    *(DWORD*)opt = intel (dhcp_iplease);
    opt += sizeof (dhcp_iplease);
  }
#endif

  if (user_class.data)
  {
    *opt++ = DHCP_OPT_USER_CLASS;
    *opt++ = user_class.size;
    memcpy (opt, user_class.data, user_class.size);
    opt += user_class.size;
  }

  if (vend_class.data)
  {
    *opt++ = DHCP_OPT_CLASS_ID;
    *opt++ = vend_class.size;
    memcpy (opt, vend_class.data, vend_class.size);
    opt += vend_class.size;
  }

  *opt++ = DHCP_OPT_END;
  TRACE (("DHCP req: renew(%d) dst(%s)\n",
          renew, INET_NTOA(sock->udp.hisaddr)));
  return DHCP_SEND (opt);
}

/**
 * Send a DHCP release or decline message.
 * \arg \b msg_type \b DHCP_RELEASE or \b DHCP_DECLINE.
 * \arg \b msg      Optional message describing the cause.
 */
static int DHCP_release_decline (int msg_type, const char *msg)
{
  BYTE *opt;

  exchange_id = set_timeout (0);   /* new exchange ID */
  opt = make_boot_header();

  *opt++ = DHCP_OPT_MSG_TYPE;
  *opt++ = 1;
  *opt++ = msg_type;
  *opt++ = DHCP_OPT_SRV_IDENTIFIER;
  *opt++ = sizeof (dhcp_server);
  *(DWORD*)opt = intel (dhcp_server);
  opt += sizeof (dhcp_server);

  if (msg)
  {
    const BYTE *end = &dhcp_out.dh_opt [sizeof(dhcp_out.dh_opt)-1];
    size_t      len = strlen (msg);

    len = min (len, 255);
    len = min (len, (size_t)(end-opt-3));
    *opt++ = DHCP_OPT_MSG;
    *opt++ = (BYTE)len;
    memcpy (opt, msg, len);
    opt += len;
  }
  *opt++ = DHCP_OPT_END;
  return DHCP_SEND (opt);
}

/**
 * Send an ARP reply to announce our new IP-address. This is in
 * order to fill ARP cache of other hosts (gratuitous ARP).
 *
 * Optionally (if \b arp_check_ip is TRUE), send an ARP request
 * to check if our assigned IP-address is really vacant.
 */
static BOOL DHCP_arp_check (DWORD my_ip)
{
  eth_address eth;

  if (_pktserial)
     return (TRUE);

  /* ARP broadcast to announce our new IP.
   */
  _arp_reply (NULL, my_ip, my_ip | ~sin_mask);
  DHCP_did_gratuitous_arp = TRUE;

  if (!arp_check_ip)
     return (TRUE);

  /* Check if IP is used by anybody else
   */
  TRACE (("Checking ARP.."));

  if (!_arp_check_own_ip(&eth))
  {
    TRACE (("station %s claims to have my IP %s! Declining..\n",
            MAC_address(&eth), INET_NTOA(my_ip)));
    if (!trace_on)
       outsnl ("Someone claims to have my IP! Declining..\7");
    return (FALSE);
  }
  TRACE (("\n"));
  return (TRUE);
}

/**
 * Parse DHCP offer reply.
 * \note May be called recursively.
 */
static int DHCP_offer (const struct dhcp *in)
{
  int   len;
  DWORD ip;
  const BYTE *opt = (const BYTE*) &in->dh_opt[4];

  while (opt < in->dh_opt + sizeof(in->dh_opt))
  {
    switch (*opt)
    {
      case DHCP_OPT_PAD:
           opt++;
           continue;

      case DHCP_OPT_SUBNET_MASK:
           sin_mask = intel (*(DWORD*)(opt+2));
           TRACE (("Net-mask:  %s\n", INET_NTOA(sin_mask)));
           break;

      case DHCP_OPT_TIME_OFFSET:
#if defined(USE_DEBUG)
           {
             long ofs = *(long*)(opt+2);
             TRACE (("Time-ofs:  %.2fh\n", (double)ofs/3600.0));
           }
#endif
           break;

      case DHCP_OPT_ROUTERS_ON_SNET:
           {
             static BOOL gw_added = FALSE;

             if (!gw_added)
                _arp_kill_gateways(); /* delete gateways from cfg-file */
             gw_added = TRUE;
             router = intel (*(DWORD*)(opt+2));
             _arp_add_gateway (NULL, router);
             TRACE (("Gateway:   %s\n", INET_NTOA(router)));
           }
           break;

      case DHCP_OPT_DNS_SRV:
           {
             static BOOL dns_added = FALSE;

             for (len = 0; len < *(opt+1); len += sizeof(DWORD))
             {
               ip = intel (*(DWORD*)(opt+2+len));
               if (!dns_added)
                  last_nameserver = 0;  /* delete nameserver from cfg-file */

               /*
                * !!fix-me: Add only first name-server cause resolve() doesn't
                * handle multiple servers (in different nets) very well
                */
               if (len == 0)
               {
                 nameserver = ip;
                 _add_server (&last_nameserver, def_nameservers,
                              DIM(def_nameservers), ip);
               }
               dns_added = TRUE;
               TRACE (("DNS:       %s\n", INET_NTOA(ip)));
             }
           }
           break;

#if defined(USE_BSD_API)
      case DHCP_OPT_LOG_SRV:
           if (!syslog_host_name[0] &&  /* not already set */
               opt[1] % 4 == 0)         /* length = n * 4 */
           {
             ip = intel (*(DWORD*)(opt+2));   /* select 1st host */
             _strlcpy (syslog_host_name, _inet_ntoa(NULL,ip),
                       sizeof(syslog_host_name));
             TRACE (("Syslog:    %s\n", syslog_host_name));
           }
           break;

      case DHCP_OPT_NBIOS_NAME_SRV:
           ip = intel (*(DWORD*)(opt+2));
           TRACE (("WINS:      %s\n", INET_NTOA(ip)));
           /** \todo make a hook for SMB-lib */
           break;

      case DHCP_OPT_NBIOS_NODE_TYPE:
           TRACE (("NBT node:  %02X\n", opt[2]));
           break;

      case DHCP_OPT_NBIOS_SCOPE:
           TRACE (("NBT scope: %.*s\n", opt[1], opt+2));
           break;
#endif
      case DHCP_OPT_HOST_NAME:
          /* Don't use sethostname() because '*(opt+2)' is not a FQDN.
           */
           len = min (opt[1], sizeof(hostname));
           memcpy (hostname, opt+2, len);
           hostname[len] = '\0';
           TRACE (("Host name:  `%s'\n", hostname));
           break;

      case DHCP_OPT_DOMAIN_NAME:
           len = min (opt[1], sizeof(defaultdomain)-1);
           setdomainname ((const char*)(opt+2), len+1);
           TRACE (("Domain:   `%s'\n", def_domain));
           break;

      case DHCP_OPT_IP_DEFAULT_TTL:
      case DHCP_OPT_TCP_DEFAULT_TTL:
           _default_ttl = opt[2];
           break;

      case DHCP_OPT_MSG_TYPE:
           if (opt[2] == DHCP_OFFER)
              got_offer = TRUE;
           break;

      case DHCP_OPT_MSG:
           outsn ((const char*)(opt+2), *(opt+1));
           break;

      case DHCP_OPT_SRV_IDENTIFIER:
           dhcp_server = intel (*(DWORD*)(opt+2));
           TRACE (("Server:    %s\n", INET_NTOA(dhcp_server)));
           break;

      case DHCP_OPT_IP_ADDR_LEASE_TIME:
           dhcp_iplease = intel (*(DWORD*)(opt+2));
           TRACE (("IP lease:  %s\n", period(dhcp_iplease)));
           break;

      case DHCP_OPT_T1_VALUE:
           dhcp_renewal = intel (*(DWORD*)(opt+2));
           TRACE (("Renewal:   %s\n", period(dhcp_renewal)));
           break;

      case DHCP_OPT_T2_VALUE:
           dhcp_rebind = intel (*(DWORD*)(opt+2));
           TRACE (("Rebind:    %s\n", period(dhcp_rebind)));
           break;

      case DHCP_OPT_TCP_KEEPALIVE_INTERVAL:
#if !defined(USE_UDP_ONLY)
           tcp_keep_intvl = intel (*(DWORD*)(opt+2));
#endif
           break;

      case DHCP_OPT_OVERLOAD:
           switch (opt[2])
           {
             case 1:
                  TRACE (("Overload:  `dh_file' options\n"));
                  dhcp_options_add (in->dh_file, sizeof(in->dh_file));
                  break;
             case 2:
                  TRACE (("Overload:  `dh_sname' options\n"));
                  dhcp_options_add (in->dh_sname, sizeof(in->dh_sname));
                  break;
             case 3:
                  TRACE (("Overload:  `dh_file/dh_sname' options\n"));
                  dhcp_options_add (in->dh_file, sizeof(in->dh_file));
                  dhcp_options_add (in->dh_sname, sizeof(in->dh_sname));
                  break;
           }
           break;

#if defined(USE_TFTP)
      case DHCP_OPT_TFTP_SERVER:
           {
             const char *serv;
             len  = opt[1];
             serv = tftp_set_server ((const char*)(opt+2), len);
             TRACE (("TFTP-serv: `%s'\n", serv));
             ARGSUSED (serv);
           }
           break;

      case DHCP_OPT_BOOT_FILENAME:
           {
             const char *file;
             len  = opt[1];
             file = tftp_set_boot_fname ((const char*)(opt+2), len);
             TRACE (("BOOT-file: `%s'\n", file));
             ARGSUSED (file);
           }
           break;
#endif

      case DHCP_OPT_USER_CLASS:  /* these options are ignored in replies */
      case DHCP_OPT_CLASS_ID:
           break;

      case DHCP_OPT_GRUB_MENU:  /* GRUB loader config-file */
           TRACE (("GRUB:      %.*s\n", opt[1], opt+2));
           break;

      case DHCP_OPT_ETHERNET_ENCAPSULATION:
           TRACE (("Encap:     %s\n", opt[2] ? "IEEE 802.3" : "Ethernet II"));
           if (opt[2] != 0)
              outsnl (_LANG("Only Ethernet II encapsulation supported"));
           break;

      case DHCP_OPT_END:
           TRACE (("got end-option\n"));
           return (got_offer);

      default:
           TRACE (("Ignoring option %d\n", *opt));
           break;
    }
    opt += *(opt+1) + 2;
  }

  if (extra_options.data)
  {
    struct dhcp ext;

    len = min (extra_options.size, sizeof(ext.dh_opt));
    extra_options.data [len] = DHCP_OPT_END;
    memcpy (ext.dh_opt, extra_options.data, len);
    DHCP_offer (&ext);
  }
  return (got_offer);
}

/**
 * Return TRUE if DHCP message is an ACK.
 */
static int DHCP_is_ack (void)
{
  const BYTE *opt = (const BYTE*) &dhcp_in.dh_opt[4];

  return (opt[0] == DHCP_OPT_MSG_TYPE && opt[1] == 1 && opt[2] == DHCP_ACK);
}

/**
 * Return TRUE if DHCP message is a NACK.
 */
static int DHCP_is_nack (void)
{
  const BYTE *opt = (const BYTE*) &dhcp_in.dh_opt[4];

  return (opt[0] == DHCP_OPT_MSG_TYPE && opt[1] == 1 && opt[2] == DHCP_NAK);
}

/**
 * Possibly send a DHCP release.
 * \arg \b force If TRUE, release unconditionally.
 * \note Send release if we are configured \b and remaining lease
 *       is below minimum lease (DHCP_MIN_LEASE = 10 sec).
 */
void W32_CALL DHCP_release (BOOL force)
{
  if (force)
  {
    TRACE (("Sending DHCP release 01\n"));
    DHCP_release_decline (DHCP_RELEASE, NULL);
  }
  else if (configured)
  {
    /* Don't release if:
     *   DHCP-config is saved on disk and remaining lease is
     *   above minimum.
     */
    if (!(cfg_saved &&
          (lease_timeout && (lease_timeout - time(NULL) > DHCP_MIN_LEASE))))
    {
      TRACE (("Sending DHCP release 02\n"));
      DHCP_release_decline (DHCP_RELEASE, NULL);
    }
  }
  DAEMON_DEL (dhcp_fsm);
  if (sock)
  {
    sock_close (sock);
    free (sock);
    sock = NULL;
  }
}

/**
 * Return current DHCP server address.
 */
DWORD W32_CALL DHCP_get_server (void)
{
  return (dhcp_server);
}

/**
 * Allocate and open a UDP socket for DHCP message exchange.
 */
static sock_type *dhcp_open (const char *msg, BOOL use_broadcast)
{
  udp_Socket *sock = malloc (sizeof(*sock));
  DWORD       host;

  if (!sock)
  {
    outs (_LANG("DHCP: malloc failed\n"));
    return (NULL);
  }

  if (msg && (trace_on || debug_on))
     outs (msg);

  if (use_broadcast)
       host = IP_BCAST_ADDR; /* 255.255.255.255 */
  else host = dhcp_server;   /* default is 0.0.0.0 which maps to 255.255.255.255 */

  if (!udp_open (sock, IPPORT_BOOTPC, host, IPPORT_BOOTPS, NULL))
  {
    if (trace_on || debug_on)
    {
      outs ("DHCP: ");
      outsnl (sock->err_msg);
    }
    free (sock);
    sock = NULL;
  }
  TRACE (("DHCP open: bc(%d) srvr(%s)\n",
          use_broadcast, INET_NTOA(dhcp_server)));
  return (sock_type*)sock;   /* sock is free'd in DHCP_exit() */
}

/**
 * Add MAC address of DHCP sever to our ARP cache.
 */
static void store_DHCP_server_MAC (void)
{
  if ((_pktdevclass == PDCLASS_ETHER || _pktdevclass == PDCLASS_TOKEN) &&
      memcmp(sock->udp.his_ethaddr, _eth_brdcast, sizeof(_eth_brdcast)))
  {
    const eth_address *eth = (const eth_address*) sock->udp.his_ethaddr;
    _arp_cache_add (dhcp_server, eth, TRUE);
  }
}

/**
 * DHCP state machine: state BOUND.
 * - Awaits T1-TIMEOUT and enter RENEWING state.
 */
static void DHCP_state_BOUND (int event)
{
  if (event == EVENT_T1_TIMEOUT) /* renewal timeout */
  {
    old_ip_addr = my_ip_addr;    /* remember current address */
    got_offer   = FALSE;

    TRACE (("Sending DHCP request 01\n"));
    DHCP_request (1);
    DHCP_state = DHCP_state_RENEWING;
  }
}

/**
 * DHCP state machine: state REQUESTING.
 * - Awaits SEND-TIMEOUT and send request.
 * - Awaits ACK and do ARP check. Enter BOUND if successful.
 * - Awaits NAK and enter INIT.
 */
static void DHCP_state_REQUESTING (int event)
{
  if (event == EVENT_SEND_TIMEOUT)
  {
    TRACE (("Sending DHCP request 00\n"));
    DHCP_request (0);

    /*UPDATE: 05MAR2006 paul.suggs@vgt.net
     *  There is a timing condition within the FSM where the state has changed
     *  to REQUESTING but in dhcp_fsm(), chk_timeout() will be evaluated true
     *  which sets send_timeout to 0 before the first execution of this state
     *  handler. Once set to 0, we have to wait for rollover to resend the
     *  above request if it is lost for some reason
     */
    send_timeout = set_timeout (Random(4000,6000));
  }
  else if (event == EVENT_ACK)
  {
    TRACE (("Got DHCP ack while requesting\n"));

    if (!DHCP_arp_check(my_ip_addr))
    {
      my_ip_addr = 0;     /* decline from 0.0.0.0 */
      DHCP_release_decline (DHCP_DECLINE, "IP is not free");
      send_timeout = set_timeout (Random(4000,6000));
      DHCP_state   = DHCP_state_INIT;
    }
    else
    {
      DHCP_offer (&dhcp_in);  /* parse options in the ack too */
      configured = 1;         /* we are (re)configured */
      if (dhcp_server)
         store_DHCP_server_MAC();
      dhcp_set_timers();
      send_timeout = 0UL;
      DHCP_state   = DHCP_state_BOUND;
    }
  }
  else if (event == EVENT_NAK)
  {
    send_timeout = set_timeout (Random(4000,6000));
    my_ip_addr = 0UL;
    DHCP_state = DHCP_state_INIT;
  }
}

/**
 * DHCP state machine: state REBINDING.
 * - Awaits ACK and enter BOUND state.
 * - Awaits NAK and enter INIT state.
 */
static void DHCP_state_REBINDING (int event)
{
  if (event == EVENT_ACK)
  {
    TRACE (("Got DHCP ack while rebinding\n"));
    dhcp_set_timers();
    DHCP_offer (&dhcp_in);
    change_ip_addr();
    DHCP_state = DHCP_state_BOUND;
  }
  else if (event == EVENT_NAK)
  {
    send_timeout = set_timeout (Random(4000,6000));
    my_ip_addr = 0UL;
    DHCP_state = DHCP_state_INIT;
  }
}

/**
 * DHCP state machine: state RENEWING.
 * - Awaits SEND-TIMEOUT and send request.
 * - Awaits T2-TIMEOUT and send request, enter REBINDING state.
 * - Awaits ACK and enter BOUND state.
 * - Awaits NAK and enter INIT state.
 */
static void DHCP_state_RENEWING (int event)
{
  if (event == EVENT_SEND_TIMEOUT)
  {
    TRACE (("Sending DHCP request for renew\n"));
    DHCP_request (1);
  }
  else if (event == EVENT_T2_TIMEOUT)
  {
    TRACE (("Sending DHCP request for rebind\n"));
    bcast_flag = TRUE;
    DHCP_request (1);
    DHCP_state = DHCP_state_REBINDING;
  }
  else if (event == EVENT_ACK)
  {
    TRACE (("Got DHCP ack while renewing\n"));
    dhcp_set_timers();
    DHCP_offer (&dhcp_in);
    change_ip_addr();
    DHCP_state = DHCP_state_BOUND;
  }
  else if (event == EVENT_NAK)
  {
    TRACE (("Got DHCP nack while renewing\n"));
    send_timeout = set_timeout (Random(4000,6000));
    my_ip_addr = 0;
    DHCP_state = DHCP_state_INIT;
  }
}

/**
 * DHCP state machine: state SELECTING.
 * - Awaits OFFER, set SEND_TIMER and enter REQUESTING state.
 * - Awaits SEND-TIMEOUT, enter INIT state and resend discover.
 */
static void DHCP_state_SELECTING (int event)
{
  if (event == EVENT_OFFER && !got_offer && DHCP_offer(&dhcp_in))
  {
    TRACE (("Got DHCP offer\n"));
    send_timeout = set_timeout (Random(100,500));

    if (dhcp_renewal == 0L)
        dhcp_renewal = dhcp_iplease / 2;     /* default T1 time */
    if (dhcp_rebind == 0)
        dhcp_rebind = dhcp_iplease * 7 / 8;  /* default T2 time */

    /* Remember my_ip_addr from OFFER because WinNT server
     * doesn't include it in ACK message.
     */
    my_ip_addr = ((_udp_Socket*)sock)->myaddr = intel (dhcp_in.dh_yiaddr);
    TRACE (("my_ip_addr = %s\n", INET_NTOA(my_ip_addr)));
    send_timeout = set_timeout (100);
    DHCP_state = DHCP_state_REQUESTING;
  }
  else if (event == EVENT_SEND_TIMEOUT)  /* retransmit timeout */
  {
    DHCP_state = DHCP_state_INIT;
    (*DHCP_state) (event);
  }
}

/**
 * DHCP state machine: state INIT.
 * - Awaits SEND-TIMEOUT, send discover and enter SELECTING state.
 */
static void DHCP_state_INIT (int event)
{
  if (event == EVENT_SEND_TIMEOUT)
  {
    discover_loops++;
    exchange_id = set_timeout (0);       /* random exchange ID */

    TRACE (("Sending DHCP discover (%d)\n", discover_loops));

    send_timeout = set_timeout (1000 * dhcp_timeout);
    DHCP_discover();
    DHCP_state = DHCP_state_SELECTING;
  }
}

#ifdef NOT_USED
/**
 * DHCP state machine: Combined BOOT and BOOTING state.
 */
static void DHCP_state_REBOOTING (int event)
{
  if (event == EVENT_ACK)
  {
    DHCP_state = DHCP_state_BOUND;
  }
  else if (event == EVENT_NAK)
  {
    send_timeout = set_timeout (Random(4000,6000));
    DHCP_state = DHCP_state_INIT;
  }
}
#endif

/**
 * DHCP state machine: the event driver.
 */
static void W32_CALL dhcp_fsm (void)
{
  if (sock_dataready(sock))
  {
    int len = sock_fastread (sock, (BYTE*)&dhcp_in, sizeof(dhcp_in));

    if (len >= DHCP_MIN_SIZE &&                 /* packet large enough */
        dhcp_in.dh_op  == BOOTP_REPLY &&        /* got a BOOTP reply */
        dhcp_in.dh_xid == dhcp_out.dh_xid &&    /* got our exchange ID */
        !memcmp(dhcp_in.dh_chaddr, _eth_addr,   /* correct hardware addr */
                sizeof(eth_address)))
    {
      if (DHCP_is_ack())
      {
        TRACE (("%s/ACK: ", state_name()));
        (*DHCP_state) (EVENT_ACK);
      }
      else if (DHCP_is_nack())
      {
        TRACE (("%s/NAK: ", state_name()));
        (*DHCP_state) (EVENT_NAK);
      }
      else
      {
        TRACE (("%s/OFFER: ", state_name()));
        (*DHCP_state) (EVENT_OFFER);
      }
    }
  }

  if (chk_timeout(send_timeout))
  {
    send_timeout = 0UL;
    TRACE (("%s/SEND_TIMEOUT: ", state_name()));
    (*DHCP_state) (EVENT_SEND_TIMEOUT);
  }

  if (renewal_timeout && time(NULL) >= renewal_timeout)
  {
    renewal_timeout = 0UL;
    TRACE (("%s/T1_TIMEOUT: ", state_name()));
    (*DHCP_state) (EVENT_T1_TIMEOUT);
  }

  if (rebind_timeout && time(NULL) >= rebind_timeout)
  {
    rebind_timeout = 0UL;
    TRACE (("%s/T2_TIMEOUT: ", state_name()));
    (*DHCP_state) (EVENT_T2_TIMEOUT);
  }
}

/**
 * Our first time DHCP handler.
 * Called if:
 *  - we don't have a WATTCP.CFG file
 *  - or we specified "MY_IP = DHCP".
 *  - or reading a previous W32DHCP.TMP file with transient config failed.
 *  - or lease times call for a renew or rebind.
 * It doesn't hurt that it is blocking.
 */
int DHCP_do_boot (void)
{
  int save_mtu = _mtu;

  if (cfg_read)    /* DHCP_read_config() okay */
     return (1);

  outs (_LANG("Configuring through DHCP.."));
  if (!sock)
     sock = dhcp_open (NULL, bcast_flag);
  if (!sock)
     return (0);

  if (DHCP_state != DHCP_state_RENEWING &&
      DHCP_state != DHCP_state_REBINDING)
  {
    my_ip_addr = 0;
    sin_mask   = 0;
  }

  _mtu = ETH_MAX_DATA;
  discover_loops = 0;

  erase_config();           /* delete old configuration */
  DAEMON_ADD (dhcp_fsm);    /* add "background" daemon */

  /* kick start DISCOVER message
   */
  send_timeout = set_timeout (100);

  if (DHCP_state != DHCP_state_RENEWING &&
      DHCP_state != DHCP_state_REBINDING)
     DHCP_state = DHCP_state_INIT;

  while (DHCP_state != DHCP_state_BOUND)
  {
    tcp_tick (NULL);
    if (discover_loops >= max_retries)  /* retries exhaused */
       break;
  }

  got_offer = FALSE;   /* ready for next cycle */
  _mtu = save_mtu;

  if (my_ip_addr)
  {
    cfg_saved = write_config() > 0;
    return (1);
  }
  return (0);
}

/**
 * Add options from a \b DHCP_OPT_OVERLOAD tag to 'extra_options' list.
 */
static void dhcp_options_add (const BYTE *opt, unsigned max)
{
  int   len = 0;
  char *add;
  const BYTE *end = opt + max;

  extra_options.data = (BYTE*) realloc (extra_options.data, max);
  if (!extra_options.data)
     return;

  add = (char*)&extra_options.data + extra_options.size;

  /* Loop over `opt' and append to `add'. Strip away DHCP_OPT_END
   * and DHCP_OPT_PAD options.
   */
  while (opt < end)
  {
    if (*opt == DHCP_OPT_PAD)
       continue;
    if (*opt == DHCP_OPT_END)
       return;

    len = opt[1];
    memcpy (add, opt, len+2);
    add += len + 2;
    opt += len + 2;
    extra_options.size += len + 2;
  }
}

/**
 * Set all timers required by the DHCP state machine.
 */
static void dhcp_set_timers (void)
{
  time_t now = time (NULL);

  if (dhcp_iplease == 0UL)
       lease_timeout = (time_t)-1;
  else lease_timeout = now + dhcp_iplease;

  if (dhcp_renewal == 0UL)
       renewal_timeout = (time_t)-1;
  else renewal_timeout = now + dhcp_renewal;

  if (dhcp_rebind == 0UL)
     rebind_timeout = (time_t)-1;
  else
  {
    rebind_timeout = now + dhcp_rebind;
    if (rebind_timeout == renewal_timeout)
        rebind_timeout += 10;             /* add 10 seconds */
  }
  if (dhcp_iplease == 0UL)
     TRACE (("Infinite lease!!\n"));
}

/**
 * Change my_ip_addr of all tcp/udp sockets.
 */
static void change_ip_addr (void)
{
  _udp_Socket *udp;
  _tcp_Socket *tcp;

  if (my_ip_addr == old_ip_addr)
     return;

#if !defined(USE_UDP_ONLY)
  for (tcp = _tcp_allsocs; tcp; tcp = tcp->next)
      if (tcp->myaddr)
          tcp->myaddr = my_ip_addr;
#endif
  for (udp = _udp_allsocs; udp; udp = udp->next)
      if (udp->myaddr)
          udp->myaddr = my_ip_addr;
}

/**
 * Parse a list of DHCP-request options from config-file.
 * e.g DHCP_REQ_LIST = 1,23,24,28,36
 */
static int set_request_list (char *options)
{
  static BOOL init = FALSE;
  int    num       = 0;
  int    maxreq    = 312 - 27; /* sizeof(dh_opt) - min size of rest */
  BYTE  *list, *start, *tok, *end;
  char  *tok_buf = NULL;

  if (init || (list = calloc(maxreq,1)) == NULL)
     return (0);

  init  = TRUE;
  start = list;
  end   = start + maxreq - 1;
  tok   = (BYTE*) strtok_r (options, ", \t", &tok_buf);

  while (tok && list < end)
  {
    *list = ATOI ((const char*)tok);
    tok   = (BYTE*) strtok_r (NULL, ", \t", &tok_buf);

    /* If request list start with Pad option ("DHCP.REQ_LIST=0"),
     * disable options all-together.
     */
    if (num == 0 && *list == '0')
       break;
    num++;
    list++;
  }

  request_list.data = start;
  request_list.size = num;

#if 0  /* test */
  {
    int i;
    for (i = 0; i < request_list.size; i++)
        printf ("%2d, ", request_list.data[i]);
    puts ("");
  }
#endif
  return (1);
}

/**
 * Append options from 'request_list' to 'opt'.
 */
static BYTE *put_request_list (BYTE *opt, int filled)
{
  size_t size = min (request_list.size, sizeof(dhcp_out.dh_opt)-filled-1);
                                              /* room for DHCP_OPT_END ^ */
  if (size > 0 && request_list.data)
  {
    size = min (size, 255);
    *opt++ = DHCP_OPT_PARAM_REQUEST;
    *opt++ = (BYTE)size;
    memcpy (opt, request_list.data, size);
    opt += size;
  }
  return (opt);
}

/**
 * Parse user-class as specified in config-file. Format is:
 *   "DHCP.USERCLASS = value1,value2,...".
 * Build up user_class.data along the way.
 */
static int set_user_class (const char *value)
{
  static BOOL init = FALSE;
  int    total     = 0;
  int    maxreq    = sizeof(dhcp_out.dh_opt) - 27; /* adjust for rest */
  char  *list, *list_start;
  char  *list_end;
  const char *value_end;

  if (init || (list = (char*)calloc(maxreq,1)) == NULL)
     return (0);

  init = TRUE;
  list_start = list;
  list_end   = list + maxreq - 1;
  value_end  = value + strlen (value);

  while (list < list_end && value < value_end)
  {
    const char *comma = strchr (value, ',');
    int   size;

    if (comma)
         size = comma - value;    /* "xxx,xxx" */
    else size = strlen (value);   /* "xxxx" */
    *list++ = size;
    memcpy (list, value, size);
    list  += size;
    value += size + 1;
    total += size + 1;
  }

  user_class.data = (BYTE*) list_start;
  user_class.size = total;

#if 0  /* test */
  {
    size_t i;
    for (i = 0; i < user_class.size; i++)
    {
      int ch = user_class.data[i];

      if (isprint(ch))
           printf ("%c", ch);
      else printf ("\\x%02X", ch);
    }
    puts ("");
  }
#endif
  return (1);
}

/**
 * Set vendor class option.
 */
static int set_vend_class (const char *value)
{
  /** \todo Support setting vendor class */
  ARGSUSED (value);
  return (0);
}

/*-------------------------------------------------------------------*/

static void (W32_CALL *prev_hook) (const char*, const char*) = NULL;

/* Absolute times for expiry of lease, renewal and rebind
 * read from user (config_func) or transient config file.
 */
static time_t cfg_dhcp_iplease;
static time_t cfg_dhcp_renewal;
static time_t cfg_dhcp_rebind;

/**
 * Parser for DHCP configuration.
 * Matches all "\c DHCP.xx" values from \c WATTCP.CFG file and
 * make appropriate actions.
 */
static void W32_CALL DHCP_cfg_hook (const char *name, const char *value)
{
  static const struct config_table dhcp_cfg[] = {
             { "REQ_LIST", ARG_FUNC,   (void*)set_request_list },
             { "TRACE",    ARG_ATOI,   (void*)&trace_on        },
             { "BCAST",    ARG_ATOI,   (void*)&bcast_flag      },
             { "TIMEOUT",  ARG_ATOI,   (void*)&dhcp_timeout    },
             { "RETRIES",  ARG_ATOI,   (void*)&max_retries     },
             { "ARPCHECK", ARG_ATOI,   (void*)&arp_check_ip    },
             { "HOST",     ARG_RESOLVE,(void*)&dhcp_server     },
             { "USERCLASS",ARG_FUNC,   (void*)set_user_class   },
             { "VENDCLASS",ARG_FUNC,   (void*)set_vend_class   },
             { "CONFIG",   ARG_STRCPY, (void*)&config_file     },
             { NULL,       0,          NULL                    }
           };

  if (!parse_config_table(&dhcp_cfg[0], "DHCP.", name, value) && prev_hook)
     (*prev_hook) (name, value);
}

/**
 * Free allocated memory.
 */
static void W32_CALL DHCP_exit (void)
{
  if (_watt_fatal_error)
     return;

  DO_FREE (user_class.data);
  DO_FREE (extra_options.data);
  DO_FREE (sock);
  if (request_list.data != (BYTE*)&default_request_list)
     DO_FREE (request_list.data);

#if defined(USE_BSD_API)
  syslog_host_name[0] = '\0';
#endif
}

/**
 * Initialises the DHCP config-parser.
 */
void DHCP_init (void)
{
  prev_hook = usr_init;
  usr_init  = DHCP_cfg_hook;
  RUNDOWN_ADD (DHCP_exit, 259);
}

/**
 * Functions called by config-file parser while reading c:/W32DHCP.TMP.
 */
static void set_my_ip (const char *value)
{
  TRACE (("DHCP: using previous address  %s\n", value));
  my_ip_addr = aton (value);
}

static void set_netmask (const char *value)
{
  TRACE (("DHCP: using previous netmask  %s\n", value));
  sin_mask = aton (value);
}

static void set_gateway (const char *value)
{
  router = aton (value);
  if (router)
  {
    _arp_kill_gateways();   /* delete gateways from cfg-file */
    TRACE (("DHCP: using previous gateway  %s\n", value));
    _arp_add_gateway (NULL, router);
  }
  else
    TRACE (("DHCP: previous gateway is 0.0.0.0!\n"));
}

static void set_nameserv (const char *value)
{
  TRACE (("DHCP: using previous nameserv %s\n", value));
  nameserver = aton (value);
  _add_server (&last_nameserver, def_nameservers, DIM(def_nameservers), nameserver);
}

static void set_server (const char *value)
{
  TRACE (("DHCP: using previous server   %s\n", value));
  dhcp_server = aton (value);
}

static void set_domain (const char *value)
{
  TRACE (("DHCP: using previous domain   %s\n", value));
  setdomainname (value, strlen(value)+1);
}

static void set_lease (const char *value)
{
  cfg_dhcp_iplease = ATOL (value);
}

static void set_renew (const char *value)
{
  cfg_dhcp_renewal = ATOL (value);
}

static void set_rebind (const char *value)
{
  cfg_dhcp_rebind = ATOL (value);
}

/**
 * Check timers read from config-file and set state according
 * to following rules:
 *  - now < renewal                     => BOUND
 *  - now > renewal && now < rebind     => RENEWING
 *  - now >= rebind && now < lease-end  => REBINDING
 *  - now >= lease-end                  => INIT-REBOOT
 *
 * \retval FALSE if lease times expired.
 * \retval TRUE otherwise.
 */
static BOOL eval_timers (void)
{
  time_t now = time (NULL);
#if defined(USE_DEBUG)
  char ct_buf[30];
#endif

  TRACE (("DHCP: IP-lease expires  %s", ctime_r(&cfg_dhcp_iplease,ct_buf)));
  TRACE (("DHCP: rebinding expires %s", ctime_r(&cfg_dhcp_rebind,ct_buf)));
  TRACE (("DHCP: renewal expires   %s", ctime_r(&cfg_dhcp_renewal,ct_buf)));

  if (cfg_dhcp_iplease < now)
       dhcp_iplease = DHCP_MIN_LEASE;
  else dhcp_iplease = (DWORD)(cfg_dhcp_iplease - now);

  if (cfg_dhcp_renewal < now)
       dhcp_renewal = dhcp_iplease / 2;
  else dhcp_renewal = (DWORD)(cfg_dhcp_renewal - now);

  if (cfg_dhcp_rebind < now)
       dhcp_rebind = dhcp_iplease * 7 / 8;
  else dhcp_rebind = (DWORD)(cfg_dhcp_rebind  - now);

  discover_loops = 0;

  if (now < cfg_dhcp_renewal)
  {
    TRACE (("DHCP: BOUND\n"));
    DHCP_state = DHCP_state_BOUND;          /* no action, goto BOUND */
    return (TRUE);
  }

  if (now >= cfg_dhcp_renewal && now < cfg_dhcp_rebind)
  {
    TRACE (("DHCP: RENEWING\n"));
    DHCP_state = DHCP_state_BOUND;
    (*DHCP_state) (EVENT_T1_TIMEOUT);       /* goto RENEWING */
    return (FALSE);
  }

  if (now >= cfg_dhcp_rebind && now < cfg_dhcp_iplease)
  {
    TRACE (("DHCP: REBINDING\n"));
    DHCP_state = DHCP_state_RENEWING;
    (*DHCP_state) (EVENT_T2_TIMEOUT);       /* goto REBINDING */
    return (FALSE);
  }
  return (FALSE);
}

/**
 * Open and parse Wattcp transient configuration. i.e.
 * values that must be known between consecutive runs of an Watt application.
 * We should write the configured values to this file, and read back
 * on next run to avoid doing DHCP boot/release for every program.
 */
static const struct config_table transient_cfg[] = {
           { "DHCP.LEASE",    ARG_FUNC,   (void*)set_lease       },
           { "DHCP.RENEW",    ARG_FUNC,   (void*)set_renew       },
           { "DHCP.REBIND",   ARG_FUNC,   (void*)set_rebind      },
           { "DHCP.MY_IP",    ARG_FUNC,   (void*)set_my_ip       },
           { "DHCP.NETMASK",  ARG_FUNC,   (void*)set_netmask     },
           { "DHCP.GATEWAY",  ARG_FUNC,   (void*)set_gateway     },
           { "DHCP.NAMESERV", ARG_FUNC,   (void*)set_nameserv    },
           { "DHCP.SERVER",   ARG_FUNC,   (void*)set_server      },
           { "DHCP.DOMAIN",   ARG_FUNC,   (void*)set_domain      },
           { "DHCP.HOSTNAME", ARG_STRCPY, (void*)hostname        },
#if defined(USE_BSD_API)
           { "DHCP.LOGHOST",  ARG_STRCPY, (void*)&syslog_host_name },
#endif
           { "DHCP.DEF_TTL",  ARG_ATOI,   (void*)&_default_ttl   },
#if !defined(USE_UDP_ONLY)
           { "DHCP.TCP_KEEP", ARG_ATOI,   (void*)&tcp_keep_intvl },
#endif
           { NULL,            0,          NULL                   }
         };

/*
 * Return name of transient config-file.
 */
static const char *get_config_file (void)
{
  if (config_file[0])
     return (config_file);
  return expand_var_str ("$(TEMP)\\W32DHCP.TMP");
}

/*
 * The standard DHCP config eraser.
 */
static void std_erase_config (void)
{
  unlink (get_config_file());
}

/*
 * The standard DHCP config reader.
 */
static int std_read_config (void)
{
  WFILE file;
  const char *fname = get_config_file();

  if (!FOPEN_TXT(file, fname))
  {
    TRACE (("`%s' not found\n", fname));
    return (0);
  }

  prev_hook = usr_init;
  usr_init  = NULL;     /* don't chain to other parsers */
  tcp_parse_file (file, &transient_cfg[0]);
  usr_init = prev_hook;

  FCLOSE (file);
  return (1);
}

/**
 * Write the transient DHCP configuration to file.
 * Append to file if found, else create the file.
 * \note "ifdef USE_BUFFERED_IO" ignored here.
 */
static int std_write_config (void)
{
  char   buf[20];
  FILE  *file;
  time_t tim, now = time (NULL);
  int    rc  = 0;
  const  char *fname = get_config_file();
  char   ct_buf [30];

  if (!FILE_EXIST(fname))  /* file not found, create */
  {
    file = fopen (fname, "w+t");
    if (!file)
       goto fail;

    rc = fprintf (file,
                  "#\n"
                  "# GENERATED FILE. DO NOT EDIT!\n"
                  "#\n"
                  "# DHCP transient configuration; values that must\n"
                  "# be known between consecutive runs of applications.\n"
                  "# Version: %s\n"
                  "#\n", wattcpVersion());
  }
  else
  {
    file = fopen (fname, "at");
    if (!file)
       goto fail;
  }

  rc += fprintf (file, "# This file saved at %.20s\n#\n", ctime_r(&now,ct_buf)+4);

  tim = dhcp_iplease + now;
  rc += fprintf (file, "DHCP.LEASE    = %-20lu # lease expires  %.20s\n",
                 (u_long)tim, ctime_r(&tim,ct_buf)+4);

  tim = dhcp_renewal + now;
  rc += fprintf (file, "DHCP.RENEW    = %-20lu # renew expires  %.20s\n",
                 (u_long)tim, ctime_r(&tim,ct_buf)+4);

  tim = dhcp_rebind + now;
  rc += fprintf (file,
                 "DHCP.REBIND   = %-20lu # rebind expires %.20s\n"
                 "DHCP.MY_IP    = %-20s # assigned ip-address\n",
                 (u_long)tim, ctime_r(&tim,ct_buf)+4, _inet_ntoa(buf,my_ip_addr));

  rc += fprintf (file, "DHCP.NETMASK  = %-20s # assigned netmask\n",
                 _inet_ntoa(buf,sin_mask));

  rc += fprintf (file, "DHCP.GATEWAY  = %-20s # assigned gateway\n",
                 _inet_ntoa(buf,router));

  rc += fprintf (file, "DHCP.NAMESERV = %-20s # assigned nameserver\n",
                 _inet_ntoa(buf,nameserver));

  rc += fprintf (file, "DHCP.SERVER   = %-20s # DHCP server\n",
                 _inet_ntoa(buf,dhcp_server));

  rc += fprintf (file, "DHCP.HOSTNAME = %-20s # assigned hostname\n",
                 hostname);

  rc += fprintf (file, "DHCP.DOMAIN   = %-20s # assigned domain\n",
                 def_domain);

#if defined(USE_BSD_API)
  if (syslog_host_name[0])
     rc += fprintf (file, "DHCP.LOGHOST  = %-20s # assigned syslog host\n",
                    syslog_host_name);
#endif

  rc += fprintf (file, "DHCP.DEF_TTL  = %-20d # default TTL\n",
                 _default_ttl);

#if !defined(USE_UDP_ONLY)
  rc += fprintf (file, "DHCP.TCP_KEEP = %-20d # TCP keepalive interval\n",
                 tcp_keep_intvl);
#endif

  FCLOSE (file);
  return (rc);

fail:
  TCP_CONSOLE_MSG (0, ("Writing %s failed; %s\n", fname, strerror(errno)));
  return (0);
}

/*
 * The user-defined DHCP config eraser.
 */
static void usr_erase_config (void)
{
  (*config_func) (DHCP_OP_ERASE, NULL);
}

static int usr_read_config (void)
{
  struct DHCP_config cfg;

  memset (&cfg, 0, sizeof(cfg));

  if (!(*config_func)(DHCP_OP_READ, &cfg))
     return (0);

  TRACE (("DHCP: using previous address  %s\n", INET_NTOA(cfg.my_ip)));
  my_ip_addr = cfg.my_ip;

  TRACE (("DHCP: using previous netmask  %s\n", INET_NTOA(cfg.netmask)));
  sin_mask = cfg.netmask;

  if (!_arp_have_default_gw())
  {
    TRACE (("DHCP: using previous gateway  %s\n", INET_NTOA(cfg.gateway)));
    router = cfg.gateway;
    _arp_add_gateway (NULL, router);
  }
  else
    TRACE (("DHCP: already have default gateway\n"));

  TRACE (("DHCP: using previous nameserv %s\n", INET_NTOA(cfg.nameserver)));
  nameserver = cfg.nameserver;
  _add_server (&last_nameserver, def_nameservers, DIM(def_nameservers), nameserver);

  TRACE (("DHCP: using previous server   %s\n", INET_NTOA(cfg.server)));
  dhcp_server = cfg.server;

  cfg_dhcp_iplease = cfg.iplease;
  cfg_dhcp_renewal = cfg.renewal;
  cfg_dhcp_rebind  = cfg.rebind;

  _default_ttl = cfg.default_ttl;
#if !defined(USE_UDP_ONLY)
  tcp_keep_intvl = cfg.tcp_keep_intvl;
#endif

  _strlcpy (hostname, cfg._hostname, sizeof(hostname));

  TRACE (("DHCP: using previous domain   %s\n", cfg.domain));
  setdomainname (cfg.domain, strlen(cfg.domain)+1);

#if defined(USE_BSD_API)
  if (cfg.loghost[0])
     _strlcpy (syslog_host_name, cfg.loghost, sizeof(syslog_host_name));
#endif

  return (1);
}

static int usr_write_config (void)
{
  struct DHCP_config cfg;
  time_t             now = time (NULL);

  memset (&cfg, 0, sizeof(cfg));
  cfg.my_ip       = my_ip_addr;
  cfg.netmask     = sin_mask;
  cfg.gateway     = router;
  cfg.nameserver  = nameserver;
  cfg.server      = dhcp_server;
  cfg.iplease     = (DWORD)now + dhcp_iplease;
  cfg.renewal     = (DWORD)now + dhcp_renewal;
  cfg.rebind      = (DWORD)now + dhcp_rebind;
  cfg.default_ttl = _default_ttl;
#if !defined(USE_UDP_ONLY)
  cfg.tcp_keep_intvl = tcp_keep_intvl;
#endif

  _strlcpy (cfg._hostname, hostname, sizeof(cfg._hostname));
  _strlcpy (cfg.domain, def_domain, sizeof(cfg.domain));

#if defined(USE_BSD_API)
  if (syslog_host_name[0])
     _strlcpy (cfg.loghost, syslog_host_name, sizeof(cfg.loghost));
#endif

  return (*config_func) (DHCP_OP_WRITE, &cfg);
}

/**
 * Erase the transient configuration.
 */
static void erase_config (void)
{
  config_func ? usr_erase_config() : std_erase_config();
}

/**
 * Called from watt_sock_init() after "\c WATTCP.CFG" has been parsed.
 * Open and parse transient configuration from 'config_file' or call
 * user-defined function to read transient configuration.
 */
int W32_CALL DHCP_read_config (void)
{
  cfg_read = FALSE;

  if (config_func ? usr_read_config() : std_read_config() > 0)
  {
    sock = dhcp_open (NULL, bcast_flag);
    if (!sock)
       return (0);

    if (eval_timers())
    {
      cfg_read = TRUE;
      dhcp_set_timers();
      return (1);
    }
    TRACE (("DHCP: config too old.\n"));
    sock->udp.myaddr = 0;
    sock->udp.hisaddr = IP_BCAST_ADDR;
  }

  /* Reading config failed or timers expired; must do the whole
   * DHCP configuration when sock_init.c calls DHCP_do_boot().
   */
  return (0);
}

/**
 * Write the transient DHCP configuration, either to file
 * or via user-defined function.
 */
static int write_config (void)
{
  return (config_func ? usr_write_config() : std_write_config());
}

/**
 * Sets up an application hook for doing DHCP operations (DHCP_config_op)
 */
WattDHCPConfigFunc W32_CALL DHCP_set_config_func (WattDHCPConfigFunc fn)
{
  WattDHCPConfigFunc old_fn = config_func;
  config_func = fn;
  return (old_fn);
}
#endif /* USE_DHCP */
