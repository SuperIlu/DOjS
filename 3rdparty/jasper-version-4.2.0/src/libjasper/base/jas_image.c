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
 * Image Library
 *
 * $Id$
 */

/******************************************************************************\
* Includes.
\******************************************************************************/

#define JAS_FOR_INTERNAL_USE_ONLY

#include "jasper/jas_init.h"
#include "jasper/jas_image.h"
#include "jasper/jas_math.h"
#include "jasper/jas_malloc.h"
#include "jasper/jas_string.h"
#include "jasper/jas_debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

/******************************************************************************\
* Types.
\******************************************************************************/

#define	FLOORDIV(x, y) ((x) / (y))

/******************************************************************************\
* Local prototypes.
\******************************************************************************/

static jas_image_cmpt_t *jas_image_cmpt_create0(void);
static void jas_image_cmpt_destroy(jas_image_cmpt_t *cmpt);
static jas_image_cmpt_t *jas_image_cmpt_create(int_fast32_t tlx,
  int_fast32_t tly, int_fast32_t hstep, int_fast32_t vstep,
  int_fast32_t width, int_fast32_t height, uint_fast16_t depth, bool sgnd,
  bool inmem);
static void jas_image_setbbox(jas_image_t *image);
static jas_image_cmpt_t *jas_image_cmpt_copy(jas_image_cmpt_t *cmpt);
static int jas_image_growcmpts(jas_image_t *image, unsigned maxcmpts);
static uint_fast32_t inttobits(jas_seqent_t v, unsigned prec, bool sgnd);
static jas_seqent_t bitstoint(uint_fast32_t v, unsigned prec, bool sgnd);
static int putint(jas_stream_t *out, bool sgnd, unsigned prec, long val);
static int getint(jas_stream_t *in, bool sgnd, unsigned prec, long *val);
static long uptomult(long x, long y);
static long downtomult(long x, long y);
static long convert(long val, bool oldsgnd, unsigned oldprec, bool newsgnd,
  unsigned newprec);
static void jas_image_calcbbox2(const jas_image_t *image,
  jas_image_coord_t *tlx, jas_image_coord_t *tly, jas_image_coord_t *brx,
  jas_image_coord_t *bry);
static void jas_image_fmtinfo_init(jas_image_fmtinfo_t *fmtinfo);
static void jas_image_fmtinfo_cleanup(jas_image_fmtinfo_t *fmtinfo);

/******************************************************************************\
* Create and destroy operations.
\******************************************************************************/

jas_image_t *jas_image_create(unsigned numcmpts,
  const jas_image_cmptparm_t *cmptparms, jas_clrspc_t clrspc)
{
	jas_image_t *image;
	size_t rawsize;
	unsigned cmptno;
	const jas_image_cmptparm_t *cmptparm;

	image = 0;

	JAS_LOGDEBUGF(100, "jas_image_create(%d, %p, %d)\n", numcmpts, cmptparms,
	  clrspc);

	if (!(image = jas_image_create0())) {
		goto error;
	}

	image->clrspc_ = clrspc;
	image->maxcmpts_ = numcmpts;
//	image->inmem_ = true;

	/* Allocate memory for the per-component information. */
	if (!(image->cmpts_ = jas_alloc2(image->maxcmpts_,
	  sizeof(jas_image_cmpt_t *)))) {
		goto error;
	}
	/* Initialize in case of failure. */
	for (cmptno = 0; cmptno < image->maxcmpts_; ++cmptno) {
		image->cmpts_[cmptno] = 0;
	}

#if 0
	/* Compute the approximate raw size of the image. */
	rawsize = 0;
	for (cmptno = 0, cmptparm = cmptparms; cmptno < numcmpts; ++cmptno,
	  ++cmptparm) {
		rawsize += cmptparm->width * cmptparm->height *
		  (cmptparm->prec + 7) / 8;
	}
	/* Decide whether to buffer the image data in memory, based on the
	  raw size of the image. */
	inmem = (rawsize < JAS_IMAGE_INMEMTHRESH);
#endif

	/* Create the individual image components. */
	for (cmptno = 0, cmptparm = cmptparms; cmptno < numcmpts; ++cmptno,
	  ++cmptparm) {
		if (!jas_safe_size_mul3(cmptparm->width, cmptparm->height,
		  (cmptparm->prec + 7), &rawsize)) {
			goto error;
		}
		rawsize /= 8;
		const bool inmem = (rawsize < JAS_IMAGE_INMEMTHRESH);
		if (!(image->cmpts_[cmptno] = jas_image_cmpt_create(cmptparm->tlx,
		  cmptparm->tly, cmptparm->hstep, cmptparm->vstep,
		  cmptparm->width, cmptparm->height, cmptparm->prec,
		  cmptparm->sgnd, inmem))) {
			goto error;
		}
		++image->numcmpts_;
	}

	/* Determine the bounding box for all of the components on the
	  reference grid (i.e., the image area) */
	jas_image_setbbox(image);

	return image;

error:
	if (image) {
		jas_image_destroy(image);
	}
	return 0;
}

jas_image_t *jas_image_create0()
{
	jas_image_t *image;

	if (!(image = jas_malloc(sizeof(jas_image_t)))) {
		return 0;
	}

	image->tlx_ = 0;
	image->tly_ = 0;
	image->brx_ = 0;
	image->bry_ = 0;
	image->clrspc_ = JAS_CLRSPC_UNKNOWN;
	image->numcmpts_ = 0;
	image->maxcmpts_ = 0;
	image->cmpts_ = 0;
//	image->inmem_ = true;
	image->cmprof_ = 0;

	return image;
}

jas_image_t *jas_image_copy(jas_image_t *image)
{
	jas_image_t *newimage;

	if (!(newimage = jas_image_create0())) {
		goto error;
	}

	if (jas_image_growcmpts(newimage, image->numcmpts_)) {
		goto error;
	}
	for (unsigned cmptno = 0; cmptno < image->numcmpts_; ++cmptno) {
		if (!(newimage->cmpts_[cmptno] = jas_image_cmpt_copy(
		  image->cmpts_[cmptno]))) {
			goto error;
		}
		++newimage->numcmpts_;
	}

	jas_image_setbbox(newimage);

	if (image->cmprof_) {
		if (!(newimage->cmprof_ = jas_cmprof_copy(image->cmprof_))) {
			goto error;
		}
	}

	return newimage;
error:
	if (newimage) {
		jas_image_destroy(newimage);
	}
	return 0;
}

static jas_image_cmpt_t *jas_image_cmpt_create0()
{
	jas_image_cmpt_t *cmpt;
	if (!(cmpt = jas_malloc(sizeof(jas_image_cmpt_t)))) {
		return 0;
	}
	memset(cmpt, 0, sizeof(jas_image_cmpt_t));
	cmpt->type_ = JAS_IMAGE_CT_UNKNOWN;
	return cmpt;
}

static jas_image_cmpt_t *jas_image_cmpt_copy(jas_image_cmpt_t *cmpt)
{
	jas_image_cmpt_t *newcmpt;

	if (!(newcmpt = jas_image_cmpt_create0())) {
		return 0;
	}
	newcmpt->tlx_ = cmpt->tlx_;
	newcmpt->tly_ = cmpt->tly_;
	newcmpt->hstep_ = cmpt->hstep_;
	newcmpt->vstep_ = cmpt->vstep_;
	newcmpt->width_ = cmpt->width_;
	newcmpt->height_ = cmpt->height_;
	newcmpt->prec_ = cmpt->prec_;
	newcmpt->sgnd_ = cmpt->sgnd_;
	newcmpt->cps_ = cmpt->cps_;
	newcmpt->type_ = cmpt->type_;
	if (!(newcmpt->stream_ = jas_stream_memopen(0, 0))) {
		goto error;
	}
	if (jas_stream_seek(cmpt->stream_, 0, SEEK_SET)) {
		goto error;
	}
	if (jas_stream_copy(newcmpt->stream_, cmpt->stream_, -1)) {
		goto error;
	}
	if (jas_stream_seek(newcmpt->stream_, 0, SEEK_SET)) {
		goto error;
	}
	return newcmpt;
error:
	jas_image_cmpt_destroy(newcmpt);
	return 0;
}

void jas_image_destroy(jas_image_t *image)
{
	if (image->cmpts_) {
		for (unsigned i = 0; i < image->numcmpts_; ++i) {
			jas_image_cmpt_destroy(image->cmpts_[i]);
			image->cmpts_[i] = 0;
		}
		jas_free(image->cmpts_);
	}
	if (image->cmprof_) {
		jas_cmprof_destroy(image->cmprof_);
	}
	jas_free(image);
}

static jas_image_cmpt_t *jas_image_cmpt_create(int_fast32_t tlx,
  int_fast32_t tly, int_fast32_t hstep, int_fast32_t vstep,
  int_fast32_t width, int_fast32_t height, uint_fast16_t depth, bool sgnd,
  bool inmem)
{
	jas_image_cmpt_t *cmpt;
	size_t size;

	JAS_LOGDEBUGF(100,
	  "jas_image_cmpt_create(%ld, %ld, %ld, %ld, %ld, %ld, %d, %d, %d)\n",
	  JAS_CAST(long, tlx),
	  JAS_CAST(long, tly),
	  JAS_CAST(long, hstep),
	  JAS_CAST(long, vstep),
	  JAS_CAST(long, width),
	  JAS_CAST(long, height),
	  JAS_CAST(int, depth),
	  sgnd,
	  inmem
	  );

	if (depth < 1U + sgnd) {
		/* we need at least 1 bit for unsigned samples and 2
		   bits for signed samples */
		return NULL;
	}

	if (width < 0 || height < 0 || hstep <= 0 || vstep <= 0) {
		return NULL;
	}
	if (!jas_safe_intfast32_add(tlx, width, 0) ||
	  !jas_safe_intfast32_add(tly, height, 0)) {
		return NULL;
	}
	if (!jas_safe_intfast32_mul3(width, height, depth, 0)) {
		return NULL;
	}

	if (!(cmpt = jas_malloc(sizeof(jas_image_cmpt_t)))) {
		return NULL;
	}

	cmpt->type_ = JAS_IMAGE_CT_UNKNOWN;
	cmpt->tlx_ = tlx;
	cmpt->tly_ = tly;
	cmpt->hstep_ = hstep;
	cmpt->vstep_ = vstep;
	cmpt->width_ = width;
	cmpt->height_ = height;
	cmpt->prec_ = depth;
	cmpt->sgnd_ = sgnd;
	cmpt->stream_ = 0;
	cmpt->cps_ = (depth + 7) / 8;

	/*
	 * Compute the number of samples in the image component, while protecting
	 * against overflow.
	 * size = cmpt->width_ * cmpt->height_ * cmpt->cps_;
	 */
	if (!jas_safe_size_mul3(cmpt->width_, cmpt->height_, cmpt->cps_, &size)) {
		goto error;
	}
	cmpt->stream_ = (inmem) ? jas_stream_memopen(0, size) :
	  jas_stream_tmpfile();
	if (!cmpt->stream_) {
		goto error;
	}

	/* Zero the component data.  This isn't necessary, but it is
	convenient for debugging purposes. */
	/* Note: conversion of size - 1 to long can overflow */
	if (size > 0) {
		if (size - 1 > LONG_MAX) {
			goto error;
		}
		if (jas_stream_seek(cmpt->stream_, size - 1, SEEK_SET) < 0 ||
		  jas_stream_putc(cmpt->stream_, 0) == EOF ||
		  jas_stream_seek(cmpt->stream_, 0, SEEK_SET) < 0) {
			goto error;
		}
	}

	return cmpt;

error:
	jas_image_cmpt_destroy(cmpt);
	return 0;
}

static void jas_image_cmpt_destroy(jas_image_cmpt_t *cmpt)
{
	if (cmpt->stream_) {
		jas_stream_close(cmpt->stream_);
	}
	jas_free(cmpt);
}

/******************************************************************************\
* Load and save operations.
\******************************************************************************/

jas_image_t *jas_image_decode(jas_stream_t *in, int fmt, const char *optstr)
{
	const jas_image_fmtinfo_t *fmtinfo;
	jas_image_t *image;

	image = 0;

	/* If possible, try to determine the format of the input data. */
	if (fmt < 0) {
		if ((fmt = jas_image_getfmt(in)) < 0) {
			jas_logerrorf("jas_image_decode: cannot determine image format\n");
			goto error;
		}
	}

	/* Is it possible to decode an image represented in this format? */
	if (!(fmtinfo = jas_image_lookupfmtbyid(fmt))) {
		goto error;
	}
	if (!fmtinfo->ops.decode) {
		jas_logerrorf("jas_image_decode: no decode operation available\n");
		goto error;
	}

	/* Decode the image. */
	if (!(image = (*fmtinfo->ops.decode)(in, optstr))) {
		jas_logerrorf("jas_image_decode: decode operation failed\n");
		goto error;
	}

	/* Create a color profile if needed. */
	if (!jas_clrspc_isunknown(image->clrspc_) &&
	  !jas_clrspc_isgeneric(image->clrspc_) && !image->cmprof_) {
		if (!(image->cmprof_ =
		  jas_cmprof_createfromclrspc(jas_image_clrspc(image)))) {
			jas_logerrorf("jas_image_decode: cannot create CM profile\n");
			goto error;
		}
	}

	return image;

error:
	if (image) {
		jas_image_destroy(image);
	}
	return 0;
}

int jas_image_encode(jas_image_t *image, jas_stream_t *out, int fmt,
  const char *optstr)
{
	const jas_image_fmtinfo_t *fmtinfo;
	if (!(fmtinfo = jas_image_lookupfmtbyid(fmt))) {
		jas_logerrorf("format lookup failed\n");
		return -1;
	}
	return (fmtinfo->ops.encode) ? (*fmtinfo->ops.encode)(image, out,
	  optstr) : (-1);
}

/******************************************************************************\
* Component read and write operations.
\******************************************************************************/

int jas_image_readcmpt(jas_image_t *image, unsigned cmptno, jas_image_coord_t x,
  jas_image_coord_t y, jas_image_coord_t width, jas_image_coord_t height,
  jas_matrix_t *data)
{
	jas_image_cmpt_t *cmpt;
	jas_image_coord_t i;
	jas_image_coord_t j;
	jas_seqent_t v;
	int c;
	jas_seqent_t *dr;
	jas_seqent_t *d;

	JAS_LOGDEBUGF(100, "jas_image_readcmpt(%p, %d, %ld, %ld, %ld, %ld, %p)\n",
	  image, cmptno, JAS_CAST(long, x), JAS_CAST(long, y),
	  JAS_CAST(long, width), JAS_CAST(long, height), data);

	if(data == NULL) {
		return -1;
	}

	if (cmptno >= image->numcmpts_) {
		return -1;
	}

	cmpt = image->cmpts_[cmptno];
	if (x >= cmpt->width_ || y >= cmpt->height_ ||
	  x + width > cmpt->width_ ||
	  y + height > cmpt->height_) {
		return -1;
	}

	if (!jas_matrix_numrows(data) || !jas_matrix_numcols(data)) {
		return -1;
	}

	if (jas_matrix_numrows(data) != height || jas_matrix_numcols(data) !=
	  width) {
		if (jas_matrix_resize(data, height, width)) {
			return -1;
		}
	}

	jas_stream_t *const stream = cmpt->stream_;
	const uint_least32_t cmpt_width = cmpt->width_;
	const unsigned cps = cmpt->cps_;
	const unsigned prec = cmpt->prec_;
	const bool sgnd = cmpt->sgnd_;

	dr = jas_matrix_getref(data, 0, 0);
	const uint_least32_t drs = jas_matrix_rowstep(data);

#ifdef _MSC_VER
	jas_uchar *stack_buffer;
	if (cps == 1 && !sgnd && width <= 16384) {
		/* can't use variable-length arrays here because MSVC
		   doesn't support this C99 feature */
		stack_buffer = _alloca(width);
	}
#endif

	for (i = 0; i < height; ++i, dr += drs) {
		d = dr;
		if (jas_stream_seek(stream, (cmpt_width * (y + i) + x)
		  * cps, SEEK_SET) < 0) {
			return -1;
		}

		if (cps == 1 && !sgnd && width <= 16384) {
			/* fast path for 1 byte per sample and
			   unsigned with bulk reads */

#ifdef _MSC_VER
			jas_uchar *buffer = stack_buffer;
#else
			jas_uchar buffer[width];
#endif

			if (jas_stream_read(stream, buffer, width) != width) {
				return -1;
			}

			for (j = 0; j < width; ++j) {
				d[j] = buffer[j];
			}

			continue;
		}

		for (j = width; j > 0; --j, ++d) {
			v = 0;
			for (unsigned k = cps; k > 0; --k) {
				if ((c = jas_stream_getc(stream)) == EOF) {
					return -1;
				}
				v = (v << 8) | (c & 0xff);
			}
			*d = bitstoint(v, prec, sgnd);
		}
	}

	return 0;
}

int jas_image_writecmpt(jas_image_t *image, unsigned cmptno,
  jas_image_coord_t x, jas_image_coord_t y, jas_image_coord_t width,
  jas_image_coord_t height, const jas_matrix_t *data)
{
	jas_image_cmpt_t *cmpt;
	jas_image_coord_t i;
	jas_image_coord_t j;
	jas_seqent_t v;
	int c;

	JAS_LOGDEBUGF(100, "jas_image_writecmpt(%p, %d, %ld, %ld, %ld, %ld, %p)\n",
	  image, cmptno, JAS_CAST(long, x), JAS_CAST(long, y),
	  JAS_CAST(long, width), JAS_CAST(long, height), data);

	if (cmptno >= image->numcmpts_) {
		return -1;
	}

	cmpt = image->cmpts_[cmptno];
	if (x >= cmpt->width_ || y >= cmpt->height_ ||
	  x + width > cmpt->width_ ||
	  y + height > cmpt->height_) {
		return -1;
	}

	if (!jas_matrix_numrows(data) || !jas_matrix_numcols(data)) {
		return -1;
	}

	if (jas_matrix_numrows(data) != height ||
	  jas_matrix_numcols(data) != width) {
		return -1;
	}

	jas_stream_t *const stream = cmpt->stream_;
	const uint_least32_t cmpt_width = cmpt->width_;
	const unsigned cps = cmpt->cps_;
	const unsigned prec = cmpt->prec_;
	const bool sgnd = cmpt->sgnd_;

	const jas_seqent_t *dr = jas_matrix_getref(data, 0, 0);
	const uint_least32_t drs = jas_matrix_rowstep(data);

#ifdef _MSC_VER
	jas_uchar *stack_buffer;
	if (cps == 1 && !sgnd && width <= 16384) {
		/* can't use variable-length arrays here because MSVC
		   doesn't support this C99 feature */
		stack_buffer = _alloca(width);
	}
#endif

	for (i = 0; i < height; ++i, dr += drs) {
		const jas_seqent_t *d = dr;
		if (jas_stream_seek(stream, (cmpt_width * (y + i) + x)
		  * cps, SEEK_SET) < 0) {
			return -1;
		}

		if (cps == 1 && !sgnd && width <= 16384) {
			/* fast path for 1 byte per sample and
			   unsigned with bulk writes */

#ifdef _MSC_VER
			jas_uchar *buffer = stack_buffer;
#else
			jas_uchar buffer[width];
#endif

			for (j = 0; j < width; ++j)
				buffer[j] = d[j];

			jas_stream_write(stream, buffer, width);
			continue;
		}

		for (j = width; j > 0; --j, ++d) {
			v = inttobits(*d, prec, sgnd);
			for (unsigned k = cps; k > 0; --k) {
				c = (v >> (8 * (cps - 1))) & 0xff;
				if (jas_stream_putc(stream,
				  (unsigned char) c) == EOF) {
					return -1;
				}
				v <<= 8;
			}
		}
	}

	return 0;
}

/******************************************************************************\
* File format operations.
\******************************************************************************/

static void jas_image_fmtinfo_init(jas_image_fmtinfo_t *fmtinfo)
{
	fmtinfo->id = -1;
	fmtinfo->name = 0;
	fmtinfo->ext = 0;
	fmtinfo->exts = 0;
	fmtinfo->max_exts = 0;
	fmtinfo->num_exts = 0;
	fmtinfo->enabled = 0;
	fmtinfo->desc = 0;
	memset(&fmtinfo->ops, 0, sizeof(jas_image_fmtops_t));
}

static void jas_image_fmtinfo_cleanup(jas_image_fmtinfo_t *fmtinfo)
{
	if (fmtinfo->name) {
		jas_free(fmtinfo->name);
		fmtinfo->name = 0;
	}
	if (fmtinfo->ext) {
		jas_free(fmtinfo->ext);
		fmtinfo->ext = 0;
	}
	if (fmtinfo->exts) {
		assert(fmtinfo->max_exts > 0);
		for (int i = 0; i < fmtinfo->num_exts; ++i) {
			jas_free(fmtinfo->exts[i]);
		}
		jas_free(fmtinfo->exts);
		fmtinfo->exts = 0;
	}
	if (fmtinfo->desc) {
		jas_free(fmtinfo->desc);
		fmtinfo->desc = 0;
	}
}

JAS_EXPORT
const jas_image_fmtinfo_t *jas_image_getfmtbyind(int index)
{
	jas_ctx_t *ctx = jas_get_ctx();
	assert(index >= 0 && index < ctx->image_numfmts);
	return &ctx->image_fmtinfos[index];
}

JAS_EXPORT
int jas_image_getnumfmts(void)
{
	jas_ctx_t *ctx = jas_get_ctx();
	return ctx->image_numfmts;
}

JAS_EXPORT
int jas_image_setfmtenable(int index, int enabled)
{
	jas_ctx_t *ctx = jas_get_ctx();
	ctx->image_fmtinfos[index].enabled = enabled;
	return 0;
}

void jas_image_clearfmts_internal(jas_image_fmtinfo_t *image_fmtinfos,
  size_t *image_numfmts)
{
	jas_image_fmtinfo_t *fmtinfo;
	for (int fmtind = 0; fmtind < *image_numfmts; ++fmtind) {
		fmtinfo = &image_fmtinfos[fmtind];
		jas_image_fmtinfo_cleanup(fmtinfo);
	}
	*image_numfmts = 0;
}

void jas_image_clearfmts()
{
	jas_ctx_t *ctx = jas_get_ctx();
	jas_image_clearfmts_internal(ctx->image_fmtinfos, &ctx->image_numfmts);
}

int jas_image_addfmt_internal(jas_image_fmtinfo_t *image_fmtinfos,
  size_t *image_numfmts, int id, const char *name, const char *ext,
  const char *desc, const jas_image_fmtops_t *ops)
{
	const char delim[] = " \t";
	int ret = 0;
	jas_image_fmtinfo_t *fmtinfo = 0;

	assert(id >= 0 && name && ext && ops);
	if (*image_numfmts >= JAS_IMAGE_MAXFMTS) {
		ret = -1;
		goto done;
	}
	fmtinfo = &image_fmtinfos[*image_numfmts];
	jas_image_fmtinfo_init(fmtinfo);
	fmtinfo->id = id;
	if (jas_stringtokenize(ext, delim, &fmtinfo->exts, &fmtinfo->max_exts,
	  &fmtinfo->num_exts)) {
		assert(!fmtinfo->exts && !fmtinfo->max_exts && !fmtinfo->num_exts);
		ret = -1;
		goto done;
	}
	assert(fmtinfo->num_exts > 0);
	char *primary_ext = fmtinfo->exts[0];
	if (!(fmtinfo->name = jas_strdup(name))) {
		ret = -1;
		goto done;
	}
	if (!(fmtinfo->ext = jas_strdup(primary_ext))) {
		ret = -1;
		goto done;
	}
	if (!(fmtinfo->desc = jas_strdup(desc))) {
		ret = -1;
		goto done;
	}
	fmtinfo->ops = *ops;
	++(*image_numfmts);
done:
	if (ret && fmtinfo) {
		jas_image_fmtinfo_cleanup(fmtinfo);
	}
	return ret;
}

int jas_image_addfmt(int id, const char *name, const char *ext,
  const char *desc, const jas_image_fmtops_t *ops)
{
	jas_ctx_t *ctx = jas_get_ctx();
	return jas_image_addfmt_internal(ctx->image_fmtinfos, &ctx->image_numfmts,
	  id, name, ext, desc, ops);
}

/* This is for future consideration for addition to the library API. */
#if 0
JAS_EXPORT
int jas_image_delfmt(int fmtid)
{
	if (fmtid < 0 || fmtid >= JAS_CAST(int, jas_image_numfmts)) {
		return -1;
	}
	const jas_image_fmtinfo_t *fmtinfo;
	if (!(fmtinfo = jas_image_lookupfmtbyid(fmtid))) {
		return -1;
	}
	for (int i = fmtinfo - jas_image_fmtinfos; i < JAS_CAST(int,
	  jas_image_numfmts) - 1; ++i) {
		jas_image_fmtinfos[i] = jas_image_fmtinfos[i + 1];
	}
	--jas_image_numfmts;
	return 0;
}
#endif

int jas_image_strtofmt(const char *name)
{
	const jas_image_fmtinfo_t *fmtinfo;
	if (!(fmtinfo = jas_image_lookupfmtbyname(name))) {
		return -1;
	}
	return fmtinfo->id;
}

const char *jas_image_fmttostr(int fmt)
{
	const jas_image_fmtinfo_t *fmtinfo;
	if (!(fmtinfo = jas_image_lookupfmtbyid(fmt))) {
		return 0;
	}
	return fmtinfo->name;
}

int jas_image_getfmt(jas_stream_t *in)
{
	jas_ctx_t *ctx = jas_get_ctx();
	const jas_image_fmtinfo_t *fmtinfo;

	/* Check for data in each of the supported formats. */
	unsigned i;
	for (i = 0, fmtinfo = ctx->image_fmtinfos; i < ctx->image_numfmts; ++i,
	  ++fmtinfo) {
		if (fmtinfo->enabled && fmtinfo->ops.validate) {
			/* Is the input data valid for this format? */
			JAS_LOGDEBUGF(20, "testing for format %s\n", fmtinfo->name);
			if (!(*fmtinfo->ops.validate)(in)) {
				JAS_LOGDEBUGF(20, "test succeeded\n");
				return fmtinfo->id;
			}
			JAS_LOGDEBUGF(20, "test failed\n");
		}
	}
	return -1;
}

int jas_image_fmtfromname(const char *name)
{
	jas_ctx_t *ctx = jas_get_ctx();
	const char *ext;
	const jas_image_fmtinfo_t *fmtinfo;
	/* Get the file name extension. */
	if (!(ext = strrchr(name, '.'))) {
		return -1;
	}
	++ext;
	/* Try to find a format that uses this extension. */	
	unsigned i;
	for (i = 0, fmtinfo = ctx->image_fmtinfos; i < ctx->image_numfmts; ++i,
	  ++fmtinfo) {
		/* Do we have a match? */
		if (fmtinfo->enabled && !strcmp(ext, fmtinfo->ext)) {
			return fmtinfo->id;
		}
	}
	return -1;
}

/******************************************************************************\
* Miscellaneous operations.
\******************************************************************************/

bool jas_image_cmpt_domains_same(const jas_image_t *image)
{
	jas_image_cmpt_t *cmpt;
	jas_image_cmpt_t *cmpt0;

	cmpt0 = image->cmpts_[0];
	for (unsigned cmptno = 1; cmptno < image->numcmpts_; ++cmptno) {
		cmpt = image->cmpts_[cmptno];
		if (cmpt->tlx_ != cmpt0->tlx_ || cmpt->tly_ != cmpt0->tly_ ||
		  cmpt->hstep_ != cmpt0->hstep_ || cmpt->vstep_ != cmpt0->vstep_ ||
		  cmpt->width_ != cmpt0->width_ || cmpt->height_ != cmpt0->height_) {
			return 0;
		}
	}
	return 1;
}

uint_fast32_t jas_image_rawsize(const jas_image_t *image)
{
	uint_fast32_t rawsize;
	jas_image_cmpt_t *cmpt;

	rawsize = 0;
	for (unsigned cmptno = 0; cmptno < image->numcmpts_; ++cmptno) {
		cmpt = image->cmpts_[cmptno];
		rawsize += (cmpt->width_ * cmpt->height_ * cmpt->prec_ +
		  7) / 8;
	}
	return rawsize;
}

void jas_image_delcmpt(jas_image_t *image, unsigned cmptno)
{
	if (cmptno >= image->numcmpts_) {
		return;
	}
	jas_image_cmpt_destroy(image->cmpts_[cmptno]);
	if (cmptno < image->numcmpts_) {
		memmove(&image->cmpts_[cmptno], &image->cmpts_[cmptno + 1],
		  (image->numcmpts_ - 1 - cmptno) * sizeof(jas_image_cmpt_t *));
	}
	--image->numcmpts_;

	jas_image_setbbox(image);
}

int jas_image_addcmpt(jas_image_t *image, int cmptno,
  const jas_image_cmptparm_t *cmptparm)
{
	jas_image_cmpt_t *newcmpt;
	if (cmptno < 0) {
		cmptno = image->numcmpts_;
	}
	assert(cmptno >= 0 && (unsigned)cmptno <= image->numcmpts_);
	if (image->numcmpts_ >= image->maxcmpts_) {
		if (jas_image_growcmpts(image, image->maxcmpts_ + 128)) {
			return -1;
		}
	}
	if (!(newcmpt = jas_image_cmpt_create(cmptparm->tlx,
	  cmptparm->tly, cmptparm->hstep, cmptparm->vstep,
	  cmptparm->width, cmptparm->height, cmptparm->prec,
	  cmptparm->sgnd, true))) {
		return -1;
	}
	if ((unsigned)cmptno < image->numcmpts_) {
		memmove(&image->cmpts_[cmptno + 1], &image->cmpts_[cmptno],
		  (image->numcmpts_ - (unsigned)cmptno) * sizeof(jas_image_cmpt_t *));
	}
	image->cmpts_[cmptno] = newcmpt;
	++image->numcmpts_;

	jas_image_setbbox(image);

	return 0;
}

const jas_image_fmtinfo_t *jas_image_lookupfmtbyid(int id)
{
	jas_ctx_t *ctx = jas_get_ctx();
	unsigned i;
	const jas_image_fmtinfo_t *fmtinfo;

	for (i = 0, fmtinfo = ctx->image_fmtinfos; i < ctx->image_numfmts;
	  ++i, ++fmtinfo) {
		if (fmtinfo->id == id) {
			return fmtinfo;
		}
	}
	return 0;
}

const jas_image_fmtinfo_t *jas_image_lookupfmtbyname(const char *name)
{
	jas_ctx_t *ctx = jas_get_ctx();
	unsigned i;
	const jas_image_fmtinfo_t *fmtinfo;
	for (i = 0, fmtinfo = ctx->image_fmtinfos; i < ctx->image_numfmts;
	  ++i, ++fmtinfo) {
		if (!strcmp(fmtinfo->name, name)) {
			return fmtinfo;
		}
	}
	return 0;
}

static uint_fast32_t inttobits(jas_seqent_t v, unsigned prec, bool sgnd)
{
	uint_fast32_t ret;
	assert(v >= 0 || sgnd);
	ret = ((sgnd && v < 0) ? (JAS_POW2_X(jas_seqent_t, prec) + v) : v) &
	  JAS_ONES(prec);
	return ret;
}

static jas_seqent_t bitstoint(uint_fast32_t v, unsigned prec, bool sgnd)
{
	jas_seqent_t ret;
	v &= JAS_ONES(prec);
	ret = (sgnd && (v & (1 << (prec - 1)))) ? (v - (1 << prec)) : v;
	return ret;
}

static void jas_image_setbbox(jas_image_t *image)
{
	jas_image_cmpt_t *cmpt;
	int_fast32_t x;
	int_fast32_t y;

	if (image->numcmpts_ > 0) {
		/* Determine the bounding box for all of the components on the
		  reference grid (i.e., the image area) */
		cmpt = image->cmpts_[0];
		image->tlx_ = cmpt->tlx_;
		image->tly_ = cmpt->tly_;
		image->brx_ = cmpt->tlx_ + cmpt->hstep_ * (cmpt->width_ - 1) + 1;
		image->bry_ = cmpt->tly_ + cmpt->vstep_ * (cmpt->height_ - 1) + 1;
		for (unsigned cmptno = 1; cmptno < image->numcmpts_; ++cmptno) {
			cmpt = image->cmpts_[cmptno];
			if (image->tlx_ > cmpt->tlx_) {
				image->tlx_ = cmpt->tlx_;
			}
			if (image->tly_ > cmpt->tly_) {
				image->tly_ = cmpt->tly_;
			}
			x = cmpt->tlx_ + cmpt->hstep_ * (cmpt->width_ - 1) + 1;
			if (image->brx_ < x) {
				image->brx_ = x;
			}
			y = cmpt->tly_ + cmpt->vstep_ * (cmpt->height_ - 1) + 1;
			if (image->bry_ < y) {
				image->bry_ = y;
			}
		}
	} else {
		image->tlx_ = 0;
		image->tly_ = 0;
		image->brx_ = 0;
		image->bry_ = 0;
	}
}

static int jas_image_growcmpts(jas_image_t *image, unsigned maxcmpts)
{
	jas_image_cmpt_t **newcmpts;

	newcmpts = (!image->cmpts_) ? jas_alloc2(maxcmpts,
	  sizeof(jas_image_cmpt_t *)) :
	  jas_realloc2(image->cmpts_, maxcmpts, sizeof(jas_image_cmpt_t *));
	if (!newcmpts) {
		return -1;
	}
	image->cmpts_ = newcmpts;
	image->maxcmpts_ = maxcmpts;
	for (unsigned cmptno = image->numcmpts_; cmptno < image->maxcmpts_; ++cmptno) {
		image->cmpts_[cmptno] = 0;
	}
	return 0;
}

int jas_image_copycmpt(jas_image_t *dstimage, unsigned dstcmptno,
  jas_image_t *srcimage, unsigned srccmptno)
{
	jas_image_cmpt_t *newcmpt;
	if (dstimage->numcmpts_ >= dstimage->maxcmpts_) {
		if (jas_image_growcmpts(dstimage, dstimage->maxcmpts_ + 128)) {
			return -1;
		}
	}
	if (!(newcmpt = jas_image_cmpt_copy(srcimage->cmpts_[srccmptno]))) {
		return -1;
	}
	if (dstcmptno < dstimage->numcmpts_) {
		memmove(&dstimage->cmpts_[dstcmptno + 1], &dstimage->cmpts_[dstcmptno],
		  (dstimage->numcmpts_ - dstcmptno) * sizeof(jas_image_cmpt_t *));
	}
	dstimage->cmpts_[dstcmptno] = newcmpt;
	++dstimage->numcmpts_;

	jas_image_setbbox(dstimage);
	return 0;
}

int jas_image_dump(jas_image_t *image, FILE *out)
{
	long buf[1024];
	jas_image_cmpt_t *cmpt;
	for (unsigned cmptno = 0; cmptno < image->numcmpts_; ++cmptno) {
		cmpt = image->cmpts_[cmptno];
		fprintf(out, "prec=%d, sgnd=%d, cmpttype=%"PRIiFAST32"\n", cmpt->prec_,
		  cmpt->sgnd_, cmpt->type_);
		const unsigned width = jas_image_cmptwidth(image, cmptno);
		const unsigned height = jas_image_cmptheight(image, cmptno);
		const unsigned n = JAS_MIN(16, width);
		if (jas_image_readcmpt2(image, cmptno, 0, 0, n, 1, buf)) {
			return -1;
		}
		for (unsigned i = 0; i < n; ++i) {
			fprintf(out, " f(%d,%d)=%ld", i, 0, buf[i]);
		}
		fprintf(out, "\n");
		if (jas_image_readcmpt2(image, cmptno, width - n, height - 1, n, 1,
		  buf)) {
			return -1;
		}
		for (unsigned i = 0; i < n; ++i) {
			fprintf(out, " f(%d,%d)=%ld", width - n + i, height - 1, buf[i]);
		}
		fprintf(out, "\n");
	}
	return 0;
}

int jas_image_depalettize(jas_image_t *image, unsigned cmptno,
  unsigned numlutents, const int_fast32_t *lutents, unsigned dtype,
  unsigned newcmptno)
{
	jas_image_cmptparm_t cmptparms;
	const jas_image_cmpt_t *cmpt = image->cmpts_[cmptno];

	const uint_least32_t width = cmpt->width_;
	const uint_least32_t height = cmpt->height_;

	cmptparms.tlx = cmpt->tlx_;
	cmptparms.tly = cmpt->tly_;
	cmptparms.hstep = cmpt->hstep_;
	cmptparms.vstep = cmpt->vstep_;
	cmptparms.width = width;
	cmptparms.height = height;
	cmptparms.prec = JAS_IMAGE_CDT_GETPREC(dtype);
	cmptparms.sgnd = JAS_IMAGE_CDT_GETSGND(dtype);

	if (jas_image_addcmpt(image, newcmptno, &cmptparms)) {
		return -1;
	}
	if (newcmptno <= cmptno) {
		++cmptno;
		cmpt = image->cmpts_[cmptno];
	}

	for (uint_least32_t j = 0; j < height; ++j) {
		for (uint_least32_t i = 0; i < width; ++i) {
			int v = jas_image_readcmptsample(image, cmptno, i, j);
			if (v < 0) {
				v = 0;
			} else if ((unsigned)v >= numlutents) {
				assert(numlutents > 0);
				v = numlutents - 1;
			}
			jas_image_writecmptsample(image, newcmptno, i, j,
			  lutents[v]);
		}
	}
	return 0;
}

int jas_image_readcmptsample(jas_image_t *image, unsigned cmptno, unsigned x, unsigned y)
{
	jas_image_cmpt_t *cmpt;
	uint_fast32_t v;
	int c;

	cmpt = image->cmpts_[cmptno];

	if (jas_stream_seek(cmpt->stream_, (cmpt->width_ * y + x) * cmpt->cps_,
	  SEEK_SET) < 0) {
		return -1;
	}
	v = 0;
	for (unsigned k = cmpt->cps_; k > 0; --k) {
		if ((c = jas_stream_getc(cmpt->stream_)) == EOF) {
			return -1;
		}
		v = (v << 8) | (c & 0xff);
	}
	return bitstoint(v, cmpt->prec_, cmpt->sgnd_);
}

void jas_image_writecmptsample(jas_image_t *image, unsigned cmptno, unsigned x, unsigned y,
  int_fast32_t v)
{
	jas_image_cmpt_t *cmpt;
	uint_fast32_t t;
	int c;

	cmpt = image->cmpts_[cmptno];

	if (jas_stream_seek(cmpt->stream_, (cmpt->width_ * y + x) * cmpt->cps_,
	  SEEK_SET) < 0) {
		return;
	}
	t = inttobits(v, cmpt->prec_, cmpt->sgnd_);
	for (unsigned k = cmpt->cps_; k > 0; --k) {
		c = (t >> (8 * (cmpt->cps_ - 1))) & 0xff;
		if (jas_stream_putc(cmpt->stream_, (unsigned char) c) == EOF) {
			return;
		}
		t <<= 8;
	}
}

int jas_image_getcmptbytype(const jas_image_t *image,
  jas_image_cmpttype_t ctype)
{
	for (unsigned cmptno = 0; cmptno < image->numcmpts_; ++cmptno) {
		if (image->cmpts_[cmptno]->type_ == ctype) {
			return cmptno;
		}
	}
	return -1;
}

/***********************************************/
/***********************************************/
/***********************************************/
/***********************************************/

int jas_image_readcmpt2(jas_image_t *image, unsigned cmptno,
  jas_image_coord_t x, jas_image_coord_t y, jas_image_coord_t width,
  jas_image_coord_t height, long *buf)
{
	jas_image_cmpt_t *cmpt;
	jas_image_coord_t i;
	jas_image_coord_t j;
	long v;
	long *bufptr;

	if (cmptno >= image->numcmpts_) {
		goto error;
	}
	cmpt = image->cmpts_[cmptno];
	if (x < 0 || x >= cmpt->width_ || y < 0 || y >= cmpt->height_ ||
	  width < 0 || height < 0 || x + width > cmpt->width_ ||
	  y + height > cmpt->height_) {
		goto error;
	}

	bufptr = buf;
	for (i = 0; i < height; ++i) {
		if (jas_stream_seek(cmpt->stream_, (cmpt->width_ * (y + i) + x)
		  * cmpt->cps_, SEEK_SET) < 0) {
			goto error;
		}
		for (j = 0; j < width; ++j) {
			if (getint(cmpt->stream_, cmpt->sgnd_, cmpt->prec_, &v)) {
				goto error;
			}
			*bufptr++ = v;
		}
	}

	return 0;
error:
	return -1;
}

int jas_image_writecmpt2(jas_image_t *image, unsigned cmptno,
  jas_image_coord_t x, jas_image_coord_t y, jas_image_coord_t width,
  jas_image_coord_t height, const long *buf)
{
	jas_image_cmpt_t *cmpt;
	jas_image_coord_t i;
	jas_image_coord_t j;
	long v;

	if (cmptno >= image->numcmpts_) {
		goto error;
	}
	cmpt = image->cmpts_[cmptno];
	if (x < 0 || x >= cmpt->width_ || y < 0 || y >= cmpt->height_ ||
	  width < 0 || height < 0 || x + width > cmpt->width_ ||
	  y + height > cmpt->height_) {
		goto error;
	}

	const long *bufptr = buf;
	for (i = 0; i < height; ++i) {
		if (jas_stream_seek(cmpt->stream_, (cmpt->width_ * (y + i) + x)
		  * cmpt->cps_, SEEK_SET) < 0) {
			goto error;
		}
		for (j = 0; j < width; ++j) {
			v = *bufptr++;
			if (putint(cmpt->stream_, cmpt->sgnd_, cmpt->prec_, v)) {
				goto error;
			}
		}
	}

	return 0;
error:
	return -1;
}

int jas_image_sampcmpt(jas_image_t *image, unsigned cmptno, unsigned newcmptno,
  jas_image_coord_t ho, jas_image_coord_t vo, jas_image_coord_t hs,
  jas_image_coord_t vs, int sgnd, unsigned prec)
{
	jas_image_coord_t tlx;
	jas_image_coord_t tly;
	jas_image_coord_t brx;
	jas_image_coord_t bry;
	jas_image_cmptparm_t cmptparm;
	jas_image_coord_t ax;
	jas_image_coord_t ay;
	jas_image_coord_t bx;
	jas_image_coord_t by;
	jas_image_coord_t d0;
	jas_image_coord_t d1;
	jas_image_coord_t d2;
	jas_image_coord_t d3;
	jas_image_coord_t oldx;
	jas_image_coord_t oldy;
	jas_image_coord_t x;
	jas_image_coord_t y;
	long v;
	jas_image_coord_t cmptbrx;
	jas_image_coord_t cmptbry;

	assert(cmptno < image->numcmpts_);
	const jas_image_cmpt_t *const oldcmpt = image->cmpts_[cmptno];
	assert(oldcmpt->tlx_ == 0 && oldcmpt->tly_ == 0);
	jas_image_calcbbox2(image, &tlx, &tly, &brx, &bry);
	const unsigned width = FLOORDIV(brx - ho + hs, hs);
	const unsigned height = FLOORDIV(bry - vo + vs, vs);
	cmptparm.tlx = ho;
	cmptparm.tly = vo;
	cmptparm.hstep = hs;
	cmptparm.vstep = vs;
	cmptparm.width = width;
	cmptparm.height = height;
	cmptparm.prec = prec;
	cmptparm.sgnd = sgnd;
	if (jas_image_addcmpt(image, newcmptno, &cmptparm)) {
		goto error;
	}
	cmptbrx = oldcmpt->tlx_ + (oldcmpt->width_ - 1) * oldcmpt->hstep_;
	cmptbry = oldcmpt->tly_ + (oldcmpt->height_ - 1) * oldcmpt->vstep_;
	const jas_image_cmpt_t *const newcmpt = image->cmpts_[newcmptno];
	jas_stream_rewind(newcmpt->stream_);
	for (unsigned i = 0; i < height; ++i) {
		y = newcmpt->tly_ + newcmpt->vstep_ * i;
		for (unsigned j = 0; j < width; ++j) {
			x = newcmpt->tlx_ + newcmpt->hstep_ * j;
			ax = downtomult(x - oldcmpt->tlx_, oldcmpt->hstep_) + oldcmpt->tlx_;
			ay = downtomult(y - oldcmpt->tly_, oldcmpt->vstep_) + oldcmpt->tly_;
			bx = uptomult(x - oldcmpt->tlx_, oldcmpt->hstep_) + oldcmpt->tlx_;
			if (bx > cmptbrx) {
				bx = cmptbrx;
			}
			by = uptomult(y - oldcmpt->tly_, oldcmpt->vstep_) + oldcmpt->tly_;
			if (by > cmptbry) {
				by = cmptbry;
			}
			d0 = (ax - x) * (ax - x) + (ay - y) * (ay - y);
			d1 = (bx - x) * (bx - x) + (ay - y) * (ay - y);
			d2 = (bx - x) * (bx - x) + (by - y) * (by - y);
			d3 = (ax - x) * (ax - x) + (by - y) * (by - y);
			if (d0 <= d1 && d0 <= d2 && d0 <= d3) {
				oldx = (ax - oldcmpt->tlx_) / oldcmpt->hstep_;
				oldy = (ay - oldcmpt->tly_) / oldcmpt->vstep_;
			} else if (d1 <= d0 && d1 <= d2 && d1 <= d3) {
				oldx = (bx - oldcmpt->tlx_) / oldcmpt->hstep_;
				oldy = (ay - oldcmpt->tly_) / oldcmpt->vstep_;
			} else if (d2 <= d0 && d2 <= d1 && d1 <= d3) {
				oldx = (bx - oldcmpt->tlx_) / oldcmpt->hstep_;
				oldy = (by - oldcmpt->tly_) / oldcmpt->vstep_;
			} else {
				oldx = (ax - oldcmpt->tlx_) / oldcmpt->hstep_;
				oldy = (by - oldcmpt->tly_) / oldcmpt->vstep_;
			}
			assert(oldx >= 0 && oldx < oldcmpt->width_ &&
			  oldy >= 0 && oldy < oldcmpt->height_);
			if (jas_stream_seek(oldcmpt->stream_, oldcmpt->cps_ *
			  (oldy * oldcmpt->width_ + oldx), SEEK_SET) < 0) {
				goto error;
			}
			if (getint(oldcmpt->stream_, oldcmpt->sgnd_,
			  oldcmpt->prec_, &v)) {
				goto error;
			}
			if (newcmpt->prec_ != oldcmpt->prec_ ||
			  newcmpt->sgnd_ != oldcmpt->sgnd_) {
				v = convert(v, oldcmpt->sgnd_, oldcmpt->prec_,
				  newcmpt->sgnd_, newcmpt->prec_);
			}
			if (putint(newcmpt->stream_, newcmpt->sgnd_,
			  newcmpt->prec_, v)) {
				goto error;
			}
		}
	}
	return 0;
error:
	return -1;
}

int jas_image_ishomosamp(const jas_image_t *image)
{
	jas_image_coord_t hstep;
	jas_image_coord_t vstep;
	int result;
	hstep = jas_image_cmpthstep(image, 0);
	vstep = jas_image_cmptvstep(image, 0);
	result = 1;
	for (unsigned i = 0; i < image->numcmpts_; ++i) {
		if (jas_image_cmpthstep(image, i) != hstep ||
		  jas_image_cmptvstep(image, i) != vstep) {
			result = 0;
			break;
		}
	}
	return result;
}

/* Note: This function defines a bounding box differently. */
static void jas_image_calcbbox2(const jas_image_t *image,
  jas_image_coord_t *tlx, jas_image_coord_t *tly, jas_image_coord_t *brx,
  jas_image_coord_t *bry)
{
	jas_image_coord_t tmptlx;
	jas_image_coord_t tmptly;
	jas_image_coord_t tmpbrx;
	jas_image_coord_t tmpbry;
	jas_image_coord_t t;
	if (image->numcmpts_ > 0) {
		const jas_image_cmpt_t *cmpt = image->cmpts_[0];
		tmptlx = cmpt->tlx_;
		tmptly = cmpt->tly_;
		tmpbrx = cmpt->tlx_ + cmpt->hstep_ * (cmpt->width_ - 1);
		tmpbry = cmpt->tly_ + cmpt->vstep_ * (cmpt->height_ - 1);
		for (unsigned i = 0; i < image->numcmpts_; ++i) {
			cmpt = image->cmpts_[i];
			if (cmpt->tlx_ < tmptlx) {
				tmptlx = cmpt->tlx_;
			}
			if (cmpt->tly_ < tmptly) {
				tmptly = cmpt->tly_;
			}
			t = cmpt->tlx_ + cmpt->hstep_ * (cmpt->width_ - 1);
			if (t > tmpbrx) {
				tmpbrx = t;
			}
			t = cmpt->tly_ + cmpt->vstep_ * (cmpt->height_ - 1);
			if (t > tmpbry) {
				tmpbry = t;
			}
		}
	} else {
		tmptlx = 0;
		tmptly = 0;
		tmpbrx = -1;
		tmpbry = -1;
	}
	*tlx = tmptlx;
	*tly = tmptly;
	*brx = tmpbrx;
	*bry = tmpbry;
}

JAS_ATTRIBUTE_CONST
static inline long decode_twos_comp(jas_ulong c, unsigned prec)
{
	long result;
	assert(prec >= 2);
	jas_logwarnf("warning: support for signed data is untested\n");
	// NOTE: Is this correct?
	result = (c & ((1 << (prec - 1)) - 1)) - (c & (1 << (prec - 1)));
	return result;
}

JAS_ATTRIBUTE_CONST
static inline jas_ulong encode_twos_comp(long n, unsigned prec)
{
	jas_ulong result;
	assert(prec >= 2);
	jas_logwarnf("warning: support for signed data is untested\n");
	// NOTE: Is this correct?
	if (n < 0) {
		result = -n;
		result = (result ^ 0xffffffffUL) + 1;
		result &= (1 << prec) - 1;
	} else {
		result = n;
	}
	return result;
}

static int getint(jas_stream_t *in, bool sgnd, unsigned prec, long *val)
{
	long v;
	int c;
	assert((!sgnd && prec >= 1) || (sgnd && prec >= 2));
	v = 0;
	for (unsigned n = (prec + 7) / 8; n-- > 0;) {
		if ((c = jas_stream_getc(in)) == EOF) {
			return -1;
		}
		v = (v << 8) | c;
	}
	v &= ((1 << prec) - 1);
	if (sgnd) {
		*val = decode_twos_comp(v, prec);
	} else {
		*val = v;
	}
	return 0;
}

static int putint(jas_stream_t *out, bool sgnd, unsigned prec, long val)
{
	int c;
	assert((!sgnd && prec >= 1) || (sgnd && prec >= 2));
	if (sgnd) {
		val = encode_twos_comp(val, prec);
	}
	assert(val >= 0);
	val &= (1 << prec) - 1;
	for (unsigned n = (prec + 7) / 8; n-- > 0;) {
		c = (val >> (n * 8)) & 0xff;
		if (jas_stream_putc(out, c) != c) {
			return -1;
		}
	}
	return 0;
}

static long convert(long val, bool oldsgnd, unsigned oldprec, bool newsgnd,
  unsigned newprec)
{
	/* TODO: The following code looks suspicious (i.e., an empty if body). */
	if (newsgnd != oldsgnd) {
	}
	if (newprec != oldprec) {
		if (newprec > oldprec) {
			val <<= newprec - oldprec;
		} else if (oldprec > newprec) {
			val >>= oldprec - newprec;
		}
	}
	return val;
}

static long downtomult(long x, long y)
{
	assert(x >= 0);
	return (x / y) * y;
}

static long uptomult(long x, long y)
{
	assert(x >= 0);
	return ((x + y - 1) / y) * y;
}

jas_image_t *jas_image_chclrspc(jas_image_t *image,
  const jas_cmprof_t *outprof, jas_cmxform_intent_t intent)
{
	jas_image_t *inimage;
	int k;
	jas_image_t *outimage;
	jas_cmprof_t *inprof;
	jas_cmprof_t *tmpprof;
	jas_image_cmptparm_t cmptparm;
	jas_cmxform_t *xform;
	jas_cmpixmap_t inpixmap;
	jas_cmpixmap_t outpixmap;
	jas_cmcmptfmt_t *incmptfmts;
	jas_cmcmptfmt_t *outcmptfmts;

#if 0
	jas_eprintf("IMAGE\n");
	jas_image_dump(image, stderr);
#endif

	if (image->numcmpts_ == 0) {
		/*
		can't work with a file with no components;
		continuing would crash because we'd attempt to
		obtain information about the first component
		*/
		return 0;
	}

	outimage = 0;
	xform = 0;
	if (!(inimage = jas_image_copy(image))) {
		goto error;
	}
	image = 0;

	if (!jas_image_ishomosamp(inimage)) {
		unsigned minhstep = jas_image_cmpthstep(inimage, 0);
		unsigned minvstep = jas_image_cmptvstep(inimage, 0);
		for (unsigned i = 1; i < jas_image_numcmpts(inimage); ++i) {
			const unsigned hstep = jas_image_cmpthstep(inimage, i);
			const unsigned vstep = jas_image_cmptvstep(inimage, i);
			if (hstep < minhstep) {
				minhstep = hstep;
			}
			if (vstep < minvstep) {
				minvstep = vstep;
			}
		}
		unsigned n = jas_image_numcmpts(inimage);
		for (unsigned i = 0; i < n; ++i) {
			if (jas_image_sampcmpt(inimage, i, i + 1, 0, 0, minhstep, minvstep,
			  jas_image_cmptsgnd(inimage, i), jas_image_cmptprec(inimage,
			  i))) {
				goto error;
			}
			const jas_image_cmpttype_t cmpttype = jas_image_cmpttype(inimage,
			  i);
			jas_image_setcmpttype(inimage, i + 1, cmpttype);
			jas_image_delcmpt(inimage, i);
		}
	}

	const unsigned width = jas_image_cmptwidth(inimage, 0);
	const unsigned height = jas_image_cmptheight(inimage, 0);
	const unsigned hstep = jas_image_cmpthstep(inimage, 0);
	const unsigned vstep = jas_image_cmptvstep(inimage, 0);

	if (!(inprof = jas_image_cmprof(inimage))) {
		// formerly call to abort()
		goto error;
	}
	const unsigned numinclrchans =
	  jas_clrspc_numchans(jas_cmprof_clrspc(inprof));
	const unsigned numoutclrchans =
	  jas_clrspc_numchans(jas_cmprof_clrspc(outprof));
	const unsigned prec = 8;

	if (!(outimage = jas_image_create0())) {
		goto error;
	}

	/* Create a component for each of the colorants. */
	for (unsigned i = 0; i < numoutclrchans; ++i) {
		cmptparm.tlx = 0;
		cmptparm.tly = 0;
		cmptparm.hstep = hstep;
		cmptparm.vstep = vstep;
		cmptparm.width = width;
		cmptparm.height = height;
		cmptparm.prec = prec;
		cmptparm.sgnd = 0;
		if (jas_image_addcmpt(outimage, -1, &cmptparm)) {
			goto error;
		}
		jas_image_setcmpttype(outimage, i, JAS_IMAGE_CT_COLOR(i));
	}
#if 0
	/* Copy the auxiliary components without modification. */
	for (i = 0; i < jas_image_numcmpts(inimage); ++i) {
		if (!ISCOLOR(jas_image_cmpttype(inimage, i))) {
			jas_image_copycmpt(outimage, -1, inimage, i);
/* XXX - need to specify laydown of component on ref. grid */
		}
	}
#endif

	if (!(tmpprof = jas_cmprof_copy(outprof))) {
		goto error;
	}
	assert(!jas_image_cmprof(outimage));
	jas_image_setcmprof(outimage, tmpprof);
	tmpprof = 0;
	jas_image_setclrspc(outimage, jas_cmprof_clrspc(outprof));

	if (!(xform = jas_cmxform_create(inprof, outprof, 0, JAS_CMXFORM_OP_FWD,
	  intent, JAS_CMXFORM_OPTM_SPEED))) {
		goto error;
	}

	inpixmap.numcmpts = numinclrchans;
	if (!(incmptfmts = jas_alloc2(numinclrchans, sizeof(jas_cmcmptfmt_t)))) {
		// formerly call to abort()
		goto error;
	}
	inpixmap.cmptfmts = incmptfmts;
	for (unsigned i = 0; i < numinclrchans; ++i) {
		const int j = jas_image_getcmptbytype(inimage, JAS_IMAGE_CT_COLOR(i));
		if (!(incmptfmts[i].buf = jas_alloc2(width, sizeof(long)))) {
			goto error;
		}
		incmptfmts[i].prec = jas_image_cmptprec(inimage, j);
		incmptfmts[i].sgnd = jas_image_cmptsgnd(inimage, j);
		incmptfmts[i].width = width;
		incmptfmts[i].height = 1;
	}

	outpixmap.numcmpts = numoutclrchans;
	if (!(outcmptfmts = jas_alloc2(numoutclrchans, sizeof(jas_cmcmptfmt_t)))) {
		// formerly call to abort()
		goto error;
	}
	outpixmap.cmptfmts = outcmptfmts;

	for (unsigned i = 0; i < numoutclrchans; ++i) {
		const int j = jas_image_getcmptbytype(outimage, JAS_IMAGE_CT_COLOR(i));
		if (!(outcmptfmts[i].buf = jas_alloc2(width, sizeof(long)))) {
			goto error;
		}
		outcmptfmts[i].prec = jas_image_cmptprec(outimage, j);
		outcmptfmts[i].sgnd = jas_image_cmptsgnd(outimage, j);
		outcmptfmts[i].width = width;
		outcmptfmts[i].height = 1;
	}

	for (unsigned i = 0; i < height; ++i) {
		for (unsigned j = 0; j < numinclrchans; ++j) {
			k = jas_image_getcmptbytype(inimage, JAS_IMAGE_CT_COLOR(j));
			if (jas_image_readcmpt2(inimage, k, 0, i, width, 1,
			  incmptfmts[j].buf)) {
				goto error;
			}
		}
		jas_cmxform_apply(xform, &inpixmap, &outpixmap);
		for (unsigned j = 0; j < numoutclrchans; ++j) {
			k = jas_image_getcmptbytype(outimage, JAS_IMAGE_CT_COLOR(j));
			if (jas_image_writecmpt2(outimage, k, 0, i, width, 1,
			  outcmptfmts[j].buf)) {
				goto error;
			}
		}
	}

	for (unsigned i = 0; i < numoutclrchans; ++i) {
		jas_free(outcmptfmts[i].buf);
	}
	jas_free(outcmptfmts);
	for (unsigned i = 0; i < numinclrchans; ++i) {
		jas_free(incmptfmts[i].buf);
	}
	jas_free(incmptfmts);
	jas_cmxform_destroy(xform);
	jas_image_destroy(inimage);

#if 0
	jas_eprintf("INIMAGE\n");
	jas_image_dump(inimage, stderr);
	jas_eprintf("OUTIMAGE\n");
	jas_image_dump(outimage, stderr);
#endif
	return outimage;
error:
	if (xform) {
		jas_cmxform_destroy(xform);
	}
	if (inimage) {
		jas_image_destroy(inimage);
	}
	if (outimage) {
		jas_image_destroy(outimage);
	}
	return 0;
}
