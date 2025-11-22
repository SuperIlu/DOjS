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

#include "mif_cod.h"

#include "jasper/jas_cm.h"
#include "jasper/jas_tvp.h"
#include "jasper/jas_stream.h"
#include "jasper/jas_image.h"
#include "jasper/jas_seq.h"
#include "jasper/jas_string.h"
#include "jasper/jas_malloc.h"
#include "jasper/jas_debug.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/******************************************************************************\
* Local types.
\******************************************************************************/

typedef enum {
	MIF_END = 0,
	MIF_CMPT
} mif_tagid2_t;

typedef enum {
	MIF_TLX = 0,
	MIF_TLY,
	MIF_WIDTH,
	MIF_HEIGHT,
	MIF_HSAMP,
	MIF_VSAMP,
	MIF_PREC,
	MIF_SGND,
	MIF_DATA
} mif_tagid_t;

/******************************************************************************\
* Local functions.
\******************************************************************************/

static mif_hdr_t *mif_hdr_create(int maxcmpts);
static void mif_hdr_destroy(mif_hdr_t *hdr);
static int mif_hdr_growcmpts(mif_hdr_t *hdr, int maxcmpts);
static mif_hdr_t *mif_hdr_get(jas_stream_t *in);
static int mif_process_cmpt(mif_hdr_t *hdr, char *buf);
static int mif_hdr_put(mif_hdr_t *hdr, jas_stream_t *out);
static int mif_hdr_addcmpt(mif_hdr_t *hdr, int cmptno, mif_cmpt_t *cmpt);
static mif_cmpt_t *mif_cmpt_create(void);
static void mif_cmpt_destroy(mif_cmpt_t *cmpt);
static char *mif_getline(jas_stream_t *jas_stream, char *buf, int bufsize);
static int mif_getc(jas_stream_t *in);
static mif_hdr_t *mif_makehdrfromimage(jas_image_t *image);

/******************************************************************************\
* Local data.
\******************************************************************************/

static const jas_taginfo_t mif_tags2[] = {
	{MIF_CMPT, "component"},
	{MIF_END, "end"},
	{-1, 0}
};

static const jas_taginfo_t mif_tags[] = {
	{MIF_TLX, "tlx"},
	{MIF_TLY, "tly"},
	{MIF_WIDTH, "width"},
	{MIF_HEIGHT, "height"},
	{MIF_HSAMP, "sampperx"},
	{MIF_VSAMP, "samppery"},
	{MIF_PREC, "prec"},
	{MIF_SGND, "sgnd"},
	{MIF_DATA, "data"},
	{-1, 0}
};

/******************************************************************************\
* Code for load operation.
\******************************************************************************/

/* Load an image from a stream in the MIF format. */

jas_image_t *mif_decode(jas_stream_t *in, const char *optstr)
{
	mif_hdr_t *hdr;
	jas_image_t *image;
	jas_image_t *tmpimage;
	jas_stream_t *tmpstream;
	int cmptno;
	mif_cmpt_t *cmpt;
	jas_image_cmptparm_t cmptparm;
	jas_seq2d_t *data;
	int_fast32_t x;
	int_fast32_t y;
	int bias;

	JAS_LOGDEBUGF(10, "mif_decode(%p, \"%s\")\n", in, (optstr ? optstr : ""));

	JAS_UNUSED(optstr);

	hdr = 0;
	image = 0;
	tmpimage = 0;
	tmpstream = 0;
	data = 0;

	JAS_LOGDEBUGF(10, "getting MIF header\n");
	if (!(hdr = mif_hdr_get(in))) {
		jas_logerrorf("cannot get MIF header\n");
		goto error;
	}

	if (!(image = jas_image_create0())) {
		jas_logerrorf("cannot create image\n");
		goto error;
	}

	for (cmptno = 0; cmptno < hdr->numcmpts; ++cmptno) {
		JAS_LOGDEBUGF(10, "processing component %d of %d\n", cmptno,
		  hdr->numcmpts);
		cmpt = hdr->cmpts[cmptno];
		tmpstream = cmpt->data ? jas_stream_fopen(cmpt->data, "rb") : in;
		if (!tmpstream) {
			jas_logerrorf("cannot open component file %s\n", cmpt->data);
			goto error;
		}
		JAS_LOGDEBUGF(10, "decoding component %d\n", cmptno);
		if (!(tmpimage = jas_image_decode(tmpstream, -1, "allow_trunc=1"))) {
			jas_logerrorf("cannot decode image\n");
			goto error;
		}
		if (tmpstream != in) {
			jas_stream_close(tmpstream);
			tmpstream = 0;
		}
		if (!cmpt->width) {
			cmpt->width = jas_image_cmptwidth(tmpimage, 0);
		}
		if (!cmpt->height) {
			cmpt->height = jas_image_cmptwidth(tmpimage, 0);
		}
		if (!cmpt->prec) {
			cmpt->prec = jas_image_cmptprec(tmpimage, 0);
		}
		if (cmpt->sgnd < 0) {
			cmpt->sgnd = jas_image_cmptsgnd(tmpimage, 0);
		}
		cmptparm.tlx = cmpt->tlx;
		cmptparm.tly = cmpt->tly;
		cmptparm.hstep = cmpt->sampperx;
		cmptparm.vstep = cmpt->samppery;
		cmptparm.width = cmpt->width;
		cmptparm.height = cmpt->height;
		cmptparm.prec = cmpt->prec;
		cmptparm.sgnd = cmpt->sgnd;
		JAS_LOGDEBUGF(10, "adding component %d\n", cmptno);
		if (jas_image_addcmpt(image, jas_image_numcmpts(image), &cmptparm)) {
			jas_logerrorf("cannot add component\n");
			goto error;
		}
		JAS_LOGDEBUGF(10, "copying component %d\n", cmptno);
		if (!(data = jas_seq2d_create(0, 0, cmpt->width, cmpt->height))) {
			jas_logerrorf("cannot create sequence\n");
			goto error;
		}
		JAS_LOGDEBUGF(10, "reading component %d\n", cmptno);
		if (jas_image_readcmpt(tmpimage, 0, 0, 0, cmpt->width, cmpt->height,
		  data)) {
			jas_logerrorf("cannot read component\n");
			goto error;
		}
		if (cmpt->sgnd) {
			bias = 1 << (cmpt->prec - 1);
			for (y = 0; y < cmpt->height; ++y) {
				for (x = 0; x < cmpt->width; ++x) {
					*jas_seq2d_getref(data, x, y) -= bias;
				}
			}
		}
		JAS_LOGDEBUGF(10, "writing component %d\n", cmptno);
		if (jas_image_writecmpt(image, jas_image_numcmpts(image) - 1, 0, 0,
		  cmpt->width, cmpt->height, data)) {
			jas_logerrorf("cannot write component\n");
			goto error;
		}
		jas_seq2d_destroy(data);
		data = 0;
		jas_image_destroy(tmpimage);
		tmpimage = 0;
	}

	mif_hdr_destroy(hdr);
	hdr = 0;
	return image;

error:
	if (image) {
		jas_image_destroy(image);
	}
	if (hdr) {
		mif_hdr_destroy(hdr);
	}
	if (tmpstream && tmpstream != in) {
		jas_stream_close(tmpstream);
	}
	if (tmpimage) {
		jas_image_destroy(tmpimage);
	}
	if (data) {
		jas_seq2d_destroy(data);
	}
	return 0;
}

/******************************************************************************\
* Code for save operation.
\******************************************************************************/

/* Save an image to a stream in the the MIF format. */

int mif_encode(jas_image_t *image, jas_stream_t *out, const char *optstr)
{
	mif_hdr_t *hdr;
	jas_image_t *tmpimage;
	int fmt;
	int cmptno;
	mif_cmpt_t *cmpt;
	jas_image_cmptparm_t cmptparm;
	jas_seq2d_t *data;
	int_fast32_t x;
	int_fast32_t y;
	int bias;

	JAS_LOGDEBUGF(10, "mif_encode(%p, %p, \"%s\")\n", image, out,
	  (optstr ? optstr : ""));

	hdr = 0;
	tmpimage = 0;
	data = 0;

	if (optstr && *optstr != '\0') {
		jas_logwarnf("warning: ignoring unsupported options\n");
	}

	if ((fmt = jas_image_strtofmt("pnm")) < 0) {
		jas_logerrorf("error: PNM support required\n");
		goto error;
	}

	if (!(hdr = mif_makehdrfromimage(image))) {
		jas_logerrorf("cannot make MIF header\n");
		goto error;
	}
	if (mif_hdr_put(hdr, out)) {
		jas_logerrorf("cannot write MIF header\n");
		goto error;
	}

	/* Output component data. */
	for (cmptno = 0; cmptno < hdr->numcmpts; ++cmptno) {
		cmpt = hdr->cmpts[cmptno];
		if (!cmpt->data) {
			if (!(tmpimage = jas_image_create0())) {
				jas_logerrorf("cannot create image\n");
				goto error;
			}	
			cmptparm.tlx = 0;
			cmptparm.tly = 0;
			cmptparm.hstep = cmpt->sampperx;
			cmptparm.vstep = cmpt->samppery;
			cmptparm.width = cmpt->width;
			cmptparm.height = cmpt->height;
			cmptparm.prec = cmpt->prec;
			cmptparm.sgnd = false;
			if (jas_image_addcmpt(tmpimage, jas_image_numcmpts(tmpimage),
			  &cmptparm)) {
				jas_logerrorf("cannot add component\n");
				goto error;
			}
			jas_image_setclrspc(tmpimage, JAS_CLRSPC_SGRAY);
			jas_image_setcmpttype(tmpimage, 0,
			  JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_GRAY_Y));
			if (!(data = jas_seq2d_create(0, 0, cmpt->width, cmpt->height))) {
				jas_logerrorf("cannot create sequence\n");
				goto error;
			}
			if (jas_image_readcmpt(image, cmptno, 0, 0, cmpt->width,
			  cmpt->height, data)) {
				jas_logerrorf("cannot read component\n");
				goto error;
			}
			if (cmpt->sgnd) {
				bias = 1 << (cmpt->prec - 1);
				for (y = 0; y < cmpt->height; ++y) {
					for (x = 0; x < cmpt->width; ++x) {
						*jas_seq2d_getref(data, x, y) += bias;
					}
				}
			}
			if (jas_image_writecmpt(tmpimage, 0, 0, 0, cmpt->width,
			  cmpt->height, data)) {
				jas_logerrorf("cannot write component\n");
				goto error;
			}
			jas_seq2d_destroy(data);
			data = 0;
			if (jas_image_encode(tmpimage, out, fmt, 0)) {
				jas_logerrorf("cannot encode image\n");
				goto error;
			}
			jas_image_destroy(tmpimage);
			tmpimage = 0;
		}
	}

	mif_hdr_destroy(hdr);

	return 0;

error:
	if (hdr) {
		mif_hdr_destroy(hdr);
	}
	if (tmpimage) {
		jas_image_destroy(tmpimage);
	}
	if (data) {
		jas_seq2d_destroy(data);
	}
	return -1;
}

/******************************************************************************\
* Code for validate operation.
\******************************************************************************/

int mif_validate(jas_stream_t *in)
{
	jas_uchar buf[MIF_MAGICLEN];
	uint_fast32_t magic;

	JAS_LOGDEBUGF(10, "mif_validate(%p)\n", in);

	assert(JAS_STREAM_MAXPUTBACK >= MIF_MAGICLEN);

	/* Read the validation data (i.e., the data used for detecting
	  the format). */
	if (jas_stream_peek(in, buf, sizeof(buf)) != sizeof(buf)) {
		JAS_LOGDEBUGF(10, "mif_validate peek failed\n");
		return -1;
	}


	/* Compute the signature value. */
	magic = (JAS_CAST(uint_fast32_t, buf[0]) << 24) |
	  (JAS_CAST(uint_fast32_t, buf[1]) << 16) |
	  (JAS_CAST(uint_fast32_t, buf[2]) << 8) |
	  buf[3];

	/* Ensure that the signature is correct for this format. */
	if (magic != MIF_MAGIC) {
		JAS_LOGDEBUGF(10, "mif_validate magic mismatch %x %x\n", magic,
		  MIF_MAGIC);
		return -1;
	}

	return 0;
}

/******************************************************************************\
* Code for MIF header class.
\******************************************************************************/

static mif_hdr_t *mif_hdr_create(int maxcmpts)
{
	JAS_LOGDEBUGF(10, "mif_hdr_create(%d)\n", maxcmpts);

	mif_hdr_t *hdr;
	if (!(hdr = jas_malloc(sizeof(mif_hdr_t)))) {
		return 0;
	}
	hdr->numcmpts = 0;
	hdr->maxcmpts = 0;
	hdr->cmpts = 0;
	if (mif_hdr_growcmpts(hdr, maxcmpts)) {
		mif_hdr_destroy(hdr);
		return 0;
	}
	return hdr;
}

static void mif_hdr_destroy(mif_hdr_t *hdr)
{
	JAS_LOGDEBUGF(10, "mif_hdr_destroy(%p)\n", hdr);
	int cmptno;
	if (hdr->cmpts) {
		for (cmptno = 0; cmptno < hdr->numcmpts; ++cmptno) {
			mif_cmpt_destroy(hdr->cmpts[cmptno]);
		}
		jas_free(hdr->cmpts);
	}
	jas_free(hdr);
}

static int mif_hdr_growcmpts(mif_hdr_t *hdr, int maxcmpts)
{
	JAS_LOGDEBUGF(10, "mif_hdr_growcmpts(%p, %d)\n", hdr, maxcmpts);

	int cmptno;
	mif_cmpt_t **newcmpts;
	assert(maxcmpts >= hdr->numcmpts);
	newcmpts = (!hdr->cmpts) ? jas_alloc2(maxcmpts, sizeof(mif_cmpt_t *)) :
	  jas_realloc2(hdr->cmpts, maxcmpts, sizeof(mif_cmpt_t *));
	if (!newcmpts) {
		return -1;
	}
	hdr->maxcmpts = maxcmpts;
	hdr->cmpts = newcmpts;
	for (cmptno = hdr->numcmpts; cmptno < hdr->maxcmpts; ++cmptno) {
		hdr->cmpts[cmptno] = 0;
	}
	return 0;
}

static mif_hdr_t *mif_hdr_get(jas_stream_t *in)
{
	jas_uchar magicbuf[MIF_MAGICLEN];
	char buf[4096];
	mif_hdr_t *hdr;
	bool done;
	jas_tvparser_t *tvp;
	int id;

	JAS_LOGDEBUGF(10, "mif_hdr_get(%p)\n", in);

	hdr = 0;
	tvp = 0;

	if (jas_stream_read(in, magicbuf, MIF_MAGICLEN) != MIF_MAGICLEN) {
		jas_logerrorf("cannot read MIF signature\n");
		goto error;
	}
	if (magicbuf[0] != (MIF_MAGIC >> 24) || magicbuf[1] != ((MIF_MAGIC >> 16) &
	  0xff) || magicbuf[2] != ((MIF_MAGIC >> 8) & 0xff) || magicbuf[3] !=
	  (MIF_MAGIC & 0xff)) {
		jas_logerrorf("bad signature\n");
		goto error;
	}

	if (!(hdr = mif_hdr_create(0))) {
		jas_logerrorf("cannot create MIF header\n");
		goto error;
	}

	done = false;
	do {
		JAS_LOGDEBUGF(10, "top of loop\n");
		if (!mif_getline(in, buf, sizeof(buf))) {
			jas_logerrorf("mif_hdr_get: mif_getline failed\n");
			goto error;
		}
		if (buf[0] == '\0') {
			continue;
		}
		JAS_LOGDEBUGF(10, "header line: len=%d; %s\n", strlen(buf), buf);
		if (!(tvp = jas_tvparser_create(buf))) {
			jas_logerrorf("mif_hdr_get: jas_tvparser_create failed\n");
			goto error;
		}
		JAS_LOGDEBUGF(10, "mif_hdr_get: invoking jas_tvparser_next\n");
		if (jas_tvparser_next(tvp)) {
			jas_logerrorf("cannot get record type\n");
			goto error;
		}
		JAS_LOGDEBUGF(10, "mif_hdr_get: looking up tag\n");
		id = jas_taginfo_nonull(jas_taginfos_lookup(mif_tags2,
		  jas_tvparser_gettag(tvp)))->id;
		jas_tvparser_destroy(tvp);
		tvp = 0;
		switch (id) {
		case MIF_CMPT:
			if (mif_process_cmpt(hdr, buf)) {
				jas_logerrorf("cannot get component information\n");
				goto error;
			}
			break;
		case MIF_END:
			done = 1;
			break;
		default:
			jas_logerrorf("invalid header information: %s\n", buf);
			goto error;
			break;
		}
	} while (!done);

	JAS_LOGDEBUGF(10, "mif_hdr_get: returning (success)\n");
	return hdr;

error:
	JAS_LOGDEBUGF(10, "mif_hdr_get: returning (failure)\n");
	if (hdr) {
		mif_hdr_destroy(hdr);
	}
	if (tvp) {
		jas_tvparser_destroy(tvp);
	}
	return 0;
}

static int mif_process_cmpt(mif_hdr_t *hdr, char *buf)
{
	jas_tvparser_t *tvp;
	mif_cmpt_t *cmpt;
	int id;

	cmpt = 0;
	tvp = 0;

	JAS_LOGDEBUGF(10, "mif_process_cmpt(%p, %p)\n", hdr, buf);

	if (!(cmpt = mif_cmpt_create())) {
		jas_logerrorf("cannot create component\n");
		goto error;
	}
	cmpt->tlx = 0;
	cmpt->tly = 0;
	cmpt->sampperx = 0;
	cmpt->samppery = 0;
	cmpt->width = 0;
	cmpt->height = 0;
	cmpt->prec = 0;
	cmpt->sgnd = -1;
	cmpt->data = 0;

	if (!(tvp = jas_tvparser_create(buf))) {
		jas_logerrorf("cannot create parser\n");
		goto error;
	}

	// Skip the component keyword
	if ((id = jas_tvparser_next(tvp))) {
		// This should never happen.
		abort();
	}

	// Process the tag-value pairs.
	while (!(id = jas_tvparser_next(tvp))) {
		switch (jas_taginfo_nonull(jas_taginfos_lookup(mif_tags,
		  jas_tvparser_gettag(tvp)))->id) {
		case MIF_TLX:
			cmpt->tlx = atoi(jas_tvparser_getval(tvp));
			break;
		case MIF_TLY:
			cmpt->tly = atoi(jas_tvparser_getval(tvp));
			break;
		case MIF_WIDTH:
			cmpt->width = atoi(jas_tvparser_getval(tvp));
			break;
		case MIF_HEIGHT:
			cmpt->height = atoi(jas_tvparser_getval(tvp));
			break;
		case MIF_HSAMP:
			cmpt->sampperx = atoi(jas_tvparser_getval(tvp));
			break;
		case MIF_VSAMP:
			cmpt->samppery = atoi(jas_tvparser_getval(tvp));
			break;
		case MIF_PREC:
			cmpt->prec = atoi(jas_tvparser_getval(tvp));
			break;
		case MIF_SGND:
			cmpt->sgnd = atoi(jas_tvparser_getval(tvp));
			break;
		case MIF_DATA:
			if (!(cmpt->data = jas_strdup(jas_tvparser_getval(tvp)))) {
				goto error;
			}
			break;
		default:
			jas_logerrorf("invalid component information: %s\n", buf);
			goto error;
			break;
		}
	}
	if (!cmpt->sampperx || !cmpt->samppery) {
		goto error;
	}
	if (!cmpt->width || !cmpt->height || !cmpt->prec || cmpt->sgnd < 0) {
		goto error;
	}
	if (mif_hdr_addcmpt(hdr, hdr->numcmpts, cmpt)) {
		jas_logerrorf("cannot add component\n");
		goto error;
	}
	jas_tvparser_destroy(tvp);

	JAS_LOGDEBUGF(10, "mif_process_cmpt returning (success)\n");
	return 0;

error:
	if (cmpt) {
		mif_cmpt_destroy(cmpt);
	}
	if (tvp) {
		jas_tvparser_destroy(tvp);
	}
	JAS_LOGDEBUGF(10, "mif_process_cmpt returning (error)\n");
	return -1;
}

static int mif_hdr_put(mif_hdr_t *hdr, jas_stream_t *out)
{
	int cmptno;
	mif_cmpt_t *cmpt;

	JAS_LOGDEBUGF(10, "mif_hdr_put(%p, %p)\n", hdr, out);

	/* Output signature. */
	if (jas_stream_putc(out, (MIF_MAGIC >> 24) & 0xff) == EOF ||
	    jas_stream_putc(out, (MIF_MAGIC >> 16) & 0xff) == EOF ||
	    jas_stream_putc(out, (MIF_MAGIC >> 8) & 0xff) == EOF ||
	    jas_stream_putc(out, MIF_MAGIC & 0xff) == EOF)
		return -1;

	/* Output component information. */
	for (cmptno = 0; cmptno < hdr->numcmpts; ++cmptno) {
		cmpt = hdr->cmpts[cmptno];
		jas_stream_printf(out, "component tlx=%ld tly=%ld "
		  "sampperx=%ld samppery=%ld width=%ld height=%ld prec=%d sgnd=%d",
		  cmpt->tlx, cmpt->tly, cmpt->sampperx, cmpt->samppery, cmpt->width,
		  cmpt->height, cmpt->prec, cmpt->sgnd);
		if (cmpt->data) {
			jas_stream_printf(out, " data=%s", cmpt->data);
		}
		jas_stream_printf(out, "\n");
	}

	/* Output end of header indicator. */
	jas_stream_printf(out, "end\n");

	return 0;
}

static int mif_hdr_addcmpt(mif_hdr_t *hdr, int cmptno, mif_cmpt_t *cmpt)
{
	assert(cmptno >= hdr->numcmpts);

	JAS_LOGDEBUGF(10, "mif_hdr_addcmpt(%p, %d, %p)\n", hdr, cmptno, cmpt);
	JAS_UNUSED(cmptno);

	if (hdr->numcmpts >= hdr->maxcmpts) {
		if (mif_hdr_growcmpts(hdr, hdr->numcmpts + 128)) {
			return -1;
		}
	}
	hdr->cmpts[hdr->numcmpts] = cmpt;
	++hdr->numcmpts;
	return 0;
}

/******************************************************************************\
* Code for MIF component class.
\******************************************************************************/

static mif_cmpt_t *mif_cmpt_create()
{
	JAS_LOGDEBUGF(10, "mif_cmpt_create()\n");
	mif_cmpt_t *cmpt;
	if (!(cmpt = jas_malloc(sizeof(mif_cmpt_t)))) {
		return 0;
	}
	memset(cmpt, 0, sizeof(mif_cmpt_t));
	return cmpt;
}

static void mif_cmpt_destroy(mif_cmpt_t *cmpt)
{
	JAS_LOGDEBUGF(10, "mif_cmpt_destroy(%p)\n", cmpt);
	if (cmpt->data) {
		jas_free(cmpt->data);
	}
	jas_free(cmpt);
}

/******************************************************************************\
* MIF parsing code.
\******************************************************************************/

static char *mif_getline(jas_stream_t *stream, char *buf, int bufsize)
{
	int c;
	char *bufptr;
	assert(bufsize > 0);

	JAS_LOGDEBUGF(10, "mif_getline(%p, %p, %d)\n", stream, buf, bufsize);

	bufptr = buf;
	while (bufsize > 1) {
		if ((c = mif_getc(stream)) == EOF) {
			break;
		}
		*bufptr++ = c;
		--bufsize;
		if (c == '\n') {
			break;
		}
	}
	*bufptr = '\0';
	if (!(bufptr = strchr(buf, '\n'))) {
		return 0;
	}
	*bufptr = '\0';
	return buf;
}

static int mif_getc(jas_stream_t *in)
{
	int c;
	bool done;

	JAS_LOGDEBUGF(10, "mif_getc(%p)\n", in);

	done = false;
	do {
		switch (c = jas_stream_getc(in)) {
		case EOF:
			done = true;
			break;
		case '#':
			for (;;) {
				if ((c = jas_stream_getc(in)) == EOF) {
					done = true;
					break;
				}	
				if (c == '\n') {
					done = true;
					break;
				}
			}
			break;
		case '\\':
			if (jas_stream_peekc(in) == '\n') {
				jas_stream_getc(in);
			}
			break;
		default:
			done = true;
			break;
		}
	} while (!done);

	JAS_LOGDEBUGF(10, "mif_getc(%p) returning %d\n", in, c);

	return c;
}

/******************************************************************************\
* Miscellaneous functions.
\******************************************************************************/

static mif_hdr_t *mif_makehdrfromimage(jas_image_t *image)
{
	mif_hdr_t *hdr;
	int cmptno;
	mif_cmpt_t *cmpt;

	JAS_LOGDEBUGF(10, "mif_makehdrfromimage(%p)\n", image);

	if (!(hdr = mif_hdr_create(jas_image_numcmpts(image)))) {
		return 0;
	}
	hdr->magic = MIF_MAGIC;
	hdr->numcmpts = jas_image_numcmpts(image);
	for (cmptno = 0; cmptno < hdr->numcmpts; ++cmptno) {
		if (!(hdr->cmpts[cmptno] = jas_malloc(sizeof(mif_cmpt_t)))) {
			goto error;
		}
		cmpt = hdr->cmpts[cmptno];
		cmpt->tlx = jas_image_cmpttlx(image, cmptno);
		cmpt->tly = jas_image_cmpttly(image, cmptno);
		cmpt->width = jas_image_cmptwidth(image, cmptno);
		cmpt->height = jas_image_cmptheight(image, cmptno);
		cmpt->sampperx = jas_image_cmpthstep(image, cmptno);
		cmpt->samppery = jas_image_cmptvstep(image, cmptno);
		cmpt->prec = jas_image_cmptprec(image, cmptno);
		cmpt->sgnd = jas_image_cmptsgnd(image, cmptno);
		cmpt->data = 0;
	}
	return hdr;

error:
	for (cmptno = 0; cmptno < hdr->numcmpts; ++cmptno) {
		if (hdr->cmpts[cmptno]) {
			jas_free(hdr->cmpts[cmptno]);
		}
	}
	if (hdr) {
		jas_free(hdr);
	}
	return 0;
}
