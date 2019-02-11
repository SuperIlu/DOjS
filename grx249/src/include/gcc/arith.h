/**
 ** arith.h ---- some common integer arithmetic macros/inline functions
 **              Special GNU-C handling
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

/*
 * [i|u]scale(X,N,D)
 * scale an integer with long intermediate result but without using long
 * arithmetic all the way
 */

#define irscale(X,N,D) ({                                           \
	register int _SclVal_ = iscale(((int)(X) << 1),N,D);        \
	(_SclVal_ + (_SclVal_ >> (bitsof(int) - 1)) + 1) >> 1;      \
})

/* ================================================================ */
/* ==                     80386 FAMILY                           == */
/* ================================================================ */
#ifdef __i386__
/*
 * replicate_<FROM>2<TO>(byte_or_word_value)
 * copy the lower byte(s) of a byte or word into the upper byte(s)
 */

#define replicate_b2w(BYTE) (__builtin_constant_p(BYTE) ?       \
	(long)(0x00000101UL * (GR_int8u)(BYTE)) :               \
	({                                                      \
	register long _repvalue;                                \
	__asm__(                                                \
	       "movzbl  %b1,%0  "                       "\n\t"  \
	       "movb    %b0,%h0 "                               \
		: "=q" (_repvalue)                              \
		: "0m"  ((char)(BYTE))                          \
	);                                                      \
	_repvalue;                                              \
	})                                                      \
)

#define replicate_w2l(WORD) (__builtin_constant_p(WORD) ?       \
	(long)(0x00010001UL * (GR_int16u)(WORD)) :              \
	({                                                      \
	  register long  _repvalue;                             \
	  __asm__(                                              \
	       "movw    %w1,%w0  "                      "\n\t"  \
	       "shll    $16,%0   "                      "\n\t"  \
	       "movw    %w1,%w0  "                              \
		: "=&r" (_repvalue)                             \
		: "rm" ((GR_int16u)(WORD))                      \
	  );                                                    \
	_repvalue;                                              \
	})                                                      \
)

#define replicate_b2l(BYTE) (__builtin_constant_p(BYTE) ?       \
	(long)(0x01010101UL * (GR_int8u)(BYTE)) :               \
	({                                                      \
	register long  _repvalue;                               \
	__asm__(                                                \
	       "movb    %b1,%b0  "                      "\n\t"  \
	       "movb    %b0,%h0  "                      "\n\t"  \
	       "shll    $16,%0   "                      "\n\t"  \
	       "movb    %b1,%b0  "                      "\n\t"  \
	       "movb    %b0,%h0  "                              \
		: "=&q" (_repvalue)                             \
		: "qm" ((char)(BYTE))                           \
	);                                                      \
	_repvalue;                                              \
	})                                                      \
)

#endif /* __i386__ */
