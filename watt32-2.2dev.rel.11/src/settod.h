/*!\file settod.h
 *
 *  settimeofday() for non-djgpp targets.
 *
 *  The exported prototype used is the one specified in the
 *  XOpen/POSIX 1.3 standards and the one used on modern (ie 4.4BSD spec)
 *  BSDs. ie 'int settimeofday (struct timeval *, ...)', i.e. the second
 *  arg, if specified, is ignored.
 */

#ifndef _w32_SETTIMEOFDAY_H
#define _w32_SETTIMEOFDAY_H

#if !defined(__DJGPP__) && !defined(__CYGWIN__)

extern int MS_CDECL settimeofday (struct timeval *tv, ...);

#endif
#endif
