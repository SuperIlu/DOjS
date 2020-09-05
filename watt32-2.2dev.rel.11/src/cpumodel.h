/*!\file cpumodel.h
 */

/*
 *  This file contains declaration for variables and code
 *  that may be used to get the Intel Cpu identification
 *  that has been performed by CheckCpuType() function.
 *
 *  COPYRIGHT (c) 1998 valette@crf.canon.fr
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.OARcorp.com/rtems/license.html.
 *
 *  $Id: cpumodel.h,v 1.2 1998/08/05 16:51:39 joel Exp $
 */

#if !defined(_w32_CPUMODEL_H) && (DOSX)  /* Only for DOSX/Win32 targets */
#define _w32_CPUMODEL_H

#if defined(CPU_TEST) || defined(__LCC__) || (defined(_M_X64) && !defined(__GNUC__))
  #undef  CONST
  #define CONST
#else
  #define CONST const
#endif

#if defined(__DMC__)
  #define SYSCALL __syscall        /* no underscore prefix */
#else
  #define SYSCALL
#endif

#if defined(__GNUC__) || defined(__POCC__) || defined(_MSC_VER)
  #define DATA_DECL
#else
  #define DATA_DECL cdecl
#endif

extern CONST char  DATA_DECL x86_type;       /* type of CPU (3=386, 4=486, ...) */
extern CONST char  DATA_DECL x86_model;
extern CONST char  DATA_DECL x86_mask;
extern CONST int   DATA_DECL x86_hard_math;  /* FPU present */
extern CONST DWORD DATA_DECL x86_capability;
extern CONST int   DATA_DECL x86_have_cpuid;
extern CONST char  DATA_DECL x86_vendor_id[13];

/* x86_capability bits
 */
#define X86_CAPA_FPU        (1UL << 0)     /* Floating Point processor */
#define X86_CAPA_VME        (1UL << 1)     /* V86 Mode Extensions */
#define X86_CAPA_DE         (1UL << 2)     /* Debug Extensions */
#define X86_CAPA_PSE        (1UL << 3)     /* Page Size Extensions */
#define X86_CAPA_TSC        (1UL << 4)     /* Time Stamp Counter */
#define X86_CAPA_MSR        (1UL << 5)     /* Model Specific Registers */
#define X86_CAPA_PAE        (1UL << 6)     /* Physical Address Extensions */
#define X86_CAPA_MCE        (1UL << 7)     /* Machine Check Exception */
#define X86_CAPA_CX8        (1UL << 8)     /* CMPXCHG8B instruction available */
#define X86_CAPA_APIC       (1UL << 9)     /* APIC present (multiproc support) */
#define X86_CAPA_FSC        (1UL << 10)    /* Fast System Call (AMD K6/Cyrix) */
#define X86_CAPA_SEP        (1UL << 11)    /* Fast system call (SYSENTER/SYSEXIT) */
#define X86_CAPA_MTRR       (1UL << 12)    /* Memory Type Range Registers */
#define X86_CAPA_PGE        (1UL << 13)    /* Page Global Enable */
#define X86_CAPA_MCA        (1UL << 14)    /* Machine Check Architecture */
#define X86_CAPA_CMOV       (1UL << 15)    /* Conditional MOVe instructions */
#define X86_CAPA_PAT        (1UL << 16)    /* Page Attribute Table */
#define X86_CAPA_PSE36      (1UL << 17)    /* 36 bit Page Size Extenions */
#define X86_CAPA_PSN        (1UL << 18)    /* Processor Serial Number */
#define X86_CAPA_CFLSH      (1UL << 19)    /* Cache Flush */
#define X86_CAPA_RSRV_20    (1UL << 20)    /* Reserved */
#define X86_CAPA_DTES       (1UL << 21)    /* Debug Trace Store */
#define X86_CAPA_ACPI       (1UL << 22)    /* ACPI support */
#define X86_CAPA_MMX        (1UL << 23)    /* MultiMedia Extensions */
#define X86_CAPA_FXSR       (1UL << 24)    /* FXSAVE/FXRSTOR instructions */
#define X86_CAPA_SSE        (1UL << 25)    /* SSE instructions */
#define X86_CAPA_SSE2       (1UL << 26)    /* SSE2 instructions */
#define X86_CAPA_SSNOOP     (1UL << 27)    /* SELFSNOOP */
#define X86_CAPA_RSRV_28    (1UL << 28)    /* Reserved */
#define X86_CAPA_ACC        (1UL << 29)    /* Automatic Clock Control */
#define X86_CAPA_IA64       (1UL << 30)    /* IA64 instructions */
#define X86_CAPA_RSRV_31    (1UL << 31)    /* Reserved */

/* CR4 register bits.
 */
#define CR4_TS_DISABLE  0x004
#define CR4_OSFXSR      0x100  /* ?? OS does FXSAVE/FXRSTOR on task switch */

#if !defined(__LCC__)
  #define CheckCpuType W32_NAMESPACE (CheckCpuType)
  #define MY_CS        W32_NAMESPACE (MY_CS)
  #define MY_DS        W32_NAMESPACE (MY_DS)
  #define MY_ES        W32_NAMESPACE (MY_ES)
  #define MY_SS        W32_NAMESPACE (MY_SS)
  #define asm_ffs      W32_NAMESPACE (asm_ffs)
  #define Get_CR4      W32_NAMESPACE (Get_CR4)
  #define SelWriteable W32_NAMESPACE (SelWriteable)
  #define SelReadable  W32_NAMESPACE (SelReadable)
  #define get_cpuid    W32_NAMESPACE (get_cpuid)
  #define get_rdtsc    W32_NAMESPACE (get_rdtsc)
  #define get_rdtsc2   W32_NAMESPACE (get_rdtsc2)

  extern void  SYSCALL cdecl CheckCpuType (void);
  extern int   SYSCALL cdecl asm_ffs (int val);

  extern WORD  SYSCALL cdecl MY_CS   (void);
  extern WORD  SYSCALL cdecl MY_DS   (void);
  extern WORD  SYSCALL cdecl MY_ES   (void);
  extern WORD  SYSCALL cdecl MY_SS   (void);
  extern DWORD SYSCALL cdecl Get_CR4 (void);

  extern BOOL  SYSCALL cdecl SelWriteable (WORD sel);
  extern BOOL  SYSCALL cdecl SelReadable (WORD sel);
#endif

#if defined(__WATCOMC__) && 0 /* no more need for this */
  #pragma aux x86_type                    "*"
  #pragma aux x86_model                   "*"
  #pragma aux x86_mask                    "*"
  #pragma aux x86_hard_math               "*"
  #pragma aux x86_capability              "*"
  #pragma aux x86_vendor_id               "*"
  #pragma aux x86_have_cpuid              "*"

  #pragma aux (__cdecl) _w32_CheckCpuType "*"
  #pragma aux (__cdecl) _w32_MY_CS        "*"
  #pragma aux (__cdecl) _w32_MY_DS        "*"
  #pragma aux (__cdecl) _w32_MY_ES        "*"
  #pragma aux (__cdecl) _w32_MY_SS        "*"
  #pragma aux (__cdecl) _w32_asm_ffs      "*"
  #pragma aux (__cdecl) _w32_Get_CR4      "*"
  #pragma aux (__cdecl) _w32_SelWriteable "*"
  #pragma aux (__cdecl) _w32_SelReadable  "*"

#elif defined(__HIGHC__)
  #pragma alias (x86_type,         "x86_type")
  #pragma alias (x86_model,        "x86_model")
  #pragma alias (x86_mask,         "x86_mask")
  #pragma alias (x86_hard_math,    "x86_hard_math")
  #pragma alias (x86_capability,   "x86_capability")
  #pragma alias (x86_vendor_id,    "x86_vendor_id")
  #pragma alias (x86_have_cpuid,   "x86_have_cpuid")

  #pragma alias (_w32_CheckCpuType,"_w32_CheckCpuType")
  #pragma alias (_w32_MY_CS,       "_w32_MY_CS")
  #pragma alias (_w32_MY_DS,       "_w32_MY_DS")
  #pragma alias (_w32_MY_ES,       "_w32_MY_ES")
  #pragma alias (_w32_MY_SS,       "_w32_MY_SS")
  #pragma alias (_w32_asm_ffs,     "_w32_asm_ffs")
  #pragma alias (_w32_Get_CR4,     "_w32_Get_CR4")
  #pragma alias (_w32_SelWriteable,"_w32_SelWriteable")
  #pragma alias (_w32_SelReadable, "_w32_SelReadable")
#endif


#if defined(__GNUC__) && defined(__i386__) && !defined(__NO_INLINE__)
  /*
   * Call this only if x86_have_cpuid == TRUE.
   */
  W32_GCC_INLINE void get_cpuid (DWORD  val, DWORD *eax,
                                 DWORD *ebx, DWORD *ecx,
                                 DWORD *edx)
  {
    __asm__ __volatile__ (
              ".byte 0x0F,0xA2;"   /* cpuid opcode */
            : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
            : "0" (val));
    ARGSUSED (val);  /* for lint etc. */
    ARGSUSED (eax);
    ARGSUSED (ebx);
    ARGSUSED (ecx);
    ARGSUSED (edx);
  }

  /*
   * Return CPU timestamp value. Don't call unless 'has_rdtsc'
   * is TRUE. This code is originally by
   *   Tom Burgess <Tom_Burgess@bc.sympatico.ca> and
   *   Douglas Eleveld <deleveld@dds.nl>
   */
  W32_GCC_INLINE uint64 get_rdtsc (void)
  {
    register uint64 tsc;
    __asm__ __volatile__ (
              ".byte 0x0F, 0x31;"   /* rdtsc opcode */
            : "=A" (tsc) );
    return (tsc);
  }

  W32_GCC_INLINE void get_rdtsc2 (struct ulong_long *tsc)
  {
    __asm__ __volatile__ (
              ".byte 0x0F, 0x31;"
            : "=a" (tsc->lo), "=d" (tsc->hi) : );
  }

#elif defined(__GNUC__) && defined(__x86_64__) && !defined(__NO_INLINE__)
  /*
   * We can safely assume all x64 CPUs have 'x86_have_cpuid == TRUE'.
   */
  W32_GCC_INLINE void get_cpuid (DWORD val, DWORD *r1,
                                 DWORD *r2, DWORD *r3,
                                 DWORD *r4)
  {
    __asm__ __volatile__ (
              "cpuid"
            : "=a" (*r1), "=b" (*r2), "=c" (*r3), "=d" (*r4)
            : "a" (val));
    ARGSUSED (val);  /* for lint */
    ARGSUSED (r1);
    ARGSUSED (r2);
    ARGSUSED (r3);
    ARGSUSED (r4);
  }

  #if 1
    /*
     * Taken from:
     *   http://stackoverflow.com/questions/9887839/clock-cycle-count-wth-gcc
     */
    W32_GCC_INLINE uint64 get_rdtsc (void)
    {
      unsigned hi, lo;
      __asm__ __volatile__ (
                "rdtsc" : "=a" (lo), "=d" (hi) );
      return ( (uint64)lo) | ( ((uint64)hi) << 32 );
    }
  #else
    #undef  get_rdtsc
    #define get_rdtsc() __builtin_ia32_rdtsc()
  #endif

#elif defined(WATCOM386)
  /*
   * Call this only if x86_have_cpuid == TRUE.
   */
  extern void get_cpuid (DWORD val, DWORD *eax, DWORD *ebx, DWORD *ecx, DWORD *edx);
  #pragma aux _w32_get_cpuid =   \
        ".586"              \
        "push eax"          \
        "push ebx"          \
        "push ecx"          \
        "push edx"          \
        "mov eax, esi"      \
        "cpuid"             \
        "mov esi, [esp]"    \
        "mov [esi], edx"    \
        "mov esi, [esp+4]"  \
        "mov [esi], ecx"    \
        "mov esi, [esp+8]"  \
        "mov [esi], ebx"    \
        "mov esi, [esp+12]" \
        "mov [esi], eax"    \
        "add esp, 16"       \
        parm [esi] [eax] [ebx] [ecx] [edx] \
        modify [esi eax ebx ecx edx];

  extern uint64 get_rdtsc (void);
  #pragma aux _w32_get_rdtsc = \
          ".586"          \
          "db 0Fh, 31h"   \
          "cld"           \
          "nop"           \
          "nop"           \
          "nop"           \
          "nop"           \
          modify [eax edx];

#elif defined(__LCC__)
  #undef  get_rdtsc
  #define get_rdtsc() _rdtsc() /* in  <intrinsics.h> */

#else
  /*
   * No inlining for other targets
   */
  extern void SYSCALL cdecl get_cpuid (DWORD  val,  DWORD *_eax,
                                       DWORD *_ebx, DWORD *_ecx,
                                       DWORD *_edx);
  #ifdef HAVE_UINT64
  extern uint64 cdecl get_rdtsc (void);
  #endif
#endif

#if (DOSX) && !defined(__GNUC__)
  extern void cdecl get_rdtsc2 (struct ulong_long *);
#endif

#if defined(__HIGHC__)
  #pragma alias (get_cpuid,  "_w32_get_cpuid")
  #pragma alias (get_rdtsc,  "_w32_get_rdtsc")
  #pragma alias (get_rdtsc2, "_w32_get_rdtsc2")
#endif

#if defined(WIN32)
  extern uint64 win_get_rdtsc (void);
  #define GET_RDTSC()  win_get_rdtsc()
#else
  #define GET_RDTSC()  get_rdtsc()
#endif


#if 0 && (defined(__BORLANDC__) || defined(__DMC__)) && \
         (defined(__SMALL__) || defined(__LARGE__)) && \
         defined(__SMALL32__)
  /*
   * Save FPU state.
   */
  static void pkt_save_fpu (char far *state)
  {
    __asm les bx, state
    __asm fnstcw  es:[bx]     /* save IEM bit status */
    __asm nop                 /* delay while control word saved */
    __asm fndisi              /* disable BUSY signal */
    __asm mov ax, es:[bx]     /* get original control word in AX */
    __asm fsave   es:[bx]     /* save FPU state */
    __asm fwait               /* wait for save to complete */
    __asm mov es:[bx],ax      /* put original control word in saved state */
  }

  /*
   * Restore FPU state.
   */
  static void pkt_restore_fpu (char far *state)
  {
    __asm les bx, state
    __asm frstor es:[bx]
  }
#endif

#undef SYSCALL
#undef DATA_DECL

#if defined(__x86_64__) || defined(_M_X64)
  /*
   * All 64-bit CPU's have CPUID
   */
  W32_INLINE BOOL have_cpuid (void)
  {
    return (TRUE);
  }
#endif

/*
 * Check if a CPUID instruction is available on this CPU.
 * Not used at the moment. See cpumodel.asm instead.
 */
#if defined(__GNUC__) && defined(__i386__) && !defined(__NO_INLINE__)
  W32_GCC_INLINE BOOL have_cpuid (void)
  {
    int result = -1;

    /* We're checking if the bit #21 of EFLAGS
     * can be toggled. If yes = CPUID is available.
     */
    __asm__ __volatile__ (
              "pushf\n"
              "popl %%eax\n"
              "xorl $0x200000, %%eax\n"
              "movl %%eax, %%ecx\n"
              "andl $0x200000, %%ecx\n"
              "pushl %%eax\n"
              "popf\n"
              "pushf\n"
              "popl %%eax\n"
              "andl $0x200000, %%eax\n"
              "xorl %%eax, %%ecx\n"
              "movl %%ecx, %0\n"
            : "=r" (result) : : "eax", "ecx");
    return (result == 0);
  }

#elif (defined(_MSC_VER) || defined(__POCC__)) && !defined(_M_X64)   /* Not for 64-bit compilers */
  #if defined(__POCC__)
    #pragma warn(push)
    #pragma warn(disable:2135)
  #endif

  __declspec(naked) W32_INLINE BOOL have_cpuid (void)
  {
    _asm {
      pushfd
      pop  eax
      mov  ecx, eax
      xor  eax, 1 << 21
      push eax
      popfd
      pushfd
      pop  eax
      xor  eax, ecx
      bt   eax, 21
      jnc  no_cpuid
      mov  eax, 1
      ret
  no_cpuid:
      mov  eax, 0
      ret
    }
  }

  #if defined(__POCC__)
    #pragma warn(pop)
  #endif

/*
 * And finally a RDTSC + CPUID hack for 64-bit CPUs.
 * Use the one in cpumodel0.asm.
 */
#elif defined(_MSC_VER) && defined(_M_X64) && 0
  #pragma intrinsic   (__rdtsc)
  #undef  get_rdtsc
  #define get_rdtsc() __rdtsc()

  W32_INLINE void get_cpuid (DWORD val, DWORD *rax,
                             DWORD *rbx, DWORD *rcx,
                             DWORD *rdx)
  {
    int regs[4];

    __cpuid (regs, val);
    if (rax)
       *rax = (DWORD) regs[0];
    if (rbx)
       *rbx = (DWORD) regs[1];
    if (rcx)
       *rcx = (DWORD) regs[2];
    if (rdx)
       *rdx = (DWORD) regs[3];
  }
#endif

#endif   /* !_w32_CPUMODEL_H && DOSX */

