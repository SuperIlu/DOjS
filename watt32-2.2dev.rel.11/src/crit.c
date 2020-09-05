/*!\file crit.c
 * Watt-32 critical error (int24h).
 */

/* Targets:
 *    Metaware HighC / PharLap
 *    WatcomC (real/prot mode)  \todo finish suport for Watcom
 *    GNU C / djgpp2
 *    real-mode DOS targets
 *
 *  Copyright (c) 1997-2002 Gisle Vanem <gvanem@yahoo.no>
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. All advertising materials mentioning features or use of this software
 *     must display the following acknowledgement:
 *       This product includes software developed by Gisle Vanem
 *       Bergen, Norway.
 *
 *  THIS SOFTWARE IS PROVIDED BY ME (Gisle Vanem) AND CONTRIBUTORS ``AS IS''
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL I OR CONTRIBUTORS BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "wattcp.h"
#include "language.h"
#include "x32vm.h"
#include "misc.h"

#if defined(NOT_USED_ANYMORE) || 1

#if (DOSX & PHARLAP)
#include <mw/exc.h>
#endif

#include "nochkstk.h"     /* disable stack-checking here */

#if defined(__HIGHC__)
  #pragma off(Call_trace)
  #pragma off(Prolog_trace)
  #pragma off(Epilog_trace)
#endif

#if (DOSX & (PHARLAP|X32VM))
  static REALPTR int24_old, rmcb;

  static void cdecl int24_isr (SWI_REGS *reg)
  {
    reg->r_ax = 3;  /* simply fail the function */
  }                 /* note: this only works on DOS 3.0 and above */

  void int24_restore (void)
  {
    if (!rmcb)
       return;
    _dx_ulock_pgsn (&int24_isr, 20);
    _dx_rmiv_set (CRIT_VECT, int24_old);
    _dx_free_rmode_wrapper (rmcb);
    rmcb = 0;
  }

  void int24_init (void)
  {
    if (_watt_os_ver < 0x300)
       return;

    _dx_rmiv_get (CRIT_VECT, &int24_old);
    rmcb = _dx_alloc_rmode_wrapper_iret (int24_isr, 256);

    if (!rmcb)
       return;

    _dx_lock_pgsn (&int24_isr, 20);
    _dx_rmiv_set (CRIT_VECT, rmcb);
    RUNDOWN_ADD (int24_restore, 20);
  }

/*---------------------------------------------------------------------*/

#elif (defined (__DMC__) && !defined(DMC386)) || defined(__WATCOMC__)
  static int _far int24_isr (unsigned dev_err, unsigned err_code,
                             unsigned _far *dev_hdr)
  {
    ARGSUSED (dev_err);
    ARGSUSED (dev_hdr);
    ARGSUSED (err_code);
    return (_HARDERR_FAIL);
  }

  void int24_init (void)
  {
    if (_watt_os_ver >= 0x300)
      _harderr (int24_isr);
  }

/*---------------------------------------------------------------------*/

#elif (DOSX & DJGPP)
  static _go32_dpmi_seginfo rm_cb, int24_old;
  static __dpmi_regs        rm_reg;

  static void int24_isr (void)
  {
    rm_reg.x.ax = 3;
  }

  void int24_restore (void)
  {
    if (int24_old.rm_segment)
    {
      _go32_dpmi_set_real_mode_interrupt_vector (CRIT_VECT, &int24_old);
      _go32_dpmi_free_real_mode_callback (&rm_cb);
      int24_old.rm_segment = 0;
    }
  }

  void int24_init (void)
  {
    if (_watt_os_ver < 0x300)
       return;

    _go32_dpmi_get_real_mode_interrupt_vector (CRIT_VECT, &int24_old);
    rm_cb.pm_offset = (DWORD) int24_isr;
    if (_go32_dpmi_allocate_real_mode_callback_iret(&rm_cb,&rm_reg))
       return;

    _go32_dpmi_set_real_mode_interrupt_vector (CRIT_VECT, &rm_cb);
    RUNDOWN_ADD (int24_restore, 20);
  }

/*---------------------------------------------------------------------*/

#elif DOSX == 0      /* real-mode targets */
  static W32_IntrHandler int24_old;

  static INTR_PROTOTYPE int24_isr (bp,di,si,ds,es,dx,cx,bx,ax,ip,cs,flags)
  {
    ax = 3;
    ARGSUSED (bp); ARGSUSED (di); ARGSUSED (si); ARGSUSED (ds);
    ARGSUSED (es); ARGSUSED (dx); ARGSUSED (cx); ARGSUSED (bx);
    ARGSUSED (ax); ARGSUSED (ip); ARGSUSED (cs); ARGSUSED (flags);
  }

  void int24_restore (void)
  {
    if (int24_old)
       setvect (CRIT_VECT, int24_old);
    int24_old = NULL;
  }

  void int24_init (void)
  {
    if (_watt_os_ver >= 0x300)
    {
      int24_old = getvect (CRIT_VECT);
      setvect (CRIT_VECT, int24_isr);
      RUNDOWN_ADD (int24_restore, 20);
    }
  }
#endif

#endif /* NOT_USED_ANYMORE */
