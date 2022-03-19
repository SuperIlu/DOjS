#include "maglConfig.h"
#include "maglMemory.h"
#include "maglError.h"
#include "maglRenderers.h"

#include <agl.h>
#include <gliMesa.h>

int MAGL_GetFirstRenderer(void)
{
   return 1;
}
int MAGL_GetNextRenderer(int inGLIRenderer)
{ 
   return 0;
}

int MAGL_GetNumOfRenderers(void)
{
   return 1;
}

int	MAGL_RendererID2Renderer(GLint	inGliRendererID)
{
   return 1;
}
GLenum MAGLCall_gliChoosePixelFormat(
			GLIPixelFormat 		**fmt_ret, 
			const GLIDevice 	*device, 
			GLint 			ndevs, 
			const GLint 		*attribs)
{
   return gliChoosePixelFormat(fmt_ret,device,ndevs,attibs);
}
GLenum MAGLCall_gliDestroyPixelFormat(GLIPixelFormat *fmt);
{
  return gliDestroyPixelFormat(fmt);
}

GLenum	   MAGLCall_gliCreateContext(
			GLIContext *ctx_ret, 
			const GLIPixelFormat *fmt, 
			GLIContext share_list)
{
   return gliCreateContext(ctx_ret,fmt,share_list);
}

GLenum	   MAGLCall_gliDestroyContext(
			int				inRenderer,
			GLIContext 			inContext)
{
   return gliDestroyContext(inContext);
}
GLenum	   MAGLCall_gliAttachDrawable(
			int			inRenderer,
			GLIContext 		inContext,
			const TGLIDrawable	inDrawable)
{
   return gliAttachDrawable(inContext,inDrawable);
}

GLenum	MAGLCall_gliQueryRendererInfo(
			int inRenderer,
			GLIRendererInfo **info_ret, 
			const GLIDevice *dev, 
			GLint ndevs);
{
   return gliQueryRendererInfo(info_ret,dev,ndevs);
}

GLenum MAGLCall_mgliSetInteger(
			int			inRenderer,
			GLIContext		src,
			GLint			pname,
			const GLint		*value)
{
	MAGL_Assert(IsValidRenderer(inRenderer));

	return gliSetInteger(
}
GLenum MAGLCall_mgliGetInteger(
							int							inRenderer,
							GLIContext					src,
							GLint						pname,
							GLint						*value)
{
	MAGL_Assert(IsValidRenderer(inRenderer));

	return gGliApiTables[inRenderer-1].mGetInteger(gGliApiTables[inRenderer-1].mRefContext,src,pname,value);
}
