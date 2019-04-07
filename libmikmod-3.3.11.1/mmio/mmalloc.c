/*	MikMod sound library
	(c) 1998, 1999 Miodrag Vallat and others - see file AUTHORS for
	complete list.

	This library is free software; you can redistribute it and/or modify
	it under the terms of the GNU Library General Public License as
	published by the Free Software Foundation; either version 2 of
	the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
	02111-1307, USA.
*/

/*==============================================================================

  $Id$

  Dynamic memory routines

==============================================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_POSIX_MEMALIGN
#define _XOPEN_SOURCE 600 /* for posix_memalign */
#endif

#include "string.h"
#include "mikmod_internals.h"

#if defined(HAVE_SSE2) || defined(HAVE_ALTIVEC)
#undef WIN32_ALIGNED_MALLOC
#if defined(_WIN32) && !defined(_WIN32_WCE)
# if defined(_WIN64) /* OK with MSVC and MinGW */
#  define WIN32_ALIGNED_MALLOC
# elif defined(_MSC_VER) && (_MSC_VER >= 1300)
#  define WIN32_ALIGNED_MALLOC
# elif defined(__MINGW32__)
  /* no guarantees that msvcrt.dll will have it */
# endif
#endif

#define PTRSIZE (sizeof(void*))

/* return a 16 byte aligned address */
void* MikMod_amalloc(size_t size)
{
	void *d;
#if defined(HAVE_POSIX_MEMALIGN)
	if (!posix_memalign(&d, 16, size)) {
		memset(d, 0, size);
		return d;
	}
#elif defined(WIN32_ALIGNED_MALLOC)
	d = _aligned_malloc(size, 16);
	if (d) {
		ZeroMemory(d, size);
		return d;
	}
#else
	size_t s = (size)? ((size + (PTRSIZE-1)) & ~(PTRSIZE-1)) : PTRSIZE;
	s += PTRSIZE + 16;
	d = calloc(1, s);
	if (d) {
		char *pptr = (char *)d + PTRSIZE;
		size_t err = ((size_t)pptr) & 15;
		char *fptr = pptr + (16 - err);
		*(size_t*)(fptr - PTRSIZE) = (size_t)d;
		return fptr;
	}
#endif

	_mm_errno = MMERR_OUT_OF_MEMORY;
	if(_mm_errorhandler) _mm_errorhandler();
	return NULL;
}

void MikMod_afree(void *data)
{
	if (!data) return;
#if defined(HAVE_POSIX_MEMALIGN)
	free(data);
#elif defined(WIN32_ALIGNED_MALLOC)
	_aligned_free(data);
#else
	free((void *) *(size_t*)((unsigned char *)data - PTRSIZE));
#endif
}
#endif /* (HAVE_SSE2) || (HAVE_ALTIVEC) */

void* MikMod_realloc(void *data, size_t size)
{
	if (data) return realloc(data, size);
	return calloc(1, size);
}

/* Same as malloc, but sets error variable _mm_error when fails */
void* MikMod_malloc(size_t size)
{
	return MikMod_calloc(1, size);
}

/* Same as calloc, but sets error variable _mm_error when fails */
void* MikMod_calloc(size_t nitems, size_t size)
{
	void *d = calloc(nitems, size);
	if (d) return d;

	_mm_errno = MMERR_OUT_OF_MEMORY;
	if(_mm_errorhandler) _mm_errorhandler();
	return NULL;
}

void MikMod_free(void *data)
{
	if (data) free(data);
}

/* like strdup(), but the result must be freed using MikMod_free() */
CHAR *MikMod_strdup(const CHAR *s)
{
	size_t l;
	CHAR *d;

	if (!s) return NULL;

	l = strlen(s) + 1;
	d = (CHAR *) MikMod_calloc(1, l * sizeof(CHAR));
	if (d) strcpy(d, s);
	return d;
}

/* ex:set ts=4: */
