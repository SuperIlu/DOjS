/**
 ** highlow.h ---- combining two BYTES into one WORD -- GNU-C special
 **
 ** Copyright (c) 1997 Hartmut Schirmer
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
#if __GNUC_MAJOR__==2 && __GNUC_MINOR__<=8
/* should not be used for EGCS/GCC after v2.8.x */

/* combine two bytes into one word: optimized i386 version */
#define highlow(hi,lo) ({                                           \
    register GR_int32u _res_;                                       \
    if(__builtin_constant_p((hi))&& __builtin_constant_p((lo)))     \
	_res_ = __highlow__((hi),(lo));                             \
    else                                                            \
    if(__builtin_constant_p((hi)))                                  \
      __asm__ volatile( "  movb %b1,%b0"                            \
	  : "=&q" (_res_)                                           \
	  : "qnm" ((GR_int8u)(lo)), "0" (((int)(hi))<<8) );         \
    else                                                            \
      __asm__ volatile( "  movb %b1,%h0"                            \
	  : "=&q" (_res_)                                           \
	  : "qnm" ((GR_int8u)(hi)), "0" ((int)(lo)) );              \
    _res_;                                                          \
})


/* high from *p / low from *(p+1) */
#define highlowP(p) ({                          \
    register GR_int32u _res_;                   \
    __asm__ volatile( "xorl   %0,%0      \n\t"  \
		      "movw   (%1),%w0   \n\t"  \
		      "exch   %b0,%h0        "  \
	: "=&q" (_res_)                         \
	: "r" ((GR_int8u *)(p))                 \
    );                                          \
    _res_;                                      \
})

#endif /* __GNUC_MAJOR__==2 && __GNUC_MINOR__<=8 */
#endif /* __i386__ */
