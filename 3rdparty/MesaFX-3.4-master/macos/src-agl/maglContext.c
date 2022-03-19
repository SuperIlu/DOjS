/*
	File:		maglContext.c

	Contains:	AGLContext related routines - AGL for GLI.

	Written by:	Mikl—s Fazekas

	Copyright:	Copyright(C) 1995-99 Miklos Fazekas
				E-Mail: boga@valerie.inf.elte.hu
				WWW: http://www.mesa3d.org/mac/
				
				Part of the Mesa 3D Graphics Library for MacOS.


	Change History (most recent first):
	
        <4+>      6/8/99    ???     Added __aglTerminate
         <4>     4/27/99    miklos  Fixed a bug about destroying the current context.
         <3>     4/14/99    miklos  Added aglSet,aglGetInteger and aglIsEnabled.
         <2>     4/10/99    miklos  Revision for Fullscreen and Offscreen support.
         <1>      4/8/99    miklos  Revision for AGL 2.0
         <7>     4/19/98    miklos  Adopted new error handler functions.
         <6>      4/7/98    miklos  Some typos fixed.
         <5>      4/7/98    miklos  Revision for Mesa 3.0b4
         <4>      4/6/98    miklos  Revision for Mesa 3.0b4
         <3>      3/1/98    miklos  Fixed compiler warnings.
         <2>    12/27/97    miklos  Inital Revision
         <1>    11/22/97    miklos  Initial Revision
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

/* MAGL */
#include "maglConfig.h"
#include "maglMemory.h"
#include "maglError.h"
#include "maglRenderers.h"
#include "maglValidate.h"

/* OpenGL/AGL */
#include "aglContext.h"
#include "aglMesa.h"
#include "gliMesa.h"

#include <agl.h>
#include "gliMesa.h"

/* Gli-utils */
#include "gliutils.h"

/* The current context! */
static AGLContext glc_cc = NULL;


typedef struct __AGLContextRec __AGLContextRec;
typedef struct __AGLPrivateRec {
	GLIPixelFormat 		gliPixelFormat;
	int					gliRenderer;
	GLenum				aglError;
	TGLIUtilsDrawable	gliDrawable;
} __AGLPrivateRec;


typedef enum TMAGL_ErrorMode {
	kMAGL_ErrorModeAttachDrawable	= 1
} TMAGL_ErrorMode;



/******************************************************************************************
 * 																						  *
 * Managing the current context 														  *
 *																						  *
 ******************************************************************************************/
static void
DetachCurrentContext(
			void)
{
	/* NOTHING YET (TODO) */
}

static void
SetCurrentContext(
			AGLContext 	inContext)
{
	glc_cc = inContext;
}

static AGLContext
GetCurrentContext(
						void)
{
	return glc_cc;
}

static int
PixelFormat2RendererID(
			const GLIPixelFormat 	*inPixelFmt)
{
	MAGL_Assert(inPixelFmt != NULL);
	
	return (inPixelFmt->renderer_id);
}

/******************************************************************************************
 * 																						  *
 * IsValidContext:													  				 	  *
 *  returns: GL_TRUE if inContext is valid GL_FALSE otherwise							  *
 *																						  *
 ******************************************************************************************/
GLboolean
IsValidContext(
			const AGLContext inContext)
{
	return (inContext != NULL);
}

/******************************************************************************************
 * 																						  *
 * AGL api implementation														  		  *
 *																						  *
 ******************************************************************************************/

GLboolean aglIsEnabled(AGLContext ctx, GLenum pname)
{
	return GL_TRUE;
}
GLboolean aglSetInteger(AGLContext ctx, GLenum pname, const GLint *value)
{
	GLenum error;
	if (!IsValidContext(ctx))
	{
		MAGL_SetError(NULL,AGL_BAD_CONTEXT,("aglSetInteger: The ctx %h is not valid!",ctx));
		return NULL;
	}
	error = gliSetInteger(ctx->rend,pname,value);
	if (error != GLI_NONE)
	{
		MAGL_SetError(NULL,error,("aglGetInteger: renderer error!"));
		return GL_FALSE;
	}
	return GL_TRUE;
}
GLboolean aglGetInteger(AGLContext ctx, GLenum pname, GLint *value)
{
	GLenum error;
	if (!IsValidContext(ctx))
	{
		MAGL_SetError(NULL,AGL_BAD_CONTEXT,("aglGetInteger: The ctx %h is not valid!",ctx));
		return NULL;
	}
	error = gliGetInteger(ctx->rend,pname,value);
	if (error != GLI_NONE)
	{
		MAGL_SetError(NULL,error,("aglGetInteger: renderer error!"));
		return GL_FALSE;
	}
	return GL_TRUE;} 
/******************************************************************************************
 * 																						  *
 * aglGetCurrentContext: returns the current AGL context, as specified by aglMakeCurrent. *
 *                       If there is no current context, NULL is returned.				  *
 *																						  *
 ******************************************************************************************/
AGLContext
aglGetCurrentContext(
				void)
{
	return GetCurrentContext();
}

/******************************************************************************************
 * 																						  
 * aglGetDrawable: aglGetDrawable returns the AGL drawable (a Mac OS CGrafPtr) that 
 *			was last attached to ctx with aglSetDrawable. If the drawable last attached 
 *			to ctx was an off-screen drawable (attached with aglSetOffScreen)
 *          aglGetDrawable returns the base address of the off-screen memory area.
 *          If the drawable last attached to ctx was a full-screen graphics device 
 *			(attached with aglSetFullScreen) aglGetDrawable returns the integer device 
 *			number of the full-screen graphics device. 
 *			aglGetDrawable returns NULL if no drawable is attached to ctx.		
 *	Errors:
 *		AGL_BAD_CONTEXT - is set if ctx is not a valid context.																				
 ******************************************************************************************/
AGLDrawable
aglGetDrawable(AGLContext ctx)
{
	MAGL_CHECK_STATE();
	
	/* ToDo: XXX: Support for Offscreen/Fullscreen buffers */
	if (!IsValidContext(ctx))
	{
		MAGL_SetError(NULL,AGL_BAD_CONTEXT,("aglGetDrawable: The ctx %h is not valid!",ctx));
		return NULL;
	}
	
	switch (ctx->priv->gliDrawable.type)
	{
		case GLI_NONE:
			return (AGLDrawable)NULL;
		case GLI_WINDOW:
			return (AGLDrawable)ctx->priv->gliDrawable.data.window.port;
		case GLI_OFFSCREEN:
			return (AGLDrawable)ctx->priv->gliDrawable.data.offscreen.baseaddr;
		case GLI_FULLSCREEN:
			return (AGLDrawable)ctx->priv->gliDrawable.data.fullscreen.device;
		default:
		{
			MAGL_SetError(NULL,AGL_BAD_CONTEXT,("aglGetDrawable: The ctx %h is not valid!",ctx));
			return NULL;
		}
	}
	
	return NULL;
}

#ifdef OLD_AGL
/******************************************************************************************
 * 																						  *
 * aglGetCurrentDrawable: Return the current drawable	(deprecated)					  *
 *																						  *
 ******************************************************************************************/
AGLDrawable
aglGetCurrentDrawable(
				void)
{
	MAGL_CHECK_STATE();
	
	if (GetCurrentContext() == NULL)
	{
		return NULL;
	}
	else
	{
		
		return aglGetDrawable(GetCurrentContext());
	}
}
#endif

static void InitDispatchTable(GLIFunctionDispatch inDispTable)
{
#pragma unused (inDispTable)	
	
}

static void	InitDispatchExtTable(GLIExtensionDispatch inExtDispTable)
{
#pragma unused (inExtDispTable)

}

static GLboolean IsInErrorMode(AGLContext inContext,TMAGL_ErrorMode errorMode)
{
#pragma unused(inContext)
#pragma unused(errorMode)
	return GL_FALSE;
}

static void EndErrorMode(AGLContext inContext,TMAGL_ErrorMode errorMode)
{
#pragma unused(inContext)
#pragma unused(errorMode)
	return;
}

static void BeginErrorMode(AGLContext inContext,TMAGL_ErrorMode errorMode)
{
#pragma unused(inContext)
#pragma unused(errorMode)
	return;
}

/***********************************************************************************************
 *
 * Function:	aglCreateContext
 * Parameters:	inPixelFormat		- Specifies the pixel format for the new rendering context.
 *				inShare				- Specifies the context with which to share display lists.  
 *									  NULL indicates that no sharing is to take place. 
 *									  (Not all renderers can share its context with other 
 *									   renderers. ToDo)
 * Returns:		the newly created AGL renderering context on succes, NULL otherwise.
 *
 * Description: aglCreateContext creates an AGL rendering context and returns its handle.  
 *				This context can be used to render into a Macintosh graphics port.  
 *				If pix  was chosen with the AGL_OFFSCREEN attribute, then the context 
 *				can be used to render into an off-screen  graphics port.
 *				If pix was choosen width the AGL_FULLSCREEN attribute, then the context
 *				can be used to renderer into a fullscreen device. (Such as the 3Dfx).
 * 	Errors:		AGL_BAD_MATCH		is generated if the context to be created could not share 
 *									display lists with the context specified by share.
 *				AGL_BAD_CONTEXT		is generated if share  is not a valid AGL context and is not 
 *									NULL.
 *				AGL_BAD_PIXELFMT	is generated if pix  is not a valid pixel format.
 *				AGL_GL_ERROR		is generated if an OpenGL renderer fails to create a context.
 *
 ***********************************************************************************************/
AGLContext
aglCreateContext(
				AGLPixelFormat inPixelFormat,
				AGLContext	   inShare)
{
	AGLContext result;
	GLenum error;
	
	if (!IsValidPixelFormat(inPixelFormat))
	{
		MAGL_SetError(NULL,(GLenum)AGL_BAD_PIXELFMT,("aglCreateContext: invalid pixel format!"));
		return NULL;
	}
	if ((inShare != NULL) && (!IsValidContext(inShare)))
	{
		MAGL_SetError(NULL,(GLenum)AGL_BAD_CONTEXT,("aglCreateContext: share is invalid!"));
		return NULL;
	}
	
	result = (AGLContext)MAGL_Alloc(sizeof(__AGLContextRec));
	
	if (result == NULL)
	{
		MAGL_SetError(NULL,(GLenum)AGL_BAD_ALLOC,("aglCreateContext: out of memory!"));
		return NULL;
	}
	
	result->priv = (AGLPrivate)MAGL_Alloc(sizeof(__AGLPrivateRec));
	
	if (result->priv == NULL)
	{
		MAGL_Free(result);
		MAGL_SetError(NULL,(GLenum)AGL_BAD_ALLOC,("aglCreateContext: out of memory!"));
		return NULL;
	}
	
	
/*	result->priv->gliRenderer 	= MAGL_RendererID2Renderer(((GLIPixelFormat*)inPixelFormat)->renderer_id); */
	result->rend = NULL;
	error = gliCreateContext(&result->rend,(GLIPixelFormat*)inPixelFormat,(inShare != NULL) ? inShare->rend : NULL);
	
	if (error != GLI_NONE || result->rend == NULL)
	{
		/* ToDo: What kind of error should we signal here? */
		/* MAGL_SetError(NULL,(GLenum)AGL_BAD_ALLOC,("aglCreateContext: the renderer failed to initialize the context!")); */
		MAGL_Free(result->priv);
		MAGL_Free(result);
		
		return NULL;
	}
	
	result->priv->gliPixelFormat = *((GLIPixelFormat*)inPixelFormat);
	
	MAGL_InitError(result);
	
	result->priv->gliDrawable.type = GLI_NONE;
	/*
	InitDispatchTable(result->disp);
	InitDispatchExtTable(result->exts);
	*/
	return result;
}

GLboolean 
aglDestroyContext(
				AGLContext inContext)
{
	GLboolean result;
	GLenum error;
	
	if (!IsValidContext(inContext))
	{
		MAGL_SetError(NULL,(GLenum)AGL_BAD_CONTEXT,("aglDestroyContext: inContext: %h is invalid!",inContext));
		return GL_FALSE;
	}
	
	if (inContext == GetCurrentContext())
		SetCurrentContext(NULL);
		
	error = gliDestroyContext(inContext->rend);
	
	if (error != GLI_NONE)
	{
		MAGL_SetError(NULL,error,("aglDestroyContext: the renderer can't destroy the ctx!")); 
		MAGL_Free(inContext);
		return GL_FALSE;
	}
	
	MAGL_Free(inContext);
	return GL_TRUE;

}

/***********************************************************************************************
 *
 * Function:	aglCreateContext
 * Parameters:	inContext		- Specifies the AGL context to update.
 * Returns:		GL_FALSE if it fails for any reason, GL_TRUE otherwise.
 *
 * Description: Notify context that the window geometry has changed. 
 *				aglUpdateContext must be called by the application any time the graphics port 
 *				geometry has changed. It should be called after any drag, grow, or zoom action 
 *				is performed on the window.
 *
 *				+aglUpdateContext can be called for context which have attached GrafPort!
 *				So calling aglUpdateContext for a fullscreen context will result in an
 *				AGL_BAD_CONTEXT error.
 * 	Errors:		AGL_BAD_CONTEXT		is generated if ctx  is not a valid AGL context or
 *									it hasn't got a valid GrafPort assigned to it.
 *				AGL_BAD_ALLOC 		is set if a renderer is unable resize a buffer.
 *
 ***********************************************************************************************/

GLboolean aglUpdateContext(AGLContext inContext)
{
	GLboolean wasInErrorMode;
	GLboolean succes;
	GLenum error;
	if (!IsValidContext(inContext))
	{
		MAGL_SetError(NULL,AGL_BAD_CONTEXT,("aglUpdateCurrent: ctx is invalid agl context!"));
		return GL_FALSE;
	}
	
	wasInErrorMode = IsInErrorMode(inContext,kMAGL_ErrorModeAttachDrawable);
	
	error = gliAttachDrawable(inContext->rend,inContext->priv->gliDrawable.type,_gliUtilsGetGLIDrawable(&inContext->priv->gliDrawable));
	
	if (error == GLI_NONE)
	{
		if (wasInErrorMode)
			EndErrorMode(inContext,kMAGL_ErrorModeAttachDrawable);
		
		return GL_TRUE;
	}
	else
	{
		if (!wasInErrorMode)
			BeginErrorMode(inContext,kMAGL_ErrorModeAttachDrawable);
		
		MAGL_SetError(inContext,error,("aglUpdateCurrent: Renderer is unable to reattach the drawable!"));
		return GL_FALSE;
	}	
}
#ifdef OLD_AGL
GLboolean
aglUpdateCurrent(void)
{
	AGLContext	currentContext;
	GLboolean	wasInErrorMode;
	GLboolean 	succes;
	
	/* ToDO XXX Offscreen/Fullscreen support */
	
	currentContext = aglGetCurrentContext();
	if (currentContext == NULL)
	{
		MAGL_SetError(NULL,(GLenum)AGL_BAD_CONTEXT,("aglUpdateCurrent: no current agl context!"));
		return GL_FALSE;
	}
	MAGL_Assert(IsValidContext(currentContext));
	
	wasInErrorMode = IsInErrorMode(currentContext,kMAGL_ErrorModeAttachDrawable);
	
	succes = MAGLCall_gliAttachDrawable(currentContext->priv->gliRenderer,currentContext->rend,currentContext->priv->aglDrawable);
	
	if (succes)
	{
		if (wasInErrorMode)
			EndErrorMode(currentContext,kMAGL_ErrorModeAttachDrawable);
		
		return GL_TRUE;
	}
	else
	{
		if (!wasInErrorMode)
			BeginErrorMode(currentContext,kMAGL_ErrorModeAttachDrawable);
		
		MAGL_SetError(currentContext,(GLenum)AGL_BAD_ALLOC,("aglUpdateCurrent: Renderer is unable to reattach the drawable!"));
		return GL_FALSE;
	}
}
#endif

#if 0
GLboolean
aglSetCurrent(AGLContext inContext)
{
	AGLContext oldCurrent;
	if (inContext == NULL)
	{
		/* Detach current  context */
		if (GetCurrentContext() != NULL)
			{
				DetachCurrentContext();
				SetCurrentContext(NULL);
			}
		return GL_TRUE;
	}
	
	if (!IsValidContext(inContext))
	{
		MAGL_SetError(NULL,(GLenum)AGL_BAD_CONTEXT,("aglMakeCurrent: Bad context: %h!",inContext));
		return GL_FALSE;
	}
	
	oldCurrent = GetCurrentContext();
	
	/* Detach the old current context */
	DetachCurrentContext();
	
	/* Set the new context to the current one */
	SetCurrentContext(inContext);
	
	return GL_TRUE;
}

GLboolean
aglSetDrawable(AGLContext inContext,
				AGLDrawable inDrawable)
{
	if (!IsValidDrawable(inDrawable))
	{
		MAGL_SetError(NULL,(GLenum)AGL_BAD_DRAWABLE,("aglMakeCurrent: Bad drawable: %h!",inDrawable));
		return GL_FALSE;
	}
	
	/* Attach drawable inDrawable to the inContext */
	{
		GLboolean 	succes;
		GLboolean	wasInError;
			
		inContext->priv->aglDrawable = inDrawable;
			
		wasInError = IsInErrorMode(inContext,kMAGL_ErrorModeAttachDrawable);
		succes = MAGLCall_gliAttachDrawable(inContext->priv->gliRenderer,inContext->rend,inContext->priv->aglDrawable);
		
		if (!succes)
		{
			BeginErrorMode(inContext,kMAGL_ErrorModeAttachDrawable);
			
			MAGL_SetError(inContext,(GLenum)AGL_GL_ERROR,("aglMakeCurrent:  gliAttachDrawable failed!"));
			return GL_FALSE;
		}
		else /* (succes) */
		{
			if (wasInError)
				EndErrorMode(inContext,kMAGL_ErrorModeAttachDrawable);
		}
	}
	return GL_TRUE;
}

GLboolean
aglMakeCurrent(
			AGLDrawable		inDrawable,
			AGLContext		inContext)
{
	AGLContext oldCurrent;

	if ((inDrawable == NULL) && (inContext == NULL))
	{
		/* Detach the current context */
		if (GetCurrentContext() != NULL)
			{
				DetachCurrentContext();
				SetCurrentContext(NULL);
			}
		return GL_TRUE;
	}
	
	if (!IsValidContext(inContext))
	{
		MAGL_SetError(NULL,(GLenum)AGL_BAD_CONTEXT,("aglMakeCurrent: Bad context: %h!",inContext));
		return GL_FALSE;
	}
	
	if (!IsValidDrawable(inDrawable))
	{
		MAGL_SetError(NULL,(GLenum)AGL_BAD_DRAWABLE,("aglMakeCurrent: Bad drawable: %h!",inDrawable));
		return GL_FALSE;
	}
	
	oldCurrent = GetCurrentContext();
	
	/* Attach drawable inDrawable to the inContext */
	{
		GLboolean 	succes;
		GLboolean	wasInError;
			
		inContext->priv->aglDrawable = inDrawable;
			
		wasInError = IsInErrorMode(inContext,kMAGL_ErrorModeAttachDrawable);
		succes = MAGLCall_gliAttachDrawable(inContext->priv->gliRenderer,inContext->rend,inContext->priv->aglDrawable);
		
		if (!succes)
		{
			BeginErrorMode(inContext,kMAGL_ErrorModeAttachDrawable);
			
			MAGL_SetError(inContext,(GLenum)AGL_GL_ERROR,("aglMakeCurrent:  gliAttachDrawable failed!"));
			return GL_FALSE;
		}
		else /* (succes) */
		{
			if (wasInError)
				EndErrorMode(inContext,kMAGL_ErrorModeAttachDrawable);
		}
	}
	
	/* Detach the old current context */
	DetachCurrentContext();
	
	/* Set the new context to the current one */
	SetCurrentContext(inContext);
	
	return GL_TRUE;
}
#endif
/***********************************************************************************************
 *
 * Function:	aglSetCurentContext
 * Parameters:	ctx					- Specifies an AGL rendering context.
 * Returns:		GL_TRUE if suceed, GL_FALSE otherwise.
 *
 * Description: aglSetCurrentContext makes ctx the current AGL rendering context, replacing 
 *				the previously current context if there was one. As a result of this action, 
 *				subsequent OpenGL rendering calls go to rendering context ctx to modify its 
 *				drawable. Because aglSetCurrentContext always replaces the current rendering 
 *				context with ctx , there can be only one current context.
 *				To release the current context without assigning a new one, call 
 *				aglSetCurrentContext with ctx set to NULL.
 *				If aglSetCurrentContext fails, the previously current rendering context 
 *				remain unchanged.
 * 	Errors:		AGL_BAD_CONTEXT		is generated if ctx is not a valid AGL context or ctx is 
 *									NULL.
 *
 ***********************************************************************************************/
GLboolean aglSetCurrentContext(AGLContext ctx)
{

	if ((ctx != NULL) && !IsValidContext(ctx))
	{
		MAGL_SetError(ctx,AGL_BAD_CONTEXT,("aglSetCurrentContext: the ctx %h is invalid!\n",ctx));
		return GL_FALSE;
	}
	
	/* Detach the old current context */
	DetachCurrentContext();
	/* Set the new context to the current one */
	SetCurrentContext(ctx);
	
	return GL_TRUE;
}

static GLboolean setDrawable(AGLContext ctx, const TGLIUtilsDrawable draw)
{
	TGLIUtilsDrawable	nulldrawable;
	nulldrawable.type = GLI_NONE;
		
	if (ctx == NULL || !IsValidContext(ctx))
	{
		MAGL_SetError(NULL,AGL_BAD_CONTEXT,("aglSetDrawable: the ctx %h is invalid!\n",ctx));
		return GL_FALSE;
	}
	if (!_gliUtilsIsValidDrawable(&draw))
	{
		MAGL_SetError(NULL,AGL_BAD_DRAWABLE,("aglSetDrawable th drawable %h is invalid!\n",draw));
		return GL_FALSE;
	}
	
	if (draw.type == GLI_NONE)
	{
		/* Detach the drawable */
		gliAttachDrawable(ctx->rend,GLI_NONE,NULL);
		BeginErrorMode(ctx,kMAGL_ErrorModeAttachDrawable);
		return GL_TRUE;
	}
	else
	{
		GLboolean wasInError;
		GLenum error;
		
		wasInError 	= IsInErrorMode(ctx,kMAGL_ErrorModeAttachDrawable);
		error 		= gliAttachDrawable(ctx->rend,draw.type,_gliUtilsGetGLIDrawable(&draw));
		
		if (error != GLI_NONE)
		{
			ctx->priv->gliDrawable.type = GLI_NONE;
			gliAttachDrawable(ctx->rend,GLI_NONE,NULL);
			BeginErrorMode(ctx,kMAGL_ErrorModeAttachDrawable);
			
			MAGL_SetError(ctx,(GLenum)error,("aglMakeCurrent:  gliAttachDrawable failed!")); 
			return GL_FALSE;
		}
		else /* (succes) */
		{
			if (wasInError)
				EndErrorMode(ctx,kMAGL_ErrorModeAttachDrawable);
		
			if ((draw.type != GLI_WINDOW) ||
				 (ctx->priv->gliDrawable.data.window.port != draw.data.window.port))
			{
				/* The drawable was chaged, call glViewport */
				/* ToDo, or should it be done by the gli engines?! */
				ctx->priv->gliDrawable = draw;
			}
			
			return GL_TRUE;
		}
		
	}
	return GL_TRUE;
}

GLboolean aglEnable(AGLContext ctx, GLenum pname)
{
	GLint enable  = 1;
	GLenum error;
	
	error = gliSetInteger(ctx->rend,pname+1,&enable);
	if (error != GLI_NONE)
	{
		MAGL_SetError(NULL,(GLenum)error,("aglEnable: gliSetInteger failed!"));
		return GL_FALSE;
	}
	return GL_TRUE;
}
GLboolean aglDisable(AGLContext ctx, GLenum pname)
{
	GLint disable  = 0;
	GLenum error;
	
	error = gliSetInteger(ctx->rend,pname+1,&disable);
	if (error != GLI_NONE)
	{
		MAGL_SetError(NULL,(GLenum)error,("aglEnable: gliSetInteger failed!"));
		return GL_FALSE;
	}
	return GL_TRUE;
}
/***********************************************************************************************
 *
 * Function:	aglSetDrawable
 * Parameters:	ctx					- Specifies an AGL rendering context.
 *				drawable			- Specifies an AGL drawable.
 *
 * Returns:		GL_TRUE if suceed, GL_FALSE otherwise.
 *
 * Description: aglSetDrawable attaches context ctx to draw, a Macintosh Grpahics Port.
 *				aglSetDrawable performs all actions performed by aglUpdateContext.
 *				When a context is first attached to a specific drawable, its viewport
 *				is set to the full size of the drawable. If the context is subsequently
 *				attached to the same drawable its viewport is unaltered. (And this function
 *				has the same effect as aglUpdateContext the only difference is that if
 *				it fails it sets the drawable to NULL)
 *				To disable a rendering context, call aglSetDrawable with draw set to NULL.
 *				If aglSetDrawable fails, the drawable of the context is set to NULL.
 *	Returns:	GL_FALSE 			if fails for any reason
 *				GL_TRUE				if suceeds.
 * 	Errors:		AGL_BAD_CONTEXT		if the context is not a valid agl context.
 *				AGL_BAD_DRAWABLE	if the drawable is not a valid agl drawable.
 *				AGL_GL_ERROR		if renderer cannot set the drawable. (Posibble resource
 *									limitations..)
 *
 ***********************************************************************************************/
GLboolean aglSetDrawable(AGLContext ctx, AGLDrawable draw)
{	
	TGLIUtilsDrawable	drawable;
	drawable.type = GLI_WINDOW;
	drawable.data.window.port = (CGrafPtr)draw;
	
		
	if (ctx == NULL || !IsValidContext(ctx))
	{
		MAGL_SetError(NULL,AGL_BAD_CONTEXT,("aglSetDrawable: the ctx %h is invalid!\n",ctx));
		return GL_FALSE;
	}
	if ((draw != NULL) && !_gliUtilsIsValidDrawable(&drawable))
	{
		MAGL_SetError(NULL,AGL_BAD_DRAWABLE,("aglSetDrawable th drawable %h is invalid!\n",draw));
	}
	
	if (draw == NULL)
	{
		/* Detach the drawable */
		gliAttachDrawable(ctx->rend,GLI_NONE,NULL);
		BeginErrorMode(ctx,kMAGL_ErrorModeAttachDrawable);
		
	}
	else
	{
		GLboolean wasInError;
		GLenum error;
		
		wasInError 	= IsInErrorMode(ctx,kMAGL_ErrorModeAttachDrawable);
		error 		= gliAttachDrawable(ctx->rend,drawable.type,_gliUtilsGetGLIDrawable(&drawable));
		
		if (error != GLI_NONE)
		{
			drawable.type = GLI_NONE;
			ctx->priv->gliDrawable = drawable;
			error = gliAttachDrawable(ctx->rend,GLI_NONE,NULL);
			BeginErrorMode(ctx,kMAGL_ErrorModeAttachDrawable);
			
			/* ToDo XXX: MAGL_SetError(ctx,(GLenum)AGL_GL_ERROR,("aglMakeCurrent:  gliAttachDrawable failed!")); */
			return GL_FALSE;
		}
		else /* (succes) */
		{
			if (wasInError)
				EndErrorMode(ctx,kMAGL_ErrorModeAttachDrawable);
		
			if (ctx->priv->gliDrawable.data.window.port != draw)
			{
				/* The drawable was chaged, call glViewport */
				/* ToDo!!!! */
				ctx->priv->gliDrawable = drawable;
			}
			return GL_TRUE;
		}
		
	}
}

/***********************************************************************************************
 *
 * aglSetFullScreen (agl-api)
 *
 * Parameters:	ctx				- Specifies an OpenGL context.
 *				width			- Specifies the width of the off-screen memory area in pixels.
 *				height			- Specified the height of the off-screen memory area in pixels.
 *				rowbytes 		- Specifies the number of bytes in one row of the off-screen 
 *								  memory area.
 *				baseaddr 		- Specifies the base address of the memory area.
 * Returns:		GL_FALSE if it fails for any reason, GL_TRUE otherwise.				
 * Description: aglSetOffScreen attaches context ctx to an off-screen memory area. As a
 *			    result of this action, subsequent OpenGL rendering calls directed to ctx 
 *				modify the off-screen memory. The context must have been created with respect 
 *				to a pixel format that supports off-screen rendering, which is requested with
 *				the AGL_OFFSCREEN attribute for aglChoosePixelFormat. 
 *				aglSetOffScreen also performs all of the actions performed by aglUpdateContext.
 *				When a context is attached to an off-screen memory area, its viewport is set to
 *			    the full size of the off-screen area.
 *				To disable a rendering context, call aglSetDrawable with draw set to NULL.
 *				If aglSetOffScreen fails, the drawable of the context is set to NULL.
 *
 *	Errors:		AGL_BAD_OFFSCREEN 	is set if the combination of width and rowbytes do not support the
 *								 	pixel size of the context.
 *				AGL_BAD_CONTEXT 	is set if ctx is not a valid AGL context.
 *			+   AGL_BAD_CONTEXT		is the context wasn't created with AGL_OFFSCREEN pixel format.
 ***********************************************************************************************/

GLboolean aglSetOffScreen(AGLContext ctx, GLsizei width, GLsizei height, GLsizei rowbytes, GLvoid *baseaddr)
{
	GLboolean 			succes;
	TGLIUtilsDrawable	drawable;
	Boolean				wasInError;
	GLenum				error;
	
	drawable.type = GLI_OFFSCREEN;
	drawable.data.offscreen.width = width;
	drawable.data.offscreen.height = height;
	drawable.data.offscreen.rowbytes = rowbytes;
	drawable.data.offscreen.baseaddr = baseaddr;
	
	wasInError 	= IsInErrorMode(ctx,kMAGL_ErrorModeAttachDrawable);
	error 		= gliAttachDrawable(ctx->rend,drawable.type,_gliUtilsGetGLIDrawable(&drawable));
	
	if (error != GLI_NONE)
	{
		/* Detach */
			drawable.type = GLI_NONE;
			ctx->priv->gliDrawable = drawable;
			gliAttachDrawable(ctx->rend,GLI_NONE,NULL);
			BeginErrorMode(ctx,kMAGL_ErrorModeAttachDrawable);
			
			/* ToDo XXX: MAGL_SetError(ctx,(GLenum)AGL_GL_ERROR,("aglMakeCurrent:  gliAttachDrawable failed!")); */
			return GL_FALSE;
	}
	else /* (succes) */
	{
			if (wasInError)
				EndErrorMode(ctx,kMAGL_ErrorModeAttachDrawable);
		
			/* The drawable was chaged, call glViewport */
			ctx->priv->gliDrawable = drawable;
			
			return GL_TRUE;		
	}
	
	return GL_FALSE;
}
/***********************************************************************************************
 *
 * aglSetFullScreen (agl-api)
 *
 * Parameters:	ctx				- Specifies an OpenGL context.
 *				width			- Specifies the width of the fullscreen
 * Returns:						
 * Description:	aglSetFullScreen attach a fulscreen device defined by width,height,freq
 *				,device to the OpenGL context ctx.
 *				The integer device number specifies which full-screen graphics device will be 
 *				used on a system with more than one full-screen device. device should be set to 
 *				zero on a system with a single device. There is no correlation between Macintosh 
 *				GDHandle's and full-screen device numbers, so an application must determine which 
 *				device to use by allowing the user to select it. If aglSetFullScreen fails, the 
 *				drawable of the context is set to NULL.
 *				When a context is first attached to a full-screen device, its viewport is set to 
 *				the full size of the device. If the context is subsequently attached to the same 
 *				device, its viewport is unaltered.
 *
 *	Errors:		AGL_BAD_FULLSCREEN is set if width, height, or freq are not supported by the device.
 *				AGL_BAD_CONTEXT is set if ctx is not a valid AGL context.
 ***********************************************************************************************/

GLboolean aglSetFullScreen ( AGLContext ctx,
							 GLsizei width,
							 GLsizei height,
							 GLsizei freq,
							 GLint device )
{
	GLboolean 			succes;
	TGLIUtilsDrawable	drawable;
	Boolean				wasInError;
	
	drawable.type = GLI_FULLSCREEN;
	drawable.data.fullscreen.width = width;
	drawable.data.fullscreen.height = height;
	drawable.data.fullscreen.freq = freq;
	drawable.data.fullscreen.device = device;
	
	wasInError 	= IsInErrorMode(ctx,kMAGL_ErrorModeAttachDrawable);
	succes 		= gliAttachDrawable(ctx->rend,drawable.type,_gliUtilsGetGLIDrawable(&drawable));
	
	if (! succes)
	{
		/* Detach */
			drawable.type = GLI_NONE;
			ctx->priv->gliDrawable = drawable;
			gliAttachDrawable(ctx->rend,drawable.type,_gliUtilsGetGLIDrawable(&drawable));
			BeginErrorMode(ctx,kMAGL_ErrorModeAttachDrawable);
			
			/* ToDo XXX: MAGL_SetError(ctx,(GLenum)AGL_GL_ERROR,("aglMakeCurrent:  gliAttachDrawable failed!")); */
			return GL_FALSE;
	}
	else /* (succes) */
	{
			if (wasInError)
				EndErrorMode(ctx,kMAGL_ErrorModeAttachDrawable);
		
			/* The drawable was chaged, call glViewport */
			ctx->priv->gliDrawable = drawable;
			
			return GL_TRUE;		
	}
	
	return GL_FALSE;

}
#ifdef OLD_AGL
GLboolean aglSwapHint(GLint x, GLint y, GLsizei width, GLsizei height)
{
	AGLContext cc;
	
	cc = GetCurrentContext();
	if (!IsValidContext(cc))
	{
		MAGL_SetError(NULL,(GLenum)AGL_BAD_CONTEXT,("aglSwapHint: the current context %h is invalid!\n",cc));
		return GL_FALSE;
	}
	
	return MAGLCall_gliSetSwapRect(cc->priv->gliRenderer,cc->rend,x,y,width,height);
}
#endif

void aglSwapBuffers(AGLContext ctx)
{
	AGLContext cc;
	GLboolean succes;
	GLenum 	error;
	cc = GetCurrentContext();
	if (!IsValidContext(cc))
	{
		MAGL_SetError(NULL,(GLenum)AGL_BAD_CONTEXT,("aglSwapBuffers: %h the current context is invalid!\n",cc));
		/* return GL_FALSE; */
	}
	
	error = gliSwapBuffers(cc->rend);	
	if (error != GLI_NONE)
	{
		MAGL_SetError(NULL,(GLenum)error,("aglSwapBuffers: gliSwapBuffers failed!\n"));
	}
}

/***********************************************************************************************
 *
 * Function:	aglGetVersion
 * Parameters:	outMajor		- Returns the Major version of the AGL library.
 *				outMinor		-  Returns the Minor version of the AGL library.
 *				inStateMask			- Specifies which portions of inSourceContext  
 *									  state are to be copied to inDestContext.
 * Returns:		the newly created AGL renderering context on succes, NULL otherwise.
 *
 * Description: aglGetVersion returns the major and minor version numbers of the AGL library. 
 *				AGL implementations with the same major version number are upward compatible, 
 *				meaning that the implementation with the higher minor number is a superset of 
 *				the version with the lower minor number. major and minor do not return values
 *				if they are specified as NULL.
 *	Returns:
 *				
 * 	Errors:		
 *
 ***********************************************************************************************/
void aglGetVersion(GLint *outMajor, GLint *outMinor)
{
	if (!outMajor || !outMinor)
		return;
		
	*outMajor = 2;
	*outMinor = 0;	
}

/***********************************************************************************************
 *
 * Function:	aglCopyContext
 * Parameters:	inSourceContext		- Specifies the source context.
 *				inDestContext		- Specifies the destination context.
 *				inStateMask			- Specifies which portions of inSourceContext  
 *									  state are to be copied to inDestContext.
 * Returns:		GL_FALSE if it fails for any reason, GL_TRUE otherwise.
 *
 * Description: aglCopyContext copies selected groups of state variables from src  to dst.    
 *				inStateMask indicates which groups of state variables are to be copied.  
 *				inStateMask contains the bitwise OR of the same symbolic names that are passed 
 *				to the OpenGL command glPushAttrib. The single symbolic constant 
 *				GL_ALL_ATTRIB_BITS can be used to copy the maximum possible portion of rendering state.
 *
 *				Not all values for OpenGL states can be copied. For example, pixel pack and unpack 
 *				state, render mode state, and select and feedback state are not copied. The state 
 *				that can be copied is exactly the state that is manipulated by OpenGL command 
 *				glPushAttrib.
 *				
 * 	Errors:		AGL_BAD_CONTEXT 	is set if either inSourceContext or inDestContext is not a 
 *									valid AGL context.
 *				OpenGL errors on either context may be generated if a renderer fails to get or set 
 *				the attributes. See glGetError.
 *
 ***********************************************************************************************/
GLboolean 
aglCopyContext(
			AGLContext 		inSourceContext, 
			AGLContext 		ioDestContext, 
			GLuint 			inStateMask)
{
	if (!IsValidContext(inSourceContext) || !IsValidContext(ioDestContext))
	{
		MAGL_SetError(NULL,(GLenum)AGL_BAD_CONTEXT,("Bad context in aglCopyContext!"));
		return GL_FALSE;
	}
	
	if ((inStateMask & ~GL_ALL_ATTRIB_BITS) != 0)
		return GL_FALSE;	/* TODO: We might need signal an error here! */
		
	if (inStateMask == 0)			
		return GL_TRUE;		/* Nothing should be copied. */
	#if 0
	if (!MAGLCall_mgliCopyContext(inSourceContext->priv->gliRenderer,inSourceContext->rend,ioDestContext->rend,inStateMask))
	{
		/* MAGL_SetError(NULL,(GLenum)AGL_GL_ERROR,("Can't copy width mask %x!",inStateMask)); */
		
		return GL_FALSE;
	}
	else
		return GL_TRUE;
	#endif	
	/* This would be the real agl-code, unused currently... */
	#ifdef REAL_AGL
	int i;
	for (i = 0; i < 20; i++) /* Currently only first 20-bits defined */
	{	
		GLIAttrib	attrib;
		
		if ((1 << i) & mask) 
		{
			if (!MAGLCall_gliGetAttribute(src->rend,(GLbitfield)1 << i,&attrib))
			{
				MAGL_SetError(NULL,AGL_GL_ERROR,("Can't read attrib, bit: %d.",i));
				return GL_FALSE;
			}	
			else {
				if (!MAGLCall_gliSetAttibute(dst->redn,1 << i,&attrib) 
				{
					MAGL_SetError(NULL,AGL_GL_ERROR,("Can't write attribute bit: %d.",i));
					return GL_FALSE;
				}	
			}
		}
	}
	
	return GL_TRUE;
	#endif /* REAL_AGL */
}

/* This is just for X-Plane */
extern pascal void __aglterminate(void);

pascal void __aglterminate(void)
{
	if (aglGetCurrentContext()  != nil)
	{
		aglDestroyContext(aglGetCurrentContext());
	}
}

