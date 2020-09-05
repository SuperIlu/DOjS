#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

unsigned long cdecl Get_CR4 (void);
#pragma aux (__cdecl) Get_CR4 "*"

#ifdef __cplusplus
}
#endif

int main (void)
{
  printf ("CR4 %08lX:\n", Get_CR4());
  return (0);
}
