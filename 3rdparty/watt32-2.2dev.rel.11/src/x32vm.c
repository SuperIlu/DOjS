/*!\file x32vm.c
 *
 *  FlashTek DOS extender interface.
 *
 *  A Pharlap-like interface for FlashTek's X32VM DOS extender
 *  Currently this only supports the Digital Mars compiler.
 *
 *  by G. Vanem <gvanem@yahoo.no>  July 2002
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wattcp.h"
#include "printk.h"
#include "strings.h"
#include "misc.h"
#include "cpumodel.h"
#include "x32vm.h"

#if (DOSX & X32VM)   /* rest of file */

#define MAX_WRAPPERS        32
#define MARKER              0xDEADBEEF
#define SEG_OFS_TO_LIN(s,o) ((DWORD)_x386_zero_base_ptr +  \
                             (DWORD)(o) + ((DWORD)(s) << 4))

typedef struct RMCB_record {
        SWI_REGS   rm_reg;
        REALPTR    dos_real;
        REALPTR    dos_para_addr;
        UINT       stack_size;
        UINT      *stack_bottom;
        UINT      *stack_top;
        pmodeHook  callback;
        DWORD      marker;
      } RMCB_record;

typedef enum  {
        RETF = 0xCB,   /* far return */
        IRET = 0xCF    /* interrupt return */
      } ReturnType;

static char   num_wrappers = 0;
static DWORD  exc_app_start = 4096; /**\todo find the real application start */
static struct RMCB_record rmcb [MAX_WRAPPERS];

#if defined(DMC386)

static void __far PmodeGlue (int taskIndex);
static void PmGlueEnd  (void);

void _dx_rmiv_get (int int_no, REALPTR *handler)
{
  __asm {
    mov eax, 2503h
    mov ecx, int_no
    int 21h
    mov eax, handler
    mov [eax], ebx
  }
}

void _dx_rmiv_set (int int_no, REALPTR handler)
{
  __asm {
    push ebx
    mov eax, 2505h
    mov ebx, handler
    mov ecx, int_no
    int 21h
    pop ebx
  }
}

void _dx_pmiv_get  (int int_no, FARPTR *handler)
{
  __asm {
    push es
    push ebx
    mov eax, 2502h
    mov ecx, int_no
    int 21h
    mov eax, handler
    mov [eax], ebx
    mov [eax+4], es
    pop ebx
    pop es
  }
}

void _dx_apmiv_set (int int_no, FARPTR handler)
{
  __asm {
    push ds
    mov edx, dword ptr handler[0]
    mov ax, word ptr handler[2]
    mov ds, ax
    mov eax, 2506h
    mov ecx, int_no
    int 21h
    pop ds
  }
}

void _dx_real_int (int int_no, SWI_REGS *regs)
{
  RMI_BLK rmi;

  __asm {
    push ebx
    push esi
    push edi
    mov eax, int_no
    mov rmi.inum, ax              /* rmi.inum = int_no */

    mov eax, regs
    mov cx, word ptr [eax].r_ds
    mov rmi.ds_reg, cx            /* rmi.ds_reg = regs->r_ds */

    mov cx, word ptr [eax].r_es
    mov rmi.es_reg, cx            /* rmi.es_reg = regs->r_es */

    mov ecx, [eax].r_ax
    mov rmi.eax_reg, ecx          /* rmi.eax_reg = regs->r_ax */

    mov ecx, [eax].r_dx
    mov rmi.edx_reg, ecx          /* rmi.edx_reg = regs->r_dx */

    mov ebx, [eax].r_bx
    mov ecx, [eax].r_cx
    mov edi, [eax].r_di
    mov esi, [eax].r_si
    mov eax, 2511h
    lea edx, rmi
    push ebp
    int 21h                       /* X32VM function 2511h */
    pop ebp

    push eax
    mov eax, [regs]
    pushfd
    pop [eax].r_flags
    mov [eax].r_bx, ebx
    mov [eax].r_cx, ecx
    mov [eax].r_di, edi
    mov [eax].r_si, esi
    pop [eax].r_ax
    pop edi
    pop esi
    pop ebx
  }
  regs->r_ds = rmi.ds_reg;
  regs->r_es = rmi.es_reg;
  regs->r_dx = rmi.edx_reg;
}

/*
 * !! Doesn't work yet (only needed in pcintr.c which isn't tested).
 */
int _dx_call_real (REALPTR proc, RMC_BLK *rm_reg, DWORD count, ...)
{
  __asm {
    push esi
    push edi
    push ebx
    mov  eax, 250Eh
    mov  ebx, proc
    mov  ecx, count
    int  21h
    pop  ebx
    pop  edi
    pop  esi
    jc fail
  }
  ARGSUSED (rm_reg);
  return (1);
fail:
  return (0);
}

void _dx_rmlink_get (REALPTR *cback, REALPTR *rm_cbuf,
                     DWORD *rm_cbuf_size, void _far **pm_cbuf)
{
  __asm {
    push es
    push esi
    push ebx
    mov ax, 250Dh
    int 21h
    mov esi, [cback]
    mov [esi], eax
    mov esi, [rm_cbuf]
    mov [esi], ebx
    mov esi, [rm_cbuf_size]
    mov [esi], ecx
    mov ebx, [pm_cbuf]
    mov [ebx], edx
    mov [ebx+4], es
    pop ebx
    pop esi
    pop es
  }
}

/*
 * A simplified version of Pharlap's 2537h function.
 *  - Allocate conventional memory above DOS data buffer.
 */
int _dx_real_above (int para_count, WORD *para, WORD *largest)
{
  REALPTR rp = _x32_real_alloc (16*para_count);

  if (largest)
     *largest = 0;
  if (!rp)
     return (-1);
  *para = RP_SEG (rp);
  return (0);
}

int _dx_real_free (WORD seg)
{
  _x32_real_free (seg << 16);
  return (0);
}

/*
 * A simplified version of Pharlap's 25C0h function.
 *  - Allocate conventional memory.
 */
int _dx_real_alloc (int para_count, WORD *para, WORD *largest)
{
  WORD seg = _x32_real_alloc (16*para_count);

  if (!seg)
     return (-1);
  *para = seg;
  *largest = 0;  /**< \todo */
  return (0);
}

#elif defined(WATCOM386)

extern void __far PmodeGlue (int task_index);
extern void PmGlueEnd  (void);

  /**< \todo Support 32-bit Watcom with X32VM */
#endif

int _dx_lock_pgs (const void _far *addr, int len)
{
  return _x386_memlock ((void _far*)addr, len);
}

int _dx_lock_pgsn (const void *addr, int len) /* addr must not be on stack */
{
  return _x386_memlock (MK_FP(MY_DS(),addr), len);
}

int _dx_ulock_pgsn (const void *addr, int len)
{
  return _x386_memunlock (MK_FP(MY_DS(),addr), len);
}

void ReadRealMem (void *buf, REALPTR rp, unsigned len)
{
  const BYTE *src = (const BYTE*) SEG_OFS_TO_LIN (RP_SEG(rp), RP_OFF(rp));
  memcpy (buf, src, len);
}

void WriteRealMem (REALPTR rp, const void *src, int len)
{
  BYTE *dest = (BYTE*) SEG_OFS_TO_LIN (RP_SEG(rp), RP_OFF(rp));
  memcpy (dest, src, len);
}

void FillRealMem (REALPTR rp, char val, int len)
{
  BYTE *dest = (BYTE*) SEG_OFS_TO_LIN (RP_SEG(rp), RP_OFF(rp));
  memset (dest, val, len);
}

void PokeRealWord (REALPTR rp, WORD val)
{
  WORD *dest = (WORD*) SEG_OFS_TO_LIN (RP_SEG(rp), RP_OFF(rp));
  *dest = val;
}

#ifdef __SW_3R
#error !? Cannot use this with reg-calls.
#endif

void stack_rewind (DWORD start, DWORD base)
{
  int    loop    = 10;            /* print max 10 stack frames  */
  DWORD  eip     = start;
  DWORD *context = (DWORD*) base; /* EBP increases as we rewind */
  DWORD  limit   = get_cs_limit();

  _printk ("Stack trace:\r\n");

  while (context >= (DWORD*)base && ((DWORD)context & 3) == 0 && --loop)
  {
    DWORD *next;

    if ((DWORD)context >= limit - 4 || eip >= limit ||
        (DWORD)context <= exc_app_start || eip <= exc_app_start)
       break;

    _printk ("  %08X ", eip);

    next = (DWORD*)*context;          /* get ESP stack value from context */

    /* Compute number of args to display
     */
    if (next == NULL ||               /* we're rewound back to CRT */
        next <= (DWORD*)exc_app_start)
       _printk ("( *** )");

    else
    {
      DWORD *argv;
      DWORD  argc = next - context - 2;

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


/*
 * Take a seg-x:ofs REALPTR and return a normalized seg-y:0 REALPTR
 */
static REALPTR normalize (REALPTR rp)
{
  WORD seg = RP_SEG (rp);
  WORD ofs = RP_OFF (rp);
  seg += (ofs >> 4);
  if (ofs & 0xF)
     seg++;
  RP_SET (rp, 0, seg);
  return (rp);
}

static REALPTR _dx_alloc_rmode_wrapper (pmodeHook pmHook,
                                        int len, int stack_size,
                                        ReturnType returnType)
{
  #define DOFS  0x22  /* offset of data section (tiny model)   */
  #define DSIZE 12    /* sizeof data section at wrapper end    */
                             
  static BYTE rm_wrapper[] =
  {
    0x06,                     /* 00    push es                         */
    0x1E,                     /* 01    push ds                         */
    0x0E,                     /* 02    push cs                         */
    0x1F,                     /* 03    pop  ds               ; DS = CS */
    0x66,0x60,                /* 04    pushad                          */
    0x66,0xFF,0x36,DOFS+8,0,  /* 06    push dword ptr task_ptr         */
    0x66,0x6A,0,              /* 0B    push dword ptr 0      ; use default selectors */
    0x6A,0,                   /* 0E    push word ptr 0       ;                    */
    0x66,0xFF,0x36,DOFS,0,    /* 10    push dword ptr prot_glue                   */
    0xFF,0x1E,DOFS+4,0,       /* 15    call r2p_addr                              */
    0x83,0xC4,14,             /* 19    add  sp,14            ; discard used stack */
    0x66,0x61,                /* 1C    popad          */
    0x1F,                     /* 1E    pop  ds        */
    0x07,                     /* 1F    pop  es        */
    0xCB,                     /* 20    retf           */
    0x90,                     /* 21    nop            */
    0,0,0,0,                  /* 22+0  prot_glue dd ? */
    0,0,0,0,                  /* 22+4  r2p_addr  dd ? */
    0,0,0,0                   /* 22+8  task_ptr  dd ? */
  };

  BYTE _far *fp_wrapper;
  UINT      *stack, i;
  REALPTR    dos_real;        /* allocated DOS memory */
  REALPTR    r2p_addr;        /* address of real to pmode switch */
  REALPTR    dos_tb_real;     /* rmode-address of transfer buffer */
  void _far *dos_tb_addr;     /* ditto for pmode */
  DWORD      dos_tb_size;     /* size of transfer buffer */

  WATT_ASSERT (sizeof(rm_wrapper) == DOFS+DSIZE);

  if (!pmHook || num_wrappers == MAX_WRAPPERS)
     return (0);

  stack_size += sizeof(UINT) + 3;   /* add one for marker and round up */
  stack_size &= ~3;
  stack = malloc (stack_size);
  if (!stack)
     return (0);

  len += sizeof(rm_wrapper) + 16;
  dos_real = _x32_real_alloc (len);
  if (!dos_real)
  {
    free (stack);
    return (0);
  }

  _dx_rmlink_get (&r2p_addr, &dos_tb_real, &dos_tb_size, &dos_tb_addr);

  for (i = 0; i < MAX_WRAPPERS; i++)
      if (!rmcb[i].dos_real)
         break;

  *(DWORD*) (rm_wrapper+DOFS+0) = (DWORD)&PmodeGlue;  /* pmode helper */
  *(DWORD*) (rm_wrapper+DOFS+4) = r2p_addr;           /* rm->pm addr */
  *(DWORD*) (rm_wrapper+DOFS+8) = (DWORD)(rmcb + i);  /* task_ptr */
  rm_wrapper [0x20] = returnType;                     /* RETF or IRET */

  /* lock pages used by callback
   */
  fp_wrapper = MK_FP (_x32_zero_base_selector, RP_SEG(dos_real) << 4);
  _dx_lock_pgs  (fp_wrapper, len);
  _dx_lock_pgsn ((void*)&PmodeGlue, (UINT)&PmGlueEnd - (UINT)&PmodeGlue);
  _dx_lock_pgsn ((void*)stack, stack_size);

  rmcb [i].dos_real      = dos_real;
  rmcb [i].callback      = pmHook;
  rmcb [i].stack_bottom  = stack;
  rmcb [i].stack_size    = stack_size;
  rmcb [i].stack_top     = stack + stack_size/sizeof(UINT) - sizeof(UINT);
  rmcb [i].stack_top[1]  = MARKER;
  rmcb [i].marker        = MARKER;
  rmcb [i].dos_para_addr = normalize (dos_real);
  WriteRealMem (rmcb[i].dos_para_addr, &rm_wrapper, sizeof(rm_wrapper));

#if 0
  printf ("dos_real %08lX, r2p-addr %08lX, stack %08X\n",
          dos_real, r2p_addr, stack);
#endif

  num_wrappers++;
  return (rmcb[i].dos_para_addr);
}

void _dx_free_rmode_wrapper (REALPTR dos_addr)
{
  int i;

  for (i = 0; i < MAX_WRAPPERS; i++)
  {
    struct RMCB_record *tr = rmcb + i;

    if (!rmcb[i].dos_real || rmcb[i].dos_para_addr != dos_addr)
       continue;

    if (tr->marker == MARKER)
       _x32_real_free (tr->dos_real);

    if (tr->stack_top[1] == MARKER)
       free (tr->stack_bottom);
    memset (tr, 0, sizeof(*tr));
  }
}

REALPTR _dx_alloc_rmode_wrapper_retf (pmodeHook pmHook, rmodeHook rmHook,
                                      int len, int stack_size)
{
  ARGSUSED (rmHook);
  return _dx_alloc_rmode_wrapper (pmHook, len, stack_size, RETF);
}

REALPTR _dx_alloc_rmode_wrapper_iret (pmodeHook pmHook, int stack_size)
{
  return _dx_alloc_rmode_wrapper (pmHook, 0, stack_size, IRET);
}

#if defined(DMC386)

static void __far PmodeGlue (struct RMCB_record *task_ptr)
{
/*
 * We need an assembly language "glue" routine on the protected mode side
 * to deal with several things:
 *   1.  It gets called with a FAR procedure call.
 *   2.  Copy all real-mode registers into the 'rm_reg' structure of the
 *       correct task-index.
 *   3.  Switch back to a stack in the data segment.
 *   4.  Call the HLL prot-mode callback with '&rm_reg' on stack.
 *   5.  Switch back to the stack DOSX has allocated for us.
 *   6.  Copy 'rm_reg' back onto the stack.
 *
 * Doesn't store ESP/EBP/FS/GS to SWI_REGS (not needed in Watt-32)
 */

  __asm {
       mov ebx, task_ptr
       pushfd
       push gs
       push fs

/*
 * fill the 'rm_reg' struct based on the "pushad" instruction and the
 * "push ds/es" of the wrapper. Registers that haven't changed are
 * stored as-is.
 *
 * in wrapper:   PUSH ES                  */   #define REG_ES  [ebp+50]
/*               PUSH DS                  */   #define REG_DS  [ebp+48]
/*   (PUSHAD)    PUSH EAX                 */   #define REG_EAX [ebp+44]
/*               PUSH ECX                 */   #define REG_ECX [ebp+40]
/*               PUSH EDX                 */   #define REG_EDX [ebp+36]
/*               PUSH EBX                 */   #define REG_EBX [ebp+32]
/*               PUSH ESP                 */   #define REG_ESP [ebp+28]
/*               PUSH EBP                 */   #define REG_EBP [ebp+24]
/*               PUSH ESI                 */   #define REG_ESI [ebp+20]
/*               PUSH EDI                 */   #define REG_EDI [ebp+16]
/*                                        */
/*               PUSH dword ptr task_ptr  */
/*               PUSH dword ptr 0         */
/*               PUSH word  ptr 0         */
/*               PUSH dword ptr prot_glue */

       mov [ebx].r_ax, eax
       mov [ebx].r_cx, ecx
       mov [ebx].r_dx, edx
       mov [ebx].r_si, esi
       mov [ebx].r_di, edi

#if 0  /* test */
       movzx eax, word ptr REG_DS
       push  eax
       call  dhex4int        /* print DS */
       add   esp, 4

       movzx eax, word ptr REG_ES
       push  eax
       call  dhex4int        /* print ES */
       add   esp, 4

       mov   ax, ss
       movzx eax, ax
       push  eax
       call  dhex4int        /* print SS */
       add   esp, 4
#endif

       pushfd
       pop eax
       mov [ebx].r_flags, eax

       mov eax, REG_EBX
       mov [ebx].r_bx, eax

       mov eax, REG_DS
       mov [ebx].r_ds, ax

       mov eax, REG_ES
       mov [ebx].r_es, ax

       mov  eax, [ebx].stack_bottom
       push _x386_stacklow
       mov  _x386_stacklow, eax  /* set new stack-low value */

       mov  dx, ss
       mov  esi, esp             /* DX:ESI = current SS:ESP */
       mov  ax, ds
       mov  ss, ax
       mov  esp, [ebx].stack_top /* new SS:ESP = DS:top-of-task-stack */

       push edx                  /* save old stack pointer on top */
       push esi                  /* of new stack */

       push ebx                  /* because rm_reg is first in the structure */
       cld                       /* C code need direction flag off */
       call [ebx].callback       /* call the C handler function */
       lss  esp, [esp+4]         /* switch back to original stack */
       pop  _x386_stacklow

/* fill stack with the rm_reg struct based on the "popad" instruction
 * and the "pop ds/es" of the wrapper.
 */
       mov ax, [ebx].r_es
       mov REG_ES, ax

       mov ax, [ebx].r_ds
       mov REG_DS, ax

       mov eax, [ebx].r_ax
       mov REG_EAX, eax

       mov eax, [ebx].r_cx
       mov REG_ECX, eax

       mov eax, [ebx].r_dx
       mov REG_EDX, eax

       mov eax, [ebx].r_bx
       mov REG_EBX, eax

       mov eax, [ebx].r_si
       mov REG_ESI, eax

       mov eax, [ebx].r_di
       mov REG_EDI, eax

       pop fs
       pop gs
       popfd
  }
}

static void PmGlueEnd (void)  /* end of area to lock */
{
}
#endif  /* DMC386 */
#endif  /* DOSX & X32VM */

