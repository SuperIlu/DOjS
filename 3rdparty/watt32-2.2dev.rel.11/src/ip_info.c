#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501

/*
 * MinGW:
 *   gcc -Wall ip_info.c -o ip_info.exe -lws2_32 -liphlpapi
 *
 * MSVC:
 *   cl -nologo -W3 ip_info.c -Feip_info.exe ws2_32.lib iphlpapi.lib
 *
 * OpenWatcom:
 *   wcl386 -q -w3 -bt=nt ip_info.c ws2_32.lib iphlpapi.lib
 */

#include <stdio.h>
#include <malloc.h>
#include <windows.h>
#include <winsock2.h>
#include <iptypes.h>
#include <iphlpapi.h>

#ifndef GAA_FLAG_INCLUDE_ALL_INTERFACES
#define GAA_FLAG_INCLUDE_ALL_INTERFACES  0x0100L
#endif

/*
 * Convert a wide-char string to UTF-8.
 */
static const char *utf8_str (const wchar_t *str)
{
  static char buf[100];
  DWORD len = WideCharToMultiByte (CP_ACP, 0, str, -1, buf, sizeof(buf), NULL, NULL);

  if (len == 0)
     _snprintf (buf, sizeof(buf), "WideCharToMultiByte() failed: %ld", GetLastError());
  return (buf);
}

void print_GetAdaptersAddresses (void)
{
  DWORD rc = 0;
  ULONG family = AF_UNSPEC;
  ULONG flags = GAA_FLAG_INCLUDE_ALL_INTERFACES;
  ULONG outBufLen = 0;
  IP_ADAPTER_ADDRESSES *addr;

  printf ("Information from GetAdaptersAddresses():\n");

  rc = GetAdaptersAddresses (family, flags, NULL, NULL, &outBufLen);
  if (rc != ERROR_BUFFER_OVERFLOW)
     return;

  addr = alloca (outBufLen);
  rc = GetAdaptersAddresses (family, flags, NULL, addr, &outBufLen);
  if (rc != NO_ERROR)
  {
    printf ("\t\tError: %lu\n",rc);
    return;
  }

  while (addr)
  {
    printf ("\tAdapter name: %s\n", addr->AdapterName);
    printf ("\t Description:   %s\n", utf8_str(addr->Description));
    printf ("\t Friendly name: %s\n", utf8_str(addr->FriendlyName));
    addr = addr->Next;
  }
}

int main(void)
{
  print_GetAdaptersAddresses();
  return (0);
}
