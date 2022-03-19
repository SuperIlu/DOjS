/*
	File:		gliutils.h

	Contains:	Utility functions for GLI.

	Written by:	miklos

	Copyright:	Copyright © 1999 Miklós Fazekas. All rights reversed.

	Change History (most recent first):

         <1>      7/5/99    miklos  Utility functions for GLI.
*/


#ifndef __GLI_UTILS_H__
#define __GLI_UTILS_H__

#include "gliMesa.h"


typedef struct TGLIUtilsColorMode
{
	GLbitfield mode;
	GLboolean rgba;
	GLint pixel_size;	/* The pixel size is the number of bits required to store each pixel in the color buffer,
						   including unused bits. If the pixel format has an alpha channel that is stored in a separate
						   buffer, it’s size is not included in the pixel size,
						   Contains buffer_size if indexed.  */
	GLint red_size;
	GLint green_size;
	GLint blue_size;
	GLint alpha_size;
	GLint macos_depth;
} TGLIUtilsColorMode;

typedef struct TGLIUtilsGrafPortDrawableData
{
	CGrafPtr		port;
}
TGLIUtilsGrafPortDrawableData;
typedef struct TGLIUtilsOffscreenDrawableData
{
	GLsizei width; 
	GLsizei height; 
	GLsizei rowbytes;
	GLvoid *baseaddr;
}
TGLIUtilsOffscreenDrawableData;
typedef struct TGLIUtilsFullscreenDrawableData
{
	GLsizei width;
    GLsizei height; 
    GLsizei freq;
    GLint 	device;
}
TGLIUtilsFullscreenDrawableData;
typedef union TGLIUtilsDrawableDataRec
{
	TGLIUtilsOffscreenDrawableData	offscreen;
	TGLIUtilsFullscreenDrawableData	fullscreen;
	TGLIUtilsGrafPortDrawableData	window;
}
TGLIUtilsDrawableData;
typedef struct TGLIUtilsDrawable 
{
	GLint						type; 		/* One of GLI_OFFSCREEN,GLI_FULLSCREEN,GLI_WINDOW */
	TGLIUtilsDrawableData		data;	/* Interpereted for the different kind of drawables */
}
TGLIUtilsDrawable;
/***********************************************************************************************
 *
 * _gliUtilsGetGLIDrawable 
 *
 * Description: Returns drawable parameter passed to gliSetDrawable.
 *
 ***********************************************************************************************/
GLIDrawable *_gliUtilsGetGLIDrawable(const TGLIUtilsDrawable	*drawable);
/***********************************************************************************************
 *
 * _gliUtilsIsValidDrawable 
 *
 * Description: Returns whetever the drawable passed in the drawable parameter is a valid one.
 *
 ***********************************************************************************************/
GLboolean _gliUtilsIsValidDrawable(const TGLIUtilsDrawable	*drawable);
/***********************************************************************************************
 *
 * GetFirstGDevice
 *
 * Description: Returns the first graphics device on the system.
 *
 ***********************************************************************************************/
GDHandle
_gliUtilsGetFirstGDevice();
/***********************************************************************************************
 *
 * GetNextGDevice 
 *
 * Description: Returns the next graphics device on the system.
 *
 ***********************************************************************************************/
GDHandle
_gliUtilsGetNextGDevice(GDHandle inDevice);
/***********************************************************************************************
 *
 * GetGDevicesNum
 *
 * Description: Returns the number of  graphics device on the system.
 *
 ***********************************************************************************************/
GLint
_gliUtilsGetGDevicesNum(void);

/***********************************************************************************************
 *
 * Retrieves information from the buffer_mode.
 *
 ***********************************************************************************************/
extern GLboolean _gliUtilsGetColorMode(GLint buffer_mode,TGLIUtilsColorMode *mode);
/***********************************************************************************************
 *
 * gliUtilsGetBufferSize
 * Returns the size in bits of a specific mode. (Returns 0 for GLI_NONE)
 *
 ***********************************************************************************************/
extern GLint _gliUtilsGetBitDepthSize(GLbitfield mode);
/***********************************************************************************************
 *
 * _gliUtilsScorePixelFormat
 *	Returns a score for a pixel format+attributes, -1 means that the format doesn't meets the
 *	requirements.
 *
 ***********************************************************************************************/
extern GLint _gliUtilsScorePixelFormat(
								GLIPixelFormat 		*pix,
								const GLIDevice 	*devices,
								GLint 				ndevs,
								const GLint 		attrib[]);
/***********************************************************************************************
 *
 * _gliUtilsConvertGLToMacOSRect
 *	Converts an OpenGL rectangle to the mac-os rectangle.
 *
 ***********************************************************************************************/
extern void _gliUtilsGetDrawableDimensions(
								const TGLIUtilsDrawable	*drawable,
								GLint					*width,
								GLint					*height);
/***********************************************************************************************
 *
 * _gliUtilsConvertGLToMacOSRect
 *	Converts an OpenGL rectangle to the mac-os rectangle.
 *
 ***********************************************************************************************/
extern void _gliUtilsConvertGLToMacOSRect(
								const TGLIUtilsDrawable	*drawable,
								const GLint 		inGLRect[],
								Rect 				*outRect);
/***********************************************************************************************
 *
 * _gliUtilsSetDrawable(GLint type,GLIDrawable data,TGLIUtilsDrawable *out)
 *	Converts the drawable to the sturcture on the top.
 *
 ***********************************************************************************************/
extern void _gliUtilsSetDrawable(
								const GLint 			type,
								const GLIDrawable 		*data,
								TGLIUtilsDrawable 		*out);															

/***********************************************************************************************
 *
 * _gliUtilsIsNullDrawable(TGLIUtilsDrawable *out)
 *	Determines if the drawable is null or not.
 *
 ***********************************************************************************************/
extern GLboolean _gliUtilsIsNullDrawable(TGLIUtilsDrawable *out);															
/***********************************************************************************************
 *
 * _gliUtilsIsNullDrawable(TGLIUtilsDrawable *out)
 *	Determines if the drawable's are the same or not.
 *
 ***********************************************************************************************/
extern GLboolean _gliUtilsAreTheSameDrawables(TGLIUtilsDrawable *drw1,TGLIUtilsDrawable *drw2);	


#endif