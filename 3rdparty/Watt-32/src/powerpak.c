/*!\file powerpak.c
 *
 *  A djgpp-like DOS memory interface for Borland's DOS extender (32rtm).
 *  Only Borland's BCC32 v4.5+ supported.
 *
 *  by G. Vanem <gvanem@yahoo.no>  Nov 2002
 *
 *  Running PowerPak under plain DOS, the 32rtm extender tries to emulate
 *  a subset of the Win32 API. Running the same app. under Win32 uses the
 *  real Win32 API (from kernel32.dll etc.). Therefore we cannot use int386()
 *  etc., but must use native calls. This isn't finished yet (if at all
 *  possible).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wattcp.h"
#include "printk.h"
#include "misc.h"
#include "wdpmi.h"
#include "powerpak.h"

#if (DOSX & POWERPAK) && defined(BORLAND386)

extern int _ostype;

/* Don't include <windows.h> because of conflicting types etc.
 */
#define WINAPI __stdcall __import

extern void * WINAPI GetModuleHandleA (const char *module);
extern void * WINAPI GetProcAddress (void *module, const char *func);

/* these are missing in Win32, damn...
 */
static UINT (WINAPI* _AllocSelector) (UINT);
static UINT (WINAPI* _FreeSelector) (UINT);

BOOL powerpak_init (void)
{
  void *mod;

  if (_ostype == 2)  /* plain DOS */
     return (TRUE);

  mod = GetModuleHandleA ("kernel32.dll");
  if (!mod)
     return (FALSE);

  _AllocSelector = (UINT (WINAPI*)(UINT)) GetProcAddress (mod, "AllocSelector");
  _FreeSelector  = (UINT (WINAPI*)(UINT)) GetProcAddress (mod, "FreeSelector");
  return (_AllocSelector && _FreeSelector);
}

WORD dpmi_create_dos_selector (void)
{
  WORD sel;

  if (_ostype != 2)   /* Not plain DOS */
     return (*_AllocSelector) (0);

  __dpmi_errno = 0;
  _EAX = 0;       /* DPMI 0: create selector */
  _ECX = 1;
  geninterrupt (0x31);
  if (_FLAGS & CARRY_BIT)
  {
    __dpmi_errno = _AX;
    return (0);
  }
  sel = _AX;
  _EAX = 8;       /* DPMI 8: set limit 1MByte */
  _BX = sel;
  _CX = 0x0F;
  _DX = 0xFFFF;
  geninterrupt (0x31);
  if (_FLAGS & CARRY_BIT)
  {
    __dpmi_errno = _AX;
    return (0);
  }
  return (sel);
}

int dpmi_free_dos_selector (WORD sel)
{
  if (_ostype != 2)  /* Not plain DOS */
     return (*_FreeSelector) (sel);

  __dpmi_errno = 0;
  _EAX = 1;
  _BX  = sel;
  geninterrupt (0x31);
  if (_FLAGS & CARRY_BIT)
  {
    __dpmi_errno = _AX;
    return (0);
  }
  return (1);
}

void ReadRealMem (void *dst, REALPTR rp, unsigned size)
{
  DWORD lin = (RP_SEG(rp) << 4) + RP_OFF(rp);

  __asm {
    push ds
    push edi
    push esi
    mov  ds, _watt_dos_ds
    mov  esi, lin
    mov  edi, dst
    mov  ecx, size
    shr  ecx, 2
    cld
    db   0F3h, 0A5h   /* rep movsd */
    mov  ecx, size
    and  ecx, 3
    rep  movsb        /* move rest */
    pop  esi
    pop  edi
    pop  ds
  }
}

void WriteRealMem (REALPTR rp, const void *src, int size)
{
  DWORD lin = (RP_SEG(rp) << 4) + RP_OFF(rp);

  __asm {
    push es
    push edi
    push esi
    mov  es, _watt_dos_ds
    mov  edi, lin
    mov  esi, src
    mov  ecx, size
    shr  ecx, 2
    cld
    db   0F3h, 0A5h   /* rep movsd */
    mov  ecx, size
    and  ecx, 3
    rep  movsb        /* move rest */
    pop  esi
    pop  edi
    pop  es
  }
}

BYTE _farpeekb (WORD sel, DWORD ofs)
{
  __asm {
    push es
    mov  ax, sel
    mov  edx, ofs
    mov  es, ax
    mov  al, es:[edx]
    pop  es
  }
  return (_AL);
}

WORD _farpeekw (WORD sel, DWORD ofs)
{
  __asm {
    push es
    mov  ax, sel
    mov  edx, ofs
    mov  es, ax
    mov  ax, es:[edx]
    pop  es
  }
  return (_AX);
}

DWORD _farpeekl (WORD sel, DWORD ofs)
{
  __asm {
    push es
    mov  ax, sel
    mov  edx, ofs
    mov  es, ax
    mov  eax, es:[edx]
    pop  es
  }
  return (_EAX);
}

void _farpokeb (WORD sel, DWORD ofs, BYTE val)
{
  __asm {
    push es
    mov  ax, sel
    mov  edx, ofs
    mov  es, ax
    mov  al, val
    mov  es:[edx], al
    pop  es
  }
}

void _farpokew (WORD sel, DWORD ofs, WORD val)
{
  __asm {
    push es
    mov  ax, sel
    mov  edx, ofs
    mov  es, ax
    mov  ax, val
    mov  es:[edx], ax
    pop  es
  }
}

void _farpokel (WORD sel, DWORD ofs, DWORD val)
{
  __asm {
    push es
    mov  ax, sel
    mov  edx, ofs
    mov  es, ax
    mov  eax, val
    mov  es:[edx], eax
    pop  es
  }
}

int dpmi_getvect (int intr, WORD *sel, DWORD *ofs)
{
  __asm {
    push ebx
    push es
    mov  ah, 0x35
    mov  al, byte ptr intr
    int  0x21
    mov  ax, es
    pop  es
    mov  edx, sel
    mov  [edx], ax
    mov  edx, ofs
    mov  [edx], ebx
    pop  ebx
  }
  return (1);
}

int dpmi_setvect (int intr, WORD sel, DWORD ofs)
{
  __asm {
    push ds
    mov  edx, ofs
    mov  ax, sel
    mov  ds, ax
    mov  ah, 0x25
    mov  al, byte ptr intr
    int  0x21
    pop  ds
  }
  return (1);
}

#if !defined(USE_FAST_PKT)

/* Some code below borrowed from djgpp, some heavily modified */

/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */

/* This code really can't be nested since the RMCB structure isn't copied,
 * so the stack check isn't really useful.  But someone may fix it someday.
 * On entry CS is known to be ours, ES is probably ours (since we passed it),
 * SS:ESP is locked 4K stack.  ES:EDI is regs structure, DS:ESI is RM SS:SP.
 * Do NOT enable interrupts in the user routine.  Thanks to ctm@ardi.com for
 * the improvements.  C. Sandmann 3/95
 */

#define FILL  0x00

static BYTE wrapper_code[] = {
/* 00 */ 0x06,                          /*     push    es               */
/* 01 */ 0x1E,                          /*     push    ds               */
/* 02 */ 0x06,                          /*     push    es               */
/* 03 */ 0x1F,                          /*     pop     ds               */
/* 04 */ 0x66, 0xB8,                    /*     mov ax,                  */
/* 06 */ FILL, FILL,                    /*         _our_selector        */
/* 08 */ 0x8E, 0xD8,                    /*     mov ds, ax               */
/* 0A */ 0xFF, 0x05,                    /*     incl                     */
/* 0C */ FILL, FILL, FILL, FILL,        /*          _call_count         */
/* 10 */ 0x83, 0x3D,                    /*     cmpl                     */
/* 12 */ FILL, FILL, FILL, FILL,        /*         _in_this_handler     */
/* 16 */ 0x00,                          /*         $0                   */
/* 17 */ 0x75,                          /*     jne                      */
/* 18 */ 0x33,                          /*         out                  */
/* 19 */ 0xC6, 0x05,                    /*     movb                     */
/* 1B */ FILL, FILL, FILL, FILL,        /*         _in_this_handler     */
/* 1F */ 0x01,                          /*         $1                   */
/* 20 */ 0x8E, 0xC0,                    /*     mov es, ax               */
/* 22 */ 0x8E, 0xE0,                    /*     mov fs, ax               */
/* 24 */ 0x8E, 0xE8,                    /*     mov gs, ax               */
/* 26 */ 0xBB,                          /*     mov ebx,                 */
/* 27 */ FILL, FILL, FILL, FILL,        /*         _local_stack         */
/* 2B */ 0xFC,                          /*     cld                      */
/* 2C */ 0x89, 0xE1,                    /*     mov ecx, esp             */
/* 2E */ 0x8C, 0xD2,                    /*     mov dx, ss               */
/* 30 */ 0x8E, 0xD0,                    /*     mov ss, ax               */
/* 32 */ 0x89, 0xDC,                    /*     mov esp, ebx             */
/* 34 */ 0x52,                          /*     push edx                 */
/* 35 */ 0x51,                          /*     push ecx                 */
/* 36 */ 0x56,                          /*     push esi                 */
/* 37 */ 0x57,                          /*     push edi                 */
/* 38 */ 0xE8,                          /*     call                     */
/* 39 */ FILL, FILL, FILL, FILL,        /*         _rmcb                */
/* 3D */ 0x5F,                          /*     pop edi                  */
/* 3E */ 0x5E,                          /*     pop esi                  */
/* 3F */ 0x58,                          /*     pop eax                  */
/* 40 */ 0x5B,                          /*     pop ebx                  */
/* 41 */ 0x8E, 0xD3,                    /*     mov ss, bx               */
/* 43 */ 0x89, 0xC4,                    /*     mov esp, eax             */
/* 45 */ 0xC6, 0x05,                    /*     movb                     */
/* 47 */ FILL, FILL, FILL, FILL,        /*         _in_this_handler     */
/* 4B */ 0x00,                          /*         $0                   */
/* 4C */ 0x1F,                          /* out:pop ds                   */
/* 4D */ 0x07,                          /*     pop es                   */
/* 4E */ 0x8B, 0x06,                    /*     mov eax,[esi]            */
/* 50 */ 0x26, 0x89, 0x47,0x2A,         /*     mov es:[edi+42],eax      */
/* 54 */ 0x66, 0x26, 0x83,0x47,0x2E,4,  /* add     es:[edi+46],0x4      */
         0xCF                           /* iret                         */
       };

#define RMCB_STACK_SIZE (4*1024)

static REALPTR alloc_callback (void (*callback)(void), REAL_regs *regs)
{
  __asm {   /* I had to use assembly here because int386x() crashed */
    mov  __dpmi_errno, 0
    push es
    push ds
    push edi
    push esi
    mov  ax, ds

    mov  edi, regs
    mov  es, ax         /* ES:EDI = DS:regs */

    mov  ax, cs
    mov  ds, ax
    mov  esi, callback  /* DS:ESI = CS:callback */

    mov  ax, 303h
    int  31h
    pop  esi
    pop  edi
    pop  ds
    pop  es
    jc   fail:
  }
  return (_CX << 16) + _DX;

fail:
  __dpmi_errno = _AX;
  return (0);
}

REALPTR dpmi_alloc_callback_retf (DPMI_callback *cb)
{
  REALPTR rmcb;
  BYTE   *wrapper = calloc (sizeof(wrapper_code) + RMCB_STACK_SIZE, 1);
  BYTE   *stack   = wrapper + sizeof(wrapper_code);

  if (!wrapper)
     return (0);

  memcpy (wrapper, wrapper_code, sizeof(wrapper_code));

  if (!dpmi_lock_region(&cb->rm_regs, sizeof(cb->rm_regs)) ||
      !dpmi_lock_region(wrapper, sizeof(wrapper_code) + RMCB_STACK_SIZE))
  {
    fprintf (stderr, "dpmi_lock_region: DPMI error %04Xh\n", __dpmi_errno);
    free (wrapper);
    free (stack);
    return (0);
  }

  *(short*)(wrapper+0x06) = _DS;
  *(long*) (wrapper+0x0C) = (long) stack + 8;
  *(long*) (wrapper+0x12) = (long) stack + 4;
  *(long*) (wrapper+0x1B) = (long) stack + 4;
  *(long*) (wrapper+0x27) = (long) stack + RMCB_STACK_SIZE; /* !! was -4 */
  *(long*) (wrapper+0x39) = (long) cb->pm_func - ((long)wrapper + 0x3D);
  *(long*) (wrapper+0x47) = (long) stack + 4;

  rmcb = alloc_callback ((void(*)(void))wrapper, &cb->rm_regs);
  if (!rmcb)
  {
    free (wrapper);
    fprintf (stderr, "dpmi_alloc_callback: DPMI error %04Xh\n", __dpmi_errno);
    return (0);
  }
  cb->wrapper = wrapper;
  cb->rm_addr = rmcb;
  return (1);
}

int dpmi_free_callback_retf (DPMI_callback *cb)
{
  union REGS r;

  if (!cb || !cb->wrapper)
     return (0);

  free (cb->wrapper);
  cb->wrapper = NULL;
  cb->pm_func = NULL;

  __dpmi_errno = 0;
  r.w.ax = 0x304;
  r.w.cx = RP_SEG (cb->rm_addr);
  r.w.dx = RP_OFF (cb->rm_addr);
  int386 (0x31, &r, &r);
  if (r.w.cflag & 1)
  {
    __dpmi_errno = r.w.ax;
    return (0);
  }
  return (1);
}
#endif   /* USE_FAST_PKT */
#endif   /* (DOSX & POWERPAK) && BORLAND386 */

