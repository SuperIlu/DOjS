/*
 * Simple BSD-socket datagram test program for djgpp (+Windows).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _Windows  /* not Watt-32. Only Win32 using Winsock */
  #define WIN32_LEAN_AND_MEAN
  #include <winsock.h>
  #include <conio.h>

  static struct WSAData wsa_state;

  static void cleanup (void)
  {
    WSACleanup();
  }
  #define sleep(s) Sleep((s)*1000)
  #define close(s) closesocket(s)
#else
  #ifndef __CYGWIN__
  #include <dos.h>
  #endif
  #ifndef _MSC_VER
  #include <unistd.h>
  #endif
  #include <netdb.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
#endif

#ifdef WATT32
  #include <tcp.h>
  #define close(s) close_s(s)
  #define kbhit()  watt_kbhit()
#endif

#if defined(WIN32)
  #include <windows.h>
  #define sleep(x)  Sleep((x)*1000)
#endif

#define DEFAULT_PORT  6543
#define DEFAULT_IP    "172.18.9.187"
#define DEFAULT_MSG   "This is only a test"
#define MSGBUFSIZE    256

int sendUdp (char *message, int len, struct sockaddr_in *addr)
{
    int fd, ret;

    fd = socket (AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        perror ("socket");
        return (-1);
    }

   ret = sendto (fd, message, len, 0, (struct sockaddr*)addr, sizeof(struct sockaddr));
    if (ret < 0)
      perror ("sendto");

    printf("Ret %d\n", ret);
    sleep (4);

    shutdown (fd, 2);
    close(fd);

    return (0);
}


int main (int argc, char **argv)
{
    struct sockaddr_in addr;
    char *pAddr, *message, sndMsg[MSGBUFSIZE];
    int i, port;

    printf ("Usage: %s [[Ip] [Port]] [-mMessage]\n\n", argv[0]);

#if defined(WATT32)
    dbug_init();

#elif defined(_Windows)
    memset (&wsa_state, 0, sizeof(wsa_state));
    if (WSAStartup(MAKEWORD(1,1),&wsa_state) != 0)
    {
      printf ("Unable to start WinSock, error code=%d\n", WSAGetLastError());
      return (0);
    }
    atexit (cleanup);
#endif

    /* set up destination address */
    memset (&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    pAddr = (argc >= 2 ? argv[1] : DEFAULT_IP);
    port  = (argc >= 3 ? atoi(argv[2]) : DEFAULT_PORT);

    addr.sin_addr.s_addr = inet_addr (pAddr);
    addr.sin_port        = htons (port);

    if (strncmp(argv[argc-1], "-m", 2) == 0)
         message = &argv[argc-1][2];
    else message = DEFAULT_MSG;

    for (i = 0; i < 10 && !kbhit(); i++)
    {
        printf("[%d] Sending [%s] to %s port %d\n", i, message, pAddr, port);
        sprintf(sndMsg, "[% 2d]%s", i, message);
        sendUdp (sndMsg, strlen(sndMsg), &addr);
    }
    return (-1);
}
