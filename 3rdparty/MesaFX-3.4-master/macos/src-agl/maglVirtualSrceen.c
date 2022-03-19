/*
	File:		maglVirtualSrceen.c

	Contains:	Virtual screen related functions. AGL for GLI.

	Written by:	Mikl—s Fazekas

	Copyright:	Copyright(C) 1995-99 Miklos Fazekas
				E-Mail: boga@valerie.inf.elte.hu
				WWW: http://www.mesa3d.org/mac/
				
				Part of the Mesa 3D Graphics Library for MacOS.


	Change History (most recent first):

         <1>      4/8/99    miklos  Revision for AGL 2.0
         <4>     4/19/98    miklos  Fixed compiler warnings.
         <3>      4/7/98    miklos  Revision for Mesa 3.0b4
         <2>    12/27/97    miklos  Inital Revision
         <1>    11/30/97    miklos  
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

/* OpenGL */
#include <agl.h>
#include <gliMesa.h>
/***********************************************************************************************
 *
 * Function:	aglGetVirtualScreen
 * Parameters:	ctx					- The open gl context.
 * Returns:		The virtual screen numer  if succeeded, -1 if fails for any reason
 *
 * Description:	aglGetVirtualScreen may be used on multi-monitor systems to find which virtual
 *			screen is associated with the OpenGL renderer that is currently processing OpenGL 
 *			commands. On a single monitor system aglGetVirtualScreen always returns zero. 
 *			The current virtual screen is normally set automatically by aglUpdateCurrent to 
 *			be the virtual screen that includes the smallest set of graphics devices that
 *			contain the entire drawable, so the current virtual screen may change when the
 *			drawable is moved or resized across graphics device boundaries. A change in 
 *			the current virtual screen may effect the return values of some OpenGL functions.
 *
 *  Notes:	Each virtual screen includes one or more Mac OS graphics devices. Virtual screen 
 *          zero of a particular AGL context always includes all graphics devices that are 
 *			supported by the context and all other virtual screens include non-intersecting 
 *			subsets of those devices. The total number of virtual screens is less than or 
 *			equal to the number of graphics devices plus one. There is one OpenGL renderer 
 *			and one pixel format associated with each virtual screen - OpenGL commands are
 *			processed by the renderer associated with the current virtual screen. The 
 *			relationship between virtual screens and their respective renderers and pixel 
 *			formats is determined entirely by aglChoosePixelFormat.
 *			The virtual screen number and OpenGL renderer ID associated with a specific 
 *			pixel format are found by passing aglDescribePixelFormat the AGL_VIRTUAL_SCREEN 
 *			and AGL_RENDERER_ID attributes, respectively, and the set of graphics devices 
 *			associated with a pixel format is found with aglDevicesOfPixelFormat. 
 *			aglNextPixelFormat and aglDescribePixelFormat can be used repeatedly to examine
 * 			all the pixel formats returned by aglChoosePixelFormat.
 *
 * 	Errors:	AGL_BAD_CONTEXT			 if ctx is not a valid context.
 *
 ***********************************************************************************************/

GLint aglGetVirtualScreen(AGLContext ctx)
{
#pragma unused(ctx)
	return 0;	/* TODO */
}
/***********************************************************************************************
 *
 * Function:	aglSetVirtualScreen
 * Parameters:	ctx					- The open gl context.
 *				screen				- Specifies the virtual screen number.
 * Returns:		GL_TRUE if succeed, GL_FALSE if fails for any reason
 *
 * Description:	aglSetVirtualScreen may be used on multi-monitor systems to specify the 
 *			virtual screen, and associated OpenGL renderer, that will subsequently 
 *			process OpenGL commands. The current virtual screen is normally set automatically
 *			by aglSetDrawable or aglUpdateContext to be the virtual screen that includes the 
 *			smallest set of graphics devices that contain the entire drawable.
 *   		aglSetVirtualScreen should be used only when it is necessary to override 
 *			the default behavior.
 *
 *  Notes:	Each virtual screen includes one or more Mac OS graphics devices. Virtual screen 
 *          zero of a particular AGL context always includes all graphics devices that are 
 *			supported by the context and all other virtual screens include non-intersecting 
 *			subsets of those devices. The total number of virtual screens is less than or 
 *			equal to the number of graphics devices plus one. There is one OpenGL renderer 
 *			and one pixel format associated with each virtual screen - OpenGL commands are
 *			processed by the renderer associated with the current virtual screen. The 
 *			relationship between virtual screens and their respective renderers and pixel 
 *			formats is determined entirely by aglChoosePixelFormat.
 *			The virtual screen number and OpenGL renderer ID associated with a specific 
 *			pixel format are found by passing aglDescribePixelFormat the AGL_VIRTUAL_SCREEN 
 *			and AGL_RENDERER_ID attributes, respectively, and the set of graphics devices 
 *			associated with a pixel format is found with aglDevicesOfPixelFormat. 
 *			aglNextPixelFormat and aglDescribePixelFormat can be used repeatedly to examine
 * 			all the pixel formats returned by aglChoosePixelFormat.
 *
 * 	Errors:	AGL_BAD_CONTEXT			 if ctx is not a valid context.
 *
 ***********************************************************************************************/

GLboolean aglSetVirtualScreen(AGLContext ctx,
							 GLint screen )
{
#pragma unused(screen)
#pragma unused(ctx)
	return GL_FALSE; /* TODO */
}