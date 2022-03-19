/*
 * CPU-model tool. Uses CPUID to figure out a more
 * excact name of the running processor. Supports these 32 and 64-bit
 * CPUs:
 *   GenuineIntel, AuthenticAMD and CentaurHauls.
 *
 * Supports GNU-C 4+ and MSVC v16+ compilers.
 *
 * Some of the below code has been taken from the MPIR Library. cpuid.c.
 *
 * Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006
 * Free Software Foundation, Inc.
 *
 * Copyright 2008 William Hart.
 *
 * Copyright 2009,2010,2011 Jason Moxham
 *
 * Copyright 2010 Gonzalo Tornaria
 *
 * The rest has been taken from my Watt-32 tcp/ip stack at:
 * http://www.watt-32.net
 *
 * G. Vanem <gvanem@yahoo.no> 2012.
 *
 * References:
 *   https://en.wikipedia.org/wiki/CPUID
 *   http://www.sandpile.org/x86/cpuid.htm
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#if defined(__GNUC__)
  #include <cpuid.h>

#elif defined(_MSC_VER)
  #include <intrin.h>
#endif

static int trace_level (void);

#define TRACE(level, fmt, ...)                          \
        do {                                            \
          if (trace_level() >= level) {                 \
            if (trace_level() >= 2)                     \
               printf ("%s:%4u: ", __FILE__, __LINE__); \
            printf (fmt, ##__VA_ARGS__);                \
          }                                             \
        } while (0)

static void print_reg (DWORD reg, const char *which)
{
  BYTE a = LOBYTE (reg);
  BYTE b = HIBYTE (reg);
  BYTE c = LOBYTE (reg >> 16);
  BYTE d = HIBYTE (reg >> 16);

  printf ("  %s: %08lX, %c%c%c%c\n", which, reg,
          isprint(a) ? a : '.',
          isprint(b) ? b : '.',
          isprint(c) ? c : '.',
          isprint(d) ? d : '.');
}

#if defined(_MSC_VER)
#if defined(_M_X64) || 1
static void msvc_cpuid (DWORD level, DWORD *_eax, DWORD *_ebx, DWORD *_ecx, DWORD *_edx)
{
  int regs[4];

  __cpuid (regs, level);
  *_eax = (DWORD) regs[0];
  *_ebx = (DWORD) regs[1];
  *_ecx = (DWORD) regs[2];
  *_edx = (DWORD) regs[3];
}
#else
__declspec(naked) static void msvc_cpuid (DWORD level, DWORD *_eax, DWORD *_ebx, DWORD *_ecx, DWORD *_edx)
{
  __asm  {
    enter 0, 0
    push ebx
    push esi
    mov  eax, [ebp+8]        /* EAX = level */
    cpuid
    mov  esi, [ebp+12]
    mov  [esi], eax          /* *_eax = EAX */
    mov  esi, [ebp+16]
    mov  [esi], ebx          /* *_ebx = EBX */
    mov  esi, [ebp+20]
    mov  [esi], ecx          /* *_ecx = ECX */
    mov  esi, [ebp+24]
    mov  [esi], edx          /* *_edx = EDX */
    pop  esi
    pop  ebx
    leave
    ret
  }
}
#endif
#endif  /* _MSC_VER */

static unsigned int get_cpuid2 (int level, void *result)
{
  unsigned int  eax = 0, ebx = 0, ecx = 0, edx = 0;
  unsigned int *res = (unsigned int*) result;
  const char   *cpuid_func;

#if defined(__GNUC__)
  cpuid_func = "__get_cpuid";
  __get_cpuid (level, &eax, &ebx, &ecx, &edx);
#elif defined(_MSC_VER)
  cpuid_func = "msvc_cpuid";
  msvc_cpuid (level, &eax, &ebx, &ecx, &edx);
#endif

  if (trace_init() >= 1)
  {
    TRACE (1, "\nFrom %s (0x%08X):\n", cpuid_func, level);
    print_reg (eax, "EAX");
    print_reg (ebx, "EBX");
    print_reg (ecx, "ECX");
    print_reg (edx, "EDX");
  }
  res[0] = ebx;
  res[1] = edx;  /* Not a typo; we want EDX into 'res[1]' */
  res[2] = ecx;
  return (eax);
}

/*
 * Returns a specific name for an "GenuineIntel" based on type, model and features.
 */
static const char *get_Intel_model (int type, int model, const void *features)
{
  char features2 [12];
  int  feat, id_max;

  switch (type)
  {
    case 5:
         if (model <= 2)
            return ("Pentium");
         if (model >= 4)
            return ("PentiumMMX");

    case 6:
         if (model == 1)
            return ("PentiumPro");
         if (model <= 6)

            return ("Pentium2");
         if (model <= 13)
            return ("Pentium3");

         if (model == 14 || model == 16)
            return ("Core");

         if (model == 15 || model == 22)
            return ("Core2");

         if (model == 17 || model == 23 || model == 29)
            return ("Penryn");

         if (model == 25 || model == 37 || model == 44 || model == 47)
            return ("Westmere");

         if (model == 26 || model == 30 || model == 31 || model == 46)
            return ("Nehalem");

         if (model == 28 || model == 38 || model == 39 || model == 54 || model == 55)
            return ("Atom");

         if (model == 42)
         {
           feat = ((int*) features) [2];
           if (feat & 0x10000000)
              return ("Sandybridge");
           return ("Westmere");
         }

         if (model == 43 || model == 45)
            return ("Sandybridge");

         if (model == 58 || model == 62)
            return ("Ivybridge");

         if (model == 60 || model == 63 || model == 69 || model == 70)
            return ("Haswell");
         break;

    case 15:
         id_max = get_cpuid2 (0x80000000, features2) & 0xff;
         if (id_max >= 1)
         {
           get_cpuid2 (0x80000001, features2);
           if (features2[8] & 1)
              return ("Netburstlahf");
           return ("Netburst");
         }
         if (model <= 6)
            return ("Pentium4");
         feat = ((int*)features)[2];
         if (feat & 1)
            return ("Prescott");
         break;
  }
  return (NULL);
}

/*
 * Returns a specific name for an "AuthenticAMD" CPU based on type and model.
 */
static const char *get_AMD_model (int type, int model)
{
  switch (type)
  {
    case 5:
         if (model <= 3)
            return ("K5");
         if (model <= 7)
            return ("K6");
         if (model <= 8)
            return ("K62");
         if (model <= 9)
            return ("K63");
         break;

    case 6:
         return ("K7");

    case 15:
         return ("K8");

    case 16:
         if (model == 2)
            return ("K10");
         if (model == 4 || model == 5 || model == 6 || model == 8 || model == 9 || model == 10)
            return ("K102");
         break;

    case 17:
         return ("K8");     /* Low power k8 */

    case 18:
         return ("K103");   /* Like k102 but with hardware divider, this is lano */

    case 20:
         return ("Bobcat"); /* Fusion of Bobcat and GPU */

    case 21:
         if (model == 1)
            return ("Bulldozer");
         if (model == 2 || model == 3 || model == 16 || model == 18 || model == 19)
            return ("Piledriver");

     case 22:
          return ("Jaguar");
  }
  return (NULL);
}

/*
 * Returns a specific name for an "CentaurHauls" CPU based on type and model.
 */
static const char *get_Centaur_model (int type, int model)
{
  if (type != 6)
     return (NULL);

  if (model == 15)
     return ("Nano");

  if (model < 9)
     return ("ViaC3");
  return ("ViaC32");
}

const char *cpu_get_model (void)
{
  static char  vendor_string[13];
  char   features[12];
  int    type, model, stepping;
  DWORD  id_max, fms;

  vendor_string[0] = '\0';
  id_max = get_cpuid2 (0, vendor_string);
  vendor_string[12] = '\0';

  TRACE (1, "  id_max: %08lX.\n", id_max);

  fms = get_cpuid2 (1, features);

  type = (fms >> 8) & 0x0F;
/*if (type == 15) */
     type |= (fms >> 16) & 0xFF0;  /* set extended family */

  model = (fms >> 4) & 0x0F;
/*if (model == 15) */
     model |= (fms >> 12) & 0xF0;  /* set extended model */

  stepping = fms & 15;

  TRACE (1, "Found vendor_string: \"%s\".\n", vendor_string);
  TRACE (1, "Found type:          %d.\n", type);
  TRACE (1, "Found model:         %d.\n", model);
  TRACE (1, "Found stepping:      %d.\n", stepping);

  if (!strcmp(vendor_string,"GenuineIntel"))
     return get_Intel_model (type, model, &features);

  if (!strcmp(vendor_string, "AuthenticAMD"))
     return get_AMD_model (type, model);

  if (!strcmp(vendor_string, "CentaurHauls"))
     return get_Centaur_model (type, model);

  if (vendor_string[0])
     return (vendor_string);

  return (NULL);
}

const char *cpu_get_freq_info (void)
{
  static char  result [100];
  char         info[13];
  DWORD        id_max = get_cpuid2 (0, info);
  unsigned int eax = 0, ebx = 0, ecx = 0;

  if (id_max < 0x16)
     return (NULL);

  eax = get_cpuid2 (0x16, info);
  ebx = ((unsigned int*) info) [1];
  ecx = ((unsigned int*) info) [2];

  snprintf (result, sizeof(result), "Core base: %d, Core max: %d, Core bus: %d (MHz)",
            eax & (1 << 15), ebx & (1 << 15), ecx & (1 << 15));
  return (result);
}

static int t_level = 0;

static int trace_level (void)
{
  const char *env;
  static int done = 0;

  if (done)
     return (t_level);

  env = getenv("CPU_TRACE");
  if (env)
     t_level = *env - '0';
  done = 1;
  return (t_level);
}

int main (int argc, char **argv)
{
  const char *cpu, *freq;

  if (argc > 1 && !strcmp(argv[1],"-d"))
     t_level = 1;

  cpu = cpu_get_model();
  printf ("CPU-model:    %s\n", cpu ? cpu : "<unknown>");

  freq = cpu_get_freq_info();
  printf ("CPU-freq info: %s\n", freq? freq : "<unknown>");
  return 0;
}

