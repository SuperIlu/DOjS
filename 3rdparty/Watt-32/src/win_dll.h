/*!\file win_dll.h
 *
 * Loading Windows modules dynamically.
 */
#ifndef _w32_WIN_DLL_H
#define _w32_WIN_DLL_H

#if defined(WIN32)  /* Rest of file */

/* Generic table for loading DLLs and functions from them.
 */
struct LoadTable {
       HINSTANCE    mod_handle;
       const TCHAR *mod_name;
       const char  *func_name;
       void       **func_addr;
     };

extern struct LoadTable dyn_funcs[];
extern size_t           dyn_funcs_num;

extern int load_dynamic_table   (struct LoadTable *tab, int tab_size);
extern int unload_dynamic_table (struct LoadTable *tab, int tab_size);

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif

/*
 * Instead of including <NtDDK.h> here, we define undocumented stuff
 * needed for NtQueryInformationThread() here:
 */
typedef LONG NTSTATUS;

typedef enum _THREADINFOCLASS {
              ThreadBasicInformation,
              ThreadTimes,
              ThreadPriority,
              ThreadBasePriority,
              ThreadAffinityMask,
              ThreadImpersonationToken,
              ThreadDescriptorTableEntry,
              ThreadEnableAlignmentFaultFixup,
              ThreadEventPair_Reusable,
              ThreadQuerySetWin32StartAddress,
              ThreadZeroTlsCell,
              ThreadPerformanceCount,
              ThreadAmILastThread,
              ThreadIdealProcessor,
              ThreadPriorityBoost,
              ThreadSetTlsArrayAddress,
              ThreadIsIoPending,
              MaxThreadInfoClass
            } THREADINFOCLASS;

/* end <NTddk.h> stuff */

/*
 * All function typedefs and function pointers are declared below
 * so that load_dynamic_table() can see the function pointers.
 */

typedef BOOL  (WINAPI *func_QueryThreadCycleTime) (
        IN     HANDLE   thread_handle,
        OUT    ULONG64 *cycle_time);

typedef NTSTATUS (WINAPI *func_NtQueryInformationThread) (
        IN        HANDLE           thread_handle,
        IN        THREADINFOCLASS  thread_information_class,
        IN OUT    void            *thread_information,
        IN        ULONG            thread_information_length,
        OUT       ULONG           *return_length  OPTIONAL);

typedef NTSTATUS (WINAPI *func_NtQuerySystemInformation) (
        IN  ULONG  system_information_class,
        OUT void  *system_information,
        IN  ULONG  system_information_length,
        OUT ULONG *return_length);

typedef NTSTATUS (WINAPI *func_RtlGetVersion) (
        OUT OSVERSIONINFOW *ver_info);


/*
 * MSDN forgot to tell us what values are in the 'THREAD_INFORMATION_CLASS'
 * enum type. According [1], the only suported value is 'ThreadMemoryPriority'.
 * Will figure out this later.
 *
 * When using 'p_GetThreadInformation()', cast 2nd parameter
 * [1] http://msdn.microsoft.com/en-us/library/windows/desktop/hh448382(v=vs.85).aspx
 */
typedef BOOL (WINAPI *func_GetThreadInformation) (
        IN        HANDLE   thread_handle,
        IN        int      thread_information_class,  /* Really is 'THREAD_INFORMATION_CLASS' */
        IN OUT    void    *thread_information,
        IN        DWORD    thread_information_length);

typedef DWORD (WINAPI *func_GetFileVersionInfoSizeA) (char *, DWORD*);
typedef BOOL  (WINAPI *func_GetFileVersionInfoA) (char *, DWORD, DWORD, void *);
typedef BOOL  (WINAPI *func_GetFileVersionInfoExA) (DWORD, LPCTSTR, DWORD, DWORD, void*);
typedef BOOL  (WINAPI *func_VerQueryValueA) (const void **, char *, void **, UINT *);
typedef BOOL  (WINAPI *func_GetComputerNameExA) (int, char*, DWORD*);
typedef BOOL  (WINAPI *func_IsWow64Process) (HANDLE, BOOL*);
typedef void  (WINAPI *func_GetSystemTimePreciseAsFileTime) (FILETIME *);

extern func_GetFileVersionInfoSizeA        p_GetFileVersionInfoSizeA;
extern func_GetFileVersionInfoA            p_GetFileVersionInfoA;
extern func_GetFileVersionInfoExA          p_GetFileVersionInfoExA;
extern func_VerQueryValueA                 p_VerQueryValueA;
extern func_QueryThreadCycleTime           p_QueryThreadCycleTime;
extern func_NtQueryInformationThread       p_NtQueryInformationThread;
extern func_NtQuerySystemInformation       p_NtQuerySystemInformation;
extern func_RtlGetVersion                  p_RtlGetVersion;
extern func_GetComputerNameExA             p_GetComputerNameExA;
extern func_IsWow64Process                 p_IsWow64Process;
extern func_GetSystemTimePreciseAsFileTime p_GetSystemTimePreciseAsFileTime;

#if defined(HAVE_WINDNS_H)
  /*
   * DnsApi.dll is used dynamically. So we need the function pointers below.
   * <windns.h> should be part of the "Windows Platform SDK".
   */
  #include <windns.h>

  typedef DNS_STATUS (WINAPI *func_DnsQuery_A) (
          IN     const char  *name,
          IN     WORD         wType,
          IN     DWORD        options,
          IN     IP4_ARRAY   *aipServers,
          IN OUT DNS_RECORD **queryResults,
          IN OUT void       **reserved);

  typedef DNS_STATUS (WINAPI *func_DnsModifyRecordsInSet_A) (
          IN     DNS_RECORD  *addRecords,
          IN     DNS_RECORD  *deleteRecords,
          IN     DWORD        options,
          IN     HANDLE       context,    OPTIONAL
          IN     IP4_ARRAY   *serverList, OPTIONAL
          IN     void        *reserved);

  typedef void (WINAPI *func_DnsFree) (
          IN OUT void         *data,
          IN     DNS_FREE_TYPE freeType);

  extern func_DnsFree                 p_DnsFree;
  extern func_DnsQuery_A              p_DnsQuery_A;
  extern func_DnsModifyRecordsInSet_A p_DnsModifyRecordsInSet_A;
#endif

#if 1
  #include "packet32.h"
#else
 typedef void WAN_ADAPTER;  /* The details of this struct is not important to us.
                             * Ref. WAN_ADAPTER_INT in WanPacket.cpp (from CACE Technologies)
                             */
  struct bpf_stat;          /* Forward. Defined in packet32.h */
#endif

/* "func_": WanPacket-function typedefs.
 * These are *not* WINAPI, but cdecl.
 */
typedef BOOL         (cdecl *func_WanPacketSetBpfFilter) (WAN_ADAPTER *, PUCHAR, DWORD);
typedef WAN_ADAPTER *(cdecl *func_WanPacketOpenAdapter) (void);
typedef BOOL         (cdecl *func_WanPacketCloseAdapter) (WAN_ADAPTER *);
typedef BOOL         (cdecl *func_WanPacketSetBufferSize)(WAN_ADAPTER *, DWORD);
typedef DWORD        (cdecl *func_WanPacketReceivePacket) (WAN_ADAPTER *, PUCHAR, DWORD);
typedef BOOL         (cdecl *func_WanPacketSetMinToCopy) (WAN_ADAPTER *, DWORD);
typedef BOOL         (cdecl *func_WanPacketGetStats) (WAN_ADAPTER *, struct bpf_stat *);
typedef BOOL         (cdecl *func_WanPacketSetReadTimeout) (WAN_ADAPTER *, DWORD);
typedef BOOL         (cdecl *func_WanPacketSetMode) (WAN_ADAPTER *, DWORD);
typedef HANDLE       (cdecl *func_WanPacketGetReadEvent) (WAN_ADAPTER *);
typedef BOOL         (cdecl *func_WanPacketTestAdapter) (void);

extern func_WanPacketSetBpfFilter   p_WanPacketSetBpfFilter;
extern func_WanPacketOpenAdapter    p_WanPacketOpenAdapter;
extern func_WanPacketCloseAdapter   p_WanPacketCloseAdapter;
extern func_WanPacketSetBufferSize  p_WanPacketSetBufferSize;
extern func_WanPacketReceivePacket  p_WanPacketReceivePacket;
extern func_WanPacketSetMinToCopy   p_WanPacketSetMinToCopy;
extern func_WanPacketGetStats       p_WanPacketGetStats;
extern func_WanPacketSetReadTimeout p_WanPacketSetReadTimeout;
extern func_WanPacketSetMode        p_WanPacketSetMode;
extern func_WanPacketGetReadEvent   p_WanPacketGetReadEvent;
extern func_WanPacketTestAdapter    p_WanPacketTestAdapter;

#if defined(USE_BUGTRAP)
  /*
   * Dynamically load the BugTrap DLL.
   * Ref: http://www.intellesoft.net/default.shtml and/or
   *      https://github.com/bchavez/BugTrap
   *
   * The 1st URL seems broken now.
   * 'APIENTRY' is '__stdcall'
   */
  #include "bugtrap.h"

  #define EXC_FUN  LPTOP_LEVEL_EXCEPTION_FILTER

  typedef EXC_FUN (APIENTRY *func_BT_InstallSehFilter)    (void);
  typedef void    (APIENTRY *func_BT_SetPreErrHandler)    (BT_ErrHandler, INT_PTR);
  typedef void    (APIENTRY *func_BT_SetAppVersion)       (const char *appVersion);
  typedef void    (APIENTRY *func_BT_SetAppName)          (const char *appName);
  typedef void    (APIENTRY *func_BT_SetReportFormat)     (BUGTRAP_REPORTFORMAT format);
  typedef void    (APIENTRY *func_BT_SetSupportEMail)     (const char *supportEmail);
  typedef INT_PTR (APIENTRY *func_BT_OpenLogFile)         (const char *logFileName, BUGTRAP_LOGFORMAT logFormat);
  typedef void    (APIENTRY *func_BT_SetFlags)            (DWORD dwFlags);
  typedef BOOL    (APIENTRY *func_BT_SetLogSizeInEntries) (INT_PTR handle, DWORD logSizeInEntries);
  typedef BOOL    (APIENTRY *func_BT_SetLogFlags)         (INT_PTR handle, DWORD flags);
  typedef BOOL    (APIENTRY *func_BT_SetLogEchoMode)      (INT_PTR handle, DWORD logEchoMode);
  typedef BOOL    (APIENTRY *func_BT_InsLogEntryF)        (INT_PTR handle, BUGTRAP_LOGLEVEL logLevel, LPCTSTR format, ...);
  typedef BOOL    (APIENTRY *func_BT_CloseLogFile)        (INT_PTR handle);
  typedef void    (APIENTRY *func_BT_AddLogFile)          (const char *logFile);

  extern func_BT_InstallSehFilter    p_BT_InstallSehFilter;
  extern func_BT_SetPreErrHandler    p_BT_SetPreErrHandler;
  extern func_BT_SetAppVersion       p_BT_SetAppVersion;
  extern func_BT_SetAppName          p_BT_SetAppName;
  extern func_BT_SetReportFormat     p_BT_SetReportFormat;
  extern func_BT_SetSupportEMail     p_BT_SetSupportEMail;
  extern func_BT_SetFlags            p_BT_SetFlags;
  extern func_BT_SetLogSizeInEntries p_BT_SetLogSizeInEntries;
  extern func_BT_SetLogFlags         p_BT_SetLogFlags;
  extern func_BT_SetLogEchoMode      p_BT_SetLogEchoMode;
  extern func_BT_InsLogEntryF        p_BT_InsLogEntryF;
  extern func_BT_OpenLogFile         p_BT_OpenLogFile;
  extern func_BT_CloseLogFile        p_BT_CloseLogFile;
  extern func_BT_AddLogFile          p_BT_AddLogFile;

#endif  /* USE_BUGTRAP */
#endif  /* WIN32 */
#endif  /* _w32_WIN_DLL_H */

