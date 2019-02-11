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

/* bcc can't do 32bit copies -> use 16bit instead */
#ifndef NO_32BIT_COPY
# define NO_32BIT_COPY
#endif

#include "bcc/asmsupp.h"

#define __INLINE_STD_ROWCOPY__(D,S,C,DM,SM,SIZE,TYPE) do {              \
    __EMIT__(0x1e);                             /* push ds */           \
    _ES = (unsigned)(void _seg *)(void far *)(S);                       \
    _SI = (unsigned)(void near *)(S);                                   \
    __EMIT__(0x06);                             /* push es */           \
    _ES = (unsigned)(void _seg *)(void far *)(D);                       \
    _DI = (unsigned)(void near *)(D);                                   \
    _CX = (int)(C);                                                     \
    __EMIT__(0x1f);                             /* pop ds  */           \
    __EMIT__(0xfc);                             /* cld     */           \
    __EMIT__(0xf3);                             /* rep     */           \
    __EMIT__(0xa4 + sizeof(TYPE) - 1);          /* movsB|W */           \
    __EMIT__(0x1f);                             /* pop ds  */           \
    (unsigned)(void near *)(D) = _DI;                                   \
    (unsigned)(void near *)(S) = _SI;                                   \
} while(0)

/* -------------------------------------------------------------------------- */

#define __INLINE_BCC_OPRCOPY_b__(D,S,C,WROPCODE) do {                   \
    __EMIT__(0x1e);                             /* push ds */           \
    _ES = (unsigned)(void _seg *)(void far *)(S);                       \
    _SI = (unsigned)(void near *)(S);                                   \
    __EMIT__(0x06);                             /* push es */           \
    _ES = (unsigned)(void _seg *)(void far *)(D);                       \
    _DI = (unsigned)(void near *)(D);                                   \
    _CX = (int)(C);                                                     \
    __EMIT__(0x1f);                             /* pop ds  */           \
    __EMIT__(0xfc);                             /* cld     */           \
 /* loop: */                                                            \
    __EMIT__(0xac);                             /* lodsb   */           \
    __EMIT__(0x26);                  /* es:                          */ \
    __EMIT__(WROPCODE);              /*    xor/or/and/mov [..],al/ax */ \
    __EMIT__(0x05);                  /*                    di        */ \
    __EMIT__(0x47);                             /* inc di  */           \
    __EMIT__(0x49);                             /* dec cx  */           \
    __EMIT__(0x75);                             /* jnz     */           \
      __EMIT__(0xf8);                           /*     loop*/           \
    __EMIT__(0x1f);                             /* pop ds  */           \
    (unsigned)(void near *)(D) = _DI;                                   \
    (unsigned)(void near *)(S) = _SI;                                   \
} while(0)


#define __INLINE_BCC_OPRCOPY_w__(D,S,C,WROPCODE) do {                   \
    __EMIT__(0x1e);                             /* push ds */           \
    _ES = (unsigned)(void _seg *)(void far *)(S);                       \
    _SI = (unsigned)(void near *)(S);                                   \
    __EMIT__(0x06);                             /* push es */           \
    _ES = (unsigned)(void _seg *)(void far *)(D);                       \
    _DI = (unsigned)(void near *)(D);                                   \
    _CX = (int)(C);                                                     \
    __EMIT__(0x1f);                             /* pop ds  */           \
    __EMIT__(0xfc);                             /* cld     */           \
 /* loop: */                                                            \
    __EMIT__(0xad);                             /* lodsw   */           \
    __EMIT__(0x26);                  /* es:                          */ \
    __EMIT__(WROPCODE);              /*    xor/or/and/mov [..],al/ax */ \
    __EMIT__(0x05);                  /*                    di        */ \
    __EMIT__(0x47);                             /* inc di  */           \
    __EMIT__(0x47);                             /* inc di  */           \
    __EMIT__(0x49);                             /* dec cx  */           \
    __EMIT__(0x75);                             /* jnz     */           \
      __EMIT__(0xf7);                           /*     loop*/           \
    __EMIT__(0x1f);                             /* pop ds  */           \
    (unsigned)(void near *)(D) = _DI;                                   \
    (unsigned)(void near *)(S) = _SI;                                   \
} while(0)

/* need an additional indirection to resolve INS and SIZE */
#define __INLINE_TMP_OPRCOPY__(D,S,C,INS,SIZE,TY)                       \
	__INLINE_BCC_OPRCOPY_##SIZE##__(D,S,C,OPCODE_##INS##_mem_##SIZE,TY)

#define __INLINE_STD_OPRCOPY__(D,S,C,DM,SM,INS,SIZE,TY)                 \
	__INLINE_TMP_OPRCOPY__(D,S,C,INS,SIZE,TY)

/* -------------------------------------------------------------------------- */

#define __INLINE_BCC_COLCOPY__(D,DS,S,SS,C,WROPCODE,TYPE) do {          \
    volatile int _dskip_ = (int)(DS);                                   \
    volatile int _sskip_ = (int)(SS);                                   \
    volatile unsigned int _cnt_ = (unsigned int)(C);                    \
    __EMIT__(0x1e);                             /* push ds */           \
    _ES = (unsigned)(void _seg *)(void far *)(S);                       \
    _SI = (unsigned)(void near *)(S);                                   \
    __EMIT__(0x06);                             /* push es */           \
    _ES = (unsigned)(void _seg *)(void far *)(D);                       \
    _DI = (unsigned)(void near *)(D);                                   \
    _BX = (int)(_sskip_);                                               \
    _CX = (int)(_cnt_);                                                 \
    _DX = (int)(_dskip_);                                               \
    __EMIT__(0x1f);                             /* pop ds  */           \
    __EMIT__(0xfc);                             /* cld     */           \
 /* loop: */                                                            \
    __EMIT__(0x8a+sizeof(TYPE)-1);              /* mov al/ax,[si] */    \
      __EMIT__(0x04);                                                   \
    __EMIT__(0x26);                  /* es:                          */ \
    __EMIT__(WROPCODE);              /*    xor/or/and/mov [..],al/ax */ \
    __EMIT__(0x05);                  /*                    di        */ \
    __EMIT__(0x01);                             /* add di,dx */         \
      __EMIT__(0xd7);                                                   \
    __EMIT__(0x01);                             /* add si,bx */         \
      __EMIT__(0xde);                                                   \
    __EMIT__(0x49);                             /* dec cx  */           \
    __EMIT__(0x75);                             /* jnz     */           \
      __EMIT__(0xf4);                           /*     loop*/           \
    __EMIT__(0x1f);                             /* pop ds  */           \
    (unsigned)(void near *)(D) = _DI;                                   \
    (unsigned)(void near *)(S) = _SI;                                   \
} while(0)

/* need an additional indirection to resolve INS and SIZE */
#define __INLINE_STD_COLCOPY__(D,DSKP,S,SSKP,C,DM,SM,INS,SIZE,TYPE)     \
	__INLINE_TMP1_COLCOPY__(D,DSKP,S,SSKP,C,INS,SIZE,TYPE)
#define __INLINE_TMP1_COLCOPY__(D,DS,S,SS,C,INS,SIZE,TY)                \
	__INLINE_TMP2_COLCOPY__(D,DS,S,SS,C,OPCODE_##INS##_mem_##SIZE,TY)
#define __INLINE_TMP2_COLCOPY__(D,DS,S,SS,C,WOPC,TY)                    \
	__INLINE_BCC_COLCOPY__(D,DS,S,SS,C,WOPC,TY)


/* memory -> video copy */
#define rowcopy_b_f_set rowcopy_b_set
#define rowcopy_w_f_set rowcopy_w_set
#define rowcopy_l_f_set rowcopy_l_set

#define rowcopy_b_f_xor rowcopy_b_xor
#define rowcopy_w_f_xor rowcopy_w_xor
#define rowcopy_l_f_xor rowcopy_l_xor

#define rowcopy_b_f_or  rowcopy_b_or
#define rowcopy_w_f_or  rowcopy_w_or
#define rowcopy_l_f_or  rowcopy_l_or

#define rowcopy_b_f_and rowcopy_b_and
#define rowcopy_w_f_and rowcopy_w_and
#define rowcopy_l_f_and rowcopy_l_and

/* video -> memory copy */
#define rowcopy_b_set_f rowcopy_b_set
#define rowcopy_w_set_f rowcopy_w_set
#define rowcopy_l_set_f rowcopy_l_set

#define rowcopy_b_xor_f rowcopy_b_xor
#define rowcopy_w_xor_f rowcopy_w_xor
#define rowcopy_l_xor_f rowcopy_l_xor

#define rowcopy_b_or_f  rowcopy_b_or
#define rowcopy_w_or_f  rowcopy_w_or
#define rowcopy_l_or_f  rowcopy_l_or

#define rowcopy_b_and_f rowcopy_b_and
#define rowcopy_w_and_f rowcopy_w_and
#define rowcopy_l_and_f rowcopy_l_and

/* video -> video copy */
#define rowcopy_b_f_set_f rowcopy_b_set
#define rowcopy_w_f_set_f rowcopy_w_set
#define rowcopy_l_f_set_f rowcopy_l_set

#define rowcopy_b_f_xor_f rowcopy_b_xor
#define rowcopy_w_f_xor_f rowcopy_w_xor
#define rowcopy_l_f_xor_f rowcopy_l_xor

#define rowcopy_b_f_or_f  rowcopy_b_or
#define rowcopy_w_f_or_f  rowcopy_w_or
#define rowcopy_l_f_or_f  rowcopy_l_or

#define rowcopy_b_f_and_f rowcopy_b_and
#define rowcopy_w_f_and_f rowcopy_w_and
#define rowcopy_l_f_and_f rowcopy_l_and


/* ------------------------------------------- optimal fills */
extern void far *_GR_fwdcopy_set(void near *ap, void far *d,
				 void far  *s,  unsigned cnt);
extern void far *_GR_fwdcopy_xor(void near *ap, void far *d,
				 void far  *s,  unsigned cnt);
extern void far *_GR_fwdcopy_or( void near *ap, void far *d,
				 void far  *s,  unsigned cnt);
extern void far *_GR_fwdcopy_and(void near *ap, void far *d,
				 void far  *s,  unsigned cnt);

#define __BCC_FWD_COPY__(OP,ap,d,s,c) do {                                    \
  (d) = _GR_fwdcopy_##OP((void near *)(ap),(void far *)(d),                   \
			 (void far  *)(s), (unsigned)(c)   );                 \
  ((char far *)(s)) += (unsigned)(c);                                         \
} while (0)

#define fwdcopy_set(ap,d,s,c) __BCC_FWD_COPY__(set,ap,d,s,c)
#define fwdcopy_f_set   fwdcopy_set
#define fwdcopy_set_f   fwdcopy_set
#define fwdcopy_f_set_f fwdcopy_set

#define fwdcopy_xor(ap,d,s,c) __BCC_FWD_COPY__(xor,ap,d,s,c)
#define fwdcopy_f_xor   fwdcopy_xor
#define fwdcopy_xor_f   fwdcopy_xor
#define fwdcopy_f_xor_f fwdcopy_xor

#define fwdcopy_or(ap,d,s,c) __BCC_FWD_COPY__(or,ap,d,s,c)
#define fwdcopy_f_or    fwdcopy_or
#define fwdcopy_or_f    fwdcopy_or
#define fwdcopy_f_or_f  fwdcopy_or

#define fwdcopy_and(ap,d,s,c) __BCC_FWD_COPY__(and,ap,d,s,c)
#define fwdcopy_f_and   fwdcopy_and
#define fwdcopy_and_f   fwdcopy_and
#define fwdcopy_f_and_f fwdcopy_and
