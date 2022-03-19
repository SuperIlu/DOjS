/* s3mesa.c - S3/Mesa interface */

/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


/* This driver is written by S3 Inc. Contact rkoduri@s3.com with questions
 * comments and bugs.
 *
 * The structure of this driver is based on the 3Dfx fxMesa driver by David
 * Bucciarelli (with assistance from Henri Fousse, Jack Palevich, Diego Picciani,
 * Brian Paul)
 */

/* Bugs found in Mesa:
 * Pointers being changed that aren't required
 * Texture coordinate clipping is broken?
 * A lot of the driver functions aren't checked for NULL
 */

#ifdef S3


#include <windows.h>  /* is this really needed? */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "context.h"
#include "macros.h"
#include "matrix.h"
#include "texture.h"
#include "types.h"
#include "vb.h"
#include "xform.h"
#include "s3mesa.h"


#include <ddraw.h>



/* Different debug modes - any extra debug strings should go under one of these */
//#define DEBUG_INIT				// Debug initialisation code
//#define DEBUG_SURFACES			// Debug all DirectDraw and S3 surface data
//#define DEBUG_PRIMITIVES		// Debug primitive data - VERY extensive!
//#define DEBUG_TEXTURES			// Debug textures
//#define DEBUG_STATE				// Debug all renderer state not involved in texturing
//#define DEBUG_GL				// Debug GL interface




LPDIRECTDRAW gpDD;
S3DTK_LPFUNCTIONLIST s3f;

#define MAXNUM_TEX	512

#define PACKEDCOLOR(r,g,b,a) (( ((unsigned int)(a))<<24 )|( ((unsigned int)(b))<<16 )|( ((unsigned int)(g))<<8 )|(r))

#define FUNC_DEPTH	0x01
#define FUNC_SMOOTH	0x02
#define FUNC_TEX_DECAL	0x04
#define FUNC_TEX_MOD	0x08


/* Information stored internally about a texture */
typedef struct {
	int width, height;			// obvious I hope
	int mipmaplevels;			// Number of mipmap levels in this texture
	void *data;					// Pointer to converted data
	int size;					// Texture size

	int transparent;			// Set if this texture needs transparency enabled
	int have_mipmap;			// Set if have a mipmap for this texture

	LPDIRECTDRAWSURFACE vram_surface;	// Surface in texture cache
	int on_card;				// Set if currently uploaded to card

	GLenum minfilter;			// Texture filtering
	GLenum magfilter;
	ULONG filter;				// S3DTK filtering code

	S3DTK_SURFACE s3_surface;	// S3 surface associated with this texture
} texinfo;


/*
 * All the things needed for
 */
struct s3_mesa_context {
	GLcontext *gl_ctx;             /* the core Mesa context */
	GLvisual *gl_vis;              /* describes the color buffer */
	GLframebuffer *gl_buffer;      /* the ancillary buffers */

	GLint width, height;           /* size of color buffer */
	GLboolean double_buffer;

	HWND hwnd;
	LPDIRECTDRAWSURFACE front_surface, back_surface, z_surface;
	LPDIRECTDRAWCLIPPER clipper;
	int fullscreen;
	int screen_width, screen_height, window_width, window_height;
	S3DTK_SURFACE s3_draw, s3_z;

	GLenum current_buffer;	// Current draw buffer

	dword color;
	dword clearc;
	dword cleara;

	/* Texture management */
	texinfo ti[MAXNUM_TEX];
	int currenttex;

	/* The texture cache: a list of which textures are currently in vram. */
	int texture_cache[MAXNUM_TEX];	// the texObject number of this entry
	int texture_cache_head;
	int texture_cache_tail;

	float wscale, hither, yon;
	float fog_start, fog_end;
	float fog_const;				// Constant for calculating fog addresses

	ULONG vram_base;			// PHYSICAL address, in case things turn up in different spaces

	int renderdisabled;			// bitvector of all the different things that can block rendering

	/* Output vertices */
	S3DTK_VERTEX_TEX vertex_tex[VB_SIZE];
};

s3MesaContext Currents3MesaCtx=NULL;
LPDIRECTDRAWSURFACE front_surface;

static void BuildDispatchTable(GLcontext *ctx);
static void SetS3RenderState(GLcontext *ctx);


int gamma_table[256];



/*
 * Correct LinearToPhysical function - this works for both 95 and NT
 */
ULONG LinearToPhysical(ULONG linear)
{
    ULONG low12bits;
    low12bits = linear & 0x00000fff;
    return(S3DTK_LinearToPhysical(linear & 0xfffff000) + low12bits);
}



/*
 * The ViRGE usually has quite dark output. Adding this will
 * prescale the textures on the way in, which helps a lot for Quake.
 */
void BuildGammaTable(void)
{
	int i;
	float val;
//	float gamma = 1 / 1.4;
	float gamma = 1.0f;

	for(i=0;i<256;i++) {
		val = ((float) i) / 255;
		if (gamma != 1.0f)
			val = pow(val, gamma);

		gamma_table[i] = (int) (255.0f*val);
	}

}



void CleanExit(int n)
{
	if (s3f) {
		S3DTK_DestroyRenderer(&s3f);
		S3DTK_ExitLib();
	}
	if (gpDD)
		IDirectDraw_Release(gpDD);
	exit(n);
}











/* DIRECT DRAW interface */


/*
 * Allocate the front and back buffers for fullscreen (EXCLUSIVE) operation
 */
HRESULT DDSetupFullscreenSurfaces(  LPDIRECTDRAWSURFACE *front_surface,
									LPDIRECTDRAWSURFACE *back_surface)
{
	HRESULT result;
	DDSURFACEDESC ddsd;
	DDSCAPS ddsCaps;

	/* Create a primary surface with one back buffer */
	memset(&ddsd, 0, sizeof(DDSURFACEDESC));
	ddsd.dwSize = sizeof(DDSURFACEDESC);
	ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_COMPLEX | DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_VIDEOMEMORY;
	ddsd.dwBackBufferCount = 1;
	if ((result = IDirectDraw_CreateSurface(gpDD, &ddsd, front_surface, NULL)) != DD_OK)
		return(result);

	/* Pick up the back buffer */
	memset(&ddsCaps, 0, sizeof(DDSCAPS));
	ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;
	if (IDirectDrawSurface_GetAttachedSurface(*front_surface, &ddsCaps, back_surface) != DD_OK)
		return(result);

	return(DD_OK);
}


/*
 * Pick up the Windows primary surface for windowed mode
 */
HRESULT DDSetupPrimarySurface(LPDIRECTDRAWSURFACE *front_surface)
{
	HRESULT result;
	DDSURFACEDESC ddsd;

	memset(&ddsd, 0, sizeof(DDSURFACEDESC));
	ddsd.dwSize = sizeof(DDSURFACEDESC);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;
	if ((result = IDirectDraw_CreateSurface(gpDD, &ddsd, front_surface, NULL)) != DD_OK)
		return(result);
}


/*
 * Allocate a back buffer
 */
HRESULT DDSetupBackSurface(LPDIRECTDRAWSURFACE *back_surface, int w, int h)
{
	HRESULT result;
	DDSURFACEDESC ddsd;

	/* Create a back buffer */
	memset(&ddsd, 0, sizeof(DDSURFACEDESC));
	ddsd.dwSize = sizeof(DDSURFACEDESC);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
	ddsd.dwWidth = w;
	ddsd.dwHeight = h;
	if ((result = IDirectDraw_CreateSurface(gpDD, &ddsd, back_surface, NULL)) != DD_OK)
		return(result);

	return(DD_OK);
}




/*
 * Allocate a Z-buffer surface
 *
 * Recently modified to work on NT.
 */
HRESULT DDSetupZSurface(LPDIRECTDRAWSURFACE *surface, int w, int h)
{
	HRESULT result;
	DDSURFACEDESC ddsd;

	/* Allocate a Z surface in video memory so the card can access it */
	memset(&ddsd, 0, sizeof(DDSURFACEDESC));
 	ddsd.dwSize = sizeof(DDSURFACEDESC);

#if 0
	/* Old method; this is what it _should_ be, but doesn't work on NT, because
	 * NT currently doesn't have Direct3D and so doesn't support Z-buffer allocation
	 * through DirectDraw */
 	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_ZBUFFERBITDEPTH;
 	ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;
	ddsd.dwZBufferBitDepth = 16;
#else
	/* This works on 95 and NT, although it looks a bit silly it doesn't matter as
	 * we're not using Direct3D */
 	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
 	ddsd.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY;
	ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
	ddsd.ddpfPixelFormat.dwFlags  = DDPF_RGB ; 
	ddsd.ddpfPixelFormat.dwFourCC = BI_RGB ;
	ddsd.ddpfPixelFormat.dwRBitMask = 0x7c00 ; 
	ddsd.ddpfPixelFormat.dwGBitMask = 0x03e0; 
	ddsd.ddpfPixelFormat.dwBBitMask = 0x001f ; 
#endif

	ddsd.dwWidth = w;
	ddsd.dwHeight = h;
 	if ((result = IDirectDraw_CreateSurface(gpDD, &ddsd, surface, NULL)) != DD_OK)
		return(result);

	return(DD_OK);
}



/*
 * Set up a surface to be used as a texture
 */
HRESULT DDSetupTextureSurface(LPDIRECTDRAWSURFACE *surface, int w, int h, int s3format)
{
	HRESULT result;
	DDSURFACEDESC ddsd;

	memset(&ddsd, 0, sizeof(DDSURFACEDESC));
	ddsd.dwSize = sizeof(DDSURFACEDESC);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;


	/* This also works on 95 but not NT and again, doesn't matter because no D3D */
	//ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY;
	/* This works on both: NT doesn't allow pixelformat for anything but overlays, it seems */
	ddsd.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY;


	ddsd.dwWidth = w;
	ddsd.dwHeight = h;
	ddsd.ddpfPixelFormat.dwSize   = sizeof(DDPIXELFORMAT);
	switch(s3format) {
	case S3DTK_TEXARGB8888:
		ddsd.ddpfPixelFormat.dwRGBBitCount = 32;
		ddsd.ddpfPixelFormat.dwFlags  = DDPF_RGB ; 
		ddsd.ddpfPixelFormat.dwFourCC = BI_RGB ;
		ddsd.ddpfPixelFormat.dwRBitMask = 0x000000ff ; 
		ddsd.ddpfPixelFormat.dwGBitMask = 0x0000ff00; 
		ddsd.ddpfPixelFormat.dwBBitMask = 0x00ff0000 ; 
		break;
	case S3DTK_TEXARGB4444:
		ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
		ddsd.ddpfPixelFormat.dwFlags  = DDPF_RGB ; 
		ddsd.ddpfPixelFormat.dwFourCC = BI_RGB ;
#if 0
		/* If defined as an overlay, 444 is not a supported overlay format, so select one that is */
		ddsd.ddpfPixelFormat.dwRBitMask = 0x0f00 ; 
		ddsd.ddpfPixelFormat.dwGBitMask = 0x00f0; 
		ddsd.ddpfPixelFormat.dwBBitMask = 0x000f ; 
#else
		/* Use 555 instead */
		ddsd.ddpfPixelFormat.dwRBitMask = 0x7c00 ; 
		ddsd.ddpfPixelFormat.dwGBitMask = 0x03e0; 
		ddsd.ddpfPixelFormat.dwBBitMask = 0x001f ; 
#endif
		break;
	case S3DTK_TEXARGB1555:
		ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
		ddsd.ddpfPixelFormat.dwFlags  = DDPF_RGB ; 
		ddsd.ddpfPixelFormat.dwFourCC = BI_RGB ;
		ddsd.ddpfPixelFormat.dwRBitMask = 0x7c00 ; 
		ddsd.ddpfPixelFormat.dwGBitMask = 0x03e0; 
		ddsd.ddpfPixelFormat.dwBBitMask = 0x001f ; 
		break;
	default :
		// Set a value that will cause the code to fail the createsurface
		ddsd.ddpfPixelFormat.dwFlags = 0;
		break;
	}
	if ((result = IDirectDraw_CreateSurface(gpDD, &ddsd, surface, NULL)) != DD_OK)
		return(result);

	return(DD_OK);
}



/*
 * Lock and unlock
 */
HRESULT LockSurface(LPDIRECTDRAWSURFACE surface, void **addr, dword *pitch_bytes)
{
	HRESULT			result;
	DDSURFACEDESC	surface_descriptor;

	/* Lock the surface, pick up the info */
	memset(&surface_descriptor, 0, sizeof(surface_descriptor));
	surface_descriptor.dwSize = sizeof(surface_descriptor);
	result = IDirectDrawSurface_Lock(surface, NULL, &surface_descriptor,
									 DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

	if (result == DDERR_SURFACELOST) {
		result = IDirectDrawSurface_Restore(surface);
		if (result != DD_OK)
			return result;

		result = IDirectDrawSurface_Lock(surface, NULL, &surface_descriptor,
										 DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	}

	if (result != DD_OK)
		return result;

	if (addr) *addr = surface_descriptor.lpSurface;
	if (pitch_bytes) *pitch_bytes = surface_descriptor.lPitch;

	return DD_OK;
}

/*
 * Unlock surface
 */
HRESULT UnlockSurface(LPDIRECTDRAWSURFACE surface, void *addr)
{
	HRESULT	result;

	if ((result = IDirectDrawSurface_Unlock(surface, addr)) != DD_OK)
		return result;

	return DD_OK;
}


/*
 * Get address of a surface
 *
 * Note that this is quite slow, because Lock and Unlock are expensive.
 */
HRESULT GetSurfaceAddress(LPDIRECTDRAWSURFACE surface, void **addr)
{
	HRESULT result;

	if ((result = LockSurface(surface, addr, NULL)) != DD_OK)
		return(result);
	return(UnlockSurface(surface, *addr));
}



/*
 * Surface fills
 */
HRESULT DDRectFill(LPDIRECTDRAWSURFACE surface, int x, int y, int w, int h, dword colour)
{
	RECT rect;
	DDBLTFX bltfx;
	HRESULT result;

	/* Fill the rectangle using a blit */
	SetRect(&rect, x, y, x+w, y+h);
	bltfx.dwSize = sizeof(DDBLTFX);
	bltfx.dwFillColor = (DWORD)colour;
	result = IDirectDrawSurface_Blt(surface, &rect, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
	if (result == DDERR_SURFACELOST) {
		result = IDirectDrawSurface_Restore(surface);
		if (result != DD_OK)
			return result;
		result = IDirectDrawSurface_Blt(surface, &rect, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
	}
	return(result);
}

HRESULT DDRectZFill(LPDIRECTDRAWSURFACE surface, int x, int y, int w, int h, dword colour)
{
	RECT rect;
	DDBLTFX bltfx;
	HRESULT result;

	SetRect(&rect, x, y, x+w, y+h);
	bltfx.dwSize = sizeof(DDBLTFX);

#if 0	// Again, this does not work on NT
	bltfx.dwFillDepth = (DWORD)colour;
	result = IDirectDrawSurface_Blt(surface, &rect, NULL, NULL, DDBLT_DEPTHFILL | DDBLT_WAIT, &bltfx);
	if (result == DDERR_SURFACELOST) {
		result = IDirectDrawSurface_Restore(surface);
		if (result != DD_OK)
			return result;
		result = IDirectDrawSurface_Blt(surface, &rect, NULL, NULL, DDBLT_DEPTHFILL | DDBLT_WAIT, &bltfx);
	}
#else
	bltfx.dwFillColor = (DWORD)colour;
	result = IDirectDrawSurface_Blt(surface, &rect, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
	if (result == DDERR_SURFACELOST) {
		result = IDirectDrawSurface_Restore(surface);
		if (result != DD_OK)
			return result;
		result = IDirectDrawSurface_Blt(surface, &rect, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
	}
#endif
	return(result);
}









/* GL/S3D/DirectDraw low-level functions */





/*
 * Set render addresses into S3D
 */
void SetRenderAddress(s3MesaContext s3ctx)
{
	void *addr;
	LPDIRECTDRAWSURFACE surface;
	ULONG offset, correction;
	POINT pt;

	/* Set Z-buffer up first */
	s3ctx->renderdisabled &= ~2;
	if (s3ctx->z_surface) {
		if (GetSurfaceAddress(s3ctx->z_surface, &addr) != DD_OK) {
#ifdef DEBUG_SURFACES
			fprintf(stderr, "Error locking Z-buffer\n");
#endif
			s3ctx->renderdisabled |= 2;
		} else {
			offset = LinearToPhysical((ULONG) addr) - s3ctx->vram_base;
			s3ctx->s3_z.sfOffset = offset;
			s3f->S3DTK_SetState(s3f, S3DTK_ZBUFFERSURFACE, (ULONG) &s3ctx->s3_z);

#ifdef DEBUG_SURFACES
			fprintf(stderr, "Z-buffer set up at offset %x\n", offset);
#endif

		}
	}


	correction = 0;
	if (s3ctx->current_buffer == GL_FRONT) {
		surface = s3ctx->front_surface;
		if (!s3ctx->fullscreen) {
			s3ctx->s3_draw.sfWidth = s3ctx->screen_width;
			s3ctx->s3_draw.sfHeight = s3ctx->screen_height;
			pt.x = pt.y = 0;
			ClientToScreen(s3ctx->hwnd, &pt);
			correction = 2* (pt.x + (pt.y*s3ctx->screen_width));
		}
	} else if (s3ctx->current_buffer == GL_BACK) {
		surface = s3ctx->back_surface;
		if (!s3ctx->fullscreen) {
			s3ctx->s3_draw.sfWidth = s3ctx->window_width;
			s3ctx->s3_draw.sfHeight = s3ctx->window_height;
		}
	} else {
#ifdef DEBUG_SURFACES
		fprintf(stderr, "Buffer other than front/back selected\n");
#endif
		return;
	}

	s3ctx->renderdisabled &= ~4;

	if (!surface) {
		s3ctx->renderdisabled |= 4;
		return;
	}

	if (GetSurfaceAddress(surface, &addr) != DD_OK) {
#ifdef DEBUG_SURFACES
		fprintf(stderr, "Error locking back surface\n");
#endif
		s3ctx->renderdisabled |= 4;
	} else {
		offset = LinearToPhysical((ULONG) addr) - s3ctx->vram_base;
		offset += correction;
		s3ctx->s3_draw.sfOffset = offset;
		s3f->S3DTK_SetState(s3f, S3DTK_DRAWSURFACE, (ULONG) &s3ctx->s3_draw);

#ifdef DEBUG_SURFACES
			fprintf(stderr, "Back buffer set up at offset %x\n", offset);
#endif
	}
}




/*
 * Double buffer; knows whether to do it by page flip (fullscreen) or blit (windowed)
 */
HRESULT DDDoubleBuffer(HWND hwnd, LPDIRECTDRAWSURFACE front_surface, LPDIRECTDRAWSURFACE back_surface, int width, int height)
{
	HRESULT result;

#if USE_FULLSCREEN

#ifdef DEBUG_SURFACES
	fprintf(stderr, "Double Buffer by flip\n");
#endif

	result = IDirectDrawSurface_Flip(front_surface, NULL, DDFLIP_WAIT);

	if (result == DDERR_SURFACELOST) {
		/* One or more surfaces lost, so restore them and try again. This cannot return
		 * SURFACELOST again */
		IDirectDrawSurface_Restore(front_surface);
		IDirectDrawSurface_Restore(back_surface);
		result = IDirectDrawSurface_Flip(front_surface, NULL, DDFLIP_WAIT);
	}

	return(result);
#else
	POINT pt;
	RECT srect, drect;

#ifdef DEBUG_SURFACES
	fprintf(stderr, "Double Buffer by blit\n");
#endif

	/* Set the source and destination RECTs */
	SetRect(&srect, 0, 0, width, height);
	SetRect(&drect, 0, 0, width, height);

	/* Offset if not fullscreen */
	pt.x = pt.y = 0;
	ClientToScreen(hwnd, &pt);
	OffsetRect(&drect, pt.x, pt.y);

	/* Blit the pixels */
	result = IDirectDrawSurface_Blt(front_surface, &drect, back_surface, &srect, DDBLT_WAIT, NULL);
	if (result == DDERR_SURFACELOST) {
		IDirectDrawSurface_Restore(front_surface);
		IDirectDrawSurface_Restore(back_surface);
		result = IDirectDrawSurface_Blt(front_surface, &drect, back_surface, &srect, DDBLT_WAIT, NULL);
	}
	return(result);
#endif
}



/*   Miscellaneous GL functions   */


/* return buffer size information */
static void buffer_size(GLcontext *ctx, GLuint *width, GLuint *height)
{
	s3MesaContext s3ctx=(s3MesaContext)ctx->DriverCtx;

#ifdef DEBUG_GL
	fprintf(stderr, "S3MESA: buffer_size(...)\n");
#endif

	*width = s3ctx->width;
	*height = s3ctx->height;
}


/* Set current drawing color */
static void set_color(GLcontext *ctx, GLubyte red, GLubyte green,
                       GLubyte blue, GLubyte alpha )
{
	s3MesaContext s3ctx=(s3MesaContext)ctx->DriverCtx;

#ifdef DEBUG_GL
	fprintf(stderr, "S3MESA: set_color(%d,%d,%d,%d)\n",red,green,blue,alpha);
#endif

	s3ctx->color = PACKEDCOLOR(red,green,blue,alpha);
}


/* implements glClearColor() */
static void clear_color(GLcontext *ctx, GLubyte red, GLubyte green,
                         GLubyte blue, GLubyte alpha )
{
	s3MesaContext s3ctx=(s3MesaContext)ctx->DriverCtx;

#ifdef DEBUG_GL
	fprintf(stderr, "S3MESA: clear_color(%d,%d,%d,%d)\n",red,green,blue,alpha);
#endif
 
	s3ctx->clearc = PACKEDCOLOR(red,green,blue,255);
	s3ctx->cleara = alpha;
}


/*
 * Clear the frame buffer
 *
 * This may need some extra code to allow for clearing the front surface instead
 * of the back surface....
 */
static void clear(GLcontext *ctx, GLboolean all,
                   GLint x, GLint y, GLint width, GLint height )
{
	s3MesaContext s3ctx=(s3MesaContext)ctx->DriverCtx;
	dword col;

#if defined(DEBUG_GL) || defined(DEBUG_SURFACES)
	fprintf(stderr, "S3MESA: clear(%d,%d,%d,%d)\n",x,y,width,height);
#endif


	col =	((s3ctx->clearc & 0x000000f8) << 7) |
			((s3ctx->clearc & 0x0000f800) >> 6) |
			((s3ctx->clearc & 0x00f80000) >> 19);
	DDRectFill(s3ctx->back_surface, 0, 0, s3ctx->width-1, s3ctx->height-1, col);
}


/*
 * set the buffer used in double buffering
 */
static GLboolean set_buffer(GLcontext *ctx, GLenum mode )
{
	s3MesaContext s3ctx=(s3MesaContext)ctx->DriverCtx;

#if defined(DEBUG_GL) || defined(DEBUG_SURFACES)
	fprintf(stderr, "S3MESA: set_buffer(%d)\n",mode);
#endif

	if (mode == GL_FRONT)
		s3ctx->current_buffer = mode;
	else if (s3ctx->double_buffer && (mode == GL_BACK))
		s3ctx->current_buffer = mode;
	else
		return(GL_FALSE);

	/* Update rendering address */
	SetRenderAddress(s3ctx);

	return GL_TRUE;
}







/* Bitmap code - this is very slow, and not thoroughly tested */

static GLboolean drawbitmap(GLcontext *ctx, GLsizei width, GLsizei height,
                            GLfloat xorig, GLfloat yorig,
                            GLfloat xmove, GLfloat ymove,
                            const struct gl_image *bitmap)
{
	s3MesaContext s3ctx=(s3MesaContext)ctx->DriverCtx;
	word *screen_ptr, *p, color;
	GLubyte *pb;
	int x,y;
	GLint r,g,b,a,px,py,scrwidth,scrheight;
	dword stride;
	LPDIRECTDRAWSURFACE surface;

#define ISCLIPPED(rx) ( ((rx)<0) || ((rx)>=scrwidth) )
#define DRAWBIT(i) {				\
		if (!ISCLIPPED(x+px))		\
			if ((*pb) & (1<<(i)))	\
				(*p) = color;		\
		p++;						\
		x++;						\
		if (x >= width) {			\
			pb++;					\
			break;					\
		}							\
	}

	scrwidth = s3ctx->width;
	scrheight = s3ctx->height;

	px = (GLint)((ctx->Current.RasterPos[0]-xorig)+0.0F);
	py = (GLint)((ctx->Current.RasterPos[1]-yorig)+0.0F);

	if ((px>=scrwidth) || (px+width<=0) || (py>=scrheight) || (py+height<=0))
		return GL_TRUE;

	pb = (GLubyte *) bitmap->Data;

	if(py<0) {
		pb+=(bitmap->Height*(-py)) >> (3+1);
		height+=py;
		py=0;
	}

	if (py+height >= scrheight)
		height -= (py+height)-scrheight;

	if (s3ctx->current_buffer == GL_FRONT)
		surface = s3ctx->front_surface;
	else
		surface = s3ctx->back_surface;

	if (LockSurface(surface, &screen_ptr, &stride) != DD_OK) {
		fprintf(stderr, "Lock failed in drawbitmap\n");
		return GL_TRUE;
	}
	stride >>= 1;	// convert stride to pixels

	r = (GLint) (ctx->Current.RasterColor[0]*ctx->Visual->RedScale);
	g = (GLint) (ctx->Current.RasterColor[1]*ctx->Visual->GreenScale);
	b = (GLint) (ctx->Current.RasterColor[2]*ctx->Visual->BlueScale);
	a = (GLint) (ctx->Current.RasterColor[3]*ctx->Visual->AlphaScale);
	color = (word)
			( ((word)0xf8 & r) << 7) |
			( ((word)0xf8 & g) << 2) |
			( ((word)0xf8 & b) >> 3);

	/* Very slow */
	for(y=0;y<height;y++) {
		p = screen_ptr + px + ((scrheight-(y+py)) * stride);

		for(x=0;;) {
			DRAWBIT(7);	DRAWBIT(6);	DRAWBIT(5);	DRAWBIT(4);
			DRAWBIT(3);	DRAWBIT(2);	DRAWBIT(1);	DRAWBIT(0);
			pb++;
		}
	}

	UnlockSurface(surface, screen_ptr);

#undef ISCLIPPED
#undef DRAWBIT

	return GL_TRUE;
}


/************************************************************************/


							
/*	Setup functions: done with macros for speed and readability */

#define NOP(v)


/* Colour component setup; three modes (plus NOP) */
#define GOURAUD(v) {\
	(v)->R = VB->Color[i][0]; \
	(v)->G = VB->Color[i][1]; \
	(v)->B = VB->Color[i][2]; \
	(v)->A = VB->Color[i][3]; }



#if 0	// Fog does not currently function - although the macros should work, they do not appear to

#define FOG(v) {	fv = fog_const*(VB->Clip[i][3] - fog_start);\
					if (fv > 255.0f) fv = 255.0f; if (fv < 0.0f) fv = 0.0f; \
					(v)->A = fv; }

#define GOURAUDFOG(v) {\
	(v)->R = VB->Color[i][0]; \
	(v)->G = VB->Color[i][1]; \
	(v)->B = VB->Color[i][2]; \
	fv = fog_const*(VB->Clip[i][3] - fog_start); \
	if (fv > 255.0f) fv = 255.0f; if (fv < 0.0f) fv = 0.0f; \
	(v)->A = fv; }


#else	// Fog disabled

#define FOG(v)

#define GOURAUDFOG(v) {\
	(v)->R = VB->Color[i][0]; \
	(v)->G = VB->Color[i][1]; \
	(v)->B = VB->Color[i][2]; \
	(v)->A = VB->Color[i][3]; }

#endif


/* Texture component setup */
#define TEXTURE(v)  { \
	(v)->U = uscale*VB->TexCoord[i][0]; \
	(v)->V = vscale*VB->TexCoord[i][1]; \
	(v)->W = wscale*VB->Clip[i][3]; \
}


/* Z-buffer setup */
#define ZBUFFER(v) { (v)->Z = VB->Win[i][2]; }



/* Macro that builds the appropriate setup function */
#define SETUP( gouraud, texture, depth) {							\
	register unsigned int i;										\
	register struct vertex_buffer *VB = ctx->VB;					\
	s3MesaContext s3ctx = (s3MesaContext)ctx->DriverCtx; 			\
	register S3DTK_VERTEX_TEX *VTB = &s3ctx->vertex_tex[vstart];	\
	register float wscale = s3ctx->wscale; 							\
	register float uscale = (float)s3ctx->ti[s3ctx->currenttex].width;	\
	register float vscale = (float)s3ctx->ti[s3ctx->currenttex].height;	\
	register float yflip = s3ctx->height-1.0f;						\
	float fog_const = s3ctx->fog_const;								\
	float fog_start = s3ctx->fog_start;								\
	float fv;														\
																	\
	for ( i = vstart; i < vend ; i++, VTB++ )						\
	{																\
		VTB->X = VB->Win[i][0];										\
		VTB->Y = yflip - VB->Win[i][1];								\
		gouraud(VTB);												\
		depth(VTB);													\
		texture(VTB);												\
	}																\
}

static void setup( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( NOP, NOP, NOP );
}
	
static void setupG( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( GOURAUD, NOP, NOP );
}

static void setupT( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( NOP, TEXTURE, NOP );
}
	
static void setupGT( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( GOURAUD, TEXTURE, NOP );
}
	
static void setupZ( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( NOP, NOP, ZBUFFER );
}
	
static void setupGZ( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( GOURAUD, NOP, ZBUFFER );
}

static void setupTZ( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( NOP, TEXTURE, ZBUFFER );
}
	
static void setupGTZ( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( GOURAUD, TEXTURE, ZBUFFER );
}

static void setupF( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( FOG, NOP, NOP );
}
	
static void setupGF( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( GOURAUDFOG, NOP, NOP );
}

static void setupTF( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( FOG, TEXTURE, NOP );
}
	
static void setupGTF( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( GOURAUDFOG, TEXTURE, NOP );
}
	
static void setupZF( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( FOG, NOP, ZBUFFER );
}
	
static void setupGZF( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( GOURAUDFOG, NOP, ZBUFFER );
}

static void setupTZF( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( FOG, TEXTURE, ZBUFFER );
}
	
static void setupGTZF( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( GOURAUDFOG, TEXTURE, ZBUFFER );
}
	

/* Gouraud means colour values per-vertex; texture is texture coords (& therefore W)
 * Z is Z for z-buffering, Fog requires fog setup (in alpha for S3) */
#define GOURAUD_ENABLED 0x1
#define TEXTURE_ENABLED 0x2
#define ZBUFFER_ENABLED 0x4
#define FOG_ENABLED		0x8

typedef void (*setup_func)(GLcontext *, GLuint, GLuint);

setup_func s3SetupFuncs[] = {
	setup,
	setupG,
	setupT,
	setupGT,
	setupZ,
	setupGZ,
	setupTZ,
	setupGTZ,
	setupF,
	setupGF,
	setupTF,
	setupGTF,
	setupZF,
	setupGZF,
	setupTZF,
	setupGTZF,
};

setup_func 
choose_setup_function( GLcontext *ctx )
{
	unsigned int setupIndex = 0;

	if ( ctx->Light.ShadeModel == GL_SMOOTH && !ctx->Light.Model.TwoSide )
									setupIndex |= GOURAUD_ENABLED;
	if ( ctx->Texture.Enabled )		setupIndex |= TEXTURE_ENABLED;
	if ( ctx->Depth.Test )			setupIndex |= ZBUFFER_ENABLED;
	if ( ctx->Fog.Enabled )			setupIndex |= FOG_ENABLED;

	return s3SetupFuncs[setupIndex];
}






/* ***************************** */

/* Render functions */



#undef GOURAUD

#define GOURAUD(v) { \
	s3ctx->vertex_tex[(v)].R = VB->Color[(v)][0]; \
	s3ctx->vertex_tex[(v)].G = VB->Color[(v)][1]; \
	s3ctx->vertex_tex[(v)].B = VB->Color[(v)][2]; \
	s3ctx->vertex_tex[(v)].A = VB->Color[(v)][3]; }

#define FLAT(v) { \
	s3ctx->vertex_tex[(v)].R = Color[0]; \
	s3ctx->vertex_tex[(v)].G = Color[1]; \
	s3ctx->vertex_tex[(v)].B = Color[2]; \
	s3ctx->vertex_tex[(v)].A = Color[3]; }




/* Points */

static void 
s3Point(GLcontext *ctx, GLuint first, GLuint last)
{
	/* Points are not currently supported */
}


points_func 
choose_points_function( GLcontext *ctx )
{
   return s3Point;
}



/* Lines */


void RenderLine(S3DTK_VERTEX_TEX *v1, S3DTK_VERTEX_TEX *v2)
{
	S3DTK_VERTEX_TEX *v[2];

	v[0] = v1;
	v[1] = v2;
	s3f->S3DTK_TriangleSet(s3f, (ULONG *) &v[0], 2,S3DTK_LINE);
}



static void 
s3LineSmooth(GLcontext *ctx, GLuint v1, GLuint v2, GLuint pv)
{
	s3MesaContext s3ctx = (s3MesaContext)ctx->DriverCtx;

	RenderLine(&s3ctx->vertex_tex[v1], &s3ctx->vertex_tex[v2]);
}

static void 
s3LineSmoothTwoSide(GLcontext *ctx, GLuint v1, GLuint v2, GLuint pv)
{
	s3MesaContext s3ctx = (s3MesaContext)ctx->DriverCtx;
	struct vertex_buffer *VB = ctx->VB;

	GOURAUD(v1); 
	GOURAUD(v2); 
	RenderLine(&s3ctx->vertex_tex[v1], &s3ctx->vertex_tex[v2]);
}

static void 
s3LineFlat(GLcontext *ctx, GLuint v1, GLuint v2, GLuint pv)
{
	s3MesaContext s3ctx = (s3MesaContext)ctx->DriverCtx;
	GLubyte *Color = ctx->VB->Color[pv];

	FLAT(v1);
	FLAT(v2);
	RenderLine(&s3ctx->vertex_tex[v1], &s3ctx->vertex_tex[v2]);
}

line_func 
choose_line_function( GLcontext *ctx )
{
   if ( ctx->Light.ShadeModel == GL_SMOOTH )
      if ( ctx->Light.Model.TwoSide )
         return s3LineSmoothTwoSide;
      else
         return s3LineSmooth;
   else
      return s3LineFlat;
}



/* Triangles */

static void 
s3TriangleSmooth(GLcontext *ctx, GLuint v1, GLuint v2, GLuint v3, GLuint pv)
{
	S3DTK_VERTEX_TEX *v[3];
	s3MesaContext s3ctx = (s3MesaContext)ctx->DriverCtx;

	v[0] = &s3ctx->vertex_tex[v1];
	v[1] = &s3ctx->vertex_tex[v2];
	v[2] = &s3ctx->vertex_tex[v3];


#ifdef DEBUG_PRIMITIVES
	fprintf(stderr, "S: %f, %f, %f RGB %d %d %d UVW %f %f %f\n", v[0]->X, v[0]->Y, v[0]->Z, v[0]->R, v[0]->G, v[0]->B, v[0]->U, v[0]->V, v[0]->W);
	fprintf(stderr, " : %f, %f, %f RGB %d %d %d UVW %f %f %f\n", v[1]->X, v[1]->Y, v[1]->Z, v[1]->R, v[1]->G, v[1]->B, v[1]->U, v[1]->V, v[1]->W);
	fprintf(stderr, " : %f, %f, %f RGB %d %d %d UVW %f %f %f\n", v[2]->X, v[2]->Y, v[2]->Z, v[2]->R, v[2]->G, v[2]->B, v[2]->U, v[2]->V, v[2]->W);
#endif

	s3f->S3DTK_TriangleSet(s3f, (ULONG *) &v[0], 3, S3DTK_TRIFAN);
}

static void 
s3TriangleSmoothTwoSide(GLcontext *ctx, GLuint v1, GLuint v2, GLuint v3, GLuint pv)
{
	S3DTK_VERTEX_TEX *v[3];
	s3MesaContext s3ctx = (s3MesaContext)ctx->DriverCtx;
	struct vertex_buffer *VB = ctx->VB;

	GOURAUD(v1); 
	GOURAUD(v2); 
	GOURAUD(v3); 
	v[0] = &s3ctx->vertex_tex[v1];
	v[1] = &s3ctx->vertex_tex[v2];
	v[2] = &s3ctx->vertex_tex[v3];

#ifdef DEBUG_PRIMITIVES
	fprintf(stderr, "2: %f, %f, %f RGB %d %d %d UVW %f %f %f\n", v[0]->X, v[0]->Y, v[0]->Z, v[0]->R, v[0]->G, v[0]->B, v[0]->U, v[0]->V, v[0]->W);
	fprintf(stderr, " : %f, %f, %f RGB %d %d %d UVW %f %f %f\n", v[1]->X, v[1]->Y, v[1]->Z, v[1]->R, v[1]->G, v[1]->B, v[1]->U, v[1]->V, v[1]->W);
	fprintf(stderr, " : %f, %f, %f RGB %d %d %d UVW %f %f %f\n", v[2]->X, v[2]->Y, v[2]->Z, v[2]->R, v[2]->G, v[2]->B, v[2]->U, v[2]->V, v[2]->W);
#endif

	s3f->S3DTK_TriangleSet(s3f, (ULONG *) &v[0], 3, S3DTK_TRIFAN);
}

static void 
s3TriangleFlat( GLcontext *ctx, GLuint v1, GLuint v2, GLuint v3, GLuint pv)
{
	S3DTK_VERTEX_TEX *v[3];
	s3MesaContext s3ctx = (s3MesaContext)ctx->DriverCtx;
	GLubyte *Color = ctx->VB->Color[pv];
	struct vertex_buffer *VB = ctx->VB;

	FLAT(v1);
	FLAT(v2);
	FLAT(v3);
	v[0] = &s3ctx->vertex_tex[v1];
	v[1] = &s3ctx->vertex_tex[v2];
	v[2] = &s3ctx->vertex_tex[v3];

#ifdef DEBUG_PRIMITIVES
	fprintf(stderr, "F: %f, %f, %f RGB %d %d %d UVW %f %f %f\n", v[0]->X, v[0]->Y, v[0]->Z, v[0]->R, v[0]->G, v[0]->B, v[0]->U, v[0]->V, v[0]->W);
	fprintf(stderr, " : %f, %f, %f RGB %d %d %d UVW %f %f %f\n", v[1]->X, v[1]->Y, v[1]->Z, v[1]->R, v[1]->G, v[1]->B, v[1]->U, v[1]->V, v[1]->W);
	fprintf(stderr, " : %f, %f, %f RGB %d %d %d UVW %f %f %f\n", v[2]->X, v[2]->Y, v[2]->Z, v[2]->R, v[2]->G, v[2]->B, v[2]->U, v[2]->V, v[2]->W);
#endif

	s3f->S3DTK_TriangleSet(s3f, (ULONG *) &v[0], 3, S3DTK_TRIFAN);
}

triangle_func 
choose_triangle_function( GLcontext *ctx )
{
   if ( ctx->Light.ShadeModel == GL_SMOOTH )
      if ( ctx->Light.Model.TwoSide )
         return s3TriangleSmoothTwoSide;
      else
         return s3TriangleSmooth;
   else
      return s3TriangleFlat;
}



/* Quads */

static void 
s3QuadSmooth(GLcontext *ctx, GLuint v1, GLuint v2, GLuint v3, GLuint v4, GLuint pv)
{
	s3MesaContext s3ctx = (s3MesaContext)ctx->DriverCtx;
	S3DTK_VERTEX_TEX *v[4];

	v[0] = &s3ctx->vertex_tex[v1];
	v[1] = &s3ctx->vertex_tex[v2];
	v[2] = &s3ctx->vertex_tex[v3];
	v[3] = &s3ctx->vertex_tex[v4];
	s3f->S3DTK_TriangleSet(s3f, (ULONG *) &v[0], 4, S3DTK_TRIFAN);
}

static void 
s3QuadSmoothTwoSide(GLcontext *ctx, GLuint v1, GLuint v2, GLuint v3, GLuint v4, GLuint pv)
{
	s3MesaContext s3ctx = (s3MesaContext)ctx->DriverCtx;
	struct vertex_buffer *VB = ctx->VB;
	S3DTK_VERTEX_TEX *v[4];

	GOURAUD(v1); 
	GOURAUD(v2); 
	GOURAUD(v3); 
	GOURAUD(v4); 

	v[0] = &s3ctx->vertex_tex[v1];
	v[1] = &s3ctx->vertex_tex[v2];
	v[2] = &s3ctx->vertex_tex[v3];
	v[3] = &s3ctx->vertex_tex[v4];
	s3f->S3DTK_TriangleSet(s3f, (ULONG *) &v[0], 4, S3DTK_TRIFAN);
}

static void 
s3QuadFlat(GLcontext *ctx, GLuint v1, GLuint v2, GLuint v3,GLuint v4, GLuint pv)
{
   s3MesaContext s3ctx = (s3MesaContext)ctx->DriverCtx;
   GLubyte *Color = ctx->VB->Color[pv];
	S3DTK_VERTEX_TEX *v[4];

	FLAT(v1);
	FLAT(v2);
	FLAT(v3);
	FLAT(v4);

	v[0] = &s3ctx->vertex_tex[v1];
	v[1] = &s3ctx->vertex_tex[v2];
	v[2] = &s3ctx->vertex_tex[v3];
	v[3] = &s3ctx->vertex_tex[v4];
	s3f->S3DTK_TriangleSet(s3f, (ULONG *) &v[0], 4, S3DTK_TRIFAN);
}

quad_func 
choose_quad_function(GLcontext *ctx)
{
   if(ctx->Light.ShadeModel == GL_SMOOTH)
      if(ctx->Light.Model.TwoSide)
         return s3QuadSmoothTwoSide;
      else
         return s3QuadSmooth;
   else
      return s3QuadFlat;
}



/************************************/

/* Depth buffer functions */

static void alloc_depth_buffer(GLcontext *ctx)
{
	s3MesaContext s3ctx=(s3MesaContext)ctx->DriverCtx;

#if defined(DEBUG_GL) || defined(DEBUG_SURFACES)
	fprintf(stderr, "S3MESA: alloc_depth_buffer()\n");
#endif

	if (s3ctx->z_surface) {
		fprintf(stderr, "Attempt to reallocate Z-buffer; already exists\n");
		return;
	}

	if (DDSetupZSurface(&s3ctx->z_surface, s3ctx->width, s3ctx->height) != DD_OK) {
		fprintf(stderr, "Failed to set up Z buffer\n");
		CleanExit(1);
	}
}

static void clear_depth_buffer(GLcontext *ctx)
{
	s3MesaContext s3ctx=(s3MesaContext)ctx->DriverCtx;

#if defined(DEBUG_GL) || defined(DEBUG_SURFACES)
	fprintf(stderr, "S3MESA: clear_depth_buffer()\n");
#endif

	if (ctx->Depth.Test && ctx->Depth.Mask && s3ctx->z_surface)
		DDRectZFill(s3ctx->z_surface, 0, 0, s3ctx->width-1, s3ctx->height-1, 0xffff);
}







/*****************************************/

static void finish(GLcontext *ctx)
{

}

static void flush(GLcontext *ctx)
{

}




/*****************************************/


/* Texture support */


/* S3 utility functions */


static int UploadTexture(LPDIRECTDRAWSURFACE surface, void *data, int size, void **addr)
{
	HRESULT result;

	if ((result = LockSurface(surface, addr, NULL)) != DD_OK) {
		fprintf(stderr, "LOCK FAILED, code %x (%d)\n", result, result&0xffff);
		return(0);
	} else {
#ifdef DEBUG_TEXTURES
		fprintf(stderr, "Lock OK ptr %x\n", addr);
#endif
	}
	memcpy(*addr, data, size);
	result = UnlockSurface(surface, *addr);
#ifdef DEBUG_TEXTURES
	fprintf(stderr, "Unlock returned %x (%d)\n", result, result&0xffff);
#endif

	return(1);
}


/*
 * Clear texture caching data out of a texture
 *
 * Note that the clearance used has a couple of strange cases, in that
 * the texture number can be on the cache list twice. In these cases,
 * the texture may be freed more often than expected. This is not likely to
 * be a major problem - in cases where it occurs the entire cache will probably
 * be totally cycled every frame anyway, and the errant case will be cleared
 * out. This only applies if the texture is deleted.
 */
static void UncacheTexture(s3MesaContext s3ctx, int texObject)
{
	if (s3ctx->ti[texObject].on_card) {
		IDirectDrawSurface_Release(s3ctx->ti[texObject].vram_surface);
		s3ctx->ti[texObject].vram_surface = NULL;
		s3ctx->ti[texObject].on_card = 0;
	}
}


static void ClearTextureCache(s3MesaContext s3ctx)
{
	int i;

	for(i=0; i<MAXNUM_TEX; i++)
		UncacheTexture(s3ctx, i);

	/* Can wipe the head and tail structures as well, since there is
	 * nothing at all left on the card. */
	s3ctx->texture_cache_head = 0;
	s3ctx->texture_cache_tail = 0;
}




/*
 * Ensure that a texture is on the card. If couldn't, returns 0, else returns 1
 */
static int CacheTexture(s3MesaContext s3ctx, int texObject)
{
	texinfo *info = &s3ctx->ti[texObject];
	void *addr;
	ULONG offset;
	HRESULT result;
	int tail;

	if (info->on_card) {
		/* Confirm address and not-lost status */
		void *addr;

		if (GetSurfaceAddress(info->vram_surface, &addr) != DD_OK) {
			fprintf(stderr, "CacheTexture: Address confirmation failed\n");
			return(0);
		}
		offset = LinearToPhysical((ULONG) addr) - s3ctx->vram_base;
		info->s3_surface.sfOffset = offset;

		return(1);
	}

	/* Try to allocate */
	do {
		int width, height;

		width = info->width;
		height = info->height;

		if (info->have_mipmap) {
			height *= 22;
			height /= 16;
		}

		result = DDSetupTextureSurface(&info->vram_surface, width, height, info->s3_surface.sfFormat);
		if (result == DD_OK)
			break;

		if (s3ctx->texture_cache_head == s3ctx->texture_cache_tail) {
#ifdef DEBUG_TEXTURES
			fprintf(stderr, "Texture cache cleared; still cannot allocate\n");
#endif
			return(0);
		}

		/* Free up one of the existing textures */
		tail = s3ctx->texture_cache_tail;
		UncacheTexture(s3ctx, s3ctx->texture_cache[tail]);
		s3ctx->texture_cache_tail = (tail + 1) % MAXNUM_TEX;
	} while (1);

	/* Insert this one onto the cache list */
	s3ctx->texture_cache[s3ctx->texture_cache_head] = texObject;
	s3ctx->texture_cache_head = (s3ctx->texture_cache_head + 1) % MAXNUM_TEX;
	info->on_card = 1;

	/* Upload it */
	if (!UploadTexture(info->vram_surface, info->data, info->size, &addr))
		return(0);

	/* Set this to the S3 surface data */
	offset = LinearToPhysical((ULONG) addr) - s3ctx->vram_base;
	info->s3_surface.sfOffset = offset;

	return(1);
}




/*
 * Make the chosen texture current for rendering
 */
static int SelectTexture(s3MesaContext s3ctx, GLuint texObject)
{
	texinfo *info = &s3ctx->ti[texObject];

	/* Check for a bad texture */
	if (texObject > MAXNUM_TEX)
		return(0);

#ifdef DEBUG_TEXTURES
	fprintf(stderr, "SelectTexture: (%d), data %x (%d), width %d, height %d\n",
				texObject, info->data, info->size, info->width, info->height);
#endif

	/* Idiot check */
	if (!info->data || (info->size > (512*512*2))) {
		fprintf(stderr, "SelectTexture: idiot check failed in texture %d\n", texObject);
		return(0);
	}

	/* Cache it */
	if (!CacheTexture(s3ctx, texObject)) {
		fprintf(stderr, "SelectTexture: Unable to cache texture %d\n", texObject);
		return(0);
	}

	s3f->S3DTK_SetState(s3f, S3DTK_TEXTUREACTIVE, (ULONG) &info->s3_surface);

	return(1);	// OK
}






/*
 * Based on the minfilter/magfilter values, set up the appropriate filtering mode
 */
static void SetFilterMode(texinfo *info)
{
	int bilinear;
	ULONG filtermode;

	/* Whatever the min filter is, set bilinear according to mag filter */
	if (info->magfilter == GL_LINEAR)
		bilinear = 1;
	else
		bilinear = 0;

	if (!info->have_mipmap) {
		/* No mipmapping */
		if (bilinear)
			filtermode = S3DTK_TEX4TPP;
		else
			filtermode = S3DTK_TEX1TPP;
	} else {
		/* Use minfilter only to select mipmapping */
		switch(info->minfilter) {
		case GL_NEAREST:
		case GL_LINEAR:
			if (bilinear)
				filtermode = S3DTK_TEX4TPP;
			else
				filtermode = S3DTK_TEX1TPP;
			break;
		case GL_NEAREST_MIPMAP_NEAREST:
		case GL_NEAREST_MIPMAP_LINEAR:
			if (bilinear)
				filtermode = S3DTK_TEXM4TPP;
			else
				filtermode = S3DTK_TEXM1TPP;
			break;
		case GL_LINEAR_MIPMAP_LINEAR:
			if (bilinear)
				filtermode = S3DTK_TEXM8TPP;
			else
				filtermode = S3DTK_TEXM2TPP;
			break;
		}
	}

	info->filter = filtermode;
}




/*
 * Set all texture state in S3D
 */
static void UpdateTextureState(GLcontext *ctx)
{
	s3MesaContext s3ctx=(s3MesaContext)ctx->DriverCtx;
	ULONG texture_code;

	/* Set appropriately for lit/unlit textures */
	texture_code = S3DTK_LITTEXTUREPERSPECT;

	if (ctx->Texture.Enabled) {
		/* Nothing much is done in this switch, but retain for later chipsets or if
		 * someone decides that BLEND mode is really required. */
		switch(ctx->Texture.EnvMode) {
		case GL_DECAL:
			/* No changes required */
			break;
		case GL_REPLACE:
			/* This is an unlit texture */
			texture_code = S3DTK_UNLITTEXTUREPERSPECT;
			break;
		case GL_MODULATE:
			/* This means modulation with the DESTINATION, i.e. a multiplication */
			/* Not implemented: probably cannot be supported on ViRGE, although if
			 * the texture is a luminance texture, it will probably work out OK anyway
			 * with appropriate alpha format setup. */
			break;
		case GL_BLEND:
			/* This means blend with EnvColor (which is a float[4]) */
			/* Not implemented: needs a lot of work to support on ViRGE */
			break;
		default:
			fprintf(stderr, "Unknown texture environment mode %x in UpdateTextureStat\n", ctx->Texture.EnvMode);
			break;
		}

		/* Upload the texture, drop to gouraud if it fails, else set texturing on
		 * and set the appropriate filter mode. */
		if (!SelectTexture(s3ctx, s3ctx->currenttex)) {
#ifdef DEBUG_TEXTURES
			fprintf(stderr, "Texture load failed\n");
#endif
			s3f->S3DTK_SetState(s3f, S3DTK_RENDERINGTYPE, S3DTK_GOURAUD);
		} else {
			s3f->S3DTK_SetState(s3f, S3DTK_RENDERINGTYPE, texture_code);
			s3f->S3DTK_SetState(s3f, S3DTK_TEXFILTERINGMODE, s3ctx->ti[s3ctx->currenttex].filter);

			if (s3ctx->ti[s3ctx->currenttex].transparent)
				s3f->S3DTK_SetState(s3f, S3DTK_ALPHABLENDING, S3DTK_ALPHATEXTURE);
		}
	} else {
		s3f->S3DTK_SetState(s3f, S3DTK_RENDERINGTYPE, S3DTK_GOURAUD);
	}


}





/*
 * Texture environment - no action needs to be taken here, as it is put into the
 * appropriate position by Mesa and will be checked in the next render state update
 */
static void texenv(GLcontext *ctx, GLenum pname, const GLfloat *param)
{
   GLenum p=(GLenum)(GLint)*param;

#if defined(DEBUG_GL) || defined(DEBUG_TEXTURES)
   fprintf(stderr, "S3MESA: texenv pname %x, param %x\n", pname, p);
#endif
}




/*
 * Texture parameter
 */
static void texparam(GLcontext *ctx, GLenum target, GLuint texObject,
					 GLenum pname, const GLfloat *params)
{
	s3MesaContext s3ctx=(s3MesaContext)ctx->DriverCtx;
	GLenum param=(GLenum)(GLint)params[0];

	if (texObject >= MAXNUM_TEX)
		return;

#if defined(DEBUG_GL) || defined(DEBUG_TEXTURES)
	fprintf(stderr, "S3MESA: texparam object %d target %x pname %x, param %x\n", texObject, target, pname, param);
#endif

	if(target!=GL_TEXTURE_2D) {
#if !defined(DEBUG_GL) && !defined(DEBUG_TEXTURES)
		fprintf(stderr,"s3: unsupported texture in texparam()\n");
#endif
		return;
	}

	switch(pname) {
	case GL_TEXTURE_MIN_FILTER:
		switch(param) {
		case GL_NEAREST:
		case GL_LINEAR:
		case GL_NEAREST_MIPMAP_NEAREST:
		case GL_NEAREST_MIPMAP_LINEAR:
		case GL_LINEAR_MIPMAP_LINEAR:
			s3ctx->ti[texObject].minfilter = param;
			SetFilterMode(&s3ctx->ti[texObject]);
			break;
		default:
			break;
		}
		break;

	case GL_TEXTURE_MAG_FILTER:
		switch(param) {
		case GL_NEAREST:
		case GL_LINEAR:
			s3ctx->ti[texObject].magfilter = param;
			SetFilterMode(&s3ctx->ti[texObject]);
			break;
		default:
			break;
		}
		break;

	/* Wrapping/clamp not supported in S3D yet, but will be in future */
	case GL_TEXTURE_WRAP_S:
		switch(param) {
		case GL_CLAMP:
			break;
		case GL_REPEAT:
			break;
		default:
			break;
		}
		break;

	case GL_TEXTURE_WRAP_T:
		switch(param) {
		case GL_CLAMP:
			break;
		case GL_REPEAT:
			break;
		default:
			break;
		}
		break;

	case GL_TEXTURE_BORDER_COLOR:
		/* TO DO - probably cannot be supported on S3 */
		break;

	default:
		break;
	}

	s3ctx->currenttex = texObject;
	UpdateTextureState(ctx);
}



/*
 * Texture delete
 */
static void texdel(GLcontext *ctx, GLuint texObject)
{
	s3MesaContext s3ctx=(s3MesaContext)ctx->DriverCtx;

#if defined(DEBUG_GL) || defined(DEBUG_TEXTURES)
	fprintf(stderr, "S3MESA: texdel object %d", texObject);
#endif

	UncacheTexture(s3ctx, texObject);
	free(s3ctx->ti[texObject].data);
	s3ctx->ti[texObject].mipmaplevels = 0;
	s3ctx->ti[texObject].data = NULL;
}






static void texbind(GLcontext *ctx, GLenum target, GLuint texObject)
{
	s3MesaContext s3ctx=(s3MesaContext)ctx->DriverCtx;

#if defined(DEBUG_GL) || defined(DEBUG_TEXTURES)
	fprintf(stderr, "texbind: object %d\n", texObject);
#endif

	if (texObject >= MAXNUM_TEX)
		return;

	/* XXX Should probably check for the Dirty flag in the texture image and then
	 * rebuild the image if required */

	if (texObject == 0)
		fprintf(stderr, "texbind: NULL OBJECT\n");

	s3ctx->currenttex = texObject;
	UpdateTextureState(ctx);
}





static int logbase2(int n)
{
   GLint i = 1;
   GLint log2 = 0;

   if (n<0) {
      return -1;
   }

   while ( n > i ) {
      i *= 2;
      log2++;
   }
   if (i != n) {
      return -1;
   }
   else {
      return log2;
   }
}






/*
 * Copy and convert an OpenGL format texture to S3 texture formats
 * (RGB8888, RGB4444, RGB1555)
 *
 * Returns NULL if it failed to allocate a texture (either a memory failure or
 * an unsupported format).
 */
int TexBuildS3Format(word *dest, const struct gl_texture_image *image, GLint internalFormat, int *transparent, ULONG *s3format)
{
	unsigned char r,g,b,a,l,*data;
	int x,y,w,h,idx;

	w = image->Width;
	h = image->Height;

	*transparent = 0;

	data = image->Data;
	switch(internalFormat) {
	case GL_ALPHA:
	case GL_ALPHA8:
	case 1:		// == GL_LUMINANCE
	case GL_LUMINANCE:
	case GL_LUMINANCE8:
		/* This is an alpha map texture */
		*s3format = S3DTK_TEXARGB4444;
		for(y=0;y<h;y++)
			for(x=0;x<w;x++) {
				idx = (x + y*w);
				l = (gamma_table[data[idx]] & 0xf0) >> 4;
				l ^= 15;
				dest[x+y*w]= ((word) l << 12);
			}
		*transparent = 1;
		break;

	case GL_LUMINANCE_ALPHA:
	case GL_LUMINANCE8_ALPHA8:
	case 2:
		return 0;
		break;
	case GL_RGB:
	case GL_RGB8:
	case 3:
		/* RGB, no alpha data */
		*s3format = S3DTK_TEXARGB1555;
		for(y=0;y<h;y++)
			for(x=0;x<w;x++) {
				idx = (x + y*w) *3;
				r = gamma_table[data[idx]];
				g = gamma_table[data[idx+1]];
				b = gamma_table[data[idx+2]];

				dest[x+y*w]= (word)
							( ((word)0xf8 & r) << 7) |
							( ((word)0xf8 & g) << 2) |
							( ((word)0xf8 & b) >> 3); 
			}
		break;
	case GL_RGBA:
	case GL_RGBA8:
	case 4:
		/* RGB with alpha */

		/* In order to support alpha test completely, this should be a lot more complex
		 * than it is - another copy of the texture is probably needed, and from that
		 * a preprocessed-for-alpha-test image can be generated whenever the alpha test
		 * value changes. This is necessary for Quake. */
		*s3format = S3DTK_TEXARGB4444;
//		*transparent = 1;
		for(y=0;y<h;y++)
			for(x=0;x<w;x++) {
				idx = (x + y*w) *4;
				r = gamma_table[data[idx]];
				g = gamma_table[data[idx+1]];
				b = gamma_table[data[idx+2]];
				a = gamma_table[data[idx+3]];

				dest[x+y*w]= (word)
							( ((word)0xf0 & a) << 8) |
							( ((word)0xf0 & r) << 4) |
							  ((word)0xf0 & g)       |
							( ((word)0xf0 & b) >> 4); 
			}
		break;
	default:
		fprintf(stderr,"S3GL: wrong internalFormat in texbuildimagemap()\n");
		return 0;
		break;
	}

	return 1;
}



/*
 * Texture image - process and register
 */
static void teximg(GLcontext *ctx, GLenum target,
                   GLuint texObject, GLint level, GLint internalFormat,
                   const struct gl_texture_image *image)
{
	s3MesaContext s3ctx=(s3MesaContext)ctx->DriverCtx;
	texinfo *ti=s3ctx->ti;
	word *converted_texture;
	int transparent;
	ULONG s3format;


#ifdef DEBUG_TEXTURES
	{
	static int totalsize;
	int size;
	char *string;

	size = image->Width*image->Height;

	switch(internalFormat) {
	case GL_ALPHA:
	case GL_ALPHA8:
	case GL_LUMINANCE:
	case GL_LUMINANCE8:
	case 1:
		/* byte-per-pixel */
		string = "1-byte alpha or lum";
		break;

	case GL_LUMINANCE_ALPHA:
	case GL_LUMINANCE8_ALPHA8:
	case 2:
		/* 2 bytes-per-pixel */
		size *= 2;
		string = "2-byte lum/alpha";
		break;

	case GL_RGB:
	case GL_RGB8:
	case 3:
		/* 3 bytes per pixel */
		size *= 3;
		string = "3-byte RGB";
		break;

	case GL_RGBA:
	case GL_RGBA8:
	case 4:
		/* 4 bytes per pixel */
		size *= 4;
		string = "4-byte RGBA";
		break;

	default:
		string = "Strange";
		size = 0;
		break;
	}
	totalsize += size;

	fprintf(stderr, "S3MESA: teximg: object %d, target %d, format %d (%s), w/h %d, %d  size %d, totalsize %d\n",
							texObject, target, internalFormat, string,
							image->Width,
							image->Height,
							size,
							totalsize);
	}
#endif


	/* Limit to maximum number of textures */
	if (texObject >= MAXNUM_TEX)
		return;

	/* Only support 2D target textures : could support 1D with 1-pixel high textures, but
	 * what's the point of a 1D texture anyway? */
	if (target != GL_TEXTURE_2D)
		return;

	/* 512x512 max size */
	if ((image->Width > 512) || (image->Height > 512))
		return;

	/* Power of 2 on both axes only */
	if ( (logbase2(image->Width) < 0) || (logbase2(image->Height) < 0) )
		return;

	/* Free texture if there's one already in this slot */
//	if (ti[texObject].mipmaplevels)
//		texdel(ctx, texObject);

	/* For first level only, allocate space for all levels and clear the base data */
	if (level == 0) {
		/* At the moment, all formats go to 16-bit destination textures: also allocate enough
		 * space for any mipmaps */
		if (!(converted_texture = (word *) malloc(sizeof(word) * image->Width*image->Height * 22/16))) {
			fprintf(stderr,"S3GL: out of memory !\n");
			return;
		}
		ti[texObject].data = converted_texture;
		ti[texObject].size = 0;
		ti[texObject].have_mipmap = 0;
		ti[texObject].width = image->Width;
		ti[texObject].height = image->Height;
		ti[texObject].s3_surface.sfWidth = image->Width;
		ti[texObject].s3_surface.sfHeight = image->Height;

		/* Don't set filtering mode as it can be set before upload */
	} else {
		converted_texture = (word *) ((char *) ti[texObject].data + ti[texObject].size);
		ti[texObject].have_mipmap = 1;		// Should probably only set this when I have _all_ the mipmaps
	}

	/* Convert the texture from GL to a supported S3 format */
	if (!TexBuildS3Format(converted_texture, image, internalFormat, &transparent, &s3format)) {
#ifndef DEBUG_TEXTURES
		fprintf(stderr,"S3GL: unsupported texture in teximg()\n");
#endif
		free(converted_texture);
		return;
	}

	/* Update for this level */
	ti[texObject].size += image->Width*image->Height*2;
	ti[texObject].transparent |= transparent;
	ti[texObject].s3_surface.sfFormat = s3format | S3DTK_VIDEO;
	ti[texObject].mipmaplevels++;

	SetFilterMode(&ti[texObject]);

	/* This becomes the current texture */
	s3ctx->currenttex = texObject;
	UpdateTextureState(ctx);
}






/*
 * Set the near and far clipping planes into the driver
 *
 * Necessary because fog depends on the w values passed down (which go from n to f)
 */
static void SetNearFar( GLcontext *ctx, GLfloat n, GLfloat f)
{
	s3MesaContext s3ctx = (s3MesaContext)ctx->DriverCtx;

	if(s3ctx) {
		s3ctx->hither	= fabs(n);
		s3ctx->yon		= fabs(f);
		s3ctx->wscale	= 65535.0f / s3ctx->yon;

		/*
		 * Recompute fog coefficients
		 */
		if(ctx->Fog.Enabled) {
			switch(ctx->Fog.Mode)	{
			case GL_LINEAR:
				break;
			case GL_EXP:
				break;
			case GL_EXP2:
				break;
			default: /* That should never happen */
				break; 
			}
		}
	}
}

void s3MesaSetNearFar(GLfloat n, GLfloat f)
{
	SetNearFar(Currents3MesaCtx->gl_ctx, n, f);
}








/*
 * Configure hardware state
 */
static void SetS3RenderState(GLcontext *ctx)
{
	s3MesaContext s3ctx=(s3MesaContext)ctx->DriverCtx;

	if(ctx->Color.BlendEnabled) {
		switch(ctx->Color.BlendSrc) {
		case GL_ZERO:
		case GL_ONE:
		case GL_DST_COLOR:
		case GL_ONE_MINUS_DST_COLOR:
		case GL_SRC_ALPHA:
		case GL_ONE_MINUS_SRC_ALPHA:
		case GL_SRC_ALPHA_SATURATE:
		case GL_SRC_COLOR:
		case GL_ONE_MINUS_SRC_COLOR:
		case GL_DST_ALPHA:
		case GL_ONE_MINUS_DST_ALPHA:
		default:
			/* No src/dest factors for S3 yet - leave for later chipsets */
			break;
		}

		switch(ctx->Color.BlendDst) {
		case GL_ZERO:
		case GL_ONE:
		case GL_SRC_COLOR:
		case GL_ONE_MINUS_SRC_COLOR:
		case GL_SRC_ALPHA:
		case GL_ONE_MINUS_SRC_ALPHA:
		case GL_SRC_ALPHA_SATURATE:
		case GL_DST_COLOR:
		case GL_ONE_MINUS_DST_COLOR:
		case GL_DST_ALPHA:
		case GL_ONE_MINUS_DST_ALPHA:
		default:
			/* No src/dest factors for S3 yet - leave for later chipsets */
			break;
		}

		s3f->S3DTK_SetState(s3f, S3DTK_ALPHABLENDING, S3DTK_ALPHASOURCE);

		/* The S3 only supports one of alpha texture and alpha source... so
		 * which do I select? */
		if (ctx->Texture.Enabled && s3ctx->ti[s3ctx->currenttex].transparent) {

#ifdef DEBUG_STATE
			fprintf(stderr, "Alpha source/texture conflict!\n");
#endif

			s3f->S3DTK_SetState(s3f, S3DTK_ALPHABLENDING, S3DTK_ALPHATEXTURE);
		} else if (ctx->Fog.Enabled) {
			/* Can't do source alpha blend and fog at the same time either! */
#ifdef DEBUG_STATE
			fprintf(stderr, "Alpha source/fog conflict!\n");
#endif
			s3f->S3DTK_SetState(s3f, S3DTK_ALPHABLENDING, S3DTK_ALPHAOFF);
		}
	} else {
		if (s3ctx->ti[s3ctx->currenttex].transparent)
			s3f->S3DTK_SetState(s3f, S3DTK_ALPHABLENDING, S3DTK_ALPHATEXTURE);
		else
			s3f->S3DTK_SetState(s3f, S3DTK_ALPHABLENDING, S3DTK_ALPHAOFF);
	}

	UpdateTextureState(ctx);


	/* Depth buffer config */
	if (!ctx->Depth.Test) {
		/* Disable Z-buffer */
#ifdef DEBUG_STATE
    	fprintf(stderr,"Z-buffering not enabled\n");
#endif
		s3f->S3DTK_SetState(s3f, S3DTK_ZBUFFERENABLE, S3DTK_OFF);
	} else {
		/* Enable Z-buffer */
		s3f->S3DTK_SetState(s3f, S3DTK_ZBUFFERENABLE, S3DTK_ON);

		/* Set compare function */
		switch(ctx->Depth.Func) {
		case GL_NEVER:
			s3f->S3DTK_SetState(s3f, S3DTK_ZBUFFERCOMPAREMODE, S3DTK_ZNEVERPASS);
			break;
		case GL_LESS:
			s3f->S3DTK_SetState(s3f, S3DTK_ZBUFFERCOMPAREMODE, S3DTK_ZSRCLSZFB);
			break;
		case GL_GEQUAL:
			s3f->S3DTK_SetState(s3f, S3DTK_ZBUFFERCOMPAREMODE, S3DTK_ZSRCGEZFB);
			break;
		case GL_LEQUAL:
			s3f->S3DTK_SetState(s3f, S3DTK_ZBUFFERCOMPAREMODE, S3DTK_ZSRCLEZFB);
			break;
		case GL_GREATER:
			s3f->S3DTK_SetState(s3f, S3DTK_ZBUFFERCOMPAREMODE, S3DTK_ZSRCGTZFB);
			break;
		case GL_NOTEQUAL:
			s3f->S3DTK_SetState(s3f, S3DTK_ZBUFFERCOMPAREMODE, S3DTK_ZSRCNEZFB);
			break;
		case GL_EQUAL:
			s3f->S3DTK_SetState(s3f, S3DTK_ZBUFFERCOMPAREMODE, S3DTK_ZSRCEQZFB);
			break;
		case GL_ALWAYS:
			s3f->S3DTK_SetState(s3f, S3DTK_ZBUFFERCOMPAREMODE, S3DTK_ZALWAYSPASS);
			break;
		default:
			break;
		}
		/* Set write on or off */
		if (ctx->Depth.Mask)
			s3f->S3DTK_SetState(s3f, S3DTK_ZBUFFERUPDATEENABLE, S3DTK_ON);
		else
			s3f->S3DTK_SetState(s3f, S3DTK_ZBUFFERUPDATEENABLE, S3DTK_OFF);
	}


#ifdef DEBUG_STATE
   if(ctx->Color.AlphaEnabled)
	   fprintf(stderr, "Alpha test enabled, func %d, ref %d\n", ctx->Color.AlphaFunc, ctx->Color.AlphaRefUbyte);
#endif

#if 0
   if(ctx->Color.AlphaEnabled) {
      switch(ctx->Color.AlphaFunc) {
         case GL_NEVER:
            grAlphaTestFunction(GR_CMP_NEVER);
            break;
         case GL_LESS:
            grAlphaTestFunction(GR_CMP_LESS);
            break;
         case GL_EQUAL:
            grAlphaTestFunction(GR_CMP_EQUAL);
            break;
         case GL_LEQUAL:
            grAlphaTestFunction(GR_CMP_LEQUAL);
            break;
         case GL_GREATER:
            grAlphaTestFunction(GR_CMP_GREATER);
            break;
         case GL_NOTEQUAL:
            grAlphaTestFunction(GR_CMP_NOTEQUAL);
            break;
         case GL_GEQUAL:
            grAlphaTestFunction(GR_CMP_GEQUAL);
            break;
         case GL_ALWAYS:
            grAlphaTestFunction(GR_CMP_ALWAYS);
            break;
         default:
            break;
      }
      grAlphaTestReferenceValue(ctx->Color.AlphaRefUbyte);
   }
   else
      grAlphaTestFunction(GR_CMP_ALWAYS);
#endif




#if 0		// Disabled, as fog is playing up at the moment
	/* Fogging */
	if (ctx->Fog.Enabled) {
		word fog_colour;

		fog_colour = 	((word) (0x1f * ctx->Fog.Color[0]) << 10) |
						((word) (0x1f * ctx->Fog.Color[1]) << 5) |
						(word) (0x1f * ctx->Fog.Color[2]);
		/* In-case check */
		if (fog_colour = S3DTK_FOGOFF)
			fog_colour ++;
		s3f->S3DTK_SetState(s3f, S3DTK_FOGCOLOR, fog_colour);

		switch(ctx->Fog.Mode)	{
		case GL_LINEAR:
			/* These need to be set up then on a per-vertex basis into the alpha component */
			s3ctx->fog_start = ctx->Fog.Start;
			s3ctx->fog_end = ctx->Fog.End;
			s3ctx->fog_const = 255.0f / (s3ctx->fog_end - s3ctx->fog_start);
			break;
		case GL_EXP:
		case GL_EXP2:
			/* Use fog.Density... not supported atm */
			s3f->S3DTK_SetState(s3f, S3DTK_FOGCOLOR, S3DTK_FOGOFF);
			break;
		default:
			s3f->S3DTK_SetState(s3f, S3DTK_FOGCOLOR, S3DTK_FOGOFF);
			break;
		}

	} else
		s3f->S3DTK_SetState(s3f, S3DTK_FOGCOLOR, S3DTK_FOGOFF);
#endif
}


static const char *renderer_string(void)
{
   return "S3GL V0.1";
}




/*
 * Stub functions; prevents things that use accumulation buffers crashing
 */
void ReadColorSpan(	GLcontext *ctx,
					GLuint n, GLint x, GLint y,
					GLubyte red[], GLubyte green[],
					GLubyte blue[], GLubyte alpha[] )
{
}




/*
 * Writing of colour spans
 *
 * This is not very fast
 */
void WriteColorSpan(GLcontext *ctx,
					GLuint n, GLint x, GLint y,
					const GLubyte red[], const GLubyte green[],
					const GLubyte blue[], const GLubyte alpha[],
					const GLubyte mask[] )
{
	int i, offset;
	dword stride;
	word colour, *screen_ptr;
	LPDIRECTDRAWSURFACE *surface;
	s3MesaContext s3ctx=(s3MesaContext)ctx->DriverCtx;

	if (s3ctx->current_buffer == GL_FRONT)
		surface = s3ctx->front_surface;
	else
		surface = s3ctx->back_surface;

	if (LockSurface(surface, &screen_ptr, &stride) != DD_OK) {
		fprintf(stderr, "Lock failed in drawbitmap\n");
		return GL_TRUE;
	}
	stride >>= 1;	// convert stride to pixels

	offset = y*stride + x;

	if (mask) {
		/* draw some pixels */
		for (i=0; i<n; i++, x++) {
			if (mask[i]) {
				colour = (word)
					( ((word)0xf8 &	  red[i]) << 7) |
					( ((word)0xf8 & green[i]) << 2) |
					( ((word)0xf8 &  blue[i]) >> 3);
				screen_ptr[i+offset] = colour;
			}
		}
	} else {
		/* draw all pixels */
		for (i=0; i<n; i++, x++) {
			colour = (word)
				( ((word)0xf8 &	  red[i]) << 7) |
				( ((word)0xf8 & green[i]) << 2) |
				( ((word)0xf8 &  blue[i]) >> 3);
			screen_ptr[i+offset] = colour;
		}
	}

	UnlockSurface(surface, screen_ptr);
}



/*
 * GL render state has changed, so update
 */
static void SetRenderState(GLcontext *ctx)
{
#if defined(DEBUG_GL) || defined(DEBUG_STATE)
	fprintf(stderr, "S3MESA: SetRenderState()\n");
#endif

	SetS3RenderState(ctx);

	ctx->Driver.PointsFunc		= choose_points_function(ctx);
	ctx->Driver.LineFunc		= choose_line_function(ctx);
	ctx->Driver.TriangleFunc	= choose_triangle_function(ctx);
	ctx->Driver.QuadFunc		= choose_quad_function(ctx);
	ctx->Driver.RectFunc		= NULL;

	ctx->Driver.RasterSetup		= choose_setup_function(ctx);
	ctx->Driver.RenderVB		= NULL;
}


/*
 * Dummy renderer if rendering blocker
 */
void dummy(void)
{
}


/*
 * Build the device driver entries in the dispatch tables
 */
static void BuildDispatchTable(GLcontext *ctx)
{
	s3MesaContext s3ctx=(s3MesaContext)ctx->DriverCtx;

#if defined(DEBUG_GL) || defined (DEBUG_STATE)
	fprintf(stderr, "S3MESA: s3_mesa_setup_dd_pointers()\n");
#endif

	/* It would be preferable just to call SetRenderState here, but since
	 * the pointers can be changed at the higher level, this is not a good idea.
	 * This is a very strange bug in Mesa. */

	ctx->Driver.UpdateState		= BuildDispatchTable;
	 
	ctx->Driver.RendererString	= renderer_string;

	ctx->Driver.NearFar			= SetNearFar;

	ctx->Driver.Index			= NULL;
	ctx->Driver.Color			= set_color;
	ctx->Driver.ClearColor		= clear_color;

	ctx->Driver.AllocDepthBuffer = alloc_depth_buffer;

	ctx->Driver.SetBuffer		= set_buffer;
	ctx->Driver.GetBufferSize	= buffer_size;

	/* Texture stuff will still function even if rendering is knocked out */
	ctx->Driver.TexEnv			= texenv;
	ctx->Driver.TexImage		= teximg;
	ctx->Driver.TexParameter	= texparam;
	ctx->Driver.BindTexture		= texbind;
	ctx->Driver.DeleteTexture	= texdel;

	if (s3ctx->renderdisabled) {
		/* Blocking rendering */

		/* This currently gives a very large number of warnings */
		ctx->Driver.ClearIndex		= NULL;
		ctx->Driver.Clear			= dummy;

		ctx->Driver.ClearDepthBuffer = dummy;

		ctx->Driver.Bitmap			= dummy;
		ctx->Driver.DrawPixels		= NULL;

		ctx->Driver.Finish			= dummy;
		ctx->Driver.Flush			= dummy;

		ctx->Driver.WriteColorSpan       = dummy;
		ctx->Driver.WriteMonocolorSpan   = NULL;
		ctx->Driver.WriteColorPixels     = NULL;
		ctx->Driver.WriteMonocolorPixels = NULL;
		ctx->Driver.WriteIndexSpan       = NULL;
		ctx->Driver.WriteMonoindexSpan   = NULL;
		ctx->Driver.WriteIndexPixels     = NULL;
		ctx->Driver.WriteMonoindexPixels = NULL;

		ctx->Driver.ReadIndexSpan	= NULL;
		ctx->Driver.ReadColorSpan	= dummy;
		ctx->Driver.ReadIndexPixels = NULL;
		ctx->Driver.ReadColorPixels = NULL;


		ctx->Driver.PointsFunc		= dummy;
		ctx->Driver.LineFunc		= dummy;
		ctx->Driver.TriangleFunc	= dummy;
		ctx->Driver.QuadFunc		= dummy;
		ctx->Driver.RectFunc		= NULL;

		ctx->Driver.RasterSetup		= dummy;
		ctx->Driver.RenderVB		= NULL;


	} else {
		/* Normal, working stuff */
		ctx->Driver.ClearIndex		= NULL;
		ctx->Driver.Clear			= clear;

		ctx->Driver.ClearDepthBuffer = clear_depth_buffer;

		ctx->Driver.Bitmap			= drawbitmap;
		ctx->Driver.DrawPixels		= NULL;

		ctx->Driver.Finish			= flush;
		ctx->Driver.Flush			= finish;

		ctx->Driver.WriteColorSpan       = WriteColorSpan;
		ctx->Driver.WriteMonocolorSpan   = NULL;
		ctx->Driver.WriteColorPixels     = NULL;
		ctx->Driver.WriteMonocolorPixels = NULL;
		ctx->Driver.WriteIndexSpan       = NULL;
		ctx->Driver.WriteMonoindexSpan   = NULL;
		ctx->Driver.WriteIndexPixels     = NULL;
		ctx->Driver.WriteMonoindexPixels = NULL;

		ctx->Driver.ReadIndexSpan	= NULL;
		ctx->Driver.ReadColorSpan	= ReadColorSpan;
		ctx->Driver.ReadIndexPixels = NULL;
		ctx->Driver.ReadColorPixels = NULL;

		SetRenderState(ctx);
	}
}



s3MesaContext s3MesaCreateBestContext(GLuint win,GLint width, GLint height,
                                      const GLint attribList[])
{

	/* Need to set up the best resolution choice here in case width, height are not
	 * exactly supported */

#if USE_FULLSCREEN
	if (
		((width == 320) && (height == 200)) ||
		((width == 512) && (height == 384)) ||
		((width == 640) && (height == 480)) ||
		((width == 800) && (height == 600)) ) {
		/* Everything's OK. Nothing done here, it's just that reversing
		 * THAT if would be a bloody nightmare */
	} else {
		/* Pretty simple this */
		if (width <= 400) {
			width = 320;
			height = 200;
		} else {
			width = 640;
			height = 480;
		}
	}
#endif

	return s3MesaCreateContext(win, width, height, attribList);
}



#ifdef __WIN32__
static int cleangraphics(void)
{
   s3MesaDestroyContext(Currents3MesaCtx);

   return 0;
}
#endif





int s3mesa_context_count;		// How many device contexts are currently running

/*
 * Fundamental initialisation and device startup; this is only done if no
 * contexts are currently running.
 *
 * Initialise DirectDraw and S3 libraries; pick up the front buffer
 */
void s3MesaFundamentalInit(HWND hwnd)
{
    S3DTK_LIB_INIT lib_init;
    S3DTK_RENDERER_INITSTRUCT renderer_init;
	HRESULT result;
	ULONG rv;


#ifndef S3_SILENT
	fprintf(stderr,"Mesa S3 Device Driver\n");
#endif

	/* Initialise S3 and DirectDraw */
	if ((result = DirectDrawCreate(NULL, &gpDD, NULL)) != DD_OK) {
		fprintf(stderr, "Error starting DirectDraw\n");
		CleanExit(1);
	}

	/* Start S3 libraries */
	lib_init.libFlags = S3DTK_INITPIO | S3DTK_INIT2D_SERIALIZATION_ON;
	rv = S3DTK_InitLib((ULONG) &lib_init);
	if (rv != S3DTK_OK) {
		fprintf(stderr, "Error starting S3DTK\n");
		CleanExit(1);
	}

    /* Get the renderer object */
	renderer_init.initFlags = S3DTK_FORMAT_FLOAT | S3DTK_VERIFY_UVRANGE | S3DTK_VERIFY_XYRANGE;
	rv = S3DTK_CreateRenderer((ULONG) &renderer_init, &s3f);
	if (rv != S3DTK_OK) {
		fprintf(stderr, "Error starting S3DTK\n");
		CleanExit(1);
	}

#if !USE_FULLSCREEN
	if(IDirectDraw_SetCooperativeLevel(gpDD, hwnd, DDSCL_NORMAL) != DD_OK) {
#ifndef S3_SILENT
		fprintf(stderr, "Failed to set cooperative level %x (%d), hwnd %x\n", result, result&0xffff, hwnd);
#endif
		CleanExit(1);
	}

	/* Get the front surface used for everything */
	DDSetupPrimarySurface(&front_surface);
#endif

#ifdef DEBUG_INIT
	fprintf(stderr, "DirectDraw and S3 libraries running OK\n");
#endif
}


/*
 * Fundamental device shutdown; done when the last context is closed
 *
 * Clear out the S3 libraries and DirectDraw
 */
void s3MesaFundamentalShutdown(void)
{
	S3DTK_DestroyRenderer(&s3f);
	S3DTK_ExitLib();
	IDirectDraw_Release(gpDD);
}





/*
 * Create a new S3/Mesa context and return a handle to it.
 */
s3MesaContext s3MesaCreateContext(GLuint win, int width, int height, const GLint attribList[])
{
	s3MesaContext s3ctx;
	int i;
	GLboolean doubleBuffer = GL_FALSE;
	GLboolean alphaBuffer = GL_FALSE;
	GLint depthSize = 0;
	GLint stencilSize = 0;
	GLint accumSize = 0;
	HRESULT result;
	RECT rect;
	HWND hwnd = (HWND) win;
	S3DTK_RECTAREA s3rect;
	DDSURFACEDESC ddsd;//Raja

	BuildGammaTable();

	if (!hwnd)
		return(NULL);

	i = 0;
	while (attribList[i] != S3MESA_NONE) {
		switch (attribList[i]) {
		case S3MESA_DOUBLEBUFFER:
			doubleBuffer = GL_TRUE;
            break;
		case S3MESA_ALPHA_SIZE:
			i++;
			alphaBuffer = attribList[i] > 0;
			break;
		case S3MESA_DEPTH_SIZE:
			i++;
			depthSize = attribList[i];
			break;
		case S3MESA_STENCIL_SIZE:
			i++;
			stencilSize = attribList[i];
			break;
		case S3MESA_ACCUM_SIZE:
			i++;
			accumSize = attribList[i];
			break;
		default:
			return NULL;
		}
		i++;
	}


#ifdef DEBUG_INIT
	fprintf(stderr, "S3MESA: s3ctxCreateContext()\n");
#endif

	if (!s3mesa_context_count)
		s3MesaFundamentalInit(hwnd);

	s3ctx = (s3MesaContext) malloc(sizeof(struct s3_mesa_context));
	if(!s3ctx)
		return NULL;
	memset(s3ctx, 0, sizeof(struct s3_mesa_context));
	s3ctx->hwnd = hwnd;


	/* Before allocating any surfaces, should probably clear all texture caches in
	 * all contexts, to give a better chance of it working by defragging VRAM */


#if USE_FULLSCREEN
	/* THIS WILL PROBABLY BREAK UNDER MULTIPLE CONTEXTS */

#ifdef DEBUG_INIT
	fprintf(stderr, "Initialising for fullscreen\n");
#endif

	s3ctx->fullscreen = 1;

	if ((result = IDirectDraw_SetCooperativeLevel(  gpDD, hwnd,
													DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_NOWINDOWCHANGES)) != DD_OK) {
		fprintf(stderr, "Failed to set cooperative level %x (%d), hwnd %x\n", result, result&0xffff, hwnd);
		CleanExit(1);
	}

	if ((result = IDirectDraw_SetDisplayMode(gpDD, width, height, 16)) != DD_OK) {
		fprintf(stderr, "Failed to set display mode %x (%d), hwnd %x\n", result, result&0xffff, hwnd);
		CleanExit(1);
	}

	if ((result = DDSetupFullscreenSurfaces(&s3ctx->front_surface, &s3ctx->back_surface)) != DD_OK) {
		fprintf(stderr, "Failed to create surfaces %x (%d), hwnd %x\n", result, result&0xffff, hwnd);
		CleanExit(1);
	}
#else		// USE_FULLSCREEN

#ifdef DEBUG_INIT
	fprintf(stderr, "Initialising for windowed operation\n");
#endif

	/* Non-fullscreen setup */
	s3ctx->fullscreen = 0;


	GetWindowRect(hwnd, &rect);
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;

	/* Crowbar values in case (for example) window is not visible... in which
	 * case some strange things happen - this fixes it */
	if (!width) width = 640;
	if (!height) height = 480;

	/* This uses the normal Windows front surface we've already picked up */
	s3ctx->front_surface = front_surface;

	/* Create a back buffer specifically for this context */
	if ((result = DDSetupBackSurface(&s3ctx->back_surface, width, height)) != DD_OK) {
#ifndef S3_SILENT
		fprintf(stderr, "Failed to create back surface %x (%d), hwnd %x\n", result, result&0xffff, hwnd);
#endif
		return(NULL);
	}

	/* Non-fullscreen needs a clipper object attached to primary surface */
	if (IDirectDraw_CreateClipper(gpDD, 0, &s3ctx->clipper, NULL) != DD_OK) {
#ifndef S3_SILENT
		fprintf(stderr, "Failed to create clipper %x (%d), hwnd %x\n", result, result&0xffff, hwnd);
#endif
		IDirectDrawSurface_Release(s3ctx->back_surface);
		return(NULL);
	}
	if (IDirectDrawClipper_SetHWnd(s3ctx->clipper, 0, hwnd) != DD_OK) {
#ifndef S3_SILENT
		fprintf(stderr, "Failed to set clipper %x (%d), hwnd %x\n", result, result&0xffff, hwnd);
#endif
		IDirectDrawSurface_Release(s3ctx->back_surface);
		IDirectDrawClipper_Release(s3ctx->clipper);
		return(NULL);
	}
	if (IDirectDrawSurface_SetClipper(s3ctx->front_surface, s3ctx->clipper) != DD_OK) {
#ifndef S3_SILENT
		fprintf(stderr, "Failed to bind clipper %x (%d), hwnd %x\n", result, result&0xffff, hwnd);
#endif
		IDirectDrawSurface_Release(s3ctx->back_surface);
		IDirectDrawClipper_Release(s3ctx->clipper);
		return(NULL);
	}
#endif


#ifdef DEBUG_INIT
	fprintf(stderr, "Screen mode OK\n");
#endif

	s3ctx->width			= width;
	s3ctx->height			= height;
	s3ctx->double_buffer	= doubleBuffer;
	s3ctx->color			= PACKEDCOLOR(255,255,255,255);
	s3ctx->clearc			= 0;
	s3ctx->cleara			= 0;
	s3ctx->wscale			= 65535.0f / 100.0f;
	s3ctx->currenttex		= 0;
	s3ctx->renderdisabled	= 0;

	if (doubleBuffer)
		s3ctx->current_buffer = GL_BACK;
	else
		s3ctx->current_buffer = GL_FRONT;

	s3f->S3DTK_GetState(s3f, S3DTK_VIDEOMEMORYADDRESS, (ULONG) &s3ctx->vram_base);
	s3ctx->vram_base = LinearToPhysical(s3ctx->vram_base);
	s3f->S3DTK_SetState(s3f, S3DTK_RENDERINGTYPE, S3DTK_GOURAUD);
	s3f->S3DTK_SetState(s3f, S3DTK_TEXBLENDINGMODE, S3DTK_TEXMODULATE);

	s3ctx->s3_draw.sfWidth = width;
	s3ctx->s3_draw.sfHeight = height;
	if (s3ctx->fullscreen) 
	s3ctx->s3_draw.sfFormat = S3DTK_VIDEORGB15 | S3DTK_VIDEO;
	else
	{
		memset(&ddsd,0,sizeof(ddsd));
		ddsd.dwSize=sizeof(ddsd);
		IDirectDrawSurface2_GetSurfaceDesc(s3ctx->back_surface,&ddsd);
		if(ddsd.lPitch > ddsd.dwWidth*2) //Assume 24-bit - Not exactly right - lazy bum
		{
			fprintf(stderr,"Using 24-bit depth\n");
			s3ctx->s3_draw.sfFormat = S3DTK_VIDEORGB24 | S3DTK_VIDEO;
		}
		else
		{
			fprintf(stderr,"pitch = %d width=%d\n",ddsd.lPitch,ddsd.dwWidth);
			s3ctx->s3_draw.sfFormat = S3DTK_VIDEORGB15 | S3DTK_VIDEO;
		}
	}

	s3ctx->s3_z.sfWidth = width;
	s3ctx->s3_z.sfHeight = height;

	s3ctx->s3_z.sfFormat = S3DTK_Z16 | S3DTK_VIDEO;

	s3ctx->window_width = width;
	s3ctx->window_height = height;
	if (s3ctx->fullscreen) {
		s3ctx->screen_width = width;
		s3ctx->screen_height = height;
	} else {
		GetWindowRect(GetDesktopWindow(), &rect);
		s3ctx->screen_width = rect.right - rect.left;
		s3ctx->screen_height = rect.bottom - rect.top;
	}
	s3rect.left = 0;
	s3rect.right = width;
	s3rect.top = 0;
	s3rect.bottom = height;
	s3f->S3DTK_SetState(s3f, S3DTK_CLIPPING_AREA, (ULONG) &s3rect);

	SetRenderAddress(s3ctx);

	/* Texture cache */
	s3ctx->texture_cache_head = s3ctx->texture_cache_tail = 0;

#ifdef DEBUG_INIT
	fprintf(stderr, "Setstates done\n");
#endif

	/* Build the GL contexts to go with ours */


	s3ctx->gl_vis = gl_create_visual(	GL_TRUE,     /* RGB mode */
										alphaBuffer,
										doubleBuffer,
										depthSize,   /* depth_size */
										stencilSize, /* stencil_size */
										accumSize,   /* accum_size */
										0,           /* index bits */
										255.0,       /* color scales */
										255.0,
										255.0,
										255.0,
										8, 8, 8, 0);
	if (!s3ctx->gl_vis) {
#ifndef S3_SILENT
		fprintf(stderr, "Couldn't create visual\n");
#endif
		IDirectDrawSurface_Release(s3ctx->back_surface);
		IDirectDrawClipper_Release(s3ctx->clipper);
		return(NULL);
	}

	s3ctx->gl_ctx = gl_create_context(	s3ctx->gl_vis,
										NULL,  /* share list context */
										(void *) s3ctx);
	if (!s3ctx->gl_ctx) {
#ifndef S3_SILENT
		fprintf(stderr, "Couldn't create OpenGL context\n");
#endif
		IDirectDrawSurface_Release(s3ctx->back_surface);
		IDirectDrawClipper_Release(s3ctx->clipper);
		return(NULL);
	}

	s3ctx->gl_buffer = gl_create_framebuffer(s3ctx->gl_vis);
	if (!s3ctx->gl_buffer) {
#ifndef S3_SILENT
		fprintf(stderr, "Couldn't create framebuffer\n");
#endif
		IDirectDrawSurface_Release(s3ctx->back_surface);
		IDirectDrawClipper_Release(s3ctx->clipper);
		return(NULL);
	}

#ifdef DEBUG_INIT
	fprintf(stderr, "GL ready\n");
#endif

	BuildDispatchTable(s3ctx->gl_ctx);

	s3mesa_context_count++;

	return s3ctx;
}


/*
 * Destroy the given S3/Mesa context.
 */
void s3MesaDestroyContext(s3MesaContext ctx)
{
#ifdef DEBUG_INIT
	fprintf(stderr, "S3MESA: s3MesaDestroyContext()\n");
#endif

	if (ctx) {
		if (ctx->clipper)
			IDirectDrawClipper_Release(ctx->clipper);
		if (ctx->z_surface)
			IDirectDrawSurface_Release(ctx->z_surface);
		if (ctx->back_surface)
			IDirectDrawSurface_Release(ctx->back_surface);

		gl_destroy_visual(ctx->gl_vis);
		gl_destroy_context(ctx->gl_ctx);
		gl_destroy_framebuffer(ctx->gl_buffer);

		s3mesa_context_count--;

		if (!s3mesa_context_count)
			s3MesaFundamentalShutdown();

		free(ctx);
	}

	if (ctx == Currents3MesaCtx)
		Currents3MesaCtx = NULL;
}

/*
 * Make the specified S3/Mesa context the current one.
 */
void s3MesaMakeCurrent(s3MesaContext ctx)
{
#ifdef DEBUG_GL
	fprintf(stderr, "S3MESA: s3MesaMakeCurrent()\n");
#endif

	if (!ctx) {
		gl_make_current(NULL,NULL);
		Currents3MesaCtx=NULL;

		return;
	}

	Currents3MesaCtx = ctx;

	gl_make_current(ctx->gl_ctx,ctx->gl_buffer);

	SetRenderState(ctx->gl_ctx);
	gl_Viewport(ctx->gl_ctx,0,0,ctx->width,ctx->height);
}


/*
 * Swap front/back buffers for current context if double buffered.
 */
void s3MesaSwapBuffers(void)
{
	s3MesaContext s3ctx = Currents3MesaCtx;

#if defined(DEBUG_GL) || defined(DEBUG_SURFACES)
	fprintf(stderr, "S3MESA: s3MesaSwapBuffers()\n");
#endif

	if (s3ctx->double_buffer)
		DDDoubleBuffer( s3ctx->hwnd, s3ctx->front_surface, s3ctx->back_surface,
						s3ctx->width, s3ctx->height);

	SetRenderAddress(s3ctx);
}


/*
 * Called whenever a WM_MOVE message is received
 */
void s3MesaMoveWindow(s3MesaContext s3ctx, int x, int y)
{
#if USE_FULLSCREEN
#else
#endif
}

/*
 * Called whenever a WM_SIZE message is received
 */
void s3MesaResizeWindow(s3MesaContext s3ctx, int w, int h)
{
#if USE_FULLSCREEN
#else
	int reallocate_back = 0, reallocate_z = 0;

	w = (w+3) & ~3;

#ifdef DEBUG_SURFACES
	fprintf(stderr, "Resizing to %d, %d\n", w, h);
#endif

	/* Have to resize back and z surfaces: first, free everything but the front
	 * surface, to clear video memory */
	if (s3ctx->back_surface) {
		IDirectDrawSurface_Release(s3ctx->back_surface);
		reallocate_back = 1;
		s3ctx->back_surface = NULL;
	}
	if (s3ctx->z_surface) {
		IDirectDrawSurface_Release(s3ctx->z_surface);
		reallocate_z = 1;
		s3ctx->z_surface = NULL;
	}
	ClearTextureCache(s3ctx);

	/* Mark as resized in the S3 surfaces */
	s3ctx->s3_draw.sfWidth = w;
	s3ctx->s3_draw.sfHeight = h;
	s3ctx->s3_z.sfWidth = w;
	s3ctx->s3_z.sfHeight = h;
	s3ctx->window_width = w;
	s3ctx->window_height = h;

	/* Mark as resized in the context */
	s3ctx->width = w;
	s3ctx->height = h;

	/* Reallocate if there was one there before */
	s3ctx->renderdisabled &= ~1;
	if (reallocate_back) {
		if (DDSetupBackSurface(&s3ctx->back_surface, w, h) != DD_OK) {
#ifndef S3_SILENT
			fprintf(stderr, "Failed to resize back buffer\n");
#endif
			s3ctx->renderdisabled |= 1;
			BuildDispatchTable(s3ctx->gl_ctx);
			return;
		}
	}
	if (reallocate_z) {
		if (DDSetupZSurface(&s3ctx->z_surface, w, h) != DD_OK) {
#ifndef S3_SILENT
			fprintf(stderr, "Failed to resize Z buffer\n");
#endif
			s3ctx->renderdisabled |= 1;
			BuildDispatchTable(s3ctx->gl_ctx);
			return;
		}
	}

	if (s3ctx == Currents3MesaCtx)
		SetRenderAddress(s3ctx);
#endif
}



#endif  /* S3 */
