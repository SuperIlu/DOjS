#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>

#ifdef __HIGHC__
#include <mw/conio.h>  /* extension for Metaware's conio.h */
#endif

#include "keyb.h"
#include "screen.h"

#if !defined(__WATCOMC__)

static int ctrl_state;
static int last_ch;

static void print_it (char s1, char s2)
{
  char buf[3];

  SetColour (UserText);
  buf[0] = s1;
  buf[1] = s2;
  buf[2] = 0;
  cputs (buf);
}

static void process_it (char ch)
{
  if (ch == '^')                         /* possible '^C\r\n' ahead */
  {
    ctrl_state = 1;
  }
  else if (last_ch == '^' && ch != 'C')  /* no it wasn't, print it */
  {
    print_it ('^', ch);
    ctrl_state = 0;
  }
  else if (last_ch == '^' && ch == 'C')  /* yes, push it for conio */
  {
    KeyUngetKey (3);
  }
  else if (!ctrl_state)                  /* normal state, print it */
  {
    print_it (ch, 0);
  }
  else if (ch == '\n')                   /* ctrl-state ended */
  {
    ctrl_state = 0;
  }
  last_ch = ch;
}
#endif /* __WATCOMC__ */

#ifdef __HIGHC__
  #include <pharlap.h>
  #include <mw/exc.h>

  static REALPTR int29_old, rm_cb;

  static void int29_isr (SWI_REGS *reg)
  {
    process_it (reg->eax & 255);
  }

  void int29_exit (void)
  {
    if (!rm_cb)
       return;
    _dx_rmiv_set (0x29, int29_old);
    _dx_free_rmode_wrapper (rm_cb);
    rm_cb = 0;
  }

  void int29_init (void)
  {
    if (int29_old)
       return;
    _dx_rmiv_get (0x29, &int29_old);
    rm_cb = _dx_alloc_rmode_wrapper_iret (int29_isr, 40000);
    if (!rm_cb)
       return;
    _dx_rmiv_set (0x29, rm_cb);
    atexit (int29_exit);
  }

#elif defined(__DJGPP__)
  #include <dpmi.h>
  #include <go32.h>

  static _go32_dpmi_seginfo rm_cb, int29_old;
  static __dpmi_regs        rm_reg;

  static void int29_isr (void)
  {
    process_it (rm_reg.h.al);
  }

  void int29_exit (void)
  {
    if (!rm_cb.pm_offset)
       return;
    _go32_dpmi_set_real_mode_interrupt_vector (0x29, &int29_old);
    _go32_dpmi_free_real_mode_callback (&rm_cb);
    rm_cb.pm_offset = 0;
  }

  void int29_init (void)
  {
    if (int29_old.rm_segment)
       return;
    _go32_dpmi_get_real_mode_interrupt_vector (0x29, &int29_old);
    rm_cb.pm_offset = (unsigned long) &int29_isr;
    if (_go32_dpmi_allocate_real_mode_callback_iret(&rm_cb,&rm_reg) ||
        _go32_dpmi_lock_data(&rm_reg,sizeof(rm_reg)))
      return;

    _go32_dpmi_set_real_mode_interrupt_vector (0x29, &rm_cb);
    atexit (int29_exit);
  }

/*---------------------------------------------------------------------*/

#elif defined(__WATCOMC__)
  void int29_exit (void) {}
  void int29_init (void) {}

#else   /* Turbo/Borland C */

  static void interrupt (*int29_old)(void);
  #pragma argsused

  static void interrupt int29_isr (bp,di,si,ds,es,dx,cx,bx,ax,ip,cs,flags)
  {
    process_it (ax & 255);
  }

  void int29_exit (void)
  {
    if (!int29_old)
       return;
    setvect (0x29, int29_old);
    int29_old = 0;
  }

  void int29_init (void)
  {
    if (int29_old)
       return;
    int29_old = getvect (0x29);
    setvect (0x29, int29_isr);
    atexit (int29_exit);
  }
#endif

