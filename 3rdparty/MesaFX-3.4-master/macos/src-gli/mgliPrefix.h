/*
	File:		mgliPrefix.h

	Contains:	Conifuration file.

	Written by:	Miklos Fazekas

	Copyright:	Copyright(C) 1995-99 Miklos Fazekas
				E-Mail: boga@velerie.inf.elte.hu
				WWW: http://www.mesa3d.org/mac/
				
				Part of the Mesa 3D Graphics Library for MacOS.

	Change History (most recent first):

       
         <4>      7/5/99    miklos  Revision.
         <2>     4/28/99    miklos  Initial revision.
*/


/*
 * GLI_OPTIMIZED:Optimized version: Drivers shouldn't call printf/debugstr, etc.
 */
#ifndef GLI_OPTIMIZED
	#define GLI_OPTIMIZED					1
#endif
/*
 * GLI_SOFTWARE_RENDERER: Build with software renderer.
 */
#ifndef GLI_SOFTWARE_RENDERER
	#define GLI_SOFTWARE_RENDERER			1
#endif
/*
 * GLI_FX_RENDERER: Build with 3Dfx renderer.
 */
#ifndef GLI_FX_RENDERER
	#define GLI_FX_RENDERER			1
#endif
/*
 * GLI_VERBOSE: Verbose version: Drivers should print any messages to the user (unsupported features, etc.)
 */
#ifndef GLI_VERBOSE
	#define GLI_VERBOSE						0
#endif
/*
 * GLI_DEBUG: Maximum debugging assertion/features
 */
#ifndef GLI_DEBUG
	#define GLI_DEBUG							0
#endif
/*
 * GLI_DEBUG_FX_TRIANGLES: Debugs fx triangles for clipping+snapping+etc.
 */
#ifndef GLI_DEBUG_FX_TRIANGLES
	#define GLI_DEBUG_FX_TRIANGLES				0
#endif
/*
 * GLI_DEBUG_FX_TRIANGLES: Debugs fx triangles for clipping+snapping+etc.
 */
#ifndef GLI_DEBUG_FX_LINES
	#define GLI_DEBUG_FX_LINES					0
#endif
/*
 * GLI_DEBUG_FX_TRIANGLES: Debugs fx triangles for clipping+snapping+etc.
 */
#ifndef GLI_DEBUG_FX_POINTS
	#define GLI_DEBUG_FX_POINTS					0
#endif
/*
 * GLI_PER_PIXEL_CHECK: Check for every pixel for it's validity
 */
#ifndef GLI_PER_PIXEL_CHECK
	#define GLI_PER_PIXEL_CHECK					0
#endif
/*
 * GLI_QUAKE_VERSION: Special Quake version for Q3 Test.
 */
#ifndef GLI_QUAKE_VERSION
	#define GLI_QUAKE_VERSION					0
#endif
/*
 * GLI_CURSOR_EMULATION: Emulate cursor.
 */
#ifndef GLI_CURSOR_EMULATION
	#define GLI_CURSOR_EMULATION				0
#endif
/*
 * GLI_ENABLE_FULLSCREEN_HACK: Enables the fullscreen hack for accesing non fullscreen drawables 
 */
#ifndef GLI_ENABLE_FULLSCREEN_HACK
	#define GLI_ENABLE_FULLSCREEN_HACK			0
#endif
/*
 * GLI_GETENV: allows configurations to be readed from a configuration file.
 */
#ifndef GLI_GETENV
	#define GLI_GETENV							1
#endif
/*
 * GLI_3DFX_FIXGRLFBREADREGIONBUG: Fixes the read region bug found in some versions of Glide!
 */
#ifndef GLI_3DFX_FIXGRLFBREADREGIONBUG
	#define GLI_3DFX_FIXGRLFBREADREGIONBUG		0
#endif




/*
 * GLI_OPENGL_ENGINE: Will compiled as an OpenGL engine
 */
#ifndef GLI_OPENGL_ENGINE
	#define GLI_OPENGL_ENGINE					0
#endif

/*
 * USE_OPENGL_MEMORY: Uses the OpenGL memory manager.
 */
#ifndef USE_OPENGL_MEMORY
	#define USE_OPENGL_MEMORY					0
#endif
/*
 * MESA_3DFX_STANDALONE_GLI_PLUGIN: Build as a standalone 3Dfx Plugin
 */
#ifndef MESA_3DFX_STANDALONE_GLI_PLUGIN	
	#define MESA_3DFX_STANDALONE_GLI_PLUGIN		0
#endif
/*
 * MESA_3DFX_PROFILE: Extract MW profiling informations.
 */
#ifndef MESA_3DFX_PROFILE
	#define MESA_3DFX_PROFILE					0
#endif

#if MESA_3DFX_PROFILE
	#define GLI_PROFILE_FX_TRIANGLES 1
#else
	#define GLI_PROFILE_FX_TRIANGLES 0
#endif
/*
 * Defines reuired for Mesa: 
 */
/* This is required for compatibility with the GL library!!! */
#define FX										1

#if GLI_VERBOSE
	#define	DEBUG_FXMESA						1
#else
	#define FX_SILENT
#endif

/* Debug */
#if GLI_DEBUG
	#define DEBUG								1
#endif

/* 
 * USE_GLIDE_3: To use Glide3 or not.
 */
#ifndef MESA_3DFX_USE_GLIDE_3
    #define MESA_3DFX_USE_GLIDE_3                 0
#endif

#if  MESA_3DFX_USE_GLIDE_3
    #define GLIDE3 								  1
    #define GLIDE3_ALPHA						  1
    #define FX_GLIDE3							  1
 /* For profiling the VertexArray's */
#if MESA_3DFX_PROFILE
	#define  GLI_PROFILE_FX_DRAW_ARRAY 	1

#endif
    
#endif
/* GLI_DEBUG_FX_TRIANGLES: This will redirect grDrawTriangle to our __grDrawTriangle */
#if GLI_DEBUG_FX_TRIANGLES || GLI_PROFILE_FX_TRIANGLES
	#define grDrawTriangle(a,b,c)				__grDrawTriangle(a,b,c)
/*	#define grAADrawTriangle(a,b,c,d,e,f)		__grDrawTriangle(a,b,c) */
#endif

#if GLI_DEBUG_FX_TRIANGLES || GLI_PROFILE_FX_DRAW_ARRAY   
    #define grDrawVertexArray			__grDrawVertexArray
#endif

#if GLI_DEBUG_FX_LINES
	#define grDrawLine(a,b)				__grDrawLine(a,b)
	#define grAADrawLine(a,b)			__grDrawLine(a,b)
#endif

#if GLI_DEBUG_FX_POINTS
	#define grDrawPoint(a)				__grDrawPoint(a)
	#define grAADrawPoint(a)			__grDrawPoint(a)
#endif



/* GLI_3DFX_FIXGRLFBREADREGIONBUG: For fixing a bug in grLfbReadRegion */
#if GLI_3DFX_FIXGRLFBREADREGIONBUG
	#define grLfbReadRegion(a,b,c,d,e,f,g)	(*__grLfbReadRegion)(a,b,c,d,e,f,g)
#endif

/* GLI_GETENV: This will redirect getenv to our MCFG_getEnv() */
#if GLI_GETENV
	#define getenv(a)						MCFG_getEnv(a)
#endif

#if USE_OPENGL_MEMORY
	#include <stdlib.h>
	#include "glm.h"
	extern void *myAllocate(int size);	 
	extern void *myAllocateFX(int size);
#if GLI_DEBUG	     
	#define malloc(size) 		myAllocateFX(size)
#else
    #define malloc				glmMalloc
#endif

	#define free				glmFree
	#define realloc				glmRealloc
	#define calloc				glmCalloc
#endif

/* Use ARGB */
#define FXMESA_USE_ARGB			1

#if defined(FX_GLIDE3)
  #define FX_V2 				1
  #define FX_USE_PARGB			1 
#endif


/* Optimized for Q2 */
#define grSstIdle		(*__grSstIdle)


/*
 * For compiling with 3.0 version of Mesa:
 */
#ifndef GLI_MESA_3_0
	#define GLI_MESA_3_0						(MESA_MAJOR_VERSION == 3) && (MESA_MINOR_VERSION == 0)
#endif

/* #define __inline__		*/	