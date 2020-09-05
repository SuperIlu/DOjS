/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */

/* This code really can't be nested since the RMCB structure isn't copied,
   so the stack check isn't really useful.  But someone may fix it someday.
   On entry CS is known to be ours, ES is probably ours (since we passed it),
   SS:ESP is locked 4K stack.  ES:EDI is regs structure, DS:ESI is RM SS:SP.
   Do NOT enable interrupts in the user routine.  Thanks to ctm@ardi.com for
   the improvements.  C. Sandmann 3/95 */

#ifndef RMCB_STK_SIZE
#error This file must be included from pcpkt.c only
#endif

#if !defined(USE_FAST_PKT)

#define STACK_WAS_MALLOCED (1 << 0)

#define FILL 0x00


/* !! Doesn't save FS/GS registers */

static unsigned char wrapper[] = {
/* 00 */ 0x06,                          /*     push    es           */
/* 01 */ 0x1e,                          /*     push    ds           */
/* 02 */ 0x06,                          /*     push    es           */
/* 03 */ 0x1f,                          /*     pop     ds           */
/* 04 */ 0x66, 0xb8,                    /*     mov ax,              */
/* 06 */ FILL, FILL,                    /*         _our_selector    */
/* 08 */ 0x8e, 0xd8,                    /*     mov ds, ax           */
/* 0a */ 0xff, 0x05,                    /*     incl                 */
/* 0c */ FILL, FILL, FILL, FILL,        /*       _call_count        */
/* 10 */ 0x83, 0x3d,                    /*     cmpl                 */
/* 12 */ FILL, FILL, FILL, FILL,        /*         _in_this_handler */
/* 16 */ 0x00,                          /*         $0               */
/* 17 */ 0x75,                          /*     jne                  */
/* 18 */ 0x33,                          /*         bypass           */
/* 19 */ 0xc6, 0x05,                    /*     movb                 */
/* 1b */ FILL, FILL, FILL, FILL,        /*         _in_this_handler */
/* 1f */ 0x01,                          /*         $1               */
/* 20 */ 0x8e, 0xc0,                    /*     mov es, ax           */
/* 22 */ 0x8e, 0xe0,                    /*     mov fs, ax           */
/* 24 */ 0x8e, 0xe8,                    /*     mov gs, ax           */
/* 26 */ 0xbb,                          /*     mov ebx,             */
/* 27 */ FILL, FILL, FILL, FILL,        /*         _local_stack     */
/* 2b */ 0xfc,                          /*     cld                  */
/* 2c */ 0x89, 0xe1,                    /*     mov ecx, esp         */
/* 2e */ 0x8c, 0xd2,                    /*     mov dx, ss           */
/* 30 */ 0x8e, 0xd0,                    /*     mov ss, ax           */
/* 32 */ 0x89, 0xdc,                    /*     mov esp, ebx         */
/* 34 */ 0x52,                          /*     push edx             */
/* 35 */ 0x51,                          /*     push ecx             */
/* 36 */ 0x56,                          /*     push esi             */
/* 37 */ 0x57,                          /*     push edi             */
/* 38 */ 0xe8,                          /*     call                 */
/* 39 */ FILL, FILL, FILL, FILL,        /*         _rmcb            */
/* 3d */ 0x5f,                          /*     pop edi              */
/* 3e */ 0x5e,                          /*     pop esi              */
/* 3f */ 0x58,                          /*     pop eax              */
/* 40 */ 0x5b,                          /*     pop ebx              */
/* 41 */ 0x8e, 0xd3,                    /*     mov ss, bx           */
/* 43 */ 0x89, 0xc4,                    /*     mov esp, eax         */
/* 45 */ 0xc6, 0x05,                    /*     movb                 */
/* 47 */ FILL, FILL, FILL, FILL,        /*         _in_this_handler */
/* 4b */ 0x00,                          /*         $0               */
/* 4c */ 0x1f,                          /* bypass:  pop ds          */
/* 4d */ 0x07,                          /*     pop es               */

/* 4e */ 0x8b, 0x06,                    /*     mov eax,[esi]                  */
/* 50 */ 0x26, 0x89, 0x47, 0x2a,        /*     mov es:[edi+42],eax ; 42 = EIP */
    0x66, 0x26, 0x83, 0x47, 0x2e, 0x04, /*     add es:[edi+46],0x4 ; 46 = ESP */
    0xcf                                /*     iret                           */
};

static DWORD  stack_length;
static BYTE  *stack;

static int setup_rmcb (unsigned char *stub,
                       _go32_dpmi_seginfo *info,
                       __dpmi_regs *regs)
{
  stack_length = RMCB_STK_SIZE;
  int i;

  if (stack_length < 512 ||
      (stack = calloc (stack_length, 1)) == NULL)
     return (0x8015);

  if (_go32_dpmi_lock_data(stack, stack_length))
  {
    free (stack);
    return (0x8015);
  }

  ((long*)stack)[0] = STACK_WAS_MALLOCED;
  ((long*)stack)[1] = 0;
  ((long*)stack)[2] = 0;

  if (_go32_dpmi_lock_data(regs, sizeof(__dpmi_regs)))
  {
    free (stack);
    return (0x8015);
  }

  *(short*)(stub+0x06) = __djgpp_ds_alias;
  *(long *)(stub+0x0c) = (long) stack + 8;
  *(long *)(stub+0x12) = (long) stack + 4;
  *(long *)(stub+0x1b) = (long) stack + 4;
  *(long *)(stub+0x27) = (long) stack + stack_length - 8; /* !! added '-4' */
  *(long *)(stub+0x39) = info->pm_offset - ((long)stub + 0x3d);
  *(long *)(stub+0x47) = (long) stack + 4;

  for (i = 0x10; i <= 0x18 ; i++)
     *(BYTE*) (stub+i) = 0x90;

  info->size = (int)stub;

  if (__dpmi_allocate_real_mode_callback ((void*)stub, regs,
                          (__dpmi_raddr *)&info->rm_offset))
  {
    free (stack);
    return (0x8015);
  }
  return (0);
}

static DWORD get_rmcb_callcount (void)
{
  if (!stack)
     return (0);
  return *(DWORD*) (stack + 8);
}

static int _pkt_dpmi_allocate_real_mode_callback_retf (
               _go32_dpmi_seginfo *info, __dpmi_regs *regs)
{
  unsigned char *stub = calloc (sizeof(wrapper)+4, 1);

  if (stub == 0)
     return (0x8015);

  if (_go32_dpmi_lock_data(stub,sizeof(wrapper)+4))
  {
    free (stub);
    return (0x8015);
  }
  memcpy (stub, wrapper, sizeof(wrapper));
  if (setup_rmcb (stub, info, regs))
  {
    free (stub);
    return (0x8015);
  }
  return (0);
}

static int _pkt_dpmi_free_real_mode_callback (_go32_dpmi_seginfo *info)
{
  BYTE *stk = (BYTE*) (*(long*)((long)info->size+0x47) - 4);

  if (stk != stack)
     fprintf (stderr, "%s: stack error; stk %08lX, stack %08lX\n",
              __FUNCTION__, (DWORD)stk, (DWORD)stack);
  else
  {
    if (*(long*)stack & STACK_WAS_MALLOCED)
       free (stk);
    free ((char*)info->size);  /* free 'stub' */
  }
  stack = NULL;
  return __dpmi_free_real_mode_callback ((__dpmi_raddr *)&info->rm_offset);
}

#endif  /* !USE_FAST_PKT */
