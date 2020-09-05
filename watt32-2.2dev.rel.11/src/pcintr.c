/*!\file pcintr.c
 * Background network ISR poller.
 *
 * Add interrupt based processing, improve performance
 * during disk slowdowns
 *
 * - wintr_init()     - call once
 * - wintr_shutdown() - called automatically
 * - wintr_enable()   - enable interrupt based calls
 * - wintr_disable()  - disable interrupt based calls (default)
 * - (*wintr_chain)() - a place to chain in your own calls, must live
 *                      point to a function with at least a 1K stack
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "wattcp.h"
#include "strings.h"
#include "language.h"
#include "misc.h"
#include "run.h"
#include "pctcp.h"
#include "wdpmi.h"
#include "x32vm.h"
#include "powerpak.h"

#if (DOSX & DJGPP)
  #include <sys/time.h>
#elif (DOSX & PHARLAP)
  #include <mw/exc.h>  /* _dx_alloc_rmode_wrapper_iret() */
#endif

#if defined(__MSDOS__)

#define TIMER_INTR  0x08
#define STK_SIZE    1024

void (W32_CALL *wintr_chain)(void) = NULL;

static BOOL on_isr8;

#if !(DOSX & POWERPAK)
  static BOOL inside_isr8 = FALSE;
  static WORD indos_flag_seg, indos_flag_ofs;
  static WORD crit_err_seg, crit_err_ofs;   /* only for DOS 3.1+ */

  static void find_DOS_flags (void);
  static BOOL DOS_is_idle (void);
#endif

#if (DOSX & DJGPP)
  static void (*old_sigalrm) (int);

#elif !(DOSX & (PHARLAP|X32VM|POWERPAK))
  static UINT locstack [STK_SIZE];

  static void stack_probe (void)
  {
#if defined(USE_DEBUG)
    int   used;
    UINT *p;

    for (p = locstack, used = 0; *p && used < DIM(locstack); p++)
        used++;
    if (used >= DIM(locstack))
    {
      (*_printf) ("pcintr.c: Stack overflow. Increase STK_SIZE\n");
      fflush (stdout);
      exit (-1);
    }
#endif
  }
#endif

BOOL wintr_enable (void)
{
  BOOL old = on_isr8;
  on_isr8 = TRUE;
  return (old);
}

void wintr_disable (void)
{
  on_isr8 = FALSE;
}

#if (DOSX & (PHARLAP|X32VM))
  static REALPTR oldint, timer_cb;
  static RMC_BLK rmRegs;

  static void NewTimer (void)
  {
    _dx_call_real (oldint, &rmRegs, 1);

    if (!inside_isr8)
    {
      inside_isr8 = TRUE;
      if (on_isr8 && DOS_is_idle())
      {
        if (wintr_chain)
          (*wintr_chain)();
        tcp_tick (NULL);
      }
      inside_isr8 = FALSE;
    }
  }

  void wintr_shutdown (void)
  {
    if (oldint)
    {
      _dx_rmiv_set (TIMER_INTR, oldint);
      _dx_free_rmode_wrapper (timer_cb);
      oldint = 0;
    }
  }

  void wintr_init (void)
  {
    timer_cb = _dx_alloc_rmode_wrapper_iret ((pmodeHook)NewTimer, 80000);
    if (!timer_cb)
    {
      outsnl (_LANG("Cannot allocate real-mode timer callback"));
      exit (1);
    }
    find_DOS_flags();
    _dx_rmiv_get (TIMER_INTR, &oldint);
    _dx_rmiv_set (TIMER_INTR, timer_cb);
    RUNDOWN_ADD (wintr_shutdown, -1); /* call it very early */
  }

#elif (DOSX & DJGPP)
  static void NewTimer (int sig)
  {
    if (!inside_isr8)
    {
      inside_isr8 = TRUE;
      if (old_sigalrm)
        (*old_sigalrm) (sig);
      if (on_isr8 && DOS_is_idle())
      {
        if (wintr_chain)
          (*wintr_chain)();
        tcp_tick (NULL);
      }
      inside_isr8 = FALSE;
    }
  }

  void wintr_shutdown (void)
  {
    struct itimerval tim;

    tim.it_interval.tv_sec  = 0;
    tim.it_interval.tv_usec = 0;
    signal (SIGALRM, old_sigalrm);
    setitimer (ITIMER_REAL, &tim, NULL);
  }

  void wintr_init (void)
  {
    struct itimerval tim;

    find_DOS_flags();
    tim.it_interval.tv_usec = 54945;  /* 1000000/18.2 */
    tim.it_interval.tv_sec  = 0;
    tim.it_value = tim.it_interval;
    old_sigalrm = signal (SIGALRM, NewTimer);
    setitimer (ITIMER_REAL, &tim, NULL);
  }

#elif defined(WATCOM386) && (DOSX & DOS4GW)
  static void (__interrupt __far *oldint)();

  static void __interrupt __far NewTimer (void)
  {
    if (!inside_isr8)
    {
      inside_isr8 = TRUE;
      if (on_isr8 && DOS_is_idle())
      {
        DISABLE();
        STACK_SET (&locstack[STK_SIZE-1]);

        if (wintr_chain)
          (*wintr_chain)();
        tcp_tick (NULL);

        STACK_RESTORE();
        ENABLE();
        stack_probe();
      }
      inside_isr8 = FALSE;
    }
    _chain_intr (oldint);
  }

  void wintr_shutdown (void)
  {
    if (oldint)
    {
      _dos_setvect (TIMER_INTR, oldint);
      oldint = NULL;
    }
  }

  void wintr_init (void)
  {
    find_DOS_flags();
    oldint = _dos_getvect (TIMER_INTR);
    _dos_setvect (TIMER_INTR, NewTimer);
    memset (&locstack, 0, sizeof(locstack));
    RUNDOWN_ADD (wintr_shutdown, -1);
  }

#elif (DOSX & POWERPAK) && defined(NOT_YET)
  static struct {
         DWORD  offset;
         WORD   selector;
       } oldint;

  static WORD our_ds;

  void NewTimerPre (void);

  static void NewTimer (void)
  {
    __asm {
      push es
      push ds
      mov  ds, cs:[our_ds]
      mov  es, cs:[our_ds]
    }

    if (!inside_isr8)
    {
      inside_isr8 = TRUE;
      if (on_isr8 && DOS_is_idle())
      {
        DISABLE();
        STACK_SET (offset locstack[STK_SIZE-1]);

        if (wintr_chain)
          (*wintr_chain)();
        tcp_tick (NULL);

        STACK_RESTORE();
        ENABLE();
        stack_probe();
      }
      inside_isr8 = FALSE;
    }
    dpmi_chain_intr (oldint.selector, oldint.offset);
  }

  void wintr_shutdown (void)
  {
    if (oldint.selector)
    {
      dpmi_setvect (TIMER_INTR, oldint.selector, oldint.offset);
      oldint.selector = NULL;
    }
  }

  void wintr_init (void)
  {
    find_DOS_flags();
    our_ds = _DS;
    dpmi_getvect (TIMER_INTR, &oldint.selector, &oldint.offset);
    dpmi_setvect (TIMER_INTR, _CS, (DWORD)NewTimer);
    memset (&locstack, 0, sizeof(locstack));
    RUNDOWN_ADD (wintr_shutdown, -1);
  }

#elif (DOSX == 0) && !defined(NO_INLINE_ASM)

  static W32_IntrHandler oldint;

  static INTR_PROTOTYPE NewTimer (void)
  {
    (*oldint)();    /* chain now */

    if (!inside_isr8)
    {
      inside_isr8 = TRUE;

      if (on_isr8 && DOS_is_idle())
      {
  #ifdef __WATCOMC__
        DISABLE();
        STACK_SET (&locstack[STK_SIZE-1]);
  #else
        static UINT old_SP;
        static WORD old_SS;
        asm  pushf
        asm  cli
        asm  mov ax, ss
        asm  mov old_SS, ax
        asm  mov ax, sp
        asm  mov old_SP, ax
        asm  mov ax, ds
        asm  mov ss, ax
        asm  lea sp, locstack[STK_SIZE-1]
  #endif
        ENABLE();

        if (wintr_chain)
          (*wintr_chain)();
        tcp_tick (NULL);

        DISABLE();
  #ifdef __WATCOMC__
        STACK_RESTORE();
  #else
        asm  mov ax,old_SS
        asm  mov ss,ax
        asm  mov ax,old_SP
        asm  mov sp,ax
  #endif
        ENABLE();
        stack_probe();
      }
      inside_isr8 = FALSE;
    }
  }

  void wintr_shutdown (void)
  {
    if (oldint)
    {
      setvect (TIMER_INTR, oldint);
      oldint = NULL;
    }
  }

  void wintr_init (void)
  {
    find_DOS_flags();
    oldint = getvect (TIMER_INTR);
    setvect (TIMER_INTR, NewTimer);
    memset (&locstack, 0, sizeof(locstack));
    RUNDOWN_ADD (wintr_shutdown, -1);
  }
#endif

#if !(DOSX & POWERPAK)
static void find_DOS_flags (void)
{
  IREGS regs;

  memset (&regs, 0, sizeof(regs));
  regs.r_ax = 0x3400;

  GEN_INTERRUPT (0x21, &regs);
  indos_flag_seg = regs.r_es;
  indos_flag_ofs = regs.r_bx;
  if (_watt_os_ver >= 0x0301)
  {
    crit_err_seg = regs.r_es;
    crit_err_ofs = regs.r_bx - 1;
  }
}

static BOOL DOS_is_idle (void)
{
  BYTE indos, crit = 0;

  indos = PEEKB (indos_flag_seg, indos_flag_ofs);
  if (_watt_os_ver >= 0x0301)
     crit = PEEKB (crit_err_seg, crit_err_ofs);
  return (crit == 0 && indos == 0);
}
#endif
#endif  /* __MSDOS__ */

