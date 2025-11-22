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
 * $Id$
 */

/******************************************************************************\
* Includes.
\******************************************************************************/

#include "jpc_t1cod.h"
#include "jpc_cod.h"
#include "jpc_cs.h"
#include "jpc_mqcod.h"
#include "jpc_tsfb.h"

#include "jasper/jas_math.h"

#include <stdlib.h>
#include <assert.h>
#include <math.h>

JAS_ATTRIBUTE_CONST
static double jpc_pow2i(int n);

/******************************************************************************\
* Global data.
\******************************************************************************/

uint_least8_t jpc_zcctxnolut[4 * 256];
bool jpc_spblut[256];
uint_least8_t jpc_scctxnolut[256];
uint_least8_t jpc_magctxnolut[4096];

jpc_fix_t jpc_signmsedec[1 << JPC_NMSEDEC_BITS];
jpc_fix_t jpc_refnmsedec[1 << JPC_NMSEDEC_BITS];
jpc_fix_t jpc_signmsedec0[1 << JPC_NMSEDEC_BITS];
jpc_fix_t jpc_refnmsedec0[1 << JPC_NMSEDEC_BITS];

jpc_mqctx_t jpc_mqctxs[JPC_NUMCTXS];

/******************************************************************************\
*
\******************************************************************************/

JAS_ATTRIBUTE_CONST
static uint_least8_t jpc_getzcctxno(unsigned f, enum jpc_tsfb_orient orient);

JAS_ATTRIBUTE_CONST
static bool jpc_getspb(unsigned f);

JAS_ATTRIBUTE_CONST
static uint_least8_t jpc_getscctxno(unsigned f);

JAS_ATTRIBUTE_CONST
static uint_least8_t jpc_getmagctxno(unsigned f);

/******************************************************************************\
* Code.
\******************************************************************************/

enum jpc_passtype JPC_PASSTYPE(unsigned passno)
{
	unsigned passtype;
	switch (passno % 3) {
	case 0:
		passtype = JPC_CLNPASS;
		break;
	case 1:
		passtype = JPC_SIGPASS;
		break;
	case 2:
		passtype = JPC_REFPASS;
		break;
	default:
		assert(0);
		JAS_UNREACHABLE();
	}
	return passtype;
}

unsigned JPC_NOMINALGAIN(unsigned qmfbid, unsigned numlvls, unsigned lvlno, enum jpc_tsfb_orient orient)
{
	JAS_UNUSED(numlvls);

	if (qmfbid == JPC_COX_INS) {
		return 0;
	}
	assert(qmfbid == JPC_COX_RFT);
	if (lvlno == 0) {
		assert(orient == JPC_TSFB_LL);
		return 0;
	} else {
		switch (orient) {
		case JPC_TSFB_LH:
		case JPC_TSFB_HL:
			return 1;
		case JPC_TSFB_HH:
			return 2;
		default:
			assert(false);
			JAS_UNREACHABLE();
		}
	}
	JAS_UNREACHABLE();
}

/******************************************************************************\
* Coding pass related functions.
\******************************************************************************/

enum jpc_segtype JPC_SEGTYPE(unsigned passno, unsigned firstpassno, bool bypass)
{
	if (bypass) {
		enum jpc_passtype passtype = JPC_PASSTYPE(passno);
		if (passtype == JPC_CLNPASS) {
			return JPC_SEG_MQ;
		}
		return ((passno < firstpassno + 10) ? JPC_SEG_MQ : JPC_SEG_RAW);
	} else {
		return JPC_SEG_MQ;
	}
}

unsigned JPC_SEGPASSCNT(unsigned passno, unsigned firstpassno, unsigned numpasses, bool bypass, bool termall)
{
	unsigned ret;

	if (termall) {
		ret = 1;
	} else if (bypass) {
		if (passno < firstpassno + 10) {
			ret = 10 - (passno - firstpassno);
		} else {
			enum jpc_passtype passtype = JPC_PASSTYPE(passno);
			switch (passtype) {
			case JPC_SIGPASS:
				ret = 2;
				break;
			case JPC_REFPASS:
				ret = 1;
				break;
			case JPC_CLNPASS:
				ret = 1;
				break;
			default:
				assert(0);
				JAS_UNREACHABLE();
			}
		}
	} else {
		ret = JPC_PREC * 3 - 2;
	}
	ret = JAS_MIN(ret, numpasses - passno);
	return ret;
}

bool JPC_ISTERMINATED(unsigned passno, unsigned firstpassno, unsigned numpasses, bool termall,
  bool lazy)
{
	if (passno - firstpassno == numpasses - 1) {
		return true;
	} else {
		unsigned n = JPC_SEGPASSCNT(passno, firstpassno, numpasses, lazy, termall);
		return n <= 1;
	}
}

/******************************************************************************\
* Lookup table code.
\******************************************************************************/

void jpc_initluts()
{
	float u;
	float v;
	float t;

	for (unsigned orient = 0; orient < 4; ++orient) {
		for (unsigned i = 0; i < 256; ++i) {
			jpc_zcctxnolut[(orient << 8) | i] = jpc_getzcctxno(i, orient);
		}
	}

	for (unsigned i = 0; i < 256; ++i) {
		jpc_spblut[i] = jpc_getspb(i << 4);
	}

	for (unsigned i = 0; i < 256; ++i) {
		jpc_scctxnolut[i] = jpc_getscctxno(i << 4);
	}

	for (unsigned refine = 0; refine < 2; ++refine) {
		for (unsigned i = 0; i < 2048; ++i) {
			jpc_magctxnolut[(refine << 11) + i] = jpc_getmagctxno((refine ? JPC_REFINE : 0) | i);
		}
	}

	for (unsigned i = 0; i < (1 << JPC_NMSEDEC_BITS); ++i) {
		t = i * jpc_pow2i(-JPC_NMSEDEC_FRACBITS);
		u = t;
		v = t - 1.5f;
		jpc_signmsedec[i] = jpc_dbltofix(floor((u * u - v * v) *
		  jpc_pow2i(JPC_NMSEDEC_FRACBITS) + 0.5) /
		  jpc_pow2i(JPC_NMSEDEC_FRACBITS));
/* XXX - this calc is not correct */
		jpc_signmsedec0[i] = jpc_dbltofix(floor((u * u) *
		  jpc_pow2i(JPC_NMSEDEC_FRACBITS) + 0.5) /
		  jpc_pow2i(JPC_NMSEDEC_FRACBITS));
		u = t - 1.0f;
		if (i & (1 << (JPC_NMSEDEC_BITS - 1))) {
			v = t - 1.5f;
		} else {
			v = t - 0.5f;
		}
		jpc_refnmsedec[i] = jpc_dbltofix(floor((u * u - v * v) *
		  jpc_pow2i(JPC_NMSEDEC_FRACBITS) + 0.5) /
		  jpc_pow2i(JPC_NMSEDEC_FRACBITS));
/* XXX - this calc is not correct */
		jpc_refnmsedec0[i] = jpc_dbltofix(floor((u * u) *
		  jpc_pow2i(JPC_NMSEDEC_FRACBITS) + 0.5) /
		  jpc_pow2i(JPC_NMSEDEC_FRACBITS));
	}
}

static uint_least8_t jpc_getzcctxno(unsigned f, enum jpc_tsfb_orient orient)
{
	assert(orient < 4);

	unsigned n;
	unsigned t;
	unsigned hv;

	unsigned h = ((f & JPC_WSIG) != 0) + ((f & JPC_ESIG) != 0);
	unsigned v = ((f & JPC_NSIG) != 0) + ((f & JPC_SSIG) != 0);
	const unsigned d = ((f & JPC_NWSIG) != 0) + ((f & JPC_NESIG) != 0) + ((f & JPC_SESIG) != 0) + ((f & JPC_SWSIG) != 0);
	switch (orient) {
	case JPC_TSFB_HL:
		t = h;
		h = v;
		v = t;
		/* fall through */
	case JPC_TSFB_LL:
	case JPC_TSFB_LH:
		if (!h) {
			if (!v) {
				if (!d) {
					n = 0;
				} else if (d == 1) {
					n = 1;
				} else {
					n = 2;
				}
			} else if (v == 1) {
				n = 3;
			} else {
				n = 4;
			}
		} else if (h == 1) {
			if (!v) {
				if (!d) {
					n = 5;
				} else {
					n = 6;
				}
			} else {
				n = 7;
			}
		} else {
			n = 8;
		}
		break;
	case JPC_TSFB_HH:
		hv = h + v;
		if (!d) {
			if (!hv) {
				n = 0;
			} else if (hv == 1) {
				n = 1;
			} else {
				n = 2;
			}
		} else if (d == 1) {
			if (!hv) {
				n = 3;
			} else if (hv == 1) {
				n = 4;
			} else {
				n = 5;
			}
		} else if (d == 2) {
			if (!hv) {
				n = 6;
			} else {
				n = 7;
			}
		} else {
			n = 8;
		}
		break;

	default:
		assert(false);
		JAS_UNREACHABLE();
	}
	assert(n < JPC_NUMZCCTXS);
	return JPC_ZCCTXNO + n;
}

static bool jpc_getspb(unsigned f)
{
	int hc;
	int vc;
	bool n;

	hc = JAS_MIN(((f & (JPC_ESIG | JPC_ESGN)) == JPC_ESIG) + ((f & (JPC_WSIG | JPC_WSGN)) == JPC_WSIG), 1) -
	  JAS_MIN(((f & (JPC_ESIG | JPC_ESGN)) == (JPC_ESIG | JPC_ESGN)) + ((f & (JPC_WSIG | JPC_WSGN)) == (JPC_WSIG | JPC_WSGN)), 1);
	vc = JAS_MIN(((f & (JPC_NSIG | JPC_NSGN)) == JPC_NSIG) + ((f & (JPC_SSIG | JPC_SSGN)) == JPC_SSIG), 1) -
	  JAS_MIN(((f & (JPC_NSIG | JPC_NSGN)) == (JPC_NSIG | JPC_NSGN)) + ((f & (JPC_SSIG | JPC_SSGN)) == (JPC_SSIG | JPC_SSGN)), 1);
	if (!hc && !vc) {
		n = 0;
	} else {
		n = (!(hc > 0 || (!hc && vc > 0)));
	}
	return n;
}

static uint_least8_t jpc_getscctxno(unsigned f)
{
	int hc;
	int vc;

	hc = JAS_MIN(((f & (JPC_ESIG | JPC_ESGN)) == JPC_ESIG) + ((f & (JPC_WSIG | JPC_WSGN)) == JPC_WSIG),
	  1) - JAS_MIN(((f & (JPC_ESIG | JPC_ESGN)) == (JPC_ESIG | JPC_ESGN)) +
	  ((f & (JPC_WSIG | JPC_WSGN)) == (JPC_WSIG | JPC_WSGN)), 1);
	vc = JAS_MIN(((f & (JPC_NSIG | JPC_NSGN)) == JPC_NSIG) + ((f & (JPC_SSIG | JPC_SSGN)) == JPC_SSIG),
	  1) - JAS_MIN(((f & (JPC_NSIG | JPC_NSGN)) == (JPC_NSIG | JPC_NSGN)) +
	  ((f & (JPC_SSIG | JPC_SSGN)) == (JPC_SSIG | JPC_SSGN)), 1);
	assert(hc >= -1 && hc <= 1 && vc >= -1 && vc <= 1);
	if (hc < 0) {
		hc = -hc;
		vc = -vc;
	}

	unsigned n;
	if (!hc) {
		if (vc == -1) {
			n = 1;
		} else if (!vc) {
			n = 0;
		} else {
			n = 1;
		}
	} else {
		assert(hc == 1);

		if (vc == -1) {
			n = 2;
		} else if (!vc) {
			n = 3;
		} else {
			n = 4;
		}
	}
	assert(n < JPC_NUMSCCTXS);
	return JPC_SCCTXNO + n;
}

static uint_least8_t jpc_getmagctxno(unsigned f)
{
	unsigned n;

	if (!(f & JPC_REFINE)) {
		n = (f & (JPC_OTHSIGMSK)) ? 1 : 0;
	} else {
		n = 2;
	}

	assert(n < JPC_NUMMAGCTXS);
	return JPC_MAGCTXNO + n;
}

static void jpc_initctxs(jpc_mqctx_t *ctxs)
{
	jpc_mqctx_t *ctx;

	ctx = ctxs;
	for (unsigned i = 0; i < JPC_NUMCTXS; ++i) {
		ctx->mps = 0;
		switch (i) {
		case JPC_UCTXNO:
			ctx->ind = 46;
			break;
		case JPC_ZCCTXNO:
			ctx->ind = 4;
			break;
		case JPC_AGGCTXNO:
			ctx->ind = 3;
			break;
		default:
			ctx->ind = 0;
			break;
		}
		++ctx;
	}
}

void jpc_initmqctxs()
{
	jpc_initctxs(jpc_mqctxs);
}

/* Calculate the real quantity exp2(n), where x is an integer. */
static double jpc_pow2i(int n)
{
	double x;
	double a;

	x = 1.0;
	if (n < 0) {
		a = 0.5;
		n = -n;
	} else {
		a = 2.0;
	}
	while (--n >= 0) {
		x *= a;
	}
	return x;
}
