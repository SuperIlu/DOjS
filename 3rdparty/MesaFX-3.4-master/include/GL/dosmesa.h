/* $Id: dosmesa.h,v 1.2 1999/11/24 18:44:53 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  3.3
 * Copyright (C) 1995-1998  Brian Paul
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


/*
 * DOS/Mesa interface.
 */


/*
 * Intro to using the DOS/Mesa interface
 *
 * 1. #include the <vga.h> file
 * 2. Call vga_init() to initialize the DOS library.
 * 3. Call vga_setmode() to specify the screen size and color depth.
 * 4. Call DOSMesaCreateContext() to setup a Mesa context.  If using 8-bit
 *    color Mesa assumes color index mode, if using 16-bit or deeper color
 *    Mesa assumes RGB mode.
 * 5. Call DOSMesaMakeCurrent() to activate the Mesa context.
 * 6. You can now use the Mesa API functions.
 * 7. Before exiting, call DOSMesaDestroyContext() then vga_setmode(TEXT)
 *    to restore the original text screen.
 *
 * Notes
 * 1. You must run your executable as root (or use the set UID-bit) because
 *    the DOS library requires it.
 * 2. The DOS driver is not fully implemented yet.  See DOSmesa.c for what
 *    has to be done yet.
 */


#ifndef DOSMESA_H
#define DOSMESA_H


#define DOSMESA_MAJOR_VERSION 3
#define DOSMESA_MINOR_VERSION 3



#ifdef __cplusplus
extern "C" {
#endif


/*
 * This is the DOSMesa context 'handle':
 */
typedef struct DOSmesa_context *DOSMesaContext;



extern DOSMesaContext DOSMesaCreateContext( void );

extern void DOSMesaDestroyContext( DOSMesaContext ctx );

extern void DOSMesaMakeCurrent( DOSMesaContext ctx );

extern DOSMesaContext DOSMesaGetCurrentContext( void );



#ifdef __cplusplus
}
#endif

#ifndef BOOL
#define BOOL unsigned char
#endif

#define HWND unsigned long
#define HDC unsigned long
#define HPALETTE unsigned long
#define WCHAR char
#define PASCAL
#define UINT unsigned int
#define ULONG unsigned long
#define USHORT unsigned short
#define DWORD unsigned long
#define WORD unsigned short
#define LONG long




#endif

