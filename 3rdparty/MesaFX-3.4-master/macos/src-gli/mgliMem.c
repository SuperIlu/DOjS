/*
	File:		mgliMem.c

	Contains:	Memory allocation routines for Mac Mesa.

	Written by:	Miklos Fazekas

	Copyright:	Miklos Fazekas © 1999.  All rights reversed.
    See http://www.mesa3d.org/mac/ for more details.

	Change History (most recent first):

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

#include "mgliConfig.h"

#include "gli.h"

#include "mgliError.h"
#include "mgliMem.h"
#include "mgliContext.h"

/* MacOS */
#include <Memory.h>

/* ANSI */
#include <stdlib.h>


#define  	kMGLI_TempMemSize 	 MAX_WIDTH*sizeof(double)	/* 10 kb of space */


static char 		gTempMem[kMGLI_TempMemSize];
static GLboolean	gTempMemUsed = GL_FALSE;	/* Indicates whatever the TempMem is in use */

/*******************************************************************************************
 *
 * Routines for allocating, and releasing used only by MESA!!!
 * If there is not enough memory it won't give back NULL, it will give back an addres of
 * a large (64 KB, or as set in "aglMemUtil.c" in kTempMemSize size variable), it's also generates 
 * a GL_OUT_OF_MEMORY error.
 *
 * For performace issuse we are using the ansi-lib version of malloc/free/calloc.
 * TODO: Maybe do a simple memory managment in this 64 kb.
 * 
 ******************************************************************************************/ 

char *AGLAlloc(int size)
{
	char*	result;
	if (!gTempMemUsed && size < kMGLI_TempMemSize)
	{
		result = gTempMem;
		gTempMemUsed = GL_TRUE;
	}
	else
	{
		result = (char *)malloc(size);
	}
	
	if (result != NULL)
	{
		return result;
	}
	else
	{
		if (size >= kMGLI_TempMemSize)
		{
			mgliFatalError(kMGLIOutOfMemory,"AGLAlloc: We are going to corrupt the memory, :) !");
		}
		
		/* 
		 * TODO: Tell Mesa that we've run out of memory!
		 */
		mgliSetGLError(NULL,GL_OUT_OF_MEMORY,"Not enough memory!");
		
		/*
		 * Give out the Temporary memory.
		 */
		return gTempMem;
	}
}

void AGLFree( 
						char* ptr )			/* Data to dispose */
{
	if (ptr != gTempMem)
	{
		free(ptr);
	}
	else
	{
		gTempMemUsed = GL_FALSE;
		/*
		 * We can't dispose the Temporary mem.
		 */
	}
}

void *mgliAlloc(int size)
{
	return (void*)NewPtrClear(size);
}
void mgliFree(void *data)
{
	DisposePtr((Ptr)data);
}