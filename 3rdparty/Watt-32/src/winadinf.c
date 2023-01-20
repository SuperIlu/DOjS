/*!\file winadinf.c
 *
 *  Functions for getting the details of network adapters under Windows.
 *  Using IPHlpApi Adapter Information (Win32/64).
 *
 *  Copyright (c) 2011 Gisle Vanem <gvanem@yahoo.no>
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
 *
 *  Version
 *
 *  0.5 : July 07, 2011 : G. Vanem - created
 *
 */

#define _SYS_SELECT_H        /* For CygWin's <sys/select.h> */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <wchar.h>

//#define OLE2ANSI           /* Use ASCII from Ole32.dll functions */

/**
 * \todo: check out and maybe use:
 *        WSAIoctl (SIO_GET_INTERFACE_LIST).
 */
#if defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__)

#define INSIDE_WINADINF_C  /* Pull in private stuff in "winadinf.h" */

#include "winadinf.h"
#include "wattcp.h"
#include "pcdbug.h"
#include "packet32.h"
#include "misc.h"
#include "misc_str.h"
#include "run.h"
#include "gettod.h"
#include "timer.h"
#include "win_dll.h"

/*
 * If 'COMPILE_WINADINF_C' was defined in "winadinf.h", then the rest of this file
 * is code for MSVC, OpenWatcom, MinGW/MinGW-w64 or a recent __BORLANDC__ compiler.
 *
 * But Orange-C is not even able to parse 'winadinf.h'.
 */
#if defined(COMPILE_WINADINF_C)

/* If on Windows NT 3.5 or on early Windows 95 betas, this is defined
 */
#ifdef WINNT35COMPATIBLE
#error "Win-NT 3.5 compatible!!??"
#endif

#define NA_STR         "<N/A>"
#define NONE_STR       "<None>"
#define NONE_STR_W     L"<None>"
#define _w32_AF_INET   2
#define _w32_AF_INET6  24

#define PRINT_RAS_ERROR(spaces, ret) \
        (*_printf) ("%s%s(%u): error: %s\n", spaces, __FILE__, __LINE__, ras_strerror(ret))

#define TRACE(level, ...)  do {                                         \
                             if (verbose_level >= level) {              \
                               printf ("%s(%u): ", __FILE__, __LINE__); \
                               printf (__VA_ARGS__);                    \
                             }                                          \
                           } while (0)

#ifndef __in
#define __in
#endif

#ifndef __inout
#define __inout
#endif

#ifndef __in_opt
#define __in_opt
#endif

#ifndef __out
#define __out
#endif

#ifndef __out_opt
#define __out_opt
#endif

#ifndef __deref_out
#define __deref_out
#endif

#ifndef __reserved
#define __reserved
#endif

W32_GCC_PRAGMA (GCC diagnostic push)
W32_GCC_PRAGMA (GCC diagnostic ignored "-Wformat")
W32_GCC_PRAGMA (GCC diagnostic ignored "-Wformat-extra-args")

/**
 * We only need 1 set of function-pointers for each loaded .DLL.
 *
 * \def DEF_FUNC(ret,f,(args))
 *   define the `typedef` and declare the function-pointer for
 *   the function we want to import.
 *   \param ret    the return value type (or `void`)
 *   \param f      the name of the function (without any `"`).
 *   \param (args) the function arguments (as one list).
 */
#define DEF_FUNC(ret,f,args)  typedef ret (WINAPI *func_##f) args; \
                              static func_##f p_##f = NULL

/* From "ws2_32.dll"
 */
DEF_FUNC (int, WSAStartup, (__in    WORD     version,
                            __inout WSADATA *data));

DEF_FUNC (int, WSACleanup, (void));
DEF_FUNC (int, WSAGetLastError, (void));

DEF_FUNC (INT, WSAAddressToStringA,
          (__in      SOCKADDR         *lpsaAddress,
           __in      DWORD             dwAddressLength,
           __in_opt  WSAPROTOCOL_INFO *lpProtocolInfo,
           __inout   CHAR             *lpszAddressString,
           __inout   DWORD            *lpdwAddressStringLength));

DEF_FUNC (INT, WSALookupServiceBeginA,
          (__in  WSAQUERYSETA *lpqsRestrictions,
           __in  DWORD         dwControlFlags,
           __out HANDLE       *lphLookup));

DEF_FUNC (INT, WSALookupServiceEnd,
          (__in HANDLE lphLookup));

DEF_FUNC (INT, WSALookupServiceNextA,
          (__in   HANDLE        hLookup,
          __in    DWORD         dwControlFlags,
          __inout DWORD        *lpdwBufferLength,
          __out   WSAQUERYSETA *lpqsResults));

/* From "iphlpapi.dll"
 */
DEF_FUNC (ULONG, GetAdaptersAddresses,
          (__in    ULONG                 Family,
           __in    ULONG                 Flags,
           __in    void                 *Reserved,
           __out   IP_ADAPTER_ADDRESSES *AdapterAddresses,
           __inout ULONG                *outBufLen));

DEF_FUNC (IP_ADAPTER_ORDER_MAP *, GetAdapterOrderMap, (void));

DEF_FUNC (DWORD, GetIfTable,
          (__out    MIB_IFTABLE *pIfTable,
           __inout  ULONG       *pdwSize,
           __in     BOOL         bOrder));

DEF_FUNC (DWORD, GetIfTable2, (__out MIB_IF_TABLE2 **table));
DEF_FUNC (DWORD, GetIpInterfaceEntry, (__inout MIB_IPINTERFACE_ROW *row));

DEF_FUNC (DWORD, GetIfTable2Ex, (__in    MIB_IF_TABLE_LEVEL level,
                                 __out   MIB_IF_TABLE2    **table));

DEF_FUNC (DWORD, GetIpNetTable,
          (__out    MIB_IPNETTABLE *table,
           __inout  ULONG          *size,
           __in     BOOL            sort));

DEF_FUNC (DWORD, GetIpNetTable2,
          (__in  u_short            family,
           __out MIB_IPNET_TABLE2 **table));

DEF_FUNC (void, FreeMibTable, (__in void *mem));

DEF_FUNC (DWORD, GetIpAddrTable,
          (__out   MIB_IPADDRTABLE *table,
           __inout ULONG           *size,
           __in    BOOL             order));

DEF_FUNC (DWORD, GetBestRoute,
          (__in   DWORD             dest_addr,
           __in   DWORD             source_addr,
           __out  MIB_IPFORWARDROW *best_route));

DEF_FUNC (DWORD, GetBestRoute2,
          (__in_opt NET_LUID             *interface_Luid,
           __in     NET_IFINDEX           interface_index,
           __in     const SOCKADDR_INET  *source_address,
           __in     const SOCKADDR_INET  *destination_address,
           __in     ULONG                 address_sort_options,
           __out    MIB_IPFORWARD_ROW2   *best_route,
           __out    SOCKADDR_INET        *best_source_address));

DEF_FUNC (DWORD, GetIpForwardTable2,
          (__in  WORD                   family,
           __out MIB_IPFORWARD_TABLE2 **table));

DEF_FUNC (BOOL, GetIpForwardEntry2,
          (__inout MIB_IPFORWARD_ROW2 *row));

DEF_FUNC (DWORD, ConvertInterfaceLuidToIndex,
          (__in  const NET_LUID    *luid,
           __out       NET_IFINDEX *index));

DEF_FUNC (DWORD, ConvertInterfaceLuidToNameA,
          (__in  const NET_LUID *luid,
           __out       char     *if_name,
           __in        size_t    length));

DEF_FUNC (DWORD, GetIfEntry, (__inout MIB_IFROW *if_row));

DEF_FUNC (BOOL, GetIpNetworkConnectionBandwidthEstimates,
          (__in  NET_IFINDEX                                    index,
           __in  ADDRESS_FAMILY                                 family,
           __out MIB_IP_NETWORK_CONNECTION_BANDWIDTH_ESTIMATES *bw_estimates));

/* From "rasapi32.dll"
 */
DEF_FUNC (DWORD, RasEnumConnectionsA,
          (__inout RASCONN *lprasconn,
           __inout DWORD   *lpcb,
           __out   DWORD   *lpcConnections));

DEF_FUNC (DWORD, RasGetConnectionStatistics,
          (__in    HRASCONN   hRasConn,
           __inout RAS_STATS *lpStatistics));

DEF_FUNC (DWORD, RasGetErrorStringA,
          (__in  UINT   err,
           __out CHAR  *err_str,
           __in  DWORD  size));

DEF_FUNC (DWORD, RasGetProjectionInfoA,
          (__in    HRASCONN      hrasconn,
           __in    RASPROJECTION rasprojection,
           __out   void         *lpprojection,
           __inout DWORD        *lpcb));

/* From "wlanapi.dll"
 */
DEF_FUNC (DWORD, WlanOpenHandle,
          (__in       DWORD   dwClientVersion,
           __reserved void   *pReserved,
           __out      DWORD  *pdwNegotiatedVersion,
           __out      HANDLE *phClientHandle));

DEF_FUNC (DWORD, WlanCloseHandle,
          (__in       HANDLE dwClientHandle,
           __reserved void  *pReserved));

DEF_FUNC (DWORD, WlanEnumInterfaces,
          (__in        HANDLE                     hClientHandle,
           __reserved  void                      *pReserved,
           __deref_out WLAN_INTERFACE_INFO_LIST **ppInterfaceList));

DEF_FUNC (DWORD, WlanQueryInterface,
          (__in       HANDLE                  hClientHandle,
           __in       const GUID             *pInterfaceGuid,
           __in       WLAN_INTF_OPCODE        OpCode,
           __reserved void                   *pReserved,
           __out      DWORD                  *pdwDataSize,
           __out      void                  **ppData,
           __out_opt  WLAN_OPCODE_VALUE_TYPE *pWlanOpcodeValueType));

DEF_FUNC (DWORD, WlanGetAvailableNetworkList,
          (__in       HANDLE                        hClientHandle,
           __in       const GUID                   *pInterfaceGuid,
           __in       DWORD                         dwFlags,
           __reserved void                         *pReserved,
           __out      WLAN_AVAILABLE_NETWORK_LIST **AvailableNetworks));

DEF_FUNC (DWORD, WlanGetNetworkBssList,
          (__in       HANDLE            hClientHandle,
           __in       const GUID       *pInterfaceGuid,
           __in_opt   const DOT11_SSID *pDot11Ssid,
           __in       DOT11_BSS_TYPE    dot11BssType,
           __in       BOOL              bSecurityEnabled,
           __reserved void             *pReserved,
           __out      WLAN_BSS_LIST   **WlanBssList));

DEF_FUNC (DWORD, WlanReasonCodeToString,
          (__in       DWORD  dwReasonCode,
           __in       DWORD  dwBufferSize,
           __in       WCHAR *pStringBuffer,
           __reserved void  *pReserved));

DEF_FUNC (DWORD, WlanGetInterfaceCapability,
          (__in       HANDLE                      hClientHandle,
           __in       const GUID                 *pInterfaceGuid,
           __reserved void                       *pReserved,
           __out      WLAN_INTERFACE_CAPABILITY **ppCapability));

DEF_FUNC (DWORD, WlanGetSupportedDeviceServices,
          (__in  HANDLE                           hClientHandle,
           __in  const GUID                      *pInterfaceGuid,
           __out WLAN_DEVICE_SERVICE_GUID_LIST2 **svc_guid_list));

DEF_FUNC (void, WlanFreeMemory, (__in void *memory));

/* From "ole32.dll"
 */
DEF_FUNC (int, StringFromGUID2,
          (__in  REFGUID  rguid,
           __out LPOLESTR lpsz,
           __in  int      cchMax));

struct one_addr {
       char addr [MAX_IP6_SZ];
       int  family;  /* AF_INET or AF_INET6 */
     };

typedef char address_buf [MAX_IP6_SZ+1];

W32_DATA int                  _w32_errno;
W32_FUNC unsigned long  W32_CDECL _w32_intel (unsigned long x);
W32_FUNC unsigned short W32_CDECL _w32_intel16 (unsigned short x);
W32_FUNC const char *W32_CALL _w32_inet_ntop (int af, const void *src,
                                              char *dst, size_t size);
/*
 * Some of the values under the Registry key
 *   HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\{4D36E972-E325-11CE-BFC1-08002BE10318}\000x
 */
struct setup_info_st {
       char                 *NetCfgInstanceId;
       char                 *MatchingDeviceId;
       char                 *DriverDate;
       char                 *DriverVersion;
       char                 *ComponentId;
       char                 *DeviceInstanceID;
       ULONGLONG             NetworkInterfaceInstallTimestamp;
       unsigned              IfTypePreStart;      /* 'IF_TYPE_xx' values from <ipifcons.h> */
       struct setup_info_st *next;
     };

/* 50 elements should be plenty
 */
static struct setup_info_st setup_info [50];

static void setup_info_get (void);
static int  setup_info_populate (const char *key_name, struct setup_info_st *info);
static void setup_info_dump (void);
static void setup_info_adapter_print (const char *adapter_name);
static void setup_info_free (void);
static const char *convert_ch_to_freq (int ch);
static const char *get_if_type (DWORD if_type);

static BOOL ipv4_only_iface = TRUE;
static char work_buf [10000];
static int  verbose_level = 0;

static void print_wlan_bss_list (const WLAN_BSS_LIST *bss_list);

#define ADD_VALUE(dll, func)  { NULL, dll, #func, (void**)&p_##func }

static struct LoadTable dyn_funcs2[] = {
                        ADD_VALUE ("ws2_32.dll",   WSAStartup),
                        ADD_VALUE ("ws2_32.dll",   WSACleanup),
                        ADD_VALUE ("ws2_32.dll",   WSAAddressToStringA),
                        ADD_VALUE ("ws2_32.dll",   WSAGetLastError),
                        ADD_VALUE ("ws2_32.dll",   WSALookupServiceBeginA),
                        ADD_VALUE ("ws2_32.dll",   WSALookupServiceEnd),
                        ADD_VALUE ("ws2_32.dll",   WSALookupServiceNextA),
                        ADD_VALUE ("ole32.dll",    StringFromGUID2),
                        ADD_VALUE ("iphlpapi.dll", GetAdaptersAddresses),
                        ADD_VALUE ("iphlpapi.dll", GetAdapterOrderMap),
                        ADD_VALUE ("iphlpapi.dll", GetIfTable),
                        ADD_VALUE ("iphlpapi.dll", GetIfTable2),
                        ADD_VALUE ("iphlpapi.dll", GetIfTable2Ex),
                        ADD_VALUE ("iphlpapi.dll", GetIpInterfaceEntry),
                        ADD_VALUE ("iphlpapi.dll", GetIpNetTable),
                        ADD_VALUE ("iphlpapi.dll", GetIpNetTable2),
                        ADD_VALUE ("iphlpapi.dll", FreeMibTable),
                        ADD_VALUE ("iphlpapi.dll", GetIpAddrTable),
                        ADD_VALUE ("iphlpapi.dll", GetBestRoute),
                        ADD_VALUE ("iphlpapi.dll", GetBestRoute2),
                        ADD_VALUE ("iphlpapi.dll", GetIpForwardTable2),
                        ADD_VALUE ("iphlpapi.dll", GetIpForwardEntry2),
                        ADD_VALUE ("iphlpapi.dll", ConvertInterfaceLuidToIndex),
                        ADD_VALUE ("iphlpapi.dll", ConvertInterfaceLuidToNameA),
                        ADD_VALUE ("iphlpapi.dll", GetIpNetworkConnectionBandwidthEstimates),
                        ADD_VALUE ("rasapi32.dll", RasEnumConnectionsA),
                        ADD_VALUE ("rasapi32.dll", RasGetConnectionStatistics),
                        ADD_VALUE ("rasapi32.dll", RasGetErrorStringA),
                        ADD_VALUE ("rasapi32.dll", RasGetProjectionInfoA),
                        ADD_VALUE ("wlanapi.dll" , WlanOpenHandle),
                        ADD_VALUE ("wlanapi.dll" , WlanCloseHandle),
                        ADD_VALUE ("wlanapi.dll" , WlanEnumInterfaces),
                        ADD_VALUE ("wlanapi.dll" , WlanFreeMemory),
                        ADD_VALUE ("wlanapi.dll" , WlanGetAvailableNetworkList),
                        ADD_VALUE ("wlanapi.dll" , WlanGetNetworkBssList),
                        ADD_VALUE ("wlanapi.dll" , WlanQueryInterface),
                        ADD_VALUE ("wlanapi.dll" , WlanReasonCodeToString),
                        ADD_VALUE ("wlanapi.dll" , WlanGetInterfaceCapability),
                        ADD_VALUE ("wlanapi.dll" , WlanGetSupportedDeviceServices)
                      };

/*
 * Load the above DLLs, set func-ptrs and
 * call the real WSAStartup().
 */
static BOOL load_dlls (void)
{
  BOOL    rc;
  WORD    ver = MAKEWORD (2,2);
  WSADATA data = { 0 };

  rc = load_dynamic_table (dyn_funcs2, DIM(dyn_funcs2));
  if (rc)
  {
    int err = (*p_WSAStartup) (ver, &data); /* Required for WSAAddressToStringA() etc. */
    TRACE (3, "WSAStartup() -> err: %s\n"
              "                ver: %u.%u, high-ver: %u.%u, "
              "description: \"%s\", status: \"%s\"\n",
              err ? win_strerror ((*p_WSAGetLastError)()) : NONE_STR,
              loBYTE(data.wVersion), hiBYTE(data.wVersion),
              loBYTE(data.wHighVersion), hiBYTE(data.wHighVersion),
              data.szDescription, data.szSystemStatus);
    rc = (err == 0);
  }
  TRACE (3, "load_dlls(): %s.\n", rc ? "okay" : "failed");
  return (rc);
}

/*
 * The 'rundown()' (atexit()) handler for this module.
 */
static void W32_CALL pkt_win_exit (void)
{
  setup_info_free();
  convert_ch_to_freq (-1); /* free the 'freq_to_ch_mapping' memory */

  if (p_WSACleanup)
    (*p_WSACleanup)();
  unload_dynamic_table (dyn_funcs2, DIM(dyn_funcs2));
}

/*
 * Main initializer for this module. Do this once:
 *  *) load DLLs.
 *  *) set func-ptrs.
 *  *) call the real WSAStartup().
 */
static BOOL pkt_win_init (void)
{
  static BOOL rc = FALSE;
  static BOOL done = FALSE;

  if (!done)
  {
    setup_info_get();
    if (verbose_level >= 2)
       setup_info_dump();
    rc = load_dlls();
    RUNDOWN_ADD (pkt_win_exit, 300);
  }
  done = TRUE;
  return (rc);
}

/**
 * Search 'list' for 'type' and return it's name.
 * Duplicate it here in-case 'USE_DEBUG' isn't defined for misc.c.
 */
static const char *_list_lookup (DWORD value, const struct search_list *list, int num)
{
  static char buf [sizeof("_IF_TYPE_DIGITAL_WRAPPER_OVERHEAD_CHANNEL")+1]; /* Longest return */

  while (num > 0 && list->name)
  {
    if (list->type == value)
       return (list->name);
    num--;
    list++;
  }
  sprintf (buf, "?%lu", (u_long)value);
  return (buf);
}

#if defined(NOT_USED)
static const struct search_list wsock_err_tab[] = {
                  { 10004, "Call interrupted" },  /* WSAEINTR */
                };

static const char *get_wsock_err (void)
{
  static char buf[100];
  int    err;

  if (!p_WSAGetLastError)
     return ("WS2_32.DLL not loaded");

  err = (*p_WSAGetLastError)();

  SNPRINTF (buf, sizeof(buf), "%s", err ?
            _list_lookup(err, wsock_err_tab, DIM(wsock_err_tab)) : NONE_STR);
  return (buf);
}
#endif

static const char *get_guid_str (const GUID *guid)
{
  char  *out;
  const  BYTE *bytes;
  BYTE   v, hi_nibble, lo_nibble;
  GUID   guid_copy = *guid;
  int    i, j;
  static const char mask[] = "12345678-1234-1234-1234-123456789012";

  /* The GUID is the "data-structure from hell": the 1st 64-bit are on
   * big-endian format. But the rest are little endian. Go figure!
   */
  guid_copy.Data1 = _w32_intel (guid_copy.Data1);
  guid_copy.Data2 = _w32_intel16 (guid_copy.Data2);
  guid_copy.Data3 = _w32_intel16 (guid_copy.Data3);

  /* E.g. guid: {E70F1AA0-AB8B-11CF-8CA3-00805F48A192}
   */
  bytes  = (const BYTE*) &guid_copy;
  out    = work_buf;
  *out++ = '{';

  for (i = j = 0; i < sizeof(guid_copy); i++)
  {
    v = *bytes++;
    lo_nibble = v % 16;
    hi_nibble = v >> 4;
    *out++ = hex_chars_upper [(int)hi_nibble];
    *out++ = hex_chars_upper [(int)lo_nibble];
    if (i == 3 || i == 5 || i == 7 || i == 9)
       *out++ = '-';
    assert (j < sizeof(mask)-1);
  }
  *out++ = '}';
  *out++ = '\0';
  return (work_buf);
}

/*
 * Terminate a string 'buf' if 'ch' is found at the end.
 */
static char *strip_end (char *buf, char ch)
{
  char *p = buf + strlen(buf) - 1;

  if (*p == ch)
     *p = '\0';
   return (buf);
}

/*
 * Return TRUE if block at 'p' (with legnth 'len') contains only '0' bytes.
 */
static BOOL all_zeroes (const void *p, size_t len)
{
  const BYTE *x = (const BYTE*)p;
  size_t      i;

  for (i = 0; i < len; i++, x++)
      if (*x)
         return (FALSE);
  return (TRUE);
}

/*
 * Function that prints the line argument while limiting it
 * to at most 'MAX_CHARS_PER_LINE'. An appropriate number
 * of spaces are added on subsequent lines.
 *
 * Stolen from Wget (main.c) and simplified.
 */
#define MAX_CHARS_PER_LINE 80

static const char *format_line (const char *line, int indent)
{
  char *p, *p_max, *token, *line_dup = strdup (line);
  int   remaining_chars = MAX_CHARS_PER_LINE + indent;

  p     = work_buf;
  p_max = p + sizeof(work_buf) - 1;

  /* We break on commas.
   */
  token = strtok (line_dup, ",");
  while (token && p < p_max)
  {
    /* If a token is much larger than the maximum
     * line length, we print the token on the next line.
     */
    if (remaining_chars <= (int)strlen(token))
    {
      p += sprintf (p, "\n%*c", indent, ' ');
      remaining_chars = MAX_CHARS_PER_LINE - indent;
    }
    p += sprintf (p, "%s,", token);
    remaining_chars -= strlen (token) + 1;  /* account for "," */
    token = strtok (NULL, ",");
  }
  free (line_dup);
  return strip_end (work_buf, ',');
}

/**
 * Return nicely formatted string "xx.xxx.xxx"
 * with thousand separators (left adjusted).
 * Use 8 buffers in round-robin.
 */
static const char *dword_string (DWORD val)
{
  static char buf[8][20];
  static int  idx = 0;
  char   tmp[20];
  char  *rc = buf [idx++];

  if (val < 1000UL)
  {
    sprintf (rc, "%lu", (u_long)val);
  }
  else if (val < 1000000UL)      /* 1E6 */
  {
    sprintf (rc, "%lu.%03lu", (u_long)(val/1000UL), (u_long)(val % 1000UL));
  }
  else if (val < 1000000000UL)   /* 1E9 */
  {
    sprintf (tmp, "%9lu", (u_long)val);
    sprintf (rc, "%.3s.%.3s.%.3s", tmp, tmp+3, tmp+6);
  }
  else                           /* >= 1E9 */
  {
    sprintf (tmp, "%12lu", (u_long)val);
    sprintf (rc, "%.3s.%.3s.%.3s.%.3s", tmp, tmp+3, tmp+6, tmp+9);
  }
  idx &= 7;
  return strltrim (rc);
}

/**
 * Return a number with suffix for a link-speed 32-bit value.
 *
 * Assume the given `buf` is at lest 30 bytes.
 */
static const char *speed_string (DWORD val, char *buf)
{
  char   suffix = '\0';
  double fl_val;

  if (val == 0UL)
     return strcpy (buf, "0 B/s");

  fl_val = (double) (val+1);
  if (fl_val >= 1E9)
  {
    suffix = 'G';
    fl_val /= 1E9;
  }
  else if (fl_val >= 1E6)
  {
    suffix = 'M';
    fl_val /= 1E6;
  }
  else if (fl_val >= 1E3)
  {
    suffix  = 'k';
    fl_val /= 1E3;
  }
  sprintf (buf, "%.0f %cB/s", fl_val, suffix);
  return (buf);
}

#if defined(ON_WIN_VISTA)
/**
 * Return a number with a suitable suffix for a link-speed
 * of a 64-bit value.
 *
 * Assume the given `buf` is at lest 30 bytes.
 */
static const char *speed64_string (ULONG64 val, char *buf)
{
  char   suffix = '\0';
  double fl_val;

  if (val <= 1000000000ULL)
     return speed_string ((DWORD)val, buf);

  fl_val = (double) (val+1);
  if (fl_val >= 1E12)
  {
    suffix = 'T';
    fl_val /= 1E12;
  }
  else if (fl_val >= 1E9)
  {
    suffix = 'G';
    fl_val /= 1E9;
  }
  sprintf (buf, "%.0f %cB/s", fl_val, suffix);
  return (buf);
}
#endif

/*
 * Return string "<days> HH:MM:SS.MS" for a time in milli-seconds.
 */
static const char *duration_string (DWORD msec)
{
  static char buf[60];
  DWORD  sec   = msec / 1000;
  WORD   hours = (WORD) (sec / 3600UL);
  WORD   mins  = (WORD) (sec / 60) - (60 * hours);
  WORD   days  = 0;
  char  *p = buf;

  while (hours >= 24)
  {
    hours -= 24;
    days++;
  }
  if (days > 1)
     p += sprintf (p, "%ud ", days);

  sprintf (p, "%02u:%02u:%02u.%lu", hours, mins, (UINT)(sec % 60UL), (u_long)(msec % sec));
  return (buf);
}

/**
 * Enumerate all "000x" keys under:
 *   HKLM\SYSTEM\CurrentControlSet\Control\Class\{4D36E972-E325-11CE-BFC1-08002BE10318}
 *
 * This is the "System-Defined Device Setup Classes Available to Vendors".
 * Ref:
 *   https://docs.microsoft.com/en-us/windows-hardware/drivers/install/system-defined-device-setup-classes-available-to-vendors
 *
 * \note A "Software Loopback Interface" does not have a key under the KEY_NAME branch.
 */
#define KEY_NAME "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}"

static void setup_info_get (void)
{
  struct setup_info_st *inf, *prev = NULL;
  HKEY   key = NULL;
  int    num;
  DWORD  rc  = RegOpenKeyEx (HKEY_LOCAL_MACHINE, KEY_NAME, 0, KEY_READ, &key);

  memset (&setup_info, '\0', sizeof(setup_info));
  inf = setup_info + 0;

  TRACE (3, "RegOpenKeyEx (HKEY_LOCAL_MACHINE\\%s, KEY_READ): %lu\n", KEY_NAME, rc);

  for (num = 0; rc == ERROR_SUCCESS; num++)
  {
    char  sub_key [700];  /* Full key name for this leaf */
    char  new_key [512];  /* The current leaf we're at now */
    DWORD size = sizeof(new_key);

    rc = RegEnumKeyEx (key, num, new_key, &size, NULL, NULL, NULL, NULL);
    if (rc == ERROR_NO_MORE_ITEMS)
       break;

    SNPRINTF (sub_key, sizeof(sub_key), "%s\\%s", KEY_NAME, new_key);
    TRACE (3, "  RegOpenKeyEx (HKEY_LOCAL_MACHINE\\%s):\n", sub_key);
    if (new_key[0] != '0')
    {
      TRACE (3, "  Ignoring the rest of the leafs: %s\n", new_key);
      break;
    }
    if (num >= DIM(setup_info)-1)
    {
      TRACE (3, "  Too many leafs: %s\n", new_key);
      break;
    }
    if (setup_info_populate(sub_key, inf) > 0)
    {
      inf->next = inf + 1;
      prev = inf;
      inf++;
    }
  }
  if (prev)
     prev->next = NULL;
  if (key)
     RegCloseKey (key);
}

static void setup_info_dump (void)
{
  struct setup_info_st *inf = setup_info + 0;
  int    i;

  (*_printf) ("\nFrom setup_info:\n");
  for (i = 0; inf; inf = inf->next, i++)
  {
    (*_printf) (" %2d: NetCfgInstanceId: %s\n"
                "     DriverDate:       %s\n"
                "     DriverVersion:    %s\n"
                "     ComponentId:      %s\n"
                "     DeviceInstanceID: %s\n"
                "     IfTypePreStart:   %s (%lu)\n"
                "     next:             %p\n\n",
                i,
                inf->NetCfgInstanceId,
                inf->DriverDate,
                inf->DriverVersion,
                inf->ComponentId,
                inf->DeviceInstanceID,
                get_if_type(inf->IfTypePreStart), inf->IfTypePreStart,
                inf->next);
  }
}

/**
 * Walk the 'setup_info' array looking for a 'inf->NetCfgInstanceId == adapter_name'
 * (there can be only one). Print some details for this physical adapter.
 *
 * For a software adapter (like the "Loopback" or a "QoS Packet Scheduler")
 * all these will be `"<unknown>"` or `"SWD\xx"`.
 *
 * For physical adapters, the `DeviceInstanceID` starts with `"PCI\\"` or `"USB\\"`
 */
static void setup_info_adapter_print (const char *adapter_name)
{
  const struct setup_info_st *inf;
  const char  *DriverDate       = "<unknown>";
  const char  *DriverVersion    = "<unknown>";
  const char  *ComponentId      = "<unknown>";
  const char  *DeviceInstanceID = "<unknown>";
  const char  *InstallTimeStamp = "<unknown>";
  unsigned     IfTypePreStart   = 0;

  for (inf = setup_info + 0; inf; inf = inf->next)
  {
    if (!inf->NetCfgInstanceId || strcmp(inf->NetCfgInstanceId, adapter_name))
       continue;

    IfTypePreStart   = inf->IfTypePreStart;
    DriverDate       = inf->DriverDate       ? inf->DriverDate       : "<none>";
    DriverVersion    = inf->DriverVersion    ? inf->DriverVersion    : "<none>";
    ComponentId      = inf->ComponentId      ? inf->ComponentId      : "<none>";
    DeviceInstanceID = inf->DeviceInstanceID ? inf->DeviceInstanceID : "<none>";
    InstallTimeStamp = inf->NetworkInterfaceInstallTimestamp ?
                            ULONGLONG_to_ctime(inf->NetworkInterfaceInstallTimestamp) :
                            "<none>";
    break;
  }
  (*_printf) ("    From setup_info:\n");
  (*_printf) ("      InstallTimeStamp:  %s\n", InstallTimeStamp);
  (*_printf) ("      DriverDate:        %s\n", DriverDate);
  (*_printf) ("      DriverVersion:     %s\n", DriverVersion);
  (*_printf) ("      ComponentId:       %s\n", ComponentId);
  (*_printf) ("      DeviceInstanceID:  %s\n", DeviceInstanceID);
  (*_printf) ("      IfTypePreStart:    %s (%lu)\n", get_if_type(IfTypePreStart), IfTypePreStart);
}

static void setup_info_free (void)
{
  struct setup_info_st *inf;

  for (inf = setup_info + 0 ; inf; inf = inf->next)
  {
    if (inf->NetCfgInstanceId)
       free (inf->NetCfgInstanceId);
    if (inf->MatchingDeviceId)
       free (inf->MatchingDeviceId);
    if (inf->DriverDate)
       free (inf->DriverDate);
    if (inf->DriverVersion)
       free (inf->DriverVersion);
    if (inf->ComponentId)
       free (inf->ComponentId);
    if (inf->DeviceInstanceID)
       free (inf->DeviceInstanceID);
  }
  setup_info[0].next = NULL;
}

static const char *reg_type_name (DWORD type)
{
  return (type == REG_NONE             ? "REG_NONE"             :
          type == REG_SZ               ? "REG_SZ"               :
          type == REG_MULTI_SZ         ? "REG_MULTI_SZ"         :
          type == REG_LINK             ? "REG_LINK"             :
          type == REG_BINARY           ? "REG_BINARY"           :
          type == REG_DWORD            ? "REG_DWORD"            :
          type == REG_QWORD            ? "REG_QWORD"            :
          type == REG_EXPAND_SZ        ? "REG_EXPAND_SZ"        :
          type == REG_RESOURCE_LIST    ? "REG_RESOURCE_LIST"    :
          type == REG_DWORD_BIG_ENDIAN ? "REG_DWORD_BIG_ENDIAN" : "?");
}

static int setup_info_populate (const char *key_name, struct setup_info_st *info)
{
  HKEY  key = NULL;
  DWORD num, rc = RegOpenKeyEx (HKEY_LOCAL_MACHINE, key_name, 0, KEY_READ, &key);
  int   added = 0;

  if (rc != ERROR_SUCCESS)
  {
    (*_printf) ("    Error opening registry key \"HKEY_LOCAL_MACHINE\\%s\", rc=%lu\n",
                key_name, (u_long)rc);
    return (0);
  }

  for (num = 0; rc == ERROR_SUCCESS; num++)
  {
    char   value [512] = "\0";
    char   data [512]  = "\0";
    DWORD  value_size  = sizeof(value);
    DWORD  data_size   = sizeof(data);
    DWORD  type        = REG_NONE;
    DWORD  val32;
    LONG64 val64;
    const char *t_val = "";

    rc = RegEnumValue (key, num, value, &value_size, NULL, &type, (LPBYTE)&data, &data_size);
    if (rc == ERROR_NO_MORE_ITEMS)
       break;

    val32 = *(DWORD*) &data[0];
    val64 = *(LONG64*) &data[0];

    switch (type)
    {
      case REG_SZ:
           TRACE (3, "    %2lu: %-12s %-35s -> %s\n",
                  num, reg_type_name(type),
                  value[0] ? value : "(no value)",
                  data[0]  ? data  : "(no data)");

           if (!strcmp(value, "NetCfgInstanceId"))
           {
             info->NetCfgInstanceId = strdup (data);
             added++;
           }
           else if (!strcmp(value, "MatchingDeviceId"))
           {
             info->MatchingDeviceId = strdup (data);
             added++;
           }
           else if (!strcmp(value, "DriverDate"))
           {
             info->DriverDate = strdup (data);
             added++;
           }
           else if (!strcmp(value, "DriverVersion"))
           {
             info->DriverVersion = strdup (data);
             added++;
           }
           else if (!strcmp(value, "DeviceInstanceID"))
           {
             info->DeviceInstanceID = strdup (data);
             added++;
           }
           else if (!strcmp(value, "ComponentId"))
           {
             info->ComponentId = strdup (data);
             added++;
           }
           break;

      case REG_DWORD:
           if (!strcmp(value, "IfTypePreStart"))
           {
             info->IfTypePreStart = val32;
             added++;
           }
           TRACE (3, "    %2lu: %-12s %-35s -> %lu\n",
                  num, reg_type_name(type), value[0] ? value : "(no value)", (u_long)val32);
           break;

      case REG_QWORD:
           if (!strcmp(value, "NetworkInterfaceInstallTimestamp"))
           {
             info->NetworkInterfaceInstallTimestamp = (ULONGLONG)val64;
             t_val = ULONGLONG_to_ctime (info->NetworkInterfaceInstallTimestamp);
             added++;
           }
           TRACE (3, "    %2lu: %-12s %-35s -> %" S64_FMT " %s\n",
                  num, reg_type_name(type), value[0] ? value : "(no value)", val64, t_val);
           break;

      default:
           TRACE (3, "    %2lu: %-12s %-35s -> <ignored>\n",
                  num, reg_type_name(type), value[0] ? value : "(no value)");
           break;
    }
  }
  TRACE (3, "\n");
  if (key)
     RegCloseKey (key);
  return (added);
}

/*
 * Return a printable address from a 'SOCKADDR_INET*' union.
 * Use 4 buffers in round-robin.
 */
static const char *get_sockaddr_inet_str (const SOCKADDR_INET *sa)
{
  static address_buf abuf[4];
  static int idx = 0;
  char  *rc = abuf [idx++];

  strcpy (rc, "?");

  if (sa->si_family == AF_INET)
     _w32_inet_ntop (_w32_AF_INET, &sa->Ipv4.sin_addr, rc, sizeof(abuf[0]));

#if defined(USE_IPV6)
  else if (sa->si_family == AF_INET6)
     _w32_inet_ntop (_w32_AF_INET6, &sa->Ipv6.sin6_addr, rc, sizeof(abuf[0]));
#endif

  idx &= 3;
  return (rc);
}

/*
 * Print the count of ifaces not printed due to:
 *  1) verbose_level too low.
 *  2) not up
 *  3) filtered; name ends in "-000X".
 */
static int num_ifaces_skipped  = 0;
static int num_ifaces_not_up   = 0;
static int num_ifaces_filtered = 0;

static void print_skipped (void)
{
  int sum = num_ifaces_skipped + num_ifaces_not_up + num_ifaces_filtered;

  if (verbose_level >= 1 && sum > 0)
     (*_printf) ("** Skipped %d interfaces due to verbose level (%d not up, %d filtered)**.\n",
                 sum, num_ifaces_not_up, num_ifaces_filtered);

  num_ifaces_skipped  = 0;
  num_ifaces_not_up   = 0;
  num_ifaces_filtered = 0;
}

static BOOL skip_this_iface (const IP_ADAPTER_ADDRESSES *addr, const MIB_IFROW *if_row, const void *_if_row2)
{
  if (verbose_level >= 2)
     return (FALSE);

  if ((addr && addr->OperStatus != IfOperStatusUp) ||
      (if_row && if_row->dwSpeed == 0UL))
     num_ifaces_not_up++;

  if (addr)
  {
#if defined(ON_WIN_VISTA)
    if (addr->ReceiveLinkSpeed == 0ULL || addr->TransmitLinkSpeed == 0ULL)
       return (TRUE);
#endif
    if (addr->OperStatus == IfOperStatusDown)
      return (TRUE);
  }
  else if (if_row)
  {
    if (if_row->dwSpeed == 0UL)
       return (TRUE);
  }
  else if (_if_row2)
  {
#if defined(ON_WIN_VISTA)
    const MIB_IF_ROW2 *if_row2 = (const MIB_IF_ROW2*) _if_row2;

    if (if_row2->ReceiveLinkSpeed == 0ULL || if_row2->TransmitLinkSpeed == 0ULL)
       return (TRUE);
#endif
  }
  return (FALSE);
}

/*
 * Skip interfaces that has a description ending in "-000x".
 * Handle both ASCII and UNICODE descriptions.
 */
static BOOL skip_filter_iface (const BYTE *descr_a, const wchar_t *descr_w)
{
  const char    *end_a;
  const wchar_t *end_w;

  if (verbose_level >= 2)
     return (FALSE);

  if (descr_w)
  {
    end_w = wcschr (descr_w, L'\0') - 5;
//  printf ("end_w: '%" WIDESTR_FMT "'\n", end_w);
    return (memcmp(end_w, (const void*)L"-000", 2*4) == 0);
  }

  end_a = strchr ((const char*)descr_a, '\0') - 5;
//printf ("end_a: '%s'\n", end_a);
  return (memcmp(end_a, (const void*)"-000", 4) == 0);
}

#undef  ADD_VALUE
#define ADD_VALUE(v) { v, #v }

static const struct search_list if_types[] = {
                  { _IF_TYPE_OTHER,              "Other type" },
                  { _IF_TYPE_ETHERNET_CSMACD,    "Ethernet" },
                  { _IF_TYPE_ISO88025_TOKENRING, "Token Ring" },
                  { _IF_TYPE_FDDI,               "FDDI" },
                  { _IF_TYPE_PPP,                "Point-to-Point" },
                  { _IF_TYPE_SOFTWARE_LOOPBACK,  "Loopback" },
                  { _IF_TYPE_ATM,                "ATM" },
                  { _IF_TYPE_IEEE80211,          "IEEE 802.11 wireless" },
                  { _IF_TYPE_TUNNEL,             "Tunnel" },
                  { _IF_TYPE_PROP_VIRTUAL,       "Virtual" },
                  { _IF_TYPE_IEEE1394,           "IEEE 1394 (Firewire)" },
                  { _IF_TYPE_IEEE80216_WMAN,     "WiMax broadband" },
                  { _IF_TYPE_WWANPP,             "GSM mobile" },
                  { _IF_TYPE_WWANPP2,            "CDMA mobile" },
                  { _IF_TYPE_REGULAR_1822,       "Regular 1822" },
                  { _IF_TYPE_HDH_1822,           "HDH 1822" },
                  { _IF_TYPE_DDN_X25,            "DDN X25" },
                  { _IF_TYPE_RFC877_X25,         "RFC877 X25" },
                  { _IF_TYPE_IS088023_CSMACD,    "IS088023 CSMACD" },
                  { _IF_TYPE_ISO88024_TOKENBUS,  "ISO88024 TokenBus" },
                  { _IF_TYPE_ISO88026_MAN,       "ISO88026 MAN" },
                  { _IF_TYPE_STARLAN,            "StarLAN" },
                  { _IF_TYPE_PROTEON_10MBIT,     "Proteon 10MBit" },
                  { _IF_TYPE_PROTEON_80MBIT,     "Proteon 80MBit" },
                  { _IF_TYPE_HYPERCHANNEL,       "HyperChannel" },
                  { _IF_TYPE_LAP_B,              "LAP-B" },
                  { _IF_TYPE_SDLC,               "SDLC" },
                  { _IF_TYPE_DS1,                "DS1-MIB" },
                  { _IF_TYPE_E1,                 "E1" },
                  ADD_VALUE (_IF_TYPE_BASIC_ISDN),
                  ADD_VALUE (_IF_TYPE_PRIMARY_ISDN),
                  ADD_VALUE (_IF_TYPE_PROP_POINT2POINT_SERIAL),
                  ADD_VALUE (_IF_TYPE_EON),
                  ADD_VALUE (_IF_TYPE_ETHERNET_3MBIT),
                  ADD_VALUE (_IF_TYPE_NSIP),
                  ADD_VALUE (_IF_TYPE_SLIP),
                  ADD_VALUE (_IF_TYPE_ULTRA),
                  ADD_VALUE (_IF_TYPE_DS3),
                  ADD_VALUE (_IF_TYPE_SIP),
                  ADD_VALUE (_IF_TYPE_FRAMERELAY),
                  ADD_VALUE (_IF_TYPE_RS232),
                  ADD_VALUE (_IF_TYPE_PARA),
                  ADD_VALUE (_IF_TYPE_ARCNET),
                  ADD_VALUE (_IF_TYPE_ARCNET_PLUS),
                  ADD_VALUE (_IF_TYPE_MIO_X25),
                  ADD_VALUE (_IF_TYPE_SONET),
                  ADD_VALUE (_IF_TYPE_X25_PLE),
                  ADD_VALUE (_IF_TYPE_ISO88022_LLC),
                  ADD_VALUE (_IF_TYPE_LOCALTALK),
                  ADD_VALUE (_IF_TYPE_SMDS_DXI),
                  ADD_VALUE (_IF_TYPE_FRAMERELAY_SERVICE),
                  ADD_VALUE (_IF_TYPE_V35),
                  ADD_VALUE (_IF_TYPE_HSSI),
                  ADD_VALUE (_IF_TYPE_HIPPI),
                  ADD_VALUE (_IF_TYPE_MODEM),
                  ADD_VALUE (_IF_TYPE_AAL5),
                  ADD_VALUE (_IF_TYPE_SONET_PATH),
                  ADD_VALUE (_IF_TYPE_SONET_VT),
                  ADD_VALUE (_IF_TYPE_SMDS_ICIP),
                  ADD_VALUE (_IF_TYPE_PROP_MULTIPLEXOR),
                  ADD_VALUE (_IF_TYPE_IEEE80212),
                  ADD_VALUE (_IF_TYPE_FIBRECHANNEL),
                  ADD_VALUE (_IF_TYPE_HIPPIINTERFACE),
                  ADD_VALUE (_IF_TYPE_FRAMERELAY_INTERCONNECT),
                  ADD_VALUE (_IF_TYPE_AFLANE_8023),
                  ADD_VALUE (_IF_TYPE_AFLANE_8025),
                  ADD_VALUE (_IF_TYPE_CCTEMUL),
                  ADD_VALUE (_IF_TYPE_FASTETHER),
                  ADD_VALUE (_IF_TYPE_ISDN),
                  ADD_VALUE (_IF_TYPE_V11),
                  ADD_VALUE (_IF_TYPE_V36),
                  ADD_VALUE (_IF_TYPE_G703_64K),
                  ADD_VALUE (_IF_TYPE_G703_2MB),
                  ADD_VALUE (_IF_TYPE_QLLC),
                  ADD_VALUE (_IF_TYPE_FASTETHER_FX),
                  ADD_VALUE (_IF_TYPE_CHANNEL),
                  ADD_VALUE (_IF_TYPE_IBM370PARCHAN),
                  ADD_VALUE (_IF_TYPE_ESCON),
                  ADD_VALUE (_IF_TYPE_DLSW),
                  ADD_VALUE (_IF_TYPE_ISDN_S),
                  ADD_VALUE (_IF_TYPE_ISDN_U),
                  ADD_VALUE (_IF_TYPE_LAP_D),
                  ADD_VALUE (_IF_TYPE_IPSWITCH),
                  ADD_VALUE (_IF_TYPE_RSRB),
                  ADD_VALUE (_IF_TYPE_ATM_LOGICAL),
                  ADD_VALUE (_IF_TYPE_DS0),
                  ADD_VALUE (_IF_TYPE_DS0_BUNDLE),
                  ADD_VALUE (_IF_TYPE_BSC),
                  ADD_VALUE (_IF_TYPE_ASYNC),
                  ADD_VALUE (_IF_TYPE_CNR),
                  ADD_VALUE (_IF_TYPE_ISO88025R_DTR),
                  ADD_VALUE (_IF_TYPE_EPLRS),
                  ADD_VALUE (_IF_TYPE_ARAP),
                  ADD_VALUE (_IF_TYPE_PROP_CNLS),
                  ADD_VALUE (_IF_TYPE_HOSTPAD),
                  ADD_VALUE (_IF_TYPE_TERMPAD),
                  ADD_VALUE (_IF_TYPE_FRAMERELAY_MPI),
                  ADD_VALUE (_IF_TYPE_X213),
                  ADD_VALUE (_IF_TYPE_ADSL),
                  ADD_VALUE (_IF_TYPE_RADSL),
                  ADD_VALUE (_IF_TYPE_SDSL),
                  ADD_VALUE (_IF_TYPE_VDSL),
                  ADD_VALUE (_IF_TYPE_ISO88025_CRFPRINT),
                  ADD_VALUE (_IF_TYPE_MYRINET),
                  ADD_VALUE (_IF_TYPE_VOICE_EM),
                  ADD_VALUE (_IF_TYPE_VOICE_FXO),
                  ADD_VALUE (_IF_TYPE_VOICE_FXS),
                  ADD_VALUE (_IF_TYPE_VOICE_ENCAP),
                  ADD_VALUE (_IF_TYPE_VOICE_OVERIP),
                  ADD_VALUE (_IF_TYPE_ATM_DXI),
                  ADD_VALUE (_IF_TYPE_ATM_FUNI),
                  ADD_VALUE (_IF_TYPE_ATM_IMA),
                  ADD_VALUE (_IF_TYPE_PPPMULTILINKBUNDLE),
                  ADD_VALUE (_IF_TYPE_IPOVER_CDLC),
                  ADD_VALUE (_IF_TYPE_IPOVER_CLAW),
                  ADD_VALUE (_IF_TYPE_STACKTOSTACK),
                  ADD_VALUE (_IF_TYPE_VIRTUALIPADDRESS),
                  ADD_VALUE (_IF_TYPE_MPC),
                  ADD_VALUE (_IF_TYPE_IPOVER_ATM),
                  ADD_VALUE (_IF_TYPE_ISO88025_FIBER),
                  ADD_VALUE (_IF_TYPE_TDLC),
                  ADD_VALUE (_IF_TYPE_GIGABITETHERNET),
                  ADD_VALUE (_IF_TYPE_HDLC),
                  ADD_VALUE (_IF_TYPE_LAP_F),
                  ADD_VALUE (_IF_TYPE_V37),
                  ADD_VALUE (_IF_TYPE_X25_MLP),
                  ADD_VALUE (_IF_TYPE_X25_HUNTGROUP),
                  ADD_VALUE (_IF_TYPE_TRANSPHDLC),
                  ADD_VALUE (_IF_TYPE_INTERLEAVE),
                  ADD_VALUE (_IF_TYPE_FAST),
                  ADD_VALUE (_IF_TYPE_IP),
                  ADD_VALUE (_IF_TYPE_DOCSCABLE_MACLAYER),
                  ADD_VALUE (_IF_TYPE_DOCSCABLE_DOWNSTREAM),
                  ADD_VALUE (_IF_TYPE_DOCSCABLE_UPSTREAM),
                  ADD_VALUE (_IF_TYPE_A12MPPSWITCH),
                  ADD_VALUE (_IF_TYPE_COFFEE),
                  ADD_VALUE (_IF_TYPE_CES),
                  ADD_VALUE (_IF_TYPE_ATM_SUBINTERFACE),
                  ADD_VALUE (_IF_TYPE_L2_VLAN),
                  ADD_VALUE (_IF_TYPE_L3_IPVLAN),
                  ADD_VALUE (_IF_TYPE_L3_IPXVLAN),
                  ADD_VALUE (_IF_TYPE_DIGITALPOWERLINE),
                  ADD_VALUE (_IF_TYPE_MEDIAMAILOVERIP),
                  ADD_VALUE (_IF_TYPE_DTM),
                  ADD_VALUE (_IF_TYPE_DCN),
                  ADD_VALUE (_IF_TYPE_IPFORWARD),
                  ADD_VALUE (_IF_TYPE_MSDSL),
                  ADD_VALUE (_IF_TYPE_IF_GSN),
                  ADD_VALUE (_IF_TYPE_DVBRCC_MACLAYER),
                  ADD_VALUE (_IF_TYPE_DVBRCC_DOWNSTREAM),
                  ADD_VALUE (_IF_TYPE_DVBRCC_UPSTREAM),
                  ADD_VALUE (_IF_TYPE_ATM_VIRTUAL),
                  ADD_VALUE (_IF_TYPE_MPLS_TUNNEL),
                  ADD_VALUE (_IF_TYPE_SRP),
                  ADD_VALUE (_IF_TYPE_VOICEOVERATM),
                  ADD_VALUE (_IF_TYPE_VOICEOVERFRAMERELAY),
                  ADD_VALUE (_IF_TYPE_IDSL),
                  ADD_VALUE (_IF_TYPE_COMPOSITELINK),
                  ADD_VALUE (_IF_TYPE_SS7_SIGLINK),
                  ADD_VALUE (_IF_TYPE_PROP_WIRELESS_P2P),
                  ADD_VALUE (_IF_TYPE_FR_FORWARD),
                  ADD_VALUE (_IF_TYPE_RFC1483),
                  ADD_VALUE (_IF_TYPE_USB),
                  ADD_VALUE (_IF_TYPE_IEEE8023AD_LAG),
                  ADD_VALUE (_IF_TYPE_BGP_POLICY_ACCOUNTING),
                  ADD_VALUE (_IF_TYPE_FRF16_MFR_BUNDLE),
                  ADD_VALUE (_IF_TYPE_H323_GATEKEEPER),
                  ADD_VALUE (_IF_TYPE_H323_PROXY),
                  ADD_VALUE (_IF_TYPE_MPLS),
                  ADD_VALUE (_IF_TYPE_MF_SIGLINK),
                  ADD_VALUE (_IF_TYPE_HDSL2),
                  ADD_VALUE (_IF_TYPE_SHDSL),
                  ADD_VALUE (_IF_TYPE_DS1_FDL),
                  ADD_VALUE (_IF_TYPE_POS),
                  ADD_VALUE (_IF_TYPE_DVB_ASI_IN),
                  ADD_VALUE (_IF_TYPE_DVB_ASI_OUT),
                  ADD_VALUE (_IF_TYPE_PLC),
                  ADD_VALUE (_IF_TYPE_NFAS),
                  ADD_VALUE (_IF_TYPE_TR008),
                  ADD_VALUE (_IF_TYPE_GR303_RDT),
                  ADD_VALUE (_IF_TYPE_GR303_IDT),
                  ADD_VALUE (_IF_TYPE_ISUP),
                  ADD_VALUE (_IF_TYPE_PROP_DOCS_WIRELESS_MACLAYER),
                  ADD_VALUE (_IF_TYPE_PROP_DOCS_WIRELESS_DOWNSTREAM),
                  ADD_VALUE (_IF_TYPE_PROP_DOCS_WIRELESS_UPSTREAM),
                  ADD_VALUE (_IF_TYPE_HIPERLAN2),
                  ADD_VALUE (_IF_TYPE_PROP_BWA_P2MP),
                  ADD_VALUE (_IF_TYPE_SONET_OVERHEAD_CHANNEL),
                  ADD_VALUE (_IF_TYPE_DIGITAL_WRAPPER_OVERHEAD_CHANNEL),
                  ADD_VALUE (_IF_TYPE_AAL2),
                  ADD_VALUE (_IF_TYPE_RADIO_MAC),
                  ADD_VALUE (_IF_TYPE_ATM_RADIO),
                  ADD_VALUE (_IF_TYPE_IMT),
                  ADD_VALUE (_IF_TYPE_MVL),
                  ADD_VALUE (_IF_TYPE_REACH_DSL),
                  ADD_VALUE (_IF_TYPE_FR_DLCI_ENDPT),
                  ADD_VALUE (_IF_TYPE_ATM_VCI_ENDPT),
                  ADD_VALUE (_IF_TYPE_OPTICAL_CHANNEL),
                  ADD_VALUE (_IF_TYPE_OPTICAL_TRANSPORT),
                  ADD_VALUE (_IF_TYPE_IEEE802154),
                  ADD_VALUE (_IF_TYPE_XBOX_WIRELESS)
                };

static const struct search_list mib_oper_status[] = {
                  { IF_OPER_STATUS_NON_OPERATIONAL, "disabled" },
                  { IF_OPER_STATUS_UNREACHABLE,     "WAN not connected" },
                  { IF_OPER_STATUS_DISCONNECTED,    "disconnected/no carrier" },
                  { IF_OPER_STATUS_CONNECTING,      "WAN connecting" },
                  { IF_OPER_STATUS_CONNECTED,       "WAN connected" },
                  { IF_OPER_STATUS_OPERATIONAL,     "LAN okay" }
                };

static const struct search_list oper_status[] = {
                  { IfOperStatusUp,             "Up" },
                  { IfOperStatusDown,           "Down" },
                  { IfOperStatusTesting,        "Testing" },
                  { IfOperStatusUnknown,        "Unknown" },
                  { IfOperStatusDormant,        "Dormant" },
                  { IfOperStatusNotPresent,     "Not present" },
                  { IfOperStatusLowerLayerDown, "Lower layer down" }
                };

static const struct search_list arp_types[] = {
                  { MIB_IPNET_TYPE_OTHER,   "Other" },
                  { MIB_IPNET_TYPE_INVALID, "Invalid" },
                  { MIB_IPNET_TYPE_DYNAMIC, "Dynamic" },
                  { MIB_IPNET_TYPE_STATIC,  "Static" }
                };

static const struct search_list neighbour_states[] = {
                  { NlnsUnreachable, "Unreachable" },
                  { NlnsIncomplete,  "Incomplete" },
                  { NlnsProbe,       "Probe" },
                  { NlnsDelay,       "Delay" },
                  /*
                   * A typo in Watcom's <nt/nldef.h> calls this 'NlnsState'
                   */
                  { 4,               "Stale" },
                  { NlnsReachable,   "Reachable" },
                  { NlnsPermanent,   "Permanent" }
                };

#if defined(ON_WIN_VISTA_SP1)
  static const struct search_list conn_type[] = {
                    { 0,                           "Unknown" },
                    { NET_IF_CONNECTION_DEDICATED, "Dedicated" },
                    { NET_IF_CONNECTION_PASSIVE,   "Passive" },
                    { NET_IF_CONNECTION_DEMAND,    "Demand" }
                  };
#endif

#if defined(ON_WIN_VISTA) && defined(HAVE_NETIOAPI_H)

#if (WINVER < 0x0601)
  #define NdisMediumWiMAX (NdisMediumMax-2)
  #define NdisMediumIP    (NdisMediumMax-1)
#endif

  static const struct search_list tunnel_types[] = {
                    { TUNNEL_TYPE_NONE,   "None" },
                    { TUNNEL_TYPE_OTHER,  "Other" },
                    { TUNNEL_TYPE_DIRECT, "Direct" },
                    { TUNNEL_TYPE_6TO4,   "6to4" },
                    { TUNNEL_TYPE_ISATAP, "ISATAP" },
                    { TUNNEL_TYPE_TEREDO, "Teredo" }
                  };

  static const struct search_list media_types[] = {
                    { NdisMedium802_3,         "Ethernet 802.3" },
                    { NdisMedium802_5,         "TokenRing" },
                    { NdisMediumFddi,          "FDDI" },
                    { NdisMediumWan,           "WAN" },
                    { NdisMediumLocalTalk,     "LocalTalk" },
                    { NdisMediumDix,           "DIX" },
                    { NdisMediumArcnetRaw,     "ARCNET" },
                    { NdisMediumArcnet878_2,   "ARCNET (878.2)" },
                    { NdisMediumAtm,           "ATM" },
                    { NdisMediumWirelessWan,   "WWAN" },
                    { NdisMediumIrda,          "IrDA" },
                    { NdisMediumBpc,           "Broadcast PC" },
                    { NdisMediumCoWan,         "CoWan" },
                    { NdisMedium1394,          "IEEE 1394" },
                    { NdisMediumInfiniBand,    "InfiniBand" },
                    { NdisMediumTunnel,        "Tunnel" },
                    { NdisMediumNative802_11,  "IEEE 802.11" },
                    { NdisMediumLoopback,      "Loopback" },
                    { NdisMediumWiMAX,         "WiMax" },
                    { NdisMediumIP,            "IP" }
                  };

  static const struct search_list access_types[] = {
                    { NET_IF_ACCESS_LOOPBACK,             "Loopback" },
                    { NET_IF_ACCESS_BROADCAST,            "Broadcast" },
                    { NET_IF_ACCESS_POINT_TO_POINT,       "Point-to-point" },
                    { NET_IF_ACCESS_POINT_TO_MULTI_POINT, "Point-to-Multipoint" }
  };
#endif

#define SMI_IETF 0

#if defined(HAVE_OUI_GENERATED_C)
  #include "oui-generated.c"
#endif

static const char *get_phys_address (const void *a, ULONG len, BOOL show_manuf)
{
  const UCHAR *addr = (const UCHAR*)a;
  char        *p, *p_max = work_buf + sizeof(work_buf) - 4;
  ULONG        i;

  ARGSUSED (show_manuf);

  len = min (IF_MAX_PHYS_ADDRESS_LENGTH, len);
  if (len == 0)
     return (NONE_STR);

  p = work_buf;
  for (i = 0; i < len && p < p_max; i++)
  {
    UCHAR b = addr[i];

    *p++ = hex_chars_upper [b >> 4];
    *p++ = hex_chars_upper [b & 0xf];
    if (i < len-1)
         *p++ = ':';
    else *p = '\0';
  }

#if defined(HAVE_OUI_GENERATED_C)
  /**
   * Get the manufacturer name for this MAC-address.
   * \todo Get addresses such as 00:FF:DB:EF:7D:48 from a
   *       "Well Known Address" source.
   *       Ref. Wireshark's 'wka' file.
   */
  if (show_manuf)
  {
    DWORD oui = (addr[0] << 16) + (addr[1] << 8) + addr[2];

    *p++ = ' ';
    *p++ = '(';
    _strlcpy (p, oui_val_to_name(oui), p_max-p);
    p = strchr (p, '\0');
    *p++ = ')';
    *p++ = '\0';
  }
#endif
  return (work_buf);
}

static const char *get_address_flags (DWORD flags)
{
  char *p = work_buf;

  *p = '\0';

  if (flags == 0)
     return (NONE_STR);

  if (flags & IP_ADAPTER_DDNS_ENABLED)
     p += sprintf (p, "Dynamic DNS enabled, ");

  if (flags & IP_ADAPTER_REGISTER_ADAPTER_SUFFIX)
     p += sprintf (p, "Register DNS suffix, ");

  if (flags & IP_ADAPTER_DHCP_ENABLED)
     p += sprintf (p, "DHCP enabled, ");

  if (flags & IP_ADAPTER_RECEIVE_ONLY)
     p += sprintf (p, "Rx only, ");

  if (flags & IP_ADAPTER_NO_MULTICAST)
     p += sprintf (p, "No multicast, ");

  if (flags & IP_ADAPTER_IPV6_OTHER_STATEFUL_CONFIG)
     p += sprintf (p, "IPv6 stateful config, ");

  if (flags & IP_ADAPTER_NETBIOS_OVER_TCPIP_ENABLED)
     p += sprintf (p, "NetBIOS over tcp/ip, ");

  if (flags & IP_ADAPTER_IPV4_ENABLED)
     p += sprintf (p, "IPv4 enabled, ");

  if (flags & IP_ADAPTER_IPV6_ENABLED)
     p += sprintf (p, "IPv6 enabled, ");

  if (p > work_buf)
     *(p -= 2) = '\0';

  assert (p < work_buf + sizeof(work_buf));
  return (work_buf);
}

static const char *get_addr_common (int num, const address_buf *abuf)
{
  char *p = work_buf;
  int   i;

  *p = '\0';
  if (num == 0)
     return (NONE_STR);

  for (i = 0; i < num; i++)
  {
    strncat (p, abuf[i], sizeof(abuf[0])-1);
    p += strlen (p);
    if (i < num - 1)
    {
      strcat (p, ", ");
      p += 2;
    }
    else
      break;
  }
  return strupr (work_buf);
}

#if defined(ON_WIN_VISTA)
static const char *socket_addr_str (const SOCKET_ADDRESS *addr)
{
  static address_buf abuf;
  DWORD              size = sizeof(abuf);

  if (addr->iSockaddrLength == 0)
  {
    strcpy (abuf, NONE_STR);
    return (abuf);
  }
  if ((*p_WSAAddressToStringA) ((SOCKADDR*)&addr->lpSockaddr,
                                addr->iSockaddrLength, NULL, abuf, &size))
  {
    sprintf (work_buf, "WSAAddressToString(): %s",
             win_strerror((*p_WSAGetLastError)()));
    return (work_buf);
  }
  return (abuf);
}
#endif

#ifdef NOT_USED
static const char *socket_addr_str2 (const SOCKET_ADDRESS *addr)
{
  if (addr->iSockaddrLength == 0)
     strcpy (work_buf, NONE_STR);
  else
  {
    int af = addr->lpSockaddr->sa_family;

    if (af != AF_INET && af != AF_INET6)
       sprintf (work_buf, "Unknown AF %d", af);
    else
    {
      address_buf abuf;

      if (af == AF_INET6)
         af = _w32_AF_INET6;

      if (!_w32_inet_ntop(af, &addr->lpSockaddr->sa_data, abuf, sizeof(abuf)))
            sprintf (work_buf, "inet_ntop(); error %d", _w32_errno);
      else strcpy (work_buf, abuf);
    }
  }
  return (work_buf);
}
#endif

static const char *get_netmask_prefix (const IP_ADAPTER_PREFIX *pf)
{
  address_buf                abuf;
  const SOCKET_ADDRESS      *sa = &pf->Address;
  int   af = sa->lpSockaddr->sa_family;

  union {
    struct sockaddr_in  sa4;
    struct sockaddr_in6 sa6;
  } addr;

  if (sa->iSockaddrLength == 0)
     return strcpy (work_buf, NONE_STR);

  memset (&addr, '\0', sizeof(addr));
  memcpy (&addr, sa->lpSockaddr, sa->iSockaddrLength);

  if (af == AF_INET)
  {
    if (!_w32_inet_ntop(AF_INET, &addr.sa4.sin_addr, abuf, sizeof(abuf)))
         sprintf (work_buf, "inet_ntop(AF_INET); error %d", _w32_errno);
    else strcpy (work_buf, abuf);
  }
  else if (af == AF_INET6)
  {
#if defined(USE_IPV6)
    if (!_w32_inet_ntop(_w32_AF_INET6, &addr.sa6.sin6_addr, abuf, sizeof(abuf)))
       sprintf (work_buf, "inet_ntop(_w32_AF_INET6); error %d", _w32_errno);
    else
    {
      int   scope = addr.sa6.sin6_scope_id;
      char *p;

      strcpy (work_buf, abuf);
      if (scope > 0 && scope <= 15)
      {
        p = strchr (work_buf, '\0');
        *p++ = SCOPE_DELIMITER;
        *p++ = hex_chars_upper [scope];
        *p = '\0';
      }
    }
#else
    strcpy (work_buf, "AF_INET6 (USE_IPV6 not defined)");
#endif
  }
  else
  {
    sprintf (work_buf, "Unknown AF %d", af);
  }
  return (work_buf);
}

#define GET_ONE_ADDR(addr, alen, buf, buf_sz)                          \
        if ((*p_WSAAddressToStringA) (addr, alen, NULL, buf, &buf_sz)) \
        {                                                              \
          (*_printf) (" WSAAddressToString(): %s\n",                   \
                      win_strerror((*p_WSAGetLastError)()));           \
          break;                                                       \
        }
        /* Or use 'getnameinfo (addr, alen, buf, buf_sz, NULL ,0, NI_NUMERICHOST)' ?? */

#define GET_ADDRESSES(func, type, var)                           \
        static const char * func (const type * var)              \
        {                                                        \
          address_buf abuf[10]; /* todo 'struct one_addr[10]' */ \
          int i;                                                 \
                                                                 \
          for (i = 0; var && i < DIM(abuf); i++)                 \
          {                                                      \
            SOCKADDR *addr  = var->Address.lpSockaddr;           \
            DWORD     alen  = var->Address.iSockaddrLength;      \
            DWORD     asize = sizeof(abuf[i]);                   \
                                                                 \
            if (addr->sa_family != AF_INET)                      \
               ipv4_only_iface = FALSE;                          \
            strcpy (abuf[i], "??");                              \
            GET_ONE_ADDR (addr, alen, abuf[i], asize);           \
            var = var->Next;                                     \
          }                                                      \
          return get_addr_common (i, (const address_buf*)&abuf); \
        }

GET_ADDRESSES (get_unicast_addrs,    IP_ADAPTER_UNICAST_ADDRESS, uca)
GET_ADDRESSES (get_anycast_addrs,    IP_ADAPTER_ANYCAST_ADDRESS, aca)
GET_ADDRESSES (get_multicast_addrs,  IP_ADAPTER_MULTICAST_ADDRESS, mca)
GET_ADDRESSES (get_dns_server_addrs, IP_ADAPTER_DNS_SERVER_ADDRESS, dns)

#if defined(ON_WIN_VISTA)
  GET_ADDRESSES (get_wins_addrs,     IP_ADAPTER_WINS_SERVER_ADDRESS, wins)
  GET_ADDRESSES (get_gateway_addrs,  IP_ADAPTER_GATEWAY_ADDRESS, gw)

  /**
   * We need the `row.Metric` from `print_ip_interface_details()` to
   * get the true Metric for the route. Hence store 'addr->Ipv4Metric' or
   * 'addr->Ipv6Metric' in a local LUID-cache below.
   *
   * Ref:
   *   https://docs.microsoft.com/en-us/windows/win32/api/netioapi/ns-netioapi-mib_ipforward_row2
   *
   * Quote:
   *   Note the actual route metric used to compute the route preference is the summation of
   *   interface metric specified in the Metric member of the MIB_IPINTERFACE_ROW structure
   *   and the route metric offset specified in this member.
   */
  typedef struct luid_metrics {
          ULONG64 luid_value;
          ULONG   metric;
          int     family;
        } luid_metrics;

  static struct luid_metrics metrics_cache [40];

  static BOOL iface_store_metric (const NET_LUID *luid, DWORD metric, int family)
  {
    luid_metrics *lm = metrics_cache + 0;
    int  i;

    for (i = 0; i < DIM(metrics_cache); i++, lm++)
    {
      if (lm->luid_value == 0)
      {
        lm->luid_value = luid->Value;
        lm->metric = metric;
        lm->family = family;
        return (TRUE);
      }
    }
    return (FALSE);  /* All buckets full */
  }

  static DWORD iface_lookup_metric (const NET_LUID *luid, int family)
  {
    const luid_metrics *lm = metrics_cache + 0;
    int   i;

    for (i = 0; i < DIM(metrics_cache); i++, lm++)
    {
      if (lm->luid_value == luid->Value && lm->family == family)
         return (lm->metric);
    }
    return (0);  /* not found!? */
  }
#endif  /* ON_WIN_VISTA */

/**
 * \todo: print these too:
 *   'ValidLifeTime', 'PreferredLifetime', 'LeaseLifetime',
 *   'PrefixOrigin', 'SuffixOrigin'
 *   '(Address.lpSockaddr)->sa_family'  for 'uca' and 'aca'
 *
 *  and:
 *   '(Address.lpSockaddr)->sa_family'  for 'mca' and 'dns'
 */
static void dump_data (const void *data_p, UINT datalen, const char *indent)
{
  const BYTE *data = (const BYTE*) data_p;
  UINT  ofs;

  if (!indent)
     indent = "";

  for (ofs = 0; ofs < datalen; ofs += 16)
  {
    UINT j;

    if (ofs == 0)
         (*_printf) ("%s%u:%s%04X: ", indent, datalen,
                     datalen > 9999 ? " "    :
                     datalen > 999  ? "  "   :
                     datalen > 99   ? "   "  :
                     datalen > 9    ? "    " :
                                  "     ",
                 ofs);
    else (*_printf) ("%s       %04X: ", indent, ofs);

    for (j = 0; j < 16 && j+ofs < datalen; j++)
        (*_printf) ("%02X%c", (unsigned)data[j+ofs],
                    j == 7 ? '-' : ' ');  /* no beeps */

    for ( ; j < 16; j++)       /* pad line to 16 positions */
        (*_printf) ("   ");

    for (j = 0; j < 16 && j+ofs < datalen; j++)
    {
      int ch = data[j+ofs];

      (*_printf) ("%c", ch < ' ' ? '.' : ch);
    }
    (*_printf) ("\n");
  }
  (*_printf) ("\n");
}

/*
 * Use GetIfTable() and dump information from MIB.
 */
static int _pkt_win_print_GetIfTable (void)
{
  MIB_IFTABLE *if_table = NULL;
  ULONG        out_len = 0;
  DWORD        rc      = 0;
  UINT         i       = 0;

  if (!pkt_win_init())
     return (0);

  (*_printf) ("\nFrom GetIfTable():\n");

  if (!p_GetIfTable)
  {
    (*_printf) ("This function not available on this OS.");
     return (0);
  }

  (*p_GetIfTable) (if_table, &out_len, TRUE);
  if_table = alloca (out_len);
  rc = (*p_GetIfTable) (if_table, &out_len, TRUE);
  if (rc != NO_ERROR)
  {
    (*_printf) ("error: %s\n", win_strerror(rc));
    return (0);
  }

  (*_printf) ("  Number of MIB entries: %ld\n", (long)if_table->dwNumEntries);

  for (i = 0; i < if_table->dwNumEntries; i++)
  {
    const MIB_IFROW *if_row = (const MIB_IFROW*) (if_table->table + i);

    if (skip_this_iface(NULL, if_row, NULL))
    {
      num_ifaces_skipped++;
      continue;
    }
    if (skip_filter_iface(if_row->bDescr, NULL))
    {
      num_ifaces_filtered++;
      continue;
    }

    print_mib_if_row (i, if_row);
  }
  return (1);
}

/*
 * Use GetIfTable2() and dump information from MIB.
 */
static int _pkt_win_print_GetIfTable2 (void)
{
#if !defined(ON_WIN_VISTA) || !defined(HAVE_NETIOAPI_H)
  (*_printf) ("%s(%u): MIB_IF_TABLE2 not available with this compiler/SDK.",
              __FILE__, __LINE__);
  return (0);

#else
  MIB_IF_TABLE2 *if_table2 = NULL;
  BOOL           rc;
  ULONG          i;

  if (!pkt_win_init())
     return (0);

  (*_printf) ("\nFrom GetIfTable2():\n");

  if (!p_GetIfTable2 || !p_FreeMibTable)
  {
    (*_printf) ("This function not available on this OS.");
     return (0);
  }

  rc = (*p_GetIfTable2) (&if_table2);
  if (rc != NO_ERROR)
  {
    (*_printf) ("error: %s\n", win_strerror(rc));
    return (0);
  }

  (*_printf) ("  Number of MIB entries: %ld\n", if_table2->NumEntries);

  for (i = 0; i < if_table2->NumEntries; i++)
  {
    const MIB_IF_ROW2 *if_row2 = if_table2->Table + i;

    if (skip_this_iface(NULL, NULL, if_row2))
    {
      num_ifaces_skipped++;
      continue;
    }
    if (skip_filter_iface(NULL, if_row2->Description))
    {
      num_ifaces_filtered++;
      continue;
    }

    print_mib_if_row2 (i, if_row2);
    if (i < if_table2->NumEntries-1)
      (*_printf) ("\n");
  }
  (*p_FreeMibTable) (if_table2);
  return (1);
#endif
}

static void print_mib_if_row (DWORD index, const MIB_IFROW *row)
{
  char speed[30];

  ARGSUSED (index);

  /* Note: The 'row->wszName' may *not* have the same GUID
   * as returned in 'GetAdapteraddresses()' despite it's the same
   * interface. To verify same interface, we can compare the
   * descriptions from the 2 API functions.
   */
  (*_printf) ("  Interface name: %.*s\n", MAX_INTERFACE_NAME_LEN, wstring_utf8(row->wszName));
  (*_printf) ("  Description:    %.*s\n", MAXLEN_IFDESCR, row->bDescr);

  (*_printf) ("    MTU:          %lu\n", (u_long)row->dwMtu);
  (*_printf) ("    Speed:        %s\n", speed_string(row->dwSpeed,speed));
  (*_printf) ("    Status:       %s\n",
              _list_lookup(row->dwOperStatus, mib_oper_status, DIM(mib_oper_status)));

  #define COLUMN_FORMAT "%13.13s %13.13s %13.13s %13.13s %13.13s %13.13s\n"

  (*_printf) ("           " COLUMN_FORMAT,
              "Bytes", "Unicasts", "Non-unicasts", "Discarded", "Errors", "Unk Proto");

  (*_printf) ("      In:  " COLUMN_FORMAT,
              dword_string(row->dwInOctets),     dword_string(row->dwInUcastPkts),
              dword_string(row->dwInNUcastPkts), dword_string(row->dwInDiscards),
              dword_string(row->dwInErrors),     dword_string(row->dwInUnknownProtos));

  (*_printf) ("      Out: " COLUMN_FORMAT "\n",
              dword_string(row->dwOutOctets),     dword_string(row->dwOutUcastPkts),
              dword_string(row->dwOutNUcastPkts), dword_string(row->dwOutDiscards),
              dword_string(row->dwOutErrors),     NA_STR);
}

#if defined(ON_WIN_VISTA) && defined(HAVE_NETIOAPI_H)
static void print_ip_interface_details (const NET_LUID *luid, int family, int indent)
{
#if defined(__WATCOMC__)
  (*_printf) ("Not for Watcom.\n");

#else
  MIB_IPINTERFACE_ROW row;
  DWORD rc;

  if (!p_GetIpInterfaceEntry)
  {
    (*_printf) ("p_GetIpInterfaceEntry == NULL\n");
    return;
  }

  memset (&row, '\0', sizeof(row));
  memcpy (&row.InterfaceLuid, luid, sizeof(*luid));
  row.Family = family;

  rc = (*p_GetIpInterfaceEntry) (&row);
  if (rc != NO_ERROR)
  {
    if (rc == ERROR_NOT_FOUND)
         (*_printf) ("NOT_FOUND\n");
    else (*_printf) ("error: %s\n", win_strerror(rc));
    return;
  }

  #define IF_PRINT0(fmt, what) (*_printf) ("%s:%*s" fmt "\n",                  \
                                           #what, (int)(44-strlen(#what)), "", \
                                           row.what)

  #define IF_PRINT(fmt, what)  (*_printf) ("%*s%s:%*s" fmt "\n",                           \
                                           indent, "", #what, (int)(44-strlen(#what)), "", \
                                           row.what)
  IF_PRINT0("%d",  Family);
  IF_PRINT ("%lu", InterfaceIndex);
  IF_PRINT ("%d",  ForwardingEnabled);
  IF_PRINT ("%d",  WeakHostSend);
  IF_PRINT ("%d",  WeakHostReceive);
  IF_PRINT ("%d",  UseAutomaticMetric);
  IF_PRINT ("%d",  UseNeighborUnreachabilityDetection);
  IF_PRINT ("%d",  ManagedAddressConfigurationSupported);
  IF_PRINT ("%d",  OtherStatefulConfigurationSupported);
  IF_PRINT ("%d",  AdvertiseDefaultRoute);
  IF_PRINT ("%d",  RouterDiscoveryBehavior);
  IF_PRINT ("%lu", DadTransmits);
  IF_PRINT ("%lu", BaseReachableTime);
  IF_PRINT ("%lu", RetransmitTime);
  IF_PRINT ("%lu", PathMtuDiscoveryTimeout);
  IF_PRINT ("%d",  LinkLocalAddressBehavior);
  IF_PRINT ("%lu", LinkLocalAddressTimeout);
  IF_PRINT ("%lu", SitePrefixLength);
  IF_PRINT ("%lu", Metric);
  IF_PRINT ("%lu", NlMtu);
  IF_PRINT ("%d",  SupportsWakeUpPatterns);
  IF_PRINT ("%d",  SupportsNeighborDiscovery);
  IF_PRINT ("%d",  SupportsRouterDiscovery);
  IF_PRINT ("%lu", ReachableTime);
  IF_PRINT ("%lu", MinRouterAdvertisementInterval);
  IF_PRINT ("%lu", MaxRouterAdvertisementInterval);
  IF_PRINT ("%d",  Connected);

  IF_PRINT ("%d",  TransmitOffload.NlChecksumSupported);
  IF_PRINT ("%d",  TransmitOffload.NlOptionsSupported);
  IF_PRINT ("%d",  TransmitOffload.TlDatagramChecksumSupported);
  IF_PRINT ("%d",  TransmitOffload.TlStreamChecksumSupported);
  IF_PRINT ("%d",  TransmitOffload.TlStreamOptionsSupported);
  IF_PRINT ("%d",  TransmitOffload.FastPathCompatible);
  IF_PRINT ("%d",  TransmitOffload.TlLargeSendOffloadSupported);
  IF_PRINT ("%d",  TransmitOffload.TlGiantSendOffloadSupported);

  IF_PRINT ("%d",  ReceiveOffload.NlChecksumSupported);
  IF_PRINT ("%d",  ReceiveOffload.NlOptionsSupported);
  IF_PRINT ("%d",  ReceiveOffload.TlDatagramChecksumSupported);
  IF_PRINT ("%d",  ReceiveOffload.TlStreamChecksumSupported);
  IF_PRINT ("%d",  ReceiveOffload.TlStreamOptionsSupported);
  IF_PRINT ("%d",  ReceiveOffload.FastPathCompatible);
  IF_PRINT ("%d",  ReceiveOffload.TlLargeSendOffloadSupported);
  IF_PRINT ("%d",  ReceiveOffload.TlGiantSendOffloadSupported);

  #undef IF_PRINT
#endif  /* __WATCOMC__ */
}

static const char *get_if_type (DWORD if_type)
{
  const char *ret = _list_lookup (if_type, if_types, DIM(if_types));

  if (!strncmp(ret, "_IF_TYPE_", 9))
     ret += 9;
  return (ret);
}

static void print_mib_if_row2 (DWORD index, const MIB_IF_ROW2 *row)
{
  char speed [30];

  (*_printf) ("  GUID:           %s\n", get_guid_str(&row->InterfaceGuid));
  (*_printf) ("    Alias:        %.*" WIDESTR_FMT "\n", IF_MAX_STRING_SIZE, row->Alias);
  (*_printf) ("    Description:  %.*" WIDESTR_FMT "\n", IF_MAX_STRING_SIZE, row->Description);
  (*_printf) ("    MAC-address:  %s\n", get_phys_address(&row->PhysicalAddress,row->PhysicalAddressLength,TRUE));
  (*_printf) ("    MTU:          %lu\n", row->Mtu);
  (*_printf) ("    Type:         %s (%lu)\n", get_if_type(row->Type), row->Type);

  if (row->Type == _IF_TYPE_TUNNEL)
     (*_printf) ("    Tunnel type:  %s (%d)\n",
                 _list_lookup(row->TunnelType, tunnel_types, DIM(tunnel_types)), row->TunnelType);

  (*_printf) ("    Media type:   %s (%d)\n",
              _list_lookup(row->MediaType, media_types, DIM(media_types)), row->MediaType);

  (*_printf) ("    Access type:  %s (%d)\n",
              _list_lookup(row->AccessType, access_types, DIM(access_types)), row->AccessType);

  (*_printf) ("    Tx speed:     %s\n", speed64_string(row->TransmitLinkSpeed, speed));
  (*_printf) ("    Rx speed:     %s\n", speed64_string(row->ReceiveLinkSpeed, speed));

  if (verbose_level >= 1)
  {
    int indent = sizeof ("    LUID:         ") - 1;

    (*_printf) ("    LUID:         ");
    print_net_luid (&row->InterfaceLuid, indent);

    if (verbose_level >= 2)
    {
      (*_printf) ("      Entry (AF_INET):  ");
      indent = sizeof ("      Entry (AF_INET):  ") - 1;
      print_ip_interface_details (&row->InterfaceLuid, AF_INET, indent);

      (*_printf) ("      Entry (AF_INET6): ");
      indent = sizeof ("      Entry (AF_INET6): ") - 1;
      print_ip_interface_details (&row->InterfaceLuid, AF_INET6, indent);
    }
  }
}

#else
static const char *get_if_type (DWORD if_type)
{
  static char buf [10];

  return itoa ((int)if_type, buf, 10);
}
#endif /* ON_WIN_VISTA && HAVE_NETIOAPI_H */

/*
 * Use GetIfTable2Ex() and dump information from MIB.
 */
static int _pkt_win_print_GetIfTable2Ex (void)
{
#if !defined(ON_WIN_VISTA) || !defined(HAVE_NETIOAPI_H)
  (*_printf) ("%s(%u): MIB_IF_TABLE2 not available with this compiler/SDK.",
              __FILE__, __LINE__);
  return (0);
#else
  MIB_IF_TABLE2 *if_table2 = NULL;
  BOOL           rc;
  ULONG          i;

  if (!pkt_win_init())
     return (0);

  (*_printf) ("\nFrom GetIfTable2Ex():\n");

  if (!p_GetIfTable2Ex || !p_FreeMibTable)
  {
    (*_printf) ("This function not available on this OS.");
     return (0);
  }

  rc = (*p_GetIfTable2Ex) (MibIfTableRaw, &if_table2);
  if (rc != NO_ERROR)
  {
    (*_printf) ("error: %s\n", win_strerror(rc));
    return (0);
  }

  (*_printf) ("  Number of MIB entries: %ld\n", if_table2->NumEntries);

  for (i = 0; i < if_table2->NumEntries; i++)
  {
    const MIB_IF_ROW2 *if_row2 = if_table2->Table + i;

    if (skip_this_iface(NULL, NULL, if_row2))
    {
      num_ifaces_skipped++;
      continue;
    }
    if (skip_filter_iface(NULL, if_row2->Description))
    {
      num_ifaces_filtered++;
      continue;
    }

    print_mib_if_row2 (i, if_row2);
    if (i < if_table2->NumEntries-1)
      (*_printf) ("\n");
  }

  (*p_FreeMibTable) (if_table2);
  return (1);
#endif
}

/*
 * Use GetIpNetTable() and dump ARP information from MIB.
 * IPv4 addresses only.
 * Sort on Type ('arp_types[]').
 */
static int _pkt_win_print_GetIpNetTable (void)
{
  MIB_IPNETTABLE *table;
  DWORD           i, rc, len = 0;

  if (!pkt_win_init() || !p_GetIpNetTable)
     return (0);

  (*_printf) ("\nFrom GetIpNetTable():\n");

  rc = (*p_GetIpNetTable) (NULL, &len, FALSE);
  if (rc == ERROR_INSUFFICIENT_BUFFER)
     table = alloca (len);
  else
  {
    (*_printf) ("  Error: %s\n", win_strerror(rc));
    return (0);
  }

  rc = (*p_GetIpNetTable) (table, &len, TRUE);
  if (rc != NO_ERROR && rc != ERROR_NO_DATA)
  {
    (*_printf) ("  Error: %s\n", win_strerror(rc));
    return (0);
  }

  qsort (table->table, table->dwNumEntries, sizeof(MIB_IPNETROW), compare_ipnetrow);

  for (i = 0; i < table->dwNumEntries; i++)
      print_mib_ipnetrow (i, table->table + i);
  return (0);
}

/*
 * Use GetIpNetTable2() and dump ARP information from MIB.
 * IPv4 + IPv6 addresses.
 * Sort on State (neighbour_state).
 */
static int _pkt_win_print_GetIpNetTable2 (void)
{
  MIB_IPNET_TABLE2 *table = NULL;
  DWORD     i, rc;

  if (!pkt_win_init() || !p_GetIpNetTable2 || !p_FreeMibTable)
     return (0);

  (*_printf) ("\nFrom GetIpNetTable2():\n");

  rc = (*p_GetIpNetTable2) (AF_UNSPEC, &table);
  if (rc != NO_ERROR && rc != ERROR_NOT_FOUND)
  {
    (*_printf) ("  Error: %s\n", win_strerror(rc));
    return (0);
  }

  qsort (table->Table, table->NumEntries, sizeof(MIB_IPNET_ROW2), compare_ipnetrow2);

  for (i = 0; i < table->NumEntries; i++)
      print_mib_ipnet_row2 (i, table->Table + i);

  (*p_FreeMibTable) (table);
  return (0);
}

/*
 * Use GetIpAddrTable().
 */
static int _pkt_win_print_GetIpAddrTable (void)
{
   /* \todo */
  (*_printf) ("\nFrom GetIpAddrTable(): <not implemented>\n");
  return (0);
}


/*
 * Use GetAdaptersAddresses and dump information on all IPv4/IPv6 adapters;
 * use AF_UNSPEC to get info on all.
 *
 * Rewritten from an MSDN example:
 * http://msdn.microsoft.com/en-us/library/aa366058(v=vs.85).aspx
 *
 * Also refer these:
 *   http://www.ipv6style.jp/files/ipv6/en/apps/20060320_2/GetAdaptersAddresses-EN.c
 *   https://code.woboq.org/qt5/qtbase/src/plugins/bearer/nativewifi/qnativewifiengine.cpp.html
 */
static int _pkt_win_print_GetAdaptersAddresses (void)
{
  DWORD rc = 0;
  UINT  num, num_pfx, i = 0;
  ULONG family = AF_UNSPEC;
  ULONG out_len = 0;
  ULONG flags = GAA_FLAG_INCLUDE_PREFIX |
                GAA_FLAG_INCLUDE_ALL_INTERFACES |
                GAA_FLAG_INCLUDE_WINS_INFO |
                GAA_FLAG_INCLUDE_TUNNEL_BINDINGORDER |
                GAA_FLAG_INCLUDE_GATEWAYS;

  IP_ADAPTER_ADDRESSES *addr     = NULL;
  IP_ADAPTER_ADDRESSES *old_addr = NULL;
  IP_ADAPTER_PREFIX    *prefix   = NULL;

//flags |= GAA_FLAG_SKIP_FRIENDLY_NAME;

  if (!pkt_win_init())
     return (0);

  (*_printf) ("\nFrom GetAdaptersAddresses():\n");

  /* Make an initial call to GetAdaptersAddresses() to get the
   * size needed into the out_len variable.
   */
  rc = (*p_GetAdaptersAddresses) (family, flags, NULL, NULL, &out_len);
  if (rc != ERROR_BUFFER_OVERFLOW)
  {
    (*_printf) ("  Error: %s\n", win_strerror(rc));
    return (0);
  }

  addr = alloca (out_len);

  /* Make a second call to GetAdaptersAddresses() to get the
   * actual data we want
   */
  rc = (*p_GetAdaptersAddresses) (family, flags, NULL, addr, &out_len);
  if (rc != NO_ERROR)
  {
    (*_printf) ("  Error: %s\n", win_strerror(rc));
    return (0);
  }

  old_addr = addr;
  for (num = 0; addr; addr = addr->Next)
      num++;
  (*_printf) ("  Number of adapter entries: %u\n", num);
  addr = old_addr;
  num = 0;

  while (addr)
  {
    const char *prefix_fmt;
    const char *extra_indent;

    ipv4_only_iface = TRUE;
    num++;

    if (skip_this_iface(addr, NULL, NULL))
    {
      addr = addr->Next;
      num_ifaces_skipped++;
      continue;
    }

    if (skip_filter_iface(NULL, addr->Description))
    {
      addr = addr->Next;
      num_ifaces_filtered++;
      continue;
    }

    (*_printf) ("  Adapter name:          %s (number %u)\n", addr->AdapterName, num);
    (*_printf) ("    Description:         %s\n", wstring_utf8(addr->Description));

    /* With IPv6 enabled, this is typical list for the 'Unicast Addresses:
     *
     *   Unicast Addresses:   2002:6DBD:2078:1:94F3:7B8:2773:8B31,    6DBD:2078 = IPv4 address 109.189.32.120
     *                        2002:6DBD:2078:1:69A9:4995:5BB8:A863,     -- ditto --
     *                        2002:6DBD:2078:1:99E6:3289:3A70:6B45,     -- ditto --
     *                        FE80::94F3:7B8:2773:8B31%5, 10.0.0.10
     *
     * IPv6-addresses seems to be listed first (preferred?). And 'ipconfig /all' calls these
     * "Temporary IPv6 Address."
     *
     * And the last one above is this printed as:
     * Link-local IPv6 Address . . . . . : fe80::94f3:7b8:2773:8b31%5(Preferred)
     */
    (*_printf) ("    Unicast Addresses:   %s\n", get_unicast_addrs(addr->FirstUnicastAddress));
    (*_printf) ("    Anycast Addresses:   %s\n", get_anycast_addrs(addr->FirstAnycastAddress));
    (*_printf) ("    Multicast Addresses: %s\n", get_multicast_addrs(addr->FirstMulticastAddress));
    (*_printf) ("    DNS Servers:         %s\n", format_line(get_dns_server_addrs(addr->FirstDnsServerAddress), 24));
    (*_printf) ("    DNS Suffix:          %" WIDESTR_FMT "\n", addr->DnsSuffix[0] ? addr->DnsSuffix : NONE_STR_W);

#if defined(ON_WIN_VISTA)
    if (addr->Flags & IP_ADAPTER_IPV6_ENABLED)
         iface_store_metric (&addr->Luid, addr->Ipv6Metric, AF_INET6);
    else if (addr->Flags & IP_ADAPTER_IPV4_ENABLED)
         iface_store_metric (&addr->Luid, addr->Ipv4Metric, AF_INET);

    if (_watt_os_ver < 0x0601)
       (*_printf) ("    Not Win-Vista SP1+\n");
    else
    {
      const IP_ADAPTER_DNS_SERVER_ADDRESS *dns = addr->FirstDnsServerAddress;
      int   dns_num;

      if (addr->FirstDnsSuffix)
         (*_printf) ("    1st DNS Suffix:        %" WIDESTR_FMT "\n",
                     addr->FirstDnsSuffix->String[0] ? addr->FirstDnsSuffix->String : NONE_STR_W);

      if (verbose_level >= 1)
      {
        for (dns_num = 1; dns ; dns = dns->Next, dns_num++)
        {
          SOCKADDR_INET sa;

          (*_printf) ("      GetBestRoute2() for DNS %d: ", dns_num);

          if (addr->OperStatus != IfOperStatusUp)
          {
            (*_printf) ("iface is not up.\n");
          }
          else
          {
            memset (&sa, '\0', sizeof(sa));
            memcpy (&sa, &dns->Address.lpSockaddr->sa_data, sizeof(sa));
            sa.si_family = dns->Address.lpSockaddr->sa_family;
            (*_printf) ("%s\n", get_best_route2(&addr->Luid, &sa));
          }
        }
      }
    }
#endif

    if (!(flags & GAA_FLAG_SKIP_FRIENDLY_NAME))
      (*_printf) ("    Friendly name:       %s\n", wstring_utf8(addr->FriendlyName));
    (*_printf) ("    MAC-address:         %s\n", get_phys_address(&addr->PhysicalAddress,addr->PhysicalAddressLength,TRUE));
    (*_printf) ("    Flags:               %s\n", get_address_flags(addr->Flags));
    (*_printf) ("    MTU:                 %s\n", dword_str(addr->Mtu));

#if defined(ON_WIN_VISTA)
    {
      char  speed [30];

      (*_printf) ("    Tx speed:            %s\n", speed64_string(addr->TransmitLinkSpeed, speed));
      (*_printf) ("    Rx speed:            %s\n", speed64_string(addr->ReceiveLinkSpeed, speed));
      (*_printf) ("    WINS servers:        %s\n", get_wins_addrs(addr->FirstWinsServerAddress));
      (*_printf) ("    Gateways:            %s\n", get_gateway_addrs(addr->FirstGatewayAddress));
    }

    if (addr->Flags & IP_ADAPTER_IPV4_ENABLED)
    {
      (*_printf) ("    IPv4 metric:         %lu\n", addr->Ipv4Metric);
      if (addr->Flags & IP_ADAPTER_DHCP_ENABLED)
         (*_printf) ("    DHCP v4 addr:        %s\n", socket_addr_str(&addr->Dhcpv4Server));
    }
    if (addr->Flags & IP_ADAPTER_IPV6_ENABLED)
    {
      (*_printf) ("    IPv6 metric:         %lu\n", addr->Ipv6Metric);
      if (addr->Flags & IP_ADAPTER_DHCP_ENABLED)
         (*_printf) ("    DHCP v6 addr:        %s\n", socket_addr_str(&addr->Dhcpv6Server));
    }
#endif

    (*_printf) ("    IfType:              %s (%lu)\n",
                _list_lookup(addr->IfType, if_types, DIM(if_types)),
                (u_long)addr->IfType);

    (*_printf) ("    OperStatus:          %s (%u)\n",
                _list_lookup(addr->OperStatus, oper_status, DIM(oper_status)),
                addr->OperStatus);

    (*_printf) ("    Ipv6IfIndex:         %ld\n", (long)addr->Ipv6IfIndex);

#if defined(ON_WIN_VISTA_SP1)
    (*_printf) ("    Connection type:     %s (%d)\n",
                _list_lookup(addr->ConnectionType, conn_type, DIM(conn_type)),
                addr->ConnectionType);
#endif

    (*_printf) ("    ZoneIndices:         ");
    for (i = 0; i < 16; i++)
        (*_printf) ("%lX ", (u_long)addr->ZoneIndices[i]);
    (*_printf) ("\n");

    prefix = addr->FirstPrefix;
    for (i = 0; prefix; i++)
       prefix = prefix->Next;

    num_pfx = i;
    extra_indent = (num_pfx >= 10 ? " " : "");

    prefix_fmt = (ipv4_only_iface ? "%s%15s / %-2d":
                                    "%s%46s / %-3d");

    (*_printf) ("    Num Prefix entries:  %d:\n", num_pfx);

    for (i = 0, prefix = addr->FirstPrefix; prefix; prefix = prefix->Next, i++)
    {
      (*_printf) ("      Prefix[%d]: ", i);
      (*_printf) (prefix_fmt, extra_indent, get_netmask_prefix(prefix), (int)prefix->PrefixLength);

      if (_watt_os_ver >= 0x0601)
      {
       /*
        * From https://msdn.microsoft.com/en-us/library/windows/desktop/aa366058(v=vs.85).aspx
        *
        * On Windows Vista and later, the linked IP_ADAPTER_PREFIX structures pointed to by the
        * FirstPrefix member include three IP adapter prefixes for each IP address assigned to the
        * adapter. These include the
        *   1) host IP address prefix,
        *   2) the subnet IP address prefix, and the
        *   3) subnet broadcast IP address prefix.
        *
        * In addition, for each adapter there is a multicast
        * address prefix and a broadcast address prefix.
        */
        if (i == 0)
          (*_printf) (" (Host)");
        else if (i == 1)
          (*_printf) (" (Subnet)");
        else if (i == 2)
          (*_printf) (" (Broadcast)");
        else if (i == 3)
          (*_printf) (" (Multicast)");
        else if (i == num_pfx-1)
          (*_printf) (" (All Broadcast)");
      }
      (*_printf) ("\n");
    }

    setup_info_adapter_print (addr->AdapterName);

    addr = addr->Next;
    if (addr)
       (*_printf) ("\n");
  }
  return (1);
}

static int _pkt_win_print_GetAdapterOrderMap (void)
{
  IP_ADAPTER_ORDER_MAP *order = NULL;

  if (!pkt_win_init() || !p_GetAdapterOrderMap)
     return (0);

  (*_printf) ("\nFrom GetAdapterOrderMap():\n");

  order = (*p_GetAdapterOrderMap)();
  if (order)
  {
    ULONG i;

    (*_printf) ("  NumAdapters:   %lu\n", order->NumAdapters);
    for (i = 0; i < order->NumAdapters; i++)
    {
      (*_printf) ("    Adapter: %2lu, order: %2lu", i, order->AdapterOrder[i]);
      if (order->AdapterOrder[i] == 1)
         (*_printf) (" << --");
      (*_printf) ("\n");
    }
    LocalFree (order);
  }
  return (1);
}

/*
 * From libdnet-stripped/src/route-win32.c
 */
#if defined(HAVE_NETIOAPI_H)
static const char *get_route_origin (enum _NL_ROUTE_ORIGIN origin)
{
  #undef  ADD_VALUE
  #define ADD_VALUE(v) { v, #v }
  static const struct search_list route_origins[] = {
                                  ADD_VALUE (NlroManual),
                                  ADD_VALUE (NlroWellKnown),
                                  ADD_VALUE (NlroDHCP),
                                  ADD_VALUE (NlroRouterAdvertisement),
                                  ADD_VALUE (Nlro6to4)
                                };
  return _list_lookup ((DWORD)origin, route_origins, DIM(route_origins));
}

static const char *get_route_protocol (DWORD protocol)
{
  static const struct search_list route_protocols[] = {
                                { 1,     "MIB_IPPROTO_OTHER"             },
                                { 2,     "MIB_IPPROTO_LOCAL"             },
                                { 3,     "MIB_IPPROTO_NETMGMT"           },
                                { 4,     "MIB_IPPROTO_ICMP"              },
                                { 5,     "MIB_IPPROTO_EGP"               },
                                { 6,     "MIB_IPPROTO_GGP"               },
                                { 7,     "MIB_IPPROTO_HELLO"             },
                                { 8,     "MIB_IPPROTO_RIP"               },
                                { 9,     "MIB_IPPROTO_IS_IS"             },
                                { 10,    "MIB_IPPROTO_ES_IS"             },
                                { 11,    "MIB_IPPROTO_CISCO"             },
                                { 12,    "MIB_IPPROTO_BBN"               },
                                { 13,    "MIB_IPPROTO_OSPF"              },
                                { 14,    "MIB_IPPROTO_BGP"               },
                                { 10002, "MIB_IPPROTO_NT_AUTOSTATIC"     },
                                { 10006, "MIB_IPPROTO_NT_STATIC"         },
                                { 10007, "MIB_IPPROTO_NT_STATIC_NON_DOD" }
                              };
  return _list_lookup (protocol, route_protocols, DIM(route_protocols));
}

static void print_mib_ipforward_row2 (DWORD index, const MIB_IPFORWARD_ROW2 *row, int family)
{
  const char              *common_fmt, *header_fmt;
  const char              *luid   = "   LUID:   ";
  const IP_ADDRESS_PREFIX *prefix = &row->DestinationPrefix;
  int                      size   = (family == AF_INET) ? 15 : 35;
  DWORD                    true_metric = row->Metric;

#if defined(ON_WIN_VISTA)
  true_metric += iface_lookup_metric (&row->InterfaceLuid, family);
#endif

  common_fmt = "  %2lu   %2lu     %3lu     %-3s       %-3s   %-3s  %10s  %d     ";
  header_fmt = "  %s  %s  %s  %s  %s  %s  %s        %s  %-*s  %-*s\n";
  if (index == 0)
  {
    (*_printf) (header_fmt,
                "idx",
                "iface",
                "Metric",
                "Loopback",
                "Auto",
                "Publ",
                "Age",
                "Orig" ,
                size, "NextHop",
                size, "Prefix");

    (*_printf) ("  ----------------------------------------------"
                "------------------------------------------------");
    if (family == AF_INET6)
         (*_printf) ("--------------------------------------------\n");
    else (*_printf) ("\n");
  }

  (*_printf) (common_fmt,
              index,
              row->InterfaceIndex,
              true_metric,
              row->Loopback             ? "YES" : "NO",
              row->AutoconfigureAddress ? "YES" : "NO",
              row->Publish              ? "YES" : "NO",
              hms_str(row->Age),
              (int)row->Origin);

  (*_printf) ("%-*s  %s/%d\n",
              size, get_sockaddr_inet_str(&row->NextHop),
              get_sockaddr_inet_str(&prefix->Prefix), prefix->PrefixLength );

  if (verbose_level >= 2)
  {
    if (row->ValidLifetime == 0xFFFFFFFF)
         (*_printf) ("   Lifetime:      <infinite>\n");
    else (*_printf) ("   Lifetime:      %lu\n", row->ValidLifetime);

    if (row->PreferredLifetime == 0xFFFFFFFF)
         (*_printf) ("   Pref Lifetime: <infinite>\n");
    else (*_printf) ("   Pref Lifetime: %lu\n", row->PreferredLifetime);

    (*_printf) ("   Protocol:      %s (%lu)\n", get_route_protocol(row->Protocol), row->Protocol);
    (*_printf) ("   Immortal:      %s\n", row->Immortal ? "YES" : "NO");
    (*_printf) ("   Origin:        %s (%d)\n", get_route_origin(row->Origin), (int)row->Origin);

    (*_printf) (luid);

#if defined(ON_WIN_VISTA) && defined(HAVE_NETIOAPI_H)
    print_net_luid (&row->InterfaceLuid, strlen(luid));
#endif
    (*_printf) ("\n");
  }
}

static ULONG internal_print_GetIpForwardTable2 (int family)
{
  MIB_IPFORWARD_TABLE2 *table2 = NULL;
  ULONG i, rc = (*p_GetIpForwardTable2) (family, &table2);

  if (rc != NO_ERROR)
     (*_printf) ("  Error: %s\n", win_strerror(rc));
  else
  {
    for (i = 0; table2 && i < table2->NumEntries; i++)
        print_mib_ipforward_row2 (i, table2->Table + i, family);
  }
  if (table2)
    (*p_FreeMibTable) (table2);
  return (rc);
}

static int _pkt_win_print_GetIpForwardTable2 (void)
{
  ULONG rc1, rc2;

  if (!pkt_win_init() || !p_GetIpForwardTable2 || !p_FreeMibTable)
     return (0);

  (*_printf) ("\nFrom GetIpForwardTable2(), AF_INET:\n");
  rc1 = internal_print_GetIpForwardTable2 (AF_INET);

  (*_printf) ("\nFrom GetIpForwardTable2(), AF_INET6:\n");
  rc2 = internal_print_GetIpForwardTable2 (AF_INET6);

  /* From:
   *   https://docs.microsoft.com/en-gb/windows/desktop/api/netioapi/ns-netioapi-_mib_ipforward_row2
   */
  if (rc1 == NO_ERROR || rc2 == NO_ERROR)
    (*_printf) ("\n"
                "Orig 0: A result of manual configuration.\n"
                "     1: A well-known route.\n"
                "     2: A result of DHCP configuration.\n"
                "     3: The result of router advertisement.\n"
                "     4: A result of 6to4 tunneling.\n");
  return ((rc1 == NO_ERROR || rc2 == NO_ERROR) ? 1 : 0);
}

#else
static int _pkt_win_print_GetIpForwardTable2 (void)
{
  unimplemented ("_pkt_win_print_GetIpForwardTable2", __FILE__, __LINE__);
  return (0);
}
#endif

/*
 * Return err-number+string for 'err' returned from a RAS function.
 * Remove trailing [\r\n.]
 */
static const char *ras_strerror (DWORD err)
{
  char err_buf[512], *p;

  err_buf[0] = '\0';
  if (!p_RasGetErrorStringA ||
      (*p_RasGetErrorStringA) (err,err_buf,sizeof(err_buf)) != ERROR_SUCCESS)
     strcpy (err_buf, "Unknown error");

  SNPRINTF (work_buf, sizeof(work_buf), "%lu/0x%lX %s", (u_long)err, (u_long)err, err_buf);
  rip (work_buf);
  p = strrchr (work_buf, '.');
  if (p && p[1] == '\0')
     *p = '\0';
  return (work_buf);
}


static const struct search_list auth_proto[] = {
                  { 0,             NONE_STR },
                  { RASLCPAP_PAP,  "PAP" },
                  { RASLCPAP_SPAP, "Shiva" },
                  { RASLCPAP_CHAP, "CHAP" },
                  { RASLCPAP_EAP,  "EAP" }
                };

static const struct search_list auth_data[] = {
                  { 0,                  NONE_STR },
                  { RASLCPAD_CHAP_MD5,  "CHAP MD5" },
                  { RASLCPAD_CHAP_MS,   "Microsoft CHAP" },
                  { RASLCPAD_CHAP_MSV2, "Microsoft CHAP v2" }
                };

static const struct search_list compression[] = {
                  { 0,             "<None>" },
                  { RASCCPCA_MPPC, "MPPC" },
                  { RASCCPCA_STAC, "STAC" }
                };
/*
 * Return description of "PPP Compression Control Protocol (CCP)" option
 * flags. Server or client side.
 */
static const char *get_ccp_flags (DWORD flags)
{
  char *p = work_buf;

  *p = '\0';

  if (flags == 0)
     return (NONE_STR);

  if (flags & RASCCPO_Compression)
     p += sprintf (p, "Compression/No encryption, ");

  if (flags & RASCCPO_HistoryLess)
     p += sprintf (p, "MPPE stateless, ");

  if (flags & RASCCPO_Encryption40bit)
     p += sprintf (p, "MPPE compression (40 bit keys), ");

  if (flags & RASCCPO_Encryption56bit)
     p += sprintf (p, "MPPE compression (56 bit keys), ");

  if (flags & RASCCPO_Encryption128bit)
     p += sprintf (p, "MPPE compression (128 bit keys), ");

  if (p > work_buf)
     *(p -= 2) = '\0';
  assert (p < work_buf + sizeof(work_buf));
  return (work_buf);
}

/*
 * Return description of "PPP Link Control Protocol (LCP)" option
 * flags. Server or client side.
 */
static const char *get_lcp_flags (DWORD flags)
{
  char *p = work_buf;

  *p = '\0';

  if (flags == 0)
     return (NONE_STR);

  if (flags & RASLCPO_PFC)
     p += sprintf (p, "PF compr., ");

  if (flags & RASLCPO_ACFC)
     p += sprintf (p, "ACF compr., ");

  if (flags & RASLCPO_SSHF)
     p += sprintf (p, "Short sequence num. header format, ");

  if (flags & RASLCPO_DES_56)
     p += sprintf (p, "DES encr., ");

  if (flags & RASLCPO_3_DES)
     p += sprintf (p, "3DES encr., ");

  if (flags & RASLCPO_AES_128)
    p += sprintf (p, "AES 128-bit encr., ");   /* Win7+ */

  if (flags & RASLCPO_AES_256)
    p += sprintf (p, "AES 256-bit encr., ");   /* Win7+ */

  if (p > work_buf)
     *(p -= 2) = '\0';

  assert (p < work_buf + sizeof(work_buf));
  return (work_buf);
}

#if defined(ON_WIN_VISTA)
static const char *get_best_route2 (NET_LUID *luid, const SOCKADDR_INET *dest)
{
  MIB_IPFORWARD_ROW2 row;
  SOCKADDR_INET      src;
  DWORD              rc;
  DWORD              sort_opt = 0;
  char              *p = work_buf;
  size_t             left = sizeof(work_buf);

  if (!p_GetBestRoute2)
     return ("<N/A>");

  memset (&row, '\0', sizeof(row));
  memset (&src, '\0', sizeof(src));

  p    += SNPRINTF (p, left, "(fam:%d), ", dest->si_family);
  left -= (p - work_buf);

  /* Contrary to the comment in <netioapi.h> the 'SourceAddress' can be NULL.
   * Except maybe for an IPv6-address.
   */
  if (dest->si_family == AF_INET6)
       rc = (*p_GetBestRoute2) (luid, 0, dest, dest, sort_opt, &row, &src);
  else rc = (*p_GetBestRoute2) (luid, 0, NULL, dest, sort_opt, &row, &src);

  if (rc == ERROR_NETWORK_UNREACHABLE)
        SNPRINTF (p, left, "failed: ERROR_NETWORK_UNREACHABLE");
  else if (rc != NO_ERROR)
        SNPRINTF (p, left, "failed: %s", win_strerror(rc));
  else  SNPRINTF (p, left, "%s -> %s",
                  get_sockaddr_inet_str(&src),
                  get_sockaddr_inet_str(&row.NextHop));
  return (work_buf);
}
#endif

static int _pkt_win_print_RasEnumConnections (void)
{
  DWORD     i, ret;
  DWORD     len = 0, num_conn = 0;
  RASCONN  *ras_conn = alloca (len);

  if (!pkt_win_init() || !p_RasEnumConnectionsA)
     return (0);

  (*_printf) ("\nFrom RasEnumConnections():\n");

  /* RasEnumConnections returns the handles of the current active RAS connections
   */
  ret = (*p_RasEnumConnectionsA) (NULL, &len, &num_conn);
  if (num_conn == 0)
  {
    (*_printf) ("  No active RAS connections:\n");
    return (0);
  }
  if (ret != ERROR_BUFFER_TOO_SMALL)
  {
    PRINT_RAS_ERROR ("  ", ret);
    return (0);
  }
  ras_conn = alloca (len);
  ras_conn->dwSize = sizeof(RASCONN);
  ret = (*p_RasEnumConnectionsA) (ras_conn, &len, &num_conn);
  if (ret != ERROR_SUCCESS)
  {
    PRINT_RAS_ERROR ("  ", ret);
    return (0);
  }

  for (i = 0; i < num_conn; i++, ras_conn++)
  {
    RAS_STATS  stats;
    RASPPPIPA  pppIp;
    RASPPPCCP  pppCcp;
    RASPPPLCP  pppLcp;
    char       speed [30];

    memset (&stats, 0, sizeof(stats));
    stats.dwSize = sizeof(stats);

    ret = (*p_RasGetConnectionStatistics) (ras_conn->hrasconn, &stats);
    if (ret != ERROR_SUCCESS)
    {
      PRINT_RAS_ERROR ("    ", ret);
      continue;
    }
    (*_printf) ("  Statistics for connection \"%s\":\n", ras_conn->szEntryName);
    (*_printf) ("    Bytes Xmited:            %s\n", dword_string(stats.dwBytesXmited));
    (*_printf) ("    Bytes Received:          %s\n", dword_string(stats.dwBytesRcved));
    (*_printf) ("    Frames Xmited:           %s\n", dword_string(stats.dwFramesXmited));
    (*_printf) ("    Frames Received:         %s\n", dword_string(stats.dwFramesRcved));
    (*_printf) ("    CRC Error:               %s\n", dword_string(stats.dwCrcErr));
    (*_printf) ("    Timeout Error:           %s\n", dword_string(stats.dwTimeoutErr));
    (*_printf) ("    Alignment Error:         %s\n", dword_string(stats.dwAlignmentErr));
    (*_printf) ("    Hardware Overrun Error:  %s\n", dword_string(stats.dwHardwareOverrunErr));
    (*_printf) ("    Framing Error:           %s\n", dword_string(stats.dwFramingErr));
    (*_printf) ("    Buffer Overrun Error:    %s\n", dword_string(stats.dwBufferOverrunErr));
    (*_printf) ("    Compression Ratio [In]:  %lu%%\n", (u_long)stats.dwCompressionRatioIn);
    (*_printf) ("    Compression Ratio [Out]: %lu%%\n", (u_long)stats.dwCompressionRatioOut);
    (*_printf) ("    Speed:                   %s\n", speed_string(stats.dwBps,speed));
    (*_printf) ("    Connection Duration:     %s\n", duration_string(stats.dwConnectDuration));

#if 1
    (*_printf) ("  From RasGetProjectionInfo() (PPP_IP):\n");
    memset (&pppIp, 0, sizeof(pppIp));
    len = pppIp.dwSize = sizeof(pppIp);
    ret = (*p_RasGetProjectionInfoA) (ras_conn->hrasconn, RASP_PppIp, &pppIp, &len);
    if (ret != ERROR_SUCCESS)
       PRINT_RAS_ERROR ("     ", ret);
    else
    {
      (*_printf) ("    Client IP-address:       %s\n", pppIp.szIpAddress);
      (*_printf) ("    Server IP-address:       %s\n", pppIp.szServerIpAddress);
      (*_printf) ("    PPP-negotiation error:   %lu\n", (u_long)pppIp.dwError);
      (*_printf) ("    IP Client options:       %lu\n", (u_long)pppIp.dwOptions);
      (*_printf) ("    IP Server options:       %lu\n", (u_long)pppIp.dwServerOptions);
    }

    (*_printf) ("  From RasGetProjectionInfo() (PPP_CCP):\n");
    memset (&pppCcp, 0, sizeof(pppCcp));
    len = pppCcp.dwSize = sizeof(pppCcp);
    ret = (*p_RasGetProjectionInfoA) (ras_conn->hrasconn, RASP_PppCcp, &pppCcp, &len);
    if (ret != ERROR_SUCCESS)
       PRINT_RAS_ERROR ("    ", ret);
    else
    {
      (*_printf) ("    Client compression:      %s\n", _list_lookup(pppCcp.dwCompressionAlgorithm, compression, DIM(compression)));
      (*_printf) ("    Server compression:      %s\n", _list_lookup(pppCcp.dwServerCompressionAlgorithm, compression, DIM(compression)));
      (*_printf) ("    CCP client options:      %s\n", get_ccp_flags(pppCcp.dwOptions));
      (*_printf) ("    CCP server options:      %s\n", get_ccp_flags(pppCcp.dwServerOptions));
    }

    (*_printf) ("  From RasGetProjectionInfo() (PPP_LCP):\n");
    memset (&pppLcp, 0, sizeof(pppLcp));
    len = pppLcp.dwSize = sizeof(pppLcp);
    ret = (*p_RasGetProjectionInfoA) (ras_conn->hrasconn, RASP_PppLcp, &pppLcp, &len);
    if (ret != ERROR_SUCCESS)
       PRINT_RAS_ERROR ("    ", ret);
    else
    {
      (*_printf) ("    Multilink:               %s\n", pppLcp.dwSize && pppLcp.fBundled ? "Yes" : "No");
      (*_printf) ("    Negotiation error:       %lu\n", (u_long)pppLcp.dwError);
      (*_printf) ("    Client Auth protocol:    %s\n", _list_lookup(pppLcp.dwAuthenticationProtocol, auth_proto, DIM(auth_proto)));
      (*_printf) ("    Server Auth protocol:    %s\n", _list_lookup(pppLcp.dwServerAuthenticationProtocol, auth_proto, DIM(auth_proto)));
      (*_printf) ("    Client Auth data:        %s\n", _list_lookup(pppLcp.dwAuthenticationData, auth_data, DIM(auth_data)));
      (*_printf) ("    Server Auth data:        %s\n", _list_lookup(pppLcp.dwServerAuthenticationData, auth_data, DIM(auth_data)));
      (*_printf) ("    Authentication message:  %s\n", pppLcp.szReplyMessage[0] ? pppLcp.szReplyMessage : NONE_STR);

      (*_printf) ("    EAP type:                %s\n", pppLcp.dwServerAuthenticationProtocol == RASLCPAP_EAP ?
                                                       itoa(pppLcp.dwServerEapTypeId,work_buf,10) : NA_STR);
      (*_printf) ("    LCP client options:      %s\n", get_lcp_flags(pppLcp.dwOptions));
      (*_printf) ("    LCP server options:      %s\n", get_lcp_flags(pppLcp.dwServerOptions));
    }
#endif
  }
  return (1);
}

static const struct search_list wlan_intf_state[] = {     /* table for 'WLAN_INTERFACE_STATE' */
     { wlan_interface_state_not_ready,             "Not ready" },
     { wlan_interface_state_connected,             "Connected" },
     { wlan_interface_state_ad_hoc_network_formed, "First node in a Ad Hoc network" },
     { wlan_interface_state_disconnecting,         "Disconnecting" },
     { wlan_interface_state_disconnected,          "Disconnected" },
     { wlan_interface_state_associating,           "Attempting to associate with a network" },
     { wlan_interface_state_discovering,           "Auto configuration is discovering settings for the network" },
     { wlan_interface_state_authenticating,        "Authenticating in process" },
   };

static const struct search_list wlan_conn_mode[] = {     /* table for 'WLAN_CONNECTION_MODE' */
     { wlan_connection_mode_profile,               "Profile" },
     { wlan_connection_mode_temporary_profile,     "Temporary profile" },
     { wlan_connection_mode_discovery_secure,      "Secure discovery" },
     { wlan_connection_mode_discovery_unsecure,    "Unsecure discovery" },
     { wlan_connection_mode_auto,                  "Auto" },
     { wlan_connection_mode_invalid,               "N/A" }
   };

static const struct search_list wlan_radio_states[] = {
     { dot11_radio_state_unknown,                  "Unknown" },
     { dot11_radio_state_on,                       "On" },
     { dot11_radio_state_off,                      "Off" }
   };

/*
 * Some of the MS docs [1] on the op-codes used in WlanQueryInterface(), seems wrong.
 * This .AU3 Basic-language reference seems to do it correct:
 *   https://github.com/RIEI/Vistumbler/blob/master/VistumblerMDB/UDFs/NativeWifi.au3
 *
 *  [1] https://msdn.microsoft.com/en-us/library/windows/desktop/ms706765(v=vs.85).aspx
 */
static BOOL wlan_query (HANDLE           client,
                        const GUID      *guid,
                        WLAN_INTF_OPCODE opcode,
                        size_t           data_size,
                        void            *data)
{
  void  *p = &p;
  DWORD  res;
  ULONG  size   = (ULONG)data_size;
  ULONG  p_size = sizeof(p);

  assert (size > 0);
  memset (data, '\0', size);
  res = (*p_WlanQueryInterface) (client, guid, opcode, NULL, &p_size, p, NULL);
  if (res != ERROR_SUCCESS || !p_size || !p)
  {
    (*_printf) ("WlanQueryInterface() op-code: 0x%08X (size: %lu) failed: %lu\n",
                opcode, size, res);
    return (FALSE);
  }
  memcpy (data, p, (size_t)size);
  (*p_WlanFreeMemory) (p);

  (void) &p_GetIfEntry;    /* Turn off some warnings */
  (void) &p_WlanGetInterfaceCapability;
  return (TRUE);
}

#ifdef NOT_YET
static void print_wlan_svc_guid_list (const WLAN_DEVICE_SERVICE_GUID_LIST2 *list)
{
  (*_printf) ("list->dwNumberOfItems: %lu\n", list->dwNumberOfItems);
}
#endif

static int _pkt_win_print_WlanEnumInterfaces (void)
{
  WLAN_INTERFACE_INFO_LIST *if_list = NULL;
  HANDLE client = NULL;
  DWORD  cli_ver, cur_version = 0;
  DWORD  res = 0;
  int    i;

  if (!pkt_win_init())
     return (0);

  (*_printf) ("\nFrom WlanEnumInterfaces():\n");

  if (_watt_os_ver <= _WIN32_WINNT_WS03)  /* = 0x0502, Win-XP SP3 */
       cli_ver = 1;
  else cli_ver = 2;

  res = (*p_WlanOpenHandle) (cli_ver, NULL, &cur_version, &client);
  if (res != ERROR_SUCCESS)
  {
    (*_printf) ("  WlanOpenHandle() failed: %s\n", win_strerror(res));
    (*p_WlanCloseHandle) (client, NULL);
    return (0);
  }

  (*_printf) ("  Current Version: %lu\n", cur_version);

  res = (*p_WlanEnumInterfaces) (client, NULL, &if_list);
  if (res != ERROR_SUCCESS)
  {
    (*_printf) ("  WlanEnumInterfaces() failed: %s\n", win_strerror(res));
    (*p_WlanCloseHandle) (client, NULL);
    return (0);
  }

  for (i = 0; i < (int)if_list->dwNumberOfItems; i++)
  {
    const WLAN_INTERFACE_INFO      *if_info       = (const WLAN_INTERFACE_INFO*) &if_list->InterfaceInfo[i];
    const GUID                     *guid          = &if_info->InterfaceGuid;
    WLAN_AVAILABLE_NETWORK_LIST    *network_list  = NULL;
    WLAN_BSS_LIST                  *bss_list      = NULL;
#ifdef NOT_YET
    WLAN_DEVICE_SERVICE_GUID_LIST2 *svc_guid_list = NULL;
#endif
    BOOL                            auto_conf;
    BOOL                            bkg_scan;
    BOOL                            str_mode;
    BOOL                            safe_mode;
    ULONG                           ch_number, op_mode, rssi;
    WLAN_RADIO_STATE                radio_state;
    WLAN_STATISTICS                 wlan_stats;
    WLAN_CONNECTION_ATTRIBUTES      conn_attr;
    WLAN_AUTH_CIPHER_PAIR_LIST2     auth_pairs;

    (*_printf) ("  Index:           %d\n", i);
    (*_printf) ("  Description:     %" WIDESTR_FMT "\n", if_info->strInterfaceDescription);
    (*_printf) ("  GUID:            %s\n", get_guid_str(guid));
    (*_printf) ("  State:           %s\n", _list_lookup(if_info->isState, wlan_intf_state, DIM(wlan_intf_state)));

    if (if_info->isState == wlan_interface_state_connected &&
        wlan_query(client, guid, wlan_intf_opcode_current_connection, sizeof(conn_attr), &conn_attr))
       print_wlan_current_connection (&conn_attr, 4);

    (*_printf) ("  Autoconf:           ");
    if (wlan_query(client, guid, wlan_intf_opcode_autoconf_enabled, sizeof(auto_conf), &auto_conf))
      (*_printf) ("%s\n", auto_conf ? "Yes" : "No");

    (*_printf) ("  Background scan:    ");
    if (wlan_query(client, guid, wlan_intf_opcode_background_scan_enabled, sizeof(bkg_scan), &bkg_scan))
      (*_printf) ("%s\n", bkg_scan ? "Yes" : "No");

    (*_printf) ("  Radio state:        ");
    if (wlan_query(client, guid, wlan_intf_opcode_radio_state, sizeof(radio_state), &radio_state))
       print_radio_state (&radio_state, sizeof("  Radio state:        ")-1);

    (*_printf) ("  Statistics:         ");
    if (if_info->isState != wlan_interface_state_connected)
       (*_printf) ("%s\n", NONE_STR);
    else
    if (wlan_query(client, guid, wlan_intf_opcode_statistics, sizeof(wlan_stats), &wlan_stats))
       print_wlan_stats (&wlan_stats, sizeof("  Statistics:         ")-1);

    (*_printf) ("  Channel number:     ");
    if (if_info->isState != wlan_interface_state_connected)
       (*_printf) ("%s\n", NONE_STR);
    else
    if (wlan_query(client, guid, wlan_intf_opcode_channel_number, sizeof(ch_number), &ch_number))
      (*_printf) ("%lu  (%s MHz)\n", ch_number, convert_ch_to_freq(ch_number));

    (*_printf) ("  Streaming mode:     ");
    if (wlan_query(client, guid, wlan_intf_opcode_media_streaming_mode, sizeof(str_mode), &str_mode))
      (*_printf) ("%s\n", str_mode ? "Yes" : "No");

    (*_printf) ("  Safe mode:          ");
    if (wlan_query(client, guid, wlan_intf_opcode_supported_safe_mode, sizeof(safe_mode), &safe_mode))
      (*_printf) ("%s\n", safe_mode ? "Yes" : "No");

    (*_printf) ("  Cert safe:          ");
    if (wlan_query(client, guid, wlan_intf_opcode_certified_safe_mode, sizeof(safe_mode), &safe_mode))
      (*_printf) ("%s\n", safe_mode ? "Yes" : "No");

    /*
     * \todo: Use some of this code:
     * https://github.com/nmap/npcap/blob/1329082e00a57846823f20c1b4edcdb3ae9b2701/packetWin7/WlanHelper/WlanHelper/Tool.cpp#L673
     */
    (*_printf) ("  Operation mode:     ");
    if (wlan_query(client, guid, wlan_intf_opcode_current_operation_mode, sizeof(op_mode), &op_mode))
    {
      sprintf (work_buf, "0x%08lX", op_mode);
      (*_printf) ("%s\n",
                  (op_mode == DOT11_OPERATION_MODE_EXTENSIBLE_STATION ? "Extensible station" :
                   op_mode == DOT11_OPERATION_MODE_NETWORK_MONITOR    ? "Network monitor"    : work_buf));
    }

    (*_printf) ("  RSSI:               ");
    if (wlan_query(client, guid, wlan_intf_opcode_rssi, sizeof(rssi), &rssi))
      (*_printf) ("%ld dBm\n", (long)rssi);

    (*_printf) ("  AuthCipher pairs:   ");
    if (wlan_query(client, guid, wlan_intf_opcode_supported_infrastructure_auth_cipher_pairs, sizeof(auth_pairs), &auth_pairs))
       print_auth_pairs (&auth_pairs, sizeof("  AuthCipher pairs:   ")-1);

    (*_printf) ("  Ad-hocs pairs:      ");
    if (wlan_query(client, guid, wlan_intf_opcode_supported_adhoc_auth_cipher_pairs, sizeof(auth_pairs), &auth_pairs))
       print_auth_pairs (&auth_pairs, sizeof("  Ad-hocs pairs:      ")-1);

    (*_printf) ("  \nFrom WlanGetAvailableNetworkList():\n");

    res = (*p_WlanGetAvailableNetworkList) (client, guid,
                                            WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_ADHOC_PROFILES |
                                            WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_MANUAL_HIDDEN_PROFILES,
                                            NULL, &network_list);
    if (res == ERROR_SUCCESS && network_list)
         print_wlan_networklist (network_list);
    else (*_printf) ("    error: %s\n", win_strerror(res));

    if (network_list)
      (*p_WlanFreeMemory) (network_list);

    (*_printf) ("  \n  From WlanGetNetworkBssList():\n");

    res = (*p_WlanGetNetworkBssList) (client, guid, NULL, dot11_BSS_type_any, FALSE, NULL, &bss_list);

    if (res == ERROR_SUCCESS && bss_list)
         print_wlan_bss_list (bss_list);
    else (*_printf) ("    error: %s\n", win_strerror(res));

    if (bss_list)
      (*p_WlanFreeMemory) (bss_list);

#ifdef NOT_YET   /* Does not work */
    (*_printf) ("  \n  From WlanGetSupportedDeviceServices():\n");

    res = (*p_WlanGetSupportedDeviceServices) (client, guid, &svc_guid_list);
    if (res == ERROR_SUCCESS && svc_guid_list)
         print_wlan_svc_guid_list (svc_guid_list);
    else (*_printf) ("    error: %s\n", win_strerror(res));

    if (svc_guid_list)
      (*p_WlanFreeMemory) (svc_guid_list);
#endif
  }

  if (if_list)
    (*p_WlanFreeMemory) (if_list);

  (*p_WlanCloseHandle) (client, NULL);
  return (1);
}

static const struct search_list dot11_auth_algo[] = {
     { DOT11_AUTH_ALGO_80211_OPEN,       "802.11 Open" },
     { DOT11_AUTH_ALGO_80211_SHARED_KEY, "802.11 Shared" },
     { DOT11_AUTH_ALGO_WPA,              "WPA" },
     { DOT11_AUTH_ALGO_WPA_PSK,          "WPA-PSK" },
     { DOT11_AUTH_ALGO_WPA_NONE,         "WPA-None" },
     { DOT11_AUTH_ALGO_RSNA,             "RSNA" },
     { DOT11_AUTH_ALGO_RSNA_PSK,         "RSNA with PSK (WEP2)" }
   };

static const struct search_list dot11_cipher_algo[] = {
     { DOT11_CIPHER_ALGO_NONE,           "None" },
     { DOT11_CIPHER_ALGO_WEP40,          "WEP-40" },
     { DOT11_CIPHER_ALGO_TKIP,           "TKIP" },
     { DOT11_CIPHER_ALGO_CCMP,           "CCMP" },
     { DOT11_CIPHER_ALGO_WEP104,         "WEP-104" },
     { DOT11_CIPHER_ALGO_WPA_USE_GROUP,  "WPA-Group" },
     { DOT11_CIPHER_ALGO_RSN_USE_GROUP,  "RSN-Group" },
     { DOT11_CIPHER_ALGO_WEP,            "WEP" }
  };

static const struct search_list bss_types[] = {
     { dot11_BSS_type_infrastructure,    "Infrastructure" },
     { dot11_BSS_type_independent,       "Ad-hoc" }
   };

static const struct search_list dot11_phy_types[] = {
     { dot11_phy_type_any ,              "ANY" },
     { dot11_phy_type_fhss,              "FHSS" },
     { dot11_phy_type_dsss,              "DSSS" },
     { dot11_phy_type_irbaseband,        "Infrared" },
     { dot11_phy_type_ofdm,              "OFDM" },
     { dot11_phy_type_hrdsss,            "HRDSSS" },
     { dot11_phy_type_erp,               "802.11g" },
     { dot11_phy_type_ht,                "802.11n" },
     { 8,                                "802.11ac" }  /* 'dot11_phy_type_vht'. Added in SDK 8.1? */
   };

static const char *get_phy_types (DWORD num, const DOT11_PHY_TYPE *phy)
{
  char *p   = work_buf;
  char *end = p + sizeof(work_buf) - 1;
  DWORD i;

  *p = '\0';
  if (num == 0)
     return (NONE_STR);

  for (i = 0; i < num && i < WLAN_MAX_PHY_TYPE_NUMBER; i++, phy++)
  {
    const char *str = _list_lookup (*phy, dot11_phy_types, DIM(dot11_phy_types));

    if (p + strlen(str) > end)
       break;
    p += sprintf (p, "%s, ", str);
  }
  if (p > work_buf)
     *(p -= 2) = '\0';

  if (num > WLAN_MAX_PHY_TYPE_NUMBER && p < end-4)
     strcat (p, "...");
  return (work_buf);
}

static const char *get_ssid (const DOT11_SSID *ssid)
{
  if (ssid->uSSIDLength == 0 || !isprint(ssid->ucSSID[0]))
       strncpy (work_buf, "<none>", sizeof(work_buf));
  else SNPRINTF (work_buf, sizeof(work_buf), "%.*s", (int)ssid->uSSIDLength, ssid->ucSSID);
  return (work_buf);
}

static const char *get_dot11_auth (enum _DOT11_AUTH_ALGORITHM auth)
{
  if (auth >= DOT11_AUTH_ALGO_IHV_START && auth <= DOT11_AUTH_ALGO_IHV_END)
  {
    auth -= DOT11_AUTH_ALGO_IHV_START;
    SNPRINTF (work_buf, sizeof(work_buf), "proprietary %lu", (unsigned long)auth);
    return (work_buf);
  }
  return _list_lookup (auth,dot11_auth_algo,DIM(dot11_auth_algo));
}

static const char *get_dot11_cipher (enum _DOT11_CIPHER_ALGORITHM cipher)
{
  if (cipher >= DOT11_CIPHER_ALGO_IHV_START && cipher <= DOT11_CIPHER_ALGO_IHV_END)
  {
    cipher -= DOT11_CIPHER_ALGO_IHV_START;
    SNPRINTF (work_buf, sizeof(work_buf), "proprietary %lu", (unsigned long)cipher);
    return (work_buf);
  }
  return _list_lookup (cipher,dot11_cipher_algo,DIM(dot11_cipher_algo));
}

static void print_wlan_networklist (const WLAN_AVAILABLE_NETWORK_LIST *wlist)
{
  DWORD i;

  for (i = 0; i < wlist->dwNumberOfItems; i++)
  {
    const WLAN_AVAILABLE_NETWORK *bss = (const WLAN_AVAILABLE_NETWORK*) wlist->Network + i;
    int   dBm;

    (*_printf) ("    Profile Name[%lu]:      %s%" WIDESTR_FMT "\n",
                (u_long)i, i >= 10 ? " " : "  ", bss->strProfileName[0] ? bss->strProfileName : L"<Not connected>");

    (*_printf) ("    SSID:                   %s\n", get_ssid(&bss->dot11Ssid));
    (*_printf) ("    BSS Network type:       %s\n",
                _list_lookup(bss->dot11BssType, bss_types, DIM(bss_types)));

    (*_printf) ("    Number of BSSIDs:       %lu\n", (u_long)bss->uNumberOfBssids);
    (*_printf) ("    Connectable:            ");
    if (bss->bNetworkConnectable)
         (*_printf) ("Yes\n");
    else (*_printf) ("No: not connectable. WLAN_REASON_CODE value: %lu\n",
                     (u_long)bss->wlanNotConnectableReason);

    (*_printf) ("    # Phy types supported:  %lu: %s\n",
                (u_long)bss->uNumberOfPhyTypes,
                get_phy_types(bss->uNumberOfPhyTypes,bss->dot11PhyTypes));

    if (bss->wlanSignalQuality == 0)
         dBm = -100;
    else if (bss->wlanSignalQuality == 100)
         dBm = -50;
    else dBm = -100 + (bss->wlanSignalQuality/2);

    (*_printf) ("    Signal Quality:         %lu (RSSI: %d dBm)\n",
                (u_long)bss->wlanSignalQuality, dBm);

    (*_printf) ("    Security Enabled:       %s\n",
                bss->bSecurityEnabled ? "Yes" : "No");

    (*_printf) ("    Def AuthAlgorithm:      %s\n",
                get_dot11_auth(bss->dot11DefaultAuthAlgorithm));

    (*_printf) ("    Def CipherAlgorithm:    %s\n",
                get_dot11_cipher(bss->dot11DefaultCipherAlgorithm));

    (*_printf) ("    Flags:                  0x%08lX", (u_long)bss->dwFlags);

    if (bss->dwFlags & WLAN_AVAILABLE_NETWORK_CONNECTED)
       (*_printf) (" - Currently connected");
    if (bss->dwFlags & WLAN_AVAILABLE_NETWORK_HAS_PROFILE)
       (*_printf) (" - Has profile");

    (*_printf) ("\n");
    if (i < wlist->dwNumberOfItems-1)
      (*_printf) ("\n");
  }
}

static void print_wlan_current_connection (const WLAN_CONNECTION_ATTRIBUTES *attr, int indent)
{
  const WLAN_ASSOCIATION_ATTRIBUTES *assoc = &attr->wlanAssociationAttributes;
  const WLAN_SECURITY_ATTRIBUTES    *sec   = &attr->wlanSecurityAttributes;
  char  speed [30];
  int   dBm;

  if (assoc->wlanSignalQuality == 0)
       dBm = -100;
  else if (assoc->wlanSignalQuality == 100)
       dBm = -50;
  else dBm = -100 + (assoc->wlanSignalQuality/2);

  (*_printf) ("%*sInterface state:  %s\n",
              indent, "", _list_lookup(attr->isState, wlan_intf_state, DIM(wlan_intf_state)));

  (*_printf) ("%*sConnection mode:  %s\n",
              indent, "", _list_lookup(attr->wlanConnectionMode, wlan_conn_mode, DIM(wlan_conn_mode)));

  (*_printf) ("%*sProfile name:     %" WIDESTR_FMT "\n",
              indent, "", attr->strProfileName);

  (*_printf) ("%*sAssoc SSID:       %s\n",
              indent, "", get_ssid(&assoc->dot11Ssid));

  (*_printf) ("%*sBSS Network type: %s\n",
              indent, "", _list_lookup(assoc->dot11BssType, bss_types, DIM(bss_types)));

  (*_printf) ("%*sBSSID:            %s\n",
              indent, "", get_phys_address(&assoc->dot11Bssid,DIM(assoc->dot11Bssid),TRUE));

  (*_printf) ("%*sPhysical type:    %s\n",
              indent, "", _list_lookup(assoc->dot11PhyType, dot11_phy_types, DIM(dot11_phy_types)));

  (*_printf) ("%*sSignal Quality:   %lu (RSSI: %d dBm)\n",
              indent, "", (u_long)assoc->wlanSignalQuality, dBm);

  (*_printf) ("%*sRx rate:          %s\n",
              indent, "", speed_string(1000*assoc->ulRxRate,speed));

  (*_printf) ("%*sTx rate:          %s\n",
              indent, "", speed_string(1000*assoc->ulTxRate,speed));

  (*_printf) ("%*sSecurity on:      %s\n",
              indent, "", sec->bSecurityEnabled ? "Yes" : "No");

  (*_printf) ("%*s802.1X on:        %s\n",
              indent, "", sec->bOneXEnabled     ? "Yes" : "No");

  (*_printf) ("%*sAuth algo:        %s\n",
              indent, "",  get_dot11_auth(sec->dot11AuthAlgorithm));

  (*_printf) ("%*sCipher algo:      %s\n",
              indent, "", get_dot11_cipher(sec->dot11CipherAlgorithm));
}

static void print_wlan_stats (const WLAN_STATISTICS *stats, int indent)
{
  int  i;

  (*_printf) ("4-way failures:               %s\n", qword_str(stats->ullFourWayHandshakeFailures));
  (*_printf) ("%*sTKIP countermeasures:         %s\n", indent, "", qword_str(stats->ullTKIPCounterMeasuresInvoked));

  for (i = 0; i <= 1; i++)
  {
    const WLAN_MAC_FRAME_STATISTICS *s     = (i == 0) ? &stats->MacUcastCounters : &stats->MacMcastCounters;
    const char                      *which = (i == 0) ? "Unicast " : "Mcast   ";
    const char                      *filler = "        ";

    (*_printf) ("%*s%sMAC Tx:               %s\n", indent, "", which,  qword_str(s->ullTransmittedFrameCount));
    (*_printf) ("%*s%sMAC Rx:               %s\n", indent, "", filler, qword_str(s->ullReceivedFrameCount));
    (*_printf) ("%*s%sWEP discarded:        %s\n", indent, "", filler, qword_str(s->ullWEPExcludedCount));
    (*_printf) ("%*s%sTKIP failures:        %s\n", indent, "", filler, qword_str(s->ullTKIPLocalMICFailures));
    (*_printf) ("%*s%sTKIP replay errors:   %s\n", indent, "", filler, qword_str(s->ullTKIPReplays));
    (*_printf) ("%*s%sTKIP decrypt errors:  %s\n", indent, "", filler, qword_str(s->ullTKIPICVErrorCount));
  }

#if 0
  for (i = 0; i < (int)stats->dwNumberOfPhys; i++)
  {
    const WLAN_PHY_FRAME_STATISTICS *phy = stats->PhyCounters + i;
  }
#endif
}

#if defined(NOT_USED)
/*
 * Return FILETIME in seconds as a double.
 */
static double filetime_sec (const FILETIME *filetime)
{
  const LARGE_INTEGER *ft = (const LARGE_INTEGER*) filetime;
  long double          rc = (long double) ft->QuadPart;

  return (double) (rc/1E7);    /* from 100 nano-sec periods to sec */
}
#endif  /* NOT_USED */

/*
 * Convert a frequency in the 2.4GHz band to a channel number.
 * Return -1 for an unknown channel.

 * 2.4GHz: https://en.wikipedia.org/wiki/List_of_WLAN_channels#2.4_GHz_(802.11b/g/n/ax)
 * 5GHz:   https://en.wikipedia.org/wiki/List_of_WLAN_channels#5_GHz_(802.11a/h/j/n/ac/ax)
 */
static const struct search_list ch_mapping24[] = {
                              { 2412, "1",  },
                              { 2417, "2",  },
                              { 2422, "3",  },
                              { 2427, "4",  },
                              { 2432, "5",  },
                              { 2437, "6",  },
                              { 2442, "7",  },
                              { 2447, "8",  },
                              { 2452, "9",  },
                              { 2457, "10", },
                              { 2462, "11", },
                              { 2467, "12", },
                              { 2472, "13", },
                              { 2484, "14"  }
                            };
static const struct search_list ch_mapping50[] = {
                              { 5035, "7"   },
                              { 5040, "8"   },
                              { 5045, "9"   },
                              { 5055, "11"  },
                              { 5060, "12"  },
                              { 5080, "16"  },
                              { 5160, "32"  },
                              { 5170, "34"  },
                              { 5180, "36"  },
                              { 5190, "38"  },
                              { 5200, "40"  },
                              { 5210, "42"  },
                              { 5220, "44"  },
                              { 5230, "46"  },
                              { 5240, "48"  },
                              { 5250, "50"  },
                              { 5260, "52"  },
                              { 5270, "54"  },
                              { 5280, "56"  },
                              { 5290, "58"  },
                              { 5300, "60"  },
                              { 5310, "62"  },
                              { 5320, "64"  },
                              { 5340, "68"  },
                              { 5480, "96"  },
                              { 5500, "100" },
                              { 5510, "102" },
                              { 5520, "104" },
                              { 5530, "106" },
                              { 5540, "108" },
                              { 5550, "110" },
                              { 5560, "112" },
                              { 5570, "114" },
                              { 5580, "116" },
                              { 5590, "118" },
                              { 5600, "120" },
                              { 5610, "122" },
                              { 5620, "124" },
                              { 5630, "126" },
                              { 5640, "128" },
                              { 5660, "132" },
                              { 5670, "134" },
                              { 5680, "136" },
                              { 5690, "138" },
                              { 5700, "140" },
                              { 5710, "142" },
                              { 5720, "144" },
                              { 5745, "149" },
                              { 5755, "151" },
                              { 5765, "153" },
                              { 5775, "155" },
                              { 5785, "157" },
                              { 5795, "159" },
                              { 5805, "161" },
                              { 5825, "165" },
                              { 5845, "169" },
                              { 5865, "173" },
                              { 4915, "183" },
                              { 4920, "184" },
                              { 4925, "185" },
                              { 4935, "187" },
                              { 4940, "188" },
                              { 4945, "189" },
                              { 4960, "192" },
                              { 4980, "196" }
                           };

static const char *convert_freq_to_ch (DWORD freq)
{
  char *ret;

  if (freq >= 2400 && freq <= 2500)
       ret = (char*) _list_lookup (freq, ch_mapping24, DIM(ch_mapping24));
  else if (freq >= 4980 && freq <= 5865)
       ret = (char*) _list_lookup (freq, ch_mapping50, DIM(ch_mapping50));
  else ret = (char*) "??";

  if (ret[0] == '?')
     ret[1] = '\0';
  return (ret);
}

static struct search_list *create_freq_to_ch_mapping (size_t *num_p)
{
  struct search_list *dst_list;
  char   freq[7];  /* need 4 digits, but add some slack */
  char  *freq_start;
  int    chan;
  size_t i, j, num = DIM(ch_mapping24) + DIM(ch_mapping50);
  size_t size = num * (sizeof(*dst_list) + sizeof(freq));

  *num_p = 0;
  dst_list = malloc (size);
  if (!dst_list)
     return (NULL);

  freq_start = (char*)dst_list + (num * sizeof(*dst_list)); /* Put all 'freq' after the 'dst_list[]' */

  for (i = j = 0; i < num; i++, j++)
  {
    const struct search_list *src_list = (i < DIM(ch_mapping24)) ? ch_mapping24 + 0 : ch_mapping50 + 0;

    src_list += j;
    if (i == DIM(ch_mapping24)) /* switch to 'ch_mapping50[]' */
       j = 0;

    itoa (src_list->type, freq, 10);
    chan = ATOI (src_list->name);
    dst_list[i].type = chan;
    dst_list[i].name = strcpy (freq_start, freq);
    freq_start += sizeof(freq);
  }
  *num_p = num;
  return (dst_list);
}

/*
 * The inverse of the above. Return the freq in MHz units.
 * But this is not reliable since e.g. ch 7 is in both 2.4 GHz and the 5 GHz band.
 * So we really need to know the band too.
 */
static const char *convert_ch_to_freq (int ch)
{
  static struct search_list *freq_to_ch_mapping = NULL;
  static size_t freq_to_ch_num = 0;

  if (ch < 0)
  {
    if (freq_to_ch_mapping)
       free (freq_to_ch_mapping);
    freq_to_ch_mapping = NULL;
    freq_to_ch_num = 0;
    return (NULL);
  }

  if (!freq_to_ch_mapping)
     freq_to_ch_mapping = create_freq_to_ch_mapping (&freq_to_ch_num);

  if (!freq_to_ch_mapping)
     return ("?");
  return _list_lookup (ch, freq_to_ch_mapping, freq_to_ch_num);
}

/*
 * Print the information in a list of Basic Service Set (BSS).
 */
static void print_wlan_bss_list (const WLAN_BSS_LIST *bss_list)
{
  DWORD i;

  for (i = 0; i < bss_list->dwNumberOfItems; i++)
  {
    const WLAN_BSS_ENTRY *bss = bss_list->wlanBssEntries + i;
    const char *chan;
    int   dBm;
    DWORD MHz;

    if (bss->uLinkQuality == 0)
         dBm = -100;
    else if (bss->uLinkQuality == 100)
         dBm = -50;
    else dBm = -100 + (bss->uLinkQuality/2);

    (*_printf) ("    %2lu: SSID:             %s\n"
                "        Quality:          %d dBm\n",
                i, get_ssid(&bss->dot11Ssid), dBm);

    (*_printf) ("        MAC-address:      %s\n",
                get_phys_address(&bss->dot11Bssid, DIM(bss->dot11Bssid),TRUE));

    (*_printf) ("        Phy:              %lu\n"
                "        BSS Network type: %s\n",
                bss->uPhyId, _list_lookup(bss->dot11BssType, bss_types, DIM(bss_types)));

    (*_printf) ("        Phy type:         %s\n"
                "        Capability:       0x%04X\n"
                "        Beacon-period:    %u ms, Time-stamp: %" U64_FMT "\n",
                get_phy_types(1,&bss->dot11BssPhyType),
                bss->usCapabilityInformation,
                (bss->usBeaconPeriod*1024)/1000,
                bss->ullTimestamp/1024);

    MHz = bss->ulChCenterFrequency / 1000;  /* kHz -> MHz */
    chan = convert_freq_to_ch (MHz);

    (*_printf) ("        Host time-stamp:  %s\n"
                "        Freq:             %lu MHz (ch: %s)\n",
                ULONGLONG_to_ctime(bss->ullHostTimestamp), MHz, chan);

#if 0
     rate_in_mbps = (bss->wlanRateSet[i] & 0x7FFF) * 0.5;
     (*_printf) ("    Rates:              %s\n", get_wlan_rates(bss->uRateSetLength, bss->wlanRateSet));

    /* Information element blobs
     */
    ie_start = (const BYTE*)bss + bss->ulIeOffset;
    ie_end = ie_start + bss->ulIeSize;
    print_wlan_elements (ie_start, ie_end);
#endif
    (*_printf) ("\n");
  }
}

static void print_auth_pairs (const WLAN_AUTH_CIPHER_PAIR_LIST2 *auth_list, int indent)
{
  DWORD i;

  for (i = 0; i < auth_list->dwNumberOfItems && i < DIM(auth_list->pAuthCipherPairList); i++)
  {
    const DOT11_AUTH_CIPHER_PAIR *auth = auth_list->pAuthCipherPairList + i;

    (*_printf) ("%*sauth: %-20.20s  ",  i > 0 ? indent : 0, "", get_dot11_auth(auth->AuthAlgoId));
    (*_printf) ("cipher: %-20.20s\n", get_dot11_cipher(auth->CipherAlgoId));
  }
  if (i == 0)
     (*_printf) ("%s\n", NONE_STR);
}

static void print_radio_state (const WLAN_RADIO_STATE *rs, int indent)
{
  DWORD i;

  for (i = 0; i < rs->dwNumberOfPhys && i < DIM(rs->PhyRadioState); i++)
  {
    const WLAN_PHY_RADIO_STATE *phy = rs->PhyRadioState + i;

    (*_printf) ("%*sPHY[%lu]: SW: %-7.7s",
                i > 0 ? indent : 0, "", i,
                _list_lookup(phy->dot11SoftwareRadioState, wlan_radio_states, DIM(wlan_radio_states)));

    (*_printf) ("HW: %s\n",
                _list_lookup(phy->dot11HardwareRadioState, wlan_radio_states, DIM(wlan_radio_states)));
  }
  if (i == 0)
     (*_printf) ("%*s%s\n", indent, "", NONE_STR);
}

/*
 * Helper functions for GetIpNetTable() and GetIpNetTable2().
 */
static void print_mib_ipnetrow (DWORD index, const MIB_IPNETROW *row)
{
  address_buf abuf = "?";
  ULONG       len  = min (row->dwPhysAddrLen, MAXLEN_PHYSADDR);

  if (index == 0)
     (*_printf) ("  IPv4             MAC-addr           Type\n"
                 "  --------------------------------------------\n");

  _w32_inet_ntop (_w32_AF_INET, &row->dwAddr, abuf, sizeof(abuf));
  (*_printf) ("  %-15.15s  %-17s  %s\n",
              abuf, get_phys_address(&row->bPhysAddr, len, FALSE),
              _list_lookup(row->dwType, arp_types, DIM(arp_types)));
}

static void print_mib_ipnet_row2 (DWORD index, const MIB_IPNET_ROW2 *row)
{
  ULONG       sec, len  = min (row->PhysicalAddressLength, IF_MAX_PHYS_ADDRESS_LENGTH);
  const char *mac_addr, *nw_addr;
  const char *last_reach;

  nw_addr = get_sockaddr_inet_str (&row->Address);

  if (row->IsUnreachable)
       mac_addr = NONE_STR;
  else if (all_zeroes(&row->PhysicalAddress, len))
       mac_addr = "00:...00";
  else mac_addr = get_phys_address (&row->PhysicalAddress, len, FALSE);

  sec = (DWORD)(row->ReachabilityTime.LastReachable/1000.0);
  if (sec >= 100*3600)
       last_reach = "> 10h";
  else last_reach = hms_str (sec);

  if (index == 0)
     (*_printf) ("  Iface IsRouter LastReachable IP-addr                              MAC-addr              State\n"
                 "  ----------------------------------------------------------------------------------------------------\n");

  if (row->State != NlnsUnreachable)
  {
    (*_printf) ("  %2lu    %d        %-10s    %-36.36s", row->InterfaceIndex, row->IsRouter, last_reach, nw_addr);
    (*_printf) (" %-17.17s     %s\n", mac_addr, _list_lookup(row->State, neighbour_states, DIM(neighbour_states)));
  }
}

/*
 * Problem with pulling in 'struct hostent' on CygWin.
 */
#if defined(__CYGWIN__)
  #define print_blob(blob)
#else
/*
 * Example rewritten from:
 *  https://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancednsrnr8d.html
 */
static void FixList (char ***list, const char *base)
{
  if (*list)
  {
    char **addr = *list = (char**) ((ULONG_PTR)*list + base);

    while (*addr)
    {
      *addr = (char*)(((ULONG_PTR)*addr + base));
      addr++;
    }
  }
}

static void UnpackHostEnt (struct hostent *hostent)
{
  const char *p = (char*) hostent;

  if (hostent->h_name)
     hostent->h_name = (char*) ((ULONG_PTR)hostent->h_name + p);

  FixList (&hostent->h_aliases, p);
  FixList (&hostent->h_addr_list, p);
}

static void print_blob (const BLOB *blob)
{
  struct hostent *hp = alloca (blob->cbSize);
  int    i;

  memcpy (hp, blob->pBlobData, blob->cbSize);

  UnpackHostEnt (hp);

  /* Print out the addresses
   */
  for (i = 0; hp->h_addr_list[i]; i++)
  {
    address_buf buf = { "?" };
    DWORD       ipv4 = *(DWORD*) hp->h_addr_list[i];

    _w32_inet_ntop (_w32_AF_INET, &ipv4, buf, sizeof(buf));
    (*_printf) ("  IP: %s\n", buf);
  }
  for (i = 0; hp->h_aliases[i] ;i++)
      (*_printf) ("  Alias: %s\n", hp->h_aliases[i]);
}
#endif  /* __CYGWIN__ */

/*
 * From Windows' SDK <SvcGuid.h>:
 */
#define _SVCID_HOSTNAME            { 0x0002a800, 0, 0, { 0xC0,0,0,0,0,0,0,0x46 } }
#define _SVCID_INET_HOSTADDRBYNAME { 0x0002a803, 0, 0, { 0xC0,0,0,0,0,0,0,0x46 } }

static int _pkt_win_print_WSALookupServices (void)
{
  WSAQUERYSETA q;
  DWORD        flags;
  HANDLE       h = INVALID_HANDLE_VALUE;
  BOOL         get_bth = FALSE;
  BOOL         get_NLA = FALSE;
  GUID         guid = _SVCID_HOSTNAME;
  GUID         HostnameGuid = _SVCID_INET_HOSTADDRBYNAME;
  int          err = 0;
  AFPROTOCOLS  afproto[2] = { { AF_INET, IPPROTO_UDP },
                              { AF_INET, IPPROTO_TCP }
                            };

  (*_printf) ("\nFrom %s():\n", __FUNCTION__);

  if (!p_WSALookupServiceBeginA || !p_WSALookupServiceEnd || !p_WSALookupServiceNextA)
  {
    (*_printf) ("Some needed 'WSALookupService*()' functions not found in 'ws2_32.dll'\n");
    return (0);
  }

  memset (&q, '\0', sizeof(q));
  q.dwSize = sizeof(q);

  if (get_bth)
  {
    q.dwNameSpace = NS_BTH;
    flags = LUP_CONTAINERS;
  }
  else if (get_NLA)  /* this will return a BLOB for each adapter connected (detected by the NLA) */
  {
    q.dwNameSpace = NS_NLA;
    flags = LUP_RETURN_ALL | LUP_RETURN_NAME | LUP_RETURN_COMMENT | LUP_DEEP;
  }
  else
  {
    q.dwNameSpace             = NS_ALL;
    q.lpServiceClassId        = &guid;
    q.lpServiceClassId        = &HostnameGuid;
    q.lpszServiceInstanceName = "www.microsoft.com";
    q.dwNumberOfProtocols     = DIM(afproto);
    q.lpafpProtocols          = afproto;
    flags = LUP_RETURN_ADDR | LUP_RETURN_BLOB;
  }

  if ((*p_WSALookupServiceBeginA)(&q, flags, &h) != 0)
  {
    err = (*p_WSAGetLastError)();
    (*_printf) ("WSALookupServiceBegin() failed: %s\n", win_strerror(err));
    goto quit;
  }

  while (1)
  {
    DWORD         len = sizeof(WSAQUERYSETA) + 5000;  /* should be plenty */
    WSAQUERYSETA *q_ret = alloca (len);
    const BLOB   *blob;

    memset (q_ret, '\0', len);
    q_ret->dwSize = sizeof(*q_ret);

    if ((*p_WSALookupServiceNextA)(h, 0, &len, q_ret) != NO_ERROR)
    {
      err = (*p_WSAGetLastError)();

      if (err == WSAEFAULT)
         (*_printf) ("WSALookupServiceNext(): q_ret too small; %lu needed.\n", len);

      else if (err != WSAENOMORE && err != WSA_E_NO_MORE)
         (*_printf) ("WSALookupServiceNext(): %s\n", win_strerror(err));
      break;
    }

    if (q_ret->lpszComment)
       (*_printf) ("  Comment: %s\n", q_ret->lpszComment);

    if (q_ret->lpszContext)
       (*_printf) ("  Context: %s\n", q_ret->lpszContext);

    (*_printf) ("  Service instance name: '%s'\n",  q_ret->lpszServiceInstanceName);
    (*_printf) ("  Name-space num:        %lu\n", q_ret->dwNameSpace);
    (*_printf) ("  Number of protocols:   %lu\n", q_ret->dwNumberOfProtocols);

    if (q_ret->lpVersion)
         (*_printf) ("  Version:               %u.%u\n",
                     HIWORD(q_ret->lpVersion->dwVersion),
                     LOWORD(q_ret->lpVersion->dwVersion));
    else (*_printf) ("  Version:               ?.?\n");

    if (q_ret->lpBlob)
    {
      blob = q_ret->lpBlob;
      (*_printf) ("\nBLOB:\n");

      dump_data (&blob->pBlobData, blob->cbSize, "  ");

      if (!get_bth && !get_NLA)
         print_blob (blob);
    }
  }

quit:
  if (h != INVALID_HANDLE_VALUE)
     (*p_WSALookupServiceEnd) (h);
  return (err == 0);
}

#if defined(ON_WIN_VISTA) && defined(HAVE_NETIOAPI_H)
  /*
   * Print a speed value from the 'NL_BANDWIDTH_INFORMATION' record.
   */
  static void print_bw_estimate (const char *what, LONG64 Bps, int indent)
  {
    char speed[30];

    (*_printf) ("%*s%s: %s", indent, "", what, speed64_string(Bps, speed));
  }

  /*
   * Print a NET_LUID union:
   *
   * typedef union _NET_LUID {
   *         ULONG64 Value;
   *         struct {       // bitfield with 64 bit types.
   *           ULONG64 Reserved     : 24;
   *           ULONG64 NetLuidIndex : 24;
   *           ULONG64 IfType       : 16;
   *         } Info;
   *       } NET_LUID;
   *
   */
  static void print_net_luid (const NET_LUID *luid, int indent)
  {
    NET_IFINDEX idx = 0;
    char idx_str [20] = { "?" };
    char if_name [NDIS_IF_MAX_STRING_SIZE+1] = { "?" };

    (*_printf) ("value:             0x%016" X64_FMT "\n"
                "%*sInfo.NetLuidIndex: %" U64_FMT "\n"
                "%*sInfo.IfType:       %s (%" U64_FMT ")\n",
                luid->Value,
                indent, "", (uint64)luid->Info.NetLuidIndex,
                indent, "", _list_lookup(luid->Info.IfType, if_types, DIM(if_types)),
                (uint64)luid->Info.IfType);

    if (p_ConvertInterfaceLuidToIndex && (*p_ConvertInterfaceLuidToIndex)(luid,&idx) == NO_ERROR)
       itoa (idx, idx_str, 10);
    (*_printf) ("%*sindex:             %s\n", indent, "", idx_str);

    if (p_ConvertInterfaceLuidToNameA)
       (*p_ConvertInterfaceLuidToNameA) (luid, if_name, sizeof(if_name));
    (*_printf) ("%*sif-name:           %s\n", indent, "", if_name);

    /* Get the network bandwidth rate for the adapter. A Win-8+ function.
     */
    if (idx && p_GetIpNetworkConnectionBandwidthEstimates)
    {
      MIB_IP_NETWORK_CONNECTION_BANDWIDTH_ESTIMATES bw_estimates;
      DWORD rc;

      memset (&bw_estimates, '\0', sizeof(bw_estimates));
      rc = (*p_GetIpNetworkConnectionBandwidthEstimates)(idx, AF_INET, &bw_estimates);
      if (rc == NO_ERROR)
      {
        print_bw_estimate ("In  Bandwidth", bw_estimates.InboundBandwidthInformation.Bandwidth, indent);
        print_bw_estimate (", Instability", bw_estimates.InboundBandwidthInformation.Instability, 0);
        (*_printf) ("\n");

        print_bw_estimate ("Out Bandwidth", bw_estimates.OutboundBandwidthInformation.Bandwidth, indent);
        print_bw_estimate (", Instability", bw_estimates.OutboundBandwidthInformation.Instability, 0);
        (*_printf) ("\n");
      }
      else
        (*_printf) ("%*sGetIpNetworkConnectionBandwidthEstimates(): %s\n",
                   indent, "", win_strerror(rc));
    }
  }
#endif  /* ON_WIN_VISTA && HAVE_NETIOAPI_H */

static int compare_ipnetrow (const void *_a, const void *_b)
{
  const MIB_IPNETROW *a = (const MIB_IPNETROW*)_a;
  const MIB_IPNETROW *b = (const MIB_IPNETROW*)_b;

  return (int) (a->dwType - b->dwType);
}

static int compare_ipnetrow2 (const void *_a, const void *_b)
{
  const MIB_IPNET_ROW2 *a = (const MIB_IPNET_ROW2*)_a;
  const MIB_IPNET_ROW2 *b = (const MIB_IPNET_ROW2*)_b;

  return (a->State - b->State);
}

#define DEFINE_FUNC(func) \
        int W32_CALL func (void)  \
        {                         \
          int rc = _##func();     \
          print_skipped();        \
          return (rc);            \
        }

#else
#define DEFINE_FUNC(func)       \
        int W32_CALL func (void)        \
        {                               \
          unimplemented ("_w32_" #func, \
              __FILE__, __LINE__);      \
          return (0);                   \
        }
#endif  /* COMPILE_WINADINF_C */

/*
 * Maybe these functions can be interfaced into something like:
 *   int size;
 *   struct kinfo_ndd *nddp;
 *   ...
 *   getkerninfo (KINFO_NDD, nddp, &size, 0);
 */

DEFINE_FUNC (pkt_win_print_GetIfTable)
DEFINE_FUNC (pkt_win_print_GetIfTable2)
DEFINE_FUNC (pkt_win_print_GetIfTable2Ex)
DEFINE_FUNC (pkt_win_print_GetIpNetTable)
DEFINE_FUNC (pkt_win_print_GetIpNetTable2)
DEFINE_FUNC (pkt_win_print_GetIpAddrTable)
DEFINE_FUNC (pkt_win_print_GetIpForwardTable2)
DEFINE_FUNC (pkt_win_print_GetAdaptersAddresses)
DEFINE_FUNC (pkt_win_print_GetAdapterOrderMap)
DEFINE_FUNC (pkt_win_print_RasEnumConnections)
DEFINE_FUNC (pkt_win_print_WlanEnumInterfaces)
DEFINE_FUNC (pkt_win_print_WSALookupServices)

int W32_CALL pkt_win_set_verbose_level (int level)
{
#if defined(COMPILE_WINADINF_C)
  int old = verbose_level;

  verbose_level = level;
  return (old);
#else
  return (0);
#endif
}

int W32_CALL pkt_win_get_verbose_level (void)
{
#if defined(COMPILE_WINADINF_C)
  return (verbose_level);
#else
  return (0);
#endif
}
#endif  /* WIN32 || _WIN32 */

