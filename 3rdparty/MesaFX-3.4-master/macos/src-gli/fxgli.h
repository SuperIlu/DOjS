#ifndef __FX_GLI_H__
#define __FX_GLI_H__


#include "gliMesa.h"
#include "gliUtils.h"
#include "types.h"
#include "fxmesa.h"
#include "mgliGetEnv.h"
/***********************************************************************************************
 *
 * MACROS:
 *
 * There are two version of the fx-Mesa for MacOS:
 *		1. Built into the Mesa/AGL code.
 *		2. Built as standalone GLI plugin for Apple OpenGL. 
 *	(Hopefully they will be merged in the future...)
 ***********************************************************************************************/
#if !(MESA_3DFX_STANDALONE_GLI_PLUGIN)
	#define gliGetAttribute			fx_gliGetAttribute
	#define gliSetAttribute			fx_gliSetAttribute
	#define gliQueryRendererInfo	fx_gliQueryRendererInfo
	#define gliDestroyRendererInfo	fx_gliDestroyRendererInfo
	#define gliGetSystemDispatch	fx_gliGetSystemDispatch
	#define gliGetFunctionDispatch	fx_gliGetFunctionDispatch
	#define gliGetExtensionDispatch	fx_gliGetExtensionDispatch
	#define gliDestroyContext		fx_gliDestroyContext
	#define gliCreateContext		fx_gliCreateContext
	#define gliSwapBuffers			fx_gliSwapBuffers
	#define gliGetInteger			fx_gliGetInteger
	#define gliSetInteger			fx_gliSetInteger
	#define gliAttachDrawable		fx_gliAttachDrawable
#endif
/***********************************************************************************************
 *
 * Constants: (These are defined in fxgli_constants.c)
 *
 ***********************************************************************************************/
/* These are strings to be used with mesaconfig.cfg*/
extern const char* 	kEnv_EmulateWindowRendering;
extern const char* 	kEnv_EmulateCompliance;
extern const char*	kEnv_EmulateMouseCursor;
extern const char*	kEnv_DefaultGammaValue;
extern const char*	kEnv_CheatTextureMemory;
extern const char*	kEnv_CheatVideoMemory;
extern const char*  kEnv_SwapIntervall;
extern const char*  kEnv_MaxSwapPending;
extern const char*  kEnv_DitherMode;
extern const char*  kEnv_ScreenRefresh;
extern const char* 	kEnv_IgnoreGLFinish;

/* These are version numbers */
extern const int 	kMajorVersion;
extern const int 	kMinorVersion;

/***********************************************************************************************
 *
 * Prototypes for determining configurations
 *
 ***********************************************************************************************/
extern GLboolean fxgli_IsEmulateWindowRendering(void);
extern GLboolean fxgli_IsEmulateCompliance(void);
extern GLboolean fxgli_IsEmulateCursor(void);

extern int fxgli_GetDefaultTextureMemorySize(void);
extern int fxgli_GetDefaultVideoMemorySize(void);
extern int fxgli_GetDefaultDitherMode(void);
extern int fxgli_GetDefaultScreenRefresh(void);
/***********************************************************************************************
 *
 * Prototypes for determining default configurations:
 *
 ***********************************************************************************************/
extern	int fxgli_GetDefaultGammaValue(void);			
extern 	int fxgli_GetDefaultSwapIntervall(void);			
extern 	int fxgli_GetDefaultMaxSwapPendingBuffers(void);

/***********************************************************************************************
 *
 * Macros for errors (TODO):
 *
 ***********************************************************************************************/
#define GLI_HARDWARE_ERROR					GLI_BAD_ALLOC

/***********************************************************************************************
 *
 * Type definitions:
 *
 ***********************************************************************************************/
/* Warper record for FXContext */
typedef struct TFXContext {
	GLcontext			*gl_ctx;
	
	/* Just for identifiing */
	GLint				renderer_id; /* Should be: GLI_RENDERER_MESA_3DFX */


	TGLIUtilsDrawable 	drawable;
	

	
	fxMesaContext		ctx;
	
	GLIPixelFormat		fmt;
	
	/* For GetInteger, and SetInteger */
	GLint				gamma_value;
	GLint				swap_interval;
	GLint				max_pending_num;
	GLboolean			screen_takeover;
	
	/* Cursor emulation */
	GLboolean 			emulate_cursor;		
#ifdef GLI_DEBUG_FX_TRIANGLES	
	/* For Debugging */
	float				nearMin,farMax;
#endif
} TFXContext;
/***********************************************************************************************
 *
 * Currently we can handle only one context!
 *
 ***********************************************************************************************/
extern TFXContext	*current_context;

/***********************************************************************************************
 *
 * Prototypes:
 *
 ***********************************************************************************************/
extern void setDefaultIntegerValues(TFXContext *ctx);

extern void setupFixGrLfbReadRegionCall();

extern void fxgli_PixelFormatToFXAttribList(const GLIPixelFormat *fmt, GLint attrib[]);

extern void fxgli_FillExtDispatchTable(GLIExtensionDispatch* inDT);
extern void fxgli_FillDispatchTable(GLIExtensionDispatch* inDT);

#if !(MESA_3DFX_STANDALONE_GLI_PLUGIN)
	extern void 	 fx_gliMakeCurrent(GLIContext ctx);
	extern GLboolean fx_gliGetAttribute(GLIContext ctx, GLenum type, void *attrib);
	extern GLboolean fx_gliSetAttribute(GLIContext ctx, GLenum type, const void *attrib);
	extern GLenum 	 fx_gliQueryRendererInfo(GLIRendererInfo **info_ret, const GLIDevice *dev, GLint ndevs);
	extern GLenum 	 fx_gliDestroyRendererInfo(GLIRendererInfo *info);
	extern GLenum 	 fx_gliGetSystemDispatch(GLIContext ctx, GLIFunctionDispatch *table, GLbitfield change_flags);
	extern GLenum 	 fx_gliGetFunctionDispatch(GLenum mode,GLIFunctionDispatch *table);
	extern GLenum 	 fx_gliGetExtensionDispatch(GLenum mode,GLIExtensionDispatch *table);
	extern GLenum	 fx_gliAttachDrawable(GLIContext ctx,GLint drawable_type,const GLIDrawable *drw);
	extern GLenum 	 fx_gliCreateContext(GLIContext *ctx_ret, const GLIPixelFormat *fmt, GLIContext share_list);
	extern GLenum 	 fx_gliDestroyContext(GLIContext ctx);
	extern GLenum 	 fx_gliSwapBuffers(GLIContext ctx);
	extern GLenum 	 fx_gliSetInteger(GLIContext inCtx,GLenum pname,const GLint	*value);
	extern GLenum 	 fx_gliGetInteger(GLIContext inCtx,GLenum pname,GLint *value);
#endif

extern void setupHacks();

#if MESA_3DFX_PROFILE
#include "Profiler.h"
#endif


#endif /* __FX_GLI_H__ */