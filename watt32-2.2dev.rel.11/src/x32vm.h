/*!\file x32vm.h
 */
#ifndef _w32_X32VM_H
#define _w32_X32VM_H

#if (DOSX & X32VM)   /* FlashTek's X32 extender */

#undef  REALPTR
#define REALPTR             DWORD  /* segment in upper 16 bits */
#define RP_SET(rp,ofs,seg)  (rp = ((DWORD)(seg) << 16) + (ofs))
#define RP_OFF(rp)          (WORD)((DWORD)(rp) & 0xFFFF)
#define RP_SEG(rp)          (WORD)((DWORD)(rp) >> 16)

#include <sys/pack_on.h>

typedef struct SWI_REGS {
        DWORD  r_ax;
        DWORD  r_bx;
        DWORD  r_cx;
        DWORD  r_dx;
        DWORD  r_si;
        DWORD  r_di;
        WORD   r_ds;
        WORD   r_es;
        WORD   r_fs;
        WORD   r_gs;
        DWORD  r_flags;
      } SWI_REGS;

typedef struct RMC_BLK {
        WORD   ds;
        WORD   es;
        WORD   fs;
        WORD   gs;
        DWORD  eax;
        DWORD  ebx;
        DWORD  ecx;
        DWORD  edx;
      } RMC_BLK;

typedef struct {
        WORD  inum;
        WORD  ds_reg, es_reg;
        WORD  fs_reg, gs_reg;
        DWORD eax_reg, edx_reg;
      } RMI_BLK;

typedef void (*pmodeHook) (SWI_REGS *);
typedef void (*rmodeHook) (void);

#include <sys/pack_off.h>

#define _dx_rmiv_get    W32_NAMESPACE (_dx_rmiv_get)
#define _dx_rmiv_set    W32_NAMESPACE (_dx_rmiv_set)
#define _dx_real_int    W32_NAMESPACE (_dx_real_int)
#define _dx_real_free   W32_NAMESPACE (_dx_real_free)
#define _dx_real_above  W32_NAMESPACE (_dx_real_above)
#define _dx_real_alloc  W32_NAMESPACE (_dx_real_alloc)
#define _dx_call_real   W32_NAMESPACE (_dx_call_real)
#define _dx_lock_pgs    W32_NAMESPACE (_dx_lock_pgs)
#define _dx_lock_pgsn   W32_NAMESPACE (_dx_lock_pgsn)
#define _dx_ulock_pgsn  W32_NAMESPACE (_dx_ulock_pgsn)
#define _dx_rmlink_get  W32_NAMESPACE (_dx_rmlink_get)

#define ReadRealMem     W32_NAMESPACE (ReadRealMem)
#define WriteRealMem    W32_NAMESPACE (WriteRealMem)
#define PokeRealWord    W32_NAMESPACE (PokeRealWord)
#define PeekRealWord    W32_NAMESPACE (PeekRealWord)
#define PeekRealDWord   W32_NAMESPACE (PeekRealDWord)
#define stack_rewind    W32_NAMESPACE (stack_rewind)

#define _dx_alloc_rmode_wrapper_retf W32_NAMESPACE (_dx_alloc_rmode_wrapper_retf)
#define _dx_alloc_rmode_wrapper_iret W32_NAMESPACE (_dx_alloc_rmode_wrapper_iret)
#define _dx_free_rmode_wrapper       W32_NAMESPACE (_dx_free_rmode_wrapper)

extern void _dx_rmiv_get  (int int_no, REALPTR *handler);
extern void _dx_rmiv_set  (int int_no, REALPTR handler);
extern void _dx_pmiv_get  (int int_no, FARPTR *handler);
extern void _dx_apmiv_set (int int_no, FARPTR handler);
extern void _dx_real_int  (int int_no, SWI_REGS *regs);
extern int  _dx_real_free (WORD segment);
extern int  _dx_real_above(int paras, WORD *para, WORD *largest);
extern int  _dx_real_alloc(int paras, WORD *para, WORD *largest);
extern int  _dx_call_real (REALPTR proc, RMC_BLK *rm_reg, DWORD count, ...);

extern int  _dx_lock_pgs   (const void _far *addr, int len);
extern int  _dx_lock_pgsn  (const void *addr, int len);
extern int  _dx_ulock_pgsn (const void *addr, int len);

extern void _dx_rmlink_get (REALPTR *cback, REALPTR *rm_cbuf,
                            DWORD *rm_cbuf_size, void _far **pm_cbuf);

extern REALPTR _dx_alloc_rmode_wrapper_retf (pmodeHook pmHook,
                                             rmodeHook rmHook,
                                             int len, int stack_size);

extern REALPTR _dx_alloc_rmode_wrapper_iret (pmodeHook pmHook,
                                             int stack_size);

extern void    _dx_free_rmode_wrapper (REALPTR dosAddr);

extern void  ReadRealMem  (void *buf, REALPTR rp, unsigned len);
extern void  WriteRealMem (REALPTR rp, const void *src, int size);
extern void  PokeRealWord (REALPTR rp, WORD val);
extern WORD  PeekRealWord (REALPTR rp);
extern DWORD PeekRealDWord(REALPTR rp);
extern void  stack_rewind (DWORD start, DWORD base);

extern UINT cdecl _x386_stacklow;  /* in FlashTek's x32.lib */

#endif   /* DOSX & X32VM */
#endif   /* _w32_X32VM_H */

