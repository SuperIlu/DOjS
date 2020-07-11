/*
 * DZcomm : a serial port API.
 * file : dzmngw32.h
 *
 * Configuration defines for use with djgpp.
 * 
 * See readme.txt for copyright information.
 */

#ifndef DZMNGW32_H
#define DZMNGW32_H

#ifndef __MINGW32__
   #error bad include
#endif

//#include <io.h>
//#include <fcntl.h>
//#include <direct.h>
//#include <malloc.h>


/* describe this platform */
#define DZCOMM_PLATFORM_STR  "Mingw32"
#define DZCOMM_MINGW32
#define DZCOMM_WINDOWS
#define DZCOMM_I386
#define DZCOMM_LITTLE_ENDIAN


/* describe how function prototypes look to MINGW32 __declspec(dllexport)
      works reliably only for functions and standard C varibles (Apr. 99) */

//#define AL_METHOD(type, name, args)          type (*name) args
//#ifndef AL_INLINE
//  #define AL_INLINE(type, name, args, code)    extern inline type name args code
//#endif

//#define _AL_DLL   __declspec(dllimport)
//#define _FIX_DLL  _AL_DLL
//#define AL_VAR(type, name)                   extern _AL_DLL type name
//#define AL_ARRAY(type, name)                 extern _AL_DLL type name[]
//#define AL_FUNCPTR(type, name, args)         extern _AL_DLL type (*name) args
//#define AL_FUNC(type, name, args)            type __cdecl name args

/* windows specific defines */
//#define NONAMELESSUNION

/* arrange for other headers to be included later on */
#define DZCOMM_EXTRA_HEADER     "dzcomm/dzming.h"
//#define DZCOMM_INTERNAL_HEADER  "dzcomm/dintwin.h"
//#define DZCOMM_MMX_HEADER       "obj/mingw32/mmx.h"

#endif /* DZMNGW32_H */
