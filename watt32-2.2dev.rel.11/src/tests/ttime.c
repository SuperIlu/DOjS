#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifndef __CYGWIN__
#include <dos.h>
#endif

#include "wattcp.h"
#include "misc.h"
#include "timer.h"
#include "printk.h"

#if defined(__DMC__)
  #define dosdate_t  dos_date_t
  #define dostime_t  dos_time_t
#endif

#ifdef __MSDOS__
static int use_isr = 0;
#endif

int test_timers (DWORD msec)
{
  int i;

#ifdef __MSDOS__
  if (use_isr)
     init_timer_isr();
#endif

  printf ("system ref. (now) = %lu\n"
          "set_timeout(1000) = %lu\n",
          set_timeout(0), set_timeout(1000));

  for (i = 0; i < 10; i++)
  {
    DWORD t = set_timeout (msec);

    fputc ('.', stderr);
    while (!chk_timeout(t))
          ;
  }
  return (0);
}

int test_intel (void)
{
  printf ("intel  (11223344h) = %08lX\n", intel(0x11223344L));
  printf ("intel16(1122h)     = %04X\n",intel16(0x1122));
  return (0);
}

int test_gettod2 (void)
{
  struct timeval tv, last;
  DWORD  loop = 0;

#ifdef __MSDOS__
  if (use_isr)
     init_timer_isr();
#endif

  while (!kbhit())
  {
    gettimeofday2 (&tv, NULL);
    printf ("tod: %6ld.%06lu", (long)tv.tv_sec, tv.tv_usec);

    usleep (1000000);

    if (loop++ > 0)
         printf (", diff: %.6fs\n", timeval_diff(&tv,&last)/1E6);
    else putchar ('\n');

    last.tv_sec  = tv.tv_sec;
    last.tv_usec = tv.tv_usec;
  }
  return (0);
}

#ifdef __MSDOS__
/*
 * test timers around midnight.
 * Set time to 23:59:45, set timer to timeout at 00:00:15.
 * Check for timeout. Restore time.
 */
int test_rollover (void)
{
  struct dostime_t new, old;
  struct dosdate_t date;
  time_t start;
  DWORD  elapsed, t, tnow, tprev = 0;

  if (use_isr)
     init_timer_isr();

  _dos_gettime (&old);
  _dos_getdate (&date);
  new.hour    = 23;
  new.minute  = 59;
  new.second  = 45;
  new.hsecond = 0;
  _dos_settime (&new);       /* set to 23:59:45 */

  t = set_timeout (30000);
  printf ("timeout at timer-count %lu (00:00:15)\n", t);
  start = time (NULL);

  while (!kbhit())
  {
    struct dostime_t now;
    int    expired;
    long   td1;
    DWORD  td2;

    sleep (1);           /* wait 1 sec */

    expired = (chk_timeout (t) != 0);
    _dos_gettime (&now);
    tnow = set_timeout (0);

    if (tprev)
    {
      td1 = (long) (tnow - tprev);
      td2 = get_timediff (tnow, tprev);
    }
    else
    {
      td1 = 0;
      td2 = 0;
    }

    printf ("%10lu, td1 %10ld, td2 %10lu, %02d:%02d:%02d: %c\n",
            tnow, td1, td2, now.hour, now.minute, now.second,
            expired ? '+' : '-');
    tprev = tnow;
  }

  elapsed = (DWORD) difftime (time(NULL), start);

  old.second += elapsed;    /* restore old time */
  old.second %= 60;
  old.minute += elapsed / 60;

  _dos_settime (&old);
  _dos_setdate (&date);
  return (0);
}
#endif  /*  __MSDOS__ */

#ifdef HAVE_UINT64
const char *get_cpu_wrap_time (void)
{
  return (NULL);  /* \todo */
}
#endif

void Usage (void)
{
#ifdef __MSDOS__
  puts ("Usage: ttime [-hT] <-r | -g | -t | -i> \n"
        "  -h : don't use hi-resolution timers\n"
        "  -T : use timer ISR\n"
        "  -r : test timer day-rollover\n"
#else
  puts ("Usage: ttime <-g | -t | -i> \n"
#endif
        "  -g : test gettimeofday2()\n"
        "  -t : test general timers\n"
        "  -i : test intel() functions\n");
  exit (-1);
}


int main (int argc, char **argv)
{
  int ch;

  if (argc < 2)
     Usage();

  init_misc();

#ifdef HAVE_UINT64
  printf ("CPU-clock wraps in %s seconds\n", get_cpu_wrap_time());
#endif

  while ((ch = getopt(argc, argv, "hrgtiT")) != EOF)
    switch (ch)
    {
#ifdef __MSDOS__
      case 'h':
           hires_timer (0);
           break;

      case 'r':
           exit (test_rollover());

      case 'T':
           use_isr = 1;
           break;
#endif

      case 'g':
           exit (test_gettod2());

      case 't':
           exit (test_timers(1000));

      case 'i':
           exit (test_intel());

      default:
           Usage();
    }

  return (0);
}

