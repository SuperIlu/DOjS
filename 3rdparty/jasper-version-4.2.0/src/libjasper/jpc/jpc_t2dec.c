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
 * Tier 2 Decoder
 *
 * $Id$
 */

/******************************************************************************\
* Includes.
\******************************************************************************/

#include "jpc_t2dec.h"
#include "jpc_bs.h"
#include "jpc_dec.h"
#include "jpc_cs.h"
#include "jpc_t1cod.h"
#include "jpc_math.h"

#include "jasper/jas_types.h"
#include "jasper/jas_malloc.h"
#include "jasper/jas_math.h"
#include "jasper/jas_stream.h"
#include "jasper/jas_debug.h"

#include <stdio.h>

/******************************************************************************\
*
\******************************************************************************/

static long jpc_dec_lookahead(jas_stream_t *in);
static int jpc_getcommacode(jpc_bitstream_t *in);
static int jpc_getnumnewpasses(jpc_bitstream_t *in);
static int jpc_dec_decodepkt(jpc_dec_t *dec, jas_stream_t *pkthdrstream, jas_stream_t *in, int compno, int lvlno,
  int prcno, unsigned lyrno);

/******************************************************************************\
* Code.
\******************************************************************************/

static int jpc_getcommacode(jpc_bitstream_t *in)
{
	int n;
	int v;

	n = 0;
	for (;;) {
		if ((v = jpc_bitstream_getbit(in)) < 0) {
			return -1;
		}
		if (jpc_bitstream_eof(in)) {
			return -1;
		}
		if (!v) {
			break;
		}
		++n;
	}

	return n;
}

static int jpc_getnumnewpasses(jpc_bitstream_t *in)
{
	int n;

	if ((n = jpc_bitstream_getbit(in)) > 0) {
		if ((n = jpc_bitstream_getbit(in)) > 0) {
			if ((n = jpc_bitstream_getbits(in, 2)) == 3) {
				if ((n = jpc_bitstream_getbits(in, 5)) == 31) {
					if ((n = jpc_bitstream_getbits(in, 7)) >= 0) {
						n += 36 + 1;
					}
				} else if (n >= 0) {
					n += 5 + 1;
				}
			} else if (n >= 0) {
				n += 2 + 1;
			}
		} else if (!n) {
			n += 2;
		}
	} else if (!n) {
		++n;
	}

	return n;
}

static int jpc_dec_decodepkt(jpc_dec_t *dec, jas_stream_t *pkthdrstream, jas_stream_t *in, int compno, int rlvlno,
  int prcno, unsigned lyrno)
{
	jpc_bitstream_t *inb;
	jpc_dec_tcomp_t *tcomp;
	jpc_dec_rlvl_t *rlvl;
	jpc_dec_band_t *band;
	jpc_dec_cblk_t *cblk;
	int m;
	jpc_tagtreenode_t *leaf;
	int included;
	int ret;
	int numnewpasses;
	jpc_dec_seg_t *seg;
	int len;
	int present;
	jpc_ms_t *ms;
	jpc_dec_tile_t *tile;
	jpc_dec_ccp_t *ccp;
	jpc_dec_cp_t *cp;
	jpc_dec_prc_t *prc;
	uint_fast32_t bodylen;
	bool discard;

	/* Avoid compiler warning about possible use of uninitialized
	  variable. */
	bodylen = 0;

	discard = (lyrno >= dec->maxlyrs);

	tile = dec->curtile;
	cp = tile->cp;
	ccp = &cp->ccps[compno];

	/*
	 * Decode the packet header.
	 */

	/* Decode the SOP marker segment if present. */
	if (cp->csty & JPC_COD_SOP) {
		if (jpc_dec_lookahead(in) == JPC_MS_SOP) {
			if (!(ms = jpc_getms(in, dec->cstate))) {
				jas_logerrorf("cannot get marker segment\n");
				return -1;
			}
			if (jpc_ms_gettype(ms) != JPC_MS_SOP) {
				jpc_ms_destroy(ms);
				jas_logerrorf("cannot get (SOP) marker segment\n");
				return -1;
			}
			unsigned int maxNsop = 65536;
			/* checking the packet sequence number */
			if (tile->pi->pktno % maxNsop != ms->parms.sop.seqno) {
				jas_logerrorf("incorrect packet sequence number %d was found, but expected %d\n",
					ms->parms.sop.seqno, tile->pi->pktno % maxNsop);
				jpc_ms_destroy(ms);
				return -1;
			}
			jpc_ms_destroy(ms);
		}
	}

	const uint_least64_t hdroffstart = jas_stream_getrwcount(pkthdrstream);

	if (!(inb = jpc_bitstream_sopen(pkthdrstream, "r"))) {
		jas_logerrorf("cannot open bitstream\n");
		return -1;
	}

	if ((present = jpc_bitstream_getbit(inb)) < 0) {
		jas_logerrorf("cannot get bit\n");
		jpc_bitstream_close(inb);
		return 1;
	}
	JAS_LOGDEBUGF(10, "\n", present);
	JAS_LOGDEBUGF(10, "present=%d ", present);

	/* Is the packet non-empty? */
	if (present) {
		/* The packet is non-empty. */
		tcomp = &tile->tcomps[compno];
		rlvl = &tcomp->rlvls[rlvlno];
		bodylen = 0;
		unsigned bandno;
		for (bandno = 0, band = rlvl->bands; bandno < (unsigned)rlvl->numbands;
		  ++bandno, ++band) {
			if (!band->data) {
				continue;
			}
			prc = &band->prcs[prcno];
			if (!prc->cblks) {
				continue;
			}
			unsigned cblkno;
			unsigned usedcblkcnt = 0;
			for (cblkno = 0, cblk = prc->cblks; cblkno < (unsigned)prc->numcblks;
			  ++cblkno, ++cblk) {
				++usedcblkcnt;
				if (!cblk->numpasses) {
					leaf = jpc_tagtree_getleaf(prc->incltagtree, usedcblkcnt - 1);
					if ((included = jpc_tagtree_decode(prc->incltagtree, leaf, lyrno + 1, inb)) < 0) {
						jas_logerrorf("cannot decode tagtree\n");
						jpc_bitstream_close(inb);
						return -1;
					}
				} else {
					if ((included = jpc_bitstream_getbit(inb)) < 0) {
						jas_logerrorf("cannot get bit\n");
						jpc_bitstream_close(inb);
						return -1;
					}
				}
				JAS_LOGDEBUGF(10, "\n");
				JAS_LOGDEBUGF(10, "included=%d ", included);
				if (!included) {
					continue;
				}
				if (!cblk->numpasses) {
					unsigned i = 1;
					leaf = jpc_tagtree_getleaf(prc->numimsbstagtree, usedcblkcnt - 1);
					for (;;) {
						if ((ret = jpc_tagtree_decode(prc->numimsbstagtree, leaf, i, inb)) < 0) {
							jas_logerrorf("cannot decode tagtree\n");
							jpc_bitstream_close(inb);
							return -1;
						}
						if (ret) {
							break;
						}
						++i;
					}
					cblk->numimsbs = i - 1;
					cblk->firstpassno = cblk->numimsbs * 3;
				}
				if ((numnewpasses = jpc_getnumnewpasses(inb)) < 0) {
					jas_logerrorf("cannot get number of new passes\n");
					jpc_bitstream_close(inb);
					return -1;
				}
				JAS_LOGDEBUGF(10, "numnewpasses=%d ", numnewpasses);
				seg = cblk->curseg;
				const unsigned savenumnewpasses = numnewpasses;
				unsigned mycounter = 0;
				if (numnewpasses > 0) {
					if (cblk->firstpassno > 10000) {
						/* workaround for
						   CVE-2016-9398: this
						   large value would
						   make
						   JPC_SEGPASSCNT()
						   return a negative
						   value, causing an
						   assertion failure
						   in
						   jpc_floorlog2() */
						jas_logerrorf("first pass number unreasonably large\n");
						jpc_bitstream_close(inb);
						return -1;
					}
					if ((m = jpc_getcommacode(inb)) < 0) {
						jas_logerrorf("cannot get comma code\n");
						jpc_bitstream_close(inb);
						return -1;
					}
					cblk->numlenbits += m;
					JAS_LOGDEBUGF(10, "increment=%d ", m);
					while (numnewpasses > 0) {
						const unsigned passno = cblk->firstpassno + cblk->numpasses + mycounter;
						if (passno >= 10000) {
							/* with this value,
							   JPC_SEGPASSCNT()
							   would return 0,
							   which is an illegal
							   value and would
							   later crash in
							   jpc_floorlog2() */
							jas_logerrorf("pass number unreasonably large\n");
							jpc_bitstream_close(inb);
							return -1;
						}
	/* XXX - the maxpasses is not set precisely but this doesn't matter... */
						const unsigned maxpasses = JPC_SEGPASSCNT(passno, cblk->firstpassno, 10000, (ccp->cblkctx & JPC_COX_LAZY) != 0, (ccp->cblkctx & JPC_COX_TERMALL) != 0);
						if (!discard && !seg) {
							if (!(seg = jpc_seg_alloc())) {
								jas_logerrorf("cannot allocate segment\n");
								jpc_bitstream_close(inb);
								return -1;
							}
							jpc_seglist_insert(&cblk->segs, cblk->segs.tail, seg);
							if (!cblk->curseg) {
								cblk->curseg = seg;
							}
							seg->passno = passno;
							seg->type = JPC_SEGTYPE(seg->passno, cblk->firstpassno, (ccp->cblkctx & JPC_COX_LAZY) != 0);
							seg->maxpasses = maxpasses;
						}
						const unsigned n = JAS_MIN((unsigned)numnewpasses, maxpasses);
						mycounter += n;
						numnewpasses -= n;
						if ((len = jpc_bitstream_getbits(inb, cblk->numlenbits + jpc_floorlog2(n))) < 0) {
							jpc_bitstream_close(inb);
							jas_logerrorf("cannot get bits\n");
							return -1;
						}
						JAS_LOGDEBUGF(10, "len=%d ", len);
						if (!discard) {
							seg->lyrno = lyrno;
							seg->numpasses += n;
							seg->cnt = len;
							seg = seg->next;
						}
						bodylen += len;
					}
				}
				cblk->numpasses += savenumnewpasses;
			}
		}

		jpc_bitstream_inalign(inb, 0, 0);

	} else {
		if (jpc_bitstream_inalign(inb, 0x7f, 0)) {
			jas_logerrorf("alignment failed\n");
			jpc_bitstream_close(inb);
			return -1;
		}
	}
	jpc_bitstream_close(inb);

	if (jas_get_debug_level() >= 5) {
		const uint_least64_t hdroffend = jas_stream_getrwcount(pkthdrstream);
		const unsigned long hdrlen = hdroffend - hdroffstart;
		jas_logdebugf(5, "hdrlen=%lu bodylen=%lu \n", (unsigned long) hdrlen,
		  (unsigned long) bodylen);
	}

	if (cp->csty & JPC_COD_EPH) {
		if (!(ms = jpc_getms(pkthdrstream, dec->cstate))) {
			jas_logerrorf("cannot get (EPH) marker segment\n");
			return -1;
		}
		if (jpc_ms_gettype(ms) != JPC_MS_EPH) {
			jpc_ms_destroy(ms);
			jas_logerrorf("missing EPH marker segment\n");
			return -1;
		}
		jpc_ms_destroy(ms);
	}

	/* decode the packet body. */

	if (jas_get_debug_level() >= 1) {
		jas_logdebugf(1, "packet body offset=%06ld\n", (long) jas_stream_getrwcount(in));
	}

	if (!discard) {
		tcomp = &tile->tcomps[compno];
		rlvl = &tcomp->rlvls[rlvlno];
		unsigned bandno;
		for (bandno = 0, band = rlvl->bands; bandno < (unsigned)rlvl->numbands;
		  ++bandno, ++band) {
			if (!band->data) {
				continue;
			}
			prc = &band->prcs[prcno];
			if (!prc->cblks) {
				continue;
			}
			unsigned cblkno;
			for (cblkno = 0, cblk = prc->cblks; cblkno < (unsigned)prc->numcblks;
			  ++cblkno, ++cblk) {
				seg = cblk->curseg;
				while (seg) {
					if (!seg->stream) {
						if (!(seg->stream = jas_stream_memopen(0, 0))) {
							jas_logerrorf("cannot open memory stream\n");
							return -1;
						}
					}
#if 0
					jas_eprintf("lyrno=%02d, compno=%02d, lvlno=%02d, prcno=%02d, bandno=%02d, cblkno=%02d, passno=%02d numpasses=%02d cnt=%d numbps=%d, numimsbs=%d\n",
					  lyrno, compno, rlvlno, prcno, band - rlvl->bands,
					  cblk - prc->cblks, seg->passno, seg->numpasses, seg->cnt,
					  band->numbps, cblk->numimsbs);
#endif
					if (seg->cnt > 0) {
						if (jpc_getdata(in, seg->stream, seg->cnt) < 0) {
							jas_logerrorf("jas_getdata failed\n");
							return -1;
						}
						seg->cnt = 0;
					}
					if (seg->numpasses >= seg->maxpasses) {
						cblk->curseg = seg->next;
					}
					seg = seg->next;
				}
			}
		}
	} else {
		if (jas_stream_gobble(in, bodylen) != JAS_CAST(int, bodylen)) {
			jas_logerrorf("jas_stream_gobble failed\n");
			return -1;
		}
	}
	return 0;
}

/********************************************************************************************/
/********************************************************************************************/

int jpc_dec_decodepkts(jpc_dec_t *dec, jas_stream_t *pkthdrstream, jas_stream_t *in)
{
	jpc_dec_tile_t *tile;
	jpc_pi_t *pi;
	int ret;

	tile = dec->curtile;
	pi = tile->pi;
	for (;;) {
		if (!tile->pkthdrstream ||
		  jas_stream_peekc(tile->pkthdrstream) == EOF) {
			switch (jpc_dec_lookahead(in)) {
			case JPC_MS_EOC:
			case JPC_MS_SOT:
				return 0;
			case JPC_MS_SOP:
			case JPC_MS_EPH:
			case 0:
				break;
			default:
				jas_logerrorf("jpc_dec_lookahead failed\n");
				return -1;
			}
		}
		// jpc_pi_dump(pi);
		if ((ret = jpc_pi_next(pi))) {
			if (ret < 0) {
				jas_logerrorf("jpc_pi_next failed\n");
			}
			//jas_logerrorf("jpc_pi_next failed (%d)\n", ret);
			return ret;
		}
		if (dec->maxpkts >= 0 && dec->numpkts >= (unsigned)dec->maxpkts) {
			jas_logwarnf("warning: stopping decode prematurely as requested\n");
			return 0;
		}
		if (jas_get_debug_level() >= 1) {
			jas_logdebugf(1, "packet offset=%08ld prg=%d cmptno=%02d "
			  "rlvlno=%02d prcno=%03d lyrno=%02d\n", (long)
			  jas_stream_getrwcount(in), jpc_pi_prg(pi), jpc_pi_cmptno(pi),
			  jpc_pi_rlvlno(pi), jpc_pi_prcno(pi), jpc_pi_lyrno(pi));
		}
		if (jpc_dec_decodepkt(dec, pkthdrstream, in, jpc_pi_cmptno(pi),
		  jpc_pi_rlvlno(pi), jpc_pi_prcno(pi), jpc_pi_lyrno(pi))) {
			jas_logerrorf("jpc_dec_decodepkt failed\n");
			return -1;
		}
		++dec->numpkts;
	}

	return 0;
}

jpc_pi_t *jpc_dec_pi_create(jpc_dec_t *dec, jpc_dec_tile_t *tile)
{
	jpc_pi_t *pi;
	unsigned compno;
	jpc_picomp_t *picomp;
	jpc_pirlvl_t *pirlvl;
	jpc_dec_tcomp_t *tcomp;
	unsigned rlvlno;
	jpc_dec_rlvl_t *rlvl;
	unsigned prcno;
	unsigned *prclyrno;
	jpc_dec_cmpt_t *cmpt;

	if (!(pi = jpc_pi_create0())) {
		return 0;
	}
	pi->numcomps = dec->numcomps;
	if (!(pi->picomps = jas_alloc2(pi->numcomps, sizeof(jpc_picomp_t)))) {
		jpc_pi_destroy(pi);
		return 0;
	}
	for (compno = 0, picomp = pi->picomps; compno < pi->numcomps; ++compno,
	  ++picomp) {
		picomp->pirlvls = 0;
	}

	for (compno = 0, tcomp = tile->tcomps, picomp = pi->picomps;
	  compno < pi->numcomps; ++compno, ++tcomp, ++picomp) {
		picomp->numrlvls = tcomp->numrlvls;
		if (!(picomp->pirlvls = jas_alloc2(picomp->numrlvls,
		  sizeof(jpc_pirlvl_t)))) {
			jpc_pi_destroy(pi);
			return 0;
		}
		for (rlvlno = 0, pirlvl = picomp->pirlvls; rlvlno <
		  picomp->numrlvls; ++rlvlno, ++pirlvl) {
			pirlvl->prclyrnos = 0;
		}
		for (rlvlno = 0, pirlvl = picomp->pirlvls, rlvl = tcomp->rlvls;
		  rlvlno < picomp->numrlvls; ++rlvlno, ++pirlvl, ++rlvl) {
/* XXX sizeof(long) should be sizeof different type */
			pirlvl->numprcs = rlvl->numprcs;
			if (!(pirlvl->prclyrnos = jas_alloc2(pirlvl->numprcs,
			  sizeof(long)))) {
				jpc_pi_destroy(pi);
				return 0;
			}
		}
	}

	pi->maxrlvls = 0;
	for (compno = 0, tcomp = tile->tcomps, picomp = pi->picomps, cmpt =
	  dec->cmpts; compno < pi->numcomps; ++compno, ++tcomp, ++picomp,
	  ++cmpt) {
		picomp->hsamp = cmpt->hstep;
		picomp->vsamp = cmpt->vstep;
		for (rlvlno = 0, pirlvl = picomp->pirlvls, rlvl = tcomp->rlvls;
		  rlvlno < picomp->numrlvls; ++rlvlno, ++pirlvl, ++rlvl) {
			pirlvl->prcwidthexpn = rlvl->prcwidthexpn;
			pirlvl->prcheightexpn = rlvl->prcheightexpn;
			for (prcno = 0, prclyrno = pirlvl->prclyrnos;
			  prcno < pirlvl->numprcs; ++prcno, ++prclyrno) {
				*prclyrno = 0;
			}
			pirlvl->numhprcs = rlvl->numhprcs;
		}
		if (pi->maxrlvls < tcomp->numrlvls) {
			pi->maxrlvls = tcomp->numrlvls;
		}
	}

	pi->numlyrs = tile->cp->numlyrs;
	pi->xstart = tile->xstart;
	pi->ystart = tile->ystart;
	pi->xend = tile->xend;
	pi->yend = tile->yend;

	pi->picomp = 0;
	pi->pirlvl = 0;
	pi->x = 0;
	pi->y = 0;
	pi->compno = 0;
	pi->rlvlno = 0;
	pi->prcno = 0;
	pi->lyrno = 0;
	pi->xstep = 0;
	pi->ystep = 0;

	pi->pchgno = -1;

	pi->defaultpchg.prgord = tile->cp->prgord;
	pi->defaultpchg.compnostart = 0;
	pi->defaultpchg.compnoend = pi->numcomps;
	pi->defaultpchg.rlvlnostart = 0;
	pi->defaultpchg.rlvlnoend = pi->maxrlvls;
	pi->defaultpchg.lyrnoend = pi->numlyrs;
	pi->pchg = 0;

	pi->valid = 0;

	return pi;
}

static long jpc_dec_lookahead(jas_stream_t *in)
{
	uint_fast16_t x;
	if (jpc_getuint16(in, &x)) {
		return -1;
	}
	if (jas_stream_ungetc(in, x & 0xff) == EOF ||
	  jas_stream_ungetc(in, x >> 8) == EOF) {
		return -1;
	}
	if (x >= JPC_MS_INMIN && x <= JPC_MS_INMAX) {
		return x;
	}
	return 0;
}
