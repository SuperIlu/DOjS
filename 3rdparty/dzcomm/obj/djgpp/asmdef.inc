/* Dzcomm 0.9.9e (WIP), djgpp */
/* automatically generated structure offsets for use by asm code */

#ifndef DZCOMM_DJGPP
#define DZCOMM_DJGPP
#endif

#ifndef DZCOMM_DOS
#define DZCOMM_DOS
#endif


#define IRQ_SIZE 16
#define IRQ_HANDLER 0
#define IRQ_NUMBER 4
#define IRQ_OLDVEC 8

#define DPMI_AX 28
#define DPMI_BX 16
#define DPMI_CX 24
#define DPMI_DX 20
#define DPMI_SP 46
#define DPMI_SS 48
#define DPMI_FLAGS 32

#define USE_FS
#define FSEG %fs:

#define FUNC(name) .globl _##name ; _align_ ; _##name:
#define GLOBL(name) _##name


