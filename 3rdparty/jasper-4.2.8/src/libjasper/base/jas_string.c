/*
 * Copyright (c) 1999-2000 Image Power, Inc. and the University of
 *   British Columbia.
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

/*
 * String Library
 *
 * $Id$
 */

/******************************************************************************\
* Includes
\******************************************************************************/

#define JAS_FOR_INTERNAL_USE_ONLY

#include "jasper/jas_string.h"
#include "jasper/jas_malloc.h"
#include "jasper/jas_debug.h"

#include <string.h>

/******************************************************************************\
* Miscellaneous Functions
\******************************************************************************/

/* This function is equivalent to the popular but non-standard (and
  not-always-available) strdup function. */

char *jas_strdup(const char *s)
{
	size_t n;
	char *p;
	n = strlen(s) + 1;
	if (!(p = jas_malloc(n))) {
		return 0;
	}
	strcpy(p, s);
	return p;
}

char *jas_strtok(char *s, const char *delim, char **save_ptr)
{
#if 1
	char *result;
	char *end;
	if (!s) {
		s = *save_ptr;
	}
	if (*s != '\0') {
		s += strspn(s, delim);
		if (*s != '\0') {
			end = s + strcspn(s, delim);
			if (*end == '\0') {
				*save_ptr = end;
			} else {
				*end = '\0';
				*save_ptr = end + 1;
			}
			result = s;
		} else {
			*save_ptr = s;
			result = 0;
		}
	} else {
		*save_ptr = s;
		result = 0;
	}
	return result;
#else
	return strtok_r(str, delim, save_ptr);
#endif
}

JAS_EXPORT
int jas_stringtokenize(const char *string, const char *delim,
  char ***tokens_buf, size_t *max_tokens_buf, size_t *num_tokens_buf)
{
	char **tokens = 0;
	size_t max_tokens = 0;
	size_t num_tokens = 0;
	char **new_tokens;
	size_t new_max_tokens;
	char *buffer = 0;
	int ret = 0;
	char *token = 0;

	if (!(buffer = jas_strdup(string))) {
		ret = -1;
		goto done;
	}
	//new_max_tokens = 1;
	new_max_tokens = 0;
	if (new_max_tokens > 0) {
		if (!(tokens = jas_malloc(new_max_tokens * sizeof(char *)))) {
			ret = -1;
			goto done;
		}
		max_tokens = new_max_tokens;
	}

	bool first = true;
	char *saveptr = 0;
	for (;;) {
		char *cp;
		if (!(cp = jas_strtok(first ? buffer : 0, delim, &saveptr))) {
			break;
		}
		first = false;
		if (!(token = jas_strdup(cp))) {
			ret = -1;
			goto done;
		}
		if (num_tokens == max_tokens) {
			new_max_tokens = max_tokens ? 2 * max_tokens : 1;
			if (!(new_tokens = jas_realloc(tokens, new_max_tokens *
			  sizeof(char *)))) {
				ret = -1;
				goto done;
			}
			tokens = new_tokens;
			max_tokens = new_max_tokens;
		}
		assert(num_tokens < max_tokens);
		tokens[num_tokens] = token;
		token = 0;
		++num_tokens;
	}

done:
	if (buffer) {
		jas_free(buffer);
	}
	if (ret && tokens) {
		for (int i = 0; i < num_tokens; ++i) {
			jas_free(tokens[i]);
		}
		jas_free(tokens);
		tokens = 0;
		max_tokens = 0;
		num_tokens = 0;
	}
	if (token) {
		jas_free(token);
	}
	if (!ret) {
		*tokens_buf = tokens;
		*max_tokens_buf = max_tokens;
		*num_tokens_buf = num_tokens;
	}
	if (jas_get_debug_level() >= 100) {
		jas_eprintf("tokens %p; max_tokens %zu; num_tokens %zu\n",
		  JAS_CAST(void *, tokens), max_tokens, num_tokens);
		for (int i = 0; i < num_tokens; ++i) {
			jas_eprintf("[%d] = %s\n", i, tokens[i]);
		}
	}
	return ret;
}
