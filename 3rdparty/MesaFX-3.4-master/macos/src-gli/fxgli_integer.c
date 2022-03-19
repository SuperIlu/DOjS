/*
	File:		fxgli_integer.c

	Contains:	Implementation of gliSetInteger/gliGetInteger.

	Written by:	miklos

	Copyright:	Copyright ©

	Change History (most recent first):

         <3>      7/4/99    miklos  Revision.
*/

#include "aglMesaInt.h"
#include "fxgli.h"
#include "fxglidew.h"
#include "fxdrv.h"
void setDefaultIntegerValues(TFXContext *ctx)
{
	/* Get/Set Integers default value */
	ctx->gamma_value = fxgli_GetDefaultGammaValue();
    ctx->swap_interval = fxgli_GetDefaultSwapIntervall();
    ctx->max_pending_num = fxgli_GetDefaultMaxSwapPendingBuffers();
   	ctx->screen_takeover = GL_TRUE;
    /* Cursor emulation */
    ctx->emulate_cursor = fxgli_IsEmulateCursor();	
}

GLenum gliSetInteger(
				GLIContext		inCtx,
				GLenum			pname,
				const GLint		*value)
{
	TFXContext *c = (TFXContext*)inCtx; 
	
	if (!c)
		return (GLenum)GLI_BAD_CONTEXT;

	switch (pname)
	{

		case GLI_SWAP_RECT:
			/* Maybe, we could support it but why? */
			return (GLenum)GLI_NO_ERROR;
		case GLI_BUFFER_RECT:
			/* Maybe, we could support it but why? */
			return (GLenum)GLI_NO_ERROR;
		case GLI_SWAP_INTERVAL:
			c->swap_interval = *value;
			if (c->ctx != NULL)
				c->ctx->swapInterval = *value;
			return (GLenum)GLI_NO_ERROR;
		case GLI_MESA_3DFX_GAMMA_VALUE:
			c->gamma_value = (*value);
			if (c->ctx)
			{
			/*	grGammaCorrectionValue((*value)*1.0/(GLI_3DFX_GAMMA_SCALE)); */
			}
			return (GLenum)GLI_NO_ERROR;
		case AGL_MESA_3DFX_SET_CONFIGURATION_FILE:
			{
				AGLMesaConfigurationFile *file = (AGLMesaConfigurationFile *)value;
				
				MCFG_setMesaConfigurationFile(file->lines_num,file->lines);
				setDefaultIntegerValues(c);
			}
			return (GLenum)GLI_NO_ERROR;
		case GLI_ACTIVE_FULLSCREEN+1:
		case GLI_ACTIVE_FULLSCREEN:
			if ((*value != GL_TRUE) && (*value != GL_FALSE))
				return (GLenum)GLI_BAD_VALUE;
			c->screen_takeover = (*value);
			if (c->screen_takeover)
				FX_grSstControl(GR_CONTROL_ACTIVATE);
			else
				FX_grSstControl(GR_CONTROL_DEACTIVATE);
			return (GLenum)GLI_NO_ERROR;
		default:
			return (GLenum)GLI_BAD_ATTRIBUTE;
	}
}

GLenum gliGetInteger(
				GLIContext		inCtx,
				GLenum			pname,
				GLint			*value)
{
	TFXContext *c = (TFXContext*)inCtx;
   
	switch (pname)
	{
		case GLI_SWAP_INTERVAL:
			*value = c->swap_interval;
			return (GLenum)GLI_NO_ERROR;
		case GLI_MESA_3DFX_GAMMA_VALUE:
			*value = c->gamma_value;
			return (GLenum)GLI_NO_ERROR;
		case GLI_ACTIVE_FULLSCREEN:
			*value = c->screen_takeover;
			return (GLenum)GLI_NO_ERROR;
		default:
			return (GLenum)GLI_BAD_ATTRIBUTE;
	}
}