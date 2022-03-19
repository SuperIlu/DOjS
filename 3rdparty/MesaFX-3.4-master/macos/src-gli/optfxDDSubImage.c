#include "gl.h" 
#include "fxdrv.h"

extern void fxTexInvalidate(GLcontext *ctx, struct gl_texture_object *tObj);

GLboolean fastFxDDTexSubImage2D(
	GLcontext *ctx,
	GLenum target, GLint level,
        GLint xoffset, GLint yoffset,
        GLsizei width, GLsizei height,
        GLenum format, GLenum type,
        const GLvoid *pixels )
{
   struct gl_texture_unit *texUnit = &ctx->Texture.Unit[ctx->Texture.CurrentUnit];
   struct gl_texture_image *destTex;
   
   if (target != GL_TEXTURE_2D || format != GL_RGBA || type != GL_UNSIGNED_BYTE)
   	return GL_FALSE;
   if (level < 0 || level > ctx->Const.MaxTextureLevels)
   	return GL_FALSE;
   	
   destTex = texUnit->CurrentD[2]->Image[level];

   if (!destTex)
   	return GL_FALSE;
   
   if (xoffset < -((GLint)destTex->Border)) {
      return GL_FALSE;
   }
   if (yoffset < -((GLint)destTex->Border)) {
      return GL_FALSE;
   }
   if (xoffset + width > (GLint) (destTex->Width + destTex->Border)) {
      return GL_FALSE;
   }
   if (yoffset + height > (GLint) (destTex->Height + destTex->Border)) {
      return GL_FALSE;
   }
   
   /* Here comes fxDDSubImage */
   {
   	fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
        tfxTexInfo *ti;
  	GrTextureFormat_t gldformat;
  	struct gl_texture_object *tObj = texUnit->CurrentD[2];
  	const struct gl_texture_image *image = destTex;
  	int wscale,hscale;
  	tfxMipMapLevel *mml;
   	GLint internalFormat =texUnit->CurrentD[2]->Image[level]->IntFormat;
   	
   	if(target!=GL_TEXTURE_2D)
    	   return GL_FALSE;
	if (tObj == NULL)
	   return GL_FALSE;
  	if(!tObj->DriverData)
    	   return GL_FALSE;

 	ti=(tfxTexInfo *)tObj->DriverData;
  	   mml=&ti->tmi.mipmapLevel[level];
	
  	fxTexGetFormat(internalFormat,&gldformat,NULL);
  	
  	if(mml->glideFormat!=gldformat) 
  	   return GL_FALSE;
  	 
  	fxTexGetInfo(image->Width,image->Height,NULL,NULL,NULL,NULL,NULL,NULL,&wscale,&hscale);
    	if((wscale!=1) || (hscale!=1)) 
    	   return GL_FALSE;
    	 
    	if (!mml->translated)	
	   return GL_FALSE;
	   
	if (internalFormat != 3)
	   return GL_FALSE;
	   
	/* 
	 * Unpacking:
	 */
	if (ctx->Unpack.ImageHeight != 0)
	   return GL_FALSE;
	if (ctx->Unpack.SkipRows != 0)
	   return GL_FALSE;
	if (ctx->Unpack.SkipImages != 0)
	   return GL_FALSE;
	 
	
	{
	     /* Do the conversion here */
	     int x,y;
	     GLuint *src;
	     GLushort *dst;
	     int simgw,dimgw;
	     GLint rowbytes = ctx->Unpack.RowLength*4;
	     GLushort *destimg = mml->data;
	     if (rowbytes == 0)
	    	rowbytes = width*4;
	     
	     src = (GLuint*)pixels;
	     dst=destimg+(yoffset*image->Width+xoffset);
	     simgw=(image->Width-width)*4;
	     dimgw=image->Width-width;
	     for (y = 0; y < height; y++)
	     {
	        GLubyte *fsrc;
	 	fsrc = (GLubyte*)src;
	        for (x= 0; x < width ; x++)
	        {
	        	GLuint r,g,b;
	     		r = (src[x] >> 24) & 0xFF;
	     		g = (src[x] >> 16) & 0xFF;
	     		b = (src[x] >> 8) & 0xFF;
	     	
	     		*dst++ = ((0xf8 & r) << (11-3))  |
            	      	 ((0xfc & g) << (5-3+1)) |
                         ((0xf8 & b) >> 3); 
	    	}
	    	dst  += dimgw;
	    	fsrc += rowbytes;
	    	src = (GLuint*)fsrc;
	     }  
	}
	   
	if (ti->validated && ti->tmi.isInTM)
	   fxTMReloadSubMipMapLevel(fxMesa,tObj,level,yoffset,height);
 	else
    	   fxTexInvalidate(ctx,tObj);
    return GL_TRUE;
   }
}
	
	