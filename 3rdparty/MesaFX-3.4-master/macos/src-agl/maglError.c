/*
	File:		maglError.c

	Contains:	Error managing functions.

	Written by:	Miklos Fazekas

	Copyright:	Copyright(C) 1995-97 Miklos Fazekas
				E-Mail: boga@valerie.inf.elte.hu
				WWW: http://www.mesa3d.org/mac/Mesa.html
				
				Part of the Mesa 3D Graphics Library for MacOS.

	Change History (most recent first):

         <4>     4/13/99    miklos  Removed aglSet,GetInteger and aglIsEnabled.
         <3>     4/10/99    miklos  Added stub aglSetInteger,aglGetInteger calls.
         <2>      4/8/99    miklos  Revision for AGL 2.0
         <1>      4/8/99    miklos
                                    Error managing functions for AGL.
*/


/*
	File:		maglError.c

	Contains:	Error managing functions.

	Written by:	Miklos Fazekas

	Copyright:	Copyright(C) 1995-97 Miklos Fazekas
				E-Mail: boga@valerie.inf.elte.hu
				WWW: http://www.mesa3d.org/mac/Mesa.html
				
				Part of the Mesa 3D Graphics Library for MacOS.

	Change History (most recent first):

         <2>      4/8/99    miklos  Revision for AGL 2.0
         <1>      4/8/99    miklos
                                    Error managing functions for AGL.
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


#include "maglConfig.h"
#include "maglMemory.h"
#include "maglError.h"
#include "maglRenderers.h"
#include "maglValidate.h"

/* ANSI C */
#include <stdlib.h>
#include <stdarg.h> 
#include <string.h>
#include <stdio.h>


/***********************************************************************************************
 *
 * Globals:
 *
 ***********************************************************************************************/


static GLenum	gAGLError 			= (GLenum)AGL_NO_ERROR;		/* The current OpenGL-error, the AGL specifications says, that we shoudl
																   use global and non-local error handling */
			
static char     gStrTemp[255];  								/* This holds the string returned from MAGL_ParseArgs */

/***********************************************************************************************
 *
 * MAGL_ParseArgs
 *
 * Parameters:	inDebugMsg		- printf style format
 *				...				- see printf.
 *
 * Description: Makes one C string from the inDebugMsg. (It uses gStrTemp, so a time only
 *  			only one of this ParseArgs can be called, or it results should be copied)
 *				DON'T free its' result c-string! The maximal length can be 255, otherwise
 *				this function will crash!
 *
 ***********************************************************************************************/

extern char* 	MAGL_ParseArgs(								/* Parse the printf like debug str to a c string */
						const char 			*inDebugMsg,
						...	)
{
	va_list args;

	va_start(args, inDebugMsg);
	vsprintf(&gStrTemp[0],inDebugMsg,args);
	va_end(args);
	
	return &gStrTemp[0];
}
/***********************************************************************************************
 *
 * MAGL_FatalErrorDebug
 *
 * Parameters:	ctx				- context
 *				inDebugMsg		- this will be rinted on error
 *				inFileName		- the name of the source file
 *				inLineNum		- the number of the source file
 *				
 *				...				- see printf.
 *
 * Description: This function will drops you to the debugger if AGL_DEBUG defined, whith the
 *				debug message defined in inDebugMsg.
 *				And quits the application. (both in debug/non-debug mode)
 *
 ***********************************************************************************************/
void MAGL_DoFatalError(
				AGLContext 			ctx,
				char 				*inDebugMsg,
				char 				*inFile,
				int 				inLineNum)
{
#pragma unused(ctx)
	MAGL_DebugMsg(inDebugMsg,inFile,inLineNum);
	
	abort();
}

/***********************************************************************************************
 *
 * MAGL_RegisterAGLError
 *
 * Parameters:	inDebugMsg		- string to dispaly
 *				inFile			- source file
 *				inLine			- source line
 *
 * Description: Displays a debug message (trought DebugStr)
 *
 ***********************************************************************************************/
extern void 	MAGL_DebugMsg(								
						const char 			*inDebugMsg,
						const char			*inFile,
						int					inLine)
{
	char error_msg[256];
	
	sprintf(&error_msg[1],"MAGLError: %s, in file: %s, line: %d!\n",inDebugMsg,inFile,inLine);
	error_msg[0] = strlen(&error_msg[1]);
	DebugStr((ConstStr255Param)&error_msg[0]);
}

/***********************************************************************************************
 *
 * MAGL_RegisterAGLError
 *
 * Parameters:	ctx				- context
 *				error			- AGL-error
 *
 * Description: This functions registers the global error code.
 *
 ***********************************************************************************************/

void MAGL_RegisterAGLError(
			AGLContext 			ctx,		
			GLenum 				error)
{
#pragma unused(ctx)
	if (gAGLError == AGL_NO_ERROR)
		gAGLError = error;
}

/***********************************************************************************************
 *
 * MAGL_InitError
 *
 * Parameters:	ctx				- context
 *
 * Description: This functions is called at context initialization, its currently a NoOp (TODO)
 *
 ***********************************************************************************************/
void MAGL_InitError(AGLContext ctx)
{
#pragma unused(ctx)
	/* NoOp */
}
/***********************************************************************************************
 *
 * MAGL_CloseError
 *
 * Parameters:	ctx				- context
 *
 * Description: This functions is called at context destroy, its currently a NoOp (TODO)
 *
 ***********************************************************************************************/
void MAGL_CloseError(AGLContext ctx)
{
#pragma unused(ctx)
	/* NoOp */
}

/***********************************************************************************************
 *
 * aglGetError (agl-api call)
 *
 * Description: Return error information.   
 *			aglGetError returns the value of the AGL error flag for the current context.  
 *			Each error is assigned a numeric code and symbolic name.  When an error occurs, 
 *			the error flag is set to the appropriate error code value.  
 *			No other errors are recorded until aglGetError is called, the error code is 
 *			returned, and the flag is reset to AGL_NO_ERROR.  If a call to aglGetError 
 *			returns AGL_NO_ERROR, there has been no detectable error since the last call 
 *			to aglGetError, or since the context was initialized.
 *
 ***********************************************************************************************/
GLenum aglGetError(void)
{
	GLenum error;
	
	error = gAGLError;
	return error;
}
/***********************************************************************************************
 *
 * aglErrorString (agl-api call)
 *
 * Description: Return an error string for an AGL error code.   
 *			aglErrorString produces an error string from an AGL error code. 
 *			The standard AGL error codes are AGL_NO_ERROR and all the numical codes between 
 *			AGL_BAD_ATTRIBUTE and AGL_BAD_ALLOC, inclusive. 
 *			aglErrorString always returns a string, even if code is invalid.
 *
 ***********************************************************************************************/

 const GLubyte *aglErrorString ( GLenum code )
 {
 	switch (code) {
 		case AGL_NO_ERROR:	
 			return "\pAGL_NO_ERROR - no error";
 		case AGL_BAD_ATTRIBUTE:
 			return "\pAGL_BAD_ATTRIBUTE - invalid pixel format attribute";
 		case AGL_BAD_PROPERTY:
 			return "\pAGL_BAD_PROPERTY - invalid renderer property";
 		case AGL_BAD_PIXELFMT:
 			return "\pAGL_BAD_PIXELFMT - invalid pixel format";
 		case AGL_BAD_RENDINFO:
 			return "\pAGL_BAD_RENDINFO - invalid renderer info";
 		case AGL_BAD_CONTEXT:
 			return "\pAGL_BAD_CONTEXT - invalid context";
 		case AGL_BAD_DRAWABLE:
 			return "\pAGL_BAD_DRAWABLE - invalid drawable";
 		case AGL_BAD_GDEV:
 			return "\pAGL_BAD_GDEV - invalid graphics device";
 		case AGL_BAD_STATE:
 			return "\pAGL_BAD_STATE - invalid context state";
 		case AGL_BAD_VALUE:
 			return "\pAGL_BAD_VALUE - invalid numerical value";
 		case AGL_BAD_MATCH:
 			return "\pAGL_BAD_MATCH - invalid share context ";
 		case AGL_BAD_ENUM:
 			return "\pAGL_BAD_ENUM - invalid enumerant ";
 		case AGL_BAD_OFFSCREEN:
 			return "\pAGL_BAD_OFFSCREEN - invalid offscreen drawable";
		case AGL_BAD_FULLSCREEN:
			return "\pAGL_BAD_FULLSCREEN - invalid fullscreen drawable";
		case AGL_BAD_WINDOW:
			return "\pAGL_BAD_WINDOW - invalid window";
		case AGL_BAD_POINTER:
			return "\pAGL_BAD_POINTER - invalid pointer";
		case AGL_BAD_MODULE:
			return "\pAGL_BAD_MODULE - invalid code module";
		case AGL_BAD_ALLOC:
			return "\pAGL_BAD_ALLOC - memory allocation failure";
 		default:
 			return "\pAGL_UNKNOWN_ERROR";
 	}
 }