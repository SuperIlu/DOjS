/*!\file winpkt.c
 *
 *  Packet-driver like interface to WinPcap and SwsVpkt.
 *
 *  Copyright (c) 2004-2006 Gisle Vanem <gvanem@yahoo.no>
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

#include <process.h>
#include <arpa/inet.h>

#if defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__)
#include <windowsx.h>

#include "wattcp.h"
#include "strings.h"
#include "misc.h"
#include "timer.h"
#include "profile.h"
#include "sock_ini.h"
#include "language.h"
#include "pcconfig.h"
#include "pcdbug.h"
#include "pcstat.h"
#include "pcpkt.h"

#include "win_dll.h"
#include "winpkt.h"
#include "w32_ndis.h"
#include "packet32.h"
#include "swsvpkt.h"

#if (defined(_DLL) && !defined(_MT)) && !defined(__LCC__)
#error This file must be compiled for threads
#endif

#define AIR_ADAPTER ADAPTER   /* for the time being */

/* Rewritten from <ntddndis.h>
 */
#define ND_PHY_Unspecified   0
#define ND_PHY_WirelessLan   1
#define ND_PHY_CableModem    2
#define ND_PHY_PhoneLine     3
#define ND_PHY_PowerLine     4
#define ND_PHY_DSL           5
#define ND_PHY_FibreChannel  6
#define ND_PHY_1394          7
#define ND_PHY_WirelessWan   8
#define ND_PHY_Native802_11  9
#define ND_PHY_Bluetooth    10
#define ND_PHY_Infiniband   11
#define ND_PHY_WiMax        12
#define ND_PHY_UWB          13
#define ND_PHY_802_3        14
#define ND_PHY_802_5        15
#define ND_PHY_Irda         16
#define ND_PHY_WiredWAN     17
#define ND_PHY_WiredCoWan   18
#define ND_PHY_Other        19

WORD _pktdevclass = PDCLASS_UNKNOWN;
WORD _pkt_ip_ofs  = 0;
BOOL _pktserial   = FALSE;
int  _pkt_errno   = 0;
int  _pkt_rxmode  =  RXMODE_DEFAULT;  /**< active receive mode */
int  _pkt_rxmode0 = -1;               /**< startup receive mode */
int  _pkt_forced_rxmode = -1;         /**< wattcp.cfg receive mode */
char _pktdrvrname   [MAX_VALUELEN+1] = "";
char _pktdrvr_descr [MAX_VALUELEN+1] = "";

struct pkt_info *_pkt_inf = NULL;

/* Determines size of work buffers of NPF kernel and PacketReceivePacket().
 * It doesn't influence the size of thread queue yet.
 */
static int pkt_num_rx_bufs = RX_BUFS;

/* WinPcap/SwsVpkt doesn't keep a count of transmitted packets and errors.
 * So we maintain these counters locally.
 */
static DWORD num_tx_pkt    = 0UL;
static DWORD num_tx_bytes  = 0UL;
static DWORD num_tx_errors = 0UL;
static DWORD num_rx_pkt    = 0UL;
static DWORD num_rx_bytes  = 0UL;
static DWORD pkt_drop_cnt  = 0UL;
static int   pkt_txretries = 2;
static int   thr_realtime  = 0;
static BOOL  thr_stopped   = FALSE;
static BOOL  config_pass_1_done = FALSE;
static char  dump_fname [MAX_PATHLEN] = "$(TEMP)\\winpkt_dump.txt";

#define NdisMediumNull  -1

static const struct search_list logical_media[] = {
     { NdisMediumNull,         "Unknown?" },
     { NdisMedium802_3,        "802.3" },
     { NdisMedium802_5,        "802.5" },
     { NdisMediumFddi,         "FDDI" },
     { NdisMediumWan,          "WAN" },
     { NdisMediumLocalTalk,    "LocalTalk" },
     { NdisMediumDix,          "DIX" },
     { NdisMediumArcnetRaw,    "ArcNet (raw)" },
     { NdisMediumArcnet878_2,  "ArcNet (878.2)" },
     { NdisMediumAtm,          "ATM" },
     { NdisMediumWirelessWan,  "WiFi" },
     { NdisMediumIrda,         "IrDA" },
     { NdisMediumBpc,          "BPC" },       // 11
     { NdisMediumCoWan,        "CoWan" },     // 12
     { NdisMedium1394,         "1394 FW" },   // 13
     { NdisMediumInfiniBand,   "Inf-band" },  // 14
     { NdisMediumTunnel,       "Tunnel" },
     { NdisMediumNative802_11, "Native 802.11" },
     { NdisMediumLoopback,     "Loopback" },
     { NdisMediumWiMAX,        "WiMax" },
     { NdisMediumIP,           "IP" },
     { NdisMediumCHDLC,        "CHDLC" },     // -2 (Custom linktype)
     { NdisMediumPPPSerial,    "PPP-ser" }
    };

static const struct search_list phys_media[] = {
     { ND_PHY_WirelessLan,  "Wireless LAN" },
     { ND_PHY_CableModem,   "Cable"        },
     { ND_PHY_PhoneLine,    "Phone"        },
     { ND_PHY_PowerLine,    "PowerLine"    },
     { ND_PHY_DSL,          "DSL"          },
     { ND_PHY_FibreChannel, "Fibre"        },
     { ND_PHY_1394,         "IEEE1394"     },
     { ND_PHY_WirelessWan,  "Wireless WAN" },
     { ND_PHY_Native802_11, "802.11"       },
     { ND_PHY_Bluetooth,    "Bluetooth"    },
     { ND_PHY_Infiniband,   "Infiniband"   },
     { ND_PHY_WiMax,        "Wi-Max"       },
     { ND_PHY_UWB,          "UWB"          },
     { ND_PHY_802_3,        "802.3"        },
     { ND_PHY_802_5,        "80.25"        },
     { ND_PHY_Irda,         "Irda"         },
     { ND_PHY_WiredWAN,     "Wired WAN"    },
     { ND_PHY_WiredCoWan,   "Wired CoWan"  },
     { ND_PHY_Other,        "Other?"       }
   };

static enum eth_init_result open_winpcap_adapter (const char *name);
static enum eth_init_result open_npcap_adapter (const char *name);
static enum eth_init_result open_win10pcap_adapter (const char *name);
static enum eth_init_result open_airpcap_adapter (const char *name);
static enum eth_init_result open_wanpacket_adapter (const char *name);
static enum eth_init_result open_swsvpkt_adapter (const char *name);

static BOOL get_if_stat_swsvpkt (const struct SwsVpktUsr *usr, BOOL *is_up);
static BOOL get_perm_mac_address (void *mac);
static BOOL get_interface_mtu (DWORD *mtu);
static void show_link_details (void);
static void set_txmode (const char *value);

static void swsvpkt_callback (struct SwsVpktUsr *, const void *, unsigned);
static DWORD __stdcall winpcap_recv_thread (void *arg);

const char *winpkt_trace_func;      /* the function being traced */
const char *winpkt_trace_file;      /* .. and in which source file */
UINT        winpkt_trace_line;      /* .. and at what line */
UINT        winpkt_trace_level = 0;

#if defined(USE_DEBUG)
  static FILE *trace_file;

  void winpkt_trace (const char *fmt, ...)
  {
    static DWORD start_tick = 0UL;
    time_t  now;
    va_list args;

    WATT_ASSERT (config_pass_1_done);

    if (!trace_file)
       return;

    if (!start_tick)
    {
      now = time (NULL);
      fprintf (trace_file, "Trace started at %s", ctime(&now));
      fputs ("dT [ms]  File       Line Function etc.\n---------------------------"
              "----------------------------------------------------------\n", trace_file);
      start_tick = GetTickCount();
    }
    ENTER_CRIT();

    fprintf (trace_file, "%6lu   %-10s %4u %s(): ",
             (u_long)(GetTickCount() - start_tick),
             winpkt_trace_file, winpkt_trace_line, winpkt_trace_func);

    va_start (args, fmt);
    vfprintf (trace_file, fmt, args);
    fflush (trace_file);
    if (ferror(trace_file))
    {
      CONSOLE_MSG (1, ("error writing dump file %s; %s\n",
                   dump_fname, strerror(errno)));
      winpkt_trace_fclose();
    }
    LEAVE_CRIT();
    va_end (args);
  }

  void winpkt_trace_fclose (void)
  {
    if (trace_file && trace_file != stdout)
       fclose (trace_file);
    trace_file = NULL;
  }
#endif

/**
 * WATTCP.CFG parser for "WINPKT." keywords in WATTCP.CFG.
 */
static BOOL parse_config_pass_1 (void)
{
  static const struct config_table pkt_init_cfg[] = {
                   { "WINPKT.DEVICE",   ARG_STRCPY, (void*)&_pktdrvrname },
                   { "WINPKT.DUMPFILE", ARG_STRCPY, (void*)&dump_fname },
                   { "WINPKT.TRACE",    ARG_ATOI,   (void*)&winpkt_trace_level },
                   { "WINPKT.TXRETRIES",ARG_ATOB,   (void*)&pkt_txretries },
                   { "WINPKT.TXMODE",   ARG_FUNC,   (void*)set_txmode },
                   { "WINPKT.RXMODE",   ARG_ATOX_W, (void*)&_pkt_forced_rxmode },
                   { "WINPKT.RXBUFS",   ARG_ATOI,   (void*)&pkt_num_rx_bufs },
                   { "WINPKT.HIGHPRIO", ARG_ATOI,   (void*)&thr_realtime },
                   { NULL,              0,          NULL }
                 };
  const struct config_table *cfg_save = watt_init_cfg;
  void (W32_CALL *init_save) (const char*, const char*) = usr_init;
  int    rc;

  watt_init_cfg = pkt_init_cfg;
  usr_init      = NULL;    /* only pkt_init_cfg[] gets parsed */
  rc = tcp_config (NULL);
  usr_init      = init_save;
  watt_init_cfg = cfg_save;
  config_pass_1_done = 1;
  return (rc > 0);
}

/**
 * Traverse 'adapters_list' and select 1st device with an IPv4
 * address. Assuming that device is a physical adapter we can use.
 */
static BOOL find_winpcap_adapter (char *aname, size_t size)
{
  const ADAPTER_INFO *ai;
  int   i;

  for (ai = PacketGetAdInfo(); ai; ai = ai->Next)
  {
    for (i = 0; i < ai->NNetworkAddresses; i++)
    {
      const npf_if_addr        *if_addr = ai->NetworkAddresses + i;
      const struct sockaddr_in *ip_addr = (const struct sockaddr_in*) &if_addr->IPAddress;

      if (ip_addr->sin_family == AF_INET)
      {
        _strlcpy (aname, ai->Name, size);
        return (TRUE);
      }
    }
  }
  return (FALSE);
}

/*
 * Loop through "\\.\SwsVpkt<0-31>" and return TRUE on 1st available
 * device.
 */
static BOOL find_swsvpkt_adapter (char *aname, size_t size)
{
  char name [sizeof("\\\\.\\SwsVpktXX")];
  BOOL found;
  int  i;

  for (i = 0, found = FALSE; i <= 31 && !found; i++)
  {
    struct SwsVpktUsr         *sw_usr;
    struct SwsVpktOpenInfo     sw_open;
    struct SwsVpktAdapterState sw_state;

    memset (&sw_state,'\0', sizeof(sw_state));
    sprintf (name, "\\\\.\\SwsVpkt%d", i);
    sw_open.uRxBufs = pkt_num_rx_bufs;
    sw_open.pfnRx   = swsvpkt_callback;
    sw_usr = SwsVpktOpen (name, &sw_open);
    if (sw_usr)
    {
      get_if_stat_swsvpkt (sw_usr, &found);
      SwsVpktClose (sw_usr);
    }
  }
  if (found)
     _strlcpy (aname, name, size);
  return (found);
}

/**
 * Search for the first proper WinPcap or SwsVpkt device.
 * Only called if user didn't specify a device in WATTCP.CFG
 * (with "winpkt.device").
 */
static BOOL find_adapter (char *aname, size_t size)
{
  BOOL rc;

  rc = find_winpcap_adapter (aname, size);
  if (rc)
  {
    _eth_winpcap = TRUE;
    return (TRUE);
  }
#if 0  /* \to-do */
  rc = find_npcap_adapter (aname, size);
  if (rc)
  {
    _eth_npcap = TRUE;
    return (TRUE);
  }
  rc = find_win10pcap_adapter (aname, size);
  if (rc)
  {
    _eth_win10pcap = TRUE;
    return (TRUE);
  }
#endif
  rc = find_swsvpkt_adapter (aname, size);
  if (rc)
  {
    _eth_SwsVpkt = TRUE;
    return (TRUE);
  }
#if 0
  rc = find_airpcap_adapter (aname, size);
  if (rc)
  {
    _eth_airpcap = TRUE;
    return (TRUE);
  }
#endif
  return (rc);
}


/*
 * Check receive-mode 'bits' for an illegal bit by turning all
 * legal bits off.
 */
static BOOL legal_recv_mode (WORD bits)
{
  if (bits & RXMODE_OFF)
     bits &= ~RXMODE_OFF;

  if (bits & RXMODE_DIRECT)
     bits &= ~RXMODE_DIRECT;

  if (bits & RXMODE_MULTICAST1)
     bits &= ~RXMODE_MULTICAST1;

  if (bits & RXMODE_MULTICAST2)
     bits &= ~RXMODE_MULTICAST2;

  if (bits & RXMODE_BROADCAST)
     bits &= ~RXMODE_BROADCAST;

  if (bits & RXMODE_PROMISCOUS)
     bits &= ~RXMODE_PROMISCOUS;

  if (bits & RXMODE_ALL_LOCAL)
     bits &= ~RXMODE_ALL_LOCAL;

  return (bits == 0);
}


/**
 * Initialise one of WinPcap, SwsVpkt, AirPcap, Win10Pcap, NPcap or WanPacket
 * and return our MAC address.
 */
int W32_CALL pkt_eth_init (mac_address *mac_addr)
{
  enum eth_init_result rc;
  DWORD thread_id;
  BOOL  is_up;
  char  descr [512];

  if (_watt_is_win9x)  /**< \todo Support Win-9x/ME/CE too */
  {
    (*_printf) (_LANG("Only Win-NT+ is supported.\n"));
    _pkt_errno = PDERR_GEN_FAIL;
    return (WERR_ILL_DOSX);
  }

  if (!_watt_no_config || _watt_user_config_fn)
     parse_config_pass_1();

  _pkt_inf = calloc (sizeof(*_pkt_inf), 1);
  if (!_pkt_inf)
  {
    (*_printf) (_LANG("Failed to allocate DRIVER data.\n"));
    _pkt_errno = PDERR_GEN_FAIL;
    return (WERR_NO_MEM);
  }

#if defined(USE_DEBUG)
  if (winpkt_trace_level > 0 && dump_fname[0])
  {
    const char *file = expand_var_str (dump_fname);

    _strlcpy (dump_fname, file, sizeof(dump_fname));
    if (!strcmp(dump_fname,"-"))
         trace_file = stdout;
    else trace_file = fopen_excl (file, "w+t");
  }
#endif

  _eth_SwsVpkt   = (strnicmp(_pktdrvrname,"\\\\.\\SwsVpkt",11) == 0);
  _eth_airpcap   = (strnicmp(_pktdrvrname,"\\\\.\\airpcap",11) == 0);
  _eth_winpcap   = (strnicmp(_pktdrvrname,"\\Device\\NPF_{",13) == 0);
  _eth_npcap     = (strnicmp(_pktdrvrname,"\\Device\\NPCAP_{",13) == 0);
  _eth_win10pcap = (strnicmp(_pktdrvrname,"\\Device\\WTCAP_A_{",15) == 0);
  _eth_wanpacket = (strnicmp(_pktdrvrname,"\\Device\\NPF_Generic",19) == 0);

  if ((_eth_winpcap || _eth_npcap || _eth_win10pcap) && !PacketInitModule())
  {
    (*_printf) (_LANG("Failed to initialise WinPcap.\n"));
    pkt_release();
    _pkt_errno = PDERR_NO_DRIVER;
    return (WERR_PKT_ERROR);
  }

  if (!_pktdrvrname[0] && !find_adapter(_pktdrvrname,sizeof(_pktdrvrname)))
  {
    const char *problem = "";
    BOOL  env_ok = (getenv("WATTCP.CFG") || getenv("WATTCP_CFG"));

    if (!env_ok)
       problem = "Seems you have not defined a valid %WATTCP.CFG%. Check the INSTALL instructions.\n";
    (*_printf) (_LANG("No WinPcap or SwsVpkt driver found.\n%s"), problem);

    _pkt_errno = PDERR_NO_DRIVER;
    return (WERR_NO_DRIVER);
  }

  CONSOLE_MSG (2, ("device %s\n", _pktdrvrname));

  rc = _eth_SwsVpkt   ? open_swsvpkt_adapter   (_pktdrvrname) :
       _eth_airpcap   ? open_airpcap_adapter   (_pktdrvrname) :
       _eth_wanpacket ? open_wanpacket_adapter (_pktdrvrname) :
       _eth_win10pcap ? open_win10pcap_adapter (_pktdrvrname) :
       _eth_npcap     ? open_npcap_adapter     (_pktdrvrname) :
                        open_winpcap_adapter   (_pktdrvrname);

  if (rc != WERR_NO_ERROR)
  {
    pkt_release();
    return (rc);
  }

  if (_pkt_inf->get_descr_op &&
      (*_pkt_inf->get_descr_op)(_pkt_inf->adapter, descr, sizeof(descr)))
     _strlcpy (_pktdrvr_descr, strrtrim(descr), sizeof(_pktdrvr_descr));

  if (!_pkt_inf->get_if_type_op ||
      !(*_pkt_inf->get_if_type_op)(_pkt_inf->adapter, &_pktdevclass))
  {
    (*_printf) (_LANG("Failed to get interface type.\n"));
    pkt_release();
    return (WERR_PKT_ERROR);
  }

  if (_pkt_inf->get_if_stat_op &&
      (*_pkt_inf->get_if_stat_op) (_pkt_inf->adapter, &is_up) && !is_up)
     (*_printf) (_LANG("Warning: the adapter %s is down\n"), _pktdrvrname);

  switch (_pktdevclass)
  {
    case PDCLASS_TOKEN:
         _pkt_ip_ofs = sizeof(tok_Header);
         break;
    case PDCLASS_ETHER:
         _pkt_ip_ofs = sizeof(eth_Header);
         break;
    case PDCLASS_FDDI:
         _pkt_ip_ofs = sizeof(fddi_Header);
         break;
    case PDCLASS_ARCNET:
         _pkt_ip_ofs = ARC_HDRLEN;
         break;
    default:
         pkt_release();
         (*_printf) (_LANG("WinPkt-ERROR: Unsupported driver class %dh\n"),
                     _pktdevclass);
         _pkt_errno = PDERR_NO_CLASS;
         return (WERR_PKT_ERROR);
  }

  if (!pkt_get_addr(mac_addr))  /* get our MAC address */
  {
    pkt_release();
    return (WERR_PKT_ERROR);
  }

  pktq_init (&_pkt_inf->pkt_queue,
             sizeof(_pkt_inf->rx_buf[0]),   /* RX_SIZE */
             DIM(_pkt_inf->rx_buf),         /* RX_BUFS */
             (char*)&_pkt_inf->rx_buf);

  if (_eth_winpcap || _eth_win10pcap)   /* to-do: do this in 'open_XX_adapter' */
  {
    _pkt_inf->npf_buf_size = RX_SIZE * pkt_num_rx_bufs;
    _pkt_inf->npf_buf = malloc (_pkt_inf->npf_buf_size);
    if (!_pkt_inf->npf_buf)
    {
      (*_printf) (_LANG("Failed to allocate %d byte Rx buffer.\n"),
                  _pkt_inf->npf_buf_size);
      pkt_release();
      _pkt_errno = PDERR_GEN_FAIL;
      return (WERR_NO_MEM);
    }

    PacketSetLoopbackBehavior (_pkt_inf->adapter, NPF_DISABLE_LOOPBACK);
    PacketSetMode (_pkt_inf->adapter, PACKET_MODE_CAPT);
    PacketSetBuff (_pkt_inf->adapter, _pkt_inf->npf_buf_size);
    PacketSetMinToCopy (_pkt_inf->adapter, ETH_MIN);

    /* PacketReceivePacket() blocks until something is received
     */
    PacketSetReadTimeout ((ADAPTER*)_pkt_inf->adapter, 0);
  }

  /* Check Rx-mode from config-file.
   */
  if (_pkt_forced_rxmode != -1)
  {
    _pkt_forced_rxmode &= 0xFFFF;     /* clear bits not set via ARG_ATOX_W */
    if (_pkt_forced_rxmode == 0 || !legal_recv_mode(_pkt_forced_rxmode))
    {
      CONSOLE_MSG (0, ("Illegal Rx-mode (0x%02X) specified. Forcing default mode (0x%02X).\n",
                   _pkt_forced_rxmode, RXMODE_DEFAULT));
      _pkt_forced_rxmode = RXMODE_DEFAULT;
    }
  }

  if (pkt_get_rcv_mode())
     _pkt_rxmode0 = _pkt_rxmode;

  /* Set Rx-mode; default or forced via config.
   */
  if (_pkt_forced_rxmode != -1)
       pkt_set_rcv_mode (_pkt_forced_rxmode);
  else pkt_set_rcv_mode (RXMODE_DEFAULT);

  if (_eth_winpcap || _eth_win10pcap || _eth_airpcap)  /* to-do: do this in 'open_XX_adapter' */
  {
    _pkt_inf->recv_thread = CreateThread (NULL, 2048, winpcap_recv_thread,
                                          NULL, 0, &thread_id);
    if (!_pkt_inf->recv_thread)
    {
      (*_printf) (_LANG("Failed to create receiver thread; %s\n"),
                  win_strerror(GetLastError()));
      pkt_release();
      _pkt_errno = PDERR_GEN_FAIL;
      return (WERR_PKT_ERROR);
    }

#if defined(__CYGWIN__) && 0
    CONSOLE_MSG (2, ("bp->bh_caplen    at ofs: %ld\n", offsetof(struct bpf_hdr, bh_caplen)));
    CONSOLE_MSG (2, ("bp->bh_datalen   at ofs: %ld\n", offsetof(struct bpf_hdr, bh_datalen)));
    CONSOLE_MSG (2, ("bp->bh_bh_hdrlen at ofs: %ld\n", offsetof(struct bpf_hdr, bh_hdrlen)));
#endif

    if (thr_realtime)
       SetThreadPriority (_pkt_inf->recv_thread,
                          THREAD_PRIORITY_TIME_CRITICAL);

    CONSOLE_MSG (2, ("capture thread-id %lu\n", (u_long)thread_id));
  }

#if defined(USE_DEBUG)
  if (winpkt_trace_level >= 3)
  {
    (*_printf) ("link-details:\n");
    show_link_details();
  }
#endif

  return (0);
}

int W32_CALL pkt_release (void)
{
  ADAPTER *adapter;
  DWORD    status = 1;

  if (!_pkt_inf || _watt_fatal_error)
     return (0);

  adapter = (ADAPTER*) _pkt_inf->adapter;

  CONSOLE_MSG (2, ("pkt_release(): adapter 0x%" ADDR_FMT "\n",
               ADDR_CAST(adapter)));

  if (adapter && adapter != INVALID_HANDLE_VALUE)
  {
    /* Don't close the adapter before telling the thread about it.
     * Only WinPcap/AirPcap devices have a receiver thread here.
     */
    if (_pkt_inf->recv_thread)
    {
      _pkt_inf->stop_thread = TRUE;
      SetEvent (adapter->ReadEvent);
      Sleep (50);

      GetExitCodeThread (_pkt_inf->recv_thread, &status);
      if (status == STILL_ACTIVE)
           CONSOLE_MSG (2, ("pkt_release(): killing thread.\n"));
      else CONSOLE_MSG (2, ("pkt_release(): thread exit code %lu.\n",
                            (u_long)status));

      if (debug_on >= 2)
      {
        print_thread_times (_pkt_inf->recv_thread);
        print_perf_times();
      }

      TerminateThread (_pkt_inf->recv_thread, 1);
      CloseHandle (_pkt_inf->recv_thread);
    }

    (*_pkt_inf->close_op) ((void*)_pkt_inf->adapter);
    _pkt_inf->adapter = NULL;
  }

  if (_eth_winpcap || _eth_win10pcap)
     DO_FREE (_pkt_inf->npf_buf);

  DO_FREE (_pkt_inf);

  if (_eth_winpcap || _eth_win10pcap)
     PacketExitModule();

#if defined(USE_DEBUG)
  winpkt_trace_fclose();
#endif

  return (int) status;
}

/*
 * Return the MAC address.
 */
int W32_CALL pkt_get_addr (mac_address *eth)
{
  mac_address mac;

  if (get_perm_mac_address(&mac))
  {
    memcpy (eth, &mac, sizeof(*eth));
    return (1);
  }
  CONSOLE_MSG (2, ("Failed to get our MAC address; %s\n",
               win_strerror(GetLastError())));
  return (0);
}

int W32_CALL pkt_set_addr (const void *eth)
{
  ARGSUSED (eth);
  return (0);
}

/* Return TRUE is adapter is up.
 */
BOOL W32_CALL pkt_is_init (void)
{
  return (_pkt_inf != NULL);
}

/*
 * Check 'my_ip' against the IPv4 address(es) Winsock is using for
 * this adapter. Only applies to WinPcap adapters.
 */
BOOL W32_CALL pkt_check_address (DWORD my_ip)
{
  const ADAPTER_INFO *ai;
  int   i;

  if (!_pkt_inf || !_pkt_inf->adapter_info)
     return (TRUE);

  ai = (const ADAPTER_INFO*) _pkt_inf->adapter_info;

  for (i = 0; i < ai->NNetworkAddresses; i++)
  {
    const npf_if_addr        *if_addr = ai->NetworkAddresses + i;
    const struct sockaddr_in *ip_addr = (const struct sockaddr_in*) &if_addr->IPAddress;

    if (ntohl(ip_addr->sin_addr.s_addr) == my_ip)
       return (FALSE);
  }
  return (TRUE);  /* Okay, no possible conflict */
}


int W32_CALL pkt_buf_wipe (void)
{
  if (_pkt_inf)
     pktq_clear (&_pkt_inf->pkt_queue);
  return (1);
}

int W32_CALL pkt_waiting (void)
{
  if (_pkt_inf)
     return pktq_queued (&_pkt_inf->pkt_queue);
  return (-1);
}

void W32_CALL pkt_free_pkt (const void *pkt)
{
  struct pkt_ringbuf *q;
  const  char *q_tail;
  int    delta;

  if (!_pkt_inf || !pkt)
     return;

  q = &_pkt_inf->pkt_queue;
  pkt_drop_cnt = q->num_drop;

  q_tail = (const char*)pkt - _pkt_ip_ofs - RX_ELEMENT_HEAD_SIZE;
  delta  = q_tail - pktq_out_buf (q);
  if (delta)
  {
    CONSOLE_MSG (0, ("%s: freeing illegal packet 0x%p, delta %d.\n",
                     __FILE__, pkt, delta));
    pktq_clear (q);
  }
  else
    pktq_inc_out (q);
}

/**
 * Select async (overlapping) tx-mode. NOT YET.
 */
static void set_txmode (const char *value)
{
  ARGSUSED (value);
}

/**
 * Our low-level WinPcap capture thread.
 *
 * Loop waiting for packet(s) in PacketReceivePacket(). In
 * '_pkt_release()' the receive event-handle (adapter->Readvent) is
 * set. This causes 'WaitForSingleObject()' in 'PacketReceivePacket()'
 * to return. It is vital that the ADAPTER object isn't deallocated
 * before 'PacketReceivePacket()' returns.
 *
 * The return-value is > 0 if thread exited on it's own. Otherwise
 * it is 0  (from 'GetExitCodeThread()').
 *
 * \note: If multiple threads are using Watt-32 function, there could
 *   be a race contention here. Maybe 'ENTER_CRIT()' should be moved
 *   further up? E.g. if 'gethostbyname()' is called in one side-thread
 *   and 'select()' is called in the main-thread, this could cause race
 *   hazards in several places. Need to test this carefully. Ref. the
 *  '_WINDOWS_NSL' code in Lynx/Win32.
 *
 * \todo: Take the 'adapter' info from 'arg' instead of using the global
 *       '_pkt_inf' structure.
 */
static DWORD __stdcall winpcap_recv_thread (void *arg)
{
  int rc = 0;

  while (1)
  {
    const struct bpf_hdr *bp;
    BYTE  *pkt, *pkt_end;
    size_t pkt_len, cap_len, hdr_len;
    UINT   total_len, chunk;

    if (!_pkt_inf || _pkt_inf->stop_thread)  /* this signals closure */
    {
      rc = 1;
      break;
    }

    pkt_len = _pkt_inf->npf_buf_size;
    pkt     = _pkt_inf->npf_buf;

    total_len = (*_pkt_inf->recv_op) (_pkt_inf->adapter, pkt, pkt_len);
    if (total_len <= sizeof(*bp))
    {
      rc = 2;
      break;
    }

    ENTER_CRIT();

    for (pkt_end = pkt + total_len, chunk = 1;
         pkt < pkt_end;
         pkt += Packet_WORDALIGN(cap_len+hdr_len), chunk++)
    {
      struct pkt_ringbuf    *q;

      q  = &_pkt_inf->pkt_queue;
      bp = (const struct bpf_hdr*) pkt;

      cap_len = bp->bh_caplen;
      hdr_len = bp->bh_hdrlen;

      CONSOLE_MSG (2, ("winpcap_recv_thread(): total_len %d, "
                       "chunk %d, cap_len %d, hdr_len %d, in-idx %d\n",
                       total_len, chunk, (int)cap_len, (int)hdr_len,
                       q->in_index));

      num_rx_bytes += total_len - sizeof(*bp);
      num_rx_pkt++;

      if (cap_len > ETH_MAX)
      {
        _pkt_inf->error = "Large size";
        STAT (macstats.num_too_large++);
      }
      else if (pktq_in_index(q) == q->out_index)  /* queue is full, drop it */
              q->num_drop++;
      else
      {
        struct pkt_rx_element *head;
        int    pad_len, buf_max;

        head = (struct pkt_rx_element*) pktq_in_buf (q);
        memcpy (head->rx_buf, pkt + hdr_len, cap_len);

        /* The NPF.SYS timestamp is very unrealiable. So we simply use
         * QueryPerformanceCounter() as it is now.
         */
        head->tstamp_put = win_get_perf_count();
        head->rx_length  = cap_len;

        /* zero-pad up to ETH_MAX (don't destroy marker at end)
         */
        buf_max = q->buf_size - RX_ELEMENT_HEAD_SIZE - 4;
        pad_len = min (ETH_MAX, buf_max - cap_len);
        if (pad_len > 0)
           memset (&head->rx_buf[cap_len], 0, pad_len);
        pktq_inc_in (q);
      }
    }
    LEAVE_CRIT();
  }

  CONSOLE_MSG (2, ("winpcap_recv_thread(): rc %d\n", rc));
  fflush (stdout);
  ARGSUSED (arg);
  thr_stopped = TRUE;
  return (rc);
}

/**
 * Our low-level SwsVpkt receive callback.
 */
static void swsvpkt_callback (struct SwsVpktUsr *usr, const void *buf, unsigned len)
{
  struct pkt_ringbuf    *q;
  struct pkt_rx_element *head;

  q = &_pkt_inf->pkt_queue;

  ENTER_CRIT();

  CONSOLE_MSG (2, ("swsvpkt_callback(): len %d, in-idx %d\n", len, q->in_index));

  if (len > ETH_MAX)
  {
    _pkt_inf->error = "Large size";
    STAT (macstats.num_too_large++);
  }
  else if (pktq_in_index(q) == q->out_index)  /* queue is full, drop it */
  {
    q->num_drop++;
    CONSOLE_MSG (2, ("swsvpkt_callback(): num_drop %lu\n", (u_long)q->num_drop));
  }
  else
  {
    int pad_len, buf_max;

    head = (struct pkt_rx_element*) pktq_in_buf (q);
    memcpy (head->rx_buf, buf, len);
    head->rx_length  = len;
    head->tstamp_put = win_get_perf_count();

    /* zero-pad up to ETH_MAX (don't destroy marker at end)
     */
    buf_max = q->buf_size - RX_ELEMENT_HEAD_SIZE - 4;
    pad_len = min (ETH_MAX, buf_max - len);
    if (pad_len > 0)
       memset (&head->rx_buf[len], 0, pad_len);
    pktq_inc_in (q);
  }
  ARGSUSED (usr);
  LEAVE_CRIT();
}

/*
 * Poll the tail of queue
 */
struct pkt_rx_element *pkt_poll_recv (void)
{
  struct pkt_ringbuf    *q;
  struct pkt_rx_element *rc = NULL;
  const char  *out;
  BOOL  empty;

  if (!_pkt_inf)
     return (NULL);

#if 0
  {
    DWORD status = 0;

    GetExitCodeThread (_pkt_inf->recv_thread, &status);
    if (status != STILL_ACTIVE && !thr_stopped)
    {
      CONSOLE_MSG (2, ("winpcap_recv_thread() is dead. status: %lXh\n", status));
      _pkt_inf->thread_stopped = 1;
      Sleep (100);
    }
  }
#endif

  ENTER_CRIT();

  q = &_pkt_inf->pkt_queue;
  out = pktq_out_buf (q);
  empty = (q->in_index == q->out_index);

  LEAVE_CRIT();

  if (!empty)
  {
    rc = (struct pkt_rx_element*) out;
    rc->tstamp_get = win_get_perf_count();
  }
  return (rc);
}

/*
 * Our WinPcap/SwsVpkt send function.
 * Not sure if partial writes are possible with NPF. Retry if rc != length.
 */
int W32_CALL pkt_send (const void *tx, int length)
{
  int tx_cnt, rc = 0;

  ASSERT_PKT_INF (0);

  PROFILE_START ("pkt_send");

  for (tx_cnt = 1 + pkt_txretries; tx_cnt > 0; tx_cnt--)
  {
#if 1
    WATT_ASSERT (_pkt_inf->send_op);
    rc = (*_pkt_inf->send_op) (_pkt_inf->adapter, tx, length);
    if (rc == length)
       break;
#else
    struct SwsVpktUsr *sws_usr = (struct SwsVpktUsr*)_pkt_inf->adapter;
    const  ADAPTER    *adapter = (const ADAPTER*)_pkt_inf->adapter;

    if (_eth_SwsVpkt ? SwsVpktSend(sws_usr, tx, length) :
    //  _eth_airpcap ? AirpcapWrite(air_adapter, tx, length) :
                       PacketSendPacket(adapter, tx, length))
    {
      rc = length;
      break;
    }
#endif
    STAT (macstats.num_tx_retry++);
  }

  if (rc == length)
  {
    num_tx_pkt++;
    num_tx_bytes += length;
  }
  else
  {
    num_tx_errors++;  /* local copy */
    STAT (macstats.num_tx_err++);
  }
  PROFILE_STOP();
  return (rc);
}

/*
 * Sets the receive mode of the WinPcap interface.
 */
int W32_CALL pkt_set_rcv_mode (int mode)
{
  struct {
    PACKET_OID_DATA oidData;
    DWORD           filter;
  } oid;
  BOOL  rc;
  const ADAPTER *adapter;

  if (!_pkt_inf || _eth_SwsVpkt || _eth_airpcap)
     return (0);

  adapter = (const ADAPTER*) _pkt_inf->adapter;

  memset (&oid, 0, sizeof(oid));
  oid.oidData.Oid    = OID_GEN_CURRENT_PACKET_FILTER;
  oid.oidData.Length = sizeof(oid.filter);
  *(DWORD*)oid.oidData.Data = mode;
  rc = PacketRequest (adapter, TRUE, &oid.oidData);
  if (rc)
       _pkt_rxmode = mode;
  else _pkt_errno = GetLastError();

  CONSOLE_MSG (2, ("pkt_set_rcv_mode(); mode 0x%02X, rc %d; %s\n",
               mode, rc, !rc ? win_strerror(GetLastError()) : "okay"));
  return (rc);
}

/*
 * Gets the receive mode of the WinPcap interface.
 *  \retval !0 Okay  - _pkt_errno = 0, _pkt_rxmode and retval is current mode.
 *  \retval 0  Error - _pkt_errno = GetLastError().
 */
int W32_CALL pkt_get_rcv_mode (void)
{
  struct {
    PACKET_OID_DATA oidData;
    DWORD           filter;
  } oid;
  DWORD mode;
  BOOL  rc;
  const ADAPTER *adapter;

  if (!_pkt_inf || _eth_SwsVpkt || _eth_airpcap)
     return (0);

  adapter = (const ADAPTER*) _pkt_inf->adapter;

  memset (&oid, 0, sizeof(oid));
  oid.oidData.Oid    = OID_GEN_CURRENT_PACKET_FILTER;
  oid.oidData.Length = sizeof(oid.filter);
  rc = PacketRequest (adapter, FALSE, &oid.oidData);
  mode = *(DWORD*)oid.oidData.Data;
  if (rc)
       _pkt_rxmode = mode;
  else _pkt_errno = GetLastError();
  return (rc);
}

static BOOL get_stats_pcap (const ADAPTER *adapter,
                            struct PktStats *stats,
                            struct PktStats *total)
{
  struct {
    PACKET_OID_DATA oidData;
    DWORD           value;
  } oid;
  struct bpf_stat b_stats;
  DWORD  count[4];

  memset (total, 0, sizeof(*total));  /* we don't know yet */
  memset (&count, 0, sizeof(count));

  if (!PacketGetStatsEx(adapter,&b_stats))
     return (FALSE);

  /* Query: OID_GEN_RCV_OK, OID_GEN_RCV_ERROR
   *        OID_GEN_XMIT_OK, OID_GEN_XMIT_ERROR
   */
  memset (&oid, 0, sizeof(oid));
  oid.oidData.Oid    = OID_GEN_RCV_OK;
  oid.oidData.Length = sizeof(oid.value);
  if (!PacketRequest(adapter, FALSE, &oid.oidData))
     goto no_total;
  count[0] = *(DWORD*) &oid.oidData.Data;

  memset (&oid, 0, sizeof(oid));
  oid.oidData.Oid    = OID_GEN_XMIT_OK;
  oid.oidData.Length = sizeof(oid.value);
  if (!PacketRequest(adapter, FALSE, &oid.oidData))
     goto no_total;
  count[1] = *(DWORD*) &oid.oidData.Data;

  memset (&oid, 0, sizeof(oid));
  oid.oidData.Oid    = OID_GEN_RCV_ERROR;
  oid.oidData.Length = sizeof(oid.value);
  if (!PacketRequest(adapter, FALSE, &oid.oidData))
     goto no_total;
  count[2] = *(DWORD*) &oid.oidData.Data;

  memset (&oid, 0, sizeof(oid));
  oid.oidData.Oid    = OID_GEN_XMIT_ERROR;
  oid.oidData.Length = sizeof(oid.value);
  if (!PacketRequest (adapter, FALSE, &oid.oidData))
     goto no_total;
  count[3] = *(DWORD*) &oid.oidData.Data;

no_total:
  total->in_packets  = count[0];
  total->out_packets = count[1];
  total->in_errors   = count[2];
  total->out_errors  = count[3];

  stats->in_packets  = num_rx_pkt;   /* !! b_stats.bs_recv */
  stats->in_bytes    = num_rx_bytes;
  stats->out_packets = num_tx_pkt;
  stats->out_bytes   = num_tx_bytes;
  stats->in_errors   = 0UL;
  stats->out_errors  = num_tx_errors;
  stats->lost        = b_stats.bs_drop + b_stats.ps_ifdrop;
  return (TRUE);
}

static BOOL get_stats_swsvpkt (const struct SwsVpktUsr *usr,
                               struct PktStats *stats,
                               struct PktStats *total)
{
  memset (stats, 0, sizeof(*stats));
  memset (total, 0, sizeof(*total));  /* not known */
  return SwsVpktGetStatistics (usr, stats);
}

/*
 * Return traffic statistics since started (stats) and
 * total since adapter was opened (total).
 *
 * \note 'stats' only count traffic we received and transmitted.
 */
int W32_CALL pkt_get_stats (struct PktStats *stats, struct PktStats *total)
{
  if (!_pkt_inf || !_pkt_inf->get_stats_op)
     return (0);
  return (*_pkt_inf->get_stats_op) (_pkt_inf->adapter,stats,total);
}

DWORD W32_CALL pkt_dropped (void)
{
  if (_pkt_inf)
     return (_pkt_inf->pkt_queue.num_drop);
  return (pkt_drop_cnt);    /* return last known value */
}

int W32_CALL pkt_get_mtu (void)
{
  DWORD MTU;

  if (get_interface_mtu(&MTU))
     return (MTU);
  return (0);
}

const char * W32_CALL pkt_strerror (int code)
{
  ARGSUSED (code);

 /** \todo
  *  Return strings for these:
  *    PDERR_GEN_FAIL
  *    PDERR_NO_DRIVER
  *    PDERR_NO_CLASS
  *    PDERR_NO_MULTICAST
  *    PDERR_NO_SPACE
  *  + GetLastError(). Use win_strerror()
  */
  return (NULL);
}

#if defined(USE_MULTICAST)
/*
 * Don't think we need to support this since we can use promiscous mode
 */
int pkt_get_multicast_list (mac_address *listbuf, int *len)
{
  _pkt_errno = PDERR_NO_MULTICAST;
  *len = 0;
  ARGSUSED (listbuf);
  ARGSUSED (len);
  return (0);
}

int pkt_set_multicast_list (const void *listbuf, int len)
{
  _pkt_errno = PDERR_NO_MULTICAST;
  ARGSUSED (listbuf);
  ARGSUSED (len);
  return (0);
}
#endif


/*
 * Low-level NDIS/Mini-driver functions.
 */
static BOOL get_perm_mac_address (void *mac)
{
  if (!_pkt_inf || !_pkt_inf->get_mac_op)
     return (FALSE);
  memset (mac, '\0', sizeof(mac_address));
  return (*_pkt_inf->get_mac_op) (_pkt_inf->adapter, mac);
}

static BOOL get_if_mtu_pcap (const ADAPTER *adapter, DWORD *mtu)
{
  struct {
    PACKET_OID_DATA oidData;
    DWORD           mtu;
  } oid;

  memset (&oid, 0, sizeof(oid));
  oid.oidData.Oid    = OID_GEN_MAXIMUM_TOTAL_SIZE;
  oid.oidData.Length = sizeof(oid.mtu);

  if (!PacketRequest(adapter, FALSE, &oid.oidData))
     return (FALSE);
  *mtu = *(DWORD*) &oid.oidData.Data;
  return (TRUE);
}

static BOOL get_if_mtu_generic (const void *adapter, DWORD *mtu)
{
  *mtu = ETH_MAX;
  ARGSUSED (adapter);
  return (TRUE);
}

static BOOL get_interface_mtu (DWORD *mtu)
{
  if (!_pkt_inf || !_pkt_inf->get_if_mtu_op)
     return (FALSE);
  return (*_pkt_inf->get_if_mtu_op) (_pkt_inf->adapter, mtu);
}

static BOOL get_if_type_pcap (const ADAPTER *adapter, WORD *type)
{
  struct {
    PACKET_OID_DATA oidData;
    DWORD           link;
  } oid;

  memset (&oid, 0, sizeof(oid));
  oid.oidData.Oid    = OID_GEN_MEDIA_IN_USE;
  oid.oidData.Length = sizeof(oid.link);

  if (!PacketRequest(adapter, FALSE, &oid.oidData))
     return (FALSE);
  *type = *(WORD*) &oid.oidData.Data;
  return (TRUE);
}

static BOOL get_if_type_swsvpkt (const struct SwsVpktUsr *usr, WORD *type)
{
  *type = PDCLASS_ETHER;   /** \todo Fix me */
  ARGSUSED (usr);
  return (TRUE);
}

static BOOL get_if_type_airpcap (const AIR_ADAPTER *adapter, WORD *type)
{
  *type = PDCLASS_ETHER;   /** \todo Fix me */
  ARGSUSED (adapter);
  return (TRUE);
}

/* Query the NIC driver for the adapter description
 */
static BOOL get_descr_pcap (const ADAPTER *adapter, char *buf, size_t max)
{
  struct {
    PACKET_OID_DATA oidData;
    char            descr[512];
  } oid;

  memset (&oid, 0, sizeof(oid));
  oid.oidData.Oid    = OID_GEN_VENDOR_DESCRIPTION;
  oid.oidData.Length = sizeof(oid.descr);

  if (!PacketRequest(adapter, FALSE, &oid.oidData))
     return (FALSE);
  strncpy (buf, (const char*)&oid.oidData.Data, max);
  return (TRUE);
}

static BOOL get_descr_swsvpkt (const struct SwsVpktUsr *usr, char *buf, size_t max)
{
  return SwsVpktGetDescription (usr, buf, max);
}

static BOOL get_if_speed_pcap (const ADAPTER *adapter, DWORD *Mbit_s)
{
  struct {
    PACKET_OID_DATA oidData;
    DWORD           speed;
  } oid;

  memset (&oid, 0, sizeof(oid));
  oid.oidData.Oid    = OID_GEN_LINK_SPEED;
  oid.oidData.Length = sizeof(oid.speed);

  if (!PacketRequest(adapter, FALSE, &oid.oidData))
     return (FALSE);

  *Mbit_s = (*(DWORD*)&oid.oidData.Data) / 10000;
  return (TRUE);
}

static BOOL get_if_speed_airpcap (const AIR_ADAPTER *adapter, DWORD *Mbit_s)
{
  *Mbit_s = 54;        /* fixed at 54MB/s? */
  ARGSUSED (adapter);
  return (TRUE);
}

#if defined(USE_DEBUG)
static BOOL get_interface_speed (DWORD *speed)
{
  if (!_pkt_inf || !_pkt_inf->get_if_speed_op)
     return (FALSE);
  return (*_pkt_inf->get_if_speed_op) (_pkt_inf->adapter, speed);
}

static BOOL get_phys_media (int *media)
{
  struct {
    PACKET_OID_DATA oidData;
    DWORD           media;
  } oid;
  const ADAPTER *adapter;

  if (!_pkt_inf || _eth_SwsVpkt || _eth_airpcap)
     return (FALSE);

  adapter = (const ADAPTER*) _pkt_inf->adapter;
  memset (&oid, 0, sizeof(oid));
  oid.oidData.Oid    = OID_GEN_PHYSICAL_MEDIUM;
  oid.oidData.Length = sizeof(oid.media);

  if (!PacketRequest(adapter, FALSE, &oid.oidData))
     return (FALSE);
  *media = *(int*) &oid.oidData.Data;
  return (TRUE);
}
#endif

static BOOL get_if_stat_pcap (const ADAPTER *adapter, BOOL *is_up)
{
  struct {
    PACKET_OID_DATA oidData;
    DWORD           connected;
  } oid;

  memset (&oid, 0, sizeof(oid));
  oid.oidData.Oid    = OID_GEN_MEDIA_CONNECT_STATUS;
  oid.oidData.Length = sizeof(oid.connected);

  if (!PacketRequest(adapter, FALSE, &oid.oidData))
     return (FALSE);
  *is_up = (*(DWORD*)&oid.oidData.Data == NdisMediaStateConnected);
  return (TRUE);
}

static BOOL get_if_stat_swsvpkt (const struct SwsVpktUsr *usr, BOOL *is_up)
{
  struct SwsVpktAdapterState state;

  memset (&state, '\0', sizeof(state));
  if (!SwsVpktGetAdapterState(usr,&state))
     return (FALSE);

  *is_up = (state.isMediaConnected) || (state.isWan && !state.isWanDown);
  return (TRUE);
}

#ifdef NOT_USED
/*
 * NDIS has the annoying feature (?) of looping packets we send in
 * NDIS_PACKET_TYPE_ALL_LOCAL mode (the default?). Disable it, but
 * doesn't seem to have any effect yet. Maybe need to use a BPF filter?
 *
 * No need when using RXMODE_DEFAULT (9).
 */
static BOOL ndis_set_loopback (BOOL enable)
{
  struct {
    PACKET_OID_DATA oidData;
    DWORD  options;
  } oid;
  const ADAPTER *adapter;
  BOOL  rc;
  DWORD options;
  DWORD gen_oid = OID_GEN_MAC_OPTIONS;
  DWORD opt_bit = NDIS_MAC_OPTION_NO_LOOPBACK;

  if (!_pkt_inf || _eth_SwsVpkt || _eth_airpcap)
     return (FALSE);

  adapter = (const ADAPTER*) _pkt_inf->adapter;
  memset (&oid, 0, sizeof(oid));
  oid.oidData.Oid    = gen_oid;
  oid.oidData.Length = sizeof(oid.options);

  /* Get current MAC/protocol options
   */
  rc = PacketRequest (adapter, FALSE, &oid.oidData);
  options = *(DWORD*) &oid.oidData.Data;

 CONSOLE_MSG (2, ("ndis_set_loopback(); rc %d, options 0x%02lX; %s\n",
              rc, options, !rc ? win_strerror(GetLastError()) : "okay"));
  if (enable)
       options &= ~opt_bit;
  else options |= opt_bit;

  /* Set it back
   */
  memset (&oid, 0, sizeof(oid));
  oid.oidData.Oid    = gen_oid;
  oid.oidData.Length = sizeof(oid.options);
  *(DWORD*) &oid.oidData.Data = options;
  rc = PacketRequest (adapter, TRUE, &oid.oidData);

 CONSOLE_MSG (2, ("ndis_set_loopback(); rc %d; %s\n",
              rc, !rc ? win_strerror(GetLastError()) : "okay"));
  return (rc);
}
#endif /* NOT_USED */


/*
 * Return NDIS version as MSB.LSB.
 */
int W32_CALL pkt_get_api_ver (WORD *ver_p)
{
  BOOL rc = FALSE;

  if (!_pkt_inf)
     return (0);

  if (_eth_SwsVpkt)
  {
    const struct SwsVpktUsr *sw_usr = _pkt_inf->adapter;
    DWORD ver = 0;

    rc = SwsVpktGetNDISversion (sw_usr, &ver); /* This IOCTL always seems to fail */
    if (rc)
       *ver_p = (WORD)ver;
  }
  else if (_eth_airpcap)
  {
  }
  else if (_eth_winpcap || _eth_win10pcap)
  {
    struct {
      PACKET_OID_DATA oidData;
      DWORD           version;
    } oid;
    const ADAPTER *adapter = (const ADAPTER*) _pkt_inf->adapter;

    memset (&oid, 0, sizeof(oid));
    oid.oidData.Length = sizeof(oid.version);
    oid.oidData.Oid    = OID_GEN_DRIVER_VERSION;

    rc = PacketRequest (adapter, FALSE, &oid.oidData);
    if (rc)
    {
      WORD ver = *(WORD*) &oid.oidData.Data;  /* There's only 16-bit valid in version */
      *ver_p = ver;
    }
  }
  return (rc);
}

/*
 * Returns NPF.SYS/SwsVpkt.sys/airpcap.sys version as "major,minor,0,build" or
 * "major.minor.0.build" (8 bits each).
 */
int W32_CALL pkt_get_drvr_ver (WORD *major, WORD *minor, WORD *unused, WORD *build)
{
  const char *ver = (_pkt_inf && _pkt_inf->get_drv_ver_op) ?
                     (*_pkt_inf->get_drv_ver_op)() : NULL;
  if (!ver)
     return (0);

  if (sscanf(ver, "%hu,%hu,%hu,%hu", major, minor, unused, build) == 4 ||
      sscanf(ver, "%hu.%hu.%hu.%hu", major, minor, unused, build) == 4)
     return (1);
  return (0);
}

/*
 * Returns name of driver; NPF.SYS/SwsVpkt.sys/airpcap.sys etc.
 */
const char * W32_CALL pkt_get_drvr_name (void)
{
  if (_pkt_inf)
     return (_pkt_inf->sys_drvr_name);
  return (NULL);
}

/*
 * Returns low-level device-name of driver; "\Device\NPF_{..."
 */
const char * W32_CALL pkt_get_device_name (void)
{
  return (_pktdrvrname);
}

/*
 * Return driver description
 */
const char * W32_CALL pkt_get_drvr_descr (void)
{
  return (_pktdrvr_descr);
}

/*
 * Return driver class
 */
W32_FUNC WORD W32_CALL pkt_get_drvr_class (void)
{
  return (_pktdevclass);
}

/**
 * Open the WinPcap device by 'name'.
 */
static enum eth_init_result open_winpcap_adapter (const char *name)
{
  const ADAPTER *adapter = PacketOpenAdapter (name);

  if (!adapter)
  {
    (*_printf) (_LANG("PacketOpenAdapter (\"%s\") failed:\n  %s.\n"),
                name, win_strerror(GetLastError()));
    _pkt_errno = PDERR_NO_DRIVER;
    return (WERR_NO_DRIVER);
  }

  _pkt_inf->adapter         = adapter;
  _pkt_inf->adapter_info    = PacketFindAdInfo (name);
  _pkt_inf->init_op         = PacketInitModule;
  _pkt_inf->close_op        = (func_close)   PacketCloseAdapter;
  _pkt_inf->send_op         = (func_send)    PacketSendPacket;
  _pkt_inf->recv_op         = (func_recv)    PacketReceivePacket;
  _pkt_inf->get_mac_op      = (func_get_mac) PacketGetMacAddress;
  _pkt_inf->get_stats_op    = (func_get_stats)    get_stats_pcap;
  _pkt_inf->get_if_stat_op  = (func_get_if_stat)  get_if_stat_pcap;
  _pkt_inf->get_if_type_op  = (func_get_if_type)  get_if_type_pcap;
  _pkt_inf->get_if_speed_op = (func_get_if_speed) get_if_speed_pcap;
  _pkt_inf->get_if_mtu_op   = (func_get_if_mtu)   get_if_mtu_pcap;
  _pkt_inf->get_descr_op    = (func_get_descr)    get_descr_pcap;
  _pkt_inf->get_drv_ver_op  = PacketGetDriverVersion;
  _pkt_inf->api_name        = "WinPcap";
  _pkt_inf->sys_drvr_name   = "NPF.sys";

  return (WERR_NO_ERROR);
}

/**
 * Open the NPcap device by 'name'.
 */
static enum eth_init_result open_npcap_adapter (const char *name)
{
  const ADAPTER *adapter = PacketOpenAdapter (name);

  if (!adapter)
  {
    (*_printf) (_LANG("PacketOpenAdapter (\"%s\") failed:\n  %s.\n"),
                name, win_strerror(GetLastError()));
    _pkt_errno = PDERR_NO_DRIVER;
    return (WERR_NO_DRIVER);
  }

  _pkt_inf->adapter         = adapter;
  _pkt_inf->adapter_info    = PacketFindAdInfo (name);
  _pkt_inf->init_op         = PacketInitModule;
  _pkt_inf->close_op        = (func_close)   PacketCloseAdapter;
  _pkt_inf->send_op         = (func_send)    PacketSendPacket;
  _pkt_inf->recv_op         = (func_recv)    PacketReceivePacket;
  _pkt_inf->get_mac_op      = (func_get_mac) PacketGetMacAddress;
  _pkt_inf->get_stats_op    = (func_get_stats)    get_stats_pcap;
  _pkt_inf->get_if_stat_op  = (func_get_if_stat)  get_if_stat_pcap;
  _pkt_inf->get_if_type_op  = (func_get_if_type)  get_if_type_pcap;
  _pkt_inf->get_if_speed_op = (func_get_if_speed) get_if_speed_pcap;
  _pkt_inf->get_if_mtu_op   = (func_get_if_mtu)   get_if_mtu_pcap;
  _pkt_inf->get_descr_op    = (func_get_descr)    get_descr_pcap;
  _pkt_inf->get_drv_ver_op  = PacketGetDriverVersion;
  _pkt_inf->api_name        = "NPcap";
  _pkt_inf->sys_drvr_name   = "NPCAP.sys";

  return (WERR_NO_ERROR);
}

/**
 * Open the Win10Pcap device by 'name'.
 */
static enum eth_init_result open_win10pcap_adapter (const char *name)
{
  const ADAPTER *adapter = PacketOpenAdapter (name);

  if (!adapter)
  {
    (*_printf) (_LANG("PacketOpenAdapter (\"%s\") failed:\n  %s.\n"),
                name, win_strerror(GetLastError()));
    _pkt_errno = PDERR_NO_DRIVER;
    return (WERR_NO_DRIVER);
  }

  _pkt_inf->adapter         = adapter;
  _pkt_inf->adapter_info    = PacketFindAdInfo (name);
  _pkt_inf->init_op         = PacketInitModule;
  _pkt_inf->close_op        = (func_close)   PacketCloseAdapter;
  _pkt_inf->send_op         = (func_send)    PacketSendPacket;
  _pkt_inf->recv_op         = (func_recv)    PacketReceivePacket;
  _pkt_inf->get_mac_op      = (func_get_mac) PacketGetMacAddress;
  _pkt_inf->get_stats_op    = (func_get_stats)    get_stats_pcap;
  _pkt_inf->get_if_stat_op  = (func_get_if_stat)  get_if_stat_pcap;
  _pkt_inf->get_if_type_op  = (func_get_if_type)  get_if_type_pcap;
  _pkt_inf->get_if_speed_op = (func_get_if_speed) get_if_speed_pcap;
  _pkt_inf->get_if_mtu_op   = (func_get_if_mtu)   get_if_mtu_pcap;
  _pkt_inf->get_descr_op    = (func_get_descr)    get_descr_pcap;
  _pkt_inf->get_drv_ver_op  = PacketGetDriverVersion;
  _pkt_inf->api_name        = "Win10Pcap";
  _pkt_inf->sys_drvr_name   = "Win10Pcap.sys";

  return (WERR_NO_ERROR);
}

/**
 * Open the named AirPcap device.
 */
static enum eth_init_result open_airpcap_adapter (const char *name)
{
  const AIR_ADAPTER *adapter;

  /**< \todo */
  (*_printf) ("adapter-name: %s\n", name);
  _pkt_errno = PDERR_NO_DRIVER;
  UNFINISHED();

#if 0
  adapter = AirPcapOpen (name);
  _pkt_inf->adapter    = adapter;
  _pkt_inf->send_op    = AirpcapWrite;
  _pkt_inf->get_mac_op = AirpcapGetMacAddress;
  _pkt_inf->close_op   = AirpcapClose;
#endif
  _pkt_inf->get_if_mtu_op   = (func_get_if_mtu)   get_if_mtu_generic;
  _pkt_inf->get_if_speed_op = (func_get_if_speed) get_if_speed_airpcap;
  _pkt_inf->get_if_type_op  = (func_get_if_type)  get_if_type_airpcap;
//_pkt_inf->get_drv_ver_op  = AirpcapGetDriverVersion;
  _pkt_inf->api_name        = "airpcap";
  _pkt_inf->sys_drvr_name   = "airpcap.sys";

  ARGSUSED (adapter);
  return (WERR_NO_ERROR);
}

/**
 * Open the named WanPacket device.
 */
static enum eth_init_result open_wanpacket_adapter (const char *name)
{
  WAN_ADAPTER *ad;

  /**< \todo */
  (*_printf) ("adapter-name: %s\n", name);

  ad = WanPacketOpenAdapter();
  if (!ad)
  {
    _pkt_errno = PDERR_NO_DRIVER;
    return (WERR_PKT_ERROR);
  }

  UNFINISHED();

#if 0   // !!
  if (ad)
     ad->flags = INFO_FLAG_NDISWAN_ADAPTER;
#endif

  _pkt_inf->get_if_mtu_op  = get_if_mtu_generic;
  _pkt_inf->get_drv_ver_op = NULL;
  _pkt_inf->api_name       = "WanPacket";
  _pkt_inf->sys_drvr_name  = "wanpacket.dll";

  return (WERR_NO_ERROR);
}

/**
 * Open the named SwsVpkt device.
 */
static enum eth_init_result open_swsvpkt_adapter (const char *name)
{
  struct SwsVpktOpenInfo   sw_open;
  const struct SwsVpktUsr *sw_usr;

  sw_open.uRxBufs = pkt_num_rx_bufs;
  sw_open.pfnRx   = swsvpkt_callback;
  sw_usr = SwsVpktOpen (name, &sw_open);
  if (!sw_usr)
  {
    (*_printf) (_LANG("SwsVpktOpen (\"%s\") failed:\n  %s.\n"),
                name, win_strerror(GetLastError()));
    _pkt_errno = PDERR_NO_DRIVER;
    return (WERR_NO_DRIVER);
  }
  _pkt_inf->adapter        = sw_usr;
  _pkt_inf->send_op        = (func_send)        SwsVpktSend;
  _pkt_inf->close_op       = (func_close)       SwsVpktClose;
  _pkt_inf->get_mac_op     = (func_get_mac)     SwsVpktGetMacAddress;
  _pkt_inf->get_descr_op   = (func_get_descr)   get_descr_swsvpkt;
  _pkt_inf->get_stats_op   = (func_get_stats)   get_stats_swsvpkt;
  _pkt_inf->get_if_mtu_op  = (func_get_if_mtu)  get_if_mtu_generic;
  _pkt_inf->get_if_type_op = (func_get_if_type) get_if_type_swsvpkt;
  _pkt_inf->get_if_stat_op = (func_get_if_stat) get_if_stat_swsvpkt;
  _pkt_inf->get_drv_ver_op =                    SwsVpktGetDriverVersion;
  _pkt_inf->api_name       = "SwsVpkt";
  _pkt_inf->sys_drvr_name  = "SwsVpkt.sys";

  return (WERR_NO_ERROR);
}

/*
 * Show some WinPcap adapter details.
 * The IP-address, netmask etc. are what Winsock uses. We ignore
 * them and use what WATTCP.CFG specifies.
 */
static void show_link_details (void)
{
#if defined(USE_DEBUG)
  const ADAPTER_INFO *ai;
  int   i, phy;
  BOOL  is_up;
  DWORD MTU, speed;
  WORD  ver;
  mac_address mac;

  if (!_pkt_inf || !_pkt_inf->adapter_info)
     return;

  ai = (const ADAPTER_INFO*) _pkt_inf->adapter_info;
  if (!ai)
  {
    (*_printf) ("  not available\n");
    return;
  }

  (*_printf) ("  %d network address%s:\n",
              ai->NNetworkAddresses, ai->NNetworkAddresses > 1 ? "es" : "");

  for (i = 0; i < ai->NNetworkAddresses; i++)
  {
    const npf_if_addr        *if_addr = ai->NetworkAddresses + i;
    const struct sockaddr_in *ip_addr = (const struct sockaddr_in*) &if_addr->IPAddress;
    const struct sockaddr_in *netmask = (const struct sockaddr_in*) &if_addr->SubnetMask;
    const struct sockaddr_in *brdcast = (const struct sockaddr_in*) &if_addr->Broadcast;

    (*_printf) ("  IP-addr %s", inet_ntoa(ip_addr->sin_addr));
    (*_printf) (", netmask %s", inet_ntoa(netmask->sin_addr));
    (*_printf) (", broadcast %s\n", inet_ntoa(brdcast->sin_addr));
  }
  if (ai->NNetworkAddresses <= 0)
     (*_printf) ("\n");

  if (get_perm_mac_address(&mac))
     (*_printf) ("  MAC-addr %s, ", MAC_address(&mac));

  if (get_interface_mtu(&MTU))
     (*_printf) ("MTU %lu, ", (u_long)MTU);

  (*_printf) ("link-type %s, ",
              list_lookup(_pktdevclass, logical_media, DIM(logical_media)));

  if (get_phys_media(&phy))
     (*_printf) (" over %s, ", list_lookup(phy, phys_media, DIM(phys_media)));

  if (_pkt_inf->get_if_stat_op &&
      (*_pkt_inf->get_if_stat_op) (_pkt_inf->adapter, &is_up))
     (*_printf) ("%s, ", is_up ? "UP" : "DOWN");

  if (get_interface_speed(&speed))
     (*_printf) ("%luMb/s, ", (u_long)speed);

  if (pkt_get_api_ver(&ver))
     (*_printf) ("NDIS %u.%u", hiBYTE(ver), loBYTE(ver));
  (*_printf) ("\n");
#endif
}
#endif  /* WIN32 || _WIN32 || __CYGWIN__ */

