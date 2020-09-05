#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <signal.h>
#include <time.h>
#include <limits.h>

#include "wattcp.h"
#include "misc.h"
#include "timer.h"
#include "sock_ini.h"
#include "gettod.h"
#include "chksum.h"
#include "cpumodel.h"

#undef START_TIME

/* The 64-bit version of in_checksum_fast() simply does a
 * 'jmp _w32_in_checksum'. Doesn't seems to hurt on speed.
 */
#if (DOSX)
  #define HAVE_IN_CHKSUM_FAST
#endif

long loops   = 20000;
long ip_size = 10000;

#if defined(HAVE_UINT64)
  static uint64 start64, delta64;

  const char *get_clk_calls (void)
  {
    static char buf[30];

    if (!has_rdtsc)
       return ("");

    sprintf (buf, "(%" U64_FMT " clk/call, ", delta64/loops);
    return (buf);
  }

  const char *get_usec_calls (void)
  {
    static char buf[30];
    uint64 clocks;
    double usec;

    if (clocks_per_usec == 0)
       return ("? uS/call)");

    clocks = delta64 / loops;
    usec = (double)clocks / (double)clocks_per_usec;
    sprintf (buf, "%.3f uS/call)", usec);
    return (buf);
  }

  #define START_TIME() start64 = get_rdtsc()
  #define DELTA_TIME() delta64 = get_rdtsc() - start64

#else
  #define START_TIME()        ((void)0)
  #define DELTA_TIME()        ((void)0)
  #define get_clk_calls(d,l)  ""
  #define get_usec_calls(d,l) ""
#endif


/*
 * Some notes:
 *
 * Timing 'in_checksum_fast()' and 'in_checksum()' reveals that
 * Watcom's wcc386 is around 100% faster that gcc 2.95.2. And that
 * 'in_checksum_fast()' is around 2.5 times faster than 'in_checksum()'
 * compiled with wcc386. Hence using 'in_checksum_fast()' for djgpp
 * targets gives around 450% performance gain !!
 */

const char * Delta_ms (clock_t dt)
{
  static char buf[10];
  double delta = 1000.0 * (double) dt / (double)CLOCKS_PER_SEC;
  sprintf (buf, "%6.1f", delta);
  return (buf);
}

WORD ip_checksum (const char *ptr, int len)
{
  register long sum = 0;
  register const WORD *wrd = (const WORD*) ptr;

  while (len > 1)
  {
    sum += *wrd++;
    len -= 2;
  }
  if (len > 0)
     sum += *(const BYTE*)wrd;

  while (sum >> 16)
      sum = (sum & 0xFFFF) + (sum >> 16);

  return (WORD)sum;
}

WORD ip_checksum_dummy (const char *ptr, int len)
{
  ARGSUSED (ptr);
  ARGSUSED (len);
  return (0);
}

int test_checksum_speed (const char *buf)
{
  struct timeval start, now;
  long   j;

  /*---------------------------------------------------------------*/
  printf ("Timing in_checksum()....... ");
  fflush (stdout);
  gettimeofday2 (&start, NULL);
  START_TIME();

  for (j = 0; j < loops; j++)
      in_checksum (buf, ip_size);

  gettimeofday2 (&now, NULL);
  DELTA_TIME();
  printf ("time ....%.6fs %s%s\n",
          timeval_diff(&now, &start)/1E6, get_clk_calls(), get_usec_calls());

  /*---------------------------------------------------------------*/
  printf ("Timing ip_checksum()....... ");
  fflush (stdout);
  gettimeofday2 (&start, NULL);
  START_TIME();

  for (j = 0; j < loops; j++)
      ip_checksum (buf, ip_size);

  gettimeofday2 (&now, NULL);
  DELTA_TIME();
  printf ("time ....%.6fs %s%s\n",
          timeval_diff (&now, &start)/1E6, get_clk_calls(), get_usec_calls());

#if defined(HAVE_IN_CHKSUM_FAST)
  /*---------------------------------------------------------------*/
  printf ("Timing in_checksum_fast().. ");
  fflush (stdout);
  gettimeofday2 (&start, NULL);
  START_TIME();

  for (j = 0; j < loops; j++)
      in_checksum_fast (buf, ip_size);

  gettimeofday2 (&now, NULL);
  DELTA_TIME();
  printf ("time ....%.6fs %s%s\n",
          timeval_diff(&now, &start)/1E6, get_clk_calls(), get_usec_calls());
#endif

  /*---------------------------------------------------------------*/
  printf ("Timing overhead............ ");
  fflush (stdout);
  gettimeofday2 (&start, NULL);
  START_TIME();

  for (j = 0; j < loops; j++)
      ip_checksum_dummy (buf, ip_size);

  gettimeofday2 (&now, NULL);
  DELTA_TIME();
  printf ("time ....%.6fs %s%s\n",
          timeval_diff(&now, &start)/1E6, get_clk_calls(), get_usec_calls());
  return (0);
}

int test_checksum_correctness (const char *buf)
{
  long i;

  for (i = 0; i < loops; i++)
  {
    WORD len   = 10 + (i % 1000);
    WORD csum1 = ~in_checksum (buf, len);
    WORD csum2 = ~ip_checksum (buf, len);
#if defined(HAVE_IN_CHKSUM_FAST)
    WORD csum3 = ~in_checksum_fast (buf, len);
#else
    WORD csum3 = 0;
#endif

    printf ("in_checksum = %04X, ip_checksum = %04X, "
            "in_checksum_fast = %04X %c %c\n",
            csum1, csum2, csum3,
            (csum1 != csum2 ? '!' : ' '),
            (csum1 != csum3 ? '!' : ' '));
  }
  return (0);
}

void sig_handler (int sig)
{
  (void)sig;
  exit (0);
}

void Usage (char *argv0)
{
  char *p = strrchr (argv0, '/');

  if (!p)
     p = strrchr (argv0, '\\');
  if (p)
  {
    *p++ = '\0';
    argv0 = p;
  }
  printf ("Usage: %s [-i ip-size] [-l loops]  <-s | -c> \n"
          "  -s : test in_checksum() speed\n"
          "  -c : test in_checksum() correctness\n"
          "  -i : size of checksum buffer  (default %ld)\n"
          "  -l : number of checksum loops (default %ld)\n",
          argv0, ip_size, loops);
  exit (-1);
}

int main (int argc, char **argv)
{
  int   cflag = 0;
  int   sflag = 0;
  int   ch, i;
  char *buf = NULL;

  if (argc < 2)
     Usage (argv[0]);

  while ((ch = getopt(argc, argv, "l:i:sc?")) != EOF)
    switch (ch)
    {
      case 'l':
           loops = atol (optarg);
           break;
      case 'i':
           ip_size = atol (optarg);
           if (ip_size <= 100 || ip_size > (long)USHRT_MAX)
           {
             printf ("`-i' argument must be 101 - %u", USHRT_MAX);
             return (-1);
           }
           break;
      case 's':
           sflag = 1;
           break;
      case 'c':
           cflag = 1;
           break;
      default:
           Usage (argv[0]);
           break;
    }

  sock_init();

  printf ("has_rdtsc %d, use_rdtsc %d\n", has_rdtsc, use_rdtsc);

  signal (SIGINT, sig_handler);

  if (sflag || cflag)
  {
    buf = alloca (ip_size);
    if (!buf)
    {
      puts ("alloca() failed");
      return (-1);
    }
    for (i = 0; i < ip_size; i++)
        buf[i] = i % 255;
  }

  if (sflag)
     return test_checksum_speed (buf);

  if (cflag)
     return test_checksum_correctness (buf);

  return (0);
}

