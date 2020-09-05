/*!\file netback.c
 * \deprecated Background ISR.
 */
#include <stdio.h>
#include <stdlib.h>

#include "wattcp.h"
#include "language.h"
#include "strings.h"
#include "pctcp.h"
#include "wdpmi.h"
#include "misc.h"

#define TIMER_INTR 8
#define STK_SIZE   1024


#if (DOSX)
  void backgroundon (void)
  {
    outsnl (_LANG("Use wintr_init() / wintr_enable() instead"));
    exit (3);
  }

#elif !defined(NO_INLINE_ASM)  /* MSC <=6 unsupported */

  static void (*user_routine)(void) = NULL;
  static int inside = 0;

  static W32_IntrHandler oldinterrupt;

  static INTR_PROTOTYPE newinterrupt (void)
  {
    (*oldinterrupt)();
    DISABLE();
    if (inside)
    {
      static UINT tempstack [STK_SIZE];
  #ifdef __WATCOMC__
      STACK_SET (&tempstack[STK_SIZE-1]);
  #else
        static UINT old_SP;
        static WORD old_SS;
        asm  mov ax, ss
        asm  mov old_SS, ax
        asm  mov ax, sp
        asm  mov old_SP, ax
        asm  mov ax, ds
        asm  mov ss, ax
        asm  lea sp, tempstack[STK_SIZE-1]

  #endif
      ENABLE();

      if (user_routine)
        (*user_routine)();
      tcp_tick (NULL);

      DISABLE();

  #ifdef __WATCOMC__
      STACK_RESTORE();
  #else
      asm  mov ax, old_SS
      asm  mov ss, ax
      asm  mov ax, old_SP
      asm  mov sp, ax

  #endif
      inside = 0;
    }
    ENABLE();
  }

  void backgroundon (void)
  {
    oldinterrupt = getvect (TIMER_INTR);
    setvect (TIMER_INTR, newinterrupt);
  }

  void backgroundoff (void)
  {
    setvect (TIMER_INTR, oldinterrupt);
  }

  void backgroundfn (void (*fn)(void))
  {
    user_routine = fn;
  }
#endif  /* DOSX */
