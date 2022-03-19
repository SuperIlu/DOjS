#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/wtime.h>
#include <tcp.h>

#ifdef WIN32
#include <windows.h>
#define usleep(us)  Sleep((us)/1000UL)
#endif

int main (int argc, char **argv)
{
#ifdef __MSDOS__
  if (argc > 1 && !strcmp(argv[1],"-t"))
     init_timer_isr();
#else
  (void) argc;
  (void) argv;
#endif

  while (!watt_kbhit())
  {
    struct timeval tv;
    const char *tstr;

    gettimeofday2 (&tv, NULL);
    tstr = ctime (&tv.tv_sec);
    printf ("%10lu.%06lu, %s",
            tv.tv_sec, tv.tv_usec, tstr ? tstr : "illegal\n");
    usleep (500000);
  }
  return (0);
}
