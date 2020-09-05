/*!\file wdpmi.h
 */
#ifndef _w32_WDPMI_H
#define _w32_WDPMI_H

#if defined(WATCOM386)
  extern BOOL dpmi_init (void);
#endif

#if (DOSX & (DOS4GW|POWERPAK))

  #include "misc.h"        /* IREGS */
  #include <sys/pack_on.h>

  struct FAULT_STRUC {   /* Exception structure (only used by CauseWay) */
         DWORD  ebp, edi, esi, edx, ecx, ebx, eax;
         WORD   gs, fs, es, ds;
         DWORD  eip;
         WORD   cs, reserved1;
         DWORD  eflags, esp;
         WORD   ss, reserved2;
         WORD   tr;
         DWORD  cr0, cr1, cr2, cr3;
         DWORD  csAddr;
         DWORD  dsAddr;
         DWORD  esAddr;
         DWORD  fsAddr;
         DWORD  gsAddr;
         DWORD  ssAddr;
         WORD   fault_num;
         DWORD  code;
         BOOL   mode;  /* 0: exception in pmode, 1: real-mode */
       };

  #include <sys/pack_off.h>

  #define SEG_OFS_TO_LIN(seg,ofs) (void*)(((WORD)(seg) << 4) + (WORD)(ofs))

  extern WORD __dpmi_errno;

  extern WORD  dpmi_real_malloc     (WORD size, WORD *sel);
  extern int   dpmi_real_free       (WORD selector);
  extern int   dpmi_get_base_address(WORD sel, DWORD *base);
  extern int   dpmi_lock_region     (void *address, unsigned length);
  extern int   dpmi_unlock_region   (void *address, unsigned length);
  extern void *dpmi_get_real_vector (int intr);
  extern int   dpmi_real_interrupt  (int intr, IREGS *reg);
  extern int   dpmi_real_call_retf  (IREGS *reg);
#endif

#if (DOSX & DOS4GW)
  extern BOOL  dpmi_is_wdosx    (void);
  extern BOOL  dpmi_is_pmodew   (void);
  extern BOOL  dpmi_is_dos32a   (void);
  extern BOOL  dpmi_is_dos4gw   (void);
  extern BOOL  dpmi_is_causeway (void);
  extern BOOL  dpmi_is_hxdos    (void);

  typedef void (*exceptionHook) (const struct FAULT_STRUC *);

  extern int dpmi_except_handler (exceptionHook exc_func);
  extern const char *dos4gw_extender_name (void);

  #define stack_rewind  W32_NAMESPACE (stack_rewind)
  extern void stack_rewind (DWORD start, DWORD base);
#endif

#endif /* !_w32_WDPMI_H */

