/* -*- mode: C; tab-width:8; c-basic-offset:3 -*- */

/*
 * Mesa 3-D graphics library
 * Version:  3.4
 *
 * Copyright (C) 1999-2000  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef TYPES_H
#define TYPES_H


#include "glheader.h"
#include "config.h"
#include "fixed.h"
#include "glapitable.h"
#include "glthread.h"
#include "macros.h"
#include "matrix.h"
#include "vb.h"
#include "vector.h"


/*
 * Color channel data type:
 */
#if CHAN_BITS==8
   typedef GLubyte GLchan;
#elif CHAN_BITS==16
   typedef GLushort GLchan;
#else
   illegal number of color channel bits
#endif


/*
 * Accumulation buffer data type:
 */
#if ACCUM_BITS==8
   typedef GLbyte GLaccum;
#elif ACCUM_BITS==16
   typedef GLshort GLaccum;
#else
#  error "illegal number of accumulation bits"
#endif


/*
 * Stencil buffer data type:
 */
#if STENCIL_BITS==8
   typedef GLubyte GLstencil;
#  define STENCIL_MAX 0xff
#elif STENCIL_BITS==16
   typedef GLushort GLstencil;
#  define STENCIL_MAX 0xffff
#else
#  error "illegal number of stencil bits"
#endif


/*
 * Depth buffer data type:
 */
typedef GLuint GLdepth;  /* Must be 32-bits! */



/*
 * Some forward type declarations
 */
struct _mesa_HashTable;

typedef struct gl_visual GLvisual;

typedef struct gl_frame_buffer GLframebuffer;


/*
 * Functions for transformation of normals in the VB.
 */
typedef void (_NORMAPIP normal_func)( const GLmatrix *mat,
			     GLfloat scale,
			     const GLvector3f *in,
			     const GLfloat lengths[],
			     const GLubyte mask[],
			     GLvector3f *dest );



/*
 * Point, line, triangle, quadrilateral and rectangle rasterizer functions:
 */
typedef void (*points_func)( GLcontext *ctx, GLuint first, GLuint last );

typedef void (*line_func)( GLcontext *ctx, GLuint v1, GLuint v2, GLuint pv );

typedef void (*triangle_func)( GLcontext *ctx,
                               GLuint v1, GLuint v2, GLuint v3, GLuint pv );

typedef void (*quad_func)( GLcontext *ctx, GLuint v1, GLuint v2,
                           GLuint v3, GLuint v4, GLuint pv );

typedef void (*rect_func)( GLcontext *ctx, GLint x, GLint y,
                           GLint width, GLint height );



/*
 * The auxiliary interpolation function for clipping - now always in
 * clip space.
 */
typedef void (*clip_interp_func)( struct vertex_buffer *VB, GLuint dst,
                                  GLfloat t, GLuint in, GLuint out );


/*
 * Renders vertices in vertex buffer
 */
typedef void (*render_vb_func)( GLcontext *ctx );



typedef void (*render_func)( struct vertex_buffer *VB,
			     GLuint start,
			     GLuint count,
			     GLuint parity );


/*
 * Blending function
 */
#ifdef USE_MMX_ASM
typedef void (_ASMAPIP blend_func)( GLcontext *ctx, GLuint n,
                                    const GLubyte mask[],
                                    GLubyte src[][4], CONST GLubyte dst[][4] );
#else
typedef void (*blend_func)( GLcontext *ctx, GLuint n, const GLubyte mask[],
                            GLubyte src[][4], CONST GLubyte dst[][4] );
#endif



/*
 * For texture sampling:
 */
struct gl_texture_object;

typedef void (*TextureSampleFunc)( const struct gl_texture_object *tObj,
                                   GLuint n,
                                   const GLfloat s[], const GLfloat t[],
                                   const GLfloat u[], const GLfloat lambda[],
                                   GLubyte rgba[][4] );

/* Texture format record */
/* GH: This is an interim structure until 3.5 */
struct gl_texture_format {
   GLint IntFormat;		/* One of the MESA_FORMAT_* values */

   GLubyte RedBits;		/* Bits per texel component */
   GLubyte GreenBits;
   GLubyte BlueBits;
   GLubyte AlphaBits;
   GLubyte LuminanceBits;
   GLubyte IntensityBits;
   GLubyte IndexBits;

   GLint TexelBytes;
};

/* Texture image record */
struct gl_texture_image {
   GLenum Format;		/* GL_ALPHA, GL_LUMINANCE, GL_LUMINANCE_ALPHA,
				 * GL_INTENSITY, GL_RGB, GL_RGBA, or
				 * GL_COLOR_INDEX only
				 */
   GLenum IntFormat;		/* Internal format as given by the user */
   GLuint Border;		/* 0 or 1 */
   GLuint Width;		/* = 2^WidthLog2 + 2*Border */
   GLuint Height;		/* = 2^HeightLog2 + 2*Border */
   GLuint Depth;		/* = 2^DepthLog2 + 2*Border */
   GLuint Width2;		/* = Width - 2*Border */
   GLuint Height2;		/* = Height - 2*Border */
   GLuint Depth2;		/* = Depth - 2*Border */
   GLuint WidthLog2;		/* = log2(Width2) */
   GLuint HeightLog2;		/* = log2(Height2) */
   GLuint DepthLog2;		/* = log2(Depth2) */
   GLuint MaxLog2;		/* = MAX(WidthLog2, HeightLog2) */
   GLubyte *Data;		/* Image data as unsigned bytes */

   const struct gl_texture_format *TexFormat;

   GLboolean IsCompressed;	/* GL_ARB_texture_compression */
   GLuint CompressedSize;	/* GL_ARB_texture_compression */

   /* For device driver: */
   void *DriverData;		/* Arbitrary device driver data */
};


/* Data structure for color tables */
struct gl_color_table {
   GLvoid *Table;
   GLenum TableType;      /* GL_UNSIGNED_BYTE or GL_FLOAT */
   GLuint Size;           /* number of entries (rows) in table */
   GLenum Format;
   GLenum IntFormat;
   GLint RedSize;
   GLint GreenSize;
   GLint BlueSize;
   GLint AlphaSize;
   GLint LuminanceSize;
   GLint IntensitySize;
};



/* This has to included here. */
#include "dd.h"


/*
 * Bit flags used for updating material values.
 */
#define FRONT_AMBIENT_BIT     0x1
#define BACK_AMBIENT_BIT      0x2
#define FRONT_DIFFUSE_BIT     0x4
#define BACK_DIFFUSE_BIT      0x8
#define FRONT_SPECULAR_BIT   0x10
#define BACK_SPECULAR_BIT    0x20
#define FRONT_EMISSION_BIT   0x40
#define BACK_EMISSION_BIT    0x80
#define FRONT_SHININESS_BIT 0x100
#define BACK_SHININESS_BIT  0x200
#define FRONT_INDEXES_BIT   0x400
#define BACK_INDEXES_BIT    0x800

#define FRONT_MATERIAL_BITS	(FRONT_EMISSION_BIT | FRONT_AMBIENT_BIT | \
				 FRONT_DIFFUSE_BIT | FRONT_SPECULAR_BIT | \
				 FRONT_SHININESS_BIT | FRONT_INDEXES_BIT)

#define BACK_MATERIAL_BITS	(BACK_EMISSION_BIT | BACK_AMBIENT_BIT | \
				 BACK_DIFFUSE_BIT | BACK_SPECULAR_BIT | \
				 BACK_SHININESS_BIT | BACK_INDEXES_BIT)

#define ALL_MATERIAL_BITS	(FRONT_MATERIAL_BITS | BACK_MATERIAL_BITS)



/*
 * Specular exponent and material shininess lookup table sizes:
 */
#define EXP_TABLE_SIZE 512
#define SHINE_TABLE_SIZE 256

struct gl_light {
   struct gl_light *next;	        /* double linked list with sentinel */
   struct gl_light *prev;

   GLfloat Ambient[4];		/* ambient color */
   GLfloat Diffuse[4];		/* diffuse color */
   GLfloat Specular[4];		/* specular color */
   GLfloat EyePosition[4];	/* position in eye coordinates */
   GLfloat EyeDirection[4];	/* spotlight dir in eye coordinates */
   GLfloat SpotExponent;
   GLfloat SpotCutoff;		/* in degress */
   GLfloat CosCutoff;		/* = MAX(0, cos(SpotCutoff)) */
   GLfloat ConstantAttenuation;
   GLfloat LinearAttenuation;
   GLfloat QuadraticAttenuation;
   GLboolean Enabled;		/* On/off flag */

   /* Derived fields */
   GLuint Flags;		/* State */

   GLfloat Position[4];		/* position in eye/obj coordinates */
   GLfloat VP_inf_norm[3];	/* Norm direction to infinite light */
   GLfloat h_inf_norm[3];	/* Norm( VP_inf_norm + <0,0,1> ) */
   GLfloat NormDirection[4];	/* normalized spotlight direction */
   GLfloat VP_inf_spot_attenuation;

   GLfloat SpotExpTable[EXP_TABLE_SIZE][2];  /* to replace a pow() call */
   GLfloat MatAmbient[2][3];	/* material ambient * light ambient */
   GLfloat MatDiffuse[2][3];	/* material diffuse * light diffuse */
   GLfloat MatSpecular[2][3];	/* material spec * light specular */
   GLfloat dli;			/* CI diffuse light intensity */
   GLfloat sli;			/* CI specular light intensity */
   GLboolean IsMatSpecular[2];
};


struct gl_lightmodel {
   GLfloat Ambient[4];		/* ambient color */
   GLboolean LocalViewer;	/* Local (or infinite) view point? */
   GLboolean TwoSide;		/* Two (or one) sided lighting? */
   GLenum ColorControl;		/* either GL_SINGLE_COLOR */
				/* or GL_SEPARATE_SPECULAR_COLOR */
};


/*
 * Attribute structures:
 *    We define a struct for each attribute group to make pushing and
 *    popping attributes easy.  Also it's a good organization.
 */


struct gl_accum_attrib {
   GLfloat ClearColor[4];	/* Accumulation buffer clear color */
};


/*
 * Used in DrawDestMask below
 */
#define FRONT_LEFT_BIT  1
#define FRONT_RIGHT_BIT 2
#define BACK_LEFT_BIT   4
#define BACK_RIGHT_BIT  8


struct gl_colorbuffer_attrib {
   GLuint ClearIndex;			/* Index to use for glClear */
   GLfloat ClearColor[4];		/* Color to use for glClear */

   GLuint IndexMask;			/* Color index write mask */
   GLubyte ColorMask[4];		/* Each flag is 0xff or 0x0 */
   GLboolean SWmasking;			/* Do color/CI masking in software? */

   GLenum DrawBuffer;			/* Which buffer to draw into */
   GLenum DriverDrawBuffer;		/* Current device driver dest buffer */
   GLboolean MultiDrawBuffer;		/* Drawing to mutliple buffers? */
   GLubyte DrawDestMask;		/* bitwise-OR of bitflags above */

   /* alpha testing */
   GLboolean AlphaEnabled;		/* Alpha test enabled flag */
   GLenum AlphaFunc;			/* Alpha test function */
   GLubyte AlphaRef;			/* Alpha ref value in range [0,255] */

   /* blending */
   GLboolean BlendEnabled;		/* Blending enabled flag */
   GLenum BlendSrcRGB;			/* Blending source operator */
   GLenum BlendDstRGB;			/* Blending destination operator */
   GLenum BlendSrcA;			/* GL_INGR_blend_func_separate */
   GLenum BlendDstA;			/* GL_INGR_blend_func_separate */
   GLenum BlendEquation;
   GLfloat BlendColor[4];
   blend_func BlendFunc;		/* Points to C blending function */

   /* logic op */
   GLenum LogicOp;			/* Logic operator */
   GLboolean IndexLogicOpEnabled;	/* Color index logic op enabled flag */
   GLboolean ColorLogicOpEnabled;	/* RGBA logic op enabled flag */
   GLboolean SWLogicOpEnabled;		/* Do logic ops in software? */

   GLboolean DitherFlag;		/* Dither enable flag */
};


struct gl_current_attrib {
   /* KW: These values valid only when the VB is flushed.
    */
   GLuint Flag;		                	/* Contains size information */
   GLfloat Normal[3];
   GLubyte ByteColor[4];			/* Current RGBA color */
   GLuint Index;				/* Current color index */
   GLboolean EdgeFlag;				/* Current edge flag */
   GLfloat Texcoord[MAX_TEXTURE_UNITS][4];	/* Current texture coords */
   GLenum Primitive;		                /* Prim or GL_POLYGON+1 */

   /* KW: No change to these values.
    */
   GLfloat RasterPos[4];			/* Current raster position */
   GLfloat RasterDistance;			/* Current raster distance */
   GLfloat RasterColor[4];			/* Current raster color */
   GLuint  RasterIndex;				/* Current raster index */
   GLfloat *RasterTexCoord;			/* Current raster texcoord*/
   GLfloat RasterMultiTexCoord[MAX_TEXTURE_UNITS][4];
   GLboolean RasterPosValid;			/* Raster po valid flag */
};


struct gl_depthbuffer_attrib {
   GLenum Func;			/* Function for depth buffer compare */
   GLfloat Clear;		/* Value to clear depth buffer to */
   GLboolean Test;		/* Depth buffering enabled flag */
   GLboolean Mask;		/* Depth buffer writable? */
   GLboolean OcclusionTest;	/* GL_HP_occlusion_test */
};


struct gl_enable_attrib {
   GLboolean AlphaTest;
   GLboolean AutoNormal;
   GLboolean Blend;
   GLboolean ClipPlane[MAX_CLIP_PLANES];
   GLboolean ColorMaterial;
   GLboolean Convolution1D;
   GLboolean Convolution2D;
   GLboolean Separable2D;
   GLboolean CullFace;
   GLboolean DepthTest;
   GLboolean Dither;
   GLboolean Fog;
   GLboolean Histogram;
   GLboolean Light[MAX_LIGHTS];
   GLboolean Lighting;
   GLboolean LineSmooth;
   GLboolean LineStipple;
   GLboolean IndexLogicOp;
   GLboolean ColorLogicOp;
   GLboolean Map1Color4;
   GLboolean Map1Index;
   GLboolean Map1Normal;
   GLboolean Map1TextureCoord1;
   GLboolean Map1TextureCoord2;
   GLboolean Map1TextureCoord3;
   GLboolean Map1TextureCoord4;
   GLboolean Map1Vertex3;
   GLboolean Map1Vertex4;
   GLboolean Map2Color4;
   GLboolean Map2Index;
   GLboolean Map2Normal;
   GLboolean Map2TextureCoord1;
   GLboolean Map2TextureCoord2;
   GLboolean Map2TextureCoord3;
   GLboolean Map2TextureCoord4;
   GLboolean Map2Vertex3;
   GLboolean Map2Vertex4;
   GLboolean MinMax;
   GLboolean Normalize;
   GLboolean PixelTexture;
   GLboolean PointSmooth;
   GLboolean PolygonOffsetPoint;
   GLboolean PolygonOffsetLine;
   GLboolean PolygonOffsetFill;
   GLboolean PolygonSmooth;
   GLboolean PolygonStipple;
   GLboolean RescaleNormals;
   GLboolean Scissor;
   GLboolean Stencil;
   GLuint Texture[MAX_TEXTURE_UNITS];
   GLuint TexGen[MAX_TEXTURE_UNITS];
};


struct gl_eval_attrib {
   /* Enable bits */
   GLboolean Map1Color4;
   GLboolean Map1Index;
   GLboolean Map1Normal;
   GLboolean Map1TextureCoord1;
   GLboolean Map1TextureCoord2;
   GLboolean Map1TextureCoord3;
   GLboolean Map1TextureCoord4;
   GLboolean Map1Vertex3;
   GLboolean Map1Vertex4;
   GLboolean Map2Color4;
   GLboolean Map2Index;
   GLboolean Map2Normal;
   GLboolean Map2TextureCoord1;
   GLboolean Map2TextureCoord2;
   GLboolean Map2TextureCoord3;
   GLboolean Map2TextureCoord4;
   GLboolean Map2Vertex3;
   GLboolean Map2Vertex4;
   GLboolean AutoNormal;
   /* Map Grid endpoints and divisions and calculated du values */
   GLint MapGrid1un;
   GLfloat MapGrid1u1, MapGrid1u2, MapGrid1du;
   GLint MapGrid2un, MapGrid2vn;
   GLfloat MapGrid2u1, MapGrid2u2, MapGrid2du;
   GLfloat MapGrid2v1, MapGrid2v2, MapGrid2dv;
};


struct gl_fog_attrib {
   GLboolean Enabled;		/* Fog enabled flag */
   GLfloat Color[4];		/* Fog color */
   GLfloat Density;		/* Density >= 0.0 */
   GLfloat Start;		/* Start distance in eye coords */
   GLfloat End;			/* End distance in eye coords */
   GLfloat Index;		/* Fog index */
   GLenum Mode;			/* Fog mode */
};


struct gl_hint_attrib {
   /* always one of GL_FASTEST, GL_NICEST, or GL_DONT_CARE */
   GLenum PerspectiveCorrection;
   GLenum PointSmooth;
   GLenum LineSmooth;
   GLenum PolygonSmooth;
   GLenum Fog;

   /* GL_PGI_misc_hints */
   GLenum AllowDrawWin;
   GLenum AllowDrawFrg;
   GLenum AllowDrawMem;
   GLenum StrictLighting;

   /* GL_EXT_clip_volume_hint */
   GLenum ClipVolumeClipping;

   /* GL_ARB_texture_compression */
   GLenum TextureCompression;
};


struct gl_histogram_attrib {
   GLuint Width;
   GLint Format;
   GLboolean Sink;
   GLuint RedSize;
   GLuint GreenSize;
   GLuint BlueSize;
   GLuint AlphaSize;
   GLuint LuminanceSize;
   GLuint Count[HISTOGRAM_TABLE_SIZE][4];
};


struct gl_minmax_attrib {
   GLenum Format;
   GLboolean Sink;
   GLfloat Min[4], Max[4];   /* RGBA */
};


struct gl_convolution_attrib {
   /* XXX not done yet */
   GLenum Format;
   GLenum InternalFormat;
   GLuint Width;
   GLuint Height;
   GLfloat Filter[MAX_CONVOLUTION_WIDTH * MAX_CONVOLUTION_HEIGHT * 4];
};


struct gl_light_attrib {
   struct gl_light Light[MAX_LIGHTS];	/* Array of lights */
   struct gl_lightmodel Model;		/* Lighting model */
   struct gl_material Material[2];	/* Material 0=front, 1=back */
   GLboolean Enabled;			/* Lighting enabled flag */
   GLenum ShadeModel;			/* GL_FLAT or GL_SMOOTH */
   GLenum ColorMaterialFace;		/* GL_FRONT, BACK or FRONT_AND_BACK */
   GLenum ColorMaterialMode;		/* GL_AMBIENT, GL_DIFFUSE, etc */
   GLuint ColorMaterialBitmask;		/* bitmask formed from Face and Mode */
   GLboolean ColorMaterialEnabled;

   struct gl_light EnabledList;         /* List sentinel */

   /* Derived for optimizations: */
   GLboolean NeedVertices;		/* Use fast shader? */
   GLuint  Flags;		        /* State, see below */
   GLfloat BaseColor[2][3];
   GLubyte BaseAlpha[2];
};


#define LIGHT_POSITIONAL   0x4
#define LIGHT_SPECULAR     0x8
#define LIGHT_SPOT         0x10
#define LIGHT_LOCAL_VIEWER 0x20
#define LIGHT_TWO_SIDE     0x40

#define LIGHT_NEED_VERTICES (LIGHT_POSITIONAL|LIGHT_LOCAL_VIEWER)

struct gl_line_attrib {
   GLboolean SmoothFlag;	/* GL_LINE_SMOOTH enabled? */
   GLboolean StippleFlag;	/* GL_LINE_STIPPLE enabled? */
   GLushort StipplePattern;	/* Stipple pattern */
   GLint StippleFactor;		/* Stipple repeat factor */
   GLfloat Width;		/* Line width */
};


struct gl_list_attrib {
   GLuint ListBase;
};


struct gl_pixel_attrib {
   GLenum ReadBuffer;		/* src buffer for glRead/CopyPixels */
   GLenum DriverReadBuffer;	/* Driver's current source buffer */
   GLfloat RedBias, RedScale;
   GLfloat GreenBias, GreenScale;
   GLfloat BlueBias, BlueScale;
   GLfloat AlphaBias, AlphaScale;
   GLboolean ScaleOrBiasRGBA;
   GLfloat DepthBias, DepthScale;
   GLint IndexShift, IndexOffset;
   GLboolean MapColorFlag;
   GLboolean MapStencilFlag;
   GLfloat ZoomX, ZoomY;
   GLint MapStoSsize;		/* Size of each pixel map */
   GLint MapItoIsize;
   GLint MapItoRsize;
   GLint MapItoGsize;
   GLint MapItoBsize;
   GLint MapItoAsize;
   GLint MapRtoRsize;
   GLint MapGtoGsize;
   GLint MapBtoBsize;
   GLint MapAtoAsize;
   GLint MapStoS[MAX_PIXEL_MAP_TABLE];	/* Pixel map tables */
   GLint MapItoI[MAX_PIXEL_MAP_TABLE];
   GLfloat MapItoR[MAX_PIXEL_MAP_TABLE];
   GLfloat MapItoG[MAX_PIXEL_MAP_TABLE];
   GLfloat MapItoB[MAX_PIXEL_MAP_TABLE];
   GLfloat MapItoA[MAX_PIXEL_MAP_TABLE];
   GLubyte MapItoR8[MAX_PIXEL_MAP_TABLE];  /* converted to 8-bit color */
   GLubyte MapItoG8[MAX_PIXEL_MAP_TABLE];
   GLubyte MapItoB8[MAX_PIXEL_MAP_TABLE];
   GLubyte MapItoA8[MAX_PIXEL_MAP_TABLE];
   GLfloat MapRtoR[MAX_PIXEL_MAP_TABLE];
   GLfloat MapGtoG[MAX_PIXEL_MAP_TABLE];
   GLfloat MapBtoB[MAX_PIXEL_MAP_TABLE];
   GLfloat MapAtoA[MAX_PIXEL_MAP_TABLE];
   /* GL_EXT_histogram */
   GLboolean HistogramEnabled;
   GLboolean MinMaxEnabled;
   /* GL_SGIS_pixel_texture */
   GLboolean PixelTextureEnabled;
   GLenum FragmentRgbSource;
   GLenum FragmentAlphaSource;
   /* GL_SGI_color_matrix */
   GLfloat PostColorMatrixScale[4];  /* RGBA */
   GLfloat PostColorMatrixBias[4];   /* RGBA */
   GLboolean ScaleOrBiasRGBApcm;
   /* GL_SGI_color_table */
   GLfloat ColorTableScale[4];
   GLfloat ColorTableBias[4];
   GLboolean ColorTableEnabled;
   GLfloat PCCTscale[4];
   GLfloat PCCTbias[4];
   GLboolean PostConvolutionColorTableEnabled;
   GLfloat PCMCTscale[4];
   GLfloat PCMCTbias[4];
   GLboolean PostColorMatrixColorTableEnabled;
   /* Convolution */
   GLboolean Convolution1DEnabled;
   GLboolean Convolution2DEnabled;
   GLboolean Separable2DEnabled;
   GLfloat ConvolutionBorderColor[3][4];
   GLenum ConvolutionBorderMode[3];
   GLfloat ConvolutionFilterScale[3][4];
   GLfloat ConvolutionFilterBias[3][4];
   GLfloat PostConvolutionScale[4];  /* RGBA */
   GLfloat PostConvolutionBias[4];   /* RGBA */
};


struct gl_point_attrib {
   GLboolean SmoothFlag;/* True if GL_POINT_SMOOTH is enabled */
   GLfloat UserSize;	/* User-specified point size */
   GLfloat Size;	/* Point size actually used */
   GLfloat Params[3];	/* Point Parameters EXT distance atenuation
			   factors by default = {1,0,0} */
   GLfloat MinSize;	/* Default 0.0 always >=0 */
   GLfloat MaxSize;	/* Default MAX_POINT_SIZE */
   GLfloat Threshold;	/* Default 1.0 */
   GLboolean Attenuated;
};


struct gl_polygon_attrib {
   GLenum FrontFace;		/* Either GL_CW or GL_CCW */
   GLenum FrontMode;		/* Either GL_POINT, GL_LINE or GL_FILL */
   GLenum BackMode;		/* Either GL_POINT, GL_LINE or GL_FILL */
   GLboolean FrontBit;		/*  */
   GLboolean Unfilled;		/* True if back or front mode is not GL_FILL */
   GLboolean CullFlag;		/* Culling on/off flag */
   GLubyte CullBits;		/* Used for cull testing */
   GLboolean SmoothFlag;	/* True if GL_POLYGON_SMOOTH is enabled */
   GLboolean StippleFlag;	/* True if GL_POLYGON_STIPPLE is enabled */
   GLenum CullFaceMode;		/* Culling mode GL_FRONT or GL_BACK */
   GLfloat OffsetFactor;	/* Polygon offset factor */
   GLfloat OffsetUnits;		/* Polygon offset units */
   GLboolean OffsetPoint;	/* Offset in GL_POINT mode? */
   GLboolean OffsetLine;	/* Offset in GL_LINE mode? */
   GLboolean OffsetFill;	/* Offset in GL_FILL mode? */
};


struct gl_scissor_attrib {
   GLboolean Enabled;		/* Scissor test enabled? */
   GLint X, Y;			/* Lower left corner of box */
   GLsizei Width, Height;		/* Size of box */
};


struct gl_stencil_attrib {
   GLboolean Enabled;		/* Enabled flag */
   GLenum Function;		/* Stencil function */
   GLenum FailFunc;		/* Fail function */
   GLenum ZPassFunc;		/* Depth buffer pass function */
   GLenum ZFailFunc;		/* Depth buffer fail function */
   GLstencil Ref;			/* Reference value */
   GLstencil ValueMask;		/* Value mask */
   GLstencil Clear;		/* Clear value */
   GLstencil WriteMask;		/* Write mask */
};


/* TexGenEnabled flags */
#define S_BIT 1
#define T_BIT 2
#define R_BIT 4
#define Q_BIT 8

/* Texture Enabled flags */
#define TEXTURE0_1D   0x1     /* Texture unit 0 (default) */
#define TEXTURE0_2D   0x2
#define TEXTURE0_3D   0x4
#define TEXTURE0_CUBE 0x8
#define TEXTURE0_ANY  (TEXTURE0_1D | TEXTURE0_2D | TEXTURE0_3D | TEXTURE0_CUBE)
#define TEXTURE1_1D   (TEXTURE0_1D << 4)    /* Texture unit 1 */
#define TEXTURE1_2D   (TEXTURE0_2D << 4)
#define TEXTURE1_3D   (TEXTURE0_3D << 4)
#define TEXTURE1_CUBE (TEXTURE0_CUBE << 4)
#define TEXTURE1_ANY  (TEXTURE1_1D | TEXTURE1_2D | TEXTURE1_3D | TEXTURE1_CUBE)

/* Bitmap versions of the GL_ constants.
 */
#define TEXGEN_SPHERE_MAP        0x1
#define TEXGEN_OBJ_LINEAR        0x2
#define TEXGEN_EYE_LINEAR        0x4
#define TEXGEN_REFLECTION_MAP_NV 0x8
#define TEXGEN_NORMAL_MAP_NV     0x10

#define TEXGEN_NEED_M            (TEXGEN_SPHERE_MAP)
#define TEXGEN_NEED_F            (TEXGEN_SPHERE_MAP        | \
				  TEXGEN_REFLECTION_MAP_NV)
#define TEXGEN_NEED_NORMALS      (TEXGEN_SPHERE_MAP        | \
				  TEXGEN_REFLECTION_MAP_NV | \
				  TEXGEN_NORMAL_MAP_NV)
#define TEXGEN_NEED_VERTICES     (TEXGEN_OBJ_LINEAR | 		\
				  TEXGEN_EYE_LINEAR |		\
				  TEXGEN_REFLECTION_MAP_NV |	\
				  TEXGEN_SPHERE_MAP )
#define TEXGEN_NEED_EYE_COORD    (TEXGEN_SPHERE_MAP        | \
				  TEXGEN_REFLECTION_MAP_NV | \
				  TEXGEN_NORMAL_MAP_NV     | \
				  TEXGEN_EYE_LINEAR)


/* A selection of state flags to make choosing optimized pipelines easier.
 */
#define ENABLE_TEX0         0xf	/* TEXTURE0_ANY */
#define ENABLE_TEX1         0xf0   /* TEXTURE1_ANY */
#define ENABLE_LIGHT        0x100
#define ENABLE_FOG          0x200
#define ENABLE_USERCLIP     0x400
#define ENABLE_TEXGEN0      0x800
#define ENABLE_TEXGEN1      0x1000
#define ENABLE_TEXMAT0      0x2000	/* Ie. not the identity matrix */
#define ENABLE_TEXMAT1      0x4000
#define ENABLE_VIEWCLIP     0x8000
#define ENABLE_NORMALIZE    0x10000
#define ENABLE_RESCALE      0x20000
#define ENABLE_POINT_ATTEN  0x40000


#define ENABLE_TEX_ANY  (ENABLE_TEX0 | ENABLE_TEX1)


typedef void (*texgen_func)( struct vertex_buffer *VB,
			     GLuint textureSet);


/* Texture object record */
struct gl_texture_object {
   _glthread_Mutex Mutex;	/* for thread safety */
   GLint RefCount;		/* reference count */
   GLuint Name;			/* an unsigned integer */
   GLuint Dimensions;		/* 1 or 2 or 3 */
   GLfloat Priority;		/* in [0,1] */
   GLubyte BorderColor[4];	/* as integers in [0,255] */
   GLenum WrapS;		/* GL_CLAMP, REPEAT or CLAMP_TO_EDGE */
   GLenum WrapT;		/* GL_CLAMP, REPEAT or CLAMP_TO_EDGE */
   GLenum WrapR;		/* GL_CLAMP, REPEAT or CLAMP_TO_EDGE */
   GLenum MinFilter;		/* minification filter */
   GLenum MagFilter;		/* magnification filter */
   GLfloat MinLod;		/* OpenGL 1.2 */
   GLfloat MaxLod;		/* OpenGL 1.2 */
   GLint BaseLevel;		/* OpenGL 1.2 */
   GLint MaxLevel;		/* OpenGL 1.2 */
   GLint P;			/* Highest mipmap level */
   GLfloat M;			/* = MIN(MaxLevel, P) - BaseLevel */
   GLfloat MinMagThresh;	/* min/mag threshold */
   struct gl_texture_image *Image[MAX_TEXTURE_LEVELS];

   /* Texture cube faces */
   /* Image[] is alias for *PosX[MAX_TEXTURE_LEVELS]; */
   struct gl_texture_image *NegX[MAX_TEXTURE_LEVELS];
   struct gl_texture_image *PosY[MAX_TEXTURE_LEVELS];
   struct gl_texture_image *NegY[MAX_TEXTURE_LEVELS];
   struct gl_texture_image *PosZ[MAX_TEXTURE_LEVELS];
   struct gl_texture_image *NegZ[MAX_TEXTURE_LEVELS];

   /* GL_EXT_paletted_texture */
   struct gl_color_table Palette;

   /* For device driver: */
   GLboolean Dirty;	/* Is this texture object in dirty list? */
   void *DriverData;	/* Arbitrary device driver data */

   GLboolean Complete;			/* Complete set of images? */
   TextureSampleFunc SampleFunc;	/* The texel sampling function */
   struct gl_texture_object *Next;	/* Next in linked list */
   struct gl_texture_object *NextDirty;	/* Next in dirty linked list */
};



/*
 * Texture units are new with the multitexture extension.
 */
struct gl_texture_unit {
   GLuint Enabled;              /* bitmask of TEXTURE0_1D, _2D, _3D, _CUBE */
   GLuint ReallyEnabled;        /* 0 or one of TEXTURE0_1D, _2D, _3D, _CUBE */

   GLenum EnvMode;	   /* GL_MODULATE, GL_DECAL, GL_BLEND, GL_COMBINE_EXT */
   GLenum LastEnvMode;

   GLenum CombineSourceRGB[3];  /* arg[i] for rgb EXT_texture_combine   */
   GLenum CombineSourceA[3];    /* dito for alpha combiner              */
   GLenum CombineModeRGB;       /* GL_REPLACE, GL_DECAL, GL_ADD ...     */
   GLenum CombineModeA;
   GLenum CombineOperandRGB[3]; /* SRC_COLOR or ONE_MINUS_SRC_COLOR     */
   GLenum CombineOperandA[3];   /* SRC_ALPHA or ONE_MINUS_SRC_ALPHA     */
   GLuint CombineScaleShiftRGB; /* 0, 1 or 2   */
   GLuint CombineScaleShiftA;

   GLfloat EnvColor[4];
   GLuint TexGenEnabled;	/* Bitwise-OR of [STRQ]_BIT values */
   GLenum GenModeS;		/* Tex coord generation mode, either */
   GLenum GenModeT;		/*	GL_OBJECT_LINEAR, or */
   GLenum GenModeR;		/*	GL_EYE_LINEAR, or    */
   GLenum GenModeQ;		/*      GL_SPHERE_MAP        */
   GLuint GenBitS;
   GLuint GenBitT;
   GLuint GenBitR;
   GLuint GenBitQ;
   GLuint GenFlags;		/* bitwise or of GenBit[STRQ] */
   GLfloat ObjectPlaneS[4];
   GLfloat ObjectPlaneT[4];
   GLfloat ObjectPlaneR[4];
   GLfloat ObjectPlaneQ[4];
   GLfloat EyePlaneS[4];
   GLfloat EyePlaneT[4];
   GLfloat EyePlaneR[4];
   GLfloat EyePlaneQ[4];
   texgen_func *func;		/* points to array of func pointers */
   GLubyte Holes;		/* elements not generated by texgen */
   GLuint  TexgenSize;		/* size of element generated */
   GLboolean AnyTransform;	/* texgen or non-identity matrix */
   GLfloat LodBias;		/* for biasing mipmap levels */

   struct gl_texture_object *CurrentD[4];
   struct gl_texture_object *CurrentCubeMap; /* GL_ARB_texture_cube_map */

   struct gl_texture_object *Current;
   GLuint CurrentDimension;

   struct gl_texture_object Saved1D;  /* only used by glPush/PopAttrib */
   struct gl_texture_object Saved2D;
   struct gl_texture_object Saved3D;
   struct gl_texture_object SavedCubeMap;
};


struct gl_texture_attrib {
   /* multitexture */
   GLuint CurrentUnit;			/* Current texture unit */
   GLuint CurrentTransformUnit;		/* Current texture xform unit */

   GLuint ReallyEnabled;     /* Really enabled (w.r.t. completeness, etc) */

   GLuint LastEnabled;	/* Decide whether enabled has really changed */

   GLboolean NeedNormals;
   GLboolean NeedEyeCoords;

   struct gl_texture_unit Unit[MAX_TEXTURE_UNITS];

   struct gl_texture_object *Proxy1D;
   struct gl_texture_object *Proxy2D;
   struct gl_texture_object *Proxy3D;
   struct gl_texture_object *ProxyCubeMap;

   /* GL_EXT_shared_texture_palette */
   GLboolean SharedPalette;
   struct gl_color_table Palette;
};



/* KW: Renamed ClipEquation to avoid having 'ClipClipEquation'
 */
struct gl_transform_attrib {
   GLenum MatrixMode;				/* Matrix mode */
   GLfloat EyeUserPlane[MAX_CLIP_PLANES][4];
   GLfloat ClipUserPlane[MAX_CLIP_PLANES][4];	/* derived */
   GLboolean ClipEnabled[MAX_CLIP_PLANES];
   GLubyte   AnyClip;				/* How many ClipEnabled? */
   GLboolean Normalize;				/* Normalize all normals? */
   GLboolean RescaleNormals;			/* GL_EXT_rescale_normal */
};


struct gl_viewport_attrib {
   GLint X, Y;			/* position */
   GLsizei Width, Height;	/* size */
   GLfloat Near, Far;		/* Depth buffer range */
   GLmatrix WindowMap;		/* Mapping transformation as a matrix. */
};


/* For the attribute stack: */
struct gl_attrib_node {
   GLbitfield kind;
   void *data;
   struct gl_attrib_node *next;
};



/*
 * Client pixel packing/unpacking attributes
 */
struct gl_pixelstore_attrib {
   GLint Alignment;
   GLint RowLength;
   GLint SkipPixels;
   GLint SkipRows;
   GLint ImageHeight;     /* for GL_EXT_texture3D */
   GLint SkipImages;      /* for GL_EXT_texture3D */
   GLboolean SwapBytes;
   GLboolean LsbFirst;
};


/*
 * Client vertex array attributes
 */
struct gl_client_array {
   GLint Size;
   GLenum Type;
   GLsizei Stride;		/* user-specified stride */
   GLsizei StrideB;		/* actual stride in bytes */
   void *Ptr;
   GLboolean Enabled;
};


/* KW:  New functions to populate a vector from data of a different type.
 *      Used in the array interface.  Flags & match only used in the
 *      specialized array-element case.
 */
typedef void (*trans_1ui_func)(GLuint *to,
			       const struct gl_client_array *from,
			       GLuint start,
			       GLuint n );

typedef void (*trans_1ub_func)(GLubyte *to,
			       const struct gl_client_array *from,
			       GLuint start,
			       GLuint n );

typedef void (*trans_4ub_func)(GLubyte (*to)[4],
			       const struct gl_client_array *from,
			       GLuint start,
			       GLuint n );

typedef void (*trans_4f_func)(GLfloat (*to)[4],
			      const struct gl_client_array *from,
			      GLuint start,
			      GLuint n );

typedef void (*trans_3f_func)(GLfloat (*to)[3],
			      const struct gl_client_array *from,
			      GLuint start,
			      GLuint n );



typedef void (*trans_elt_1ui_func)(GLuint *to,
				   const struct gl_client_array *from,
				   GLuint *flags,
				   GLuint *elts,
				   GLuint match,
				   GLuint start,
				   GLuint n );

typedef void (*trans_elt_1ub_func)(GLubyte *to,
				   const struct gl_client_array *from,
				   GLuint *flags,
				   GLuint *elts,
				   GLuint match,
				   GLuint start,
				   GLuint n );

typedef void (*trans_elt_4ub_func)(GLubyte (*to)[4],
				   const struct gl_client_array *from,
				   GLuint *flags,
				   GLuint *elts,
				   GLuint match,
				   GLuint start,
				   GLuint n );

typedef void (*trans_elt_4f_func)(GLfloat (*to)[4],
				  const struct gl_client_array *from,
				  GLuint *flags,
				  GLuint *elts,
				  GLuint match,
				  GLuint start,
				  GLuint n );

typedef void (*trans_elt_3f_func)(GLfloat (*to)[3],
				  const struct gl_client_array *from,
				  GLuint *flags,
				  GLuint *elts,
				  GLuint match,
				  GLuint start,
				  GLuint n );


/*#define VAO*/
#ifdef VAO
struct gl_array_object {
   GLuint Name;
   GLuint RefCount;

   struct gl_client_array Vertex;	     /* client data descriptors */
   struct gl_client_array Normal;
   struct gl_client_array Color;
   struct gl_client_array Index;
   struct gl_client_array TexCoord[MAX_TEXTURE_UNITS];
   struct gl_client_array EdgeFlag;

   trans_4f_func  VertexFunc;       /* conversion functions */
   trans_3f_func  NormalFunc;
   trans_4ub_func ColorFunc;
   trans_1ui_func IndexFunc;
   trans_4f_func  TexCoordFunc[MAX_TEXTURE_UNITS];
   trans_1ub_func EdgeFlagFunc;

   trans_elt_4f_func  VertexEltFunc; /* array elt conversion functions */
   trans_elt_3f_func  NormalEltFunc;
   trans_elt_4ub_func ColorEltFunc;
   trans_elt_1ui_func IndexEltFunc;
   trans_elt_4f_func  TexCoordEltFunc[MAX_TEXTURE_UNITS];
   trans_elt_1ub_func EdgeFlagEltFunc;

   GLint TexCoordInterleaveFactor;

   GLuint LockFirst;
   GLuint LockCount;

   GLuint Flag[VB_SIZE];	/* crock */
   GLuint Flags;
   GLuint Summary;		/* Like flags, but no size information */
};
#endif


struct gl_array_attrib {
#ifdef VAO
   struct gl_array_object *Current;
   GLint ActiveTexture;		/* Client Active Texture */
   GLuint NewArrayState;	/* Tracks which arrays have been changed. */
#else
   struct gl_client_array Vertex;	     /* client data descriptors */
   struct gl_client_array Normal;
   struct gl_client_array Color;
   struct gl_client_array Index;
   struct gl_client_array TexCoord[MAX_TEXTURE_UNITS];
   struct gl_client_array EdgeFlag;

   trans_4f_func  VertexFunc;       /* conversion functions */
   trans_3f_func  NormalFunc;
   trans_4ub_func ColorFunc;
   trans_1ui_func IndexFunc;
   trans_4f_func  TexCoordFunc[MAX_TEXTURE_UNITS];
   trans_1ub_func EdgeFlagFunc;

   trans_elt_4f_func  VertexEltFunc; /* array elt conversion functions */
   trans_elt_3f_func  NormalEltFunc;
   trans_elt_4ub_func ColorEltFunc;
   trans_elt_1ui_func IndexEltFunc;
   trans_elt_4f_func  TexCoordEltFunc[MAX_TEXTURE_UNITS];
   trans_elt_1ub_func EdgeFlagEltFunc;

   GLint TexCoordInterleaveFactor;
   GLint ActiveTexture;		/* Client Active Texture */

   GLuint LockFirst;
   GLuint LockCount;

   GLuint Flag[VB_SIZE];	/* crock */
   GLuint Flags;
   GLuint Summary;		/* Like flags, but no size information */
   GLuint NewArrayState;	/* Tracks which arrays have been changed. */
#endif
};


/* These are used to make the ctx->Current values look like
 * arrays (with zero StrideB).
 */
struct gl_fallback_arrays {
   struct gl_client_array Normal;
   struct gl_client_array Color;
   struct gl_client_array Index;
   struct gl_client_array TexCoord[MAX_TEXTURE_UNITS];
   struct gl_client_array EdgeFlag;
};

#define MAX_PIPELINE_STAGES     30

#define PIPE_IMMEDIATE          0x1
#define PIPE_PRECALC            0x2

#define PIPE_OP_VERT_XFORM      0x1
#define PIPE_OP_NORM_XFORM      0x2
#define PIPE_OP_LIGHT           0x4
#define PIPE_OP_FOG             0x8
#define PIPE_OP_TEX0            0x10
#define PIPE_OP_TEX1            0x20
#define PIPE_OP_RAST_SETUP_0    0x40
#define PIPE_OP_RAST_SETUP_1    0x80
#define PIPE_OP_RENDER          0x100
#define PIPE_OP_CVA_PREPARE     0x200


/* Nodes in a state machine to drive rendering of the various
 * point/line/tri primitives.
 */
struct gl_prim_state {
   GLuint v0, v1;
   GLboolean draw;
   GLboolean finish_loop;	/* not used... */
   struct gl_prim_state *next;
};


typedef void (*vb_func)( struct vertex_buffer *VB );
typedef void (*ctx_func)( GLcontext * );

struct gl_pipeline_stage {
   const char *name;
   GLuint ops;			/* PIPE_OP flags */
   GLuint type;			/* PIPE flags */
   GLuint special;		/* PIPE flags - force update_inputs() */
   GLuint state_change;	        /* state flags - trigger update_inputs() */
   GLuint cva_state_change;          /* state flags - recalc cva buffer */
   GLuint elt_forbidden_inputs;      /* VERT flags - force a pipeline recalc */
   GLuint pre_forbidden_inputs;	     /* VERT flags - force a pipeline recalc */
   GLuint active;		     /* PIPE flags */
   GLuint inputs;		     /* VERT flags */
   GLuint outputs;		     /* VERT flags */
   void (*check)( GLcontext *ctx, struct gl_pipeline_stage * );
   void (*run)( struct vertex_buffer *VB );
};


struct gl_pipeline {
   GLuint state_change;		/* state changes which require recalc */
   GLuint cva_state_change;	/* ... which require re-run */
   GLuint forbidden_inputs;	/* inputs which require recalc */
   GLuint ops;			/* what gets done in this pipe */
   GLuint changed_ops;
   GLuint inputs;
   GLuint outputs;
   GLuint new_inputs;
   GLuint new_outputs;
   GLuint fallback;
   GLuint type;
   GLuint pipeline_valid:1;
   GLuint data_valid:1;
   GLuint copy_transformed_data:1;
   GLuint replay_copied_vertices:1;
   GLuint new_state;		/* state changes since last recalc */
   struct gl_pipeline_stage *stages[MAX_PIPELINE_STAGES];
};


struct gl_cva {
   struct gl_pipeline pre;
   struct gl_pipeline elt;

   struct gl_client_array Elt;
   trans_1ui_func EltFunc;

   struct vertex_buffer *VB;
   struct vertex_arrays v;
   struct vertex_data store;

   GLuint elt_count;
   GLenum elt_mode;
   GLuint elt_size;

   GLuint forbidden_inputs;
   GLuint orflag;
   GLuint merge;

   GLuint lock_changed;
   GLuint last_orflag;
   GLuint last_array_flags;
   GLuint last_array_new_state;
};


struct gl_feedback {
   GLenum Type;
   GLuint Mask;
   GLfloat *Buffer;
   GLuint BufferSize;
   GLuint Count;
};



struct gl_selection {
   GLuint *Buffer;
   GLuint BufferSize;	/* size of SelectBuffer */
   GLuint BufferCount;	/* number of values in SelectBuffer */
   GLuint Hits;		/* number of records in SelectBuffer */
   GLuint NameStackDepth;
   GLuint NameStack[MAX_NAME_STACK_DEPTH];
   GLboolean HitFlag;
   GLfloat HitMinZ, HitMaxZ;
};



/*
 * 1-D Evaluator control points
 */
struct gl_1d_map {
   GLuint Order;	/* Number of control points */
   GLfloat u1, u2, du;	/* u1, u2, 1.0/(u2-u1) */
   GLfloat *Points;	/* Points to contiguous control points */
};


/*
 * 2-D Evaluator control points
 */
struct gl_2d_map {
   GLuint Uorder;		/* Number of control points in U dimension */
   GLuint Vorder;		/* Number of control points in V dimension */
   GLfloat u1, u2, du;
   GLfloat v1, v2, dv;
   GLfloat *Points;		/* Points to contiguous control points */
};


/*
 * All evalutator control points
 */
struct gl_evaluators {
   /* 1-D maps */
   struct gl_1d_map Map1Vertex3;
   struct gl_1d_map Map1Vertex4;
   struct gl_1d_map Map1Index;
   struct gl_1d_map Map1Color4;
   struct gl_1d_map Map1Normal;
   struct gl_1d_map Map1Texture1;
   struct gl_1d_map Map1Texture2;
   struct gl_1d_map Map1Texture3;
   struct gl_1d_map Map1Texture4;

   /* 2-D maps */
   struct gl_2d_map Map2Vertex3;
   struct gl_2d_map Map2Vertex4;
   struct gl_2d_map Map2Index;
   struct gl_2d_map Map2Color4;
   struct gl_2d_map Map2Normal;
   struct gl_2d_map Map2Texture1;
   struct gl_2d_map Map2Texture2;
   struct gl_2d_map Map2Texture3;
   struct gl_2d_map Map2Texture4;
};



/*
 * State which can be shared by multiple contexts:
 */
struct gl_shared_state {
   _glthread_Mutex Mutex;		   /* for thread safety */
   GLint RefCount;			   /* Reference count */
   struct _mesa_HashTable *DisplayList;	   /* Display lists hash table */
   struct _mesa_HashTable *TexObjects;	   /* Texture objects hash table */
#ifdef VAO
   struct _mesa_HashTable *ArrayObjects;   /* GL_EXT_vertex_array_set */
#endif
   struct gl_texture_object *TexObjectList;/* Linked list of texture objects */
   struct gl_texture_object *DirtyTexObjList; /* List of dirty tex objects */

   /* Default texture objects (shared by all multi-texture units) */
   struct gl_texture_object *DefaultD[4];
   struct gl_texture_object *DefaultCubeMap;

   void *DriverData;  /* Device driver shared state */
};



/*
 * Describes the color, depth, stencil and accum buffer parameters.
 * In C++ terms, think of this as a base class from which device drivers
 * will make derived classes.
 */
struct gl_visual {
   GLboolean RGBAflag;		/* Is frame buffer in RGBA mode, not CI? */
   GLboolean DBflag;		/* Is color buffer double buffered? */
   GLboolean StereoFlag;	/* stereo buffer? */

   GLint RedBits;		/* Bits per color component */
   GLint GreenBits;
   GLint BlueBits;
   GLint AlphaBits;

   GLint IndexBits;		/* Bits/pixel if in color index mode */

   GLint AccumRedBits;		/* Number of bits in red accum channel */
   GLint AccumGreenBits;	/* Number of bits in green accum channel */
   GLint AccumBlueBits;		/* Number of bits in blue accum channel */
   GLint AccumAlphaBits;	/* Number of bits in alpha accum channel */
   GLint DepthBits;		/* Number of bits in depth buffer, or 0 */
   GLint StencilBits;		/* Number of bits in stencil buffer, or 0 */
   GLint NumSamples;            /* Samples/pixel for multisampling */

   GLuint DepthMax;		/* Max depth buffer value */
   GLfloat DepthMaxF;		/* Float max depth buffer value */
};



/*
 * A "frame buffer" is a color buffer and its optional ancillary buffers:
 * depth, accum, stencil, and software-simulated alpha buffers.
 * In C++ terms, think of this as a base class from which device drivers
 * will make derived classes.
 */
struct gl_frame_buffer {
   GLvisual *Visual;		/* The corresponding visual */

   GLint Width, Height;		/* size of frame buffer in pixels */

   GLboolean UseSoftwareDepthBuffer;
   GLboolean UseSoftwareAccumBuffer;
   GLboolean UseSoftwareStencilBuffer;
   GLboolean UseSoftwareAlphaBuffers;

   /* Software depth (aka Z) buffer */
   GLvoid *DepthBuffer;		/* array [Width*Height] of GLushort or GLuint*/

   /* Software stencil buffer */
   GLstencil *Stencil;		/* array [Width*Height] of GLstencil values */

   /* Software accumulation buffer */
   GLaccum *Accum;		/* array [4*Width*Height] of GLaccum values */

   /* Software alpha planes */
   GLubyte *FrontLeftAlpha;	/* array [Width*Height] of GLubyte */
   GLubyte *BackLeftAlpha;	/* array [Width*Height] of GLubyte */
   GLubyte *FrontRightAlpha;	/* array [Width*Height] of GLubyte */
   GLubyte *BackRightAlpha;	/* array [Width*Height] of GLubyte */
   GLubyte *Alpha;		/* Points to current alpha buffer */

   /* Drawing bounds: intersection of window size and scissor box */
   GLint Xmin, Xmax, Ymin, Ymax;  /* [Xmin,Xmax] X [Ymin,Ymax] */
};


/*
 * Constants which may be overriden by device driver.
 */
struct gl_constants {
   GLint MaxTextureSize;
   GLint MaxCubeTextureSize;
   GLint MaxTextureLevels;
   GLuint MaxTextureUnits;
   GLuint MaxArrayLockSize;
   GLint SubPixelBits;
   GLfloat MinPointSize, MaxPointSize;		/* aliased */
   GLfloat MinPointSizeAA, MaxPointSizeAA;	/* antialiased */
   GLfloat PointSizeGranularity;
   GLfloat MinLineWidth, MaxLineWidth;		/* aliased */
   GLfloat MinLineWidthAA, MaxLineWidthAA;	/* antialiased */
   GLfloat LineWidthGranularity;
   GLuint NumAuxBuffers;
   GLuint MaxColorTableSize;
   GLuint MaxConvolutionWidth;
   GLuint MaxConvolutionHeight;
   GLuint NumCompressedTextureFormats;	/* GL_ARB_texture_compression */
   GLenum CompressedTextureFormats[MAX_COMPRESSED_TEXTURE_FORMATS];
};


/*
 * List of extensions.
 */
struct extension;
struct gl_extensions {
   char *ext_string;
   struct extension *ext_list;
   /* flags to quickly test if certain extensions are available */
   GLboolean HaveTextureEnvAdd;
   GLboolean HaveTextureEnvCombine;
   GLboolean HaveTextureEnvDot3;
   GLboolean HaveTextureLodBias;
   GLboolean HaveHpOcclusionTest;
   GLboolean HaveTextureCubeMap;
   GLboolean HaveTextureCompression;
   GLboolean HaveTextureCompressionS3TC;
   GLboolean HaveTextureCompressionFXT1;
   GLboolean HaveBlendSquare;
};


/*
 * Bitmasks to indicate which rasterization options are enabled (RasterMask)
 */
#define ALPHATEST_BIT		0x001	/* Alpha-test pixels */
#define BLEND_BIT		0x002	/* Blend pixels */
#define DEPTH_BIT		0x004	/* Depth-test pixels */
#define FOG_BIT			0x008	/* Per-pixel fog */
#define LOGIC_OP_BIT		0x010	/* Apply logic op in software */
#define SCISSOR_BIT		0x020	/* Scissor pixels */
#define STENCIL_BIT		0x040	/* Stencil pixels */
#define MASKING_BIT		0x080	/* Do glColorMask or glIndexMask */
#define ALPHABUF_BIT		0x100	/* Using software alpha buffer */
#define WINCLIP_BIT		0x200	/* Clip pixels/primitives to window */
#define MULTI_DRAW_BIT		0x400	/* Write to more than one color- */
                                        /* buffer or no buffers. */
#define OCCLUSION_BIT           0x800   /* GL_HP_occlusion_test enabled */
#define TEXTURE_BIT		0x1000	/* Texturing really enabled */


/*
 * Bits to indicate what state has to be updated (NewState)
 */
#define NEW_LIGHTING	        0x1
#define NEW_RASTER_OPS	        0x2
#define NEW_TEXTURING	        0x4
#define NEW_POLYGON	        0x8
#define NEW_DRVSTATE0	        0x10 /* Reserved for drivers */
#define NEW_DRVSTATE1	        0x20 /* Reserved for drivers */
#define NEW_DRVSTATE2	        0x40 /* Reserved for drivers */
#define NEW_DRVSTATE3	        0x80 /* Reserved for drivers */
#define NEW_MODELVIEW	        0x100
#define NEW_PROJECTION	        0x200
#define NEW_TEXTURE_MATRIX	0x400
#define NEW_USER_CLIP	        0x800
#define NEW_TEXTURE_ENV         0x1000
#define NEW_CLIENT_STATE        0x2000
#define NEW_FOG                 0x4000
#define NEW_NORMAL_TRANSFORM    0x8000
#define NEW_VIEWPORT            0x10000
#define NEW_TEXTURE_ENABLE      0x20000
#define NEW_COLOR_MATRIX        0x40000
#define NEW_ALL		        ~0


#define NEW_DRIVER_STATE (NEW_DRVSTATE0 | NEW_DRVSTATE1 |	\
                          NEW_DRVSTATE2 | NEW_DRVSTATE3)

/* What can the driver do, what requires us to call render_triangle or
 * a non-driver rasterize function?
 */
#define DD_FEEDBACK                 0x1
#define DD_SELECT                   0x2
#define DD_FLATSHADE                0x4
#define DD_MULTIDRAW                0x8
#define DD_SEPERATE_SPECULAR        0x10
#define DD_TRI_LIGHT_TWOSIDE        0x20
#define DD_TRI_UNFILLED             0x40
#define DD_TRI_SMOOTH               0x80
#define DD_TRI_STIPPLE              0x100
#define DD_TRI_OFFSET               0x200
#define DD_TRI_CULL                 0x400
#define DD_LINE_SMOOTH              0x800
#define DD_LINE_STIPPLE             0x1000
#define DD_LINE_WIDTH               0x2000
#define DD_POINT_SMOOTH             0x4000
#define DD_POINT_SIZE               0x8000
#define DD_POINT_ATTEN              0x10000
#define DD_LIGHTING_CULL            0x20000 /* never set by driver */
#define DD_POINT_SW_RASTERIZE       0x40000
#define DD_LINE_SW_RASTERIZE        0x80000
#define DD_TRI_SW_RASTERIZE         0x100000
#define DD_QUAD_SW_RASTERIZE        0x200000
#define DD_TRI_CULL_FRONT_BACK      0x400000 /* not supported by most drivers */
#define DD_Z_NEVER                  0x800000
#define DD_STENCIL                  0x1000000
#define DD_CLIP_FOG_COORD           0x2000000



#define DD_SW_SETUP           (DD_TRI_CULL|		\
                               DD_TRI_CULL_FRONT_BACK|	\
                               DD_TRI_OFFSET|		\
			       DD_TRI_LIGHT_TWOSIDE|	\
                               DD_TRI_UNFILLED)

#define DD_ANY_CULL           (DD_TRI_CULL_FRONT_BACK|	\
                               DD_TRI_CULL|		\
                               DD_LIGHTING_CULL)

#define DD_SW_RASTERIZE       (DD_POINT_SW_RASTERIZE|	\
                               DD_LINE_SW_RASTERIZE|	\
                               DD_TRI_SW_RASTERIZE|	\
                               DD_QUAD_SW_RASTERIZE)


/* Vertex buffer clipping flags
 */
#define CLIP_RIGHT_SHIFT 	0
#define CLIP_LEFT_SHIFT 	1
#define CLIP_TOP_SHIFT  	2
#define CLIP_BOTTOM_SHIFT       3
#define CLIP_NEAR_SHIFT  	4
#define CLIP_FAR_SHIFT  	5

#define CLIP_RIGHT_BIT   0x01
#define CLIP_LEFT_BIT    0x02
#define CLIP_TOP_BIT     0x04
#define CLIP_BOTTOM_BIT  0x08
#define CLIP_NEAR_BIT    0x10
#define CLIP_FAR_BIT     0x20
#define CLIP_USER_BIT    0x40
#define CLIP_CULLED_BIT  0x80	/* Vertex has been culled */
#define CLIP_ALL_BITS    0x3f


/*
 * Bits to indicate which faces a vertex participates in,
 * what facing the primitive provoked by that vertex has,
 * and some misc. flags.
 */
#define VERT_FACE_FRONT       0x1	/* is in a front-color primitive */
#define VERT_FACE_REAR        0x2	/* is in a rear-color primitive */
#define PRIM_FACE_FRONT       0x4	/* use front color */
#define PRIM_FACE_REAR        0x8	/* use rear color */
#define PRIM_CLIPPED          0x10	/* needs clipping */
#define PRIM_USER_CLIPPED     CLIP_USER_BIT	/* 0x40 */


#define PRIM_FLAG_SHIFT  2
#define PRIM_FACE_FLAGS  (PRIM_FACE_FRONT|PRIM_FACE_REAR)
#define VERT_FACE_FLAGS  (VERT_FACE_FRONT|VERT_FACE_REAR)

#define PRIM_ANY_CLIP    (PRIM_CLIPPED|PRIM_USER_CLIPPED)
#define PRIM_NOT_CULLED  (PRIM_ANY_CLIP|PRIM_FACE_FLAGS)

/* Flags for ctx->VB->CullMode.
 */
#define CULL_MASK_ACTIVE  0x1
#define COMPACTED_NORMALS 0x2
#define CLIP_MASK_ACTIVE  0x4

/* Flags for selecting a shading function.  The first two bits are
 * shared with the cull mode (ie. cull_mask_active and
 * compacted_normals.)
 */
#define SHADE_TWOSIDE           0x4

/* Flags for selecting a normal transformation function.
 */
#define NORM_RESCALE   0x1	/* apply the scale factor */
#define NORM_NORMALIZE 0x2	/* normalize */
#define NORM_TRANSFORM 0x4	/* apply the transformation matrix */
#define NORM_TRANSFORM_NO_ROT 0x8	/* apply the transformation matrix */


/*
 * Different kinds of 4x4 transformation matrices:
 */
#define MATRIX_GENERAL		0	/* general 4x4 matrix */
#define MATRIX_IDENTITY		1	/* identity matrix */
#define MATRIX_3D_NO_ROT	2	/* ortho projection and others... */
#define MATRIX_PERSPECTIVE	3	/* perspective projection matrix */
#define MATRIX_2D		4	/* 2-D transformation */
#define MATRIX_2D_NO_ROT	5	/* 2-D scale & translate only */
#define MATRIX_3D		6	/* 3-D transformation */

#define MAT_FLAG_IDENTITY        0
#define MAT_FLAG_GENERAL        0x1
#define MAT_FLAG_ROTATION       0x2
#define MAT_FLAG_TRANSLATION    0x4
#define MAT_FLAG_UNIFORM_SCALE  0x8
#define MAT_FLAG_GENERAL_SCALE  0x10
#define MAT_FLAG_GENERAL_3D     0x20
#define MAT_FLAG_PERSPECTIVE    0x40
#define MAT_DIRTY_TYPE          0x80
#define MAT_DIRTY_FLAGS         0x100
#define MAT_DIRTY_INVERSE       0x200
#define MAT_DIRTY_DEPENDENTS    0x400


/* Give symbolic names to some of the entries in the matrix to help
 * out with the rework of the viewport_map as a matrix transform.
 */
#define MAT_SX 0
#define MAT_SY 5
#define MAT_SZ 10
#define MAT_TX 12
#define MAT_TY 13
#define MAT_TZ 14


#define MAT_FLAGS_ANGLE_PRESERVING (MAT_FLAG_ROTATION | \
				    MAT_FLAG_TRANSLATION | \
				    MAT_FLAG_UNIFORM_SCALE)

#define MAT_FLAGS_LENGTH_PRESERVING (MAT_FLAG_ROTATION | \
				     MAT_FLAG_TRANSLATION)

#define MAT_FLAGS_3D (MAT_FLAG_ROTATION | \
		      MAT_FLAG_TRANSLATION | \
		      MAT_FLAG_UNIFORM_SCALE | \
		      MAT_FLAG_GENERAL_SCALE | \
		      MAT_FLAG_GENERAL_3D)

#define MAT_FLAGS_GEOMETRY (MAT_FLAG_GENERAL | \
			    MAT_FLAG_ROTATION | \
			    MAT_FLAG_TRANSLATION | \
			    MAT_FLAG_UNIFORM_SCALE | \
			    MAT_FLAG_GENERAL_SCALE | \
			    MAT_FLAG_GENERAL_3D | \
			    MAT_FLAG_PERSPECTIVE)

#define MAT_DIRTY_ALL_OVER (MAT_DIRTY_TYPE | \
			    MAT_DIRTY_DEPENDENTS | \
			    MAT_DIRTY_FLAGS | \
			    MAT_DIRTY_INVERSE)

#define TEST_MAT_FLAGS(mat, a)  ((MAT_FLAGS_GEOMETRY&(~(a))&((mat)->flags))==0)


/*
 * FogMode values:
 */
#define FOG_NONE	0	/* no fog */
#define FOG_VERTEX	1	/* apply per vertex */
#define FOG_FRAGMENT	2	/* apply per fragment */



typedef void (*gl_shade_func)( struct vertex_buffer *VB );



/*
 * Forward declaration of display list datatypes:
 */
union node;
typedef union node Node;




/* KW: Flags that describe the current vertex state, and the contents
 * of a vertex in a vertex-cassette.
 *
 * For really major expansion, consider a 'VERT_ADDITIONAL_FLAGS' flag,
 * which means there is data in another flags array (eg, extra_flags[]).
 */

#define VERT_OBJ_2           0x1	/* glVertex2 */
#define VERT_OBJ_3           0x2        /* glVertex3 */
#define VERT_OBJ_4           0x4        /* glVertex4 */
#define VERT_BEGIN           0x8	/* glBegin */
#define VERT_END             0x10	/* glEnd */
#define VERT_ELT             0x20	/* glArrayElement */
#define VERT_RGBA            0x40	/* glColor */
#define VERT_NORM            0x80	/* glNormal */
#define VERT_INDEX           0x100	/* glIndex */
#define VERT_EDGE            0x200	/* glEdgeFlag */
#define VERT_MATERIAL        0x400	/* glMaterial */
#define VERT_TEX0_1          0x800
#define VERT_TEX0_2          0x1000
#define VERT_TEX0_3          0x2000
#define VERT_TEX0_4          0x4000
#define VERT_TEX1_1          0x8000
#define VERT_TEX1_2          0x10000
#define VERT_TEX1_3          0x20000
#define VERT_TEX1_4          0x40000
#define VERT_TEX2_1          0x80000    /* 3rd texunit - reserved but unused */
#define VERT_TEX2_2          0x100000
#define VERT_TEX2_3          0x200000
#define VERT_TEX2_4          0x400000
#define VERT_END_VB          0x800000   /* end vb marker */
#define VERT_EVAL_C1         0x1000000  /* could reuse OBJ bits for this? */
#define VERT_EVAL_C2         0x2000000  /*    - or just use 3 bits */
#define VERT_EVAL_P1         0x4000000  /*  */
#define VERT_EVAL_P2         0x8000000  /*  */
#define VERT_SPEC_RGB        0x10000000
#define VERT_FOG_COORD       0x20000000	/* internal use only, currently */

#define VERT_EYE             VERT_BEGIN /* for pipeline management & cva */
#define VERT_WIN             VERT_END   /* some overlaps can be tolerated */
#define VERT_SETUP_FULL      0x4000000
#define VERT_SETUP_PART      0x8000000
#define VERT_PRECALC_DATA    VERT_END_VB /* overlap! */

/* Shorthands.
 */
#define VERT_TEX0_SHIFT 11

#define VERT_EVAL_ANY      (VERT_EVAL_C1|VERT_EVAL_P1|	\
                            VERT_EVAL_C2|VERT_EVAL_P2)

#define VERT_OBJ_23       (VERT_OBJ_3|VERT_OBJ_2)
#define VERT_OBJ_234      (VERT_OBJ_4|VERT_OBJ_23)
#define VERT_OBJ_ANY      VERT_OBJ_2

#define VERT_TEX0_12      (VERT_TEX0_2|VERT_TEX0_1)
#define VERT_TEX0_123     (VERT_TEX0_3|VERT_TEX0_12)
#define VERT_TEX0_1234    (VERT_TEX0_4|VERT_TEX0_123)
#define VERT_TEX0_ANY     VERT_TEX0_1

#define VERT_TEX1_12      (VERT_TEX1_2|VERT_TEX1_1)
#define VERT_TEX1_123     (VERT_TEX1_3|VERT_TEX1_12)
#define VERT_TEX1_1234    (VERT_TEX1_4|VERT_TEX1_123)
#define VERT_TEX1_ANY     VERT_TEX1_1

/* not used */
#define VERT_TEX2_12      (VERT_TEX2_2|VERT_TEX2_1)
#define VERT_TEX2_123     (VERT_TEX2_3|VERT_TEX2_12)
#define VERT_TEX2_1234    (VERT_TEX2_4|VERT_TEX2_123)
#define VERT_TEX2_ANY     VERT_TEX2_1

/* not used
#define VERT_TEX3_12      (VERT_TEX3_2|VERT_TEX3_1)
#define VERT_TEX3_123     (VERT_TEX3_3|VERT_TEX3_12)
#define VERT_TEX3_1234    (VERT_TEX3_4|VERT_TEX3_123)
#define VERT_TEX3_ANY     VERT_TEX3_1
*/

#define NR_TEXSIZE_BITS   4
#define VERT_TEX_ANY(i)   (VERT_TEX0_ANY<<(i*NR_TEXSIZE_BITS))

#define VERT_FIXUP         (VERT_TEX0_ANY|VERT_TEX1_ANY|VERT_RGBA| \
			    VERT_INDEX|VERT_EDGE|VERT_NORM)

#define VERT_DATA          (VERT_TEX0_ANY|VERT_TEX1_ANY|VERT_RGBA| \
			    VERT_INDEX|VERT_EDGE|VERT_NORM| \
	                    VERT_OBJ_ANY|VERT_MATERIAL|VERT_ELT| \
	                    VERT_EVAL_ANY|VERT_FOG_COORD)


#define VERT_TO_PIPE      (~VERT_END_VB)
#define VERT_FLAGS_TO_PIPE_FLAGS(x) (x & VERT_TO_PIPE)
#define PIPE_FLAGS_TO_VERT_FLAGS(x) (x & VERT_TO_PIPE)
#define PIPE_TEX(i)       VERT_TEX_ANY(i)



/* For beginstate
 */
#define VERT_BEGIN_0    0x1	   /* glBegin (if initially inside beg/end) */
#define VERT_BEGIN_1    0x2	   /* glBegin (if initially outside beg/end) */
#define VERT_ERROR_0    0x4	   /* invalid_operation in initial state 0 */
#define VERT_ERROR_1    0x8        /* invalid_operation in initial state 1 */




typedef void (*vb_transform_func)( GLcontext *ctx );


typedef GLuint (*clip_line_func)( struct vertex_buffer *VB,
				  GLuint *i, GLuint *j,
				  GLubyte mask);
typedef GLuint (*clip_poly_func)( struct vertex_buffer *VB,
				  GLuint n, GLuint vlist[],
				  GLubyte mask );

/*
 * The library context:
 */

struct gl_context {
   /* State possibly shared with other contexts in the address space */
   struct gl_shared_state *Shared;

   /* API function pointer tables */
   struct _glapi_table *Save;	/* Display list save funcs */
   struct _glapi_table *Exec;	/* Execute funcs */
   struct _glapi_table *CurrentDispatch;  /* == Save or Exec !! */

   GLvisual *Visual;
   GLframebuffer *DrawBuffer;	/* buffer for writing */
   GLframebuffer *ReadBuffer;	/* buffer for reading */

   /* Driver function pointer table */
   struct dd_function_table Driver;

   triangle_func TriangleFunc; /* driver or indirect triangle func */
   quad_func     QuadFunc;
   triangle_func ClippedTriangleFunc;
   clip_poly_func *poly_clip_tab;
   clip_line_func *line_clip_tab;

   void *DriverCtx;	/* Points to device driver context/state */
   void *DriverMgrCtx;	/* Points to device driver manager (optional)*/

   /* Core/Driver constants */
   struct gl_constants Const;

   /* Modelview matrix and stack */
   GLmatrix ModelView;           /* current matrix, not stored on stack */
   GLuint ModelViewStackDepth;
   GLmatrix ModelViewStack[MAX_MODELVIEW_STACK_DEPTH - 1];

   /* Projection matrix and stack */
   GLmatrix ProjectionMatrix;    /* current matrix, not stored on stack */
   GLuint ProjectionStackDepth;
   GLmatrix ProjectionStack[MAX_PROJECTION_STACK_DEPTH - 1];
   GLfloat NearFarStack[MAX_PROJECTION_STACK_DEPTH][2];

   /* Combined modelview and projection matrix */
   GLmatrix ModelProjectMatrix;

   /* Combined modelview, projection and window matrix */
   GLmatrix ModelProjectWinMatrix;
   GLboolean ModelProjectWinMatrixUptodate;

   /* Texture matrix and stack */
   GLmatrix TextureMatrix[MAX_TEXTURE_UNITS];
   GLuint TextureStackDepth[MAX_TEXTURE_UNITS];
   GLmatrix TextureStack[MAX_TEXTURE_UNITS][MAX_TEXTURE_STACK_DEPTH - 1];

   /* Color matrix and stack */
   GLmatrix ColorMatrix;
   GLuint ColorStackDepth;
   GLmatrix ColorStack[MAX_COLOR_STACK_DEPTH - 1];

   /* Display lists */
   GLuint CallDepth;		/* Current recursion calling depth */
   GLboolean ExecuteFlag;	/* Execute GL commands? */
   GLboolean CompileFlag;	/* Compile GL commands into display list? */
   GLboolean CompileCVAFlag;
   Node *CurrentListPtr;	/* Head of list being compiled */
   GLuint CurrentListNum;	/* Number of the list being compiled */
   Node *CurrentBlock;		/* Pointer to current block of nodes */
   GLuint CurrentPos;		/* Index into current block of nodes */

   /* Extensions */
   struct gl_extensions Extensions;


   /* Pipeline stages - shared between the two pipelines,
    * which live in CVA.
    */
   struct gl_pipeline_stage PipelineStage[MAX_PIPELINE_STAGES];
   GLuint NrPipelineStages;

   /* Cva */
   struct gl_cva CVA;

   /* Renderer attribute stack */
   GLuint AttribStackDepth;
   struct gl_attrib_node *AttribStack[MAX_ATTRIB_STACK_DEPTH];

   /* Renderer attribute groups */
   struct gl_accum_attrib	Accum;
   struct gl_colorbuffer_attrib	Color;
   struct gl_current_attrib	Current;
   struct gl_depthbuffer_attrib	Depth;
   struct gl_eval_attrib	Eval;
   struct gl_fog_attrib		Fog;
   struct gl_hint_attrib	Hint;
   struct gl_light_attrib	Light;
   struct gl_line_attrib	Line;
   struct gl_list_attrib	List;
   struct gl_pixel_attrib	Pixel;
   struct gl_point_attrib	Point;
   struct gl_polygon_attrib	Polygon;
   GLuint PolygonStipple[32];
   struct gl_scissor_attrib	Scissor;
   struct gl_stencil_attrib	Stencil;
   struct gl_texture_attrib	Texture;
   struct gl_transform_attrib	Transform;
   struct gl_viewport_attrib	Viewport;

   /* Other attribute groups */
   struct gl_histogram_attrib	Histogram;
   struct gl_minmax_attrib	MinMax;
   struct gl_convolution_attrib Convolution1D;
   struct gl_convolution_attrib Convolution2D;
   struct gl_convolution_attrib Separable2D;

   /* Client attribute stack */
   GLuint ClientAttribStackDepth;
   struct gl_attrib_node *ClientAttribStack[MAX_CLIENT_ATTRIB_STACK_DEPTH];

   /* Client attribute groups */
   struct gl_array_attrib	Array;	/* Vertex arrays */
   struct gl_pixelstore_attrib	Pack;	/* Pixel packing */
   struct gl_pixelstore_attrib	Unpack;	/* Pixel unpacking */

   struct gl_evaluators EvalMap;   /* All evaluators */
   struct gl_feedback   Feedback;  /* Feedback */
   struct gl_selection  Select;    /* Selection */

   struct gl_color_table ColorTable;       /* Pre-convolution */
   struct gl_color_table ProxyColorTable;  /* Pre-convolution */
   struct gl_color_table PostConvolutionColorTable;
   struct gl_color_table ProxyPostConvolutionColorTable;
   struct gl_color_table PostColorMatrixColorTable;
   struct gl_color_table ProxyPostColorMatrixColorTable;

   /* Optimized Accumulation buffer info */
   GLboolean IntegerAccumMode;	/* Storing unscaled integers? */
   GLfloat IntegerAccumScaler;	/* Implicit scale factor */

   struct gl_fallback_arrays Fallback;

   GLenum ErrorValue;        /* Last error code */

   /* Miscellaneous */
   GLuint NewState;          /* bitwise OR of NEW_* flags */
   GLuint Enabled;           /* bitwise or of ENABLE_* flags */
   GLenum RenderMode;        /* either GL_RENDER, GL_SELECT, GL_FEEDBACK */
   GLuint StippleCounter;    /* Line stipple counter */
   GLuint RasterMask;        /* OR of rasterization flags */
   GLuint TriangleCaps;      /* OR of DD_* flags */
   GLuint IndirectTriangles; /* TriangleCaps not handled by the driver */
   GLfloat PolygonZoffset;   /* Z offset for GL_FILL polygons */
   GLfloat LineZoffset;      /* Z offset for GL_LINE polygons */
   GLfloat PointZoffset;     /* Z offset for GL_POINT polygons */
   GLboolean NeedNormals;    /* Are vertex normal vectors needed? */
   GLuint FogMode;           /* FOG_OFF, FOG_VERTEX or FOG_FRAGMENT */

   GLboolean DoViewportMapping;

   GLuint RenderFlags;	/* Active inputs to render stage */

   GLuint RequireWriteableFlags; /* What can the driver/clipping tolerate? */

   /* Points to function which interpolates colors, etc when clipping */
   clip_interp_func ClipInterpFunc;
   GLuint ClipTabMask;

   normal_func *NormalTransform;

   /* Current shading function table */
   gl_shade_func *shade_func_tab;

   GLfloat EyeZDir[3];
   GLfloat rescale_factor;

   GLfloat vb_rescale_factor;
   GLmatrix *vb_proj_matrix;

   GLubyte   AllowVertexCull; /* To be set by the geometry driver */
   GLboolean NeedEyeCoords;
   GLboolean NeedEyeNormals;
   GLboolean NeedClipCoords;

   GLfloat backface_sign;

   GLboolean OcclusionResult;  /* GL_HP_occlusion_test */
   GLboolean OcclusionResultSaved;  /* GL_HP_occlusion_test */

   /* Destination of immediate mode commands */
   struct immediate *input;


   /* Cache of unused immediate structs
    */
   struct immediate *freed_im_queue;
   GLuint nr_im_queued;

   /* The vertex buffer being used by this context.
    */
   struct vertex_buffer *VB;

   /* The pixel buffer being used by this context */
   struct pixel_buffer* PB;

   struct gl_shine_tab *ShineTable[4];  /* Active shine tables */
   struct gl_shine_tab *ShineTabList;   /* Mru list of inactive shine tables */

   /* Should 3Dfx Glide driver catch signals? */
   GLboolean CatchSignals;

   /* For debugging/development only */
   GLboolean NoRaster;
   GLboolean FirstTimeCurrent;

   /* Dither disable via MESA_NO_DITHER env var */
   GLboolean NoDither;

#ifdef DEBUG
   GLboolean Rendering;
#endif
};


#ifdef MESA_DEBUG
extern int MESA_VERBOSE;
extern int MESA_DEBUG_FLAGS;
#else
# define MESA_VERBOSE 0
# define MESA_DEBUG_FLAGS 0
# ifndef NDEBUG
#  define NDEBUG
# endif
#endif


enum _verbose {
	VERBOSE_VARRAY          = 0x1,
	VERBOSE_TEXTURE         = 0x2,
	VERBOSE_IMMEDIATE       = 0x4,
	VERBOSE_PIPELINE        = 0x8,
	VERBOSE_DRIVER          = 0x10,
	VERBOSE_STATE           = 0x20,
	VERBOSE_API             = 0x40,
	VERBOSE_TRIANGLE_CHECKS = 0x80,
	VERBOSE_CULL            = 0x100,
	VERBOSE_DISPLAY_LIST    = 0x200,
	VERBOSE_LIGHTING        = 0x400
};


enum _debug {
	DEBUG_ALWAYS_FLUSH          = 0x1
};


extern void gl_flush_vb( GLcontext *ctx, const char *where );

extern void RESET_IMMEDIATE( GLcontext *ctx );

#define FLUSH_VB( ctx, where )			\
do {						\
	struct immediate *IM = ctx->input;	\
	if (IM->Flag[IM->Start])		\
		gl_flush_vb( ctx, where );	\
} while (0)




/* Test if we're inside a glBegin / glEnd pair:
*/
#define ASSERT_OUTSIDE_BEGIN_END( ctx, where )				\
do {									\
	struct immediate *IM = ctx->input;				\
	GLuint flag = IM->Flag[IM->Count];				\
	if ((flag & (VERT_BEGIN|VERT_END)) != VERT_END) {		\
                FLUSH_VB(ctx, where);					\
		if (ctx->Current.Primitive != GL_POLYGON+1) {		\
			gl_error( ctx, GL_INVALID_OPERATION, where );	\
			return;						\
		}							\
	}								\
} while (0)

#define ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL( ctx, where, what )	\
do {									\
	struct immediate *IM = ctx->input;				\
	GLuint flag = IM->Flag[IM->Count];				\
	if ((flag & (VERT_BEGIN|VERT_END)) != VERT_END) {		\
                FLUSH_VB(ctx, where);					\
		if (ctx->Current.Primitive != GL_POLYGON+1) {		\
			gl_error( ctx, GL_INVALID_OPERATION, where );	\
			return what;					\
		}							\
	}								\
} while (0)


#define ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH( ctx, where )	\
do {								\
	FLUSH_VB( ctx, where );					\
	if (ctx->Current.Primitive != GL_POLYGON+1) {		\
		gl_error( ctx, GL_INVALID_OPERATION, where );	\
		return;						\
	}							\
} while (0)


#define ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH_WITH_RETVAL( ctx, where, what )  \
do {								\
	FLUSH_VB( ctx, where );		                    	\
	if (ctx->Current.Primitive != GL_POLYGON+1) {		\
		gl_error( ctx, GL_INVALID_OPERATION, where );	\
		return what;					\
	}							\
} while (0)

#if 0
#undef ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL
#define ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH_WITH_RETVAL

#undef ASSERT_OUTSIDE_BEGIN_END
#define ASSERT_OUTSIDE_BEGIN_END ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH
#endif


#define Elements(x) sizeof(x)/sizeof(*(x))


#ifdef DEBUG

#define RENDER_START(CTX)			\
   do {						\
      assert(!(CTX)->Rendering);		\
      (CTX)->Rendering = GL_TRUE;		\
      if ((CTX)->Driver.RenderStart) {		\
         (*(CTX)->Driver.RenderStart)(CTX);	\
      }						\
   } while (0)

#define RENDER_FINISH(CTX)			\
   do {						\
      assert((CTX)->Rendering);			\
      (CTX)->Rendering = GL_FALSE;		\
      if ((CTX)->Driver.RenderFinish) {		\
         (*(CTX)->Driver.RenderFinish)(CTX);	\
      }						\
   } while (0)

#else

#define RENDER_START(CTX)			\
   do {						\
      if ((CTX)->Driver.RenderStart) {		\
         (*(CTX)->Driver.RenderStart)(CTX);	\
      }						\
   } while (0)

#define RENDER_FINISH(CTX)			\
   do {						\
      if ((CTX)->Driver.RenderFinish) {		\
         (*(CTX)->Driver.RenderFinish)(CTX);	\
      }						\
   } while (0)

#endif


#endif /* TYPES_H */
