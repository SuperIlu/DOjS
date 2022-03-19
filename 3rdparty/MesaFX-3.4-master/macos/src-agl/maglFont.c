/*
	File:		maglFont.c

	Contains:	Implements aglUseFont

	Written by:	Mikl—s Fazekas

	Copyright:	Copyright(C) 1995-97 Miklos Fazekas
				E-Mail: boga@valerie.inf.elte.hu
				WWW: http://valerie.inf.elte.hu/~boga/Mesa.html
				
				Part of the Mesa 3D Graphics Library for MacOS.


	Change History (most recent first):

        <3+>      7/8/99    miklos  Revision for new GLI headers.
         <3>     4/27/99    miklos  aglUseFont implemented.
         <2>      4/8/99    miklos  Revision for AGL 2.0
         <1>      4/8/99    miklos  Revision for AGL 2.0 headers.
         <4>     4/19/98    miklos  Fixed compiler warnings.
         <3>     4/19/98    miklos  Adopted new error handler functions.
         <2>      4/7/98    miklos  Revision for Mesa 3.0b4
         <1>    11/30/97    miklos  
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
#include "maglValidate.h"

#include <agl.h>
#include "gliMesa.h"

#include <QDOffscreen.h>
#include <Fonts.h>


static GLboolean getFontRect(int fontID, Style face, int size,Rect *outRect)
{
	GWorldPtr	oldPort;
	GDHandle	oldDev;
	Rect 		fontRect;
	GWorldPtr	fontBitmap;
	FontInfo	fontInfo;
	
	/* Create a small GWorld just for measuring the size... */
	SetRect(&fontRect,0,0,10,10);
	NewGWorld(&fontBitmap,1,&fontRect,NULL,NULL,keepLocal);

	if (!fontBitmap)
	{
		MAGL_SetError(NULL,AGL_BAD_ALLOC,("aglUseFont unable to create GWorld!"));
		return GL_FALSE;
	}
	
	GetGWorld(&oldPort,&oldDev);
	SetGWorld(fontBitmap,NULL);	
	
		TextSize(size);
		TextFace(face);
		TextFont(fontID);
		
		GetFontInfo(&fontInfo);
		/* Set font info! */
		fontRect.left = 0;
		fontRect.right = fontInfo.widMax;
		fontRect.top = -fontInfo.ascent; /*-fontInfo.ascent; */
		fontRect.bottom = fontInfo.descent;
	
	SetGWorld(oldPort,oldDev);
	DisposeGWorld(fontBitmap);
	
	*outRect = fontRect;
	
	return GL_TRUE;
}
/*
** Font function
*/
GLboolean aglUseFont(AGLContext ctx, GLint fontID, Style face, GLint size, GLint first, GLint count, GLint listBase)
{
	GWorldPtr	oldPort;
	GDHandle	oldDev;
	Rect 		fontRect;
	GWorldPtr	fontBitmap;
	GLint 		swapbytes;
	GLint 		lsbfirst;
	GLint 		rowlength;
	GLint 		skiprows;
	GLint 		skippixels;
	GLint 		alignment;
	GLint 		rowBytes;
	GLint 		height;
	GLubyte 	*imgdata;
	GLint		w,k;
	
	if (!IsValidContext(aglGetCurrentContext()))
	{
		MAGL_SetError(aglGetCurrentContext(),(GLenum)AGL_BAD_CONTEXT,("aglUseFont: Bad current context %h !",aglGetCurrentContext()));
		return GL_FALSE;
	}
	if (aglGetCurrentContext() == NULL)
	{
		MAGL_SetError(NULL,AGL_BAD_CONTEXT,("aglUseFont: No current context!"));
		return GL_FALSE;
	}
	
	if (!RealFont(fontID,size))
	{
		MAGL_SetError(NULL,AGL_BAD_VALUE,("aglUseFont: fontID=%d with size=%d is invalid!",fontID,size));
		return GL_FALSE;
	}
	
	if (!getFontRect(fontID,face,size,&fontRect))
	{		
		MAGL_SetError(NULL,AGL_BAD_ALLOC,("aglUseFont: Can't alloc GWorld!"));
		return GL_FALSE;
	}
	
	NewGWorld(&fontBitmap,1,&fontRect,NULL,NULL,keepLocal);
	if (fontBitmap == NULL)
	{
		MAGL_SetError(NULL,AGL_BAD_ALLOC,("aglUseFont: Can't alloc GWorld!"));
		return GL_FALSE;
	}
	
	/*
	 * SetUpData's+Prepare for drawing
	 */
	LockPixels(GetGWorldPixMap(fontBitmap));
	rowBytes = ((**GetGWorldPixMap(fontBitmap)).rowBytes & 0x3FFF);
	height = (fontBitmap->portRect.bottom-fontBitmap->portRect.top);
	imgdata = (GLubyte*)GetPixBaseAddr(GetGWorldPixMap(fontBitmap));
	GetGWorld(&oldPort,&oldDev);
	SetGWorld(fontBitmap,NULL);
	TextFont(fontID);
	TextFace(face);
	TextSize(size);
	
	/*
	 * Save old integer states!
	 */
	glGetIntegerv(GL_UNPACK_SWAP_BYTES, &swapbytes);
	glGetIntegerv(GL_UNPACK_LSB_FIRST, &lsbfirst);
	glGetIntegerv(GL_UNPACK_ROW_LENGTH, &rowlength);
	glGetIntegerv(GL_UNPACK_SKIP_ROWS, &skiprows);
	glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &skippixels);
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
	
	/*
	 * Do the drawing:
	 */
	glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
	glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, rowBytes*8);
	
	for (k = 0; k < count; k++)
	{
		
		MoveTo(0,0);
		w = CharWidth(first+k);
	
		fontRect.right = fontRect.left+w;
	
		BackColor(whiteColor);
		ForeColor(blackColor);
		EraseRect(&fontRect);
		/* FrameRect(&fontRect); */
	
		DrawChar(first+k);
	
		/*
	 	 * Swap OpenGL data for coordinates
	 	 */
		{
			GLint i,j;
				
			for (i = 0; i< height/2; i++)
				for (j = 0; j < rowBytes; j++)
				{
					GLubyte tmp;
					tmp = imgdata[i*rowBytes+j];
					imgdata[i*rowBytes+j] = imgdata[(height-1-i)*rowBytes+j];
					imgdata[(height-1-i)*rowBytes+j] = tmp;
				}
		}
		glNewList(listBase+k,GL_COMPILE);
		
		glBitmap(w,height,0,fontBitmap->portRect.bottom,w,0.0,imgdata);
	
		glEndList();
	}
	/*
	 * Restore old integer states:
	 */
	glPixelStorei(GL_UNPACK_SWAP_BYTES, swapbytes);
	glPixelStorei(GL_UNPACK_LSB_FIRST, lsbfirst);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, rowlength);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, skiprows);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, skippixels);
	glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
	
	SetGWorld(oldPort,oldDev);		 
	UnlockPixels(GetGWorldPixMap(fontBitmap));
	DisposeGWorld(fontBitmap);
	
	return GL_TRUE;
}

