#include <stdio.h>

#ifdef __WATCOMC__

typedef struct {
        unsigned eax;
        unsigned ebx;
        unsigned ecx;
        unsigned edx;
      } CPUID_DATA;

CPUID_DATA cpuid (int);

#pragma aux cpuid =  \
            ".586"   \
            "cpuid"  \
            "mov [esi],eax"    \
            "mov [esi+4],ebx"  \
            "mov [esi+8],ecx"  \
            "mov [esi+12],edx" \
            __modify [eax ebx ecx edx] __parm [eax];


int main (void)
{
  CPUID_DATA data;

  data = cpuid (0);
  printf ("Maximum value permitted for CPUID instruction = %lu\n", data.eax);
  printf ("Signature = [%.4s%.4s%.4s]\n", &data.ebx, &data.edx, &data.ecx);

  data = cpuid (1);
  printf ("CPU Family   = %u\n", (data.eax >> 8)  & 15);
  printf ("CPU Type     = %u\n", (data.eax >> 12) & 15);
  printf ("CPU Model    = %u\n", (data.eax >> 4)  & 15);
  printf ("CPU Stepping = %u\n", data.eax & 15);

  if (data.edx & 0x800000)
     puts ("MMX is available");

  return (0);
}
#else
#error For Watcom only
#endif
