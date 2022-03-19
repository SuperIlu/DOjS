/************************************************************************************\
  S3D ToolKit library.

  Copyright S3 Inc, 1995 - 1997.

  History
  -------
  ??-??-?? AL Initial library by Alex Lyashevsky
  97-09-08 MH cleanup
\************************************************************************************/

#ifndef __S3DTK_H
#define __S3DTK_H

/*
  S3D ToolKit library is aimed to ease programming the S3 graphical
  controller advanced 3D/2D capabilities. It is conceptually a HAL
  of the S3D engine with several utility functions.

  Environment and OS requirement:

  1. DOS + Rational System Extender.
  2. WIN95 + S3KRNL32.DLL + S3KERNEL.VXD.

  The library consists of four groups of functions :

  1. Set/GetState functions.
  2. 3D primitive functions.
  3. 2D primitive functions.
  4. Initialization and service functions.

  Outline for multi-threading Apps.

  1. Renderers created in different processes and in different thread of
  the same process could rely on the contention resolving scheme built in
  the ToolKit library. There is no need for explicit call to S3DK_EnterCritical
  if an user calls any ToolKit rendering functions (2D or 3D).

  2. If however user will decide to use S3D_EnterCtritical/ ReleaseCritical
  (it might be if he/she uses the library by single primitive basis but needs
  to prevent another app to use S3 2D/3D HW per primitives' packet basis)
  there is no restriction to use them.
  An user could call EnterCritical several times without any harm - only the
  first call will be taken into account. Similarly he/she can call
  ReleaseCritical pretty flexible. If there was a previous EnterCritical call
  from the same thread the critical section will be freed for entering by any
  other threads. If there was not at least one EnterCritical from the thread
  there will not be any action.
*/

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************\
   Definitions and macros
\************************************************************************************/

#ifdef  __DOS__
  // Set structure packing on long word boundaries
  #define DllExport     ULONG
#else
  #define DllExport     __declspec(dllexport) ULONG _cdecl
#endif

#define S3DTKVALUE  float
#define S3DTK_TRUE  1
#define S3DTK_FALSE 0

#define S3DTK_ERR	0
#define S3DTK_OK	1

#define S3DTK_ON	1
#define S3DTK_OFF	0

/*** Set/GetState functions' KEY parameters
***/

typedef enum {

  /*** SYSTEM INFORMATION 
  ***/

  /* Returns version number in the lowest 2 bytes (Get) */
  S3DTK_VERSION,

  /* VIDEO/MEMORY CONTROL AND INFORMATION (DOS ONLY) */

  /* Sets possible SVGA video mode (see the list below) (Set)
     Returns current video mode (Get) */
  S3DTK_VIDEOMODE,
  /* Returns linear address of the video memory (Get) */
  S3DTK_VIDEOMEMORYADDRESS,
  /* Returns size of the video memory (Get) */
  S3DTK_VIDEOMEMORYSIZE,

  /*** BUFFERIZATION
  ***/

  /* Sets current rendering surface using a pointer to the S3DTK_SURFACE
  // structure (see STRUCTURES section) as a second parameter (Set).
  // Returns information of the current rendering surface using
  // the second parameters as pointer to memory where the content
  // of the S3DTK_SURFACE structure will be copied (Get) */
  S3DTK_DRAWSURFACE,
  /* Sets current visible surface using pointer on the S3DTK_SURFACE
  // structure (see STRUCTURES section) as a second parameter (Set) (DOS ONLY).
  // Returns information of the current visible surface using
  // the second parameters as a pointer to memory where the content
  // of the S3DTK_SURFACE structure will be copied (Get) (DOS ONLY) */
  S3DTK_DISPLAYSURFACE,
  /* Returns 1 if Vsync interrupt status pending (display has been
  // fully refreshed), otherwise returns 0. (Get). 
  // This call can be used to prevent current the visible surface 
  // from being overdrawn by the next one or skipping previous frame */
  S3DTK_DISPLAYADDRESSUPDATED,
  /* Returns S3DTK_TRUE if S3 graphical engine idle. The call gives the 
  // correct time point for switching back anf fron buffer */
  S3DTK_GRAPHICS_ENGINE_IDLE,
  /* Turns OFF and ON DMA in DMA data transfer mode */
  S3DTK_DMA_OFF,
  S3DTK_DMA_ON,

  /*** RENDERING TYPE
  ***/

  /* Sets/returns current rendering type (see list below) (Set/Get) */
  S3DTK_RENDERINGTYPE,

  /*** Z BUFFER CONTROL
  ***/

  /* Sets/Returns Z-buffer offset in video memory (Set/Get) */
  S3DTK_ZBUFFERSURFACE,
  /* Sets/returns Z-buffer compare mode (see the list below) (Set/Get) */
  S3DTK_ZBUFFERCOMPAREMODE,
  /* Sets Z-buffering off or on with value eq S3DTK_OFF or S3DTK_ON 
  // respectively (Set). Note: ZBUFFERENABLE should be called after 
  // setting rendering buffer; otherwise the call will return an error. */
  S3DTK_ZBUFFERENABLE,
  /* Enables or disables Z-buffer updating. 
  // By default it is enabled but might be disabled for transparency */
  S3DTK_ZBUFFERUPDATEENABLE,

  /*** TEXTURE CONTROL
  ***/

  /* Sets current texture using a pointer to the S3DTK_SURFACE
  // structure (see STRUCTURES section) as a second parameter (Set).
  // Returns information of the current texture using the second parameter
  // as pointer to memory where the content of the S3DTK_SURFACE structure 
  // will be copied to (Get) */
  S3DTK_TEXTUREACTIVE,
  /* Sets/returns current texture filtering mode (see the list below) (Set/Get) */
  S3DTK_TEXFILTERINGMODE,
  /* Sets/returns current texture blending mode (see the list below) (Set/Get) */
  S3DTK_TEXBLENDINGMODE,
  /* Sets/returns the maximum number of mipmap levels for textures (Set/Get) */
  S3DTK_TEXMAXMIPMAPLEVEL,

  /*** ALPHABLENDING
  ***/

  /* Sets/returns current way of using ALPHA channel (see the list below) (Set/Get) */
  S3DTK_ALPHABLENDING,
  
  /*** FOG
  ***/

  /* Sets current color for fogging in RGBX888 format and turns fogging on. If value 
  // is eq S3DTK_FOGOFF fogging will be tuned off (Set)
  // Returns current fog color or S3DTK_FOGOFF if fogging is off (Get) */
  S3DTK_FOGCOLOR,
  /* If S3DTK_ON is set the Toolkit won't compute D level but will use D specified
  // from application instead, otherwise (S3DTK_OFF - default) D will be computed 
  // within the Toolkit. */
  S3DTK_D_LEVEL_SUPPLIED,

  /*** MISC STUFF
  ***/

  /* Set/Get clipping sub-rect area inside a current rendering surface */
  S3DTK_CLIPPING_AREA,

  /* Flip and wait until done (default is S3DTK_TRUE) - DOS only */
  S3DTK_FLIP_WAIT

} S3DTK_STATEKEY;

/*** SUPPORTED VIDEO MODES 
***/
#define S3DTK_MODE320x200x8       0x13
#define S3DTK_MODE320x200x15      0x10D
#define S3DTK_MODE512x384x8       0x215
#define S3DTK_MODE512x384x15      0x216
#define S3DTK_MODE640x400x8       0x100
#define S3DTK_MODE640x480x8       0x101
#define S3DTK_MODE640x480x15      0x110
#define S3DTK_MODE640x480x24      0x112
#define S3DTK_MODE800x600x8       0x103
#define S3DTK_MODE800x600x15      0x113
#define S3DTK_MODE800x600x24      0x115
#define S3DTK_MODE1024x768x8      0x105
#define S3DTK_MODE1024x768x15     0x116
#define S3DTK_MODE1024x768x24     0x118

#define S3DTK_MODESET_NO_CLEAR    0x8000    /* OR the mode with this flag to */
                                            /* set mode without clear the    */
                                            /* video memory                  */

/*** POSSIBLE RENDERING TYPES 
***/
#define S3DTK_GOURAUD               0L
#define S3DTK_LITTEXTURE            0x8000000L
#define S3DTK_UNLITTEXTURE          0x10000000L
#define S3DTK_LITTEXTUREPERSPECT    0x28000000L
#define S3DTK_UNLITTEXTUREPERSPECT  0x30000000L

/*** Z COMPARE MODES 
***/
#define S3DTK_ZNEVERPASS            0L
#define S3DTK_ZSRCGTZFB             0x100000L
#define S3DTK_ZSRCEQZFB             0x200000L
#define S3DTK_ZSRCGEZFB             0x300000L
#define S3DTK_ZSRCLSZFB             0x400000L
#define S3DTK_ZSRCNEZFB             0x500000L
#define S3DTK_ZSRCLEZFB             0x600000L
#define S3DTK_ZALWAYSPASS           0x700000L

/*** SUPPORTED TEXTURE COLOR FORMATS 
***/
#define FORMAT_SHIFT  5
#define S3DTK_TEXARGB8888      ( 0 << FORMAT_SHIFT )
#define S3DTK_TEXARGB4444      ( 1L << FORMAT_SHIFT )
#define S3DTK_TEXARGB1555      ( 2L << FORMAT_SHIFT )
#define S3DTK_TEXPALETTIZED8   ( 6L << FORMAT_SHIFT )

/*** TEXTURE FILTERING MODE 
***/
#define FILTER_SHIFT  12
#define S3DTK_TEXM1TPP         ( 0L << FILTER_SHIFT )  /* MIP_NEAREST */
#define S3DTK_TEXM2TPP         ( 1L << FILTER_SHIFT )  /* LINEAR MIP NEAREST */
#define S3DTK_TEXM4TPP         ( 2L << FILTER_SHIFT )  /* MIP LINEAR */
#define S3DTK_TEXM8TPP         ( 3L << FILTER_SHIFT )  /* LINEAR MIP LINEAR */
#define S3DTK_TEX1TPP          ( 4L << FILTER_SHIFT )  /* NEAREST ( NO MIP ) */
#define S3DTK_TEXV2TPP         ( 5L << FILTER_SHIFT )  /* thought to be VIDEO but actually is linear filter */
#define S3DTK_TEX4TPP          ( 6L << FILTER_SHIFT )  /* LINEAR ( NO MIP ) */

/*** TEXTURE BLENDING MODE 
***/
#define BLEND_MODE_SHIFT 15
#define S3DTK_TEXMODULATE         ( 1L << BLEND_MODE_SHIFT )
#define S3DTK_TEXDECAL            ( 2L << BLEND_MODE_SHIFT )

/*** ALPHA BLENDING TYPES 
***/
#define ALPHA_BLEND_CONTROL_SHIFT   18
#define S3DTK_ALPHAOFF               0
#define S3DTK_ALPHATEXTURE           2 << ALPHA_BLEND_CONTROL_SHIFT
#define S3DTK_ALPHASOURCE            3 << ALPHA_BLEND_CONTROL_SHIFT

/*** FOG
***/
#define S3DTK_FOGOFF        0xFFFFFFFF

/*** ERROR LIST 
***/
#define S3DTK_3DCAPNOTSUPPORTED       -1	/* 3D capabilities are not supported */
#define S3DTK_CANTCONVERT             -2	/* cant convert from linear to phys address */
#define S3DTK_INVALIDFORMAT           -3	/* invalid format */
#define S3DTK_NULLPOINTER             -4	/* null pointer */
#define S3DTK_INVALIDRENDETINGTYPE    -5	/* invalid rendering type */
#define S3DTK_INVALIDVALUE            -6	/* invalid value*/
#define S3DTK_UNSUPPORTEDVIDEOMODE    -7	/* unsupported video mode */
#define S3DTK_INVALIDFILTERINGMODE    -8	/* invalid filtering mode */
#define S3DTK_UNSUPPORTEDKEY          -9	/* unsupported key */
#define S3DTK_UNSUPPORTEDMETHOD       -10	/* unsupported method */
#define S3DTK_INVALIDSURFACE          -11	/* surface is not set */
#define S3DTK_NOS3KERNEL              -12	/* can't find S3KERNEL VxD */

/************************************************************************************\
   Structures (and acompanying definitions)
\************************************************************************************/

/*** S3DTK_LIB_INIT
***/
typedef struct _S3DTK_LIB_INIT {

  ULONG libFlags;					/* see LIB_INIT Flags below */
  ULONG libVideoBufferLinAddr;		/* DO not use */
  ULONG libMMIOSpaceLinAddr;		/* DO not use */

} S3DTK_LIB_INIT, * S3DTK_LPLIB_INIT;

/*** LIB_INIT Flags
***/
#define S3DTK_INITPIO                    0		/* programming device using registers */
#define S3DTK_INITDMA                    1		/* programming device using command DMA */
#define S3DTK_INIT2D_SERIALIZATION_ON    2		/* serialize access to S3D endine */
#define S3DTK_INIT_DMA_SIZE_64K          0x10	/* if bit 0 is set this bit defines the DMA size */
												/* either needed or provided. if 0 - 4K( default ), 1 - 64k */
/* DO not use */
#define S3DTK_INIT_VIDEO_BUFFER_ADDR_PROVIDED 4 /* addrs for video buff and MMI0 space are */
												/* provided by this call */
/* DO not use */
#define S3DTK_INIT_DMA_BUFFER_ADDR_PROVIDED 8	/* address of contigiuos memblock for DMA */
												/* is supplied */

/*** S3DTK_RENDERER_INITSTRUCT
***/
typedef struct {

  ULONG initFlags;		/* see RENDERER_INITSTRUCT Flags bits below */
  ULONG initUserID;		/* see RENDERER_INITSTRUCT User ID below */
  ULONG initAppID;		/* see RENDERER_INITSTRUCT App ID below */

} S3DTK_RENDERER_INITSTRUCT, * S3DTK_LPRENDERER_INITSTRUCT;

/*** RENDERER_INITSTRUCT Flags Bits
***/
#define S3DTK_GENERIC         0   /* float + do not clip UV + do not clip XY */
#define S3DTK_FORMAT_FLOAT    0
#define S3DTK_FORMAT_FIXED    1L  /* not implemented */
#define S3DTK_VERIFY_UVRANGE  2L  /* is needed only for perspactive texture mapping */
#define S3DTK_VERIFY_XYRANGE  4L  /* is needed if upper layer sw does not consider to clip */
                                  /* along the view port window */

/*** RENDERER_INITSTRUCT User ID
***/
#define S3DTK_USER_GENERIC 0
#define S3DTK_USER_WIN95   1

/*** RENDERER_INITSTRUCT App ID
***/
#define S3DTK_APP_GENERIC 0
#define S3DTK_APP_D3D     1

/*** S3DTK_SURFACE
***/
typedef struct {

  ULONG sfOffset;		/* offset either in sytem or video memory */
  ULONG sfWidth;        /* width of surface in pixel */
  ULONG sfHeight;       /* height of surface in pixel */
  ULONG sfFormat;       /* format and location of surface, see SUPPORTED SURFACE */
						/* FORMATS below */
  ULONG reserved[ 5 ];	/* DO not use */

} S3DTK_SURFACE, * S3DTK_LPSURFACE;

/*** SUPPORTED SURFACE FORMATS 
***/
#define S3DTK_VIDEORGB8           0				/* palettized */
#define S3DTK_VIDEORGB15          0x4L			/* 5, 5, 5    */
#define S3DTK_VIDEORGB24          0x8L			/* 8, 8 ,8    */
#define S3DTK_SYSTEM              0x80000000L   /* surface is in system memory*/
#define S3DTK_VIDEO               0x00000000L   /* surface is in video memory */
#define S3DTK_TEXTURE             0x40000000L   /* surface for texture        */
#define S3DTK_Z16                 0x10000000L   /* artificial Z-buffer format */

/*** S3DTK_RECTAREA
***/
typedef struct {

  long int left;
  long int top;
  long int right;
  long int bottom;

} S3DTK_RECTAREA, * S3DTK_LPRECTAREA;

/*** S3DTK_VERTEX_LIT
***/
typedef struct _s3dtk_vertex_lit
{

  S3DTKVALUE  X;
  S3DTKVALUE  Y;
  S3DTKVALUE  Z;
  S3DTKVALUE  W; /* not used */
  BYTE        B;
  BYTE        G;
  BYTE        R;
  BYTE        A;

} S3DTK_VERTEX_LIT, * S3DTK_LPVERTEX_LIT;

/*** S3DTK_VERTEX_TEX
***/
typedef struct _s3dtk_vertex_tex {

  S3DTKVALUE X;
  S3DTKVALUE Y;
  S3DTKVALUE Z;
  S3DTKVALUE W;
  BYTE       B;
  BYTE       G;
  BYTE       R;
  BYTE       A;
  S3DTKVALUE D;
  S3DTKVALUE U;
  S3DTKVALUE V;

} S3DTK_VERTEX_TEX, * S3DTK_LPVERTEX_TEX;

/************************************************************************************\
   Function prototypes
\************************************************************************************/

/*** Acceptable primitive types for TriangleSet/TriangleSetEx
***/
#define S3DTK_TRILIST  0
#define S3DTK_TRISTRIP 1
#define S3DTK_TRIFAN   2
#define S3DTK_LINE     3
#define S3DTK_POINT    4

/*** S3DTK FUNCTIONS LIST
***/
typedef struct _s3dtk_function_list *S3DTK_LPFUNCTIONLIST;
typedef struct _s3dtk_function_list
{

  /*** 1. Set/GetState functions
  **** -------------------------
  ***/

  /* Set 3D engine states */
  ULONG (* S3DTK_SetState)(S3DTK_LPFUNCTIONLIST pFuncStruct,
                           ULONG state, ULONG value);
  /* Function updates the state of 3D engine. The updated state
  // will take effect from the next primitive being rendered
  //
  // Params:
  //    pFuncStruct     pointer to current instance of renderer
  //                    as created by S3DTK_CreateRenderer
  //    state           KEY identifying one of the possible
  //                    3D engine status entries, see list at
  //                    beginning of header
  //    value           content depends on state value passed to
  //                    function
  */

  /* Get 3D engine states */
  ULONG (* S3DTK_GetState)(S3DTK_LPFUNCTIONLIST pFuncStruct,
                           ULONG state, void *value);
  /* Function returns the current state of the 3D engine.
  //
  // Params:
  //    pFuncStruct     see above
  //    state           see S3DTK_SetState
  //    value           for some state values this has to be
  //                    a valid pointer to a structure in order
  //                    to return function results
  */

  /*** 2. 3D primitive functions
  **** -------------------------
  ***/

  /* Render primitives (triangles/lines) */
  ULONG (* S3DTK_TriangleSet)(S3DTK_LPFUNCTIONLIST pFuncStruct,
                              void **ppVertexSet,
                              ULONG NumVertices, ULONG SetType);
  /* This call causes the S3D engine to render the specified type
  // of primitives.
  //
  // Params:
  //    pFuncStruct     see above
  //    ppVertexSet     points to an array of vertex pointers 
  //                        (S3DTK_VERTEX_xxx)
  //    NumVertices     number of entries in this array
  //    SetType         type of primitive to be rendered (see above)
  */

  /* Render primitives (triangles/lines), extended call */
  ULONG (* S3DTK_TriangleSetEx)(S3DTK_LPFUNCTIONLIST pFuncStruct,
                                void **pVertexSet, ULONG NumVertices,
                                ULONG SetType, 
                                ULONG *pSetState, ULONG NumStates);
  /* like S3DTK_TriangleSet, but sets S3D engine to specified state
  // before rendering primitives
  //
  // Params:
  //    pFuncStruct     see above
  //    ppVertexSet     see above
  //    NumVertices     see above
  //    SetType         see above
  //    pSetState       points to an array of ULONG value pairs,
  //                    alternatig ULONG key; ULONG value which have
  //                    the same meaning as in the S3DTK_SetState function.
  //    NumStates       number of *pairs* in this array
  */

  /*** 3. 2D primitive functions
  **** -------------------------
  ***/

  /* Copy a rect area inside video memory */
  ULONG (* S3DTK_BitBlt)(S3DTK_LPFUNCTIONLIST pFuncStruct, 
                         S3DTK_LPSURFACE pDestSurface, 
                         S3DTK_LPRECTAREA pDestRect,
                         S3DTK_LPSURFACE pSrcSurface, 
                         S3DTK_LPRECTAREA pSrcRect);
  /* This function copies the rectangle area from video memory defined
  // by the second parameter set to the location in the video
  // memory defined by the first parameter set.
  //
  // Params:
  //    pFuncStruct     see above
  //    pDestSurface    destination surface 
  //    pDestRect       destination rectangle (no stretching possible)
  //    pSrcSurface     source surface
  //    pSrcRect        source rectangle
  */

  /* copy a rect area inside video memory with screen door transparency */
  ULONG (* S3DTK_BitBltTransparent)(S3DTK_LPFUNCTIONLIST pFuncStruct, 
                                    S3DTK_LPSURFACE pDestSurface, 
                                    S3DTK_LPRECTAREA pDestRect,
                                    S3DTK_LPSURFACE pSrcSurface, 
                                    S3DTK_LPRECTAREA pSrcRect,
                                    ULONG TranspColor);
  /* This function causes the rectangle area from video memory defined
  // by the second parameter set to be copied to the location in the video
  // memory defined by the first parameter set. If a source pixel color is
  // equal to the color defined by TransparentColor the destination
  // pixel will not be updated. Color format for transparent color matches
  // the surfaces' color format.
  //
  // Params:
  //    pFuncStruct     see above
  //    pDestSurface    destination surface 
  //    pDestRect       destination rectangle (no stretching possible)
  //    pSrcSurface     source surface
  //    pSrcRect        source rectangle
  //    TranspColor     transparent color
  */

  /* fill a rect area in video memory with color */
  ULONG (* S3DTK_RectFill)(S3DTK_LPFUNCTIONLIST pFuncStruct,
                           S3DTK_LPSURFACE pDestSurface,
                           S3DTK_LPRECTAREA pDestRect,
                           ULONG FillColor);

  /* This function fills the specified area in video memory with
  // the given color. The color format of FillColor matches the
  // current surface format.
  //
  // Params:
  //    pFuncStruct     see above
  //    pDestSurface    destination surface 
  //    pDestRect       destination rectangle (no stretching possible)
  //    FillColor       color used for filling
  */

  /*** 4. Initialization and service functions
  **** ---------------------------------------
  ****/

  /* return lats error occurred inside toolkit library */
  int (* S3DTK_GetLastError)(S3DTK_LPFUNCTIONLIST pFuncStruct);
  /* This function returns the code of the last error if it
  // is called immediately after the function that returned S3DTK_ERR.
  //
  // Params:
  //    pFuncStruct     see above
  */

#ifndef WIN32
  /*** 5. Hardware stretching function (DOS only)
  **** ------------------------------------------
  ****/

  ULONG (* S3DTK_StretchDisplaySurface)(S3DTK_LPFUNCTIONLIST pFuncStruct,
                                        ULONG width0, ULONG height0, 
                                        ULONG width1, ULONG height1);
  /* Stretch the surface pointed by pDisplaySurface to width by height.
  // This function is supported in DOS only. Use DirectDraw functions in Win95.
  //
  // Params:
  //    pFuncStruct     see above
  //    width0          width of display area before stretching
  //    height0         height of display area before stretching
  //    width1          width of display area after being stretched
  //    height1         height of display area after being stretched
  */
#endif /* not for WIN32 */

} S3DTK_FUNCTIONLIST;

/*** S3DTK_PhysicalToLinear
***/
DllExport S3DTK_PhysicalToLinear(ULONG PhysAddr, ULONG Size);
/* Maps given physical address and the size of the physical
// memory area to linear address.
// Caution: In a multi-threading environment linear address is valid
// only in the calling thread's memory context.
//
// Params:
//      PhysAddr    physical address (usually device address in
//                      386 4GB address space)
//      Size        size of the area to be mapped
// Return:
//      Linear address or S3DTK_ERR
*/

/*** S3DTK_LinearToPhysical
***/
DllExport S3DTK_LinearToPhysical(ULONG Linear);
/* Finds a physical address being mapped on the given linear
// address.
//
// Params:
//      Linear      linear address in the current thread's memory context
// Return:
//      Physical address or S3DTK_ERR
*/

/*** S3DTK_EnterCritical
***/
DllExport S3DTK_EnterCritical(void);
/* Serializes access to shared resources in WIN95 multi-thread
// environment.
// Function is for serializing an access to S3 2D/3D engine
// among 32bit threads and WIN95 GUI subsystem. The call
// to the function either permit the current thread to enter
// the following code section or put the thread on hold until
// the thread owing the section will call S3DTK_ReleaseCritical.
//
// Return:
//      S3DTK_OK if it is successful.
//      S3DTK_ERR if sharing mechanism isn't in place.
*/

/*** S3DTK_ReleaseCritical
***/
DllExport S3DTK_ReleaseCritical(void);
/* Releases the critical section owned by the thread that issued
// S3DTK_EnterCritical
//
// Return:
//      S3DTK_OK.
*/

/*** S3DTK_InitLib
***/
DllExport S3DTK_InitLib(S3DTK_LPLIB_INIT pInit);
/* Init 2D/3D engine and Toolkit lobrary.
// This function initializes the Toolkit library. Must be called
// before using any other function of this toolkit.
//
// Params:
//      pInit   pointer to S3DTK_LIB_INIT structure, NULL for
//              default values
//
// Return:
//      S3DTK_OK or one of the possible error codes (see list)
*/

/*** S3DTK_ExitLib
***/
DllExport S3DTK_ExitLib(void);
/* Clean up. Must be called before exiting the App.
//
// Return:
//      S3DTK_OK
*/

/*** S3DTK_CreateRenderer
***/
DllExport S3DTK_CreateRenderer(S3DTK_LPRENDERER_INITSTRUCT pInit,
                               S3DTK_LPFUNCTIONLIST *ppFunctionList);
/* This function creates a new instance of the rendering device and returns
// a pointer to the list of S3DTK interface functions.
//
// Params:
//      pInit           pointer to valid S3DTK_RENDERER_INITSTRUCT or
//                      NULL for default values
//      ppFunctionList  pointer to function list pointer, filled by
//                      S3DTK_CreateRenderer
//
// Return:
//      S3DTK_OK or one of the possible error codes (see list)
*/

/*** S3DTK_DestroyRenderer
***/
DllExport S3DTK_DestroyRenderer(S3DTK_LPFUNCTIONLIST *ppFunctionList);
/* Destroys the given instance of the rendering device.
// Note: After this call ppFunctionList is invalid!
//
// Params:
//      ppFunctionList  pointer to function list pointer
//
// Return:
//      S3DTK_OK
*/

#ifdef __cplusplus
}
#endif

#endif /* __S3DTK_H */
