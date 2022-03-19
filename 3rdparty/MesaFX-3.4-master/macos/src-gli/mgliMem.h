/*
	File:		mgliMem.h

	Contains:	Memory functions for GLI.

	Written by:	Fazekas Mikl—s

	Copyright:	Copyright(C) 1995-98 Miklos Fazekas
				E-Mail: boga@valerie.inf.elte.hu
				WWW: http://valerie.inf.elte.hu/~boga/Mesa.html
				
				Part of the Mesa 3D Graphics Library for MacOS.

	Change History (most recent first):

        <1+>      7/2/98    miklos  Email address changed.
         <1>      2/1/98    miklos  Initial Revision.
*/


/*
** mgliMem.h - gli bindings for Mesa, version: 0.1
*/

/*
** AGL driver for Mesa 3D Graphics Library.
** Version: 0.1
** Mesa version: 2.4
**
** Copyright(C) 1995-97 Miklos Fazekas
** E-Mail: boga@augusta.inf.elte.hu
** WWW: http://www.elte.hu/~boga/Mesa.html
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

#ifndef __MGLI_MEM_H__
#define __MGLI_MEM_H__

#include "gliMesa.h"

#include "mgliConfig.h"
#include "mgliError.h"

extern void *mgliAlloc(int size);
extern void mgliFree(void *data);

#endif /* __MGLI_ERROR_H__ */