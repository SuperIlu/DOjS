/*
	File:		MDD.h

	Contains:	Mesa device driver interface for Mac OS.

	Written by:	Fazekas Mikl—s

	Copyright:	Copyright(C) 1995-98 Miklos Fazekas
				E-Mail: boga@valerie.inf.elte.hu
				WWW: http://valerie.inf.elte.hu/~boga/Mesa.html
				
				Part of the Mesa 3D Graphics Library for MacOS.

	Change History (most recent first):


       <12+>      7/2/98    miklos  Changed mtypes to types.
        <12>     6/18/98    miklos  Revision for Mesa 3.0.
        <11>     1/30/98    miklos  Changed the global rendering  id to Conix's conforming one.
        <10>     1/29/98    miklos  Removed unused functions
         <9>     1/23/98    miklos  Added MDDFatalContextError
         <8>     1/23/98    miklos  Added MDDSetGLError
         <3>     1/21/98    miklos  Deleted device informations from TMDDEngineInfo
         <2>     1/21/98    miklos  Added MDDSetSwapRect
         <1>     1/21/98    miklos  Initial Revision
*/

/*
** GLI driver for Mesa 3D Graphics Library.
** Version: 1.1
** Mesa version: 3.0
**
** Copyright(C) 1995-98 Miklos Fazekas
** E-Mail: boga@valerie.inf.elte.hu
** WWW: http://valerie.inf.elte.hu/~boga/Mesa.html
** 
** Part of the Mesa 3D Graphics Library for MacOS.
**
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



#ifndef __MDD_H__
#define __MDD_H__

/* MacOS Types.h header */
#include <MacTypes.h>


/* Mesa types.h header */
#include "types.h"

#include "gli.h"
#include "gliMesa.h"

/******************************************************************************
 *
 * Contants.
 *
 *****************************************************************************/
#define 		kMDDGlobalRenderingID						1100		 
 

 
/******************************************************************************
 *
 * Basic data types.
 *
 *****************************************************************************/
 
 
typedef struct	__TMDDEngine				TMDDEngine;			/* Drawing context for an engine */
typedef struct 	TMDDContext					TMDDContext;		/* Pointer to a drawing engine */

typedef enum	TMDDError
{
	kMDDNoError					= 0,							/* No error */
	kMDDError					= 1,							/* Generic error flag */
	kMDDOutOfMemory				= 2
} TMDDError;


/****************************************************************************
 * TMDDEngineInfo:															*
 * Purpose:																	*
 *   	Holds information about the rendering engine. 						*
 *		The GLI driver generates (by default) the requested pixel formats, 	*
 *		and mesa visual from those data. 									*
 *		If the GLbitfield contains more than one format, for example 		*
 *		index_sizes contains more GLI_8_BIT and GLI_4_BIT, the one that		*
 *		best meets the attrib list choosen.									*
 *																			*
 *		RGBA-sizes for pixel format	choosen with the following algo:		*
 *			- it will be choosed to match the screen's bit depth if no 		*
 *			  policy. (kMDDDefaultPolicy).									*
 *			- if no such exists it will choose one that best meets the 		*
 *			  screen bit depths. (First it tries deeper, than less deeper).	*
 *																			*
 *	Notes:																	*
 *		Because this structure is device independent. It can be used only,	*
 *		if all draw-context holds the same charaterisitcs.					*	
 ****************************************************************************/

typedef struct TMDDEngineInfo {
	GLIRendererInfo	rend;
	
	/* These fields used for automatic pixel-format generation */
	GLboolean		mesa_alpha_flag;	/* Allocate software alpha buffers by Mesa */
	GLboolean		mesa_depth_flag;	/* Allocate software depth buffers by Mesa,
									   	   HW-acced drivers typically set this to GL_FALSE */
} TMDDEngineInfo;


typedef struct TMDDVisualInfo {
	GLboolean rgb_flag;				/* rgb_flag 	- true if RGB(A) mode, false if color indexed */
    GLboolean alpha_flag;			/* alpha_flag	- alloc software alpha buffers */
    GLboolean db_flag;				/* db_flag 		- double buffering */
    GLboolean stereo_flag;			/* stereo_flag	- stereo buffer */
	GLint depth_bits;				/* depth_bits 	- requested minimum bits per depth buffer value */
	GLint stencil_bits;				/* stencil_bits - requested minimum bits per stencil buffer value */
	GLint accum_bits;				/* accum_bits 	- requested minimum bits per accum buffer component */
	GLint index_bits;				/* index_bits 	- number of bits per pixel, ignored if rgb_flag */
	GLint red_bits;					/* red_bits 	- number of bits per color component in frame buffer */
	GLint green_bits;				/*				  for RGB(A) mode, these only minimal requirements.	*/			
	GLint blue_bits;				/* 				  ignored if !rgba_flag */
	GLint alpha_bits;	
} TMDDVisualInfo;

/************************************************************************************************
 *
 * Utility functions:
 *
 ***********************************************************************************************/
extern 		GLboolean	IsMDDEngine(const TMDDEngine *engine);	/* Returns GL_TRUE if the engine is a valid 
																   drawing engine, GL_FALSE othervise. */

#define 	MDDAssert(condition)	if (!(condition)) 	Debugger();



/************************************************************************************************
 *
 * Prototypes to pixel format managment.
 *
 ***********************************************************************************************/
struct gl_visual	*MDDGetMesaVisual(
							const TMDDEngine 		*engine,			/* MDD engine */
							GLIPixelFormat 			*pix);				/* Requested pixel format */
void				MDDDeleteMesaVisual(struct gl_visual*	visual);
TMDDError			MDDGetPixelFormat(
							const TMDDEngine		*engine,			/* MDD engine */
							GLIPixelFormat			*requestedFormat,	/* Requested pixel-format, it contains device info also! */
							GLIPixelFormat			**outFormats);		/* One or more result pixel format */

 												
/************************************************************************************************
 *
 * Acceleration manager function prototypes.
 *
 ***********************************************************************************************/
typedef TMDDError	(*TMDDContextNew) (
							const TMDDEngine		*engine,
							GLIPixelFormat 			*pix,				/* Requested pixel format */
							struct gl_context		*gl_ctx);			/* GL-Context made by mesa, with the pix */
							
typedef void		(*TMDDContextDelete) (
							const TMDDEngine		*engine,
							struct gl_context		*gl_ctx);			/* Should'nt destory the whole only the DriverCtx */
							
typedef TMDDError	(*TMDDAttachDrawable) (
							const TMDDEngine		*engine,
							struct gl_context 		*context,			/* Mesa's context sctructure */
							const TGLIDrawable		drawable);			/* GL-drawable, to attach the context to */
							
typedef TMDDError	(*TMDDGetPixelFormat) (
							const TMDDEngine				*engine,
							GLint							attrib[],
							GLint							ndev,
							GLIDevices						devices[],
							GLIPixelFormat					**outFormats);		/* One or more result pixel format, allocated with NewPointer*/	

typedef GLboolean  ( *TMDDGetVisualInfo) (
								const TMDDEngine 		*engine,	/* MDD engine */
								GLIPixelFormat 			*pix,		/* Pixel format */
								TMDDVisualInfo			*outVisualInfo);			

typedef GLboolean	(*TMDDSwapBuffers) (
								const TMDDEngine		*engine,
								struct gl_context 		*context);

typedef GLboolean	(*TMDDEnable)(
								const TMDDEngine		*engine,
								struct gl_context		*gl_ctx,
								GLenum					pname);
typedef GLboolean	(*TMDDIsEnabled)(
								const TMDDEngine		*engine,
								struct gl_context		*gl_ctx,
								GLenum 					pname,
								GLboolean 				*result);
								
typedef GLboolean	(*TMDDDisable)(
								const TMDDEngine		*engine,
								struct gl_context		*gl_ctx,
								GLenum					pname);

typedef GLboolean	(*TMDDSetInteger)(
								const TMDDEngine		*engine,
								struct gl_context		*gl_ctx,
								GLenum					pname,
								const GLint 			*param);
						
typedef GLboolean	(*TMDDGetInteger)(
								const TMDDEngine		*engine,
								struct gl_context		*gl_ctx,
								GLenum					pname,
								GLint 					*outParam);


typedef struct __TMDDEngine {
		void 				*privateData;		/* Given at MDDRegisterEngine, pointer isn't disposed */
		
		TMDDGetPixelFormat  getPixelFormat;		/* METHOD: Get pixel format */
		TMDDGetVisualInfo	getVisualInfo;		/* METHOD: (Optional) Get visual info */
		TMDDContextNew		contextNew;			/* METHOD: New context */
		TMDDContextDelete	contextDelete;		/* METHOD: Delete context */
		TMDDAttachDrawable	attachDrawable;		/* METHOD: Attach drawable */
		TMDDSwapBuffers		swapBuffers;		/* METHOD: Swap buffers */
		
		TMDDSetInteger		setInteger;			/* METHOD: (Optional) Set integer */
		TMDDGetInteger		getInteger;			/* METHOD: (Optional) Get integer */
		TMDDEnable			enable;				/* METHOD: (Optional) enable */
		TMDDDisable			disable;			/* METHOD: (Optional) disable */
		TMDDIsEnabled		isEnabled;			/* METHOD: (Optional) isEnable */
		
		int					engine_num;			/* Private, don't use */
		MDDEngineInfo		info;				/* Private, don't use */	
} __TMDDEngine;

/******************************************************************************
 *
 * Searching for engines.
 *
 *****************************************************************************/
extern TMDDEngine	*MDDGetFirstEngine(void);

extern TMDDEngine	*MDDGetNextEngine(
						const TMDDEngine 		*engine);

/******************************************************************************
 *
 * For implementing GLI functions.
 *
 *****************************************************************************/
extern void					MDDEngineRendererInfo(
								const TMDDEngine		*engine,			/* MDD engine */
								GLIRendererInfo			*renderer_info);	/* The output info from engine */
						
extern TMDDError			MDDGetPixelFormat(
								const TMDDEngine		*engine,			/* MDD engine */
								GLIPixelFormat			*pixelFormat,		/* Requested pixel-format, should contains all devices, 
																			   we are going to draw onto 
																			   (ie.: pixelFormat->num_devices can't be 0). */
								TMDDPolicy				policy,				/* Pixel-format choosing policy */
								GLIPixelFormat			**outFormats);		/* One or more result pixel format each 
																			   allocated with NewPointer */
																			 						

extern TMDDEngine			*MDDGetEngineFromRendererID(
								GLint					inRendererID);		

extern struct gl_visual		*MDDGetMesaVisual(
								const TMDDEngine 		*engine,	/* MDD engine */
								GLIPixelFormat 			*pix);		/* Requested pixel format */
								

extern TMDDError			MDDContextNew(	
								const TMDDEngine 		*engine,	/* MDD engine */
								GLIPixelFormat 			*pix,		/* Requested pixel format */
								struct gl_context		*gl_ctx);	/* GL-Context made by mesa, with the pixformat */

extern void					MDDContextDelete(
								const TMDDEngine 		*engine,	/* MDD engine */
								struct gl_context		*gl_ctx);
						
extern TMDDError			MDDAttachDrawable(
								const TMDDEngine 		*engine,	/* MDD engine */
								struct gl_context		*gl_ctx,
								const TGLIDrawable			drawable);

extern GLboolean			MDDSwapBuffers(
								const TMDDEngine		*engine,
								struct gl_context		*gl_ctx);

extern GLboolean			MDDEnable(
								const TMDDEngine		*engine,
								struct gl_context		*gl_ctx,
								GLenum					pname);
extern GLboolean			MDDIsEnabled(
								const TMDDEngine		*engine,
								struct gl_context		*gl_ctx,
								GLenum 					pname,
								GLboolean 				*result);
								
extern GLboolean			MDDDisable(
								const TMDDEngine		*engine,
								struct gl_context		*gl_ctx,
								GLenum					pname);

extern GLboolean			MDDSetInteger(
								const TMDDEngine		*engine,
								struct gl_context		*gl_ctx,
								GLenum					pname,
								const GLint 			*param);
								
extern GLboolean			MDDSetInteger(
								const TMDDEngine		*engine,
								struct gl_context		*gl_ctx,
								GLenum					pname,
								const GLint 			*param);
							
extern GLboolean			MDDGetInteger(
								const TMDDEngine		*engine,
								struct gl_context		*gl_ctx,
								GLenum					pname,
								GLint 					*outParam);


/******************************************************************************
 *
 * Registering MDD engines.
 *
 *****************************************************************************/
extern TMDDError MDDRegisterEngine(
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
							
							void				*privateData );		/* Engines Prvate data */		/* Engines Prvate data */
						

extern void InstallRAVEEngines();

extern void MDDSetGLError(	GLcontext		*ctx,			/* The context, on which the error occured */
							GLenum			error_code,		/* One of GL_OUT_OF_MEMORY, etc. error codes */
							char			*debug_str,		/* Debug string, with format like printf */
							...);    						/* Parameters for debug string like in printf */
extern void	MDDFatalContextError( GLcontext *ctx);



/******************************************************************************
 *
 * Utility function for MDD drivers:
 *
 *****************************************************************************/
 extern void	MDDUMesaGetEngineInfo(TMDDEngineInfo *engineInfo);

#endif /* __MDD_H__ */