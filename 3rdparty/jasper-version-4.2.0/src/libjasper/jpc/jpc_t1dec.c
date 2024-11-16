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
 * Tier 1 Decoder
 *
 * $Id$
 */

/******************************************************************************\
* Includes.
\******************************************************************************/

#include "jpc_t1dec.h"
#include "jpc_bs.h"
#include "jpc_mqdec.h"
#include "jpc_t1cod.h"
#include "jpc_dec.h"

#include "jasper/jas_thread.h"
#include "jasper/jas_stream.h"
#include "jasper/jas_math.h"
#include "jasper/jas_debug.h"

#include <assert.h>

/******************************************************************************\
*
\******************************************************************************/

static int jpc_dec_decodecblk(jpc_dec_tile_t *tile, jpc_dec_tcomp_t *tcomp, jpc_dec_band_t *band,
  jpc_dec_cblk_t *cblk, int dopartial, int maxlyrs);
static int dec_sigpass(jpc_mqdec_t *mqdec, unsigned bitpos, enum jpc_tsfb_orient orient,
  bool vcausalflag, jas_matrix_t *flags, jas_matrix_t *data);
static int dec_rawsigpass(jpc_bitstream_t *in, unsigned bitpos,
  bool vcausalflag, jas_matrix_t *flags, jas_matrix_t *data);
static int dec_refpass(jpc_mqdec_t *mqdec, unsigned bitpos,
  jas_matrix_t *flags, jas_matrix_t *data);
static int dec_rawrefpass(jpc_bitstream_t *in, unsigned bitpos,
  jas_matrix_t *flags, jas_matrix_t *data);
static int dec_clnpass(jpc_mqdec_t *mqdec, unsigned bitpos, enum jpc_tsfb_orient orient,
  bool vcausalflag, bool segsymflag, jas_matrix_t *flags, jas_matrix_t *data);

#if defined(JAS_ENABLE_NON_THREAD_SAFE_DEBUGGING)
static size_t t1dec_cnt = 0;
#endif

JAS_FORCE_INLINE
static bool JPC_T1D_GETBIT(jpc_mqdec_t *mqdec, const char *passtypename, const char *symtypename)
{
	bool v = jpc_mqdec_getbit(mqdec);
#if defined(JAS_ENABLE_NON_THREAD_SAFE_DEBUGGING)
	if (jas_get_debug_level() >= 100) {
		jas_logdebugf(100, "index = %zu; passtype = %s; symtype = %s; sym = %d\n",
		  t1dec_cnt, passtypename, symtypename, v);
		++t1dec_cnt;
	}
#else
	JAS_UNUSED(passtypename);
	JAS_UNUSED(symtypename);
#endif
	return v;
}

JAS_FORCE_INLINE
static bool JPC_T1D_GETBITNOSKEW(jpc_mqdec_t *mqdec, const char *passtypename, const char *symtypename)
{
	return JPC_T1D_GETBIT(mqdec, passtypename, symtypename);
}

JAS_FORCE_INLINE
static int JPC_T1D_RAWGETBIT(jpc_bitstream_t *bitstream, const char *passtypename, const char *symtypename)
{
	int v = jpc_bitstream_getbit(bitstream);
#if defined(JAS_ENABLE_NON_THREAD_SAFE_DEBUGGING)
	if (jas_get_debug_level() >= 100) {
		jas_logdebugf(100, "index = %ld; passtype = %s; symtype = %s; sym = %d\n", t1dec_cnt, passtypename, symtypename, v);
		++t1dec_cnt;
	}
#else
	JAS_UNUSED(passtypename);
	JAS_UNUSED(symtypename);
#endif
	return v;
}

/******************************************************************************\
* Code.
\******************************************************************************/

int jpc_dec_decodecblks(jpc_dec_t *dec, jpc_dec_tile_t *tile)
{
	jpc_dec_tcomp_t *tcomp;
	jpc_dec_rlvl_t *rlvl;
	jpc_dec_band_t *band;
	jpc_dec_prc_t *prc;
	jpc_dec_cblk_t *cblk;

	unsigned compcnt;
	for (compcnt = dec->numcomps, tcomp = tile->tcomps; compcnt > 0;
	  --compcnt, ++tcomp) {
		unsigned rlvlcnt;
		for (rlvlcnt = tcomp->numrlvls, rlvl = tcomp->rlvls;
		  rlvlcnt > 0; --rlvlcnt, ++rlvl) {
			if (!rlvl->bands) {
				continue;
			}
			unsigned bandcnt;
			for (bandcnt = rlvl->numbands, band = rlvl->bands;
			  bandcnt > 0; --bandcnt, ++band) {
				if (!band->data) {
					continue;
				}
				unsigned prccnt;
				for (prccnt = rlvl->numprcs, prc = band->prcs;
				  prccnt > 0; --prccnt, ++prc) {
					if (!prc->cblks) {
						continue;
					}
					unsigned cblkcnt;
					for (cblkcnt = prc->numcblks,
					  cblk = prc->cblks; cblkcnt > 0;
					  --cblkcnt, ++cblk) {
						if (jpc_dec_decodecblk(tile, tcomp,
						  band, cblk, 1, JPC_MAXLYRS)) {
							return -1;
						}
					}
				}

			}
		}
	}

	return 0;
}

static int jpc_dec_decodecblk(jpc_dec_tile_t *tile, jpc_dec_tcomp_t *tcomp, jpc_dec_band_t *band,
  jpc_dec_cblk_t *cblk, int dopartial, int maxlyrs)
{
	jpc_dec_seg_t *seg;
	int bpno;
	int ret;
	int filldata;
	int fillmask;

	const size_t compno = tcomp - tile->tcomps;
	const jpc_dec_ccp_t *const ccp = &tile->cp->ccps[compno];

	/* The MQ decoder. */
	jpc_mqdec_t *mqdec = NULL;

	/* The raw bit stream decoder. */
	jpc_bitstream_t *nulldec = NULL;

	/* The per-sample state information for this code block. */
	/* Note: matrix is assumed to be zeroed */
	jas_matrix_t *const flags = jas_matrix_create(jas_matrix_numrows(cblk->data) + 2, jas_matrix_numcols(cblk->data) + 2);
	if (!flags)
		goto error;

	seg = cblk->segs.head;
	while (seg && (seg != cblk->curseg || dopartial) && (maxlyrs < 0 ||
	  seg->lyrno < (unsigned)maxlyrs)) {
		assert(seg->numpasses >= seg->maxpasses || dopartial);
		assert(seg->stream);
		jas_stream_rewind(seg->stream);
		jas_stream_setrwcount(seg->stream, 0);
		if (seg->type == JPC_SEG_MQ) {
			if (!mqdec) {
				if (!(mqdec = jpc_mqdec_create(JPC_NUMCTXS, 0))) {
					goto error;
				}
				jpc_mqdec_setctxs(mqdec, JPC_NUMCTXS, jpc_mqctxs);
			}
			jpc_mqdec_setinput(mqdec, seg->stream);
			jpc_mqdec_init(mqdec);
		} else {
			assert(seg->type == JPC_SEG_RAW);
			if (!nulldec) {
				if (!(nulldec = jpc_bitstream_sopen(seg->stream, "r"))) {
					goto error;
				}
			}
		}


		for (unsigned i = 0; i < seg->numpasses; ++i) {
			if (cblk->numimsbs > band->numbps) {
				if (ccp->roishift <= 0) {
					jas_logwarnf("warning: corrupt code stream\n");
				} else {
					if (cblk->numimsbs < ccp->roishift - band->numbps) {
						jas_logwarnf("warning: corrupt code stream\n");
					}
				}
			}
			bpno = band->roishift + band->numbps - 1 - (cblk->numimsbs +
			  (seg->passno + i - cblk->firstpassno + 2) / 3);
if (bpno < 0) {
	goto premature_exit;
}
			enum jpc_passtype passtype = JPC_PASSTYPE(seg->passno + i);
			assert(bpno >= 0 && bpno < 31);
			switch (passtype) {
			case JPC_SIGPASS:
				ret = (seg->type == JPC_SEG_MQ) ? dec_sigpass(mqdec, bpno, band->orient,
				  (ccp->cblkctx & JPC_COX_VSC) != 0,
				  flags, cblk->data) :
				  dec_rawsigpass(nulldec, bpno,
				  (ccp->cblkctx & JPC_COX_VSC) != 0,
				  flags, cblk->data);
				break;
			case JPC_REFPASS:
				ret = (seg->type == JPC_SEG_MQ) ?
				  dec_refpass(mqdec, bpno,
				  flags, cblk->data) :
				  dec_rawrefpass(nulldec, bpno,
				  flags, cblk->data);
				break;
			case JPC_CLNPASS:
				assert(seg->type == JPC_SEG_MQ);
				ret = dec_clnpass(mqdec, bpno,
				  band->orient, (ccp->cblkctx &
				  JPC_COX_VSC) != 0, (ccp->cblkctx &
				  JPC_COX_SEGSYM) != 0, flags,
				  cblk->data);
				break;
			default:
				assert(false);
				JAS_UNREACHABLE();
			}
			/* Do we need to reset after each coding pass? */
			if ((ccp->cblkctx & JPC_COX_RESET) && mqdec) {
				jpc_mqdec_setctxs(mqdec, JPC_NUMCTXS, jpc_mqctxs);
			}

			if (ret) {
				jas_logerrorf("coding pass failed passtype=%d segtype=%d\n", passtype, seg->type);
				goto error;
			}

		}

		if (seg->type == JPC_SEG_MQ) {
/* Note: dont destroy mq decoder because context info will be lost */
		} else {
			assert(seg->type == JPC_SEG_RAW);
			if (ccp->cblkctx & JPC_COX_PTERM) {
				fillmask = 0x7f;
				filldata = 0x2a;
			} else {
				fillmask = 0;
				filldata = 0;
			}
			if ((ret = jpc_bitstream_inalign(nulldec, fillmask,
			  filldata)) < 0) {
				goto error;
			} else if (ret > 0) {
				jas_logwarnf("warning: bad termination pattern detected\n");
			}
			jpc_bitstream_close(nulldec);
			nulldec = 0;
		}

		cblk->curseg = seg->next;
		jpc_seglist_remove(&cblk->segs, seg);
		jpc_seg_destroy(seg);
		seg = cblk->curseg;
	}

	assert(dopartial ? (!cblk->curseg) : 1);

premature_exit:
	if (mqdec)
		jpc_mqdec_destroy(mqdec);
	if (nulldec)
		jpc_bitstream_close(nulldec);
	if (flags)
		jas_matrix_destroy(flags);
	return 0;

error:
	if (mqdec)
		jpc_mqdec_destroy(mqdec);
	if (nulldec)
		jpc_bitstream_close(nulldec);
	if (flags)
		jas_matrix_destroy(flags);
	return -1;
}

/******************************************************************************\
* Code for significance pass.
\******************************************************************************/

JAS_FORCE_INLINE
static void jpc_sigpass_step(jpc_fix_t *fp, size_t frowstep, jpc_fix_t *dp, jpc_fix_t oneplushalf, enum jpc_tsfb_orient orient, jpc_mqdec_t *mqdec, bool vcausalflag)
{
	const jpc_fix_t f = *(fp);
	if ((f & JPC_OTHSIGMSK) && !(f & (JPC_SIG | JPC_VISIT))) {
		jpc_mqdec_setcurctx(mqdec, JPC_GETZCCTXNO(f, orient));

		if (JPC_T1D_GETBIT(mqdec, "SIG", "ZC")) {
			jpc_mqdec_setcurctx(mqdec, JPC_GETSCCTXNO(f));
			bool v = JPC_T1D_GETBIT(mqdec, "SIG", "SC");
			v ^= JPC_GETSPB(f);
			JPC_UPDATEFLAGS4(fp, frowstep, v, vcausalflag);
			*fp |= JPC_SIG;
			*dp = v ? -oneplushalf : oneplushalf;
		}
		*fp |= JPC_VISIT;
	}
}

static int dec_sigpass(jpc_mqdec_t *mqdec, unsigned bitpos, enum jpc_tsfb_orient orient,
  bool vcausalflag, jas_matrix_t *flags, jas_matrix_t *data)
{
	int i;
	jpc_fix_t *fp;
	jpc_fix_t *fstripestart;
	jpc_fix_t *fvscanstart;
	jpc_fix_t *dp;
	jpc_fix_t *dstripestart;
	jpc_fix_t *dvscanstart;

	const unsigned width = jas_matrix_numcols(data);
	const unsigned height = jas_matrix_numrows(data);
	const unsigned frowstep = jas_matrix_rowstep(flags);
	const unsigned drowstep = jas_matrix_rowstep(data);
	const unsigned fstripestep = frowstep << 2;
	const unsigned dstripestep = drowstep << 2;

	const jpc_fix_t one = (jpc_fix_t)1 << bitpos;
	const jpc_fix_t half = one >> 1;
	const jpc_fix_t oneplushalf = one | half;

	fstripestart = jas_matrix_getref(flags, 1, 1);
	dstripestart = jas_matrix_getref(data, 0, 0);
	for (i = height; i > 0; i -= 4, fstripestart += fstripestep,
	  dstripestart += dstripestep) {
		fvscanstart = fstripestart;
		dvscanstart = dstripestart;
		const unsigned vscanlen = JAS_MIN(i, 4);
		for (unsigned j = width; j > 0; --j, ++fvscanstart, ++dvscanstart) {
			fp = fvscanstart;
			dp = dvscanstart;
			unsigned k = vscanlen;

			/* Process first sample in vertical scan. */
			jpc_sigpass_step(fp, frowstep, dp, oneplushalf,
			  orient, mqdec, vcausalflag);
			if (--k <= 0) {
				continue;
			}
			fp += frowstep;
			dp += drowstep;

			/* Process second sample in vertical scan. */
			jpc_sigpass_step(fp, frowstep, dp, oneplushalf,
			  orient, mqdec, 0);
			if (--k <= 0) {
				continue;
			}
			fp += frowstep;
			dp += drowstep;

			/* Process third sample in vertical scan. */
			jpc_sigpass_step(fp, frowstep, dp, oneplushalf,
			  orient, mqdec, 0);
			if (--k <= 0) {
				continue;
			}
			fp += frowstep;
			dp += drowstep;

			/* Process fourth sample in vertical scan. */
			jpc_sigpass_step(fp, frowstep, dp, oneplushalf,
			  orient, mqdec, 0);
		}
	}
	return 0;
}

JAS_FORCE_INLINE
static int jpc_rawsigpass_step(jpc_fix_t *fp, size_t frowstep, jpc_fix_t *dp, jpc_fix_t oneplushalf, jpc_bitstream_t *in, bool vcausalflag)
{
	const jpc_fix_t f = *fp;
	if ((f & JPC_OTHSIGMSK) && !(f & (JPC_SIG | JPC_VISIT))) {
		int v = JPC_T1D_RAWGETBIT(in, "SIG", "ZC");
		if (v < 0) {
			return -1;
		}
		if (v) {
			v = JPC_T1D_RAWGETBIT(in, "SIG", "SC");
			if (v < 0) {
				return -1;
			}
			JPC_UPDATEFLAGS4(fp, frowstep, v, vcausalflag);
			*fp |= JPC_SIG;
			*dp = v ? -oneplushalf : oneplushalf;
		}
		*fp |= JPC_VISIT;
	}

	return 0;
}

static int dec_rawsigpass(jpc_bitstream_t *in, unsigned bitpos, bool vcausalflag,
  jas_matrix_t *flags, jas_matrix_t *data)
{
	int i;
	jpc_fix_t *fp;
	jpc_fix_t *fstripestart;
	jpc_fix_t *fvscanstart;
	jpc_fix_t *dp;
	jpc_fix_t *dstripestart;
	jpc_fix_t *dvscanstart;

	const unsigned width = jas_matrix_numcols(data);
	const unsigned height = jas_matrix_numrows(data);
	const unsigned frowstep = jas_matrix_rowstep(flags);
	const unsigned drowstep = jas_matrix_rowstep(data);
	const unsigned fstripestep = frowstep << 2;
	const unsigned dstripestep = drowstep << 2;

	const jpc_fix_t one = (jpc_fix_t)1 << bitpos;
	const jpc_fix_t half = one >> 1;
	const jpc_fix_t oneplushalf = one | half;

	fstripestart = jas_matrix_getref(flags, 1, 1);
	dstripestart = jas_matrix_getref(data, 0, 0);
	for (i = height; i > 0; i -= 4, fstripestart += fstripestep,
	  dstripestart += dstripestep) {
		fvscanstart = fstripestart;
		dvscanstart = dstripestart;
		const unsigned vscanlen = JAS_MIN(i, 4);
		for (unsigned j = width; j > 0; --j, ++fvscanstart, ++dvscanstart) {
			fp = fvscanstart;
			dp = dvscanstart;
			unsigned k = vscanlen;

			/* Process first sample in vertical scan. */
			if (jpc_rawsigpass_step(fp, frowstep, dp, oneplushalf, in, vcausalflag))
				return -1;

			if (--k <= 0) {
				continue;
			}
			fp += frowstep;
			dp += drowstep;

			/* Process second sample in vertical scan. */
			if (jpc_rawsigpass_step(fp, frowstep, dp, oneplushalf, in, 0))
				return -1;

			if (--k <= 0) {
				continue;
			}
			fp += frowstep;
			dp += drowstep;

			/* Process third sample in vertical scan. */
			if (jpc_rawsigpass_step(fp, frowstep, dp, oneplushalf, in, 0))
				return -1;

			if (--k <= 0) {
				continue;
			}
			fp += frowstep;
			dp += drowstep;

			/* Process fourth sample in vertical scan. */
			jpc_rawsigpass_step(fp, frowstep, dp, oneplushalf,
			  in, 0);

		}
	}
	return 0;
}

/******************************************************************************\
* Code for refinement pass.
\******************************************************************************/

JAS_FORCE_INLINE
static void jpc_refpass_step(jpc_fix_t *fp, jpc_fix_t *dp, jpc_fix_t poshalf, jpc_fix_t neghalf, jpc_mqdec_t *mqdec)
{
	if (((*(fp)) & (JPC_SIG | JPC_VISIT)) == JPC_SIG) { \
		jpc_mqdec_setcurctx(mqdec, JPC_GETMAGCTXNO(*fp));
		const bool v = JPC_T1D_GETBITNOSKEW(mqdec, "REF", "MR");
		const jpc_fix_t t = v ? poshalf : neghalf;
		*dp += *dp < 0 ? -t : t;
		*fp |= JPC_REFINE;
	}
}

static int dec_refpass(jpc_mqdec_t *mqdec, unsigned bitpos,
  jas_matrix_t *flags, jas_matrix_t *data)
{
	int i;
	jpc_fix_t *fp;
	jpc_fix_t *fstripestart;
	jpc_fix_t *fvscanstart;
	jpc_fix_t *dp;
	jpc_fix_t *dstripestart;
	jpc_fix_t *dvscanstart;

	const unsigned width = jas_matrix_numcols(data);
	const unsigned height = jas_matrix_numrows(data);
	const unsigned frowstep = jas_matrix_rowstep(flags);
	const unsigned drowstep = jas_matrix_rowstep(data);
	const unsigned fstripestep = frowstep << 2;
	const unsigned dstripestep = drowstep << 2;

	const jpc_fix_t one = (jpc_fix_t)1 << bitpos;
	const jpc_fix_t poshalf = one >> 1;
	const jpc_fix_t neghalf = bitpos > 0 ? -poshalf : -1;

	fstripestart = jas_matrix_getref(flags, 1, 1);
	dstripestart = jas_matrix_getref(data, 0, 0);
	for (i = height; i > 0; i -= 4, fstripestart += fstripestep,
	  dstripestart += dstripestep) {
		fvscanstart = fstripestart;
		dvscanstart = dstripestart;
		const unsigned vscanlen = JAS_MIN(i, 4);
		for (unsigned j = width; j > 0; --j, ++fvscanstart, ++dvscanstart) {
			fp = fvscanstart;
			dp = dvscanstart;

			for (unsigned k = 0; k < vscanlen; ++k) {
				jpc_refpass_step(fp, dp, poshalf, neghalf, mqdec);
				fp += frowstep;
				dp += drowstep;
			}
		}
	}

	return 0;
}

static int jpc_rawrefpass_step(jpc_fix_t *fp, jpc_fix_t *dp, jpc_fix_t poshalf, jpc_fix_t neghalf, jpc_bitstream_t *in)
{
	if ((*fp & (JPC_SIG | JPC_VISIT)) == JPC_SIG) {
		int v = JPC_T1D_RAWGETBIT(in, "REF", "MAGREF");
		if (v < 0) {
			return -1;
		}
		jpc_fix_t t = v ? poshalf : neghalf;
		*dp += *dp < 0 ? -t : t;
		*fp |= JPC_REFINE;
	}

	return 0;
}

static int dec_rawrefpass(jpc_bitstream_t *in, unsigned bitpos,
  jas_matrix_t *flags, jas_matrix_t *data)
{
	int i;
	jpc_fix_t *fp;
	jpc_fix_t *fstripestart;
	jpc_fix_t *fvscanstart;
	jpc_fix_t *dp;
	jpc_fix_t *dstripestart;
	jpc_fix_t *dvscanstart;

	const unsigned width = jas_matrix_numcols(data);
	const unsigned height = jas_matrix_numrows(data);
	const unsigned frowstep = jas_matrix_rowstep(flags);
	const unsigned drowstep = jas_matrix_rowstep(data);
	const unsigned fstripestep = frowstep << 2;
	const unsigned dstripestep = drowstep << 2;

	const jpc_fix_t one = (jpc_fix_t)1 << bitpos;
	const jpc_fix_t poshalf = one >> 1;
	const jpc_fix_t neghalf = bitpos > 0 ? -poshalf : -1;

	fstripestart = jas_matrix_getref(flags, 1, 1);
	dstripestart = jas_matrix_getref(data, 0, 0);
	for (i = height; i > 0; i -= 4, fstripestart += fstripestep,
	  dstripestart += dstripestep) {
		fvscanstart = fstripestart;
		dvscanstart = dstripestart;
		const unsigned vscanlen = JAS_MIN(i, 4);
		for (unsigned j = width; j > 0; --j, ++fvscanstart, ++dvscanstart) {
			fp = fvscanstart;
			dp = dvscanstart;

			for (unsigned k = 0; k < vscanlen; ++k) {
				if (jpc_rawrefpass_step(fp, dp, poshalf, neghalf, in))
					return -1;
				fp += frowstep;
				dp += drowstep;
			}
		}
	}
	return 0;
}

/******************************************************************************\
* Code for cleanup pass.
\******************************************************************************/

#define	jpc_clnpass_step(f, fp, frowstep, dp, oneplushalf, orient, mqdec, flabel, plabel, vcausalflag) \
{ \
flabel \
	if (!((f) & (JPC_SIG | JPC_VISIT))) { \
		jpc_mqdec_setcurctx((mqdec), JPC_GETZCCTXNO((f), (orient))); \
		if (JPC_T1D_GETBIT((mqdec), "CLN", "ZC")) { \
plabel \
			/* Coefficient is significant. */ \
			jpc_mqdec_setcurctx((mqdec), JPC_GETSCCTXNO(f)); \
			bool v = JPC_T1D_GETBIT((mqdec), "CLN", "SC"); \
			v ^= JPC_GETSPB(f); \
			*(dp) = (v) ? (-(jpc_fix_t)(oneplushalf)) : (jpc_fix_t)(oneplushalf); \
			JPC_UPDATEFLAGS4((fp), (frowstep), v, (vcausalflag)); \
			*(fp) |= JPC_SIG; \
		} \
	} \
	/* XXX - Is this correct?  Can aggregation cause some VISIT bits not to be reset?  Check. */ \
	*(fp) &= ~JPC_VISIT; \
}

static int dec_clnpass(jpc_mqdec_t *mqdec, unsigned bitpos, enum jpc_tsfb_orient orient,
  bool vcausalflag, bool segsymflag, jas_matrix_t *flags, jas_matrix_t *data)
{
	int f;

	jpc_fix_t *fp;
	jpc_fix_t *fstripestart;
	jpc_fix_t *fvscanstart;

	jpc_fix_t *dp;
	jpc_fix_t *dstripestart;
	jpc_fix_t *dvscanstart;

	const jpc_fix_t one = (jpc_fix_t)1 << bitpos;
	const jpc_fix_t half = one >> 1;
	const jpc_fix_t oneplushalf = one | half;

	const unsigned width = jas_matrix_numcols(data);
	const unsigned height = jas_matrix_numrows(data);

	const unsigned frowstep = jas_matrix_rowstep(flags);
	const unsigned drowstep = jas_matrix_rowstep(data);
	const unsigned fstripestep = frowstep << 2;
	const unsigned dstripestep = drowstep << 2;

	fstripestart = jas_matrix_getref(flags, 1, 1);
	dstripestart = jas_matrix_getref(data, 0, 0);
	for (unsigned i = 0; i < height; i += 4, fstripestart += fstripestep,
	  dstripestart += dstripestep) {
		fvscanstart = fstripestart;
		dvscanstart = dstripestart;
		const unsigned vscanlen = JAS_MIN(4, height - i);
		for (unsigned j = width; j > 0; --j, ++fvscanstart, ++dvscanstart) {
			fp = fvscanstart;
			unsigned k;
			if (vscanlen >= 4 && (!((*fp) & (JPC_SIG | JPC_VISIT |
			  JPC_OTHSIGMSK))) && (fp += frowstep, !((*fp) & (JPC_SIG |
			  JPC_VISIT | JPC_OTHSIGMSK))) && (fp += frowstep, !((*fp) &
			  (JPC_SIG | JPC_VISIT | JPC_OTHSIGMSK))) && (fp += frowstep,
			  !((*fp) & (JPC_SIG | JPC_VISIT | JPC_OTHSIGMSK)))) {

				jpc_mqdec_setcurctx(mqdec, JPC_AGGCTXNO);
				if (!JPC_T1D_GETBIT(mqdec, "CLN", "AGG")) {
					continue;
				}
				jpc_mqdec_setcurctx(mqdec, JPC_UCTXNO);
				unsigned runlen = JPC_T1D_GETBITNOSKEW(mqdec, "CLN", "RL");
				runlen = (runlen << 1) | JPC_T1D_GETBITNOSKEW(mqdec, "CLN", "RL");
				f = *(fp = fvscanstart + frowstep * runlen);
				dp = dvscanstart + drowstep * runlen;
				k = vscanlen - runlen;
				switch (runlen) {
				case 0:
					goto clnpass_partial0;
				case 1:
					goto clnpass_partial1;
				case 2:
					goto clnpass_partial2;
				case 3:
					goto clnpass_partial3;
				}
			} else {
				f = *(fp = fvscanstart);
				dp = dvscanstart;
				k = vscanlen;
				goto clnpass_full0;
			}

			/* Process first sample in vertical scan. */
			jpc_clnpass_step(f, fp, frowstep, dp, oneplushalf, orient,
			  mqdec, clnpass_full0:, clnpass_partial0:,
			  vcausalflag);
			if (--k <= 0) {
				continue;
			}
			fp += frowstep;
			dp += drowstep;

			/* Process second sample in vertical scan. */
			f = *fp;
			jpc_clnpass_step(f, fp, frowstep, dp, oneplushalf, orient,
				mqdec, ;, clnpass_partial1:, 0);
			if (--k <= 0) {
				continue;
			}
			fp += frowstep;
			dp += drowstep;

			/* Process third sample in vertical scan. */
			f = *fp;
			jpc_clnpass_step(f, fp, frowstep, dp, oneplushalf, orient,
				mqdec, ;, clnpass_partial2:, 0);
			if (--k <= 0) {
				continue;
			}
			fp += frowstep;
			dp += drowstep;

			/* Process fourth sample in vertical scan. */
			f = *fp;
			jpc_clnpass_step(f, fp, frowstep, dp, oneplushalf, orient,
				mqdec, ;, clnpass_partial3:, 0);
		}
	}

	if (segsymflag) {
		unsigned segsymval = 0;
		jpc_mqdec_setcurctx(mqdec, JPC_UCTXNO);
		segsymval = (segsymval << 1) | JPC_T1D_GETBITNOSKEW(mqdec, "CLN", "SEGSYM");
		segsymval = (segsymval << 1) | JPC_T1D_GETBITNOSKEW(mqdec, "CLN", "SEGSYM");
		segsymval = (segsymval << 1) | JPC_T1D_GETBITNOSKEW(mqdec, "CLN", "SEGSYM");
		segsymval = (segsymval << 1) | JPC_T1D_GETBITNOSKEW(mqdec, "CLN", "SEGSYM");
		if (segsymval != 0xa) {
			jas_logwarnf("warning: bad segmentation symbol\n");
		}
	}

	return 0;
}
