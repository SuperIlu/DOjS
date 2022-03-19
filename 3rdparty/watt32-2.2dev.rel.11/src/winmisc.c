/*!\file winmisc.c
 *
 *  Various stuff for Windows only.
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

#include "socket.h"
#include "pcdns.h"
#include "get_xby.h"
#include "cpumodel.h"
#include "run.h"
#include "misc.h"

#if defined(WIN32) || defined(WIN64)    /* rest of file */

#include "packet32.h"
#include "win_dll.h"

#include <w32-fakes/winsock2.h>

#define STATUS_SUCCESS 0

CRITICAL_SECTION _watt_crit_sect;
BOOL             _watt_is_win9x    = FALSE;  /*!< Process is running under Win9x/ME. */
BOOL             _watt_is_wow64    = FALSE;  /*!< Process is running under WOW64. */
BOOL             _watt_use_bugtrap = TRUE;
BOOL             _watt_is_gui_app  = FALSE;

HANDLE stdin_hnd  = INVALID_HANDLE_VALUE;
HANDLE stdout_hnd = INVALID_HANDLE_VALUE;

CONSOLE_SCREEN_BUFFER_INFO console_info;

#if defined(USE_STACKWALKER)
static void (MS_CDECL *orig_abort_handler)(int) = NULL;
#endif

/* WinBase.h (SDK) stuff added for Windows 7.
 */
#ifndef BASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE
#define BASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE 0x1
#endif

#ifndef BASE_SEARCH_PATH_PERMANENT
#define BASE_SEARCH_PATH_PERMANENT 0x8000
#endif

/*
 * 'dns_windns' bits.
 */
#define WINDNS_QUERY_A4     0x0001
#define WINDNS_QUERY_A6     0x0002
#define WINDNS_QUERY_PTR4   0x0004
#define WINDNS_QUERY_PTR6   0x0008
#define WINDNS_CACHEPUT_A4  0x0100
#define WINDNS_CACHEPUT_A6  0x0200

#ifndef DNS_TYPE_AAAA
#define DNS_TYPE_AAAA  0x001C
#endif

#ifndef DNS_ERROR_RESPONSE_CODES_BASE
#define DNS_ERROR_RESPONSE_CODES_BASE  9000
#endif

#ifndef DNS_ERROR_RCODE_LAST
#define DNS_ERROR_RCODE_LAST  9018
#endif

#if defined(__MINGW32__)  /* Missing in <winnt.h>, but is in libkernel32.a */
  __declspec(dllimport) void WINAPI RtlCaptureContext (CONTEXT *ContextRecord);
#endif

/*
 * Various stuff to initialise.
 */
static BOOL get_win_version (WORD *ver, BOOL *is_win9x)
{
  OSVERSIONINFO ovi;
  DWORD os_ver = GetVersion();
  DWORD major_ver = LOBYTE (LOWORD(os_ver));
  BOOL  rc = FALSE;

  *is_win9x = (os_ver >= 0x80000000 && major_ver >= 4);

  memset (&ovi, 0, sizeof(ovi));
  ovi.dwOSVersionInfoSize = sizeof(ovi);

  /* We only support Win-NT style OSes.
   */
  if (!GetVersionEx(&ovi) || ovi.dwPlatformId != VER_PLATFORM_WIN32_NT)
    rc = FALSE;
  else
  {
    *ver = (WORD)(ovi.dwMajorVersion << 8) + (WORD)ovi.dwMinorVersion;
    rc = TRUE;
  }

#if 0
  (*_printf) ("rc: %d, dwMajorVersion: 0x%04X, dwMinorVersion: 0x%04X\n",
              rc, ovi.dwMajorVersion, ovi.dwMinorVersion);
#endif

  return (rc);
}

/*
 * Check if program is a GUI app with no stdout handle.
 */
static BOOL is_gui_app (void)
{
  const IMAGE_DOS_HEADER *dos;
  const IMAGE_NT_HEADERS *nt;
  HMODULE mod = GetModuleHandle (NULL);
  HANDLE  hnd;

  if (!mod)
     return (FALSE);

  /* A GUI app should have no stdout handle.
   */
  hnd = GetStdHandle (STD_OUTPUT_HANDLE);
  if (hnd != INVALID_HANDLE_VALUE && GetFileType(hnd) != FILE_TYPE_UNKNOWN)
     return (FALSE);

  dos = (const IMAGE_DOS_HEADER*) mod;
  nt  = (const IMAGE_NT_HEADERS*) ((const BYTE*)mod + dos->e_lfanew);
  return (nt->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI);
}

/*
 * \todo: make a GUI list-control with "Abort, Show-stack" etc.
 *        mimicking the normal console.
 */
int MS_CDECL gui_printf (const char *fmt, ...)
{
  char    buf[1024];
  int     len;
  char   *prog_name, *s;
  DWORD   mb_flags;
  va_list args;

  va_start (args, fmt);
  len = VSNPRINTF (buf, sizeof(buf), fmt, args);

  s = (char*) get_argv0();
#if 0
  if ((prog_name = strrchr(s,'\\')) == NULL &&
      (prog_name = strrchr(s, ':')) == NULL)
       prog_name = s;
  else prog_name++;
#else
  prog_name = s;  /* Show the full program-name */
#endif

  mb_flags = MB_ICONSTOP | MB_SETFOREGROUND;
  mb_flags |= _watt_is_win9x ? MB_SYSTEMMODAL : MB_TASKMODAL;
  MessageBoxA (NULL, buf, prog_name, mb_flags);

  va_end (args);
  return (len);
}

static void W32_CALL win32_exit (void)
{
  stdin_hnd  = INVALID_HANDLE_VALUE;
  stdout_hnd = INVALID_HANDLE_VALUE;

  unload_dynamic_table (dyn_funcs, dyn_funcs_num);

#if defined(__LCC__)
  DeleteCriticalSection ((struct _CRITICAL_SECTION*)&_watt_crit_sect);
#else
  DeleteCriticalSection (&_watt_crit_sect);
#endif
  _watt_crit_sect.SpinCount = -1;
}

/**
 * Return err-number and string for 'err'. Only use this with GetLastError().
 * (or WSAGetLastError() if Winsock is used somehow ... linked dynamically?).
 * Does not handle libc errno's. Remove trailing [\r\n.]
 */
char * W32_CALL win_strerror (DWORD err)
{
  static char buf[512+20];
  char   err_buf[512], *p;

  if (!FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,
                      LANG_NEUTRAL, err_buf, sizeof(err_buf)-1, NULL))
     strcpy (err_buf, "Unknown error");
  SNPRINTF (buf, sizeof(buf), "%lu/0x%lX: %s", (u_long)err, (u_long)err, err_buf);
  rip (buf);
  p = strrchr (buf, '.');
  if (p && p[1] == '\0')
     *p = '\0';
  return (buf);
}

/**
 * Returns the version of a PE image (.sys, .dll or .exe).
 */
BOOL get_file_version (const char *file_name,
                       char       *version_buf,
                       size_t      version_buf_len)
{
  DWORD  ver_info_size;         /* Size of version information block */
  DWORD  err, ver_hnd = 0;      /* An 'ignored' parameter, always '0' */
  UINT   bytes_read;
  char   sub_block[64];
  void  *res_buf, *vff_info;

  const struct LANG_AND_CODEPAGE {
               WORD language;
               WORD code_page;
            } *lang_info;
  void *lang_info_ptr;

  if (!p_GetFileVersionInfoSizeA || !p_GetFileVersionInfoA || !p_VerQueryValueA)
  {
    CONSOLE_MSG (2, ("Missing functions from version.dll\n"));
    return (FALSE);
  }

  /* Pull out the version information
   */
  ver_info_size = (*p_GetFileVersionInfoSizeA) ((char*)file_name, &ver_hnd);
  CONSOLE_MSG (2, ("file %s, ver-size %lu\n", file_name, (u_long)ver_info_size));

  if (!ver_info_size)
  {
    err = GetLastError();
    CONSOLE_MSG (2, ("failed to call GetFileVersionInfoSizeA; %s\n",
                     win_strerror(err)));
    return (FALSE);
  }

  vff_info = alloca (ver_info_size);

  if (!(*p_GetFileVersionInfoA) ((char*)file_name, ver_hnd,
                                 ver_info_size, vff_info))
  {
    err = GetLastError();
    CONSOLE_MSG (2, ("failed to call GetFileVersionInfoA; %s\n",
                     win_strerror(err)));
    return (FALSE);
  }

  /* Read the list of languages and code pages.
   */
  if (!(*p_VerQueryValueA) (vff_info, "\\VarFileInfo\\Translation",
                            &lang_info_ptr, &bytes_read) ||
      bytes_read < sizeof(*lang_info))
  {
    err = GetLastError();
    CONSOLE_MSG (2, ("failed to call VerQueryValueA; %s\n",
                     win_strerror(err)));
    return (FALSE);
  }

  lang_info = (const struct LANG_AND_CODEPAGE*) lang_info_ptr;

  /* Create the file version string for the first (i.e. the only
   * one) language.
   */
  sprintf (sub_block, "\\StringFileInfo\\%04x%04x\\FileVersion",
           lang_info->language, lang_info->code_page);

  /* Retrieve the file version string for the language. 'res_buf' will
   * point into 'vff_info'. Hence it doesn't have to be freed.
   */
  if (!(*p_VerQueryValueA) (vff_info, sub_block, &res_buf, &bytes_read))
  {
    err = GetLastError();
    CONSOLE_MSG (2, ("failed to call VerQueryValueA; %s\n",
                     win_strerror(err)));
    return (FALSE);
  }

  CONSOLE_MSG (2, ("sub-block '%s' -> '%.*s'\n",
                   sub_block, bytes_read, (const char*)res_buf));

  if (strlen(res_buf) >= version_buf_len)
  {
    CONSOLE_MSG (2, ("GetFileVersionA(): input buffer too small\n"));
    return (FALSE);
  }
  strcpy (version_buf, res_buf);
  return (TRUE);
}


#if defined(USE_STACKWALKER)

#include "stkwalk.h"

static void MS_CDECL new_abort_handler (int sig)
{
  CONTEXT ctx;

  memset (&ctx, 0, sizeof(ctx));
  ctx.ContextFlags = CONTEXT_FULL;
  RtlCaptureContext (&ctx);

  /** \todo: show a better MessageBox(). Show backtrace
   * from where abort() was called; skip the 2 first CRT locations (raise, abort).
   * Indent the printout 2 spaces.
   */
  CONSOLE_MSG (0, ("\nabort() called. Backtrace:\n"));
  ShowStack (GetCurrentThread(), &ctx, NULL);

  if (orig_abort_handler)
    (*orig_abort_handler) (sig);
  _exit (-1);
}
#endif

static void init_win_abort (void)
{
#if defined(USE_STACKWALKER)
  if (!IsDebuggerPresent())
  {
    orig_abort_handler = signal (SIGABRT, new_abort_handler);
#ifdef _MSC_VER
    _set_abort_behavior (0, _WRITE_ABORT_MSG);
#endif
  }
#endif
}

#if defined(USE_BUGTRAP)

static void CALLBACK pre_err_handler (INT_PTR arg)
{
  INT_PTR handle = arg;

  if (!handle)
     return;

  (*p_BT_InsLogEntryF) (handle, BTLL_INFO, _T("Starting Watt-32 crash-report:"));

  if (_watt_assert_buf[0])
     (*p_BT_InsLogEntryF) (handle, BTLL_IMPORTANT, _watt_assert_buf);
  (*p_BT_CloseLogFile) (handle);
}

static int init_win_bugtrap (void)
{
  BUGTRAP_LOGECHOTYPE bt_log_mode = 0;
  INT_PTR             bt_log_handle = 0;

  if (!p_BT_SetAppName || !p_BT_InstallSehFilter ||
      !p_BT_SetFlags || !p_BT_SetLogFlags ||
      !p_BT_SetReportFormat || !p_BT_OpenLogFile ||
      !p_BT_SetLogSizeInEntries || !p_BT_SetLogEchoMode ||
      !p_BT_AddLogFile)
     return (0);

  (*p_BT_SetAppName) ("Watt-32_library");
//(*p_BT_SetSupportEMail) ("gvanem@yahoo.no");

  (*p_BT_InstallSehFilter)();
  (*p_BT_SetFlags) (BTF_DETAILEDMODE | BTF_ATTACHREPORT);
  (*p_BT_SetReportFormat) (BTRF_TEXT);

  bt_log_handle = (*p_BT_OpenLogFile) ("watt-32.dll.log", BTLF_TEXT);
  (*p_BT_SetLogSizeInEntries) (bt_log_handle, 100);
  (*p_BT_SetLogFlags) (bt_log_handle, BTLF_SHOWTIMESTAMP);

  bt_log_mode = BTLE_DBGOUT;
  if (!_watt_is_gui_app)
     bt_log_mode |= BTLE_STDERR;
  (*p_BT_SetLogEchoMode) (bt_log_handle, bt_log_mode);

#if 0
  PCTSTR pszLogFileName = (*p_BT_GetLogFileName) (bt_log_handle);
  (*p_BT_AddLogFile) (pszLogFileName);
#endif

  (*p_BT_SetPreErrHandler) (pre_err_handler, bt_log_handle);

  return (1);
}
#endif  /* USE_BUGTRAP */


/*
 * Called from init_misc() to initialise Win32/win64 specific things.
 */
BOOL init_win_misc (void)
{
  char env[20];
  BOOL wow64 = FALSE;
  BOOL rc = FALSE;

#if defined(__LCC__)
  InitializeCriticalSection ((struct _CRITICAL_SECTION*)&_watt_crit_sect);
#else
  InitializeCriticalSection (&_watt_crit_sect);
#endif

  if ((GetEnvironmentVariableA("WATT32-NOEXC", env, sizeof(env)) ||
       GetEnvironmentVariableA("WATT32-NOEXCEPT", env, sizeof(env))) &&
      env[0] != '0')
     _watt_use_bugtrap = FALSE;

  _watt_is_gui_app = is_gui_app();

  _watt_os_ver = 0x0400;   /* defaults to Win-NT 4.0 */
  get_win_version (&_watt_os_ver, &_watt_is_win9x);

  if (!_watt_is_gui_app)
  {
    stdin_hnd  = GetStdHandle (STD_INPUT_HANDLE);
    stdout_hnd = GetStdHandle (STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo (stdout_hnd, &console_info);
  }

  load_dynamic_table (dyn_funcs, dyn_funcs_num);

#if !defined(WIN64)
  if (p_IsWow64Process)
  {
     rc = (*p_IsWow64Process) (GetCurrentProcess(), &wow64);
     if (rc)
       _watt_is_wow64 = wow64;
  }
#endif

  CONSOLE_MSG (2, ("IsWow64Process(): rc: %d, wow64: %d.\n",
                   rc, wow64));

  if (_watt_use_bugtrap)
  {
#if defined(USE_BUGTRAP)
    init_win_bugtrap();
#endif
    init_win_abort();
  }

  RUNDOWN_ADD (win32_exit, 310);
  ARGSUSED (rc);
  return (TRUE);
}

#if defined(HAVE_WINDNS_H)
/*
 * Perform a query on the WinDns cache. We ask only for
 * 'type'; An address (A/AAAA records) or a name (PTR record).
 */
static BOOL WinDnsQueryCommon (WORD type, const void *what,
                               void *result, size_t size)
{
  DNS_RECORD *rr, *dr = NULL;
  DNS_STATUS  rc;
  BOOL   found;
  DWORD  opt = DNS_QUERY_NO_WIRE_QUERY | /* query the cache only */
               DNS_QUERY_NO_NETBT |      /* no NetBT names */
               DNS_QUERY_NO_HOSTS_FILE;  /* no Winsock hosts file */

  from_windns = FALSE;

  if (!p_DnsQuery_A || !p_DnsFree)
     return (FALSE);

  rc = (*p_DnsQuery_A) ((const char*)what, type, opt, NULL, &dr, NULL);

  CONSOLE_MSG (2, ("DnsQuery_A: type %d, dr %p: %s\n",
                   type, dr, win_strerror(rc)));

  if (rc != ERROR_SUCCESS || !dr)
     return (FALSE);

  dom_ttl = dr->dwTtl;
  found = FALSE;

  for (rr = dr; rr; rr = rr->pNext)
  {
    CONSOLE_MSG (2, ("RR-type: %d: ", rr->wType));

    /* Use only 1st A/AAAA record
     */
    if (rr->wType == DNS_TYPE_A && type == DNS_TYPE_A)
    {
      DWORD ip = ntohl (rr->Data.A.IpAddress);

      CONSOLE_MSG (2, ("A: %s, ttl %lus\n",
                       _inet_ntoa(NULL,ip), (u_long)rr->dwTtl));
      if (!found)
         *(DWORD*) result = ip;
      found = TRUE;
    }

#if defined(USE_BSD_API) || defined(USE_IPV6)
    else if (rr->wType == DNS_TYPE_AAAA && type == DNS_TYPE_AAAA)
    {
      const void *ip6 = &rr->Data.AAAA.Ip6Address.IP6Dword[0];

      CONSOLE_MSG (2, ("AAAA: %s\n", _inet6_ntoa(ip6)));
      if (!found)
         memcpy (result, ip6, size);
      found = TRUE;
    }
#endif

    else if (rr->wType == DNS_TYPE_PTR && type == DNS_TYPE_PTR)
    {
      _tcsncpy (result, dr->Data.PTR.pNameHost, size);
      CONSOLE_MSG (2, ("PTR: %" TSTR2ASCII_FMT "\n", (const TCHAR*)result));
    }
    else if (rr->wType == DNS_TYPE_CNAME)
    {
#ifdef UNICODE
       const char *src = wstring_acp (dr->Data.CNAME.pNameHost);
#else
       const char *src = dr->Data.CNAME.pNameHost;
#endif
      _strlcpy (dom_cname, src, sizeof(dom_cname));
      CONSOLE_MSG (2, ("CNAME: %s\n", dom_cname));
    }
    else
      CONSOLE_MSG (2, ("\n"));
  }
  (*p_DnsFree) (dr, DnsFreeRecordList);
  from_windns = found;
  return (TRUE);
}
#endif  /* HAVE_WINDNS_H */


BOOL WinDnsQuery_A4 (const char *name, DWORD *ip)
{
#if defined(HAVE_WINDNS_H)
  if (!(dns_windns & WINDNS_QUERY_A4))
     return (FALSE);
  return WinDnsQueryCommon (DNS_TYPE_A, name, ip, sizeof(*ip));
#else
  ARGSUSED (name);
  ARGSUSED (ip);
  return (FALSE);
#endif
}

BOOL WinDnsQuery_A6 (const char *name, void *ip)
{
#if defined(HAVE_WINDNS_H)
  if (!(dns_windns & WINDNS_QUERY_A6))
     return (FALSE);
  return WinDnsQueryCommon (DNS_TYPE_AAAA, name, ip, sizeof(ip6_address));
#else
  ARGSUSED (name);
  ARGSUSED (ip);
  return (FALSE);
#endif
}

BOOL WinDnsQuery_PTR4 (DWORD ip, TCHAR *name, size_t size)
{
#if defined(HAVE_WINDNS_H)
  if (!(dns_windns & WINDNS_QUERY_PTR4))
     return (FALSE);
  return WinDnsQueryCommon (DNS_TYPE_PTR, &ip, name, size);
#else
  ARGSUSED (ip);
  ARGSUSED (name);
  ARGSUSED (size);
  return (FALSE);
#endif
}

BOOL WinDnsQuery_PTR6 (const void *ip, TCHAR *name, size_t size)
{
#if defined(HAVE_WINDNS_H)
  if (!(dns_windns & WINDNS_QUERY_PTR6))
     return (FALSE);
  return WinDnsQueryCommon (DNS_TYPE_PTR, &ip, name, size);
#else
  ARGSUSED (ip);
  ARGSUSED (name);
  ARGSUSED (size);
  return (FALSE);
#endif
}

/*
 * This doesn't seem to simply put a name/IP pair in the
 * local cache, but do a complete registration with the
 * Winsock registered DNS server(s). Hence off by default.
 */
BOOL WinDnsCachePut_A4 (const char *name, DWORD ip4)
{
#if defined(HAVE_WINDNS_H)
  DNS_RECORD rr;
  DNS_STATUS rc;
  DWORD      opt = DNS_UPDATE_SECURITY_OFF |
                   DNS_UPDATE_CACHE_SECURITY_CONTEXT;

  if (!p_DnsModifyRecordsInSet_A ||
      !(dns_windns & WINDNS_CACHEPUT_A4))
     return (FALSE);

  memset (&rr, 0, sizeof(rr));

#ifdef UNICODE
  rr.pName = _tcsdup (astring_acp(name));
#else
  rr.pName = strdup (name);
#endif

  rr.wType = DNS_TYPE_A;
  rr.Data.A.IpAddress = htonl (ip4);
  rr.wDataLength      = sizeof(rr.Data.A);
#ifdef USE_BSD_API
  rr.dwTtl = netdbCacheLife;
#else
  rr.dwTtl = MAX_CACHE_LIFE;
#endif

  rc = (*p_DnsModifyRecordsInSet_A) (&rr, NULL, opt,
                                     NULL, NULL, NULL);
  DO_FREE (rr.pName);

  CONSOLE_MSG (2, ("DnsModifyRecordsInSet_A: %s ", win_strerror(rc)));

  if (rc >= DNS_ERROR_RESPONSE_CODES_BASE && rc <= DNS_ERROR_RCODE_LAST)
     dns_windns &= ~WINDNS_CACHEPUT_A4;  /* don't do this again */
  return (rc == ERROR_SUCCESS);
#else
  ARGSUSED (name);
  ARGSUSED (ip4);
  return (FALSE);
#endif  /* HAVE_WINDNS_H */
}

BOOL WinDnsCachePut_A6 (const char *name, const void *ip6)
{
  if (!(dns_windns & WINDNS_CACHEPUT_A6))
     return (FALSE);
  ARGSUSED (name);
  ARGSUSED (ip6);
  return (FALSE);
}


int __stdcall WSAStartup (WORD version_required, WSADATA *wsa_data)
{
  WORD max_ver = MAKEWORD (2,2);
  int  rc;

  if (version_required > max_ver)
     return (EINVAL);  /* we don't have WSAVERNOTSUPPORTED */

  _watt_do_exit = 0;
  rc = watt_sock_init (0, 0, 0);

  wsa_data->wVersion     = version_required;
  wsa_data->wHighVersion = 2;
  wsa_data->iMaxSockets  = MAX_SOCKETS;
  wsa_data->iMaxUdpDg    = MAX_SOCKETS;
  wsa_data->lpVendorInfo = NULL;
  wsa_data->szSystemStatus[0] = '\0';
  strcpy (wsa_data->szDescription, "Watt-32 tcp/ip");
  return (rc);
}

int __stdcall WSACleanup (void)
{
  sock_exit();
  return (0);
}

int __stdcall __WSAFDIsSet (int s, winsock_fd_set *fd)
{
  UNFINISHED();
  ARGSUSED (s);
  ARGSUSED (fd);
  return (0);
}

#ifdef NOT_USED
static BOOL CALLBACK callback (LPSTR cp)
{
  printf ("\t callback(): cp: \"%s\", valid: %d\n",
          cp, IsValidCodePage(atoi(cp)));
  return (TRUE);
}

static void enum_codepages (void)
{
  puts ("\nEnumerating codepages");
  EnumSystemCodePages (callback, CP_INSTALLED);

  if (IsValidCodePage(65001))
       SetConsoleCP (65001);
  else puts ("CP=65001 (UTF-8) is not valid");
}
#endif  /* NOT_USED */


#if defined(_MSC_VER) && (_MSC_VER >= 1300) && defined(NOT_USED)
__declspec(naked) unsigned __int64 _ftol2 (double x)
{
  _asm {
    push        ebp
    mov         ebp,esp
    sub         esp,0x00000020
    and         esp,0xfffffff0
    fld         st
    fst         dword ptr 0x18[esp]
    fistp       qword ptr 0x10[esp]
    fild        qword ptr 0x10[esp]
    mov         edx,dword ptr 0x18[esp]
    mov         eax,dword ptr 0x10[esp]
    test        eax,eax
    je          integer_QnaN_or_zero
arg_is_not_integer_QnaN:
    fsubp       st(1),st
    test        edx,edx
    jns         positive
    fstp        dword ptr [esp]
    mov         ecx,dword ptr [esp]
    xor         ecx,0x80000000
    add         ecx,0x7fffffff
    adc         eax,0x00000000
    mov         edx,dword ptr 0x14[esp]
    adc         edx,0x00000000
    jmp         localexit
positive:
    fstp        dword ptr [esp]
    mov         ecx,dword ptr [esp]
    add         ecx,0x7fffffff
    sbb         eax,0x00000000
    mov         edx,dword ptr 0x14[esp]
    sbb         edx,0x00000000
    jmp         localexit
integer_QnaN_or_zero:
    mov         edx,dword ptr 0x14[esp]
    test        edx,0x7fffffff
    jne         arg_is_not_integer_QnaN
    fstp        dword ptr 0x18[esp]
    fstp        dword ptr 0x18[esp]
localexit:
    leave
    ret
  }
}

__declspec(naked) unsigned __int64 _aulldvrm (void)
{
  _asm {
    push        esi
    mov         eax,dword ptr 0x14[esp]
    or          eax,eax
    jne         L1
    mov         ecx,dword ptr 0x10[esp]
    mov         eax,dword ptr 0xc[esp]
    xor         edx,edx
    div         ecx
    mov         ebx,eax
    mov         eax,dword ptr 0x8[esp]
    div         ecx
    mov         esi,eax
    mov         eax,ebx
    mul         dword ptr 0x10[esp]
    mov         ecx,eax
    mov         eax,esi
    mul         dword ptr 0x10[esp]
    add         edx,ecx
    jmp         L2
L1:
    mov         ecx,eax
    mov         ebx,dword ptr 0x10[esp]
    mov         edx,dword ptr 0xc[esp]
    mov         eax,dword ptr 0x8[esp]
L3:
    shr         ecx,0x01
    rcr         ebx,0x01
    shr         edx,0x01
    rcr         eax,0x01
    or          ecx,ecx
    jne         L3
    div         ebx
    mov         esi,eax
    mul         dword ptr 0x14[esp]
    mov         ecx,eax
    mov         eax,dword ptr 0x10[esp]
    mul         esi
    add         edx,ecx
    jb          L4
    cmp         edx,dword ptr 0xc[esp]
    ja          L4
    jb          L5
    cmp         eax,dword ptr 0x8[esp]
    jbe         L5
L4:
    dec         esi
    sub         eax,dword ptr 0x10[esp]
    sbb         edx,dword ptr 0x14[esp]
L5:
    xor         ebx,ebx
L2:
    sub         eax,dword ptr 0x8[esp]
    sbb         edx,dword ptr 0xc[esp]
    neg         edx
    neg         eax
    sbb         edx,0x00000000
    mov         ecx,edx
    mov         edx,ebx
    mov         ebx,ecx
    mov         ecx,eax
    mov         eax,esi
    pop         esi
    ret         0x0010
  }
}
#endif  /* _MSC_VER && NOT_USED */

/*
 * From the Open Watcom Project
 *
 *  Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
 */
static int is_real_key (const INPUT_RECORD *k)
{
  if (k->EventType != KEY_EVENT)
     return (0);

  if (k->Event.KeyEvent.bKeyDown)
  {
    switch (k->Event.KeyEvent.wVirtualKeyCode)
    {
      case VK_SHIFT:
      case VK_CONTROL:
      case VK_MENU:       /* Alt */
           return(0);
    }
    return (1);
  }
  return (0);
}


/*
 * CygWin doesn't even have <conio.h>. Let alone a simple kbhit()
 * and getch(). Hence we make them for all Win32 targets here
 * (but prefixed with '_w32_').
 */
int W32_NAMESPACE(kbhit) (void)
{
  INPUT_RECORD r;
  DWORD num;

  if (stdin_hnd == INVALID_HANDLE_VALUE)
     return (0);

  while (1)
  {
    PeekConsoleInput (stdin_hnd, &r, 1, &num);
    if (num == 0)
       break;
    if (is_real_key(&r))
       break;
    ReadConsoleInput (stdin_hnd, &r, 1, &num); /* flush out mouse, window, and key up events */
  }
  return (num);
}

static int do_getch (HANDLE h)
{
  while (1)
  {
    INPUT_RECORD ir;
    DWORD num;
    int   ch;

    if (!ReadConsoleInput(h, &ir, 1, &num))
       break;
    if (!is_real_key(&ir))
       continue;
    ch = ir.Event.KeyEvent.uChar.AsciiChar;
    if ((ir.Event.KeyEvent.dwControlKeyState & ENHANCED_KEY) || ch == 0)
       ch = 0;
    return (ch);
  }
  return (EOF);
}

int W32_NAMESPACE(getch) (void)
{
  DWORD mode;
  int   c;

  if (stdin_hnd == INVALID_HANDLE_VALUE)
     return (0);

  GetConsoleMode (stdin_hnd, &mode);
  if (!tcp_cbreak_off())               /* if not ignoring ^C */
     SetConsoleMode (stdin_hnd, 0);    /* binary; ignores ^C */
  c = do_getch (stdin_hnd);
  SetConsoleMode (stdin_hnd, mode);
  return (c);
}

/*
 * itoa(): convert 'value' and put into 'buf'
 */
#if 0

char *W32_NAMESPACE(itoa) (int value, char *buf, int radix)
{
  WATT_ASSERT (radix == 8 || radix == 10);
  sprintf (buf, (radix == 8) ? "%o" : "%d", value);
  return (buf);
}

#else

/* This version should be a lot faster. Ref:
 * http://stackoverflow.com/questions/190229/where-is-the-itoa-function-in-linux
 */
char *W32_NAMESPACE(itoa) (int value, char *buf, int radix)
{
  int sign, i = 0;

  WATT_ASSERT (radix == 10);
  sign = value;

  if (sign < 0)      /* record sign */
     value = -value; /* make 'value' positive */

  do                 /* generate digits in reverse order */
  {
    buf[i++] = (value % 10) + '0';
  }
  while ((value /= 10) > 0);

  if (sign < 0)
     buf [i++] = '-';
  buf [i] = '\0';
  return strreverse (buf);
}
#endif  /* 0 */


/*
 * Return FILETIME in seconds as a double.
 */
static double filetime_sec (const FILETIME *filetime)
{
  const LARGE_INTEGER *ft = (const LARGE_INTEGER*) filetime;
  long double          rc = (long double) ft->QuadPart;

  return (double) (rc/1E7);    /* from 100 nano-sec periods to sec */
}

/*
 * Print some times (and CPU cycle counts) for a thread.
 * I.e. the WinPcap receiver thread.
 */
void print_thread_times (HANDLE thread)
{
  FILETIME ctime, etime, ktime, utime;

  if (!GetThreadTimes(thread, &ctime, &etime, &ktime, &utime))
  {
    DWORD err = GetLastError();
    CONSOLE_MSG (2, ("  GetThreadTimes() %s, ", win_strerror(err)));
  }
  CONSOLE_MSG (2, ("  kernel-time: %.6fs, user-time: %.6fs, life-span: %.6fs",
               filetime_sec(&ktime), filetime_sec(&utime),
               filetime_sec(&etime) - filetime_sec(&ctime)));

  if (p_QueryThreadCycleTime)
  {
    ULONG64 cycle_time;
    if (!(*p_QueryThreadCycleTime) (thread, &cycle_time))
         CONSOLE_MSG (2, (", cycle-time: <failed>"));
    else CONSOLE_MSG (2, (", cycle-time: %s clocks", qword_str(cycle_time)));
  }

  if (p_NtQueryInformationThread)
  {
    LARGE_INTEGER perf_count;
    NTSTATUS rc = (*p_NtQueryInformationThread) (thread, ThreadPerformanceCount,
                                                 &perf_count, sizeof(perf_count),
                                                 NULL);
    if (rc != STATUS_SUCCESS)
         CONSOLE_MSG (2, (", perf-count: <fail %ld>", (long)rc));
    else CONSOLE_MSG (2, (", perf-count: %s", qword_str(perf_count.QuadPart)));
  }
  CONSOLE_MSG (2, ("\n"));
}

/*
 * Print some times for a process.
 */
void print_process_times (void)
{
#if 0 /* \todo */
  HANDLE proc = OpenProcess (PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                             FALSE, GetCurrentProcessId());

  FILETIME                 ctime, etime, ktime, utime;
  PERFORMANCE_INFORMATION  perf_info;
  PROCESS_MEMORY_COUNTERS  ctrs;

  if (GetProcessTimes(proc, &ctime, &etime.ft, &ktime, &utime))
  {
    /* The units returned by GetProcessTimes are 100 nanoseconds */
    u_time.lt = (u_time.lt + 5) / 10;
    s_time.lt = (s_time.lt + 5) / 10;

    usage->ru_utime.tv_sec  = (long)(u_time.lt / 1000000ll);
    usage->ru_stime.tv_sec  = (long)(s_time.lt / 1000000ll);
    usage->ru_utime.tv_usec = (long)(u_time.lt % 1000000ll);
    usage->ru_stime.tv_usec = (long)(s_time.lt % 1000000ll);

    if (GetProcessMemoryInfo(proc, &ctrs, sizeof(ctrs)))
    {
      GetPerformanceInfo (&perf_info, sizeof(perf_info));
      usage->ru_maxrss = (DWORD) (ctrs.WorkingSetSize / perf_info.PageSize);
      usage->ru_majflt = ctrs.PageFaultCount;
    }
  }
  CloseHandle (proc);
#endif
}

/*
 * Taken from NetPerf's netcpu_ntperf.c:
 *   http://www.netperf.org/netperf
 *
 * System CPU time information class.
 * Used to get CPU time information.
 *
 *     SDK\inc\ntexapi.h
 * Function x8:   SystemProcessorPerformanceInformation
 * DataStructure: SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION
 */
#define SystemProcessorPerformanceInformation 0x08

typedef struct {
        LARGE_INTEGER  IdleTime;
        LARGE_INTEGER  KernelTime;
        LARGE_INTEGER  UserTime;
        LARGE_INTEGER  DpcTime;
        LARGE_INTEGER  InterruptTime;
        LONG           InterruptCount;
      } SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;

#define MAX_CPUS 256

/* \todo:
 *   Store the counters before and after to get the delta-times.
 */
void print_perf_times (void)
{
  SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION info [MAX_CPUS];
  DWORD     i, ret_len, ret_num_CPUs;
  NTSTATUS  rc;

  if (!p_NtQuerySystemInformation || num_cpus == 0)
  {
    CONSOLE_MSG (2, ("  p_NtQuerySystemInformation = NULL!\n"));
    return;
  }

  /* Get the current CPUTIME information.
   */
  rc = (*p_NtQuerySystemInformation) (SystemProcessorPerformanceInformation,
                                      &info, sizeof(info), &ret_len);
  if (rc != 0)
  {
    DWORD err = GetLastError();
    CONSOLE_MSG (2, ("  NtQuerySystemInformation() %s, ", win_strerror(err)));
    return;
  }

  /* Validate that NtQuery returned a reasonable amount of data
   */
  if ((ret_len % sizeof(info[0])) != 0)
  {
    CONSOLE_MSG (1, ("NtQuery didn't return expected amount of data\n"
                     "Expected a multiple of %u, returned %lu\n",
                     SIZEOF(info[0]), (u_long)ret_len) );
    return;
  }

  ret_num_CPUs = ret_len / sizeof(info[0]);
  if (ret_num_CPUs != num_cpus)
  {
    CONSOLE_MSG (1, ("NtQuery didn't return expected amount of data\n"
                     "Expected data for %i CPUs, returned %lu\n",
                     num_cpus, (u_long)ret_num_CPUs) );
    return;
  }

  /* Print total all of the CPUs:
   *      KernelTime needs to be fixed-up; it includes both idle & true kernel time.
   */
  for (i = 0; i < ret_num_CPUs; i++)
  {
    ULONG64 x;

    CONSOLE_MSG (2, ("CPU %lu:%s", (u_long)i, (i == 0) ? "\t\t\t  CPU clocks\n" : "\n"));

    x = info[i].KernelTime.QuadPart - info[i].IdleTime.QuadPart;
    CONSOLE_MSG (2, ("  KernelTime:     %18s\n", qword_str(x)));

    x = info[i].IdleTime.QuadPart;
    CONSOLE_MSG (2, ("  IdleTime:       %18s\n", qword_str(x)));

    x = info[i].UserTime.QuadPart;
    CONSOLE_MSG (2, ("  UserTime:       %18s\n", qword_str(x)));

    x = info[i].DpcTime.QuadPart;
    CONSOLE_MSG (2, ("  DpcTime:        %18s\n", qword_str(x)));

    x = info[i].InterruptTime.QuadPart;
    CONSOLE_MSG (2, ("  InterruptTime:  %18s\n", qword_str(x)));
    CONSOLE_MSG (2, ("  InterruptCount: %18s\n", dword_str(info[i].InterruptCount)));
  }
}
#endif  /* WIN32 || WIN64 */

