/*
 * DZcomm : a serial port API.
 * file : dzdjgpp.h
 *
 * Configuration defines for use with djgpp.
 * 
 * See readme.txt for copyright information.
 */



#ifndef DJGPP
   #error bad include
#endif


#include <dos.h>
#include <sys/movedata.h>
#include <conio.h>
#include <io.h>

#include <pc.h>
#include <dir.h>
#include <dpmi.h>
#include <go32.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/farptr.h>


/* describe this platform */
#define DZCOMM_PLATFORM_STR  "djgpp"
#define DZCOMM_DJGPP
#define DZCOMM_DOS
#define DZCOMM_LITTLE_ENDIAN
#define DZCOMM_CONSOLE_OK
#define DZCOMM_VRAM_SINGLE_SURFACE


/* memory locking macros */
void _unlock_dpmi_data(void *addr, int size);

#define END_OF_FUNCTION(x)          void x##_end(void) { }
#define END_OF_STATIC_FUNCTION(x)   static void x##_end(void) { }

#ifndef LOCK_DATA
#define LOCK_DATA(d, s)             _go32_dpmi_lock_data(d, s)
#endif

#ifndef LOCK_CODE
#define LOCK_CODE(c, s)             _go32_dpmi_lock_code(c, s)
#endif

#ifndef UNLOCK_DATA
#define UNLOCK_DATA(d,s)            _unlock_dpmi_data(d, s)
#endif

#ifndef LOCK_VARIABLE
#define LOCK_VARIABLE(x)            LOCK_DATA((void *)&x, sizeof(x))
#endif

#ifndef LOCK_FUNCTION
#define LOCK_FUNCTION(x)            LOCK_CODE(x, (long)x##_end - (long)x)
#endif


/* describe the asm syntax for this platform */
#define DZCOMM_ASM_PREFIX    "_"
#define DZCOMM_ASM_USE_FS


/* arrange for other headers to be included later on */
#define DZCOMM_EXTRA_HEADER     "dzcomm/dzdos.h"
#define DZCOMM_INTERNAL_HEADER  "dzcomm/dintdos.h"
#define DZCOMM_MMX_HEADER       "obj/djgpp/mmx.h"

