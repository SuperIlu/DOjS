/*
	File:		mgliError.c

	Contains:	GLI Error handling routines.

	Written by:	Miklos Fazekas

	Copyright:	Miklos Fazekas © 1999.  All rights reversed.
    See http://www.mesa3d.org/mac/ for more details.

	Change History (most recent first):

         <3>      4/6/99    miklos  Fixed compiler warning
         <2>      4/6/99    miklos  Fixed compiler warning
         <1>      4/3/99    miklos  Initial revision for Mesa 3.1b2.
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

#include "mgliError.h"

/* Mesa */
#include "types.h"
#include "context.h"
/* ANSI-C */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

void mgliSetGLError(GLIContext glContext,GLenum error, const char *debug_str)
{
	/* Give it to Mesa's error handler: */
	if (glContext == NULL)
	{
		gl_error((GLcontext*)glContext,error,debug_str);
	}
}

void mgliFatalError(TMGLIError error, const char *str, ...)
{
	char error_str[256];
	va_list args;
	(void)error;

	va_start(args, str);
	vsprintf(&error_str[1],str,args);
	va_end(args);
	
	error_str[0] = strlen(&error_str[1]);
	DebugStr((ConstStr255Param)error_str);
}
void mgliWarning(TMGLIError error,const char *str, ...)
{
#if (GLI_VERBOSE && GLI_DEBUG)
	char error_str[256];
	va_list args;

	va_start(args, str);
	vsprintf(&error_str[1],str,args);
	va_end(args);
	
	error_str[0] = strlen(&error_str[1]);
	DebugStr((ConstStr255Param)error_str);
#endif /* GLI_DEBUG && GLI_VERBOSE */
	(void)error;
	(void)str;
}
void mgliError(TMGLIError error, const char *str, ...)
{
#if GLI_DEBUG
	char error_str[256];
	va_list args;

	va_start(args, str);
	vsprintf(&error_str[1],str,args);
	va_end(args);
	
	error_str[0] = strlen(&error_str[1]);
	DebugStr((ConstStr255Param)error_str);
#endif
 	(void)error;
 	(void)str;
}
