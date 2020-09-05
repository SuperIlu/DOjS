/*!\file wdpmi.c
 *
 *  DPMI interface for Watcom-type targets.
 *
 *  DOS-extender/DPMI interface for DOS4GW Pro, DOS4G, DOS32A,
 *  Pmode/W, CauseWay, EDOS, HXDOS, WDOSX and Borland's POWERPAK (32rtm).
 *  Possibly usable with gcc and WDOSX also.
 *
 *  From udplib 1.111 by
 *    John Snagel  <jslagel@volition-inc.com> and
 *    Freek Brysse <frbrysse@vub.ac.be>
 *
 *  11 november, 1997
 *  Ref. http://igweb.vub.ac.be/knights/udplib.html
 *
 *  Heavily changed by G. Vanem <gvanem@yahoo.no>  August 1998
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wattcp.h"
#include "strings.h"
#include "language.h"
#include "pcsed.h"
#include "printk.h"
#include "sock_ini.h"
#include "cpumodel.h"
#include "misc.h"
#include "wdpmi.h"

#if defined(__MSDOS__)  /* rest of file */

/* Don't put this in config.h, but define to 1 here when it works.
 */
#undef USE_EXCEPT_HANDLER

#if defined(WATCOM386)
  /*
   * Values for '_Extender'
   */
  #define DOSX_ERGO     0
  #define DOSX_RATIONAL 1  /* DOS4Gxx, WDOSX, Pmode/W, CauseWay, EDOS */
  #define DOSX_PHAR_V2  2
  #define DOSX_PHAR_V3  3
  #define DOSX_PHAR_V4  4
  #define DOSX_PHAR_V5  5
  #define DOSX_PHAR_V6  6
  #define DOSX_PHAR_V7  7
  #define DOSX_PHAR_V8  8
  #define DOSX_INTEL    9  /* Intel C-Builder */
  #define DOSX_WIN386  10  /* Windows 3.11+ */

  /*
   * Values for '_ExtenderSubtype'
   */
  #define XS_NONE                  0
  #define XS_RATIONAL_ZEROBASE     0
  #define XS_RATIONAL_NONZEROBASE  1  /* Only in DOS4G Pro */

  extern void cdecl cstart_ (void);
  extern char      _Extender;
  extern char      _ExtenderSubtype;
#endif

#if defined(DMC386)
  #define int386     int86_real
  #define int386x    int86x_real
  #define AX_REG(r)  r.x.ax
  #define BX_REG(r)  r.x.bx
  #define CX_REG(r)  r.x.cx
  #define DX_REG(r)  r.x.dx
  #define EAX_REG(r) r.e.eax
  #define EBX_REG(r) r.e.ebx
  #define ECX_REG(r) r.e.ecx
  #define EDX_REG(r) r.e.edx
  #define ESI_REG(r) r.e.esi
  #define EDI_REG(r) r.e.edi
  #define CARRY(r)   (r.x.cflag & 1)

#else
  #define AX_REG(r)  r.w.ax
  #define BX_REG(r)  r.w.bx
  #define CX_REG(r)  r.w.cx
  #define DX_REG(r)  r.w.dx
  #define EAX_REG(r) r.x.eax
  #define EBX_REG(r) r.x.ebx
  #define ECX_REG(r) r.x.ecx
  #define EDX_REG(r) r.x.edx
  #define ESI_REG(r) r.x.esi
  #define EDI_REG(r) r.x.edi
  #define CARRY(r)   (r.w.cflag & 1)
#endif

#if (DOSX & (DOS4GW|POWERPAK))

#if defined(__CCDL__)
#define int386 _int386
#endif

WORD __dpmi_errno = 0;        /* last DPMI error */

#if (!(DOSX & POWERPAK))

WORD causeway_ver = 0;        /* major=MSB, minor=LSB */
BOOL dos32a_nullptr_chk = 0;  /* NULL-ptr traps active */

#define DOS32A_SIGN  (('I'<<24) + ('D'<<16) + ('3'<<8) + ('2'<<0))
#define PMODEW_SIGN  (('P'<<24) + ('M'<<16) + ('D'<<8) + ('W'<<0))
#define WDOSX_SIGN   (('W'<<24) + ('D'<<16) + ('S'<<8) + ('X'<<0))

#if (DOSX & DOS4GW) && defined(WATCOM386) && defined(USE_EXCEPT_HANDLER)
  static struct FAULT_STRUC exc_struct;
  static exceptionHook      user_exc_hook = NULL;

  static void far __loadds exception_main (void);
#endif

/*
 * WDOSX and Pmode/W uses same detect function with
 * unique signatures.
 */
static BOOL is_wdosx_or_pmodew (DWORD sign)
{
  union REGS reg;

#ifdef __DJGPP__
  reg.x.eax = 0xEEFF;
  int86 (0x31, &reg, &reg);
#else
  EAX_REG(reg) = 0xEEFF;
  int386 (0x31, &reg, &reg);
#endif
  return (EAX_REG(reg) == sign);
}

BOOL dpmi_is_wdosx (void)
{
  return is_wdosx_or_pmodew (WDOSX_SIGN);
}

BOOL dpmi_is_pmodew (void)
{
  return is_wdosx_or_pmodew (PMODEW_SIGN);
}

BOOL dpmi_is_dos32a (void)
{
  union REGS reg;

#ifdef __DJGPP__
  reg.x.eax = 0xFF89;  /* Get configuation info */
  int86 (0x21, &reg);
#else
  EAX_REG(reg) = 0xFF89;
  int386 (0x21, &reg, &reg);
#endif

  if (EAX_REG(reg) == DOS32A_SIGN)
  {
    dos32a_nullptr_chk = (EDX_REG(reg) & 0x80);
    return (TRUE);
  }
  return (FALSE);
}

BOOL dpmi_is_dos4gw (void)
{
  union REGS reg;

#ifdef __DJGPP__   /* I doubt DOS4GW and djgpp is possible */
  reg.x.eax = 0xFF00;
  reg.x.edx = 0x78;
  int86 (0x21, &reg);
#else
  EAX_REG(reg) = 0xFF00;
  EDX_REG(reg) = 0x78;
  int386 (0x21, &reg, &reg);
#endif
  return (EAX_REG(reg) == 0xFFFF3447);
}

BOOL dpmi_is_causeway (void)
{
#if defined(__DJGPP__)
  union  REGS  r;
  struct SREGS s;
  DWORD  rp;
  char   sign[10];

  s.ds = s.es = _my_ds();
  r.x.eax = 0x3531;
  int86x (0x21, &r, &r, &s);
  if (s.es == 0 || !SelReadable(s.es) ||
      __dpmi_get_segment_limit(s.es) < r.x.ebx)
     return (FALSE);

  movedata (s.es, r.x.ebx, _my_ds(), (unsigned)&sign, sizeof(sign));
  if (!memcmp(sign,"CAUSEWAY",8))
  {
    causeway_ver = (sign[8] << 8) + sign[9];
    return (TRUE);
  }

#elif defined(__CCDL__)
  /* !! to-do */

#elif defined(__WATCOMC__)
  union  REGS  r;
  struct SREGS s;
  BYTE  _far *rp;

  EAX_REG(r) = 0x3531;
  segread (&s);
  int386x (0x21, &r, &r, &s);

  rp = MK_FP (s.es, EBX_REG(r)-10);

  if (s.es && SelReadable(s.es)    &&
      EAX_REG(r) < GET_LIMIT(s.es) &&
      !_fstrncmp(rp,"CAUSEWAY",8))
  {
    rp += 8;
    causeway_ver = (rp[0] << 8) + rp[1];
    return (TRUE);
  }
#endif
  return (FALSE);
}

BOOL dpmi_is_hxdos (void)
{
  union REGS reg;
  char  buf[128] = { '\0' };

#ifdef __DJGPP__
  reg.x.eax = 0x401;
  reg.x.edi = (DWORD) &buf;
  int86 (0x31, &reg, &reg);
#else
  EAX_REG(reg) = 0x401;
  EDI_REG(reg) = (DWORD) &buf;
  int386 (0x31, &reg, &reg);
#endif
  return !strncmp(buf+2,"HDPMI",5);
}

/*
 * Return true name of "DOS4GW-type" extenders.
 */
const char *dos4gw_extender_name (void)
{
  if (dpmi_is_causeway())
     return ("CAUSEWAY");
  if (dpmi_is_wdosx())
     return ("WDOSX");
  if (dpmi_is_pmodew())
     return ("PMODEW");
  if (dpmi_is_hxdos())
     return ("HXDOS");
  if (dpmi_is_dos32a())
     return ("DOS32A");
  if (dpmi_is_dos4gw())
     return ("DOS4GW");
  return ("DOS4GW");  /* assume the rest are DOS4GW compatible */
}

/*
 * Exception handler for Causeway stubbed programs.
 * Setup to call 'exc_func' when exception occurs.
 */
int dpmi_except_handler (exceptionHook exc_func)
{
#if defined(WATCOM386) && defined(USE_EXCEPT_HANDLER)
  union  REGS  r;
  struct SREGS s;

  if (!dpmi_is_causeway())
  {
    __dpmi_errno = 0x8001;
    return (0);
  }

  __dpmi_errno = 0;
  segread (&s);
  s.ds    = FP_SEG (exception_main); /* vector address */
  r.x.esi = FP_OFF (exception_main);
  s.es    = FP_SEG (&exc_struct);    /* info dump address */
  r.x.edi = FP_OFF (&exc_struct);
  r.h.cl  = 1;                       /* 32-bit routine */
  r.w.ax  = 0xFF31;                  /* USER_ERR_TERM */
  int386x (0x31, &r, &r, &s);
  if (CARRY(r))
  {
    __dpmi_errno = AX_REG(r);
    return (0);
  }
  r.w.ax = 0xFF30;                   /* SetDump */
  r.h.cl = 0;
  int386 (0x31, &r, &r);

  user_exc_hook = exc_func;
  memset (&exc_struct, 0, sizeof(exc_struct));
  dpmi_lock_region ((void*)&exc_struct, sizeof(exc_struct));
  dpmi_lock_region ((void*)exception_main, 100);
  return (1);
#else
  ARGSUSED (exc_func);
  __dpmi_errno = 0x8001;
  return (0);
#endif
}
#endif /* !(DOSX & POWERPAK) */


void *dpmi_get_real_vector (int intr)
{
  union REGS r;

  __dpmi_errno = 0;
  EAX_REG(r) = 0x200;
  EBX_REG(r) = (DWORD) intr;
  int386 (0x31, &r, &r);
  return SEG_OFS_TO_LIN (CX_REG(r), DX_REG(r));
}

int dpmi_get_base_address (WORD sel, DWORD *base)
{
  union REGS r;

  __dpmi_errno = 0;
  EAX_REG(r) = 0x06;
  EBX_REG(r) = sel;
  int386 (0x31, &r, &r);
  if (CARRY(r))
  {
    __dpmi_errno = AX_REG(r);
    return (0);
  }
  *base = (CX_REG(r) << 16) + DX_REG(r);
  return (1);
}

WORD dpmi_real_malloc (WORD size, WORD *selector)
{
  union REGS r;

  __dpmi_errno = 0;
  EAX_REG(r) = 0x0100;           /* DPMI allocate DOS memory */
  EBX_REG(r) = (size + 15) / 16; /* Number of paragraphs requested */
  int386 (0x31, &r, &r);
  if (CARRY(r))
  {
    __dpmi_errno = AX_REG(r);
    return (0);
  }
  *selector = DX_REG(r);         /* Return selector */
  return AX_REG(r);              /* Return segment address */
}

int dpmi_real_free (WORD selector)
{
  union REGS r;

  __dpmi_errno = 0;
  EAX_REG(r) = 0x101;            /* DPMI free DOS memory */
  EBX_REG(r) = selector;         /* Selector to free */
  int386 (0x31, &r, &r);
  if (CARRY(r))
  {
    __dpmi_errno = AX_REG(r);
    return (0);
  }
  return (1);
}

int dpmi_lock_region (void *address, unsigned length)
{
  union REGS r;
  DWORD base, linear;

  if (!dpmi_get_base_address(MY_CS(),&base))
     return (0);

  linear = base + (DWORD)address;
  __dpmi_errno = 0;

  EAX_REG(r) = 0x600;            /* DPMI Lock Linear Region */
  EBX_REG(r) = (linear >> 16);   /* Linear address in BX:CX */
  ECX_REG(r) = (linear & 0xFFFF);
  ESI_REG(r) = (length >> 16);   /* Length in SI:DI */
  EDI_REG(r) = (length & 0xFFFF);
  int386 (0x31, &r, &r);
  if (CARRY(r))
  {
    __dpmi_errno = AX_REG(r);
    return (0);
  }
  return (1);
}

int dpmi_unlock_region (void *address, unsigned length)
{
  union REGS r;
  DWORD base, linear;

  if (!dpmi_get_base_address(MY_CS(),&base))
     return (0);

  linear = base + (DWORD)address;
  __dpmi_errno = 0;

  EAX_REG(r) = 0x601;            /* DPMI Unlock Linear Region */
  EBX_REG(r) = (linear >> 16);   /* Linear address in BX:CX */
  ECX_REG(r) = (linear & 0xFFFF);
  ESI_REG(r) = (length >> 16);   /* Length in SI:DI */
  EDI_REG(r) = (length & 0xFFFF);
  int386 (0x31, &r, &r);
  if (CARRY(r))
  {
    __dpmi_errno = AX_REG(r);
    return (0);
  }
  return (1);
}

#if !defined(__CCDL__)
int dpmi_real_interrupt (int intr, IREGS *reg)
{
  union  REGS  r;
  struct SREGS s;

  __dpmi_errno = 0;
  memset (&r, 0, sizeof(r));
  segread (&s);
  EAX_REG(r) = 0x300;
  EBX_REG(r) = intr;
  ECX_REG(r) = 0;
  s.es       = FP_SEG (reg);
  EDI_REG(r) = FP_OFF (reg);
  reg->r_flags = 0;
  reg->r_ss = reg->r_sp = 0;     /* DPMI host provides stack */

  int386x (0x31, &r, &r, &s);
  if (CARRY(r))
  {
    reg->r_flags |= 1;
    __dpmi_errno = AX_REG(r);
    return (0);
  }
  return (1);
}
#endif

int dpmi_real_call_retf (IREGS *reg)
{
  union  REGS  r;
  struct SREGS s;

  __dpmi_errno = 0;
  memset (&r, 0, sizeof(r));
  segread (&s);
  EAX_REG(r) = 0x301;
  EBX_REG(r) = 0;
  ECX_REG(r) = 0;
  s.es       = FP_SEG (reg);
  EDI_REG(r) = FP_OFF (reg);
  reg->r_flags = 0;
  reg->r_ss = reg->r_sp = 0;     /* DPMI host provides stack */

  int386x (0x31, &r, &r, &s);
  if (CARRY(r))
  {
    reg->r_flags |= 1;
    __dpmi_errno = AX_REG(r);
    return (0);
  }
  return (1);
}

/*
 * PowerPak already have int386().
 */
#if (defined(DMC386) || defined(MSC386) || defined(BORLAND386)) && \
    !(DOSX & POWERPAK)
static int real_interrupt (int intr, IREGS *reg)
{
  __asm {
      push ebx
      push edi
      mov  edi, reg        /* es:edi -> reg */
      mov  edx, esp        /* save esp */
      mov  ebx, intr
      mov  eax, 300h       /* simulate real-int */
      int  31h
      mov  esp, edx
      xor  eax, eax        /* assume ok */
      pop  edi
      pop  ebx
      jc   fail
      pop  ebp
      ret
  }
fail:
  return (-1);
}

int int386 (int intno, union REGS *ireg, union REGS *oreg)
{
  struct SREGS sreg;

  segread (&sreg);
  return int386x (intno, ireg, oreg, &sreg);
}

int int386x (int intno, union REGS *ireg, union REGS *oreg, struct SREGS *sreg)
{
  static IREGS rm_reg;

  rm_reg.r_ss = rm_reg.r_sp = rm_reg.r_flags = 0;
  rm_reg.r_ax = ireg->x.eax;
  rm_reg.r_bx = ireg->x.ebx;
  rm_reg.r_cx = ireg->x.ecx;
  rm_reg.r_dx = ireg->x.edx;
  rm_reg.r_si = ireg->x.esi;
  rm_reg.r_di = ireg->x.edi;
  rm_reg.r_ds = sreg->ds;
  rm_reg.r_es = sreg->es;
  rm_reg.r_gs = 0;     /* needed? */
  rm_reg.r_fs = 0;     /* needed? */

  if (real_interrupt(intno,&rm_reg) < 0)
  {
    oreg->x.cflag |= 1; /* Set carry bit */
    return (-1);
  }
  oreg->x.eax   = rm_reg.r_ax;
  oreg->x.ebx   = rm_reg.r_bx;
  oreg->x.ecx   = rm_reg.r_cx;
  oreg->x.edx   = rm_reg.r_dx;
  oreg->x.esi   = rm_reg.r_si;
  oreg->x.edi   = rm_reg.r_di;
  oreg->x.flags = rm_reg.r_flags;
  return (int)rm_reg.r_ax;
}

void segread (struct SREGS *sreg)
{
  sreg->ds = MY_DS();
  sreg->es = MY_ES();
  sreg->ss = MY_SS();
  sreg->cs = MY_CS();
}
#endif  /* (DMC386 || MSC386 || BORLAND386) && !(DOSX & POWERPAK) */


#if (DOSX & DOS4GW) && defined(WATCOM386)

#if defined(USE_EXCEPT_HANDLER)

extern int _STACKTOP;  /* internal variable, top of Watcom stack */

/*
 * Various contortions necessary to set CauseWay internal stack to
 * Watcom DGROUP-based stack and to keep the unknown number of stack
 * parameters relative to register EBP unchanged.
 */
extern void SS_TO_DGROUP (void);
#pragma aux SS_TO_DGROUP =          \
        "sub ebp, esp"              \
        "xor eax, eax"              \
        "mov ax, ss"                \
        "mov es, ax"                \
        "mov ebx, esp"              \
        "mov edx, _STACKTOP"        \
        "mov cx, ds"                \
        "mov ss, cx"                \
        "mov esp, edx"              \
        "mov edx, esi"              \
        "mov edi, esi"              \
        "sub edi, ebx"              \
        "sub edx, 2"                \
        "looper: mov cx, es:[edx]"  \
        "sub esp, 2"                \
        "mov ss:[esp], cx"          \
        "sub edx, 2"                \
        "sub edi, 2"                \
        "jg  looper"                \
        "add ebp, esp"              \
        "push eax"                  \
        "push esi"                  \
        modify [eax ebx ecx edx edi];

extern void RESTORE_SS_AND_GO (void);
#pragma aux RESTORE_SS_AND_GO = \
        "pop esi"               \
        "pop eax"               \
        "sub esi, 8"            \
        "mov ss, ax"            \
        "mov esp, esi"          \
        "retf";

/*
 * Termination routine MUST NOT have stack-checking enabled.
 * If virtual memory is in use, this routine and all code and data called
 * or used by the routine (including Watcom runtime library functions)
 * must be locked in memory.
 */
#include "nochkstk.h"

static void far __loadds exception_main (void)
{
  SS_TO_DGROUP();

  exc_struct.mode = (exc_struct.cs != MY_CS());

  if (user_exc_hook)
       (*user_exc_hook) (&exc_struct);
  else _printk ("Exception %d at CS:EIP %04X:%08X\n)",
                exc_struct.fault_num, exc_struct.cs, exc_struct.eip);
  RESTORE_SS_AND_GO();
}
#endif /* USE_EXCEPT_HANDLER */


/*! \todo find the real application start
 */
static DWORD exc_app_start = 4096UL;

/*
 * Perform a stack trace back to CRT. Function arguments printed only
 * for stack-based calls (-3s).
 *
 * \todo Merge this function with the one in x32vm.c.
 */
void stack_rewind (DWORD start, DWORD base)
{
  int   loop  = 10;                           /* print max 10 stack frames */
  DWORD eip   = start;
  DWORD limit = get_cs_limit();
  const DWORD *context = (const DWORD*) base; /* EBP increases as we rewind */
  BOOL  reg_calls = FALSE;

#if defined(__SW_3R)
  reg_calls = TRUE;
#endif

  _printk ("Stack trace:\r\n");

  while (context >= (DWORD*)base && ((DWORD)context & 3) == 0 && --loop)
  {
    const DWORD *next;

    if ((DWORD)context >= limit - 4 || eip >= limit ||
        (DWORD)context <= exc_app_start || eip <= exc_app_start)
       break;

    _printk ("  %08X ", eip);

    next = (const DWORD*) *context;  /* get ESP stack value from context */

    /* Compute number of args to display
     */
    if (next == NULL ||              /* we're rewound back to CRT */
        next <= (const DWORD*)exc_app_start || reg_calls)
       _printk ("( *** )");

    else
    {
      const DWORD *argv;
      DWORD        argc = next - context - 2;

      argc = (argc > 5) ? 5 : argc;
      _printk ("(");

      argv = context + 2; /* Args start after saved EBP and return address */
      while (argc--)
      {
        if (argc == 0)
             _printk ("%04X)\r\n", *argv++);
        else _printk ("%04X ",     *argv++);
      }
    }
    eip     = *(context+1);  /* get next return address */
    context = next;
  }
  _printk ("\r\n");
}
#endif  /* (DOSX & DOS4GW) && WATCOM386 */
#endif  /* DOSX & (DOS4GW|POWERPAK) */


#if defined(WATCOM386)
BOOL dpmi_init (void)
{
  switch (_Extender)
  {
    case DOSX_PHAR_V2:  /* Test for Pharlap extenders */
    case DOSX_PHAR_V3:
    case DOSX_PHAR_V4:
    case DOSX_PHAR_V5:
    case DOSX_PHAR_V6:
    case DOSX_PHAR_V7:
    case DOSX_PHAR_V8:
#if (DOSX & PHARLAP)
         return (TRUE);
#else
         outsnl (_LANG("\7The Watt-32 library was not compiled for Pharlap."));
         return (FALSE);

    case DOSX_RATIONAL:
         if (_ExtenderSubtype & XS_RATIONAL_NONZEROBASE)
         {
           outsnl (_LANG("\7Only zero-based DOS4GW applications supported."));
           return (FALSE);
         }
         break;  /* okay */
#endif

    default:
#if (DOSX & PHARLAP)
         outsnl (_LANG("\7Only Pharlap extenders supported"));
#else
         outsnl (_LANG("\7Only DOS4GW style extenders supported"));
#endif
         return (FALSE);
  }
  return (TRUE);
}
#endif  /* WATCOM386 */
#endif  /* __MSDOS__ */

