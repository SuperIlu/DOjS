/*!\file powerpak.h
 */
#ifndef _w32_POWERPAK_H
#define _w32_POWERPAK_H

#if (DOSX & POWERPAK) && defined(BORLAND386)

#undef  REALPTR
#define REALPTR             DWORD  /* segment in upper 16 bits */
#define RP_SET(rp,ofs,seg)  (rp = ((DWORD)(seg) << 16) + (ofs))
#define RP_OFF(rp)          (WORD)((DWORD)(rp) & 0xFFFF)
#define RP_SEG(rp)          (WORD)((DWORD)(rp) >> 16)
#define PokeRealWord(rp,x)  WriteRealMem (rp, &(x), sizeof(WORD))

#include <sys/pack_on.h>

typedef struct {
        DWORD  r_di;
        DWORD  r_si;
        DWORD  r_bp;
        DWORD  reserved;
        DWORD  r_bx;
        DWORD  r_dx;
        DWORD  r_cx;
        DWORD  r_ax;
        WORD   r_flags;
        WORD   r_es, r_ds, r_fs, r_gs;
        WORD   r_ip, r_cs, r_sp, r_ss;
      } REAL_regs;

#include <sys/pack_off.h>

typedef struct {
        void    (*pm_func)(void);
        BYTE     *wrapper;
        REALPTR   rm_addr;
        REAL_regs rm_regs;
      } DPMI_callback;

extern BOOL    powerpak_init (void);

extern WORD    dpmi_create_dos_selector (void);
extern int     dpmi_free_dos_selector   (WORD sel);
extern int     dpmi_delete_memory_alias (void);
extern DWORD   dpmi_create_memory_alias (DWORD addr, DWORD len);
extern REALPTR dpmi_alloc_callback_retf (DPMI_callback *cb);
extern int     dpmi_free_callback_retf  (DPMI_callback *cb);

extern int     dpmi_getvect (int intr, WORD *sel, DWORD *ofs);
extern int     dpmi_setvect (int intr, WORD  sel, DWORD  ofs);

extern void    ReadRealMem  (void *buf, REALPTR rp, unsigned len);
extern void    WriteRealMem (REALPTR rp, const void *src, int size);

extern BYTE   _farpeekb (WORD sel, DWORD ofs);
extern WORD   _farpeekw (WORD sel, DWORD ofs);
extern DWORD  _farpeekl (WORD sel, DWORD ofs);
extern void   _farpokeb (WORD sel, DWORD ofs, BYTE val);
extern void   _farpokew (WORD sel, DWORD ofs, WORD val);
extern void   _farpokel (WORD sel, DWORD ofs, DWORD val);

#endif
#endif
