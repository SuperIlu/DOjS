#ifndef _WATT32_FAKE_WINSOCK2_H
#define _WATT32_FAKE_WINSOCK2_H

#ifndef __SYS_W32API_H
#include <sys/w32api.h>
#endif

#ifndef __SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifndef __SYS_WERRNO_H
#include <sys/werrno.h>
#endif

#if !defined(WIN32) && !defined(_WIN32)
#error This file is only for Watt-32 targeting Windows programs.
#endif

#if !defined(_WATT32_FAKE_WINSOCK_H)

typedef struct _SOCKET_ADDRESS {
        struct sockaddr lpSockaddr;
        int             iSockaddrLength;
      } SOCKET_ADDRESS, *PSOCKET_ADDRESS, *LPSOCKET_ADDRESS;

#define WSADESCRIPTION_LEN  256
#define WSASYS_STATUS_LEN   128

typedef struct WSAData {
        unsigned short wVersion;
        unsigned short wHighVersion;
        char           szDescription[WSADESCRIPTION_LEN+1];
        char           szSystemStatus[WSASYS_STATUS_LEN+1];
        unsigned short iMaxSockets;
        unsigned short iMaxUdpDg;
        char          *lpVendorInfo;
      } WSADATA, *LPWSADATA;

W32_FUNC int __stdcall WSAStartup (unsigned short wVersionRequired,
                                   WSADATA *WSAData);

W32_FUNC int __stdcall WSACleanup (void);

#ifndef FD_SETSIZE
#define FD_SETSIZE  64
#endif

/*
 * Needed if user compiled with the normal <winsock*.h> and just relinked
 * with Watt-32 (import) library.
 */
typedef struct winsock_fd_set {
        unsigned int fd_count;               /* how many are SET? */
        int          fd_array [FD_SETSIZE];  /* an array of sockets */
      } winsock_fd_set;

W32_FUNC int __stdcall __WSAFDIsSet (int s, winsock_fd_set *fd);

#endif  /* _WATT32_FAKE_WINSOCK_H */
#endif  /* _WATT32_FAKE_WINSOCK2_H */

