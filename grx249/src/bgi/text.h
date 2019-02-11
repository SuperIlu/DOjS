/**
 ** BCC2GRX  -  Interfacing Borland based graphics programs to LIBGRX
 ** Copyright (C) 1993-97 by Hartmut Schirmer
 **
 **
 ** Contact :                Hartmut Schirmer
 **                          Feldstrasse 118
 **                  D-24105 Kiel
 **                          Germany
 **
 ** e-mail : hsc@techfak.uni-kiel.de
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

#ifndef __BCC2GRX_TEXT_H__
#define __BCC2GRX_TEXT_H__

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef unsigned char  uchar;
typedef   signed char  schar;
typedef unsigned short _ushort;

#if defined(__TURBOC__) || (defined(_MSC_VER) && defined(_WIN32))
#include <io.h>
#else
#include <unistd.h>
#endif
#include "bccgrx00.h"

#define FirstUserFont    11
#define LastUserFont     (FirstUserFont+9)
#define FirstGrxFont     (LastUserFont+1)
#define LastGrxFont      (FirstGrxFont+9)
#define NrFonts          (LastGrxFont+1)
#define PreSkip          0x080

#ifdef __GNUC__
#define ZERO2ONE(chrsze) ((chrsze) ? : 1)
#else
#define ZERO2ONE(chrsze) ((chrsze) ? (chrsze) : 1)
#endif

#define BITMAP(f) ((f)==DEFAULT_FONT || ((f)>=FirstGrxFont && (f)<=LastGrxFont))

typedef struct {
  uchar width;
  _ushort *cmd;
} CharInfo;

/* -------------------------------------------------------------- */

typedef char FontNameTyp[4];

#ifdef __GNUC__
#define PACKED __attribute ((packed))
#elif defined(_MSC_VER)
#pragma pack(push,1)
#endif

#ifndef PACKED
#define PACKED
#endif

typedef struct FontFileHeader {
  _ushort     header_size PACKED;   /* Version 2.0 Header Format   */
  FontNameTyp font_name   PACKED;   /* Font Internal Name          */
  _ushort     font_size   PACKED;   /* filesize in byte            */
  uchar       font_major  PACKED,   /* Driver Version Information  */
		  font_minor  PACKED;
  uchar       min_major   PACKED,   /* BGI Revision Information    */
		  min_minor   PACKED;
} FontFileHeader;

typedef struct FontHeaderTyp {
  char    sig PACKED;            /* SIGNATURE byte                        */
  _ushort nchrs PACKED;          /* number of characters in file          */
  char    unused1 PACKED;        /* Currently Undefined                   */
  uchar   firstch PACKED;        /* first character in file               */
  _ushort cdefs PACKED;          /* offset to char definitions            */
  uchar   scan_flag PACKED;      /* <> 0 if set is scanable               */
  uchar   org_to_cap PACKED;     /* Height from origin to top of capitol  */
  uchar   org_to_base PACKED;    /* Height from origin to baseline        */
  schar   org_to_dec PACKED;     /* Height from origin to bot of decender */
  uchar   unused2[0x5] PACKED;   /* Currently undefined                   */
} FontHeaderTyp;

#undef PACKED
#if defined(_MSC_VER)
#pragma pack(pop)
#endif

extern int  __gr_text_height;
extern int  __gr_text_multx, __gr_text_divx,
		__gr_text_multy, __gr_text_divy;
extern int  __gr_text_usr_multx, __gr_text_usr_divx,
		__gr_text_usr_multy, __gr_text_usr_divy;

extern void *__gr_text_Fonts[];
#define Fonts __gr_text_Fonts
extern CharInfo *__gr_text_fntptr;
#define fntptr __gr_text_fntptr
extern char *__gr_text_StdFonts[];
#define StdFonts __gr_text_StdFonts

extern GrTextOption __gr_text_style;
#define Style __gr_text_style

extern struct textsettingstype __gr_text_setting;
#define TXT __gr_text_setting

#ifdef GRX_VERSION
extern GrFont *__gr_text_DefaultFonts[11];
#define DefaultFonts __gr_text_DefaultFonts
#endif

extern void __gr_text_init(void);
extern void __gr_text_vec(int *xx, int *yy, int XX, int YY, int len, uchar *textstring);
extern void __gr_text_bit(GrFont *fnt, int *xx, int *yy, int XX, int YY, int len, uchar *txt);
extern int  __gr_text_ChrFontInfo(void *Font, CharInfo *fntptr, int *height);
extern int  __gr_text_registerfont( int start, int stop, void *font);
extern int  __gr_text_installfont( int start, int stop, const char *name);

extern int __gr_text_Width(int len, const char  *txt);
extern int __gr_text_Height(int len, const char *txt);

#endif
