/*
	File:		fxgli_hacks.c

	Contains:	Contains hacks for the FX driver, like mouse emulation+fixing bugs in glide.

	Written by:	miklos

	Copyright:	Copyright © 1999 Mikl—s Fazekas. All rights reversed.

	Change History (most recent first):

         <3>      7/4/99    miklos  Revision.
         <2>     6/19/99    miklos  Initial revision.
         <1>     6/19/99    miklos  Initial revision.
*/
#include "fxgli.h"
#include "fxglidew.h"

/********************************************************************************
 *
 * AGLAlloc+AGLFree code.
 *
 ********************************************************************************/
char* AGLAlloc(GLint size)
{
	return malloc(size);
}

void AGLFree(char *ptr)
{
	free(ptr);
}

void *myAllocateFX(GLint size)
{
   int i;
   char ch = 0x00;
   char *result = glmMalloc(size);
   if (result)
   {
   	   for (i = 0; i < size; i++)
   	   {
           result[i] = ch;
       }
   }
   else
   {
      DebugStr("\pMalloc failed!!!!");
   }
   return result;
}
void *myAllocate(GLint size)
{
   int i;
   char ch = 0x00;
   char *result = glmMalloc(size);
   if (result)
   {
   	   for (i = 0; i < size; i++)
   	   {
           result[i] = ch;
       }
   }
   else
   {
      DebugStr("\pMalloc failed!!!!");
   }
   return result;
}

/*
 * The following codes is for fixing the bug founds in the MC versions of MacGlide!!!
 */
#if GLI_3DFX_FIXGRLFBREADREGIONBUG
	#undef grLfbReadRegion
#endif

 
#if GLI_3DFX_FIXGRLFBREADREGIONBUG

#include <glide.h>

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
	FX_grLfbWriteRegion(GR_BUFFER_FRONTBUFFER,0,0,GR_LFB_SRC_FMT_888,5,1,20,black);
	/* Read it out using */
	FX_grLfbReadRegion(GR_BUFFER_FRONTBUFFER,0,0,2,1,4,data);

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


#ifdef grSstIdle
	#undef grSstIdle
	extern void grSstIdle(void);
#endif


void (*__grSstIdle)(void);

static void skipIdle(void)
{
} 

void setupHacks(void)
{
	/*
	 * Setup Disable Finish hack.
	 */
	if (MCFG_getEnv(kEnv_IgnoreGLFinish) != NULL)
	{
		__grSstIdle = skipIdle;
	}
	else
		__grSstIdle = FX_grFlush;
	/*
	 * Setup grlfbFixRegioncall.
	 */
#if GLI_3DFX_FIXGRLFBREADREGIONBUG
	setupFixGrLfbReadRegionCall();
#endif
}


