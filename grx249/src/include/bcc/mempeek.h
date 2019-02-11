/**
 ** mempeek.h ---- (far) memory read/write operations
 **                Borland-C code
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

/* There's actually nothing special except of far pointer usage */
#define __INLINE_STD_PEEK__(P,S,T)        (*(unsigned T far *)(P))
#define __INLINE_STD_POKE__(P,V,OP,I,S,T) (*(unsigned T far *)(P) OP (V))

#define __INLINE_FAR_PEEK__(P,S,T)        (*(volatile unsigned T far *)(P))
#define __INLINE_FAR_POKE__(P,V,OP,I,S,T) (*(volatile unsigned T far *)(P) OP (V))


/* optimized 24bpp access */

#include "access24.h"

#define __INLINE_24_PEEK__(P) (                                              \
   _ES = (unsigned)(void _seg *)(void far *)(P),                             \
   _DI = (unsigned)(void near *)(P),                                         \
   (__emit__(0x26),__emit__(0x8b),__emit__(0x05), /* mov ax,es:[di]   */     \
    __emit__(0x26),__emit__(0x8a),                /* mov dl,es:[di+2] */     \
      __emit__(0x55), __emit__(0x02),                                        \
    __emit__(0x30), __emit__(0xf6)   ),           /* xor dh,dh        */     \
   (long)((void _seg *)_DX + (void near *)_AX)                               \
)
#define __INLINE_24_FAR_PEEK__ __INLINE_24_PEEK__


#define __INLINE_24_BCC_POKE__(P,C,OPC_w,OPC_b)  do {                        \
   _AX = (unsigned)(C);                                                      \
   _ES = (unsigned)(void _seg *)(void far *)(P);                             \
   _DI = (unsigned)(void near *)(P);                                         \
   __emit__((char)(0x26));          /* es:                       */          \
   __emit__((char)(OPC_w));         /*    xor/or/and/mov [..],ax */          \
   __emit__((char)(0x05));          /*                    di     */          \
   __emit__((char)(0x47));          /*    inc di                 */          \
   __emit__((char)(0x47));          /*    inc di                 */          \
   _AL = RD24BYTE((C),2);                                                    \
   __emit__((char)(0x26));          /* es:                       */          \
   __emit__((char)(OPC_b));         /*    xor/or/and/mov [..],al */          \
   __emit__((char)(0x05));          /*                    di     */          \
} while (0)


#define __INLINE_24_POKE__(P,C,OP,INS) \
	__INLINE_24_TMP1_POKE__(P,C,INS)
#define __INLINE_24_TMP1_POKE__(P,C,INS) \
	__INLINE_24_TMP2_POKE__(P,C,OPCODE_##INS##_mem_w,OPCODE_##INS##_mem_b)
#define __INLINE_24_TMP2_POKE__(P,C,OPCw,OPCb) \
	__INLINE_24_BCC_POKE__(P,C,OPCw,OPCb)

#define __INLINE_24_FAR_POKE__ __INLINE_24_POKE__
