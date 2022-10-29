/**
 ** libgrx.h ---- MGRX library private include file
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
 ** Modifications
 ** 071201 Introduction of GR_PtrInt (integer of same length as a pointer)
 **        to suppress warnings (in fact errors) when compiling with
 **        x86_64 platforms. Backport from GRX 2.4.7 (M.Lombardi)
 ** 170630 Added _GrIniUserEncoding and _GrRecode protoptypes
 ** 190813 Define here sttcopy and sttzero using string.h routines
 ** 190818 Solved a bug in getting BYTE_ORDED showed in win32 after remove of
 **        assembler code in memfill.h
 ** 190829 Added ARM support
 **/

#ifndef __LIBGRX_H_INCLUDED__
#define __LIBGRX_H_INCLUDED__

#define USE_GRX_INTERNAL_DEFINITIONS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef __MGRX_H_INCLUDED__
#include "mgrx.h"
#endif

#ifndef NULL
#define NULL ((void *)(0))
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* x86 allow misaligned access to non byte location */
#if defined(__i386__) || defined(__x86_64__) || defined(__arm__)
#define MISALIGNED_16bit_OK
#define MISALIGNED_32bit_OK
#endif

/* need some n-bit types */
/* char should always be 8bit and short 16bit ... */
#define GR_int8 char
#define GR_int16 short
#if defined(__x86_64__)
#define GR_int32 int
#define GR_int64 long
#define GR_PtrInt long
#else
#define GR_int32 long
#define GR_PtrInt int
#endif

/* signed and unsigned variants of the above */
typedef signed GR_int8 GR_int8s;
typedef signed GR_int16 GR_int16s;
typedef signed GR_int32 GR_int32s;
typedef unsigned GR_int8 GR_int8u;
typedef unsigned GR_int16 GR_int16u;
typedef unsigned GR_int32 GR_int32u;
#ifdef GR_int64
typedef signed GR_int64 GR_int64s;
typedef unsigned GR_int64 GR_int64u;
#endif

/* get system endian */
#if !defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__) || defined(__arm__))
#define _LITTLE_ENDIAN
#else
#include <sys/byteorder.h>
#endif
#endif

#if defined(__BYTE_ORDER__) && !defined(BYTE_ORDER)
#define BYTE_ORDER __BYTE_ORDER__
#define LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#define BIG_ENDIAN __ORDER_BIG_ENDIAN__
#endif

#if !defined(BYTE_ORDER) && defined(_LITTLE_ENDIAN)
#define LITTLE_ENDIAN 0x1234
#define BIG_ENDIAN 0x4321
#define BYTE_ORDER LITTLE_ENDIAN
#endif

#if !defined(BYTE_ORDER) && defined(_BIG_ENDIAN)
#define LITTLE_ENDIAN 0x1234
#define BIG_ENDIAN 0x4321
#define BYTE_ORDER BIG_ENDIAN
#endif

#ifndef BYTE_ORDER
#error Unknown byte ordering !
#endif

/* sttcopy and sttzero */

#define sttcopy(dstp, srcp) memcpy((dstp), (srcp), sizeof(*(srcp)))
#define sttzero(p) memset((p), 0, sizeof(*(p)))

/* Debug support */
#if defined(DEBUG) && !defined(__GRXDEBUG_H_INCLUDED__)
#include "grxdebug.h"
#endif

#ifndef DBGPRINTF
#define DBGPRINTF(chk, x)
#define GRX_ENTER()
#define GRX_LEAVE()
#define GRX_RETURN(x) return x
#endif

/* simple pointer arithmetic */
#define ptrdiff(a, b) (((GR_int8 *)(a)) - ((GR_int8 *)(b)))
#define ptradd(P, SKIP) ((void *)(((GR_int8 *)(P)) + (SKIP)))
#define ptrinc(P, SKIP)           \
    do (P) = ptradd((P), (SKIP)); \
    while (0)

/* function inline */
#ifdef __GNUC__
#define INLINE __inline__
#endif
#ifndef INLINE
#define INLINE
#endif

/* global library data structures */
extern struct _GR_driverInfo _GrDriverInfo;
extern struct _GR_contextInfo _GrContextInfo;
extern struct _GR_colorInfo _GrColorInfo;
extern struct _GR_mouseInfo _GrMouseInfo;

#define GrDriverInfo (&_GrDriverInfo)
#define GrContextInfo (&_GrContextInfo)
#define GrColorInfo (&_GrColorInfo)
#define GrMouseInfo (&_GrMouseInfo)

#define DRVINFO (&_GrDriverInfo)
#define CXTINFO (&_GrContextInfo)
#define CLRINFO (&_GrColorInfo)
#define MOUINFO (&_GrMouseInfo)

#define CURC (&(CXTINFO->current))
#define SCRN (&(CXTINFO->screen))
#define FDRV (&(DRVINFO->fdriver))
#define SDRV (&(DRVINFO->sdriver))
#define VDRV ((DRVINFO->vdriver))

/* banking stuff */
#ifndef BANKHOOK
#define BANKHOOK
#endif

#ifndef RWBANKHOOK
#define RWBANKHOOK
#endif

#ifndef BANKPOS
#define BANKPOS(offs) ((GR_int16u)(offs))
#endif

#ifndef BANKNUM
#define BANKNUM(offs) ((int)((GR_int32u)(offs) >> 16))
#endif

#ifndef BANKLFT
#define BANKLFT(offs) (0x10000 - BANKPOS(offs))
#endif

#define SETBANK(bk)                     \
    do {                                \
        register int _bankval_ = (bk);  \
        DRVINFO->curbank = _bankval_;   \
        (*DRVINFO->setbank)(_bankval_); \
        BANKHOOK;                       \
    } while (0)

#define SRWBANK(rb, wb)                     \
    do {                                    \
        DRVINFO->curbank = (-1);            \
        (*DRVINFO->setrwbanks)((rb), (wb)); \
        RWBANKHOOK;                         \
    } while (0)

#define CHKBANK(bk)                          \
    do {                                     \
        register int _bankval_ = (bk);       \
        if (_bankval_ != DRVINFO->curbank) { \
            DRVINFO->curbank = _bankval_;    \
            (*DRVINFO->setbank)(_bankval_);  \
            BANKHOOK;                        \
        }                                    \
    } while (0)

/* color stuff */
extern int _GR_firstFreeColor; /* can't access all colors on all systems */
extern int _GR_lastFreeColor;  /* eg. X11 and other windowing systems    */
int _GrResetColors(void);      /* like GrResetColors but return true on success */

#define C_OPER(color) (unsigned int)(((GrColor)(color) >> 24) & 15)
#define C_WRITE (int)(GrWRITE >> 24)
#define C_XOR (int)(GrXOR >> 24)
#define C_OR (int)(GrOR >> 24)
#define C_AND (int)(GrAND >> 24)
#define C_IMAGE (int)(GrIMAGE >> 24)
#define C_COLOR GrCVALUEMASK

/* mouse stuff */
#define mouse_block(c, x1, y1, x2, y2) \
    {                                  \
        int __mouse_block_flag = 0;    \
        mouse_addblock(c, x1, y1, x2, y2);
#define mouse_addblock(c, x1, y1, x2, y2)                                     \
    if (MOUINFO->docheck && (c)->gc_onscreen) {                               \
        __mouse_block_flag |= (*MOUINFO->block)((c), (x1), (y1), (x2), (y2)); \
    }
#define mouse_unblock()                          \
    if (__mouse_block_flag) {                    \
        (*MOUINFO->unblock)(__mouse_block_flag); \
    }                                            \
    }

/* internal utility functions */

GrFrameDriver *_GrFindFrameDriver(GrFrameMode mode);
GrFrameDriver *_GrFindRAMframeDriver(GrFrameMode mode);

GrPixmap *_GrCreatePixmap(GrContext *ctx, int w, int h);
GrColor _GrRadGradientColor(GrGradient *g, int x, int y, int xo, int yo);
GrColor _GrLinGradientColor(GrGradient *g, int x, int y, int xo, int yo);

void _GrCloseVideoDriver(void);
void _GrDummyFunction(void);

void _GrIniUserEncoding(void); /* called in GrSetMode */

int _GrRecode_CP437_UCS2(unsigned char src, long *des);
int _GrRecode_CP850_UCS2(unsigned char src, long *des);
int _GrRecode_CP1252_UCS2(unsigned char src, long *des);
int _GrRecode_ISO88591_UCS2(unsigned char src, long *des);
int _GrRecode_UTF8_UCS2(unsigned char *src, long *des);
int _GrRecode_mgrx512_UCS2(unsigned short src, long *des);
int _GrRecode_ISO88595_UCS2(unsigned char src, long *des);
int _GrRecode_ISO88597_UCS2(unsigned char src, long *des);
int _GrRecode_CP437Ext_UCS2(unsigned short src, long *des);
int _GrRecode_UCS2_CP437(long src, unsigned char *des);
int _GrRecode_UCS2_CP850(long src, unsigned char *des);
int _GrRecode_UCS2_CP1252(long src, unsigned char *des);
int _GrRecode_UCS2_ISO88591(long src, unsigned char *des);
int _GrRecode_UCS2_UTF8(long src, unsigned char *des);
int _GrRecode_UCS2_mgrx512(long src, long *des);
int _GrRecode_UCS2_ISO88595(long src, unsigned char *des);
int _GrRecode_UCS2_ISO88597(long src, unsigned char *des);
int _GrRecode_UCS2_CP437Ext(long src, long *des);

#ifdef __XWIN__
void _GrXwinEventGenExpose(int when);
void _GrXwinEventGenWMEnd(int when);
#endif

#ifdef __WIN32__
void _GrW32EventGenWMEnd(int when);
#endif

#endif /* whole file */
