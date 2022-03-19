/*
	File:		fxgli_renderer.c

	Contains:	Query RendererInfo for OpenGL. 

	Written by:	miklos

	Copyright:	Copyright ©

	Change History (most recent first):

         <2>      7/4/99    miklos  Revision.
         <2>      7/4/99    miklos  Revision.
*/


#include "fxgli.h"
#include "config.h"
#include "glide.h"
#include "fxglidew.h"

#include <stdio.h>
#include <stdlib.h>

/**************************************************************************************
 *
 * getGlideMemory (internal)
 * 
 * @short: 		Queries for the video+texture memory in glide.
 * @returns: 	GLI_NO_ERROR if succeed, GLI_HARDWARE_ERROR otherwise. 
 * @params:		textureMemory	- the texture memory in bytes.
 *				videoMemory		- the video memory in bytes.
 **************************************************************************************/
static int getGlideTextureMemory(int ntextelfx,GrTMUConfig_t textelcfg[])
{
	int result = 0;
	int i;
	
	for (i = 0; i < ntextelfx; i++)
	{
		result += textelcfg[i].tmuRam;
	}
	return result;
}

static GLenum getGlideMemory(GLint *textureMemory,GLint *videoMemory)
{
	GrHwConfiguration	hwconfig;
	int					current = 0;
	
	/* ToDo */
	*textureMemory = 4048*1024;
	*videoMemory = 4048*1024;
	
	grGlideInit();
	if (!FX_grSstQueryHardware( &hwconfig ))
		return (GLenum)GLI_HARDWARE_ERROR;
	if (hwconfig.num_sst == 0)
		return (GLenum)GLI_HARDWARE_ERROR;
		
	
	
	/* Currently only supports the First Voodoo hardware! */
	switch (hwconfig.SSTs[current].type)
	{
		case GR_SSTTYPE_VOODOO:
		{
			GrVoodooConfig_t vconfig = hwconfig.SSTs[current].sstBoard.VoodooConfig;
			*videoMemory = vconfig.fbRam*1024*1024;
			
			if (vconfig.sliDetect)
				*videoMemory *= 2;
			
			*textureMemory = getGlideTextureMemory(vconfig.nTexelfx,vconfig.tmuConfig)*1024*1024;				
			break;
		}
		case GR_SSTTYPE_Voodoo2:
		{
			GrVoodoo2Config_t vconfig = hwconfig.SSTs[current].sstBoard.Voodoo2Config;
			*videoMemory = vconfig.fbRam*1024*1024;
			
			if (vconfig.sliDetect)
				*videoMemory *= 2;
			
			*textureMemory = getGlideTextureMemory(vconfig.nTexelfx,vconfig.tmuConfig)*1024*1024;				
			break;
		
		}
		case GR_SSTTYPE_SST96:
		{
			GrSst96Config_t vconfig = hwconfig.SSTs[current].sstBoard.SST96Config;
			*videoMemory = vconfig.fbRam*1024*1024;
			
			*textureMemory = getGlideTextureMemory(1/*TODO*/,&vconfig.tmuConfig)*1024*1024;				
			break;
		}
		case GR_SSTTYPE_AT3D:/* ToDo */
		{
			*videoMemory 	= 0;
			*textureMemory 	= 0;
			break;
		}
	}
	
	if (fxgli_GetDefaultTextureMemorySize() != 0)
		*textureMemory = fxgli_GetDefaultTextureMemorySize();
	if (fxgli_GetDefaultVideoMemorySize() != 0)
		*videoMemory = fxgli_GetDefaultVideoMemorySize();
	return (GLenum)GLI_NO_ERROR;
}

/**************************************************************************************
 *
 * depthToGliDepth (internal)
 * 
 * @short: 		Returns the given depth as a GLI_XXX_DEPTH constant.
 *				-1 is returned if GLI doesn't have such constant.
 *
 **************************************************************************************/
static GLenum	depthToGliDepth(int depth)
{
	switch(depth)
	{
		case 0: return (GLenum)GLI_0_BIT;
		case 1: return (GLenum)GLI_1_BIT;
		case 2: return (GLenum)GLI_2_BIT;
		case 3: return (GLenum)GLI_3_BIT;
		case 4: return (GLenum)GLI_4_BIT;
		case 5: return (GLenum)GLI_5_BIT;
		case 6: return (GLenum)GLI_6_BIT;
		case 8: return (GLenum)GLI_8_BIT;
		case 10: return (GLenum)GLI_10_BIT;
		case 12: return (GLenum)GLI_12_BIT;
		case 16: return (GLenum)GLI_16_BIT;
		case 24: return (GLenum)GLI_24_BIT;
		case 32: return (GLenum)GLI_32_BIT;
		case 48: return (GLenum)GLI_48_BIT;
		case 64: return (GLenum)GLI_64_BIT;
		case 128: return (GLenum)GLI_128_BIT;
		default: return (GLenum)-1;
	}
}
/**************************************************************************************
 *
 * gliGetVersion (export)
 * 
 * @short: 		Queries for renderer info.
 * @returns: 	GLI_TRUE, if suceed, GL_FALSE if no Voodoo is installed. (ToDo)
 * @params:		major		
 *				minor			
 *				rendID	
 *
 **************************************************************************************/
GLboolean gliGetVersion(
		GLint *major, GLint *minor, GLint *rendID)
{
   *major = 2;
   *minor = 0;
   *rendID = GLI_RENDERER_MESA_3DFX_ID;
   
   MCFG_readMesaConfigurationFile("MesaSettings");
   return GL_TRUE;
}
/**************************************************************************************
 *
 * gliQueryRendererInfo (export)
 * 
 * @short: 		Queries for renderer info.
 * @returns: 	GLI_NO_ERROR if succeed, GLI_HARDWARE_ERROR,GLI_BAD_ALLOC otherwise. 
 * @params:		info_ret		- the info to be returned
 *				dev				- the graphics devices list
 *				ndevs			- the number of devices
 *
 **************************************************************************************/
GLenum gliQueryRendererInfo(GLIRendererInfo **info_ret, const GLIDevice *dev, GLint ndevs)
{
 
   GLIRendererInfo 	*info = NULL;
   GLenum 			result;   
   
   /*
   ** Initial error checking
   */
   if (!info_ret)
   	  return (GLenum)GLI_BAD_POINTER;
   if (dev == NULL && ndevs != 0)
   	  return (GLenum)GLI_BAD_GDEV;	  
   
   /*
   ** Main part
   */
   
   /* Allocate pixel format */
   info = (GLIRendererInfo *) malloc(sizeof(GLIRendererInfo));

	
   info->next_renderer_info     = NULL;

   /* Renderer info */
   info->renderer_id            = GLI_RENDERER_MESA_3DFX_ID;

   /* OS feature bits */
   info->os_support     		= GLI_ACCELERATED_BIT;
   
   if (fxgli_IsEmulateWindowRendering())
   		info->os_support   |= GLI_WINDOW_BIT;
  
   info->os_support   |= GLI_FULLSCREEN_BIT;
   		
   if (fxgli_IsEmulateCompliance())
   		info->os_support   |= GLI_COMPLIANT_BIT;

   /* buffer modes */
   info->buffer_modes           = GLI_SINGLEBUFFER_BIT | GLI_DOUBLEBUFFER_BIT | GLI_MONOSCOPIC_BIT;



   /* Frame buffers */
   info->color_modes            = GLI_RGB565_BIT /*| GLI_RGB565_A8_BIT*/;	/* This might not correct (?) */
   info->accum_modes            = GLI_NONE;
   info->depth_modes            = GLI_0_BIT | GLI_16_BIT;
   info->stencil_modes          = GLI_NONE;								/* Me might want to add software emulated stencil buffering? */

   /* Aux buffers */
   info->max_aux_buffers        = 0;
   info->magic 					= 0;
   
   result = getGlideMemory(&info->video_memory_size,&info->texture_memory_size);
   if (result != GLI_NO_ERROR)
   {
   		free(info);
   		return result;
   }
   
   *info_ret			= info;
	
   return (GLenum)GLI_NO_ERROR;
}

GLenum gliDestroyRendererInfo(GLIRendererInfo *info)
{
    /*
    ** Initial error checking
    */
  	if (!info)
		return (GLenum)GLI_BAD_POINTER;
	
    /*
    ** Main part
    */ 
	free((char*) info);
	return (GLenum)GLI_NO_ERROR;
}
