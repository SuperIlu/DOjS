/*
 * DZcomm : a serial port API.
 * file : dintunix.h
 *
 * Some definitions for internal use by the Unix library code.
 *
 * By Neil Townsend
 * 
 * See readme.txt for copyright information.
 */

#ifndef DINTUNIX_H
#define DINTUNIX_H

/* macros to enable and disable interrupts */
#define DISABLE() _sigalrm_disable_interrupts()
#define ENABLE()  _sigalrm_enable_interrupts()

#include <termios.h>

#ifdef DZCOMM_LINUX
   #include "dintlnx.h"
#endif

#endif /* ifndef DINTUNIX_H */

