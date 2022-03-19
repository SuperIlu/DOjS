/*
	File:		fxgli.c

	Contains:	GLI interface on the to of fxApi.

	Written by:	Fazekas Mikl—s

	Copyright:	Copyright(C) 1995-98 Miklos Fazekas
			E-Mail: boga@valerie.inf.elte.hu
			WWW: http://valerie.inf.elte.hu/~boga/Mesa.html
				
			Part of the Mesa 3D Graphics Library for MacOS.

	Change History (most recent first):
	
       <10+>     5/15/99    miklos  Reverted to older check.
        <10>     5/15/99    miklos  Added better check for buggy Glide.
       	<8+>		    miklos  Bug fixes for V2-s Voodoo driver.
         <8>      5/3/99    miklos  Revsion for 3.1b10.
         <7>     4/28/99    miklos  Added triangle checking.
         <5>     4/27/99    miklos  Added a hack for TEXTURE_MEMORY.
        <1>      7/2/98     miklos  Initial revision.
*/




#include "gli.h"

#include "mgliGetEnv.h"

#include "fxMesa.h"
#include "fxdrv.h"

/* MacOS */
#include "Errors.h"
#include "Events.h"
#include <CodeFragments.h>

/* ANSI */
#include <stdlib.h>
#include <stdio.h>

/* Mesa */
#include "types.h"



#include <glide.h>

#include "GLIAttribParser.c"

/***********************************************************************************************
 *
 * Type Definitions
 *
 ***********************************************************************************************/
typedef struct TFXContext {
	TGLIDrawable 	drawable;
	fxMesaContext	ctx;
	GLIPixelFormat	fmt;
	float			nearMin,farMax;
	/* This should be true if we want to fix the read region bug in Glide */
	GLboolean		fixReadRegion;
} TFXContext;



/*
 * The following codes is for fixing the bug founds in the MC versions of MacGlide!!!
 */
#if GLI_3DFX_FIXGRLFBREADREGIONBUG
#undef grLfbReadRegion

int (*__grLfbReadRegion)( GrBuffer_t src_buffer,
                						 FxU32 src_x, FxU32 src_y,
                 					     FxU32 src_width, FxU32 src_height,
                						 FxU32 dst_stride, void *dst_data ) = NULL;
                						
extern int grLfbReadRegion( GrBuffer_t src_buffer,
                 FxU32 src_x, FxU32 src_y,
                 FxU32 src_width, FxU32 src_height,
                 FxU32 dst_stride, void *dst_data );
/* We wan't it to output: BGR!!! */             
static int ri = 2;
static int gi = 1;
static int bi = 0;
                 

static int __fixedGrLfbReadRegionCall(GrBuffer_t src_buffer,
                 		FxU32 src_x, FxU32 src_y,
                 		FxU32 src_width, FxU32 src_height,
                 		FxU32 dst_stride, void *dst_data)
{
	GLushort *data;
	int i;
	int result;
	
	
	result = grLfbReadRegion(src_buffer,src_x,src_y,src_width,src_height,dst_stride,dst_data);
	/* Fix endiannes first! */
	{
		data = (GLushort*)dst_data;
		/* we only need to swap bytes */
		for ( i = 0; i< src_width/2; i++)
		{
			GLushort tmp;
			
			tmp = data[2*i];
			data[2*i] = data[2*i+1];
			data[2*i+1] = tmp;
			
			data[2*i] = (((data[2*i] & 0x00FF) << 8) | ((data[2*i] & 0xFF00) >> 8));
			data[2*i+1] = (((data[2*i+1] & 0x00FF) << 8) | ((data[2*i+1] & 0xFF00) >> 8));
		}
		/* Fix odd pixels! */
		if (src_width & 1)
		{
			data[src_width-1] = (((data[src_width-1] & 0x00FF) << 8) | ((data[src_width-1] & 0xFF00) >> 8));
		}
	}
	/*  Deal with color issues! */
	if (((src_buffer == GR_BUFFER_FRONTBUFFER) || (src_buffer == GR_BUFFER_BACKBUFFER)) && (ri != 2))
	{
		for (i = 0; i < src_width; i++)
		{
			int red,green,blue;
			int offset[3] = {11,5,0};
			GLushort out;
			
			out = data[i];
			red = (out & 0x001f) >> offset[2];
			green = (out & 0x07e0) >> offset[1];
			blue = (out & 0xf800) >> offset[0];
			
			out = 0;
			out |= red << offset[ri];
			out |= green << offset[gi];
			out |= blue << offset[bi];
			
			data[i] = out;
		} 
	}
	
	return result;
}

/* Glide should have initialized when we reach here!!! */
static void setupFixGrLfbReadRegionCall()
{
    	GLint	  black[5] = {0x00000000,0xFFFFFFFF,0xFFFF0000,0xFF00FF00,0xFF0000FF};
	char 	  data[4];
	
	/* Write a black and a white,red,green,blue pixel to 0,0 */
	grLfbWriteRegion(GR_BUFFER_FRONTBUFFER,0,0,GR_LFB_SRC_FMT_888,5,1,20,black);
	/* Read it out using */
	grLfbReadRegion(GR_BUFFER_FRONTBUFFER,0,0,2,1,4,data);

	/* If the driver is buggy we get:
		0xFFFF0000, still with some drivers it's even more complicated... */
	if (data[0] != 00)
	{
		GLushort out[5];
		__grLfbReadRegion = __fixedGrLfbReadRegionCall;
		/* Call fixed version so we don't have to care about endiannes */
		(*__grLfbReadRegion)(GR_BUFFER_FRONTBUFFER,0,0,5,1,10,out);
		
		if (MCFG_getEnv("MESA_3DFX_SWAP_SWAP_RGB_TO_BRG") != NULL)
		{
			ri = 0;
			gi = 1;
			bi = 2;
		}
	}
	else
	{
		__grLfbReadRegion = grLfbReadRegion;
	}
}

#endif /* GLI_3DFX_FIXGRLFBREADREGIONBUG */
	
#define EGLI_PREFIX 		void*  inRefCtx  ,

static GLboolean fullscreen_hack = GL_FALSE;
static GLboolean emulate_cursor = GL_FALSE;

void gl_read_config_file( GLcontext *ctx );
void gl_read_config_file( GLcontext *ctx )
{
#pragma unused(ctx)
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
 * fxgliChoosePixelFormat 
 *
 * Description: Chooses a pixel format from the attrib list.
 *
 ***********************************************************************************************/
static
GLIPixelFormat *fxgliChoosePixelFormat(
			EGLI_PREFIX
			const GLint	*attrib)
{
#pragma unused(inRefCtx)
   GLIPixelFormat	pixFmt;
   GLIPixelFormat	*result;
   GLbitfield		policy;
   
   if (!parseGLIAttribList(attrib,&pixFmt,&policy))
     return NULL;
   
   if (!pixFmt.rgba_mode)
     return NULL;
   if (pixFmt.stereo_mode)
     return NULL;
   if (pixFmt.level != 0)
     return NULL;
     
   pixFmt.pixel_size = 16;
   pixFmt.red_size = 5;
   pixFmt.green_size = 6;
   pixFmt.blue_size = 5;
   pixFmt.alpha_size = (pixFmt.alpha_size != 0) ? 8 : 0;
   pixFmt.depth_size = (pixFmt.depth_size != 0) ? 16 : 0;
   if (pixFmt.aux_buffers != 0)
     return NULL;
   if (pixFmt.stencil_size > 16)
     return NULL;
   if ((pixFmt.stencil_size > 0) && (pixFmt.stencil_size < 16))
   {
     pixFmt.stencil_size = 16;
     return NULL;
   }
   if ( (pixFmt.accum_red_size != 0) ||  (pixFmt.accum_green_size != 0) ||
   	(pixFmt.accum_blue_size != 0) || (pixFmt.accum_alpha_size != 0))
   {
   	pixFmt.accum_red_size = 8*sizeof(GLaccum);
   	pixFmt.accum_green_size = 8*sizeof(GLaccum);
   	pixFmt.accum_blue_size = 8*sizeof(GLaccum);
   	pixFmt.accum_alpha_size = 8*sizeof(GLaccum);
   }
   
   pixFmt.compliance = GLI_ALL_OPTIONS;
   pixFmt.next_pixel_format = NULL;
   pixFmt.renderer_id = GLI_RENDERER_MESA_3DFX;
   pixFmt.num_devices = 0;
   
   /*
    * If we act like a non-fullscreen renderer:
    */
   if (fullscreen_hack)
   {
  		GDHandle device = GetFirstGDevice();
  		while (device)
  		{
  			pixFmt.devices[pixFmt.num_devices++]=device;
  			device = GetNextGDevice(device);
  		}
   }
   
   /*
    * Fullscreen rendering:
    */
   if (!fullscreen_hack)
   {
      if (!(pixFmt.os_support & GLI_FULLSCREEN_RENDERING_BIT))
   	  	return NULL;
   }
   if (pixFmt.os_support & GLI_OFFSCREEN_RENDERING_BIT)
   		return NULL;
   
   pixFmt.msb_performance = GLI_SMOOTH_SHADING | GLI_TEXTURE | GLI_DEPTH_TEST | 
   			       GLI_FOG | GLI_BLEND | GLI_LINE | GLI_POINT | GLI_AA_LINE |
   			       GLI_AA_POLYGON | GLI_AA_POINT;
   pixFmt.lsb_performance = GLI_NONE;
   
   
   /*
    * Other extended:
    */
   pixFmt.accelerated = GL_TRUE;
   
   result = malloc(sizeof(GLIPixelFormat));
   if (!result)
    	return NULL;
   *result = pixFmt;
   
   
   return result;
}

static
GLboolean fxgliDestroyPixelFormat(EGLI_PREFIX
				GLIPixelFormat *fmt)
{
#pragma unused(inRefCtx)
	free(fmt);
   	return GL_TRUE;
}
static
GLboolean fxgliGetRendererInfo(EGLI_PREFIX
			       GLIRendererInfo *rend)
{
#pragma unused(inRefCtx)
   	rend->renderer_id = GLI_RENDERER_MESA_3DFX;
   	rend->os_support = GLI_FULLSCREEN_RENDERING_BIT;
   	rend->buffer_modes = GLI_DOUBLEBUFFER_MODE | GLI_SINGLEBUFFER_MODE;
   	rend->min_level = 0;
   	rend->max_level = 0;
   	rend->index_sizes = 0;
   	rend->color_modes = GLI_RGB_565 | GLI_RGB565_A8_BIT;
   	rend->accum_modes = GLI_16_BIT;
   	rend->depth_modes = GLI_16_BIT;
   	rend->stencil_modes = GLI_8_BIT;
   	rend->max_aux_buffers = 0;
   
   	rend->compliance = GLI_ALL_OPTIONS;
   	rend->msb_performance = GLI_SMOOTH_SHADING | GLI_TEXTURE | GLI_DEPTH_TEST | 
   			       GLI_FOG | GLI_BLEND | GLI_LINE | GLI_POINT | GLI_AA_LINE |
   			       GLI_AA_POLYGON | GLI_AA_POINT;
   	rend->lsb_performance = GLI_NONE;
   
   	rend->texture_method = GLI_NICEST | GLI_FASTEST;
   	rend->point_aa_method = GLI_NICEST | GLI_FASTEST;
   	rend->line_aa_method = GLI_NICEST | GLI_FASTEST;
   	rend->polygon_aa_method = GLI_NICEST | GLI_FASTEST;
   	rend->fog_method = GLI_NICEST | GLI_FASTEST;
	rend->accelerated = GL_TRUE;
   	/* ToDo: It's INCORRECT! */
	rend->texture_memory = 4096*1024;
   
	return GL_TRUE;
}

/* Problems about create context:
	FX driver doesn't let us create a context without assigning a drawable to it 
   We should use gliCreateContext,gliAttachDrawable instead of this.
   What about just creating a cheat context here, and do the real work in gliAttachOffscreen ?.
   Whats are the problems with it:
  	- ????
  	- We can't do context sharing. [Neither we can with fx... :) ]
  	- So, maybe its a good idea...
  The other idea is to split the original fx code to createContext, and attachScreen.
  (Seems to be a much better idea tought...)
  
*/
static
GLIContext fxgliCreateContext(EGLI_PREFIX
			      GLIPixelFormat *fmt, 
			      GLIContext share_list)
{
#pragma unused(inRefCtx)
#pragma unused (share_list)
	TFXContext *result;
   
	if (share_list != NULL)
		return NULL;
   	
	result = malloc(sizeof(TFXContext));
	if (result == NULL)
		return NULL;
	result->drawable.type = kGLIDrawableType_Null;
	result->ctx = NULL;
	result->fmt = (*fmt);
   
	return (GLIContext)result;

}

static
GLboolean fxgliDestroyContext(EGLI_PREFIX
		GLIContext ctx)
{
#pragma unused(inRefCtx)
	TFXContext *c = (TFXContext*)ctx; 
   
	fxMesaDestroyContext(c->ctx);
	free(c);
	return GL_TRUE;
}


TFXContext *current;

static 
GLboolean AttachFullscreen(
		TFXContext *ctx,const TGLIDrawable drawable,
		int attribList[])
{
   if (ctx->ctx != NULL)
   {
   		fxMesaUpdateScreenSize(ctx->ctx);
   		return GL_TRUE;
   }  
   current = ctx;
   ctx->ctx = fxMesaCreateBestContext(NULL,drawable.data.fullscreen.width,drawable.data.fullscreen.height,attribList);  

   ctx->farMax = 0.0;
   ctx->nearMin = 10000.0;

   
   if (ctx->ctx)
   {
   		ctx->drawable = drawable;
   		fxMesaMakeCurrent(ctx->ctx);

#if GLI_3DFX_FIXGRLFBREADREGIONBUG   		
   		setupFixGrLfbReadRegionCall();
#endif /* GLI_3DFX_FIXGRLFBREADREGIONBUG */

		/* Read Gamma value from mesaconfig.cfg is avaliable */
		if (MCFG_getEnv("MESA_DEFAULT_GAMMA"))
		{
		   float gamma;
		   char *str = MCFG_getEnv("MESA_DEFAULT_GAMMA");
		   if (sscanf(str,"%f",&gamma))
		   {
		   	grGammaCorrectionValue(gamma);
		   }
		}
			
   		return GL_TRUE;
   }
   else
   {
   		ctx->drawable.type = kGLIDrawableType_Null;
   		return GL_FALSE;
   }
}



/************************************************************************************
 * Function: fxgliAttachDrawable
 * Parameters:
 *			ctx			- the context
 *			drawable	- an offscreen,fullscreen or window drawable.
 * Returns: GL_TRUE		- if succeed, GL_FALSE otherwise...
 * 
 *************************************************************************************/
static
GLboolean fxgliAttachDrawable(EGLI_PREFIX
		GLIContext ctx, const TGLIDrawable drawable)
{
#pragma unused(inRefCtx)
   	int attribList[255];
   	int i = 0;
   	TFXContext *c = (TFXContext*)ctx; 
   	
   	if (c->fmt.double_buffer)
   	{
    	attribList[i++] = FXMESA_DOUBLEBUFFER;
   	}
   	if (c->fmt.depth_size > 0)
   	{
   		attribList[i++] = FXMESA_DEPTH_SIZE;
   		attribList[i++] = c->fmt.depth_size;
   	}
   	if (c->fmt.alpha_size > 0)
    {
   		attribList[i++] = FXMESA_ALPHA_SIZE;
   		attribList[i++] = c->fmt.alpha_size;
    }
    if (c->fmt.stencil_size > 0)
    {
   		attribList[i++] = FXMESA_STENCIL_SIZE;
   		attribList[i++] = c->fmt.stencil_size;
    }
    if (c->fmt.accum_red_size > 0)
    {
     	attribList[i++] = FXMESA_ACCUM_SIZE;
     	attribList[i++] = c->fmt.accum_red_size;
    }
	attribList[i++] = FXMESA_NONE;
   
   if (drawable.type == kGLIDrawableType_Fullscreen)
   {
   		return AttachFullscreen(c,drawable,attribList);
   }
   else if (drawable.type == kGLIDrawableType_Null)
   {
  	/* ToDo */
  		c->drawable.type = kGLIDrawableType_Null;
   }
   else if (drawable.type == kGLIDrawableType_GrafPort)
   {
		if (fullscreen_hack)
   		{
   			TGLIDrawable drw;
   		
   			drw.type = kGLIDrawableType_Fullscreen;
   			/* Get the size of the drawable as the size of the screen... */
   			GetDrawableDimensions(&drawable,&drw.data.fullscreen.width,&drw.data.fullscreen.height);
 
   			drw.data.fullscreen.freq = 75;
   			drw.data.fullscreen.device = 1;
   		
   			return AttachFullscreen(c,drw,attribList);
   		}
   		
   		return GL_FALSE;
	}
	else
 		return GL_FALSE;


	return GL_TRUE;
}



static
GLboolean fxgliSetSwapRect(EGLI_PREFIX
		GLIContext ctx, GLint x, GLint y, GLsizei width, GLsizei height)
{
#pragma unused(inRefCtx)
#pragma unused(ctx)
#pragma unused(x)
#pragma unused(y)
#pragma unused(width)
#pragma unused(height)
    /* Do nothing */
    return GL_TRUE;
}



/*
** This is a hack for showing an emulated cursor on Voodoo!
*/
static void Show_Cursor(int width,int height)
{
Cursor cursor = 	{
	{0000,0x4000,0x6000,0x7000,0x7800,0x7C00,0x7E00,0x7F00,
	 0x7F80,0x7C00,0x6C00,0x4600,0x0600,0x0300,0x0300,0x0000},
	{0xC000,0xE000,0xF000,0xF800,0xFC00,0xFE00,0xFF00,0xFF80,
	0xFFC0,0xFFE0,0xFE00,0xEF00,0xCF00,0x8780,0x0780,0x0380},
	{1, 1}
};
	GLubyte curs[16][16][3];
	GLubyte	mask[16][16];
	int i,j;
	Point mouse;
	
	for (i =0 ; i< 16; i++)
		for (j = 0; j < 16; j++)
		{
			if (cursor.mask[i] & (1 << j))
				mask[i][15-j] = 1;
			else 
				mask[i][15-j] = 0;
			
			if (cursor.data[i] & (1 << j))
			{
				curs[i][15-j][0] = 0;
				curs[i][15-j][1] = 0;
				curs[i][15-j][2] = 0;
			}
			else
			{
				curs[i][15-j][0] = 255;
				curs[i][15-j][1] = 255;
				curs[i][15-j][2] = 255;
			}
		}
		
		GetMouse(&mouse);
		/*
		grLfbWriteRegion(GR_BUFFER_BACKBUFFER,mouse.h-1,mouse.v-1,GR_LFB_SRC_FMT_8888,16,16,64,curs);
		*/
		/* Write it for line by line */
		{
			int i,j;
			for (i = 0; i < 16; i++)
			{
				for (j = 0; j < 16; j++)
				{
					if (mask[i][j])
					{
						int x,y;
						x = mouse.h+j-1;
						y = mouse.v+i-1;
						if ((x < width) && (y < height) && (x > 0) && (y > 0))
							grLfbWriteRegion(GR_BUFFER_BACKBUFFER,x,y,GR_LFB_SRC_FMT_888,1,1,64,curs[i][j]);
					}
				}
			}
		}	
}


static
GLboolean fxgliSwapBuffers(EGLI_PREFIX
		GLIContext ctx)
{
	(void)ctx;
	(void)inRefCtx;

	if (emulate_cursor)
	{
	 	TFXContext *c = (TFXContext*)ctx;
		Show_Cursor(c->drawable.data.fullscreen.width,c->drawable.data.fullscreen.height);
	}

    fxMesaSwapBuffers();
    return GL_TRUE;
}
static
GLboolean fxgliGetAttribute(EGLI_PREFIX
		GLIContext ctx, GLenum type, GLIAttrib *attrib)
{
#pragma unused(inRefCtx)
#pragma unused(ctx)
#pragma unused(type)
#pragma unused(attrib)
    return GL_FALSE;
}

static
GLboolean fxgliSetAttribute(EGLI_PREFIX
		GLIContext ctx, GLenum type, const GLIAttrib *attrib)
{
#pragma unused(inRefCtx)
#pragma unused(ctx)
#pragma unused(type)
#pragma unused(attrib)
    return GL_FALSE;
}
static
GLboolean fxgliQueryVersion(EGLI_PREFIX
		GLint *major, GLint *minor, GLint *rendID)
{
#pragma unused(inRefCtx)
   *major = 1;
   *minor = 2;
   *rendID = GLI_RENDERER_MESA_3DFX;
   
   return GL_TRUE;
}

static 
GLboolean fxgliCopyContext(
					void			*inRefCtx,
					GLIContext 		src, 
					GLIContext 		dst, 
					GLbitfield 		mask)
{
   #pragma unused(inRefCtx)
   #pragma unused(src)
   #pragma unused(dst)
   #pragma unused(mask)
   return GL_FALSE;
}
static 
GLboolean fxgliEnable(
				void 			*inRefCtx,
				GLIContext		inCtx,
				GLenum			pname)
{
#pragma unused(inRefCtx)
#pragma unused(inCtx)
	
	if (pname == GLI_3DFX_TAKEOVER)
	{
		grSstControl(GR_CONTROL_ACTIVATE);
		return GL_TRUE;
	}
	else
		return GL_FALSE;
}

static 
GLboolean fxgliIsEnabled(
				void 			*inRefCtx,
				GLIContext		inCtx,
				GLenum			pname,
				GLboolean		*out)
{
#pragma unused(inRefCtx)
#pragma unused(inCtx)
	if (pname == GLI_3DFX_TAKEOVER)
	{
		/* ToDo */
		*out = GL_TRUE;
		return GL_TRUE;
	}
	else
		return GL_FALSE;
}

static 
GLboolean fxgliDisable(
				void 			*inRefCtx,
				GLIContext		inCtx,
				GLenum			pname)
{
#pragma unused(inRefCtx)
#pragma unused(inCtx)
	if (pname == GLI_3DFX_TAKEOVER)
	{
		grSstControl(GR_CONTROL_DEACTIVATE);
		return GL_TRUE;
	}
	else
		return GL_FALSE;
}

static 
GLboolean fxgliSetInteger(
				void 			*inRefCtx,
				GLIContext		inCtx,
				GLenum			pname,
				const GLint		*value)
{
#pragma unused(inRefCtx)
   TFXContext *c = (TFXContext*)inCtx; 
   
   
	switch (pname)
	{
		case GLI_3DFX_GAMMA_VALUE:
			grGammaCorrectionValue((*value)*1.0/(65536.0));
			return GL_TRUE;
		case GLI_SWAP_RECT:
			return GL_TRUE;
		case GLI_BUFFER_RECT:
			return GL_TRUE;
		case GLI_SWAP_INTERVAL:
			/* If has ... */
			if (c->ctx != NULL)
			{
				c->ctx->swapInterval = *value;
			}
			return GL_TRUE;
		default:
			return GL_FALSE;
	}
}
static 
GLboolean fxgliGetInteger(
				void			*inRefCtx,
				GLIContext		inCtx,
				GLenum			pname,
				GLint			*value)
{
#pragma unused(inRefCtx)
   TFXContext *c = (TFXContext*)inCtx;
   
	switch (pname)
	{
		case GLI_3DFX_GAMMA_VALUE:
			/* ToDo */
			return GL_FALSE;
		case GLI_SWAP_INTERVAL:
			/* If has ... */
			if (c->ctx != NULL)
			{
				*value = c->ctx->swapInterval;
			}
			return GL_TRUE;
		default:
			return GL_FALSE;
	}
}


static
GLboolean SetupFXDriver(void)
{
   /*
   ** If glide is not installed, don't force it...
   */
   if ( (UInt32)grSstQueryHardware == (UInt32)kUnresolvedCFragSymbolAddress )
   	 return GL_FALSE;
  
   if (fxQueryHardware()== -1)
   	return GL_FALSE;
   	

  	 
   return RegisterGLIRenderer(
   		GLI_RENDERER_MESA_3DFX,
   		NULL,
   		fxgliChoosePixelFormat,
   		fxgliDestroyPixelFormat,
   		fxgliGetRendererInfo,
   		fxgliCreateContext,
   		fxgliDestroyContext,
   		fxgliAttachDrawable,
   		fxgliSetSwapRect,
   		fxgliSwapBuffers,
   		fxgliCopyContext,
   		fxgliQueryVersion,
   		fxgliEnable,
   		fxgliDisable,
   		fxgliGetInteger,
   		fxgliSetInteger
   		);
}

OSErr Install3DfxEngine(void);
OSErr Install3DfxEngine(void)
{
   if (MCFG_getEnv("MESA_3DFX_EMULATE_WINDOW_RENDERING"))
  		fullscreen_hack = GL_TRUE;
   else
  		fullscreen_hack = GL_FALSE;
  		
#ifdef GLI_ENABLE_FULLSCREEN_HACK
		fullscreen_hack = GL_TRUE;
#endif

	if (MCFG_getEnv("MESA_3DFX_EMULATE_CURSOR"))
		emulate_cursor = GL_TRUE;
	else
		emulate_cursor = GL_FALSE;

   if (SetupFXDriver())
      return noErr;
   else
      return paramErr;
}

