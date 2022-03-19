/*
	File:		mgliContext.h

	Contains:	GLI interface for Mesa.

	Written by:	Fazekas Mikl—s

	Copyright:	Copyright(C) 1995-98 Miklos Fazekas
				E-Mail: boga@valerie.inf.elte.hu
				WWW: http://valerie.inf.elte.hu/~boga/Mesa.html
				
				Part of the Mesa 3D Graphics Library for MacOS.

	Change History (most recent first):

         <2>     4/27/99    miklos  Revision for Mesa 3.1b6.
         <1>     4/10/99    miklos  Added support for TGLIDrawable.
         <2>      4/8/98    miklos  Implemented revised rendering id's.
         <1>     1/24/98    miklos  Initial Revision
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

#ifndef __mgliContext_h__
#define __mgliContext_h__

#include "gli.h"

#include "MDD.h"

/* OpenGL */
#include <gl.h>
#include <agl.h>

/* Mesa */
#include "types.h"

#ifdef _cplusplus
extern "C" {
#endif

#define kMesaID		kMDDGlobalRenderingID

/************************************************************************************************
 *
 * The __GLIContextRec datatype.
 *
 ***********************************************************************************************/
typedef struct MGLIContextRec{
	GLcontext 		*glContext;		/* The Mesa context */
	GLframebuffer 	*framebuffer;	/* The Mesa framebuffer */
	GLvisual		*visual;		/* The Mesa visual */
	
	/*
	 * Drawable informations:
	 */
	TGLIDrawable 				drawable;	/* The drawable we are currently drawing onto */
	
	/*
	 * MDD private structures:
	 */
	TMDDEngine					*engine; 		/* Mesa Device Driver, 
												   engine */
	/*
	 * Validation information:
	 */
	long 			mesaID;				/* 4 bytes containing the kMesaID word */
} MGLIContextRec;

/*
 * mgliIsValidContext: return GL_FALSE if the inCtx is an invalid context.
 */
extern GLboolean mgliValidMesaContext(GLIContext inCtx);
/*
 * mgliIsValidContext: installs an engine as an MDDEngine.
 */
extern GLboolean	mgliInstallMDDEngine(TMDDEngine		*engine);

#ifdef _cplusplus
}
#endif

#endif /* __mgliContext_h__ */