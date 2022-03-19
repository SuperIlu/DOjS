/*
	File:		GLI.mcp.SW.dbg.h

	Contains:	Header file for the Software Only version of Mesa.

	Written by:	Miklos Fazekas

	Copyright:	Miklos Fazekas © 1999.  All rights reversed.
See http://www.mesa3d.org/mac/ for more details.

	Change History (most recent first):

         <3+>	 5/7/99		miklos	Revision for the new prefix style.
         <3>      4/8/99    miklos  Revision for AGL 2.0
         <2>      4/3/99    miklos  Initial revision for Mesa 3.1b2.
         <1>      4/3/99    miklos  Initial revision.
*/



/*
** Copyright 1995-99, Mikl—s Fazekas.
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

/*
 * GLI_OPTIMIZED:Optimized version: Drivers shouldn't call printf/debugstr, etc.
 */
#define GLI_OPTIMIZED					1
/*
 * GLI_SOFTWARE_RENDERER: Build with software renderer.
 */
#define GLI_SOFTWARE_RENDERER			1
/*
 * GLI_FX_RENDERER: Build with 3Dfx renderer.
 */
#define GLI_FX_RENDERER					1
/*
 * GLI_VERBOSE: Verbose version: Drivers should print any messages to the user (unsupported features, etc.)
 */
#define GLI_VERBOSE						0
/*
 * GLI_DEBUG: Maximum debugging assertion/features
 */
#define GLI_DEBUG						1
/*
 * GLI_DEBUG_FX_TRIANGLES: Debugs fx triangles for clipping+snapping+etc.
 */
#define GLI_DEBUG_FX_TRIANGLES			1
/*
 * GLI_DEBUG_FX_LINES: Debugs fx lines for clipping+snapping+etc.
 */
#define GLI_DEBUG_FX_LINES				1
/*
 * GLI_DEBUG_FX_POINTS: Debugs fx points for clippint+snapping+etc.
 */
#define GLI_DEBUG_FX_POINTS				1
/*
 * GLI_PER_PIXEL_CHECK: Check for every pixel for it's validity
 */
#define GLI_PER_PIXEL_CHECK				0
/*
 * GLI_QUAKE_VERSION: Special Quake version for Q3 Test.
 */
#define GLI_QUAKE_VERSION				0
/*
 * GLI_CURSOR_EMULATION: Emulate cursor.
 */
#define GLI_CURSOR_EMULATION			0
/*
 * GLI_ENABLE_FULLSCREEN_HACK: Enables the fullscreen hack for accesing non fullscreen drawables 
 */
#define GLI_ENABLE_FULLSCREEN_HACK		0
/*
 * GLI_GETENV: allows configurations to be readed from a configuration file.
 */
#define GLI_GETENV						1
/*
 * GLI_3DFX_FIXGRLFBREADREGIONBUG: Fix's the possible bug in grLftReadRegion!
 */
#define GLI_3DFX_FIXGRLFBREADREGIONBUG	1


/*
** This will do the configuration for us.
*/
#include "mgliPrefix.h"
