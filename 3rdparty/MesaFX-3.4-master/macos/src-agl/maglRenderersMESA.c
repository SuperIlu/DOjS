/*
	File:		maglRenderersMESA.c

	Contains:	Implements the calling interface for the statically linked Mesa GLI driver.

	Written by:	Mikl—s Fazekas

	Copyright:	Copyright(C) 1995-99 Miklos Fazekas
				E-Mail: boga@valerie.inf.elte.hu
				WWW: http://www.mesa3d.org/mac/
				
				Part of the Mesa 3D Graphics Library for MacOS.


	Change History (most recent first):

        <3+>      5/7/99    miklos  Fixed errors for MPW.
         <3>     4/14/99    miklos  Support for GetInteger,SetInteger
         <2>     4/10/99    miklos  Revision for 3Dfx OpenGL.
         <1>      4/8/99    miklos  Revision for AGL 2.0
         <8>     7/15/98    miklos  Revision for compiling with MPW C.
         <7>     4/19/98    miklos  Adopted new error handler functions.
         <6>      4/7/98    miklos  Revision for Mesa 3.0b4
         <5>      4/6/98    miklos  Added support for multiplie gli-renderers.
         <4>      3/1/98    miklos  Added mgliCopyContext.
         <3>     1/30/98    miklos  Changed kMesaRendererID to 1100.
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

#include <agl.h>
#include <gli.h>
#include <gliMesa.h>


typedef struct TGLIApiTable {
	int						mRendererID;
	void					*mRefContext;
	
	TGLIChoosePixelFormat	mChoosePixelFormat;
	TGLIDestroyPixelFormat	mDestroyPixelFormat;
	TGLIGetRendererInfo		mGetRendererInfo;
	TGLICreateContext		mCreateContext;
	TGLIDestroyContext		mDestroyContext;
	TGLIAttachDrawable		mAttachDrawable;
	TGLIAttachFullscreen	mAttachFullscreen;
	TGLISetSwapRect			mSetSwapRect;
	TGLISwapBuffers			mSwapBuffers;
	TGLICopyContext			mCopyContext;
	TGLIQueryVersion		mQueryVersion;
	TGLIEnable				mEnable;
	TGLIDisable				mDisable;
	TGLIGetInteger			mGetInteger;
	TGLISetInteger			mSetInteger;
}TGLIApiTable;

static GLint			gGliRenderersNum		= 	0;
#define					kMaxGliRenderersNum			10

static TGLIApiTable		gGliApiTables[kMaxGliRenderersNum];

GLboolean RegisterGLIRenderer(
					int						inRendererID,
					void					*inRefContext,
					TGLIChoosePixelFormat	inChoosePixelFormat,
					TGLIDestroyPixelFormat	inDestroyPixelFormat,
					TGLIGetRendererInfo		inGetRendererInfo,
					TGLICreateContext		inCreateContext,
					TGLIDestroyContext		inDestroyContext,
					TGLIAttachDrawable		inAttachDrawable,
					TGLISetSwapRect			inSetSwapRect,
					TGLISwapBuffers			inSwapBuffers,
					TGLICopyContext			inCopyContext,
					TGLIQueryVersion		inQueryVerison,
					TGLIEnable				inEnable,
					TGLIDisable				inDisable,
					TGLIGetInteger			inGetInteger,
					TGLISetInteger			inSetInteger)
{
	TGLIApiTable	apiTable;
	int				i;
	
	apiTable.mRendererID			= inRendererID;
	apiTable.mRefContext			= inRefContext;
	apiTable.mChoosePixelFormat		= inChoosePixelFormat;
	apiTable.mDestroyPixelFormat	= inDestroyPixelFormat;
	apiTable.mGetRendererInfo		= inGetRendererInfo;
	apiTable.mCreateContext			= inCreateContext;
	apiTable.mDestroyContext		= inDestroyContext;
	apiTable.mAttachDrawable		= inAttachDrawable;
	apiTable.mSetSwapRect			= inSetSwapRect;
	apiTable.mSwapBuffers			= inSwapBuffers;
	apiTable.mCopyContext			= inCopyContext;
	apiTable.mQueryVersion			= inQueryVerison;
	apiTable.mEnable				= inEnable;
	apiTable.mDisable				= inDisable;
	apiTable.mSetInteger			= inSetInteger;
	apiTable.mGetInteger			= inGetInteger;
	
	/* See if the inRendererID is unique */
	for (i = 0; i < gGliRenderersNum; i++)
	{
		if (gGliApiTables[gGliRenderersNum].mRendererID == inRendererID)
		{
			MAGL_Warning(NULL,("RegisterGLIRenderer:The ID %d is already used!",inRendererID));
			return GL_FALSE;
		}
	}
	
	if (gGliRenderersNum < kMaxGliRenderersNum)
	{
		gGliRenderersNum++;
		gGliApiTables[gGliRenderersNum-1] = apiTable;
		return GL_TRUE;
	}
	else
	{
		MAGL_Warning(NULL,("RegisterGLIRenderer:The GLI renderer table is full!"));
		return GL_FALSE;
	}
}
					
static GLboolean 
IsValidRenderer(int inRenderer)
{
	return ((inRenderer >= 1) && (inRenderer <= gGliRenderersNum));
}
extern void IntsallGLIEngines(void);


int MAGL_GetFirstRenderer(void)
{
	if (gGliRenderersNum == 0)
		IntsallGLIEngines();
		
	if (gGliRenderersNum >= 1)
		return 1;
	else
		return 0;
}

int MAGL_GetNextRenderer(int inGLIRenderer)
{
	if (inGLIRenderer < gGliRenderersNum)
		return inGLIRenderer+1;
	else
		return 0;
}

int MAGL_GetNumOfRenderers(void)
{
	if (gGliRenderersNum == 0)
		IntsallGLIEngines();
		
	return gGliRenderersNum;
}

int	MAGL_RendererID2Renderer(GLint				inGliRendererID)
{
	int i;
	for (i = 0; i < gGliRenderersNum; i++)
	{
		if (gGliApiTables[i].mRendererID == inGliRendererID)
			return i+1;
	}
	
	return 0;
}

GLIPixelFormat *MAGLCall_gliChoosePixelFormat(
					int 				inRenderer,
					const GLint 		*attrib)
{
	MAGL_Assert(IsValidRenderer(inRenderer));
	
	return gGliApiTables[inRenderer-1].mChoosePixelFormat(gGliApiTables[inRenderer-1].mRefContext,attrib);
}

GLboolean MAGLCall_gliDestroyPixelFormat(
							int					inRenderer,
							GLIPixelFormat		*inPixelFormat)
{
	MAGL_Assert(IsValidRenderer(inRenderer));
	
	return gGliApiTables[inRenderer-1].mDestroyPixelFormat(gGliApiTables[inRenderer-1].mRefContext,inPixelFormat);
}

GLIContext	   MAGLCall_gliCreateContext(
							int					inRenderer,
							GLIPixelFormat 		*inPixelFormat,
							GLIContext 			inShare)
{
	MAGL_Assert(IsValidRenderer(inRenderer));
	
	return gGliApiTables[inRenderer-1].mCreateContext(gGliApiTables[inRenderer-1].mRefContext,inPixelFormat,inShare);
}

GLboolean	   MAGLCall_gliDestroyContext(
							int					inRenderer,
							GLIContext 			inContext)
{
	MAGL_Assert(IsValidRenderer(inRenderer));
	
	return gGliApiTables[inRenderer-1].mDestroyContext(gGliApiTables[inRenderer-1].mRefContext,inContext);
}
#if 0
GLboolean	   MAGLCall_gliAttahcFullscreen(
								int				inRenderer,
								AGLContext 		ctx,
							 	GLsizei 		width,
								GLsizei 		height,
							 	GLsizei 		freq,
							 	GLint			device 
								)
{
	MAGL_Assert(IsValidRenderer(inRenderer));
	
	return gGliApiTables[inRenderer-1].mAttachFullscreen(gGliApiTables[inRenderer-1].mRefContext,inContext,inDrawable);
}
#endif
GLenum	   MAGLCall_gliAttachDrawable(
							int						inRenderer,
							GLIContext 				inContext,
							const TGLIDrawable		inDrawable)
{
	MAGL_Assert(IsValidRenderer(inRenderer));
	
	return gGliApiTables[inRenderer-1].mAttachDrawable(gGliApiTables[inRenderer-1].mRefContext,inContext,inDrawable);
}

GLboolean	MAGLCall_gliGetRendererInfo(
							int					inRenderer,
							GLIRendererInfo 	*outRendererInfo)
{
	MAGL_Assert(IsValidRenderer(inRenderer));
	
	return gGliApiTables[inRenderer-1].mGetRendererInfo(gGliApiTables[inRenderer-1].mRefContext,outRendererInfo);
}

GLboolean MAGLCall_gliSetSwapRect(
							int					inRenderer,
							GLIContext 			inContext,
							GLint x, 
							GLint y, 
							GLsizei width, 
							GLsizei height)
{
	MAGL_Assert(IsValidRenderer(inRenderer));
	
	return gGliApiTables[inRenderer-1].mSetSwapRect(gGliApiTables[inRenderer-1].mRefContext,inContext,x,y,width,height);
}

GLboolean MAGLCall_gliEnable(
							int					inRenderer,
							GLIContext 			inContext,
							int					pname)
{
	MAGL_Assert(IsValidRenderer(inRenderer));
	
	return gGliApiTables[inRenderer-1].mEnable(gGliApiTables[inRenderer-1].mRefContext,inContext,pname);
}
GLboolean MAGLCall_gliDisable(
							int					inRenderer,
							GLIContext 			inContext,
							int					pname)
{
	MAGL_Assert(IsValidRenderer(inRenderer));
	
	return gGliApiTables[inRenderer-1].mDisable(gGliApiTables[inRenderer-1].mRefContext,inContext,pname);
}

GLboolean MAGLCall_gliSwapBuffers(
							int					inRenderer,
							GLIContext 			inContext)
{
	MAGL_Assert(IsValidRenderer(inRenderer));
	
	return gGliApiTables[inRenderer-1].mSwapBuffers(gGliApiTables[inRenderer-1].mRefContext,inContext);
}

GLboolean MAGLCall_mgliCopyContext(
							int							inRenderer,
							GLIContext					src,
							GLIContext					dst,
							GLbitfield					mask)
{
	MAGL_Assert(IsValidRenderer(inRenderer));

	return gGliApiTables[inRenderer-1].mCopyContext(gGliApiTables[inRenderer-1].mRefContext,src,dst,mask);
}

GLboolean MAGLCall_mgliSetInteger(
							int							inRenderer,
							GLIContext					src,
							GLint						pname,
							const GLint					*value)
{
	MAGL_Assert(IsValidRenderer(inRenderer));

	return gGliApiTables[inRenderer-1].mSetInteger(gGliApiTables[inRenderer-1].mRefContext,src,pname,value);
}
GLboolean MAGLCall_mgliGetInteger(
							int							inRenderer,
							GLIContext					src,
							GLint						pname,
							GLint						*value)
{
	MAGL_Assert(IsValidRenderer(inRenderer));

	return gGliApiTables[inRenderer-1].mGetInteger(gGliApiTables[inRenderer-1].mRefContext,src,pname,value);
}


