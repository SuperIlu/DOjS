/*
	File:		fxgli_context.c

	Contains:	Routines for creating/destroying the 3Dfx OpenGL context.

	Written by:	miklos

	Copyright:	Copyright ©

	Change History (most recent first):

         <2>      7/5/99    miklos  Revision
         <2>      7/5/99    miklos  Revision
         <2>      7/5/99    miklos  Revision
         <1>      6/8/99    ???     Initial revision.
*/


#include "fxgli.h"
/* ANSI */
#include "stdlib.h"

#if MESA_3DFX_PROFILE
	#include "profiler.h"
#endif


/***********************************************************************************************
 *
 * current_context global holds, the current context.
 *
 ***********************************************************************************************/

TFXContext *current_context;

/***********************************************************************************************
 *
 * gliCreateContext:
 *
 ***********************************************************************************************/
/***********************************************************************************************
 * Notes:
 *	The FX driver (see fxapi.h) doesn't have function like this.
 *  The current soultion to this problem is that we are creating a "fake" context here, and
 *  we create the real context for gliAttachDrawable.
 *  The limitaion of this code:
 *    - One can't share context's. (This isn't a real limitation, since 3DFX usually will handle
 *	  								only one context at a time.)
 *	The soultion:
 *	  - Split the fxApi so that it will have similiar functions.
 *
 ***********************************************************************************************/
GLenum gliCreateContext(GLIContext *ctx_ret, const GLIPixelFormat *fmt, GLIContext share_list)
{
	TFXContext *result;


	/*
	** Initial Error Checking
	*/
   	if (fmt->renderer_id != GLI_RENDERER_MESA_3DFX_ID)
   		return (GLenum)GLI_BAD_PIXELFMT;
   	if (ctx_ret == NULL)
   		return (GLenum)GLI_BAD_POINTER;
	if (share_list != NULL)
		return (GLenum)GLI_BAD_CONTEXT;
   	/*
   	** Main
   	*/
	result = (TFXContext*)malloc(sizeof(TFXContext));
	if (result == NULL)
	    return (GLenum)GLI_BAD_ALLOC;
	    
	result->renderer_id = GLI_RENDERER_MESA_3DFX_ID;
	result->drawable.type = GLI_NONE;
	result->ctx = NULL;
	result->gl_ctx = NULL;
	result->fmt = (*fmt);
	
	
	setDefaultIntegerValues(result);
    
   	*ctx_ret = (GLIContext)result;
		return (GLenum)GLI_NO_ERROR;
}

/***********************************************************************************************
 *
 * gliDestroyContext: Destroys the context created by gliCreateContext.
 *
 ***********************************************************************************************/
GLenum gliDestroyContext(GLIContext ctx)
{
	TFXContext *context = (TFXContext*)ctx;
	/*
	** Initial error checking
	*/

	if(context == NULL)
   		return (GLenum)GLI_BAD_CONTEXT;
   	
   
   	if (context->renderer_id != GLI_RENDERER_MESA_3DFX_ID) 
   		return (GLenum)GLI_BAD_CONTEXT;
   	/*
   	** Main
   	*/
   	if (context->ctx != NULL)
   		fxMesaDestroyContext(context->ctx);
   	
   	if (context == current_context)
   		current_context = NULL;
   		
   	free(context);   
   	
   	return (GLenum)GLI_NO_ERROR;  
}
