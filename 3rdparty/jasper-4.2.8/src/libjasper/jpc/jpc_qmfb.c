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
 * Quadrature Mirror-Image Filter Bank (QMFB) Library
 *
 * $Id$
 */

/******************************************************************************\
*
\******************************************************************************/

#undef WT_LENONE /* This is not needed due to normalization. */
#define WT_DOSCALE

/******************************************************************************\
* Includes.
\******************************************************************************/

#include "jpc_qmfb.h"
#include "jpc_math.h"

#include "jasper/jas_malloc.h"
#include "jasper/jas_math.h"

#include <stdlib.h>

/******************************************************************************\
*
\******************************************************************************/

#define QMFB_SPLITJOINBUFSIZE 1024

static int jpc_ft_analyze(jpc_fix_t *a, int xstart, int ystart, int width, int height,
  int stride);
static int jpc_ft_synthesize(jpc_fix_t *a, int xstart, int ystart, int width, int height,
  int stride);

static int jpc_ns_analyze(jpc_fix_t *a, int xstart, int ystart, int width, int height,
  int stride);
static int jpc_ns_synthesize(jpc_fix_t *a, int xstart, int ystart, int width,
  int height, int stride);

static void jpc_ft_fwdlift_row(jpc_fix_t *a, unsigned numcols, bool parity);
static void jpc_ft_fwdlift_colgrp(jpc_fix_t *a, unsigned numrows, unsigned stride,
  bool parity);
static void jpc_ft_fwdlift_colres(jpc_fix_t *a, unsigned numrows, unsigned numcols,
  unsigned stride, bool parity);

static void jpc_ft_invlift_row(jpc_fix_t *a, unsigned numcols, bool parity);
static void jpc_ft_invlift_colgrp(jpc_fix_t *a, unsigned numrows, unsigned stride,
  bool parity);
static void jpc_ft_invlift_colres(jpc_fix_t *a, unsigned numrows, unsigned numcols,
  unsigned stride, bool parity);

static void jpc_ns_fwdlift_row(jpc_fix_t *a, unsigned numcols, bool parity);
static void jpc_ns_fwdlift_colgrp(jpc_fix_t *a, unsigned numrows, unsigned stride, bool parity);
static void jpc_ns_fwdlift_colres(jpc_fix_t *a, unsigned numrows, unsigned numcols, unsigned stride,
  bool parity);
static void jpc_ns_invlift_row(jpc_fix_t *a, unsigned numcols, bool parity);
static void jpc_ns_invlift_colgrp(jpc_fix_t *a, unsigned numrows, unsigned stride, bool parity);
static void jpc_ns_invlift_colres(jpc_fix_t *a, unsigned numrows, unsigned numcols, unsigned stride,
  bool parity);

static void jpc_qmfb_split_row(jpc_fix_t *a, unsigned numrows, bool parity, jpc_fix_t*, unsigned);
static void jpc_qmfb_split_colgrp(jpc_fix_t *a, unsigned numrows, unsigned stride, bool parity, jpc_fix_t*, unsigned);
static void jpc_qmfb_split_colres(jpc_fix_t *a, unsigned numrows, unsigned numcols, unsigned stride,
  bool parity, jpc_fix_t*, unsigned);

static void jpc_qmfb_join_row(jpc_fix_t *a, unsigned numcols, bool parity, jpc_fix_t*, unsigned);
static void jpc_qmfb_join_colgrp(jpc_fix_t *a, unsigned numrows, unsigned stride, bool parity, jpc_fix_t*, unsigned);
static void jpc_qmfb_join_colres(jpc_fix_t *a, unsigned numrows, unsigned numcols, unsigned stride,
  bool parity, jpc_fix_t*, unsigned);

/******************************************************************************\
* Data.
\******************************************************************************/

static const double jpc_ft_lpenergywts[32] = {
	1.2247448713915889,
	1.6583123951776999,
	2.3184046238739260,
	3.2691742076555053,
	4.6199296531440819,
	6.5323713152269596,
	9.2377452606141937,
	13.0639951297449581,
	18.4752262333915667,
	26.1278968190610392,
	36.9504194305524791,
	52.2557819580462777,
	73.9008347315741645,
	104.5115624560829133,
	147.8016689469569656,
	209.0231247296646018,
	295.6033378293900000,
	418.0462494347059419,
	591.2066756503630813,
	836.0924988714708661,
	/* approximations */
	836.0924988714708661,
	836.0924988714708661,
	836.0924988714708661,
	836.0924988714708661,
	836.0924988714708661,
	836.0924988714708661,
	836.0924988714708661,
	836.0924988714708661,
	836.0924988714708661,
	836.0924988714708661,
	836.0924988714708661,
	836.0924988714708661
};

static const double jpc_ft_hpenergywts[32] = {
	0.8477912478906585,
	0.9601432184835760,
	1.2593401049756179,
	1.7444107171191079,
	2.4538713036750726,
	3.4656517695088755,
	4.8995276398597856,
	6.9283970402160842,
	9.7980274940131444,
	13.8564306871112652,
	19.5959265076535587,
	27.7128159494245487,
	39.1918369552045860,
	55.4256262207444053,
	78.3836719028959124,
	110.8512517317256822,
	156.7673435548526868,
	221.7025033739244293,
	313.5346870787551552,
	443.4050067351659550,
	/* approximations */
	443.4050067351659550,
	443.4050067351659550,
	443.4050067351659550,
	443.4050067351659550,
	443.4050067351659550,
	443.4050067351659550,
	443.4050067351659550,
	443.4050067351659550,
	443.4050067351659550,
	443.4050067351659550,
	443.4050067351659550,
	443.4050067351659550
};

static const double jpc_ns_lpenergywts[32] = {
	1.4021081679297411,
	2.0303718560817923,
	2.9011625562785555,
	4.1152851751758002,
	5.8245108637728071,
	8.2387599345725171,
	11.6519546479210838,
	16.4785606470644375,
	23.3042776444606794,
	32.9572515613740435,
	46.6086013487782793,
	65.9145194076860861,
	93.2172084551803977,
	131.8290408510004283,
	186.4344176300625691,
	263.6580819564562148,
	372.8688353500955373,
	527.3161639447193920,
	745.7376707114038936,
	1054.6323278917823245,
	/* approximations follow */
	1054.6323278917823245,
	1054.6323278917823245,
	1054.6323278917823245,
	1054.6323278917823245,
	1054.6323278917823245,
	1054.6323278917823245,
	1054.6323278917823245,
	1054.6323278917823245,
	1054.6323278917823245,
	1054.6323278917823245,
	1054.6323278917823245,
	1054.6323278917823245
};

static const double jpc_ns_hpenergywts[32] = {
	1.4425227650161456,
	1.9669426082455688,
	2.8839248082788891,
	4.1475208393432981,
	5.8946497530677817,
	8.3471789178590949,
	11.8086046551047463,
	16.7012780415647804,
	23.6196657032246620,
	33.4034255108592362,
	47.2396388881632632,
	66.8069597416714061,
	94.4793162154500692,
	133.6139330736999113,
	188.9586372358249378,
	267.2278678461869390,
	377.9172750722391356,
	534.4557359047058753,
	755.8345502191498326,
	1068.9114718353569060,
	/* approximations follow */
	1068.9114718353569060,
	1068.9114718353569060,
	1068.9114718353569060,
	1068.9114718353569060,
	1068.9114718353569060,
	1068.9114718353569060,
	1068.9114718353569060,
	1068.9114718353569060,
	1068.9114718353569060,
	1068.9114718353569060,
	1068.9114718353569060
};

const jpc_qmfb2d_t jpc_ft_qmfb2d = {
	jpc_ft_analyze,
	jpc_ft_synthesize,
	jpc_ft_lpenergywts,
	jpc_ft_hpenergywts
};

const jpc_qmfb2d_t jpc_ns_qmfb2d = {
	jpc_ns_analyze,
	jpc_ns_synthesize,
	jpc_ns_lpenergywts,
	jpc_ns_hpenergywts
};

/******************************************************************************\
* generic
\******************************************************************************/

static void jpc_qmfb_split_row(jpc_fix_t *a, unsigned numcols, bool parity,
  jpc_fix_t *buffer, unsigned buffersize)
{
	jpc_fix_t *buf = buffer;
	register jpc_fix_t *srcptr;
	register jpc_fix_t *dstptr;

	const size_t bufsize = JPC_CEILDIVPOW2(numcols, 1);
	assert(buffersize >= bufsize);

	if (numcols >= 2) {
		const unsigned hstartcol = (numcols + !parity) >> 1;
		// ORIGINAL (WRONG): m = (parity) ? hstartcol : (numcols - hstartcol);
		const unsigned m = numcols - hstartcol;

		/* Save the samples destined for the highpass channel. */
		dstptr = buf;
		srcptr = &a[!parity];
		for (unsigned n = m; n > 0; --n) {
			*dstptr = *srcptr;
			++dstptr;
			srcptr += 2;
		}
		/* Copy the appropriate samples into the lowpass channel. */
		dstptr = &a[!parity];
		srcptr = &a[2 - parity];
		for (unsigned n = numcols - m - (!parity); n > 0; --n) {
			*dstptr = *srcptr;
			++dstptr;
			srcptr += 2;
		}
		/* Copy the saved samples into the highpass channel. */
		dstptr = &a[hstartcol];
		srcptr = buf;
		for (unsigned n = m; n > 0; --n) {
			*dstptr = *srcptr;
			++dstptr;
			++srcptr;
		}
	}
}

static void jpc_qmfb_split_colgrp(jpc_fix_t *a, unsigned numrows, unsigned stride,
  bool parity,
  jpc_fix_t *buffer, unsigned buffersize)
{
	jpc_fix_t *buf = buffer;
	jpc_fix_t *srcptr;
	jpc_fix_t *dstptr;
	register jpc_fix_t *srcptr2;
	register jpc_fix_t *dstptr2;

	const size_t bufsize = JPC_QMFB_COLGRPSIZE * JPC_CEILDIVPOW2(numrows, 1);
	assert(buffersize >= bufsize);

	if (numrows >= 2) {
		const unsigned hstartrow = (numrows + !parity) >> 1;
		// ORIGINAL (WRONG): m = (parity) ? hstartrow : (numrows - hstartrow);
		const unsigned m = numrows - hstartrow;

		/* Save the samples destined for the highpass channel. */
		dstptr = buf;
		srcptr = &a[(!parity) * stride];
		for (unsigned n = m; n > 0; --n) {
			dstptr2 = dstptr;
			srcptr2 = srcptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				*dstptr2 = *srcptr2;
				++dstptr2;
				++srcptr2;
			}
			dstptr += JPC_QMFB_COLGRPSIZE;
			srcptr += stride << 1;
		}
		/* Copy the appropriate samples into the lowpass channel. */
		dstptr = &a[(!parity) * stride];
		srcptr = &a[(2 - parity) * stride];
		for (unsigned n = numrows - m - (!parity); n > 0; --n) {
			dstptr2 = dstptr;
			srcptr2 = srcptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				*dstptr2 = *srcptr2;
				++dstptr2;
				++srcptr2;
			}
			dstptr += stride;
			srcptr += stride << 1;
		}
		/* Copy the saved samples into the highpass channel. */
		dstptr = &a[hstartrow * stride];
		srcptr = buf;
		for (unsigned n = m; n > 0; --n) {
			dstptr2 = dstptr;
			srcptr2 = srcptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				*dstptr2 = *srcptr2;
				++dstptr2;
				++srcptr2;
			}
			dstptr += stride;
			srcptr += JPC_QMFB_COLGRPSIZE;
		}
	}
}

static void jpc_qmfb_split_colres(jpc_fix_t *a, unsigned numrows, unsigned numcols,
  unsigned stride, bool parity,
  jpc_fix_t *buffer, unsigned buffersize)
{
	jpc_fix_t *buf = buffer;
	jpc_fix_t *srcptr;
	jpc_fix_t *dstptr;
	register jpc_fix_t *srcptr2;
	register jpc_fix_t *dstptr2;

	const size_t bufsize = numcols * JPC_CEILDIVPOW2(numrows, 1);
	assert(buffersize >= bufsize);

	if (numrows >= 2) {
		const unsigned hstartcol = (numrows + !parity) >> 1;
		// ORIGINAL (WRONG): m = (parity) ? hstartcol : (numrows - hstartcol);
		const unsigned m = numrows - hstartcol;

		/* Save the samples destined for the highpass channel. */
		dstptr = buf;
		srcptr = &a[(!parity) * stride];
		for (unsigned n = m; n > 0; --n) {
			dstptr2 = dstptr;
			srcptr2 = srcptr;
			for (unsigned i = 0; i < numcols; ++i) {
				*dstptr2 = *srcptr2;
				++dstptr2;
				++srcptr2;
			}
			dstptr += numcols;
			srcptr += stride << 1;
		}
		/* Copy the appropriate samples into the lowpass channel. */
		dstptr = &a[(!parity) * stride];
		srcptr = &a[(2 - parity) * stride];
		for (unsigned n = numrows - m - (!parity); n > 0; --n) {
			dstptr2 = dstptr;
			srcptr2 = srcptr;
			for (unsigned i = 0; i < numcols; ++i) {
				*dstptr2 = *srcptr2;
				++dstptr2;
				++srcptr2;
			}
			dstptr += stride;
			srcptr += stride << 1;
		}
		/* Copy the saved samples into the highpass channel. */
		dstptr = &a[hstartcol * stride];
		srcptr = buf;
		for (unsigned n = m; n > 0; --n) {
			dstptr2 = dstptr;
			srcptr2 = srcptr;
			for (unsigned i = 0; i < numcols; ++i) {
				*dstptr2 = *srcptr2;
				++dstptr2;
				++srcptr2;
			}
			dstptr += stride;
			srcptr += numcols;
		}
	}
}

void jpc_qmfb_join_row(jpc_fix_t *a, unsigned numcols, bool parity,
  jpc_fix_t *buffer, unsigned buffersize)
{
	jpc_fix_t *buf = buffer;
	register jpc_fix_t *srcptr;
	register jpc_fix_t *dstptr;

	/* Allocate memory for the join buffer from the heap. */
	const size_t bufsize = JPC_CEILDIVPOW2(numcols, 1);
	assert(buffersize >= bufsize);

	const unsigned hstartcol = (numcols + !parity) >> 1;

	/* Save the samples from the lowpass channel. */
	srcptr = &a[0];
	dstptr = buf;
	for (unsigned n = hstartcol; n > 0; --n) {
		*dstptr = *srcptr;
		++srcptr;
		++dstptr;
	}
	/* Copy the samples from the highpass channel into place. */
	srcptr = &a[hstartcol];
	dstptr = &a[!parity];
	for (unsigned n = numcols - hstartcol; n > 0; --n) {
		*dstptr = *srcptr;
		dstptr += 2;
		++srcptr;
	}
	/* Copy the samples from the lowpass channel into place. */
	srcptr = buf;
	dstptr = &a[parity];
	for (unsigned n = hstartcol; n > 0; --n) {
		*dstptr = *srcptr;
		dstptr += 2;
		++srcptr;
	}
}

static void jpc_qmfb_join_colgrp(jpc_fix_t *a, unsigned numrows, unsigned stride,
  bool parity,
  jpc_fix_t *buffer, unsigned buffersize)
{
	jpc_fix_t *buf = buffer;
	jpc_fix_t *srcptr;
	jpc_fix_t *dstptr;
	register jpc_fix_t *srcptr2;
	register jpc_fix_t *dstptr2;

	const size_t bufsize = JPC_QMFB_COLGRPSIZE * JPC_CEILDIVPOW2(numrows, 1);
	assert(buffersize >= bufsize);

	const unsigned hstartcol = (numrows + !parity) >> 1;

	/* Save the samples from the lowpass channel. */
	srcptr = &a[0];
	dstptr = buf;
	for (unsigned n = hstartcol; n > 0; --n) {
		dstptr2 = dstptr;
		srcptr2 = srcptr;
		for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
			*dstptr2 = *srcptr2;
			++dstptr2;
			++srcptr2;
		}
		srcptr += stride;
		dstptr += JPC_QMFB_COLGRPSIZE;
	}
	/* Copy the samples from the highpass channel into place. */
	srcptr = &a[hstartcol * stride];
	dstptr = &a[(!parity) * stride];
	for (unsigned n = numrows - hstartcol; n > 0; --n) {
		dstptr2 = dstptr;
		srcptr2 = srcptr;
		for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
			*dstptr2 = *srcptr2;
			++dstptr2;
			++srcptr2;
		}
		dstptr += 2 * stride;
		srcptr += stride;
	}
	/* Copy the samples from the lowpass channel into place. */
	srcptr = buf;
	dstptr = &a[parity * stride];
	for (unsigned n = hstartcol; n > 0; --n) {
		dstptr2 = dstptr;
		srcptr2 = srcptr;
		for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
			*dstptr2 = *srcptr2;
			++dstptr2;
			++srcptr2;
		}
		dstptr += 2 * stride;
		srcptr += JPC_QMFB_COLGRPSIZE;
	}
}

static void jpc_qmfb_join_colres(jpc_fix_t *a, unsigned numrows, unsigned numcols,
  unsigned stride, bool parity,
  jpc_fix_t *buffer, unsigned buffersize)
{
	jpc_fix_t *buf = buffer;
	jpc_fix_t *srcptr;
	jpc_fix_t *dstptr;
	register jpc_fix_t *srcptr2;
	register jpc_fix_t *dstptr2;

	/* Allocate memory for the join buffer from the heap. */
	const size_t bufsize = numcols * JPC_CEILDIVPOW2(numrows, 1);
	assert(buffersize >= bufsize);

	const unsigned hstartcol = (numrows + !parity) >> 1;

	/* Save the samples from the lowpass channel. */
	srcptr = &a[0];
	dstptr = buf;
	for (unsigned n = hstartcol; n > 0; --n) {
		dstptr2 = dstptr;
		srcptr2 = srcptr;
		for (unsigned i = 0; i < numcols; ++i) {
			*dstptr2 = *srcptr2;
			++dstptr2;
			++srcptr2;
		}
		srcptr += stride;
		dstptr += numcols;
	}
	/* Copy the samples from the highpass channel into place. */
	srcptr = &a[hstartcol * stride];
	dstptr = &a[(!parity) * stride];
	for (unsigned n = numrows - hstartcol; n > 0; --n) {
		dstptr2 = dstptr;
		srcptr2 = srcptr;
		for (unsigned i = 0; i < numcols; ++i) {
			*dstptr2 = *srcptr2;
			++dstptr2;
			++srcptr2;
		}
		dstptr += 2 * stride;
		srcptr += stride;
	}
	/* Copy the samples from the lowpass channel into place. */
	srcptr = buf;
	dstptr = &a[parity * stride];
	for (unsigned n = hstartcol; n > 0; --n) {
		dstptr2 = dstptr;
		srcptr2 = srcptr;
		for (unsigned i = 0; i < numcols; ++i) {
			*dstptr2 = *srcptr2;
			++dstptr2;
			++srcptr2;
		}
		dstptr += 2 * stride;
		srcptr += numcols;
	}
}

/******************************************************************************\
* 5/3 transform
\******************************************************************************/

static void jpc_ft_fwdlift_row(jpc_fix_t *a, unsigned numcols, bool parity)
{

	register jpc_fix_t *lptr;
	register jpc_fix_t *hptr;

	const unsigned llen = (numcols + !parity) >> 1;
	const bool end_parity = parity == (numcols & 1);

	if (numcols > 1) {

		/* Apply the first lifting step. */
		lptr = &a[0];
		hptr = &a[llen];
		if (parity) {
			hptr[0] -= lptr[0];
			++hptr;
		}
		for (unsigned n = numcols - llen - parity - end_parity; n > 0; --n) {
			//hptr[0] -= (lptr[0] + lptr[1]) >> 1;
			hptr[0] -= jpc_fix_asr(lptr[0] + lptr[1], 1);
			++hptr;
			++lptr;
		}
		if (end_parity) {
			hptr[0] -= lptr[0];
		}

		/* Apply the second lifting step. */
		lptr = &a[0];
		hptr = &a[llen];
		if (!parity) {
			//lptr[0] += (hptr[0] + 1) >> 1;
			lptr[0] += jpc_fix_asr(hptr[0] + 1, 1);
			++lptr;
		}
		for (unsigned n = llen - (!parity) - (!end_parity); n > 0; --n) {
			//lptr[0] += (hptr[0] + hptr[1] + 2) >> 2;
			lptr[0] += jpc_fix_asr(hptr[0] + hptr[1] + 2, 2);
			++lptr;
			++hptr;
		}
		if (!end_parity) {
			//lptr[0] += (hptr[0] + 1) >> 1;
			lptr[0] += jpc_fix_asr(hptr[0] + 1, 1);
		}

	} else {

		if (parity) {
			lptr = &a[0];
			//lptr[0] <<= 1;
			lptr[0] = jpc_fix_asl(lptr[0], 1);
		}

	}

}

static void jpc_ft_fwdlift_colgrp(jpc_fix_t *a, unsigned numrows, unsigned stride, bool parity)
{

	jpc_fix_t *lptr;
	jpc_fix_t *hptr;
	register jpc_fix_t *lptr2;
	register jpc_fix_t *hptr2;

	const unsigned llen = (numrows + !parity) >> 1;
	const bool end_parity = parity == (numrows & 1);

	if (numrows > 1) {

		/* Apply the first lifting step. */
		lptr = &a[0];
		hptr = &a[llen * stride];
		if (parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				hptr2[0] -= lptr2[0];
				++hptr2;
				++lptr2;
			}
			hptr += stride;
		}
		for (unsigned n = numrows - llen - parity - end_parity; n > 0; --n) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				//hptr2[0] -= (lptr2[0] + lptr2[stride]) >> 1;
				hptr2[0] -= jpc_fix_asr(lptr2[0] + lptr2[stride], 1);
				++lptr2;
				++hptr2;
			}
			hptr += stride;
			lptr += stride;
		}
		if (end_parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				hptr2[0] -= lptr2[0];
				++lptr2;
				++hptr2;
			}
		}

		/* Apply the second lifting step. */
		lptr = &a[0];
		hptr = &a[llen * stride];
		if (!parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				//lptr2[0] += (hptr2[0] + 1) >> 1;
				lptr2[0] += jpc_fix_asr(hptr2[0] + 1, 1);
				++lptr2;
				++hptr2;
			}
			lptr += stride;
		}
		for (unsigned n = llen - (!parity) - (parity != (numrows & 1)); n > 0; --n) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				//lptr2[0] += (hptr2[0] + hptr2[stride] + 2) >> 2;
				lptr2[0] += jpc_fix_asr(hptr2[0] + hptr2[stride] + 2, 2);
				++lptr2;
				++hptr2;
			}
			lptr += stride;
			hptr += stride;
		}
		if (parity != (numrows & 1)) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				//lptr2[0] += (hptr2[0] + 1) >> 1;
				lptr2[0] += jpc_fix_asr(hptr2[0] + 1, 1);
				++lptr2;
				++hptr2;
			}
		}

	} else {

		if (parity) {
			lptr2 = &a[0];
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				//lptr2[0] <<= 1;
				lptr2[0] = jpc_fix_asl(lptr2[0], 1);
				++lptr2;
			}
		}

	}

}

static void jpc_ft_fwdlift_colres(jpc_fix_t *a, unsigned numrows, unsigned numcols, unsigned stride,
  bool parity)
{

	jpc_fix_t *lptr;
	jpc_fix_t *hptr;
	register jpc_fix_t *lptr2;
	register jpc_fix_t *hptr2;

	const unsigned llen = (numrows + !parity) >> 1;
	const bool end_parity = parity == (numrows & 1);

	if (numrows > 1) {

		/* Apply the first lifting step. */
		lptr = &a[0];
		hptr = &a[llen * stride];
		if (parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				hptr2[0] -= lptr2[0];
				++hptr2;
				++lptr2;
			}
			hptr += stride;
		}
		for (unsigned n = numrows - llen - parity - end_parity; n > 0; --n) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				//hptr2[0] -= (lptr2[0] + lptr2[stride]) >> 1;
				hptr2[0] -= jpc_fix_asr(lptr2[0] + lptr2[stride], 1);
				++lptr2;
				++hptr2;
			}
			hptr += stride;
			lptr += stride;
		}
		if (end_parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				hptr2[0] -= lptr2[0];
				++lptr2;
				++hptr2;
			}
		}

		/* Apply the second lifting step. */
		lptr = &a[0];
		hptr = &a[llen * stride];
		if (!parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				//lptr2[0] += (hptr2[0] + 1) >> 1;
				lptr2[0] += jpc_fix_asr(hptr2[0] + 1, 1);
				++lptr2;
				++hptr2;
			}
			lptr += stride;
		}
		for (unsigned n = llen - (!parity) - (parity != (numrows & 1)); n > 0; --n) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				//lptr2[0] += (hptr2[0] + hptr2[stride] + 2) >> 2;
				lptr2[0] += jpc_fix_asr(hptr2[0] + hptr2[stride] + 2, 2);
				++lptr2;
				++hptr2;
			}
			lptr += stride;
			hptr += stride;
		}
		if (parity != (numrows & 1)) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				//lptr2[0] += (hptr2[0] + 1) >> 1;
				lptr2[0] += jpc_fix_asr(hptr2[0] + 1, 1);
				++lptr2;
				++hptr2;
			}
		}

	} else {

		if (parity) {
			lptr2 = &a[0];
			for (unsigned i = 0; i < numcols; ++i) {
				//lptr2[0] <<= 1;
				lptr2[0] = jpc_fix_asl(lptr2[0], 1);
				++lptr2;
			}
		}

	}

}

static void jpc_ft_invlift_row(jpc_fix_t *a, unsigned numcols, bool parity)
{

	register jpc_fix_t *lptr;
	register jpc_fix_t *hptr;

	const unsigned llen = (numcols + !parity) >> 1;
	const bool end_parity = parity == (numcols & 1);

	if (numcols > 1) {

		/* Apply the first lifting step. */
		lptr = &a[0];
		hptr = &a[llen];
		if (!parity) {
			//lptr[0] -= (hptr[0] + 1) >> 1;
			lptr[0] -= jpc_fix_asr(hptr[0] + 1, 1);
			++lptr;
		}
		for (unsigned n = llen - (!parity) - (!end_parity); n > 0; --n) {
			//lptr[0] -= (hptr[0] + hptr[1] + 2) >> 2;
			lptr[0] -= jpc_fix_asr(hptr[0] + hptr[1] + 2, 2);
			++lptr;
			++hptr;
		}
		if (!end_parity) {
			//lptr[0] -= (hptr[0] + 1) >> 1;
			lptr[0] -= jpc_fix_asr(hptr[0] + 1, 1);
		}

		/* Apply the second lifting step. */
		lptr = &a[0];
		hptr = &a[llen];
		if (parity) {
			hptr[0] += lptr[0];
			++hptr;
		}
		for (unsigned n = numcols - llen - parity - end_parity; n > 0; --n) {
			//hptr[0] += (lptr[0] + lptr[1]) >> 1;
			hptr[0] += jpc_fix_asr(lptr[0] + lptr[1], 1);
			++hptr;
			++lptr;
		}
		if (end_parity) {
			hptr[0] += lptr[0];
		}

	} else {

		if (parity) {
			lptr = &a[0];
			//lptr[0] >>= 1;
			lptr[0] = jpc_fix_asr(lptr[0], 1);
		}

	}

}

static void jpc_ft_invlift_colgrp(jpc_fix_t *a, unsigned numrows, unsigned stride, bool parity)
{

	jpc_fix_t *lptr;
	jpc_fix_t *hptr;
	register jpc_fix_t *lptr2;
	register jpc_fix_t *hptr2;

	const unsigned llen = (numrows + !parity) >> 1;
	const bool end_parity = parity == (numrows & 1);

	if (numrows > 1) {

		/* Apply the first lifting step. */
		lptr = &a[0];
		hptr = &a[llen * stride];
		if (!parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				//lptr2[0] -= (hptr2[0] + 1) >> 1;
				lptr2[0] -= jpc_fix_asr(hptr2[0] + 1, 1);
				++lptr2;
				++hptr2;
			}
			lptr += stride;
		}
		for (unsigned n = llen - (!parity) - (parity != (numrows & 1)); n > 0; --n) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				//lptr2[0] -= (hptr2[0] + hptr2[stride] + 2) >> 2;
				lptr2[0] -= jpc_fix_asr(hptr2[0] + hptr2[stride] + 2, 2);
				++lptr2;
				++hptr2;
			}
			lptr += stride;
			hptr += stride;
		}
		if (parity != (numrows & 1)) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				//lptr2[0] -= (hptr2[0] + 1) >> 1;
				lptr2[0] -= jpc_fix_asr(hptr2[0] + 1, 1);
				++lptr2;
				++hptr2;
			}
		}

		/* Apply the second lifting step. */
		lptr = &a[0];
		hptr = &a[llen * stride];
		if (parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				hptr2[0] += lptr2[0];
				++hptr2;
				++lptr2;
			}
			hptr += stride;
		}
		for (unsigned n = numrows - llen - parity - end_parity; n > 0; --n) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				//hptr2[0] += (lptr2[0] + lptr2[stride]) >> 1;
				hptr2[0] += jpc_fix_asr(lptr2[0] + lptr2[stride], 1);
				++lptr2;
				++hptr2;
			}
			hptr += stride;
			lptr += stride;
		}
		if (end_parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				hptr2[0] += lptr2[0];
				++lptr2;
				++hptr2;
			}
		}

	} else {

		if (parity) {
			lptr2 = &a[0];
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				//lptr2[0] >>= 1;
				lptr2[0] = jpc_fix_asr(lptr2[0], 1);
				++lptr2;
			}
		}

	}

}

static void jpc_ft_invlift_colres(jpc_fix_t *a, unsigned numrows, unsigned numcols, unsigned stride,
  bool parity)
{

	jpc_fix_t *lptr;
	jpc_fix_t *hptr;
	register jpc_fix_t *lptr2;
	register jpc_fix_t *hptr2;

	const unsigned llen = (numrows + !parity) >> 1;
	const bool end_parity = parity == (numrows & 1);

	if (numrows > 1) {

		/* Apply the first lifting step. */
		lptr = &a[0];
		hptr = &a[llen * stride];
		if (!parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				//lptr2[0] -= (hptr2[0] + 1) >> 1;
				lptr2[0] -= jpc_fix_asr(hptr2[0] + 1, 1);
				++lptr2;
				++hptr2;
			}
			lptr += stride;
		}
		for (unsigned n = llen - (!parity) - (parity != (numrows & 1)); n > 0; --n) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				//lptr2[0] -= (hptr2[0] + hptr2[stride] + 2) >> 2;
				lptr2[0] -= jpc_fix_asr(hptr2[0] + hptr2[stride] + 2, 2);
				++lptr2;
				++hptr2;
			}
			lptr += stride;
			hptr += stride;
		}
		if (parity != (numrows & 1)) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				//lptr2[0] -= (hptr2[0] + 1) >> 1;
				lptr2[0] -= jpc_fix_asr(hptr2[0] + 1, 1);
				++lptr2;
				++hptr2;
			}
		}

		/* Apply the second lifting step. */
		lptr = &a[0];
		hptr = &a[llen * stride];
		if (parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				hptr2[0] += lptr2[0];
				++hptr2;
				++lptr2;
			}
			hptr += stride;
		}
		for (unsigned n = numrows - llen - parity - end_parity; n > 0; --n) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				//hptr2[0] += (lptr2[0] + lptr2[stride]) >> 1;
				hptr2[0] += jpc_fix_asr(lptr2[0] + lptr2[stride], 1);
				++lptr2;
				++hptr2;
			}
			hptr += stride;
			lptr += stride;
		}
		if (end_parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				hptr2[0] += lptr2[0];
				++lptr2;
				++hptr2;
			}
		}

	} else {

		if (parity) {
			lptr2 = &a[0];
			for (unsigned i = 0; i < numcols; ++i) {
				//lptr2[0] >>= 1;
				lptr2[0] = jpc_fix_asr(lptr2[0], 1);
				++lptr2;
			}
		}

	}

}

int jpc_ft_analyze(jpc_fix_t *a, int xstart, int ystart, int width, int height,
  int stride)
{
	jpc_fix_t localbuf[QMFB_SPLITJOINBUFSIZE];
	unsigned bufsize;
	jpc_fix_t *buf;
	const unsigned numrows = height;
	const unsigned numcols = width;
	const unsigned rowparity = ystart & 1;
	const unsigned colparity = xstart & 1;
	jpc_fix_t *startptr;

	const unsigned maxcols = (numcols / JPC_QMFB_COLGRPSIZE) * JPC_QMFB_COLGRPSIZE;
	bufsize = JAS_MAX(width, JPC_QMFB_COLGRPSIZE * height);
	if (bufsize > QMFB_SPLITJOINBUFSIZE) {
		if (!(buf = jas_alloc2(bufsize, sizeof(jpc_fix_t)))) {
			return -1;
		}
	} else {
		buf = localbuf;
		bufsize = QMFB_SPLITJOINBUFSIZE;
	}

	startptr = &a[0];
	for (unsigned i = 0; i < maxcols; i += JPC_QMFB_COLGRPSIZE) {
		jpc_qmfb_split_colgrp(startptr, numrows, stride, rowparity, buf, bufsize);
		jpc_ft_fwdlift_colgrp(startptr, numrows, stride, rowparity);
		startptr += JPC_QMFB_COLGRPSIZE;
	}
	if (maxcols < numcols) {
		jpc_qmfb_split_colres(startptr, numrows, numcols - maxcols, stride,
		  rowparity, buf, bufsize);
		jpc_ft_fwdlift_colres(startptr, numrows, numcols - maxcols, stride,
		  rowparity);
	}

	startptr = &a[0];
	for (unsigned i = 0; i < numrows; ++i) {
		jpc_qmfb_split_row(startptr, numcols, colparity, buf, bufsize);
		jpc_ft_fwdlift_row(startptr, numcols, colparity);
		startptr += stride;
	}

	if (buf != localbuf) {
		jas_free(buf);
	}

	return 0;

}

int jpc_ft_synthesize(jpc_fix_t *a, int xstart, int ystart, int width, int height,
  int stride)
{
	jpc_fix_t localbuf[QMFB_SPLITJOINBUFSIZE];
	unsigned bufsize;
	jpc_fix_t *buf;
	const unsigned numrows = height;
	const unsigned numcols = width;
	const unsigned rowparity = ystart & 1;
	const unsigned colparity = xstart & 1;
	jpc_fix_t *startptr;

	const unsigned maxcols = (numcols / JPC_QMFB_COLGRPSIZE) * JPC_QMFB_COLGRPSIZE;
	bufsize = JAS_MAX(width, JPC_QMFB_COLGRPSIZE * height);
	if (bufsize > QMFB_SPLITJOINBUFSIZE) {
		if (!(buf = jas_alloc2(bufsize, sizeof(jpc_fix_t)))) {
			return -1;
		}
	} else {
		buf = localbuf;
		bufsize = QMFB_SPLITJOINBUFSIZE;
	}

	startptr = &a[0];
	for (unsigned i = 0; i < numrows; ++i) {
		jpc_ft_invlift_row(startptr, numcols, colparity);
		jpc_qmfb_join_row(startptr, numcols, colparity, buf, bufsize);
		startptr += stride;
	}

	startptr = &a[0];
	for (unsigned i = 0; i < maxcols; i += JPC_QMFB_COLGRPSIZE) {
		jpc_ft_invlift_colgrp(startptr, numrows, stride, rowparity);
		jpc_qmfb_join_colgrp(startptr, numrows, stride, rowparity, buf, bufsize);
		startptr += JPC_QMFB_COLGRPSIZE;
	}
	if (maxcols < numcols) {
		jpc_ft_invlift_colres(startptr, numrows, numcols - maxcols, stride,
		  rowparity);
		jpc_qmfb_join_colres(startptr, numrows, numcols - maxcols, stride,
		  rowparity, buf, bufsize);
	}

	if (buf != localbuf) {
		jas_free(buf);
	}

	return 0;
}

/******************************************************************************\
* 9/7 transform
\******************************************************************************/

#define ALPHA (-1.586134342059924)
#define BETA (-0.052980118572961)
#define GAMMA (0.882911075530934)
#define DELTA (0.443506852043971)
#define LGAIN (1.0 / 1.23017410558578)
#define HGAIN (1.0 / 1.62578613134411)

static void jpc_ns_fwdlift_row(jpc_fix_t *a, unsigned numcols, bool parity)
{

	register jpc_fix_t *lptr;
	register jpc_fix_t *hptr;

	const unsigned llen = (numcols + !parity) >> 1;
	const bool end_parity = parity == (numcols & 1);

	if (numcols > 1) {

		/* Apply the first lifting step. */
		lptr = &a[0];
		hptr = &a[llen];
		if (parity) {
			jpc_fix_pluseq(hptr[0], jpc_fix_mul(jpc_dbltofix(2.0 * ALPHA),
			  lptr[0]));
			++hptr;
		}
		for (unsigned n = numcols - llen - parity - end_parity; n > 0; --n) {
			jpc_fix_pluseq(hptr[0], jpc_fix_mul(jpc_dbltofix(ALPHA),
			  jpc_fix_add(lptr[0], lptr[1])));
			++hptr;
			++lptr;
		}
		if (end_parity) {
			jpc_fix_pluseq(hptr[0], jpc_fix_mul(jpc_dbltofix(2.0 * ALPHA),
			  lptr[0]));
		}

		/* Apply the second lifting step. */
		lptr = &a[0];
		hptr = &a[llen];
		if (!parity) {
			jpc_fix_pluseq(lptr[0], jpc_fix_mul(jpc_dbltofix(2.0 * BETA),
			  hptr[0]));
			++lptr;
		}
		for (unsigned n = llen - (!parity) - (!end_parity); n > 0; --n) {
			jpc_fix_pluseq(lptr[0], jpc_fix_mul(jpc_dbltofix(BETA),
			  jpc_fix_add(hptr[0], hptr[1])));
			++lptr;
			++hptr;
		}
		if (!end_parity) {
			jpc_fix_pluseq(lptr[0], jpc_fix_mul(jpc_dbltofix(2.0 * BETA),
			  hptr[0]));
		}

		/* Apply the third lifting step. */
		lptr = &a[0];
		hptr = &a[llen];
		if (parity) {
			jpc_fix_pluseq(hptr[0], jpc_fix_mul(jpc_dbltofix(2.0 * GAMMA),
			  lptr[0]));
			++hptr;
		}
		for (unsigned n = numcols - llen - parity - end_parity; n > 0; --n) {
			jpc_fix_pluseq(hptr[0], jpc_fix_mul(jpc_dbltofix(GAMMA),
			  jpc_fix_add(lptr[0], lptr[1])));
			++hptr;
			++lptr;
		}
		if (end_parity) {
			jpc_fix_pluseq(hptr[0], jpc_fix_mul(jpc_dbltofix(2.0 * GAMMA),
			  lptr[0]));
		}

		/* Apply the fourth lifting step. */
		lptr = &a[0];
		hptr = &a[llen];
		if (!parity) {
			jpc_fix_pluseq(lptr[0], jpc_fix_mul(jpc_dbltofix(2.0 * DELTA),
			  hptr[0]));
			++lptr;
		}
		for (unsigned n = llen - (!parity) - (!end_parity); n > 0; --n) {
			jpc_fix_pluseq(lptr[0], jpc_fix_mul(jpc_dbltofix(DELTA),
			  jpc_fix_add(hptr[0], hptr[1])));
			++lptr;
			++hptr;
		}
		if (!end_parity) {
			jpc_fix_pluseq(lptr[0], jpc_fix_mul(jpc_dbltofix(2.0 * DELTA),
			  hptr[0]));
		}

		/* Apply the scaling step. */
#if defined(WT_DOSCALE)
		lptr = &a[0];
		for (unsigned n = llen; n > 0; --n) {
			lptr[0] = jpc_fix_mul(lptr[0], jpc_dbltofix(LGAIN));
			++lptr;
		}
		hptr = &a[llen];
		for (unsigned n = numcols - llen; n > 0; --n) {
			hptr[0] = jpc_fix_mul(hptr[0], jpc_dbltofix(HGAIN));
			++hptr;
		}
#endif

	} else {

#if defined(WT_LENONE)
		if (parity) {
			lptr = &a[0];
			//lptr[0] <<= 1;
			lptr[0] = jpc_fix_asl(lptr[0], 1);
		}
#endif

	}

}

static void jpc_ns_fwdlift_colgrp(jpc_fix_t *a, unsigned numrows, unsigned stride,
  bool parity)
{

	jpc_fix_t *lptr;
	jpc_fix_t *hptr;
	register jpc_fix_t *lptr2;
	register jpc_fix_t *hptr2;

	const unsigned llen = (numrows + !parity) >> 1;
	const bool end_parity = parity == (numrows & 1);

	if (numrows > 1) {

		/* Apply the first lifting step. */
		lptr = &a[0];
		hptr = &a[llen * stride];
		if (parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				jpc_fix_pluseq(hptr2[0], jpc_fix_mul(jpc_dbltofix(2.0 * ALPHA),
				  lptr2[0]));
				++hptr2;
				++lptr2;
			}
			hptr += stride;
		}
		for (unsigned n = numrows - llen - parity - end_parity; n > 0; --n) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				jpc_fix_pluseq(hptr2[0], jpc_fix_mul(jpc_dbltofix(ALPHA),
				  jpc_fix_add(lptr2[0], lptr2[stride])));
				++lptr2;
				++hptr2;
			}
			hptr += stride;
			lptr += stride;
		}
		if (end_parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				jpc_fix_pluseq(hptr2[0], jpc_fix_mul(jpc_dbltofix(2.0 * ALPHA),
				  lptr2[0]));
				++lptr2;
				++hptr2;
			}
		}

		/* Apply the second lifting step. */
		lptr = &a[0];
		hptr = &a[llen * stride];
		if (!parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				jpc_fix_pluseq(lptr2[0], jpc_fix_mul(jpc_dbltofix(2.0 * BETA),
				  hptr2[0]));
				++lptr2;
				++hptr2;
			}
			lptr += stride;
		}
		for (unsigned n = llen - (!parity) - (parity != (numrows & 1)); n > 0; --n) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				jpc_fix_pluseq(lptr2[0], jpc_fix_mul(jpc_dbltofix(BETA),
				  jpc_fix_add(hptr2[0], hptr2[stride])));
				++lptr2;
				++hptr2;
			}
			lptr += stride;
			hptr += stride;
		}
		if (parity != (numrows & 1)) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				jpc_fix_pluseq(lptr2[0], jpc_fix_mul(jpc_dbltofix(2.0 * BETA),
				  hptr2[0]));
				++lptr2;
				++hptr2;
			}
		}

		/* Apply the third lifting step. */
		lptr = &a[0];
		hptr = &a[llen * stride];
		if (parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				jpc_fix_pluseq(hptr2[0], jpc_fix_mul(jpc_dbltofix(2.0 * GAMMA),
				  lptr2[0]));
				++hptr2;
				++lptr2;
			}
			hptr += stride;
		}
		for (unsigned n = numrows - llen - parity - end_parity; n > 0; --n) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				jpc_fix_pluseq(hptr2[0], jpc_fix_mul(jpc_dbltofix(GAMMA),
				  jpc_fix_add(lptr2[0], lptr2[stride])));
				++lptr2;
				++hptr2;
			}
			hptr += stride;
			lptr += stride;
		}
		if (end_parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				jpc_fix_pluseq(hptr2[0], jpc_fix_mul(jpc_dbltofix(2.0 * GAMMA),
				  lptr2[0]));
				++lptr2;
				++hptr2;
			}
		}

		/* Apply the fourth lifting step. */
		lptr = &a[0];
		hptr = &a[llen * stride];
		if (!parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				jpc_fix_pluseq(lptr2[0], jpc_fix_mul(jpc_dbltofix(2.0 * DELTA),
				  hptr2[0]));
				++lptr2;
				++hptr2;
			}
			lptr += stride;
		}
		for (unsigned n = llen - (!parity) - (parity != (numrows & 1)); n > 0; --n) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				jpc_fix_pluseq(lptr2[0], jpc_fix_mul(jpc_dbltofix(DELTA),
				  jpc_fix_add(hptr2[0], hptr2[stride])));
				++lptr2;
				++hptr2;
			}
			lptr += stride;
			hptr += stride;
		}
		if (parity != (numrows & 1)) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				jpc_fix_pluseq(lptr2[0], jpc_fix_mul(jpc_dbltofix(2.0 * DELTA),
				  hptr2[0]));
				++lptr2;
				++hptr2;
			}
		}

		/* Apply the scaling step. */
#if defined(WT_DOSCALE)
		lptr = &a[0];
		for (unsigned n = llen; n > 0; --n) {
			lptr2 = lptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				lptr2[0] = jpc_fix_mul(lptr2[0], jpc_dbltofix(LGAIN));
				++lptr2;
			}
			lptr += stride;
		}
		hptr = &a[llen * stride];
		for (unsigned n = numrows - llen; n > 0; --n) {
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				hptr2[0] = jpc_fix_mul(hptr2[0], jpc_dbltofix(HGAIN));
				++hptr2;
			}
			hptr += stride;
		}
#endif

	} else {

#if defined(WT_LENONE)
		if (parity) {
			lptr2 = &a[0];
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				//lptr2[0] <<= 1;
				lptr2[0] = jpc_fix_asl(lptr2[0], 1);
				++lptr2;
			}
		}
#endif

	}

}

static void jpc_ns_fwdlift_colres(jpc_fix_t *a, unsigned numrows, unsigned numcols,
  unsigned stride, bool parity)
{

	jpc_fix_t *lptr;
	jpc_fix_t *hptr;
	register jpc_fix_t *lptr2;
	register jpc_fix_t *hptr2;

	const unsigned llen = (numrows + !parity) >> 1;
	const bool end_parity = parity == (numrows & 1);

	if (numrows > 1) {

		/* Apply the first lifting step. */
		lptr = &a[0];
		hptr = &a[llen * stride];
		if (parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				jpc_fix_pluseq(hptr2[0], jpc_fix_mul(jpc_dbltofix(2.0 * ALPHA),
				  lptr2[0]));
				++hptr2;
				++lptr2;
			}
			hptr += stride;
		}
		for (unsigned n = numrows - llen - parity - end_parity; n > 0; --n) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				jpc_fix_pluseq(hptr2[0], jpc_fix_mul(jpc_dbltofix(ALPHA),
				  jpc_fix_add(lptr2[0], lptr2[stride])));
				++lptr2;
				++hptr2;
			}
			hptr += stride;
			lptr += stride;
		}
		if (end_parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				jpc_fix_pluseq(hptr2[0], jpc_fix_mul(jpc_dbltofix(2.0 * ALPHA),
				  lptr2[0]));
				++lptr2;
				++hptr2;
			}
		}

		/* Apply the second lifting step. */
		lptr = &a[0];
		hptr = &a[llen * stride];
		if (!parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				jpc_fix_pluseq(lptr2[0], jpc_fix_mul(jpc_dbltofix(2.0 * BETA),
				  hptr2[0]));
				++lptr2;
				++hptr2;
			}
			lptr += stride;
		}
		for (unsigned n = llen - (!parity) - (parity != (numrows & 1)); n > 0; --n) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				jpc_fix_pluseq(lptr2[0], jpc_fix_mul(jpc_dbltofix(BETA),
				  jpc_fix_add(hptr2[0], hptr2[stride])));
				++lptr2;
				++hptr2;
			}
			lptr += stride;
			hptr += stride;
		}
		if (parity != (numrows & 1)) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				jpc_fix_pluseq(lptr2[0], jpc_fix_mul(jpc_dbltofix(2.0 * BETA),
				  hptr2[0]));
				++lptr2;
				++hptr2;
			}
		}

		/* Apply the third lifting step. */
		lptr = &a[0];
		hptr = &a[llen * stride];
		if (parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				jpc_fix_pluseq(hptr2[0], jpc_fix_mul(jpc_dbltofix(2.0 * GAMMA),
				  lptr2[0]));
				++hptr2;
				++lptr2;
			}
			hptr += stride;
		}
		for (unsigned n = numrows - llen - parity - end_parity; n > 0; --n) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				jpc_fix_pluseq(hptr2[0], jpc_fix_mul(jpc_dbltofix(GAMMA),
				  jpc_fix_add(lptr2[0], lptr2[stride])));
				++lptr2;
				++hptr2;
			}
			hptr += stride;
			lptr += stride;
		}
		if (end_parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				jpc_fix_pluseq(hptr2[0], jpc_fix_mul(jpc_dbltofix(2.0 * GAMMA),
				  lptr2[0]));
				++lptr2;
				++hptr2;
			}
		}

		/* Apply the fourth lifting step. */
		lptr = &a[0];
		hptr = &a[llen * stride];
		if (!parity) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				jpc_fix_pluseq(lptr2[0], jpc_fix_mul(jpc_dbltofix(2.0 * DELTA),
				  hptr2[0]));
				++lptr2;
				++hptr2;
			}
			lptr += stride;
		}
		for (unsigned n = llen - (!parity) - (parity != (numrows & 1)); n > 0; --n) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				jpc_fix_pluseq(lptr2[0], jpc_fix_mul(jpc_dbltofix(DELTA),
				  jpc_fix_add(hptr2[0], hptr2[stride])));
				++lptr2;
				++hptr2;
			}
			lptr += stride;
			hptr += stride;
		}
		if (parity != (numrows & 1)) {
			lptr2 = lptr;
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				jpc_fix_pluseq(lptr2[0], jpc_fix_mul(jpc_dbltofix(2.0 * DELTA),
				  hptr2[0]));
				++lptr2;
				++hptr2;
			}
		}

		/* Apply the scaling step. */
#if defined(WT_DOSCALE)
		lptr = &a[0];
		for (unsigned n = llen; n > 0; --n) {
			lptr2 = lptr;
			for (unsigned i = 0; i < numcols; ++i) {
				lptr2[0] = jpc_fix_mul(lptr2[0], jpc_dbltofix(LGAIN));
				++lptr2;
			}
			lptr += stride;
		}
		hptr = &a[llen * stride];
		for (unsigned n = numrows - llen; n > 0; --n) {
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				hptr2[0] = jpc_fix_mul(hptr2[0], jpc_dbltofix(HGAIN));
				++hptr2;
			}
			hptr += stride;
		}
#endif

	} else {

#if defined(WT_LENONE)
		if (parity) {
			lptr2 = &a[0];
			for (unsigned i = 0; i < numcols; ++i) {
				//lptr2[0] <<= 1;
				lptr2[0] = jpc_fix_asl(lptr2[0], 1);
				++lptr2;
			}
		}
#endif

	}

}

#ifdef _MSC_VER
/* MSVC doesn't support C99 "restrict" */
#define restrict
#endif

JAS_FORCE_INLINE
static void jpc_invlift_n(jpc_fix_t *restrict dest,
			  const jpc_fix_t *restrict src,
			  jpc_fix_t factor,
			  size_t n)
{
	for (size_t i = 0; i < n; ++i)
		jpc_fix_minuseq(dest[i], jpc_fix_mul(factor, src[i]));
}

JAS_FORCE_INLINE
static void jpc_invlift_pair_stride(jpc_fix_t *restrict dest,
				    const jpc_fix_t *restrict src,
				    jpc_fix_t factor,
				    size_t n, size_t stride)
{
	for (size_t i = 0; i < n; ++i)
		jpc_fix_minuseq(dest[i],
				jpc_fix_mul(factor,
					    jpc_fix_add(src[i], src[i + stride])));
}

JAS_FORCE_INLINE
static void jpc_invlift_pair(jpc_fix_t *restrict dest,
			     const jpc_fix_t *restrict src,
			     jpc_fix_t factor,
			     size_t n)
{
	jpc_invlift_pair_stride(dest, src, factor, n, 1);
}

static void jpc_invlift_pair_with_parity(jpc_fix_t *restrict dest,
					 const jpc_fix_t *restrict src,
					 jpc_fix_t factor,
					 jpc_fix_t border_factor,
					 size_t n,
					 bool start_parity, bool end_parity)
{
	if (start_parity) {
		jpc_fix_minuseq(*dest, jpc_fix_mul(border_factor, *src));
		++dest;
	}

	n -= start_parity + end_parity;
	jpc_invlift_pair(dest, src, factor, n);
	dest += n;
	src += n;

	if (end_parity)
		jpc_fix_minuseq(*dest, jpc_fix_mul(border_factor, *src));
}

static void jpc_invlift_column_with_parity(jpc_fix_t *restrict dest,
					   const jpc_fix_t *restrict src,
					   jpc_fix_t factor,
					   jpc_fix_t border_factor,
					   size_t n_columns,
					   size_t n_rows,
					   size_t stride,
					   bool start_parity, bool end_parity)
{
	if (start_parity) {
		jpc_invlift_n(dest, src, border_factor, n_columns);
		dest += stride;
	}

	n_rows -= start_parity + end_parity;
	for (size_t i = 0; i < n_rows; ++i) {
		jpc_invlift_pair_stride(dest, src, factor,
					n_columns, stride);
		dest += stride;
		src += stride;
	}

	if (end_parity)
		jpc_invlift_n(dest, src, border_factor, n_columns);
}

static void jpc_ns_invlift_row(jpc_fix_t *a, unsigned numcols, bool parity)
{

	register jpc_fix_t *lptr;
	register jpc_fix_t *hptr;

	const unsigned llen = (numcols + !parity) >> 1;
	const bool end_parity = parity == (numcols & 1);

	if (numcols > 1) {

		/* Apply the scaling step. */
#if defined(WT_DOSCALE)
		lptr = &a[0];
		for (unsigned n = llen; n > 0; --n) {
			lptr[0] = jpc_fix_mul(lptr[0], jpc_dbltofix(1.0 / LGAIN));
			++lptr;
		}
		hptr = &a[llen];
		for (unsigned n = numcols - llen; n > 0; --n) {
			hptr[0] = jpc_fix_mul(hptr[0], jpc_dbltofix(1.0 / HGAIN));
			++hptr;
		}
#endif

		/* Apply the first lifting step. */
		jpc_invlift_pair_with_parity(&a[0], &a[llen],
					     jpc_dbltofix(DELTA),
					     jpc_dbltofix(2 * DELTA),
					     llen,
					     !parity, !end_parity);

		/* Apply the second lifting step. */
		jpc_invlift_pair_with_parity(&a[llen], &a[0],
					     jpc_dbltofix(GAMMA),
					     jpc_dbltofix(2 * GAMMA),
					     numcols - llen,
					     parity, end_parity);

		/* Apply the third lifting step. */
		jpc_invlift_pair_with_parity(&a[0], &a[llen],
					     jpc_dbltofix(BETA),
					     jpc_dbltofix(2 * BETA),
					     llen, !parity, !end_parity);

		/* Apply the fourth lifting step. */
		jpc_invlift_pair_with_parity(&a[llen], &a[0],
					     jpc_dbltofix(ALPHA),
					     jpc_dbltofix(2 * ALPHA),
					     numcols - llen,
					     parity, end_parity);

	} else {

#if defined(WT_LENONE)
		if (parity) {
			lptr = &a[0];
			//lptr[0] >>= 1;
			lptr[0] = jpc_fix_asr(lptr[0], 1);
		}
#endif

	}

}

static void jpc_ns_invlift_colgrp(jpc_fix_t *a, unsigned numrows, unsigned stride,
  bool parity)
{

	jpc_fix_t *lptr;
	jpc_fix_t *hptr;
	register jpc_fix_t *lptr2;
	register jpc_fix_t *hptr2;

	const unsigned llen = (numrows + !parity) >> 1;
	const bool end_parity = parity == (numrows & 1);

	if (numrows > 1) {

		/* Apply the scaling step. */
#if defined(WT_DOSCALE)
		lptr = &a[0];
		for (unsigned n = llen; n > 0; --n) {
			lptr2 = lptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				lptr2[0] = jpc_fix_mul(lptr2[0], jpc_dbltofix(1.0 / LGAIN));
				++lptr2;
			}
			lptr += stride;
		}
		hptr = &a[llen * stride];
		for (unsigned n = numrows - llen; n > 0; --n) {
			hptr2 = hptr;
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				hptr2[0] = jpc_fix_mul(hptr2[0], jpc_dbltofix(1.0 / HGAIN));
				++hptr2;
			}
			hptr += stride;
		}
#endif

		/* Apply the first lifting step. */
		jpc_invlift_column_with_parity(&a[0], &a[llen * stride],
					       jpc_dbltofix(DELTA),
					       jpc_dbltofix(2 * DELTA),
					       JPC_QMFB_COLGRPSIZE,
					       llen, stride,
					       !parity, !end_parity);

		/* Apply the second lifting step. */
		jpc_invlift_column_with_parity(&a[llen * stride], &a[0],
					       jpc_dbltofix(GAMMA),
					       jpc_dbltofix(2 * GAMMA),
					       JPC_QMFB_COLGRPSIZE,
					       numrows - llen, stride,
					       parity, end_parity);

		/* Apply the third lifting step. */
		jpc_invlift_column_with_parity(&a[0], &a[llen * stride],
					       jpc_dbltofix(BETA),
					       jpc_dbltofix(2 * BETA),
					       JPC_QMFB_COLGRPSIZE,
					       llen, stride,
					       !parity, !end_parity);

		/* Apply the fourth lifting step. */
		jpc_invlift_column_with_parity(&a[llen * stride], &a[0],
					       jpc_dbltofix(ALPHA),
					       jpc_dbltofix(2 * ALPHA),
					       JPC_QMFB_COLGRPSIZE,
					       numrows - llen, stride,
					       parity, end_parity);

	} else {

#if defined(WT_LENONE)
		if (parity) {
			lptr2 = &a[0];
			for (unsigned i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
				//lptr2[0] >>= 1;
				lptr2[0] = jpc_fix_asr(lptr2[0], 1);
				++lptr2;
			}
		}
#endif

	}

}

static void jpc_ns_invlift_colres(jpc_fix_t *a, unsigned numrows, unsigned numcols,
  unsigned stride, bool parity)
{

	jpc_fix_t *lptr;
	jpc_fix_t *hptr;
	register jpc_fix_t *lptr2;
	register jpc_fix_t *hptr2;

	const unsigned llen = (numrows + !parity) >> 1;
	const bool end_parity = parity == (numrows & 1);

	if (numrows > 1) {

		/* Apply the scaling step. */
#if defined(WT_DOSCALE)
		lptr = &a[0];
		for (unsigned n = llen; n > 0; --n) {
			lptr2 = lptr;
			for (unsigned i = 0; i < numcols; ++i) {
				lptr2[0] = jpc_fix_mul(lptr2[0], jpc_dbltofix(1.0 / LGAIN));
				++lptr2;
			}
			lptr += stride;
		}
		hptr = &a[llen * stride];
		for (unsigned n = numrows - llen; n > 0; --n) {
			hptr2 = hptr;
			for (unsigned i = 0; i < numcols; ++i) {
				hptr2[0] = jpc_fix_mul(hptr2[0], jpc_dbltofix(1.0 / HGAIN));
				++hptr2;
			}
			hptr += stride;
		}
#endif

		/* Apply the first lifting step. */
		jpc_invlift_column_with_parity(&a[0], &a[llen * stride],
					       jpc_dbltofix(DELTA),
					       jpc_dbltofix(2 * DELTA),
					       numcols,
					       llen, stride,
					       !parity, !end_parity);

		/* Apply the second lifting step. */
		jpc_invlift_column_with_parity(&a[llen * stride], &a[0],
					       jpc_dbltofix(GAMMA),
					       jpc_dbltofix(2 * GAMMA),
					       numcols,
					       numrows - llen, stride,
					       parity, end_parity);

		/* Apply the third lifting step. */
		jpc_invlift_column_with_parity(&a[0], &a[llen * stride],
					       jpc_dbltofix(BETA),
					       jpc_dbltofix(2 * BETA),
					       numcols,
					       llen, stride,
					       !parity, !end_parity);

		/* Apply the fourth lifting step. */
		jpc_invlift_column_with_parity(&a[llen * stride], &a[0],
					       jpc_dbltofix(ALPHA),
					       jpc_dbltofix(2 * ALPHA),
					       numcols,
					       numrows - llen, stride,
					       parity, end_parity);

	} else {

#if defined(WT_LENONE)
		if (parity) {
			lptr2 = &a[0];
			for (unsigned i = 0; i < numcols; ++i) {
				//lptr2[0] >>= 1;
				lptr2[0] = jpc_fix_asr(lptr2[0], 1);
				++lptr2;
			}
		}
#endif

	}

}

int jpc_ns_analyze(jpc_fix_t *a, int xstart, int ystart, int width, int height,
  int stride)
{
	jpc_fix_t localbuf[QMFB_SPLITJOINBUFSIZE];
	unsigned bufsize;
	jpc_fix_t *buf;

	const unsigned numrows = height;
	const unsigned numcols = width;
	const unsigned rowparity = ystart & 1;
	const unsigned colparity = xstart & 1;
	jpc_fix_t *startptr;
	const unsigned maxcols = (numcols / JPC_QMFB_COLGRPSIZE) * JPC_QMFB_COLGRPSIZE;

	bufsize = JAS_MAX(width, JPC_QMFB_COLGRPSIZE * height);
	if (bufsize > QMFB_SPLITJOINBUFSIZE) {
		if (!(buf = jas_alloc2(bufsize, sizeof(jpc_fix_t)))) {
			return -1;
		}
	} else {
		buf = localbuf;
		bufsize = QMFB_SPLITJOINBUFSIZE;
	}

	startptr = &a[0];
	for (unsigned i = 0; i < maxcols; i += JPC_QMFB_COLGRPSIZE) {
		jpc_qmfb_split_colgrp(startptr, numrows, stride, rowparity, buf, bufsize);
		jpc_ns_fwdlift_colgrp(startptr, numrows, stride, rowparity);
		startptr += JPC_QMFB_COLGRPSIZE;
	}
	if (maxcols < numcols) {
		jpc_qmfb_split_colres(startptr, numrows, numcols - maxcols, stride,
		  rowparity, buf, bufsize);
		jpc_ns_fwdlift_colres(startptr, numrows, numcols - maxcols, stride,
		  rowparity);
	}

	startptr = &a[0];
	for (unsigned i = 0; i < numrows; ++i) {
		jpc_qmfb_split_row(startptr, numcols, colparity, buf, bufsize);
		jpc_ns_fwdlift_row(startptr, numcols, colparity);
		startptr += stride;
	}

	if (buf != localbuf) {
		jas_free(buf);
	}

	return 0;
}

int jpc_ns_synthesize(jpc_fix_t *a, int xstart, int ystart, int width,
  int height, int stride)
{
	jpc_fix_t localbuf[QMFB_SPLITJOINBUFSIZE];
	unsigned bufsize;
	jpc_fix_t *buf;

	const unsigned numrows = height;
	const unsigned numcols = width;
	const unsigned rowparity = ystart & 1;
	const unsigned colparity = xstart & 1;
	jpc_fix_t *startptr;

	const unsigned maxcols = (numcols / JPC_QMFB_COLGRPSIZE) * JPC_QMFB_COLGRPSIZE;
	bufsize = JAS_MAX(width, JPC_QMFB_COLGRPSIZE * height);
	if (bufsize > QMFB_SPLITJOINBUFSIZE) {
		if (!(buf = jas_alloc2(bufsize, sizeof(jpc_fix_t)))) {
			return -1;
		}
	} else {
		buf = localbuf;
		bufsize = QMFB_SPLITJOINBUFSIZE;
	}

	startptr = &a[0];
	for (unsigned i = 0; i < numrows; ++i) {
		jpc_ns_invlift_row(startptr, numcols, colparity);
		jpc_qmfb_join_row(startptr, numcols, colparity, buf, bufsize);
		startptr += stride;
	}

	startptr = &a[0];
	for (unsigned i = 0; i < maxcols; i += JPC_QMFB_COLGRPSIZE) {
		jpc_ns_invlift_colgrp(startptr, numrows, stride, rowparity);
		jpc_qmfb_join_colgrp(startptr, numrows, stride, rowparity, buf, bufsize);
		startptr += JPC_QMFB_COLGRPSIZE;
	}
	if (maxcols < numcols) {
		jpc_ns_invlift_colres(startptr, numrows, numcols - maxcols, stride,
		  rowparity);
		jpc_qmfb_join_colres(startptr, numrows, numcols - maxcols, stride,
		  rowparity, buf, bufsize);
	}

	if (buf != localbuf) {
		jas_free(buf);
	}

	return 0;
}
