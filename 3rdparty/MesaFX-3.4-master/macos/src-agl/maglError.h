/*
	File:		maglError.h

	Contains:	Error handler functions - AGL for GLI.

	Written by:	Mikl—s Fazekas

	Copyright:	Copyright(C) 1995-97 Miklos Fazekas
				E-Mail: boga@valerie.inf.elte.hu
				WWW: http://valerie.inf.elte.hu/~boga/Mesa.html
				
				Part of the Mesa 3D Graphics Library for MacOS.


	Change History (most recent first):

         <1>      4/8/99    miklos  Revision for AGL 2.0 headers.
         <5>     4/19/98    miklos  New debug/error handling functions.
         <4>     4/13/98    miklos  Changed tha AGL_DEBUG symbol to MAGL_DEBUG as used otherwise.
                                    (Jesse John)
         <3>      4/7/98    miklos  Revision for Mesa 3.0b4
         <2>      3/1/98    miklos  Added better error handling functions.
         <1>    11/29/97    miklos  
*/

/*
** Copyright 1995-98, Mikl—s Fazekas.
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

#ifndef __MAGL_ERROR_H__
#define __MAGL_ERROR_H__

#include "maglConfig.h"

#include <agl.h>
#include <gl.h>

#include "assert.h"

#ifdef MAGL_DEBUG

#define MAGL_SetError(ctx,error,debugstr)	   do { MAGL_RegisterAGLError(ctx,error); MAGL_DebugMsg(MAGL_ParseArgs debugstr, __FILE__,__LINE__);} while (0)
#define MAGL_Assert(condition) 				   ((condition) ? ((void) 0) : MAGL_DoFatalError(NULL,"Assertion falied:"#condition" Is not true!",__FILE__,__LINE__))
#define MAGL_Warning(ctx,debugstr)			   MAGL_DebugMsg( MAGL_ParseArgs debugstr,__FILE__,__LINE__)  
#define MAGL_FatalError(ctx,debugstr)		   MAGL_DoFatalError(ctx,MAGL_ParseArgs debugstr,__FILE__,__LINE__)

#else

#define MAGL_SetError(ctx,error,debugstr)		MAGL_RegisterAGLError(ctx,error)
#define MAGL_Assert(condition) 					((void) 0)
#define MAGL_Warning(ctx,debugstr)				((void) 0)
#define MAGL_FatalError(ctx,debugstr)		   MAGL_DoFatalError(ctx,MAGL_ParseArgs debugstr,__FILE__,__LINE__)

#endif

extern void		MAGL_DoFatalError(						/* Drops to the debugger in debug, and quits from the application */
						AGLContext 			ctx,
						char 				*inDebugMsg,
						char 				*inFile,
						int 				inLineNum);

extern char* 	MAGL_ParseArgs(								/* Parse the printf like debug str to a c string */
						const char 			*inDebugMsg,
						...	);

extern void 	MAGL_DebugMsg(								/* Drops to the debugger, and shows the debugMsg. */
						const char 			*inDebugMsg,
						const char			*inFile,
						int					inLine); 	
 
extern void 	MAGL_InitError(								/* Starts the error handling for the context ctx. */
						AGLContext 			ctx);				
extern void 	MAGL_CloseError(							/* Closes the error handler for the context ctx. */
						AGLContext 			ctx);
						
extern void MAGL_RegisterAGLError(AGLContext ctx,GLenum error);
/*
 * Registers an OpenGL error for the context ctx.
 * This is global and not per context!!!!
 */
 
#endif /* __MAGL_ERROR_H__ */