#include <stdio.h>
#include <stdlib.h>

int main (void)
{
  printf ("Hello, I'm __GNUC__ %d\n", __GNUC__);

#ifdef __CYGWIN__
  printf ("Hello, I'm __CYGWIN__ %d\n", __CYGWIN__);
#if defined(__x86_64__)
  printf ("On '__x86_64__'\n");
#endif

#endif

  return (0);
}

