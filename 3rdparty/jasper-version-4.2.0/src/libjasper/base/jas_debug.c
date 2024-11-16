/*
 * Copyright (c) 2001-2002 Michael David Adams.
 * All rights reserved.
 */

/* __START_OF_JASPER_LICENSE__
 * 
 * JasPer License Version 2.0
 * 
 * Copyright (c) 2001-2006 Michael David Adams
 * Copyright (c) 1999-2000 Image Power, Inc.
 * Copyright (c) 1999-2000 The University of British Columbia
 * 
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person (the
 * "User") obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 * 
 * 1.  The above copyright notices and this permission notice (which
 * includes the disclaimer below) shall be included in all copies or
 * substantial portions of the Software.
 * 
 * 2.  The name of a copyright holder shall not be used to endorse or
 * promote products derived from the Software without specific prior
 * written permission.
 * 
 * THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL PART OF THIS
 * LICENSE.  NO USE OF THE SOFTWARE IS AUTHORIZED HEREUNDER EXCEPT UNDER
 * THIS DISCLAIMER.  THE SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.  IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
 * INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.  NO ASSURANCES ARE
 * PROVIDED BY THE COPYRIGHT HOLDERS THAT THE SOFTWARE DOES NOT INFRINGE
 * THE PATENT OR OTHER INTELLECTUAL PROPERTY RIGHTS OF ANY OTHER ENTITY.
 * EACH COPYRIGHT HOLDER DISCLAIMS ANY LIABILITY TO THE USER FOR CLAIMS
 * BROUGHT BY ANY OTHER ENTITY BASED ON INFRINGEMENT OF INTELLECTUAL
 * PROPERTY RIGHTS OR OTHERWISE.  AS A CONDITION TO EXERCISING THE RIGHTS
 * GRANTED HEREUNDER, EACH USER HEREBY ASSUMES SOLE RESPONSIBILITY TO SECURE
 * ANY OTHER INTELLECTUAL PROPERTY RIGHTS NEEDED, IF ANY.  THE SOFTWARE
 * IS NOT FAULT-TOLERANT AND IS NOT INTENDED FOR USE IN MISSION-CRITICAL
 * SYSTEMS, SUCH AS THOSE USED IN THE OPERATION OF NUCLEAR FACILITIES,
 * AIRCRAFT NAVIGATION OR COMMUNICATION SYSTEMS, AIR TRAFFIC CONTROL
 * SYSTEMS, DIRECT LIFE SUPPORT MACHINES, OR WEAPONS SYSTEMS, IN WHICH
 * THE FAILURE OF THE SOFTWARE OR SYSTEM COULD LEAD DIRECTLY TO DEATH,
 * PERSONAL INJURY, OR SEVERE PHYSICAL OR ENVIRONMENTAL DAMAGE ("HIGH
 * RISK ACTIVITIES").  THE COPYRIGHT HOLDERS SPECIFICALLY DISCLAIM ANY
 * EXPRESS OR IMPLIED WARRANTY OF FITNESS FOR HIGH RISK ACTIVITIES.
 * 
 * __END_OF_JASPER_LICENSE__
 */

/******************************************************************************\
* Includes.
\******************************************************************************/

#define JAS_FOR_INTERNAL_USE_ONLY

#include "jasper/jas_init.h"
#include "jasper/jas_compiler.h"
#include "jasper/jas_debug.h"
#include "jasper/jas_types.h"
#include "jasper/jas_log.h"

#include <stdarg.h>
#include <stdio.h>

/******************************************************************************\
* Code for getting/setting the debug level.
\******************************************************************************/

/* Set the library debug level. */
JAS_DEPRECATED
int jas_setdbglevel(int level)
{
	jas_deprecated("jas_setdbglevel is deprecated\n");
	int old_level = jas_get_debug_level();
	jas_set_debug_level(level);
	return old_level;
}

/******************************************************************************\
* Code.
\******************************************************************************/

/* Perform formatted output to standard error. */
int jas_eprintf(const char *fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vfprintf(stderr, fmt, ap);
	va_end(ap);
	return ret;
}

/* Generate formatted error log message. */
int jas_logprintf(const char *fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = jas_vlogmsgf(jas_logtype_init(JAS_LOGTYPE_CLASS_NULL, 0), fmt,
	  ap);
	va_end(ap);
	return ret;
}

/* Generate formatted error log message. */
int jas_logerrorf(const char *fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = jas_vlogmsgf(jas_logtype_init(JAS_LOGTYPE_CLASS_ERROR, 0), fmt,
	  ap);
	va_end(ap);
	return ret;
}

/* Generate formatted warning log message. */
int jas_logwarnf(const char *fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = jas_vlogmsgf(jas_logtype_init(JAS_LOGTYPE_CLASS_WARN, 0), fmt,
	  ap);
	va_end(ap);
	return ret;
}

/* Generate formatted informational log message. */
int jas_loginfof(const char *fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = jas_vlogmsgf(jas_logtype_init(JAS_LOGTYPE_CLASS_INFO, 0), fmt,
	  ap);
	va_end(ap);
	return ret;
}

/* Generate formatted debugging log message. */
int jas_logdebugf(int priority, const char *fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = jas_vlogmsgf(jas_logtype_init(JAS_LOGTYPE_CLASS_DEBUG, priority),
	  fmt, ap);
	va_end(ap);
	return ret;
}

/*!
@brief
@details
*/
JAS_EXPORT
int jas_vlogmsgf(jas_logtype_t type, const char *fmt, va_list ap)
{
	int ret;
	jas_vlogmsgf_t *func = jas_get_vlogmsgf();
	ret = func(type, fmt, ap);
	return ret;
}

/* Perform formatted output to standard error. */
JAS_EXPORT
int jas_vlogmsgf_stderr(jas_logtype_t type, const char *fmt, va_list ap)
{
#if 1
	JAS_UNUSED(type);
	int result = vfprintf(stderr, fmt, ap);
	return result;
#else
	const char *s = "INVALID";
	switch (jas_logtype_getclass(type)) {
	case JAS_LOGTYPE_CLASS_NULL:
		s = "OTHER";
		break;
	case JAS_LOGTYPE_CLASS_ERROR:
		s = "ERROR";
		break;
	case JAS_LOGTYPE_CLASS_WARN:
		s = "WARNING";
		break;
	case JAS_LOGTYPE_CLASS_INFO:
		s = "INFO";
		break;
	case JAS_LOGTYPE_CLASS_DEBUG:
		s = "DEBUG";
		break;
	}
	int r1 = fprintf(stderr, "%s: ", s);
	int r2 = vfprintf(stderr, fmt, ap);
	int result = -1;
	if (r1 > 0 && r2 > 0) {
		result = r1 + r2;
	}
	return result;
#endif
}

/* Perform formatted output to standard error. */
JAS_EXPORT
int jas_vlogmsgf_discard(jas_logtype_t type, const char *fmt, va_list ap)
{
	JAS_UNUSED(type);
	JAS_CAST(void, fmt);
	JAS_CAST(void, ap);
	return 0;
}

/* Dump memory to a stream. */
int jas_memdump(FILE *out, const void *data, size_t len)
{
	size_t i;
	size_t j;
	const jas_uchar *dp = data;
	for (i = 0; i < len; i += 16) {
		fprintf(out, "%04zx:", i);
		for (j = 0; j < 16; ++j) {
			if (i + j < len) {
				fprintf(out, " %02x", dp[i + j]);
			}
		}
		fprintf(out, "\n");
	}
	return 0;
}

/* Dump memory to a stream. */
int jas_logmemdump(const void *data, size_t len)
{
	size_t i;
	size_t j;
	const jas_uchar *dp = data;
	for (i = 0; i < len; i += 16) {
		jas_logprintf("%04zx:", i);
		for (j = 0; j < 16; ++j) {
			if (i + j < len) {
				jas_logprintf(" %02x", dp[i + j]);
			}
		}
		jas_logprintf("\n");
	}
	return 0;
}

/******************************************************************************\
* Code.
\******************************************************************************/

void jas_deprecated(const char *fmt, ...)
{
	static const char message[] =
	"WARNING: YOUR CODE IS RELYING ON DEPRECATED FUNCTIONALITY IN THE JASPER\n"
	"LIBRARY.  THIS FUNCTIONALITY WILL BE REMOVED IN THE NEAR FUTURE. PLEASE\n"
	"FIX THIS PROBLEM BEFORE YOUR CODE STOPS WORKING.\n"
	;
	fprintf(stderr, "%s", message);
	fprintf(stderr, "deprecation warning: ");
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}
