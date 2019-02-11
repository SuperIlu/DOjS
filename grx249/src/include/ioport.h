/**
 ** ioport.h ---- port input/output macros
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
 ** Intel CPU specific input/output instructions. This file
 ** supports i386 GCC and Turbo C.
 **
 **/

#ifndef __IOPORT_H_INCLUDED__
#define __IOPORT_H_INCLUDED__

#ifdef __TURBOC__
/* prototype for __emit__() */
#include <dos.h>
#endif

#ifdef _MSC_VER
/* prototype for _inp/_inpw/_outp/_outpw */
#include <conio.h>
/* prototype _enable/_disable */
#include <dos.h>
#endif

#ifdef  SLOW_DOWN_IO
#ifndef SLOW_DOWN_IO_PORT
#define SLOW_DOWN_IO_PORT   0x80
#endif
#ifdef  __GNUC__
#ifdef  __i386__
#define __INLINE_SLOW_IO_ONCE__  ({                     \
    __asm__ volatile(                                   \
    "outb %%al,%0"                                      \
    : /* no outputs */                                  \
    : "n" (SLOW_DOWN_IO_PORT)                           \
    );                                                  \
})
#endif
#endif
#ifdef  __TURBOC__
#define __INLINE_SLOW_IO_ONCE__  (                      \
    __emit__((char)(0xe6)),   /* outb to const port */  \
    __emit__((char)(SLOW_DOWN_IO_PORT))                 \
)
#endif

#ifdef _MSC_VER
#define __INLINE_SLOW_IO_ONCE__ outp(SLOW_DOWN_IO_PORT,0)
#endif

#if (SLOW_DOWN_IO - 0) <= 1
#define __INLINE_SLOW_DOWN_IO__                         \
    __INLINE_SLOW_IO_ONCE__
#elif   (SLOW_DOWN_IO - 0) == 2
#define __INLINE_SLOW_DOWN_IO__                         \
    __INLINE_SLOW_IO_ONCE__,                            \
    __INLINE_SLOW_IO_ONCE__
#elif   (SLOW_DOWN_IO - 0) == 3
#define __INLINE_SLOW_DOWN_IO__                         \
    __INLINE_SLOW_IO_ONCE__,                            \
    __INLINE_SLOW_IO_ONCE__,                            \
    __INLINE_SLOW_IO_ONCE__
#else
#define __INLINE_SLOW_DOWN_IO__                         \
    __INLINE_SLOW_IO_ONCE__,                            \
    __INLINE_SLOW_IO_ONCE__,                            \
    __INLINE_SLOW_IO_ONCE__,                            \
    __INLINE_SLOW_IO_ONCE__
#endif
#define __INLINE_SLOW_DWN_IOC__ __INLINE_SLOW_DOWN_IO__,
#else   /* SLOW_DOWN_IO */
#define __INLINE_SLOW_DOWN_IO__
#define __INLINE_SLOW_DWN_IOC__
#endif

#ifdef  __GNUC__
#ifdef  __i386__
#define __INLINE_INPORT__(P,SIZE,T) ({                  \
    register T _value;                                  \
    __asm__ volatile(                                   \
    "in"#SIZE" %w1,%0"                                  \
    : "=a"  (_value)                                    \
    : "Nd"   ((unsigned short)(P))                      \
    );                                                  \
    __INLINE_SLOW_DOWN_IO__;                            \
    _value;                                             \
})
#define __INLINE_OUTPORT__(P,V,SIZE,T) ({               \
    __asm__ volatile(                                   \
    "out"#SIZE" %"#SIZE"0,%w1"                          \
    : /* no outputs */                                  \
    : "a" ((unsigned T)(V)),                            \
      "Nd" ((unsigned short)(P))                        \
    );                                                  \
    __INLINE_SLOW_DOWN_IO__;                            \
})
#ifndef SLOW_DOWN_IO
#define __INLINE_INPORTS__(P,B,C,SIZE,T) ({             \
    register void *_inportptr = ((void *)(B));          \
    register int   _inportcnt = ((int)(C));             \
    __asm__ volatile(                                   \
    "cld; "                                             \
    "rep; ins"#SIZE""                                   \
    : "=D" (_inportptr), "=c" (_inportcnt)              \
    : "0"  (_inportptr), "1"  (_inportcnt),             \
      "d"  ((unsigned short)(P))                        \
    );                                                  \
})
#define __INLINE_OUTPORTS__(P,B,C,SIZE,T) ({            \
    register void *_outportptr = ((void *)(B));         \
    register int   _outportcnt = ((int)(C));            \
    __asm__ volatile(                                   \
    "cld; "                                             \
    "rep; outs"#SIZE""                                  \
    : "=S" (_outportptr), "=c" (_outportcnt)            \
    : "0"  (_outportptr), "1"  (_outportcnt),           \
      "d"  ((unsigned short)(P))                        \
    );                                                  \
})
#else   /* SLOW_DOWN_IO */
#define __INLINE_INPORTS__(P,B,C,SIZE,T) ({             \
    register T  *_inportptr = ((T *)(B));               \
    register int _inportcnt = ((int)(C));               \
    register int _inportadr = ((int)(P));               \
    do *_inportptr++ = inport_##SIZE(_inportadr);       \
    while(--_inportcnt);                                \
})
#define __INLINE_OUTPORTS__(P,B,C,SIZE,T) ({            \
    register T  *_outportptr = ((T *)(B));              \
    register int _outportcnt = ((int)(C));              \
    register int _outportadr = ((int)(P));              \
    do outport_##SIZE(_outportadr,*_outportptr++);      \
    while(--_outportcnt);                               \
})
#endif  /* SLOW_DOWN_IO */
#endif  /* __i386__ */
#endif  /* __GNUC__ */

#ifdef  __TURBOC__
/* void    __emit__(); */
#define __INLINE_INPORT__(P,SIZE,T) (                   \
    _DX = ((unsigned short)(P)),                        \
    __emit__((char)(0xec+sizeof(T)-1)), /* inB|W  */    \
    __INLINE_SLOW_DWN_IOC__                             \
    (unsigned T)_AX                                     \
)
#define __INLINE_OUTPORT__(P,V,SIZE,T) do {             \
    _AX = ((unsigned short)(V));                        \
    _DX = ((unsigned short)(P));                        \
    __emit__((char)(0xee+sizeof(T)-1)); /* outB|W */    \
    __INLINE_SLOW_DOWN_IO__;                            \
} while(0)
#ifndef SLOW_DOWN_IO
#define __INLINE_INPORTS__(P,B,C,SIZE,T) do {           \
    _ES = (unsigned)(void _seg *)(void far *)(B);       \
    _DI = (unsigned)(void near *)(B);                   \
    _CX = ((unsigned short)(C));                        \
    _DX = ((unsigned short)(P));                        \
    __emit__((char)(0xfc));     /* cld    */            \
    __emit__((char)(0xf3));     /* rep    */            \
    __emit__((char)(0x6c+sizeof(T)-1)); /* insB|W */    \
} while(0)
#define __INLINE_OUTPORTS__(P,B,C,SIZE,T) do {          \
    _ES = (unsigned)(void _seg *)(void far *)(B);       \
    _SI = (unsigned)(void near *)(B);                   \
    _CX = ((unsigned short)(C));                        \
    _DX = ((unsigned short)(P));                        \
    __emit__((char)(0xfc));     /* cld     */           \
    __emit__((char)(0x26));     /* seg es  */           \
    __emit__((char)(0xf3));     /* rep     */           \
    __emit__((char)(0x6e+sizeof(T)-1)); /* outsB|W */   \
} while(0)
#else   /* SLOW_DOWN_IO */
#define __INLINE_INPORTS__(P,B,C,SIZE,T) do {           \
    _ES = (unsigned)(void _seg *)(void far *)(B);       \
    _BX = (unsigned)(void near *)(B);                   \
    _CX = ((unsigned short)(C));                        \
    _DX = ((unsigned short)(P));                        \
    do {                                                \
    __emit__((char)(0xec+sizeof(T)-1)); /* inB|W */     \
    __INLINE_SLOW_DOWN_IO__;                            \
    *((T _seg *)_ES + (T near *)_BX) = (T)_AX;          \
    _BX += sizeof(T);                                   \
    _CX--;                                              \
    } while(_CX);                                       \
} while(0)
#define __INLINE_OUTPORTS__(P,B,C,SIZE,T) do {          \
    _ES = (unsigned)(void _seg *)(void far *)(B);       \
    _BX = (unsigned)(void near *)(B);                   \
    _CX = ((unsigned short)(C));                        \
    _DX = ((unsigned short)(P));                        \
    do {                                                \
    (T)_AX = *((T _seg *)_ES + (T near *)_BX);          \
    __emit__((char)(0xee+sizeof(T)-1)); /* outB|W */    \
    __INLINE_SLOW_DOWN_IO__;                            \
    _BX += sizeof(T);                                   \
    _CX--;                                              \
    } while(_CX);                                       \
} while(0)
#endif  /* SLOW_DOWN_IO */
#endif  /* __TURBOC__ */

#ifdef  __WATCOMC__ /* GS - Watcom C++ 11.0 */
#include <conio.h>
/* 8bit port access */
#define inpb(a) inp(a)
#define outpb(a,b) outp(a,b)
/* 16bit port access */
/* inpw(a)    already defined */
/* outpw(a,b) already defined */
/* 32bit port access */
#define inpl(a) inpd(a)
#define outpl(a,b) outpd(a,b)
#define __INLINE_INPORT__(P,SIZE,T) (                   \
    (unsigned T) inp##SIZE (P)                          \
)
#define __INLINE_OUTPORT__(P,V,SIZE,T) (                \
    (unsigned T) outp##SIZE (P,V)                       \
)
#ifndef SLOW_DOWN_IO
#define __INLINE_INPORTS__(P,B,C,SIZE,T) do {           \
    do {                                                \
	*B = __INLINE_INPORT__(P,SIZE,T);               \
	(T)B ++;                                        \
    } while(C--);                                       \
} while(0)
#define __INLINE_OUTPORTS__(P,B,C,SIZE,T) do {          \
	do{                                             \
	__INLINE_OUTPORTS__(P,*B,SIZE,T);               \
	(T)B ++;                                        \
	} while (C--);                                  \
} while(0)
#else   /* SLOW_DOWN_IO */
#define __INLINE_INPORTS__(P,B,C,SIZE,T) do {           \
    do {                                                \
	*B = __INLINE_INPORT__(P,SIZE,T);               \
	(T)B ++;                                        \
    } while(C--);                                       \
} while(0)
#define __INLINE_OUTPORTS__(P,B,C,SIZE,T) do {          \
	do{                                             \
	__INLINE_OUTPORTS__(P,*B,SIZE,T);               \
	(T)B ++;                                        \
	} while (C--);                                  \
} while(0)
#endif  /* SLOW_DOWN_IO */
#endif  /* __WATCOMC__ */

#ifdef _MSC_VER
#define inport_b(port)          _inp((unsigned)(port))
#define inport_w(port)          _inpw((unsigned)(port))
#define outport_b(port,val)     _outp(((unsigned)(port)),((int)(val)))
#define outport_w(port,val)     _outp(((unsigned)(port)),((unsigned)(val)))
#endif

#ifndef inport_b
#define inport_b(port)          __INLINE_INPORT__(port,b,char)
#endif
#ifndef inport_w
#define inport_w(port)          __INLINE_INPORT__(port,w,short)
#endif
#ifndef inport_l
#define inport_l(port)          __INLINE_INPORT__(port,l,long)
#endif

#ifndef outport_b
#define outport_b(port,val)     __INLINE_OUTPORT__(port,val,b,char)
#endif
#ifndef outport_w
#define outport_w(port,val)     __INLINE_OUTPORT__(port,val,w,short)
#endif
#ifndef outport_l
#define outport_l(port,val)     __INLINE_OUTPORT__(port,val,l,long)
#endif

#ifndef inport_b_s
#define inport_b_s(port,buf,cnt)    __INLINE_INPORTS__(port,buf,cnt,b,char)
#endif
#ifndef inport_w_s
#define inport_w_s(port,buf,cnt)    __INLINE_INPORTS__(port,buf,cnt,w,short)
#endif
#ifndef inport_l_s
#define inport_l_s(port,buf,cnt)    __INLINE_INPORTS__(port,buf,cnt,l,long)
#endif

#ifndef outport_b_s
#define outport_b_s(port,buf,cnt)   __INLINE_OUTPORTS__(port,buf,cnt,b,char)
#endif
#ifndef outport_w_s
#define outport_w_s(port,buf,cnt)   __INLINE_OUTPORTS__(port,buf,cnt,w,short)
#endif
#ifndef outport_l_s
#define outport_l_s(port,buf,cnt)   __INLINE_OUTPORTS__(port,buf,cnt,l,long)
#endif

#ifdef  __GNUC__
#ifdef  __i386__
#ifdef  __MSDOS__
#define int_disable()           ({ __asm__ volatile("cli"); })
#define int_enable()            ({ __asm__ volatile("sti"); })
#else
#define int_disable()
#define int_enable()
#endif
#endif
#endif

#ifdef  __TURBOC__
#define int_disable()           __emit__((char)(0xfa))
#define int_enable()            __emit__((char)(0xfb))
#endif

#if defined(_MSC_VER) || defined(__WATCOMC__) /* GS - Watcom C++ 11.0 */
#define int_disable()           _disable()
#define int_enable()            _enable()
#endif


#if defined(__TURBOC__) || defined(_MSC_VER)
/*
 * These are not really here!
 */
#undef  inport_l
#undef  outport_l
#undef  inport_l_s
#undef  outport_l_s
#endif

#endif  /* whole file */

