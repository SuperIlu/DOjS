/*!\file win_dll.c
 *
 *  Dynamic loading of Windows functions at runtime.
 *
 *  Copyright (c) 2004 Gisle Vanem <gvanem@yahoo.no>
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

#include "wattcp.h"
#include "misc.h"
#include "misc_str.h"
#include "pcdbug.h"

#if defined(WIN32)  /* Rest of file */

#include "packet32.h"
#include "win_dll.h"

/* Function-ptr for functions in these DLLs:
 */
func_GetComputerNameExA             p_GetComputerNameExA;             /* Kernel32.dll, >= Win-2000 Pro */
func_GetFileVersionInfoSizeA        p_GetFileVersionInfoSizeA;        /* version.dll */
func_GetFileVersionInfoA            p_GetFileVersionInfoA;            /* version.dll */
func_GetFileVersionInfoExA          p_GetFileVersionInfoExA;          /* version.dll, >= Win-Vista (NOT USED) */
func_VerQueryValueA                 p_VerQueryValueA;                 /* version.dll, >= Win-2000 Pro  */
func_QueryThreadCycleTime           p_QueryThreadCycleTime;           /* kernel32.dll, >= Win-Vista */
func_RtlGetVersion                  p_RtlGetVersion;                  /* ntdll.dll, >= Win-2000 */
func_NtQueryInformationThread       p_NtQueryInformationThread;       /* ntdll.dll, >= Win-2003 */
func_NtQuerySystemInformation       p_NtQuerySystemInformation;       /* ntdll.dll, >= Win-2003 */
func_IsWow64Process                 p_IsWow64Process;                 /* Kernel32.dll, >= Win-XP SP2 */
func_GetThreadInformation           p_GetThreadInformation;           /* Kernel32.dll, >= Win-8 */
func_GetSystemTimePreciseAsFileTime p_GetSystemTimePreciseAsFileTime; /* Kernel32.dll, >= Win-8 */

/* Function-ptr for functions in WinPacket.dll:
 *  (the WinPcap low-level NetMon wrapper library)
 */
func_WanPacketSetBpfFilter   p_WanPacketSetBpfFilter   = NULL;
func_WanPacketOpenAdapter    p_WanPacketOpenAdapter    = NULL;
func_WanPacketCloseAdapter   p_WanPacketCloseAdapter   = NULL;
func_WanPacketSetBufferSize  p_WanPacketSetBufferSize  = NULL;
func_WanPacketReceivePacket  p_WanPacketReceivePacket  = NULL;
func_WanPacketSetMinToCopy   p_WanPacketSetMinToCopy   = NULL;
func_WanPacketGetStats       p_WanPacketGetStats       = NULL;
func_WanPacketSetReadTimeout p_WanPacketSetReadTimeout = NULL;
func_WanPacketSetMode        p_WanPacketSetMode        = NULL;
func_WanPacketGetReadEvent   p_WanPacketGetReadEvent   = NULL;
func_WanPacketTestAdapter    p_WanPacketTestAdapter    = NULL;

#if defined(HAVE_WINDNS_H)
  func_DnsFree                 p_DnsFree = NULL;
  func_DnsQuery_A              p_DnsQuery_A = NULL;
  func_DnsModifyRecordsInSet_A p_DnsModifyRecordsInSet_A = NULL;
#endif

#if defined(USE_BUGTRAP)
  func_BT_InstallSehFilter    p_BT_InstallSehFilter    = NULL;
  func_BT_SetPreErrHandler    p_BT_SetPreErrHandler    = NULL;
  func_BT_SetAppVersion       p_BT_SetAppVersion       = NULL;
  func_BT_SetAppName          p_BT_SetAppName          = NULL;
  func_BT_SetReportFormat     p_BT_SetReportFormat     = NULL;
  func_BT_SetSupportEMail     p_BT_SetSupportEMail     = NULL;
  func_BT_SetFlags            p_BT_SetFlags            = NULL;
  func_BT_SetLogSizeInEntries p_BT_SetLogSizeInEntries = NULL;
  func_BT_SetLogFlags         p_BT_SetLogFlags         = NULL;
  func_BT_SetLogEchoMode      p_BT_SetLogEchoMode      = NULL;
  func_BT_InsLogEntryF        p_BT_InsLogEntryF        = NULL;
  func_BT_OpenLogFile         p_BT_OpenLogFile         = NULL;
  func_BT_CloseLogFile        p_BT_CloseLogFile        = NULL;
  func_BT_AddLogFile          p_BT_AddLogFile          = NULL;
#endif

#define ADD_VALUE(dll, func)  { NULL, dll, #func, (void**)&p_##func }

/*
 * The DLL names must be grouped together for load_dynamic_table() to work.
 */
struct LoadTable dyn_funcs [] = {
                 ADD_VALUE ("kernel32.dll",  QueryThreadCycleTime),
                 ADD_VALUE ("kernel32.dll",  GetComputerNameExA),
                 ADD_VALUE ("kernel32.dll",  IsWow64Process),
                 ADD_VALUE ("kernel32.dll",  GetThreadInformation),
                 ADD_VALUE ("kernel32.dll",  GetSystemTimePreciseAsFileTime),
                 ADD_VALUE ("version.dll",   GetFileVersionInfoSizeA),
                 ADD_VALUE ("version.dll",   GetFileVersionInfoA),
                 ADD_VALUE ("version.dll",   GetFileVersionInfoExA),
                 ADD_VALUE ("version.dll",   VerQueryValueA),
                 ADD_VALUE ("ntdll.dll",     NtQueryInformationThread),
                 ADD_VALUE ("ntdll.dll",     NtQuerySystemInformation),
                 ADD_VALUE ("ntdll.dll",     RtlGetVersion),
                 ADD_VALUE ("WanPacket.dll", WanPacketSetBpfFilter),
                 ADD_VALUE ("WanPacket.dll", WanPacketOpenAdapter),
                 ADD_VALUE ("WanPacket.dll", WanPacketCloseAdapter),
                 ADD_VALUE ("WanPacket.dll", WanPacketSetBufferSize),
                 ADD_VALUE ("WanPacket.dll", WanPacketReceivePacket),
                 ADD_VALUE ("WanPacket.dll", WanPacketSetMinToCopy),
                 ADD_VALUE ("WanPacket.dll", WanPacketGetStats),
                 ADD_VALUE ("WanPacket.dll", WanPacketSetReadTimeout),
                 ADD_VALUE ("WanPacket.dll", WanPacketSetMode),
                 ADD_VALUE ("WanPacket.dll", WanPacketGetReadEvent),
                 ADD_VALUE ("WanPacket.dll", WanPacketTestAdapter),
#if defined(HAVE_WINDNS_H)
                 ADD_VALUE ("dnsapi.dll",    DnsQuery_A),
                 ADD_VALUE ("dnsapi.dll",    DnsFree),
                 ADD_VALUE ("dnsapi.dll",    DnsModifyRecordsInSet_A),
#endif
#if defined(USE_BUGTRAP)
                 ADD_VALUE ("BugTrap.dll", BT_InstallSehFilter),
                 ADD_VALUE ("BugTrap.dll", BT_SetPreErrHandler),
                 ADD_VALUE ("BugTrap.dll", BT_SetAppVersion),
                 ADD_VALUE ("BugTrap.dll", BT_SetAppName),
                 ADD_VALUE ("BugTrap.dll", BT_SetReportFormat),
                 ADD_VALUE ("BugTrap.dll", BT_SetSupportEMail),
                 ADD_VALUE ("BugTrap.dll", BT_SetFlags),
                 ADD_VALUE ("BugTrap.dll", BT_SetLogSizeInEntries),
                 ADD_VALUE ("BugTrap.dll", BT_SetLogFlags),
                 ADD_VALUE ("BugTrap.dll", BT_SetLogEchoMode),
                 ADD_VALUE ("BugTrap.dll", BT_InsLogEntryF),
                 ADD_VALUE ("BugTrap.dll", BT_OpenLogFile),
                 ADD_VALUE ("BugTrap.dll", BT_CloseLogFile),
                 ADD_VALUE ("BugTrap.dll", BT_AddLogFile)
#endif
   };

size_t dyn_funcs_num = DIM (dyn_funcs);

/*
 * Handling of dynamic loading and unloading of DLLs and their functions.
 */
int load_dynamic_table (struct LoadTable *tab, int tab_size)
{
  int i;

  for (i = 0; i < tab_size; tab++, i++)
  {
    HINSTANCE mod_handle;

    if (!_watt_use_bugtrap && !stricmp(tab->mod_name,"BugTrap.dll"))
       continue;

    if (i > 0 && !stricmp(tab->mod_name, (tab-1)->mod_name))
         mod_handle = (tab-1)->mod_handle;
    else mod_handle = LoadLibrary (tab->mod_name);

    if (mod_handle && mod_handle != INVALID_HANDLE_VALUE)
    {
      FARPROC addr = GetProcAddress (mod_handle, tab->func_name);

      if (!addr)
         TRACE_CONSOLE (4, "Function \"%s\" not found in %s.\n",
                        tab->func_name, tab->mod_name);
      *tab->func_addr = addr;
    }
    tab->mod_handle = mod_handle;

    TRACE_CONSOLE (4, "%2d: Module 0x%08lX/%s, func \"%s\" -> 0x%" ADDR_FMT ".\n",
                   i, (u_long)HandleToUlong(tab->mod_handle), tab->mod_name, tab->func_name,
                   ADDR_CAST(*tab->func_addr));
  }
  return (i);
}

int unload_dynamic_table (struct LoadTable *tab, int tab_size)
{
  int i;

  for (i = 0; i < tab_size; tab++, i++)
  {
    int m_unload = 0;
    int f_unload = 0;

    if (tab->mod_handle && tab->mod_handle != INVALID_HANDLE_VALUE)
    {
      FreeLibrary (tab->mod_handle);
      m_unload = 1;
    }
    tab->mod_handle = INVALID_HANDLE_VALUE;

    if (*tab->func_addr)
       f_unload = 1;
    *tab->func_addr = NULL;

    TRACE_CONSOLE (4, "%2d: function \"%s\" %s. Module \"%s\" %s.\n",
                   i, tab->func_name, f_unload ? "freed"    : "not used",
                   tab->mod_name,  m_unload ? "unloaded" : "not used");
  }
  return (i);
}
#endif   /* WIN32 */

