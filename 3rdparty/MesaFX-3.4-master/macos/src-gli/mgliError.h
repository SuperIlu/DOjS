/*
	File:		mgliError.h

	Contains:	Error handling functions for Mesa.

	Written by:	Fazekas Mikl—s

	Copyright:	Copyright(C) 1995-98 Miklos Fazekas
				E-Mail: boga@valerie.inf.elte.hu
				WWW: http://valerie.inf.elte.hu/~boga/Mesa.html
				
				Part of the Mesa 3D Graphics Library for MacOS.

	Change History (most recent first):

        <4+>      7/2/98    miklos  Email address changed.
         <4>      3/2/98    miklos  Fixed a compiler warning.
         <3>      3/2/98    miklos  Don't do asserts in optimized mode.
         <2>      3/2/98    miklos  Don't do asserts in optimized mode.
         <1>     1/29/98    miklos  Initial Revision
*/



/*
** AGL driver for Mesa 3D Graphics Library.
** Version: 0.1
** Mesa version: 2.4
**
** Copyright(C) 1995-97 Miklos Fazekas
** E-Mail: boga@augusta.inf.elte.hu
** WWW: http://www.elte.hu/~boga/Mesa.html
** 
** Part of the Mesa 3D Graphics Library for MacOS.
**
*/
 
/*
** Copyright 1995-97, Mikl—s Fazekas.
** All Rights Reserved.
** 
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the Free
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __MGLI_ERROR_H__
#define __MGLI_ERROR_H__

#include "mgliConfig.h"

#include "gliMesa.h"


typedef enum TMGLIError {
		kMGLINoError		= 0,				/* No error */
		kMGLIError			= 1,				/* Generic error flag */
		kMGLIOutOfMemory	= 2,				/* Insufficient memory */
		kMGLIBadAttribute	= 3,				/* Unknown pixel format attribute  */
		kMGLIBadPixelFormat = 4,				/* Invalid pixel fmt specified */
		kMGLIInvalidContext	= 5,				/* Invalid context specified */
		kMGLIInvalidShare	= 6,				/* Invalid share context match */
		kMGLIAssertFailed	= 7,				/* Assertion failed */
		kMGLIGLError		= 8,
		
		kMGLIMDDError		= 101				/* MDD Generic error flag */					
} TMGLIError;

/*
 * Set's the OpenGL error to the error-code defined by error.
 */
extern void mgliSetGLError(GLIContext glContext,GLenum error, const char *debugt_str);

extern void mgliFatalError(TMGLIError error, const char *msg, ...);

extern void mgliWarning(TMGLIError error,const char *msg, ...);

extern void mgliError(TMGLIError error, const char *msg, ...);

#if GLI_DEBUG
#define mgliAssert(condition) ((condition) ? ((void) 0) : mgliError(kMGLIAssertFailed,"Assertion failed: "#condition" is not true, at file %s, line %d:", __FILE__, __LINE__))
#else
#define mgliAssert(condition)	((void) 0) 
#endif


#endif /* __MGLI_ERROR_H__ */