#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sysdep.h"

int main (int argc, char **argv)
{
  int i;

#ifdef __MSDOS__
  if (argc > 1 && !strcmp(argv[1],"-t"))
     init_timer_isr();
#else
  (void) argc;
  (void) argv;
#endif

  puts ("Running 30 times or until a keypress.");

  for (i = 0; i < 30 && !watt_kbhit(); i++)
  {
    struct timeval tv;
    const char *tstr;

    gettimeofday2 (&tv, NULL);
    tstr = ctime ((const time_t*)&tv.tv_sec);
    printf ("%10lu.%06lu, %s",
            (long unsigned int)tv.tv_sec,    /* shut-up djgpp */
            tv.tv_usec, tstr ? tstr : "illegal\n");
    usleep (500000);
  }
  return (0);
}
