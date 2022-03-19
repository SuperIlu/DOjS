/*
	File:		maglRendererInfo.c

	Contains:	Implements AGLRendererInfo, related routines. AGL for GLI.

	Written by:	Mikl—s Fazekas

	Copyright:	Copyright(C) 1995-99 Miklos Fazekas
				E-Mail: boga@valerie.inf.elte.hu
				WWW: http://www.mesa3d.org/mac/
				
				Part of the Mesa 3D Graphics Library for MacOS.


	Change History (most recent first):

         <1>      4/8/99    miklos  Revision for AGL 2.0
         <7>     4/19/98    miklos  Adopted new error handler functions.
         <6>     4/13/98    miklos  aglDescribeRenderer handles AGL_MAX_LEVEL (Jesse Jones).
         <5>      4/7/98    miklos  Revision for Mesa 3.0b4
         <4>      4/6/98    miklos  aglGetRendererInfos always returned NULL.
         <3>      2/5/98    miklos  Implemented aglGetRendererInfos.
         <2>    12/27/97    miklos  Inital Revision
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

#include "maglConfig.h"
#include "maglMemory.h"
#include "maglError.h"
#include "maglRenderers.h"

#include "aglContext.h"
#include <agl.h>
#include <gliMesa.h>

/* MacOS */
#include <Types.h>
#include <Memory.h>


/***********************************************************************************************
 *
 * Function:	aglQueryRendererInfo
 * Parameters:	gdev				- An array of Mac OS graphics devices (type GDHandle)
 *				ndev 				- The number of graphics devices in gdev
 * Returns:		the list of AGLRendererInfo data structures that describe the
 *				capabilities of OpenGL renderers, returns NULL if it fails for any reason.
 *
 * Description:	aglQueryRendererInfo returns a list of AGLRendererInfo data structures that 
 *				describe the capabilities of OpenGL renderers. One AGLRendererInfo is returned 
 *				for each OpenGL rendering engine installed on the system. To access the 
 *				AGLRendererInfo data, use aglDescribeRenderer.
 *				To free the data returned by this function, use aglDestroyRendererInfo.
 *				If gdev and ndev are NULL and zero, respectively, the returned information will 
 *				apply to all graphics devices on the system. Otherwise, information will be 
 *				returned for only the specified	devices.
 * 	Errors:		AGL_BAD_DEVICE		if ndev is nonzero and gdev is not an array of valid devices.
 *
 ***********************************************************************************************/
AGLRendererInfo aglQueryRendererInfo(const AGLDevice *gdev,
										GLint ndev)
{
	int 						gliRenderer;
	GLIRendererInfo				*rendererInfos;
	GLIRendererInfo				*origRenderInfos;
	GLboolean					succes;
	GLenum						error;
	int 						num,i;
	GLIRendererInfo				*current;
#pragma warning todo	
	/*
	** ToDo/XXX: Support the gdev,ndev parameter
	*/

	succes = GL_TRUE;
	
	error = gliQueryRendererInfo(&origRenderInfos,(GLIDevice*)gdev,ndev);
	if (error != GLI_NONE)
	{
		MAGL_SetError(NULL,(GLenum)error,("aglGetRendererInfos:gliQueryRendererInfo failed!"));
		return NULL;
	}
	num = 0;
	current = origRenderInfos;
	while (current)
	{
	    current = current->next_renderer_info;
		num++;
	}
	
	
	rendererInfos = MAGL_Alloc((sizeof(GLIRendererInfo)*num));
	if (rendererInfos == NULL)
	{
		MAGL_SetError(NULL,(GLenum)AGL_BAD_ALLOC,("aglGetRendererInfos:Not enough memory!"));
		return NULL;
	}
	
	current = origRenderInfos;
	for ( i =0; i < num; i++)
	{
		rendererInfos[i] = *current;
		rendererInfos[i].next_renderer_info = &rendererInfos[i+1];
		current = current->next_renderer_info;
	}
	gliDestroyRendererInfo(origRenderInfos);
	rendererInfos[num-1].next_renderer_info = NULL;
	
	return (AGLRendererInfo)rendererInfos;	
}

/***********************************************************************************************
 *
 * Function:	aglDestroyRendererInfo
 * Parameters:	gdev				- An array of Mac OS graphics devices (type GDHandle)
 *				ndev 				- The number of graphics devices in gdev
 * Returns:		the list of AGLRendererInfo data structures that describe the
 *				capabilities of OpenGL renderers, returns NULL if it fails for any reason.
 *
 * Description:	aglDestroyRendererInfo frees the memory allocated by aglQueryRendererInfo.
 *			Specific information is obtained from a renderer info with aglDescribeRendererInfo.
 *			Do not pass the return from aglNextRendererInfo to aglDestroyRendererInfo. Doing so
 *			will set the AGL_BAD_RENDINFO error.
 * 	Errors:	AGL_BAD_RENDINFO 			is set if rend is not a valid renderer info.
 *
 ***********************************************************************************************/

void aglDestroyRendererInfo ( AGLRendererInfo rend )
{
	if (rend == NULL)
	{
		MAGL_SetError(NULL,(GLenum)AGL_BAD_RENDINFO,("aglDestroyRendererInfo: Bad render info (null)!\n"));
		return;
	}
	
	MAGL_Free(rend);
}
AGLRendererInfo aglNextRendererInfo(AGLRendererInfo rend)
{
	if (rend == NULL)
	{
		MAGL_SetError(NULL,(GLenum)AGL_BAD_RENDINFO,("aglNextRendererInfo: Bad render info!\n"));
		return NULL;
	}
	
	return (AGLRendererInfo)((GLIRendererInfo*)rend)->next_renderer_info;
}
GLboolean aglDescribeRenderer(AGLRendererInfo inrend, GLint prop, GLint *value)
{
	GLIRendererInfo * rend = (GLIRendererInfo*)inrend;
	
	if (rend == NULL)
	{
		MAGL_SetError(NULL,(GLenum)AGL_BAD_RENDINFO,(""));
		return GL_FALSE;
	}
	
	switch (prop)
	{
		case AGL_RENDERER_ID:
			*value = rend->renderer_id;
			break;
		case AGL_OFFSCREEN:
			if (rend->os_support & GLI_OFFSCREEN_BIT)
				*value = GL_TRUE;
			else
				*value = GL_FALSE;
			break;
		case AGL_FULLSCREEN:
			if (rend->os_support & GLI_FULLSCREEN_BIT)
				*value = GL_TRUE;
			else
				*value = GL_FALSE;
			break;
		case AGL_WINDOW:
			if (rend->os_support & GLI_WINDOW_BIT)
				*value = GL_TRUE;
			else
				*value = GL_FALSE;
			break;
		case AGL_ACCELERATED:
			if (rend->os_support & GLI_ACCELERATED_BIT)
				*value = GL_TRUE;
			else
				*value = GL_FALSE;
			break;	
		case AGL_BACKING_STORE:
			if (rend->os_support & GLI_BACKING_STORE_BIT)
				*value = GL_TRUE;
			else
				*value = GL_FALSE;
			break;
		case AGL_ROBUST:
			if (rend->os_support & GLI_ROBUST_BIT)
				*value = GL_TRUE;
			else
				*value = GL_FALSE;
			break;
		case AGL_MP_SAFE:
			if (rend->os_support & GLI_MP_SAFE_BIT)
				*value = GL_TRUE;
			else
				*value = GL_FALSE;
			break;
		case AGL_COMPLIANT:
			if (rend->os_support & GLI_COMPLIANT_BIT)
				*value = GL_TRUE;
			else
				*value = GL_FALSE;
			break;
		case AGL_MULTISCREEN:
			if (rend->os_support & GLI_MULTISCREEN_BIT)
				*value = GL_TRUE;
			else
				*value = GL_FALSE;
			break;
		case AGL_BUFFER_MODES:
			*value = rend->buffer_modes;
			break;
		case AGL_MIN_LEVEL:
			*value = 0;
			break;
		case AGL_MAX_LEVEL:
			*value = 0;
			break;
		case AGL_COLOR_MODES:
			*value = rend->color_modes;
			break;
		case AGL_ACCUM_MODES:
			*value = rend->accum_modes;
			break;	
		case AGL_DEPTH_MODES:
			*value = rend->depth_modes;
			break;
		case AGL_STENCIL_MODES:
			*value = rend->stencil_modes;
			break;
		case AGL_MAX_AUX_BUFFERS:
			*value = rend->max_aux_buffers;
			break;	
		case AGL_VIDEO_MEMORY:
			*value = rend->video_memory_size;
			break;
		case AGL_TEXTURE_MEMORY:
			*value = rend->texture_memory_size;
			break;
		/*	
		case AGL_COMPLIANCE:
			*value = rend->data.compliance;
			break;
		case AGL_PERFORMANCE_MSB:
			*value = rend->data.msb_performance;
			break;
		case AGL_PERFORMANCE_LSB:
			*value = rend->data.lsb_performance;
			break;
		
		
		case AGL_TEXTURE_METHOD:
			*value = rend->data.texture_method;
			break;
		case  AGL_POINT_AA_METHOD:
			*value = rend->data.point_aa_method;
			break;
		case AGL_LINE_AA_METHOD:
			*value = rend->data.line_aa_method;
			break;
		case AGL_POLYGON_AA_METHOD:
			*value = rend->data.polygon_aa_method;
			break;
		case AGL_FOG_METHOD:
			*value = rend->data.fog_method;
			break;
		
		case AGL_INDEX_SIZES:
			*value = rend->data.index_sizes;
			break;
		
		*/
		default:
			MAGL_SetError(NULL,(GLenum)AGL_BAD_PROPERTY,("aglDescribeRenderer: bad property: %d",prop));
			return GL_FALSE;
			break;
	}
	
	return GL_TRUE;
}





