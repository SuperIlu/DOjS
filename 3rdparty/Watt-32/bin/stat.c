#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <tcp.h>
#include <conio.h>

int MS_CDECL main (int argc, char **argv)
{
  udp_Socket sock;
  int port = 5433;

  if (argc == 2 && isdigit(argv[1][1]))
     port = atoi (argv[1]);

  sock_init();
  tzset();

  printf ("Network Monitor. Port = %d\n", port);
  puts ("Name     Time   UpTime   Users     CPU Load     PacketsI  PacketsOProc");

  udp_listen (&sock,port,0L,0,NULL);

  while (!watt_kbhit() && getch() != 27)
  {
    char buf[100];

    tcp_tick (NULL);
    if (sock_fastread (&sock, (BYTE*)buf, sizeof(buf)))
    {
      printf ("%s", buf);
      sock_close (&sock);
      udp_listen (&sock, port, 0L, 0, NULL);
    }
  }
  sock_close (&sock);
  return (0);
}
