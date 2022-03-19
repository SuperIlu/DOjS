/*
 * DZcomm : a serial port API.
 * file : inline.c
 *
 * Copies of the inline functions in allegro.h, in case anyone needs
 * to take the address of them, or is compiling without optimisation.
 * 
 * v1.0 From Shawn Hargreaves
 *
 * See readme.txt for copyright information.
 */

#define DZ_INLINE(type, name, args, code)    type name args code

/* This #define tells dzcomm.h that this is source file for the library and
 * not to include things the are only for the users code.
 */
#define DZCOMM_LIB_SRC_FILE

#include "dzcomm.h"
#include "dzcomm/dzintern.h"

#ifdef DZCOMM_INTERNAL_HEADER
   #include DZCOMM_INTERNAL_HEADER
#endif

