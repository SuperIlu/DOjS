/*
	File:		maglPixelFormat.c

	Contains:	Pixel format selection routines - AGL interface for GLI.

	Written by:	Mikl—s Fazekas

	Copyright:	Copyright(C) 1995-99 Miklos Fazekas
				E-Mail: boga@valerie.inf.elte.hu
				WWW: http://www.mesa3d.org/mac/
				
				Part of the Mesa 3D Graphics Library for MacOS.


	Change History (most recent first):

        <2+>      7/8/99    miklos  Revision for new GLI headers.
         <2>     4/10/99    miklos  Fixed errors in PixelFormat selection.
         <1>      4/8/99    miklos  Revision for AGL 2.0
        <10>     4/19/98    miklos  Fixed a bug in aglChoosePixelFormat.
         <9>     4/19/98    miklos  Adopted new error handler functions.
         <8>     4/13/98    miklos  Fixed compiler warnings.
         <7>      4/7/98    miklos  Revision for Mesa 3.0b4
         <6>      4/6/98    miklos  Fixed fullscreen support.
         <5>      3/1/98    miklos  Added error chechk after gliDestroyPixelFormat.
         <4>     1/29/98    miklos  Fixed a bug in parsing AGL_ALL_RENDERS argument.
         <3>      1/4/98    miklos  Implemented CalcFPScore
         <2>    12/27/97    miklos  Inital Revision
         <1>    11/22/97    miklos  Initial Revision
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

#include "maglConfig.h"
#include "maglMemory.h"
#include "maglError.h"
#include "maglRenderers.h"
#include "maglValidate.h"

/* OpenGL */
#include <agl.h>
#include <aglContext.h>
/* GLI */
#include <gliMesa.h>

/*GLIutils */
#include "gliUtils.h"

/* MacOS */
#include <Types.h>
#include <Memory.h>



/***********************************************************************************************
 *
 * Contsants
 *
 ***********************************************************************************************/

/* The maximum number of allowed attributes */ 
#define		kMAGL_MaxAttributesNum			256
#define 	kMAGL_MaxDevicesNum				20


/* Pixel format scoring constants */
#define		kMAGL_AccelerationScore				100

/***********************************************************************************************
 *
 * IsValidGDev: Return GL_FALSE, if the graphics device inDevice is not a valid device.
 *
 ***********************************************************************************************/
static GLboolean	
IsValidGDev(GDHandle inDevice)
{
	return (inDevice != NULL);
}

/***********************************************************************************************
 *
 * IsValidPixelFormat: Return GL_FALSE, if the inPixelFormat is not a valid 
 *					   GLIPixelFormat/AGLPixelFormat.
 *
 ***********************************************************************************************/
GLboolean	
IsValidPixelFormat(AGLPixelFormat inPixelFormat)
{
	return (inPixelFormat != NULL);
}

/***********************************************************************************************
 *
 * IsDeviceOfPixelFormat: Return GL_TRUE, if the pixel format inPixelFormat supports the 
 *						  graphics device inDevice
 *
 ***********************************************************************************************/
static GLboolean 
IsDeviceOfPixelFormat(
					GLIPixelFormat 	*inPixelFormat,
					GDHandle		inDevice)
{
	int i;
	
	for (i = 0; i < inPixelFormat->num_devices; i++)
	{
		if (inPixelFormat->devices[i].gdevice == inDevice)
			return GL_TRUE;
	}
	
	return GL_FALSE;
}

/***********************************************************************************************
 *
 * GetFirstGDevice (internal)
 *
 * Description: Returns the first graphics device on the system.
 *
 ***********************************************************************************************/
static GDHandle
GetFirstGDevice()
{
	GDHandle gdNthDevice;
	
	gdNthDevice = GetDeviceList();
	
	while (gdNthDevice != NULL)
	{
		if (TestDeviceAttribute(gdNthDevice,screenDevice))
		{
			return gdNthDevice;
		}
		
		GetNextDevice(gdNthDevice);
	}
	
	return NULL;	/* No devices at all???? */
}
/***********************************************************************************************
 *
 * GetNextGDevice (internal)
 *
 * Description: Returns the next graphics device on the system.
 *
 ***********************************************************************************************/
static GDHandle
GetNextGDevice(GDHandle inDevice)
{
	GDHandle gdNthDevice;
	
	gdNthDevice = GetNextDevice(inDevice);
	
	while (gdNthDevice != NULL)
	{
		if (TestDeviceAttribute(gdNthDevice,screenDevice))
		{
			return gdNthDevice;
		}
		
		GetNextDevice(gdNthDevice);
	}
	
	return NULL;	
}



/***********************************************************************************************
 *
 * CalcPFScore (internal)
 *
 * Parameters:	inAttribute	- Specifies the attribute
 *				inReqValue	- Specifies the requested value
 *				inValue		- Specifies the value returned by the pixel-format.
 *				inPolicy	- Specifies the requested policy
 *				
 ************************************************************************************************/
static int
CalcScoreForAttribute(	int inAttribute,
						int	inValue,
						int inReqValue,
						int inPolicy)
{
#pragma unused(inAttribute)
	if (inPolicy == AGL_MINIMUM_POLICY)
	{	
		if (inValue < inReqValue)
			return -1;
		else
			return 0;
	}
	
	if ((inReqValue != 0) && (inValue == 0))
		return -1;
	/* TODO */
	return 0;
}
/***********************************************************************************************
 *
 * CalcPFScore (internal)
 *
 * Parameters:	inPixelFormat	- Specifies the pixel format.
 *				inRendererIDS	- Specifies the allowed renderer id's.
 *				inRenderersNum	- Specifies the number of rendererer id's in the inRendererIDS.
 *				inAttribs		- Specifies a list of Boolean attributes and integer, GLI_ATTRIBUTRES 
 *                            	  attribute/value pairs. The last attribute must be GLI_NONE.
 *				inDevices		- Specifies the list of graphics devices which one pixel format must support.
 *				inDevicesNum	- Specifies the number of graphics devices in the inDevices.
 * Returns:						- an integer representing the good'nes of the pixel format, for the 
 *								  requested attributes, etc. -1 is returned if the pixel-format doesn't
 *								  meet with the specifications.
 * Description:	Returns the score of a pixel format.
 *
 ***********************************************************************************************/
static int 
CalcPFScore(	GLIPixelFormat 		*inPixelFormat,			/* The pixel-format */
				int					inAttribs[],			/* The requested pix-fmt */
				GLint				inRendererIDS[],		/* Extended info about renderer's */
				GLint				inRendererNum,
				GLint				inPolicy,				/* Extended info about the pixel-policy */
				GLbitfield			inCompliance,			/* Minimum level of compliance */
				GLbitfield			inPerformance,			/* Optimize the pixel format for this flag */
				GDHandle			inDevices[],			/* The number of devices to support */
				int					inDeviceNum)			/* The devices */
{
#pragma unused(inPerformance)
	GLboolean 			attribPresent[ GLI_MP_SAFE+100];
	TGLIUtilsColorMode	colorMode;
	int 				result;
	int 				i,j;
	
	if (!_gliUtilsGetColorMode(inPixelFormat->buffer_mode,&colorMode))
		return -1;
		
	result = 0;
	for (j = 0; j < GLI_MP_SAFE+100; j++)
	{
		attribPresent[j] = GL_FALSE;
	}
	/*
	 * Check if the inPixelFormat meets the specifications:
	 */
	i = 0;
	while (inAttribs[i] != GLI_NONE)
	{
		attribPresent[inAttribs[i]] = GL_TRUE;
		switch (inAttribs[i++])
		{
				case GLI_ALL_RENDERERS:
					/* SKIP */
					break;
				case GLI_BUFFER_SIZE:
					{
						int score;
						
						score = CalcScoreForAttribute(GLI_BUFFER_SIZE,inAttribs[i++],colorMode.pixel_size,inPolicy);
						if (score < 0)
							return -1;
						result += score; 
					}
					break;
				case GLI_LEVEL:
					if (inPixelFormat->level != inAttribs[i++])
						return -1;
					break;
				case GLI_RGBA:
					if (colorMode.rgba != GL_TRUE)
						return -1;
					break;
				case GLI_DOUBLEBUFFER:
					if (!(inPixelFormat->buffer_mode & GLI_DOUBLEBUFFER_BIT))
						return -1;
					break;
				case GLI_STEREO:
					if (!(inPixelFormat->buffer_mode & GLI_STEREOSCOPIC_BIT))
						return -1;
					break;
				case GLI_AUX_BUFFERS:
					{
						int score = CalcScoreForAttribute(GLI_AUX_BUFFERS,inPixelFormat->aux_buffers,inAttribs[i++],inPolicy);
						if (score < 0)
							return -1;
						result += score;
					break; }
				case GLI_RED_SIZE:
					{	int score = CalcScoreForAttribute(GLI_RED_SIZE,colorMode.red_size,inAttribs[i++],inPolicy);
						if (score < 0)
							return -1;
						result += score;
					break; }
				case GLI_GREEN_SIZE:
					{	int score = CalcScoreForAttribute(GLI_GREEN_SIZE,colorMode.green_size,inAttribs[i++],inPolicy);
						if (score < 0)
							return -1;
						result += score;
					break; }
				case GLI_BLUE_SIZE:
					{	int score = CalcScoreForAttribute(GLI_BLUE_SIZE,colorMode.blue_size,inAttribs[i++],inPolicy);
						if (score < 0)
							return -1;
						result += score;
					break; }
				case GLI_ALPHA_SIZE:
					{	int score = CalcScoreForAttribute(GLI_ALPHA_SIZE,colorMode.alpha_size,inAttribs[i++],inPolicy);
						if (score < 0)
							return -1;
						result += score;
					break; }
				case GLI_DEPTH_SIZE:
					{	int score = CalcScoreForAttribute(GLI_DEPTH_SIZE,_gliUtilsGetBitDepthSize(inPixelFormat->depth_mode),inAttribs[i++],inPolicy);
						if (score < 0)
							return -1;
						result += score;
					break; }
				case GLI_STENCIL_SIZE:
					{	int score = CalcScoreForAttribute(GLI_STENCIL_SIZE,_gliUtilsGetBitDepthSize(inPixelFormat->stencil_mode),inAttribs[i++],inPolicy);
						if (score < 0)
							return -1;
						result += score;
					break; }
				case GLI_ACCUM_RED_SIZE:
					{	int score = CalcScoreForAttribute(GLI_ACCUM_RED_SIZE,_gliUtilsGetBitDepthSize(inPixelFormat->accum_mode),inAttribs[i++],inPolicy);
						if (score < 0)
							return -1;
						result += score;
					break; }
				case GLI_ACCUM_GREEN_SIZE:
					{	int score = CalcScoreForAttribute(GLI_ACCUM_GREEN_SIZE,_gliUtilsGetBitDepthSize(inPixelFormat->accum_mode),inAttribs[i++],inPolicy);
						if (score < 0)
							return -1;
						result += score;
					break; }
				case GLI_ACCUM_BLUE_SIZE:
					{	int score = CalcScoreForAttribute(GLI_ACCUM_BLUE_SIZE,_gliUtilsGetBitDepthSize(inPixelFormat->accum_mode),inAttribs[i++],inPolicy);
						if (score < 0)
							return -1;
						result += score;
					break; }
				case GLI_ACCUM_ALPHA_SIZE:
					{	int score = CalcScoreForAttribute(GLI_ACCUM_ALPHA_SIZE,_gliUtilsGetBitDepthSize(inPixelFormat->accum_mode),inAttribs[i++],inPolicy);
						if (score < 0)
							return -1;
						result += score;
					break; }
				/* Extended attributes */
				case GLI_OFFSCREEN:
					if (!(inPixelFormat->os_support & GLI_OFFSCREEN_BIT))
						return -1;
					break;
				case GLI_FULLSCREEN:
					if (!(inPixelFormat->os_support & GLI_FULLSCREEN_BIT))
						return -1;
					break;
				case GLI_MINIMUM_POLICY:
						/* SKIP */
					break;
				case GLI_MAXIMUM_POLICY:
						/* SKIP */
					break;
				case GLI_CLOSEST_POLICY:
						/* SKIP */
					break;
				case GLI_PIXEL_SIZE:
						if (colorMode.pixel_size != inAttribs[i++])
							return -1;
					break;
				case GLI_ACCELERATED:
						if (!(inPixelFormat->os_support & GLI_ACCELERATED_BIT))
							return -1;
						break;
				case GLI_BACKING_STORE:
						if (!(inPixelFormat->os_support & GLI_BACKING_STORE_BIT))
							return -1;
						break;
				case GLI_MP_SAFE:
						if (!(inPixelFormat->os_support & GLI_MP_SAFE))
							return -1;
						break;	
				default:
					MAGL_FatalError(NULL,("CalcPFScore: Internal error in attrib %d\n",inAttribs[i-1]));
					return -1;
		}
	}
	
	if (!attribPresent[GLI_ALL_RENDERERS] && !(inPixelFormat->os_support & GLI_COMPLIANT_BIT))
		return -1;
	
	if (!attribPresent[GLI_RGBA] && colorMode.rgba)
		return -1;
		
	if (!attribPresent[GLI_LEVEL] && (inPixelFormat->level != 0))
		return -1;
		
	if (!attribPresent[GLI_DOUBLEBUFFER] && (inPixelFormat->buffer_mode & GLI_DOUBLEBUFFER_BIT))
		return -1;
	
	if (!attribPresent[GLI_STEREO] && (inPixelFormat->buffer_mode & GLI_STEREOSCOPIC_BIT))
		return -1;
		
	/* Score the prerformance */
	/* TODO: Make it better */
	if ((inPixelFormat->os_support & GLI_ACCELERATED_BIT))
	{
		result += kMAGL_AccelerationScore;
	}
	
	/*
	 * See if the pixel-format renderer id is good
	 */
	if (inRendererNum != 0)
	{
		GLboolean validRenderer;
		
		validRenderer = GL_FALSE;
		for (i = 0; i < inRendererNum; i++)
		{
			if (inRendererIDS[i] == inPixelFormat->renderer_id)
				validRenderer = GL_TRUE;
		}
		
		if (!validRenderer)
			return GL_FALSE;
	
	}
	
	/*
	 * See if the pixel format supports all devices
	 */
	for (i = 0; i < inDeviceNum; i++)
	{
		GLboolean supported = GL_FALSE;
		
		for (j = 0; j < inPixelFormat->num_devices; j++)
		{
			if (inDevices[i] == inPixelFormat->devices[j].gdevice)
				supported = GL_TRUE;
		}
		
		if (!supported)
			return -1;	/* Pixel-format does'nt supports this/these device */
	}
	
	return result;
}	



/***********************************************************************************************
 *
 * aglChoosePixelFormat (agl-api call)
 *
 * Parameters:	inDevices		- Specifies the array of graphics devices.
 *				inDeviceNum		- Specifies the number of graphics devices in inDevices.
 *				inAttribs		- Specifies a list of Boolean attributes and integer 
 *                            	  attribute/value pairs. The last attribute must be AGL_NONE.
 * Returns:						- a pointer to data describing a pixel format that is
 *             					  supported by all the graphics devices in inDevices and best 
 *								  meets the specification defined inAttribs.
 *								  If inDevices and inDeviceNum are set to NULL and zero, 
 *								  respectively, aglChoosePixelFormat will return a pixel format 
 *								  that is supported by all graphics devices on the system.
 * Description:	Select a pixel format to support specified graphics devices and buffer attributes.  
 *				The Boolean AGL attributes of the returned format will match the specified  
 *           	values, and the integer AGL attributes will be as close to the specified values 
 *           	as can  be provided by the system.  If no conforming pixel format exists, NULL is   
 *           	returned. To free  the data returned by this function, use DisposePtr. 
 *				All Boolean AGL attributes default to GL_FALSE. All integer AGL attributes
 *				default to zero.  Default specifications are superseded by attributes included 
 *				in inAttribs .  Boolean attributes included in attribs  are understood to be 
 *				GL_TRUE. Integer attributes are followed immediately by the corresponding 
 *				desired value.  The list must be terminated with AGL_NONE.
 *              Use aglNextPixelFormat to access subsequent pixel formats in an 
 *				AGLPixelFormat structure.
 * Errors:		aglChoosePixelFormat returns NULL if it fails for any reason.
 *              AGL_BAD_ATTRIBUTE	is generated if an undefined AGL attribute is encountered
 *              in attribs.
 *				AGL_BAD_VALUE	is generated if ndev is zero or the AGL_OFFSCREEN
 *         		attribute is specified and gdevs  is not NULL.
 *				AGL_BAD_GDEV	is generated if ndev is nonzero and gdevs is not a valid
 *            	graphics device handle.
 *				AGL_GL_ERROR	is generated if an OpenGL renderer fails to initialize.
 *				(NOT IN CONIX)
 *				AGL_BAD_ALLOC is generated if there is not enough memory, for allocating 
 *				the AGLPixelFormat list. 
 *				AGL_BAD_VALUE 	is generated if an integer value other than is -1.
 *
 ***********************************************************************************************/
 /* 
  * If gdev specifies more than one graphics device (or is NULL on multi-screen system)
  * aglChoosePixelFormat attempts to find a renderer or renderers to support all the devices with
  * one AGL context. If a single hardware accelerated renderer is found that can support the requested
  * pixel format on all devices, this renderer is chosen. If accelerated renderers are found that can
  * support only a subset of the devices, then pixel formats from multiple renderers are chosen. Thus, a
  * hardware accelerated renderer is used when the current graphics port is entirely displayed on the
  * device it supports and a software renderer is used when the graphics ports overlaps a device that is
  * not supported by the hardware renderers.
  */
AGLPixelFormat
aglChoosePixelFormat(
						const GDHandle inGDevices[],		/* The array of graphics devices */
						int inDevicesNum,					/* The number of graphics devices in inDevices */
						const int inAttribs[])		/* The list of boolean and integer attribute/value pairs */
{
	/* Extended attibutes */
	GLint		extPolicy = AGL_CLOSEST_POLICY;				/* Masks of kMAGL_Y */
						   				   	
	GLint		extRendererIDS[10];	/* The list of requested renderers */
	GLint		extRendererIDNum = 0;					/* The length of requested render id's */
	
	GLbitfield	extCompliance;						/* Compliance bitfield */	
	GLbitfield	extPerformance;						/* Performance bitfield */
	GLboolean	extSingleRenderer;					/* Whatewer single renderer should support all screens or not */
	GLint		extPixelSize;						/* PixelSize */
	GLboolean	extOffscreen;						/* Offscreen-Support required */
	GLboolean	extFullscreen;						/* Fullscreen-Support required */
	GLboolean	extAccelerated;						/* Acceleration required */
	
	GDHandle	devices[20];
	GLint		devicesNum 	= 0;
	
	
	GLIPixelFormat bestPFForAll;				/* The best pixel format for all devices */
	int			   bestPFForAllScore = -1;
	GLIPixelFormat bestPF[20];		/* The best pixel format for each device */
	int			   bestPFScore[20];
	
	int				attribCmd;
	int				attribCopy[kMAGL_MaxAttributesNum+2];
	int				attribNum = 0;
	int 			i,j;
	int				error;
	
	/* You should enable Mesa before using it */
	if (!is_mesa_enabled)
		AlertUser();
	if (!is_mesa_enabled)
		return NULL;
		
	/* Copy and process attributes, i: source, j: dest */
	extOffscreen 		= GL_FALSE;
	extSingleRenderer 	= GL_FALSE;
	extPixelSize 		= -1;
	extRendererIDNum 	= 0;
	extCompliance 		= GLI_NONE;
	extPerformance		= GLI_NONE;
	extFullscreen		= GL_FALSE;
	extAccelerated		= GL_FALSE;
	extOffscreen		= GL_FALSE;
	error	 			= AGL_NONE;
	
	
	MAGL_CHECK_STATE();
	
	i = 0; j = 0;
	while ( ((inAttribs[i] != AGL_NONE) && (error == AGL_NONE)) 
			&& (i < kMAGL_MaxAttributesNum)
			)
	{
		attribCmd = inAttribs[i++];
		switch (attribCmd) 
		{
			case AGL_BUFFER_SIZE:
			/*	
			 *  Must be followed by a nonnegative integer that indicates the desired color index buffer
			 *	size. The smallest color index buffer of at least the specified size is preferred. Ignored if
			 *	AGL_RGBA is asserted. 
			 */
				attribCopy[j++] = GLI_BUFFER_SIZE;
				attribCopy[j++] = inAttribs[i++];
				if (attribCopy[i-1] < 0)
					error = AGL_BAD_VALUE;
				break;
			case AGL_LEVEL:
			/*
			 * Must be followed by an integer buffer-level specification. This specification is honored
			 * (exactly. Buffer level zero corresponds to the default frame buffer of the display. Buffer
			 * level one is the first overlay frame buffer, level two the second overlay frame buffer, and
			 * so on. Negative buffer levels correspond to underlay frame buffers.
			 */
				attribCopy[j++] = GLI_LEVEL;
				attribCopy[j++] = inAttribs[i++];
				break;
			case AGL_RGBA:
			/*
			 * If present, only RGBA pixel formats are considered. Otherwise, only color index pixel
			 * formats are considered.
			 */
				attribCopy[j++] = GLI_RGBA;
				/* Boolean attribute */
				break;
			case AGL_DOUBLEBUFFER:
				attribCopy[j++] = GLI_DOUBLEBUFFER;
				/* Boolean attribute */
				break;
			case AGL_STEREO:
				attribCopy[j++] = GLI_STEREO;
				/* Boolean attribute */
				break;
			case AGL_AUX_BUFFERS:
			/*
			 * Must be followed by a nonnegative integer that indicates the desired number of auxiliary
			 * buffers. Pixel formats with the smallest number of auxiliary buffers that meets or
			 * exceeds the specified number are preferred.
			 */
				attribCopy[j++] = GLI_AUX_BUFFERS;
				attribCopy[j++] = inAttribs[i++];
				if (inAttribs[i-1] < 0)
					error = AGL_BAD_VALUE;
				break;

			case AGL_RED_SIZE:
				attribCopy[j++] = GLI_RED_SIZE;
				attribCopy[j++] = inAttribs[i++];
				if (inAttribs[i-1] < 0)
					error = AGL_BAD_VALUE;
				break;
			case AGL_GREEN_SIZE:
				attribCopy[j++] = GLI_GREEN_SIZE;
				attribCopy[j++] = inAttribs[i++];
				if (inAttribs[i-1] < 0)
					error = AGL_BAD_VALUE;
				break;
			case AGL_BLUE_SIZE:
				attribCopy[j++] = GLI_BLUE_SIZE;
				attribCopy[j++] = inAttribs[i++];
				if (inAttribs[i-1] < 0)
					error = AGL_BAD_VALUE;
				break;
			case AGL_ALPHA_SIZE:
				attribCopy[j++] = GLI_ALPHA_SIZE;
				attribCopy[j++] = inAttribs[i++];
				if (inAttribs[i-1] < 0)
					error = AGL_BAD_VALUE;
				break;
			case AGL_DEPTH_SIZE:
				attribCopy[j++] = GLI_DEPTH_SIZE;
				attribCopy[j++] = inAttribs[i++];
				if (inAttribs[i-1] < 0)
					error = AGL_BAD_VALUE;
				break;
			case AGL_STENCIL_SIZE:
				attribCopy[j++] = GLI_STENCIL_SIZE;
				attribCopy[j++] = inAttribs[i++];
				if (inAttribs[i-1] < 0)
					error = AGL_BAD_VALUE;
				break;
			case AGL_ACCUM_RED_SIZE:
				attribCopy[j++] = GLI_ACCUM_RED_SIZE;
				attribCopy[j++] = inAttribs[i++];
				if (inAttribs[i-1] < 0)
					error = AGL_BAD_VALUE;
				break;
			case AGL_ACCUM_GREEN_SIZE:
				attribCopy[j++] = GLI_ACCUM_GREEN_SIZE;
				attribCopy[j++] = inAttribs[i++];
				if (inAttribs[i-1] < 0)
					error = AGL_BAD_VALUE;
				break;
			case AGL_ACCUM_BLUE_SIZE:
				attribCopy[j++] = GLI_ACCUM_BLUE_SIZE;
				attribCopy[j++] = inAttribs[i++];
				if (inAttribs[i-1] < 0)
					error = AGL_BAD_VALUE;;
				break;
			case AGL_ACCUM_ALPHA_SIZE:
				attribCopy[j++] = GLI_ACCUM_ALPHA_SIZE;
				attribCopy[j++] = inAttribs[i++];
				if (inAttribs[i-1] < 0)
					error = AGL_BAD_VALUE;
				break;

		

			
			
			/*
			 ** Extended format attributes
			 */
			case AGL_PIXEL_SIZE:
			/* 
			 * Must be followed by a nonnegative bits-per-pixel specification that is matched exactly.
			 * The pixel size is the number of bits required to store each pixel in the color buffer,
			 * including unused bits. If the pixel format has an alpha channel that is stored in a separate
			 * buffer, itÕs size is not included in the pixel size.
			 */
				attribCopy[j++] = GLI_PIXEL_SIZE;
				attribCopy[j++] = inAttribs[i++];
				extPixelSize = inAttribs[i-1];
				if (inAttribs[i-1] < 0)
					error = AGL_BAD_VALUE;
				break;
			case AGL_MINIMUM_POLICY:
			/*
			 * If present, the pixel format choosing policy is altered for the color, depth, and
			 * accumulation buffers such that only buffers of size greater than or equal to the desired
			 * size are considered.
			 */
				attribCopy[j++] = GLI_MINIMUM_POLICY;
				extPolicy = GLI_MINIMUM_POLICY;
				break;
			case AGL_MAXIMUM_POLICY:
			/*
			 * If present, the pixel format choosing policy is altered for the color, depth, and
			 * accumulation buffers such that, if a nonzero buffer size is requested, the largest available
			 * buffer is preferred.
			 */
				attribCopy[j++] = GLI_MAXIMUM_POLICY;
				extPolicy = GLI_MAXIMUM_POLICY;
				break;
			case AGL_CLOSEST_POLICY:
			/*
			 * If present, the pixel format choosing policy is altered for the color buffer such that the
			 * buffer closest to the requested size is preferred, regardless of the actual color buffer depth
			 * of the supported graphics device.
			 */
				attribCopy[j++] = GLI_CLOSEST_POLICY;
				extPolicy |= GLI_CLOSEST_POLICY;
				break;
			case AGL_OFFSCREEN:	
			/* If present, only renderers that are capable of rendering to an off-screen memory area and
			 * have buffer depth exactly equal to the desired buffer depth are considered. Furthermore,
			 * gdev and ndev must be set to NULL and zero when AGL_OFFSCREEN is present.
			 * When AGL_OFFSCREEN is present the AGL_CLOSEST_POLICY attribute is
			 * implied.
			 */
			 	extOffscreen = GL_TRUE;
				attribCopy[j++] = GLI_OFFSCREEN;
				break;
			case AGL_FULLSCREEN:
			/* If present, only renderers that are capable of rendering to a full-screen graphics device are
			 * considered. Furthermore, gdev and ndev must be set to NULL and zero when
			 * AGL_FULLSCREEN is present.
			 */
				extFullscreen 	= GL_TRUE;
				attribCopy[j++] = GLI_FULLSCREEN;
				break;
			case AGL_ALL_RENDERERS:
			/* If present, pixel format selection will be open to all available renderers, including debug
			 * and special purpose renderers that are not OpenGL compliant.
			 */
				attribCopy[j++] = GLI_ALL_RENDERERS;
				/* Boolean attribute */
				break;
			case AGL_RENDERER_ID:
			/* Must be followed by a nonnegative renderer ID number. If present, OpenGL renderers
			 * that match the specified ID are preferred. Two constants are provided in the agl.h header
			 * to select specific renderers: AGL_GENERIC_RENDERER_ID selects the Apple
			 * software renderer, and AGL_RAVE_RENDERER_ID selects the Apple OpenGL
			 * RAVE driver, which in turns selects a suitable RAVE renderer.
			 */
				extRendererIDS[extRendererIDNum++] = inAttribs[i++];
				break;
			case AGL_SINGLE_RENDERER:
			/* If present, a single rendering engine is chosen to render to all specified graphics devices.
			 * On systems with multiple screens, this disables the AGL library's ability to drive different
			 * monitors through different graphics accelerator cards with a single AGL context.
			 */
				extSingleRenderer = GL_TRUE;
				break;
			case AGL_NO_RECOVERY:
			/* If present, the AGL library's failure recovery mechanisms are disabled. Normally, if an
			 * accelerated renderer cannot attach to a drawable due to insufficient video memory AGL
			 * automatically switches to another renderer. This attribute disables these features so that
			 * rendering will always be done by the chosen renderer.
			 */
			 	/* TODO: This is not yet supported by Mesa's AGL! */
				break;
			case AGL_ACCELERATED:
			/* If present, only renderers that are attached to a hardware accelerated graphics device are
			 * considered. It is usually impossible to support more than one graphics device if the
			 * AGL_ACCELERATED attribute is given.
			 */
			 	extAccelerated = GL_TRUE;
				attribCopy[j++] = GLI_ACCELERATED;
				break;
			case AGL_BACKING_STORE:
			/* If present, only renderers that have a back color buffer that is the full size of the drawable
			 * regardless of window visibility, and that guarantee the back buffer contents to be valid
			 * after a call to aglSwapBuffers are considered.
			 */
			 	attribCopy[j++] = GLI_BACKING_STORE;
				break;
			case AGL_ROBUST:
			/* If present, only renderers that do not have any failure modes associated with a lack of
			 * video card resources are considered.
			 */
			 	/* TODO: I really dont understand what this means....
				   So i leave it for now... */
				break;
			case AGL_MP_SAFE:
			/* If present, only renderers that are multi-processor safe are considered. To execute
			 * OpenGL commands on a second processor, an application must use an MP safe pixel
			 * format and also put the OpenGL library into an MP safe memory allocation mode with the
			 * GLM interface.
			 */
			 	attribCopy[j++] = GLI_MP_SAFE;
			 	break;
			default:
				error = AGL_BAD_ATTRIBUTE;
				break;
		}
	}

	/* Test for errors */
	if (i >= kMAGL_MaxAttributesNum)
	{
		MAGL_SetError(NULL,(GLenum)AGL_BAD_ATTRIBUTE,("aglChossePixelFormat: Too many attributes, list possibly not terminated with AGL_NONE"));
		return GL_FALSE;
	}
	if ((error == AGL_BAD_ATTRIBUTE) || (error == AGL_BAD_VALUE))
	{
		MAGL_SetError(NULL,(GLenum)error,("aglChoosePixelFmt: bad attribute/(value) at/after %d!",attribCmd));
		return GL_FALSE;
	}
	if ((extOffscreen) && ((extPixelSize == -1) || ((inGDevices != NULL) || (inDevicesNum != 0))))
	{
		MAGL_SetError(NULL,(GLenum)AGL_BAD_ATTRIBUTE,("aglChossePixelFormat: AGL_OFFSCREEN specified, but pixel_size wasn't or the devs != 0 or ndev != 0!"));
		return GL_FALSE;
	}
	if ((extFullscreen) && (inGDevices != NULL || inDevicesNum != 0))
	{
		MAGL_SetError(NULL,(GLenum)AGL_BAD_ATTRIBUTE,("aglChossePixelFormat: AGL_FULLSCREEN specified, devs!=0 || ndev != 0 !"));
		return GL_FALSE;		
	}
	attribCopy[j++] = GLI_NONE;
	
	/* 
	 * Check dev/ndev parameters, and copy them to devices/devicesNum: 
	 */
	if (((inDevicesNum == 0) && (inGDevices != NULL)) || ((inGDevices == NULL) && (inDevicesNum != 0))) 
	{
		
		MAGL_SetError(NULL, (GLenum)AGL_BAD_GDEV,("aglChoosePixelFmt: one of inGDevices(%h),inDevicesNum(%d) is NULL but not both!",inGDevices,inDevicesNum));
		 return GL_FALSE;
	}
	
	
	

	/*
	 * If no devices specified set devices to all graphics devices of the system, 
	 * otherwise check them for validity.
	 */
	if (inGDevices == NULL) /* No Devices specified, so search for the screen devices */
	{
		GDHandle	gdNthDevice;
		
		devicesNum = 0;

		for (gdNthDevice = GetFirstGDevice();
			 gdNthDevice != NULL;
			 gdNthDevice = GetNextGDevice(gdNthDevice))
		{
			if (devicesNum > kMAGL_MaxDevicesNum)
			{
				MAGL_SetError(NULL,(GLenum)AGL_BAD_GDEV,("aglChoosePixelFmt:Too many devices (> %d)!",kMAGL_MaxDevicesNum));
				return GL_FALSE;
			}
			devices[devicesNum] = gdNthDevice;
			devicesNum++;
		}
	}
	else
	{
		if (inDevicesNum > kMAGL_MaxDevicesNum)
		{
			MAGL_SetError(NULL,(GLenum)AGL_BAD_GDEV,("aglChoosePixelFmt: Too many devices (> %d)!",kMAGL_MaxDevicesNum));
			return GL_FALSE;
		}
		
		for (i = 0; i < inDevicesNum; i++)
		{	
			if (!IsValidGDev(inGDevices[i]))
			{
				MAGL_SetError(NULL,(GLenum)AGL_BAD_GDEV,("aglChoosePixelFmt: the devices contained by gdev isn't valid!"));
				return GL_FALSE;
			}
			devices[i] = inGDevices[i];
		}
		devicesNum = inDevicesNum;
	}
	
	/*
	 * Special Case Fullscreen or Offscreen:
	 * We don't test for GDevices!
	 */
	if (extFullscreen || extOffscreen)
	{
		devicesNum = 0;
	}
	
	
	
	{
		int gliRenderer;
		
	   /*
		* Find the bestPF/bestPFForAll.
	 	*/
		bestPFForAllScore = -1;
		for (i = 0; i < devicesNum; i++)
		{
			bestPFScore[i] = -1;
		}
		
		/*
		 * Request pixel formats from all renderers
		 */
		{
			GLIPixelFormat *fmts;
			int 			score=-1;
			GLIPixelFormat 	*currentPixelFormat;
			
			GLenum error = gliChoosePixelFormat(&fmts,(GLIDevice*)inGDevices,inDevicesNum,inAttribs);
			if (error != GLI_NONE)
			{
				MAGL_SetError(NULL,error,("aglChoosePixelFormat: gliChoosePixelFormat failed!"));
				return NULL;
			}

				
			/* Find the best pixel format for all, and all particular devices */
			for (currentPixelFormat = fmts;
				 currentPixelFormat != NULL;
				 currentPixelFormat = currentPixelFormat->next_pixel_format)
			{
				/* Find the best pixel format which supports all devices */
				score = CalcPFScore(
						currentPixelFormat,
						attribCopy,
						extRendererIDS,extRendererIDNum,
						extPolicy,
						extCompliance,
						extPerformance,
						devices,devicesNum);
							
					if (score > bestPFForAllScore)
					{
						bestPFForAllScore 	= score;
						bestPFForAll 		= *currentPixelFormat; 
					}
				
					/* Multiscreen support */
					if (devicesNum > 1)
					{
						for (j = 0; j < devicesNum; j++)
						{
							/* Find the best screen for screen number j */
							score = CalcPFScore(currentPixelFormat,
											attribCopy,
											extRendererIDS,extRendererIDNum,
											extPolicy,
											extCompliance,
											extPerformance,
											&devices[j],1);
						
							if (score > bestPFScore[j])
							{
								bestPFScore[j] 	= score;
								bestPF[j] 		= *currentPixelFormat;
							}
						}
					}
					
					
				}  /* for (currentPixelFormat = tmpPixelFormats; ... */	
				
				/* Release pixelFormats */
				gliDestroyPixelFormat(fmts);
			}
		}
	
	/*
	 * Select between bestFPForAll/bestFP
     */
	{
		/* TODO Multi-pixel format support */
		if (bestPFForAllScore < 0)
		{
			return NULL;
		}
		else
		{
			GLIPixelFormat *result;
			
			result = (GLIPixelFormat*)MAGL_Alloc(sizeof(GLIPixelFormat));
			if (result == NULL)
			{
				MAGL_SetError(NULL,(GLenum)AGL_BAD_ALLOC,("aglChoosePixelFmt: NewPointer failed (not enought memory)!"));
				return NULL;
			}
			*result = bestPFForAll;
			result->next_pixel_format = NULL;
			
			return (AGLPixelFormat)result;
		}
	}
	
	return NULL; /* We shouldn't get there... */
}
/***********************************************************************************************
 *
 * aglDestroyPixelFormat (external)
 *
 * Parameters:	fmt				- Specifies a pixel format to be destroyed.
 * Returns:						
 * Description:	aglDestroyPixelFormat destroys a pixel format previously created 
 *				by aglChoosePixelFormat. The aglCreateContext made a copy of the pixel
 *				format data, so the appication may destroy the pixel format right after 
 *				aglCreateContext.
 * Errors:		AGL_BAD_PIXELFMT is generated if the fmt is not a valid pixel format.
 *
 ***********************************************************************************************/
void aglDestroyPixelFormat(AGLPixelFormat fmt)
{
	MAGL_Free((char*)fmt);
}

/***********************************************************************************************
 *
 * aglDevicesOfPixelFormat (external)
 *
 * Parameters:	inPixelFormat	- Specifies a pixel format.
 *				outDeviceNum	- Returns the number of graphics devices supported by 
 *								  the pixel format.
 * Returns:						- A list of graphics devices. Use DisposePtr to free 
 *								  data returned by this function.
 * Description:	aglDevicesOfPixelFormat is used to determine what subset of all requested  
 *        	    graphics devices is supported by a subsequent pixel format.  
 *              Use aglNextPixelFormat to access subsequent pixel formats in an 
 *				AGLPixelFormat structure.
 * Errors:		aglDevicesOfPixelFormat returns NULL if it fails for any reason.
 *              AGL_BAD_PIXELFMT is generated if pix is not a valid AGL pixel format.
 *				(NOT IN CONIX)
 *				AGL_BAD_ALLOC is generated if there is not enough memory, for the device list. 
 *
 ***********************************************************************************************/
GDHandle* 
aglDevicesOfPixelFormat(
						AGLPixelFormat  inPixelFormat,
						int				*outDeviceNum)	
{
	GDHandle	*result;
	int			resultNum;
	GDHandle	current;
	
	if (!IsValidPixelFormat(inPixelFormat))
	{
		MAGL_SetError(NULL,(GLenum)AGL_BAD_PIXELFMT,("aglDevicesOfPixelFormat: pix is invalid!"));
		return NULL;
	}
	
	result = (GDHandle*)NewPtr(sizeof(GDHandle*)*kMAGL_MaxDevicesNum);
	if (result == NULL)
	{
		MAGL_SetError(NULL,(GLenum)AGL_BAD_ALLOC,("aglDevicesOfPixelFormat: Not enough memory!"));
		return NULL;
	}
	
	/* TODO, call GetDeviceType or something like this */
	resultNum = 0;
	for (	current = GetFirstGDevice(); 
			current != NULL; 
			current = GetNextGDevice(current))
		{
			if (IsDeviceOfPixelFormat((GLIPixelFormat*)inPixelFormat,current))
			{
				result[resultNum++] = current; 
			}
			
			if (resultNum >= kMAGL_MaxDevicesNum)
			{
				MAGL_SetError(NULL,(GLenum)AGL_BAD_ALLOC,("aglDevicesOfPixelFormat: Too many devices (> %d)!",kMAGL_MaxDevicesNum));
				return NULL;
			}
		}
	
	*outDeviceNum = resultNum;
	return result; 
}
/***********************************************************************************************
 *
 * aglNextPixelFormat (external)
 *
 * Parameters:	inPixelFormat	- Specifies a pixel format.
 *
 * Returns:						- The next pixel format in a list of pixel formats.  If pix  is 
 *            					  the last pixel format in the list, NULL is returned.
 * Description:	Return the next in a list of pixel formats. 
 *
 * Errors:		aglNextPixelFormat returns NULL if it fails for any reason.
 *              AGL_BAD_PIXELFMT	is generated if pix  is not a valid AGL pixel format.
 *
 ***********************************************************************************************/

AGLPixelFormat 
aglNextPixelFormat(
			AGLPixelFormat inPixelFormat)
{
	/*
	 * Currently we only support single pixel formats...
	 */
	if (!IsValidPixelFormat(inPixelFormat))
	{
		MAGL_SetError(NULL,(GLenum)AGL_BAD_PIXELFMT,("aglGetNextPixelFormat:pix (%h) is not a valid AGL pixel format!",inPixelFormat));
		return NULL;
	}
	
	return (AGLPixelFormat)(((GLIPixelFormat*)inPixelFormat)->next_pixel_format);
}
/***********************************************************************************************
 *
 * aglDescribePixelFormat (external)
 *
 * Parameters:	inPixelFormat	- Specifies the pixel format.
 *				inAttrib		- Specifies the pixel format attribute to be returned.
 *				outValue		- Returns the requested value.
 *
 * Returns:						- GL_TRUE on success.
 *
 * Description:	Return information about AGL pixel formats.
 *
 * Errors:		aglDescribePixelFormat returns GL_FALSE if it fails for any reason.
 *              AGL_BAD_PIXELFMT is generated if pix  is not a valid pixel format.
 *			 	AGL_BAD_ATTRIBUTE is generated if attrib  is not an accepted attribute.
 *
 ***********************************************************************************************/

GLboolean 
aglDescribePixelFormat( 
					AGLPixelFormat inPixelFormat,
					GLint 		   inAttrib,
					GLint 		   *outValue )
{
	GLIPixelFormat 		*pix 			= (GLIPixelFormat*)inPixelFormat;
	GLboolean 			badAttribute 	= GL_FALSE;
	GLboolean 			notIndexed 		= GL_FALSE;
	GLboolean			notRgba 		= GL_FALSE;
	TGLIUtilsColorMode	colorMode;
	
	
	MAGL_Assert(outValue != nil);
	
	if (!IsValidPixelFormat(inPixelFormat))
	{
		MAGL_SetError(NULL,(GLenum)AGL_BAD_PIXELFMT,("aglDescribePixelFormat:pix (%h) is not a valid AGL pixel format!",inPixelFormat));
		return GL_FALSE;
	}
	
	_gliUtilsGetColorMode(pix->buffer_mode,&colorMode);

	switch (inAttrib) 
	{
		case AGL_BUFFER_SIZE:
			*outValue = colorMode.pixel_size;
			break;
		case AGL_LEVEL:
			*outValue = pix->level;
			break;
		case AGL_RGBA:
			*outValue = colorMode.rgba;
			break;
		case AGL_DOUBLEBUFFER:
			*outValue = (pix->buffer_mode & GLI_DOUBLEBUFFER_BIT);
			break;
		case AGL_STEREO:
			*outValue = (pix->buffer_mode & GLI_STEREOSCOPIC_BIT);
			break;
		case AGL_AUX_BUFFERS:
			*outValue = pix->aux_buffers;
			break;
		case AGL_RED_SIZE:
			if (!colorMode.rgba)
				notRgba = badAttribute= GL_TRUE;
			*outValue = colorMode.red_size;
			break;
		case AGL_GREEN_SIZE:
			if (!colorMode.rgba)
				notRgba = badAttribute= GL_TRUE;
			*outValue = colorMode.green_size;
			break;
		case AGL_BLUE_SIZE:
			if (!colorMode.rgba)
				notRgba = badAttribute= GL_TRUE;
			*outValue = colorMode.blue_size;
			break;
		case AGL_ALPHA_SIZE:
			if (!colorMode.rgba)
				notRgba = badAttribute= GL_TRUE;
			*outValue = colorMode.alpha_size;
			break;
		case AGL_DEPTH_SIZE:
			*outValue = _gliUtilsGetBitDepthSize(pix->depth_mode);
			break;
		case AGL_STENCIL_SIZE:
			*outValue = _gliUtilsGetBitDepthSize(pix->stencil_mode);
			break;
		case AGL_ACCUM_RED_SIZE:
			if (!colorMode.rgba)
				notRgba = badAttribute= GL_TRUE;
			*outValue =  _gliUtilsGetBitDepthSize(pix->accum_mode);
			break;
		case AGL_ACCUM_GREEN_SIZE:
			if (!colorMode.rgba)
				notRgba = badAttribute= GL_TRUE;
			*outValue = _gliUtilsGetBitDepthSize(pix->accum_mode);
			break;
		case AGL_ACCUM_BLUE_SIZE:
			if (!colorMode.rgba)
				notRgba = badAttribute= GL_TRUE;
			*outValue = _gliUtilsGetBitDepthSize(pix->accum_mode);
			break;
		case AGL_ACCUM_ALPHA_SIZE:
			if (!colorMode.rgba)
				notRgba = badAttribute= GL_TRUE;
			*outValue = _gliUtilsGetBitDepthSize(pix->accum_mode);
			break;
		case  AGL_PIXEL_SIZE:
			*outValue = colorMode.pixel_size;
			break;
		case AGL_OFFSCREEN:
			if (pix->os_support & GLI_OFFSCREEN_BIT) 
				*outValue = GL_TRUE;
			else 
				*outValue = GL_FALSE; 
			break;
		case AGL_FULLSCREEN:
			if (pix->os_support & GLI_FULLSCREEN_BIT) 
				*outValue = GL_TRUE;	
			else 
				*outValue = GL_FALSE;
			break;
		case AGL_WINDOW:
			if (pix->os_support & GLI_WINDOW_BIT)
				*outValue = GL_TRUE;
			else
				*outValue = GL_FALSE;
			break;
		case AGL_RENDERER_ID:
			*outValue = pix->renderer_id;
			break;
		case AGL_SINGLE_RENDERER:
			if (pix->next_pixel_format == NULL)
				*outValue = GL_TRUE;
			else
				*outValue = GL_FALSE;
			break;
		/*case AGL_NO_RECOVERY:
			*outValue = (pix->os_support & GLI_ROBUST_BIT);
			break;
		*/
		case AGL_ACCELERATED:
			*outValue = (pix->os_support & GLI_ACCELERATED_BIT);
			break;
		case AGL_BACKING_STORE:
			if (pix->os_support & GLI_BACKING_STORE_BIT)
			{ *outValue = GL_TRUE; }
			else
			*outValue = GL_FALSE;
			break;
		case AGL_ROBUST:
			*outValue = (pix->os_support & GLI_ROBUST_BIT);
			break;
		case AGL_MP_SAFE:
			*outValue = (pix->os_support & GLI_MP_SAFE_BIT);
			break;
		case AGL_COMPLIANT:
			*outValue = (pix->os_support & GLI_COMPLIANT_BIT);
			break;
		case AGL_MULTISCREEN:
			if (pix->os_support & GLI_MULTISCREEN_BIT)
				*outValue = GL_TRUE;
			else
				*outValue = GL_FALSE;
			break;
		case  AGL_VIRTUAL_SCREEN:
			*outValue = 0;	
			/* TODO */
			break;
		default:
			badAttribute = GL_TRUE;
			break;
			
	}
	
	if (badAttribute)
	{
		if (notRgba)
			MAGL_SetError(NULL,(GLenum)AGL_BAD_ATTRIBUTE,("aglDescribePixelFormat:AGL_RGBA is false!"));
		else
			MAGL_SetError(NULL,(GLenum)AGL_BAD_ATTRIBUTE,("aglDescribePixelFormat:bad attribute!"));
		
		return GL_FALSE;
	}
	
	return GL_TRUE;
}


