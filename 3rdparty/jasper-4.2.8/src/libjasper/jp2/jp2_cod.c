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
 * JP2 Library
 *
 * $Id$
 */

/******************************************************************************\
* Includes.
\******************************************************************************/

#include "jp2_cod.h"

#include "jasper/jas_malloc.h"
#include "jasper/jas_debug.h"
#include "jasper/jas_types.h"

#include <assert.h>
#include <string.h>
#include <limits.h>

/******************************************************************************\
* Function prototypes.
\******************************************************************************/

static const jp2_boxinfo_t *jp2_boxinfolookup(int type);

static int jp2_getuint8(jas_stream_t *in, uint_fast8_t *val);
static int jp2_getuint16(jas_stream_t *in, uint_fast16_t *val);
static int jp2_getuint32(jas_stream_t *in, uint_fast32_t *val);
static int jp2_getuint64(jas_stream_t *in, uint_fast64_t *val);
static int jp2_putuint8(jas_stream_t *out, uint_fast8_t val);
static int jp2_putuint16(jas_stream_t *out, uint_fast16_t val);
static int jp2_putuint32(jas_stream_t *out, uint_fast32_t val);
static int jp2_putuint64(jas_stream_t *out, uint_fast64_t val);

static int jp2_getint(jas_stream_t *in, int s, int n, int_fast32_t *val);

static void jp2_box_dump(const jp2_box_t *box);

static int jp2_jp_getdata(jp2_box_t *box, jas_stream_t *in);
static int jp2_jp_putdata(const jp2_box_t *box, jas_stream_t *out);
static int jp2_ftyp_getdata(jp2_box_t *box, jas_stream_t *in);
static int jp2_ftyp_putdata(const jp2_box_t *box, jas_stream_t *out);
static int jp2_ihdr_getdata(jp2_box_t *box, jas_stream_t *in);
static int jp2_ihdr_putdata(const jp2_box_t *box, jas_stream_t *out);
static void jp2_bpcc_destroy(jp2_box_t *box);
static int jp2_bpcc_getdata(jp2_box_t *box, jas_stream_t *in);
static int jp2_bpcc_putdata(const jp2_box_t *box, jas_stream_t *out);
static int jp2_colr_getdata(jp2_box_t *box, jas_stream_t *in);
static int jp2_colr_putdata(const jp2_box_t *box, jas_stream_t *out);
static void jp2_colr_dumpdata(const jp2_box_t *box);
static void jp2_colr_destroy(jp2_box_t *box);
static void jp2_cdef_destroy(jp2_box_t *box);
static int jp2_cdef_getdata(jp2_box_t *box, jas_stream_t *in);
static int jp2_cdef_putdata(const jp2_box_t *box, jas_stream_t *out);
static void jp2_cdef_dumpdata(const jp2_box_t *box);
static void jp2_cmap_destroy(jp2_box_t *box);
static int jp2_cmap_getdata(jp2_box_t *box, jas_stream_t *in);
static int jp2_cmap_putdata(const jp2_box_t *box, jas_stream_t *out);
static void jp2_cmap_dumpdata(const jp2_box_t *box);
static void jp2_pclr_destroy(jp2_box_t *box);
static int jp2_pclr_getdata(jp2_box_t *box, jas_stream_t *in);
static int jp2_pclr_putdata(const jp2_box_t *box, jas_stream_t *out);
static void jp2_pclr_dumpdata(const jp2_box_t *box);

/******************************************************************************\
\******************************************************************************/

static inline uint_fast32_t ones(int n)
{
	assert(n >= 0);
	return (JAS_CAST(uint_fast32_t, 1) << n) - 1;
}

/******************************************************************************\
* Local data.
\******************************************************************************/

static const jp2_boxinfo_t jp2_boxinfos[] = {
	{JP2_BOX_JP, 0, "JP",
	  {0, 0, jp2_jp_getdata, jp2_jp_putdata, 0}},
	{JP2_BOX_FTYP, 0, "FTYP",
	  {0, 0, jp2_ftyp_getdata, jp2_ftyp_putdata, 0}},
	{JP2_BOX_JP2H, JP2_BOX_SUPER, "JP2H",
	  {0, 0, 0, 0, 0}},
	{JP2_BOX_IHDR, 0, "IHDR",
	  {0, 0, jp2_ihdr_getdata, jp2_ihdr_putdata, 0}},
	{JP2_BOX_BPCC, 0, "BPCC",
	  {0, jp2_bpcc_destroy, jp2_bpcc_getdata, jp2_bpcc_putdata, 0}},
	{JP2_BOX_COLR, 0, "COLR",
	  {0, jp2_colr_destroy, jp2_colr_getdata, jp2_colr_putdata, jp2_colr_dumpdata}},
	{JP2_BOX_PCLR, 0, "PCLR",
	  {0, jp2_pclr_destroy, jp2_pclr_getdata, jp2_pclr_putdata, jp2_pclr_dumpdata}},
	{JP2_BOX_CMAP, 0, "CMAP",
	  {0, jp2_cmap_destroy, jp2_cmap_getdata, jp2_cmap_putdata, jp2_cmap_dumpdata}},
	{JP2_BOX_CDEF, 0, "CDEF",
	  {0, jp2_cdef_destroy, jp2_cdef_getdata, jp2_cdef_putdata, jp2_cdef_dumpdata}},
	{JP2_BOX_RES, JP2_BOX_SUPER, "RES",
	  {0, 0, 0, 0, 0}},
	{JP2_BOX_RESC, 0, "RESC",
	  {0, 0, 0, 0, 0}},
	{JP2_BOX_RESD, 0, "RESD",
	  {0, 0, 0, 0, 0}},
	{JP2_BOX_JP2C, JP2_BOX_NODATA, "JP2C",
	  {0, 0, 0, 0, 0}},
	{JP2_BOX_JP2I, 0, "JP2I",
	  {0, 0, 0, 0, 0}},
	{JP2_BOX_XML, 0, "XML",
	  {0, 0, 0, 0, 0}},
	{JP2_BOX_UUID, 0, "UUID",
	  {0, 0, 0, 0, 0}},
	{JP2_BOX_UINF, JP2_BOX_SUPER, "UINF",
	  {0, 0, 0, 0, 0}},
	{JP2_BOX_ULST, 0, "ULST",
	  {0, 0, 0, 0, 0}},
	{JP2_BOX_URL, 0, "URL",
	  {0, 0, 0, 0, 0}},
	{0, 0, 0, {0, 0, 0, 0, 0}},
};

static const jp2_boxinfo_t jp2_boxinfo_unk = {
	0, 0, "Unknown", {0, 0, 0, 0, 0}
};

/******************************************************************************\
* Box constructor.
\******************************************************************************/

static jp2_box_t *jp2_box_create0(void)
{
	jp2_box_t *box;
	if (!(box = jas_malloc(sizeof(jp2_box_t)))) {
		return 0;
	}
	memset(box, 0, sizeof(jp2_box_t));
	// Mark the box data as never having been constructed
	// so that we will not errantly attempt to destroy it later.
	box->ops = &jp2_boxinfo_unk.ops;
	return box;
}

jp2_box_t *jp2_box_create(int type)
{
	const jp2_boxinfo_t *boxinfo = jp2_boxinfolookup(type);
	assert(boxinfo != NULL);
	if (boxinfo == &jp2_boxinfo_unk) {
		// on error, jp2_boxinfolookup() returns &jp2_boxinfo_unk
		return NULL;
	}

	jp2_box_t *box;
	if (!(box = jp2_box_create0())) {
		return 0;
	}
	box->type = type;
	box->len = 0;
	box->info = boxinfo;
	box->ops = &boxinfo->ops;
	return box;
}

/******************************************************************************\
* Box destructor.
\******************************************************************************/

void jp2_box_destroy(jp2_box_t *box)
{
	if (box->ops->destroy) {
		(*box->ops->destroy)(box);
	}
	jas_free(box);
}

static void jp2_bpcc_destroy(jp2_box_t *box)
{
	jp2_bpcc_t *bpcc = &box->data.bpcc;
	if (bpcc->bpcs) {
		jas_free(bpcc->bpcs);
		bpcc->bpcs = 0;
	}
}

static void jp2_cdef_destroy(jp2_box_t *box)
{
	jp2_cdef_t *cdef = &box->data.cdef;
	if (cdef->ents) {
		jas_free(cdef->ents);
		cdef->ents = 0;
	}
}

/******************************************************************************\
* Box input.
\******************************************************************************/

jp2_box_t *jp2_box_get(jas_stream_t *in)
{
	jp2_box_t *box;
	const jp2_boxinfo_t *boxinfo;
	jas_stream_t *tmpstream;
	uint_fast32_t len;
	uint_fast64_t extlen;
	bool dataflag;

	box = 0;
	tmpstream = 0;

	if (!(box = jp2_box_create0())) {
		goto error;
	}
	if (jp2_getuint32(in, &len) || jp2_getuint32(in, &box->type)) {
		goto error;
	}
	boxinfo = jp2_boxinfolookup(box->type);
	box->info = boxinfo;
	box->len = len;
	JAS_LOGDEBUGF(10,
	  "preliminary processing of JP2 box: "
	  "type=%c%s%c (0x%08x); length=%"PRIuFAST32"\n",
	  '"', boxinfo->name, '"', box->type, box->len
	  );
	size_t hdrlen;
	if (box->len == 1) {
		JAS_LOGDEBUGF(10, "big length\n");
		if (jp2_getuint64(in, &extlen)) {
			goto error;
		}
		if (extlen > 0xffffffffUL) {
			jas_logerrorf("cannot handle large 64-bit box length\n");
			goto error;
		}
		box->len = extlen;
		hdrlen = JP2_BOX_HDRLEN(true);
	} else {
		hdrlen = JP2_BOX_HDRLEN(false);
	}
	if (box->len != 0 && box->len < 8) {
		goto error;
	}
	if (box->len > SSIZE_MAX) {
		/* This limit is the largest value which can be passed to
		  jas_stream_copy() without overflowing. */
		goto error;
	}

	dataflag = !(box->info->flags & (JP2_BOX_SUPER | JP2_BOX_NODATA));

	if (dataflag) {
		if (box->len < hdrlen)
			goto error;
		box->datalen = box->len - hdrlen;

		if (!(tmpstream = jas_stream_memopen(0, 0))) {
			goto error;
		}
		if (jas_stream_copy(tmpstream, in, box->datalen)) {
			jas_logerrorf("cannot copy box data\n");
			goto error;
		}
		jas_stream_rewind(tmpstream);

		// From here onwards, the box data will need to be destroyed.
		// So, initialize the box operations.
		box->ops = &boxinfo->ops;

		if (box->ops->getdata) {
			if ((*box->ops->getdata)(box, tmpstream)) {
				jas_logerrorf("cannot parse box data\n");
				goto error;
			}
		}
		jas_stream_close(tmpstream);
	}

	if (jas_get_debug_level() >= 1) {
		jp2_box_dump(box);
	}

	return box;

error:
	if (box) {
		jp2_box_destroy(box);
	}
	if (tmpstream) {
		jas_stream_close(tmpstream);
	}
	return 0;
}

static void jp2_box_dump(const jp2_box_t *box)
{
	const jp2_boxinfo_t *boxinfo = box->info;
	assert(boxinfo);

	jas_logprintf(
	  "JP2 box: type=%c%s%c (0x%08"PRIxFAST32"); length=%"PRIuFAST32"\n", '"',
	  boxinfo->name, '"', box->type, box->len);
	if (box->ops->dumpdata) {
		(*box->ops->dumpdata)(box);
	}
}

static int jp2_jp_getdata(jp2_box_t *box, jas_stream_t *in)
{
	jp2_jp_t *jp = &box->data.jp;
	if (jp2_getuint32(in, &jp->magic)) {
		return -1;
	}
	return 0;
}

static int jp2_ftyp_getdata(jp2_box_t *box, jas_stream_t *in)
{
	if (box->datalen < 8)
		return -1;

	jp2_ftyp_t *ftyp = &box->data.ftyp;
	unsigned int i;
	if (jp2_getuint32(in, &ftyp->majver) || jp2_getuint32(in, &ftyp->minver)) {
		return -1;
	}
	ftyp->numcompatcodes = (box->datalen - 8) / 4;
	if (ftyp->numcompatcodes > JP2_FTYP_MAXCOMPATCODES) {
		return -1;
	}
	for (i = 0; i < ftyp->numcompatcodes; ++i) {
		if (jp2_getuint32(in, &ftyp->compatcodes[i])) {
			return -1;
		}
	}
	return 0;
}

static int jp2_ihdr_getdata(jp2_box_t *box, jas_stream_t *in)
{
	jp2_ihdr_t *ihdr = &box->data.ihdr;
	if (jp2_getuint32(in, &ihdr->height) || jp2_getuint32(in, &ihdr->width) ||
	  jp2_getuint16(in, &ihdr->numcmpts) || jp2_getuint8(in, &ihdr->bpc) ||
	  jp2_getuint8(in, &ihdr->comptype) || jp2_getuint8(in, &ihdr->csunk) ||
	  jp2_getuint8(in, &ihdr->ipr)) {
		return -1;
	}
	return 0;
}

static int jp2_bpcc_getdata(jp2_box_t *box, jas_stream_t *in)
{
	if (box->datalen > 0xffff)
		/* excessive number of components - this is a
		   pessimistic limit, because in jp2_ihdr_getdata(),
		   it's a 16 bit integer */
		return -1;

	jp2_bpcc_t *bpcc = &box->data.bpcc;
	unsigned int i;
	bpcc->bpcs = 0;
	bpcc->numcmpts = box->datalen;
	if (!(bpcc->bpcs = jas_alloc2(bpcc->numcmpts, sizeof(uint_fast8_t)))) {
		return -1;
	}
	for (i = 0; i < bpcc->numcmpts; ++i) {
		if (jp2_getuint8(in, &bpcc->bpcs[i])) {
			return -1;
		}
	}
	return 0;
}

static void jp2_colr_dumpdata(const jp2_box_t *box)
{
	const jp2_colr_t *colr = &box->data.colr;
	jas_logprintf("method=%d; pri=%d; approx=%d\n", (int)colr->method, (int)colr->pri, (int)colr->approx);
	switch (colr->method) {
	case JP2_COLR_ENUM:
		jas_logprintf("csid=%d\n", (int)colr->csid);
		break;
	case JP2_COLR_ICC:
		jas_logmemdump(colr->iccp, colr->iccplen);
		break;
	}
}

static int jp2_colr_getdata(jp2_box_t *box, jas_stream_t *in)
{
	if (box->datalen < 3)
		return -1;

	jp2_colr_t *colr = &box->data.colr;
	colr->csid = 0;
	colr->iccp = 0;
	colr->iccplen = 0;

	if (jp2_getuint8(in, &colr->method) || jp2_getuint8(in, &colr->pri) ||
	  jp2_getuint8(in, &colr->approx)) {
		jas_logerrorf("cannot get COLR box data\n");
		return -1;
	}
	switch (colr->method) {
	case JP2_COLR_ENUM:
		if (jp2_getuint32(in, &colr->csid)) {
			jas_logerrorf("cannot get CSID\n");
			return -1;
		}
		break;
	case JP2_COLR_ICC:
		if (box->datalen <= 3) {
			jas_logerrorf("empty ICC profile data\n");
			return -1;
		}
		assert(box->datalen >= 3);
		colr->iccplen = box->datalen - 3;
#if 0
		if (colr->iccplen > 1024 * 1024) {
			/* refuse to read ICC profiles larger than 1
			   MB (I have no idea how large ICC profiles
			   can get, but I believe this limit might be
			   very pessimistic and should be lowered
			   further) */
			return -1;
		}
#endif
		if (!(colr->iccp = jas_alloc2(colr->iccplen, sizeof(uint_fast8_t)))) {
			return -1;
		}
		if (jas_stream_read(in, colr->iccp, colr->iccplen) != colr->iccplen) {
			return -1;
		}
		break;
	}
	return 0;
}

static void jp2_cdef_dumpdata(const jp2_box_t *box)
{
	const jp2_cdef_t *cdef = &box->data.cdef;
	unsigned int i;
	for (i = 0; i < cdef->numchans; ++i) {
		jas_logprintf(
		  "channo=%"PRIuFAST16"; type=%"PRIuFAST16"; assoc=%"PRIuFAST16"\n",
		  cdef->ents[i].channo, cdef->ents[i].type, cdef->ents[i].assoc);
	}
}

static void jp2_colr_destroy(jp2_box_t *box)
{
	jp2_colr_t *colr = &box->data.colr;
	if (colr->iccp) {
		jas_free(colr->iccp);
	}
}

static int jp2_cdef_getdata(jp2_box_t *box, jas_stream_t *in)
{
	jp2_cdef_t *cdef = &box->data.cdef;
	jp2_cdefchan_t *chan;
	unsigned int channo;
	cdef->ents = 0;
	if (jp2_getuint16(in, &cdef->numchans)) {
		return -1;
	}
	if (!(cdef->ents = jas_alloc2(cdef->numchans, sizeof(jp2_cdefchan_t)))) {
		return -1;
	}
	for (channo = 0; channo < cdef->numchans; ++channo) {
		chan = &cdef->ents[channo];
		if (jp2_getuint16(in, &chan->channo) || jp2_getuint16(in, &chan->type) ||
		  jp2_getuint16(in, &chan->assoc)) {
			return -1;
		}
	}
	return 0;
}

/******************************************************************************\
* Box output.
\******************************************************************************/

int jp2_box_put(jp2_box_t *box, jas_stream_t *out)
{
	jas_stream_t *tmpstream;
	bool extlen;
	bool dataflag;

	tmpstream = 0;

	dataflag = !(box->info->flags & (JP2_BOX_SUPER | JP2_BOX_NODATA));

	if (dataflag) {
		if (!(tmpstream = jas_stream_memopen(0, 0))) {
			goto error;
		}
		if (box->ops->putdata) {
			if ((*box->ops->putdata)(box, tmpstream)) {
				goto error;
			}
		}
		box->len = jas_stream_tell(tmpstream) + JP2_BOX_HDRLEN(false);
		jas_stream_rewind(tmpstream);
	}
	extlen = (box->len >= (((uint_fast64_t)1) << 32)) != 0;
	if (jp2_putuint32(out, extlen ? 1 : box->len)) {
		goto error;
	}
	if (jp2_putuint32(out, box->type)) {
		goto error;
	}
	if (extlen) {
		if (jp2_putuint64(out, box->len)) {
			goto error;
		}
	}

	if (dataflag) {
		if (jas_stream_copy(out, tmpstream, box->len -
		  JP2_BOX_HDRLEN(false))) {
			jas_logerrorf("cannot copy box data\n");
			goto error;
		}
		jas_stream_close(tmpstream);
	}

	return 0;

error:

	if (tmpstream) {
		jas_stream_close(tmpstream);
	}
	return -1;
}

static int jp2_jp_putdata(const jp2_box_t *box, jas_stream_t *out)
{
	const jp2_jp_t *jp = &box->data.jp;
	if (jp2_putuint32(out, jp->magic)) {
		return -1;
	}
	return 0;
}

static int jp2_ftyp_putdata(const jp2_box_t *box, jas_stream_t *out)
{
	const jp2_ftyp_t *ftyp = &box->data.ftyp;
	unsigned int i;
	if (jp2_putuint32(out, ftyp->majver) || jp2_putuint32(out, ftyp->minver)) {
		return -1;
	}
	for (i = 0; i < ftyp->numcompatcodes; ++i) {
		if (jp2_putuint32(out, ftyp->compatcodes[i])) {
			return -1;
		}
	}
	return 0;
}

static int jp2_ihdr_putdata(const jp2_box_t *box, jas_stream_t *out)
{
	const jp2_ihdr_t *ihdr = &box->data.ihdr;
	if (jp2_putuint32(out, ihdr->height) || jp2_putuint32(out, ihdr->width) ||
	  jp2_putuint16(out, ihdr->numcmpts) || jp2_putuint8(out, ihdr->bpc) ||
	  jp2_putuint8(out, ihdr->comptype) || jp2_putuint8(out, ihdr->csunk) ||
	  jp2_putuint8(out, ihdr->ipr)) {
		return -1;
	}
	return 0;
}

static int jp2_bpcc_putdata(const jp2_box_t *box, jas_stream_t *out)
{
	const jp2_bpcc_t *bpcc = &box->data.bpcc;
	unsigned int i;
	for (i = 0; i < bpcc->numcmpts; ++i) {
		if (jp2_putuint8(out, bpcc->bpcs[i])) {
			return -1;
		}
	}
	return 0;
}

static int jp2_colr_putdata(const jp2_box_t *box, jas_stream_t *out)
{
	const jp2_colr_t *colr = &box->data.colr;
	if (jp2_putuint8(out, colr->method) || jp2_putuint8(out, colr->pri) ||
	  jp2_putuint8(out, colr->approx)) {
		return -1;
	}
	switch (colr->method) {
	case JP2_COLR_ENUM:
		if (jp2_putuint32(out, colr->csid)) {
			return -1;
		}
		break;
	case JP2_COLR_ICC:
		if (jas_stream_write(out, colr->iccp, colr->iccplen) != colr->iccplen)
			return -1;
		break;
	}
	return 0;
}

static int jp2_cdef_putdata(const jp2_box_t *box, jas_stream_t *out)
{
	const jp2_cdef_t *cdef = &box->data.cdef;
	unsigned int i;

	if (jp2_putuint16(out, cdef->numchans)) {
		return -1;
	}

	for (i = 0; i < cdef->numchans; ++i) {
		const jp2_cdefchan_t *ent = &cdef->ents[i];
		if (jp2_putuint16(out, ent->channo) ||
		  jp2_putuint16(out, ent->type) ||
		  jp2_putuint16(out, ent->assoc)) {
			return -1;
		}
	}
	return 0;
}

/******************************************************************************\
* Input operations for primitive types.
\******************************************************************************/

static int jp2_getuint8(jas_stream_t *in, uint_fast8_t *val)
{
	int c;
	if ((c = jas_stream_getc(in)) == EOF) {
		return -1;
	}
	if (val) {
		*val = c;
	}
	return 0;
}

static int jp2_getuint16(jas_stream_t *in, uint_fast16_t *val)
{
	jas_uchar buffer[2];
	if (jas_stream_read(in, buffer, sizeof(buffer)) != sizeof(buffer))
		return -1;
	*val = (uint_fast16_t)buffer[0] << 8 | (uint_fast16_t)buffer[1];
	return 0;
}

static int jp2_getuint32(jas_stream_t *in, uint_fast32_t *val)
{
	jas_uchar buffer[4];
	if (jas_stream_read(in, buffer, sizeof(buffer)) != sizeof(buffer))
		return -1;
	*val = (uint_fast32_t)buffer[0] << 24 | (uint_fast32_t)buffer[1] << 16
		| (uint_fast32_t)buffer[2] << 8 | (uint_fast32_t)buffer[3];
	return 0;
}

static int jp2_getuint64(jas_stream_t *in, uint_fast64_t *val)
{
	uint_fast64_t tmpval;
	int i;
	int c;

	tmpval = 0;
	for (i = 0; i < 8; ++i) {
		tmpval <<= 8;
		if ((c = jas_stream_getc(in)) == EOF) {
			return -1;
		}
		tmpval |= (c & 0xff);
	}
	*val = tmpval;

	return 0;
}

/******************************************************************************\
* Output operations for primitive types.
\******************************************************************************/

static int jp2_putuint8(jas_stream_t *out, uint_fast8_t val)
{
	if (jas_stream_putc(out, val & 0xff) == EOF) {
		return -1;
	}
	return 0;
}

static int jp2_putuint16(jas_stream_t *out, uint_fast16_t val)
{
	if (jas_stream_putc(out, (val >> 8) & 0xff) == EOF ||
	  jas_stream_putc(out, val & 0xff) == EOF) {
		return -1;
	}
	return 0;
}

static int jp2_putuint32(jas_stream_t *out, uint_fast32_t val)
{
	if (jas_stream_putc(out, (val >> 24) & 0xff) == EOF ||
	  jas_stream_putc(out, (val >> 16) & 0xff) == EOF ||
	  jas_stream_putc(out, (val >> 8) & 0xff) == EOF ||
	  jas_stream_putc(out, val & 0xff) == EOF) {
		return -1;
	}
	return 0;
}

static int jp2_putuint64(jas_stream_t *out, uint_fast64_t val)
{
	if (jp2_putuint32(out, (val >> 32) & 0xffffffffUL) ||
	  jp2_putuint32(out, val & 0xffffffffUL)) {
		return -1;
	}
	return 0;
}

/******************************************************************************\
* Miscellaneous code.
\******************************************************************************/

static const jp2_boxinfo_t *jp2_boxinfolookup(int type)
{
	const jp2_boxinfo_t *boxinfo;
	for (boxinfo = jp2_boxinfos; boxinfo->name; ++boxinfo) {
		if (boxinfo->type == type) {
			return boxinfo;
		}
	}
	return &jp2_boxinfo_unk;
}

static void jp2_cmap_destroy(jp2_box_t *box)
{
	jp2_cmap_t *cmap = &box->data.cmap;
	if (cmap->ents) {
		jas_free(cmap->ents);
	}
}

static int jp2_cmap_getdata(jp2_box_t *box, jas_stream_t *in)
{
	jp2_cmap_t *cmap = &box->data.cmap;
	jp2_cmapent_t *ent;
	unsigned int i;
	cmap->ents = 0;

	cmap->numchans = (box->datalen) / 4;
	if (cmap->numchans > 0xff)
		/* excessive number of channels - this is a
		   pessimistic limit, because in jp2_pclr_getdata(),
		   it's a 8 bit integer */
		return -1;

	if (!(cmap->ents = jas_alloc2(cmap->numchans, sizeof(jp2_cmapent_t)))) {
		return -1;
	}
	for (i = 0; i < cmap->numchans; ++i) {
		ent = &cmap->ents[i];
		if (jp2_getuint16(in, &ent->cmptno) ||
		  jp2_getuint8(in, &ent->map) ||
		  jp2_getuint8(in, &ent->pcol)) {
			return -1;
		}
	}
	
	return 0;
}

static int jp2_cmap_putdata(const jp2_box_t *box, jas_stream_t *out)
{
	JAS_UNUSED(box);
	JAS_UNUSED(out);
	return -1;
}

static void jp2_cmap_dumpdata(const jp2_box_t *box)
{
	const jp2_cmap_t *cmap = &box->data.cmap;
	unsigned int i;
	jas_logprintf("numchans = %d\n", (int) cmap->numchans);
	for (i = 0; i < cmap->numchans; ++i) {
		const jp2_cmapent_t *ent = &cmap->ents[i];
		jas_logprintf("cmptno=%d; map=%d; pcol=%d\n",
		  (int) ent->cmptno, (int) ent->map, (int) ent->pcol);
	}
}

static void jp2_pclr_destroy(jp2_box_t *box)
{
	jp2_pclr_t *pclr = &box->data.pclr;
	if (pclr->lutdata) {
		jas_free(pclr->lutdata);
	}
	if (pclr->bpc)
		jas_free(pclr->bpc);
}

static int jp2_pclr_getdata(jp2_box_t *box, jas_stream_t *in)
{
	jp2_pclr_t *pclr = &box->data.pclr;
	int lutsize;
	unsigned int i;
	unsigned int j;
	int_fast32_t x;

	pclr->lutdata = 0;
	pclr->bpc = 0;

	if (jp2_getuint16(in, &pclr->numlutents) ||
	  jp2_getuint8(in, &pclr->numchans)) {
		return -1;
	}

	// verify in range data as per I.5.3.4 - Palette box
	if (pclr->numchans < 1 || pclr->numlutents < 1 || pclr->numlutents > 1024) {
		return -1;
	}

	lutsize = pclr->numlutents * pclr->numchans;
	if (!(pclr->lutdata = jas_alloc2(lutsize, sizeof(int_fast32_t)))) {
		return -1;
	}
	if (!(pclr->bpc = jas_alloc2(pclr->numchans, sizeof(uint_fast8_t)))) {
		return -1;
	}
	for (i = 0; i < pclr->numchans; ++i) {
		if (jp2_getuint8(in, &pclr->bpc[i])) {
			return -1;
		}
	}
	for (i = 0; i < pclr->numlutents; ++i) {
		for (j = 0; j < pclr->numchans; ++j) {
			if (jp2_getint(in, (pclr->bpc[j] & 0x80) != 0,
			  (pclr->bpc[j] & 0x7f) + 1, &x)) {
				return -1;
			}
			pclr->lutdata[i * pclr->numchans + j] = x;
		}
	}
	return 0;
}

static int jp2_pclr_putdata(const jp2_box_t *box, jas_stream_t *out)
{
#if 0
	const jp2_pclr_t *pclr = &box->data.pclr;
#endif
	JAS_UNUSED(box);
	JAS_UNUSED(out);
	return -1;
}

static void jp2_pclr_dumpdata(const jp2_box_t *box)
{
	const jp2_pclr_t *pclr = &box->data.pclr;
	unsigned int i;
	int j;
	jas_logprintf("numents=%d; numchans=%d\n", (int) pclr->numlutents,
	  (int) pclr->numchans);
	for (i = 0; i < pclr->numlutents; ++i) {
		for (j = 0; j < pclr->numchans; ++j) {
			jas_logprintf("LUT[%d][%d]=%"PRIiFAST32"\n", i, j,
			  pclr->lutdata[i * pclr->numchans + j]);
		}
	}
}

static int jp2_getint(jas_stream_t *in, int s, int n, int_fast32_t *val)
{
	int c;
	int i;
	uint_fast32_t v;
	int m;

	m = (n + 7) / 8;

	// Ensure that the integer to be read has a valid size.
	if (n < 0 || n > 32) {
		jas_logerrorf("jp2_getint: invalid integer size (%d bits)\n", n);
		return -1;
	}

	v = 0;
	for (i = 0; i < m; ++i) {
		if ((c = jas_stream_getc(in)) == EOF) {
			return -1;
		}
		v = (v << 8) | c;
	}
	v &= ones(n);
	if (s) {
		int sb;
		sb = v & (JAS_CAST(uint_fast32_t, 1) << (8 * m - 1));
		*val = ((~v) + 1) & ones(8 * m);
		if (sb) {
			*val = -*val;
		}
	} else {
		*val = v;
	}

	return 0;
}

const jp2_cdefchan_t *jp2_cdef_lookup(jp2_cdef_t *cdef, int channo)
{
	unsigned int i;
	jp2_cdefchan_t *cdefent;
	for (i = 0; i < cdef->numchans; ++i) {
		cdefent = &cdef->ents[i];
		if (cdefent->channo == JAS_CAST(unsigned int, channo)) {
			return cdefent;
		}
	}
	return 0;
}
