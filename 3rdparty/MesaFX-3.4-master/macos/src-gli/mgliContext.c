/*
	File:		mgliContext.c

	Contains:	GLI interface for Mesa.

	Written by:	Miklos Fazekas

	Copyright:	Miklos Fazekas © 1999.  All rights reversed.
    See http://www.mesa3d.org/mac/ for more details.

	Change History (most recent first):

        <6+>      6/8/99    ???     Merged with the Mesa3.0f branch.
         <6>      5/3/99    miklos  Revsion for 3.1b10.
         <5>     4/28/99    miklos  Added InstallGLI here.
         <4>     4/27/99    miklos  Revison for Mesa 3.1b6.
         <3>     4/14/99    miklos  Fixed a bug with context, and different drawables... "wayne a.
                                    lee" <wal@blarg.net>.
         <2>     4/14/99    miklos  Revision for AGL 2.0
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

#include "gli.h"
#include "gliMesa.h"

#include "mgliError.h"
#include "mgliMem.h"
#include "mgliContext.h"
#include "mgliGetEnv.h"

#include "MDD.h"

/* MacOS */
#include <MacMemory.h>

/* Mesa */
#include "types.h"
#include "context.h"
#include "dd.h"
#include <CodeFragments.h>

/***********************************************************************************************
 *
 * Prototypes:
 *
 ***********************************************************************************************/
static GLboolean egliAttachDrawable(
						void 			*refContext,
						GLIContext		ctx,
						const TGLIDrawable		drawable);

/***********************************************************************************************
 *
 * LibAbailable: Search for a shared library with a name.
 * 
 ***********************************************************************************************/
static 
Boolean LibAvailable(Str255 libName)
/* Check to see if a particular shared library is available */
{
	CFragConnectionID	connID;
	Ptr				mainAddr;
	Str255			errName;
	OSErr			err;
	
	if (!true) {
		return false;	/* No Code Fragment Manager, hence no shared library */
	} else {
		err = GetSharedLibrary(libName, kAnyCFragArch, kFindCFrag, &connID, &mainAddr, errName);
		return (err == noErr);
	}
}

/* Engines installion prototypes */
extern OSErr InstallCEngine(void);
extern OSErr Install3DfxEngine(void);
/***********************************************************************************************
 *
 * IntsallGLIEngines: This will install GLI Engines to Mesa!
 *
 ***********************************************************************************************/
static GLboolean enginesInstalled = GL_FALSE;

extern void IntsallGLIEngines(void);
void IntsallGLIEngines(void)
{
	if (!enginesInstalled)
	{
		/* Initialize MCFG here: */
		MCFG_readMesaConfigurationFile();
		
		#if GLI_FX_RENDERER
			Install3DfxEngine(); 
		#endif
		#if defined(GLI_FX_INWINDOW_RENDERER) && (GLI_FX_INWINDOW_RENDERER)
			Install3DfxInWindowEngine();
		#endif
		#if GLI_SOFTWARE_RENDERER
			InstallCEngine();
		#endif
		#if defined(GLI_RAVE_RENDERER) && (GLI_RAVE_RENDERER)
			InstallRAVEEngines();
		#endif
		enginesInstalled = GL_TRUE;
	}
}
/***********************************************************************************************
 *
 * IsTheSameDrawable (internal)
 *
 * Description: Returns if two drawable are the same.
 *
 ***********************************************************************************************/
GLboolean AreTheSameDrawable(
				const TGLIDrawable	*drawable1,const TGLIDrawable *drawable2)
{
	if (drawable1->type != drawable2->type)
		return GL_FALSE;
	if ((drawable1->type == kGLIDrawableType_GrafPort) &&
		(drawable1->data.grafPort.port == drawable2->data.grafPort.port))
		return GL_TRUE;
	
	/* Otherwise they must be different... */
	return GL_FALSE;
}
/***********************************************************************************************
 *
 * GetDrawableDimensions (internal)
 *
 * Description: Returns the dimensions of a drawable.
 *
 ***********************************************************************************************/
GLboolean GetDrawableDimensions(
					const TGLIDrawable	*drawable,int *width,int *height)
{
	if (drawable->type == kGLIDrawableType_Null)
	{
		*width = 0;
		*height = 0;
		return GL_FALSE;
	}
	
	switch (drawable->type) {
		case kGLIDrawableType_Offscreen:
			*width = drawable->data.offscreen.width;
			*height = drawable->data.offscreen.height;
			return GL_TRUE;
		case kGLIDrawableType_Fullscreen:
			*width = drawable->data.fullscreen.width;
			*height = drawable->data.fullscreen.height;
			return GL_TRUE;
		case kGLIDrawableType_GrafPort:
			{
				CGrafPtr port = drawable->data.grafPort.port;
				
				*width = port->portRect.right-port->portRect.left;
				*height = port->portRect.bottom-port->portRect.top;
			}
			return GL_TRUE;
		default:
			*width = 0;
			*height = 0;
			return GL_FALSE;
	}
}





/****************************************************************************
*
* Function:     mgliValidMesaContext
* Parameters:   ctx - The context to check.
*
* Returns:		GL_TRUE if it's a valid Mesa context, GL_FALSE otherwise.
* Description:  Check for a validaty of a Mesa context. All context functions
*				should begin with mgliAssert(mgliIsValidMesaContext(ctx)).
*
****************************************************************************/
GLboolean		
mgliValidMesaContext(GLIContext ctx)
{
	MGLIContextRec	*driverMCtx;
	
	if (ctx == NULL)
		return GL_FALSE;
	if (((GLcontext*)ctx)->DriverMgrCtx == NULL)
		return GL_FALSE;
	
	driverMCtx = (MGLIContextRec*)((GLcontext*)ctx)->DriverMgrCtx;
	if (driverMCtx->glContext != (GLcontext*)ctx)
		return GL_FALSE;
	if (driverMCtx->mesaID != kMesaID)
		return GL_FALSE;
	
	return GL_TRUE;
}

/* Defines in parseGLIAttribList */
#include "GLIAttribParser.c"

static GLIPixelFormat 
*egliChoosePixelFormat(
				
				void  		*refContext,
				const GLint *attribs)
{
	GLIPixelFormat	minPixFmt;
	GLIPixelFormat 	*result;
	TMDDEngine 		*engine;
	TMDDPolicy		policy;
	GLint			rendererID = (GLint)refContext;

	if (!parseGLIAttribList(attribs,&minPixFmt,(GLbitfield*)&policy))
	{
		mgliFatalError(kMGLIBadAttribute,"Unknow Attribute!");
		return NULL;
	}
	
	/*
	 * Get a list of Pixel-Formats to return.
	 */
	result = NULL;
	for (engine = MDDGetFirstEngine();  engine != NULL; engine = MDDGetNextEngine(engine))
	{
		if (engine->info.renderer_id == rendererID)
		{
		
			TMDDError error;
			GLIPixelFormat *pix;
			int devices_num;
			GDHandle devices[GLI_MAX_DEVICES-1];
		
			/* Process device list */
			{
				GDHandle 	device;
				int		 	i;
			
				for (device = GetFirstGDevice(), i = 0; 
					(device != NULL) && (i < (GLI_MAX_DEVICES-1)); 
					device = GetNextGDevice(device), i++)
				{
					devices[i] = device;
				}
				devices_num = i;
			}
		
			/* Process for every combination of screens.... */
			{
				unsigned int i;
				for (i = 1; i < 1 << devices_num; i++)
				{
					int j;
				
					minPixFmt.num_devices = 0;
		
					for (j = 0; j < devices_num; j++)
					{
						if ((i & 1 << j) != 0)
						{
							minPixFmt.devices[minPixFmt.num_devices] = devices[j];
							minPixFmt.num_devices++;
						}
					}
					error = MDDGetPixelFormat(engine,&minPixFmt,policy,&pix);
					if (error != kMDDNoError)
					{
						mgliWarning(kMGLIMDDError,"gliChoosePixelFormat:MesaDD failed to result a pixel %x format!",error);
						pix = NULL;
					}
		
					if (pix != NULL)
					{
						if (result == NULL)
							result = pix;
						else
						{
				
							GLIPixelFormat *last;
				
							for (last = result;  
								last->next_pixel_format != NULL; 
								last = last->next_pixel_format)
								{}
				
							last->next_pixel_format = pix; 
						}
					}
				}
			}
		}
	}

	return result;
}
/*
** PixelFormat destroy
*/
static GLboolean	
egliDestroyPixelFormat(
						void*		   refContext,
						GLIPixelFormat *fmt)
{
#pragma unused(refContext)
	if (fmt == NULL)
	{
		mgliError(kMGLIBadPixelFormat,"gliDestroyPixelFormat, fmt is null!");
		return GL_FALSE;
	}
	else
	{
		GLIPixelFormat *current;
		
		current = fmt;
		while (current != NULL)
		{
			GLIPixelFormat *next;
			
			next = current->next_pixel_format;
			DisposePtr((Ptr)current);
			current = next;
		}
		
		return GL_TRUE;
	}
	
	return GL_FALSE;
}

/*
** Renderer info operations
*/

/*
 * Concatenates two renderer info-s into one: rend |= rend2;
 */
static
void ConcatenateRendererInfos( 	GLIRendererInfo *rend,
						  		GLIRendererInfo *rend2)
{
	/*
	** Renderer id
	*/
	rend->renderer_id = rend->renderer_id;
	/*
	** OS support feature bits
	*/
	rend->os_support 	|= rend2->os_support;
	/*
	** Buffer modes
	*/
	rend->buffer_modes 	|= rend2->buffer_modes;
	/*
	** Buffer levels
	*/
	if (rend2->min_level < rend->min_level)
		rend->min_level = rend2->min_level;
	if (rend2->max_level > rend->max_level)
		rend->max_level = rend2->max_level;
	/*
	** Buffer configurations
	*/
	rend->index_sizes 		|= rend2->index_sizes;
	rend->color_modes 		|= rend2->color_modes;
	rend->accum_modes 		|= rend2->accum_modes;
	rend->depth_modes 		|= rend2->depth_modes;
	rend->stencil_modes 	|= rend2->stencil_modes;
	if (rend2->max_aux_buffers > rend->max_aux_buffers)
		rend->max_aux_buffers = rend2->max_aux_buffers;
	/*
	** Compliant bits
	*/
	rend->compliance 		|= rend2->compliance;
	/*
	** Performance bits
	*/ 
	rend->msb_performance	|= rend2->msb_performance;
	rend->lsb_performance	|= rend2->lsb_performance;
	/*
	** Algorithms
	*/
	rend->texture_method	|= rend2->texture_method;
	rend->point_aa_method	|= rend2->point_aa_method;
	rend->line_aa_method	|= rend2->line_aa_method;
	rend->polygon_aa_method	|= rend2->polygon_aa_method;
	rend->fog_method		|= rend2->fog_method;
	/*
	**
	*/
	rend->texture_memory = rend2->texture_memory;
	rend->accelerated	 = rend2->accelerated;
	
}


static GLboolean 
egliGetRendererInfo(
					void*			inRendererID,
					GLIRendererInfo *rend)
{
	GLint		rendererID = (GLint)inRendererID;
	TMDDEngine 	*engine;
	/* Init part */
	rend->renderer_id 		= rendererID;
	rend->os_support 		= GLI_NONE;
	rend->buffer_modes 		= GLI_NONE;
	rend->min_level			= 0;
	rend->max_level			= 0;
	rend->index_sizes		= GLI_NONE;
	rend->color_modes		= GLI_NONE;
	rend->accum_modes		= GLI_NONE;
	rend->depth_modes 		= GLI_NONE;
	rend->stencil_modes		= GLI_NONE;
	rend->max_aux_buffers 	= 0;
	rend->compliance 		= GLI_NONE;
	rend->texture_method	= GLI_NONE;
	rend->point_aa_method	= GLI_NONE;
	rend->line_aa_method	= GLI_NONE;
	rend->polygon_aa_method	= GLI_NONE;
	rend->fog_method		= GLI_NONE;
	
	rend->compliant			= GL_FALSE;
	rend->accelerated 		= GL_FALSE;
	
	if (MDDGetFirstEngine() == NULL)
		return GL_FALSE;	/* Unable to init the renderer */
		
	for (	engine = MDDGetFirstEngine();
			engine != NULL;
			engine = MDDGetNextEngine(engine))
	{
		if (engine->info.renderer_id == rendererID)
		{
			GLIRendererInfo rend2;
			TMDDEngineInfo	*info;
		
			info = &engine->info;
		
			rend2.renderer_id 		= info->renderer_id;
			rend2.os_support 		= info->os_support;
			rend2.buffer_modes 		= info->buffer_modes;
			rend2.min_level			= info->min_level;
			rend2.max_level			= info->max_level;
			rend2.index_sizes		= info->index_sizes;
			rend2.color_modes		= info->color_modes;
			rend2.accum_modes		= info->accum_modes;
			rend2.depth_modes		= info->depth_sizes;
			rend2.stencil_modes		= info->stencil_sizes;
			rend2.max_aux_buffers 	= info->max_aux_buffers;
			rend2.compliance 		= info->compliance;
			rend2.texture_method	= info->texture_method;
			rend2.point_aa_method	= info->point_aa_method;
			rend2.line_aa_method	= info->line_aa_method;
			rend2.polygon_aa_method	= info->polygon_aa_method;
			rend2.fog_method		= info->fog_method;
			rend2.compliant			= info->compliant;
			rend2.accelerated 		= info->accelerated;
			rend2.texture_memory	= info->texture_memory;
			rend2.video_memory 		= info->video_memory;
			
			*rend = rend2;
			/*
			ConcatenateRendererInfos(rend,&rend2);
			*/
		}
	}
		
	return GL_TRUE;
}

/*
** Renderer operations
*/
static GLIContext 
egliCreateContext(
					void			*refContext,			
					GLIPixelFormat 	*fmt, 
					GLIContext 		share_list)
{	
	GLcontext 		*glContext 	= NULL;
	GLvisual		*visual	 	= NULL;
	TMDDEngine		*engine 	= NULL;
	TMDDError		status;
	MGLIContextRec 	*driverMCtx = NULL;
	GLint			rendererID	= (GLint)refContext;
	
	if (share_list != NULL)
		if (!mgliValidMesaContext(share_list))
		{
			mgliWarning(kMGLIInvalidShare,"gliCreateContext: share_list is not a Mesa context");
			return NULL;
		}
	
	engine = MDDGetEngineFromRendererID(rendererID);
	if (engine == NULL ||  (engine->info.renderer_id != rendererID))
	{
		mgliFatalError(kMGLIBadPixelFormat,"gliCreateContext: Invalid Pixel Format in fmt!");
		return NULL;
	}
	
	driverMCtx = mgliAlloc(sizeof(MGLIContextRec));
	if (driverMCtx == NULL)
	{
		mgliError(kMGLIOutOfMemory,"gliCreateContext: mgliAlloc failed!");
		return NULL;
	}
	driverMCtx->glContext = NULL;
	driverMCtx->visual = NULL;
	driverMCtx->framebuffer = NULL;
	driverMCtx->drawable.type = kGLIDrawableType_Null;
	driverMCtx->engine = NULL;
	driverMCtx->mesaID = kMesaID;
	
	visual = MDDGetMesaVisual(engine,fmt);
	if (visual == NULL)
	{
		engine = NULL;
		mgliFree(driverMCtx);
		mgliError(kMGLIMDDError,"gliCreateContext: MDDVisualNew failed!");
		return NULL;
	}
	
	glContext = gl_create_context(visual,(GLcontext*)share_list,NULL,true);
	if (glContext == NULL)
	{
		engine = NULL;
		mgliFree(driverMCtx);
		MDDDeleteMesaVisual(visual);
		mgliError(kMGLIGLError,"gliCreateContext: gl_create_context failed!");
		return NULL;
	}
	glContext->DriverMgrCtx = driverMCtx;
	
	status = MDDContextNew(engine,fmt,glContext);
	if (status != kMDDNoError)
	{
		engine = NULL;
		mgliFree(driverMCtx);
		MDDDeleteMesaVisual(visual);
		mgliError(kMGLIGLError,"gliCreateContext: gl_create_context failed!");
		gl_destroy_context(glContext);
		return NULL;
	}
	
	driverMCtx->engine = engine;
	driverMCtx->glContext = glContext;
	driverMCtx->visual = visual;
	driverMCtx->framebuffer = NULL;
	
	return (GLIContext)glContext;
}


static GLboolean 
egliDestroyContext(
					void		*refContext,
					GLIContext 	ctx)
{
	MGLIContextRec	*driverMCtx;
	GLint			rendererID = (GLint)refContext;
	
	mgliAssert(mgliValidMesaContext(ctx));
	
	driverMCtx = (MGLIContextRec*)(((GLcontext*)ctx)->DriverMgrCtx);

	mgliAssert(driverMCtx->engine != NULL);
	mgliAssert(driverMCtx->engine->info.renderer_id == rendererID);

	
	/*
	 * Before deleting a context we should detach it:
	 */
	if (driverMCtx->drawable.type != kGLIDrawableType_Null)
	{
		TGLIDrawable nulldrawable;
		nulldrawable.type = kGLIDrawableType_Null;
		egliAttachDrawable(refContext,ctx,nulldrawable);
	}
	
	MDDContextDelete(driverMCtx->engine,driverMCtx->glContext);

	if (driverMCtx->framebuffer != NULL)
	{
		gl_destroy_framebuffer(driverMCtx->framebuffer);
	}
	
	if (driverMCtx->visual != NULL)
	{
		gl_destroy_visual(driverMCtx->visual);
	}
	
	if (driverMCtx->glContext != NULL)
	{
		gl_destroy_context(driverMCtx->glContext);
	}
	
	mgliFree(driverMCtx);
	
	
	
	return GL_TRUE;
}

extern void gl_Viewport( GLcontext *ctx,
                  GLint x, GLint y, GLsizei width, GLsizei height );

static
const char * Stuble_RendererString(void)
{
	return "error!";
}                
static
void Stuble_Func(void)
{
	/* Just do nothing */
}
static
GLboolean Stuble_False(void)
{
	return GL_FALSE;
}
static void Stuble_BufferSize( GLcontext *ctx, GLuint *width, GLuint *height )
{
#pragma unused(ctx)
	/* TODO ??? */
	*width = 1;
	*height = 1;
}
                
static                  
void mgliSetStubleDD(GLIContext ctx)
{
	/* TODO */
	struct dd_function_table *DD;
	MGLIContextRec	*driverMCtx;

	driverMCtx = (MGLIContextRec*)((GLcontext*)ctx)->DriverMgrCtx;
	DD = &((GLcontext*)ctx)->Driver;
	DD->RendererString = (void*)Stuble_RendererString;
	DD->UpdateState = (void*)mgliSetStubleDD;
	DD->ClearIndex = (void*)Stuble_Func;
	DD->ClearColor = (void*)Stuble_Func;
	DD->Clear = (void*)Stuble_Func;
	DD->Index = (void*)Stuble_Func;
	DD->Color = (void*)Stuble_Func;
	DD->SetBuffer = (void*)Stuble_False;
	DD->GetBufferSize = (void*)Stuble_BufferSize;
	DD->WriteRGBASpan = (void*)Stuble_Func;
	DD->WriteRGBSpan = (void*)Stuble_Func;
	DD->WriteMonoRGBASpan = (void*)Stuble_Func;
	DD->WriteRGBAPixels = (void*)Stuble_Func;
	DD->WriteMonoRGBAPixels = (void*)Stuble_Func;
	DD->WriteCI32Span = (void*)Stuble_Func;
	DD->WriteCI8Span = (void*)Stuble_Func;
	DD->WriteMonoCISpan = (void*)Stuble_Func;
	DD->WriteCI32Pixels = (void*)Stuble_Func;
	DD->WriteMonoCIPixels = (void*)Stuble_Func;
	DD->ReadCI32Span = (void*)Stuble_Func;
	DD->ReadRGBASpan = (void*)Stuble_Func;
	DD->ReadCI32Pixels = (void*)Stuble_Func;
	DD->ReadRGBAPixels = (void*)Stuble_Func;
	
	/* Optional functions: */
	DD->ExtensionString = (void*)Stuble_False;
	DD->Finish = (void*)Stuble_Func;
	DD->Flush = (void*)Stuble_Func;
	DD->IndexMask = (void*)Stuble_Func;
	DD->ColorMask = (void*)Stuble_Func;
	DD->LogicOp = (void*)Stuble_Func;
	DD->Dither = (void*)Stuble_Func;
	DD->Error = (void*)Stuble_Func;
	DD->NearFar = (void*)Stuble_Func;
	DD->GetParameteri = (void*)Stuble_Func;
	DD->AllocDepthBuffer = (void*)Stuble_Func;
	DD->DepthTestSpan = (void*)Stuble_Func;
	DD->DepthTestPixels = (void*)Stuble_Func;
	DD->ReadDepthSpanFloat = (void*)Stuble_Func;
	DD->ReadDepthSpanInt = (void*)Stuble_Func;
	
	DD->PointsFunc = (void*)Stuble_Func;
	DD->LineFunc = (void*)Stuble_Func;
	DD->TriangleFunc = (void*)Stuble_Func;
	DD->QuadFunc = (void*)Stuble_Func;
	DD->RectFunc = (void*)Stuble_Func;
	
	DD->DrawPixels = (void*)Stuble_Func;
	DD->Bitmap = (void*)Stuble_Func;
	#if GLI_MESA_3_0
	DD->Begin = (void*)Stuble_Func;
	DD->End = (void*)Stuble_Func;
#else
	DD->RenderStart = (void*)Stuble_Func;
	DD->RenderFinish = (void*)Stuble_Func;
#endif
	DD->RasterSetup = (void*)Stuble_Func;
#if GLI_MESA_3_0
	DD->RenderVB = (void*)Stuble_Func;
#else
	/* ??? */
	DD->RenderVBIndirect = (void*)Stuble_Func;
	
	/* CVA */
	DD->MergeAndRenderCVA = (void*)Stuble_Func;
	DD->RenderElementsCVA = (void*)Stuble_Func;
	DD->InvalidateRasterSetupCVA = (void*)Stuble_Func;
#endif	
	/* ??? */
	DD->TexEnv = (void*)Stuble_Func;
	DD->TexImage = (void*)Stuble_Func;
	DD->TexSubImage = (void*)Stuble_Func;
	DD->TexParameter = (void*)Stuble_Func;
	DD->BindTexture = (void*)Stuble_Func;
	DD->DeleteTexture = (void*)Stuble_Func;
	DD->UpdateTexturePalette = (void*)Stuble_Func;
	DD->UseGlobalTexturePalette = (void*)Stuble_Func;
}

static
void mgliClearDD(GLIContext ctx)
{
	struct dd_function_table *DD;
	MGLIContextRec	*driverMCtx;

	driverMCtx = (MGLIContextRec*)((GLcontext*)ctx)->DriverMgrCtx;
	DD = &((GLcontext*)ctx)->Driver;
	DD->RendererString = NULL;
	DD->UpdateState = NULL;
	DD->ClearIndex = NULL;
	DD->ClearColor = NULL;
	DD->Clear = NULL;
	DD->Index = NULL;
	DD->Color = NULL;
	DD->SetBuffer = NULL;
	DD->GetBufferSize = NULL;
	DD->WriteRGBASpan = NULL;
	DD->WriteRGBSpan = NULL;
	DD->WriteMonoRGBASpan = NULL;
	DD->WriteRGBAPixels = NULL;
	DD->WriteMonoRGBAPixels = NULL;
	DD->WriteCI32Span = NULL;
	DD->WriteCI8Span = NULL;
	DD->WriteMonoCISpan = NULL;
	DD->WriteCI32Pixels = NULL;
	DD->WriteMonoCIPixels = NULL;
	DD->ReadCI32Span = NULL;
	DD->ReadRGBASpan = NULL;
	DD->ReadCI32Pixels = NULL;
	DD->ReadRGBAPixels = NULL;
	
	/* Optional functions: */
	DD->ExtensionString = NULL;
	DD->Finish = NULL;
	DD->Flush = NULL;
	DD->IndexMask = NULL;
	DD->ColorMask = NULL;
	DD->LogicOp = NULL;
	DD->Dither = NULL;
	DD->Error = NULL;
	DD->NearFar = NULL;
	DD->GetParameteri = NULL;
	DD->AllocDepthBuffer = NULL;
	DD->DepthTestSpan = NULL;
	DD->DepthTestPixels = NULL;
	DD->ReadDepthSpanFloat = NULL;
	DD->ReadDepthSpanInt = NULL;
	
	DD->PointsFunc = NULL;
	DD->LineFunc = NULL;
	DD->TriangleFunc = NULL;
	DD->QuadFunc = NULL;
	DD->RectFunc = NULL;
	
	DD->DrawPixels = NULL;
	DD->Bitmap = NULL;
#if GLI_MESA_3_0
	DD->Begin = NULL;
	DD->End = NULL;
	DD->RenderVB = NULL;
	DD->RasterSetup = NULL;
#else
	
	DD->RenderStart = NULL;
	DD->RenderFinish = NULL;
	
	DD->RasterSetup = NULL;
	
	/* NEW in Mesa 3.x */
	DD->RegisterVB = NULL;
	DD->UnregisterVB = NULL;
	DD->CheckPartialRasterSetupCVA = NULL;
	DD->CheckMergeAndRenderCVA = NULL;
	DD->PartialRasterSetupCVA = NULL;
	DD->InvalidateRasterSetupCVA = NULL;
	DD->MergeAndRenderCVA = NULL;
	DD->RenderVBIndirect = NULL;
	DD->RenderElementsCVA = NULL;
#endif	
	
	DD->TexEnv = NULL;
	DD->TexImage = NULL;
	DD->TexSubImage = NULL;
	DD->TexParameter = NULL;
	DD->BindTexture = NULL;
	DD->DeleteTexture = NULL;
	DD->UpdateTexturePalette = NULL;
	DD->UseGlobalTexturePalette = NULL;
}
                  
static                  
void mgliDetachDrawable(GLIContext ctx)
{
	MGLIContextRec	*driverMCtx;

	mgliAssert(mgliValidMesaContext(ctx));
	
	driverMCtx = (MGLIContextRec*)((GLcontext*)ctx)->DriverMgrCtx;
	
	if (driverMCtx->drawable.type != kGLIDrawableType_Null) /* Already detached */
	{
		TMDDError succes;
		TGLIDrawable nulldrawable;
		
		nulldrawable.type = kGLIDrawableType_Null;
		
		succes = MDDAttachDrawable(driverMCtx->engine,driverMCtx->glContext,nulldrawable);
		
		mgliAssert(succes == kMDDNoError);
		mgliAssert(driverMCtx->framebuffer != NULL);
		
		gl_destroy_framebuffer(driverMCtx->framebuffer);
		driverMCtx->framebuffer = NULL;
		driverMCtx->drawable.type = kGLIDrawableType_Null;
		
		mgliSetStubleDD(ctx);	/* Protect the context */
	}
}

extern void mgliPutInErrorState(GLcontext *ctx);
void mgliPutInErrorState(GLcontext *ctx)
{
	mgliDetachDrawable((GLIContext)ctx);
}
/***********************************************************************************************
 *
 * egliSetSwapRect (external)
 *
 * Description: Copyes the src context attributes (defined by mask) to dst.
 *
 ***********************************************************************************************/

static GLboolean egliCopyContext(
					void			*refContext,
					GLIContext 		src, 
					GLIContext 		dst, 
					GLbitfield 		mask)
{
#pragma unused(refContext)
	gl_copy_context((GLcontext*)src,(GLcontext*)dst,(GLuint)mask);

	return GL_TRUE;
}
/*
** Drawable operations
*/
static GLboolean egliAttachDrawable(
			void				*refContext,
			GLIContext 			ctx, 
			const TGLIDrawable 	drawable)
{
	MGLIContextRec	*driverMCtx;
	TMDDError		succes;
	GLint			rendererID = (GLint)refContext;
	
	mgliAssert(mgliValidMesaContext(ctx));
	
	driverMCtx = (MGLIContextRec*)((GLcontext*)ctx)->DriverMgrCtx;
	
	if (drawable.type == kGLIDrawableType_Null)
	{
		/* 
		 * Detach, it can't fail.
		 */
		mgliDetachDrawable(ctx);
		return GL_TRUE;
	}
	else
	{
		/*
		 * Attach.
		 */
		 
		if (driverMCtx->framebuffer == NULL)
		{
			mgliAssert(driverMCtx->drawable.type == kGLIDrawableType_Null);
			
			driverMCtx->framebuffer = gl_create_framebuffer(driverMCtx->visual);
			if (driverMCtx->framebuffer == NULL)
				return GL_FALSE;
		}
		
		if ((driverMCtx->drawable.type != kGLIDrawableType_Null) && !AreTheSameDrawable(&(driverMCtx->drawable),&drawable))
		{
			TGLIDrawable nulldrawable;
			nulldrawable.type = kGLIDrawableType_Null;
			/* First detach the old context */
			MDDAttachDrawable(driverMCtx->engine,driverMCtx->glContext,nulldrawable);
			driverMCtx->drawable = nulldrawable;
		}
		
		/* Clear the DD functions if the drawable is null */
		if (driverMCtx->drawable.type == kGLIDrawableType_Null)
		{
			mgliClearDD(ctx);
		}
		
		succes = MDDAttachDrawable(driverMCtx->engine,driverMCtx->glContext,drawable);
		driverMCtx->drawable = drawable;
		
		if (succes != kMDDNoError)
		{
			/* Invaildate the buffer */
			mgliDetachDrawable(ctx);
		
			return GL_FALSE;
		}
		else
		{
			gl_make_current(driverMCtx->glContext,driverMCtx->framebuffer); /* Can't fail */
		}
	
		driverMCtx->drawable = drawable;
	
		/* At every make current set the viewport to the MAX size of the driver */
		{
			int width,height;
		
			GetDrawableDimensions(&drawable,&width,&height);
			{
				/* This is a hack code... */ 
				GLenum error;
	
				error = ((GLcontext*)ctx)->ErrorValue;			
				((GLcontext*)ctx)->ErrorValue = GL_NONE;
				gl_Viewport(driverMCtx->glContext,0,0,width,height);
				if (((GLcontext*)ctx)->ErrorValue == GL_OUT_OF_MEMORY)	/* There is'nt enough memory 
															   to allocate the buffers */
				{
					mgliDetachDrawable(ctx);
					return GL_FALSE;
				}
				else	/* It succeed */
				{
					if (error != GL_NONE)
						((GLcontext*)ctx)->ErrorValue = error;
						
					
					/*MDDSetSwapRect(driverMCtx->engine,driverMCtx->glContext,0,0,width,height); */
					
					return GL_TRUE;
				}
			}
		}
	}
	
	return GL_FALSE;
}

/***********************************************************************************************
 *
 * egliSetSwapRect (external)
 *
 * Description: Defines the minimal swap-rectangle
 *
 ***********************************************************************************************/

static GLboolean egliSetSwapRect(
						void 			*refContext,
						GLIContext 		ctx, 
						GLint 			x, 
						GLint 			y, 
						GLsizei 		width, 
						GLsizei 		height)
{
#pragma unused(refContext)
#pragma unused(x)
#pragma unused(y)
#pragma unused(width)
#pragma unused(height)

	MGLIContextRec	*driverMCtx;
	
	mgliAssert(mgliValidMesaContext(ctx));
	
	driverMCtx = (MGLIContextRec*)((GLcontext*)ctx)->DriverMgrCtx;
	if (driverMCtx->drawable.type == kGLIDrawableType_Null)	/* Invalid state can't do it */
		return GL_FALSE;
		
/*	return MDDSetSwapRect(driverMCtx->engine,driverMCtx->glContext,x,y,width,height);
 */
  return GL_FALSE;
}
/***********************************************************************************************
 *
 * egliSetSwapRect (external)
 *
 * Description: Swaps the buffers.
 *
 ***********************************************************************************************/
static GLboolean egliSwapBuffers(
						void 			*refContext,
						GLIContext 		ctx)
{
#pragma unused(refContext)
	MGLIContextRec	*driverMCtx;
	
	mgliAssert(mgliValidMesaContext(ctx));
	
	driverMCtx = (MGLIContextRec*)((GLcontext*)ctx)->DriverMgrCtx;
	
	if (driverMCtx->drawable.type == kGLIDrawableType_Null)
		return GL_FALSE;	/* Invalid state can't do it */
		
	return MDDSwapBuffers(driverMCtx->engine,driverMCtx->glContext);
}

/***********************************************************************************************
 *
 * egliQueryVersion (external)
 *
 * Description: Returns the version of the gli-extension.
 *
 ***********************************************************************************************/
static GLboolean egliQueryVersion(
					void		*refContext,
					GLint 		*major, 
					GLint 		*minor, 
					GLint 		*rendID)
{
	GLint		rendererID = (GLint)refContext;
	
	*major 		= 1;
	*minor 		= 2;
	*rendID 	= rendererID;
	
	return GL_TRUE;
}

static GLboolean	egliDisable(void 		*refCtx,
						GLIContext 	ctx,
						GLenum		pname)
{
#pragma unused(refCtx)
	MGLIContextRec	*driverMCtx;
	
	mgliAssert(mgliValidMesaContext(ctx));
	
	driverMCtx = (MGLIContextRec*)((GLcontext*)ctx)->DriverMgrCtx;

	return MDDDisable(driverMCtx->engine,driverMCtx->glContext,pname);
}

static GLboolean	egliEnable(void 		*refCtx,
						GLIContext 	ctx,
						GLenum		pname)
{
#pragma unused(refCtx)
	MGLIContextRec	*driverMCtx;
	
	mgliAssert(mgliValidMesaContext(ctx));
	
	driverMCtx = (MGLIContextRec*)((GLcontext*)ctx)->DriverMgrCtx;

	return MDDEnable(driverMCtx->engine,driverMCtx->glContext,pname);
}

static GLboolean	egliSetInteger(
						void			*refCtx,
						GLIContext		ctx,
						GLenum			pname,
						const GLint			*value)
{
#pragma unused(refCtx)
	MGLIContextRec	*driverMCtx;
	
	mgliAssert(mgliValidMesaContext(ctx));
	
	driverMCtx = (MGLIContextRec*)((GLcontext*)ctx)->DriverMgrCtx;

	return MDDSetInteger(driverMCtx->engine,driverMCtx->glContext,pname,value);
}
static GLboolean	egliGetInteger(
						void			*refCtx,
						GLIContext		ctx,
						GLenum			pname,
						GLint			*value)
{
#pragma unused(refCtx)
	MGLIContextRec	*driverMCtx;
	
	mgliAssert(mgliValidMesaContext(ctx));
	
	driverMCtx = (MGLIContextRec*)((GLcontext*)ctx)->DriverMgrCtx;

	return MDDGetInteger(driverMCtx->engine,driverMCtx->glContext,pname,value);
}

/***********************************************************************************************
 *
 * mgliInstallMDDEngine
 *
 * Description: Installs an MDD engine as an egli-renderer.
 *
 ***********************************************************************************************/
GLboolean	mgliInstallMDDEngine(
					TMDDEngine		*engine)
{
	return RegisterGLIRenderer(
					engine->info.renderer_id,
					(void*)engine->info.renderer_id,
					egliChoosePixelFormat,
					egliDestroyPixelFormat,
					egliGetRendererInfo,
					egliCreateContext,
					egliDestroyContext,
					egliAttachDrawable,
					egliSetSwapRect,
					egliSwapBuffers,
					egliCopyContext,
					egliQueryVersion,
					egliEnable,
					egliDisable,
					egliGetInteger,
					egliSetInteger);
}		

static void MesaBColor2GLIColor(GLcontext *ctx,GLIColor4 *gliColor,GLubyte byteColor[4])
{
	gliColor->r = byteColor[0]/ctx->Pixel.RedScale;
	gliColor->g = byteColor[1]/ctx->Pixel.GreenScale;
	gliColor->b = byteColor[2]/ctx->Pixel.BlueScale;
	gliColor->a = byteColor[3]/ctx->Pixel.AlphaScale;
}
static void MesaFColor2GLIColor(GLcontext *ctx,GLIColor4 *gliColor,GLfloat floatColor[4])
{
	#pragma unused (ctx)
	gliColor->r = floatColor[0];
	gliColor->g = floatColor[1];
	gliColor->b = floatColor[2];
	gliColor->a = floatColor[3];
}
static void MesaCoord3GLICoord3(GLcontext *ctx,GLICoord3 *g_coord,GLfloat m_coord[3])
{
	#pragma unused (ctx)
	g_coord->x = m_coord[0];
	g_coord->y = m_coord[1];
	g_coord->z = m_coord[2];
}
static void MesaCoord4GLICoord4(GLcontext *ctx,GLICoord4 *g_coord,GLfloat m_coord[4])
{
	#pragma unused (ctx)
	g_coord->x = m_coord[0];
	g_coord->y = m_coord[1];
	g_coord->z = m_coord[2];
	g_coord->w = m_coord[3];
}
static void MesaIMapTable2GLIMapTable(GLcontext *ctx,GLint dst[GLI_MAX_PIXEL_MAP_SIZE],
													 GLint src[GLI_MAX_PIXEL_MAP_SIZE])
{
	#pragma unused (ctx)
	int i;
	for (i = 0; i < MAX_PIXEL_MAP_TABLE; i++)
	{
		dst[i] = src[i];
	}
}

#define CB4(dst,src)	MesaBColor2GLIColor(ctx,&gli->dst,mesa->src)
#define CF4(dst,src)	MesaFColor2GLIColor(ctx,&gli->dst,mesa->src)
#define CV3(dst,src)	MesaCoord3GLICoord3(ctx,&gli->dst,mesa->src)
#define CV4(dst,src)	MesaCoord4GLICoord4(ctx,&gli->dst,mesa->src)
#define CMTI(dst,src)	MesaIMapTable2GLIMapTable(ctx,gli->dst,mesa->src);
#define CMTF(dst,src)	MesaIMapTable2GLIMapTable(ctx,gli->dst,mesa->src);
#define C(dst,src)		gli->dst = mesa->src;

GLboolean gliGetAttribute(GLIContext gli_ctx, GLenum type, GLIAttrib *attrib)
{
	#pragma unused(gli_ctx)
	#pragma unused(type)
	#pragma unused(attrib)
	return GL_FALSE;
	#ifdef THIS_IS_NOT_FINISHED
	GLcontext	*ctx = (GLcontext*)gli_ctx;
	mgliAssert(mgliValidMesaContext(gli_ctx));
	
	

	switch (type)
	{
		case GL_CURRENT_BIT:
			{
				GLIAttribCurrent		 *gli = &attrib->current;
				struct gl_current_attrib *mesa = &ctx->Current;
				
				CB4(rgba_color,				ByteColor);
				C(index_color,				Index);
				CV3(normal,					Normal);
				CV4(texture,				TexCoord);
				C(	edge_flag,				EdgeFlag);
				C(	raster_pos_valid,		RasterPosValid);
				CV4(raster_pos_coord,		RasterPos);
				CV3(raster_pos_normal,		Normal);	/* TODO: RasterPosNormal is missing */
				CV4(raster_pos_texture,		RasterTexCoord);
				CF4(raster_pos_rgba_color,	RasterColor);
				C(	raster_pos_index_color,	RasterIndex);
				
				return GL_TRUE;
			}
		case GL_POINT_BIT:
			{
				GLIAttribPoint		 		*gli = &attrib->point;
				struct gl_point_attrib 		*mesa = &ctx->Point;
				
				C  (smooth_enable,			SmoothFlag);
				C  (size,					Size);
				
				return GL_TRUE;
			}
		case GL_LINE_BIT:
			{
				GLIAttribLine		 		*gli = &attrib->line;
				struct gl_line_attrib 		*mesa = &ctx->Line;
				
				C  (smooth_enable,			SmoothFlag);
				C  (stipple_enable,			StippleFlag);
				C  (stipple_pattern, 		StipplePattern);
				C  (stipple_factor,			StippleFactor);
				C  (width,					Width);
				
				return GL_TRUE;
			}
		case GL_POLYGON_BIT:
			{
				GLIAttribPolygon		 	*gli = &attrib->polygon;
				struct gl_polygon_attrib 	*mesa = &ctx->Polygon;
				
				C  (cull_face_enable,		CullFlag);
				C  (cull_face_mode,			CullFaceMode);
				C  (front_face, 			FrontFace);
				C  (front_mode,				FrontMode);
				C  (back_mode,				BackMode);
				C  (smooth_enable,			SmoothFlag);
				C  (stipple_enable,			StippleFlag);
				C  (offset_fill_enable,		OffsetFill);
				C  (offset_line_enable,		OffsetLine);
				C  (offset_point_enable,	OffsetPoint);
				C  (offset_factor,			OffsetFactor);
				C  (offset_units,			OffsetUnits);
				
				return GL_TRUE;
			}
		case GL_POLYGON_STIPPLE_BIT:
			{
				int i;
				GLIAttribPolygonStipple		*gli = &attrib->polygon_stipple;
				
				for (i =0; i < 32; i++)
				{
					gli->poly_stipple[i] = ctx->PolygonStipple[i];
				}
				
				return GL_TRUE;
			}
		case GL_PIXEL_MODE_BIT:
			{
				{
					GLIAttribPixelMode		 	*gli = &attrib->pixel_mode;
					struct gl_pixel_attrib 		*mesa = &ctx->Pixel;
				
					C ( zoom_x,			ZoomX);
					C ( zoom_y,			ZoomY);
				}
				{
					GLIPixelTransfer		 	*gli = &attrib->pixel_mode.transfer;
					struct gl_pixel_attrib 		*mesa = &ctx->Pixel;
					
					C(red_bias,		RedBias);
					C(red_scale,		RedScale);
					C(green_bias,		GreenBias);
					C(green_scale,		GreenScale);
					C(blue_bias,		BlueBias);
					C(blue_scale,		BlueScale);
					C(alpha_bias,		AlphaBias);
					C(alpha_scale,		AlphaScale);
					C(depth_bias,		DepthBias);
					C(depth_scale,		DepthScale);
					C(index_offset,		IndexOffset);
					C(index_shift,		IndexShift);
					
				}
				{
					GLIPixelMap		 	*gli = &attrib->pixel_mode.map;
					struct gl_pixel_attrib 		*mesa = &ctx->Pixel;
					/*
					CMTI(i_to_i_table,		MapItoI);
					CMTI(i_to_r_table,		MapItoR);
					CMA(i_to_g_table,		MapItoG);
					CMA(i_to_b_table,		MapItoB);
					CMA(i_to_a_table,		MapItoA);
					CMA(r_to_r_table,		MapRtoR);
					CMA(g_to_g_table,		MapGtoG);
					CMA(b_to_b_table,		MapBtoB);
					CMA(a_to_a_table,		MapAtoA);
					CMA(s_to_s_table,		MapStoS);
					CMA(i_to_i_size,		MapItoIsize);
					CMA(s_to_s_size,		MapStoSsize);
					
					C(i_to_r_size,		MapItoRsize);
					C(i_to_g_size,		MapItoGsize);
					C(i_to_b_size,		MapItoBsize);
					C(i_to_a_size,		MapItoAsize);
					C(r_to_r_size,		MapRtoRsize);
					C(g_to_g_size,		MapGtoGsize);
					C(b_to_b_size,		MapBtoBsize);
					C(a_to_a_size,		MapAtoAsize);
					
					C(map_color,		MapColorFlag);
					C(map_stencil,		MapStencilFlag);
					
				}
				return GL_TRUE;
			}
		case GL_LIGHTING_BIT:
			{
				GLIAttribPixelMode		 	*gli = &attrib->polygon;
				struct gl_pixel_attrib 		*mesa = &ctx->Polygon;
			}
		case GL_DEPTH_BUFFER_BIT:
			{
				GLIAttribPixelMode		 	*gli = &attrib->polygon;
				struct gl_pixel_attrib 		*mesa = &ctx->Polygon;
			}
		case GL_ACCUM_BUFFER_BIT:
			{
				GLIAttribPixelMode		 	*gli = &attrib->polygon;
				struct gl_pixel_attrib 		*mesa = &ctx->Polygon;
			}		
		case GL_STENCIL_BUFFER_BIT:
			{
				GLIAttribPixelMode		 	*gli = &attrib->polygon;
				struct gl_pixel_attrib 		*mesa = &ctx->Polygon;
			}
		case GL_VIEWPORT_BIT:
			{
				GLIAttribPixelMode		 	*gli = &attrib->polygon;
				struct gl_pixel_attrib 		*mesa = &ctx->Polygon;
			}
		case GL_TRANSFORM_BIT:
			{
				GLIAttribPixelMode		 	*gli = &attrib->polygon;
				struct gl_pixel_attrib 		*mesa = &ctx->Polygon;
			}
		case GL_ENABLE_BIT:
			{
				GLIAttribPixelMode		 	*gli = &attrib->polygon;
				struct gl_pixel_attrib 		*mesa = &ctx->Polygon;
			}
		case GL_COLOR_BUFFER_BIT:
			{
				GLIAttribPixelMode		 	*gli = &attrib->polygon;
				struct gl_pixel_attrib 		*mesa = &ctx->Polygon;
			}
		case GL_HINT_BIT:
			{
				GLIAttribPixelMode		 	*gli = &attrib->polygon;
				struct gl_pixel_attrib 		*mesa = &ctx->Polygon;
			}
		case GL_EVAL_BIT:
			{
				GLIAttribPixelMode		 	*gli = &attrib->polygon;
				struct gl_pixel_attrib 		*mesa = &ctx->Polygon;
			}
		case GL_LIST_BIT:
			{
				GLIAttribPixelMode		 	*gli = &attrib->polygon;
				struct gl_pixel_attrib 		*mesa = &ctx->Polygon;
			}
		case GL_TEXTURE_BIT:
			{
				GLIAttribPixelMode		 	*gli = &attrib->polygon;
				struct gl_pixel_attrib 		*mesa = &ctx->Polygon;
			}
		case GL_SCISSOR_BIT:
			{
				GLIAttribPixelMode		 	*gli = &attrib->polygon;
				struct gl_pixel_attrib 		*mesa = &ctx->Polygon;
			}
	}*/
	#endif
}
GLboolean gliSetAttribute(GLIContext ctx, GLenum type, const GLIAttrib *attrib)
{	
	#pragma unused(ctx)
	#pragma unused (type)
	#pragma unused (attrib)
	return GL_FALSE;
}

