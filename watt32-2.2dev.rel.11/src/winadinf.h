/*!\file winadinf.h
 *
 * Getting IPHlpApi + WLanApi + RasApi Information (Win32/64).
 *
 * \note When compiling winadinf.c, this file must not include any
 *       conflicting Watt-32 headers. I.e. to get such advanced
 *       information about network adapters, only the Win-SDK must be used.
 *
 * \note It should be possible to compile this stuff for e.g. Win-Vista
 *       even if you run Win-XP SP3. Provided you have an up-to-date
 *       Windows SDK. With MSVC, the SDK version can easily be forced
 *       using the env-var "CL". E.g. "set CL=-D_WIN32_WINNT=0x601" for
 *       Win-Vista. This is done for "configur.bat visualc".
 *       The test of '_WIN32_WINNT' below ensures this variable is '0x501'
 *       or higher (Win-XP SP1+).
 *
 *       BUT this requires extreme care. The code should check against
 *       e.g. '_watt_os_ver >= 0x0601' or 'WINVER' at runtime.
 *
 *
 */
#ifndef _w32_WINADINF_H
#define _w32_WINADINF_H

#if (defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__)) && !defined(__DMC__)

#define __NETINET_IN_H           /* Don't pull in these in "../inc/" */
#define __SYS_SOCKET_H
#define __SYS_WERRNO_H
#define __SYS_WTIME_H
#define __SYS_SWAP_BYTES_H
#define _SYS_SELECT_H            /* For CygWin's <sys/select.h> */

#define _WATT32_FAKE_WINSOCK_H   /* Suppress errors from <w32api.h> */
#define _WATT32_FAKE_WINSOCK2_H
#define _WATT32_FAKE_WS2TCPIP_H
#define USE_SYS_TYPES_FD_SET     /* Shutup CygWin */

#define WATT32_COMPILING_WINADINF_C

/* We're now ready to pull in the real Winsock headers needed.
 */

#define WIN32_LEAN_AND_MEAN

/*
 * Change this for your MinGW.
 */
#if defined(__MINGW64_VERSION_MAJOR)
  #undef  _WIN32_WINNT
  #define _WIN32_WINNT 0x0601
  #undef  WINVER
  #define WINVER       _WIN32_WINNT
#endif

/*
 * OpenWatcom have rather limited Vista+ support.
 * So just leave it; fallback to Win-XP SP1+ below.
 */
#if defined(__WATCOMC__) && 0
  #undef  _WIN32_WINNT
  #define _WIN32_WINNT 0x0601
  #undef  WINVER
  #define WINVER       _WIN32_WINNT
#endif

/*
 * If wanted SDK version is not set, defaults to 'Win-XP SP1+'
 */
#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0501)
  #undef  _WIN32_WINNT
  #define _WIN32_WINNT 0x0501
  #undef  WINVER
  #define WINVER       _WIN32_WINNT
#endif

/*
 * Ditto for 'WINVER' value.
 */
#if !defined(WINVER) || (WINVER < 0x0501)
  #undef  WINVER
  #define WINVER         _WIN32_WINNT  /* Required for OpenWatcom */
  #define NTDDI_VERSION  NTDDI_VERSION_FROM_WIN32_WINNT(WINVER)
#endif

#include <windows.h>
#include <windowsx.h>
#include <winsock2.h>

/*
 * Get the maximum features out of the local "ws2tcpip.h", "iphlpapi.h" etc. headers.
 * aybe not such a good idea.
 */
#if defined(_WIN32_MAXVER) && (_WIN32_MAXVER > _WIN32_WINNT) && 0
  #undef _WIN32_WINNT
  #define _WIN32_WINNT _WIN32_MAXVER
#endif

#undef ON_WIN_VISTA
#undef ON_WIN_VISTA_SP1

#if defined(__BORLANDC__)
 /*
  * Turn off warnings:
  *  'IN6_IS_ADDR_X()' is declared but never used.
  *
  * (since these are inlined in <ws2tcpip.h>).
  */
  #pragma warn -use-

  /* Borland's <ws2tcpip.h> seems to require that '_MSC_VER' is defined ?!
   */
  #if !defined(_MSC_VER)
    #define _MSC_VER  0x1200
  #endif
#endif

#include <ws2tcpip.h>

#if defined(__BORLANDC__) || defined(__POCC__)
  #undef _MSC_VER   /* A built-in in PellesC */
#else
  #include <specstrings.h>
#endif

#if defined(__MINGW64_VERSION_MAJOR) && (_WIN32_WINNT >= 0x0601) && !defined(__DPAPI_H__)
  /*
   * This seems to be missing and required for '_WIN32_WINNT >= 0x0601' on MinGW64 and TDM-gcc.
   */
  #include <dpapi.h>
#endif

#include <objbase.h>
#include <iptypes.h>
#include <iphlpapi.h>
#include <ipifcons.h>
#include <ras.h>
#include <raserror.h>

#include <sys/cdefs.h>

#if defined(_MSC_VER)
  #define _STR2(x) #x
  #define _STR(x)  _STR2(x)

  #pragma message (__FILE__ "(" _STR(__LINE__) "): MSVC: WINVER=" _STR(WINVER))

  #if (NTDDI_VERSION >= NTDDI_VISTASP1)
    #define ON_WIN_VISTA
    #define ON_WIN_VISTA_SP1
    #pragma message (__FILE__ "(" _STR(__LINE__) "): MSVC: Using WinVista SP1+ SDK")

  #elif (NTDDI_VERSION >= NTDDI_VISTA)
    #define ON_WIN_VISTA
    #pragma message (__FILE__ "(" _STR(__LINE__) "): MSVC: Using WinVista SDK")
  #endif

#elif defined(_WIN32_WINNT)
  #define _STR2(x) #x
  #define _STR(x)  _STR2(x)

  #if ((W32_GCC_VERSION >= 40600) && (W32_GCC_VERSION < 40900))
    #pragma message ("WINVER=" _STR(WINVER))
    #pragma message (__FILE__ "(" _STR(__LINE__) "): WINVER=" _STR(WINVER))
  #endif

  #if (_WIN32_WINNT >= 0x0601)          /* Is this right? */
    #define ON_WIN_VISTA
    #define ON_WIN_VISTA_SP1

  #elif (_WIN32_WINNT >= 0x0600)
    #define ON_WIN_VISTA
  #endif
#endif

/*
 * I didn't make this compile using cl 16 and the v7.0 SDK.
 * So just don't compile it.
 */
#if defined(_MSC_VER) && (_MSC_VER >= 1700)

  #include <wtypes.h>
  #include <wlanapi.h>
  #include <windot11.h>
  #include <netioapi.h>

  #define HAVE_NETIOAPI_H
  #define HAVE_WINDOT11_H
  #define HAVE_WLANAPI_H
  #define COMPILE_WINADINF_C  /* We have all stuff need to compile winadinf.c */

#elif defined(__POCC__)
  #include <wtypes.h>
  #include <windot11.h>
  #include <netioapi.h>

  #define HAVE_NETIOAPI_H
  #define HAVE_WINDOT11_H
  #define COMPILE_WINADINF_C

#elif defined(__WATCOMC__)
  #include <wtypes.h>
  #include <wlantype.h>
  #include <windot11.h>

  #define HAVE_WINDOT11_H
  #define COMPILE_WINADINF_C

#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x0700)
  /*
   * A recent version (__BORLANDC__>= 0x0700?) of CodeGearC or Embarcadero
   * should be able to compile winadinf.c.
   */
  #include <wtypes.h>
  #include <wlanapi.h>
  #include <windot11.h>
  #include <netioapi.h>

  #define HAVE_NETIOAPI_H
  #define HAVE_WINDOT11_H
  #define HAVE_WLANAPI_H
  #define COMPILE_WINADINF_C

#elif defined(W32_IS_MINGW64 )  /* MinGW64 / TDM-gcc */
  #include <wtypes.h>
  #include <wlanapi.h>
  #include <windot11.h>
  #include <netioapi.h>

  #define HAVE_NETIOAPI_H
  #define HAVE_WLANAPI_H
  #define HAVE_WINDOT11_H
  #define COMPILE_WINADINF_C

#elif defined(__MINGW32__)      /* old-school MinGW */
  #define COMPILE_WINADINF_C

#elif defined(__CYGWIN__)       /* CygWin 32/64-bit */
  #define COMPILE_WINADINF_C
#endif

/* Assume for now, that all compilers have 'MIB_IPNETTABLE'.
 */
#define HAVE_MIB_IPNETTABLE

/* For GetIpNetTable():
 */
#if !defined(HAVE_MIB_IPNETTABLE)
  typedef struct {
          DWORD  dwIndex;
          DWORD  dwPhysAddrLen;
          BYTE   bPhysAddr [MAXLEN_PHYSADDR];  /* =8, not 6 for EtherNet */
          DWORD  dwAddr;
          DWORD  dwType;
        } MIB_IPNETROW;

  typedef struct {
          DWORD         dwNumEntries;
          MIB_IPNETROW  table [1];
        } MIB_IPNETTABLE;
#endif

/* For GetIpNetTable2() + GetIpNetTable2Ex():
 */
#if !defined(HAVE_NETIOAPI_H) && defined(COMPILE_WINADINF_C)
  typedef enum MIB_IF_TABLE_LEVEL {
          MibIfTableNormal,
          MibIfTableRaw
        } MIB_IF_TABLE_LEVEL;

  typedef struct _MIB_IF_ROW2 {
          NET_LUID              InterfaceLuid;
          NET_IFINDEX           InterfaceIndex;
          GUID                  InterfaceGuid;
          WCHAR                 Alias [IF_MAX_STRING_SIZE + 1];
          WCHAR                 Description [IF_MAX_STRING_SIZE + 1];
          ULONG                 PhysicalAddressLength;
          UCHAR                 PhysicalAddress [IF_MAX_PHYS_ADDRESS_LENGTH];
          UCHAR                 PermanentPhysicalAddress [IF_MAX_PHYS_ADDRESS_LENGTH];
          ULONG                 Mtu;
          IFTYPE                Type;
          TUNNEL_TYPE           TunnelType;
          NDIS_MEDIUM           MediaType;
          NDIS_PHYSICAL_MEDIUM  PhysicalMediumType;
          NET_IF_ACCESS_TYPE    AccessType;
          NET_IF_DIRECTION_TYPE DirectionType;
          struct {
            BOOLEAN HardwareInterface : 1;
            BOOLEAN FilterInterface   : 1;
            BOOLEAN ConnectorPresent  : 1;
            BOOLEAN NotAuthenticated  : 1;
            BOOLEAN NotMediaConnected : 1;
            BOOLEAN Paused            : 1;
            BOOLEAN LowPower          : 1;
            BOOLEAN EndPointInterface : 1;
          } InterfaceAndOperStatusFlags;

          IF_OPER_STATUS             OperStatus;
          NET_IF_ADMIN_STATUS        AdminStatus;
          NET_IF_MEDIA_CONNECT_STATE MediaConnectState;
          NET_IF_NETWORK_GUID        NetworkGuid;
          NET_IF_CONNECTION_TYPE     ConnectionType;
          ULONG64                    TransmitLinkSpeed;
          ULONG64                    ReceiveLinkSpeed;
          ULONG64                    InOctets;
          ULONG64                    InUcastPkts;
          ULONG64                    InNUcastPkts;
          ULONG64                    InDiscards;
          ULONG64                    InErrors;
          ULONG64                    InUnknownProtos;
          ULONG64                    InUcastOctets;
          ULONG64                    InMulticastOctets;
          ULONG64                    InBroadcastOctets;
          ULONG64                    OutOctets;
          ULONG64                    OutUcastPkts;
          ULONG64                    OutNUcastPkts;
          ULONG64                    OutDiscards;
          ULONG64                    OutErrors;
          ULONG64                    OutUcastOctets;
          ULONG64                    OutMulticastOctets;
          ULONG64                    OutBroadcastOctets;
          ULONG64                    OutQLen;
        } MIB_IF_ROW2;

  typedef struct _MIB_IF_TABLE2 {
          ULONG       NumEntries;
          MIB_IF_ROW2 Table [1];
        } MIB_IF_TABLE2;

  #if !defined(__WATCOMC__)
    typedef enum _NL_NEIGHBOR_STATE {
            NlnsUnreachable,
            NlnsIncomplete,
            NlnsProbe,
            NlnsDelay,
            NlnsStale,
            NlnsReachable,
            NlnsPermanent,
            NlnsMaximum,
          } NL_NEIGHBOR_STATE;
  #endif
#endif


#if !defined(HAVE_WINDOT11_H) && defined(COMPILE_WINADINF_C)
  /*
   * The DOT11 stuff ripped from OpenWatcom's <wlantype.h> and <windot11.h>.
   */
  #define DOT11_SSID_MAX_LENGTH  32 /* 802.11 SSID maximum length */

  typedef struct _DOT11_SSID {      /* 802.11 SSID */
    ULONG  uSSIDLength;
    UCHAR  ucSSID [DOT11_SSID_MAX_LENGTH];
  } DOT11_SSID;

  typedef enum _DOT11_PHY_TYPE {   /* 802.11 physical types */
    dot11_phy_type_unknown     = 0,
    dot11_phy_type_any         = dot11_phy_type_unknown,
    dot11_phy_type_fhss        = 1,
    dot11_phy_type_dsss        = 2,
    dot11_phy_type_irbaseband  = 3,
    dot11_phy_type_ofdm        = 4,
    dot11_phy_type_hrdsss      = 5,
    dot11_phy_type_erp         = 6,
    dot11_phy_type_ht          = 7,
    dot11_phy_type_vht         = 8,
    dot11_phy_type_IHV_start   = 0x80000000,
    dot11_phy_type_IHV_end     = 0xFFFFFFFF
  } DOT11_PHY_TYPE;

  typedef enum _DOT11_BSS_TYPE {   /* 802.11 BSS types */
    dot11_BSS_type_infrastructure = 1,
    dot11_BSS_type_independent    = 2,
    dot11_BSS_type_any            = 3
  } DOT11_BSS_TYPE;

  typedef enum _DOT11_AUTH_ALGORITHM {  /* 802.11 authentication algorithms */
    DOT11_AUTH_ALGO_80211_OPEN       = 1,
    DOT11_AUTH_ALGO_80211_SHARED_KEY = 2,
    DOT11_AUTH_ALGO_WPA              = 3,
    DOT11_AUTH_ALGO_WPA_PSK          = 4,
    DOT11_AUTH_ALGO_WPA_NONE         = 5,
    DOT11_AUTH_ALGO_WPA_RSNA         = 6,
    DOT11_AUTH_ALGO_WPA_RSNA_PSK     = 7,
    DOT11_AUTH_ALGO_IHV_START        = 0x80000000,
    DOT11_AUTH_ALGO_IHV_END          = 0xFFFFFFFF
  } DOT11_AUTH_ALGORITHM;

  typedef enum _DOT11_CIPHER_ALGORITHM { /* 802.11 cipher algorithms */
    DOT11_CIPHER_ALGO_NONE          = 0x00000000,
    DOT11_CIPHER_ALGO_WEP40         = 0x00000001,
    DOT11_CIPHER_ALGO_TKIP          = 0x00000002,
    DOT11_CIPHER_ALGO_CCMP          = 0x00000004,
    DOT11_CIPHER_ALGO_WEP104        = 0x00000005,
    DOT11_CIPHER_ALGO_WPA_USE_GROUP = 0x00000100,
    DOT11_CIPHER_ALGO_RSN_USE_GROUP = 0x00000100,
    DOT11_CIPHER_ALGO_WEP           = 0x00000101,
    DOT11_CIPHER_ALGO_IHV_START     = 0x80000000,
    DOT11_CIPHER_ALGO_IHV_END       = 0xFFFFFFFF
  } DOT11_CIPHER_ALGORITHM;

  typedef UCHAR DOT11_MAC_ADDRESS [6]; /* 802.11 MAC address */

#endif

#if !defined(HAVE_WLANAPI_H) && defined(COMPILE_WINADINF_C)
  #define WLAN_MAX_PHY_TYPE_NUMBER    8
  #define WLAN_MAX_NAME_LENGTH        256
  #define WLAN_MAX_PHY_INDEX          64 /* max # of PHYs supported by a NIC */

  typedef enum _WLAN_CONNECTION_MODE {
          wlan_connection_mode_profile = 0,
          wlan_connection_mode_temporary_profile,
          wlan_connection_mode_discovery_secure,
          wlan_connection_mode_discovery_unsecure,
          wlan_connection_mode_auto,
          wlan_connection_mode_invalid
        } WLAN_CONNECTION_MODE;

  typedef enum _WLAN_INTERFACE_TYPE {
          wlan_interface_type_emulated_802_11 = 0,
          wlan_interface_type_native_802_11,
          wlan_interface_type_invalid
        } WLAN_INTERFACE_TYPE;

  typedef struct _WLAN_INTERFACE_CAPABILITY {
          WLAN_INTERFACE_TYPE interfaceType;
          BOOL                bDot11DSupported;
          DWORD               dwMaxDesiredSsidListSize;
          DWORD               dwMaxDesiredBssidListSize;
          DWORD               dwNumberOfSupportedPhys;
          DOT11_PHY_TYPE      dot11PhyTypes [WLAN_MAX_PHY_INDEX];
        } WLAN_INTERFACE_CAPABILITY;

  typedef enum _WLAN_INTERFACE_STATE {
          wlan_interface_state_not_ready,
          wlan_interface_state_connected,
          wlan_interface_state_ad_hoc_network_formed,
          wlan_interface_state_disconnecting,
          wlan_interface_state_disconnected,
          wlan_interface_state_associating,
          wlan_interface_state_discovering,
          wlan_interface_state_authenticating
        } WLAN_INTERFACE_STATE;

  typedef struct _WLAN_INTERFACE_INFO {
          GUID                 InterfaceGuid;
          WCHAR                strInterfaceDescription [WLAN_MAX_NAME_LENGTH];
          WLAN_INTERFACE_STATE isState;
        } WLAN_INTERFACE_INFO;

  typedef struct _WLAN_INTERFACE_INFO_LIST {
          DWORD               dwNumberOfItems;
          DWORD               dwIndex;
          WLAN_INTERFACE_INFO InterfaceInfo [1];
        } WLAN_INTERFACE_INFO_LIST;

  typedef struct _WLAN_MAC_FRAME_STATISTICS {
          ULONGLONG  ullTransmittedFrameCount;
          ULONGLONG  ullReceivedFrameCount;
          ULONGLONG  ullWEPExcludedCount;
          ULONGLONG  ullTKIPLocalMICFailures;
          ULONGLONG  ullTKIPReplays;
          ULONGLONG  ullTKIPICVErrorCount;
          ULONGLONG  ullCCMPReplays;
          ULONGLONG  ullCCMPDecryptErrors;
          ULONGLONG  ullWEPUndecryptableCount;
          ULONGLONG  ullWEPICVErrorCount;
          ULONGLONG  ullDecryptSuccessCount;
          ULONGLONG  ullDecryptFailureCount;
        } WLAN_MAC_FRAME_STATISTICS;

  typedef struct _WLAN_PHY_FRAME_STATISTICS {
          ULONGLONG  ullTransmittedFrameCount;
          ULONGLONG  ullMulticastTransmittedFrameCount;
          ULONGLONG  ullFailedCount;
          ULONGLONG  ullRetryCount;
          ULONGLONG  ullMultipleRetryCount;
          ULONGLONG  ullMaxTXLifetimeExceededCount;
          ULONGLONG  ullTransmittedFragmentCount;
          ULONGLONG  ullRTSSuccessCount;
          ULONGLONG  ullRTSFailureCount;
          ULONGLONG  ullACKFailureCount;
          ULONGLONG  ullReceivedFrameCount;
          ULONGLONG  ullMulticastReceivedFrameCount;
          ULONGLONG  ullPromiscuousReceivedFrameCount;
          ULONGLONG  ullMaxRXLifetimeExceededCount;
          ULONGLONG  ullFrameDuplicateCount;
          ULONGLONG  ullReceivedFragmentCount;
          ULONGLONG  ullPromiscuousReceivedFragmentCount;
          ULONGLONG  ullFCSErrorCount;
        } WLAN_PHY_FRAME_STATISTICS;

  typedef struct _WLAN_STATISTICS {
          ULONGLONG                 ullFourWayHandshakeFailures;
          ULONGLONG                 ullTKIPCounterMeasuresInvoked;
          ULONGLONG                 ullReserved;
          WLAN_MAC_FRAME_STATISTICS MacUcastCounters;
          WLAN_MAC_FRAME_STATISTICS MacMcastCounters;
          DWORD                     dwNumberOfPhys;
          WLAN_PHY_FRAME_STATISTICS PhyCounters [1];
        } WLAN_STATISTICS;

  typedef enum _WLAN_INTF_OPCODE {
          wlan_intf_opcode_autoconf_start = 0x000000000,
          wlan_intf_opcode_autoconf_enabled,
          wlan_intf_opcode_background_scan_enabled,
          wlan_intf_opcode_media_streaming_mode,
          wlan_intf_opcode_radio_state,
          wlan_intf_opcode_bss_type,
          wlan_intf_opcode_interface_state,
          wlan_intf_opcode_current_connection,
          wlan_intf_opcode_channel_number,
          wlan_intf_opcode_supported_infrastructure_auth_cipher_pairs,
          wlan_intf_opcode_supported_adhoc_auth_cipher_pairs,
          wlan_intf_opcode_supported_country_or_region_string_list,
          wlan_intf_opcode_current_operation_mode,
          wlan_intf_opcode_supported_safe_mode,
          wlan_intf_opcode_certified_safe_mode,
          wlan_intf_opcode_hosted_network_capable,
          wlan_intf_opcode_autoconf_end   = 0x0FFFFFFF,
          wlan_intf_opcode_msm_start      = 0x10000100,
          wlan_intf_opcode_statistics,
          wlan_intf_opcode_rssi,
          wlan_intf_opcode_msm_end        = 0x1FFFFFFF,
          wlan_intf_opcode_security_start = 0x20010000,
          wlan_intf_opcode_security_end   = 0x2FFFFFFF,
          wlan_intf_opcode_ihv_start      = 0x30000000,
          wlan_intf_opcode_ihv_end        = 0x3FFFFFFF
        } WLAN_INTF_OPCODE;

  typedef enum _WLAN_OPCODE_VALUE_TYPE {
          wlan_opcode_value_type_query_only = 0,
          wlan_opcode_value_type_set_by_group_policy,
          wlan_opcode_value_type_set_by_user,
          wlan_opcode_value_type_invalid
        } WLAN_OPCODE_VALUE_TYPE;

  /* Structure WLAN_ASSOCIATION_ATTRIBUTES defines attributes of a wireless
   * association. The unit for Rx/Tx rate is Kbits/second.
   */
  typedef struct _WLAN_ASSOCIATION_ATTRIBUTES {
          DOT11_SSID        dot11Ssid;
          DOT11_BSS_TYPE    dot11BssType;
          DOT11_MAC_ADDRESS dot11Bssid;
          DOT11_PHY_TYPE    dot11PhyType;
          ULONG             uDot11PhyIndex;
          ULONG             wlanSignalQuality;
          ULONG             ulRxRate;
          ULONG             ulTxRate;
        } WLAN_ASSOCIATION_ATTRIBUTES;

  typedef struct _WLAN_SECURITY_ATTRIBUTES {
          BOOL                   bSecurityEnabled;
          BOOL                   bOneXEnabled;
          DOT11_AUTH_ALGORITHM   dot11AuthAlgorithm;
          DOT11_CIPHER_ALGORITHM dot11CipherAlgorithm;
        } WLAN_SECURITY_ATTRIBUTES;

  #if !defined(__WATCOMC__)
    typedef struct _DOT11_AUTH_CIPHER_PAIR {
            DOT11_AUTH_ALGORITHM   AuthAlgoId;
            DOT11_CIPHER_ALGORITHM CipherAlgoId;
          } DOT11_AUTH_CIPHER_PAIR;
  #endif

  typedef struct _WLAN_AUTH_CIPHER_PAIR_LIST {
          DWORD                  dwNumberOfItems;
          DOT11_AUTH_CIPHER_PAIR pAuthCipherPairList;
        } WLAN_AUTH_CIPHER_PAIR_LIST;

  typedef struct _WLAN_CONNECTION_ATTRIBUTES {
          WLAN_INTERFACE_STATE        isState;
          WLAN_CONNECTION_MODE        wlanConnectionMode;
          WCHAR                       strProfileName [WLAN_MAX_NAME_LENGTH];
          WLAN_ASSOCIATION_ATTRIBUTES wlanAssociationAttributes;
          WLAN_SECURITY_ATTRIBUTES    wlanSecurityAttributes;
        } WLAN_CONNECTION_ATTRIBUTES;

  typedef struct _WLAN_AVAILABLE_NETWORK {
          WCHAR          strProfileName[WLAN_MAX_NAME_LENGTH];
          DOT11_SSID     dot11Ssid;
          DOT11_BSS_TYPE dot11BssType;
          ULONG          uNumberOfBssids;
          BOOL           bNetworkConnectable;
          DWORD          wlanNotConnectableReason;
          ULONG          uNumberOfPhyTypes;
          DOT11_PHY_TYPE dot11PhyTypes[WLAN_MAX_PHY_TYPE_NUMBER];

          /* bMorePhyTypes is set to TRUE if the PHY types for the network
           * exceeds WLAN_MAX_PHY_TYPE_NUMBER.
           * In this case, uNumerOfPhyTypes is WLAN_MAX_PHY_TYPE_NUMBER and the
           * first WLAN_MAX_PHY_TYPE_NUMBER PHY types are returned.
           */
          BOOL                   bMorePhyTypes;
          ULONG                  wlanSignalQuality;
          BOOL                   bSecurityEnabled;
          DOT11_AUTH_ALGORITHM   dot11DefaultAuthAlgorithm;
          DOT11_CIPHER_ALGORITHM dot11DefaultCipherAlgorithm;
          DWORD                  dwFlags;
          DWORD                  dwReserved;
        } WLAN_AVAILABLE_NETWORK;

  typedef struct _WLAN_AVAILABLE_NETWORK_LIST {
          DWORD                  dwNumberOfItems;
          DWORD                  dwIndex;
          WLAN_AVAILABLE_NETWORK Network [1];
        } WLAN_AVAILABLE_NETWORK_LIST;

  typedef struct _WLAN_BSS_ENTRY {
          DOT11_SSID         dot11Ssid;
          ULONG              uPhyId;
          DOT11_MAC_ADDRESS  dot11Bssid;
          DOT11_BSS_TYPE     dot11BssType;
          DOT11_PHY_TYPE     dot11BssPhyType;
          LONG               lRssi;
          ULONG              uLinkQuality;
          BOOLEAN            bInRegDomain;
          USHORT             usBeaconPeriod;
          ULONGLONG          ullTimestamp;
          ULONGLONG          ullHostTimestamp;
          USHORT             usCapabilityInformation;
          ULONG              ulChCenterFrequency;
          WLAN_RATE_SET      wlanRateSet;
          ULONG              ulIeOffset;
          ULONG              ulIeSize;
        } WLAN_BSS_ENTRY;

  typedef struct _WLAN_BSS_LIST {
          DWORD          dwTotalSize;
          DWORD          dwNumberOfItems;
          WLAN_BSS_ENTRY wlanBssEntries [1];
        } WLAN_BSS_LIST;
#endif  /* HAVE_WLANAPI_H */

/*
 * Flags that control the list returned by WlanGetAvailableNetworkList
 * include all ad hoc network profiles in the available network list,
 * regardless they are visible or not
 */
#ifndef WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_ADHOC_PROFILES
#define WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_ADHOC_PROFILES         0x00000001
#endif

/*
 * Include all hidden network profiles in the available network list,
 * regardless they are visible or not
 */
#ifndef WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_MANUAL_HIDDEN_PROFILES
#define WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_MANUAL_HIDDEN_PROFILES 0x00000002
#endif

/*
 * WLAN_AVAILABLE_NETWORK::dwFlags
 */
#ifndef WLAN_AVAILABLE_NETWORK_CONNECTED
#define WLAN_AVAILABLE_NETWORK_CONNECTED             0x00000001
#endif

#ifndef WLAN_AVAILABLE_NETWORK_HAS_PROFILE
#define WLAN_AVAILABLE_NETWORK_HAS_PROFILE           0x00000002
#endif

#ifndef WLAN_AVAILABLE_NETWORK_CONSOLE_USER_PROFILE
#define WLAN_AVAILABLE_NETWORK_CONSOLE_USER_PROFILE  0x00000004
#endif

#ifndef IP_ADAPTER_NETBIOS_OVER_TCPIP_ENABLED
#define IP_ADAPTER_NETBIOS_OVER_TCPIP_ENABLED 0x0040
#endif

#ifndef IP_ADAPTER_IPV4_ENABLED
#define IP_ADAPTER_IPV4_ENABLED               0x0080
#endif

#ifndef IP_ADAPTER_IPV6_ENABLED
#define IP_ADAPTER_IPV6_ENABLED               0x0100
#endif

#ifndef GAA_FLAG_INCLUDE_ALL_INTERFACES
#define GAA_FLAG_INCLUDE_ALL_INTERFACES       0x0100L
#endif

#ifndef GAA_FLAG_INCLUDE_WINS_INFO
#define GAA_FLAG_INCLUDE_WINS_INFO            0x0040L
#endif

#ifndef GAA_FLAG_INCLUDE_GATEWAYS
#define GAA_FLAG_INCLUDE_GATEWAYS             0x0080L
#endif

#ifndef GAA_FLAG_INCLUDE_TUNNEL_BINDINGORDER
#define GAA_FLAG_INCLUDE_TUNNEL_BINDINGORDER  0x0400L
#endif

#ifndef DOT11_AUTH_ALGO_RSNA
#define DOT11_AUTH_ALGO_RSNA      6
#endif

#ifndef DOT11_AUTH_ALGO_RSNA_PSK
#define DOT11_AUTH_ALGO_RSNA_PSK  7
#endif

#ifndef DOT11_AUTH_ALGO_WPA_NONE
#define DOT11_AUTH_ALGO_WPA_NONE  5
#endif

#ifndef DOT11_OPERATION_MODE_EXTENSIBLE_STATION
#define DOT11_OPERATION_MODE_EXTENSIBLE_STATION  0x00000004
#endif

#ifndef DOT11_OPERATION_MODE_NETWORK_MONITOR
#define DOT11_OPERATION_MODE_NETWORK_MONITOR     0x80000000
#endif

#ifndef RASLCPO_AES_128
#define RASLCPO_AES_128  0x00000020
#endif

#ifndef RASLCPO_AES_256
#define RASLCPO_AES_256  0x00000040
#endif

#ifndef MAX_INTERFACE_NAME_LEN
#define MAX_INTERFACE_NAME_LEN  256
#endif

#ifndef MAXLEN_IFDESCR
#define MAXLEN_IFDESCR 256
#endif

/* Added in MS SDK 8.0?
 */
#ifndef IF_TYPE_IEEE80216_WMAN
#define IF_TYPE_IEEE80216_WMAN       237
#endif

#ifndef IF_TYPE_WWANPP
#define IF_TYPE_WWANPP               243
#endif

#ifndef IF_TYPE_WWANPP2
#define IF_TYPE_WWANPP2              244
#endif

#ifndef IF_MAX_PHYS_ADDRESS_LENGTH
#define IF_MAX_PHYS_ADDRESS_LENGTH   32
#endif

#ifndef NDIS_IF_MAX_STRING_SIZE
#define NDIS_IF_MAX_STRING_SIZE      256
#endif

#ifndef IP_ADAPTER_RECEIVE_ONLY
#define IP_ADAPTER_RECEIVE_ONLY      0x00000008
#endif

#ifndef IP_ADAPTER_NO_MULTICAST
#define IP_ADAPTER_NO_MULTICAST      0x00000010
#endif

#ifndef SCOPE_DELIMITER
#define SCOPE_DELIMITER  '%'
#endif

#ifndef MAX_IP6_SZ
#define MAX_IP6_SZ       sizeof("FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:255.255.255.255")
#endif

#if defined(INSIDE_WINADINF_C) && defined(COMPILE_WINADINF_C)
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

  /*
   * TDM-MinGW and MinGW64 got this structure wrong since the MSDN docs are
   * wrong too.
   * But since 'wlan_query()' needs to copy a complete 'WLAN_AUTH_CIPHER_PAIR_LIST2'
   * back to the caller and not simply pass a pointer back, we use this for all targets.
   */
  typedef struct {
          DWORD                  dwNumberOfItems;
          DOT11_AUTH_CIPHER_PAIR pAuthCipherPairList [16];
        } WLAN_AUTH_CIPHER_PAIR_LIST2;

  #if defined(HAVE_WLANAPI_H) && defined(_STATIC_ASSERT)
  // _STATIC_ASSERT (sizeof(WLAN_AUTH_CIPHER_PAIR_LIST2) == sizeof(WLAN_AUTH_CIPHER_PAIR_LIST));
  #endif

  #if defined(__WATCOMC__)
    typedef struct _MIB_IPNET_ROW2 {
            SOCKADDR_INET     Address;
            NET_IFINDEX       InterfaceIndex;
            NET_LUID          InterfaceLuid;
            UCHAR             PhysicalAddress[IF_MAX_PHYS_ADDRESS_LENGTH];
            ULONG             PhysicalAddressLength;
            NL_NEIGHBOR_STATE State;
            union {
              struct {
                BOOLEAN IsRouter      : 1;
                BOOLEAN IsUnreachable : 1;
              };
              UCHAR Flags;
            };
            union {
              ULONG LastReachable;
              ULONG LastUnreachable;
            } ReachabilityTime;
          } MIB_IPNET_ROW2;

    typedef struct _MIB_IPINTERFACE_ROW {
            char InterfaceLuid [1];      /* fake it */
          } MIB_IPINTERFACE_ROW;

    typedef struct _MIB_IPFORWARD_ROW2 {
            char InterfaceLuid [1];      /* fake it */
          } MIB_IPFORWARD_ROW2;

    typedef struct _MIB_IPNET_TABLE2 {
            ULONG          NumEntries;
            MIB_IPNET_ROW2 Table [1];
          } MIB_IPNET_TABLE2;

    typedef enum _DOT11_RADIO_STATE {
            dot11_radio_state_unknown,
            dot11_radio_state_on,
            dot11_radio_state_off
          } DOT11_RADIO_STATE;

    typedef struct _WLAN_PHY_RADIO_STATE {
            DWORD             dwPhyIndex;
            DOT11_RADIO_STATE dot11SoftwareRadioState;
            DOT11_RADIO_STATE dot11HardwareRadioState;
          } WLAN_PHY_RADIO_STATE;

    typedef struct _WLAN_RADIO_STATE {
            DWORD                dwNumberOfPhys;
            WLAN_PHY_RADIO_STATE PhyRadioState [64];
          } WLAN_RADIO_STATE;

    #if (_WIN32_WINNT < 0x0601)
      #define IP_ADAPTER_UNICAST_ADDRESS     IP_ADAPTER_UNICAST_ADDRESS_XP
      #define IP_ADAPTER_ANYCAST_ADDRESS     IP_ADAPTER_ANYCAST_ADDRESS_XP
      #define IP_ADAPTER_MULTICAST_ADDRESS   IP_ADAPTER_MULTICAST_ADDRESS_XP
      #define IP_ADAPTER_DNS_SERVER_ADDRESS  IP_ADAPTER_DNS_SERVER_ADDRESS_XP
      #define IP_ADAPTER_PREFIX              IP_ADAPTER_PREFIX_XP
    #endif
  #endif  /*  __WATCOMC__ */

  extern int __scope_id_to_ascii (int scope);

  static void print_wlan_networklist        (const WLAN_AVAILABLE_NETWORK_LIST *wlist);
  static void print_wlan_current_connection (const WLAN_CONNECTION_ATTRIBUTES *conn_attr, int indent);
  static void print_wlan_stats              (const WLAN_STATISTICS *stats, int indent);
  static void print_radio_state             (const WLAN_RADIO_STATE *rs, int indent);
  static void print_auth_pairs              (const WLAN_AUTH_CIPHER_PAIR_LIST2 *auth, int indent);

  static void print_mib_ipnetrow   (DWORD index, const MIB_IPNETROW *row);
  static void print_mib_ipnet_row2 (DWORD index, const MIB_IPNET_ROW2 *row);

  static int compare_ipnetrow  (const void *_a, const void *_b);
  static int compare_ipnetrow2 (const void *_a, const void *_b);

  static void print_mib_if_row (DWORD index, const MIB_IFROW *row);

  #if defined(ON_WIN_VISTA) && defined(HAVE_NETIOAPI_H)
    static void print_mib_if_row2 (DWORD index, const MIB_IF_ROW2 *row, int verbose);
    static void print_net_luid    (const NET_LUID *luid, int indent);
  #endif

  #if defined(ON_WIN_VISTA)
    static const char *get_best_route2 (IF_LUID *luid, const SOCKADDR_INET *dest);
  #endif

#endif  /* INSIDE_WINADINF_C && COMPILE_WINADINF_C */
#endif  /* (WIN32 || _WIN32 || __CYGWIN__) && !__DMC__ */
#endif  /* _w32_WINADINF_H */
