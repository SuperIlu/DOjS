/**
 ** mempeek.h ---- (far) memory read/write operations
 **                GNU-C special assembler code
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
 ** Basic memory peek and poke operations in byte, word and long sizes.
 ** The poke operations are available in WRITE, XOR, OR and AND versions.
 **
 **/

/* ================================================================ */
/* ==                     80386 FAMILY                           == */
/* ================================================================ */
#ifdef __i386__

/* the far selector peek / poke stuff is only used
** if far memory access is required (DJGPP v2)     */
#ifdef I386_GCC_FAR_MEMORY

#ifndef MOV_INS
#include "gcc/asmsupp.h"
#endif

#define __INLINE_386_PEEK__(P,SIZE,T,SEL) ({            \
    unsigned T _peekvalue;                              \
    if(sizeof(T) == 1) __asm__ volatile(                \
	"mov"#SIZE" "#SEL"(%1),%0"                      \
	: "=q" (_peekvalue)                             \
	: "r"  (((unsigned T *)(P)))                    \
    );                                                  \
    else __asm__ volatile(                              \
	"mov"#SIZE" "#SEL"(%1),%0"                      \
	: "=r" (_peekvalue)                             \
	: "r"  (((unsigned T *)(P)))                    \
    );                                                  \
    _peekvalue;                                         \
})
#define __INLINE_386_POKE__(P,V,INS,SIZE,T,SEL) ({      \
    if(sizeof(T) == 1) __asm__ volatile(                \
	""#INS""#SIZE" %1,"#SEL"%0"                     \
	: "=m" (*((unsigned T *)(P)))                   \
	: "qn" ((unsigned T)(V))                        \
    );                                                  \
    else __asm__ volatile(                              \
	""#INS""#SIZE" %1,"#SEL"%0"                     \
	: "=m" (*((unsigned T *)(P)))                   \
	: "rn" ((unsigned T)(V))                        \
    );                                                  \
})

#define __INLINE_STD_PEEK__(P,S,T)        __INLINE_386_PEEK__(P,S,T,)
#define __INLINE_STD_POKE__(P,V,OP,I,S,T) __INLINE_386_POKE__(P,V,I,S,T,)

#define I386_GCC_FAR_SELECTOR             "%%fs:"
#define __INLINE_FAR_PEEK__(P,S,T)        __INLINE_386_PEEK__(P,S,T,%%fs:)
#define __INLINE_FAR_POKE__(P,V,OP,I,S,T) __INLINE_386_POKE__(P,V,I,S,T,%%fs:)
#define setup_far_selector(S) ({                        \
    __asm__ volatile(                                   \
	"movw %0,%%fs"                                  \
	: /* no outputs */                              \
	: "r" ((unsigned short)(S))                     \
    );                                                  \
})
#endif  /* I386_GCC_FAR_MEMORY */

#ifndef I386_GCC_FAR_SELECTOR
#define I386_GCC_FAR_SELECTOR             ""
#endif  /* I386_GCC_FAR_SELECTOR */

#define __INLINE_386_PEEK24__(P,SEL) ({                                 \
	  register GR_int32u _pix_;                                     \
	  __asm__ volatile(                              "\n"           \
		"    xorl    %%eax,%%eax                  \n"           \
		"    movb   " SEL "2(%1),%%ah             \n"           \
		"    sall    $8,%%eax                     \n"           \
		"    movw    " SEL "(%1),%%ax               "           \
		: "=&a" ((GR_int32u)_pix_)                              \
		: "r" ((void *)(P))                                     \
	  );                                                            \
	  (GrColor)_pix_;                                               \
})

#define __INLINE_386_POKE24__(P,C,INS,SEL) do {                         \
          int _dummy_;                                                  \
	  __asm__ volatile(                              "\n"           \
		"    "#INS"w %%ax," SEL "(%2)             \n"           \
		"    shrl    $8,%%eax                     \n"           \
		"    "#INS"b %%ah," SEL "2(%2)            \n"           \
		: "=a" (_dummy_)                                        \
		: "0" (C), "r" ((void *)(P))                            \
	  );                                                            \
} while (0)

#define __INLINE_24_PEEK__(p) \
	__INLINE_386_PEEK24__(p,)

#define __INLINE_24_FAR_PEEK__(p) (peek_l_f(p) & 0xffffff)
#define PEEK_24_F_READS_ONE_MORE

#define __INLINE_24_POKE__(p,c,op,INS) \
	__INLINE_386_POKE24__(p,c,INS,)
#define __INLINE_24_FAR_POKE__(p,c,op,INS) \
	__INLINE_386_POKE24__(p,c,INS,I386_GCC_FAR_SELECTOR)

#endif  /* __i386__ */
