/*
 * s3wgl.c
 *
 * Originally from Mesa 3Dfx driver, rewritten and bugfixed
 */

#ifdef S3

#ifdef __cplusplus
extern "C" {
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#ifdef __cplusplus
}
#endif

#include <stdio.h>
#include <windows.h>

#include "s3mesa.h"

#define _DLLEXPORT_				__declspec(dllexport)
#define MAX_MESA_ATTRS	20

//#define DEBUG_WGL

struct __extensions__
{
	PROC	proc;
	char	*name;
};

struct __extensions__	ext[] = {

#ifdef GL_EXT_polygon_offset
	{	(PROC)glPolygonOffsetEXT,		"glPolygonOffsetEXT"		},
#endif
	{	(PROC)glBlendEquationEXT,		"glBlendEquationEXT"		},
	{	(PROC)glBlendColorEXT,			"glBlendColorExt"			},
	{	(PROC)glVertexPointerEXT,		"glVertexPointerEXT"		},
	{	(PROC)glNormalPointerEXT,		"glNormalPointerEXT"		},
	{	(PROC)glColorPointerEXT,		"glColorPointerEXT"			},
	{	(PROC)glIndexPointerEXT,		"glIndexPointerEXT"			},
	{	(PROC)glTexCoordPointerEXT,		"glTexCoordPointer"			},
	{	(PROC)glEdgeFlagPointerEXT,		"glEdgeFlagPointerEXT"		},
	{	(PROC)glGetPointervEXT,			"glGetPointervEXT"			},
	{	(PROC)glArrayElementEXT,		"glArrayElementEXT"			},
	{	(PROC)glDrawArraysEXT,			"glDrawArrayEXT"			},
	{	(PROC)glAreTexturesResidentEXT,	"glAreTexturesResidentEXT"	},
	{	(PROC)glBindTextureEXT,			"glBindTextureEXT"			},
	{	(PROC)glDeleteTexturesEXT,		"glDeleteTexturesEXT"		},
	{	(PROC)glGenTexturesEXT,			"glGenTexturesEXT"			},
	{	(PROC)glIsTextureEXT,			"glIsTextureEXT"			},
	{	(PROC)glPrioritizeTexturesEXT,	"glPrioritizeTexturesEXT"	},
	{	(PROC)glCopyTexSubImage3DEXT,	"glCopyTexSubImage3DEXT"	},
	{	(PROC)glTexImage3DEXT,			"glTexImage3DEXT"			},
	{	(PROC)glTexSubImage3DEXT,		"glTexSubImage3DEXT"		},
};
int				qt_ext = sizeof(ext) / sizeof(ext[0]);



/* See PIXELFORMATDESCRIPTOR in MSVC help */

struct __pixelformat__
{
	PIXELFORMATDESCRIPTOR	pfd;						// The standard Windows DC pixelformat
	GLint					mesaAttr[MAX_MESA_ATTRS];	// The attributes that will be interpreted by the driver
};

/* Our supported pixelformats; all reported as 32-bit */
struct __pixelformat__	pix[] = 
{
	/* Depth cases must come before non-depth cases, so in the case of the application requesting
	 * a Z-buffer depth other than 16-bits one is still allocated even if it isn't as wide
	 * as that requested */

	/* With depth buffer, without double buffer */
	{	{	sizeof(PIXELFORMATDESCRIPTOR),	1,
			PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_GENERIC_ACCELERATED|PFD_SUPPORT_GDI,
			PFD_TYPE_RGBA,
			24,	8,	0,	8,	8,	8,	16,	8,	24,
			8,	2,	2,	2,	2,	16,	8,	0,	0,	0,	0,	0,	0	},
		{
			S3MESA_ALPHA_SIZE,		8,
			S3MESA_DEPTH_SIZE,		16,
			S3MESA_STENCIL_SIZE,	8,
			S3MESA_ACCUM_SIZE,		8,
			S3MESA_NONE	}
	},

	/* Without depth buffer and double buffer */
	{	{	sizeof(PIXELFORMATDESCRIPTOR),	1,
			PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_GENERIC_ACCELERATED|PFD_SUPPORT_GDI,
			PFD_TYPE_RGBA,
			24,	8,	0,	8,	8,	8,	16,	8,	24,
			8,	2,	2,	2,	2,	0,	8,	0,	0,	0,	0,	0,	0	},
		{
			S3MESA_ALPHA_SIZE,		8,
			S3MESA_DEPTH_SIZE,		0,
			S3MESA_STENCIL_SIZE,	8,
			S3MESA_ACCUM_SIZE,		8,
			S3MESA_NONE	}
	},

	/* With depth buffer & double buffer */
	{	{	sizeof(PIXELFORMATDESCRIPTOR),	1,
			PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_GENERIC_ACCELERATED|PFD_DOUBLEBUFFER|PFD_SUPPORT_GDI,
			PFD_TYPE_RGBA,
			24,	8,	0,	8,	8,	8,	16,	8,	24,
			8,	2,	2,	2,	2,	16,	8,	0,	0,	0,	0,	0,	0	},
		{
			S3MESA_DOUBLEBUFFER,
			S3MESA_ALPHA_SIZE,		8,
			S3MESA_DEPTH_SIZE,		16,
			S3MESA_STENCIL_SIZE,	8,
			S3MESA_ACCUM_SIZE,		8,
			S3MESA_NONE	}
	},

	/* Without depth buffer, with double buffer */
	{	{	sizeof(PIXELFORMATDESCRIPTOR),	1,
			PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_GENERIC_ACCELERATED|PFD_DOUBLEBUFFER|PFD_SUPPORT_GDI,
			PFD_TYPE_RGBA,
			24,	8,	0,	8,	8,	8,	16,	8,	24,
			8,	2,	2,	2,	2,	0,	8,	0,	0,	0,	0,	0,	0	},
		{
			S3MESA_DOUBLEBUFFER,
			S3MESA_ALPHA_SIZE,		8,
			S3MESA_DEPTH_SIZE,		0,
			S3MESA_STENCIL_SIZE,	8,
			S3MESA_ACCUM_SIZE,		8,
			S3MESA_NONE	}
	},

};
int				qt_pix = sizeof(pix) / sizeof(pix[0]);		// Number of pixelformats in the driver


static int				curPFD = 0;

#define MAX_CONTEXTS		32			// THIS MUST NOT BE CHANGED without redoing the MonitorProcs

/* This structure contains enough information to let us manage everything */
typedef struct {
	s3MesaContext	s3ctx;
	HDC				hDC;
	WNDPROC			hWNDOldProc;
	HWND			hWND;
} Contextlist;
static Contextlist	ctxlist[MAX_CONTEXTS+1];
static int	context_count = 0;				// Contexts allocated
static int current_context = 0;				// Handle of the current context: 0 means no context
static s3MesaContext	ctx = NULL;			// Current rendering context: NULL is no context


#if 0

/*
 * This monitors all the Windows messages that comes through, and performs any tasks that
 * need to be done - particularly, resizes and moves in Windows mode.
 */
LONG APIENTRY __wglMonitor(HWND hwnd,UINT message,UINT wParam,LONG lParam)
{
	if (ctx && hwnd == hWND)
	{
		switch(message)
		{
			case WM_MOVE:
				s3MesaMoveWindow(ctx, lParam&0xffff, lParam>>16);
				break;

			case WM_DISPLAYCHANGE:
			case WM_SIZE:
				s3MesaResizeWindow(ctx, lParam&0xffff, lParam>>16);
				break;

			case WM_ACTIVATE:
				/* In fullscreen mode, probably need to do something here */
				break;
		}
	}

	return(hWNDOldProc(hwnd,message,wParam,lParam));
}
#endif

#define MONITORPROC(a) \
LONG APIENTRY __wglMonitor##a(HWND hwnd,UINT message,UINT wParam,LONG lParam) \
{\
	if (ctx && (ctx == ctxlist[a].s3ctx) && (hwnd == ctxlist[a].hWND)) {	\
		switch(message) {		\
		case WM_MOVE:			\
			s3MesaMoveWindow(ctxlist[a].s3ctx, lParam&0xffff, lParam>>16);	\
			break;				\
		case WM_DISPLAYCHANGE:	\
		case WM_SIZE:			\
			s3MesaResizeWindow(ctxlist[a].s3ctx, lParam&0xffff, lParam>>16);	\
			break;				\
		case WM_ACTIVATE:		\
			/* In fullscreen mode, probably need to do something here */	\
			break;				\
		}	\
	}		\
	return(ctxlist[a].hWNDOldProc(hwnd, message, wParam, lParam));	\
}

MONITORPROC( 0);	MONITORPROC( 1); MONITORPROC( 2); MONITORPROC( 3); MONITORPROC( 4);
MONITORPROC( 5);	MONITORPROC( 6); MONITORPROC( 7); MONITORPROC( 8); MONITORPROC( 9);
MONITORPROC(10);	MONITORPROC(11); MONITORPROC(12); MONITORPROC(13); MONITORPROC(14);
MONITORPROC(15);	MONITORPROC(16); MONITORPROC(17); MONITORPROC(18); MONITORPROC(19);
MONITORPROC(20);	MONITORPROC(21); MONITORPROC(22); MONITORPROC(23); MONITORPROC(24);
MONITORPROC(25);	MONITORPROC(26); MONITORPROC(27); MONITORPROC(28); MONITORPROC(29);
MONITORPROC(30);	MONITORPROC(31); MONITORPROC(32);

WNDPROC monitorproclist[MAX_CONTEXTS+1] =  {
	__wglMonitor0,  __wglMonitor1,  __wglMonitor2,  __wglMonitor3,  __wglMonitor4,
	__wglMonitor5,  __wglMonitor6,  __wglMonitor7,  __wglMonitor8,  __wglMonitor9,
	__wglMonitor10, __wglMonitor11, __wglMonitor12, __wglMonitor13, __wglMonitor14,
	__wglMonitor15, __wglMonitor16, __wglMonitor17, __wglMonitor18, __wglMonitor19,
	__wglMonitor20, __wglMonitor21, __wglMonitor22, __wglMonitor23, __wglMonitor24,
	__wglMonitor25, __wglMonitor26, __wglMonitor27, __wglMonitor28, __wglMonitor29,
	__wglMonitor30, __wglMonitor31, __wglMonitor32
};



/*
 * CopyContext copies the state from one context to another - not currently supported
 */
_DLLEXPORT_ BOOL APIENTRY wglCopyContext(HGLRC hglrcSrc,HGLRC hglrcDst,UINT mask)
{
	return(FALSE);
}


/*
 * CreateContext - creates an OpenGL rendering context to work onto a Windows DC
 */
_DLLEXPORT_ HGLRC APIENTRY wglCreateContext(HDC hdc)
{
	HWND		hWnd;
	WNDPROC	oldProc;
	int slot;

	/* Fail if we have allocated all our contexts */
	if (context_count == MAX_CONTEXTS)
	{
		SetLastError(0);
		return(NULL);
	}
	/* No window; nothing to render on; abort */
	if(!(hWnd = WindowFromDC(hdc)))
	{
		SetLastError(0);
		return(NULL);
	}
	/* A valid pixelformat must have been set into the DC and therefore our driver */
	if(curPFD == 0)
	{
		SetLastError(0);
		return(NULL);
	}
	/* Set up the log */
	fclose(stderr);
	fopen("MESA.LOG","w");

	/* Find an empty slot */
	for (slot=1; slot<MAX_CONTEXTS+1; slot++) {
		if (!ctxlist[slot].s3ctx)
			break;
	}
	if (slot == (MAX_CONTEXTS+1)) {
		SetLastError(0);
		return(NULL);
	}

	/* Set up our monitoring of the window procedure, to catch any critical Windows messages */
	if ((oldProc = (WNDPROC) GetWindowLong(hWnd, GWL_WNDPROC)) != monitorproclist[slot])
	{
		ctxlist[slot].hWNDOldProc = oldProc;
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)monitorproclist[slot]);
	}

	/* Create the S3Mesa context */
	if(!(ctxlist[slot].s3ctx = s3MesaCreateBestContext((GLuint)hWnd, 0, 0, pix[curPFD - 1].mesaAttr)))
	{
		SetLastError(0);
		return(NULL);
	}
	context_count++;

	/* Save our critical internals */
	ctxlist[slot].hDC	= hdc;
	ctxlist[slot].hWND	= hWnd;

	/* And... we're off! */
	return((HGLRC) slot);
}

/*
 * Layers not supported
 */
_DLLEXPORT_ HGLRC APIENTRY wglCreateLayerContext(HDC hdc,int iLayerPlane)
{
	SetLastError(0);
	return(NULL);
}

/*
 * DeleteContext - kill the current context and shut down OpenGL
 */
_DLLEXPORT_ BOOL APIENTRY wglDeleteContext(HGLRC hglrc)
{
	if (((int) hglrc < (MAX_CONTEXTS+1)) && ctxlist[(int) hglrc].s3ctx)
	{
		s3MesaDestroyContext(ctxlist[(int) hglrc].s3ctx);
		ctxlist[(int) hglrc].s3ctx = 0;
		context_count--;
		return(TRUE);

		if ((int) hglrc == current_context) {
			current_context = 0;
			ctx = 0;
		}
	}
	SetLastError(0);
	return(FALSE);
}

/*
 * GetCurrentContext - return it
 */
_DLLEXPORT_ HGLRC APIENTRY wglGetCurrentContext(VOID)
{
	if (current_context)
		return((HGLRC) current_context);

	SetLastError(0);
	return(NULL);
}

/*
 * GetCurrentDC - return the DC associated with the rendering context
 */
_DLLEXPORT_ HDC APIENTRY wglGetCurrentDC(VOID)
{
	if (ctx)
		return(ctxlist[current_context].hDC);

	SetLastError(0);
	return(NULL);
}

/*
 * GetProcAddress - return the address of an appropriate extension
 */
_DLLEXPORT_ PROC APIENTRY wglGetProcAddress(LPCSTR lpszProc)
{
	int		i;

#ifdef DEBUG_WGL
	fprintf(stderr, "Attempt to reference extension %s\n", lpszProc);
#endif

	for(i = 0;i < qt_ext;i++)
		if(!strcmp(lpszProc,ext[i].name))
			return(ext[i].proc);

	SetLastError(0);
	return(NULL);
}

/*
 * MakeCurrent - make a particular context the 'current' one
 */
_DLLEXPORT_ BOOL APIENTRY wglMakeCurrent(HDC hdc, HGLRC hglrc)
{
	if (hglrc == 0) {
		ctx = NULL;
		s3MesaMakeCurrent(ctx);
		return(TRUE);
	}

	if (((int) hglrc >= (MAX_CONTEXTS+1)) ||
		(ctxlist[(int) hglrc].s3ctx == NULL) ||
		WindowFromDC(hdc) != ctxlist[(int) hglrc].hWND)
	{
		SetLastError(0);
		return(FALSE);
	}

	ctxlist[current_context].hDC = hdc;
	ctx = ctxlist[(int) hglrc].s3ctx;
	s3MesaMakeCurrent(ctx);

	return(TRUE);
}

/*
 * ShareLists - not supported, but return something appropriate in idiot cases
 */
_DLLEXPORT_ BOOL APIENTRY wglShareLists(HGLRC hglrc1,HGLRC hglrc2)
{
	if (hglrc1 != hglrc2)
	{
		SetLastError(0);
		return(FALSE);
	}
	return(TRUE);
}


/*
 * Font functions: not supported
 */
_DLLEXPORT_ BOOL APIENTRY wglUseFontBitmapsA(HDC hdc,DWORD first,DWORD count,DWORD listBase)
{
	return(FALSE);
}

_DLLEXPORT_ BOOL APIENTRY wglUseFontBitmapsW(HDC hdc,DWORD first,DWORD count,DWORD listBase)
{
	return(wglUseFontBitmapsA(hdc, first, count, listBase));
}

_DLLEXPORT_ BOOL APIENTRY wglUseFontOutlinesA(HDC hdc,DWORD first,DWORD count,
										  DWORD listBase,FLOAT deviation,
										  FLOAT extrusion,int format,
										  LPGLYPHMETRICSFLOAT lpgmf)
{
	SetLastError(0);
	return(FALSE);
}

_DLLEXPORT_ BOOL APIENTRY wglUseFontOutlinesW(HDC hdc,DWORD first,DWORD count,
										  DWORD listBase,FLOAT deviation,
										  FLOAT extrusion,int format,
										  LPGLYPHMETRICSFLOAT lpgmf)
{
	SetLastError(0);
	return(FALSE);
}


/*
 * Layers not supported
 */
_DLLEXPORT_ BOOL APIENTRY wglDescribeLayerPlane(HDC hdc,int iPixelFormat,
											int iLayerPlane,UINT nBytes,
											LPLAYERPLANEDESCRIPTOR plpd)
{
	SetLastError(0);
	return(FALSE);
}

_DLLEXPORT_ int APIENTRY wglSetLayerPaletteEntries(HDC hdc,int iLayerPlane,
											   int iStart,int cEntries,
											   CONST COLORREF *pcr)
{

	fprintf(stderr, "SetLayerPaletteEntries\n");

	SetLastError(0);
	return(0);
}

_DLLEXPORT_ int APIENTRY wglGetLayerPaletteEntries(HDC hdc,int iLayerPlane,
											   int iStart,int cEntries,
											   COLORREF *pcr)
{
	fprintf(stderr, "GetLayerPaletteEntries\n");

	SetLastError(0);
	return(0);
}

_DLLEXPORT_ BOOL APIENTRY wglRealizeLayerPalette(HDC hdc,int iLayerPlane,BOOL bRealize)
{
	fprintf(stderr, "RealizeLayerPaletteEntries\n");

	SetLastError(0);
	return(FALSE);
}

_DLLEXPORT_ BOOL APIENTRY wglSwapLayerBuffers(HDC hdc,UINT fuPlanes)
{
	if (ctx &&
		(WindowFromDC(hdc) == ctxlist[current_context].hWND) &&
		(fuPlanes == WGL_SWAP_MAIN_PLANE))
	{
		s3MesaSwapBuffers();
		return(TRUE);
	}
	SetLastError(0);
	return(FALSE);
}


/* Pixel Format functions */

/*
 * ChoosePixelFormat - compare our supported device pixelformats against the one
 * supplied and match as best we can
 */
_DLLEXPORT_ int APIENTRY wglChoosePixelFormat(HDC hdc,
											CONST PIXELFORMATDESCRIPTOR *ppfd)
{
	int		i,best = -1,bestdelta = 0x7FFFFFFF,delta;

	if(ppfd->nSize != sizeof(PIXELFORMATDESCRIPTOR) || ppfd->nVersion != 1)
	{
		SetLastError(0);
		return(0);
	}

	for(i = 0;i < qt_pix;i++)
	{
		delta = 0;	// 'difference' counter

		/* Any of these failing is a critical fail */
		if((ppfd->dwFlags & PFD_DRAW_TO_WINDOW) && !(pix[i].pfd.dwFlags & PFD_DRAW_TO_WINDOW))
			continue;
		if((ppfd->dwFlags & PFD_DRAW_TO_BITMAP) && !(pix[i].pfd.dwFlags & PFD_DRAW_TO_BITMAP))
			continue;
		if((ppfd->dwFlags & PFD_SUPPORT_GDI) && !(pix[i].pfd.dwFlags & PFD_SUPPORT_GDI))
			continue;
		if((ppfd->dwFlags & PFD_SUPPORT_OPENGL) && !(pix[i].pfd.dwFlags & PFD_SUPPORT_OPENGL))
			continue;
		if(!(ppfd->dwFlags & PFD_DOUBLEBUFFER_DONTCARE) &&
			 ((ppfd->dwFlags & PFD_DOUBLEBUFFER) != (pix[i].pfd.dwFlags & PFD_DOUBLEBUFFER)))
			continue;
		if(!(ppfd->dwFlags & PFD_STEREO_DONTCARE) &&
			 ((ppfd->dwFlags & PFD_STEREO) != (pix[i].pfd.dwFlags & PFD_STEREO)))
			continue;

		/* These are non-critical fails - match as best we can */
		if(ppfd->iPixelType != pix[i].pfd.iPixelType)
			delta++;
		if (ppfd->cDepthBits != pix[i].pfd.cDepthBits)
			delta++;

		/* Is this the best? */
		if(delta < bestdelta)
		{
			best = i + 1;
			bestdelta = delta;
			if(bestdelta == 0)
				break;
		}
	}

	/* not found.... whoops! */
	if(best == -1)
	{
		SetLastError(0);
		return(0);
	}
	return(best);
}

/*
 * DescribePixelFormat
 */
_DLLEXPORT_ int APIENTRY wglDescribePixelFormat(HDC hdc,int iPixelFormat,UINT nBytes,
									LPPIXELFORMATDESCRIPTOR ppfd)
{
	if(iPixelFormat < 1 || iPixelFormat > qt_pix || nBytes != sizeof(PIXELFORMATDESCRIPTOR))
	{
		SetLastError(0);
		return(0);
	}
	*ppfd = pix[iPixelFormat - 1].pfd;
	return(qt_pix);
}

/* UNKNOWN !!!
wglGetDefaultProcAddress()
*/

/*
 * GetPixelFormat
 */
_DLLEXPORT_ int APIENTRY wglGetPixelFormat(HDC hdc)
{
	if(curPFD == 0)
	{
		SetLastError(0);
		return(0);
	}
	return(curPFD);
}

/*
 * SetPixelFormat
 */
_DLLEXPORT_ BOOL APIENTRY wglSetPixelFormat(HDC hdc,int iPixelFormat,
								PIXELFORMATDESCRIPTOR *ppfd)
{
	if(iPixelFormat < 1 || iPixelFormat > qt_pix || ppfd->nSize != sizeof(PIXELFORMATDESCRIPTOR))
	{
		SetLastError(0);
		return(FALSE);
	}
	curPFD = iPixelFormat;
	return(TRUE);
}



/*
 * Double Buffer call
 */
_DLLEXPORT_ BOOL APIENTRY wglSwapBuffers(HDC hdc)
{
	if(!ctx)
	{
		SetLastError(0);
		return(FALSE);
	}
	s3MesaSwapBuffers();
	return(TRUE);
}

#endif /* S3 */

