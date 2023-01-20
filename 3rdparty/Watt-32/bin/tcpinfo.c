/******************************************************************************

  TCPINFO - display configuration info to the screen

  Copyright (C) 1991, University of Waterloo
  portions Copyright (C) 1990, National Center for Supercomputer Applications

  This program is free software; you can redistribute it and/or modify
  it, but you may not sell it.

  This program is distributed in the hope that it will be useful,
  but without any warranty; without even the implied warranty of
  merchantability or fitness for a particular purpose.

      Erick Engelke                   or via E-Mail
      Faculty of Engineering
      University of Waterloo          Erick@development.watstar.uwaterloo.ca
      200 University Ave.,
      Waterloo, Ont., Canada
      N2L 3G1

  Numerous additions by G. Vanem <gvanem@yahoo.no>
  1995-2013

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <io.h>
#include <netdb.h>
#include <tcp.h>

#ifndef _MAX_PATH
#define _MAX_PATH 256
#endif

#ifdef _DEBUG
  #define USE_DEBUG
  #define arp_Header void
  #include "../src/pcarp.h"
#endif

#if defined(__CYGWIN__)
  #define stricmp(s1, s2)   strcasecmp (s1, s2)
#endif

#define loBYTE(w)   (BYTE)((w) & 0xFF)
#define hiBYTE(w)   (BYTE)((WORD)((w) & 0xFF00) >> 8)

#if !defined(WATT32_ON_WINDOWS)
  extern BYTE _pktdevlevel;      /**< Device level */
#endif

/*
 * Fix trouble with multiple defined symbols,
 */
#define perror(str)   perror_s(str)
#define strerror(x)   strerror_s_(x)

#if defined(USE_BSD_API)
  static void BSD_api_stats (void);
#endif

static char buffer [512], buf2 [512];
static int verbose_level = 0;

const char *EtherAddr (const void *eth_adr)
{
  static char buf[20];
  const char *eth = (const char*)eth_adr;

  if (pkt_get_drvr_class() == PDCLASS_ARCNET)
       sprintf (buf, "%02X", eth[0] & 255);
  else sprintf (buf, "%02X:%02X:%02X:%02X:%02X:%02X",
                eth[0] & 255, eth[1] & 255, eth[2] & 255,
                eth[3] & 255, eth[4] & 255, eth[5] & 255);
  return (buf);
}

#if defined(USE_DEBUG)
void dump_gateways (void)
{
  const struct gate_entry *gw;
  int   i, num = _arp_gateways_get (&gw);

  puts ("Gateways         : Gateway IP      Subnet          Subnet mask");

  for (i = 0 ; i < num; i++, gw++)
  {
    char gate[20], snet[20], mask[20];

    strcpy (gate, _inet_ntoa(NULL,gw->gate_ip));
    strcpy (snet, _inet_ntoa(NULL,gw->subnet));
    strcpy (mask, _inet_ntoa(NULL,gw->mask));

    printf ("                 : %-15s %-15s %-15s\n", gate,
            gw->subnet ? snet : "default",
            gw->mask   ? mask : "default");
  }
  if (num == 0)
     puts ("NONE");
  puts ("");
}

void dump_arp_cache (void)
{
  const struct arp_entry *ae;
  int    i, max = _arp_cache_get (&ae);

  printf ("ARP Cache        : IP Address      MAC Address        Type    Timeout\n");

  for (i = 0; i < max; i++, ae++)
  {
    char type[20];

    if (!(ae->flags & ARP_FLG_INUSE))
       continue;

    printf ("                 : %-15s %s  ",
            _inet_ntoa(NULL, ae->ip),
            EtherAddr ((void*)&ae->hardware));

    if (ae->flags & ARP_FLG_FIXED)
       strcpy (type, "fixed  ");
    else if (ae->flags & ARP_FLG_DYNAMIC)
       strcpy (type, "dynamic");
    else if (ae->flags & ARP_FLG_PENDING)
       strcpy (type, "pending");
    else
       sprintf (type, "0x%02X?  ", ae->flags);
    printf ("%s ", type);

    if (ae->expiry)
    {
      long timeout = ae->expiry - set_timeout(0);
      if (timeout < 0)
           puts ("< 0 ms");
      else printf ("%ld ms\n", timeout);
    }
    else
      puts ("N/A");
  }
  puts ("");
}
#endif

const char *pkt_class_name (WORD class)
{
  return (class == PDCLASS_ETHER  ? "Ethernet" :
          class == PDCLASS_TOKEN  ? "Token"    :
          class == PDCLASS_ARCNET ? "Arcnet"   :
          class == PDCLASS_SLIP   ? "SLIP"     :
          class == PDCLASS_AX25   ? "AX25"     :
          class == PDCLASS_FDDI   ? "FDDI"     :
          class == PDCLASS_PPP    ? "PPP"      :
          "Unknown");
}

const char *pkt_driver_ver (void)
{
  static char buf[30];
  WORD   major, minor, unused, build;

  if (!pkt_get_drvr_ver(&major,&minor,&unused,&build))
     return ("unknown");

#if defined(WATT32_ON_WINDOWS)  /* NPF.SYS ver e.g. 3.1.0.22 */
  sprintf (buf, "%u.%u.%u.%u", major, minor, unused, build);
#else
  sprintf (buf, "%d.%02d", major, minor);
#endif
  return (buf);
}

const char *pkt_api_ver (void)
{
  static char buf[10];
  WORD   ver;

  if (pkt_get_api_ver(&ver))
  {
#if defined(WATT32_ON_WINDOWS)   /* NDIS version */
    sprintf (buf, "%u.%u", hiBYTE(ver), loBYTE(ver));
#else
    if (ver >= 255)  /* assumed to be MSB.LSB */
         sprintf (buf, "%u.%02u", hiBYTE(ver), loBYTE(ver));
    else sprintf (buf, "%u", loBYTE(ver));
#endif
    return (buf);
  }
  return strcpy (buf, "?");
}

void dump_pktdrvr_info (void)
{
  mac_address addr;

#if !defined(WATT32_ON_WINDOWS)
  printf ("PKTDRVR Name     : ");
  if (!pkt_is_init())
  {
    puts ("<NO DRIVER>");
    return;
  }

  printf ("%s, version %s, API %s, intr 0x%02X\n"
          "        Class    : %s, level %u",
          pkt_get_device_name(), pkt_driver_ver(), pkt_api_ver(), pkt_get_vector(),
          pkt_class_name(pkt_get_drvr_class()), _pktdevlevel);

  if (_pktdevlevel >= 5)  /* high-performance driver */
  {
    struct PktParameters params;

    memset (&params, 0, sizeof(params));
    pkt_get_params (&params);
    printf (", addr-len %d, MTU %d, MC avail %d\n"
            "        RX bufs  : %d, TX bufs %d, EOI intr %d",
            params.addr_len, params.mtu, params.multicast_avail,
            params.rcv_bufs + 1, params.xmt_bufs + 1, params.int_num);
  }
#else
  printf ("Driver Name      : ");
  if (!pkt_is_init())
  {
    puts ("<NO DRIVER>");
    return;
  }

  printf ("%s\n                 : %s\n"
          "                 : %s %s, NDIS %s, Class %s",
          pkt_get_device_name(), pkt_get_drvr_descr(), pkt_get_drvr_name(),
          pkt_driver_ver(), pkt_api_ver(), pkt_class_name(pkt_get_drvr_class()));
#endif

#if defined(USE_MULTICAST) || defined(USE_IPV6)
  if (!_pktserial)
     printf (", RX mode %d", pkt_get_rcv_mode());
#endif

  pkt_get_addr (&addr);
  printf ("\n\nMAC-address      : %s\n", EtherAddr(&addr));
}

#if defined(WATT32_ON_WINDOWS)
static void dump_win_adapters_info (void)
{
  int save1 = pkt_win_get_verbose_level();
  int save2 = debug_on;

  /* Do not get disturbed by other things while printing adapters information.
   */
  debug_on = 0;

#if !defined(USE_DEBUG)
  _printf = printf;
#endif

  pkt_win_set_verbose_level (verbose_level);

  pkt_win_print_GetIfTable();
  pkt_win_print_GetIfTable2Ex();
  pkt_win_print_GetAdaptersAddresses();
  pkt_win_print_GetAdapterOrderMap();
  pkt_win_print_GetIpNetTable();
  pkt_win_print_GetIpNetTable2();
  pkt_win_print_GetIpForwardTable2();
  pkt_win_print_RasEnumConnections();
  pkt_win_print_WlanEnumInterfaces();
  pkt_win_print_WSALookupServices();

  pkt_win_set_verbose_level (save1);
  debug_on = save2;
}
#endif

void usage (void)
{
  printf (" Usage: tcpinfo [-d | -dd]\n"
          "   -d  sets debug-mode (WATTCP.CFG \"debug_on = 1\")\n"
          "   -dd sets debug-mode (WATTCP.CFG \"debug_on = 2\")\n");
  exit (0);
}

void check_setup (void)
{
  const char *watt_env  = getenv ("WATTCP.CFG");
  const char *watt_root = getenv ("WATT_ROOT");
  char  cfg_name [_MAX_PATH];

  if (!tcp_config_name(cfg_name,sizeof(cfg_name)))
       printf ("Failed to find WATTCP.CFG; %s\n", strerror(errno));
  else printf ("Reading configuration file `%s'\n", cfg_name);

  if (!watt_root)
     puts ("\7Warning: %WATT_ROOT% not set.\n"
           "This is okay if you're not planning to program with Watt-32");
  else
  {
    char path [_MAX_PATH];

    strcpy (path, watt_root);
    strcat (path, "\\.");
    if (access(path,0) != 0)
       printf ("\7Warning: WATT_ROOT incorrectly set: `%s'.\n", watt_root);
  }

  if (watt_root && strchr(watt_root,'/'))
     puts ("\7Warning: %WATT_ROOT% contains forward ('/') slashes.\n"
           "This may break some Makefiles.");

  if (watt_env && strchr(watt_env,'/'))
     puts ("\7Warning: %WATTCP.CFG% contains forward slashes ('/').\n"
           "This will break non-djgpp Watt-32 programs.");
}

int MS_CDECL main (int argc, char **argv)
{
  int  i;
  char name[50];

  if (argc >= 2)
  {
    if (!strncmp(argv[1],"-?",2) || !strncmp(argv[1],"/?",2))
       usage();
    if (!stricmp(argv[1],"-d") || !stricmp(argv[1],"/d"))
       debug_on = verbose_level = 1;
    else if (!stricmp(argv[1],"-dd") || !stricmp(argv[1],"/dd"))
       debug_on = verbose_level = 2;
  }

  check_setup();

  survive_eth   = 1;  /* needed to survive if pkt_eth_init() fails */
  survive_bootp = 1;  /* ditto for BOOTP */
  survive_dhcp  = 1;  /* ditto for DHCP/RARP */
  survive_rarp  = 1;

  if (debug_on)
     dbug_init();

  watt_sock_init (0, 0, 0);

#if defined(USE_IPV6) && !defined(WATT32_ON_WINDOWS)
  _ip6_pkt_init();
#endif

  if (debug_on >= 2)
     puts ("");

  dump_pktdrvr_info();

  if (multihomes)
       printf ("IP Addresses     : %s - %s\n",
               _inet_ntoa (buf2,  _gethostid()),
               _inet_ntoa (buffer,_gethostid()+multihomes));
  else printf ("IP Address       : %s\n", _inet_ntoa(buffer,_gethostid()));

  printf ("Network Mask     : %s\n\n", _inet_ntoa(NULL,sin_mask));

#if defined(USE_DEBUG)
  dump_gateways();
  dump_arp_cache();
#endif

  printf ("Host name        : ");
  if (gethostname(name,sizeof(name)) == 0)
       puts (name);
  else puts ("<NONE>");

  printf ("Domain name      : ");
  if (getdomainname(name,sizeof(name)) == 0)
       puts (name);
  else puts ("<NONE>");

#if 0
  printf ("Cookieserver%c    : ", (last_cookie > 1) ? 's' : ' ');
  if (last_cookie == 0)
     puts ("NONE DEFINED");

  for (i = 0 ; i < last_cookie; i++)
  {
    if (i)
       printf ("                 : ");
    printf ("%s\n", _inet_ntoa(NULL, Cookies[i]));
  }
  puts("");
#endif

  printf ("Nameserver%c      : ", (last_nameserver > 1) ? 's' : ' ');
  if (last_nameserver == 0)
     puts ("NONE DEFINED\n");

  for (i = 0 ; i < last_nameserver; i++)
  {
    unsigned timeout = dns_timeout ? dns_timeout :
                       (unsigned)sock_delay << 2;
    if (i)
       printf ("                 : ");
    printf ("%-15s Timeout %us\n",
            _inet_ntoa(NULL, def_nameservers[i]), timeout);
  }

#if 0
  if (_bootp_on)
  {
    puts("");
    printf ("BOOTP            : Enabled and %s\n", _gethostid() ? "SUCCEEDED" : "FAILED");
    printf ("BOOTP Server     : %s\n",           _inet_ntoa(NULL,_bootp_host));
    printf ("BOOTP Timeout    : %i seconds\n\n", _bootp_timeout);
  }
#if defined(USE_DHCP)
  else if (_dhcp_on)
  {
    puts("");
    printf ("DHCP             : Enabled and %s\n", _gethostid() ? "SUCCEEDED" : "FAILED");
    printf ("DHCP Server      : %s\n",           _inet_ntoa(NULL,DHCP_get_server()));
  }
#endif
#endif

#if defined(USE_IPV6)
  puts ("");
  printf ("IPv6-address     : %s\n", _inet6_ntoa(&in6addr_my_ip));
  printf ("6-to-4 gateway   : %s\n", _inet_ntoa(NULL,icmp6_6to4_gateway));
#endif

  puts ("");
  printf ("Max Seg Size,MSS : %u bytes\n",   _mss);
  printf ("Max Transmit,MTU : %u bytes\n\n", _mtu);

  printf ("TCP timers       : ");

#if defined(USE_UDP_ONLY)
  puts ("TCP not compiled in");
#else
  printf ("Sock delay %us, Inactivity %us, Keep-alive %ds/%ds\n",
          sock_delay, sock_inactive, tcp_keep_idle, tcp_keep_intvl);
  printf ("                 : Open %ums, Close %ums, RST time %ums\n",
          tcp_OPEN_TO, tcp_CLOSE_TO, tcp_RST_TIME);
  printf ("                 : RTO base %ums, RTO add %ums, Retrans %ums\n\n",
          tcp_RTO_BASE, tcp_RTO_ADD, tcp_RETRAN_TIME);
#endif

#if 0
  printf ("_tcp_Socket size : %u bytes\n",   (unsigned)sizeof(_tcp_Socket));
  printf ("_udp_Socket size : %u bytes\n\n", (unsigned)sizeof(_udp_Socket));
#endif

#if defined(USE_BSD_API)
  BSD_api_stats();
#endif

  printf ("Version info     : %s\n", wattcpVersion());
  printf ("Compiler/target  : %s\n", wattcpBuildCC());
  printf ("$(CC) name       : %s\n", wattcpBuildCCexe());
  printf ("$(CFLAGS)        : %s\n", wattcpBuildCflags());
  printf ("Capabilities     : %s\n", wattcpCapabilities());

#if defined(WATT32_ON_WINDOWS)
  if (verbose_level >= 1 || debug_on >= 2)
  {
    fflush (stdout);
    dump_win_adapters_info();
  }
#endif

  return (0);
}


#if defined(USE_BSD_API)
/*
 * Returning -1 means uninitialsed
 */
static int NumHostsEntries (void)
{
  int num = -1;

  while (gethostent())
        num++;
  if (num > -1)
      num++;
  return (num);
}

#if defined(USE_IPV6)
static int NumHosts6Entries (void)
{
  int num = -1;

  while (gethostent6())
        num++;
  if (num > -1)
      num++;
  return (num);
}
#endif

static int NumServEntries (void)
{
  int num = -1;

  while (getservent())
        num++;
  if (num > -1)
      num++;
  return (num);
}

static int NumProtoEntries (void)
{
  int num = -1;

  while (getprotoent())
        num++;
  if (num > -1)
      num++;
  return (num);
}

static int NumNetEntries (void)
{
  int num = -1;

  while (getnetent())
        num++;
  if (num > -1)
      num++;
  return (num);
}

static void BSD_api_stats (void)
{
  const char *name;

  printf ("HOSTS file       : ");
  name = GetHostsFile();
  if (name)
       printf ("%-40s %4d entries\n", name, NumHostsEntries());
  else puts ("<NONE>");

#if defined(USE_IPV6)
  printf ("HOSTS6 file      : ");
  name = GetHosts6File();
  if (name)
       printf ("%-40s %4d entries\n", name, NumHosts6Entries());
  else puts ("<NONE>");
#endif

  printf ("SERVICES file    : ");
  name = GetServFile();
  if (name)
       printf ("%-40s %4d entries\n", name, NumServEntries());
  else puts ("<NONE>");

  printf ("PROTOCOL file    : ");
  name = GetProtoFile();
  if (name)
       printf ("%-40s %4d entries\n", name, NumProtoEntries());
  else puts ("<NONE>");

  printf ("NETWORKS file    : ");
  name = GetNetFile();
  if (name)
       printf ("%-40s %4d entries\n", name, NumNetEntries());
  else puts ("<NONE>");

  printf ("ETHERS file      : ");
  name = GetEthersFile();
  if (name)
       printf ("%-40s %4d entries\n", name, NumEtherEntries());
  else puts ("<NONE>");
  puts ("");
}
#endif  /* USE_BSD_API */

