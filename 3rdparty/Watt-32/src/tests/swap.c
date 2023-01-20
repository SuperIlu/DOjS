#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <signal.h>
#include <time.h>
#include <limits.h>

#if defined(_MSC_VER)
  #define INVD_CACHE
  #include <intrin.h>

#elif defined(__MINGW32__) || defined(__CYGWIN__)
  #define TEST_UNDEF_OPCODE
  #include <x86intrin.h>
#endif

#if defined(__MINGW32__) || defined(__BORLANDC__)
  #define INVD_CACHE
  #define TEST_UNDEF_OPCODE

#elif defined(_MSC_VER)
  #define TEST_UNDEF_OPCODE

#elif defined(__WATCOMC__) && defined(__386__)
//#define INVD_CACHE
  #define TEST_UNDEF_OPCODE
#endif

#include "sysdep.h"
#include "gettod.h"
#include "cpumodel.h"

static volatile uint64 start64;
static volatile long   loops     = 20000;
static volatile long   swap_size = 10000;

static const char *get_clk_calls (uint64 delta)
{
  static char buf[50];

  snprintf (buf, sizeof(buf), "(%5" U64_FMT " clocks per 1000 calls)",
            4000*delta/(loops*swap_size));
  return (buf);
}

static unsigned long cdecl simple_cdecl_one (unsigned long x)
{
  return ((x & 0x000000FF) << 24) |
         ((x & 0x0000FF00) <<  8) |
         ((x & 0x00FF0000) >>  8) |
         ((x & 0xFF000000) >> 24);
}

#if defined(_MSC_VER) && !defined(_M_X64)
__declspec(naked) static unsigned long cdecl naked_cdecl_one (unsigned long x)
{
  __asm mov  eax, [esp+4]
  __asm xchg al, ah
  __asm ror  eax, 16
  __asm xchg al, ah
  __asm ret
}

__declspec(naked) static unsigned long cdecl naked_fastcall_one (unsigned long x)
{
  __asm xchg cl, ch    /* 'x' is in ecx */
  __asm ror  ecx, 16
  __asm xchg cl, ch
  __asm mov  eax, ecx
  __asm ret
}

#elif defined(_MSC_VER) && defined(_M_X64)
static W32_INLINE unsigned long cdecl naked_cdecl_one (unsigned long x)
{
  return ntohl (x);
}

static W32_INLINE unsigned long cdecl naked_fastcall_one (unsigned long x)
{
  return ntohl (x);
}

#elif defined(__GNUC__) && !defined(__x86_64__)
GCC_INLINE unsigned long cdecl naked_cdecl_one (unsigned long x)
{
  __asm__ __volatile (
           "xchgb %b0, %h0\n\t"   /* swap lower bytes  */
           "rorl  $16, %0\n\t"    /* swap words        */
           "xchgb %b0, %h0"       /* swap higher bytes */
          : "=q" (x) : "0" (x));
  return (x);
}

GCC_INLINE unsigned long GCC_FASTCALL naked_fastcall_one (unsigned long x)
{
  __asm__ __volatile (
           "xchgb %b0, %h0\n\t"
           "rorl  $16, %0\n\t"
           "xchgb %b0, %h0"
          : "=q" (x) : "0" (x));
  return (x);
}

#elif defined(__GNUC__) && defined(__x86_64__)
GCC_INLINE unsigned long cdecl naked_cdecl_one (unsigned long x)
{
  return ntohl (x);
}

GCC_INLINE unsigned long GCC_FASTCALL naked_fastcall_one (unsigned long x)
{
  return ntohl (x);
}

#elif defined(__BORLANDC__) && defined(__FLAT__)
static __inline unsigned long cdecl naked_cdecl_one (unsigned long x)
{
  return ntohl (x);
}

static __inline unsigned long cdecl naked_fastcall_one (unsigned long x)
{
  return ntohl (x);
}

#elif defined(__WATCOMC__) && (defined(__I86__) || defined(__386__))
static W32_INLINE unsigned long cdecl naked_cdecl_one (unsigned long x)
{
  return ntohl (x);
}

static W32_INLINE unsigned long cdecl naked_fastcall_one (unsigned long x)
{
  return ntohl (x);
}
#endif

static int simple_cdecl_ntohl (const void *buf, size_t max)
{
  unsigned long  *val = (unsigned long*) buf;
  volatile size_t i;

  for (i = 0; i < max; i++)
      simple_cdecl_one (*val++);
  return (1);
}

static int naked_cdecl_ntohl (const void *buf, size_t max)
{
  unsigned long  *val = (unsigned long*) buf;
  volatile size_t i;

  for (i = 0; i < max; i++)
      naked_cdecl_one (*val++);
  return (1);
}

static int naked_fastcall_ntohl (const void *buf, size_t max)
{
  unsigned long  *val = (unsigned long*) buf;
  volatile size_t i;

  for (i = 0; i < max; i++)
      naked_fastcall_one (*val++);
  return (1);
}

static int intrin_byteswap (const void *buf, size_t max)
{
#if defined(_MSC_VER)
  unsigned long  *val = (unsigned long*) buf;
  volatile size_t i;

  for (i = 0; i < max; i++)
      _byteswap_ulong (*val++);
  return (1);

#elif defined(__MINGW32__) || defined(__CYGWIN__)
  unsigned long  *val = (unsigned long*) buf;
  volatile size_t i;

  for (i = 0; i < max; i++)
      __bswapd (*val++);
  return (1);

#else
  (void)buf;
  (void)max;
  printf ("%s() not available for %s.\n", __FUNCTION__, wattcpBuildCC());
  return (0);
#endif
}

/*----------------------------------------------------------------------*/

static void test_swap_speed (const char *buf)
{
  struct timeval start, now;
  volatile long  i;

  /*---------------------------------------------------------------*/
  printf ("Timing simple_cdecl_ntohl()......... ");
  fflush (stdout);
  gettimeofday2 (&start, NULL);
  start64 = get_rdtsc();

  for (i = 0; i < loops; i++)
      simple_cdecl_ntohl (buf, swap_size/4);

  gettimeofday2 (&now, NULL);

  printf ("time ....%.6fs %s\n",
          timeval_diff(&now, &start)/1E6,
          get_clk_calls(get_rdtsc()-start64));

  /*---------------------------------------------------------------*/
  printf ("Timing naked_cdecl_ntohl().......... ");
  fflush (stdout);
  gettimeofday2 (&start, NULL);
  start64 = get_rdtsc();

  for (i = 0; i < loops; i++)
      naked_cdecl_ntohl (buf, swap_size/4);

  gettimeofday2 (&now, NULL);
  printf ("time ....%.6fs %s\n",
          timeval_diff(&now, &start)/1E6,
          get_clk_calls(get_rdtsc()-start64));

  /*---------------------------------------------------------------*/
  printf ("Timing naked_fastcall_ntohl()....... ");
  fflush (stdout);
  gettimeofday2 (&start, NULL);
  start64 = get_rdtsc();

  for (i = 0; i < loops; i++)
      naked_fastcall_ntohl (buf, swap_size/4);

  gettimeofday2 (&now, NULL);
  printf ("time ....%.6fs %s\n",
          timeval_diff(&now, &start)/1E6,
          get_clk_calls(get_rdtsc()-start64));

  /*---------------------------------------------------------------*/
  printf ("Timing intrin_byteswap()............ ");
  fflush (stdout);
  gettimeofday2 (&start, NULL);
  start64 = get_rdtsc();

  for (i = 0; i < loops; i++)
      if (!intrin_byteswap(buf, swap_size/4))
         return;

  gettimeofday2 (&now, NULL);
  printf ("time ....%.6fs %s\n",
          timeval_diff(&now, &start)/1E6,
          get_clk_calls(get_rdtsc()-start64));
}

static void test_swap_speed2 (const char *buf)
{
  puts ("Calling the same via 'TIME_IT()' macro:");
  TIME_IT (simple_cdecl_ntohl,   (buf, swap_size/4), loops);
  TIME_IT (naked_cdecl_ntohl,    (buf, swap_size/4), loops);
  TIME_IT (naked_fastcall_ntohl, (buf, swap_size/4), loops);
  TIME_IT (intrin_byteswap,      (buf, swap_size/4), loops);
}

static void sigill_handler2 (int sig)
{
  printf ("SIGILL caught");
  signal (sig, SIG_IGN);
  exit (0);
}

#if defined(__WATCOMC__) && defined(__386__)
  extern void do_ill_op (void);
  #pragma aux do_ill_op = \
            "db 0Fh, 0Bh";  /* UD2 opcode */

#elif defined(__GNUC__) || defined(__clang__)
  void do_ill_op (void)
  {
    __asm__ (".byte 0x0F, 0x0B");
  }

#elif defined(_MSC_VER) && defined(_M_IX86)
  void do_ill_op (void)
  {
    __asm ud2
  }

#elif defined(_MSC_VER) && (defined(_M_IA64) || defined(_M_X64))
  /*
   * A 'VMLOAD machine instruction':
   *   https://docs.microsoft.com/en-us/cpp/intrinsics/svm-vmload?view=vs-2019
   * Try it and see what it does.
   */
  void do_ill_op (void)
  {
    __svm_vmload(0);
  }

#elif defined(__BORLANDC__)
  void do_ill_op (void)
  {
    __emit__ (0x0f, 0x0b);
  }
#endif

static void test_undef_opcode (void)
{
#ifdef TEST_UNDEF_OPCODE
  signal (SIGILL, sigill_handler2);
  puts ("Trying something illegal.");
  do_ill_op();
#else
  puts ("Nothing happening here.");
#endif
  exit (0);
}

static void Usage (const char *argv0)
{
  printf ("Usage: %s [-i buf-size] [-l loops] [-o]\n"
          "  -i : size of swap buffer (default %ld).\n"
          "  -l : number of loops     (default %ld).\n"
          "  -o : test illegal opcode.\n",
          argv0, swap_size, loops);
  exit (-1);
}

int cdecl main (int argc, char **argv)
{
  int   ch, i;
  char *buf = NULL;

  while ((ch = getopt(argc, argv, "l:i:oh?")) != EOF)
    switch (ch)
    {
      case 'l':
           loops = atol (optarg);
           break;
      case 'i':
           swap_size = atol (optarg);
           break;
      case 'o':
           test_undef_opcode();
           break;
      default:
           Usage (argv[0]);
           break;
    }

  buf = alloca (swap_size);
  for (i = 0; i < swap_size; i++)
      buf[i] = i;

  test_swap_speed (buf);
  test_swap_speed2 (buf);
  fflush (stdout);
  return (0);
}

