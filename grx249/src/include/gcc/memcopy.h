/**
 ** memcopy.h ---- inline assembly memory copy macros -- GNU-C code
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

#define __INLINE_386_ROWCOPY__(D,S,C,SIZE) ({                           \
    __asm__ volatile(                                                   \
	" cld        \n"                                                 \
	" rep        \n"                                                 \
	" movs"#SIZE                                                     \
	: "=D" ((void *)(D)), "=S" ((void *)(S)), "=c" ((int)(C))       \
	: "0"  ((void *)(D)), "1"  ((void *)(S)), "2"  ((int)(C))       \
	: "memory"                                                      \
    );                                                                  \
})

#define __INLINE_STD_ROWCOPY__(D,S,C,DM,SM,SIZE,TYPE)                   \
	__INLINE_386_ROWCOPY__(D,S,C,SIZE)

#define __INLINE_386_G_COPY__(DP,DS,SP,SS,C,DIN,SIN,SIZE,TYPE,DSEL,SSEL) ({  \
    register TYPE _scr_;                                                     \
    __asm__ volatile(""                                                      \
	  "    incl    %2                           \n"                      \
	  "    shrl    $1,%2                        \n"                      \
	  "    jnc     1f                           \n"                      \
	  "    jmp     0f                           \n"                      \
	  "    .align 4,0x90                        \n"                      \
	  "0:  "#SIN""#SIZE"  "#SSEL"(%1),%3        \n"                      \
	  "    addl    %8,%1                        \n"                      \
	  "    "#DIN""#SIZE"   %3,"#DSEL"(%0)       \n"                      \
	  "    addl    %7,%0                        \n"                      \
	  "1:  "#SIN""#SIZE"  "#SSEL"(%1),%3        \n"                      \
	  "    addl    %8,%1                        \n"                      \
	  "    "#DIN""#SIZE"   %3,"#DSEL"(%0)       \n"                      \
	  "    addl    %7,%0                        \n"                      \
	  "    decl    %2                           \n"                      \
	  "    jne     0b"                                                   \
	  : "=r" ((void *)(DP)),"=r" ((void *)(SP)),"=r" ((int)(C)),         \
	    "=&q" ((TYPE)_scr_)                                              \
	  : "0"  ((void *)(DP)),"1"  ((void *)(SP)), "2" ((int)(C)),         \
	    "gn" ((int)(DS)), "gn" ((int)(SS))                               \
	  : "memory"                                                         \
    );                                                                       \
})

#define __INLINE_386_C_COPY__(DP,DS,SP,SS,C,DIN,SIN,SIZE,TYPE,DSEL,SSEL,DINC,SINC) ({\
    register TYPE _scr_;                                                     \
    __asm__ volatile(""                                                      \
	  "    incl    %2                           \n"                      \
	  "    shrl    $1,%2                        \n"                      \
	  "    jnc     1f                           \n"                      \
	  "    jmp     0f                           \n"                      \
	  "    .align 4,0x90                        \n"                      \
	  "0:  "#SIN""#SIZE"  "#SSEL"(%1),%3        \n"                      \
	  "    " SINC "                             \n"                      \
	  "    "#DIN""#SIZE"   %3,"#DSEL"(%0)       \n"                      \
	  "    " DINC "                             \n"                      \
	  "1:  "#SIN""#SIZE"  "#SSEL"(%1),%3        \n"                      \
	  "    " SINC "                             \n"                      \
	  "    "#DIN""#SIZE"   %3,"#DSEL"(%0)       \n"                      \
	  "    " DINC "                             \n"                      \
	  "    decl    %2                           \n"                      \
	  "    jne     0b"                                                   \
	  : "=r" ((void *)(DP)),"=r" ((void *)(SP)),"=r" ((int)(C)),         \
	    "=&q" ((TYPE)_scr_)                                              \
	  : "0"  ((void *)(DP)),"1"  ((void *)(SP)), "2" ((int)(C))          \
	  : "memory"                                                         \
    );                                                                       \
})

#define __INLINE_386_C2_COPY__(DP,DS,SP,SS,C,DIN,SIN,SIZE,TYPE,DSEL,SSEL,DINC) ({\
    if( (SS) == 1 )                                                           \
       __INLINE_386_C_COPY__(DP,DS,SP,SS,C,DIN,SIN,SIZE,TYPE,DSEL,SSEL,DINC,"incl %1");\
    else                                                                      \
    if( (SS) == -1 )                                                          \
       __INLINE_386_C_COPY__(DP,DS,SP,SS,C,DIN,SIN,SIZE,TYPE,DSEL,SSEL,DINC,"decl %1");\
    else                                                                      \
    if( (SS) == 2 )                                                           \
       __INLINE_386_C_COPY__(DP,DS,SP,SS,C,DIN,SIN,SIZE,TYPE,DSEL,SSEL,DINC,"leal 2(%1),%1");\
    else                                                                      \
    if( (SS) == -2 )                                                          \
       __INLINE_386_C_COPY__(DP,DS,SP,SS,C,DIN,SIN,SIZE,TYPE,DSEL,SSEL,DINC,"leal -2(%1),%1");\
    else                                                                      \
    if( (SS) == 4 )                                                           \
       __INLINE_386_C_COPY__(DP,DS,SP,SS,C,DIN,SIN,SIZE,TYPE,DSEL,SSEL,DINC,"leal 4(%1),%1");\
    else                                                                      \
    if( (SS) == -4 )                                                          \
       __INLINE_386_C_COPY__(DP,DS,SP,SS,C,DIN,SIN,SIZE,TYPE,DSEL,SSEL,DINC,"leal -4(%1),%1");\
    else                                                                      \
       __INLINE_386_G_COPY__(DP,DS,SP,SS,C,DIN,SIN,SIZE,TYPE,DSEL,SSEL);      \
})

#define __INLINE_386_C1_COPY__(DP,DS,SP,SS,C,DIN,SIN,SIZE,TYPE,DSEL,SSEL) ({  \
    if( (DS) == 1 )                                                           \
       __INLINE_386_C2_COPY__(DP,DS,SP,SS,C,DIN,SIN,SIZE,TYPE,DSEL,SSEL,"incl %0");\
    else                                                                      \
    if( (DS) == -1 )                                                          \
       __INLINE_386_C2_COPY__(DP,DS,SP,SS,C,DIN,SIN,SIZE,TYPE,DSEL,SSEL,"decl %0");\
    else                                                                      \
    if( (DS) == 2 )                                                           \
       __INLINE_386_C2_COPY__(DP,DS,SP,SS,C,DIN,SIN,SIZE,TYPE,DSEL,SSEL,"leal 2(%0),%0");\
    else                                                                      \
    if( (DS) == -2 )                                                          \
       __INLINE_386_C2_COPY__(DP,DS,SP,SS,C,DIN,SIN,SIZE,TYPE,DSEL,SSEL,"leal -2(%0),%0");\
    else                                                                      \
    if( (DS) == 4 )                                                           \
       __INLINE_386_C2_COPY__(DP,DS,SP,SS,C,DIN,SIN,SIZE,TYPE,DSEL,SSEL,"leal 4(%0),%0");\
    else                                                                      \
    if( (DS) == -4 )                                                          \
       __INLINE_386_C2_COPY__(DP,DS,SP,SS,C,DIN,SIN,SIZE,TYPE,DSEL,SSEL,"leal -4(%0),%0");\
    else                                                                      \
       __INLINE_386_G_COPY__(DP,DS,SP,SS,C,DIN,SIN,SIZE,TYPE,DSEL,SSEL);      \
})


#define __INLINE_386_COLCOPY__(DP,DS,SP,SS,C,DIN,SIN,SIZE,TYPE,DSEL,SSEL) ({  \
    if(__builtin_constant_p(DS) && __builtin_constant_p(SS) )                 \
       __INLINE_386_C1_COPY__(DP,DS,SP,SS,C,DIN,SIN,SIZE,TYPE,DSEL,SSEL);     \
    else                                                                      \
       __INLINE_386_G_COPY__(DP,DS,SP,SS,C,DIN,SIN,SIZE,TYPE,DSEL,SSEL);      \
})

#define __INLINE_SEGSEG_ROWCOPY__(D,S,C,SZ,T,DS,SS)                           \
	__INLINE_386_COLCOPY__(D,CPSIZE_##SZ,S,CPSIZE_##SZ,C,mov,mov,SZ,T,DS,SS)

/* memory -> memory copies */
#define __INLINE_STD_COLCOPY__(D,DSKP,S,SSKP,C,DM,SM,INS,SIZE,TYPE)           \
	__INLINE_386_COLCOPY__(D,DSKP,S,SSKP,C,INS,mov,SIZE,TYPE,,)
#ifdef  I386_GCC_FAR_MEMORY
/* memory -> video copies */
#define __INLINE_FAR_STD_COLCOPY__(DST,DSKP,SRC,SSKP,C,DM,SM,INS,SIZE,TYPE)   \
	__INLINE_386_COLCOPY__(DST,DSKP,SRC,SSKP,C,INS,mov,SIZE,TYPE,%%fs:,)
#define __INLINE_FAR_STD_ROWCOPY__(D,S,C,DM,SM,SIZE,TYPE)                     \
	__INLINE_SEGSEG_ROWCOPY__(D,S,C,SIZE,TYPE,%%fs:,)
/* video -> memory copies */
#define __INLINE_STD_FAR_COLCOPY__(DST,DSKP,SRC,SSKP,C,DM,SM,INS,SIZE,TYPE)   \
	__INLINE_386_COLCOPY__(DST,DSKP,SRC,SSKP,C,INS,mov,SIZE,TYPE,,%%fs:)
#define __INLINE_STD_FAR_ROWCOPY__(D,S,C,DM,SM,SIZE,TYPE)                     \
	__INLINE_SEGSEG_ROWCOPY__(D,S,C,SIZE,TYPE,,%%fs:)
/* video -> video copies */
#define __INLINE_FAR_FAR_COLCOPY__(DST,DSKP,SRC,SSKP,C,DM,SM,INS,SIZE,TYPE)   \
	__INLINE_386_COLCOPY__(DST,DSKP,SRC,SSKP,C,INS,mov,SIZE,TYPE,%%fs:,%%fs:)
#define __INLINE_FAR_FAR_ROWCOPY__(D,S,C,DM,SM,SIZE,TYPE)                     \
	__INLINE_SEGSEG_ROWCOPY__(D,S,C,SIZE,TYPE,%%fs:,%%fs:)
#else   /* I386_GCC_FAR_MEMORY */
#define __INLINE_FAR_STD_ROWCOPY__(D,S,C,DM,SM,SIZE,TYPE)                     \
	__INLINE_STD_ROWCOPY__(D,S,C,DM,SM,SIZE,TYPE)
#define __INLINE_STD_FAR_ROWCOPY__(D,S,C,DM,SM,SIZE,TYPE)                     \
	__INLINE_STD_ROWCOPY__(D,S,C,DM,SM,SIZE,TYPE)
#define __INLINE_FAR_FAR_ROWCOPY__(D,S,C,DM,SM,SIZE,TYPE)                     \
	__INLINE_STD_ROWCOPY__(D,S,C,DM,SM,SIZE,TYPE)
#endif  /* I386_GCC_FAR_MEMORY */

#define fwdcopy_set(AP,D,S,C) do {                                           \
  int _scr_;                                                                 \
  if ((AP)==(D))                                                             \
    __asm__ volatile("\n"                                                    \
	  "    cld                                  \n"                      \
	  "    cmpl    $4,%2                        \n"                      \
	  "    jb      3f                           \n"                      \
	  "    testl   $1,%0                        \n"                      \
	  "    je      1f                           \n"                      \
	  "    movsb                                \n"                      \
	  "    decl    %2                           \n"                      \
	  "1:  testl   $2,%0                        \n"                      \
	  "    je      2f                           \n"                      \
	  "    movsw                                \n"                      \
	  "    decl    %2                           \n"                      \
	  "    decl    %2                           \n"                      \
	  "2:  movl    %2,%3                        \n"                      \
	  "    shrl    $2,%3                        \n"                      \
	  "    je      3f                           \n"                      \
	  "    rep                                  \n"                      \
	  "    movsl                                \n"                      \
	  "3:  testl   $2,%2                        \n"                      \
	  "    je      4f                           \n"                      \
	  "    movsw                                \n"                      \
	  "4:  testl   $1,%2                        \n"                      \
	  "    je      5f                           \n"                      \
	  "    movsb                                \n"                      \
	  "5:                                         "                      \
	  : "=D" ((void *)(D)),"=S" ((void *)(S)),"=r" ((int)(C)),           \
	    "=&c" ((int)_scr_)                                               \
	  : "0"  ((void *)(D)), "1" ((void *)(S)), "2" ((int)(C))            \
	  : "memory"                                                         \
    );                                                                       \
  else                                                                       \
    __asm__ volatile("\n"                                                    \
	  "    cld                                  \n"                      \
	  "    cmpl    $4,%2                        \n"                      \
	  "    jb      3f                           \n"                      \
	  "    testl   $1,%1                        \n"                      \
	  "    je      1f                           \n"                      \
	  "    movsb                                \n"                      \
	  "    decl    %2                           \n"                      \
	  "1:  testl   $2,%1                        \n"                      \
	  "    je      2f                           \n"                      \
	  "    movsw                                \n"                      \
	  "    decl    %2                           \n"                      \
	  "    decl    %2                           \n"                      \
	  "2:  movl    %2,%3                        \n"                      \
	  "    shrl    $2,%3                        \n"                      \
	  "    je      3f                           \n"                      \
	  "    rep                                  \n"                      \
	  "    movsl                                \n"                      \
	  "3:  testl   $2,%2                        \n"                      \
	  "    je      4f                           \n"                      \
	  "    movsw                                \n"                      \
	  "4:  testl   $1,%2                        \n"                      \
	  "    je      5f                           \n"                      \
	  "    movsb                                \n"                      \
	  "5:                                         "                      \
	  : "=D" ((void *)(D)),"=S" ((void *)(S)),"=r" ((int)(C)),           \
	    "=&c" ((int)_scr_)                                               \
	  : "0"  ((void *)(D)), "1" ((void *)(S)), "2" ((int)(C))            \
	  : "memory"                                                         \
    );                                                                       \
} while (0)

#endif /* __i386__ */

