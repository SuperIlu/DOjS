/*
	File:		fxgli_drawable.c

	Contains:	Implementation of gliAttachDrawable.

	Written by:	miklos

	Copyright:	Copyright ©

	Change History (most recent first):

         <2>      7/5/99    miklos  Revision
         <1>      6/8/99    ???     Initial revision.
*/


#include "fxgli.h"
#include "glide.h"

#include "gliMesa.h"

#include "fxdrv.h"

/* MacOS */
#include <Events.h>
#include <Sound.h>

static 
GLboolean IsScreenResolutionEnabled(GrScreenResolution_t res)
{
	if (res == GR_RESOLUTION_512x384|| res == GR_RESOLUTION_640x480 || res == GR_RESOLUTION_800x600 || res == GR_RESOLUTION_1024x768 
		|| res == GR_RESOLUTION_1024x768 || res == GR_RESOLUTION_1280x1024 || res == GR_RESOLUTION_1600x1200)
		return GL_TRUE;
	else
		return GL_FALSE;
}
static 
GrScreenRefresh_t GetDefaultScreenRefreshFor(GrScreenResolution_t res)	
{
	GrScreenRefresh_t result = GR_REFRESH_75Hz;
	
	switch (fxgli_GetDefaultScreenRefresh())
	{
		case 60:
			return GR_REFRESH_60Hz;
		case 70:
			return GR_REFRESH_70Hz;
		case 72:
			return GR_REFRESH_72Hz;
		case 75:
			return GR_REFRESH_75Hz;
		case 80:
			return GR_REFRESH_80Hz;
		case 90:
			return GR_REFRESH_90Hz;
		case 100:
			return GR_REFRESH_100Hz;
		case 85:
			return GR_REFRESH_85Hz;
		case 120:
			return GR_REFRESH_120Hz;
		default:
			return result;
	}

}
static
void GetFullscreenResolution(const TGLIUtilsFullscreenDrawableData *drw,GrScreenResolution_t *res,GrScreenRefresh_t *refresh)
{
	if (drw->width <= 320 && drw->height <= 200 && IsScreenResolutionEnabled(GR_RESOLUTION_320x200))
		*res = GR_RESOLUTION_320x200;
	else if (drw->width <= 320 && drw->height <= 240 && IsScreenResolutionEnabled(GR_RESOLUTION_320x240))
		*res = GR_RESOLUTION_320x240;
	else if (drw->width <= 400 && drw->height <= 256 && IsScreenResolutionEnabled(GR_RESOLUTION_400x256))
		*res = GR_RESOLUTION_400x256;	
	else if (drw->width <= 512 && drw->height <= 256 && IsScreenResolutionEnabled(GR_RESOLUTION_512x256))
		*res = GR_RESOLUTION_512x256;
	else if (drw->width <= 512 && drw->height <= 384 && IsScreenResolutionEnabled(GR_RESOLUTION_512x384)) 
		*res = GR_RESOLUTION_512x384;
	else if (drw->width <= 640 && drw->height <= 200 && IsScreenResolutionEnabled(GR_RESOLUTION_640x200))
		*res = GR_RESOLUTION_640x200;	
	else if (drw->width <= 400 && drw->height <= 300 && IsScreenResolutionEnabled(GR_RESOLUTION_400x300))
		*res = GR_RESOLUTION_400x300;
	else if (drw->width <= 640 && drw->height <= 350 && IsScreenResolutionEnabled(GR_RESOLUTION_640x350))
		*res = GR_RESOLUTION_640x350;
	else if (drw->width <= 640 && drw->height <= 400 && IsScreenResolutionEnabled(GR_RESOLUTION_640x400))
		*res = GR_RESOLUTION_640x400;
	else if (drw->width <= 640 && drw->height <= 480 && IsScreenResolutionEnabled(GR_RESOLUTION_640x480))
		*res = GR_RESOLUTION_640x480;
	else if (drw->width <= 800 && drw->height <= 600 && IsScreenResolutionEnabled(GR_RESOLUTION_800x600))
		*res = GR_RESOLUTION_800x600;	
	else if (drw->width <= 856 && drw->height <= 480 && IsScreenResolutionEnabled(GR_RESOLUTION_856x480))
		*res = GR_RESOLUTION_856x480;
	else if (drw->width <= 960 && drw->height <= 720 && IsScreenResolutionEnabled(GR_RESOLUTION_960x720))
		*res = GR_RESOLUTION_960x720;
	else if (drw->width <= 1024 && drw->height <= 768 && IsScreenResolutionEnabled(GR_RESOLUTION_1024x768))
		*res = GR_RESOLUTION_1024x768;
	else if (drw->width <= 1280 && drw->height <= 1024 && IsScreenResolutionEnabled(GR_RESOLUTION_1280x1024))
		*res = GR_RESOLUTION_1280x1024;
	else if (drw->width <= 1600 && drw->height <= 1200 && IsScreenResolutionEnabled(GR_RESOLUTION_1600x1200))
		*res = GR_RESOLUTION_1600x1200;
	else 
		*res = GR_RESOLUTION_NONE;
	
	if (drw->freq == 0)
		*refresh = GetDefaultScreenRefreshFor(*res);
	else if (drw->freq <= 60)
		*refresh = GR_REFRESH_60Hz;
	else if (drw->freq <= 70)
		*refresh = GR_REFRESH_70Hz;
	else if (drw->freq <= 72)
		*refresh = GR_REFRESH_72Hz;
	else if (drw->freq <= 75)
		*refresh = GR_REFRESH_75Hz;
	else if (drw->freq <= 80)	
		*refresh = GR_REFRESH_80Hz;
	else if (drw->freq <= 85)
		*refresh = GR_REFRESH_85Hz;
	else if (drw->freq <= 90)
		*refresh = GR_REFRESH_90Hz;
	else if (drw->freq <= 100)
		*refresh = GR_REFRESH_100Hz;
	else if (drw->freq <= 120)
		*refresh = GR_REFRESH_120Hz;
	else 
		*refresh = GR_REFRESH_NONE;

}

static GLenum AttachFullscreenDrawable(TFXContext* ctx,const TGLIUtilsFullscreenDrawableData *fullscreen)
{
	GrScreenResolution_t	res;
	GrScreenRefresh_t		freq;
	GLint					attrib[32];
	
	if (ctx->ctx != NULL)	/* We've already attached the drawable */
		return (GLenum)GLI_NO_ERROR;
	
	GetFullscreenResolution(fullscreen,&res,&freq);
	if ((res == GR_RESOLUTION_NONE) || (freq == GR_REFRESH_NONE))
		return (GLenum)GLI_BAD_DRAWABLE;
	
	current_context  = ctx;
		
	fxgli_PixelFormatToFXAttribList(&ctx->fmt,attrib);
	
	ctx->ctx = fxMesaCreateContext(0,res,freq,attrib);

	if (!ctx->ctx)
		return (GLenum)GLI_BAD_ALLOC;/* ToDo: Find a better way of signaling this error!!! */

	ctx->drawable.type = GLI_FULLSCREEN;
	ctx->drawable.data.fullscreen = *fullscreen;
	
	
	ctx->ctx->swapInterval = ctx->swap_interval;
	ctx->ctx->maxPendingSwapBuffers = 4;
	if (ctx->gamma_value!=0)
		FX_grGammaCorrectionValue((ctx->gamma_value)*1.0/(GLI_3DFX_GAMMA_SCALE));

	setupHacks();

	if (fxgli_GetDefaultDitherMode() != -1)
	{
		switch (fxgli_GetDefaultDitherMode())
		{
			case 0: grDitherMode( GR_DITHER_DISABLE ); break;
			case 2: grDitherMode( GR_DITHER_2x2 ); break;
			case 4: grDitherMode( GR_DITHER_4x4 ); break;
		}
	}


	fxMesaMakeCurrent(ctx->ctx);
	
	return (GLenum)GLI_NO_ERROR;
}
static GLenum DetachDrawable(TFXContext *ctx)
{
	if (ctx->ctx)
		fxMesaDestroyContext(ctx->ctx);
	
	current_context = NULL;
	
	ctx->ctx = NULL;
	ctx->drawable.type = GLI_NONE;
	
	return (GLenum)GLI_NO_ERROR;
}


static void doDrawEmulatedCursor(TFXContext *ctx)
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
	int width = ctx->drawable.data.fullscreen.width;
	int height = ctx->drawable.data.fullscreen.height;
	
	if (ctx->drawable.type != GLI_FULLSCREEN)
		return;

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
							FX_grLfbWriteRegion(GR_BUFFER_BACKBUFFER,x,y,GR_LFB_SRC_FMT_888,1,1,64,curs[i][j]);
					}
				}
			}
		}	
}


/***********************************************************************************************
 * 
 * gliSwapBuffers: Swaps the context's buffers.
 *
 ***********************************************************************************************/
#if MESA_3DFX_PROFILE || GLI_DEBUG
	GLint framenum = 0;
#endif
GLenum gliSwapBuffers(GLIContext ctx)
{
	TFXContext *context;
	
	context = (TFXContext*)ctx;
	
    if (context->ctx == NULL)
    	return (GLenum)GLI_BAD_CONTEXT;
    
    {
    	fxMesaMakeCurrent(context->ctx);	/* This might not neccesary */
    	/*
    	 * If we need to emulate the cursor draw it here:
    	 */
    	if (context->emulate_cursor)
    		doDrawEmulatedCursor(context);
    	fxMesaSwapBuffers();
    }
    
    #if MESA_3DFX_PROFILE || GLI_DEBUG
    	framenum++;
    #endif
    
    #if MESA_3DFX_PROFILE
    	if (framenum == 100)
    	{
    		SysBeep(3);
    		ProfilerInit(collectDetailed,PPCTimeBase,400,50);
    	}
    	if (framenum == 600)
    	{
			ProfilerDump("\pMesa.prof");
			ProfilerTerm();
			SysBeep(3);
    	}
    #endif
    
    return (GLenum)GLI_NO_ERROR;
}

/***********************************************************************************************
 * 
 * gliAttachDrawable: Attach a drawable to the OpenGL Context - ctx.
 *
 ***********************************************************************************************/
GLenum	gliAttachDrawable(GLIContext ctx,GLint drawable_type,const GLIDrawable *drw)
{
	TFXContext *context = (TFXContext*)ctx;
	GLenum	   succes;
	/*
	** Initial error check:
	*/
	if (context == NULL)
		return (GLenum)GLI_BAD_CONTEXT;
	
	if (context->renderer_id != (GLenum)GLI_RENDERER_MESA_3DFX_ID)
		return (GLenum)GLI_BAD_CONTEXT;
	
	if (drawable_type == (GLenum)GLI_OFFSCREEN)
		return (GLenum)GLI_BAD_DRAWABLE;
		
	/*
	** Main code:
	*/
	if (drawable_type == GLI_FULLSCREEN && drw != NULL)
	{
		succes = AttachFullscreenDrawable(context,(TGLIUtilsFullscreenDrawableData*)drw);
	}
	else if (drawable_type == GLI_WINDOW)
	{
		/* Calculate the dimensions of the drawable! */
		TGLIUtilsFullscreenDrawableData fullscreen;
		
		fullscreen.width 	= ((CGrafPtr)drw)->portRect.right-((CGrafPtr)drw)->portRect.left;
		fullscreen.height 	= ((CGrafPtr)drw)->portRect.bottom-((CGrafPtr)drw)->portRect.top;
		fullscreen.device 	= 1;
		fullscreen.freq 	= 0;
		succes = AttachFullscreenDrawable(context,&fullscreen);
	}
	else if (drawable_type == GLI_NONE || drw == NULL)
	{
		succes = DetachDrawable(context);
	}
	
	return succes;
}
