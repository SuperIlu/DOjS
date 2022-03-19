/*
	File:		gliutils.c

	Contains:	Utility routines for gli.

	Written by:	miklos

	Copyright:	Copyright ©

	Change History (most recent first):

         <3>      7/4/99    miklos  Revision.
        <1+>     6/18/99    miklos  Added score pixel format.
         <1>      6/8/99    ???     Initial revision.
*/


#include "gliutils.h"
const TGLIUtilsColorMode	gRGBARecord[23]	= 
	{{GLI_RGB8_BIT,GL_TRUE,8,3,3,3,0,8},		/* 8 rgb bit/pixel,     RGB=7:0, inverse colormap         */
	 {GLI_RGB8_A8_BIT,GL_TRUE,16,3,3,3,8,8},	/* 8-8 argb bit/pixel,  A=7:0, RGB=7:0, inverse colormap  */
	 {GLI_BGR233_BIT,GL_TRUE,8,2,3,3,0,-1},
	 {GLI_BGR233_A8_BIT,GL_TRUE,16,2,3,3,8,-1},
	 {GLI_RGB332_BIT,GL_TRUE,8,3,3,2,0,-1},
	 {GLI_RGB332_A8_BIT,GL_TRUE,16,3,3,2,8,-1},
	 {GLI_RGB444_BIT,GL_TRUE,16,4,4,4,0,-1},
	 {GLI_ARGB4444_BIT,GL_TRUE,16,4,4,4,4,-1},
	 {GLI_RGB444_A8_BIT,GL_TRUE,16,4,4,4,8,-1},
	 {GLI_RGB555_BIT,GL_TRUE,16,5,5,5,0,16},
	 {GLI_RGB555_A8_BIT,GL_TRUE,24,5,5,5,8,16},
	 {GLI_RGB888_BIT,GL_TRUE,32,8,8,8,0,32},
	 {GLI_ARGB8888_BIT,GL_TRUE,32,8,8,8,8,32},
	 {GLI_RGB888_A8_BIT,GL_TRUE,32,8,8,8,8,32},
	 {GLI_RGB101010_BIT,GL_TRUE,32,10,10,10,0,-1},
	 {GLI_ARGB2101010_BIT,GL_TRUE,32,10,10,10,2,-1},
	 {GLI_RGB101010_A8_BIT,GL_TRUE,48,10,10,10,8,-1},
	 {GLI_RGB121212_BIT,GL_TRUE,48,12,12,12,0,-1},
	 {GLI_ARGB12121212_BIT,GL_TRUE,48,12,12,12,12,-1},
	 {GLI_RGB161616_BIT,GL_TRUE,64,16,16,16,16,-1},
	 {GLI_ARGB16161616_BIT,GL_TRUE,64,16,16,16,16,-1},
	 {GLI_INDEX8_BIT,GL_FALSE,8,-1,-1,-1,-1,8},		/* 8 bit color look up table                              */
	 {GLI_INDEX16_BIT,GL_FALSE,16,-1,-1,-1,-1,-1}};	/* 16 bit color look up table                             */



void _gliUtilsConvertGLToMacOSRect(const TGLIUtilsDrawable *inDrawable,const GLint inRect[],Rect *outRect)
{
	int width,height;
	
	_gliUtilsGetDrawableDimensions(inDrawable,&width,&height);
	
	outRect->left 	= inRect[0];
	outRect->right 	= inRect[0]+inRect[2];
	outRect->bottom = height-(inRect[1]);
	outRect->top 	= outRect->bottom-inRect[3];
}

/***********************************************************************************************
 *
 * GetFirstGDevice (internal)
 *
 * Description: Returns the first graphics device on the system.
 *
 ***********************************************************************************************/
GDHandle
_gliUtilsGetFirstGDevice()
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
GDHandle
_gliUtilsGetNextGDevice(GDHandle inDevice)
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
GLint _gliUtilsGetGDevicesNum(void)
{
	GLint i = 0;
	GDHandle dev;
	
	dev = _gliUtilsGetFirstGDevice();
	while (dev) {
		dev = _gliUtilsGetNextGDevice(dev);
		i++;
	}
	return i;
}

GLboolean _gliUtilsGetColorMode(GLint buffer_mode,TGLIUtilsColorMode *mode)
{
	int i;
	GLboolean found = GL_FALSE;
	
	mode->mode = 0;
	mode->rgba = GL_FALSE;
	mode->pixel_size = -1;
	mode->red_size = -1;
	mode->green_size = -1;
	mode->blue_size = -1;
	mode->alpha_size = -1;
	mode->macos_depth = -1;
	
	for (i = 0; i < 23; i++)
	{
		if (gRGBARecord[i].mode == buffer_mode)
		{
			*mode = gRGBARecord[i];
			found = GL_TRUE;
		}
	}
	
	
	return found;
}

GLint _gliUtilsGetBitDepthSize(GLbitfield mode)
{
	switch (mode)
	{
		case GLI_0_BIT:
			return 0;
		case GLI_1_BIT:
			return 1;
		case GLI_2_BIT:
			return 2;
		case GLI_3_BIT:
			return 3;
		case GLI_4_BIT:
			return 4;
		case GLI_5_BIT:
			return 5;
		case GLI_6_BIT:
			return 6;
		case GLI_8_BIT:
			return 8;
		case GLI_10_BIT:
			return 10;
		case GLI_12_BIT:
			return 12;
		case GLI_16_BIT:
			return 16;
		case GLI_24_BIT:
			return 24;
		case GLI_32_BIT:
			return 32;
		case GLI_48_BIT:
			return 48;
		case GLI_64_BIT:
			return 64;
		case GLI_96_BIT:
			return 96;
		case GLI_128_BIT:
			return 128;
		default:
			return 0;
	}
}
/*
 * Scores an attribute, -1 means that the attribute doesn't meets the requirements.
 * Higher scores are better.
 */
static GLint scoreAttribWithDefaultPolicy(GLint attrib,GLint pix_value,GLint req_value)
{
	if ((req_value != 0) && (pix_value ==0))
		return -1;

	if (attrib == GLI_STENCIL_SIZE)
	{
		if (req_value > pix_value) return -1;
		
		return 32-(pix_value-req_value);
	}
	else
	{
		int abs = (req_value-pix_value);
		if (abs < 0)
			abs = -abs;
		return 32-abs;
	}
	
	return -1;
}

static GLint CalcScoreForAttribute(GLint attrib,GLint pix_val,GLint req_val,GLint pol)
{
	(void)pol;
	/* Minimum policy: trivial */
	if (0)
	{
		if (pix_val < req_val)
			return -1;
	}
	
	return scoreAttribWithDefaultPolicy(attrib,pix_val,req_val);
}
/*
 * Scores a pixel format from it's requirements, -1 means the pixel format is not up to
 * the specifaction higher scores are better.
 *
 * The pix could contain more color_modes, what means it supports all of them.
 * (The choosen one is depends from the current resolution only.)
 */
GLint _gliUtilsScorePixelFormat(GLIPixelFormat 		*pix,
								const GLIDevice 	*devices,
								GLint 				ndevs,
								const GLint 		attrib[])
{
	GLint current = 0;
	
	TGLIUtilsColorMode	   buffer_modes[10];
	GLint				   buffer_mode_num;
	GLint				   i,j;
	GLint				   policy;
	GLint				   result;
	GLboolean			   attrib_present[GLI_STATE_VALIDATION+100];


	policy = 0; /*  ToDo: We are currently ignoring policy,  */
	
	for (j = 0; j < GLI_MP_SAFE+100; j++)
	{
		attrib_present[j] = GL_FALSE;
	}
	
	/*
	** Fill buffer_modes/buffer_mode_num:
	** Currently we only use buffer_modes[0].
	*/
	buffer_mode_num = 0;
	for (i = 0; i < 32; i++)
	{
		if (pix->color_mode & (1 << i))
		{
			if (!_gliUtilsGetColorMode((1 << i),&buffer_modes[buffer_mode_num++]))
				return GL_FALSE;
		}
	}

	
	/* Resulting score */
	result = 0;
	while (attrib[current++] != GLI_NONE)
	{
		attrib_present[attrib[current-1]] = GL_TRUE;
		switch (attrib[current-1])
		{
		    case GLI_ALL_RENDERERS:
		    	/* Skip */
		    	break;
		    case GLI_BUFFER_SIZE:
		    	current++;
		    	/* Ignored if RGBA asserted */
		    	break;
		    case GLI_LEVEL:
		    	if (attrib[current++] != 0)
		    		return -1;
		    	break;
			case GLI_RGBA:
				for (i = 0; i < buffer_mode_num; i++)
					if (!(buffer_modes[i].rgba))
						return -1;
				break;
			case GLI_DOUBLEBUFFER:
				if (!(pix->buffer_mode & GLI_DOUBLEBUFFER_BIT))
					return -1;
				break;
			case GLI_STEREO:
				if (!(pix->buffer_mode & GLI_STEREOSCOPIC_BIT))
					return -1;
				break;
			case GLI_AUX_BUFFERS:
				{
					/* 
					 * ToDo support auxuilary buffers.
				     */
					GLint num;
					num = attrib[current++];
					if (num > 0)
						return -1;
				}
				break;
			case GLI_RED_SIZE:
					{
						int score = CalcScoreForAttribute(GLI_RED_SIZE,buffer_modes[0].red_size,attrib[current++],policy);
						if (score < 0)
							return -1;
						result += score;
					}
				break;
			case GLI_GREEN_SIZE:
					{
						int score = CalcScoreForAttribute(GLI_GREEN_SIZE,buffer_modes[0].red_size,attrib[current++],policy);
						if (score < 0)
							return -1;
						result += score;
					}
				break;
			case GLI_BLUE_SIZE:
					{
						int score = CalcScoreForAttribute(GLI_BLUE_SIZE,buffer_modes[0].red_size,attrib[current++],policy);
						if (score < 0)
							return -1;
						result += score;
					}
				break;
			case GLI_ALPHA_SIZE:
					{
						int score = CalcScoreForAttribute(GLI_ALPHA_SIZE,buffer_modes[0].alpha_size,attrib[current++],policy);
						if (score < 0)
							return -1;
						result += score;
					}
				break;
			case GLI_DEPTH_SIZE:
					{
						int score = CalcScoreForAttribute(GLI_DEPTH_SIZE,_gliUtilsGetBitDepthSize(pix->depth_mode),attrib[current++],policy);
						if (score < 0)
							return -1;
						result += score;
					}
				break;
			case GLI_STENCIL_SIZE:	
					{
						int score = CalcScoreForAttribute(GLI_STENCIL_SIZE,_gliUtilsGetBitDepthSize(pix->stencil_mode),attrib[current++],policy);
						if (score < 0)
							return -1;
						result += score;
					}
				break;
			case GLI_PIXEL_SIZE:
				if (attrib[current++] != buffer_modes[0].pixel_size)
					return -1;
				break;
			case GLI_OFFSCREEN:
				if (!(pix->os_support & GLI_OFFSCREEN_BIT))
					return -1;
				break;
			case GLI_FULLSCREEN:
				if (!(pix->os_support & GLI_FULLSCREEN_BIT))
					return -1;
				break;
			case GLI_ACCELERATED:
				if (!(pix->os_support & GLI_ACCELERATED))
					return -1;
				break;
			case GLI_BACKING_STORE:
				if (!(pix->os_support & GLI_BACKING_STORE_BIT))
					return -1;
				break;
			case GLI_MP_SAFE:
				if (!(pix->os_support & GLI_MP_SAFE))
						return -1;
					break;	
			case GLI_WINDOW:
				if (!(pix->os_support & GLI_WINDOW_BIT))
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
			default:
				return -1; 
		}
	}
	
	/*
	 * See compliance!
	 */
	/* It's not our business */
	/*
	if (!attrib_present[GLI_ALL_RENDERERS] && !(pix->os_support & GLI_COMPLIANT_BIT))
		return -1;
	*/
	/* 
	 * ToDo, it might not be too good: We should check for all devices or it's enough to support one of them?
	 * Now we are checking for at least one of them to be supported.
	 */
	{
		GLint match = 0; /* J is the number of matched gdevices */
		for (i =0; i < ndevs; i++)
		{
			for (j = 0; j < pix->num_devices; j++)
			{
				if (((*(devices[i].gdevice)) == (*(pix->devices[i].gdevice))))
					match++;
			}
		}
		
		if (ndevs != 0 && match == 0)
			return -1;
	}
	
	return result;
}

GLboolean _gliUtilsIsNullDrawable(TGLIUtilsDrawable *out)
{
	if  (out->type == GLI_NONE)
		return GL_TRUE;
	else
		return GL_FALSE;
}

GLboolean _gliUtilsIsValidDrawable(const TGLIUtilsDrawable *out)
{
	return GL_TRUE;
}
GLIDrawable *_gliUtilsGetGLIDrawable(const TGLIUtilsDrawable *out)
{
	if (out->type == GLI_NONE)
		return NULL;
	if (out->type == GLI_WINDOW)
	{
		return (GLIDrawable *)out->data.window.port;
	}
	
	if (out->type == GLI_FULLSCREEN)
	{
		return (GLIDrawable *)&(out->data.fullscreen);
	}
	
	if (out->type == GLI_OFFSCREEN)
	{
		return (GLIDrawable *)&(out->data.offscreen);
	}
	
	return NULL;
}

void _gliUtilsSetDrawable(
						const GLint 			type,
						const GLIDrawable 		*data,
						TGLIUtilsDrawable 		*out)		
{
	switch (type)
	{
		case GLI_NONE:
		{
			(*out).type = GLI_NONE;
			break;
		}
		case GLI_OFFSCREEN:
		{
			(*out).type = GLI_OFFSCREEN;
			(*out).data.offscreen.width 	= ((GLint*)data)[0];
			(*out).data.offscreen.height 	= ((GLint*)data)[1];
			(*out).data.offscreen.rowbytes 	= ((GLint*)data)[2];
			(*out).data.offscreen.baseaddr 	= (GLvoid*)((GLint*)data)[3];
			break;
		}
		case GLI_FULLSCREEN:
		{
			(*out).type = GLI_FULLSCREEN;
			(*out).data.fullscreen.width = ((GLint*)data)[0];
			(*out).data.fullscreen.height = ((GLint*)data)[1];
			(*out).data.fullscreen.freq = ((GLint*)data)[2];
			(*out).data.fullscreen.device = ((GLint*)data)[3];
			break;
		}
		case GLI_WINDOW:
		{
			(*out).type = GLI_WINDOW;
			(*out).data.window.port = (CGrafPtr)data;
			break;
		}
		default:
			(*out).type = GLI_NONE;
	}
}
