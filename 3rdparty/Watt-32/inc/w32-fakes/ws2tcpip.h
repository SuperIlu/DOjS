#ifndef _WATT32_FAKE_WS2TCPIP_H
#define _WATT32_FAKE_WS2TCPIP_H

#ifndef __SYS_SOCKET_H
#include <sys/socket.h>
#endif

#if !defined(WIN32) && !defined(_WIN32)
#error This file is only for Watt-32 targeting Windows programs.
#endif

#endif

