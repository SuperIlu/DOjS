/*
	File:		gliMesa.h

	Contains:	GLI Mesa header.

	Written by:	Miklos Fazekas

	Copyright:	Copyright(C) 1995-99 Miklos Fazekas
				E-Mail: boga@velerie.inf.elte.hu
				WWW: http://www.mesa3d.org/mac/
				
				Part of the Mesa 3D Graphics Library for MacOS.

	Change History (most recent first):

         <1>     4/27/99    miklos  Initial revision.
*/


#ifndef _GLIMESA_H
#define _GLIMESA_H

#if REAL_GLI

	#include "gli.h"
	#include "aglMesa.h"
	
	/* Mesa rendering ID's */
	#define GLI_RENDERER_MESA_3DFX_ID              	AGL_RENDERER_MESA_3DFX_ID

	/* Mesa constants */
	#define GLI_ACTIVE_FULLSCREEN			AGL_ACTIVE_FULLSCREEN
	#define GLI_MESA_3DFX_GAMMA_VALUE		AGL_MESA_3DFX_GAMMA_VALUE
	#define GLI_3DFX_GAMMA_SCALE		AGL_3DFX_GAMMA_SCALE

#else


	#include "agl.h"
	#include "aglMesa.h"
	#include "gliDispatch.h"
	
	/*
	 * Software Mesa Rendere ID: unofficial, for internal use only....
	 */
	#define AGL_RENDERER_MESA_SOFTWARE_ID          			AGL_RENDERER_MESA_3DFX_ID+3

/*****************************************************************************************************
 *
 * Types:
 *
 *****************************************************************************************************/
typedef struct 			GLIDevice {
	GDHandle 		gdevice;
}GLIDevice;
/*
** Renderer info record:
*/
typedef struct GLIRendererInfo {
	struct GLIRendererInfo* next_renderer_info;
	GLint           	renderer_id;
	GLbitfield       	os_support;
	GLbitfield       	buffer_modes;
	GLbitfield       	color_modes;
	GLbitfield       	accum_modes;
	GLbitfield     	  	depth_modes;
	GLbitfield       	stencil_modes;
	GLbitfield       	index_sizes;
	GLshort            	max_aux_buffers;
	GLshort			magic;
	GLint			video_memory_size;	/* Video Memory size in bytes */
	GLint			texture_memory_size; 	/* Texture Memory size in bytes */
} GLIRendererInfo;

/*
** Pixel format record:
*/
typedef struct GLIPixelFormat {
	struct GLIPixelFormat 	*next_pixel_format;
	GLint           	renderer_id;
	GLbitfield       	os_support;
	GLbitfield		buffer_mode;
	GLbitfield		color_mode;
	GLint			accum_mode;
	GLbitfield		depth_mode;
	GLbitfield		stencil_mode;
	GLshort			aux_buffers;
	GLshort 		level;
	GLint			num_devices;
	GLIDevice		*devices;
} GLIPixelFormat;





#define GLIDrawable		AGLDrawable

/*****************************************************************************************************
 *
 * Function prototypes:
 *
 *****************************************************************************************************/
extern GLenum    gliChoosePixelFormat( GLIPixelFormat **fmt_ret, const GLIDevice *device, GLint ndevs, const GLint *attribs);
extern GLenum 	 gliDestroyPixelFormat(GLIPixelFormat *fmt);
extern GLboolean gliGetAttribute(GLIContext ctx, GLenum type, void *attrib);
extern GLboolean gliSetAttribute(GLIContext ctx, GLenum type, const void *attrib);
extern GLenum 	 gliQueryRendererInfo(GLIRendererInfo **info_ret, const GLIDevice *dev, GLint ndevs);
extern GLenum 	 gliDestroyRendererInfo(GLIRendererInfo *info);
extern GLenum 	 gliGetSystemDispatch(GLIContext ctx, GLIFunctionDispatch *table, GLbitfield change_flags);
extern GLenum 	 gliGetFunctionDispatch(GLenum mode,GLIFunctionDispatch *table);
extern GLenum 	 gliGetExtensionDispatch(GLenum mode,GLIExtensionDispatch *table);
extern GLenum	 gliAttachDrawable(GLIContext ctx,GLint drawable_type,const GLIDrawable *drw);
extern GLenum 	 gliCreateContext(GLIContext *ctx_ret, const GLIPixelFormat *fmt, GLIContext share_list);
extern GLenum 	 gliDestroyContext(GLIContext ctx);
extern GLenum 	 gliSwapBuffers(GLIContext ctx);
extern GLenum 	 gliSetInteger(GLIContext inCtx,GLenum pname,const GLint	*value);
extern GLenum 	 gliGetInteger(GLIContext inCtx,GLenum pname,GLint *value);
/* Only used in Mesa: */
extern void 	 gliMakeCurrent(GLIContext ctx);	/* Used only in Mesa !!!! */

/*****************************************************************************************************
 *
 * Contants definitions for MESA
 *
 *****************************************************************************************************/

/* Mesa rendering ID's */
#define GLI_RENDERER_MESA_3DFX_ID              	AGL_RENDERER_MESA_3DFX_ID

/* Mesa constants */
#define GLI_ACTIVE_FULLSCREEN			AGL_ACTIVE_FULLSCREEN
#define GLI_MESA_3DFX_GAMMA_VALUE		AGL_MESA_3DFX_GAMMA_VALUE
	#define GLI_3DFX_GAMMA_SCALE		AGL_3DFX_GAMMA_SCALE
/* #define GLI_MESA_3DFX_AVAILABLE_RESOLUTIONS	0x11020	*//* ToDo:Available resolutions for the 3Dfx card */


/*****************************************************************************************************
 *
 * Contants: i'm not sure about them...
 *
 *****************************************************************************************************/
#define GLI_WINDOW_BIT					   0x000000001
#define GLI_FULLSCREEN_BIT				   0x000000002
#define GLI_OFFSCREEN_BIT				   0x000000004
#define GLI_BACKING_STORE_BIT			  	   0x000000008
#define GLI_MP_SAFE_BIT					   0x000000010
#define GLI_ROBUST_BIT					   0x000000020
#define GLI_NEEDED_BIT					   0x000000080
#define GLI_ACCELERATED_BIT				   0x000000100
#define GLI_MULTISCREEN_BIT				   0x000000200    
#define GLI_COMPLIANT_BIT				   0x000000400	


/*****************************************************************************************************
 *
 * Contants definitions from AGL.
 * See agl.h for their description.
 *
 *****************************************************************************************************/

#define GLI_NONE                   AGL_NONE
#define GLI_ALL_RENDERERS          AGL_ALL_RENDERERS  	
#define GLI_BUFFER_SIZE            AGL_BUFFER_SIZE  	
#define GLI_LEVEL                  AGL_LEVEL  		
#define GLI_RGBA                   AGL_RGBA  		
#define GLI_DOUBLEBUFFER           AGL_DOUBLEBUFFER 	
#define GLI_STEREO                 AGL_STEREO  		
#define GLI_AUX_BUFFERS            AGL_AUX_BUFFERS  	
#define GLI_RED_SIZE               AGL_RED_SIZE  	
#define GLI_GREEN_SIZE             AGL_GREEN_SIZE  	
#define GLI_BLUE_SIZE              AGL_BLUE_SIZE  	
#define GLI_ALPHA_SIZE             AGL_ALPHA_SIZE  	
#define GLI_DEPTH_SIZE             AGL_DEPTH_SIZE  
#define GLI_STENCIL_SIZE           AGL_STENCIL_SIZE  
#define GLI_ACCUM_RED_SIZE         AGL_ACCUM_RED_SIZE 
#define GLI_ACCUM_GREEN_SIZE       AGL_ACCUM_GREEN_SIZE
#define GLI_ACCUM_BLUE_SIZE        AGL_ACCUM_BLUE_SIZE 
#define GLI_ACCUM_ALPHA_SIZE       AGL_ACCUM_ALPHA_SIZE

#define GLI_PIXEL_SIZE            AGL_PIXEL_SIZE
#define GLI_MINIMUM_POLICY        AGL_MINIMUM_POLICY  
#define GLI_MAXIMUM_POLICY        AGL_MAXIMUM_POLICY
#define GLI_OFFSCREEN             AGL_OFFSCREEN  
#define GLI_FULLSCREEN            AGL_FULLSCREEN 
#define GLI_RENDERER_ID           AGL_RENDERER_ID 
#define GLI_SINGLE_RENDERER       AGL_SINGLE_RENDERER 
#define GLI_NO_RECOVERY           AGL_NO_RECOVERY 
#define GLI_ACCELERATED           AGL_ACCELERATED 
#define GLI_CLOSEST_POLICY        AGL_CLOSEST_POLICY 
#define GLI_ROBUST                AGL_ROBUST 
#define GLI_BACKING_STORE         AGL_BACKING_STORE 
#define GLI_MP_SAFE               AGL_MP_SAFE

#define GLI_WINDOW                AGL_WINDOW 
#define GLI_MULTISCREEN           AGL_MULTISCREEN 
#define GLI_VIRTUAL_SCREEN        AGL_VIRTUAL_SCREEN 
#define GLI_COMPLIANT             AGL_COMPLIANT 

#define GLI_BUFFER_MODES         AGL_BUFFER_MODES
#define GLI_MIN_LEVEL            AGL_MIN_LEVEL
#define GLI_MAX_LEVEL            AGL_MAX_LEVEL
#define GLI_COLOR_MODES          AGL_COLOR_MODES
#define GLI_ACCUM_MODES          AGL_ACCUM_MODES
#define GLI_DEPTH_MODES          AGL_DEPTH_MODES
#define GLI_STENCIL_MODES        AGL_STENCIL_MODES
#define GLI_MAX_AUX_BUFFERS      AGL_MAX_AUX_BUFFERS
#define GLI_VIDEO_MEMORY         AGL_VIDEO_MEMORY
#define GLI_TEXTURE_MEMORY       AGL_TEXTURE_MEMORY

#define GLI_SWAP_RECT	         AGL_SWAP_RECT  
#define GLI_BUFFER_RECT          AGL_BUFFER_RECT 
#define GLI_COLORMAP_TRACKING    AGL_COLORMAP_TRACKING  
#define GLI_COLORMAP_ENTRY       AGL_COLORMAP_ENTRY  
#define GLI_RASTERIZATION        AGL_RASTERIZATION 
#define GLI_SWAP_INTERVAL        AGL_SWAP_INTERVAL  
#define GLI_STATE_VALIDATION     AGL_STATE_VALIDATION  

#define GLI_FORMAT_CACHE_SIZE    AGL_FORMAT_CACHE_SIZE 
#define GLI_CLEAR_FORMAT_CACHE   AGL_CLEAR_FORMAT_CACHE
#define GLI_RETAIN_RENDERERS     AGL_RETAIN_RENDERERS

#define GLI_MONOSCOPIC_BIT       AGL_MONOSCOPIC_BIT
#define GLI_STEREOSCOPIC_BIT     AGL_STEREOSCOPIC_BIT
#define GLI_SINGLEBUFFER_BIT     AGL_SINGLEBUFFER_BIT
#define GLI_DOUBLEBUFFER_BIT     AGL_DOUBLEBUFFER_BIT

#define GLI_0_BIT                AGL_0_BIT
#define GLI_1_BIT                AGL_1_BIT
#define GLI_2_BIT                AGL_2_BIT
#define GLI_3_BIT                AGL_3_BIT
#define GLI_4_BIT                AGL_4_BIT
#define GLI_5_BIT                AGL_5_BIT
#define GLI_6_BIT                AGL_6_BIT
#define GLI_8_BIT                AGL_8_BIT
#define GLI_10_BIT               AGL_10_BIT
#define GLI_12_BIT               AGL_12_BIT
#define GLI_16_BIT               AGL_16_BIT
#define GLI_24_BIT               AGL_24_BIT
#define GLI_32_BIT               AGL_32_BIT
#define GLI_48_BIT               AGL_48_BIT
#define GLI_64_BIT               AGL_64_BIT
#define GLI_96_BIT               AGL_96_BIT
#define GLI_128_BIT              AGL_128_BIT

#define GLI_RGB8_BIT             AGL_RGB8_BIT  
#define GLI_RGB8_A8_BIT          AGL_RGB8_A8_BIT 
#define GLI_BGR233_BIT           AGL_BGR233_BIT  
#define GLI_BGR233_A8_BIT        AGL_BGR233_A8_BIT 
#define GLI_RGB332_BIT           AGL_RGB332_BIT  
#define GLI_RGB332_A8_BIT        AGL_RGB332_A8_BIT 
#define GLI_RGB444_BIT           AGL_RGB444_BIT  
#define GLI_ARGB4444_BIT         AGL_ARGB4444_BIT
#define GLI_RGB444_A8_BIT        AGL_RGB444_A8_BIT 
#define GLI_RGB555_BIT           AGL_RGB555_BIT  
#define GLI_ARGB1555_BIT         AGL_ARGB1555_BIT  
#define GLI_RGB555_A8_BIT        AGL_RGB555_A8_BIT  
#define GLI_RGB565_BIT           AGL_RGB565_BIT  
#define GLI_RGB565_A8_BIT        AGL_RGB565_A8_BIT  
#define GLI_RGB888_BIT           AGL_RGB888_BIT 
#define GLI_ARGB8888_BIT         AGL_ARGB8888_BIT 
#define GLI_RGB888_A8_BIT        AGL_RGB888_A8_BIT 
#define GLI_RGB101010_BIT        AGL_RGB101010_BIT 
#define GLI_ARGB2101010_BIT      AGL_ARGB2101010_BIT  
#define GLI_RGB101010_A8_BIT     AGL_RGB101010_A8_BIT  
#define GLI_RGB121212_BIT        AGL_RGB121212_BIT 
#define GLI_ARGB12121212_BIT     AGL_ARGB12121212_BIT  
#define GLI_RGB161616_BIT        AGL_RGB161616_BIT  
#define GLI_ARGB16161616_BIT     AGL_ARGB16161616_BIT 
#define GLI_INDEX8_BIT           AGL_INDEX8_BIT  
#define GLI_INDEX16_BIT          AGL_INDEX16_BIT  


#define GLI_NO_ERROR             AGL_NO_ERROR

#define GLI_BAD_ATTRIBUTE        AGL_BAD_ATTRIBUTE 
#define GLI_BAD_PROPERTY         AGL_BAD_PROPERTY
#define GLI_BAD_PIXELFMT         AGL_BAD_PIXELFMT 
#define GLI_BAD_RENDINFO         AGL_BAD_RENDINFO
#define GLI_BAD_CONTEXT          AGL_BAD_CONTEXT 
#define GLI_BAD_DRAWABLE         AGL_BAD_DRAWABLE 
#define GLI_BAD_GDEV             AGL_BAD_GDEV
#define GLI_BAD_STATE            AGL_BAD_STATE 
#define GLI_BAD_VALUE            AGL_BAD_VALUE 
#define GLI_BAD_MATCH            AGL_BAD_MATCH 
#define GLI_BAD_ENUM             AGL_BAD_ENUM 
#define GLI_BAD_OFFSCREEN        AGL_BAD_OFFSCREEN 
#define GLI_BAD_FULLSCREEN       AGL_BAD_FULLSCREEN 
#define GLI_BAD_WINDOW           AGL_BAD_WINDOW 
#define GLI_BAD_POINTER          AGL_BAD_POINTER 
#define GLI_BAD_MODULE           AGL_BAD_MODULE 
#define GLI_BAD_ALLOC            AGL_BAD_ALLOC



#endif /* !REAL_GLI */

#endif /* _GLIMESA_H */