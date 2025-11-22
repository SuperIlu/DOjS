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
 * Sun Rasterfile Library
 *
 * $Id$
 */

/******************************************************************************\
* Includes.
\******************************************************************************/

#include "ras_cod.h"

#include "jasper/jas_init.h"
#include "jasper/jas_stream.h"
#include "jasper/jas_image.h"
#include "jasper/jas_debug.h"
#include "jasper/jas_tvp.h"
#include "jasper/jas_math.h"

#include <assert.h>
#include <stdlib.h>

/******************************************************************************\
* Local types.
\******************************************************************************/

typedef struct {
	int allow_trunc;
	size_t max_samples;
} ras_dec_importopts_t;

typedef enum {
	OPT_ALLOWTRUNC,
	OPT_MAXSIZE,
} optid_t;

/******************************************************************************\
* Prototypes.
\******************************************************************************/

static int ras_gethdr(jas_stream_t *in, ras_hdr_t *hdr);
static int ras_getint(jas_stream_t *in, int_fast32_t *val);

static int ras_getdata(jas_stream_t *in, ras_hdr_t *hdr, ras_cmap_t *cmap,
  jas_image_t *image);
static int ras_getdatastd(jas_stream_t *in, ras_hdr_t *hdr, ras_cmap_t *cmap,
  jas_image_t *image);
static int ras_getcmap(jas_stream_t *in, ras_hdr_t *hdr, ras_cmap_t *cmap);

/******************************************************************************\
* Option parsing.
\******************************************************************************/

static const jas_taginfo_t ras_decopts[] = {
	// Not yet supported
	// {OPT_ALLOWTRUNC, "allow_trunc"},
	{OPT_MAXSIZE, "max_samples"},
	{-1, 0}
};

static int ras_dec_parseopts(const char *optstr, ras_dec_importopts_t *opts)
{
	jas_tvparser_t *tvp;

	opts->max_samples = jas_get_dec_default_max_samples();
	opts->allow_trunc = 0;

	if (!(tvp = jas_tvparser_create(optstr ? optstr : ""))) {
		return -1;
	}

	while (!jas_tvparser_next(tvp)) {
		switch (jas_taginfo_nonull(jas_taginfos_lookup(ras_decopts,
		  jas_tvparser_gettag(tvp)))->id) {
		case OPT_ALLOWTRUNC:
			opts->allow_trunc = atoi(jas_tvparser_getval(tvp));
			break;
		case OPT_MAXSIZE:
			opts->max_samples = strtoull(jas_tvparser_getval(tvp), 0, 10);
			break;
		default:
			jas_logwarnf("warning: ignoring invalid option %s\n",
			  jas_tvparser_gettag(tvp));
		}
	}

	jas_tvparser_destroy(tvp);

	return 0;
}

/******************************************************************************\
* Code.
\******************************************************************************/

jas_image_t *ras_decode(jas_stream_t *in, const char *optstr)
{
	ras_hdr_t hdr;
	ras_cmap_t cmap;
	jas_image_t *image;
	jas_image_cmptparm_t cmptparms[3];
	jas_image_cmptparm_t *cmptparm;
	int clrspc;
	int numcmpts;
	int i;
	ras_dec_importopts_t opts;
	size_t num_samples;

	image = 0;

	JAS_LOGDEBUGF(10, "ras_decode(%p, \"%s\")\n", in, (optstr ? optstr : ""));

	if (ras_dec_parseopts(optstr, &opts)) {
		goto error;
	}

	/* Read the header. */
	if (ras_gethdr(in, &hdr)) {
		goto error;
	}

	/* Does the header information look reasonably sane? */
	if (hdr.magic != RAS_MAGIC || hdr.width <= 0 || hdr.height <= 0 ||
	  hdr.depth <= 0 || hdr.depth > 32) {
		goto error;
	}

	if (!jas_safe_size_mul3(hdr.width, hdr.height, (hdr.depth + 7) / 8,
	  &num_samples)) {
		jas_logerrorf("image too large\n");
		goto error;
	}
	if (opts.max_samples > 0 && num_samples > opts.max_samples) {
		jas_logerrorf(
		  "maximum number of samples would be exceeded (%zu > %zu)\n",
		  num_samples, opts.max_samples);
		goto error;
	}

	/* In the case of the old format, do not rely on the length field
	being properly specified.  Calculate the quantity from scratch. */
	if (hdr.type == RAS_TYPE_OLD) {
		hdr.length = RAS_ROWSIZE(&hdr) * hdr.height;
	}

	/* Calculate some quantities needed for creation of the image
	object. */
	if (RAS_ISRGB(&hdr)) {
		clrspc = JAS_CLRSPC_SRGB;
		numcmpts = 3;
	} else {
		clrspc = JAS_CLRSPC_SGRAY;
		numcmpts = 1;
	}
	for (i = 0, cmptparm = cmptparms; i < numcmpts; ++i, ++cmptparm) {
		cmptparm->tlx = 0;
		cmptparm->tly = 0;
		cmptparm->hstep = 1;
		cmptparm->vstep = 1;
		cmptparm->width = hdr.width;
		cmptparm->height = hdr.height;
		cmptparm->prec = RAS_ISRGB(&hdr) ? 8 : hdr.depth;
		cmptparm->sgnd = false;
	}
	/* Create the image object. */
	if (!(image = jas_image_create(numcmpts, cmptparms, JAS_CLRSPC_UNKNOWN))) {
		goto error;
	}

	/* Read the color map (if there is one). */
	if (ras_getcmap(in, &hdr, &cmap)) {
		goto error;
	}

	/* Read the pixel data. */
	if (ras_getdata(in, &hdr, &cmap, image)) {
		goto error;
	}

	jas_image_setclrspc(image, clrspc);
	if (clrspc == JAS_CLRSPC_SRGB) {
		jas_image_setcmpttype(image, 0,
		  JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R));
		jas_image_setcmpttype(image, 1,
		  JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G));
		jas_image_setcmpttype(image, 2,
		  JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B));
	} else {
		jas_image_setcmpttype(image, 0,
		  JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_GRAY_Y));
	}

	return image;

error:
	if (image) {
		jas_image_destroy(image);
	}
	return 0;
}

int ras_validate(jas_stream_t *in)
{
	jas_uchar buf[RAS_MAGICLEN];
	uint_fast32_t magic;

	assert(JAS_STREAM_MAXPUTBACK >= RAS_MAGICLEN);

	/* Read the validation data (i.e., the data used for detecting
	  the format). */
	if (jas_stream_peek(in, buf, sizeof(buf)) != sizeof(buf)) {
		return -1;
	}

	magic = (JAS_CAST(uint_fast32_t, buf[0]) << 24) |
	  (JAS_CAST(uint_fast32_t, buf[1]) << 16) |
	  (JAS_CAST(uint_fast32_t, buf[2]) << 8) |
	  buf[3];

	/* Is the signature correct for the Sun Rasterfile format? */
	if (magic != RAS_MAGIC) {
		JAS_LOGDEBUGF(20, "bad signature (0x%08lx != 0x%08lx)\n",
		  JAS_CAST(unsigned long, magic),
		  JAS_CAST(unsigned long, RAS_MAGIC));
		return -1;
	}
	return 0;
}

static int ras_getdata(jas_stream_t *in, ras_hdr_t *hdr, ras_cmap_t *cmap,
  jas_image_t *image)
{
	int ret;

	switch (hdr->type) {
	case RAS_TYPE_OLD:
	case RAS_TYPE_STD:
		ret = ras_getdatastd(in, hdr, cmap, image);
		break;
	case RAS_TYPE_RLE:
		jas_logerrorf("error: RLE encoding method not supported\n");
		ret = -1;
		break;
	default:
		jas_logerrorf("error: encoding method not supported\n");
		ret = -1;
	}
	return ret;
}

static int ras_getdatastd(jas_stream_t *in, ras_hdr_t *hdr, ras_cmap_t *cmap,
  jas_image_t *image)
{
	int pad;
	int nz;
	int z;
	int c;
	int y;
	int x;
	int v;
	jas_matrix_t *data[3];

	/* Note: This function does not properly handle images with a colormap. */

	JAS_UNUSED(cmap);

	assert(jas_image_numcmpts(image) <= 3);

	for (unsigned i = 0; i < 3; ++i) {
		data[i] = 0;
	}

	for (unsigned i = 0; i < jas_image_numcmpts(image); ++i) {
		if (!(data[i] = jas_matrix_create(1, jas_image_width(image)))) {
			goto error;
		}
	}

	pad = RAS_ROWSIZE(hdr) - (hdr->width * hdr->depth + 7) / 8;

	for (y = 0; y < hdr->height; y++) {
		nz = 0;
		z = 0;
		for (x = 0; x < hdr->width; x++) {
			while (nz < hdr->depth) {
				if ((c = jas_stream_getc(in)) == EOF) {
					goto error;
				}
				z = (z << 8) | c;
				nz += 8;
			}

			v = (z >> (nz - hdr->depth)) & RAS_ONES(hdr->depth);
			z &= RAS_ONES(nz - hdr->depth);
			nz -= hdr->depth;

			if (jas_image_numcmpts(image) == 3) {
				jas_matrix_setv(data[0], x, (RAS_GETRED(v)));
				jas_matrix_setv(data[1], x, (RAS_GETGREEN(v)));
				jas_matrix_setv(data[2], x, (RAS_GETBLUE(v)));
			} else {
				jas_matrix_setv(data[0], x, (v));
			}
		}
		if (pad) {
			if ((c = jas_stream_getc(in)) == EOF) {
				goto error;
			}
		}
		for (unsigned i = 0; i < jas_image_numcmpts(image); ++i) {
			if (jas_image_writecmpt(image, i, 0, y, hdr->width, 1,
			  data[i])) {
				goto error;
			}
		}
	}

	for (unsigned i = 0; i < jas_image_numcmpts(image); ++i) {
		jas_matrix_destroy(data[i]);
		data[i] = 0;
	}

	return 0;

error:
	for (unsigned i = 0; i < 3; ++i) {
		if (data[i]) {
			jas_matrix_destroy(data[i]);
		}
	}
	return -1;
}

static int ras_getcmap(jas_stream_t *in, ras_hdr_t *hdr, ras_cmap_t *cmap)
{
	int i;
	int j;
	int x;
	int c;
	int numcolors;
	int actualnumcolors;

	switch (hdr->maptype) {
	case RAS_MT_NONE:
		break;
	case RAS_MT_EQUALRGB:
		{
		jas_logwarnf("warning: palettized images not fully supported\n");
		numcolors = 1 << hdr->depth;
		if (numcolors > RAS_CMAP_MAXSIZ) {
			return -1;
		}
		actualnumcolors = hdr->maplength / 3;
		for (i = 0; i < numcolors; i++) {
			cmap->data[i] = 0;
		}
		if ((hdr->maplength % 3) || hdr->maplength < 0 ||
		  hdr->maplength > 3 * numcolors) {
			return -1;
		}
		for (i = 0; i < 3; i++) {
			for (j = 0; j < actualnumcolors; j++) {
				if ((c = jas_stream_getc(in)) == EOF) {
					return -1;
				}
				x = 0;
				switch (i) {
				case 0:
					x = RAS_RED(c);
					break;
				case 1:
					x = RAS_GREEN(c);
					break;
				case 2:
					x = RAS_BLUE(c);
					break;
				}
				cmap->data[j] |= x;
			}
		}
		}
		break;
	default:
		return -1;
	}

	return 0;
}

static int ras_gethdr(jas_stream_t *in, ras_hdr_t *hdr)
{
	if (ras_getint(in, &hdr->magic) || ras_getint(in, &hdr->width) ||
	  ras_getint(in, &hdr->height) || ras_getint(in, &hdr->depth) ||
	  ras_getint(in, &hdr->length) || ras_getint(in, &hdr->type) ||
	  ras_getint(in, &hdr->maptype) || ras_getint(in, &hdr->maplength)) {
		return -1;
	}

	if (hdr->magic != RAS_MAGIC) {
		return -1;
	}

	return 0;
}

static int ras_getint(jas_stream_t *in, int_fast32_t *val)
{
	int_fast32_t x;
	int c;
	int i;

	x = 0;
	for (i = 0; i < 4; i++) {
		if ((c = jas_stream_getc(in)) == EOF) {
			return -1;
		}
		x = (x << 8) | (c & 0xff);
	}

	*val = x;
	return 0;
}
