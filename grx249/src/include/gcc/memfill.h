/**
 ** memfill.h ---- inline assembly memory fill macros -- GNU-C code
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu]
 **
 ** This file is part of the GRX graphics library.
 **
 ** The GRX graphics library is free software; you can redistribute it
 ** and/or modify it under some conditions; see the "copying.grx" file
 ** for details.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 **
 **/

/* ================================================================ */
/* ==                     80386 FAMILY                           == */
/* ================================================================ */
#ifdef __i386__

#ifndef MOV_INS
#include "gcc/asmsupp.h"
#endif

/*
 * Unoptimized row and column fills
 */
#define __INLINE_386_ROWFILL__(P,V,C,SIZE,TYPE) ({                      \
	__asm__ volatile(                                               \
		" cld          \n"                                      \
		" rep          \n"                                      \
		" stos"#SIZE                                            \
		: "=D" ((void *)(P)), "=c" ((int)(C))                   \
		: "0"  ((void *)(P)), "1"  ((int)(C)),                  \
		  "a"  ((TYPE)(V))                                      \
		: "memory"                                              \
	);                                                              \
})
#define __INLINE_STD_ROWFILL__(P,V,C,FMODE,SIZE,TYPE)                   \
	__INLINE_386_ROWFILL__(P,V,C,SIZE,TYPE)

#define __INLINE_386_COLFILL__(P,V,C,SKIP,INS,SIZE,TYPE,SEL) ({         \
	if(__builtin_constant_p(SKIP) && ((SKIP) == 1))                 \
	  __asm__ volatile(""                                           \
		"    incl    %1                           \n"           \
		"    shrl    $1,%1                        \n"           \
		"    jnc     1f                           \n"           \
		"    jmp     0f                           \n"           \
		"    .align 4,0x90                        \n"           \
		"0:  "#INS""#SIZE"   %4,"#SEL"(%0)        \n"           \
		"    incl    %0                           \n"           \
		"1:  "#INS""#SIZE"   %4,"#SEL"(%0)        \n"           \
		"    incl    %0                           \n"           \
		"    decl    %1                           \n"           \
		"    jne     0b"                                        \
		: "=r" ((void *)(P)), "=r" ((int)(C))                   \
		: "0"  ((void *)(P)), "1"  ((int)(C)),                  \
		  "qn" ((TYPE)(V))                                      \
		: "memory"                                              \
	  );                                                            \
	else                                                            \
	if(__builtin_constant_p(SKIP) && ((SKIP) == 2))                 \
	  __asm__ volatile(""                                           \
		"    incl    %1                           \n"           \
		"    shrl    $1,%1                        \n"           \
		"    jnc     1f                           \n"           \
		"    jmp     0f                           \n"           \
		"    .align 4,0x90                        \n"           \
		"0:  "#INS""#SIZE"   %4,"#SEL"(%0)        \n"           \
		"    leal    2(%0),%0                     \n"           \
		"1:  "#INS""#SIZE"   %4,"#SEL"(%0)        \n"           \
		"    leal    2(%0),%0                     \n"           \
		"    decl    %1                           \n"           \
		"    jne     0b"                                        \
		: "=r" ((void *)(P)), "=r" ((int)(C))                   \
		: "0"  ((void *)(P)), "1"  ((int)(C)),                  \
		  "qn" ((TYPE)(V))                                      \
		: "memory"                                              \
	  );                                                            \
	else                                                            \
	if(__builtin_constant_p(SKIP) && ((SKIP) == 4))                 \
	  __asm__ volatile(""                                           \
		"    incl    %1                           \n"           \
		"    shrl    $1,%1                        \n"           \
		"    jnc     1f                           \n"           \
		"    jmp     0f                           \n"           \
		"    .align 4,0x90                        \n"           \
		"0:  "#INS""#SIZE"   %4,"#SEL"(%0)        \n"           \
		"    leal    4(%0),%0                     \n"           \
		"1:  "#INS""#SIZE"   %4,"#SEL"(%0)        \n"           \
		"    leal    4(%0),%0                     \n"           \
		"    decl    %1                           \n"           \
		"    jne     0b"                                        \
		: "=r" ((void *)(P)), "=r" ((int)(C))                   \
		: "0"  ((void *)(P)), "1"  ((int)(C)),                  \
		  "qn" ((TYPE)(V))                                      \
		: "memory"                                              \
	  );                                                            \
	else                                                            \
	  __asm__ volatile(""                                           \
		"    incl    %1                           \n"           \
		"    shrl    $1,%1                        \n"           \
		"    jnc     1f                           \n"           \
		"    jmp     0f                           \n"           \
		"    .align 4,0x90                        \n"           \
		"0:  "#INS""#SIZE"   %5,"#SEL"(%0)        \n"           \
		"    addl    %4,%0                        \n"           \
		"1:  "#INS""#SIZE"   %5,"#SEL"(%0)        \n"           \
		"    addl    %4,%0                        \n"           \
		"    decl    %1                           \n"           \
		"    jne     0b"                                        \
		: "=r" ((void *)(P)), "=r" ((int)(C))                   \
		: "0"  ((void *)(P)), "1"  ((int)(C)),                  \
		  "ng" ((int)(SKIP)), "qn" ((TYPE)(V))                  \
		: "memory"                                              \
	  );                                                            \
})
#define __INLINE_STD_COLFILL__(P,V,C,SKIP,FMODE,INS,SIZE,TYPE)          \
	__INLINE_386_COLFILL__(P,V,C,SKIP,INS,SIZE,TYPE,)
#ifdef  I386_GCC_FAR_MEMORY
#define __INLINE_FAR_COLFILL__(P,V,C,SKIP,FMODE,INS,SIZE,TYPE)          \
	__INLINE_386_COLFILL__(P,V,C,SKIP,INS,SIZE,TYPE,%%fs:)
#else   /* I386_GCC_FAR_MEMORY */
#define __INLINE_FAR_ROWFILL__(P,V,C,FMODE,SIZE,TYPE)                   \
	__INLINE_STD_ROWFILL__(P,V,C,FMODE,SIZE,TYPE)
#endif  /* I386_GCC_FAR_MEMORY */

/* ============================================ special optimized fills */

/* optimized byte based fill:
** if (c&(~3)) { (* c >= 4 *)
**   if (p&1) write 8bit, p++, c--;
**   if (p&2) write 16bit, p+=2,  c-=2;
**   if (c&(~3)) { (* c >= 4 *)
**     c>>2 times: write 32bit, p+=4;
**   }
** }
** if (c&2) write 16bit, p += 2;
** if (c&1) write 8bit, p++;
*/
#define repfill_b(p,v,c) do {                      \
  asm volatile(""                                  \
    "      testl   $-4,%%ecx                \n"    \
    "      je      2f                       \n"    \
    "      testl   $1,%%edi                 \n"    \
    "      je      0f                       \n"    \
    "      movb    %%al,(%%edi)             \n"    \
    "      incl    %%edi                    \n"    \
    "      decl    %%ecx                    \n"    \
    "0:    testl   $2,%%edi                 \n"    \
    "      je      1f                       \n"    \
    "      movw    %%ax,(%%edi)             \n"    \
    "      leal    -2(%%ecx),%%ecx          \n"    \
    "      leal     2(%%edi),%%edi          \n"    \
    "1:    testl   $-4,%%ecx                \n"    \
    "      je      2f                       \n"    \
    "      pushl   %%ecx                    \n"    \
    "      shrl    $2,%%ecx                 \n"    \
    "      cld                              \n"    \
    "      rep                              \n"    \
    "      stosl                            \n"    \
    "      popl    %%ecx                    \n"    \
    "2:    testb   $2,%%cl                  \n"    \
    "      je      3f                       \n"    \
    "      movw    %%ax,(%%edi)             \n"    \
    "      leal    2(%%edi),%%edi           \n"    \
    "3:    testb   $1,%%cl                  \n"    \
    "      je      4f                       \n"    \
    "      movb    %%al,(%%edi)             \n"    \
    "      incl    %%edi                    \n"    \
    "4:                                       "    \
    : "=c" ((unsigned int) (c)),                   \
      "=D" ((void *)(p))                           \
    : "0" ((unsigned int) (c)),                    \
      "1" ((void *)(p)),                           \
      "a" ((GR_int32u)(v))                         \
    : "memory"                                     \
  );                                               \
} while (0)

#ifndef  I386_GCC_FAR_MEMORY
/* Video memory is near: use optimized code */
#define repfill_b_f(p,v,c)  repfill_b((p),(v),(c))
#endif

/* ====================================================== 24bpp support */

#define __INLINE_386_REPFILL24__(p,c,b,INS,SEG) do {               \
  int _dummy_;                                                     \
  __asm__ volatile (                "\n"                           \
    "      testl  $1,%0              \n"                           \
    "      je     1f                 \n"                           \
    "    "#INS"b  %%dl,"#SEG"(%0)    \n"                           \
    "      incl   %0                 \n"                           \
    "      decl   %1                 \n"                           \
    "      shll   $8,%%edx           \n"                           \
    "      movb   %%dh,%%dl          \n"                           \
    "      rorl   $16,%%edx          \n"                           \
    "1:    testl  $2,%0              \n"                           \
    "      je     2f                 \n"                           \
    "    "#INS"w  %%dx,"#SEG"(%0)    \n"                           \
    "      leal   2(%0),%0           \n"                           \
    "      leal   -2(%1),%1          \n"                           \
    "      roll   $16,%%edx          \n"                           \
    "      movb   %%dl,%%dh          \n"                           \
    "      shrl   $8,%%edx           \n"                           \
    "2:    cmpl   $4,%1              \n"                           \
    "      jb     7f                 \n"                           \
    "      movl   %%edx,%%eax        \n"                           \
    "      shl    $8,%%eax           \n"                           \
    "      shldl  $8,%%eax,%%edx     \n"                           \
    "      movl   %%edx,%%ecx        \n"                           \
    "      shl    $8,%%ecx           \n"                           \
    "      movb   %%ah,%%al          \n"                           \
    "      rorl   $8,%%eax           \n"                           \
    "      movb   %%ah,%%cl          \n"                           \
    /* now we have : eax=La, ecx=Lb, edx=Lc */                     \
    "      subl   $12,%1             \n"                           \
    "      jb     4f                 \n"                           \
    "      jmp    3f                 \n"                           \
    "     .align  4,0x90             \n"                           \
    "3:  "#INS"l  %%eax,"#SEG"(%0)   \n"                           \
    "      leal   4(%0),%0           \n"                           \
    "    "#INS"l  %%ecx,"#SEG"(%0)   \n"                           \
    "      leal   4(%0),%0           \n"                           \
    "    "#INS"l  %%edx,"#SEG"(%0)   \n"                           \
    "      leal   4(%0),%0           \n"                           \
    "      subl   $12,%1             \n"                           \
    "      jnb    3b                 \n"                           \
    /* 0 .. 11 bytes left */                                       \
    "4:    leal   12(%1),%1          \n"                           \
    "      cmpl   $4,%1              \n"                           \
    "      jnb    5f                 \n"                           \
    "      movl   %%eax,%%edx        \n"                           \
    "      jmp    7f                 \n"                           \
    "5:  "#INS"l  %%eax,"#SEG"(%0)   \n"                           \
    "      leal  -4(%1),%1           \n"                           \
    "      leal   4(%0),%0           \n"                           \
    "      cmpl   $4,%1              \n"                           \
    "      jnb    6f                 \n"                           \
    "      movl   %%ecx,%%edx        \n"                           \
    "      jmp    7f                 \n"                           \
    "6:  "#INS"l  %%ecx,"#SEG"(%0)   \n"                           \
    "      leal   4(%0),%0           \n"                           \
    /* 0 .. 3 bytes left */                                        \
    "7:    testl  $2,%1              \n"                           \
    "      je     8f                 \n"                           \
    "    "#INS"w  %%dx,"#SEG"(%0)    \n"                           \
    "      leal   2(%0),%0           \n"                           \
    "      shrl   $16,%%edx          \n"                           \
    "8:    testl  $1,%1              \n"                           \
    "      je     9f                 \n"                           \
    "    "#INS"b  %%dl,"#SEG"(%0)    \n"                           \
    "      incl   %0                 \n"                           \
    "9:                              \n"                           \
    : "=r" ((void *)(p)), "=r" ((int)(b)), "=d" (_dummy_)          \
    : "2"  ((int)(c)), "0"  ((void *)(p)), "1"  ((int)(b))         \
    : "ax", "cx", "memory"                                         \
  );                                                               \
} while (0)

#define __INLINE_24_REPFILL__(P,C,B,FMODE,INS) \
	__INLINE_386_REPFILL24__(P,C,B,INS,)

#define __INLINE_24_FAR_REPFILL__(P,C,B,FMODE,INS) \
	__INLINE_386_REPFILL24__(P,C,B,INS,%%fs:)

#define GRX_HAVE_FAST_REPFILL24

#endif /* __i386__ */

