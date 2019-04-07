/*	MikMod sound library
	(c) 1998, 1999, 2000, 2001, 2002 Miodrag Vallat and others - see file
	AUTHORS for complete list.

	This library is free software; you can redistribute it and/or modify
	it under the terms of the GNU Library General Public License as
	published by the Free Software Foundation; either version 2 of
	the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
	02111-1307, USA.
*/

/*==============================================================================

  $Id$

  Sample mixing routines, using a 32 bits mixing buffer.

==============================================================================*/

/*

  Optional features include:
    (a) 4-step reverb (for 16 bit output only)
    (b) Interpolation of sample data during mixing
    (c) Dolby Surround Sound
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#include <string.h>

#include "mikmod_internals.h"

/*
   Constant definitions
   ====================

	BITSHIFT
		Controls the maximum volume of the sound output.  All data is shifted
		right by BITSHIFT after being mixed. Higher values result in quieter
		sound and less chance of distortion.

	REVERBERATION
		Controls the duration of the reverb. Larger values represent a shorter
		reverb loop. Smaller values extend the reverb but can result in more of
		an echo-ish sound.

*/

#define BITSHIFT		9
#define REVERBERATION	110000L

#define FRACBITS 11
#define FRACMASK ((1L<<FRACBITS)-1L)

#define TICKLSIZE 8192
#define TICKWSIZE (TICKLSIZE<<1)
#define TICKBSIZE (TICKWSIZE<<1)

#define CLICK_SHIFT  6
#define CLICK_BUFFER (1L<<CLICK_SHIFT)

#ifndef MIN
#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#endif

typedef struct VINFO {
	UBYTE     kick;              /* =1 -> sample has to be restarted */
	UBYTE     active;            /* =1 -> sample is playing */
	UWORD     flags;             /* 16/8 bits looping/one-shot */
	SWORD     handle;            /* identifies the sample */
	ULONG     start;             /* start index */
	ULONG     size;              /* samplesize */
	ULONG     reppos;            /* loop start */
	ULONG     repend;            /* loop end */
	ULONG     frq;               /* current frequency */
	int       vol;               /* current volume */
	int       pan;               /* current panning position */

	int       rampvol;
	int       lvolsel,rvolsel;   /* Volume factor in range 0-255 */
	int       oldlvol,oldrvol;

	SLONGLONG current;           /* current index in the sample */
	SLONGLONG increment;         /* increment value */
} VINFO;

static	SWORD **Samples;
static	VINFO *vinf=NULL,*vnf;
static	long tickleft,samplesthatfit,vc_memory=0;
static	int vc_softchn;
static	SLONGLONG idxsize,idxlpos,idxlend;
static	SLONG *vc_tickbuf=NULL;
static	UWORD vc_mode;

/* Reverb control variables */

static	int RVc1, RVc2, RVc3, RVc4, RVc5, RVc6, RVc7, RVc8;
static	ULONG RVRindex;

/* For Mono or Left Channel */
static	SLONG *RVbufL1=NULL,*RVbufL2=NULL,*RVbufL3=NULL,*RVbufL4=NULL,
		      *RVbufL5=NULL,*RVbufL6=NULL,*RVbufL7=NULL,*RVbufL8=NULL;

/* For Stereo only (Right Channel) */
static	SLONG *RVbufR1=NULL,*RVbufR2=NULL,*RVbufR3=NULL,*RVbufR4=NULL,
		      *RVbufR5=NULL,*RVbufR6=NULL,*RVbufR7=NULL,*RVbufR8=NULL;

#ifdef NATIVE_64BIT_INT
#define NATIVE SLONGLONG
#else
#define NATIVE SLONG
#endif

#if defined HAVE_SSE2 || defined HAVE_ALTIVEC

# if !defined(NATIVE_64BIT_INT)
static SINTPTR_T MixSIMDMonoNormal(const SWORD* srce,SLONG* dest,SINTPTR_T idx,SINTPTR_T increment,SINTPTR_T todo)
{
	/* TODO: */
	SWORD sample;
	SLONG lvolsel = vnf->lvolsel;

	while(todo--) {
		sample = srce[idx >> FRACBITS];
		idx += increment;

		*dest++ += lvolsel * sample;
	}
	return idx;
}
# endif /* !NATIVE_64BIT_INT */

static SINTPTR_T MixSIMDStereoNormal(const SWORD* srce,SLONG* dest,SINTPTR_T idx,SINTPTR_T increment,SINTPTR_T todo)
{
	SWORD vol[8] = {vnf->lvolsel, vnf->rvolsel};
	SWORD sample;
	SLONG remain = todo;

	/* Dest can be misaligned */
	while(!IS_ALIGNED_16(dest)) {
		sample=srce[idx >> FRACBITS];
		idx += increment;
		*dest++ += vol[0] * sample;
		*dest++ += vol[1] * sample;
		todo--;
		if(!todo) return idx;
	}

	/* Srce is always aligned */

#if defined HAVE_SSE2
	remain = todo&3;
	{
		__m128i v0 = _mm_set_epi16(0, vol[1],
					   0, vol[0],
					   0, vol[1],
					   0, vol[0]);
		for(todo>>=2;todo; todo--)
		{
			SWORD s0 = srce[idx >> FRACBITS];
			SWORD s1 = srce[(idx += increment) >> FRACBITS];
			SWORD s2 = srce[(idx += increment) >> FRACBITS];
			SWORD s3 = srce[(idx += increment) >> FRACBITS];
			__m128i v1 = _mm_set_epi16(0, s1, 0, s1, 0, s0, 0, s0);
			__m128i v2 = _mm_set_epi16(0, s3, 0, s3, 0, s2, 0, s2);
			__m128i v3 = _mm_load_si128((__m128i*)(dest+0));
			__m128i v4 = _mm_load_si128((__m128i*)(dest+4));
			_mm_store_si128((__m128i*)(dest+0), _mm_add_epi32(v3, _mm_madd_epi16(v0, v1)));
			_mm_store_si128((__m128i*)(dest+4), _mm_add_epi32(v4, _mm_madd_epi16(v0, v2)));
			dest+=8;
			idx += increment;
		}
	}

#elif defined HAVE_ALTIVEC
	remain = todo&3;
	{
		SWORD s[8];
		vector signed short r0 = vec_ld(0, vol);
		vector signed short v0 = vec_perm(r0, r0, (vector unsigned char)(0, 1, /* l */
										 0, 1, /* l */
										 2, 3, /* r */
										 2, 1, /* r */
										 0, 1, /* l */
										 0, 1, /* l */
										 2, 3, /* r */
										 2, 3  /* r */
										 ));

		for(todo>>=2;todo; todo--)
		{
			vector short int r1;
			vector signed short v1, v2;
			vector signed int v3, v4, v5, v6;

			/* Load constants */
			s[0] = srce[idx >> FRACBITS];
			s[1] = srce[(idx += increment) >> FRACBITS];
			s[2] = srce[(idx += increment) >> FRACBITS];
			s[3] = srce[(idx += increment) >> FRACBITS];
			s[4] = 0;

			r1 = vec_ld(0, s);
			v1 = vec_perm(r1, r1, (vector unsigned char)
								(0*2, 0*2+1, /* s0 */
								 4*2, 4*2+1, /* 0  */
								 0*2, 0*2+1, /* s0 */
								 4*2, 4*2+1, /* 0  */
								 1*2, 1*2+1, /* s1 */
								 4*2, 4*2+1, /* 0  */
								 1*2, 1*2+1, /* s1 */
								 4*2, 4*2+1  /* 0  */
								) );
			v2 = vec_perm(r1, r1, (vector unsigned char)
								(2*2, 2*2+1, /* s2 */
								 4*2, 4*2+1, /* 0  */
								 2*2, 2*2+1, /* s2 */
								 4*2, 4*2+1, /* 0  */
								 3*2, 3*2+1, /* s3 */
								 4*2, 4*2+1, /* 0  */
								 3*2, 3*2+1, /* s3 */
								 4*2, 4*2+1  /* 0  */
								) );

			v3 = vec_ld(0, dest);
			v4 = vec_ld(0, dest + 4);
			v5 = vec_mule(v0, v1);
			v6 = vec_mule(v0, v2);

			vec_st(vec_add(v3, v5), 0, dest);
			vec_st(vec_add(v4, v6), 0x10, dest);

			dest+=8;
			idx += increment;
		}
	}
#endif /* HAVE_ALTIVEC */

	/* Remaining bits */
	while(remain--) {
		sample=srce[idx >> FRACBITS];
		idx += increment;

		*dest++ += vol[0] * sample;
		*dest++ += vol[1] * sample;
	}
	return idx;
}
#endif

/*========== 32 bit sample mixers - only for 32 bit platforms */
#ifndef NATIVE_64BIT_INT

static SLONG Mix32MonoNormal(const SWORD* srce,SLONG* dest,SLONG idx,SLONG increment,SLONG todo)
{
#if defined HAVE_ALTIVEC || defined HAVE_SSE2
	if (md_mode & DMODE_SIMDMIXER) {
		return MixSIMDMonoNormal(srce, dest, idx, increment, todo);
	}
	else
#endif
	{
		SWORD sample;
		SLONG lvolsel = vnf->lvolsel;

		while(todo--) {
			sample = srce[idx >> FRACBITS];
			idx += increment;

			*dest++ += lvolsel * sample;
		}
	}
	return idx;
}

/* FIXME: This mixer should works also on 64-bit platform */
/* Hint : changes SLONG / SLONGLONG mess with intptr */
static SLONG Mix32StereoNormal(const SWORD* srce,SLONG* dest,SLONG idx,SLONG increment,SLONG todo)
{
#if defined HAVE_ALTIVEC || defined HAVE_SSE2
	if (md_mode & DMODE_SIMDMIXER) {
		return MixSIMDStereoNormal(srce, dest, idx, increment, todo);
	}
	else
#endif
	{
		SWORD sample;
		SLONG lvolsel = vnf->lvolsel;
		SLONG rvolsel = vnf->rvolsel;

		while(todo--) {
			sample=srce[idx >> FRACBITS];
			idx += increment;

			*dest++ += lvolsel * sample;
			*dest++ += rvolsel * sample;
		}
	}
	return idx;
}

static SLONG Mix32SurroundNormal(const SWORD* srce,SLONG* dest,SLONG idx,SLONG increment,SLONG todo)
{
	SWORD sample;
	SLONG lvolsel = vnf->lvolsel;
	SLONG rvolsel = vnf->rvolsel;

	if (lvolsel>=rvolsel) {
		while(todo--) {
			sample = srce[idx >> FRACBITS];
			idx += increment;

			*dest++ += lvolsel*sample;
			*dest++ -= lvolsel*sample;
		}
	}
	else {
		while(todo--) {
			sample = srce[idx >> FRACBITS];
			idx += increment;

			*dest++ -= rvolsel*sample;
			*dest++ += rvolsel*sample;
		}
	}
	return idx;
}

static SLONG Mix32MonoInterp(const SWORD* srce,SLONG* dest,SLONG idx,SLONG increment,SLONG todo)
{
	SLONG sample;
	SLONG lvolsel = vnf->lvolsel;
	SLONG rampvol = vnf->rampvol;

	if (rampvol) {
		SLONG oldlvol = vnf->oldlvol - lvolsel;
		while(todo--) {
			sample=(SLONG)srce[idx>>FRACBITS]+
				((SLONG)(srce[(idx>>FRACBITS)+1]-srce[idx>>FRACBITS])
				 *(idx&FRACMASK)>>FRACBITS);
			idx += increment;

			*dest++ += ((lvolsel << CLICK_SHIFT) + oldlvol * rampvol)
					   * sample >> CLICK_SHIFT;
			if (!--rampvol)
				break;
		}
		vnf->rampvol = rampvol;
		if (todo < 0)
			return idx;
	}

	while(todo--) {
		sample=(SLONG)srce[idx>>FRACBITS]+
			((SLONG)(srce[(idx>>FRACBITS)+1]-srce[idx>>FRACBITS])
			 *(idx&FRACMASK)>>FRACBITS);
		idx += increment;

		*dest++ += lvolsel * sample;
	}
	return idx;
}

static SLONG Mix32StereoInterp(const SWORD* srce,SLONG* dest,SLONG idx,SLONG increment,SLONG todo)
{
	SLONG sample;
	SLONG lvolsel = vnf->lvolsel;
	SLONG rvolsel = vnf->rvolsel;
	SLONG rampvol = vnf->rampvol;

	if (rampvol) {
		SLONG oldlvol = vnf->oldlvol - lvolsel;
		SLONG oldrvol = vnf->oldrvol - rvolsel;
		while(todo--) {
			sample=(SLONG)srce[idx>>FRACBITS]+
				((SLONG)(srce[(idx>>FRACBITS)+1]-srce[idx>>FRACBITS])
				 *(idx&FRACMASK)>>FRACBITS);
			idx += increment;

			*dest++ += ((lvolsel << CLICK_SHIFT) + oldlvol * rampvol)
					   * sample >> CLICK_SHIFT;
			*dest++ += ((rvolsel << CLICK_SHIFT) + oldrvol * rampvol)
					   * sample >> CLICK_SHIFT;
			if (!--rampvol)
				break;
		}
		vnf->rampvol = rampvol;
		if (todo < 0)
			return idx;
	}

	while(todo--) {
		sample=(SLONG)srce[idx>>FRACBITS]+
			((SLONG)(srce[(idx>>FRACBITS)+1]-srce[idx>>FRACBITS])
			 *(idx&FRACMASK)>>FRACBITS);
		idx += increment;

		*dest++ += lvolsel * sample;
		*dest++ += rvolsel * sample;
	}
	return idx;
}

static SLONG Mix32SurroundInterp(const SWORD* srce,SLONG* dest,SLONG idx,SLONG increment,SLONG todo)
{
	SLONG sample;
	SLONG lvolsel = vnf->lvolsel;
	SLONG rvolsel = vnf->rvolsel;
	SLONG rampvol = vnf->rampvol;
	SLONG oldvol, vol;

	if (lvolsel >= rvolsel) {
		vol = lvolsel;
		oldvol = vnf->oldlvol;
	} else {
		vol = rvolsel;
		oldvol = vnf->oldrvol;
	}

	if (rampvol) {
		oldvol -= vol;
		while(todo--) {
			sample=(SLONG)srce[idx>>FRACBITS]+
				((SLONG)(srce[(idx>>FRACBITS)+1]-srce[idx>>FRACBITS])
				 *(idx&FRACMASK)>>FRACBITS);
			idx += increment;

			sample=((vol << CLICK_SHIFT) + oldvol * rampvol)
					   * sample >> CLICK_SHIFT;
			*dest++ += sample;
			*dest++ -= sample;

			if (!--rampvol)
				break;
		}
		vnf->rampvol = rampvol;
		if (todo < 0)
			return idx;
	}

	while(todo--) {
		sample=(SLONG)srce[idx>>FRACBITS]+
			((SLONG)(srce[(idx>>FRACBITS)+1]-srce[idx>>FRACBITS])
			 *(idx&FRACMASK)>>FRACBITS);
		idx += increment;

		*dest++ += vol*sample;
		*dest++ -= vol*sample;
	}
	return idx;
}
#endif

/*========== 64 bit sample mixers - all platforms */

static SLONGLONG MixMonoNormal(const SWORD* srce,SLONG* dest,SLONGLONG idx,SLONGLONG increment,SLONG todo)
{
	SWORD sample;
	SLONG lvolsel = vnf->lvolsel;

	while(todo--) {
		sample = srce[idx >> FRACBITS];
		idx += increment;

		*dest++ += lvolsel * sample;
	}
	return idx;
}

static SLONGLONG MixStereoNormal(const SWORD* srce,SLONG* dest,SLONGLONG idx,SLONGLONG increment,SLONG todo)
{
	SWORD sample;
	SLONG lvolsel = vnf->lvolsel;
	SLONG rvolsel = vnf->rvolsel;

	while(todo--) {
		sample=srce[idx >> FRACBITS];
		idx += increment;

		*dest++ += lvolsel * sample;
		*dest++ += rvolsel * sample;
	}
	return idx;
}

static SLONGLONG MixSurroundNormal(const SWORD* srce,SLONG* dest,SLONGLONG idx,SLONGLONG increment,SLONG todo)
{
	SWORD sample;
	SLONG lvolsel = vnf->lvolsel;
	SLONG rvolsel = vnf->rvolsel;

	if(vnf->lvolsel>=vnf->rvolsel) {
		while(todo--) {
			sample = srce[idx >> FRACBITS];
			idx += increment;

			*dest++ += lvolsel*sample;
			*dest++ -= lvolsel*sample;
		}
	}
	else {
		while(todo--) {
			sample = srce[idx >> FRACBITS];
			idx += increment;

			*dest++ -= rvolsel*sample;
			*dest++ += rvolsel*sample;
		}
	}
	return idx;
}

static SLONGLONG MixMonoInterp(const SWORD* srce,SLONG* dest,SLONGLONG idx,SLONGLONG increment,SLONG todo)
{
	SLONG sample;
	SLONG lvolsel = vnf->lvolsel;
	SLONG rampvol = vnf->rampvol;

	if (rampvol) {
		SLONG oldlvol = vnf->oldlvol - lvolsel;
		while(todo--) {
			sample=(SLONG)srce[idx>>FRACBITS]+
				((SLONG)(srce[(idx>>FRACBITS)+1]-srce[idx>>FRACBITS])
				 *(idx&FRACMASK)>>FRACBITS);
			idx += increment;

			*dest++ += ((lvolsel << CLICK_SHIFT) + oldlvol * rampvol)
					   * sample >> CLICK_SHIFT;
			if (!--rampvol)
				break;
		}
		vnf->rampvol = rampvol;
		if (todo < 0)
			return idx;
	}

	while(todo--) {
		sample=(SLONG)srce[idx>>FRACBITS]+
			((SLONG)(srce[(idx>>FRACBITS)+1]-srce[idx>>FRACBITS])
			 *(idx&FRACMASK)>>FRACBITS);
		idx += increment;

		*dest++ += lvolsel * sample;
	}
	return idx;
}

static SLONGLONG MixStereoInterp(const SWORD* srce,SLONG* dest,SLONGLONG idx,SLONGLONG increment,SLONG todo)
{
	SLONG sample;
	SLONG lvolsel = vnf->lvolsel;
	SLONG rvolsel = vnf->rvolsel;
	SLONG rampvol = vnf->rampvol;

	if (rampvol) {
		SLONG oldlvol = vnf->oldlvol - lvolsel;
		SLONG oldrvol = vnf->oldrvol - rvolsel;
		while(todo--) {
			sample=(SLONG)srce[idx>>FRACBITS]+
				((SLONG)(srce[(idx>>FRACBITS)+1]-srce[idx>>FRACBITS])
				 *(idx&FRACMASK)>>FRACBITS);
			idx += increment;

			*dest++ +=((lvolsel << CLICK_SHIFT) + oldlvol * rampvol)
					   * sample >> CLICK_SHIFT;
			*dest++ +=((rvolsel << CLICK_SHIFT) + oldrvol * rampvol)
					   * sample >> CLICK_SHIFT;
			if (!--rampvol)
				break;
		}
		vnf->rampvol = rampvol;
		if (todo < 0)
			return idx;
	}

	while(todo--) {
		sample=(SLONG)srce[idx>>FRACBITS]+
			((SLONG)(srce[(idx>>FRACBITS)+1]-srce[idx>>FRACBITS])
			 *(idx&FRACMASK)>>FRACBITS);
		idx += increment;

		*dest++ += lvolsel * sample;
		*dest++ += rvolsel * sample;
	}
	return idx;
}

static SLONGLONG MixSurroundInterp(const SWORD* srce,SLONG* dest,SLONGLONG idx,SLONGLONG increment,SLONG todo)
{
	SLONG sample;
	SLONG lvolsel = vnf->lvolsel;
	SLONG rvolsel = vnf->rvolsel;
	SLONG rampvol = vnf->rampvol;
	SLONG oldvol, vol;

	if (lvolsel >= rvolsel) {
		vol = lvolsel;
		oldvol = vnf->oldlvol;
	} else {
		vol = rvolsel;
		oldvol = vnf->oldrvol;
	}

	if (rampvol) {
		oldvol -= vol;
		while(todo--) {
			sample=(SLONG)srce[idx>>FRACBITS]+
				((SLONG)(srce[(idx>>FRACBITS)+1]-srce[idx>>FRACBITS])
				 *(idx&FRACMASK)>>FRACBITS);
			idx += increment;

			sample=((vol << CLICK_SHIFT) + oldvol * rampvol)
					   * sample >> CLICK_SHIFT;
			*dest++ += sample;
			*dest++ -= sample;
			if (!--rampvol)
				break;
		}
		vnf->rampvol = rampvol;
		if (todo < 0)
			return idx;
	}

	while(todo--) {
		sample=(SLONG)srce[idx>>FRACBITS]+
			((SLONG)(srce[(idx>>FRACBITS)+1]-srce[idx>>FRACBITS])
			 *(idx&FRACMASK)>>FRACBITS);
		idx += increment;

		*dest++ += vol*sample;
		*dest++ -= vol*sample;
	}
	return idx;
}

static void (*MixReverb)(SLONG* srce,NATIVE count);

/* Reverb macros */
#define COMPUTE_LOC(n) loc##n = RVRindex % RVc##n
#define COMPUTE_LECHO(n) RVbufL##n [loc##n ]=speedup+((ReverbPct*RVbufL##n [loc##n ])>>7)
#define COMPUTE_RECHO(n) RVbufR##n [loc##n ]=speedup+((ReverbPct*RVbufR##n [loc##n ])>>7)

static void MixReverb_Normal(SLONG* srce,NATIVE count)
{
	unsigned int speedup;
	int ReverbPct;
	unsigned int loc1,loc2,loc3,loc4;
	unsigned int loc5,loc6,loc7,loc8;

	ReverbPct=58+(md_reverb<<2);

	COMPUTE_LOC(1); COMPUTE_LOC(2); COMPUTE_LOC(3); COMPUTE_LOC(4);
	COMPUTE_LOC(5); COMPUTE_LOC(6); COMPUTE_LOC(7); COMPUTE_LOC(8);

	while(count--) {
		/* Compute the left channel echo buffers */
		speedup = *srce >> 3;

		COMPUTE_LECHO(1); COMPUTE_LECHO(2); COMPUTE_LECHO(3); COMPUTE_LECHO(4);
		COMPUTE_LECHO(5); COMPUTE_LECHO(6); COMPUTE_LECHO(7); COMPUTE_LECHO(8);

		/* Prepare to compute actual finalized data */
		RVRindex++;

		COMPUTE_LOC(1); COMPUTE_LOC(2); COMPUTE_LOC(3); COMPUTE_LOC(4);
		COMPUTE_LOC(5); COMPUTE_LOC(6); COMPUTE_LOC(7); COMPUTE_LOC(8);

		/* left channel */
		*srce++ +=RVbufL1[loc1]-RVbufL2[loc2]+RVbufL3[loc3]-RVbufL4[loc4]+
			  RVbufL5[loc5]-RVbufL6[loc6]+RVbufL7[loc7]-RVbufL8[loc8];
	}
}

static void MixReverb_Stereo(SLONG* srce,NATIVE count)
{
	unsigned int speedup;
	int          ReverbPct;
	unsigned int loc1, loc2, loc3, loc4;
	unsigned int loc5, loc6, loc7, loc8;

	ReverbPct = 92+(md_reverb<<1);

	COMPUTE_LOC(1); COMPUTE_LOC(2); COMPUTE_LOC(3); COMPUTE_LOC(4);
	COMPUTE_LOC(5); COMPUTE_LOC(6); COMPUTE_LOC(7); COMPUTE_LOC(8);

	while(count--) {
		/* Compute the left channel echo buffers */
		speedup = *srce >> 3;

		COMPUTE_LECHO(1); COMPUTE_LECHO(2); COMPUTE_LECHO(3); COMPUTE_LECHO(4);
		COMPUTE_LECHO(5); COMPUTE_LECHO(6); COMPUTE_LECHO(7); COMPUTE_LECHO(8);

		/* Compute the right channel echo buffers */
		speedup = srce[1] >> 3;

		COMPUTE_RECHO(1); COMPUTE_RECHO(2); COMPUTE_RECHO(3); COMPUTE_RECHO(4);
		COMPUTE_RECHO(5); COMPUTE_RECHO(6); COMPUTE_RECHO(7); COMPUTE_RECHO(8);

		/* Prepare to compute actual finalized data */
		RVRindex++;

		COMPUTE_LOC(1); COMPUTE_LOC(2); COMPUTE_LOC(3); COMPUTE_LOC(4);
		COMPUTE_LOC(5); COMPUTE_LOC(6); COMPUTE_LOC(7); COMPUTE_LOC(8);

		/* left channel then right channel */
		*srce++ +=RVbufL1[loc1]-RVbufL2[loc2]+RVbufL3[loc3]-RVbufL4[loc4]+
			  RVbufL5[loc5]-RVbufL6[loc6]+RVbufL7[loc7]-RVbufL8[loc8];

		*srce++ +=RVbufR1[loc1]-RVbufR2[loc2]+RVbufR3[loc3]-RVbufR4[loc4]+
			  RVbufR5[loc5]-RVbufR6[loc6]+RVbufR7[loc7]-RVbufR8[loc8];
	}
}

static void (*MixLowPass)(SLONG* srce,NATIVE count);

static int nLeftNR, nRightNR;

static void MixLowPass_Stereo(SLONG* srce,NATIVE count)
{
	int n1 = nLeftNR, n2 = nRightNR;
	SLONG *pnr = srce;
	int nr=count;
	for (; nr; nr--)
	{
		int vnr = pnr[0] >> 1;
		pnr[0] = vnr + n1;
		n1 = vnr;
		vnr = pnr[1] >> 1;
		pnr[1] = vnr + n2;
		n2 = vnr;
		pnr += 2;
	}
	nLeftNR = n1;
	nRightNR = n2;
}

static void MixLowPass_Normal(SLONG* srce,NATIVE count)
{
	int n1 = nLeftNR;
	SLONG *pnr = srce;
	int nr=count;
	for (; nr; nr--)
	{
		int vnr = pnr[0] >> 1;
		pnr[0] = vnr + n1;
		n1 = vnr;
		pnr ++;
	}
	nLeftNR = n1;
}

/* shifting fudge factor for FP scaling, should be 0 < FP_SHIFT < BITSHIFT */
#define FP_SHIFT 4

/* Mixing macros */
#define EXTRACT_SAMPLE_FP(var,size) var=(*srce++>>(BITSHIFT-size)) * ((1.0f / 32768.0f) / (1 << size))
#define CHECK_SAMPLE_FP(var,bound) var=(var>bound)?bound:(var<-bound)?-bound:var
#define PUT_SAMPLE_FP(var) *dste++=var

static void Mix32ToFP(float* dste,const SLONG *srce,NATIVE count)
{
	float x1,x2,x3,x4;
	int	remain;

	remain=count&3;
	for(count>>=2;count;count--) {
		EXTRACT_SAMPLE_FP(x1,FP_SHIFT); EXTRACT_SAMPLE_FP(x2,FP_SHIFT);
		EXTRACT_SAMPLE_FP(x3,FP_SHIFT); EXTRACT_SAMPLE_FP(x4,FP_SHIFT);

		CHECK_SAMPLE_FP(x1,1.0f); CHECK_SAMPLE_FP(x2,1.0f);
		CHECK_SAMPLE_FP(x3,1.0f); CHECK_SAMPLE_FP(x4,1.0f);

		PUT_SAMPLE_FP(x1); PUT_SAMPLE_FP(x2);
		PUT_SAMPLE_FP(x3); PUT_SAMPLE_FP(x4);
	}
	while(remain--) {
		EXTRACT_SAMPLE_FP(x1,FP_SHIFT);
		CHECK_SAMPLE_FP(x1,1.0f);
		PUT_SAMPLE_FP(x1);
	}
}


/* Mixing macros */
#define EXTRACT_SAMPLE(var,size) var=*srce++>>(BITSHIFT+16-size)
#define CHECK_SAMPLE(var,bound) var=(var>=bound)?bound-1:(var<-bound)?-bound:var
#define PUT_SAMPLE(var) *dste++=var

static void Mix32To16(SWORD* dste,const SLONG *srce,NATIVE count)
{
	SLONG x1,x2,x3,x4;
	int	remain;

	remain=count&3;
	for(count>>=2;count;count--) {
		EXTRACT_SAMPLE(x1,16); EXTRACT_SAMPLE(x2,16);
		EXTRACT_SAMPLE(x3,16); EXTRACT_SAMPLE(x4,16);

		CHECK_SAMPLE(x1,32768); CHECK_SAMPLE(x2,32768);
		CHECK_SAMPLE(x3,32768); CHECK_SAMPLE(x4,32768);

		PUT_SAMPLE(x1); PUT_SAMPLE(x2); PUT_SAMPLE(x3); PUT_SAMPLE(x4);
	}
	while(remain--) {
		EXTRACT_SAMPLE(x1,16);
		CHECK_SAMPLE(x1,32768);
		PUT_SAMPLE(x1);
	}
}

static void Mix32To8(SBYTE* dste,const SLONG *srce,NATIVE count)
{
	SWORD x1,x2,x3,x4;
	int	remain;

	remain=count&3;
	for(count>>=2;count;count--) {
		EXTRACT_SAMPLE(x1,8); EXTRACT_SAMPLE(x2,8);
		EXTRACT_SAMPLE(x3,8); EXTRACT_SAMPLE(x4,8);

		CHECK_SAMPLE(x1,128); CHECK_SAMPLE(x2,128);
		CHECK_SAMPLE(x3,128); CHECK_SAMPLE(x4,128);

		PUT_SAMPLE(x1+128); PUT_SAMPLE(x2+128);
		PUT_SAMPLE(x3+128); PUT_SAMPLE(x4+128);
	}
	while(remain--) {
		EXTRACT_SAMPLE(x1,8);
		CHECK_SAMPLE(x1,128);
		PUT_SAMPLE(x1+128);
	}
}

#if defined HAVE_ALTIVEC || defined HAVE_SSE2

/* Mix 32bit input to floating point. 32 samples per iteration */
/* PC: ?, Mac OK */
static void Mix32ToFP_SIMD(float* dste,const SLONG* srce,NATIVE count)
{
	const float k = ((1.0f / 32768.0f) / (1 << FP_SHIFT));
	int	remain=count;
	simd_m128 x1, x2, xk;

	while(!IS_ALIGNED_16(dste) || !IS_ALIGNED_16(srce))
	{
		float xf;
		EXTRACT_SAMPLE_FP(xf,FP_SHIFT);
		CHECK_SAMPLE_FP(xf,1.0f);
		PUT_SAMPLE_FP(xf);
		count--;
		if (!count) return;
	}

	remain = count&7;

	xk = LOAD_PS1_SIMD(&k); /* Scale factor */

	for(count>>=3;count;count--) {
		EXTRACT_SAMPLE_SIMD_F(srce, x1, FP_SHIFT, xk);  /* Load 4 samples */
		EXTRACT_SAMPLE_SIMD_F(srce+4, x2, FP_SHIFT, xk);  /* Load 4 samples */
		PUT_SAMPLE_SIMD_F(dste, x1); /* Store 4 samples */
		PUT_SAMPLE_SIMD_F(dste+4, x2); /* Store 4 samples */
		srce+=8;
		dste+=8;
	}

	if (remain&4) {
		EXTRACT_SAMPLE_SIMD_F(srce, x1, FP_SHIFT, xk);  /* Load 4 samples */
		PUT_SAMPLE_SIMD_F(dste, x1); /* Store 4 samples */
		srce+=4;
		dste+=4;
		remain &= 3;
	}

	while(remain--) {
		float xf;
		EXTRACT_SAMPLE_FP(xf,FP_SHIFT);
		CHECK_SAMPLE_FP(xf,1.0f);
		PUT_SAMPLE_FP(xf);
	}
}

/* PC: Ok, Mac Ok */
static void Mix32To16_SIMD(SWORD* dste,const SLONG* srce,NATIVE count)
{
	int	remain = count;

	while(!IS_ALIGNED_16(dste) || !IS_ALIGNED_16(srce))
	{
		SLONG x1;
		EXTRACT_SAMPLE(x1,16);
		CHECK_SAMPLE(x1,32768);
		PUT_SAMPLE(x1);
		count--;
		if (!count) return;
	}

	remain = count&7;

	for(count>>=3;count;count--)
	{
		simd_m128i x1,x2;
		EXTRACT_SAMPLE_SIMD_16(srce, x1);  /* Load 4 samples */
		EXTRACT_SAMPLE_SIMD_16(srce+4, x2);  /* Load 4 samples */
		PUT_SAMPLE_SIMD_W(dste, x1, x2);  /* Store 8 samples */
		srce+=8;
		dste+=8;
	}

	if (remain)
		Mix32To16(dste, srce, remain);
}

/* Mix 32bit input to 8bit. 128 samples per iteration */
/* PC:OK, Mac: Ok */
static void Mix32To8_SIMD(SBYTE* dste,const SLONG* srce,NATIVE count)
{
	int	remain=count;

	while(!IS_ALIGNED_16(dste) || !IS_ALIGNED_16(srce))
	{
		SWORD x1;
		EXTRACT_SAMPLE(x1,8);
		CHECK_SAMPLE(x1,128);
		PUT_SAMPLE(x1+128);
		count--;
		if (!count) return;
	}

	remain = count&15;

	for(count>>=4;count;count--) {
		simd_m128i x1,x2,x3,x4;
		EXTRACT_SAMPLE_SIMD_8(srce, x1);  /* Load 4 samples */
		EXTRACT_SAMPLE_SIMD_8(srce+4, x2);  /* Load 4 samples */
		EXTRACT_SAMPLE_SIMD_8(srce+8, x3);  /* Load 4 samples */
		EXTRACT_SAMPLE_SIMD_8(srce+12, x4);  /* Load 4 samples */
		PUT_SAMPLE_SIMD_B(dste, x1, x2, x3, x4); /* Store 16 samples */
		srce+=16;
		dste+=16;
	}

	if (remain)
		Mix32To8(dste, srce, remain);
}

#endif


static void AddChannel(SLONG* ptr,NATIVE todo)
{
	SLONGLONG end,done;
	SWORD *s;

	if(!(s=Samples[vnf->handle])) {
		vnf->current = vnf->active  = 0;
		return;
	}

	/* update the 'current' index so the sample loops, or stops playing if it
	   reached the end of the sample */
	while(todo>0) {
		SLONGLONG endpos;

		if(vnf->flags & SF_REVERSE) {
			/* The sample is playing in reverse */
			if((vnf->flags&SF_LOOP)&&(vnf->current<idxlpos)) {
				/* the sample is looping and has reached the loopstart index */
				if(vnf->flags & SF_BIDI) {
					/* sample is doing bidirectional loops, so 'bounce' the
					   current index against the idxlpos */
					vnf->current = idxlpos+(idxlpos-vnf->current);
					vnf->flags &= ~SF_REVERSE;
					vnf->increment = -vnf->increment;
				} else
					/* normal backwards looping, so set the current position to
					   loopend index */
					vnf->current=idxlend-(idxlpos-vnf->current);
			} else {
				/* the sample is not looping, so check if it reached index 0 */
				if(vnf->current < 0) {
					/* playing index reached 0, so stop playing this sample */
					vnf->current = vnf->active  = 0;
					break;
				}
			}
		} else {
			/* The sample is playing forward */
			if((vnf->flags & SF_LOOP) &&
			   (vnf->current >= idxlend)) {
				/* the sample is looping, check the loopend index */
				if(vnf->flags & SF_BIDI) {
					/* sample is doing bidirectional loops, so 'bounce' the
					   current index against the idxlend */
					vnf->flags |= SF_REVERSE;
					vnf->increment = -vnf->increment;
					vnf->current = idxlend-(vnf->current-idxlend);
				} else
					/* normal backwards looping, so set the current position
					   to loopend index */
					vnf->current=idxlpos+(vnf->current-idxlend);
			} else {
				/* sample is not looping, so check if it reached the last
				   position */
				if(vnf->current >= idxsize) {
					/* yes, so stop playing this sample */
					vnf->current = vnf->active  = 0;
					break;
				}
			}
		}

		end=(vnf->flags&SF_REVERSE)?(vnf->flags&SF_LOOP)?idxlpos:0:
		     (vnf->flags&SF_LOOP)?idxlend:idxsize;

		/* if the sample is not blocked... */
		if((end==vnf->current)||(!vnf->increment))
			done=0;
		else {
			done=MIN((end-vnf->current)/vnf->increment+1,todo);
			if(done<0) done=0;
		}

		if(!done) {
			vnf->active = 0;
			break;
		}

		endpos=vnf->current+done*vnf->increment;

		if(vnf->vol) {
#ifndef NATIVE_64BIT_INT
			/* use the 32 bit mixers as often as we can (they're much faster) */
			if((vnf->current<0x7fffffff)&&(endpos<0x7fffffff)) {
				if((md_mode & DMODE_INTERP)) {
					if(vc_mode & DMODE_STEREO) {
						if((vnf->pan==PAN_SURROUND)&&(md_mode&DMODE_SURROUND))
							vnf->current=Mix32SurroundInterp
								   (s,ptr,vnf->current,vnf->increment,done);
						else
							vnf->current=Mix32StereoInterp
								   (s,ptr,vnf->current,vnf->increment,done);
					} else
						vnf->current=Mix32MonoInterp
								   (s,ptr,vnf->current,vnf->increment,done);
				} else if(vc_mode & DMODE_STEREO) {
					if((vnf->pan==PAN_SURROUND)&&(md_mode&DMODE_SURROUND))
						vnf->current=Mix32SurroundNormal
								   (s,ptr,vnf->current,vnf->increment,done);
					else
					{
#if defined HAVE_ALTIVEC || defined HAVE_SSE2
					    if (md_mode & DMODE_SIMDMIXER)
						vnf->current=MixSIMDStereoNormal
								   (s,ptr,vnf->current,vnf->increment,done);
					    else
#endif
						vnf->current=Mix32StereoNormal
								   (s,ptr,vnf->current,vnf->increment,done);
					}
				} else
					vnf->current=Mix32MonoNormal
								   (s,ptr,vnf->current,vnf->increment,done);
			}
			else
#endif
			{
				if((md_mode & DMODE_INTERP)) {
					if(vc_mode & DMODE_STEREO) {
						if((vnf->pan==PAN_SURROUND)&&(md_mode&DMODE_SURROUND))
							vnf->current=MixSurroundInterp
								   (s,ptr,vnf->current,vnf->increment,done);
						else
							vnf->current=MixStereoInterp
								   (s,ptr,vnf->current,vnf->increment,done);
					} else
						vnf->current=MixMonoInterp
								   (s,ptr,vnf->current,vnf->increment,done);
				} else if(vc_mode & DMODE_STEREO) {
					if((vnf->pan==PAN_SURROUND)&&(md_mode&DMODE_SURROUND))
						vnf->current=MixSurroundNormal
								   (s,ptr,vnf->current,vnf->increment,done);
					else
					{
#if defined HAVE_ALTIVEC || defined HAVE_SSE2
					    if (md_mode & DMODE_SIMDMIXER)
						vnf->current=MixSIMDStereoNormal
								   (s,ptr,vnf->current,vnf->increment,done);
					    else
#endif
						vnf->current=MixStereoNormal
								   (s,ptr,vnf->current,vnf->increment,done);
					}
				} else
					vnf->current=MixMonoNormal
								   (s,ptr,vnf->current,vnf->increment,done);
			}
		} else
			/* update sample position */
			vnf->current=endpos;

		todo-=done;
		ptr +=(vc_mode & DMODE_STEREO)?(done<<1):done;
	}
}

#ifdef NO_HQMIXER
#define VC_SetupPointers() do{}while(0)
#define VC1_Init VC_Init
#define VC1_Exit VC_Exit
#define VC1_PlayStart VC_PlayStart
#define VC1_PlayStop VC_PlayStop
#define VC1_SampleLength VC_SampleLength
#define VC1_SampleSpace VC_SampleSpace
#define VC1_SampleLoad VC_SampleLoad
#define VC1_SampleUnload VC_SampleUnload
#define VC1_SetNumVoices VC_SetNumVoices
#define VC1_SilenceBytes VC_SilenceBytes
#define VC1_VoicePlay VC_VoicePlay
#define VC1_VoiceStop VC_VoiceStop
#define VC1_VoiceGetFrequency VC_VoiceGetFrequency
#define VC1_VoiceGetPanning VC_VoiceGetPanning
#define VC1_VoiceGetPosition VC_VoiceGetPosition
#define VC1_VoiceGetVolume VC_VoiceGetVolume
#define VC1_VoiceRealVolume VC_VoiceRealVolume
#define VC1_VoiceSetFrequency VC_VoiceSetFrequency
#define VC1_VoiceSetPanning VC_VoiceSetPanning
#define VC1_VoiceSetVolume VC_VoiceSetVolume
#define VC1_VoiceStopped VC_VoiceStopped
#define VC1_WriteBytes VC_WriteBytes
#define VC1_WriteSamples VC_WriteSamples
#endif

#define _IN_VIRTCH_
#include "virtch_common.c"
#undef _IN_VIRTCH_

void VC1_WriteSamples(SBYTE* buf,ULONG todo)
{
	int left,portion=0,count;
	SBYTE  *buffer;
	int t, pan, vol;

	while(todo) {
		if(!tickleft) {
			if(vc_mode & DMODE_SOFT_MUSIC) md_player();
			tickleft=(md_mixfreq*125L)/(md_bpm*50L);
		}
		left = MIN(tickleft, todo);
		buffer    = buf;
		tickleft -= left;
		todo     -= left;
		buf += samples2bytes(left);

		while(left) {
			portion = MIN(left, samplesthatfit);
			count   = (vc_mode & DMODE_STEREO)?(portion<<1):portion;
			memset(vc_tickbuf, 0, count<<2);
			for(t=0;t<vc_softchn;t++) {
				vnf = &vinf[t];

				if(vnf->kick) {
					vnf->current=((SLONGLONG)vnf->start)<<FRACBITS;
					vnf->kick   =0;
					vnf->active =1;
				}

				if(!vnf->frq) vnf->active = 0;

				if(vnf->active) {
					vnf->increment=((SLONGLONG)(vnf->frq<<FRACBITS))/md_mixfreq;
					if(vnf->flags&SF_REVERSE) vnf->increment=-vnf->increment;
					vol = vnf->vol;  pan = vnf->pan;

					vnf->oldlvol=vnf->lvolsel;vnf->oldrvol=vnf->rvolsel;
					if(vc_mode & DMODE_STEREO) {
						if(pan != PAN_SURROUND) {
							vnf->lvolsel=(vol*(PAN_RIGHT-pan))>>8;
							vnf->rvolsel=(vol*pan)>>8;
						} else
							vnf->lvolsel=vnf->rvolsel=vol/2;
					} else
						vnf->lvolsel=vol;

					idxsize = (vnf->size)? ((SLONGLONG)vnf->size << FRACBITS)-1 : 0;
					idxlend = (vnf->repend)? ((SLONGLONG)vnf->repend << FRACBITS)-1 : 0;
					idxlpos = (SLONGLONG)vnf->reppos << FRACBITS;
					AddChannel(vc_tickbuf, portion);
				}
			}

			if(md_mode & DMODE_NOISEREDUCTION) {
				MixLowPass(vc_tickbuf, portion);
			}

			if(md_reverb) {
				if(md_reverb>15) md_reverb=15;
				MixReverb(vc_tickbuf, portion);
			}

			if (vc_callback) {
				vc_callback((unsigned char*)vc_tickbuf, portion);
			}

#if defined HAVE_ALTIVEC || defined HAVE_SSE2
			if (md_mode & DMODE_SIMDMIXER)
			{
				if(vc_mode & DMODE_FLOAT)
					Mix32ToFP_SIMD((float*) buffer, vc_tickbuf, count);
				else if(vc_mode & DMODE_16BITS)
					Mix32To16_SIMD((SWORD*) buffer, vc_tickbuf, count);
				else
					Mix32To8_SIMD((SBYTE*) buffer, vc_tickbuf, count);
			}
			else
#endif
			{
				if(vc_mode & DMODE_FLOAT)
					Mix32ToFP((float*) buffer, vc_tickbuf, count);
				else if(vc_mode & DMODE_16BITS)
					Mix32To16((SWORD*) buffer, vc_tickbuf, count);
				else
					Mix32To8((SBYTE*) buffer, vc_tickbuf, count);
			}
			buffer += samples2bytes(portion);
			left   -= portion;
		}
	}
}

int VC1_Init(void)
{
#ifndef NO_HQMIXER
	VC_SetupPointers();

	if (md_mode&DMODE_HQMIXER)
		return VC2_Init();
#endif

	if(!(Samples=(SWORD**)MikMod_amalloc(MAXSAMPLEHANDLES*sizeof(SWORD*)))) {
		_mm_errno = MMERR_INITIALIZING_MIXER;
		return 1;
	}
	if(!vc_tickbuf) {
		if(!(vc_tickbuf=(SLONG*)MikMod_amalloc((TICKLSIZE+32)*sizeof(SLONG)))) {
			_mm_errno = MMERR_INITIALIZING_MIXER;
			return 1;
		}
	}

	MixReverb=(md_mode&DMODE_STEREO)?MixReverb_Stereo:MixReverb_Normal;
	MixLowPass=(md_mode&DMODE_STEREO)?MixLowPass_Stereo:MixLowPass_Normal;
	vc_mode = md_mode;
	return 0;
}

int VC1_PlayStart(void)
{
	samplesthatfit=TICKLSIZE;
	if(vc_mode & DMODE_STEREO) samplesthatfit >>= 1;
	tickleft = 0;

	RVc1 = (5000L * md_mixfreq) / REVERBERATION;
	RVc2 = (5078L * md_mixfreq) / REVERBERATION;
	RVc3 = (5313L * md_mixfreq) / REVERBERATION;
	RVc4 = (5703L * md_mixfreq) / REVERBERATION;
	RVc5 = (6250L * md_mixfreq) / REVERBERATION;
	RVc6 = (6953L * md_mixfreq) / REVERBERATION;
	RVc7 = (7813L * md_mixfreq) / REVERBERATION;
	RVc8 = (8828L * md_mixfreq) / REVERBERATION;

	if(!(RVbufL1=(SLONG*)MikMod_calloc((RVc1+1),sizeof(SLONG)))) return 1;
	if(!(RVbufL2=(SLONG*)MikMod_calloc((RVc2+1),sizeof(SLONG)))) return 1;
	if(!(RVbufL3=(SLONG*)MikMod_calloc((RVc3+1),sizeof(SLONG)))) return 1;
	if(!(RVbufL4=(SLONG*)MikMod_calloc((RVc4+1),sizeof(SLONG)))) return 1;
	if(!(RVbufL5=(SLONG*)MikMod_calloc((RVc5+1),sizeof(SLONG)))) return 1;
	if(!(RVbufL6=(SLONG*)MikMod_calloc((RVc6+1),sizeof(SLONG)))) return 1;
	if(!(RVbufL7=(SLONG*)MikMod_calloc((RVc7+1),sizeof(SLONG)))) return 1;
	if(!(RVbufL8=(SLONG*)MikMod_calloc((RVc8+1),sizeof(SLONG)))) return 1;

	/* allocate reverb buffers for the right channel if in stereo mode only. */
	if (vc_mode & DMODE_STEREO) {
		if(!(RVbufR1=(SLONG*)MikMod_calloc((RVc1+1),sizeof(SLONG)))) return 1;
		if(!(RVbufR2=(SLONG*)MikMod_calloc((RVc2+1),sizeof(SLONG)))) return 1;
		if(!(RVbufR3=(SLONG*)MikMod_calloc((RVc3+1),sizeof(SLONG)))) return 1;
		if(!(RVbufR4=(SLONG*)MikMod_calloc((RVc4+1),sizeof(SLONG)))) return 1;
		if(!(RVbufR5=(SLONG*)MikMod_calloc((RVc5+1),sizeof(SLONG)))) return 1;
		if(!(RVbufR6=(SLONG*)MikMod_calloc((RVc6+1),sizeof(SLONG)))) return 1;
		if(!(RVbufR7=(SLONG*)MikMod_calloc((RVc7+1),sizeof(SLONG)))) return 1;
		if(!(RVbufR8=(SLONG*)MikMod_calloc((RVc8+1),sizeof(SLONG)))) return 1;
	}

	RVRindex = 0;
	return 0;
}

void VC1_PlayStop(void)
{
	MikMod_free(RVbufL1);
	MikMod_free(RVbufL2);
	MikMod_free(RVbufL3);
	MikMod_free(RVbufL4);
	MikMod_free(RVbufL5);
	MikMod_free(RVbufL6);
	MikMod_free(RVbufL7);
	MikMod_free(RVbufL8);
	RVbufL1=RVbufL2=RVbufL3=RVbufL4=RVbufL5=RVbufL6=RVbufL7=RVbufL8=NULL;
	MikMod_free(RVbufR1);
	MikMod_free(RVbufR2);
	MikMod_free(RVbufR3);
	MikMod_free(RVbufR4);
	MikMod_free(RVbufR5);
	MikMod_free(RVbufR6);
	MikMod_free(RVbufR7);
	MikMod_free(RVbufR8);
	RVbufR1=RVbufR2=RVbufR3=RVbufR4=RVbufR5=RVbufR6=RVbufR7=RVbufR8=NULL;
}

int VC1_SetNumVoices(void)
{
	int t;

	if(!(vc_softchn=md_softchn)) return 0;

	MikMod_free(vinf);
	if(!(vinf=(VINFO*)MikMod_calloc(vc_softchn,sizeof(VINFO)))) return 1;

	for(t=0;t<vc_softchn;t++) {
		vinf[t].frq=10000;
		vinf[t].pan=(t&1)?PAN_LEFT:PAN_RIGHT;
	}

	return 0;
}

/* ex:set ts=4: */
