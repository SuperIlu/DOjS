/*
 * Copyright (c) 1999-2000 Image Power, Inc. and the University of
 *   British Columbia.
 * Copyright (c) 2001-2003 Michael David Adams.
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
 * Windows Bitmap File Library
 *
 * $Id$
 */

/******************************************************************************\
* Includes.
\******************************************************************************/

#include "jasper/jas_init.h"
#include "jasper/jas_image.h"
#include "jasper/jas_types.h"
#include "jasper/jas_stream.h"
#include "jasper/jas_malloc.h"
#include "jasper/jas_math.h"
#include "jasper/jas_debug.h"
#include "jasper/jas_tvp.h"

#include "bmp_cod.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/******************************************************************************\
* Local types.
\******************************************************************************/

typedef struct {
	size_t max_samples;
} bmp_dec_importopts_t;

typedef enum {
	OPT_MAXSIZE,
} optid_t;

/******************************************************************************\
* Local prototypes.
\******************************************************************************/

static int bmp_gethdr(jas_stream_t *in, bmp_hdr_t *hdr);
static bmp_info_t *bmp_getinfo(jas_stream_t *in, const bmp_dec_importopts_t *opts);
static int bmp_getdata(jas_stream_t *in, bmp_info_t *info, jas_image_t *image);
static int bmp_getint16(jas_stream_t *in, int_fast16_t *val);
static int bmp_getint32(jas_stream_t *in, int_fast32_t *val);
static int bmp_gobble(jas_stream_t *in, long n);

/******************************************************************************\
* Option parsing.
\******************************************************************************/

static const jas_taginfo_t decopts[] = {
	{OPT_MAXSIZE, "max_samples"},
	{-1, 0}
};

static int bmp_dec_parseopts(const char *optstr, bmp_dec_importopts_t *opts)
{
	jas_tvparser_t *tvp;

	opts->max_samples = jas_get_dec_default_max_samples();

	if (!(tvp = jas_tvparser_create(optstr ? optstr : ""))) {
		return -1;
	}

	while (!jas_tvparser_next(tvp)) {
		switch (jas_taginfo_nonull(jas_taginfos_lookup(decopts,
		  jas_tvparser_gettag(tvp)))->id) {
		case OPT_MAXSIZE:
			opts->max_samples = strtoull(jas_tvparser_getval(tvp), 0, 10);
			break;
		default:
			jas_logwarnf("warning: ignoring invalid option %s\n",
			  jas_tvparser_gettag(tvp));
			break;
		}
	}

	jas_tvparser_destroy(tvp);

	return 0;
}

/******************************************************************************\
* Interface functions.
\******************************************************************************/

jas_image_t *bmp_decode(jas_stream_t *in, const char *optstr)
{
	jas_image_t *image;
	bmp_hdr_t hdr;
	bmp_info_t *info;
	uint_fast16_t cmptno;
	jas_image_cmptparm_t cmptparms[3];
	jas_image_cmptparm_t *cmptparm;
	uint_fast16_t numcmpts;
	long n;
	bmp_dec_importopts_t opts;
	size_t num_samples;

	image = 0;
	info = 0;

	if (bmp_dec_parseopts(optstr, &opts)) {
		goto error;
	}

	jas_logwarnf(
	  "THE BMP FORMAT IS NOT FULLY SUPPORTED!\n"
	  "THAT IS, THE JASPER SOFTWARE CANNOT DECODE ALL TYPES OF BMP DATA.\n"
	  "IF YOU HAVE ANY PROBLEMS, PLEASE TRY CONVERTING YOUR IMAGE DATA\n"
	  "TO THE PNM FORMAT, AND USING THIS FORMAT INSTEAD.\n"
	  );

	/* Read the bitmap header. */
	if (bmp_gethdr(in, &hdr)) {
		jas_logerrorf("cannot get header\n");
		goto error;
	}
	JAS_LOGDEBUGF(1,
	  "BMP header: magic 0x%x; siz %d; res1 %d; res2 %d; off %d\n",
	  hdr.magic, hdr.siz, hdr.reserved1, hdr.reserved2, hdr.off
	  );

	/* Read the bitmap information. */
	if (!(info = bmp_getinfo(in, &opts))) {
		jas_logerrorf("cannot get info\n");
		goto error;
	}
	JAS_LOGDEBUGF(1,
	  "BMP information: len %ld; width %ld; height %ld; numplanes %d; "
	  "depth %d; enctype %ld; siz %ld; hres %ld; vres %ld; numcolors %ld; "
	  "mincolors %ld\n", JAS_CAST(long, info->len),
	  JAS_CAST(long, info->width), JAS_CAST(long, info->height),
	  JAS_CAST(long, info->numplanes), JAS_CAST(long, info->depth),
	  JAS_CAST(long, info->enctype), JAS_CAST(long, info->siz),
	  JAS_CAST(long, info->hres), JAS_CAST(long, info->vres),
	  JAS_CAST(long, info->numcolors), JAS_CAST(long, info->mincolors));

	if (info->width < 0 || info->height < 0 || info->numplanes < 0 ||
	  info->depth < 0 || info->siz < 0 || info->hres < 0 || info->vres < 0) {
		jas_logerrorf("corrupt bit stream\n");
		goto error;
	}

	if (!jas_safe_size_mul3(info->width, info->height, info->numplanes,
	  &num_samples)) {
		jas_logerrorf("image size too large\n");
		goto error;
	}

	if (opts.max_samples > 0 && num_samples > opts.max_samples) {
		jas_logerrorf("maximum number of pixels exceeded (%zu)\n",
		  opts.max_samples);
		goto error;
	}

	/* Ensure that we support this type of BMP file. */
	if (!bmp_issupported(&hdr, info)) {
		jas_logerrorf("error: unsupported BMP encoding\n");
		goto error;
	}

	/* Skip over any useless data between the end of the palette
	  and start of the bitmap data. */
	if ((n = hdr.off - (BMP_HDRLEN + BMP_INFOLEN + BMP_PALLEN(info))) < 0) {
		jas_logerrorf("error: possibly bad bitmap offset?\n");
		goto error;
	}
	if (n > 0) {
		jas_logwarnf("skipping unknown data in BMP file\n");
		if (bmp_gobble(in, n)) {
			goto error;
		}
	}

	/* Get the number of components. */
	if ((numcmpts = bmp_numcmpts(info)) < 0) {
		jas_logerrorf("error: cannot determine number of components\n");
		goto error;
	}

	for (cmptno = 0, cmptparm = cmptparms; cmptno < numcmpts; ++cmptno,
	  ++cmptparm) {
		cmptparm->tlx = 0;
		cmptparm->tly = 0;
		cmptparm->hstep = 1;
		cmptparm->vstep = 1;
		cmptparm->width = info->width;
		cmptparm->height = info->height;
		cmptparm->prec = 8;
		cmptparm->sgnd = false;
	}

	/* Create image object. */
	if (!(image = jas_image_create(numcmpts, cmptparms,
	  JAS_CLRSPC_UNKNOWN))) {
		goto error;
	}

	if (numcmpts == 3) {
		jas_image_setclrspc(image, JAS_CLRSPC_SRGB);
		jas_image_setcmpttype(image, 0,
		  JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R));
		jas_image_setcmpttype(image, 1,
		  JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G));
		jas_image_setcmpttype(image, 2,
		  JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B));
	} else {
		jas_image_setclrspc(image, JAS_CLRSPC_SGRAY);
		jas_image_setcmpttype(image, 0,
		  JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_GRAY_Y));
	}

	/* Read the bitmap data. */
	if (bmp_getdata(in, info, image)) {
		goto error;
	}

	bmp_info_destroy(info);

	return image;

error:
	if (info) {
		bmp_info_destroy(info);
	}
	if (image) {
		jas_image_destroy(image);
	}
	return 0;
}

int bmp_validate(jas_stream_t *in)
{
	jas_uchar buf[2];

	assert(JAS_STREAM_MAXPUTBACK >= 2);

	/* Read the first two characters that constitute the signature. */
	if (jas_stream_peek(in, buf, sizeof(buf)) != sizeof(buf)) {
		return -1;
	}

	unsigned magic = (buf[0] | (buf[1] << 8));

	/* Is the signature correct for the BMP format? */
	if (magic != BMP_MAGIC) {
		JAS_LOGDEBUGF(20, "bad signature (0x%04lx != 0x%04lx)\n",
		  JAS_CAST(unsigned long, magic),
		  JAS_CAST(unsigned long, BMP_MAGIC));
		return -1;
	}
	return 0;
}

/******************************************************************************\
* Code for aggregate types.
\******************************************************************************/

static int bmp_gethdr(jas_stream_t *in, bmp_hdr_t *hdr)
{
	if (bmp_getint16(in, &hdr->magic) || hdr->magic != BMP_MAGIC ||
	  bmp_getint32(in, &hdr->siz) || bmp_getint16(in, &hdr->reserved1) ||
	  bmp_getint16(in, &hdr->reserved2) || bmp_getint32(in, &hdr->off)) {
		return -1;
	}
	return 0;
}

static bmp_info_t *bmp_getinfo(jas_stream_t *in, const bmp_dec_importopts_t *opts)
{
	bmp_info_t *info;
	int i;
	bmp_palent_t *palent;
	size_t num_pixels;

	info = 0;

	if (!(info = bmp_info_create())) {
		goto error;
		return 0;
	}

	if (bmp_getint32(in, &info->len) || info->len != 40 ||
	  bmp_getint32(in, &info->width) || bmp_getint32(in, &info->height) ||
	  bmp_getint16(in, &info->numplanes) ||
	  bmp_getint16(in, &info->depth) || bmp_getint32(in, &info->enctype) ||
	  bmp_getint32(in, &info->siz) ||
	  bmp_getint32(in, &info->hres) || bmp_getint32(in, &info->vres) ||
	  bmp_getint32(in, &info->numcolors) ||
	  bmp_getint32(in, &info->mincolors)) {
		goto error;
	}

	if (info->height < 0) {
		info->topdown = 1;
		info->height = -info->height;
	} else {
		info->topdown = 0;
	}

	if (info->width <= 0 || info->height <= 0 || info->numplanes <= 0 ||
	  info->depth <= 0  || info->numcolors < 0 || info->mincolors < 0) {
		goto error;
	}

	if (info->depth != 8 && info->depth != 24) {
		jas_logerrorf(
		  "BMP decoder only supports images with depth 8 or 24 "
		  "(depth %d)\n", info->depth);
		goto error;
	}

	if (!jas_safe_size_mul(info->width, info->height, &num_pixels) ||
	    (opts->max_samples > 0 && num_pixels > opts->max_samples)) {
		jas_logerrorf("image dimensions too large\n");
		goto error;
	}

	if (info->enctype != BMP_ENC_RGB) {
		jas_logerrorf("unsupported BMP encoding\n");
		goto error;
	}

	/* Check for a palette whose size is unreasonably large. */
	if ((uint_fast32_t)info->numcolors > 256 && (uint_fast32_t)info->numcolors > num_pixels) {
		jas_logerrorf("palette size is greater than 256 and "
		  "greater than the number of pixels "
		  "(%zu > %zu)\n",
		  (uint_fast32_t)info->numcolors > num_pixels);
		goto error;
	}

	if (info->numcolors > 0) {
		if (!(info->palents = jas_alloc2(info->numcolors,
		  sizeof(bmp_palent_t)))) {
			goto error;
		}
	} else {
		info->palents = 0;
	}

	for (i = 0; i < info->numcolors; ++i) {
		palent = &info->palents[i];
		if ((palent->blu = jas_stream_getc(in)) == EOF ||
		  (palent->grn = jas_stream_getc(in)) == EOF ||
		  (palent->red = jas_stream_getc(in)) == EOF ||
		  (palent->res = jas_stream_getc(in)) == EOF) {
			goto error;
		}
	}

	return info;

error:
	if (info) {
		bmp_info_destroy(info);
	}
	return 0;
}

static int bmp_getdata(jas_stream_t *in, bmp_info_t *info, jas_image_t *image)
{
	int i;
	int j;
	int y;
	jas_matrix_t *cmpts[3];
	int numpad;
	int red;
	int grn;
	int blu;
	int ret;
	int numcmpts;
	int cmptno;
	int ind;
	bmp_palent_t *palent;
	int mxind;
	int haspal;

	assert(info->depth == 8 || info->depth == 24);
	assert(info->enctype == BMP_ENC_RGB);

	numcmpts = bmp_numcmpts(info);
	haspal = bmp_haspal(info);

	ret = 0;
	for (i = 0; i < numcmpts; ++i) {
		cmpts[i] = 0;
	}

	/* Create temporary matrices to hold component data. */
	for (i = 0; i < numcmpts; ++i) {
		if (!(cmpts[i] = jas_matrix_create(1, info->width))) {
			ret = -1;
			goto bmp_getdata_done;
		}
	}

	/* Calculate number of padding bytes per row of image data. */
	numpad = (numcmpts * info->width) % 4;
	if (numpad) {
		numpad = 4 - numpad;
	}

	mxind = (1 << info->depth) - 1;
	for (i = 0; i < info->height; ++i) {
		for (j = 0; j < info->width; ++j) {
			if (haspal) {
				if ((ind = jas_stream_getc(in)) == EOF) {
					ret = -1;
					goto bmp_getdata_done;
				}
				if (ind > mxind) {
					ret = -1;
					goto bmp_getdata_done;
				}
				if (ind < info->numcolors) {
					palent = &info->palents[ind];
					red = palent->red;
					grn = palent->grn;
					blu = palent->blu;
				} else {
					red = ind;
					grn = ind;
					blu = ind;
				}
			} else {
				if ((blu = jas_stream_getc(in)) == EOF ||
				  (grn = jas_stream_getc(in)) == EOF ||
				  (red = jas_stream_getc(in)) == EOF) {
					ret = -1;
					goto bmp_getdata_done;
				}
			}
			if (numcmpts == 3) {
				jas_matrix_setv(cmpts[0], j, red);
				jas_matrix_setv(cmpts[1], j, grn);
				jas_matrix_setv(cmpts[2], j, blu);
			} else {
				jas_matrix_setv(cmpts[0], j, red);
			}
		}
		for (j = numpad; j > 0; --j) {
				if (jas_stream_getc(in) == EOF) {
					ret = -1;
					goto bmp_getdata_done;
				}
		}
		for (cmptno = 0; cmptno < numcmpts; ++cmptno) {
			y = info->topdown ? i : (info->height - 1 - i);
			if (jas_image_writecmpt(image, cmptno, 0, y, info->width,
			  1, cmpts[cmptno])) {
				ret = -1;
				goto bmp_getdata_done;
			}
		}
	}

bmp_getdata_done:
	/* Destroy the temporary matrices. */
	for (i = 0; i < numcmpts; ++i) {
		if (cmpts[i]) {
			jas_matrix_destroy(cmpts[i]);
		}
	}

	return ret;
}

/******************************************************************************\
* Code for primitive types.
\******************************************************************************/

static int bmp_getint16(jas_stream_t *in, int_fast16_t *val)
{
	int lo;
	int hi;
	if ((lo = jas_stream_getc(in)) == EOF || (hi = jas_stream_getc(in)) == EOF) {
		return -1;
	}
	if (val) {
		*val = (hi << 8) | lo;
	}
	return 0;
}

static int bmp_getint32(jas_stream_t *in, int_fast32_t *val)
{
	int n;
	uint_fast32_t v;
	int c;
	for (n = 4, v = 0;;) {
		if ((c = jas_stream_getc(in)) == EOF) {
			return -1;
		}
		v |= (JAS_CAST(uint_fast32_t, c) << 24);
		if (--n <= 0) {
			break;
		}
		v >>= 8;
	}
	if (val) {
		*val = v;
	}
	return 0;
}

static int bmp_gobble(jas_stream_t *in, long n)
{
	while (--n >= 0) {
		if (jas_stream_getc(in) == EOF) {
			return -1;
		}
	}
	return 0;
}
