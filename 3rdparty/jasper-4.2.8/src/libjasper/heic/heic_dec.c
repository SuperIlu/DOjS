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

#include "jasper/jas_init.h"
#include "jasper/jas_tvp.h"
#include "jasper/jas_stream.h"
#include "jasper/jas_image.h"
#include "jasper/jas_debug.h"
#include "jasper/jas_math.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <libheif/heif.h>

/******************************************************************************\
* Types.
\******************************************************************************/

typedef struct {
	size_t max_samples;
	bool print_version;
} heic_dec_importopts_t;

typedef enum {
	OPT_MAXSIZE,
	OPT_VERSION,
} optid_t;

/******************************************************************************\
* Local functions.
\******************************************************************************/

/******************************************************************************\
* Option parsing.
\******************************************************************************/

static const jas_taginfo_t decopts[] = {
	{OPT_VERSION, "version"},
	{OPT_MAXSIZE, "max_samples"},
	{-1, 0}
};

static int heic_dec_parseopts(const char *optstr, heic_dec_importopts_t *opts)
{
	jas_tvparser_t *tvp;

	opts->max_samples = jas_get_dec_default_max_samples();
	opts->print_version = false;

	if (!(tvp = jas_tvparser_create(optstr ? optstr : ""))) {
		return -1;
	}

	while (!jas_tvparser_next(tvp)) {
		switch (jas_taginfo_nonull(jas_taginfos_lookup(decopts,
		  jas_tvparser_gettag(tvp)))->id) {
		case OPT_MAXSIZE:
			opts->max_samples = strtoull(jas_tvparser_getval(tvp), 0, 10);
			break;
		case OPT_VERSION:
			opts->print_version = true;
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
* Code for load operation.
\******************************************************************************/

/* Load an image from a stream in the HEIC format. */

jas_image_t *jas_heic_decode(jas_stream_t *in, const char *optstr)
{
	heic_dec_importopts_t opts;
    jas_image_t *image = 0;
	jas_stream_t *input_stream = 0;
	struct heif_image_handle *handle = 0;
	struct heif_image *img = 0;
	struct heif_context* ctx = 0;
	jas_matrix_t *matrix = 0;

	JAS_LOGDEBUGF(10, "jas_heic_decode(%p, \"%s\")\n", in, optstr);

	if (heic_dec_parseopts(optstr, &opts)) {
		jas_logerrorf("cannot parse decoder options\n");
		goto error;
	}

	if (opts.print_version) {
		printf("%08x %s\n", heif_get_version_number(), heif_get_version());
		goto error;
	}

	JAS_LOGDEBUGF(10, "HEIF library version: %08x\n",
	  heif_get_version_number());
	JAS_LOGDEBUGF(10, "HEIF library version: %s\n", heif_get_version());
	JAS_LOGDEBUGF(10, "max_samples %zu\n", opts.max_samples);

	if (!(input_stream = jas_stream_memopen(0, 0))) {
		jas_logerrorf("cannot create memory stream\n");
		goto error;
	}
	if (jas_stream_copy(input_stream, in, -1)) {
		jas_logerrorf("cannot copy stream\n");
		goto error;
	}
	/* jas_stream_rewind(input_stream); */

	jas_stream_memobj_t *obj = JAS_CAST(jas_stream_memobj_t*,
	  input_stream->obj_);
	unsigned char *ptr = obj->buf_;
	size_t size =  obj->len_;

	if (!(ctx = heif_context_alloc())) {
		jas_logerrorf("heif_context_alloc failed\n");
		goto error;
	}

	struct heif_error err;
	err = heif_context_read_from_memory_without_copy(ctx, ptr, size, 0);
	if (err.code != 0) {
		jas_logerrorf("heif_context_read_from_memory_without_copy failed\n");
		goto error;
	}

	/* Get a handle to the primary image. */
	err = heif_context_get_primary_image_handle(ctx, &handle);
	if (err.code != 0) {
		jas_logerrorf("heif_context_get_primary_image_handle failed\n");
		goto error;
	}

	int width = heif_image_handle_get_width(handle);
	int height = heif_image_handle_get_height(handle);
	JAS_LOGDEBUGF(1, "width %d; height %d\n", width, height);

	size_t num_samples;
	if (!jas_safe_size_mul(width, height, &num_samples)) {
		jas_logerrorf("overflow detected\n");
		goto error;
	}
	JAS_LOGDEBUGF(1, "num_samples %zu\n", num_samples);
	if (opts.max_samples && num_samples > opts.max_samples) {
		jas_logerrorf(
		  "maximum number of samples would be exceeded (%zu > %zu)\n",
		  num_samples, opts.max_samples);
		goto error;
	}

	/* Should I use:
	heif_context_set_maximum_image_size_limit(ctx, max_dim);
	*/

	/* Decode the image and convert the colorspace to RGB,
	  saved as 24bit interleaved. */
	err = heif_decode_image(handle, &img, heif_colorspace_RGB,
	  heif_chroma_interleaved_RGB, 0);
	if (err.code != 0) {
		jas_logerrorf("heif_decode_image failed\n");
		goto error;
	}

	enum heif_colorspace colorspace = heif_image_get_colorspace(img);
	enum heif_chroma chroma = heif_image_get_chroma_format(img);
	assert(chroma == heif_chroma_interleaved_RGB);
	JAS_LOGDEBUGF(1, "colorspace %d; chroma %d\n", colorspace, chroma);

	int stride;
	const uint8_t* data = heif_image_get_plane_readonly(img,
	  heif_channel_interleaved, &stride);
	JAS_LOGDEBUGF(1, "stride %d\n", stride);

	jas_image_cmptparm_t cmptparm;
	int numcmpts;
	numcmpts = 3;
	if (!(image = jas_image_create0())) {
		jas_logerrorf("jas_image_create0 failed\n");
		goto error;
	}
	int cmptno;
	for (cmptno = 0; cmptno < numcmpts; ++cmptno) {
		if (width > JAS_IMAGE_COORD_MAX ||
		  height > JAS_IMAGE_COORD_MAX) {
			jas_logerrorf("image size too large\n");
			goto error;
		}
		cmptparm.tlx = 0;
		cmptparm.tly = 0;
		cmptparm.hstep = 1;
		cmptparm.vstep = 1;
		cmptparm.width = width;
		cmptparm.height = height;
		cmptparm.prec = 8;
		cmptparm.sgnd = false;
		if (jas_image_addcmpt(image, cmptno, &cmptparm)) {
			jas_logerrorf("jas_image_addcmpt failed\n");
			goto error;
		}
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

	/* Initialize the data sink object. */
	if (!(matrix = jas_matrix_create(1, width))) {
		jas_logerrorf("jas_matrix_create failed\n");
		goto error;
	}

	for (cmptno = 0; cmptno < numcmpts; ++cmptno) {
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				jas_matrix_set(matrix, 0, x,
				  data[3 * width * y + 3 * x + cmptno]);
			}
			if (jas_image_writecmpt(image, cmptno, 0, y, width, 1, matrix)) {
				jas_logerrorf("jas_image_writecmpt failed\n");
				goto error;
			}
		}
	}

	jas_matrix_destroy(matrix);
	matrix = 0;
	jas_stream_close(input_stream);
	input_stream = 0;

	heif_image_release(img);
	img = 0;
	heif_context_free(ctx);
	ctx = 0;
	heif_image_handle_release(handle);
	handle = 0;

	return image;

error:
	if (matrix) {
		jas_matrix_destroy(matrix);
	}
	if (img) {
		heif_image_release(img);
	}
	if (ctx) {
		heif_context_free(ctx);
	}
	if (handle) {
		heif_image_handle_release(handle);
	}
	if (image) {
		jas_image_destroy(image);
	}
	if (input_stream) {
		jas_stream_close(input_stream);
	}
	return 0;
}
