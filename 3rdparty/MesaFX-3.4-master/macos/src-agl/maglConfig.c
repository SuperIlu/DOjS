/*
	File:		maglConfig.c

	Contains:	Contains sources of aglConfigure and aglResetLibrary.

	Written by:	Miklos Fazekas

	Copyright:	Copyright(C) 1995-97 Miklos Fazekas
				E-Mail: boga@valerie.inf.elte.hu
				WWW: http://www.mesa3d.org/mac/Mesa.html
				
				Part of the Mesa 3D Graphics Library for MacOS.

	Change History (most recent first):

         <2>     4/10/99    miklos  Added support for AGL_ENABLE_MESA.
         <1>      4/8/99    miklos  Revision for AGL 2.0
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
#include <agl.h>
#include "gliMesa.h"

GLboolean is_mesa_enabled = GL_FALSE;

/***********************************************************************************************
 *
 * Function:	aglConfigure
 * Parameters:	pname				- Specifies the name of the parameter to be configured.
 *				param				- Specifies the new value of the parameter.
 * Returns:		GL_TRUE if suceed GL_FALSE otherwise.
 *
 * Description: aglConfigure is used to change the values of parameters that affect the operation 
 *				of the AGL library. These parameter settings affect all contexts, not just the
 *				current context.
 *				AGL_FORMAT_CACHE_SIZE
 *					param specifies the positive pixel format cache size. After an application has called
 * 					aglChoosePixelFormat for the last time, it may set the cache size to one to minimize
 *					the memory used by the AGL library. If an application intends to use n different attribute
 *					lists to choose n different pixel formats repeatedly, then the application should set the
 *					cache size to n to maximize performance. The cache size is initially set to 5.
 *				AGL_CLEAR_FORMAT_CACHE
 *					If param is nonzero, the pixel format cache contents are freed. This does not effect the
 *					size of the cache for future storage of pixel formats. To minimize the memory consumed
 *					by the cache, an application should also set the cache size to 1.
 *				AGL_RETAIN_RENDERERS
 *					If param is nonzero, the AGL library will not unload any plugin renderers even if they
 *					are no longer in use. This is useful to improve the performance of applications that
 *					repeatedly destroy and recreate their only (or last) rendering context. Normally, when the
 *					last context created by a particular plugin renderer is destroyed, that renderer is unloaded
 *					from memory. If param is zero, AGL is returned to its normal mode of operation and all
 *					renderers that are not in use are unloaded.
 *
 * 	Errors:		AGL_BAD_ENUM is set if either pname is not an accepted value.
 *				AGL_BAD_VALUE is set if param is not an appropriate setting for pname.
 *
 ***********************************************************************************************/
GLboolean aglConfigure(
			GLenum			pname,
			GLuint			param)
{
	switch (pname) {
	#ifdef AGL_ENABLE_MESA
		case AGL_ENABLE_MESA:
			is_mesa_enabled = GL_TRUE;
			return GL_TRUE;
	#endif
		case AGL_FORMAT_CACHE_SIZE:
			return GL_TRUE;
		case AGL_CLEAR_FORMAT_CACHE:
			return GL_TRUE;
		case AGL_RETAIN_RENDERERS:
			return GL_TRUE;
		default:
			return GL_FALSE;
	}
}

/***********************************************************************************************
 *
 * Function:	aglResetLibrary
 * Parameters:	
 * Returns:		
 *
 * Description: aglResetLibrary resets the OpenGL library to its initial state.  aglResetLibrary
 *		destroys all contexts created with aglCreateContext, unloads all plugin renderers from 
 *		memory, frees any data allocated by aglChoosePixelFormat or aglQueryRendererInfo, and 
 *		resets any options set with aglConfigure to their initial values.
 *		If any resources have been allocated by the OpenGL library, aglResetLibrary must be 
 *		called to free those resources before attempting to change the memory page allocation 
 *		mode of the OpenGLMemory library.
 *
 * 	Errors:		AGL_BAD_ENUM is set if either pname is not an accepted value.
 *				AGL_BAD_VALUE is set if param is not an appropriate setting for pname.
 *
 ***********************************************************************************************/

void aglResetLibrary(void)
{
	/* None of the above has any meaning on Mesa.... */
	
}