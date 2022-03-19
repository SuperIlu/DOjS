/*
	File:		fxgli.c

	Contains:	GLI interface on the to of fxApi.

	Written by:	Fazekas Mikl—s

	Copyright:	Copyright(C) 1995-98 Miklos Fazekas
			E-Mail: boga@valerie.inf.elte.hu
			WWW: http://valerie.inf.elte.hu/~boga/Mesa.html
				
			Part of the Mesa 3D Graphics Library for MacOS.

	Change History (most recent first):
	
        <1+>      6/8/99    ???     Triangle debugging moved.
         <1>      6/8/99    ???     Initial revision.
        <11>      6/8/99    ???     Initial revision.
        <10>     5/15/99    miklos  Added better check for buggy Glide.
       	<8+>		    miklos  Bug fixes for V2-s Voodoo driver.
         <8>      5/3/99    miklos  Revsion for 3.1b10.
         <7>     4/28/99    miklos  Added triangle checking.
         <5>     4/27/99    miklos  Added a hack for TEXTURE_MEMORY.
        <1>      7/2/98     miklos  Initial revision.
*/

#include "fxgli.h"
#include "gli.h"
#include "glm.h"
#include "macros.h"
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


  char *AGLAlloc(int size)
  {
  	return malloc(size);
  }
  void AGLFree(char* ptr)
  {
  	free(ptr);
  }


#if GLI_3DFX_FIXGRLFBREADREGIONBUG
	#undef grLfbReadRegion
#endif

#include <glide.h>


/***********************************************************************************************
 *
 * Type Definitions
 *
 ***********************************************************************************************/





/*
 * The following codes is for fixing the bug founds in the MC versions of MacGlide!!!
 */
#if GLI_3DFX_FIXGRLFBREADREGIONBUG

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
void setupFixGrLfbReadRegionCall()
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





GLboolean gliGetVersion(
		GLint *major, GLint *minor, GLint *rendID)
{
     FILE *f;
   printf("Hello!");
   *major = 2;
   *minor = 0;
   *rendID = GLI_RENDERER_MESA_3DFX;
   
 
   

   
   return GL_TRUE;
}

/* ToDo */









GLenum gliCopyAttributes(void)
{
    printf("Hello!");
}

void __initializeFX(void)
{
	MCFG_readMesaConfigurationFile();
}
void __main(void)
{
  Debugger();
  printf("Hello!");
}

