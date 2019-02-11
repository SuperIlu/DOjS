/**
 ** libgrx.h ---- GRX library private include file
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

#ifndef __LIBGRX_H_INCLUDED__
#define __LIBGRX_H_INCLUDED__

#define USE_GRX_INTERNAL_DEFINITIONS

/* The LCC compiler on Linux requires this */
#if defined(__LCC__) && defined(__linux__)
/* make alloca work ... */
#  define __USE_MISC
#endif

#ifdef _AIX
#define _BIG_ENDIAN
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef __GRX20_H_INCLUDED__
#include "grx20.h"
#endif

#ifndef NULL
#define NULL            ((void *)(0))
#endif

#ifndef FALSE
#define FALSE           0
#endif

#ifndef TRUE
#define TRUE            1
#endif

/*
** identify the compiler / system
** and check for special restrictions
*/
/* DEC alpha chips have special alignment
** restrictions. We'll have do care about them */
#if !defined(__alpha__) && defined(__alpha)
#define __alpha__ __alpha
#endif


/* Casting a lvalue on the left side of an assignment 
** causes error or warnings on several compilers:
**
** LCC v4.0
** Watcom C++ v11.0
** SUN cc v4.0
** GCC v > 3
*/
#if !defined(NO_LEFTSIDE_LVALUE_CAST) &&                  \
    (   defined(__LCC__)                                  \
     || defined(__WATCOMC__)                              \
     || (defined(__GNUC__) && (__GNUC__>=3))              \
     || defined(__SUNPRO_C)                               )
#define NO_LEFTSIDE_LVALUE_CAST
#endif
/* Casting a pointer on the left side of an assignment
** also cuses problems on several systems:
**
** LCC v4.0
** Watcom C++ v11.0
*/
#if !defined(NO_LEFTSIDE_PTR_CAST) &&                     \
    (   defined(__LCC__)                                  \
     || defined(__WATCOMC__)                              )
#define NO_LEFTSIDE_PTR_CAST
#endif

/* some CPU allow misaligned access to non byte location */
#if   defined(__TURBOC__) \
   || defined(_MSC_VER) \
   || defined(__386__) \
   || defined(__i386__) \
   || defined(__i386)  \
   || defined(__x86_64__)
   /* x86 can write to misalgined 16bit locations */
#  define MISALIGNED_16bit_OK
#endif

#if   defined(__386__) \
   || defined(__i386__) \
   || defined(__i386)	\
   || defined(__x86_64__)
   /* x86 can write to misalgined 32bit locations */
#  define MISALIGNED_32bit_OK
#endif


/* need some n-bit types */
/* char should always be 8bit and short 16bit ... */
#define GR_int8  char
#define GR_int16 short
#if defined(__alpha__) || (defined(_MIPS_SZLONG) && _MIPS_SZLONG == 64)	|| defined(__x86_64__)
#define GR_int32 int
#define GR_int64 long
#define GR_PtrInt long
#else
#define GR_int32 long
#define GR_PtrInt int
#endif

/* signed and unsigned variants of the above */
typedef   signed GR_int8  GR_int8s;
typedef   signed GR_int16 GR_int16s;
typedef   signed GR_int32 GR_int32s;
typedef unsigned GR_int8  GR_int8u;
typedef unsigned GR_int16 GR_int16u;
typedef unsigned GR_int32 GR_int32u;
#ifdef GR_int64
typedef   signed GR_int64 GR_int64s;
typedef unsigned GR_int64 GR_int64u;
#endif


/*
** get system endian
*/
#if !defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)
#  if   defined(__TURBOC__) \
     || defined(__WATCOMC__) \
     || defined(_MSC_VER) \
     || defined(__alpha__) \
     || (defined(__LCC__) && defined(__i386__)) \
     || (defined(__GNUC__) && \
          (defined(__i386__) || defined(__x86_64__)))
#    define _LITTLE_ENDIAN
#  else
#    include <asm/byteorder.h>
#    ifdef __LITTLE_ENDIAN
#      define _LITTLE_ENDIAN
#    endif
#    ifdef __BIG_ENDIAN
#      define _BIG_ENDIAN
#    endif
#  endif
#endif

#if defined(__BYTE_ORDER__) && !defined(BYTE_ORDER)
#  define BYTE_ORDER    __BYTE_ORDER__
#  define LITTLE_ENDIAN __LITTLE_ENDIAN__
#  define BIG_ENDIAN    __BIG_ENDIAN__
#endif
#if !defined(BYTE_ORDER) && defined(_LITTLE_ENDIAN)
#define LITTLE_ENDIAN 0x1234
#define BIG_ENDIAN    0x4321
#define BYTE_ORDER    LITTLE_ENDIAN
#endif
#if !defined(BYTE_ORDER) && defined(_BIG_ENDIAN)
#define LITTLE_ENDIAN 0x1234
#define BIG_ENDIAN    0x4321
#define BYTE_ORDER    BIG_ENDIAN
#endif
#ifndef BYTE_ORDER
#error Unknown byte ordering !
#endif

#ifndef HARDWARE_BYTE_ORDER
#define HARDWARE_BYTE_ORDER BYTE_ORDER
#endif

/*
 * Debug support
 */
#if defined(DEBUG) && !defined(__GRXDEBUG_H_INCLUDED__)
# include "grxdebug.h"
#endif
#ifndef DBGPRINTF
# define DBGPRINTF(chk,x)
# define GRX_ENTER()
# define GRX_LEAVE()
# define GRX_RETURN(x) return x
#endif

/* simple pointer arithmetic */
#define ptrdiff(a,b) ( ((GR_int8 far *)(a)) - ((GR_int8 far *)(b)) )
#define ptradd(P,SKIP) ( (void *)( ((GR_int8 *)(P))+(SKIP)) )
#ifdef NO_LEFTSIDE_LVALUE_CAST
#define ptrinc(P,SKIP) do (P) = ptradd((P),(SKIP)); while (0)
#else
#define ptrinc(P,SKIP) do ((GR_int8 *)(P)) += (SKIP); while (0)
#endif

/*
 * function inline
 */
#ifdef __GNUC__
#define INLINE __inline__
#endif
#ifndef INLINE
#define INLINE
#endif


/*
 * global library data structures
 */
extern  struct _GR_driverInfo  _GrDriverInfo;
extern  struct _GR_contextInfo _GrContextInfo;
extern  struct _GR_colorInfo   _GrColorInfo;
extern  struct _GR_mouseInfo   _GrMouseInfo;

#define GrDriverInfo    (&_GrDriverInfo)
#define GrContextInfo   (&_GrContextInfo)
#define GrColorInfo     (&_GrColorInfo)
#define GrMouseInfo     (&_GrMouseInfo)

#define DRVINFO         (&_GrDriverInfo)
#define CXTINFO         (&_GrContextInfo)
#define CLRINFO         (&_GrColorInfo)
#define MOUINFO         (&_GrMouseInfo)

#define CURC            (&(CXTINFO->current))
#define SCRN            (&(CXTINFO->screen))
#define FDRV            (&(DRVINFO->fdriver))
#define SDRV            (&(DRVINFO->sdriver))
#define VDRV            ( (DRVINFO->vdriver))

/*
 * banking stuff
 */
#ifndef BANKHOOK
#define BANKHOOK
#endif

#ifndef RWBANKHOOK
#define RWBANKHOOK
#endif

#ifdef __TURBOC__
#  define BANKPOS(offs)   ((unsigned short)(offs))
#  define BANKNUM(offs)   (((unsigned short *)(&(offs)))[1])
#  define BANKLFT(offs)   (_AX = -(int)(BANKPOS(offs)),(_AX ? _AX : 0xffffU))
#endif

#ifndef BANKPOS
#define BANKPOS(offs)   ((GR_int16u)(offs))
#endif
#ifndef BANKNUM
#define BANKNUM(offs)   ((int)((GR_int32u)(offs) >> 16))
#endif
#ifndef BANKLFT
#define BANKLFT(offs)   (0x10000 - BANKPOS(offs))
#endif

#define SETBANK(bk) do {                            \
	register int _bankval_ = (bk);              \
	DRVINFO->curbank = _bankval_;               \
	(*DRVINFO->setbank)(_bankval_);             \
	BANKHOOK;                                   \
} while(0)

#define SRWBANK(rb,wb) do {                         \
	DRVINFO->curbank = (-1);                    \
	(*DRVINFO->setrwbanks)((rb),(wb));          \
	RWBANKHOOK;                                 \
} while(0)

#define CHKBANK(bk) do {                            \
	register int _bankval_ = (bk);              \
	if(_bankval_ != DRVINFO->curbank) {         \
	DRVINFO->curbank = _bankval_;               \
	(*DRVINFO->setbank)(_bankval_);             \
	BANKHOOK;                                   \
	}                                           \
} while(0)

/*
 * color stuff
 */
extern int _GR_firstFreeColor; /* can't access all colors on all systems */
extern int _GR_lastFreeColor;  /* eg. X11 and other windowing systems    */
int _GrResetColors(void);      /* like GrResetColors but return true on success */

#ifdef __TURBOC__
#  define C_OPER(color)   (unsigned int)(((unsigned char *)(&(color)))[3] & 15)
#endif

#ifndef C_OPER
#define C_OPER(color)   (unsigned int)(((GrColor)(color) >> 24) & 15)
#endif
#define C_WRITE         (int)(GrWRITE >> 24)
#define C_XOR           (int)(GrXOR   >> 24)
#define C_OR            (int)(GrOR    >> 24)
#define C_AND           (int)(GrAND   >> 24)
#define C_IMAGE         (int)(GrIMAGE >> 24)
#define C_COLOR         GrCVALUEMASK

/*
 * mouse stuff
 */
#define mouse_block(c,x1,y1,x2,y2) {                                        \
	int __mouse_block_flag = 0;                                         \
	mouse_addblock(c,x1,y1,x2,y2);
#define mouse_addblock(c,x1,y1,x2,y2)                                       \
	if(MOUINFO->docheck && (c)->gc_onscreen) {                          \
	__mouse_block_flag |= (*MOUINFO->block)((c),(x1),(y1),(x2),(y2));   \
	}
#define mouse_unblock()                                                     \
	if(__mouse_block_flag) {                                            \
	(*MOUINFO->unblock)(__mouse_block_flag);                            \
	}                                                                       \
}

/*
 * internal utility functions
 */
GrFrameDriver *_GrFindFrameDriver(GrFrameMode mode);
GrFrameDriver *_GrFindRAMframeDriver(GrFrameMode mode);

void _GrCloseVideoDriver(void);
void _GrDummyFunction(void);


#endif  /* whole file */

