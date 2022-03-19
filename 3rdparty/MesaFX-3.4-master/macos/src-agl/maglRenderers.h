/*
	File:		maglRenderers.h

	Contains:	Interface for accesing multiplie GLI renderers. - AGL for GLI.

	Written by:	Mikl—s Fazekas

	Copyright:	Copyright(C) 1995-99 Miklos Fazekas
				E-Mail: boga@valerie.inf.elte.hu
				WWW: http://www.mesa3d.org/mac/
				
				Part of the Mesa 3D Graphics Library for MacOS.


	Change History (most recent first):

        <3+>      5/7/99    miklos  Fixed errors for MPW.
         <3>     4/14/99    miklos  Support for GetInteger,SetInteger
         <2>     4/10/99    miklos  Added TGLIDrawable support.
         <1>      4/9/99    miklos  Revision for 3Dfx OpenGL.
         <4>      4/7/98    miklos  Revision for Mesa 3.0b4
         <3>      4/6/98    miklos  Added support for multiplie gli-renderers.
         <2>      3/1/98    miklos  Added better error handling functions.
         <1>    11/23/97    miklos  
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



#ifndef __AGL_RENDERERS_H__
#define __AGL_RENDERERS_H__

#include "gliMesa.h"
#if 0
#define 	kMAGL_MaxRenderersNum		256		/* The maximum number of rederers */

int			MAGL_GetNumOfRenderers(void);	/* Returns the number of registered renderers */
											/* Renderers numbered from 1..AGL_GetNumOfRenderers() */
									
int			MAGL_PixelFormat2Renderer(
							GLIPixelFormat		*inPixelFormat);
							
int			MAGL_RendererID2Renderer(GLint				inGliRendererID);

/* Returns the fisrt renderer if its 0 there is no registered renderers */
int 			MAGL_GetFirstRenderer(void);

/* Return the next renderer (if it returns 0), the inGLIRenderer was the last renderer */
int 			MAGL_GetNextRenderer(int inGLIRenderer);



GLenum	MAGLCall_gliChoosePixelFormat(
						int					inGLIRenderer,
						GLIPixelFormat 		**fmt_ret, 
						const GLIDevice 	*device, 
						GLint 				ndevs, 
						const GLint 		*attribs);
						
GLenum MAGLCall_gliDestroyPixelFormat(
						int					inGLIRenderer,
						GLIPixelFormat 		*fmt);
						
GLenum MAGLCall_gliCreateContext(
						int							inGLIRenderer,
						GLIContext 					*ctx_ret, 
						const GLIPixelFormat	 	*fmt, 
						GLIContext 					share_list);


GLboolean	   	MAGLCall_gliDestroyContext(
							int					inRenderer,
							GLIContext 			inContext);
							
							
GLboolean	   	MAGLCall_gliAttachDrawable(
							int					inRenderer,
							GLIContext 			inContext,
							GLint 				drawable_type,
							const GLIDrawable 	*drw);
							
GLboolean	   	MAGLCall_gliSetSwapRect(
							int					inRenderer,
							GLIContext			inContext,
							GLint				x,
							GLint				y,
							GLsizei				width,
							GLsizei				height);
							
GLboolean	   	MAGLCall_gliSwapBuffers(
							int					inRenderer,
							GLIContext			inContext);
							
GLboolean 	   	MAGLCall_gliDestroyPixelFormat(
							int					inRenderer,
							GLIPixelFormat 		*inPixelFormat);
							
GLboolean		MAGLCall_gliGetRendererInfo(
							int					inRenderer,
							GLIRendererInfo 	*outRendererInfo);
							
GLboolean 		MAGLCall_mgliCopyContext(
							int					inRenderer,	/* The renderer ID of the dst context */
							GLIContext			src,
							GLIContext			dst,
							GLuint				mask);
GLboolean 		MAGLCall_mgliSetInteger(
							int							inRenderer,
							GLIContext					src,
							GLint						pname,
							const GLint					*value);
GLboolean 		MAGLCall_mgliGetInteger(
							int							inRenderer,
							GLIContext					src,
							GLint						pname,
							GLint						*value);
#endif

#endif /* __AGL_RENDERERS_H__ */