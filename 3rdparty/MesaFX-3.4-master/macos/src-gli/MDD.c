/*
	File:		MDD.c

	Contains:	Macintosh Device Driver Interface.

	Written by:	Miklos Fazekas

	Copyright:	Miklos Fazekas © 1999.  All rights reversed.
    See http://www.mesa3d.org/mac/ for more details.

	Change History (most recent first):

        <5+>     6/18/99    miklos  Updated for new GLI.
         <5>     4/28/99    miklos  Removed InstalGLI.
         <4>     4/27/99    miklos  Revison for Mesa 3.1b6.
         <3>     4/14/99    miklos  Added support  for TGLIDrawable.
         <2>      4/8/99    miklos  Corrected a bug in MDDEngineGetRendererInfo!
         <1>      4/3/99    miklos  Initial revision for Mesa3.1.b2.
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
#include <stdlib.h>
#include "MDD.h"

/* Mesa */
#include "context.h"

/* MacOS */
#include <MacMemory.h>
#include <CodeFragments.h>

/* ANSI */
#include <string.h>
#include <stdlib.h>


extern GLboolean	mgliInstallMDDEngine(TMDDEngine		*engine);
/******************************************************************************
 *
 * Constants:
 *
 *****************************************************************************/
#define kMDDMaxEngines 255

/******************************************************************************
 *
 * Global variables:
 *
 *****************************************************************************/
static TMDDEngine 	*gMDDEngines[kMDDMaxEngines];
static int		   	gMDDEngineNum = 0;


GLboolean
IsMDDEngine(
	const TMDDEngine *engine)
{
	int i;
	
	for (i = 0; i < gMDDEngineNum; i++)
	{
		if (gMDDEngines[i] == engine)
		{
			return GL_TRUE;
		}
	}

	return GL_FALSE;
}





TMDDEngine *MDDGetFirstEngine(void)
{
	if (gMDDEngineNum != 0)
	{
		return gMDDEngines[0];
	}
	else
	{
		return NULL;
	}
}

TMDDEngine * MDDGetNextEngine(
				const TMDDEngine		*engine)
{
	MDDAssert(IsMDDEngine(engine));
	
	if ((engine->engine_num+1) < gMDDEngineNum)
	{
		return gMDDEngines[engine->engine_num+1];
	}
	else
	{
		return NULL;
	}
}

void MDDEngineRendererInfo(
				const TMDDEngine		*engine,
				GLIRendererInfo			*renderer_info)
{
	MDDAssert(IsMDDEngine(engine));
	
	renderer_info->renderer_id =  engine->info.renderer_id;
	renderer_info->os_support =  engine->info.os_support;
	
	renderer_info->buffer_modes =  engine->info.buffer_modes;
	

	/* Color modes, 8A appended for all if mesa_alpha_flag is true */
	if (engine->info.mesa_alpha_flag)
	{	/* ToDo */
		renderer_info->color_modes =  engine->info.color_modes;
	}
	else
	{
		renderer_info->color_modes =  engine->info.color_modes;
	}
	/* Accumulation modes */
	renderer_info->accum_modes =  engine->info.accum_modes;
	/* Depth buffer modes, set to GL_DEPTH_SIZE if mesa_depth_flag is true */
	if (engine->info.mesa_depth_flag)
	{
		renderer_info->depth_modes = GLI_0_BIT | GLI_16_BIT;	/* ToDo */
	}
	else
	{
		renderer_info->depth_modes =  engine->info.depth_modes;
	}
	/* Stencil plane limitations */
	renderer_info->stencil_modes =  engine->info.stencil_modes;
	/* Index modes */ 
	renderer_info->index_sizes =  engine->info.index_sizes;
	/* Auxiliary buffers */
	renderer_info->max_aux_buffers =  engine->info.max_aux_buffers;
	
	renderer_info->video_memory_size = engine->info.video_memory_size;
	renderer_info->texture_memory_size = engine->info.texture_memory_size;	
}	

TMDDEngine			*MDDGetEngineFromRendererID(
								GLint	inRendererID)
{
	int i;
	
	for (i = 0; i < gMDDEngineNum; i++)
	{
		if (gMDDEngines[i]->info.renderer_id == inRendererID)
			return gMDDEngines[i];
	}
	
	return NULL;
}


TMDDError			MDDContextNew(
							const TMDDEngine		*engine,	/* MDD engine */
							GLIPixelFormat 			*pix,		/* Requested pixel format */
							struct gl_context		*gl_ctx)	/* GL-Context made by mesa, with the pix */
{
	MDDAssert(IsMDDEngine(engine));
	
	return (*engine->contextNew)(engine,pix,gl_ctx);	
}

void				MDDContextDelete(
							const TMDDEngine		*engine,
							struct gl_context		*gl_ctx)
{
	MDDAssert(IsMDDEngine(engine));
	
	(*engine->contextDelete)(engine,gl_ctx);
}

GLboolean			MDDSwapBuffers(
							const TMDDEngine 		*engine,
							struct gl_context		*gl_ctx)
{

	
	return (*engine->swapBuffers)(engine,gl_ctx);
}



TMDDError			MDDAttachDrawable(
							const TMDDEngine		*engine,	 /* MDD Engine */
							struct gl_context		*context,	 /* MDD Context */
							const TGLIDrawable		drawable)
{
	MDDAssert(IsMDDEngine(engine));
	
	return (*engine->attachDrawable)(engine,context,drawable);
}


GLboolean			MDDEnable(
								const TMDDEngine		*engine,
								struct gl_context		*gl_ctx,
								GLenum					pname)
{
	MDDAssert(IsMDDEngine(engine));
	
	if (!engine->enable)
		return GL_FALSE;
	else
		return (*engine->enable)(engine,gl_ctx,pname);
}
GLboolean			MDDIsEnabled(
								const TMDDEngine		*engine,
								struct gl_context		*gl_ctx,
								GLenum 					pname,
								GLboolean 				*result)
{
	MDDAssert(IsMDDEngine(engine));
	
	if (!engine->isEnabled)
		return GL_FALSE;
	else
		return (*engine->isEnabled)(engine,gl_ctx,pname,result);
}
								
GLboolean			MDDDisable(
								const TMDDEngine		*engine,
								struct gl_context		*gl_ctx,
								GLenum					pname)
{
	MDDAssert(IsMDDEngine(engine));
	
	if (!engine->disable)
		return GL_FALSE;
	else
		return (*engine->disable)(engine,gl_ctx,pname);
}

GLboolean			MDDSetInteger(
								const TMDDEngine		*engine,
								struct gl_context		*gl_ctx,
								GLenum					pname,
								const GLint 			*param)
{
	MDDAssert(IsMDDEngine(engine));
	
	if (!engine->setInteger)
		return GL_FALSE;
	else
		return (*engine->setInteger)(engine,gl_ctx,pname,param);
}								
					
GLboolean			MDDGetInteger(
								const TMDDEngine		*engine,
								struct gl_context		*gl_ctx,
								GLenum					pname,
								GLint 					*outParam)
{
	MDDAssert(IsMDDEngine(engine));
	
	if (!engine->getInteger)
		return GL_FALSE;
	else
		return (*engine->getInteger)(engine,gl_ctx,pname,outParam);
}
/****************************************************************************
*
* Function:     MDDSetGLError
* Parameters:   ctx			- The context, on which the error occured.
*				error_code	- One of GL_OUT_OF_MEMORY, etc. error codes.
*				debug_str	- Debug string, with format like printf.
*				...			- Parameters for debug_str like in printf.
*
* Description:  When an error occurs, the MDD-Driver should call this
*				function. 
*				For example: MDDSetGLError(gl_ctx,GL_OUT_OF_MEMORY,
*				"Not enough memory to download the texture, RAVE code %d !",rave_error);   
*
****************************************************************************/
void MDDSetGLError(	GLcontext		*ctx,			/* The context, on which the error occured */
					GLenum			error_code,		/* One of GL_OUT_OF_MEMORY, etc. error codes */
					char			*debug_str,		/* Debug string, with format like printf */
					...)  							/* Parameters for debug_str like in printf */
{
	/* TODO: Parse va_args, etc. */
	gl_error(ctx,error_code,debug_str);
}

TMDDError 			MDDRegisterEngine(
							char				*name,				/* Engine name */
							TMDDEngineInfo		*engineInfo,		/* Engine info */
							TMDDGetVisualInfo	getVisualInfo,		/* METHOD: (Optional) Get visual info */
							TMDDGetPixelFormat  getPixelFormat,		/* METHOD: Get pixel format */
							TMDDContextNew		contextNew,			/* METHOD: New context */
							TMDDContextDelete	contextDelete,		/* METHOD: Delete context */
							TMDDAttachDrawable	attachDrawable,		/* METHOD: Attach drawable */
							TMDDSwapBuffers		swapBuffers,		/* METHOD: Swap Buffers */
							
							TMDDSetInteger		setInteger,			/* METHOD: (Optional) Set integer */
							TMDDGetInteger		getInteger,			/* METHOD: (Optional) Get integer */
							TMDDEnable			enable,				/* METHOD: (Optional) enable */
							TMDDDisable			disable,			/* METHOD: (Optional) disable */
							TMDDIsEnabled		isEnabled,			/* METHOD: (Optional) isEnable */
							
							void				*privateData )		/* Engines Prvate data */
{
#pragma unused(name)
	TMDDEngine	*newEngine;
	
	newEngine = (TMDDEngine*)NewPtr(sizeof(TMDDEngine));
	
	if (newEngine == NULL)
		return kMDDOutOfMemory;
		
	newEngine->engine_num 		= gMDDEngineNum;
	newEngine->info 			= *engineInfo;
	newEngine->getPixelFormat   = getPixelFormat;
	newEngine->getVisualInfo 	= getVisualInfo;
	newEngine->contextNew 		= contextNew;
	newEngine->contextDelete 	= contextDelete;
	newEngine->attachDrawable 	= attachDrawable;
	newEngine->swapBuffers 		= swapBuffers;
	
	newEngine->setInteger		= setInteger;
	newEngine->getInteger		= getInteger;
	newEngine->enable			= enable;
	newEngine->disable			= disable;
	newEngine->isEnabled		= isEnabled;
	
	newEngine->privateData 		= privateData;
	
	gMDDEngines[gMDDEngineNum++] = newEngine;
	
	mgliInstallMDDEngine(newEngine);
	
	return kMDDNoError;
}				