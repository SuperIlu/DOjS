/*
 *  This file contains code for displaying the Intel Cpu identification
 *  that has been performed by CheckCpuType() function.
 *
 *  COPYRIGHT (c) 1998 valette@crf.canon.fr
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.OARcorp.com/rtems/license.html.
 *
 *  $Id: displayCpu.c,v 1.4 1998/08/24 16:59:20 joel Exp $
 *
 *  Heavily modified for Watt-32. Added support for Watcom-386,
 *  calculate CPU speed.  G. Vanem  <gvanem@yahoo.no> 2000
 */

/*
 * Tell us the machine setup..
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef COMPILING_PCDBUG_C
  #define HAVE_CHECK_CPU_TYPE
#else
  #include "sysdep.h"
#endif

#define CPU_TEST
#include "cpumodel.h"

#if !defined(HAVE_UINT64)
#error "I need 64-bit integer support."
#endif

#if !defined(HAVE_CHECK_CPU_TYPE)
#error "I need '_w32_CheckCpuType()'"
#endif

static char Cx86_step = 0;

const char *cpu_get_model (int type, int model);

#if !defined(COMPILING_PCDBUG_C)
  static char *Cx86_type[] = {
       "unknown", "1.3", "1.4", "1.5", "1.6", "2.4",
       "2.5", "2.6", "2.7 or 3.7", "4.2"
     };
#endif

/* Since these are already in libwatt.a
 */
#if defined(COMPILING_PCDBUG_C) || !defined(WATT32_STATIC)
const char *i486model (unsigned int nr)
{
  static char *model[] = {
              "0", "DX", "SX", "DX/2", "4", "SX/2", "6",
              "DX/2-WB", "DX/4", "DX/4-WB", "10", "11", "12", "13",
              "Am5x86-WT", "Am5x86-WB"
            };
  if (nr < DIM(model))
     return (model[nr]);
  return (NULL);
}

const char *i586model (unsigned int nr)
{
  static char *model[] = {
              "0", "Pentium 60/66", "Pentium 75+", "OverDrive PODP5V83",
              "Pentium MMX", NULL, NULL, "Mobile Pentium 75+",
              "Mobile Pentium MMX"
            };
  if (nr < DIM(model))
     return (model[nr]);
  return (NULL);
}

const char *Cx86model (void)
{
  BYTE   nr6x86 = 0;
  static const char *model[] = {
                    "unknown", "6x86", "6x86L", "6x86MX", "MII"
                  };

  switch (x86_type)
  {
    case 5:
         /* cx8 flag only on 6x86L */
         nr6x86 = ((x86_capability & X86_CAPA_CX8) ? 2 : 1);
         break;
    case 6:
         nr6x86 = 3;
         break;
    default:
         nr6x86 = 0;
  }

  /* We must get the stepping number by reading DIR1.
   * This compiles with MSVC under Windows, but will likely
   * crash at run-time. So for WIN32, just fake it.
   */
#if defined(_WIN32)
  x86_mask = 0;
#else
  outp (0x22, 0xff);
  x86_mask = inp (0x23);
#endif

  switch (x86_mask)
  {
    case 0x03:
         Cx86_step = 1;   /* 6x86MX Rev 1.3 */
         break;
    case 0x04:
         Cx86_step = 2;   /* 6x86MX Rev 1.4 */
         break;
    case 0x05:
         Cx86_step = 3;   /* 6x86MX Rev 1.5 */
         break;
    case 0x06:
         Cx86_step = 4;   /* 6x86MX Rev 1.6 */
         break;
    case 0x14:
         Cx86_step = 5;   /* 6x86 Rev 2.4 */
         break;
    case 0x15:
         Cx86_step = 6;   /* 6x86 Rev 2.5 */
         break;
    case 0x16:
         Cx86_step = 7;   /* 6x86 Rev 2.6 */
         break;
    case 0x17:
         Cx86_step = 8;   /* 6x86 Rev 2.7 or 3.7 */
         break;
    case 0x22:
         Cx86_step = 9;   /* 6x86L Rev 4.2 */
         break;
    default:
         Cx86_step = 0;
         break;
  }
  return (model[nr6x86]);
}

const char *i686model (unsigned int nr)
{
  static const char *model[] = {
                    "PPro A-step", "Pentium Pro"
                  };
  if (nr < DIM(model))
     return (model[nr]);
  return (NULL);
}

struct cpu_model_info {
       int   x86;
       char *model_names[16];
     };

static const struct cpu_model_info amd_models[] = {
  { 4,
    { NULL, NULL, NULL, "DX/2", NULL, NULL, NULL, "DX/2-WB", "DX/4",
      "DX/4-WB", NULL, NULL, NULL, NULL, "Am5x86-WT", "Am5x86-WB"
    }
  },
  { 5,
    { "K5/SSA5 (PR-75, PR-90, PR-100)", "K5 (PR-120, PR-133)",
      "K5 (PR-166)", "K5 (PR-200)", NULL, NULL,
      "K6 (166 - 266)", "K6 (166 - 300)", "K6-2 (200 - 450)",
      "K6-3D-Plus (200 - 450)", NULL, NULL, NULL, NULL, NULL, NULL
    }
  },
};

const char *AMDmodel (void)
{
  char *p = NULL;
  int   i;

  if (x86_model < 16)
  {
    for (i = 0; i < DIM(amd_models); i++)
        if (amd_models[i].x86 == x86_type)
        {
          p = amd_models[i].model_names[(int)x86_model];
          break;
        }
  }
  return (p);
}

const char *cpu_get_model (int type, int model)
{
  const  char *p = NULL;
  static char nbuf[12];

  if (!strncmp(x86_vendor_id, "Cyrix", 5))
     p = Cx86model();
  else if (!strcmp(x86_vendor_id, "AuthenticAMD"))
     p = AMDmodel();
#if 0
  else if (!strcmp(x86_vendor_id, "UMC UMC UMC "))
     p = UMCmodel();
  else if (!strcmp(x86_vendor_id, "NexGenDriven"))
     p = NexGenModel();
  else if (!strcmp(x86_vendor_id, "CentaurHauls"))
     p = CentaurModel();
  else if (!strcmp(x86_vendor_id, "RiseRiseRise"))  /* Rise Technology */
     p = RiseModel();
  else if (!strcmp(x86_vendor_id, "GenuineTMx86"))  /* Transmeta */
     p = TransmetaModel();
  else if (!strcmp(x86_vendor_id, "Geode by NSC"))  /* National Semiconductor */
     p = NationalModel();
#endif
  else   /* Intel */
  {
    switch (type)
    {
      case 4:
           p = i486model (model);
           break;
      case 5:
           p = i586model (model);   /* Pentium I */
           break;
      case 6:
           p = i686model (model);   /* Pentium II */
           break;
      case 7:
           p = "Pentium 3";
           break;
      case 8:
           p = "Pentium 4";
           break;
    }
  }
  if (p)
     return (p);

  sprintf (nbuf, "%d", model);
  return (nbuf);
}
#endif  /* COMPILING_PCDBUG_C || !WATT32_STATIC */

/* Do not compile this if this file is included from pcdbug.c.
 */
#if !defined(COMPILING_PCDBUG_C)

/*
 * A good CPU reference:
 * http://www.flounder.com/cpuid_explorer2.htm
 */
void print_cpu_info (void)
{
  static const char *x86_cap_flags[] = {
               "FPU", "VME", "DE",      /* bits 0 - 2 */
               "PSE", "TSC", "MSR",
               "PAE", "MCE", "CX8",     /* bits 6 - 8 */
               "APIC", "FSC", "SEP",
               "MTRR", "PGE", "MCA",    /* bits 12 - 14 */
               "CMOV", "PAT", "PSE36",
               "PSN", "CFLSH", "20??",  /* bits 18 - 20 */
               "DTES", "ACPI", "MMX",
               "FXSR", "SSE", "SSE2",   /* bits 24 - 26 */
               "SSNOOP", "HTT", "ACC",
               "IA64", "PBE"            /* bits 30 - 31 */
             };
  int i, len;

  /* This sets 'x86_type', 'x86_model' and 'x86_vendor_id' if
   * CPUID is detected.
   */
  CheckCpuType();

  printf ("CPU      : %d\n", x86_type);
  printf ("model    : %s\n", x86_have_cpuid ? cpu_get_model(x86_type,x86_model) :
                                              "unknown (no CPUID)");
  if (x86_vendor_id[0] == '\0')
     strcpy (x86_vendor_id, "unknown");

  if (x86_mask)
  {
    if (!strncmp (x86_vendor_id, "Cyrix", 5))
         printf ("stepping : %s\n", Cx86_type[(int)Cx86_step]);
    else printf ("stepping : %d\n", x86_mask);
  }
  else
    printf ("stepping : unknown\n");

  printf ("vendor_id: %s\n", x86_vendor_id);

  printf ("fpu      : %s\n", (x86_hard_math  ? "yes" : "no"));
  printf ("cpuid    : %s\n", (x86_have_cpuid ? "yes" : "no"));
  printf ("flags    :");

  len = 0;
  for (i = 0; i < DIM(x86_cap_flags); i++)
  {
    if (x86_capability & (1 << i))
       len += printf (" %s", x86_cap_flags[i]);
    if (len >= 65)
    {
      len = 0;
      printf ("\n          ");
    }
  }
  printf ("\n");
}

void print_cpu_serial_number (void)
{
  printf ("Serial # : ");

  if ((x86_capability & X86_CAPA_PSN) && x86_have_cpuid)
  {
    DWORD eax, ebx, ecx, edx;

    get_cpuid (3, &eax, &ebx, &ecx, &edx);
    printf ("%04X-%04X-%04X-%04X-%04X-%04X\n",
            (WORD)(ebx >> 16), (WORD)(ebx & 0xFFFF),
            (WORD)(ecx >> 16), (WORD)(ecx & 0xFFFF),
            (WORD)(edx >> 16), (WORD)(edx & 0xFFFF));
  }
  else
    printf ("not present\n");
}

void print_reg (DWORD reg, const char *what)
{
  BYTE a = loBYTE (reg);
  BYTE b = hiBYTE (reg);
  BYTE c = loBYTE (reg >> 16);
  BYTE d = hiBYTE (reg >> 16);

  printf ("%s: %08lX, %c%c%c%c\n", what, DWORD_CAST(reg),
          isprint(a) ? a : '.',
          isprint(b) ? b : '.',
          isprint(c) ? c : '.',
          isprint(d) ? d : '.');
}

void print_cpuid_info (void)
{
  DWORD eax, ebx, ecx, edx;
  DWORD max;

  printf ("\ncpuid,0 (name)\n");
  get_cpuid (0, &eax, &ebx, &ecx, &edx);
  print_reg (eax, "EAX");
  print_reg (ebx, "EBX");
  print_reg (edx, "EDX");
  print_reg (ecx, "ECX");
  max = eax & 0xFF;
  if (max < 2)
     return;

  printf ("\ncpuid,1 (family)\n");
  get_cpuid (1, &eax, &ebx, &ecx, &edx);
  print_reg (eax, "EAX");
  print_reg (ebx, "EBX");
  print_reg (ecx, "ECX");
  print_reg (edx, "EDX");

  get_cpuid (0x80000000, &eax, &ebx, &ecx, &edx);
  max = eax & 0xFF;
  if (max < 2)
     return;

  printf ("\ncpuid, 80000002h\n");
  get_cpuid (0x80000002, &eax, &ebx, &ecx, &edx);
  print_reg (eax, "EAX");
  print_reg (ebx, "EBX");
  print_reg (ecx, "ECX");
  print_reg (edx, "EDX");

  printf ("\ncpuid, 80000003h\n");
  get_cpuid (0x80000003, &eax, &ebx, &ecx, &edx);
  print_reg (eax, "EAX");
  print_reg (ebx, "EBX");
  print_reg (ecx, "ECX");
  print_reg (edx, "EDX");

  printf ("\ncpuid, 80000004h\n");
  get_cpuid (0x80000004, &eax, &ebx, &ecx, &edx);
  print_reg (eax, "EAX");
  print_reg (ebx, "EBX");
  print_reg (ecx, "ECX");
  print_reg (edx, "EDX");
}

/*
 * [1] https://software.intel.com/en-us/articles/intel-digital-random-number-generator-drng-software-implementation-guide?wapkw=rng
 */
#define DRNG_HAS_RDRAND 0x1
#define DRNG_HAS_RDSEED 0x2

/*
 * Try this inline asm-snippet from [1] with Clang-cl too.
 */
#if defined(__GNUC__) || defined(__clang__)
  int get_rdrand32 (DWORD *rand)
  {
    unsigned char ok;
    __asm__ volatile ("rdrand %0; setc %1"
        : "=r" (*rand), "=qm" (ok));
    return (int) ok;
  }

#elif defined(_MSC_VER)
  int get_rdrand32 (DWORD *rand)
  {
    return _rdrand32_step (rand);
  }

#elif defined(__WATCOMC__) && defined(__386__)
 /*
  * Disassembly for the above should be:
  *   rdrand  ecx                    ; 0F C7 F1
  *   mov     DWORD PTR [eax], ecx   ; 89 08
  *   mov     eax, 0                 ; B8 00000000
  *   mov     edx, 1                 ; BA 00000001
  *   cmovb   eax, edx               ; 0F 42 C2
  */
  extern int get_rdrand32 (DWORD *rand);
  #pragma aux get_rdrand32 =     \
          ".586"                 \
          "db   0Fh, 0C7h, 0F1h" \
          "mov  [eax], ecx"      \
          "mov  eax, 0"          \
          "mov  edx, 1"          \
          "db   0Fh, 42h, 0C2h"  \
          __parm   [__eax]       \
          __modify [__eax __edx];

#else
  #define get_rdrand32(val_p) FALSE
#endif /* __GNUC__ || __clang__ */

void get_DRND_info (void)
{
  DWORD eax, ebx, ecx, edx;
  int i, rc = 0;

  get_cpuid (1, &eax, &ebx, &ecx, &edx);
  if ((ecx & 0x40000000) == 0x40000000)
     rc |= DRNG_HAS_RDRAND;

  get_cpuid (7, &eax, &ebx, &ecx, &edx);

  if ((ebx & 0x40000) == 0x40000)
     rc |= DRNG_HAS_RDSEED;

  puts ("");
  printf ("Have RDRAND: %s\n", rc & DRNG_HAS_RDRAND ? "Yes" : "No");
  printf ("Have RSEED:  %s\n", rc & DRNG_HAS_RDSEED ? "Yes" : "No");

  /* Try the RDRAND instruction a few times.
   */
  if (rc & DRNG_HAS_RDRAND)
     for (i = 0; i < 5; i++)
     {
       DWORD val = 0;
       BOOL  ok = get_rdrand32 (&val);
       printf ("  RDRAND: %10lu %s\n", DWORD_CAST(val), ok ? "OK" : "FAIL");
     }
}

void get_cache_info (void)
{
  DWORD eax = 0, ebx = 0, ecx = 0, edx;
  DWORD cache_sz = 0;

  printf ("\nCacheLine: ");

  if (!strncmp(x86_vendor_id, "GenuineIntel", 12))
  {
    get_cpuid (1, &eax, &ebx, &ecx, &edx);
    if (ebx)
       cache_sz = 8 * loBYTE (ebx >> 8);
  }
  else if (!strcmp(x86_vendor_id, "AuthenticAMD"))
  {
    get_cpuid (0x80000005, &eax, &ebx, &ecx, &edx);
    if (ecx)
       cache_sz = loBYTE (ecx);
  }

  if (cache_sz)
       printf ("%lu bytes\n", cache_sz);
  else puts ("Unknown");
}

void print_misc_regs (void)
{
#if (DOSX) && !defined(_WIN64)
  WORD cs = MY_CS();
  WORD ds = MY_DS();
  WORD es = MY_ES();
  WORD ss = MY_SS();

  printf ("CS       : Readable: %s, Writable: %s\n",
          SelReadable(cs) ? "Yes": "No", SelWriteable(cs) ? "Yes": "No");
  printf ("DS       : Readable: %s, Writable: %s\n",
          SelReadable(ds) ? "Yes": "No", SelWriteable(ds) ? "Yes": "No");
  printf ("ES       : Readable: %s, Writable: %s\n",
          SelReadable(es) ? "Yes": "No", SelWriteable(es) ? "Yes": "No");
  printf ("SS       : Readable: %s, Writable: %s\n",
          SelReadable(ss) ? "Yes": "No", SelWriteable(ss) ? "Yes": "No");
#endif
}

#if defined(__WATCOMC__)
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
            "mov [esi], eax"    \
            "mov [esi+4], ebx"  \
            "mov [esi+8], ecx"  \
            "mov [esi+12], edx" \
            __modify [__eax __ebx __ecx __edx] __parm [__eax];

void watcom_cpuid_test (void)
{
  CPUID_DATA data;

  data = cpuid (0);
  printf ("\nMaximum value permitted for CPUID instruction = %lu\n", data.eax);
  printf ("Signature = [%.4s%.4s%.4s]\n", &data.ebx, &data.edx, &data.ecx);

  data = cpuid (1);
  printf ("CPU Family   = %u\n", (data.eax >> 8)  & 15);
  printf ("CPU Type     = %u\n", (data.eax >> 12) & 15);
  printf ("CPU Model    = %u\n", (data.eax >> 4)  & 15);
  printf ("CPU Stepping = %u\n", data.eax & 15);

  if (data.edx & 0x800000)
     puts ("MMX is available");

  printf ("CR4 = %08lX\n", Get_CR4());
}
#endif /* __WATCOMC__ */

int main (void)
{
  uint64 Hz;

  init_misc();
  print_cpu_info();
  print_cpu_serial_number();
  print_misc_regs();

  Hz = get_cpu_speed();
  if (Hz)
       printf ("clock    : %.3f MHz (%d CPU core%s)\n",
               (double)Hz/1E6, num_cpus, num_cpus > 1 ? "s" : "");
  else printf ("clock    : RDTSC not supported\n");

  if (x86_have_cpuid)
       print_cpuid_info();
  else puts ("No CPUID");

  get_cache_info();

  if (!strncmp(x86_vendor_id, "GenuineIntel",12))
     get_DRND_info();

#ifdef __WATCOMC__
  watcom_cpuid_test();
#endif

  return (0);
}
#endif  /* !COMPILING_PCDBUG_C */

