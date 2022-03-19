/*
	File:		maglConfig.h

	Contains:	Configuration file for AGL for GLI.

	Written by:	Mikl—s Fazekas

	Copyright:	Copyright(C) 1995-99 Miklos Fazekas
				E-Mail: boga@valerie.inf.elte.hu
				WWW: http://www.mesa3d.org/mac/
				
				Part of the Mesa 3D Graphics Library for MacOS.


	Change History (most recent first):

         <2>     4/10/99    miklos  Revision for 3Dfx OpenGL.
         <1>      4/8/99    miklos  Revision for AGL 2.0
         <2>      4/7/98    miklos  Revision for Mesa 3.0b4
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
#ifndef __MAGL_CONFIG_H__
#define __MAGL_CONFIG_H__

#include "stdlib.h"
#include "gl.h"

extern	GLboolean				is_mesa_enabled;

/* Allert the user when accidently using the Mesa libraries */
extern void AlertUser();
#define MAGL_CHECK_STATE()			{ if (!is_mesa_enabled) exit(-1); }


#endif /* __MAGL_CONFIG_H__ */