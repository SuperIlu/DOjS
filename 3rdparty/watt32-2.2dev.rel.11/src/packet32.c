/*
 * Copyright (c) 1999 - 2003
 * Politecnico di Torino.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the Politecnico
 * di Torino, and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

 /*
  * Changes for Watt-32:
  *  - Merged Packet32.c and AdInfo.c into one.
  *  - Rewritten for ASCII function (no Unicode).
  *  - Lots of simplifications.
  */
#include "wattcp.h"

#if defined(WIN32)  /* Rest of file */

#include <windows.h>
#include <windowsx.h>       /* GlobalAllocPtr() */
#include <winsvc.h>         /* SERVICE_ALL_ACCESS... */
#include <sys/socket.h>     /* AF_INET etc. */
#include <process.h>

#include "misc.h"
#include "timer.h"
#include "profile.h"
#include "strings.h"
#include "pcconfig.h"
#include "pcdbug.h"
#include "pcpkt.h"
#include "netaddr.h"
#include "w32_ndis.h"
#include "cpumodel.h"
#include "packet32.h"
#include "winpkt.h"
#include "win_dll.h"

/**
 * Current NPF.SYS version.
 */
static char npf_drv_ver[64] = "Unknown npf.sys version";

/**
 * Name constants (NPF).
 */
static const char NPF_service_name[]      = "NPF";
static const char NPF_service_descr[]     = "Netgroup Packet Filter";
static const char NPF_registry_location[] = "SYSTEM\\CurrentControlSet\\Services\\NPF";
static const char NPF_prefix[]            = "\\Device\\NPF_";
static const char NPF_driver_path[]       = "system32\\drivers\\npf.sys";
static const char NPF_virtual_path[]      = "sysnative\\drivers\\npf.sys";

/**
 * Name constants (Win10Pcap).
 */
static const char Win10Pcap_service_name[]       = "Win10Pcap";
static const char Win10Pcap_service_descr[]      = "Win10Pcap Packet Capture Driver";
static const char Win10Pcap_registry_location[]  = "SYSTEM\\ControlSet\\Services\\Win10Pcap";
static const char Win10Pcap_prefix[]             = "\\Device\\WTCAP_A_";
static const char Win10Pcap_driver_path[]        = "system32\\drivers\\win10pcap.sys";
static const char Win10Pcap_virtual_path[]       = "sysnative\\drivers\\win10pcap.sys";

/**
 * Defaults to NPF.
 */
static const char *service_name      = NPF_service_name;
static const char *service_descr     = NPF_service_descr;
static const char *registry_location = NPF_registry_location;
static const char *driver_prefix     = NPF_prefix;
static const char *driver_path       = NPF_driver_path;
static const char *virtual_path      = NPF_virtual_path;


#define ADD_VALUE(v)  { v, #v }

static const struct search_list serv_stat[] = {
                    ADD_VALUE (SERVICE_CONTINUE_PENDING),
                    ADD_VALUE (SERVICE_PAUSE_PENDING),
                    ADD_VALUE (SERVICE_PAUSED),
                    ADD_VALUE (SERVICE_RUNNING),
                    ADD_VALUE (SERVICE_START_PENDING),
                    ADD_VALUE (SERVICE_STOP_PENDING),
                    ADD_VALUE (SERVICE_STOPPED)
                  };

#define DEVICE_PREFIX  "\\Device\\"

/*
 * The {4D36E972-E325-11CE-BFC1-08002BE10318} subkey represents the class
 * of network adapter devices that the system supports.
 */
#define ADAPTER_KEY_CLASS \
        "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}"


/**
 * Head of the adapter information list. This list is populated
 * by AddAdapter().
 */
static ADAPTER_INFO *adapters_list = NULL;

/**
 * Mutex that protects the adapter information list.
 * \note every API that takes an ADAPTER_INFO as parameter assumes
 *       that it has been called with the mutex acquired.
 */
static HANDLE adapters_mutex = INVALID_HANDLE_VALUE;

/**
 * The WanPacket stuff; we can probably not send on this interface, but it
 * would be nice to sniff on it.
 */
static BOOL use_wanpacket = FALSE;

static BOOL PopulateAdaptersInfoList (void);
static BOOL FreeAdaptersInfoList (void);

static void set_char_pointers (const char *adapter)
{
  size_t len = strlen (Win10Pcap_prefix);

  if (!strnicmp(adapter,Win10Pcap_prefix,len))
  {
    service_name      = Win10Pcap_service_name;
    service_descr     = Win10Pcap_service_descr;
    registry_location = Win10Pcap_registry_location;
    driver_prefix     = Win10Pcap_prefix;
    driver_path       = Win10Pcap_driver_path;
    virtual_path      = Win10Pcap_virtual_path;
  }
  else
  {
    service_name      = NPF_service_name;
    service_descr     = NPF_service_descr;
    registry_location = NPF_registry_location;
    driver_prefix     = NPF_prefix;
    driver_path       = NPF_driver_path;
    virtual_path      = NPF_virtual_path;
  }
}

#if defined(USE_DEBUG)
  /**
   * Dumps a registry key to disk in text format. Uses regedit.
   *
   * \param key_name Name of the key to dump. All its subkeys will be
   *        saved recursively.
   * \param file_name Name of the file that will contain the dump.
   * \return If the function succeeds, the return value is nonzero.
   *
   * For debugging purposes, we use this function to obtain some registry
   * keys from the user's machine.
   */
  static void PacketDumpRegistryKey (const char *key_name, const char *file_name)
  {
    char command[256];

    /* Let regedit do the dirty work for us
     */
    SNPRINTF (command, sizeof(command), "regedit /e %s %s", file_name, key_name);
    system (command);
  }
#endif

/*
 * The get_file_version() fails under Win-Vista+ since files under
 * '"%SystemRoot%\system32\drivers' are hidden from non-admin users.
 * So this function is used instead. I assume the true file-version of
 * NPF.sys is the same as "DisplayVersion" in Registry.
 */
static BOOL get_npf_ver_from_registry (char *ret_ver, size_t ver_size)
{
  char  str[100];
  BOOL  rc = FALSE;
  DWORD size = sizeof(str);
  HKEY  key = NULL;
  LONG  status;

  status = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                         "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\WinPcapInst",
                         0, KEY_READ, &key);

  /* Note: WinPcap may be installed and working even though this key doesn't exist.
   */
  if (status != ERROR_SUCCESS)
     goto fail;

  status = RegQueryValueEx (key, "DisplayVersion", NULL, NULL, (BYTE*)&str, &size);
  if (status != ERROR_SUCCESS)
     goto fail;

  _strlcpy (ret_ver, str, ver_size);
  rc = TRUE;

fail:
  if (key)
     RegCloseKey (key);
  return (rc);
}

#if 0
/**
 * \todo:
 * Do the same as these command does:
 *   if 'sc GetDisplayName npf' succeedes, follow by a
 *  'sc showsid npf'. Giving:
 *
 *   NAME: npf
 *   SERVICE SID: S-1-5-80-1598306103-1873062032-3786967184-80952375-3176933300
 *   STATUS: Inactive
 */
static BOOL npf_sc_showsid (char *sid_buf, size_t sid_size)
{
}

/*
 * And as 'sc getdisplayname npf' does:
 *   [SC] GetServiceDisplayName SUCCESS
 *   Name = WinPcap Packet Driver (NPF)
 */
static BOOL npf_sc_getdisplayname (char *name_buf, size_t name_size)
{
}
#endif

/**
 * The winpcap init function (formerly DllMain).
 */
BOOL PacketInitModule (void)
{
  const struct ADAPTER_INFO *ai;
  BOOL  rc = FALSE;

  winpkt_trace_func = "PacketInitModule";
  WINPKT_TRACE ("\n");

#if defined(USE_DEBUG)
  if (winpkt_trace_level >= 3 && !_watt_is_win9x)
  {
    /* dump a bunch of registry keys
     */
    PacketDumpRegistryKey (
      "HKEY_LOCAL_MACHINE" ADAPTER_KEY_CLASS, "adapters.reg");
    PacketDumpRegistryKey ("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet"
                           "\\Services\\Tcpip", "tcpip.reg");
    PacketDumpRegistryKey ("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet"
                           "\\Services\\NPF", "npf.reg");
    PacketDumpRegistryKey ("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet"
                           "\\Services", "services.reg");
  }
#endif

  /* Create the mutex that will protect the adapter information list
   */
  adapters_mutex = CreateMutex (NULL, FALSE, NULL);

  /*
   * Retrieve NPF.sys version information from the file.
   *
   * \todo: Should maybe use an absolute path: "%SystemRoot%\system32\drivers\npf.sys" ?
   *        Or we can assume %SystemRoot% is always in %PATH?
   *
   * The get_file_version() fails under Win-Vista+ since files under
   * '"%SystemRoot%\system32\drivers' are hidden from non-admin users.
   * Hence try the "%SystemRoot%\sysnative\drivers" directory first
   * (a virtual directory). If that fails (or on < Vista), try
   * "%SystemRoot%\system32\drivers".
   *
   * Instead of checking "%SystemRoot%\sysnative\drivers" directly, MSDN
   * would recommend we use:
   *   Wow64DisableWow64FsRedirection (...)
   *   get_file_version (driver_path,...)
   *   Wow64EnableWow64FsRedirection (....)
   *
   * But this can led to strange bugs.
   *
   * Refs: https://msdn.microsoft.com/en-us/library/windows/desktop/aa365743(v=vs.85).aspx
   *       https://blogs.msdn.microsoft.com/oldnewthing/20130321-00/?p=4883/
   *
   * If both attempts fails, simply try to get the version from Registry.
   *
   * Ref. Section "Registry and file system" at:
   *   http://en.wikipedia.org/wiki/WoW64
   */
  if (_watt_os_ver >= 0x0600)
     rc = get_file_version (virtual_path, npf_drv_ver, sizeof(npf_drv_ver));

  if (!rc)
     rc = get_file_version (driver_path, npf_drv_ver, sizeof(npf_drv_ver));

  WINPKT_TRACE ("get_file_version(): rc=%d, npf_drv_ver=\"%s\"\n", rc, npf_drv_ver);

  if (!rc)
  {
    rc = get_npf_ver_from_registry (npf_drv_ver, sizeof(npf_drv_ver));
    WINPKT_TRACE ("get_npf_ver_from_registry(): rc=%d, npf_drv_ver=\"%s\"\n",
                  rc, npf_drv_ver);
  }

  /* Populate the 'adapters_list' list.
   */
  rc = PopulateAdaptersInfoList();

  winpkt_trace_func = "PacketInitModule";
  WINPKT_TRACE ("Known WinPcap adapters:\n");

  for (ai = PacketGetAdInfo(); ai; ai = ai->Next)
  {
    winpkt_trace_func = "PacketInitModule";
#if 0
    WINPKT_TRACE ("%s, `%s'\n", ai->Name, ai->Description);
#else
    WINPKT_TRACE ("%s\n", ai->Name);
#endif
  }
  WINPKT_TRACE ("rc %d\n", rc);

  /* Check if we loaded WanPacket.dll okay. We don't care if it fails (since
   * it's not critical for Watt-32/Win. These 'p_' func-ptr are et in win_dll.c
   */
  use_wanpacket = (p_WanPacketSetBpfFilter  && p_WanPacketOpenAdapter    &&
                   p_WanPacketCloseAdapter  && p_WanPacketSetBufferSize  &&
                   p_WanPacketReceivePacket && p_WanPacketSetMinToCopy   &&
                   p_WanPacketGetStats      && p_WanPacketSetReadTimeout &&
                   p_WanPacketSetMode       && p_WanPacketGetReadEvent   &&
                   p_WanPacketTestAdapter);
  return (rc);
}

/**
 * Sets the maximum possible lookahead buffer for the driver's
 * Packet_tap() function.
 *
 * \param AdapterObject Handle to the service control manager.
 * \return If the function succeeds, the return value is nonzero.
 *
 * The lookahead buffer is the portion of packet that Packet_tap() can
 * access from the NIC driver's memory without performing a copy.
 * This function tries to increase the size of that buffer.
 */
static BOOL PacketSetMaxLookaheadsize (const ADAPTER *AdapterObject)
{
  struct {
    PACKET_OID_DATA oidData;
    DWORD           filler;
  } oid;
  BOOL rc;

  winpkt_trace_func = "PacketSetMaxLookaheadsize";

  /* Set the size of the lookahead buffer to the maximum available
   * by the the NIC driver.
   */
  memset (&oid, 0, sizeof(oid));
  oid.oidData.Oid    = OID_GEN_MAXIMUM_LOOKAHEAD;
  oid.oidData.Length = sizeof(oid.filler);
  rc = PacketRequest (AdapterObject, FALSE, &oid.oidData);

  winpkt_trace_func = "PacketSetMaxLookaheadsize";
  WINPKT_TRACE ("lookahead %lu, rc %d; %s\n",
                *(u_long*)&oid.oidData, rc,
                !rc ? win_strerror(GetLastError()) : "okay");

  memset (&oid, 0, sizeof(oid));
  oid.oidData.Oid    = OID_GEN_CURRENT_LOOKAHEAD;
  oid.oidData.Length = sizeof(oid.filler);
  rc = PacketRequest (AdapterObject, TRUE, &oid.oidData);

  winpkt_trace_func = "PacketSetMaxLookaheadsize";
  WINPKT_TRACE ("rc %d; %s\n",
                rc, !rc ? win_strerror(GetLastError()) : "okay");
  return (rc);
}

/**
 * Retrieves the event associated in the driver with a capture instance
 * and stores it in an ADAPTER structure.
 *
 * \param AdapterObject Handle to the service control manager.
 * \return If the function succeeds, the return value is nonzero.
 *
 * This function is used by PacketOpenAdapter() to retrieve the read event
 * from the driver by means of an ioctl call and set it in the ADAPTER
 * structure pointed by AdapterObject.
 *
 * PacketSetReadEvt3xx() only used by version '3' NPF.SYS drivers.
 * PacketSetReadEvt4xx() used by newer versions.
 */
static BOOL PacketSetReadEvt3xx (ADAPTER *AdapterObject)
{
  DWORD BytesReturned;
  char  EventName[40];
  WCHAR EventNameU[20];

  winpkt_trace_func = "PacketSetReadEvt3xx";

  /* retrieve the Unicode name of the shared event from the driver
   */
  memset (&EventNameU, 0, sizeof(EventNameU));
  if (!DeviceIoControl(AdapterObject->hFile, pBIOCEVNAME, NULL, 0,
                       EventNameU, sizeof(EventNameU), &BytesReturned, NULL))
  {
    WINPKT_TRACE ("error retrieving the event-name from the kernel\n");
    return (FALSE);
  }

  /* this tells the terminal service to retrieve the event from the global
   * namespace
   */
  SNPRINTF (EventName, sizeof(EventName), "Global\\%.13S", EventNameU);
  WINPKT_TRACE ("  event-name %s\n", EventName);

  /* open the shared event
   */
  AdapterObject->ReadEvent = CreateEventA (NULL, TRUE, FALSE, EventName);

  /* On Win-NT4 "Global\" is not automatically ignored: try to use
   * simply the event name.
   */
  if (GetLastError() != ERROR_ALREADY_EXISTS)
  {
    if (AdapterObject->ReadEvent != INVALID_HANDLE_VALUE)
       CloseHandle (AdapterObject->ReadEvent);

    /* open the shared event */
    AdapterObject->ReadEvent = CreateEventA (NULL, TRUE, FALSE, EventName+7); /* skip "Global\" */
  }

  if (AdapterObject->ReadEvent == INVALID_HANDLE_VALUE ||
      GetLastError() != ERROR_ALREADY_EXISTS)
  {
    WINPKT_TRACE ("error retrieving the event from the kernel\n");
    return (FALSE);
  }

  AdapterObject->ReadTimeOut = 0;  /* block until something received */
  WINPKT_TRACE ("okay\n");
  return (TRUE);
}

static BOOL PacketSetReadEvt4xx (ADAPTER *AdapterObject)
{
  DWORD  BytesReturned;
  HANDLE hEvent;

  winpkt_trace_func = "PacketSetReadEvt4xx";

  if (AdapterObject->ReadEvent)
  {
    WINPKT_TRACE ("error\n");
    SetLastError (ERROR_INVALID_FUNCTION);
    return FALSE;
  }

  hEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
  if (!hEvent)
  {
    WINPKT_TRACE ("error creating event\n");
    return FALSE;
  }

  if (!DeviceIoControl(AdapterObject->hFile, pBIOCSETEVENTHANDLE, &hEvent,
                       sizeof(hEvent), NULL, 0, &BytesReturned, NULL))
  {
    WINPKT_TRACE ("error getting event-handle\n");
    return (FALSE);
  }

  WINPKT_TRACE ("event-handle %lu\n", (u_long)HandleToUlong(hEvent));

  AdapterObject->ReadEvent = hEvent;
  AdapterObject->ReadTimeOut = 0;
  WINPKT_TRACE ("okay\n");
  return (TRUE);
}

static BOOL PacketSetReadEvt (ADAPTER *AdapterObject)
{
  if (npf_drv_ver[0] == '3')
     return PacketSetReadEvt3xx (AdapterObject);
  return PacketSetReadEvt4xx (AdapterObject);
}

/**
 * Installs the NPF device driver.
 *
 * \param ascmHandle Handle to the service control manager.
 * \param srvHandle  A pointer to a handle that will receive the
 *        pointer to the driver's service.
 * \return If the function succeeds, the return value is nonzero.
 *
 * This function installs the driver's service in the system using the
 * CreateService function.
 */
static BOOL PacketInstallDriver (SC_HANDLE ascmHandle, SC_HANDLE *srvHandle)
{
  BOOL  result = FALSE;
  DWORD error  = 0UL;

  winpkt_trace_func = "PacketInstallDriver";
  WINPKT_TRACE ("\n");

  *srvHandle = CreateServiceA (ascmHandle,
                               service_name,
                               service_descr,
                               SERVICE_ALL_ACCESS,
                               SERVICE_KERNEL_DRIVER,
                               SERVICE_DEMAND_START,
                               SERVICE_ERROR_NORMAL,
                               driver_path, NULL, NULL,
                               NULL, NULL, NULL);
  if (!*srvHandle)
  {
    if (GetLastError() == ERROR_SERVICE_EXISTS)
    {
      /* npf.sys already exists */
      result = TRUE;
    }
  }
  else
  {
    /* Created service for npf.sys */
    result = TRUE;
  }

  if (result && *srvHandle)
     CloseServiceHandle (*srvHandle);

  if (!result)
  {
    error = GetLastError();
    if (error != ERROR_FILE_NOT_FOUND)
       WINPKT_TRACE ("failed; %s\n", win_strerror(error));
  }
  else
    WINPKT_TRACE ("okay\n");

  SetLastError (error);
  return (result);
}

/**
 * Opens an adapter using the NPF or Win10Pcap controlling service.
 *
 * \param AdapterName A string containing the name of the device to open.
 * \return If the function succeeds, the return value is the pointer
 *         to a properly initialized ADAPTER object, otherwise the return
 *         value is NULL.
 *
 * \note internal function used by PacketOpenAdapter() and AddAdapter().
 *
 * \todo: We can probably simplify this and assume 'registry_location'
 *        is always present and the NPF service is running. Thus removing
 *        lots of dead code.
 */
static ADAPTER *PacketOpenAdapterNPF (const char *AdapterName)
{
  ADAPTER       *lpAdapter;
  BOOL           QuerySStat, Result;
  DWORD          error, KeyRes;
  SC_HANDLE      svcHandle = NULL;
  HKEY           PathKey;
  char           SymbolicLink[128];
  const char    *sym_link;
  SERVICE_STATUS SStat;
  SC_HANDLE      scmHandle, srvHandle;
  size_t         prefix_len;

  winpkt_trace_func = "PacketOpenAdapterNPF";
  WINPKT_TRACE ("\n");

  set_char_pointers (AdapterName);

  scmHandle = OpenSCManager (NULL, NULL, GENERIC_READ);
  if (!scmHandle)
  {
    error = GetLastError();
    WINPKT_TRACE ("OpenSCManager failed; %s\n", win_strerror(error));
  }
  else
  {
    /* Check if the NPF registry key is already present.
     * This means that the driver is already installed and that
     * we don't need to call PacketInstallDriver
     */
    KeyRes = RegOpenKeyExA (HKEY_LOCAL_MACHINE, registry_location,
                            0, KEY_READ, &PathKey);
    if (KeyRes != ERROR_SUCCESS)
    {
      Result = PacketInstallDriver (scmHandle, &svcHandle);
    }
    else
    {
      Result = TRUE;
      RegCloseKey (PathKey);
    }

    winpkt_trace_func = "PacketOpenAdapterNPF";
    WINPKT_TRACE ("Service %s: \n", service_name);

    if (Result)
    {
      srvHandle = OpenServiceA (scmHandle, service_name,
                                SERVICE_START | SERVICE_QUERY_STATUS);
      if (srvHandle)
      {
        QuerySStat = QueryServiceStatus (srvHandle, &SStat);

        WINPKT_TRACE ("state %s\n", list_lookup(SStat.dwCurrentState, serv_stat, DIM(serv_stat)));

        if (!QuerySStat || SStat.dwCurrentState != SERVICE_RUNNING)
        {
          WINPKT_TRACE ("Calling startservice\n");
          if (!StartService(srvHandle, 0, NULL))
          {
            error = GetLastError();
            if (error != ERROR_SERVICE_ALREADY_RUNNING &&
                error != ERROR_ALREADY_EXISTS)
            {
              error = GetLastError();
              CloseServiceHandle (scmHandle);
              WINPKT_TRACE ("StartService failed; %s\n",
                            win_strerror(error));
              SetLastError (error);
              return (NULL);
            }
          }
        }
        CloseServiceHandle (srvHandle);
        srvHandle = NULL;
      }
      else
      {
        error = GetLastError();
        WINPKT_TRACE ("OpenService failed; %s\n", win_strerror(error));
        SetLastError (error);
      }
    }
    else  /* Registry key not found or PacketInstallDriver() failed */
    {
      if (KeyRes != ERROR_SUCCESS)
           Result = PacketInstallDriver (scmHandle, &svcHandle);
      else Result = TRUE;

      winpkt_trace_func = "PacketOpenAdapterNPF";

      if (Result)
      {
        srvHandle = OpenServiceA (scmHandle, service_name, SERVICE_START);
        if (srvHandle)
        {
          QuerySStat = QueryServiceStatus (srvHandle, &SStat);

          WINPKT_TRACE ("state: %s\n", list_lookup(SStat.dwCurrentState, serv_stat, DIM(serv_stat)));

          if (!QuerySStat || SStat.dwCurrentState != SERVICE_RUNNING)
          {
            WINPKT_TRACE ("Calling startservice\n");

            if (StartService (srvHandle, 0, NULL) == 0)
            {
              error = GetLastError();
              if (error != ERROR_SERVICE_ALREADY_RUNNING &&
                  error != ERROR_ALREADY_EXISTS)
              {
                if (scmHandle)
                   CloseServiceHandle (scmHandle);
                WINPKT_TRACE ("StartService failed; %s\n",
                              win_strerror(error));
                SetLastError (error);
                return (NULL);
              }
            }
          }
          CloseServiceHandle (srvHandle);
        }
        else
        {
          error = GetLastError();
          WINPKT_TRACE ("OpenService failed; %s", win_strerror(error));
          SetLastError (error);
        }
      }
    }
  }   /* OpenSCManager() */

  if (scmHandle)
     CloseServiceHandle (scmHandle);

  /* skip "\Device" to create a symlink name
   */
  prefix_len = strlen (DEVICE_PREFIX);
  if (strnicmp(AdapterName,driver_prefix,prefix_len))
  {
    WINPKT_TRACE ("Unexpected prefix for adapter \"%s\". Expected prefix: \"%s\"\n",
                  AdapterName, driver_prefix);
    return (NULL);
  }

  lpAdapter = GlobalAllocPtr (GMEM_MOVEABLE | GMEM_ZEROINIT,
                              sizeof(*lpAdapter));
  if (!lpAdapter)
  {
    WINPKT_TRACE ("Failed to allocate the adapter structure\n");
    return (NULL);
  }

  sym_link = AdapterName + prefix_len;
  SNPRINTF (SymbolicLink, sizeof(SymbolicLink), "\\\\.\\%s", sym_link);

  WINPKT_TRACE ("Creating SymbolicLink: \"%s\".\n", SymbolicLink);

  /* try if it is possible to open the adapter immediately
   */
  lpAdapter->hFile = CreateFileA (SymbolicLink, GENERIC_WRITE | GENERIC_READ,
                                  0, NULL, OPEN_EXISTING, 0, 0);

  if (lpAdapter->hFile != INVALID_HANDLE_VALUE)
  {
    if (!PacketSetReadEvt(lpAdapter))
    {
      error = GetLastError();
      (void) GlobalFreePtr (lpAdapter);
      WINPKT_TRACE ("failed; %s\n", win_strerror(error));
      SetLastError (error);
      return (NULL);
    }

    /* Success!
     */
    PacketSetMaxLookaheadsize (lpAdapter);
    winpkt_trace_func = "PacketOpenAdapterNPF";
    return (lpAdapter);
  }

  error = GetLastError();
  (void) GlobalFreePtr (lpAdapter);
  WINPKT_TRACE ("CreateFileA (%s) failed; \n%-49s%s\n",
                SymbolicLink, "", win_strerror(error));
  SetLastError (error);
  return (NULL);
}

/**
 * Return a string with the version of the NPF.sys device driver.
 */
const char *PacketGetDriverVersion (void)
{
  return (npf_drv_ver);
}

#ifdef NOT_NEEDED
/**
 * Stops and unloads the WinPcap device driver.
 *
 * \return If the function succeeds, the return value is nonzero,
 *         otherwise it is zero.
 *
 * This function can be used to unload the driver from memory when the
 * application no more needs it. Note that the driver is physically stopped
 * and unloaded only when all the files on its devices are closed, i.e. when
 * all the applications that use WinPcap close all their adapters.
 */
BOOL PacketStopDriver (void)
{
  SC_HANDLE scmHandle, schService;
  BOOL      rc = FALSE;

  winpkt_trace_func = "PacketStopDriver";

  scmHandle = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (scmHandle)
  {
    schService = OpenService (scmHandle, service_name, SERVICE_ALL_ACCESS);
    if (schService)
    {
      SERVICE_STATUS status;

      rc = ControlService (schService, SERVICE_CONTROL_STOP, &status);
      CloseServiceHandle (schService);
      CloseServiceHandle (scmHandle);
    }
  }

  WINPKT_TRACE ("rc %d\n", rc);
  return (rc);
}
#endif  /* NOT_NEEDED */

/**
 * Opens an adapter.
 *
 * \param AdapterName A string containing the name of the device to open.
 *
 * \return If the function succeeds, the return value is the pointer to a
 * properly initialized ADAPTER object, otherwise the return value is NULL.
 */
const ADAPTER *PacketOpenAdapter (const char *AdapterName)
{
  ADAPTER *ad;

  set_char_pointers (AdapterName);

  winpkt_trace_func = "PacketOpenAdapter";
  WINPKT_TRACE ("trying to open the adapter %s using service %s.\n", AdapterName, service_name);

  ad = PacketOpenAdapterNPF (AdapterName);
  if (ad)
     ad->flags = INFO_FLAG_NDIS_ADAPTER;

  return (ad);
}

/**
 * Closes an adapter.
 *
 * Closes the given adapter and frees the associated
 * ADAPTER structure.
 *
 * Since we only use one adapter, free the 'adapters_list'
 * and the 'adapters_mutex' too.
 */
BOOL PacketCloseAdapter (ADAPTER *adapter)
{
  BOOL rc = FALSE;
  int  err = 0;

  winpkt_trace_func = "PacketCloseAdapter";

  if (!adapter || adapter->hFile == INVALID_HANDLE_VALUE)
  {
    WINPKT_TRACE ("adapter already closed\n");
    return (rc);
  }

  WINPKT_TRACE ("adapter file 0x%" ADDR_FMT "\n", ADDR_CAST(adapter->hFile));

  rc = CloseHandle (adapter->hFile);
  adapter->hFile = INVALID_HANDLE_VALUE;
  if (!rc)
     err = GetLastError();

  SetEvent (adapter->ReadEvent);   /* might already be set */
  CloseHandle (adapter->ReadEvent);
  (void) GlobalFreePtr (adapter);

  WINPKT_TRACE ("CloseHandle() rc %d: %s\n",
                rc, !rc ? win_strerror(err) : "okay");
  return (rc);

}

/**
 * Called from pkt_release()
 */
BOOL PacketExitModule (void)
{
  BOOL rc;

  winpkt_trace_func = "PacketExitModule";
  WINPKT_TRACE ("\n");

  FreeAdaptersInfoList();
  ReleaseMutex (adapters_mutex);
  rc = CloseHandle (adapters_mutex);
  adapters_mutex = INVALID_HANDLE_VALUE;
  return (rc);
}

/**
 * Read data packets from the NPF driver.
 *
 * The data received with this function can be a group of packets.
 * The number of packets received with this function is variable. It
 * depends on the number of packets currently stored in the driver's
 * buffer, on the size of these packets and on 'buf_len' paramter.
 *
 * Each packet (starting at 'buf') has a header consisting in a 'bpf_hdr'
 * structure that defines its length and timestamp. A padding field
 * is used to word-align the data in the buffer (to speed up the access
 * to the packets). The 'bh_datalen' and 'bh_hdrlen' fields of the
 * 'bpf_hdr' structures should be used to extract the packets from the
 * parameter 'buf'.
 *
 */
UINT MS_CDECL PacketReceivePacket (const ADAPTER *adapter, void *buf, UINT buf_len)
{
  HANDLE handle;
  UINT   rc = 0;
  DWORD  recv = 0;

  if ((int)adapter->ReadTimeOut != -1)
     WaitForSingleObject (adapter->ReadEvent,
                          adapter->ReadTimeOut == 0 ?
                          INFINITE : adapter->ReadTimeOut);

  handle = adapter->hFile;
  if (handle == INVALID_HANDLE_VALUE)
       SetLastError (ERROR_INVALID_HANDLE);
  else rc = ReadFile (handle, buf, buf_len, &recv, NULL);

  winpkt_trace_func = "PacketReceivePacket";
  WINPKT_TRACE ("recv %lu; rc: %d, %s\n",
                (u_long)recv, rc, rc == 0 ? win_strerror(GetLastError()) : "okay");
  return (UINT) recv;
}

/**
 * Sends one packet to the network.
 */
UINT PacketSendPacket (const ADAPTER *adapter, const void *buf, UINT len)
{
  DWORD sent = 0UL;
  int   rc;

  TCP_CONSOLE_MSG (4, ("%s(%u): Calling WriteFile()\n", __FILE__, __LINE__));

  rc = WriteFile (adapter->hFile, buf, len, &sent, NULL);

  winpkt_trace_func = "PacketSendPacket";
  WINPKT_TRACE ("rc %d; %s\n",
                rc, rc == 0 ? win_strerror(GetLastError()) : "okay");
  return (sent);
}

BOOL PacketGetMacAddress (const ADAPTER *adapter, void *mac)
{
  struct {
    PACKET_OID_DATA oidData;
    mac_address     mac;
  } oid;

  memset (&oid, 0, sizeof(oid));

  switch (_pktdevclass)
  {
    case PDCLASS_ETHER:
         oid.oidData.Oid = OID_802_3_PERMANENT_ADDRESS;
         break;
    case PDCLASS_TOKEN:
         oid.oidData.Oid = OID_802_5_PERMANENT_ADDRESS;
         break;
    case PDCLASS_FDDI:
         oid.oidData.Oid = OID_FDDI_LONG_PERMANENT_ADDR;
         break;
    case PDCLASS_ARCNET:
         oid.oidData.Oid = OID_ARCNET_PERMANENT_ADDRESS;
         break;
#if 0
    case NdisMediumWan:
         oid.oidData.Oid = OID_WAN_PERMANENT_ADDRESS;
         break;
    case NdisMediumWirelessWan:
         oid.oidData.Oid = OID_WW_GEN_PERMANENT_ADDRESS;
         break;
#endif
    default:
         return (FALSE);
  }
  oid.oidData.Length = sizeof(oid.mac);

  if (!PacketRequest(adapter, FALSE, &oid.oidData))
     return (FALSE);
  memcpy (mac, &oid.oidData.Data, sizeof(mac_address));
  return (TRUE);
}

/**
 * Defines the minimum amount of data that will be received in a read.
 *
 * \param AdapterObject Pointer to an ADAPTER structure
 * \param nbytes the minimum amount of data in the kernel buffer that will
 *        cause the driver to release a read on this adapter.
 * \return If the function succeeds, the return value is nonzero.
 *
 * In presence of a large value for 'nbytes', the kernel waits for the arrival
 * of several packets before copying the data to the user. This guarantees a
 * low number of system calls, i.e. lower processor usage, i.e. better
 * performance, which is a good setting for applications like sniffers. Vice
 * versa, a small value means that the kernel will copy the packets as soon
 * as the application is ready to receive them. This is suggested for real
 * time applications (like, for example, a bridge) that need the better
 * responsiveness from the kernel.
 *
 * \note: this function has effect only in Windows NTx. The driver for
 *   Windows 9x doesn't offer this possibility, therefore PacketSetMinToCopy
 *   is implemented under these systems only for compatibility.
 */
BOOL PacketSetMinToCopy (const ADAPTER *AdapterObject, int nbytes)
{
  DWORD BytesReturned;
  BOOL  rc;

  rc = DeviceIoControl (AdapterObject->hFile, pBIOCSMINTOCOPY, &nbytes,
                        4, NULL, 0, &BytesReturned, NULL);

  winpkt_trace_func = "PacketSetMinToCopy";
  WINPKT_TRACE ("nbytes %d, rc %d; %s\n",
                nbytes, rc, !rc ? win_strerror(GetLastError()) :
                "okay");
  return (rc);
}

/**
 * Sets the working mode of an adapter.
 *
 * \param AdapterObject Pointer to an ADAPTER structure.
 * \param mode The new working mode of the adapter.
 * \return If the function succeeds, the return value is nonzero.
 *
 * The device driver of WinPcap has 4 working modes:
 * - Capture mode (mode = PACKET_MODE_CAPT): normal capture mode. The
 *   packets transiting on the wire are copied to the application when
 *   PacketReceivePacket() is called. This is the default working mode
 *   of an adapter.
 *
 * - Statistical mode (mode = PACKET_MODE_STAT): programmable statistical
 *   mode. PacketReceivePacket() returns, at precise intervals, statics
 *   values on the network traffic. The interval between the statistic
 *   samples is by default 1 second but it can be set to any other value
 *   (with a 1 ms precision) with the PacketSetReadTimeout() function.
 *   Two 64-bit counters are provided: the number of packets and the amount
 *   of bytes that satisfy a filter previously set with PacketSetBPF(). If
 *   no filter has been set, all the packets are counted. The counters are
 *   encapsulated in a bpf_hdr structure, so that they will be parsed
 *   correctly by wpcap. Statistical mode has a very low impact on system
 *   performance compared to capture mode.
 *
 * - Dump mode (mode = PACKET_MODE_DUMP): the packets are dumped to disk by
 *   the driver, in libpcap format. This method is much faster than saving
 *   the packets after having captured them. No data is returned by
 *   PacketReceivePacket(). If the application sets a filter with
 *   PacketSetBPF(), only the packets that satisfy this filter are dumped
 *   to disk.
 *
 * - Statistical Dump mode (mode = PACKET_MODE_STAT_DUMP): the packets are
 *   dumped to disk by the driver, in libpcap format, like in dump mode.
 *   PacketReceivePacket() returns, at precise intervals, statics values on
 *   the network traffic and on the amount of data saved to file, in a way
 *   similar to statistical mode.
 *   Three 64-bit counters are provided: the number of packets accepted, the
 *   amount of bytes accepted and the effective amount of data (including
 *   headers) dumped to file. If no filter has been set, all the packets are
 *   dumped to disk. The counters are encapsulated in a bpf_hdr structure,
 *   so that they will be parsed correctly by wpcap. Look at the NetMeter
 *   example in the WinPcap developer's pack to see how to use statistics
 *   mode.
 */
BOOL PacketSetMode (const ADAPTER *AdapterObject, DWORD mode)
{
  DWORD BytesReturned;
  BOOL  rc;

  rc = DeviceIoControl (AdapterObject->hFile, pBIOCSMODE, &mode,
                        sizeof(mode), NULL, 0, &BytesReturned, NULL);

  winpkt_trace_func = "PacketSetMode";
  WINPKT_TRACE ("mode %lu, rc %d; %s\n",
                (u_long)mode, rc, !rc ? win_strerror(GetLastError()) : "okay");
  return (rc);
}

/**
 * Sets the behavior of the NPF driver with packets sent by itself:
 * capture or drop. 'behavior' Can be one of the following:
 *  - NPF_ENABLE_LOOPBACK
 *  - NPF_DISABLE_LOOPBACK
 *
 * note: when opened, adapters have loopback capture enabled.
 * note: Effective on WinPcap 4.x+ only.
 */
BOOL PacketSetLoopbackBehavior (const ADAPTER *AdapterObject, UINT behavior)
{
  DWORD BytesReturned;
  BOOL  rc;

  rc = DeviceIoControl (AdapterObject->hFile, pBIOCISETLOBBEH,
                        &behavior, sizeof(UINT), NULL,
                        0, &BytesReturned, NULL);

  winpkt_trace_func = "PacketSetLoopbackBehavior";
  WINPKT_TRACE ("behaviour %u, rc %d; %s\n",
                behavior, rc, !rc ? win_strerror(GetLastError()) : "okay");
  return (rc);
}

/**
 * Returns the notification event associated with the read calls on an
 * adapter.
 *
 * \param AdapterObject Pointer to an ADAPTER structure.
 * \return The handle of the event that the driver signals when some data
 *         is available in the kernel buffer.
 *
 * The event returned by this function is signaled by the driver if:
 * - The adapter pointed by AdapterObject is in capture mode and an amount
 *   of data greater or equal than the one set with the PacketSetMinToCopy()
 *   function is received from the network.
 *
 * - The adapter pointed by AdapterObject is in capture mode, no data has
 *   been received from the network but the the timeout set with the
 *   PacketSetReadTimeout() function has elapsed.
 *
 * - The adapter pointed by AdapterObject is in statics mode and the the
 *   timeout set with the PacketSetReadTimeout() function has elapsed.
 *   This means that a new statistic sample is available.
 *
 * In every case, a call to PacketReceivePacket() will return immediately.
 * The event can be passed to standard Win32 functions (like
 * WaitForSingleObject or WaitForMultipleObjects) to wait until the
 * driver's buffer contains some data. It is particularly useful in GUI
 * applications that need to wait concurrently on several events.
 */
HANDLE PacketGetReadEvent (const ADAPTER *AdapterObject)
{
  return (AdapterObject->ReadEvent);
}

/**
 * Sets the timeout after which a read on an adapter returns.
 *
 * \param AdapterObject Pointer to an ADAPTER structure.
 * \param timeout indicates the timeout, in milliseconds, after which a call
 *        to PacketReceivePacket() on the adapter pointed by AdapterObject
 *        will be released, also if no packets have been captured by the
 *        driver. Setting timeout to 0 means no timeout, i.e.
 *        PacketReceivePacket() never returns if no packet arrives.
 *        A timeout of -1 causes PacketReceivePacket() to always return
 *        immediately.
 * \return If the function succeeds, the return value is nonzero.
 *
 * \note This function works also if the adapter is working in statistics
 *       mode, and can be used to set the time interval between two
 *       statistic reports.
 */
BOOL PacketSetReadTimeout (ADAPTER *AdapterObject, int timeout)
{
  DWORD BytesReturned;
  int   driverTimeout = -1;  /* NPF return immediately if a pkt is ready */
  BOOL  rc;

  AdapterObject->ReadTimeOut = timeout;

  rc = DeviceIoControl (AdapterObject->hFile, pBIOCSRTIMEOUT, &driverTimeout,
                        sizeof(driverTimeout), NULL, 0, &BytesReturned, NULL);

  winpkt_trace_func = "PacketSetReadTimeout";
  WINPKT_TRACE ("timeout %d, rc %d; %s\n",
                timeout, rc, !rc ? win_strerror(GetLastError()) : "okay");
  return (rc);
}

/**
 * Sets the size of the kernel-level buffer associated with a capture.
 *
 * \param AdapterObject Pointer to an ADAPTER structure.
 * \param dim New size of the buffer, in \b kilobytes.
 * \return The function returns TRUE if successfully completed, FALSE if
 *         there is not enough memory to allocate the new buffer.
 *
 * When a new dimension is set, the data in the old buffer is discarded and
 * the packets stored in it are lost.
 *
 * Note: the dimension of the kernel buffer affects heavily the performances
 * of the capture process. An adequate buffer in the driver is able to keep
 * the packets while the application is busy, compensating the delays of the
 * application and avoiding the loss of packets during bursts or high network
 * activity. The buffer size is set to 0 when an instance of the driver is
 * opened: the programmer should remember to set it to a proper value. As an
 * example, wpcap sets the buffer size to 1MB at the beginning of a capture.
 */
BOOL PacketSetBuff (const ADAPTER *AdapterObject, DWORD dim)
{
  DWORD BytesReturned;
  BOOL  rc;

  rc = DeviceIoControl (AdapterObject->hFile, pBIOCSETBUFFERSIZE,
                        &dim, sizeof(dim), NULL, 0, &BytesReturned, NULL);

  winpkt_trace_func = "PacketSetBuff";
  WINPKT_TRACE ("size %lu, rc %d; %s\n",
                (u_long)dim, rc, !rc ? win_strerror(GetLastError()) : "okay");
  return (rc);
}

/**
 * Returns statistic values about the current capture session.
 *
 * \param AdapterObject Pointer to an ADAPTER structure.
 * \param s Pointer to a user provided bpf_stat structure that will be
 *        filled by the function.
 * \return If the function succeeds, the return value is nonzero.
 */
BOOL PacketGetStatsEx (const ADAPTER *AdapterObject, struct bpf_stat *stat)
{
  struct bpf_stat tmp_stat;
  DWORD  read;
  BOOL   rc;

  memset (&tmp_stat, 0, sizeof(tmp_stat));
  rc = DeviceIoControl (AdapterObject->hFile,
                        pBIOCGSTATS, NULL, 0, &tmp_stat, sizeof(tmp_stat),
                        &read, NULL);

  winpkt_trace_func = "PacketGetStatsEx";
  WINPKT_TRACE ("read: %lu, rc %d; %s\n",
                (u_long)read, rc, !rc ? win_strerror(GetLastError()) : "okay");

  stat->bs_recv   = tmp_stat.bs_recv;
  stat->bs_drop   = tmp_stat.bs_drop;
  stat->ps_ifdrop = tmp_stat.ps_ifdrop;
  stat->bs_capt   = tmp_stat.bs_capt;
  return (rc);
}

#if defined(USE_DEBUG)
static const struct search_list oid_list[] = {
   ADD_VALUE (OID_802_11_ADD_KEY),
   ADD_VALUE (OID_802_11_ADD_WEP),
   ADD_VALUE (OID_802_11_ASSOCIATION_INFORMATION),
   ADD_VALUE (OID_802_11_AUTHENTICATION_MODE),
   ADD_VALUE (OID_802_11_BSSID),
   ADD_VALUE (OID_802_11_BSSID_LIST),
   ADD_VALUE (OID_802_11_BSSID_LIST_SCAN),
   ADD_VALUE (OID_802_11_CAPABILITY),
   ADD_VALUE (OID_802_11_CONFIGURATION),
   ADD_VALUE (OID_802_11_DESIRED_RATES),
   ADD_VALUE (OID_802_11_DISASSOCIATE),
   ADD_VALUE (OID_802_11_ENCRYPTION_STATUS),
   ADD_VALUE (OID_802_11_FRAGMENTATION_THRESHOLD),
   ADD_VALUE (OID_802_11_INFRASTRUCTURE_MODE),
   ADD_VALUE (OID_802_11_NETWORK_TYPES_SUPPORTED),
   ADD_VALUE (OID_802_11_NETWORK_TYPE_IN_USE),
   ADD_VALUE (OID_802_11_NUMBER_OF_ANTENNAS),
   ADD_VALUE (OID_802_11_PMKID),
   ADD_VALUE (OID_802_11_POWER_MODE),
   ADD_VALUE (OID_802_11_PRIVACY_FILTER),
   ADD_VALUE (OID_802_11_RELOAD_DEFAULTS),
   ADD_VALUE (OID_802_11_REMOVE_KEY),
   ADD_VALUE (OID_802_11_REMOVE_WEP),
   ADD_VALUE (OID_802_11_RSSI),
   ADD_VALUE (OID_802_11_RSSI_TRIGGER),
   ADD_VALUE (OID_802_11_RTS_THRESHOLD),
   ADD_VALUE (OID_802_11_RX_ANTENNA_SELECTED),
   ADD_VALUE (OID_802_11_SSID),
   ADD_VALUE (OID_802_11_STATISTICS),
   ADD_VALUE (OID_802_11_SUPPORTED_RATES),
   ADD_VALUE (OID_802_11_TEST),
   ADD_VALUE (OID_802_11_TX_ANTENNA_SELECTED),
   ADD_VALUE (OID_802_11_TX_POWER_LEVEL),
   ADD_VALUE (OID_802_11_WEP_STATUS),
   ADD_VALUE (OID_802_3_CURRENT_ADDRESS),
   ADD_VALUE (OID_802_3_MAC_OPTIONS),
   ADD_VALUE (OID_802_3_MAXIMUM_LIST_SIZE),
   ADD_VALUE (OID_802_3_MULTICAST_LIST),
   ADD_VALUE (OID_802_3_PERMANENT_ADDRESS),
   ADD_VALUE (OID_802_3_RCV_ERROR_ALIGNMENT),
   ADD_VALUE (OID_802_3_RCV_OVERRUN),
   ADD_VALUE (OID_802_3_XMIT_DEFERRED),
   ADD_VALUE (OID_802_3_XMIT_HEARTBEAT_FAILURE),
   ADD_VALUE (OID_802_3_XMIT_LATE_COLLISIONS),
   ADD_VALUE (OID_802_3_XMIT_MAX_COLLISIONS),
   ADD_VALUE (OID_802_3_XMIT_MORE_COLLISIONS),
   ADD_VALUE (OID_802_3_XMIT_ONE_COLLISION),
   ADD_VALUE (OID_802_3_XMIT_TIMES_CRS_LOST),
   ADD_VALUE (OID_802_3_XMIT_UNDERRUN),
   ADD_VALUE (OID_802_5_ABORT_DELIMETERS),
   ADD_VALUE (OID_802_5_AC_ERRORS),
   ADD_VALUE (OID_802_5_BURST_ERRORS),
   ADD_VALUE (OID_802_5_CURRENT_ADDRESS),
   ADD_VALUE (OID_802_5_CURRENT_FUNCTIONAL),
   ADD_VALUE (OID_802_5_CURRENT_GROUP),
   ADD_VALUE (OID_802_5_CURRENT_RING_STATE),
   ADD_VALUE (OID_802_5_CURRENT_RING_STATUS),
   ADD_VALUE (OID_802_5_FRAME_COPIED_ERRORS),
   ADD_VALUE (OID_802_5_FREQUENCY_ERRORS),
   ADD_VALUE (OID_802_5_INTERNAL_ERRORS),
   ADD_VALUE (OID_802_5_LAST_OPEN_STATUS),
   ADD_VALUE (OID_802_5_LINE_ERRORS),
   ADD_VALUE (OID_802_5_LOST_FRAMES),
   ADD_VALUE (OID_802_5_PERMANENT_ADDRESS),
   ADD_VALUE (OID_802_5_TOKEN_ERRORS),
   ADD_VALUE (OID_ARCNET_CURRENT_ADDRESS),
   ADD_VALUE (OID_ARCNET_PERMANENT_ADDRESS),
   ADD_VALUE (OID_ARCNET_RECONFIGURATIONS),
   ADD_VALUE (OID_ATM_ACQUIRE_ACCESS_NET_RESOURCES),
   ADD_VALUE (OID_ATM_ALIGNMENT_REQUIRED),
   ADD_VALUE (OID_ATM_ASSIGNED_VPI),
   ADD_VALUE (OID_ATM_CELLS_HEC_ERROR),
   ADD_VALUE (OID_ATM_DIGITAL_BROADCAST_VPIVCI),
   ADD_VALUE (OID_ATM_GET_NEAREST_FLOW),
   ADD_VALUE (OID_ATM_HW_CURRENT_ADDRESS),
   ADD_VALUE (OID_ATM_ILMI_VPIVCI),
   ADD_VALUE (OID_ATM_MAX_AAL0_PACKET_SIZE),
   ADD_VALUE (OID_ATM_MAX_AAL1_PACKET_SIZE),
   ADD_VALUE (OID_ATM_MAX_AAL34_PACKET_SIZE),
   ADD_VALUE (OID_ATM_MAX_AAL5_PACKET_SIZE),
   ADD_VALUE (OID_ATM_MAX_ACTIVE_VCI_BITS),
   ADD_VALUE (OID_ATM_MAX_ACTIVE_VCS),
   ADD_VALUE (OID_ATM_MAX_ACTIVE_VPI_BITS),
   ADD_VALUE (OID_ATM_RCV_CELLS_DROPPED),
   ADD_VALUE (OID_ATM_RCV_CELLS_OK),
   ADD_VALUE (OID_ATM_RCV_INVALID_VPI_VCI),
   ADD_VALUE (OID_ATM_RCV_REASSEMBLY_ERROR),
   ADD_VALUE (OID_ATM_RELEASE_ACCESS_NET_RESOURCES),
   ADD_VALUE (OID_ATM_SIGNALING_VPIVCI),
   ADD_VALUE (OID_ATM_SUPPORTED_AAL_TYPES),
   ADD_VALUE (OID_ATM_SUPPORTED_SERVICE_CATEGORY),
   ADD_VALUE (OID_ATM_SUPPORTED_VC_RATES),
   ADD_VALUE (OID_ATM_XMIT_CELLS_OK),
   ADD_VALUE (OID_CO_ADDRESS_CHANGE),
   ADD_VALUE (OID_CO_ADD_ADDRESS),
   ADD_VALUE (OID_CO_ADD_PVC),
   ADD_VALUE (OID_CO_DELETE_ADDRESS),
   ADD_VALUE (OID_CO_DELETE_PVC),
   ADD_VALUE (OID_CO_GET_ADDRESSES),
   ADD_VALUE (OID_CO_GET_CALL_INFORMATION),
   ADD_VALUE (OID_CO_SIGNALING_DISABLED),
   ADD_VALUE (OID_CO_SIGNALING_ENABLED),
   ADD_VALUE (OID_FDDI_ATTACHMENT_TYPE),
   ADD_VALUE (OID_FDDI_DOWNSTREAM_NODE_LONG),
   ADD_VALUE (OID_FDDI_FRAMES_LOST),
   ADD_VALUE (OID_FDDI_FRAME_ERRORS),
   ADD_VALUE (OID_FDDI_IF_ADMIN_STATUS),
   ADD_VALUE (OID_FDDI_IF_DESCR),
   ADD_VALUE (OID_FDDI_IF_IN_DISCARDS),
   ADD_VALUE (OID_FDDI_IF_IN_ERRORS),
   ADD_VALUE (OID_FDDI_IF_IN_NUCAST_PKTS),
   ADD_VALUE (OID_FDDI_IF_IN_OCTETS),
   ADD_VALUE (OID_FDDI_IF_IN_UCAST_PKTS),
   ADD_VALUE (OID_FDDI_IF_IN_UNKNOWN_PROTOS),
   ADD_VALUE (OID_FDDI_IF_LAST_CHANGE),
   ADD_VALUE (OID_FDDI_IF_MTU),
   ADD_VALUE (OID_FDDI_IF_OPER_STATUS),
   ADD_VALUE (OID_FDDI_IF_OUT_DISCARDS),
   ADD_VALUE (OID_FDDI_IF_OUT_ERRORS),
   ADD_VALUE (OID_FDDI_IF_OUT_NUCAST_PKTS),
   ADD_VALUE (OID_FDDI_IF_OUT_OCTETS),
   ADD_VALUE (OID_FDDI_IF_OUT_QLEN),
   ADD_VALUE (OID_FDDI_IF_OUT_UCAST_PKTS),
   ADD_VALUE (OID_FDDI_IF_PHYS_ADDRESS),
   ADD_VALUE (OID_FDDI_IF_SPECIFIC),
   ADD_VALUE (OID_FDDI_IF_SPEED),
   ADD_VALUE (OID_FDDI_IF_TYPE),
   ADD_VALUE (OID_FDDI_LCONNECTION_STATE),
   ADD_VALUE (OID_FDDI_LCT_FAILURES),
   ADD_VALUE (OID_FDDI_LEM_REJECTS),
   ADD_VALUE (OID_FDDI_LONG_CURRENT_ADDR),
   ADD_VALUE (OID_FDDI_LONG_MAX_LIST_SIZE),
   ADD_VALUE (OID_FDDI_LONG_MULTICAST_LIST),
   ADD_VALUE (OID_FDDI_LONG_PERMANENT_ADDR),
   ADD_VALUE (OID_FDDI_MAC_AVAILABLE_PATHS),
   ADD_VALUE (OID_FDDI_MAC_BRIDGE_FUNCTIONS),
   ADD_VALUE (OID_FDDI_MAC_COPIED_CT),
   ADD_VALUE (OID_FDDI_MAC_CURRENT_PATH),
   ADD_VALUE (OID_FDDI_MAC_DA_FLAG),
   ADD_VALUE (OID_FDDI_MAC_DOWNSTREAM_NBR),
   ADD_VALUE (OID_FDDI_MAC_DOWNSTREAM_PORT_TYPE),
   ADD_VALUE (OID_FDDI_MAC_DUP_ADDRESS_TEST),
   ADD_VALUE (OID_FDDI_MAC_ERROR_CT),
   ADD_VALUE (OID_FDDI_MAC_FRAME_CT),
   ADD_VALUE (OID_FDDI_MAC_FRAME_ERROR_FLAG),
   ADD_VALUE (OID_FDDI_MAC_FRAME_ERROR_RATIO),
   ADD_VALUE (OID_FDDI_MAC_FRAME_ERROR_THRESHOLD),
   ADD_VALUE (OID_FDDI_MAC_FRAME_STATUS_FUNCTIONS),
   ADD_VALUE (OID_FDDI_MAC_HARDWARE_PRESENT),
   ADD_VALUE (OID_FDDI_MAC_INDEX),
   ADD_VALUE (OID_FDDI_MAC_LATE_CT),
   ADD_VALUE (OID_FDDI_MAC_LONG_GRP_ADDRESS),
   ADD_VALUE (OID_FDDI_MAC_LOST_CT),
   ADD_VALUE (OID_FDDI_MAC_MA_UNITDATA_AVAILABLE),
   ADD_VALUE (OID_FDDI_MAC_MA_UNITDATA_ENABLE),
   ADD_VALUE (OID_FDDI_MAC_NOT_COPIED_CT),
   ADD_VALUE (OID_FDDI_MAC_NOT_COPIED_FLAG),
   ADD_VALUE (OID_FDDI_MAC_NOT_COPIED_RATIO),
   ADD_VALUE (OID_FDDI_MAC_NOT_COPIED_THRESHOLD),
   ADD_VALUE (OID_FDDI_MAC_OLD_DOWNSTREAM_NBR),
   ADD_VALUE (OID_FDDI_MAC_OLD_UPSTREAM_NBR),
   ADD_VALUE (OID_FDDI_MAC_REQUESTED_PATHS),
   ADD_VALUE (OID_FDDI_MAC_RING_OP_CT),
   ADD_VALUE (OID_FDDI_MAC_RMT_STATE),
   ADD_VALUE (OID_FDDI_MAC_SHORT_GRP_ADDRESS),
   ADD_VALUE (OID_FDDI_MAC_SMT_ADDRESS),
   ADD_VALUE (OID_FDDI_MAC_TOKEN_CT),
   ADD_VALUE (OID_FDDI_MAC_TRANSMIT_CT),
   ADD_VALUE (OID_FDDI_MAC_TVX_CAPABILITY),
   ADD_VALUE (OID_FDDI_MAC_TVX_EXPIRED_CT),
   ADD_VALUE (OID_FDDI_MAC_TVX_VALUE),
   ADD_VALUE (OID_FDDI_MAC_T_MAX),
   ADD_VALUE (OID_FDDI_MAC_T_MAX_CAPABILITY),
   ADD_VALUE (OID_FDDI_MAC_T_NEG),
   ADD_VALUE (OID_FDDI_MAC_T_PRI0),
   ADD_VALUE (OID_FDDI_MAC_T_PRI1),
   ADD_VALUE (OID_FDDI_MAC_T_PRI2),
   ADD_VALUE (OID_FDDI_MAC_T_PRI3),
   ADD_VALUE (OID_FDDI_MAC_T_PRI4),
   ADD_VALUE (OID_FDDI_MAC_T_PRI5),
   ADD_VALUE (OID_FDDI_MAC_T_PRI6),
   ADD_VALUE (OID_FDDI_MAC_T_REQ),
   ADD_VALUE (OID_FDDI_MAC_UNDA_FLAG),
   ADD_VALUE (OID_FDDI_MAC_UPSTREAM_NBR),
   ADD_VALUE (OID_FDDI_PATH_CONFIGURATION),
   ADD_VALUE (OID_FDDI_PATH_INDEX),
   ADD_VALUE (OID_FDDI_PATH_MAX_T_REQ),
   ADD_VALUE (OID_FDDI_PATH_RING_LATENCY),
   ADD_VALUE (OID_FDDI_PATH_SBA_AVAILABLE),
   ADD_VALUE (OID_FDDI_PATH_SBA_OVERHEAD),
   ADD_VALUE (OID_FDDI_PATH_SBA_PAYLOAD),
   ADD_VALUE (OID_FDDI_PATH_TRACE_STATUS),
   ADD_VALUE (OID_FDDI_PATH_TVX_LOWER_BOUND),
   ADD_VALUE (OID_FDDI_PATH_T_MAX_LOWER_BOUND),
   ADD_VALUE (OID_FDDI_PATH_T_R_MODE),
   ADD_VALUE (OID_FDDI_PORT_ACTION),
   ADD_VALUE (OID_FDDI_PORT_AVAILABLE_PATHS),
   ADD_VALUE (OID_FDDI_PORT_BS_FLAG),
   ADD_VALUE (OID_FDDI_PORT_CONNECTION_CAPABILITIES),
   ADD_VALUE (OID_FDDI_PORT_CONNECTION_POLICIES),
   ADD_VALUE (OID_FDDI_PORT_CONNNECT_STATE),
   ADD_VALUE (OID_FDDI_PORT_CURRENT_PATH),
   ADD_VALUE (OID_FDDI_PORT_EB_ERROR_CT),
   ADD_VALUE (OID_FDDI_PORT_HARDWARE_PRESENT),
   ADD_VALUE (OID_FDDI_PORT_INDEX),
   ADD_VALUE (OID_FDDI_PORT_LCT_FAIL_CT),
   ADD_VALUE (OID_FDDI_PORT_LEM_CT),
   ADD_VALUE (OID_FDDI_PORT_LEM_REJECT_CT),
   ADD_VALUE (OID_FDDI_PORT_LER_ALARM),
   ADD_VALUE (OID_FDDI_PORT_LER_CUTOFF),
   ADD_VALUE (OID_FDDI_PORT_LER_ESTIMATE),
   ADD_VALUE (OID_FDDI_PORT_LER_FLAG),
   ADD_VALUE (OID_FDDI_PORT_MAC_INDICATED),
   ADD_VALUE (OID_FDDI_PORT_MAC_LOOP_TIME),
   ADD_VALUE (OID_FDDI_PORT_MAC_PLACEMENT),
   ADD_VALUE (OID_FDDI_PORT_MAINT_LS),
   ADD_VALUE (OID_FDDI_PORT_MY_TYPE),
   ADD_VALUE (OID_FDDI_PORT_NEIGHBOR_TYPE),
   ADD_VALUE (OID_FDDI_PORT_PCM_STATE),
   ADD_VALUE (OID_FDDI_PORT_PC_LS),
   ADD_VALUE (OID_FDDI_PORT_PC_WITHHOLD),
   ADD_VALUE (OID_FDDI_PORT_PMD_CLASS),
   ADD_VALUE (OID_FDDI_PORT_REQUESTED_PATHS),
   ADD_VALUE (OID_FDDI_RING_MGT_STATE),
   ADD_VALUE (OID_FDDI_SHORT_CURRENT_ADDR),
   ADD_VALUE (OID_FDDI_SHORT_MAX_LIST_SIZE),
   ADD_VALUE (OID_FDDI_SHORT_MULTICAST_LIST),
   ADD_VALUE (OID_FDDI_SHORT_PERMANENT_ADDR),
   ADD_VALUE (OID_FDDI_SMT_AVAILABLE_PATHS),
   ADD_VALUE (OID_FDDI_SMT_BYPASS_PRESENT),
   ADD_VALUE (OID_FDDI_SMT_CF_STATE),
   ADD_VALUE (OID_FDDI_SMT_CONFIG_CAPABILITIES),
   ADD_VALUE (OID_FDDI_SMT_CONFIG_POLICY),
   ADD_VALUE (OID_FDDI_SMT_CONNECTION_POLICY),
   ADD_VALUE (OID_FDDI_SMT_ECM_STATE),
   ADD_VALUE (OID_FDDI_SMT_HI_VERSION_ID),
   ADD_VALUE (OID_FDDI_SMT_HOLD_STATE),
   ADD_VALUE (OID_FDDI_SMT_LAST_SET_STATION_ID),
   ADD_VALUE (OID_FDDI_SMT_LO_VERSION_ID),
   ADD_VALUE (OID_FDDI_SMT_MAC_CT),
   ADD_VALUE (OID_FDDI_SMT_MAC_INDEXES),
   ADD_VALUE (OID_FDDI_SMT_MANUFACTURER_DATA),
   ADD_VALUE (OID_FDDI_SMT_MASTER_CT),
   ADD_VALUE (OID_FDDI_SMT_MIB_VERSION_ID),
   ADD_VALUE (OID_FDDI_SMT_MSG_TIME_STAMP),
   ADD_VALUE (OID_FDDI_SMT_NON_MASTER_CT),
   ADD_VALUE (OID_FDDI_SMT_OP_VERSION_ID),
   ADD_VALUE (OID_FDDI_SMT_PEER_WRAP_FLAG),
   ADD_VALUE (OID_FDDI_SMT_PORT_INDEXES),
   ADD_VALUE (OID_FDDI_SMT_REMOTE_DISCONNECT_FLAG),
   ADD_VALUE (OID_FDDI_SMT_SET_COUNT),
   ADD_VALUE (OID_FDDI_SMT_STATION_ACTION),
   ADD_VALUE (OID_FDDI_SMT_STATION_ID),
   ADD_VALUE (OID_FDDI_SMT_STATION_STATUS),
   ADD_VALUE (OID_FDDI_SMT_STAT_RPT_POLICY),
   ADD_VALUE (OID_FDDI_SMT_TRACE_MAX_EXPIRATION),
   ADD_VALUE (OID_FDDI_SMT_TRANSITION_TIME_STAMP),
   ADD_VALUE (OID_FDDI_SMT_T_NOTIFY),
   ADD_VALUE (OID_FDDI_SMT_USER_DATA),
   ADD_VALUE (OID_FDDI_UPSTREAM_NODE_LONG),
   ADD_VALUE (OID_GEN_BROADCAST_BYTES_RCV),
   ADD_VALUE (OID_GEN_BROADCAST_BYTES_XMIT),
   ADD_VALUE (OID_GEN_BROADCAST_FRAMES_RCV),
   ADD_VALUE (OID_GEN_BROADCAST_FRAMES_XMIT),
   ADD_VALUE (OID_GEN_CO_BYTES_RCV),
   ADD_VALUE (OID_GEN_CO_BYTES_XMIT),
   ADD_VALUE (OID_GEN_CO_BYTES_XMIT_OUTSTANDING),
   ADD_VALUE (OID_GEN_CO_DRIVER_VERSION),
   ADD_VALUE (OID_GEN_CO_GET_NETCARD_TIME),
   ADD_VALUE (OID_GEN_CO_GET_TIME_CAPS),
   ADD_VALUE (OID_GEN_CO_HARDWARE_STATUS),
   ADD_VALUE (OID_GEN_CO_LINK_SPEED),
   ADD_VALUE (OID_GEN_CO_MAC_OPTIONS),
   ADD_VALUE (OID_GEN_CO_MEDIA_CONNECT_STATUS),
   ADD_VALUE (OID_GEN_CO_MEDIA_IN_USE),
   ADD_VALUE (OID_GEN_CO_MEDIA_SUPPORTED),
   ADD_VALUE (OID_GEN_CO_MINIMUM_LINK_SPEED),
   ADD_VALUE (OID_GEN_CO_NETCARD_LOAD),
   ADD_VALUE (OID_GEN_CO_PROTOCOL_OPTIONS),
   ADD_VALUE (OID_GEN_CO_RCV_CRC_ERROR),
   ADD_VALUE (OID_GEN_CO_RCV_PDUS_ERROR),
   ADD_VALUE (OID_GEN_CO_RCV_PDUS_NO_BUFFER),
   ADD_VALUE (OID_GEN_CO_RCV_PDUS_OK),
   ADD_VALUE (OID_GEN_CO_SUPPORTED_LIST),
   ADD_VALUE (OID_GEN_CO_TRANSMIT_QUEUE_LENGTH),
   ADD_VALUE (OID_GEN_CO_VENDOR_DESCRIPTION),
   ADD_VALUE (OID_GEN_CO_VENDOR_DRIVER_VERSION),
   ADD_VALUE (OID_GEN_CO_VENDOR_ID),
   ADD_VALUE (OID_GEN_CO_XMIT_PDUS_ERROR),
   ADD_VALUE (OID_GEN_CO_XMIT_PDUS_OK),
   ADD_VALUE (OID_GEN_CURRENT_LOOKAHEAD),
   ADD_VALUE (OID_GEN_CURRENT_PACKET_FILTER),
   ADD_VALUE (OID_GEN_DIRECTED_BYTES_RCV),
   ADD_VALUE (OID_GEN_DIRECTED_BYTES_XMIT),
   ADD_VALUE (OID_GEN_DIRECTED_FRAMES_RCV),
   ADD_VALUE (OID_GEN_DIRECTED_FRAMES_XMIT),
   ADD_VALUE (OID_GEN_DRIVER_VERSION),
   ADD_VALUE (OID_GEN_GET_NETCARD_TIME),
   ADD_VALUE (OID_GEN_GET_TIME_CAPS),
   ADD_VALUE (OID_GEN_HARDWARE_STATUS),
   ADD_VALUE (OID_GEN_LINK_SPEED),
   ADD_VALUE (OID_GEN_MAC_OPTIONS),
   ADD_VALUE (OID_GEN_MAXIMUM_FRAME_SIZE),
   ADD_VALUE (OID_GEN_MAXIMUM_LOOKAHEAD),
   ADD_VALUE (OID_GEN_MAXIMUM_SEND_PACKETS),
   ADD_VALUE (OID_GEN_MAXIMUM_TOTAL_SIZE),
   ADD_VALUE (OID_GEN_MEDIA_CAPABILITIES),
   ADD_VALUE (OID_GEN_MEDIA_CONNECT_STATUS),
   ADD_VALUE (OID_GEN_MEDIA_IN_USE),
   ADD_VALUE (OID_GEN_MEDIA_SUPPORTED),
   ADD_VALUE (OID_GEN_MULTICAST_BYTES_RCV),
   ADD_VALUE (OID_GEN_MULTICAST_BYTES_XMIT),
   ADD_VALUE (OID_GEN_MULTICAST_FRAMES_RCV),
   ADD_VALUE (OID_GEN_MULTICAST_FRAMES_XMIT),
   ADD_VALUE (OID_GEN_NETWORK_LAYER_ADDRESSES),
   ADD_VALUE (OID_GEN_PHYSICAL_MEDIUM),
   ADD_VALUE (OID_GEN_PHYSICAL_MEDIUM_EX),
   ADD_VALUE (OID_GEN_PROTOCOL_OPTIONS),
   ADD_VALUE (OID_GEN_RCV_CRC_ERROR),
   ADD_VALUE (OID_GEN_RCV_ERROR),
   ADD_VALUE (OID_GEN_RCV_NO_BUFFER),
   ADD_VALUE (OID_GEN_RCV_OK),
   ADD_VALUE (OID_GEN_RECEIVE_BLOCK_SIZE),
   ADD_VALUE (OID_GEN_RECEIVE_BUFFER_SPACE),
   ADD_VALUE (OID_GEN_SUPPORTED_LIST),
   ADD_VALUE (OID_GEN_TRANSMIT_BLOCK_SIZE),
   ADD_VALUE (OID_GEN_TRANSMIT_BUFFER_SPACE),
   ADD_VALUE (OID_GEN_TRANSMIT_QUEUE_LENGTH),
   ADD_VALUE (OID_GEN_TRANSPORT_HEADER_OFFSET),
   ADD_VALUE (OID_GEN_VENDOR_DESCRIPTION),
   ADD_VALUE (OID_GEN_VENDOR_DRIVER_VERSION),
   ADD_VALUE (OID_GEN_VENDOR_ID),
   ADD_VALUE (OID_GEN_VLAN_ID),
   ADD_VALUE (OID_GEN_XMIT_ERROR),
   ADD_VALUE (OID_GEN_XMIT_OK),
   ADD_VALUE (OID_IP4_OFFLOAD_STATS),
   ADD_VALUE (OID_IP6_OFFLOAD_STATS),
   ADD_VALUE (OID_IRDA_EXTRA_RCV_BOFS),
   ADD_VALUE (OID_IRDA_LINK_SPEED),
   ADD_VALUE (OID_IRDA_MAX_RECEIVE_WINDOW_SIZE),
   ADD_VALUE (OID_IRDA_MAX_SEND_WINDOW_SIZE),
   ADD_VALUE (OID_IRDA_MAX_UNICAST_LIST_SIZE),
   ADD_VALUE (OID_IRDA_MEDIA_BUSY),
   ADD_VALUE (OID_IRDA_RATE_SNIFF),
   ADD_VALUE (OID_IRDA_RECEIVING),
   ADD_VALUE (OID_IRDA_SUPPORTED_SPEEDS),
   ADD_VALUE (OID_IRDA_TURNAROUND_TIME),
   ADD_VALUE (OID_IRDA_UNICAST_LIST),
   ADD_VALUE (OID_LTALK_COLLISIONS),
   ADD_VALUE (OID_LTALK_CURRENT_NODE_ID),
   ADD_VALUE (OID_LTALK_DEFERS),
   ADD_VALUE (OID_LTALK_FCS_ERRORS),
   ADD_VALUE (OID_LTALK_IN_BROADCASTS),
   ADD_VALUE (OID_LTALK_IN_LENGTH_ERRORS),
   ADD_VALUE (OID_LTALK_NO_DATA_ERRORS),
   ADD_VALUE (OID_LTALK_OUT_NO_HANDLERS),
   ADD_VALUE (OID_LTALK_RANDOM_CTS_ERRORS),
   ADD_VALUE (OID_PNP_ADD_WAKE_UP_PATTERN),
   ADD_VALUE (OID_PNP_CAPABILITIES),
   ADD_VALUE (OID_PNP_ENABLE_WAKE_UP),
   ADD_VALUE (OID_PNP_QUERY_POWER),
   ADD_VALUE (OID_PNP_REMOVE_WAKE_UP_PATTERN),
   ADD_VALUE (OID_PNP_SET_POWER),
   ADD_VALUE (OID_PNP_WAKE_UP_PATTERN_LIST),
   ADD_VALUE (OID_TAPI_ACCEPT),
   ADD_VALUE (OID_TAPI_ANSWER),
   ADD_VALUE (OID_TAPI_CLOSE),
   ADD_VALUE (OID_TAPI_CLOSE_CALL),
   ADD_VALUE (OID_TAPI_CONDITIONAL_MEDIA_DETECTION),
   ADD_VALUE (OID_TAPI_CONFIG_DIALOG),
   ADD_VALUE (OID_TAPI_DEV_SPECIFIC),
   ADD_VALUE (OID_TAPI_DIAL),
   ADD_VALUE (OID_TAPI_DROP),
   ADD_VALUE (OID_TAPI_GATHER_DIGITS),
   ADD_VALUE (OID_TAPI_GET_ADDRESS_CAPS),
   ADD_VALUE (OID_TAPI_GET_ADDRESS_ID),
   ADD_VALUE (OID_TAPI_GET_ADDRESS_STATUS),
   ADD_VALUE (OID_TAPI_GET_CALL_ADDRESS_ID),
   ADD_VALUE (OID_TAPI_GET_CALL_INFO),
   ADD_VALUE (OID_TAPI_GET_CALL_STATUS),
   ADD_VALUE (OID_TAPI_GET_DEV_CAPS),
   ADD_VALUE (OID_TAPI_GET_DEV_CONFIG),
   ADD_VALUE (OID_TAPI_GET_EXTENSION_ID),
   ADD_VALUE (OID_TAPI_GET_ID),
   ADD_VALUE (OID_TAPI_GET_LINE_DEV_STATUS),
   ADD_VALUE (OID_TAPI_MAKE_CALL),
   ADD_VALUE (OID_TAPI_MONITOR_DIGITS),
   ADD_VALUE (OID_TAPI_NEGOTIATE_EXT_VERSION),
   ADD_VALUE (OID_TAPI_OPEN),
   ADD_VALUE (OID_TAPI_PROVIDER_INITIALIZE),
   ADD_VALUE (OID_TAPI_PROVIDER_SHUTDOWN),
   ADD_VALUE (OID_TAPI_SECURE_CALL),
   ADD_VALUE (OID_TAPI_SELECT_EXT_VERSION),
   ADD_VALUE (OID_TAPI_SEND_USER_USER_INFO),
   ADD_VALUE (OID_TAPI_SET_APP_SPECIFIC),
   ADD_VALUE (OID_TAPI_SET_CALL_PARAMS),
   ADD_VALUE (OID_TAPI_SET_DEFAULT_MEDIA_DETECTION),
   ADD_VALUE (OID_TAPI_SET_DEV_CONFIG),
   ADD_VALUE (OID_TAPI_SET_MEDIA_MODE),
   ADD_VALUE (OID_TAPI_SET_STATUS_MESSAGES),
   ADD_VALUE (OID_TCP4_OFFLOAD_STATS),
   ADD_VALUE (OID_TCP6_OFFLOAD_STATS),
   ADD_VALUE (OID_TCP_SAN_SUPPORT),
   ADD_VALUE (OID_TCP_TASK_IPSEC_ADD_SA),
   ADD_VALUE (OID_TCP_TASK_IPSEC_ADD_UDPESP_SA),
   ADD_VALUE (OID_TCP_TASK_IPSEC_DELETE_SA),
   ADD_VALUE (OID_TCP_TASK_IPSEC_DELETE_UDPESP_SA),
   ADD_VALUE (OID_TCP_TASK_OFFLOAD),
   ADD_VALUE (OID_WAN_CURRENT_ADDRESS),
   ADD_VALUE (OID_WAN_GET_BRIDGE_INFO),
   ADD_VALUE (OID_WAN_GET_COMP_INFO),
   ADD_VALUE (OID_WAN_GET_INFO),
   ADD_VALUE (OID_WAN_GET_LINK_INFO),
   ADD_VALUE (OID_WAN_GET_STATS_INFO),
   ADD_VALUE (OID_WAN_HEADER_FORMAT),
   ADD_VALUE (OID_WAN_LINE_COUNT),
   ADD_VALUE (OID_WAN_MEDIUM_SUBTYPE),
   ADD_VALUE (OID_WAN_PERMANENT_ADDRESS),
   ADD_VALUE (OID_WAN_PROTOCOL_TYPE),
   ADD_VALUE (OID_WAN_QUALITY_OF_SERVICE),
   ADD_VALUE (OID_WAN_SET_BRIDGE_INFO),
   ADD_VALUE (OID_WAN_SET_COMP_INFO),
   ADD_VALUE (OID_WAN_SET_LINK_INFO),
   ADD_VALUE (OID_WW_ARD_DATAGRAM),
   ADD_VALUE (OID_WW_ARD_SNDCP),
   ADD_VALUE (OID_WW_ARD_TMLY_MSG),
   ADD_VALUE (OID_WW_CDPD_AREA_COLOR),
   ADD_VALUE (OID_WW_CDPD_CHANNEL_SELECT),
   ADD_VALUE (OID_WW_CDPD_CHANNEL_STATE),
   ADD_VALUE (OID_WW_CDPD_CIRCUIT_SWITCHED),
   ADD_VALUE (OID_WW_CDPD_DATA_COMPRESSION),
   ADD_VALUE (OID_WW_CDPD_EID),
   ADD_VALUE (OID_WW_CDPD_HEADER_COMPRESSION),
   ADD_VALUE (OID_WW_CDPD_NEI),
   ADD_VALUE (OID_WW_CDPD_NEI_STATE),
   ADD_VALUE (OID_WW_CDPD_RSSI),
   ADD_VALUE (OID_WW_CDPD_SERVICE_PROVIDER_IDENTIFIER),
   ADD_VALUE (OID_WW_CDPD_SLEEP_MODE),
   ADD_VALUE (OID_WW_CDPD_SPNI),
   ADD_VALUE (OID_WW_CDPD_TEI),
   ADD_VALUE (OID_WW_CDPD_TX_POWER_LEVEL),
   ADD_VALUE (OID_WW_CDPD_WASI),
   ADD_VALUE (OID_WW_GEN_BASESTATION_ID),
   ADD_VALUE (OID_WW_GEN_BATTERY_LEVEL),
   ADD_VALUE (OID_WW_GEN_CHANNEL_ID),
   ADD_VALUE (OID_WW_GEN_CHANNEL_QUALITY),
   ADD_VALUE (OID_WW_GEN_CURRENT_ADDRESS),
   ADD_VALUE (OID_WW_GEN_DEVICE_INFO),
   ADD_VALUE (OID_WW_GEN_DISABLE_TRANSMITTER),
   ADD_VALUE (OID_WW_GEN_ENCRYPTION_IN_USE),
   ADD_VALUE (OID_WW_GEN_ENCRYPTION_STATE),
   ADD_VALUE (OID_WW_GEN_ENCRYPTION_SUPPORTED),
   ADD_VALUE (OID_WW_GEN_EXTERNAL_POWER),
   ADD_VALUE (OID_WW_GEN_HEADER_FORMATS_SUPPORTED),
   ADD_VALUE (OID_WW_GEN_HEADER_FORMAT_IN_USE),
   ADD_VALUE (OID_WW_GEN_INDICATION_REQUEST),
   ADD_VALUE (OID_WW_GEN_LATENCY),
   ADD_VALUE (OID_WW_GEN_LOCK_STATUS),
   ADD_VALUE (OID_WW_GEN_NETWORK_ID),
   ADD_VALUE (OID_WW_GEN_NETWORK_TYPES_SUPPORTED),
   ADD_VALUE (OID_WW_GEN_NETWORK_TYPE_IN_USE),
   ADD_VALUE (OID_WW_GEN_OPERATION_MODE),
   ADD_VALUE (OID_WW_GEN_PERMANENT_ADDRESS),
   ADD_VALUE (OID_WW_GEN_RADIO_LINK_SPEED),
   ADD_VALUE (OID_WW_GEN_REGISTRATION_STATUS),
   ADD_VALUE (OID_WW_GEN_SUSPEND_DRIVER),
   ADD_VALUE (OID_WW_MBX_FLEXLIST),
   ADD_VALUE (OID_WW_MBX_GROUPLIST),
   ADD_VALUE (OID_WW_MBX_LIVE_DIE),
   ADD_VALUE (OID_WW_MBX_SUBADDR),
   ADD_VALUE (OID_WW_MBX_TEMP_DEFAULTLIST),
   ADD_VALUE (OID_WW_MBX_TRAFFIC_AREA),
   ADD_VALUE (OID_WW_MET_FUNCTION),
   ADD_VALUE (OID_WW_PIN_LAST_LOCATION),
   ADD_VALUE (OID_WW_PIN_LOC_AUTHORIZE),
   ADD_VALUE (OID_WW_PIN_LOC_FIX),
   ADD_VALUE (OID_WW_TAC_COMPRESSION),
   ADD_VALUE (OID_WW_TAC_GET_STATUS),
   ADD_VALUE (OID_WW_TAC_SET_CONFIG),
   ADD_VALUE (OID_WW_TAC_USER_HEADER),
   ADD_VALUE (OID_GEN_BYTES_RCV),
   ADD_VALUE (OID_GEN_BYTES_XMIT),
   ADD_VALUE (OID_GEN_RCV_DISCARDS)
};
#endif  /* USE_DEBUG */

/**
 * Performs a query/set operation on an internal variable of the
 * network card driver.
 *
 * \param AdapterObject Pointer to an ADAPTER structure.
 * \param set determines if the operation is a set (set = TRUE) or a
 *        query (set = FALSE).
 * \param oidData A pointer to a PACKET_OID_DATA structure that contains or
 *        receives the data.
 * \return If the function succeeds, the return value is nonzero.
 *
 * \note Not all the network adapters implement all the query/set functions.
 *
 * There is a set of mandatory OID functions that is granted to be present on
 * all the adapters, and a set of facultative functions, not provided by all
 * the cards (see the Microsoft DDKs to see which functions are mandatory).
 * If you use a facultative function, be careful to enclose it in an if
 * statement to check the result.
 */
BOOL PacketRequest2 (const ADAPTER *AdapterObject, BOOL set,
                     PACKET_OID_DATA *oidData,
                     const char *file, unsigned line)
{
  DWORD  transfered, in_size, out_size;
  DWORD  in_len, out_len;
  BOOL   rc;
  int    error = 0;
  char   prof_buf[100] = "";

#if defined(USE_PROFILER) && defined(USE_DEBUG)
  uint64 start = U64_SUFFIX (0);

  if (profile_enable)
     start = win_get_perf_count();
#endif

  transfered = 0;
  in_len   = oidData->Length;
  in_size  = sizeof(*oidData) - 1 + oidData->Length;
  out_size = in_size;

  rc = DeviceIoControl (AdapterObject->hFile,
                        set ? pBIOCSETOID : pBIOCQUERYOID,
                        oidData, in_size, oidData, out_size, &transfered, NULL);

  out_len = rc ? oidData->Length : 0;

  if (!rc)
     error = GetLastError();

#if defined(USE_DEBUG)
  #if defined(USE_PROFILER)
  if (profile_enable)
  {
    int64  clocks = (int64) (win_get_perf_count() - start);
    double msec   = (double)clocks / ((double)clocks_per_usec * 1000.0);
    SNPRINTF (prof_buf, sizeof(prof_buf), " (%.3f ms)", msec);
  }
  #endif

  winpkt_trace_func = "PacketRequest";
  winpkt_trace_file = file;
  winpkt_trace_line = line;
  winpkt_trace ("%-30.30s len %lu/%lu, xfered %lu, set %d, rc %d; %s %s\n",
                list_lookup(oidData->Oid,oid_list,DIM(oid_list)),
                (u_long)in_len, (u_long)out_len, (u_long)transfered, set, rc,
                !rc ? win_strerror(error) : "okay", prof_buf);
#endif

  if (!rc)
     SetLastError (error);
  return (rc);
}

/*
 * Formerly in AdInfo.c:
 *
 * This file contains the support functions used by packet.dll to retrieve
 * information about installed adapters, like
 *
 * - the adapter list
 * - the device associated to any adapter and the description of the adapter
 */

/**
 * Scan the registry to retrieve the IP addresses of an adapter.
 *
 * \param AdapterName String that contains the name of the adapter.
 * \param buffer A user allocated array of npf_if_addr that will be filled
 *        by the function.
 * \param NEntries Size of the array (in npf_if_addr).
 * \return If the function succeeds, the return value is nonzero.
 *
 * This function grabs from the registry information like the IP addresses,
 * the netmasks and the broadcast addresses of an interface. The buffer
 * passed by the user is filled with npf_if_addr structures, each of which
 * contains the data for a single address. If the buffer is full, the reaming
 * addresses are dropeed, therefore set its dimension to sizeof(npf_if_addr)
 * if you want only the first address.
 */
static BOOL GetAddressesFromRegistry (const char  *AdapterName,
                                      npf_if_addr *buffer,
                                      int         *NEntries)
{
  struct sockaddr_in *TmpAddr, *TmpBroad;
  const  char *guid;
  HKEY   TcpIpKey = NULL, UnderTcpKey = NULL;
  char   String [1024+1];
  DWORD  RegType, BufLen, StringPos, DHCPEnabled, ZeroBroadcast;
  LONG   status;
  DWORD  line = 0;
  int    naddrs, nmasks;

  *NEntries = 0;

  winpkt_trace_func = "GetAddressesFromRegistry";

  guid = strchr (AdapterName, '{');

  WINPKT_TRACE ("guid %s\n", guid);

  if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
       "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces",
       0, KEY_READ, &UnderTcpKey) == ERROR_SUCCESS)
  {
    status = RegOpenKeyExA (UnderTcpKey, guid, 0, KEY_READ, &TcpIpKey);
    line = __LINE__ - 1;
    if (status != ERROR_SUCCESS)
       goto fail;
  }
  else
  {
    HKEY SystemKey, ParametersKey, InterfaceKey;

    /* Query the registry key with the interface's addresses
     */
    status = RegOpenKeyExA (HKEY_LOCAL_MACHINE,
                            "SYSTEM\\CurrentControlSet\\Services",
                            0, KEY_READ, &SystemKey);
    if (status != ERROR_SUCCESS)
       goto fail;

    status = RegOpenKeyExA (SystemKey, guid, 0, KEY_READ, &InterfaceKey);
    line = __LINE__ - 1;
    RegCloseKey (SystemKey);
    if (status != ERROR_SUCCESS)
       goto fail;

    status = RegOpenKeyExA (InterfaceKey, "Parameters", 0, KEY_READ,
                            &ParametersKey);
    line = __LINE__ - 1;
    RegCloseKey (InterfaceKey);
    if (status != ERROR_SUCCESS)
       goto fail;

    status = RegOpenKeyExA (ParametersKey, "TcpIp", 0, KEY_READ, &TcpIpKey);
    line = __LINE__ - 1;
    RegCloseKey (ParametersKey);
    if (status != ERROR_SUCCESS)
       goto fail;

    BufLen = sizeof(String);
  }

  BufLen = 4;

  /* Try to detect if the interface has a zero broadcast addr
   */
  status = RegQueryValueExA (TcpIpKey, "UseZeroBroadcast", NULL,
                             &RegType, (BYTE*)&ZeroBroadcast, &BufLen);
  if (status != ERROR_SUCCESS)
     ZeroBroadcast = 0;

  BufLen = 4;

  /* See if DHCP is used by this system
   */
  status = RegQueryValueExA (TcpIpKey, "EnableDHCP", NULL,
                             &RegType, (BYTE*)&DHCPEnabled, &BufLen);
  if (status != ERROR_SUCCESS)
     DHCPEnabled = 0;

  /* Retrieve the addresses
   */
  if (DHCPEnabled)
  {
    WINPKT_TRACE ("  DHCP enabled\n");

    BufLen = sizeof(String);
    status = RegQueryValueExA (TcpIpKey, "DhcpIPAddress", NULL,
                               &RegType, (BYTE*)String, &BufLen);
    line = __LINE__ - 1;
    if (status != ERROR_SUCCESS)
       goto fail;

    /* scan the key to obtain the addresses
     */
    StringPos = 0;
    for (naddrs = 0; naddrs < MAX_NETWORK_ADDRESSES; naddrs++)
    {
      TmpAddr = (struct sockaddr_in*) &buffer[naddrs].IPAddress;
      WINPKT_TRACE ("  addr %s\n", String+StringPos);

      TmpAddr->sin_addr.s_addr = htonl (aton(String+StringPos));
      if (TmpAddr->sin_addr.s_addr)
      {
        TmpAddr->sin_family = AF_INET;
        TmpBroad = (struct sockaddr_in*) &buffer[naddrs].Broadcast;
        TmpBroad->sin_family = AF_INET;
        if (!ZeroBroadcast)
             TmpBroad->sin_addr.s_addr = INADDR_BROADCAST;
        else TmpBroad->sin_addr.s_addr = INADDR_ANY;

        while (*(String+StringPos))
             StringPos++;
        StringPos++;

        if (*(String + StringPos) == '\0' || StringPos >= BufLen)
           break;
      }
      else
        break;
    }

    BufLen = sizeof(String);

    /* Open the key with the netmasks
     */
    status = RegQueryValueExA (TcpIpKey, "DhcpSubnetMask", NULL,
                               &RegType, (BYTE*)String, &BufLen);
    line = __LINE__ - 1;
    if (status != ERROR_SUCCESS)
       goto fail;

    /* scan the key to obtain the masks
     */
    StringPos = 0;
    for (nmasks = 0; nmasks < MAX_NETWORK_ADDRESSES; nmasks++)
    {
      TmpAddr = (struct sockaddr_in*) &buffer[nmasks].SubnetMask;
      WINPKT_TRACE ("  addr %s\n", String+StringPos);

      TmpAddr->sin_addr.s_addr = htonl (aton(String+StringPos));
      if (TmpAddr->sin_addr.s_addr)
      {
        TmpAddr->sin_family = AF_INET;

        while (*(String + StringPos))
              StringPos++;
        StringPos++;
        if (*(String + StringPos) == '\0' || StringPos >= BufLen)
           break;
      }
      else
        break;
    }

    /* The number of masks MUST be equal to the number of addresses
     */
    if (nmasks != naddrs)
       goto fail;
  }
  else   /* Not DHCP enabled */
  {
    WINPKT_TRACE ("  Not DHCP enabled\n");

    BufLen = sizeof(String);

    /* Open the key with the addresses
     */
    status = RegQueryValueExA (TcpIpKey, "IPAddress", NULL,
                               &RegType, (BYTE*)String, &BufLen);
    line = __LINE__ - 1;
    if (status != ERROR_SUCCESS)
       goto fail;

    /* scan the key to obtain the addresses
     */
    StringPos = 0;
    for (naddrs = 0; naddrs < MAX_NETWORK_ADDRESSES; naddrs++)
    {
      TmpAddr = (struct sockaddr_in*) &buffer[naddrs].IPAddress;
      WINPKT_TRACE ("  addr %s\n", String+StringPos);

      TmpAddr->sin_addr.s_addr = htonl (aton(String+StringPos));
      if (TmpAddr->sin_addr.s_addr)
      {
        TmpAddr->sin_family = AF_INET;

        TmpBroad = (struct sockaddr_in*) &buffer[naddrs].Broadcast;
        TmpBroad->sin_family = AF_INET;
        if (!ZeroBroadcast)
             TmpBroad->sin_addr.s_addr = INADDR_BROADCAST;
        else TmpBroad->sin_addr.s_addr = INADDR_ANY;

        while (*(String + StringPos))
              StringPos++;
        StringPos++;

        if (*(String + StringPos) == '\0' || StringPos >= BufLen)
           break;
      }
      else
        break;
    }

    /* Open the key with the netmasks
     */
    BufLen = sizeof(String);
    status = RegQueryValueExA (TcpIpKey, "SubnetMask", NULL,
                               &RegType, (BYTE*)String, &BufLen);
    line = __LINE__ - 1;
    if (status != ERROR_SUCCESS)
       goto fail;

    /* scan the key to obtain the masks
     */
    StringPos = 0;
    for (nmasks = 0; nmasks < MAX_NETWORK_ADDRESSES; nmasks++)
    {
      TmpAddr = (struct sockaddr_in*) &buffer[nmasks].SubnetMask;
      WINPKT_TRACE ("  mask %s\n", String+StringPos);

      TmpAddr->sin_addr.s_addr = htonl (aton(String+StringPos));
      if (TmpAddr->sin_addr.s_addr)
      {
        TmpAddr->sin_family = AF_INET;

        while (*(String + StringPos) != 0)
              StringPos++;
        StringPos++;

        if (*(String + StringPos) == 0 || StringPos >= BufLen)
           break;
      }
      else
        break;
    }

    /* The number of masks MUST be equal to the number of addresses
     */
    if (nmasks != naddrs)
       goto fail;
  }

  *NEntries = naddrs + 1;

  RegCloseKey (TcpIpKey);
  RegCloseKey (UnderTcpKey);

  return (status != ERROR_SUCCESS);

fail:
  WINPKT_TRACE ("failed line %lu; %s\n", (u_long)line, win_strerror(GetLastError()));

  if (TcpIpKey)
     RegCloseKey (TcpIpKey);
  if (UnderTcpKey)
     RegCloseKey (UnderTcpKey);
  *NEntries = 0;
  ARGSUSED (line);
  return (FALSE);
}

/**
 * Adds an entry to the 'adapters_list' list.
 *
 * \param AdName Name of the adapter to add
 * \return If the function succeeds, the return value is nonzero.
 *
 * Used by PacketGetAdapters(). Queries the registry to fill the
 * '*ADAPTER_INFO' describing the new adapter.
 */
static BOOL AddAdapter (const char *AdName)
{
  ADAPTER_INFO *TmpAdInfo = NULL;
  ADAPTER      *adapter   = NULL;
  BOOL          rc = FALSE;

  winpkt_trace_func = "AddAdapter";
  WINPKT_TRACE ("adapter %s\n", AdName);

  WaitForSingleObject (adapters_mutex, INFINITE);

  for (TmpAdInfo = adapters_list; TmpAdInfo; TmpAdInfo = TmpAdInfo->Next)
  {
    if (!strcmp(AdName, TmpAdInfo->Name))
    {
      WINPKT_TRACE ("adapter already present in the list\n");
      ReleaseMutex (adapters_mutex);
      return (TRUE);
    }
  }

  TmpAdInfo = NULL;

  /* Here we could have released the mutex, but what happens if two
   * threads try to add the same adapter? The adapter would be duplicated
   * on the linked list.
   */

  /* Try to Open the adapter
   */
  adapter = PacketOpenAdapterNPF (AdName);
  winpkt_trace_func = "AddAdapter";

  if (!adapter)
     goto quit;

  /* PacketOpenAdapter() was succesful. Consider this a valid adapter
   * and allocate an entry for it In the adapter list.
   */
  TmpAdInfo = GlobalAllocPtr (GMEM_MOVEABLE | GMEM_ZEROINIT,
                              sizeof(*TmpAdInfo));
  if (!TmpAdInfo)
  {
    WINPKT_TRACE ("GlobalAllocPtr Failed\n");
    goto quit;
  }

  adapter->info = TmpAdInfo;

  /* Copy the device name
   */
  strcpy (TmpAdInfo->Name, AdName);

  GetAddressesFromRegistry (TmpAdInfo->Name,
                            TmpAdInfo->NetworkAddresses,
                            &TmpAdInfo->NNetworkAddresses);

  /* Update the AdaptersInfo list
   */
  TmpAdInfo->Next  = adapters_list;
  adapters_list = TmpAdInfo;

  rc = TRUE;

quit:
  /* Close it; we don't need it until user open it again.
   */
  if (adapter)
     PacketCloseAdapter (adapter);
  if (!rc && TmpAdInfo)
     (void) GlobalFreePtr (TmpAdInfo);
  ReleaseMutex (adapters_mutex);
  return (rc);
}

/**
 * Updates the list of the adapters querying the registry.
 *
 * \return If the function succeeds, the return value is nonzero.
 *
 * This function populates the list of adapter descriptions, retrieving
 * the information from the registry.
 */
static BOOL PacketGetAdapters (void)
{
  HKEY  LinkageKey, AdapKey, OneAdapKey;
  DWORD RegKeySize = 0;
  DWORD dim, RegType;
  char  TName[256];
  char  TAName[256];
  char  AdapName[256];
  const char *guid, *err;
  LONG  Status;
  int   i = 0;

  winpkt_trace_func = "PacketGetAdapters";
  WINPKT_TRACE ("\n");

  Status = RegOpenKeyExA (HKEY_LOCAL_MACHINE, ADAPTER_KEY_CLASS,
                          0, KEY_READ, &AdapKey);
  i = 0;

  if (Status != ERROR_SUCCESS)
  {
    WINPKT_TRACE ("RegOpenKeyEx ( Class\\{networkclassguid} ) Failed\n");
    goto nt4;
  }

  WINPKT_TRACE ("cycling through the adapters:\n");

  /* Cycle through the entries inside the ADAPTER_KEY_CLASS key
   * to get the names of the adapters
   */
  while (RegEnumKeyA (AdapKey, i, AdapName, sizeof(AdapName)) == ERROR_SUCCESS)
  {
    WINPKT_TRACE ("  loop %d: sub-key %s\n", i, AdapName);
    i++;

    /* Get the adapter name from the registry key
     */
    Status = RegOpenKeyExA (AdapKey, AdapName, 0, KEY_READ, &OneAdapKey);
    if (Status != ERROR_SUCCESS)
    {
      WINPKT_TRACE ("  RegOpenKeyEx (OneAdapKey) Failed; %s\n",
                    win_strerror(GetLastError()));
      continue;
    }

    Status = RegOpenKeyExA (OneAdapKey, "Linkage", 0, KEY_READ, &LinkageKey);
    if (Status != ERROR_SUCCESS)
    {
      WINPKT_TRACE ("  RegOpenKeyEx (LinkageKey) Failed; %s\n",
                    win_strerror(GetLastError()));
      RegCloseKey (OneAdapKey);
      continue;
    }

    dim = sizeof(TName);
    Status = RegQueryValueExA (LinkageKey, "Export", NULL, NULL,
                               (BYTE*)TName, &dim);
    err = win_strerror (GetLastError());
    RegCloseKey (OneAdapKey);
    RegCloseKey (LinkageKey);
    if (Status != ERROR_SUCCESS)
    {
      WINPKT_TRACE ("  Name = SKIPPED (error reading the key); %s\n", err);
      continue;
    }
    if (!TName[0])
    {
      WINPKT_TRACE ("  Name = SKIPPED (empty name)\n");
      continue;
    }
    guid = strchr (TName, '{');
    if (!guid)
    {
      WINPKT_TRACE ("  Name = SKIPPED (missing GUID)\n");
      continue;
    }

    WINPKT_TRACE ("  key %d: %s\n", i, TName);

    /* Put the \Device\NPF_ string at the beginning of the name
     */
    SNPRINTF (TAName, sizeof(TAName), "\\Device\\NPF_%s", guid);

    /* If the adapter is valid, add it to the list.
     */
    AddAdapter (TAName);
    winpkt_trace_func = "PacketGetAdapters";
  }

  RegCloseKey (AdapKey);

nt4:

  /*
   * No adapters were found under 'ADAPTER_KEY_CLASS'.
   * This means with great probability that we are under Win-NT 4, so we
   * try to look under the tcpip bindings.
   */

  if (i == 0)
       WINPKT_TRACE ("Adapters not found under %s. Using the TCP/IP bindings.\n",
                     ADAPTER_KEY_CLASS);
  else WINPKT_TRACE ("Trying NT4 bindings anyway\n");

  Status = RegOpenKeyExA (HKEY_LOCAL_MACHINE,
                          "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Linkage",
                          0, KEY_READ, &LinkageKey);
  if (Status == ERROR_SUCCESS)
  {
    BYTE *bpStr;

    /* Retrieve the length of the binding key
     */
    RegQueryValueExA (LinkageKey, "bind", NULL, &RegType,
                      NULL, &RegKeySize);

    bpStr = calloc (RegKeySize+1, 1);
    if (!bpStr)
       return (FALSE);

    /* Query the key again to get its content
     */
    RegQueryValueExA (LinkageKey, "bind", NULL, &RegType,
                      bpStr, &RegKeySize);
    RegCloseKey (LinkageKey);

    /* Loop over the space separated "\Device\{.." strings.
     */
    for (guid = strchr((const char*)bpStr,'{');
         guid;
         guid = strchr(guid+1,'{'))
    {
      SNPRINTF (TAName, sizeof(TAName), "\\Device\\NPF_%s", guid);
      AddAdapter (TAName);
      winpkt_trace_func = "PacketGetAdapters";
    }
    free (bpStr);
  }
  else
  {
    WINPKT_TRACE ("Cannot find the TCP/IP bindings");
    return (FALSE);
  }
  return (TRUE);
}

/**
 * Find the information about an adapter scanning the global ADAPTER_INFO list.
 *
 * \param AdapterName Name of the adapter.
 * \return NULL if function fails, otherwise adapter info.
 */
const ADAPTER_INFO *PacketFindAdInfo (const char *AdapterName)
{
  const ADAPTER_INFO *ad_info;

  winpkt_trace_func = "PacketFindAdInfo";
  WINPKT_TRACE ("\n");

  for (ad_info = adapters_list; ad_info; ad_info = ad_info->Next)
      if (!strcmp(ad_info->Name, AdapterName))
         break;
  return (ad_info);
}

/**
 * Return list of adapters.
 */
const ADAPTER_INFO *PacketGetAdInfo (void)
{
  winpkt_trace_func = "PacketGetAdInfo";
  WINPKT_TRACE ("\n");

  return (adapters_list);
}

/**
 * Populates the list of the adapters.
 *
 * This function populates the list of adapter descriptions, invoking
 * PacketGetAdapters(). Called from PacketInitModule() only.
 */
static BOOL PopulateAdaptersInfoList (void)
{
  BOOL rc = TRUE;

  winpkt_trace_func = "PopulateAdaptersInfoList";
  WINPKT_TRACE ("\n");

  WaitForSingleObject (adapters_mutex, INFINITE);

  adapters_list = NULL;

  /* Fill the list
   */
  if (!PacketGetAdapters())
  {
    WINPKT_TRACE ("registry scan for adapters failed!\n");
    rc = FALSE;
  }
  ReleaseMutex (adapters_mutex);
  return (rc);
}

/**
 * Free the list of the adapters.
 */
static BOOL FreeAdaptersInfoList (void)
{
  ADAPTER_INFO *next, *ai;

  WaitForSingleObject (adapters_mutex, INFINITE);

  for (ai = adapters_list; ai; ai = next)
  {
    next = ai->Next;
    (void) GlobalFreePtr (ai);
  }
  adapters_list = NULL;
  return (TRUE);
}

BOOL WanPacketSetBpfFilter (WAN_ADAPTER *wan_adapter, PUCHAR FilterCode, DWORD Length)
{
  return (use_wanpacket ? (*p_WanPacketSetBpfFilter)(wan_adapter, FilterCode, Length) : FALSE);
}

WAN_ADAPTER *WanPacketOpenAdapter (void)
{
  return (use_wanpacket ? (*p_WanPacketOpenAdapter)() : NULL);
}

BOOL WanPacketCloseAdapter (WAN_ADAPTER *wan_adapter)
{
  return (use_wanpacket ? (*p_WanPacketCloseAdapter)(wan_adapter) : FALSE);
}

BOOL WanPacketSetBufferSize (WAN_ADAPTER *wan_adapter, DWORD BufferSize)
{
  return (use_wanpacket ? (*p_WanPacketSetBufferSize)(wan_adapter, BufferSize) : FALSE);
}

DWORD WanPacketReceivePacket (WAN_ADAPTER *wan_adapter, PUCHAR Buffer, DWORD BufferSize)
{
  return (use_wanpacket ? (*p_WanPacketReceivePacket)(wan_adapter, Buffer, BufferSize) : 0UL);
}

BOOL WanPacketSetMinToCopy (WAN_ADAPTER *wan_adapter, DWORD MinToCopy)
{
  return (use_wanpacket ? (*p_WanPacketSetMinToCopy)(wan_adapter, MinToCopy) : FALSE);
}

BOOL WanPacketGetStats (WAN_ADAPTER *wan_adapter, struct bpf_stat *s)
{
  return (use_wanpacket ? (*p_WanPacketGetStats)(wan_adapter, s) : FALSE);
}

BOOL WanPacketSetReadTimeout (WAN_ADAPTER *wan_adapter, DWORD ReadTimeout)
{
  return (use_wanpacket ? (*p_WanPacketSetReadTimeout)(wan_adapter, ReadTimeout) : FALSE);
}

BOOL WanPacketSetMode (WAN_ADAPTER *wan_adapter, DWORD Mode)
{
  return (use_wanpacket ? (*p_WanPacketSetMode)(wan_adapter, Mode) : FALSE);
}

HANDLE WanPacketGetReadEvent (WAN_ADAPTER *wan_adapter)
{
  return (use_wanpacket ? (*p_WanPacketGetReadEvent)(wan_adapter) : NULL);
}

BOOL WanPacketTestAdapter (void)
{
  return (use_wanpacket ? (*p_WanPacketTestAdapter)() : FALSE);
}
#endif   /* WIN32 */
