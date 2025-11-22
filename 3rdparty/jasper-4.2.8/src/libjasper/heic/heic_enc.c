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

#include "jasper/jas_types.h"
#include "jasper/jas_tvp.h"
#include "jasper/jas_stream.h"
#include "jasper/jas_image.h"
#include "jasper/jas_debug.h"

#include <assert.h>
#include <stdlib.h>
#include <libheif/heif.h>

/******************************************************************************\
* Types.
\******************************************************************************/

typedef struct {
	bool lossless;
	int quality;
	bool print_version;
	bool special;
} heic_encopts_t;

typedef enum {
	OPT_VERSION,
	OPT_LOSSLESS,
	OPT_QUALITY,
	OPT_SPECIAL
} heic_optid_t;

/******************************************************************************\
* Code for parsing options.
\******************************************************************************/

static const jas_taginfo_t heic_opttab[] = {
	{OPT_VERSION, "version"},
	{OPT_LOSSLESS, "lossless"},
	{OPT_QUALITY, "quality"},
	{OPT_SPECIAL, "special"},
	{-1, 0}
};

/******************************************************************************\
* Code for parsing options.
\******************************************************************************/

/* Parse the encoder options string. */
static int heic_parseencopts(const char *optstr, heic_encopts_t *encopts)
{
	jas_tvparser_t *tvp;
	int ret;

	tvp = 0;

	/* Initialize default values for encoder options. */
	encopts->print_version = 0;
	encopts->quality = 75;
	encopts->lossless = true;
	encopts->special = false;

	/* Create the tag-value parser. */
	if (!(tvp = jas_tvparser_create(optstr ? optstr : ""))) {
		jas_logerrorf("jas_tvparser_create failed\n");
		goto error;
	}

	/* Get tag-value pairs, and process as necessary. */
	while (!(ret = jas_tvparser_next(tvp))) {
		switch (jas_taginfo_nonull(jas_taginfos_lookup(heic_opttab,
		  jas_tvparser_gettag(tvp)))->id) {
		case OPT_VERSION:
			encopts->print_version = true;
			break;
		case OPT_LOSSLESS:
			encopts->lossless = true;
			break;
		case OPT_QUALITY:
			encopts->quality = atoi(jas_tvparser_getval(tvp));
			break;
		case OPT_SPECIAL:
			encopts->special = true;
			break;
		default:
			jas_logwarnf("warning: ignoring invalid option %s\n",
			  jas_tvparser_gettag(tvp));
			break;
		}	
	}
	if (ret < 0) {
		jas_logerrorf("jas_tvparser_next failed\n");
		goto error;
	}

	/* Destroy the tag-value parser. */
	jas_tvparser_destroy(tvp);

	return 0;

error:
	if (tvp) {
		jas_tvparser_destroy(tvp);
	}
	return -1;
}

/******************************************************************************\
* Code for save operation.
\******************************************************************************/

static struct heif_error jas_heic_write(struct heif_context* ctx,
  const void* data, size_t size, void* userdata);

static struct heif_error jas_heic_write(struct heif_context* ctx,
  const void* data, size_t size, void* userdata)
{
	JAS_UNUSED(ctx);
	JAS_LOGDEBUGF(10, "jas_heic_write(%p, %p, %zu, %p)\n", ctx, data, size,
	  userdata);
	struct heif_error error;
	jas_stream_t *out = JAS_CAST(jas_stream_t *, userdata);
	if (jas_stream_write(out, data, size)) {
		error.code = heif_error_Usage_error;
		error.message = "";
	}
	error.code = heif_error_Ok;
	return error;
}

/* Save an image to a stream in the HEIC format. */

int jas_heic_encode(jas_image_t *image, jas_stream_t *out, const char *optstr)
{
	struct heif_context* ctx = 0;
	struct heif_encoder* encoder = 0;
	struct heif_image* img = 0;

	heic_encopts_t opts;
	struct heif_error error;
	struct heif_writer writer;

	if (heic_parseencopts(optstr, &opts)) {
		jas_logerrorf("cannot parse encoder options\n");
		goto error;
	}
	JAS_LOGDEBUGF(10, "quality %d\n", opts.quality);

	if (opts.print_version) {
		printf("%08x %s\n", heif_get_version_number(), heif_get_version());
		goto error;
	}

	JAS_LOGDEBUGF(10, "HEIF library version: %08x\n",
	  heif_get_version_number());
	JAS_LOGDEBUGF(10, "HEIF library version: %s\n", heif_get_version());

	int width = jas_image_width(image);
	int height = jas_image_height(image);
	int num_comps = jas_image_numcmpts(image);

	if (opts.special && num_comps) {
		num_comps = 1;
	}

	if (num_comps != 1) {
		jas_logerrorf("HEIC encoder currently only supports grayscale images\n");
		goto error;
	}

	if (!(ctx = heif_context_alloc())) {
		jas_logerrorf("heif_context_alloc failed\n");
		goto error;
	}

	error = heif_context_get_encoder_for_format(ctx, heif_compression_HEVC,
	  &encoder);
	if (error.code != heif_error_Ok) {
		jas_logerrorf("heif_context_get_encoder_for_format failed\n");
		goto error;
	}

	if (opts.lossless) {
		heif_encoder_set_lossless(encoder, true);
	} else {
		heif_encoder_set_lossy_quality(encoder, opts.quality);
	}

	enum heif_colorspace colorspace;
	enum heif_chroma chroma;

	if (num_comps == 1) {
		colorspace = heif_colorspace_monochrome;
		chroma = heif_chroma_monochrome;
	} else if (num_comps == 3) {
		colorspace = heif_colorspace_RGB;
		chroma = heif_chroma_interleaved_RGB;
	} else {
		jas_logerrorf("number of components is not 1 or 3\n");
		goto error;
	}
	error = heif_image_create(width, height, colorspace, chroma, &img);
	if (error.code != heif_error_Ok) {
		jas_logerrorf("heif_image_create failed\n");
		goto error;
	}

	for (int comp_no = 0; comp_no < num_comps; ++comp_no) {
		enum heif_channel channel;
		if (num_comps == 3) {
			channel = heif_channel_R;
			channel = heif_channel_G;
			channel = heif_channel_B;
		} else {
			channel = heif_channel_Y;
		}
		error = heif_image_add_plane(img, channel, width, height, 8);
		if (error.code != heif_error_Ok) {
			jas_logerrorf("heif_image_add_plane failed (%s)\n",
			  error.message ? error.message : "(null)");
			goto error;
		}
		int row_stride;
		uint8_t* data = heif_image_get_plane(img, channel, &row_stride);
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				data[y * row_stride + x] = jas_image_readcmptsample(image,
				  comp_no, x, y);
			}
		}
	}

	error = heif_context_encode_image(ctx, img, encoder, 0, 0);
	if (error.code != heif_error_Ok) {
		jas_logerrorf("heif_context_encode_image failed (%s)\n", error.message);
		goto error;
	}

	//heif_context_write_to_file(context, "output.heic");
	writer.writer_api_version = 1;
	writer.write = &jas_heic_write;
	error = heif_context_write(ctx, &writer, out);
	if (error.code != heif_error_Ok) {
		jas_logerrorf("heif_context_write failed\n");
		goto error;
	}

	heif_image_release(img);
	img = 0;

	heif_encoder_release(encoder);
	encoder = 0;

	heif_context_free(ctx);
	ctx = 0;

	if (jas_stream_flush(out) < 0) {
		jas_logerrorf("cannot flush encoded output stream\n");
		goto error;
	}

	return 0;

error:
	if (img) {
		heif_image_release(img);
	}
	if (encoder) {
		heif_encoder_release(encoder);
	}
	if (ctx) {
		heif_context_free(ctx);
	}
	return -1;
}
