/*  cpumodel.S
 *
 *  This file contains all assembly code for the Intel Cpu identification.
 *  It is based on linux cpu detection code.
 *
 *  Only for GNU-C targets. Others use cpumodel.asm.
 *
 *  Intel also provides public similar code in the book
 *  called :
 *
 *  Pentium Processor Family
 *      Developer Family
 *  Volume  3 : Architecture and Programming Manual
 *
 * At the following place :
 *
 *  Chapter 5 : Feature determination
 *  Chapter 25: CPUID instruction
 *
 *  COPYRIGHT (c) 1998 valette@crf.canon.fr
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.OARcorp.com/rtems/license.html.
 *
 *  $Id: cpuModel.S,v 1.1 1998/08/05 15:15:46 joel Exp $
 */

#define IN_ASM_FILE
#include "register.h"

#if defined(__i386__)  /* '__x86_64__' code below */

.data

DATA (x86_type,       .byte 0)
DATA (x86_model,      .byte 0)
DATA (x86_mask,       .byte 0)
DATA (x86_hard_math,  .long 0)
DATA (x86_have_cpuid, .long 0)
DATA (x86_capability, .long 0)
DATA (x86_vendor_id,  .zero 13)

.text

/*
 * check Processor type: 386, 486, 6x86(L) or CPUID capable processor
 */
ENTRY (_w32_CheckCpuType)
    pushl %ebp
    movl  %esp, %ebp
    pushl %ebx

    /* Assume 386 for now
     */
    movl $3, _C_LABEL(x86_type)

    /* Start using the EFLAGS AC bit determination method. If this
     * bit can be set we have a 486 or above.
     */
    pushfl                          /* save EFLAGS */
    pushfl                          /* Get EFLAGS in EAX */
    popl %eax

    movl  %eax, %ecx                /* save original EFLAGS in ECX */
    xorl  $EFLAGS_ALIGN_CHECK, %eax /* flip AC bit in EAX */
    pushl %eax                      /* set EAX as EFLAGS */
    popfl
    pushfl                          /* Get new EFLAGS in EAX */
    popl  %eax

    xorl  %ecx, %eax                /* check if AC bit changed */
    andl  $EFLAGS_ALIGN_CHECK,%eax
    je is386                        /* If not : we have a 386 */

    /* Assume 486 for now
     */
    movl  $4, _C_LABEL(x86_type)
    movl  %ecx, %eax                /* Restore orig EFLAGS in EAX */
    xorl  $EFLAGS_ID, %eax          /* flip  ID flag */
    pushl %eax                      /* set EAX as EFLAGS */
    popfl
    pushfl                          /* Get new EFLAGS in EAX */
    popl  %eax

    xorl  %ecx, %eax                /* check if ID bit changed */
    andl  $EFLAGS_ID, %eax

    /*
     * if we are on a straight 486DX, SX, or 487SX we can't
     * change it. OTOH 6x86MXs and MIIs check OK.
     * Also if we are on a Cyrix 6x86(L)
     */
    je is486x

isnew:
    /* restore original EFLAGS
     */
    popfl
    incl _C_LABEL(x86_have_cpuid)   /* we have CPUID instruction */

    /* use CPUID to get :
     *   processor type,
     *   processor model,
     *   processor mask,
     * by using it with EAX = 1
     */
    movl $1, %eax
    cpuid

    movb %al, %cl                 /* save reg for future use */

    andb $0x0F, %ah               /* mask processor family (bit 8-11) */
    movb %ah, _C_LABEL(x86_type)  /* put result in x86_type (0..15) */

    andb $0xF0, %al               /* get model (bit 4-7) */
    shrb $4, %al
    movb %al, _C_LABEL(x86_model) /* store it in x86_model */

    andb $0x0F, %cl               /* get mask revision */
    movb %cl, _C_LABEL(x86_mask)  /* store it in x86_mask */

    movl %edx, _C_LABEL(x86_capability) /* store feature flags */

    /* get vendor info by using CPUID with EAX = 0
     */
    xorl %eax, %eax
    cpuid

    /* store results contained in ebx, edx, ecx in x86_vendor_id string.
     */
    movl %ebx, _C_LABEL(x86_vendor_id)
    movl %edx, _C_LABEL(x86_vendor_id+4)
    movl %ecx, _C_LABEL(x86_vendor_id+8)
    call check_x87
    jmp  end_CheckCpuType

/*
 * Now we test if we have a Cyrix 6x86(L). We didn't test before to avoid
 * clobbering the new BX chipset used with the Pentium II, which has a
 * register at the same addresses as those used to access the Cyrix special
 * configuration registers (CCRs).
 */
    /*
     * A Cyrix/IBM 6x86(L) preserves flags after dividing 5 by 2
     * (and it _must_ be 5 divided by 2) while other CPUs change
     * them in undefined ways. We need to know this since we may
     * need to enable the CPUID instruction at least.
     * We couldn't use this test before since the PPro and PII behave
     * like Cyrix chips in this respect.
     */
is486x:
    xor  %ax, %ax
    sahf
    movw $5, %ax
    movw $2, %bx
    div  %bl
    lahf
    cmpb $2, %ah
    jne  is386

    /*
     * N.B. The pattern of accesses to 0x22 and 0x23 is *essential*
     *      so do not try to "optimize" it! For the same reason we
     *      do all this with interrupts off.
     */
#define setCx86(reg, val) \
        movw reg, %ax;    \
        outw %ax, $0x22;  \
        movw val, %ax;    \
        outw %ax, $0x23

#define getCx86(reg)      \
        movw reg, %ax;    \
        outw %ax, $0x22;  \
        inw  $0x23, %ax

    cli
    getCx86 ($0xC3)         /*  get CCR3 */
    movw %ax, %cx           /* Save old value */
    movw %ax, %bx
    andw $0x0F, %bx         /* Enable access to all config registers */
    orw $0x10, %bx          /* by setting bit 4 */
    setCx86 ($0xC3, %bx)

    getCx86 ($0xE8)         /* now we can get CCR4 */
    orw $0x80, %ax          /* and set bit 7 (CPUIDEN) */
    movw %ax, %bx           /* to enable CPUID execution */
    setCx86 ($0xE8, %bx)

    /* Must check cpu id regs here and not after trying to set CCR3
     * to avoid failure when testing SG Microelectronic STPCs, which
     * lock up if you try to enable cpuid execution
     */

    getCx86 ($0xFE)         /* DIR0 : let's check if this is a 6x86(L) */
    andw $0xF0, %ax         /* should be 3xh */
    cmpw $0x30, %ax         /* STPCs return 0x80, 0x1a, 0x1b or 0x1f */
    jne  is386

    getCx86 ($0xE9)         /* CCR5 : we reset the SLOP bit */
    andw $0xFD, %ax         /* so that udelay calculation */
    movw %ax, %bx           /* is correct on 6x86(L) CPUs */
    setCx86 ($0xE9, %bx)

    setCx86 ($0xC3, %cx)    /* Restore old CCR3 */
    sti
    jmp isnew               /* We enabled CPUID now */

is386:
    popfl                   /* restore original EFLAGS */
    call check_x87

end_CheckCpuType:
    popl %ebx
    popl %ebp
    ret


/*
 * This checks for 287/387.
 */
check_x87:
    movl  $0, _C_LABEL(x86_hard_math)  /* assume no coprocessor */
    fninit
    fstsw %ax
    cmpb  $0, %al
    je    1f
    ret

1:  movl $1, _C_LABEL(x86_hard_math)
    ret

/*
 * Return value of CR4 register.
 *
 * NB! Seems to be virtualised under Win32 (always returns 0).
 * Don't call this function w/o checking for a true Pentium first
 * (see RDTSC_enabled() in misc.c). Returns 0 if CPL != 0.
 */
ENTRY (_w32_Get_CR4)
    movw  %cs, %ax
    testw $3, %ax             /* CPL=0? */
    jnz   1f
    .byte 0x0F, 0x20, 0xE0    /* movl %cr4, %eax */
    ret

1:  xor %eax, %eax
    ret

/*
 * BOOL _w32_SelWriteable (WORD sel)
 */
ENTRY (_w32_SelWriteable)
    movw  4(%esp), %ax
    verw  %ax
    sete  %al
    movzx %al, %eax
    ret

/*
 * BOOL _w32_SelReadable (WORD sel)
 */
ENTRY (_w32_SelReadable)
    movw  4(%esp), %ax
    verr  %ax
    sete  %al
    movzx %al, %eax
    ret

/*
 * WORD _w32_MY_CS (void)
 */
ENTRY (_w32_MY_CS)
    movw  %cs, %ax
    ret

/*
 * WORD _w32_MY_DS (void)
 */
ENTRY (_w32_MY_DS)
    movw  %ds, %ax
    ret

/*
 * WORD _w32_MY_ES (void)
 */
ENTRY (_w32_MY_ES)
    movw  %es, %ax
    ret

/*
 * WORD _w32_MY_SS (void)
 */
ENTRY (_w32_MY_SS)
    movw  %ss, %ax
    ret

/*
 * int _w32_asm_ffs (int val)
 */
ENTRY (_w32_asm_ffs)
    bsf  4(%esp), %eax
    jnz  1f
    movl $-1, %eax
1:  incl %eax
    ret

#elif defined(__x86_64__)

/*
 * Nice intro to 64-bit gcc asm:
 *  http://cs.lmu.edu/~ray/notes/gasexamples/
 */
.data

DATA (x86_type,       .byte 0)
DATA (x86_model,      .byte 0)
DATA (x86_mask,       .byte 0)
DATA (x86_capability, .long 0)
DATA (x86_vendor_id,  .zero 13)

/* An 'int' is 8 bytes (aka. a 'quad'). Hence these should match
 * the declaration in cpumodel.h.
 */
DATA (x86_hard_math,  .quad 0)
DATA (x86_have_cpuid, .quad 0)

.text

/*
 * Check Processor type; guaranteed to be a CPUID capable processor.
 */
ENTRY (_w32_CheckCpuType)
    push %rbp
    mov  %rsp, %rbp
    push %rbx

    /* Assume 6 for now
     */
    movb $6, _C_LABEL(x86_type)

    movq $1, _C_LABEL(x86_have_cpuid)  /* we have CPUID instruction */
    movq $1, _C_LABEL(x86_hard_math)   /* and math processor. Duh! */

    /* use CPUID to get :
     *   processor type,
     *   processor model,
     *   processor mask,
     * by using it with EAX = 1
     */
    mov $1, %rax
    cpuid

    movb %al, %cl                 /* save reg for future use */

    andb $0x0F, %ah               /* mask processor family (bit 8-11) */
    movb %ah, _C_LABEL(x86_type)  /* put result in x86_type (0..15) */

    andb $0xF0, %al               /* get model (bit 4-7) */
    shrb $4, %al
    movb %al, _C_LABEL(x86_model) /* store it in x86_model */

    andb $0x0F, %cl               /* get mask revision */
    movb %cl, _C_LABEL(x86_mask)  /* store it in x86_mask */

    mov %rdx, _C_LABEL(x86_capability) /* store feature flags */

    /* get vendor info by using CPUID with EAX = 0
     */
    xor %rax, %rax
    cpuid

    /* store results contained in rbx, rdx, rcx in x86_vendor_id string.
     */
    mov %rbx, _C_LABEL(x86_vendor_id)
    mov %rdx, _C_LABEL(x86_vendor_id+4)
    mov %rcx, _C_LABEL(x86_vendor_id+8)

    pop %rbx
    pop %rbp
    ret

/*
 * Return value of CR4 register.
 */
ENTRY (_w32_Get_CR4)
    movw  %cs, %ax
    testw $3, %ax             /* CPL=0? */
    jnz   1f
    .byte 0x0F, 0x20, 0xE0    /* movl %cr4, %eax */
    ret

1:  xor %rax, %rax
    ret

/*
 * BOOL _w32_SelWriteable (WORD sel)
 */
ENTRY (_w32_SelWriteable)
    verw  %cx
    sete  %al
    movzx %al, %rax
    ret

/*
 * BOOL _w32_SelReadable (WORD sel)
 */
ENTRY (_w32_SelReadable)
    verr  %cx
    sete  %al
    movzx %al, %rax
    ret

/*
 * WORD _w32_MY_CS (void)
 */
ENTRY (_w32_MY_CS)
    movw  %cs, %ax
    ret

/*
 * WORD _w32_MY_CS (void)
 */
ENTRY (_w32_MY_DS)
    movw  %ds, %ax
    ret

/*
 * WORD _w32_MY_ES (void)
 */
ENTRY (_w32_MY_ES)
    movw  %es, %ax
    ret

/*
 * WORD _w32_MY_SS (void)
 */
ENTRY (_w32_MY_SS)
    movw  %ss, %ax
    ret

/*
 * int _w32_asm_ffs (int val)
 */
ENTRY (_w32_asm_ffs)
    bsf  %rcx, %rax
    jnz  1f
    movq $-1, %rax
1:  incq %rax
    ret

#endif  /* __i386__ */
