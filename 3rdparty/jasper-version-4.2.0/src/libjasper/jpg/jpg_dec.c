/*
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

/******************************************************************************\
* Includes.
\******************************************************************************/

#include "jpg_jpeglib.h"

#include "jasper/jas_init.h"
#include "jasper/jas_tvp.h"
#include "jasper/jas_stream.h"
#include "jasper/jas_image.h"
#include "jasper/jas_debug.h"
#include "jasper/jas_math.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/******************************************************************************\
* Types.
\******************************************************************************/

typedef struct {
	size_t max_samples;
	bool print_version;
} jpg_dec_importopts_t;

typedef enum {
	OPT_MAXSIZE,
	OPT_VERSION,
} optid_t;

/* JPEG decoder data sink type. */

typedef struct jpg_dest_s {

	/* Initialize output. */
	void (*start_output)(j_decompress_ptr cinfo, struct jpg_dest_s *dinfo);

	/* Output rows of decompressed data. */
	void (*put_pixel_rows)(j_decompress_ptr cinfo, struct jpg_dest_s *dinfo,
	  JDIMENSION rows_supplied);

	/* Cleanup output. */
	void (*finish_output)(j_decompress_ptr cinfo, struct jpg_dest_s *dinfo);

	/* Output buffer. */
	JSAMPARRAY buffer;

	/* Height of output buffer. */
	JDIMENSION buffer_height;

	/* The current row. */
	JDIMENSION row;

	/* The image used to hold the decompressed sample data. */
	jas_image_t *image;

	/* The row buffer. */
	jas_matrix_t *data;

	/* The error indicator.  If this is nonzero, something has gone wrong
	  during decompression. */
	int error;

} jpg_dest_t;

/******************************************************************************\
* Local functions.
\******************************************************************************/

static void jpg_start_output(j_decompress_ptr cinfo, jpg_dest_t *dinfo);
static void jpg_put_pixel_rows(j_decompress_ptr cinfo, jpg_dest_t *dinfo,
  JDIMENSION rows_supplied);
static void jpg_finish_output(j_decompress_ptr cinfo, jpg_dest_t *dinfo);
static int jpg_copystreamtofile(FILE *out, jas_stream_t *in);
static jas_image_t *jpg_mkimage(j_decompress_ptr cinfo);

/******************************************************************************\
*
\******************************************************************************/

#if defined(LIBJPEG_TURBO_VERSION)
#define JAS_LIBJPEG_TURBO_VERSION JAS_STRINGIFYX(LIBJPEG_TURBO_VERSION)
#else
#define JAS_LIBJPEG_TURBO_VERSION ""
#endif
static const char jas_libjpeg_turbo_version[] = JAS_LIBJPEG_TURBO_VERSION;

/******************************************************************************\
* Option parsing.
\******************************************************************************/

static const jas_taginfo_t decopts[] = {
	{OPT_VERSION, "version"},
	{OPT_MAXSIZE, "max_samples"},
	{-1, 0}
};

static int jpg_dec_parseopts(const char *optstr, jpg_dec_importopts_t *opts)
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

/* Load an image from a stream in the JPG format. */

jas_image_t *jpg_decode(jas_stream_t *in, const char *optstr)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE *input_file;
	jpg_dest_t dest_mgr_buf;
	jpg_dest_t *dest_mgr = &dest_mgr_buf;
	JDIMENSION num_scanlines;
	jas_image_t *image;
	int ret;
	jpg_dec_importopts_t opts;
	size_t num_samples;
	bool cinfo_initialized = false;

	// In theory, the two memset calls that follow are not needed.
	// They are only here to make the code more predictable in the event
	// that the JPEG library fails to initialize a member.
	memset(&cinfo, 0, sizeof(struct jpeg_decompress_struct));
	memset(dest_mgr, 0, sizeof(jpg_dest_t));

	image = 0;
	input_file = 0;
	dest_mgr->data = 0;

	JAS_LOGDEBUGF(10, "jpg_decode(%p, \"%s\")\n", in, optstr);

	if (jpg_dec_parseopts(optstr, &opts)) {
		goto error;
	}

	if (opts.print_version) {
		printf("%d %s\n", JPEG_LIB_VERSION, jas_libjpeg_turbo_version);
		goto error;
	}

	JAS_LOGDEBUGF(10, "JPEG library version: %d\n", JPEG_LIB_VERSION);
	JAS_LOGDEBUGF(10, "JPEG Turbo library version: %s\n",
	  jas_libjpeg_turbo_version);

	if (!(input_file = tmpfile())) {
		jas_logerrorf("cannot make temporary file\n");
		goto error;
	}
	if (jpg_copystreamtofile(input_file, in)) {
		jas_logerrorf("cannot copy stream\n");
		goto error;
	}
	rewind(input_file);

	/* Allocate and initialize a JPEG decompression object. */
	JAS_LOGDEBUGF(10, "jpeg_std_error(%p)\n", &jerr);
	cinfo.err = jpeg_std_error(&jerr);
	JAS_LOGDEBUGF(10, "jpeg_create_decompress(%p)\n", &cinfo);
	jpeg_create_decompress(&cinfo);
	cinfo_initialized = 1;

	/* Specify the data source for decompression. */
	JAS_LOGDEBUGF(10, "jpeg_stdio_src(%p, %p)\n", &cinfo, input_file);
	jpeg_stdio_src(&cinfo, input_file);

	/* Read the file header to obtain the image information. */
	JAS_LOGDEBUGF(10, "jpeg_read_header(%p, TRUE)\n", &cinfo);
	ret = jpeg_read_header(&cinfo, TRUE);
	JAS_LOGDEBUGF(10, "jpeg_read_header return value %d\n", ret);
	if (ret != JPEG_HEADER_OK) {
		jas_logwarnf("jpeg_read_header did not return JPEG_HEADER_OK\n");
	}
	JAS_LOGDEBUGF(10,
	  "header: image_width %d; image_height %d; num_components %d\n",
	  cinfo.image_width, cinfo.image_height, cinfo.num_components
	  );

	if (!cinfo.image_width || !cinfo.image_height || !cinfo.num_components) {
		jas_logerrorf("image has no samples");
		goto error;
	}
	if (opts.max_samples > 0) {
		if (!jas_safe_size_mul3(cinfo.image_width, cinfo.image_height,
		  cinfo.num_components, &num_samples)) {
			goto error;
		}
		if (num_samples > opts.max_samples) {
			jas_logerrorf("image is too large (%zu > %zu)\n", num_samples,
			  opts.max_samples);
			goto error;
		}
	}

	/* Start the decompressor. */
	JAS_LOGDEBUGF(10, "jpeg_start_decompress(%p)\n", &cinfo);
	ret = jpeg_start_decompress(&cinfo);
	JAS_LOGDEBUGF(10, "jpeg_start_decompress return value %d\n", ret);
	JAS_LOGDEBUGF(10,
	  "header: output_width %d; output_height %d; output_components %d\n",
	  cinfo.output_width, cinfo.output_height, cinfo.output_components
	  );

	/* Create an image object to hold the decoded data. */
	if (!(image = jpg_mkimage(&cinfo))) {
		jas_logerrorf("jpg_mkimage failed\n");
		goto error;
	}

	/* Initialize the data sink object. */
	dest_mgr->image = image;
	if (!(dest_mgr->data = jas_matrix_create(1, cinfo.output_width))) {
		jas_logerrorf("jas_matrix_create failed\n");
		goto error;
	}
	dest_mgr->start_output = jpg_start_output;
	dest_mgr->put_pixel_rows = jpg_put_pixel_rows;
	dest_mgr->finish_output = jpg_finish_output;
    dest_mgr->buffer = (*cinfo.mem->alloc_sarray)
      ((j_common_ptr) &cinfo, JPOOL_IMAGE,
       cinfo.output_width * cinfo.output_components, (JDIMENSION) 1);
	dest_mgr->buffer_height = 1;
	dest_mgr->error = 0;

	/* Process the compressed data. */
	(*dest_mgr->start_output)(&cinfo, dest_mgr);
	while (cinfo.output_scanline < cinfo.output_height) {
		JAS_LOGDEBUGF(100, "jpeg_read_scanlines(%p, %p, %lu)\n", &cinfo,
		  dest_mgr->buffer, JAS_CAST(unsigned long, dest_mgr->buffer_height));
		num_scanlines = jpeg_read_scanlines(&cinfo, dest_mgr->buffer,
		  dest_mgr->buffer_height);
		JAS_LOGDEBUGF(100, "jpeg_read_scanlines return value %lu\n",
		  JAS_CAST(unsigned long, num_scanlines));
		(*dest_mgr->put_pixel_rows)(&cinfo, dest_mgr, num_scanlines);
	}
	(*dest_mgr->finish_output)(&cinfo, dest_mgr);

	/* Complete the decompression process. */
	JAS_LOGDEBUGF(10, "jpeg_finish_decompress(%p)\n", &cinfo);
	jpeg_finish_decompress(&cinfo);

	/* Destroy the JPEG decompression object. */
	JAS_LOGDEBUGF(10, "jpeg_destroy_decompress(%p)\n", &cinfo);
	jpeg_destroy_decompress(&cinfo);
	cinfo_initialized = 0;

	jas_matrix_destroy(dest_mgr->data);

	JAS_LOGDEBUGF(10, "fclose(%p)\n", input_file);
	fclose(input_file);
	input_file = 0;

	if (dest_mgr->error) {
		jas_logerrorf("error during decoding\n");
		goto error;
	}

	return image;

error:
	if (cinfo_initialized) {
		jpeg_destroy_decompress(&cinfo);
	}
	if (dest_mgr->data) {
		jas_matrix_destroy(dest_mgr->data);
	}
	if (image) {
		jas_image_destroy(image);
	}
	if (input_file) {
		fclose(input_file);
	}
	return 0;
}

/******************************************************************************\
*
\******************************************************************************/

#ifdef __clang__
/* suppress clang warning "result of comparison of constant
   9223372036854775807 with expression of type 'JDIMENSION' (aka
   'unsigned int') is always false" which happens on 64 bit targets
   where int_fast32_t (64 bit) is larger than JDIMENSION (= unsigned
   int, 32 bit) */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtautological-constant-out-of-range-compare"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
/* on GCC, it's this warning: */
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif

static jas_image_t *jpg_mkimage(j_decompress_ptr cinfo)
{
	jas_image_t *image;
	int cmptno;
	jas_image_cmptparm_t cmptparm;
	int numcmpts;

	JAS_LOGDEBUGF(10, "jpg_mkimage(%p)\n", cinfo);

	image = 0;
	numcmpts = cinfo->output_components;
	if (!(image = jas_image_create0())) {
		goto error;
	}
	for (cmptno = 0; cmptno < numcmpts; ++cmptno) {
		if (cinfo->image_width > JAS_IMAGE_COORD_MAX ||
		  cinfo->image_height > JAS_IMAGE_COORD_MAX) {
			goto error;
		}
		cmptparm.tlx = 0;
		cmptparm.tly = 0;
		cmptparm.hstep = 1;
		cmptparm.vstep = 1;
		cmptparm.width = cinfo->image_width;
		cmptparm.height = cinfo->image_height;
		cmptparm.prec = 8;
		cmptparm.sgnd = false;
		if (jas_image_addcmpt(image, cmptno, &cmptparm)) {
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

	return image;

error:
	if (image) {
		jas_image_destroy(image);
	}
	return 0;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

/******************************************************************************\
* Data source code.
\******************************************************************************/

static int jpg_copystreamtofile(FILE *out, jas_stream_t *in)
{
	int c;

	while ((c = jas_stream_getc(in)) != EOF) {
		if (fputc(c, out) == EOF) {
			return -1;
		}
	}
	if (jas_stream_error(in)) {
		return -1;
	}
	return 0;
}

/******************************************************************************\
* Data sink code.
\******************************************************************************/

static void jpg_start_output(j_decompress_ptr cinfo, jpg_dest_t *dinfo)
{
	JAS_UNUSED(cinfo);

	JAS_LOGDEBUGF(10, "jpg_start_output(%p, %p)\n", cinfo, dinfo);

	dinfo->row = 0;
}

static void jpg_put_pixel_rows(j_decompress_ptr cinfo, jpg_dest_t *dinfo,
  JDIMENSION rows_supplied)
{
	JSAMPLE *bufptr;
	int cmptno;
	JDIMENSION x;
	uint_fast32_t width;

	JAS_LOGDEBUGF(100, "jpg_put_pixel_rows(%p, %p)\n", cinfo, dinfo);

	if (dinfo->error) {
		return;
	}

	assert(cinfo->output_components == (int)jas_image_numcmpts(dinfo->image));

	for (cmptno = 0; cmptno < cinfo->output_components; ++cmptno) {
		width = jas_image_cmptwidth(dinfo->image, cmptno);
		bufptr = (dinfo->buffer[0]) + cmptno;
		for (x = 0; x < width; ++x) {
			jas_matrix_set(dinfo->data, 0, x, GETJSAMPLE(*bufptr));
			bufptr += cinfo->output_components;
		}
		JAS_LOGDEBUGF(100,
		  "jas_image_writecmpt called for component %d row %lu\n", cmptno,
		  JAS_CAST(unsigned long, dinfo->row));
		if (jas_image_writecmpt(dinfo->image, cmptno, 0, dinfo->row, width, 1,
		  dinfo->data)) {
			dinfo->error = 1;
		}
	}
	dinfo->row += rows_supplied;
}

static void jpg_finish_output(j_decompress_ptr cinfo, jpg_dest_t *dinfo)
{
	JAS_LOGDEBUGF(10, "jpg_finish_output(%p, %p)\n", cinfo, dinfo);

	JAS_UNUSED(cinfo);
	JAS_UNUSED(dinfo);
}
