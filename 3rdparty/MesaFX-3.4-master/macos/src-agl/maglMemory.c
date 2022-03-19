/*
	File:		maglMemory.c

	Contains:	Memory allocation/deallocation routines.

	Written by:	Mikl—s Fazekas

	Copyright:	Copyright(C) 1995-99 Miklos Fazekas
				E-Mail: boga@valerie.inf.elte.hu
				WWW: http://www.mesa3d.org/mac/
				
				Part of the Mesa 3D Graphics Library for MacOS.


	Change History (most recent first):

         <1>      4/8/99    miklos  Revision for AGL 2.0 headers.
         <4>      4/7/98    miklos  Revision for Mesa 3.0b4
         <3>      3/2/98    miklos  Revision for Mesa 3.0.
         <2>    12/27/97    miklos  Inital Revision
         <1>    12/25/97    miklos  
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

#include "maglMemory.h"

#include <stdlib.h>

MAGLPtr	MAGL_Alloc(int size)
{
	return malloc(size);
}

void	MAGL_Free(MAGLPtr inPointer)
{
	free(inPointer);
}

