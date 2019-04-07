/*	MikMod sound library
	(c) 1998, 1999, 2000 Miodrag Vallat and others - see file AUTHORS for
	complete list.

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

  High-quality sample mixing routines, using a 32 bits mixing buffer,
  interpolation, and sample smoothing to improve sound quality and remove
  clicks.

==============================================================================*/

/*

  Future Additions:
	Low-Pass filter to remove annoying staticy buzz.

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef NO_HQMIXER

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#include <string.h>

#include "mikmod_internals.h"

/*
   Constant Definitions
   ====================

	MAXVOL_FACTOR (was BITSHIFT in virtch.c)
		Controls the maximum volume of the output data. All mixed data is
		divided by this number after mixing, so larger numbers result in
		quieter mixing.  Smaller numbers will increase the likeliness of
		distortion on loud modules.

	REVERBERATION
		Larger numbers result in shorter reverb duration. Longer reverb
		durations can cause unwanted static and make the reverb sound more
		like a crappy echo.

	SAMPLING_SHIFT
		Specified the shift multiplier which controls by how much the mixing
		rate is multiplied while mixing.  Higher values can improve quality by
		smoothing the sound and reducing pops and clicks. Note, this is a shift
		value, so a value of 2 becomes a mixing-rate multiplier of 4, and a
		value of 3 = 8, etc.

	FRACBITS
		The number of bits per integer devoted to the fractional part of the
		number. Generally, this number should not be changed for any reason.

	!!! IMPORTANT !!! All values below MUST ALWAYS be greater than 0

*/

#define BITSHIFT 9
#define MAXVOL_FACTOR (1<<BITSHIFT)
#define	REVERBERATION 11000L

#define SAMPLING_SHIFT 2
#define SAMPLING_FACTOR (1UL<<SAMPLING_SHIFT)

#define	FRACBITS 28
#define FRACMASK ((1UL<<FRACBITS)-1UL)

#define TICKLSIZE 8192
#define TICKWSIZE (TICKLSIZE * 2)
#define TICKBSIZE (TICKWSIZE * 2)

#define CLICK_SHIFT_BASE 6
#define CLICK_SHIFT (CLICK_SHIFT_BASE + SAMPLING_SHIFT)
#define CLICK_BUFFER (1L << CLICK_SHIFT)

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

	int       click;
	int       rampvol;
	SLONG     lastvalL,lastvalR;
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

#ifdef _MSC_VER
/* Weird bug in compiler */ /* FIXME is this still needed? */
typedef void (*MikMod_callback_t)(unsigned char *data, size_t len);
#endif

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

/*========== 32 bit sample mixers - only for 32 bit platforms */
#ifndef NATIVE_64BIT_INT

static SLONG Mix32MonoNormal(const SWORD* const srce,SLONG* dest,SLONG idx,SLONG increment,SLONG todo)
{
	SWORD sample=0;
	SLONG i,f;

	while(todo--) {
		i=idx>>FRACBITS,f=idx&FRACMASK;
		sample=(SWORD)( (((SLONG)(srce[i]*(FRACMASK+1L-f)) +
		        ((SLONG)srce[i+1]*f)) >> FRACBITS));
		idx+=increment;

		if(vnf->rampvol) {
			*dest++ += (long)(
			  ( ( (SLONG)(vnf->oldlvol*vnf->rampvol) +
			      (vnf->lvolsel*(CLICK_BUFFER-vnf->rampvol)) ) *
			    (SLONG)sample ) >> CLICK_SHIFT );
			vnf->rampvol--;
		} else
		  if(vnf->click) {
			*dest++ += (long)(
			  ( ( ((SLONG)vnf->lvolsel*(CLICK_BUFFER-vnf->click)) *
			      (SLONG)sample ) +
			    (vnf->lastvalL*vnf->click) ) >> CLICK_SHIFT );
			vnf->click--;
		} else
			*dest++ +=vnf->lvolsel*sample;
	}
	vnf->lastvalL=vnf->lvolsel * sample;

	return idx;
}

static SLONG Mix32StereoNormal(const SWORD* const srce,SLONG* dest,SLONG idx,SLONG increment,ULONG todo)
{
	SWORD sample=0;
	SLONG i,f;

	while(todo--) {
		i=idx>>FRACBITS,f=idx&FRACMASK;
		sample=(SWORD)(((((SLONG)srce[i]*(FRACMASK+1L-f)) +
		        ((SLONG)srce[i+1] * f)) >> FRACBITS));
		idx += increment;

		if(vnf->rampvol) {
			*dest++ += (long)(
			  ( ( ((SLONG)vnf->oldlvol*vnf->rampvol) +
			      (vnf->lvolsel*(CLICK_BUFFER-vnf->rampvol))
			    ) * (SLONG)sample ) >> CLICK_SHIFT );
			*dest++ += (long)(
			  ( ( ((SLONG)vnf->oldrvol*vnf->rampvol) +
			      (vnf->rvolsel*(CLICK_BUFFER-vnf->rampvol))
			    ) * (SLONG)sample ) >> CLICK_SHIFT );
			vnf->rampvol--;
		} else
		  if(vnf->click) {
			*dest++ += (long)(
			  ( ( (SLONG)(vnf->lvolsel*(CLICK_BUFFER-vnf->click)) *
			      (SLONG)sample ) + (vnf->lastvalL * vnf->click) )
			    >> CLICK_SHIFT );
			*dest++ += (long)(
			  ( ( ((SLONG)vnf->rvolsel*(CLICK_BUFFER-vnf->click)) *
			      (SLONG)sample ) + (vnf->lastvalR * vnf->click) )
			    >> CLICK_SHIFT );
			vnf->click--;
		} else {
			*dest++ +=vnf->lvolsel*sample;
			*dest++ +=vnf->rvolsel*sample;
		}
	}
	vnf->lastvalL=vnf->lvolsel*sample;
	vnf->lastvalR=vnf->rvolsel*sample;

	return idx;
}

static SLONG Mix32StereoSurround(const SWORD* const srce,SLONG* dest,SLONG idx,SLONG increment,ULONG todo)
{
	SWORD sample=0;
	long whoop;
	SLONG i, f;

	while(todo--) {
		i=idx>>FRACBITS,f=idx&FRACMASK;
		sample=(SWORD)(((((SLONG)srce[i]*(FRACMASK+1L-f)) +
			((SLONG)srce[i+1]*f)) >> FRACBITS));
		idx+=increment;

		if(vnf->rampvol) {
			whoop=(long)(
			  ( ( (SLONG)(vnf->oldlvol*vnf->rampvol) +
			      (vnf->lvolsel*(CLICK_BUFFER-vnf->rampvol)) ) *
			    (SLONG)sample) >> CLICK_SHIFT );
			*dest++ +=whoop;
			*dest++ -=whoop;
			vnf->rampvol--;
		} else
		  if(vnf->click) {
			whoop = (long)(
			  ( ( ((SLONG)vnf->lvolsel*(CLICK_BUFFER-vnf->click)) *
			      (SLONG)sample) +
			    (vnf->lastvalL * vnf->click) ) >> CLICK_SHIFT );
			*dest++ +=whoop;
			*dest++ -=whoop;
			vnf->click--;
		} else {
			*dest++ +=vnf->lvolsel*sample;
			*dest++ -=vnf->lvolsel*sample;
		}
	}
	vnf->lastvalL=vnf->lvolsel*sample;
	vnf->lastvalR=vnf->lvolsel*sample;

	return idx;
}
#endif

/*========== 64 bit mixers */

static SLONGLONG MixMonoNormal(const SWORD* const srce,SLONG* dest,SLONGLONG idx,SLONGLONG increment,SLONG todo)
{
	SWORD sample=0;
	SLONGLONG i,f;

	while(todo--) {
		i=idx>>FRACBITS,f=idx&FRACMASK;
		sample=(SWORD)((((SLONGLONG)(srce[i]*(FRACMASK+1L-f)) +
			((SLONGLONG)srce[i+1]*f)) >> FRACBITS));
		idx+=increment;

		if(vnf->rampvol) {
			*dest++ += (long)(
			  ( ( (SLONGLONG)(vnf->oldlvol*vnf->rampvol) +
			      (vnf->lvolsel*(CLICK_BUFFER-vnf->rampvol)) ) *
			    (SLONGLONG)sample ) >> CLICK_SHIFT );
			vnf->rampvol--;
		} else
		  if(vnf->click) {
			*dest++ += (long)(
			  ( ( ((SLONGLONG)vnf->lvolsel*(CLICK_BUFFER-vnf->click)) *
			      (SLONGLONG)sample ) +
			    (vnf->lastvalL*vnf->click) ) >> CLICK_SHIFT );
			vnf->click--;
		} else
			*dest++ +=vnf->lvolsel*sample;
	}
	vnf->lastvalL=vnf->lvolsel * sample;

	return idx;
}

/* Slowest part... */

#if defined HAVE_SSE2 || defined HAVE_ALTIVEC

static __inline SWORD GetSample(const SWORD* const srce, SLONGLONG idx)
{
	SLONGLONG i=idx>>FRACBITS;
	SLONGLONG f=idx&FRACMASK;
	return (SWORD)(((((SLONGLONG)srce[i]*(FRACMASK+1L-f)) +
				((SLONGLONG)srce[i+1] * f)) >> FRACBITS));
}

static SLONGLONG MixSIMDStereoNormal(const SWORD* const srce,SLONG* dest,SLONGLONG idx,SLONGLONG increment,ULONG todo)
{
	SWORD vol[8] = {vnf->lvolsel, vnf->rvolsel};
	SWORD sample=0;
	SLONG remain = todo;

	/* Dest can be misaligned */
	while(!IS_ALIGNED_16(dest)) {
		sample=srce[idx >> FRACBITS];
		idx += increment;
		*dest++ += vol[0] * sample;
		*dest++ += vol[1] * sample;
		todo--;
		if(!todo) goto end;
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
			SWORD s0 = GetSample(srce, idx);
			SWORD s1 = GetSample(srce, idx += increment);
			SWORD s2 = GetSample(srce, idx += increment);
			SWORD s3 = GetSample(srce, idx += increment);
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
			s[0] = GetSample(srce, idx);
			s[1] = GetSample(srce, idx += increment);
			s[2] = GetSample(srce, idx += increment);
			s[3] = GetSample(srce, idx += increment);
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
			v4 = vec_ld(0x10, dest);
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
		sample=GetSample(srce, idx);
		idx+= increment;
		*dest++ += vol[0] * sample;
		*dest++ += vol[1] * sample;
	}
end:
	vnf->lastvalL=vnf->lvolsel*sample;
	vnf->lastvalR=vnf->rvolsel*sample;
	return idx;
}

static SLONGLONG MixStereoNormal(const SWORD* const srce,SLONG* dest,SLONGLONG idx,SLONGLONG increment,ULONG todo)
{
	SWORD sample=0;
	SLONGLONG i,f;

	if (vnf->rampvol)
	while(todo) {
		todo--;
		i=idx>>FRACBITS,f=idx&FRACMASK;
		sample=(SWORD)(((((SLONGLONG)srce[i]*(FRACMASK+1L-f)) +
			((SLONGLONG)srce[i+1] * f)) >> FRACBITS));
		idx += increment;

		*dest++ += (long)(
			( ( ((SLONGLONG)vnf->oldlvol*vnf->rampvol) +
			    (vnf->lvolsel*(CLICK_BUFFER-vnf->rampvol))
			) * (SLONGLONG)sample ) >> CLICK_SHIFT );
		*dest++ += (long)(
			( ( ((SLONGLONG)vnf->oldrvol*vnf->rampvol) +
			    (vnf->rvolsel*(CLICK_BUFFER-vnf->rampvol))
			) * (SLONGLONG)sample ) >> CLICK_SHIFT );
		vnf->rampvol--;

		if (!vnf->rampvol)
			break;
	}

	if (vnf->click)
	while(todo) {
		todo--;
		i=idx>>FRACBITS,f=idx&FRACMASK;
		sample=(SWORD)(((((SLONGLONG)srce[i]*(FRACMASK+1L-f)) +
			((SLONGLONG)srce[i+1] * f)) >> FRACBITS));
		idx += increment;

		*dest++ += (long)(
			  ( ( (SLONGLONG)(vnf->lvolsel*(CLICK_BUFFER-vnf->click)) *
			      (SLONGLONG)sample ) + (vnf->lastvalL * vnf->click) )
			    >> CLICK_SHIFT );

		*dest++ += (long)(
			  ( ( ((SLONGLONG)vnf->rvolsel*(CLICK_BUFFER-vnf->click)) *
			      (SLONGLONG)sample ) + (vnf->lastvalR * vnf->click) )
			    >> CLICK_SHIFT );
			vnf->click--;

		if (!vnf->click)
			break;
	}

	if (todo)
	{
		if (md_mode & DMODE_SIMDMIXER) {
			return MixSIMDStereoNormal(srce, dest, idx, increment, todo);
		}
		while(todo)
		{
			i=idx>>FRACBITS,
			f=idx&FRACMASK;
			sample=(SWORD)(((((SLONGLONG)srce[i]*(FRACMASK+1L-f)) +
							((SLONGLONG)srce[i+1] * f)) >> FRACBITS));
			idx += increment;

			*dest++ +=vnf->lvolsel*sample;
			*dest++ +=vnf->rvolsel*sample;
			todo--;
		}
	}
	vnf->lastvalL=vnf->lvolsel*sample;
	vnf->lastvalR=vnf->rvolsel*sample;

	return idx;
}

#else /* HAVE_SSE2 || HAVE_ALTIVEC */
static SLONGLONG MixStereoNormal(const SWORD* const srce,SLONG* dest,SLONGLONG idx,SLONGLONG increment,ULONG todo)
{
	SWORD sample=0;
	SLONGLONG i,f;

	while(todo--) {
		i=idx>>FRACBITS,f=idx&FRACMASK;
		sample=(SWORD)(((((SLONGLONG)srce[i]*(FRACMASK+1L-f)) +
			((SLONGLONG)srce[i+1] * f)) >> FRACBITS));
		idx += increment;

		if(vnf->rampvol) {
			*dest++ += (long)(
			  ( ( ((SLONGLONG)vnf->oldlvol*vnf->rampvol) +
			      (vnf->lvolsel*(CLICK_BUFFER-vnf->rampvol))
			    ) * (SLONGLONG)sample ) >> CLICK_SHIFT );
			*dest++ += (long)(
			  ( ( ((SLONGLONG)vnf->oldrvol*vnf->rampvol) +
			      (vnf->rvolsel*(CLICK_BUFFER-vnf->rampvol))
			    ) * (SLONGLONG)sample ) >> CLICK_SHIFT );
			vnf->rampvol--;
		} else
		  if(vnf->click) {
			*dest++ += (long)(
			  ( ( (SLONGLONG)(vnf->lvolsel*(CLICK_BUFFER-vnf->click)) *
			      (SLONGLONG)sample ) + (vnf->lastvalL * vnf->click) )
			    >> CLICK_SHIFT );
			*dest++ += (long)(
			  ( ( ((SLONGLONG)vnf->rvolsel*(CLICK_BUFFER-vnf->click)) *
			      (SLONGLONG)sample ) + (vnf->lastvalR * vnf->click) )
			    >> CLICK_SHIFT );
			vnf->click--;
		} else {
			*dest++ +=vnf->lvolsel*sample;
			*dest++ +=vnf->rvolsel*sample;
		}
	}
	vnf->lastvalL=vnf->lvolsel*sample;
	vnf->lastvalR=vnf->rvolsel*sample;

	return idx;
}
#endif /* HAVE_SSE2 || HAVE_ALTIVEC */


static SLONGLONG MixStereoSurround(const SWORD* srce,SLONG* dest,SLONGLONG idx,SLONGLONG increment,ULONG todo)
{
	SWORD sample=0;
	long whoop;
	SLONGLONG i, f;

	while(todo--) {
		i=idx>>FRACBITS,f=idx&FRACMASK;
		sample=(SWORD)(((((SLONGLONG)srce[i]*(FRACMASK+1L-f)) +
			((SLONGLONG)srce[i+1]*f)) >> FRACBITS));
		idx+=increment;

		if(vnf->rampvol) {
			whoop=(long)(
			  ( ( (SLONGLONG)(vnf->oldlvol*vnf->rampvol) +
			      (vnf->lvolsel*(CLICK_BUFFER-vnf->rampvol)) ) *
			    (SLONGLONG)sample) >> CLICK_SHIFT );
			*dest++ +=whoop;
			*dest++ -=whoop;
			vnf->rampvol--;
		} else
		  if(vnf->click) {
			whoop = (long)(
			  ( ( ((SLONGLONG)vnf->lvolsel*(CLICK_BUFFER-vnf->click)) *
			      (SLONGLONG)sample) +
			    (vnf->lastvalL * vnf->click) ) >> CLICK_SHIFT );
			*dest++ +=whoop;
			*dest++ -=whoop;
			vnf->click--;
		} else {
			*dest++ +=vnf->lvolsel*sample;
			*dest++ -=vnf->lvolsel*sample;
		}
	}
	vnf->lastvalL=vnf->lvolsel*sample;
	vnf->lastvalR=vnf->lvolsel*sample;

	return idx;
}

static	void(*Mix32toFP)(float* dste,const SLONG *srce,NATIVE count);
static	void(*Mix32to16)(SWORD* dste,const SLONG *srce,NATIVE count);
static	void(*Mix32to8)(SBYTE* dste,const SLONG *srce,NATIVE count);
static	void(*MixReverb)(SLONG *srce,NATIVE count);

/* Reverb macros */
#define COMPUTE_LOC(n) loc##n = RVRindex % RVc##n
#define COMPUTE_LECHO(n) RVbufL##n [loc##n ]=speedup+((ReverbPct*RVbufL##n [loc##n ])>>7)
#define COMPUTE_RECHO(n) RVbufR##n [loc##n ]=speedup+((ReverbPct*RVbufR##n [loc##n ])>>7)

static void MixReverb_Normal(SLONG *srce,NATIVE count)
{
	NATIVE speedup;
	int ReverbPct;
	unsigned int loc1,loc2,loc3,loc4,loc5,loc6,loc7,loc8;

	ReverbPct=58+(md_reverb*4);

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

static void MixReverb_Stereo(SLONG *srce,NATIVE count)
{
	NATIVE speedup;
	int ReverbPct;
	unsigned int loc1,loc2,loc3,loc4,loc5,loc6,loc7,loc8;

	ReverbPct=58+(md_reverb*4);

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

		/* left channel */
		*srce++ +=RVbufL1[loc1]-RVbufL2[loc2]+RVbufL3[loc3]-RVbufL4[loc4]+
			  RVbufL5[loc5]-RVbufL6[loc6]+RVbufL7[loc7]-RVbufL8[loc8];

		/* right channel */
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

/* Mixing macros */
#define EXTRACT_SAMPLE_FP(var,attenuation) var=*srce++*((1.0f / 32768.0f) / (MAXVOL_FACTOR*attenuation))
#define CHECK_SAMPLE_FP(var,bound) var=(var>bound)?bound:(var<-bound)?-bound:var

static void Mix32ToFP_Normal(float* dste,const SLONG *srce,NATIVE count)
{
	float x1,x2,tmpx;
	int i;

	for(count/=SAMPLING_FACTOR;count;count--) {
		tmpx=0.0f;

		for(i=SAMPLING_FACTOR/2;i;i--) {
			EXTRACT_SAMPLE_FP(x1,1.0f); EXTRACT_SAMPLE_FP(x2,1.0f);

			CHECK_SAMPLE_FP(x1,1.0f); CHECK_SAMPLE_FP(x2,1.0f);

			tmpx+=x1+x2;
		}
		*dste++ =tmpx*(1.0f/SAMPLING_FACTOR);
	}
}

static void Mix32ToFP_Stereo(float* dste,const SLONG *srce,NATIVE count)
{
	float x1,x2,x3,x4,tmpx,tmpy;
	int i;

	for(count/=SAMPLING_FACTOR;count;count--) {
		tmpx=tmpy=0.0f;

		for(i=SAMPLING_FACTOR/2;i;i--) {
			EXTRACT_SAMPLE_FP(x1,1.0f); EXTRACT_SAMPLE_FP(x2,1.0f);
			EXTRACT_SAMPLE_FP(x3,1.0f); EXTRACT_SAMPLE_FP(x4,1.0f);

			CHECK_SAMPLE_FP(x1,1.0f); CHECK_SAMPLE_FP(x2,1.0f);
			CHECK_SAMPLE_FP(x3,1.0f); CHECK_SAMPLE_FP(x4,1.0f);

			tmpx+=x1+x3;
			tmpy+=x2+x4;
		}
		*dste++ =tmpx*(1.0f/SAMPLING_FACTOR);
		*dste++ =tmpy*(1.0f/SAMPLING_FACTOR);
	}
}

/* Mixing macros */
#define EXTRACT_SAMPLE(var,attenuation) var=*srce++/(MAXVOL_FACTOR*attenuation)
#define CHECK_SAMPLE(var,bound) var=(var>=bound)?bound-1:(var<-bound)?-bound:var

static void Mix32To16_Normal(SWORD* dste,const SLONG *srce,NATIVE count)
{
	NATIVE x1,x2,tmpx;
	int i;

	for(count/=SAMPLING_FACTOR;count;count--) {
		tmpx=0;

		for(i=SAMPLING_FACTOR/2;i;i--) {
			EXTRACT_SAMPLE(x1,1); EXTRACT_SAMPLE(x2,1);

			CHECK_SAMPLE(x1,32768); CHECK_SAMPLE(x2,32768);

			tmpx+=x1+x2;
		}
		*dste++ =(SWORD)(tmpx/SAMPLING_FACTOR);
	}
}


static void Mix32To16_Stereo(SWORD* dste,const SLONG *srce,NATIVE count)
{
	NATIVE x1,x2,x3,x4,tmpx,tmpy;
	int i;

	for(count/=SAMPLING_FACTOR;count;count--) {
		tmpx=tmpy=0;

		for(i=SAMPLING_FACTOR/2;i;i--) {
			EXTRACT_SAMPLE(x1,1); EXTRACT_SAMPLE(x2,1);
			EXTRACT_SAMPLE(x3,1); EXTRACT_SAMPLE(x4,1);

			CHECK_SAMPLE(x1,32768); CHECK_SAMPLE(x2,32768);
			CHECK_SAMPLE(x3,32768); CHECK_SAMPLE(x4,32768);

			tmpx+=x1+x3;
			tmpy+=x2+x4;
		}
		*dste++ =(SWORD)(tmpx/SAMPLING_FACTOR);
		*dste++ =(SWORD)(tmpy/SAMPLING_FACTOR);
	}
}

static void Mix32To8_Normal(SBYTE* dste,const SLONG *srce,NATIVE count)
{
	NATIVE x1,x2,tmpx;
	int i;

	for(count/=SAMPLING_FACTOR;count;count--) {
		tmpx = 0;

		for(i=SAMPLING_FACTOR/2;i;i--) {
			EXTRACT_SAMPLE(x1,256); EXTRACT_SAMPLE(x2,256);

			CHECK_SAMPLE(x1,128); CHECK_SAMPLE(x2,128);

			tmpx+=x1+x2;
		}
		*dste++ = (SBYTE)((tmpx/SAMPLING_FACTOR)+128);
	}
}

static void Mix32To8_Stereo(SBYTE* dste,const SLONG *srce,NATIVE count)
{
	NATIVE x1,x2,x3,x4,tmpx,tmpy;
	int i;

	for(count/=SAMPLING_FACTOR;count;count--) {
		tmpx=tmpy=0;

		for(i=SAMPLING_FACTOR/2;i;i--) {
			EXTRACT_SAMPLE(x1,256); EXTRACT_SAMPLE(x2,256);
			EXTRACT_SAMPLE(x3,256); EXTRACT_SAMPLE(x4,256);

			CHECK_SAMPLE(x1,128); CHECK_SAMPLE(x2,128);
			CHECK_SAMPLE(x3,128); CHECK_SAMPLE(x4,128);

			tmpx+=x1+x3;
			tmpy+=x2+x4;
		}
		*dste++ =(SBYTE)((tmpx/SAMPLING_FACTOR)+128);
		*dste++ =(SBYTE)((tmpy/SAMPLING_FACTOR)+128);
	}
}

#if defined HAVE_SSE2
#define SHIFT_MIX_TO_16 (BITSHIFT + 16 - 16)
/* TEST: Ok */
static void Mix32To16_Stereo_SIMD_4Tap(SWORD* dste, const SLONG* srce, NATIVE count)
{
	int remain = count;

	/* Check unaligned dste buffer. srce is always aligned. */
	while(!IS_ALIGNED_16(dste))
	{
		Mix32To16_Stereo(dste, srce, SAMPLING_FACTOR);
		dste+=2;
		srce+=8;
		count--;
		if(!count) return;
	}

	/* dste and srce aligned. srce is always aligned. */
	remain = count & 15;
	/* count / 2 for 1 sample */

	for(count>>=4;count;count--)
	{
		/* Load 32bit sample. 1st average */
		__m128i v0 = _mm_add_epi32(
			_mm_srai_epi32(_mm_loadu_si128((__m128i const *)(srce+0)), SHIFT_MIX_TO_16),
			_mm_srai_epi32(_mm_loadu_si128((__m128i const *)(srce+4)), SHIFT_MIX_TO_16)
		);  /* v0: s0.l+s2.l | s0.r+s2.r | s1.l+s3.l | s1.r+s3.r */

		/* 2nd average (s0.l+s2.l+s1.l+s3.l / 4, s0.r+s2.r+s1.r+s3.r / 4). Upper 64bit is unused  (1 stereo sample) */
		__m128i v1 = _mm_srai_epi32(_mm_add_epi32(v0, mm_hiqq(v0)), 2);
		/* v1: s0.l+s2.l / 4 | s0.r+s2.r / 4 | s1.l+s3.l+s0.l+s2.l / 4 | s1.r+s3.r+s0.r+s2.r / 4 */

		__m128i v2 = _mm_add_epi32(
			_mm_srai_epi32(_mm_loadu_si128((__m128i const *)(srce+8)), SHIFT_MIX_TO_16),
			_mm_srai_epi32(_mm_loadu_si128((__m128i const *)(srce+12)), SHIFT_MIX_TO_16)
		);  /* v2: s4.l+s6.l | s4.r+s6.r | s5.l+s7.l | s5.r+s7.r */

		__m128i v3 = _mm_srai_epi32(_mm_add_epi32(v2, mm_hiqq(v2)), 2);  /* Upper 64bit is unused */
		/* v3: s4.l+s6.l /4 | s4.r+s6.r / 4| s5.l+s7.l+s4.l+s6.l / 4 | s5.r+s7.r+s4.r+s6.l / 4 */

		/* pack two stereo samples in one */
		__m128i v4 = _mm_unpacklo_epi64(v1, v3);  /* v4 = avg(s0,s1,s2,s3) | avg(s4,s5,s6,s7) */

		__m128i v6;

		/* Load 32bit sample. 1st average (s0.l+s2.l, s0.r+s2.r, s1.l+s3.l, s1.r+s3.r) */
		v0 = _mm_add_epi32(
			_mm_srai_epi32(_mm_loadu_si128((__m128i const *)(srce+16)), SHIFT_MIX_TO_16),
			_mm_srai_epi32(_mm_loadu_si128((__m128i const *)(srce+20)), SHIFT_MIX_TO_16)
		);  /* 128bit = 2 stereo samples */

		/* 2nd average (s0.l+s2.l+s1.l+s3.l / 4, s0.r+s2.r+s1.r+s3.r / 4). Upper 64bit is unused  (1 stereo sample) */
		v1 = _mm_srai_epi32(_mm_add_epi32(v0, mm_hiqq(v0)), 2);

		v2 = _mm_add_epi32(
			_mm_srai_epi32(_mm_loadu_si128((__m128i const *)(srce+24)), SHIFT_MIX_TO_16),
			_mm_srai_epi32(_mm_loadu_si128((__m128i const *)(srce+28)), SHIFT_MIX_TO_16)
		);

		v3 = _mm_srai_epi32(_mm_add_epi32(v2, mm_hiqq(v2)), 2);  /* Upper 64bit is unused */

		/* pack two stereo samples in one */
		v6 = _mm_unpacklo_epi64(v1, v3);  /* v6 = avg(s8,s9,s10,s11) | avg(s12,s13,s14,s15) */

		_mm_store_si128((__m128i*)dste, _mm_packs_epi32(v4, v6));  /* 4 interpolated stereo sample 32bit to 4 */

		dste+=8;
		srce+=32; /* 32 = 4 * 8  */
	}

	/* FIXME: THIS PART WRITES PAST DST !! */
	if (remain)
	{
		Mix32To16_Stereo(dste, srce, remain);
	}
}

#elif defined HAVE_ALTIVEC
#define SHIFT_MIX_TO_16 vec_splat_u32(BITSHIFT + 16 - 16)
/* TEST: Ok */
static void Mix32To16_Stereo_SIMD_4Tap(SWORD* dste, const SLONG* srce, NATIVE count)
{
	int remain = count;

	/* Check unaligned dste buffer. srce is always aligned. */
	while(!IS_ALIGNED_16(dste))
	{
		Mix32To16_Stereo(dste, srce, SAMPLING_FACTOR);
		dste+=2;
		srce+=8;
		count--;
		if(!count) return;
	}

	/* dste and srce aligned. srce is always aligned. */
	remain = count & 15;
	for(count>>=4;count;count--)
	{
		/* Load 32bit sample. 1st average (s0.l+s2.l, s0.r+s2.r, s1.l+s3.l, s1.r+s3.r) */
		vector signed int v0 = vec_add(
			vec_sra(vec_ld(0, srce), SHIFT_MIX_TO_16),  /* 128bit = 2 stereo samples */
			vec_sra(vec_ld(0x10, srce), SHIFT_MIX_TO_16)
		);  /* 128bit = 2 stereo samples */

		/* 2nd average (s0.l+s2.l+s1.l+s3.l / 4, s0.r+s2.r+s1.r+s3.r / 4). Upper 64bit is unused  (1 stereo sample) */
		vector signed int v1 = vec_sra(vec_add(v0, vec_hiqq(v0)), vec_splat_u32(2));

		vector signed int v2 = vec_add(
			vec_sra(vec_ld(0x20, srce), SHIFT_MIX_TO_16),
			vec_sra(vec_ld(0x30, srce), SHIFT_MIX_TO_16)
		);

		vector signed int v3 = vec_sra(vec_add(v2, vec_hiqq(v2)), vec_splat_u32(2));  /* Upper 64bit is unused */

		/* pack two stereo samples in one */
		vector signed int v6, v4 = vec_unpacklo(v1, v3); /* v4 = lo64(v1) | lo64(v3) */

		/* Load 32bit sample. 1st average (s0.l+s2.l, s0.r+s2.r, s1.l+s3.l, s1.r+s3.r) */
		v0 = vec_add(
			vec_sra(vec_ld(0x40, srce), SHIFT_MIX_TO_16),	/* 128bit = 2 stereo samples */
			vec_sra(vec_ld(0x50, srce), SHIFT_MIX_TO_16)
		);  /* 128bit = 2 stereo samples */

		/* 2nd average (s0.l+s2.l+s1.l+s3.l / 4, s0.r+s2.r+s1.r+s3.r / 4). Upper 64bit is unused  (1 stereo sample) */
		v1 = vec_sra(vec_add(v0, vec_hiqq(v0)), vec_splat_u32(2));

		v2 = vec_add(
			vec_sra(vec_ld(0x60, srce), SHIFT_MIX_TO_16),
			vec_sra(vec_ld(0x70, srce), SHIFT_MIX_TO_16)
		);

		v3 = vec_sra(vec_add(v2, vec_hiqq(v2)), vec_splat_u32(2));  /* Upper 64bit is unused */

		/* pack two stereo samples in one */
		v6 = vec_unpacklo(v1, v3);

		vec_st(vec_packs(v4, v6), 0, dste);  /* 4 interpolated stereo sample 32bit to 4 interpolated stereo sample 16bit + saturation */

		dste+=8;
		srce+=32; /* 32 = 4 * 8  */
	}

	if (remain)
	{
		Mix32To16_Stereo(dste, srce, remain);
	}
}

#endif


static void AddChannel(SLONG* ptr,NATIVE todo)
{
	SLONGLONG end,done;
	SWORD *s;

	if(!(s=Samples[vnf->handle])) {
		vnf->current = vnf->active  = 0;
		vnf->lastvalL = vnf->lastvalR = 0;
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

		if(vnf->vol || vnf->rampvol) {
#ifndef NATIVE_64BIT_INT
			/* use the 32 bit mixers as often as we can (they're much faster) */
			if((vnf->current<0x7fffffff)&&(endpos<0x7fffffff)) {
				if(vc_mode & DMODE_STEREO) {
					if((vnf->pan==PAN_SURROUND)&&(vc_mode&DMODE_SURROUND))
						vnf->current=(SLONGLONG)Mix32StereoSurround
								(s,ptr,vnf->current,vnf->increment,done);
					else
						vnf->current=Mix32StereoNormal
								(s,ptr,vnf->current,vnf->increment,done);
				} else
					vnf->current=Mix32MonoNormal
								(s,ptr,vnf->current,vnf->increment,done);
			}
			else
#endif
			{
				if(vc_mode & DMODE_STEREO) {
					if((vnf->pan==PAN_SURROUND)&&(vc_mode&DMODE_SURROUND))
						vnf->current=MixStereoSurround
								(s,ptr,vnf->current,vnf->increment,done);
					else
						vnf->current=MixStereoNormal
								(s,ptr,vnf->current,vnf->increment,done);
				} else
					vnf->current=MixMonoNormal
								(s,ptr,vnf->current,vnf->increment,done);
			}
		} else  {
			vnf->lastvalL = vnf->lastvalR = 0;
			/* update sample position */
			vnf->current=endpos;
		}

		todo -= done;
		ptr += (vc_mode & DMODE_STEREO)?(done<<1):done;
	}
}

#define _IN_VIRTCH_

#define VC1_SilenceBytes      VC2_SilenceBytes
#define VC1_WriteSamples      VC2_WriteSamples
#define VC1_WriteBytes        VC2_WriteBytes
#define VC1_Exit              VC2_Exit
#define VC1_VoiceSetVolume    VC2_VoiceSetVolume
#define VC1_VoiceGetVolume    VC2_VoiceGetVolume
#define VC1_VoiceSetPanning   VC2_VoiceSetPanning
#define VC1_VoiceGetPanning   VC2_VoiceGetPanning
#define VC1_VoiceSetFrequency VC2_VoiceSetFrequency
#define VC1_VoiceGetFrequency VC2_VoiceGetFrequency
#define VC1_VoicePlay         VC2_VoicePlay
#define VC1_VoiceStop         VC2_VoiceStop
#define VC1_VoiceStopped      VC2_VoiceStopped
#define VC1_VoiceGetPosition  VC2_VoiceGetPosition
#define VC1_SampleUnload      VC2_SampleUnload
#define VC1_SampleLoad        VC2_SampleLoad
#define VC1_SampleSpace       VC2_SampleSpace
#define VC1_SampleLength      VC2_SampleLength
#define VC1_VoiceRealVolume   VC2_VoiceRealVolume

#include "virtch_common.c"
#undef _IN_VIRTCH_

void VC2_WriteSamples(SBYTE* buf,ULONG todo)
{
	int left,portion=0;
	SBYTE *buffer;
	int t,pan,vol;

	todo*=SAMPLING_FACTOR;

	while(todo) {
		if(!tickleft) {
			if(vc_mode & DMODE_SOFT_MUSIC) md_player();
			tickleft=(md_mixfreq*125L*SAMPLING_FACTOR)/(md_bpm*50L);
			tickleft&=~(SAMPLING_FACTOR-1);
		}
		left = MIN(tickleft, todo);
		buffer    = buf;
		tickleft -= left;
		todo     -= left;
		buf += samples2bytes(left)/SAMPLING_FACTOR;

		while(left) {
			portion = MIN(left, samplesthatfit);
			memset(vc_tickbuf,0,portion<<((vc_mode&DMODE_STEREO)?3:2));
			for(t=0;t<vc_softchn;t++) {
				vnf = &vinf[t];

				if(vnf->kick) {
					vnf->current=((SLONGLONG)(vnf->start))<<FRACBITS;
					vnf->kick    = 0;
					vnf->active  = 1;
					vnf->click   = CLICK_BUFFER;
					vnf->rampvol = 0;
				}

				if(!vnf->frq) vnf->active = 0;

				if(vnf->active) {
					vnf->increment=((SLONGLONG)(vnf->frq)<<(FRACBITS-SAMPLING_SHIFT))
					               /md_mixfreq;
					if(vnf->flags&SF_REVERSE) vnf->increment=-vnf->increment;
					vol = vnf->vol;  pan = vnf->pan;

					vnf->oldlvol=vnf->lvolsel;vnf->oldrvol=vnf->rvolsel;
					if(vc_mode & DMODE_STEREO) {
						if(pan!=PAN_SURROUND) {
							vnf->lvolsel=(vol*(PAN_RIGHT-pan))>>8;
							vnf->rvolsel=(vol*pan)>>8;
						} else {
							vnf->lvolsel=vnf->rvolsel=(vol * 256L) / 480;
						}
					} else
						vnf->lvolsel=vol;

					idxsize=(vnf->size)?((SLONGLONG)(vnf->size)<<FRACBITS)-1:0;
					idxlend=(vnf->repend)?((SLONGLONG)(vnf->repend)<<FRACBITS)-1:0;
					idxlpos=(SLONGLONG)(vnf->reppos)<<FRACBITS;
					AddChannel(vc_tickbuf,portion);
				}
			}

			if(md_mode & DMODE_NOISEREDUCTION) {
				MixLowPass(vc_tickbuf, portion);
			}

			if(md_reverb) {
				if(md_reverb>15) md_reverb=15;
				MixReverb(vc_tickbuf,portion);
			}

			if (vc_callback) {
				vc_callback((unsigned char*)vc_tickbuf, portion);
			}

			if(vc_mode & DMODE_FLOAT)
				Mix32toFP((float*)buffer,vc_tickbuf,portion);
			else if(vc_mode & DMODE_16BITS)
				Mix32to16((SWORD*)buffer,vc_tickbuf,portion);
			else
				Mix32to8((SBYTE*)buffer,vc_tickbuf,portion);

			buffer += samples2bytes(portion) / SAMPLING_FACTOR;
			left   -= portion;
		}
	}
}

int VC2_Init(void)
{
	VC_SetupPointers();

	if (!(md_mode&DMODE_HQMIXER))
		return VC1_Init();

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

	if(md_mode & DMODE_STEREO) {
		Mix32toFP  = Mix32ToFP_Stereo;
#if ((defined HAVE_ALTIVEC || defined HAVE_SSE2) && (SAMPLING_FACTOR == 4))
		if (md_mode & DMODE_SIMDMIXER)
			Mix32to16  = Mix32To16_Stereo_SIMD_4Tap;
		else
#endif
			Mix32to16  = Mix32To16_Stereo;
		Mix32to8   = Mix32To8_Stereo;
		MixReverb  = MixReverb_Stereo;
		MixLowPass = MixLowPass_Stereo;
	} else {
		Mix32toFP  = Mix32ToFP_Normal;
		Mix32to16  = Mix32To16_Normal;
		Mix32to8   = Mix32To8_Normal;
		MixReverb  = MixReverb_Normal;
		MixLowPass = MixLowPass_Normal;
	}

	md_mode |= DMODE_INTERP;
	vc_mode = md_mode;
	return 0;
}

int VC2_PlayStart(void)
{
	md_mode|=DMODE_INTERP;

	samplesthatfit = TICKLSIZE;
	if(vc_mode & DMODE_STEREO) samplesthatfit >>= 1;
	tickleft = 0;

	RVc1 = (5000L * md_mixfreq) / (REVERBERATION * 10);
	RVc2 = (5078L * md_mixfreq) / (REVERBERATION * 10);
	RVc3 = (5313L * md_mixfreq) / (REVERBERATION * 10);
	RVc4 = (5703L * md_mixfreq) / (REVERBERATION * 10);
	RVc5 = (6250L * md_mixfreq) / (REVERBERATION * 10);
	RVc6 = (6953L * md_mixfreq) / (REVERBERATION * 10);
	RVc7 = (7813L * md_mixfreq) / (REVERBERATION * 10);
	RVc8 = (8828L * md_mixfreq) / (REVERBERATION * 10);

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

void VC2_PlayStop(void)
{
	MikMod_free(RVbufL1);
	MikMod_free(RVbufL2);
	MikMod_free(RVbufL3);
	MikMod_free(RVbufL4);
	MikMod_free(RVbufL5);
	MikMod_free(RVbufL6);
	MikMod_free(RVbufL7);
	MikMod_free(RVbufL8);
	MikMod_free(RVbufR1);
	MikMod_free(RVbufR2);
	MikMod_free(RVbufR3);
	MikMod_free(RVbufR4);
	MikMod_free(RVbufR5);
	MikMod_free(RVbufR6);
	MikMod_free(RVbufR7);
	MikMod_free(RVbufR8);

	RVbufL1=RVbufL2=RVbufL3=RVbufL4=RVbufL5=RVbufL6=RVbufL7=RVbufL8=NULL;
	RVbufR1=RVbufR2=RVbufR3=RVbufR4=RVbufR5=RVbufR6=RVbufR7=RVbufR8=NULL;
}

int VC2_SetNumVoices(void)
{
	int t;

	md_mode|=DMODE_INTERP;

	if(!(vc_softchn=md_softchn)) return 0;

	MikMod_free(vinf);
	if(!(vinf=(VINFO*)MikMod_calloc(vc_softchn,sizeof(VINFO)))) return 1;

	for(t=0;t<vc_softchn;t++) {
		vinf[t].frq=10000;
		vinf[t].pan=(t&1)?PAN_LEFT:PAN_RIGHT;
	}

	return 0;
}

#endif /* ! NO_HQMIXER */

/* ex:set ts=4: */
