/*
	File:		fxgli_tridebug.c

	Contains:	Contains code for low-level debugging.

	Written by:	miklos

	Copyright:	Copyright ©

	Change History (most recent first):

         <2>      7/5/99    miklos  Revison.
         <1>     6/18/99    ???     Initial revision
*/



#if GLI_DEBUG_FX_TRIANGLES || GLI_PROFILE_FX_TRIANGLES
	#undef grDrawTriangle
	#undef grAADrawTriangle
#endif

#if GLI_DEBUG_FX_LINES
	#undef grDrawLine
	#undef grAADrawLine
#endif

#if GLI_DEBUG_FX_POINTS
	#undef grDrawPoint
	#undef grAADrawPoint
#endif

#if GLI_PROFILE_FX_DRAW_ARRAY || GLI_DEBUG_FX_TRIANGLES
        #undef grDrawVertexArray
#endif

#include "fxgli.h"
#include "glm.h"

/* 3Dfx Glide */
#include <glide.h>

/* Mesa */
#include "types.h"
#include "vb.h"
#include "fxdrv.h"
#include "context.h"

#if GLI_DEBUG_FX_TRIANGLES || GLI_PROFILE_FX_TRIANGLES

#if GLI_DEBUG_FX_TRIANGLES
extern TFXContext *current;
GLfloat oozMin = 100000;
GLfloat oozMax = -100;
static int CheckVetrex(const GrVertex *vptr)
{
	fxMesaContext fxMesa = fxMesaGetCurrentContext();
	
	/* Update nearMin/farMax */
	float x,y;
	
	
	x = vptr->x;
	y = vptr->y;
	
		
	x = ((int)((vptr)->x*16.0))*(1.0/16.0);
	y = ((int)((vptr)->y*16.0))*(1.0/16.0);
		
	/* Error In vertex snapping */
	if ((x != (vptr)->x) || (y != (vptr)->y))
	{
		return 0;
	}
	
	/* Error in coordinates */
	if (((vptr)->x > fxMesa->width) || ((vptr)->x < -1.0))
	{
		return 0;
	}
	
	if (((vptr)->y > fxMesa->height) || ((vptr)->y < -1.0))
	{
		return 0;
	}
	
	/*
	Don't check for RGBA, since Flat colored doesn't requires theses.
	if ((((vptr)->r > 255.0) || ((vptr)->g > 255.0) || ((vptr)->b > 255.0) ||
		((vptr)->a > 255.0)) || (((vptr)->r < 0.0) || 
		((vptr)->g < 0.0) || ((vptr)->b < 0.0) ||
		((vptr)->a < 0.0)))
		return 0;
	*/	 
	if (fxMesa->unitsState.depthTestEnabled && !(!fxMesa->unitsState.depthMask && fxMesa->unitsState.depthTestFunc == 7))
	{   
		if ((vptr)->ooz < oozMin) 
			oozMin=(vptr)->ooz; 
		if ((vptr)->ooz > oozMax) 
			oozMax=(vptr)->ooz; 
		
		
		if (((vptr)->ooz < 0.0) || ((vptr)->ooz > 65535.0))
		{ 
			return 0;
		}
		
	}
	
	return 1;
}
#endif
extern void __grDrawTriangle(const GrVertex *a, const GrVertex *b, const GrVertex *c);
extern void __grAADrawTriangle(const GrVertex *a, const GrVertex *b, const GrVertex *c,FxBool ab_antialias, FxBool bc_antialias, FxBool ca_antialias);
extern void __grDrawVertexArray(FxU32 mode, FxU32 Count, void *pointers);
static int width = 640;
static int height = 480;
static int real_width = 640;
static int real_height = 480;

#if GLI_PROFILE_FX_DRAW_ARRAY || GLI_DEBUG_FX_TRIANGLES
void __grDrawVertexArray(FxU32 mode, FxU32 Count, void *pointers)
{
#if GLI_DEBUG_FX_TRIANGLES
   {
      int i;
      for (i = 0; i< Count; i++)
      {
        if (!CheckVetrex(((GrVertex**)pointers)[i]))
           Debugger();
      }
   }
#endif
#if FX_GLIDE3
   grDrawVertexArray(mode,Count,pointers);
#endif
}
#endif



void __grDrawTriangle(const GrVertex *a,const GrVertex *b,const GrVertex *c)
{
#if GLI_DEBUG_FX_TRIANGLES
	fxMesaContext fxMesa = fxMesaGetCurrentContext();
	float left,right,top,bottom;
	int print_vb = 0;
	
	left =  a->x;
	
	if (b->x < left)
		left = b->x;
	if (c->x <left )
		left = c->x;
	
	right = a->x;
	if (b->x > right)
		right = b->x;
	if (c->x > right )
		right = c->x;
	
	top = a->y;
	if (b->y > top)
		top = b->y;
	if (c->y > top )
		top = c->y;
		
	bottom = a->y;
	if (b->y < bottom)
		bottom = b->y;
	if (c->y < bottom )
		bottom = c->y;
	
	/*if (print_vb)
	{
		gl_print_cassette(fxMesa->glCtx->VB->IM,GL_FALSE,VERT_OBJ_ANY);
	}*/
	
	if (!CheckVetrex(a) || !CheckVetrex(b) || !CheckVetrex(c))
	{
		Debugger();
		
		grDrawTriangle(a,b,c); 
	}
	else
	{	
		grDrawTriangle(a,b,c); 
		
		/* Draw them as lines: */
		/*
		grDrawLine(a,b);
		grDrawLine(b,c);
		grDrawLine(c,a);
		*/
	}
#else
	/*
	GrVertex cpa,cpb,cpc;
	cpa = *a;
	cpb = *b;
	cpc = *c;
	cpa.x = cpa.x/(float)width;
	cpa.x = cpa.x*(float)real_width;
	cpa.y = cpa.y/(float)height;
	cpa.y = cpa.y*(float)real_height;
	cpb.x = cpb.x/(float)width;
	cpb.x = cpb.x*(float)real_width;
	cpb.y = cpb.y/(float)height;
	cpb.y = cpb.y*(float)real_height;
	cpc.x = cpc.x/(float)width;
	cpc.x = cpc.x*(float)real_width;
	cpc.y = cpc.y/(float)height;
	cpc.y = cpc.y*(float)real_height;
	grDrawTriangle(&cpa,&cpb,&cpc);
	*/
	grDrawTriangle(a,b,c);
#endif
}

#endif /* GLI_DEBUG_FX_TRIANGLES */

#ifdef GLI_DEBUG_FX_LINES
extern void __grDrawLine(const GrVertex *a, const GrVertex *b);
void __grDrawLine(const GrVertex *a,const GrVertex *b)
{
	float left,right,top,bottom;
	GrColor_t color;
	fxMesaContext fxMesa = fxMesaGetCurrentContext();
	color = fxMesa->color;
	
	right = left = a->x;
	bottom = top = a->y;
	if(b->x < left)
		left = b->x;
	if (b->x > right)
		right = b->x;
	if (b->y < top)
		top = b->y;
	if (b->y > bottom)
		bottom = b->y;
	
	grDrawLine(a,b);
	return;
}
#endif

#ifdef GLI_DEBUG_FX_POINTS
extern void __grDrawPoint(const GrVertex *a);
extern void __grAADrawPoint(const GrVertex *a);
void __grDrawPoint(const GrVertex *a)
{
	grDrawPoint(a);
}
#endif