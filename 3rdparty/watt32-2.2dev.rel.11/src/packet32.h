/*
 * Copyright (c) 1999 - 2003
 * NetGroup, Politecnico di Torino (Italy)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Politecnico di Torino nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _w32_PACKET32_H
#define _w32_PACKET32_H

/* Never include old-style BPF stuff here; use the WinPcap
 * definition copied in below
 */
#define __NET_BPF_H

#ifdef __DMC__
#include <win32/wtypes.h>  /* DMC has <wtypes.h> in include path */
#endif

#include <winioctl.h>
#include <netinet/in.h>    /* struct sockaddr_storage */

/* Working modes
 */
#define PACKET_MODE_CAPT      0x0                    /* Capture mode */
#define PACKET_MODE_STAT      0x1                    /* Statistical mode */
#define PACKET_MODE_MON       0x2                    /* Monitoring mode */
#define PACKET_MODE_DUMP      0x10                   /* Dump mode */
#define PACKET_MODE_STAT_DUMP (MODE_STAT|MODE_DUMP)  /* Statistical dump Mode */

/* Loopback behaviour definitions
 */
#define NPF_DISABLE_LOOPBACK  1   /* Drop the packets sent by the NPF driver */
#define NPF_ENABLE_LOOPBACK   2   /* Capture the packets sent by the NPF driver (default) */

/*
 * Macro definition for defining IOCTL and FSCTL function control codes.  Note
 * that function codes 0-2047 are reserved for Microsoft Corporation, and
 * 2048-4095 are reserved for customers.
 */
#ifndef CTL_CODE
#define CTL_CODE(DeviceType, Function, Method, Access) ( \
        ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) )
#endif

/* Define the method codes for how buffers are passed for I/O and FS controls.
 * These constants are normally defined in <winioctl.h> and/or <winddk.h>.
 */
#ifndef METHOD_BUFFERED
#define METHOD_BUFFERED    0
#endif

#ifndef METHOD_IN_DIRECT
#define METHOD_IN_DIRECT   1
#endif

#ifndef METHOD_OUT_DIRECT
#define METHOD_OUT_DIRECT  2
#endif

#ifndef METHOD_NEITHER
#define METHOD_NEITHER     3
#endif

#if 0
  /*
   * Duplicates from <WinIoCtl.h>:
   * The FILE_READ_ACCESS and FILE_WRITE_ACCESS constants are also defined in
   * ntioapi.h as FILE_READ_DATA and FILE_WRITE_DATA. The values for these
   * constants *MUST* always be in sync.
   */
  #ifndef FILE_ANY_ACCESS
  #define FILE_ANY_ACCESS    0
  #endif

  #ifndef FILE_READ_ACCESS
  #define FILE_READ_ACCESS   0x0001    /* file & pipe */
  #endif

  #ifndef FILE_READ_ACCESS
  #define FILE_WRITE_ACCESS  0x0002    /* file & pipe */
  #endif

  /* ioctls
   */
  #ifndef FILE_DEVICE_PROTOCOL
  #define FILE_DEVICE_PROTOCOL          0x8000
  #endif

  #ifndef FILE_DEVICE_PHYSICAL_NETCARD
  #define FILE_DEVICE_PHYSICAL_NETCARD  0x0017
  #endif

  #define IOCTL_PROTOCOL_STATISTICS CTL_CODE (FILE_DEVICE_PROTOCOL, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)
  #define IOCTL_PROTOCOL_RESET      CTL_CODE (FILE_DEVICE_PROTOCOL, 3, METHOD_BUFFERED, FILE_ANY_ACCESS)
  #define IOCTL_PROTOCOL_READ       CTL_CODE (FILE_DEVICE_PROTOCOL, 4, METHOD_BUFFERED, FILE_ANY_ACCESS)
  #define IOCTL_PROTOCOL_WRITE      CTL_CODE (FILE_DEVICE_PROTOCOL, 5, METHOD_BUFFERED, FILE_ANY_ACCESS)
  #define IOCTL_PROTOCOL_MACNAME    CTL_CODE (FILE_DEVICE_PROTOCOL, 6, METHOD_BUFFERED, FILE_ANY_ACCESS)
  #define IOCTL_OPEN                CTL_CODE (FILE_DEVICE_PROTOCOL, 7, METHOD_BUFFERED, FILE_ANY_ACCESS)
  #define IOCTL_CLOSE               CTL_CODE (FILE_DEVICE_PROTOCOL, 8, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif   /* 0 */

#define pBIOCSETBUFFERSIZE        9592   /* set kernel buffer size */
#define pBIOCSETF                 9030   /* set packet filtering program */
#define pBIOCGSTATS               9031   /* get the capture stats */
#define pATTACHPROCESS            7117   /* attach a process to the driver. Used in Win9x only */
#define pDETACHPROCESS            7118   /* detach a process from the driver. Used in Win9x only */
#define pBIOCISETLOBBEH           7410   /* set the loopback behavior of the driver with packets sent by itself: capture or drop */
#define pBIOCSMODE                7412   /* set working mode */
#define pBIOCSWRITEREP            7413   /* set number of physical repetions of every packet written by the app. */
#define pBIOCSMINTOCOPY           7414   /* set minimum amount of data in the kernel buffer that unlocks a read call */
#define pBIOCEVNAME               7415   /* get the name of the event that the driver signals when some data is present in the buffer */
#define pBIOCSRTIMEOUT            7416   /* set the read timeout */
#define pBIOCSETEVENTHANDLE       7920   /* Passes the read event HANDLE allocated by user to the kernel level driver. */
#define pBIOCSETDUMPFILENAME      9029   /* set the name of a the file used by kernel dump mode */
#define pBIOCSENDPACKETSNOSYNC    9032   /* send a buffer containing multiple packets to the network, ignoring the timestamps associated with the packets */
#define pBIOCSENDPACKETSSYNC      9033   /* send a buffer containing multiple packets to the network, respecting the timestamps associated with the packets */
#define pBIOCSETDUMPLIMITS        9034   /* set the dump file limits. See the PacketSetDumpLimits() function */
#define pBIOCSETOID        2147483648U   /* set an OID value */
#define pBIOCQUERYOID      2147483652U   /* get an OID value */

/* Alignment macro. Defines the alignment size.
 */
#define Packet_ALIGNMENT sizeof(int)

/* Alignment macro. Rounds up to the next even multiple of Packet_ALIGNMENT.
 */
#define Packet_WORDALIGN(x) (((x) + (Packet_ALIGNMENT-1)) & ~(Packet_ALIGNMENT-1))

#if 0
  #define NdisMediumNull       -1   /* Custom linktype: NDIS doesn't provide an equivalent */
  #define NdisMediumCHDLC      -2   /* Custom linktype: NDIS doesn't provide an equivalent */
  #define NdisMediumPPPSerial  -3   /* Custom linktype: NDIS doesn't provide an equivalent */
  #define NdisMediumBare80211  -4   /* Custom linktype: NDIS doesn't provide an equivalent */
  #define NdisMediumRadio80211 -5   /* Custom linktype: NDIS doesn't provide an equivalent */
#endif

/**
 * Addresses of a network adapter.
 *
 * This structure is used by the PacketGetNetInfoEx() function to return
 * the IP addresses associated with an adapter.
 */
typedef struct npf_if_addr {
        struct sockaddr_storage IPAddress;
        struct sockaddr_storage SubnetMask;
        struct sockaddr_storage Broadcast;
      } npf_if_addr;


#define MAX_LINK_NAME_LENGTH   64       /* Maximum length of the devices symbolic links */
#define ADAPTER_NAME_LENGTH   (256+12)  /* Max length for the name of an adapter */
#define ADAPTER_DESC_LENGTH    128      /* Max length for the description of an adapter */
#define MAX_MAC_ADDR_LENGTH    8        /* Max length for the link layer address */
#define MAX_NETWORK_ADDRESSES  16       /* Max # of network layer addresses */

/**
 * Contains some information about a network adapter.
 */
typedef struct ADAPTER_INFO {
        struct ADAPTER_INFO *Next;
        char                 Name [ADAPTER_NAME_LENGTH+1];
        int                  NNetworkAddresses;
        struct npf_if_addr   NetworkAddresses [MAX_NETWORK_ADDRESSES];
      } ADAPTER_INFO;

/**
 * Describes an opened network adapter.
 */
typedef struct ADAPTER {
        HANDLE hFile;
        HANDLE ReadEvent;
        UINT   ReadTimeOut;
        DWORD  flags;
        struct ADAPTER_INFO *info;
      } ADAPTER;

/**
 * Values for ADAPTER::flags
 */
#define INFO_FLAG_NDIS_ADAPTER      0
#define INFO_FLAG_NDISWAN_ADAPTER   1
#define INFO_FLAG_AIRPCAP_CARD     16
#define INFO_FLAG_NPFIM_DEVICE     32

/**
 * Structure containing an OID request.
 *
 * It is used by the PacketRequest() function to send an OID to the interface
 * card driver. It can be used, for example, to retrieve the status of the error
 * counters on the adapter, its MAC address, the list of the multicast groups
 * defined on it, and so on.
 */
typedef struct PACKET_OID_DATA {
        DWORD  Oid;
        DWORD  Length;
        BYTE   Data[1];
      } PACKET_OID_DATA;

W32_CLANG_PACK_WARN_OFF()

#include <sys/pack_on.h>

/* The 'struct timeval' layout of a 32-bit NPF.SYS driver.
 * It can NOT be 'long' to work on a 64-bit application.
 */
struct npf_driver_timeval {
       int32_t  tv_sec;
       int32_t  tv_usec;
     };

/**
 * Berkeley Packet Filter header.
 *
 * This structure defines the header associated with every packet delivered to the application.
 */
struct bpf_hdr {
       struct npf_driver_timeval bh_tstamp; /* The timestamp associated with the captured packet */
       DWORD  bh_caplen;                    /* Length of captured portion */
       DWORD  bh_datalen;                   /* Original length of packet */
       WORD   bh_hdrlen;                    /* Length of bpf header (this struct plus alignment padding) */
     };


/* The receive statistics structure from NPF.
 */
struct bpf_stat {
       DWORD  bs_recv;    /* # of packets that the driver received from the network adapter */
       DWORD  bs_drop;    /* # of packets that the driver lost from the beginning of a capture */
       DWORD  ps_ifdrop;  /* drops by interface. XXX not yet supported */
       DWORD  bs_capt;    /* # of packets that pass the filter */
     };

#include <sys/pack_off.h>

W32_CLANG_PACK_WARN_DEF()

/*
 * Protect these symbols with a namespace. Thus allowing Watt-32 (static) to
 * be used in libpcap/WinPcap programs.
 */

#define PacketInitModule          W32_NAMESPACE (PacketInitModule)
#define PacketExitModule          W32_NAMESPACE (PacketExitModule)
#define PacketOpenAdapter         W32_NAMESPACE (PacketOpenAdapter)
#define PacketFindAdInfo          W32_NAMESPACE (PacketFindAdInfo)
#define PacketGetAdInfo           W32_NAMESPACE (PacketGetAdInfo)
#define PacketCloseAdapter        W32_NAMESPACE (PacketCloseAdapter)
#define PacketRequest2            W32_NAMESPACE (PacketRequest2)
#define PacketInstallDriver       W32_NAMESPACE (PacketInstallDriver)
#define PacketGetDriverVersion    W32_NAMESPACE (PacketGetDriverVersion)
#define PacketGetMacAddress       W32_NAMESPACE (PacketGetMacAddress)
#define PacketSetMode             W32_NAMESPACE (PacketSetMode)
#define PacketSetBuff             W32_NAMESPACE (PacketSetBuff)
#define PacketSetMinToCopy        W32_NAMESPACE (PacketSetMinToCopy)
#define PacketSetReadTimeout      W32_NAMESPACE (PacketSetReadTimeout)
#define PacketGetReadEvent        W32_NAMESPACE (PacketGetReadEvent)
#define PacketGetStatsEx          W32_NAMESPACE (PacketGetStatsEx)
#define PacketReceivePacket       W32_NAMESPACE (PacketReceivePacket)
#define PacketSendPacket          W32_NAMESPACE (PacketSendPacket)
#define PacketSetLoopbackBehavior W32_NAMESPACE (PacketSetLoopbackBehavior)

BOOL PacketInitModule (void);
BOOL PacketExitModule (void);

const ADAPTER      *PacketOpenAdapter (const char *adapter_name);
const ADAPTER_INFO *PacketFindAdInfo (const char *adapter_name);
const ADAPTER_INFO *PacketGetAdInfo (void);
BOOL                PacketCloseAdapter (ADAPTER *adapter);
const char         *PacketGetDriverVersion (void);

BOOL PacketRequest2 (const ADAPTER *adapter, BOOL Set, PACKET_OID_DATA *OidData,
                     const char *file, unsigned line);

#define PacketRequest(a,set,oid)  PacketRequest2 (a, set, oid, __FILE__, __LINE__)

BOOL   PacketGetMacAddress (const ADAPTER *adapter, void *mac);
BOOL   PacketSetMode (const ADAPTER *adapter, DWORD mode);
BOOL   PacketSetBuff (const ADAPTER *adapter, DWORD dim);
BOOL   PacketSetMinToCopy (const ADAPTER *adapter, int nbytes);
BOOL   PacketSetReadTimeout (ADAPTER *adapter, int timeout);
HANDLE PacketGetReadEvent (const ADAPTER *adapter);
BOOL   PacketGetStatsEx (const ADAPTER *adapter, struct bpf_stat *st);
UINT   PacketSendPacket    (const ADAPTER *adapter, const void *buf, UINT len);
BOOL   PacketSetLoopbackBehavior (const ADAPTER *adapter, UINT LoopbackBehavior);

UINT MS_CDECL PacketReceivePacket (const ADAPTER *adapter, void *buf, UINT buf_len);

/* WanPacket stuff (move to a new WanPacket.h?)
 */

typedef void WAN_ADAPTER;  /* The details of this struct is not important to us.
                            * Ref. WAN_ADAPTER_INT in WanPacket.cpp (from CACE Technologies)
                            */

/*
 * Protect these symbols in our own namespace in case we use the real
 * WinPcap/libpcap with a static version of Watt-32.
 */
#define WanPacketSetBpfFilter         W32_NAMESPACE (WanPacketSetBpfFilter)
#define WanPacketOpenAdapter          W32_NAMESPACE (WanPacketOpenAdapter)
#define WanPacketCloseAdapter         W32_NAMESPACE (WanPacketCloseAdapter)
#define WanPacketSetBufferSize        W32_NAMESPACE (WanPacketSetBufferSize)
#define WanPacketReceivePacket        W32_NAMESPACE (WanPacketReceivePacket)
#define WanPacketSetMinToCopy         W32_NAMESPACE (WanPacketSetMinToCopy)
#define WanPacketGetStats             W32_NAMESPACE (WanPacketGetStats)
#define WanPacketSetReadTimeout       W32_NAMESPACE (WanPacketSetReadTimeout)
#define WanPacketSetMode              W32_NAMESPACE (WanPacketSetMode)
#define WanPacketGetReadEvent         W32_NAMESPACE (WanPacketGetReadEvent)
#define WanPacketTestAdapter          W32_NAMESPACE (WanPacketTestAdapter)

BOOL         WanPacketSetBpfFilter (WAN_ADAPTER *wan_adapter, PUCHAR FilterCode, DWORD Length);
WAN_ADAPTER *WanPacketOpenAdapter (void);
BOOL         WanPacketCloseAdapter (WAN_ADAPTER *wan_adapter);
BOOL         WanPacketSetBufferSize (WAN_ADAPTER *wan_adapter, DWORD BufferSize);
DWORD        WanPacketReceivePacket (WAN_ADAPTER *wan_adapter, PUCHAR Buffer, DWORD BufferSize);
BOOL         WanPacketSetMinToCopy (WAN_ADAPTER *wan_adapter, DWORD MinToCopy);
BOOL         WanPacketGetStats (WAN_ADAPTER *wan_adapter, struct bpf_stat *s);
BOOL         WanPacketSetReadTimeout (WAN_ADAPTER *wan_adapter, DWORD ReadTimeout);
BOOL         WanPacketSetMode (WAN_ADAPTER *wan_adapter, DWORD Mode);
HANDLE       WanPacketGetReadEvent (WAN_ADAPTER *wan_adapter);
BOOL         WanPacketTestAdapter (void);

#endif  /* _w32_PACKET32_H */
