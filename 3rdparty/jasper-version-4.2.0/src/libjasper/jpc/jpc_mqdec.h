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
 * MQ Arithmetic Decoder
 *
 * $Id$
 */

#ifndef JPC_MQDEC_H
#define JPC_MQDEC_H

/******************************************************************************\
* Includes.
\******************************************************************************/

#include "jasper/jas_types.h"
#include "jasper/jas_stream.h"

#include "jpc_mqcod.h"

#include <stdio.h>

/******************************************************************************\
* Types.
\******************************************************************************/

/* MQ arithmetic decoder. */

typedef struct {

	/* The C register. */
	uint_least32_t creg;

	/* The A register. */
	uint_least32_t areg;

	/* The CT register. */
	uint_least32_t ctreg;

	/* The current context. */
	const jpc_mqstate_t **curctx;

	/* The per-context information. */
	const jpc_mqstate_t **ctxs;

	/* The maximum number of contexts. */
	unsigned maxctxs;

	/* The stream from which to read data. */
	jas_stream_t *in;

	/* The last character read. */
	jas_uchar inbuffer;

	/* The EOF indicator. */
	bool eof;

} jpc_mqdec_t;

/******************************************************************************\
* Functions/macros for construction and destruction.
\******************************************************************************/

/* Create a MQ decoder. */
jpc_mqdec_t *jpc_mqdec_create(unsigned maxctxs, jas_stream_t *in);

/* Destroy a MQ decoder. */
void jpc_mqdec_destroy(jpc_mqdec_t *dec);

/******************************************************************************\
* Functions/macros for initialization.
\******************************************************************************/

/* Set the input stream associated with a MQ decoder. */
void jpc_mqdec_setinput(jpc_mqdec_t *dec, jas_stream_t *in);

/* Initialize a MQ decoder. */
void jpc_mqdec_init(jpc_mqdec_t *dec);

/******************************************************************************\
* Functions/macros for manipulating contexts.
\******************************************************************************/

/* Set the current context for a MQ decoder. */
static inline void jpc_mqdec_setcurctx(jpc_mqdec_t *dec, unsigned ctxno)
{
	dec->curctx = &dec->ctxs[ctxno];
}

/* Set the state information for all contexts of a MQ decoder. */
void jpc_mqdec_setctxs(const jpc_mqdec_t *dec, unsigned numctxs, const jpc_mqctx_t *ctxs);

/******************************************************************************\
* Functions/macros for decoding bits.
\******************************************************************************/

/* Decode a symbol. */
#ifdef NDEBUG
#define	jpc_mqdec_getbit(dec) \
	jpc_mqdec_getbit_macro(dec)
#else
#define	jpc_mqdec_getbit(dec) \
	jpc_mqdec_getbit_func(dec)
#endif

/* Decode a symbol (assuming an unskewed probability distribution). */
#ifdef NDEBUG
#define	jpc_mqdec_getbitnoskew(dec) \
	jpc_mqdec_getbit_macro(dec)
#else
#define	jpc_mqdec_getbitnoskew(dec) \
	jpc_mqdec_getbit_func(dec)
#endif

/******************************************************************************\
* Functions/macros for debugging.
\******************************************************************************/

/* Dump the MQ decoder state for debugging. */
void jpc_mqdec_dump(const jpc_mqdec_t *dec);

/******************************************************************************\
* EVERYTHING BELOW THIS POINT IS IMPLEMENTATION SPECIFIC AND NOT PART OF THE
* APPLICATION INTERFACE.  DO NOT RELY ON ANY OF THE INTERNAL FUNCTIONS/MACROS
* GIVEN BELOW.
\******************************************************************************/

bool jpc_mqdec_mpsexchrenormd(jpc_mqdec_t *dec);
bool jpc_mqdec_lpsexchrenormd(jpc_mqdec_t *dec);

JAS_FORCE_INLINE
static bool jpc_mqdec_getbit_macro(jpc_mqdec_t *dec)
{
	const jpc_mqstate_t *const state = *dec->curctx;

	dec->areg -= state->qeval;

	if (dec->creg >= (uint_least32_t)state->qeval << 16) {
		dec->creg -= (uint_least32_t)state->qeval << 16;
		return dec->areg & 0x8000
			? state->mps
			: jpc_mqdec_mpsexchrenormd(dec);
	} else {
		return jpc_mqdec_lpsexchrenormd(dec);
	}
}

JAS_FORCE_INLINE
static bool jpc_mqdec_mpsexchange(uint_least32_t areg, uint_least32_t delta, const jpc_mqstate_t **curctx)
{
	if (areg < delta) {
		const jpc_mqstate_t *state = *curctx;
		/* LPS decoded. */
		*curctx = state->nlps;
		return !state->mps;
	} else {
		const jpc_mqstate_t *state = *curctx;
		/* MPS decoded. */
		*curctx = state->nmps;
		return state->mps;
	}
}

JAS_FORCE_INLINE
static bool jpc_mqdec_lpsexchange(uint_least32_t *areg_p, uint_least32_t delta, const jpc_mqstate_t **curctx)
{
	if (*areg_p >= delta) {
		const jpc_mqstate_t *state = *curctx;
		*areg_p = delta;
		*curctx = state->nlps;
		return !state->mps;
	} else {
		const jpc_mqstate_t *state = *curctx;
		*areg_p = delta;
		*curctx = state->nmps;
		return state->mps;
	}
}

bool jpc_mqdec_getbit_func(jpc_mqdec_t *dec);

#endif
