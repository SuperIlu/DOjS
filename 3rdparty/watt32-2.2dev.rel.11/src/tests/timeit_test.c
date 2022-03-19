#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "timeit.h"

#define ARGSUSED(foo) (void)foo

#define u_int32_t unsigned long

static int __cdecl cmpint (const void *a, const void *b)
{
  return *(int*)a - *(int*)b;
}

/* This is a function ported from the Linux kernel lib/sort.c
 */
static void u_int32_t_swap (void *a, void *b, int size)
{
  u_int32_t t = *(u_int32_t*) a;

  *(u_int32_t*) a = *(u_int32_t*) b;
  *(u_int32_t*) b = t;
  ARGSUSED (size);
}

static void generic_swap (void *_a, void *_b, int size)
{
  char *a = (char*) _a;
  char *b = (char*) _b;
  char  t;

  do
  {
    t = *a;
    *a++ = *b;
    *b++ = t;
  }
  while (--size > 0);
}

/**
 * Qsort - sort an array of elements
 * @base:      pointer to data to sort
 * @num:       number of elements
 * @size:      size of each element
 * @cmp_func:  pointer to comparison function
 * @swap_func: pointer to swap function or NULL
 *
 * This function does a heapsort on the given array. You may provide a
 * swap_func function optimized to your element type.
 *
 * Sorting time is O(n log n) both on average and worst-case. While
 * qsort is about 20% faster on average, it suffers from exploitable
 * O(n*n) worst-case behavior and extra memory requirements that make
 * it less suitable for kernel use.
 */
void Qsort (void *_base, size_t num, size_t size,
            int (__cdecl *cmp_func) (const void *, const void *),
            void (*swap_func) (void *, void *, int size))
{
  /* pre-scale counters for performance
   */
  int   i = (num / 2 - 1) * size;
  int   n = num * size, c, r;
  char *base = (char *) _base;

  if (!swap_func)
     swap_func = (size == 4 ? u_int32_t_swap : generic_swap);

  /* heapify */
  for (; i >= 0; i -= (int)size)
  {
    for (r = i; r * 2 + size < (size_t)n; r = c)
    {
      c = r * 2 + size;
      if (c < n - (int)size && (*cmp_func)(base + c, base + c + size) < 0)
         c += size;
      if ((*cmp_func)(base + r, base + c) >= 0)
        break;
      (*swap_func) (base + r, base + c, size);
    }
  }

  /* sort */
  for (i = n - size; i > 0; i -= size)
  {
    (*swap_func) (base, base + i, size);
    for (r = 0; r * 2 + (int)size < i; r = c)
    {
      c = r * 2 + size;
      if (c < i - (int)size && (*cmp_func) (base + c, base + c + size) < 0)
         c += size;
      if ((*cmp_func)(base + r, base + c) >= 0)
        break;
      (*swap_func) (base + r, base + c, size);
    }
  }
}

#if defined(_MSC_VER) && (_MSC_VER >= 1200) && defined(_M_IX86) /* MSVC 6+ */
  _declspec(naked) static unsigned long echo_arg (unsigned long arg)
  {
    __asm mov eax, [esp]
    __asm ret
  }

  static void is_fastcall_compiled (void)
  {
    if (echo_arg(0xDEADBEEF) == 0xDEADBEEF)
         printf ("-Gr not in effect.\n");
    else printf ("-Gr in effect.\n");
  }
#else
  #define is_fastcall_compiled() ((void)0)
#endif

#define NUM_INTS  10000
#define NUM_LOOPS 10

int __cdecl main (void)
{
  int *a, i, r = 1;

  is_fastcall_compiled();

  a = malloc (NUM_INTS * sizeof(int));
  for (i = 0; i < NUM_INTS; i++)
  {
    r = (r * 725861) % 6599;
    a[i] = r;
  }

  TIME_IT (qsort, (a, NUM_INTS, sizeof(int), cmpint), NUM_LOOPS);
  TIME_IT (Qsort, (a, NUM_INTS, sizeof(int), cmpint, NULL), NUM_LOOPS);
  return (0);
}

