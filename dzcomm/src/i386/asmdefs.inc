/*
 * DZcomm : a serial port API.
 * file : amsdefs.h
 *
 * Macros for making asm code easier, stuff which mat need to be
 * seem by both C and asm code. Essentially from the Allegro code.
 * 
 * v1.0 By Shawn Hargreaves
 *
 * See readme.txt for copyright information.
 */

#ifndef ASMDEFS_INC
#define ASMDEFS_INC


#if defined DJGPP
   #include "obj/djgpp/asmdef.inc"
#elif defined __MINGW32__
   #include "obj/mingw32/asmdef.inc"
#elif defined _MSC_VER
   #include "obj/msvc/asmdef.inc"
#elif defined __WATCOMC__
   #include "obj/watcom/asmdef.inc"
#elif defined __unix__
   #include "obj/unix/asmdef.inc"
#elif defined __BEOS__
   #include "obj/beos/asmdef.inc"
#else
   #error unknown platform
#endif


/* helper macro in case we ever need to change this */
#define _align_   .balign 4, 0x90


/* readable way to access arguments passed from C code */
#define ARG1      8(%ebp)
#define ARG2      12(%ebp)
#define ARG3      16(%ebp)
#define ARG4      20(%ebp)
#define ARG5      24(%ebp)
#define ARG6      28(%ebp)
#define ARG7      32(%ebp)
#define ARG8      36(%ebp)
#define ARG9      40(%ebp)
#define ARG10     44(%ebp)


/* How many stacks to allocate for the irq wrappers. This can't be in the 
 * main headers, because it is used by both C and asm code. You could 
 * probably get away with fewer of these, if you want to save memory and
 * you are feeling brave...
 */
#define IRQ_STACKS      8


#endif   /* ifndef ASMDEFS_INC */

