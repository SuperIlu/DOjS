#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <math.h>

#include "wattcp.h"
#include "timer.h"
#include "misc.h"
#include "cpumodel.h"

static int num_samples = 3;
static int wait_ticks  = 2;

static struct {
       double   sum;
       double   sqSum;
       unsigned samples;
     } stats = { 0.0, 0.0, 0 };

#if defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__)
/*
 * Wait for timer-ticks to change 'num' times.
 */
static void WaitN (int num)
{
  DWORD tim;

  while (num--)
  {
    tim = GetTickCount() + 55;
    while (GetTickCount() < tim)
          Sleep (0);
  }
}
#else  /* __MSDOS__ */

/*
 * Wait for BIOS timer-tick to change 'num' times.
 */
static void WaitN (int num)
{
  while (num--)
  {
    DWORD t = PEEKL (0, BIOS_CLK);
    while (PEEKL(0,BIOS_CLK) == t)
    {
      ENABLE();
    }
  }
}
#endif

static uint64 _get_cpu_speed (void)
{
  uint64 speed;
  uint64 sample [100];
  int    i;

  assert (DIM(sample) > num_samples);
  WaitN (1);

  for (i = 0; i < num_samples; i++)
  {
    uint64 start = get_rdtsc();

    WaitN (wait_ticks);
    sample[i] = 1000 * (get_rdtsc() - start) / (wait_ticks*55);
  }

  speed = 0;
  for (i = 0; i < num_samples; i++)
      speed += sample[i];
  return (speed / num_samples);
}

void PutSample (double hz)
{
  stats.sum   += hz;
  stats.sqSum += pow (hz, 2.0);
  stats.samples++;
}

double StdDeviation (void)
{
  double rc, n;

  if (stats.samples == 0)
     return (0.0);
  n = (double) stats.samples;
  rc = sqrt ((n*stats.sqSum - pow(stats.sum,2.0)) / (n*(n-1.0)));
  return (rc);
}

void sigill (int sig)
{
  (void)sig;
  puts ("This CPU doesn't seem to have the RDTSC instruction");
  exit (-1);
}

void Usage (void)
{
  puts ("Usage: cpuspeed <num-samples> <wait-ticks>");
  exit (-1);
}

int main (int argc, char **argv)
{
  uint64 Hz, average, stdDev;
  int    i;

#if defined(__DJGPP__) && 0
  printf ("My-DS %04X readable:  %d\n", _my_ds(), SelReadable(_my_ds()));
  printf ("My-DS %04X writable: %d\n",  _my_ds(), SelWriteable(_my_ds()));
  printf ("My-CS %04X readable:  %d\n", _my_cs(), SelReadable(_my_cs()));
  printf ("My-CS %04X writable: %d\n",  _my_cs(), SelWriteable(_my_cs()));
  printf ("0 selector readable:  %d\n", SelReadable(0));
  printf ("0 selector writable: %d\n",  SelWriteable(0));
#endif

  if (argc != 3)
     Usage();

  num_samples = atoi (argv[1]);
  wait_ticks  = atoi (argv[2]);
  if (num_samples <= 0 || wait_ticks <= 0)
  {
    puts ("Illegal value");
    Usage();
  }

#if defined(SIGILL)
  signal (SIGILL, sigill);
#endif

  for (i = 0; i < 30 && !kbhit(); i++)
  {
    Hz = _get_cpu_speed();
    printf ("CPU speed %.3f MHz\n", (double)Hz/1E6);
    PutSample (Hz);
  }

  puts ("");
  average = stats.sum / stats.samples;
  stdDev  = StdDeviation();

  printf ("Average : %.3f MHz\n", average/1E6);
  printf ("Std Dev : %.3f MHz (%.1f%%)\n", stdDev/1E6, 100.0*stdDev/average);
  return (1);
}

