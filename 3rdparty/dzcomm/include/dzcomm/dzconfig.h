/*
 * DZcomm : a serial port API.
 * file : dzconfig.h
 *
 * Configuration defines
 *
 * By Neil Townsend based on the Allegro project
 *
 * See readme.txt for copyright information.
 */

/* platform generic stuff */

#define DZ_ID(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

/* include platform-specific stuff */
#if defined SCAN_EXPORT
#include "dzscanex.h"
#elif defined __RSXNT__
#include "dzrsxnt.h"
#elif defined __MINGW32__
#include "dzmngw32.h"
#elif defined _MSC_VER
#include "dzmsvc.h"
#elif defined __WATCOMC__
#include "dzwatcom.h"
#elif defined DJGPP
#include "dzdjgpp.h"
#elif defined __unix__
#include "dzucfg.h"
#elif defined __BEOS__
#include "dzbecfg.h"
#else
#error unknown platform
#endif

/* special definitions for the GCC compiler */
#ifdef __GNUC__
#define DZCOMM_GCC

#ifndef DZ_INLINE
#define DZ_INLINE(type, name, args, code) inline type name args code
//      #define DZ_INLINE(type, name, args, code)    extern inline type name args code
#endif

#define DZ_PRINTFUNC(type, name, args, a, b) DZ_FUNC(type, name, args) __attribute__((format(printf, a, b)))

//#define CONSTRUCTOR_FUNCTION(func) func __attribute__((constructor))

//#define INLINE inline

#ifndef ZERO_SIZE
#define ZERO_SIZE 0
#endif

#define LONG_LONG long long

#ifdef __i386__
#define DZCOMM_I386
#endif
#endif

/* the rest of this file fills in some default definitions of language
 * features and helper functions, which are conditionalised so they will
 * only be included if none of the above headers defined custom versions.
 */

#ifndef INLINE
#define INLINE
#endif

#ifndef ZERO_SIZE
#define ZERO_SIZE
#endif

#ifndef DZ_VAR
#define DZ_VAR(type, name) extern type name
#endif

#ifndef DZ_ARRAY
#define DZ_ARRAY(type, name) extern type name[]
#endif

#ifndef DZ_FUNC
#define DZ_FUNC(type, name, args) type name args
#endif

#ifndef DZ_PRINTFUNC
#define DZ_PRINTFUNC(type, name, args, a, b) AL_FUNC(type, name, args)
#endif

#ifndef DZ_METHOD
#define DZ_METHOD(type, name, args) type(*name) args
#endif

#ifndef DZ_FUNCPTR
#define DZ_FUNCPTR(type, name, args) extern type(*name) args
#endif

#ifndef DZ_INLINE
#define DZ_INLINE(type, name, args, code) type name args;
#endif

/* fill in default memory locking macros */
// #ifndef END_OF_FUNCTION
// #define END_OF_FUNCTION(x)
// #define END_OF_STATIC_FUNCTION(x)
// #define LOCK_DATA(d, s)
// #define LOCK_CODE(c, s)
// #define UNLOCK_DATA(d, s)
// #define LOCK_VARIABLE(x)
// #define LOCK_FUNCTION(x)
// #endif

/* emulate the FA_* flags for platforms that don't already have them */
#ifndef FA_RDONLY
#define FA_RDONLY 1
#define FA_HIDDEN 2
#define FA_SYSTEM 4
#define FA_LABEL 8
#define FA_DIREC 16
#define FA_ARCH 32
#endif

/* don't currently use these, but if in case I need to later ... */
#ifndef DZCOMM_DJGPP
#define _farsetsel(seg)
#define _farnspokeb(addr, val) (*((unsigned char *)(addr)) = (val))
#define _farnspokew(addr, val) (*((unsigned short *)(addr)) = (val))
#define _farnspokel(addr, val) (*((unsigned long *)(addr)) = (val))
#define _farnspeekb(addr) (*((unsigned char *)(addr)))
#define _farnspeekw(addr) (*((unsigned short *)(addr)))
#define _farnspeekl(addr) (*((unsigned long *)(addr)))
#endif
