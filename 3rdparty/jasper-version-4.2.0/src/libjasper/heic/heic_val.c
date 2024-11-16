/*
 * Copyright (c) 2021 Michael David Adams.
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

#include "jasper/jas_stream.h"
#include "jasper/jas_types.h"
#include "jasper/jas_image.h"

#include <stdio.h>
#include <assert.h>

/******************************************************************************\
* Code for validate operation.
\******************************************************************************/

#define	HEIF_VALIDATELEN	(JAS_MIN(4 + 4 + 8, JAS_STREAM_MAXPUTBACK))
#define	HEIF_FTYP 0x66747970U
#define	HEIC_MIF1 0x6d696631U
#define	HEIC_MSF1 0x6d736631U
#define	HEIC_HEIC 0x68656963U
#define	HEIC_HEIX 0x68656978U
#define	HEIC_HEVC 0x68657663U
#define	HEIC_HEVX 0x68657678U

int jas_heic_validate(jas_stream_t *in)
{
	unsigned char buf[HEIF_VALIDATELEN];

	assert(JAS_STREAM_MAXPUTBACK >= HEIF_VALIDATELEN);

	/* Read the validation data (i.e., the data used for detecting
	  the format). */
	if (jas_stream_peek(in, buf, sizeof(buf)) != sizeof(buf)) {
		return -1;
	}

	/* Is the box type correct? */
	uint_least32_t box_type = (JAS_CAST(uint_least32_t, buf[4]) << 24) |
	  (JAS_CAST(uint_least32_t, buf[5]) << 16) |
	  (JAS_CAST(uint_least32_t, buf[6]) << 8) |
	  JAS_CAST(uint_least32_t, buf[7]);
	if (box_type != HEIF_FTYP) {
		return -1;
	}

	/* Is the signature correct? */
	uint_least32_t sig = (JAS_CAST(uint_least32_t, buf[8]) << 24) |
	  (JAS_CAST(uint_least32_t, buf[9]) << 16) |
	  (JAS_CAST(uint_least32_t, buf[10]) << 8) |
	  JAS_CAST(uint_least32_t, buf[11]);
	if (sig != HEIC_MIF1 &&
	  sig != HEIC_MSF1 &&
	  sig != HEIC_HEIC &&
	  sig != HEIC_HEIX &&
	  sig != HEIC_HEVC &&
	  sig != HEIC_HEVX) {
		return -1;
	}

	return 0;
}
