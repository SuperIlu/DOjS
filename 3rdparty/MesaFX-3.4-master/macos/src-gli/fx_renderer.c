#include "fxgli.h"
#include "glide.h"

/**************************************************************************************
 *
 * @name: 		getGlideMemory
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
	int					i;
	grGlideInit();
	if (!grSstQueryBoards( &hwconfig ))
		return GLI_HARDWARE_ERROR;
	if (hwconfig.num_sst == 0)
		return GLI_HARDWARE_ERROR;
	
	/* Currently only supports the First Voodoo hardware! */
	switch (hwconfig.SSTs[current].type)
	{
		case GR_SSTTYPE_VOODOO:
			VoodooConfig vconfig = hwconfig.SSTs[current].sstBoard.VoodooConfig;
			*videoMemory = vconfig.fbRam*1024*1024;
			
			if (vconfig.sliDetect)
				*videoMemory *= 2;
			
			*textureMemory = getGlideTextureMemory(vconfig.nTexelfx,vconfig.tmuConfig)*1024*1024;				
			break;
		case GR_SSTTYPE_VOODOO2:
			break;
		case GR_SSTTYPE_SST96:
			break;
		case GR_SSTTYPE_AT3D:
			break;
	}
	
}

/* GLI_QUERY_RENDERER_INFO */
GLenum gliQueryRendererInfo(GLIRendererInfo **info_ret, const GLIDevice *dev, GLint ndevs)
{
 
   GLIRendererInfo 	*info = NULL;
   GLenum 			result;   
   
   if (!info_ret)
   	  return GLI_BAD_POINTER;
   if (dev == NULL && ndev != 0)
   	  return GLI_BAD_GDEV;	  
   /* Allocate pixel format */
   info = (GLIRendererInfo *) malloc(sizeof(GLIRendererInfo));

	
   info->next_renderer_info     = NULL;

   /* Renderer info */
   info->renderer_id            = GLI_RENDERER_MESA_3DFX;

   /* OS feature bits */
   info->os_support     		= /*GLI_FULLSCREEN_BIT | */ GLI_ACCELERATED_BIT;
   
   if (IsEmulateWindowRendering())
   		info->os_support   |= GLI_WINDOW_BIT;
   if (IsEmulateCompliance())
   		info->os_support   |= GLI_COMPLIANT_BIT;

   /* buffer modes */
   info->buffer_modes           = GLI_SINGLEBUFFER_BIT | GLI_DOUBLEBUFFER_BIT | GLI_MONOSCOPIC_BIT;



   /* Frame buffers */
   info->color_modes            = GLI_RGB565_BIT;
   info->accum_modes            = GLI_NONE;
   info->depth_modes            = GLI_0_BIT | GLI_16_BIT;
   info->stencil_modes          = GLI_NONE;

   /* Aux buffers */
   info->max_aux_buffers        = 0;
   
   result = getGlideMemory(&info->video_memory_size,&info->texture_memory_size);
   if (!result)
   {
   		free(info);
   		return result;
   }
   
   *info_ret			= info;
	
   return GLI_NO_ERROR;
}