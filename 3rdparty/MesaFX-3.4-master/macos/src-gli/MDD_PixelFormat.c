/*
	File:		MDD_PixelFormat.c

	Contains:	PixelFormat choosing for MDD.

	Written by:	Miklos Fazekas

	Copyright:	Miklos Fazekas © 1999.  All rights reversed.
    See http://www.mesa3d.org/mac/ for more details.

	Change History (most recent first):

        <2+>     6/18/99    miklos  Updated for new GLI.
         <2>     4/14/99    miklos  Revision for AGL 2.0
         <1>      4/3/99    miklos  Initial revision for Mesa3.1.b2.
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

#include <gli.h>
#include "MDD.h"
#include "mgliContext.h"
#include "gliUtils.h"

/* ANSI */
#include <math.h>
#include <stdlib.h>

/* MacOS */
#include <MacMemory.h>

/* Mesa */
#include "context.h"

typedef struct TMDDColorModeInfo {
	GLint 		buffer_mode;		/* One of GLI_RGB_332, etc. */
	
	GLint		red_size;				/* The number of red bits */
	GLint		green_size;				/* The number of green bits */
	GLint		blue_size;				/* The number of blue bits */
	GLint		alpha_size;				/* The number of alpha bits */
	
	GLint		macos_depth;		/* The bit depth of the corresponding macos
								   	   device depth. ie: 16 for GLI_RGB_555, etc.
								   	   -1 if no such depth exists. (GLI_RGB_8555 for example) */
	GLint  		macos_alpha_depth;	/* The macos_depth if alpha support done by Mesa, -1 
								   	   for all RGBA formats, and macos_depth otherwise. */
	GLint		pixel_size;			/* The size of the pixel to store things. 
								   	   Can be macos_depth, or less */
}TMDDColorModeInfo;

#define 		kNumColorModes 	26

const TMDDColorModeInfo gColorModeTable[kNumColorModes] = 

{ {GLI_RGB8_BIT,		3,	3,	2,	0,	-1,	-1,  8},
  {GLI_RGB8_A8_BIT,		3,	3,	2,	8,	-1,	-1,	16},
  {GLI_BGR233_BIT,		3,  3,  2,  0,  -1, -1,  8},
  {GLI_BGR233_A8_BIT,	3,	3,	2,	8,	-1,	-1,	16},
  {GLI_RGB332_BIT,		3,	3,	2,	0,	-1,	-1,	 8},
  {GLI_RGB332_A8_BIT,	3,	3,	2,	8,	-1,	-1,	16},
  {GLI_RGB444_BIT,		4,	4,	4,	0,	-1,	-1, 16},
  {GLI_ARGB4444_BIT,	4,	4,	4,	4,	-1,	-1, 16},
  {GLI_RGB444_A8_BIT,	4,	4,	4,	8,	-1, -1, 24},
  {GLI_RGB555_BIT,		5,	5,	5,	0,	16, -1, 16},
  {GLI_ARGB1555_BIT,	5,	5,	5,	1,	16, 16,	16},
  {GLI_RGB555_A8_BIT,	5,	5,	5,	8,  -1, -1, 24},
  {GLI_RGB565_BIT,		5,	6,  5,  0,	-1,	-1, 16},
  {GLI_RGB565_A8_BIT,	5,	6,  5,  8,	-1,	-1, 24},
  {GLI_RGB888_BIT,		8,	8,	8,	0,	32,	-1,	32},
  {GLI_ARGB8888_BIT,	8,	8,	8,	0,	32,	32,	32},
  {GLI_RGB888_A8_BIT,	8,	8,	8,	0,	32,	-1,	32},
  {GLI_RGB101010_BIT,  10, 10, 10, 	0,	-1, -1, 32},
  {GLI_ARGB2101010_BIT,10, 10, 10, 	2,	-1, -1, 32},
  {GLI_RGB101010_A8_BIT,10,10, 10, 	8,	-1, -1, 48},
  {GLI_RGB121212_BIT,	12,12, 12,	0,	-1,	-1,	48},
  {GLI_ARGB12121212_BIT,	12,12, 12,	0,	-1,	-1,	48},
  {GLI_RGB161616_BIT,	16,16, 16,  0,	-1, -1, 64},	
  {GLI_ARGB16161616_BIT,  16, 16, 16, 16,	-1, -1, 64},   
  {GLI_INDEX8_BIT,		3,  3,  2,  0,	8,  8,  8},
  {GLI_INDEX16_BIT,		8,  8,  8,  0,	-1, -1, 16}
};


typedef struct TMDDDepthInfo {
	GLint			depth_mode;		/* One of GLI_0_BIT, etc. */
	GLint			num_of_bits;	/* The number of bits */
} TMDDDepthInfo;

#define  		kNumDepthModes 	11

const TMDDDepthInfo				gDepthTable[kNumDepthModes] = 
	{ {GLI_0_BIT, 	0},
	  {GLI_1_BIT, 	1},
	  {GLI_2_BIT, 	2},
	  {GLI_4_BIT, 	4},
	  {GLI_8_BIT, 	8},
	  {GLI_12_BIT, 12},
	  {GLI_16_BIT, 16},
	  {GLI_24_BIT, 24},
	  {GLI_32_BIT, 32},
	  {GLI_48_BIT, 48},
	  {GLI_64_BIT, 64} };


static
int GetDevicesMaxDepth(	const int 				devices_num,
						const GDHandle			devices[])
{
	int i;
	int max_depth = -1;
	
	MDDAssert(devices != NULL);
	MDDAssert(devices_num != 0);
	
	
	for (i = 0; i < devices_num; i++)
	{
		int current_depth = (*(*devices[i])->gdPMap)->pixelSize;
		
		if (current_depth > max_depth)
			max_depth = current_depth;
	}
	
	return max_depth;
}

  
static  
int CalcColorModeScore(const	int 			mode,
						const	TMDDEngineInfo	*info,
						const	GLIPixelFormat	*req,
						const	TMDDPolicy		policy)
{
	/* The requested pixel format should be RGBA. */
	if (req->rgba_mode != GL_TRUE)
	   return -1;
	
	/* The engine should support this mode */
	if (!(info->color_modes & gColorModeTable[mode].buffer_mode))
		return -1;
	
	/* If pixel_size is defined it should match */	
	if ((req->pixel_size != 0) && (req->pixel_size !=  gColorModeTable[mode].pixel_size))
		return -1;
	
	
	{
	int score = 100;
	int red 	= gColorModeTable[mode].red_size;
	int green 	= gColorModeTable[mode].green_size;
	int blue 	= gColorModeTable[mode].blue_size;
	int alpha 	= gColorModeTable[mode].alpha_size;
	int monitor_depth = GetDevicesMaxDepth(req->num_devices,req->devices);
		
	/* Check if it has an alpha buffer, if it was requiredŒ */
	if ((req->alpha_size > 0) && (alpha == 0))
		return -1;
			
	/* Score if the format match to the devices screen depth */
	if ((alpha > 0) && info->mesa_alpha_flag)
	{
		if (monitor_depth == gColorModeTable[mode].macos_alpha_depth)
			score += 12;
	}	
	else
	{
		if (monitor_depth == gColorModeTable[mode].pixel_size)
				score += 12;
	}
		
	if (policy & kMDDMinimumPolicy)
	{
		if (req->red_size > red)
			return -1;
		if (req->green_size > green)
			return -1;
		if (req->blue_size > blue)
			return -1;
		if (req->alpha_size > alpha)
			return -1;
	}
		
	if (policy & kMDDMaximumPolicy)
	{
		score += red;
		score += green;
		score += blue;
		if (req->alpha_size == 0)
			score -= 1*alpha;
		else
			score += 2*alpha;
	}
	else if (policy & kMDDClosestPolicy)
	{	
		if (req->alpha_size != 0)
			score -= 2*abs(alpha-req->alpha_size);
		else
			score -= 3*alpha;
		
		if (req->red_size != 0)
			score -= abs(red-req->red_size);
				
		if (req->green_size != 0)
			score -= abs(red-req->red_size);
				
		if (req->blue_size != 0)
			score -= abs(red-req->red_size);
	}
	else /* Default, the screen depth should be taken into account */
	{
		if (req->alpha_size == 0)
			score -= 3*alpha;
		
		if ((alpha > 0) && info->mesa_alpha_flag)
			score -= abs(gColorModeTable[mode].macos_alpha_depth-monitor_depth);
		else
			score -= abs(gColorModeTable[mode].pixel_size-monitor_depth);
	}
	return score;
	}
}

/* TODO: Indexed MODE!!! */
#ifdef INDEXED_TODO
	{
		/* Indexed */
		int score = 100;
		int monitor_depth = GetDevicesMaxDepth(req->num_devices,req->devices);
		int buffer_size = gColorModeTable[mode].buffer_size;
		
		if (policy & kMDDMinimumPolicy)
		{
			if (req->buffer_size > buffer_size)
				return -1;
		}
		
		if (policy & kMDDMaximumPolicy)
		{
			score += 3*buffer_size;
		}
		else if ((policy & kMDDClosestPolicy) && (req->buffer_size > 0))
 		{
 			if (req->buffer_size > buffer_size)
 				score -= 5*(req->buffer_size-buffer_size);
 			else
 				score -= 5*(buffer_size-req->buffer_size);
		}
		else /* Default, the screen depth should be taken into account */
		{
			if (monitor_depth > buffer_size)
				score -= 5*(monitor_depth-buffer_size);
			else
				score += 5*(buffer_size-monitor_depth);
		}
		
		return score;
	}
#endif 

/* 
 * Must be followed by a nonnegative buffer size specification.  A red accumulation
 * buffer that most closely matches the specified size is preferred.
 * Affected by MINIMUM, and MAXIMUM policy
 */
static
int CalcAccumModeScore(	const	int 			red_size,		/* The accum mode to score */
						const 	int				green_size,
						const	int				blue_size,
						const	int				alpha_size,
						const	int				req_red,		/* The requested accum_mode */
						const	int				req_green,
						const	int				req_blue,
						const	int				req_alpha,
						const	int				r,				/* The r,g,b,a of the pixel size */
						const	int				g,
						const	int				b,
						const	int				a,
						const	TMDDPolicy		policy)
{
	int score = 100;
	
	MDDAssert(red_size >= 0); MDDAssert(green_size >= 0); MDDAssert(blue_size >= 0);
	MDDAssert(alpha_size >= 0);
	MDDAssert(req_red >= 0); MDDAssert(req_green >= 0); MDDAssert(req_blue >= 0);
	MDDAssert(req_alpha >= 0);
	MDDAssert(r >= 0); MDDAssert(g >= 0); MDDAssert(b >= 0); MDDAssert(a >= 0);
	
	if (policy & kMDDMinimumPolicy)
	{
		if (red_size 	< req_red)
			return -1;
		if (green_size 	< req_green)
			return -1;
		if (blue_size 	< req_blue)
			return -1;
		if (alpha_size	< req_alpha)
			return -1;
	}
	
	/* Check if all defined values > 0 */
	if ((req_red != 0) && (red_size == 0))
		return -1;
	if ((req_green != 0) && (green_size == 0))
		return -1;
	if ((req_blue != 0) && (blue_size == 0))
		return -1;
	if ((req_alpha != 0) && (alpha_size == 0))
		return -1;
	
	/* TODO: Give bounus score if accum mode match with the r,g,b,a modes, as it's should be faster!!! */
	if (((red_size == r) && (green_size == g)) && ((blue_size == b) && (alpha_size == a)))
		score += 20;
	
	if (policy & kMDDMaximumPolicy)
	{
		if (req_red != 0)
			score += 2*red_size;
		else
			score -= red_size;
		
		if (req_green != 0)
			score += 2*green_size;
		else
			score -= green_size;
		
		if (req_blue != 0)
			score += 2*blue_size;
		else
			score -= blue_size;
			
		if (req_alpha != 0)
			score += 5*alpha_size;
		else
			score -= 2*alpha_size;
	}
	else	/* Default policy, closest */
	{
		score -= abs(red_size-req_red);
		score -= abs(green_size-req_green);
		score -= abs(blue_size-req_blue);
		score -= 2*abs(alpha_size-req_alpha);
	}
	
	return score;
}
static int CalcDepthBufferScore(const 	int 			depth_size,
								const	GLIPixelFormat	*req,
								const	TMDDPolicy		policy)
{
	int score = 100;
	
	if (req->depth_size == 0)	/* If no depth_buffer required return the smallest */
		return 64-depth_size;
	
	if ((policy & kMDDMinimumPolicy) && (depth_size < req->depth_size))
		return -1;
		
	if ((depth_size == 0) &&  (req->depth_size != 0))
		return -1;				/* We should return depth buffer if it was requested! */
	
	if (policy & kMDDMaximumPolicy)
	{
		score += 3*depth_size;
	}
	else  /* This is the default also + kMDDClosestPolicy */
	{
		score -= 3*abs(depth_size-req->depth_size);
	}
	return score;
}

/*
 * The MDDGetPixelFormat calls the driver GetPixelFormat method if defined.
 * Otherwise it select one pixel format from the engine info record.
 *
 * It could select any number of pixel formats, but all pixel formats should:
 * 	return GL_TRUE with PixelFormatMeetsTheInfos(engine)
 */
TMDDError	MDDGetPixelFormat(
					const TMDDEngine		*engine,			/* MDD engine */
					GLIPixelFormat			*requestedFormat,	/* Requested pixel-format, it contains device info also! */
					TMDDPolicy				policy,				/* Pixel-format choosing policy */
					GLIPixelFormat			**outFormats		/* One or more result pixel format */
					)						
{
	TMDDError status;
	
	MDDAssert(IsMDDEngine(engine));
	
	if ((*engine->getPixelFormat) != NULL)
	{
		status = (*engine->getPixelFormat)(engine,requestedFormat,policy,outFormats);
		
		if (status == kMDDNoError)
		{
			GLIPixelFormat *current;
			for (current = (*outFormats); 
				 current != NULL; current = current->next_pixel_format)
			{
				current->renderer_id 	= engine->info.renderer_id;
			}
		}
		
		return status;
	}
	else
	{
		GLIPixelFormat pix;
		int i;
		
		*outFormats = NULL;
		
		/*
		 ** OS Support bit fields
		 */
		
		if ((requestedFormat->os_support & GLI_OFFSCREEN_RENDERING_BIT) &&
			!(engine->info.os_support & GLI_OFFSCREEN_RENDERING_BIT))
			return kMDDNoError;
		
		if (!(requestedFormat->os_support & GLI_FULLSCREEN_RENDERING_BIT) &&
			(engine->info.os_support & GLI_FULLSCREEN_RENDERING_BIT))
			return kMDDNoError;
		
		if ((requestedFormat->os_support & GLI_MULTISCREEN_RENDERING_BIT) &&
			!(engine->info.os_support & GLI_MULTISCREEN_RENDERING_BIT))
			return kMDDNoError;
		
		if ((requestedFormat->os_support & GLI_BACKING_STORE_BIT) &&
			!(engine->info.os_support & GLI_BACKING_STORE_BIT))
			return kMDDNoError;
		
		if (((requestedFormat->os_support & GLI_WINDOW_RENDERING_BIT))
			&& !(engine->info.os_support & GLI_WINDOW_RENDERING_BIT))
			return kMDDNoError;
		
		pix.os_support = engine->info.os_support;

		/*
		 ** rgba_mode
		 */
		if ((requestedFormat->rgba_mode) && (engine->info.color_modes == 0))
			return kMDDNoError;
		
		if ((!requestedFormat->rgba_mode) && (engine->info.index_sizes == 0))
			return kMDDNoError;
		
		pix.rgba_mode = requestedFormat->rgba_mode;
		
		/*
		 ** double_buffer
		 */
		if ((!requestedFormat->double_buffer) && (!(engine->info.buffer_modes & GLI_SINGLEBUFFER_MODE)))
			return kMDDNoError;
		
		if ((requestedFormat->double_buffer) && (!(engine->info.buffer_modes & GLI_DOUBLEBUFFER_MODE)))
			return kMDDNoError;
		
		pix.double_buffer = requestedFormat->double_buffer;
		
		/*
		 ** stereo_mode
		 */
		if ((requestedFormat->stereo_mode) && (!(engine->info.buffer_modes & GLI_STEREO_MODE)))
			return kMDDNoError;
	
		pix.stereo_mode = requestedFormat->stereo_mode;
		
		/*
		 ** reserved
		 */
		pix.reserved 	= GL_FALSE;
		
		/*
		 ** level
		 */
		if ((requestedFormat->level < engine->info.min_level) ||
			(requestedFormat->level > engine->info.max_level) )
			return kMDDNoError;
		
		pix.level = requestedFormat->level;
		
		/*
		 * Select the best buffer format:
		 *
	 	 * pixel_size,buffer_size
		 * red_size,green_size,blue_size,alpha_size,
		 */
		if (pix.rgba_mode)
		{
			int best_mode = -1;
			int best_score = -1;
			
			for (i = 0; i < kNumColorModes; i++)
			{
				int current_score;
				
				current_score = CalcColorModeScore(i,&engine->info,requestedFormat,policy);
				if (current_score > best_score)
				{
					best_score = current_score;
					best_mode = i;
				}
			}
			
			if (best_score == -1)
				return kMDDNoError;
			
			pix.red_size 	= gColorModeTable[best_mode].red_size;
			pix.green_size 	= gColorModeTable[best_mode].green_size;
			pix.blue_size 	= gColorModeTable[best_mode].blue_size;
			pix.alpha_size 	= gColorModeTable[best_mode].alpha_size;
			/* pix.buffer_size = gColorModeTable[i].buffer_size; */
			pix.pixel_size  = gColorModeTable[best_mode].pixel_size;
			
			pix.buffer_size = 0; 
			
		}
		else /* !pix.rgba_mode */
 		{	/* TODO */
			{
				pix.red_size 	= 0;
				pix.green_size 	= 0;
				pix.blue_size 	= 0;
				pix.alpha_size 	= 0;
				/* pix.buffer_size = gColorModeTable[i].buffer_size;
				pix.pixel_size  = gColorModeTable[i].pixel_size;	*/
			}
		}
		
		/*
		 * aux_buffers: Pixel formats with the smallest number of auxiliary buffers that
         *              meets or exceeds the specified number are preferred.
		 */
		if ( engine->info.max_aux_buffers < requestedFormat->aux_buffers)
			return kMDDNoError;
			
		pix.aux_buffers = requestedFormat->aux_buffers;
		
		/*
		 * depth_size: Must be followed by a nonnegative depth buffer size specification.  A depth buffer
         * 			   that most closely matches the specified size is preferred. 
         *			   Depends from MINIMUM, and MAXIMUM policy.
		 */
		{ 
			int best_mode 	= -1;
			int best_score 	= -1;
			
			for (i = 0; i < kNumDepthModes; i++)
			{
				int current_mode = gDepthTable[i].num_of_bits;
				int current_score;
				
				if (gDepthTable[i].depth_mode & (engine->info).depth_sizes)
				{
					current_score = CalcDepthBufferScore(current_mode,requestedFormat,policy);
					if (current_score > best_score)
					{
						best_score = current_score;
						best_mode = current_mode;
					}
				}
			}
			
			if (best_score == -1)
				return kMDDNoError;
			
			pix.depth_size = best_mode;
		}
		
		/*
		 * stencil_size: Must be followed by a nonnegative integer that indicates the desired number of
         *     			 stencil bitplanes  The smallest stencil buffer of at least the specified size is
         *               preferred. Doesn't depends from policies 
		 */
		{
			int found = 0;
			int i = 0;
			
			while (!found && i < kNumDepthModes)
			{
				if ((gDepthTable[i].depth_mode & (engine->info).stencil_sizes) &&
					(gDepthTable[i].num_of_bits >= requestedFormat->stencil_size))
					found = 1;
				else
					i++;
			}
			
			if (!found)
				return kMDDNoError;
				
			pix.stencil_size = gDepthTable[i].num_of_bits;
			
		}
		
		/*
		 * accum_red_size,accum_green_size,accum_blue_size,accum_alpha_size: 
		 *				   Must be followed by a nonnegative buffer size specification.  A red accumulation
         *                 buffer that most closely matches the specified size is preferred.
		 */
		if (pix.rgba_mode)
		{
			
			/* First calc the score for 0,0,0,0: */
			int i;
			int best_red 	= 0;
			int best_green 	= 0;
			int best_blue	= 0;
			int best_alpha 	= 0;
			int best_score	= CalcAccumModeScore(best_red,best_green,best_blue,best_alpha,
												  requestedFormat->accum_red_size,requestedFormat->accum_green_size,
												  requestedFormat->accum_blue_size,requestedFormat->accum_alpha_size,
												  pix.red_size,pix.green_size,pix.blue_size,pix.alpha_size,
												  policy);
			for (i = 0; i < kNumColorModes; i++)
			{
				int red_size 	= gColorModeTable[i].red_size;
				int green_size 	= gColorModeTable[i].green_size;
				int blue_size 	= gColorModeTable[i].blue_size;
				int alpha_size 	= gColorModeTable[i].alpha_size;
				if (gColorModeTable[i].buffer_mode & engine->info.accum_modes)
				{
					int current_score = CalcAccumModeScore(red_size,green_size,blue_size,alpha_size,
												  requestedFormat->accum_red_size,requestedFormat->accum_green_size,
												  requestedFormat->accum_blue_size,requestedFormat->accum_alpha_size,
												  pix.red_size,pix.green_size,pix.blue_size,pix.alpha_size,
												  policy);
					
					if (current_score > best_score)
					{
						best_score 		= current_score;
						best_red 		= red_size;
						best_green		= green_size;
						best_blue		= blue_size;
						best_alpha		= alpha_size;
					}
				}
			}
			if (best_score == -1)
				return kMDDNoError;
			else
			{
				pix.accum_red_size		= best_red;
				pix.accum_green_size 	= best_green;
				pix.accum_blue_size 	= best_blue;
				pix.accum_alpha_size 	= best_alpha;
			}	
		}
		else	/* !pix.rgba_mode */
		{
			pix.accum_red_size		= 0;
			pix.accum_green_size	= 0;
			pix.accum_blue_size		= 0;
			pix.accum_alpha_size	= 0;
		}
		
		/*
		** Compliant bits
		*/
		if (requestedFormat->compliance != (requestedFormat->compliance & engine->info.compliance))
			return kMDDNoError;
		
		pix.compliance = engine->info.compliance;
		/*
		** Performance bits
		*/
		pix.msb_performance	= engine->info.msb_performance;
		pix.lsb_performance	= engine->info.lsb_performance;
		
		/*
		** AGL 2.0 Features:
		*/ 
		pix.accelerated = engine->info.accelerated;
		pix.compliant	= engine->info.compliant;
		/* pix.video_memory = engine->info.video_memory;
		pix.texture_memory = engine->info.texture_memory; */
		pix.mp_safe = GL_FALSE;
		pix.robust = GL_FALSE;
		pix.no_recovery = GL_TRUE;
		
		/*
		** Device info
		*/
		MDDAssert(requestedFormat->num_devices < (GLI_MAX_DEVICES-1));
		pix.num_devices = requestedFormat->num_devices;
		for (i = 0; i < requestedFormat->num_devices; i++)
		{
			pix.devices[i] = requestedFormat->devices[i];
		}
		pix.renderer_id = engine->info.renderer_id;
		
		/*
		** Next pixel format:
		*/
		pix.next_pixel_format = NULL;
		
		/*
		 * Return the result.
		 */
		
		*outFormats = (GLIPixelFormat*)NewPtr(sizeof(GLIPixelFormat));
		if (*outFormats == NULL)
		{
			return kMDDOutOfMemory;
		}
		
		
		**outFormats = pix;
		return kMDDNoError;
	} /* if (*engine->getPixelFormat) != NULL) */
}

/****************************************************************************
*
* Function:     MDDGetMesaVisual
* Parameters:   engine    	- the MDD rendering engine.
*				pix			- the minimal (requested pixel format)
* Returns:		A mesa visual wich meets or exceed the pixel format pix.
*
* Description:  Allocates a new mesa visual. If there is a getVisualInfo
*				function registered for the MDD driver then calls it.
*				Othervise selects one visual based on the infos supplied by
*				the TMDDEngineInfo.
*
****************************************************************************/
struct gl_visual	*MDDGetMesaVisual(
							const TMDDEngine 		*engine,	/* MDD engine */
							GLIPixelFormat 			*pix)		/* Requested pixel format */
{
	TMDDVisualInfo vis;
	GLvisual *result;
	
	MDDAssert(IsMDDEngine(engine));
	MDDAssert(pix != NULL);
	
	if ( engine->getVisualInfo )
	{	
		(engine->getVisualInfo)(engine,pix,&vis);
	}
	else
	{
		vis.stereo_flag = GL_FALSE;	/* TODO */
		
		vis.rgb_flag = pix->rgba_mode;
		if (engine->info.mesa_alpha_flag)
			vis.alpha_flag = (pix->alpha_size != 0);
		else
			vis.alpha_flag = GL_FALSE;
		vis.db_flag = pix->double_buffer;
		if (engine->info.mesa_depth_flag)
			vis.depth_bits = pix->depth_size;
		else
			vis.depth_bits = (pix->depth_size == 0) ? 0 : DEPTH_BITS;
			
		vis.stencil_bits = pix->stencil_size;
		vis.accum_bits = pix->accum_red_size;
		if (vis.accum_bits < pix->accum_green_size)
			vis.accum_bits = pix->accum_green_size;
		if (vis.accum_bits < pix->accum_blue_size)
			vis.accum_bits = pix->accum_blue_size;
		if (vis.accum_bits < pix->accum_alpha_size)
			vis.accum_bits = pix->accum_alpha_size;
			
		vis.index_bits = pix->buffer_size;
			
		vis.red_bits 	= pix->red_size;	
		vis.green_bits 	= pix->green_size;	
		vis.blue_bits 	= pix->blue_size;	
		vis.alpha_bits 	= pix->alpha_size;	
	}
	
	result = gl_create_visual(	vis.rgb_flag,
								vis.alpha_flag,
								vis.db_flag,
								vis.stereo_flag,
								vis.depth_bits,
								vis.stencil_bits,
								vis.accum_bits,
								vis.index_bits,
								vis.red_bits,
								vis.green_bits,
								vis.blue_bits,
								vis.alpha_bits );
	
	return result;
}

void				MDDDeleteMesaVisual(
							GLvisual 			*inVisual)
{
	MDDAssert(inVisual != NULL);
	
	gl_destroy_visual(inVisual);
}

/****************************************************************************
*
* Function:     MDDUMesaGetEngineInfo
* Parameters:   outInfo    	- returns the default info parameters.
* Returns:		-
*
* Description:  Returns the default MDDEngine info structure, MDD drivers
*				should fill out the mesa_alpha_flag,mesa_depth_flag,
*				color_modes, and HW drivers also should fill the depth_sizes,
*				... structures.
*
****************************************************************************/
void MDDUMesaGetEngineInfo(TMDDEngineInfo	*outInfo)
{
	MDDAssert(outInfo != NULL);
	
	/*
	** Renderer id
	*/
	outInfo->renderer_id 		= kMesaID;
	/*
	** OS support feature bits
	*/
	outInfo->os_support 		= GLI_NONE;	/* DEPEND'S ON THE DRIVER */
	/*
	** Buffer modes
	*/
	outInfo->buffer_modes		= GLI_SINGLEBUFFER_MODE | GLI_DOUBLEBUFFER_MODE;
	/*
	** Buffer levels
	*/
	outInfo->min_level			= 0;
	outInfo->max_level			= 0;
	/*
	** Buffer configurations
	*/
	outInfo->index_sizes		= GLI_NONE;	/* DEPEND'S ON THE DRIVER */
	outInfo->color_modes		= GLI_NONE; /* DEPEND'S ON THE DRIVER */
	outInfo->accum_modes		= (ACCUM_BITS == 16) ? GLI_RGB161616_BIT|GLI_ARGB16161616_BIT : GLI_ARGB8888_BIT|GLI_RGB888_BIT;
	outInfo->depth_sizes		= GLI_0_BIT;
	outInfo->depth_sizes 		|= (DEPTH_BITS == 16) ? GLI_16_BIT : GLI_32_BIT;
	outInfo->stencil_sizes 		= GLI_0_BIT;
	outInfo->stencil_sizes 		|= (STENCIL_BITS == 8) ? GLI_8_BIT : GLI_16_BIT;	
	outInfo->max_aux_buffers 	= 8;
	/*
	** Compliant bits
	*/
	outInfo->compliance 		= GLI_ALL_OPTIONS -(GLI_AA_LINE | GLI_AA_POLYGON);
	/*
	** Performance bits
	*/
	outInfo->msb_performance	= GLI_NONE;
	outInfo->lsb_performance  	= GLI_NONE;
	/*
	** Algorithms
	*/
	outInfo->texture_method		= GLI_FASTEST;
	outInfo->point_aa_method	= GLI_FASTEST;
	outInfo->line_aa_method		= GLI_NONE;
	outInfo->polygon_aa_method	= GLI_NONE;
	outInfo->fog_method			= GLI_NICEST | GLI_FASTEST;

	/*
	 * AGL 1.2 Attributes:
	 */

	outInfo->accelerated		= GL_FALSE;
	outInfo->compliant			= GL_FALSE;
	
	outInfo->texture_memory		= 0;
	outInfo->video_memory		= 0;
	/*
	 * Extended attributes:
	 */
	outInfo->renderer_id			= -1;
	
	outInfo->mesa_alpha_flag	= GL_FALSE;
	outInfo->mesa_depth_flag	= GL_FALSE;
}