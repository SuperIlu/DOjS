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

  Driver for output via CoreAudio [MacOS X and Darwin].

==============================================================================*/

/*

	Written by Axel Wefers <awe@fruitz-of-dojo.de>

	Notes:
	- if HAVE_PTHREAD (config.h) is defined, an extra thread will be created to fill the buffers.
	- if HAVE_PTHREAD is defined, a double buffered method will be used.
	- if an unsupported frequency is selected [md_mixfreq], the native device frequency is used.
	- if mono playback is selected and is not supported by the device, we will emulate mono
	  playback.
	- if stereo/surround playback is selected and is not supported by the device, DMODE_STEREO
	  will be deactivated automagically.

	Bug fixes by Anders F Bjoerklund <afb@algonet.se>

	Changes:
	- cleared up in the macro jungle, to see what was going on in the code
	- separated "has ability to use pthreads" from "wish to use pthreads"
	- moved pthread_cond_wait inside the mutex lock, to avoid a deadlock [!]
	- added more than one back buffer, currently left at eight or something
	- gave up on whole thread idea, since it stutters if you rescale a window
	- moved a #pragma mark and added DRV_OSX/MISSING, for non-Darwin compiles
	- added support for float-point buffers, to avoid the conversion and copying
	- Altivec optimizations of the various vector transforms (S.Denis)

	Future ideas: (TODO)

	- support possibly partially filled buffers from libmikmod
	- clean up the rest of the code and lose even more macros
	- use hardware preferred native size for the sample buffers
	- provide a PPC64 version of the library, for PowerMac G5

*/


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mikmod_internals.h"

#ifdef DRV_OSX

/* INCLUDES */
#include <mach-o/arch.h>
#include <sys/sysctl.h>
#include <CoreAudio/AudioHardware.h>

/* DEFINES */
#define SOUND_BUFFER_SCALE_8BIT		(1.0f / 128.0f)		/* CoreAudio requires float input. */
#define SOUND_BUFFER_SCALE_16BIT	(1.0f / 32768.0f)	/* CoreAudio requires float input. */

#define SOUND_BUFFER_SIZE		4096		/* The buffersize libmikmod will use. */
#define USE_FILL_THREAD			0		/* Use an extra thread to fill the buffers? */

#ifndef HAVE_PTHREAD
#undef USE_FILL_THREAD
#define USE_FILL_THREAD			0		/* must have pthread supports to use thread */
#endif

#define NUMBER_BACK_BUFFERS		8		/* Number of back buffers for the thread */
#define DEBUG_TRACE_THREADS		0

/* MACROS */
#define CHECK_ERROR(ERRNO, RESULT)								\
			if (RESULT != kAudioHardwareNoError) {					\
				_mm_errno = ERRNO;						\
				return 1;							\
			}

#define SET_PROPS()	if (AudioDeviceSetProperty(gSoundDeviceID, NULL, 0, 0,			\
							kAudioDevicePropertyStreamFormat,	\
							myPropertySize,				\
							&mySoundBasicDescription)) {		\
				CHECK_ERROR (MMERR_OSX_BAD_PROPERTY,				\
					AudioDeviceGetProperty(gSoundDeviceID, 0, 0,		\
							kAudioDevicePropertyStreamFormat,	\
							&myPropertySize,			\
							&mySoundBasicDescription));		\
			}

#define SET_STEREO()	switch (mySoundBasicDescription.mChannelsPerFrame) {			\
			case 1:									\
				md_mode &= ~DMODE_STEREO;					\
				gBufferMono2Stereo = 0;						\
				break;								\
			case 2:									\
				if (md_mode & DMODE_STEREO)	gBufferMono2Stereo = 0;		\
				else				gBufferMono2Stereo = 1;		\
				break;								\
			default:								\
				_mm_errno = MMERR_OSX_SET_STEREO;				\
				return 1;							\
			}

#define FILL_BUFFER(_buffer,_size)								\
			MUTEX_LOCK (vars);							\
			if (Player_Paused_internal())						\
				VC_SilenceBytes ((SBYTE*) (_buffer), (ULONG) (_size));		\
			else	VC_WriteBytes ((SBYTE*) (_buffer), (ULONG) (_size));		\
			MUTEX_UNLOCK (vars);

/* GLOBALS */
#if USE_FILL_THREAD

static pthread_t		gBufferFillThread;
static pthread_mutex_t		gBufferMutex;
static pthread_cond_t		gBufferCondition;
static Boolean			gExitBufferFillThread = 0;

static int			gCurrentPlayBuffer;
static int			gCurrentFillBuffer;
static unsigned char		*gSoundBackBuffer[NUMBER_BACK_BUFFERS];

#else

static unsigned char 		*gSoundBuffer = NULL;

#endif /* USE_FILL_THREAD */

static AudioDeviceID 		gSoundDeviceID;
static UInt32			gInBufferSize;
static UInt32			gHardwareBufferSize = 0;
static Boolean			gIOProcIsInstalled = 0,
				gDeviceHasStarted = 0,
				gBufferMono2Stereo = 0;

static OSStatus	(*gAudioIOProc) (AudioDeviceID,
				 const AudioTimeStamp *, const AudioBufferList *,
				 const AudioTimeStamp *, AudioBufferList *,
				 const AudioTimeStamp *, void *);

/* FUNCTION PROTOTYPES */
#if USE_FILL_THREAD

static void * OSX_FillBuffer (void *);

#endif /* USE_FILL_THREAD */

static OSStatus OSX_AudioIOProc8Bit (AudioDeviceID,
					const AudioTimeStamp *, const AudioBufferList *,
					const AudioTimeStamp *, AudioBufferList *,
					const AudioTimeStamp *, void *);
static OSStatus OSX_AudioIOProc16Bit (AudioDeviceID,
					const AudioTimeStamp *, const AudioBufferList *,
					const AudioTimeStamp *, AudioBufferList *,
					const AudioTimeStamp *, void *);
static OSStatus OSX_AudioIOProcFloat (AudioDeviceID,
					const AudioTimeStamp *, const AudioBufferList *,
					const AudioTimeStamp *, AudioBufferList *,
					const AudioTimeStamp *, void *);

static BOOL OSX_IsPresent (void);
static int OSX_Init (void);
static void OSX_Exit (void);
static int OSX_PlayStart (void);
static void OSX_PlayStop (void);
static void OSX_Update (void);


#if USE_FILL_THREAD
static void *OSX_FillBuffer (void *theID)
{
	unsigned char *buffer;
	int done;

	while (1)
	{
		done = 0;

		while (!done)
		{
			/* shall the thread exit? */
			if (gExitBufferFillThread) pthread_exit (NULL);

			pthread_mutex_lock (&gBufferMutex);

			if ((gCurrentFillBuffer+1) % NUMBER_BACK_BUFFERS != gCurrentPlayBuffer) {
			#if DEBUG_TRACE_THREADS
				fprintf(stderr,"filling buffer #%d\n", gCurrentFillBuffer);
			#endif
				buffer = gSoundBackBuffer[gCurrentFillBuffer];
				if (++gCurrentFillBuffer >= NUMBER_BACK_BUFFERS)
					gCurrentFillBuffer = 0;
				FILL_BUFFER (buffer, gInBufferSize);
			}
			else {
			/* we are caught up now, give it a rest */
				done = 1;
			}

			pthread_mutex_unlock (&gBufferMutex);
		}

		pthread_mutex_lock (&gBufferMutex);
		/* wait for the next buffer-fill request */
		pthread_cond_wait (&gBufferCondition, &gBufferMutex);
		pthread_mutex_unlock (&gBufferMutex);
	}

	return theID;
}
#endif /* USE_FILL_THREAD */

static OSStatus OSX_AudioIOProc8Bit (AudioDeviceID inDevice,
				     const AudioTimeStamp *inNow, const AudioBufferList *inInputData,
				     const AudioTimeStamp *inInputTime, AudioBufferList *outOutputData,
				     const AudioTimeStamp *inOutputTime, void *inClientData)
{
	register float	*myOutBuffer = (float *) outOutputData->mBuffers[0].mData;
	register UInt8	*myInBuffer;
	register UInt32	i;

#if USE_FILL_THREAD
	pthread_mutex_lock (&gBufferMutex);

	#if DEBUG_TRACE_THREADS
	fprintf(stderr,"playing buffer #%d\n", gCurrentPlayBuffer);
	#endif
	myInBuffer = (UInt8 *) gSoundBackBuffer[gCurrentPlayBuffer];
	if (++gCurrentPlayBuffer >= NUMBER_BACK_BUFFERS)
		gCurrentPlayBuffer = 0;
	pthread_cond_signal (&gBufferCondition);
	pthread_mutex_unlock (&gBufferMutex);
#else
	myInBuffer = (UInt8 *) gSoundBuffer;
	FILL_BUFFER(gSoundBuffer, gInBufferSize);
#endif /* USE_FILL_THREAD */

	if (gBufferMono2Stereo) {
		for (i = 0; i < SOUND_BUFFER_SIZE >> 1; i++) {
			myOutBuffer[1] = myOutBuffer[0] = (*myInBuffer++) * SOUND_BUFFER_SCALE_8BIT;
			myOutBuffer+=2;
		}
	}
	else {
		for (i = 0; i < SOUND_BUFFER_SIZE; i++) {
			*myOutBuffer++ = (*myInBuffer++) * SOUND_BUFFER_SCALE_8BIT;
		}
	}

	return 0;
}

#ifdef HAVE_SSE2
/* FIXME: SSE2-specific code here? */
#endif /* HAVE_SSE2 */

#ifdef HAVE_ALTIVEC
/* note: AltiVec code needs to be in a function of its own,
 *	 since the compiler will generate vrsave instructions */

#ifdef __GNUC__
__attribute__((noinline))
#endif
static void OSX_AudioIOProc16Bit_Altivec(SInt16	*myInBuffer, float *myOutBuffer)
{
	register UInt32	i;

	float f = SOUND_BUFFER_SCALE_16BIT;
	const vector float gain = vec_load_ps1(&f); /* multiplier */
	const vector float mix = vec_setzero();

	if (gBufferMono2Stereo) {
		int j = 0;
		/* TEST: OK */
		for (i = 0; i < SOUND_BUFFER_SIZE; i += 8, j += 16) {
			vector short int v0 = vec_ld(0, myInBuffer + i); /* Load 8 shorts */
			vector float v1 = vec_ctf((vector signed int)vec_unpackh(v0), 0); /* convert to float */
			vector float v2 = vec_ctf((vector signed int)vec_unpackl(v0), 0); /* convert to float */
			vector float v3 = vec_madd(v1, gain, mix); /* scale */
			vector float v4 = vec_madd(v2, gain, mix); /* scale */

			vector float v5 = vec_mergel(v3, v3); /* v3(0,0,1,1); */
			vector float v6 = vec_mergeh(v3, v3); /* v3(2,2,3,3); */
			vector float v7 = vec_mergel(v4, v4); /* v4(0,0,1,1); */
			vector float v8 = vec_mergeh(v4, v4); /* v4(2,2,3,3); */

			vec_st(v5, 0, myOutBuffer + j); /* Store 4 floats */
			vec_st(v6, 0, myOutBuffer + 4 + j); /* Store 4 floats */
			vec_st(v7, 0, myOutBuffer + 8 + j); /* Store 4 floats */
			vec_st(v8, 0, myOutBuffer + 12 + j); /* Store 4 floats */
		}
	}
	else {
		/* TEST: OK */
		for (i = 0; i < SOUND_BUFFER_SIZE; i += 8) {
			vector short int v0 = vec_ld(0, myInBuffer + i); /* Load 8 shorts */
			vector float v1 = vec_ctf((vector signed int)vec_unpackh(v0), 0); /* convert to float */
			vector float v2 = vec_ctf((vector signed int)vec_unpackl(v0), 0); /* convert to float */
			vector float v3 = vec_madd(v1, gain, mix); /* scale */
			vector float v4 = vec_madd(v2, gain, mix); /* scale */
			vec_st(v3, 0, myOutBuffer + i); /* Store 4 floats */
			vec_st(v4, 0, myOutBuffer + 4 + i); /* Store 4 floats */
		}
	}
}
#endif /* HAVE_ALTIVEC */

static OSStatus OSX_AudioIOProc16Bit (AudioDeviceID inDevice,
				      const AudioTimeStamp *inNow, const AudioBufferList *inInputData,
				      const AudioTimeStamp *inInputTime, AudioBufferList *outOutputData,
				      const AudioTimeStamp *inOutputTime, void *inClientData)
{
	register float	*myOutBuffer = (float *) outOutputData->mBuffers[0].mData;
	register SInt16	*myInBuffer;
	register UInt32	i;

#if USE_FILL_THREAD
	pthread_mutex_lock (&gBufferMutex);

	#if DEBUG_TRACE_THREADS
	fprintf(stderr,"playing buffer #%d\n", gCurrentPlayBuffer);
	#endif

	myInBuffer = (SInt16 *) gSoundBackBuffer[gCurrentPlayBuffer];
	if (++gCurrentPlayBuffer >= NUMBER_BACK_BUFFERS)
		gCurrentPlayBuffer = 0;
	pthread_cond_signal (&gBufferCondition);
	pthread_mutex_unlock (&gBufferMutex);
#else
	myInBuffer = (SInt16 *) gSoundBuffer;
	FILL_BUFFER(gSoundBuffer, gInBufferSize);
#endif /* USE_FILL_THREAD */

#ifdef HAVE_ALTIVEC
	if (md_mode & DMODE_SIMDMIXER) {
	#if __MWERKS__
		#pragma dont_inline on
	#endif
		OSX_AudioIOProc16Bit_Altivec(myInBuffer,myOutBuffer);
	#if __MWERKS__
		#pragma dont_inline reset
	#endif
	}
	else
#endif
	{
		if (gBufferMono2Stereo) {
			for (i = 0; i < SOUND_BUFFER_SIZE >> 1; i++) {
				myOutBuffer[1] = myOutBuffer[0] = (*myInBuffer++) * SOUND_BUFFER_SCALE_16BIT;
				myOutBuffer+=2;
			}
		}
		else {
			for (i = 0; i < SOUND_BUFFER_SIZE; i++) {
				*myOutBuffer++ = (*myInBuffer++) * SOUND_BUFFER_SCALE_16BIT;
			}
		}
	}

	return 0;
}

static OSStatus OSX_AudioIOProcFloat (AudioDeviceID inDevice,
				      const AudioTimeStamp *inNow, const AudioBufferList *inInputData,
				      const AudioTimeStamp *inInputTime, AudioBufferList *outOutputData,
				      const AudioTimeStamp *inOutputTime, void *inClientData)
{
	register float	*myOutBuffer = (float *) outOutputData->mBuffers[0].mData;
	register float	*myInBuffer;
	register UInt32	i;

#if USE_FILL_THREAD
	pthread_mutex_lock (&gBufferMutex);

	#if DEBUG_TRACE_THREADS
	fprintf(stderr,"playing buffer #%d\n", gCurrentPlayBuffer);
	#endif

	myInBuffer = (float *) gSoundBackBuffer[gCurrentPlayBuffer];
	if (++gCurrentPlayBuffer >= NUMBER_BACK_BUFFERS)
		gCurrentPlayBuffer = 0;
	pthread_cond_signal (&gBufferCondition);
	pthread_mutex_unlock (&gBufferMutex);
#else
	/* avoid copy, if no conversion needed */
	myInBuffer = (gBufferMono2Stereo) ? (float *) gSoundBuffer : myOutBuffer;
	FILL_BUFFER( myInBuffer, gInBufferSize);
#endif /* USE_FILL_THREAD */

	if (gBufferMono2Stereo) {
		for (i = 0; i < SOUND_BUFFER_SIZE >> 1; i++) {
			myOutBuffer[1] = myOutBuffer[0] = *myInBuffer++;
			myOutBuffer+=2;
		}
	}
	else if (myInBuffer != myOutBuffer) {
		for (i = 0; i < SOUND_BUFFER_SIZE; i++) {
			*myOutBuffer++ = *myInBuffer++;
		}
	}

	return 0;
}

#ifdef HAVE_ALTIVEC
static BOOL OSX_HasAltivec (void)
{
	int result = 0;
	int selectors[2] = { CTL_HW, HW_VECTORUNIT };
	size_t length = sizeof(result);
	sysctl(selectors, 2, &result, &length, NULL, 0);
	return !!result;
}
#endif

static BOOL OSX_IsPresent (void)
{
/* weak_import and check syms? meh.. */
	return 1;
}

static int OSX_Init (void)
{
	AudioStreamBasicDescription	mySoundBasicDescription;
	UInt32				myPropertySize, myBufferByteCount;
#if USE_FILL_THREAD
	int				i;
#endif

	/* get the device */
	myPropertySize = sizeof (gSoundDeviceID);
	CHECK_ERROR(MMERR_DETECTING_DEVICE,
		AudioHardwareGetProperty (kAudioHardwarePropertyDefaultOutputDevice,
					  &myPropertySize, &gSoundDeviceID)
	);
	if (gSoundDeviceID == kAudioDeviceUnknown) {
		_mm_errno = MMERR_OSX_UNKNOWN_DEVICE;
		return 1;
	}

	/* get the device format */
	myPropertySize = sizeof (mySoundBasicDescription);
	CHECK_ERROR(MMERR_OSX_BAD_PROPERTY,
		AudioDeviceGetProperty (gSoundDeviceID, 0, 0, kAudioDevicePropertyStreamFormat,
					&myPropertySize, &mySoundBasicDescription)
	);

	/* set up basic md_mode, just to be secure */
	md_mode |= DMODE_SOFT_MUSIC | DMODE_SOFT_SNDFX;

#ifdef HAVE_ALTIVEC
	/* check for Altivec support */
	if (OSX_HasAltivec()) {
		md_mode |= DMODE_SIMDMIXER;
	}
#endif

	/* try the selected mix frequency, if failure, fall back to native frequency */
	if (mySoundBasicDescription.mSampleRate != md_mixfreq) {
		mySoundBasicDescription.mSampleRate = md_mixfreq;
		SET_PROPS ();
		md_mixfreq = mySoundBasicDescription.mSampleRate;
	}

	/* try selected channels, if failure select native channels */
	switch (md_mode & DMODE_STEREO) {
	case 0:
		if (mySoundBasicDescription.mChannelsPerFrame != 1) {
			mySoundBasicDescription.mChannelsPerFrame = 1;
			SET_PROPS ();
			SET_STEREO ();
		}
		break;
	case 1:
		if (mySoundBasicDescription.mChannelsPerFrame != 2) {
			mySoundBasicDescription.mChannelsPerFrame = 2;
			SET_PROPS();
			SET_STEREO();
		}
		break;
	}

	/* linear PCM is required */
	if (mySoundBasicDescription.mFormatID != kAudioFormatLinearPCM) {
		_mm_errno = MMERR_OSX_UNSUPPORTED_FORMAT;
		return 1;
	}

	/* prepare the buffers */
	if (gBufferMono2Stereo) {
		gInBufferSize = SOUND_BUFFER_SIZE >> 1;
	}
	else {
		gInBufferSize = SOUND_BUFFER_SIZE;
	}

	if (md_mode & DMODE_FLOAT) {
		gInBufferSize *= sizeof(float);
		gAudioIOProc = OSX_AudioIOProcFloat;
	}
	else if (md_mode & DMODE_16BITS) {
		gInBufferSize *= sizeof(SInt16);
		gAudioIOProc = OSX_AudioIOProc16Bit;
	}
	else {
		gInBufferSize *= sizeof(UInt8);
		gAudioIOProc = OSX_AudioIOProc8Bit;
	}

	/* save the original buffer size */
	myPropertySize = sizeof (gHardwareBufferSize);
	AudioDeviceGetProperty (gSoundDeviceID, 0, 0, kAudioDevicePropertyBufferSize,
				&myPropertySize, &gHardwareBufferSize);

	myBufferByteCount = SOUND_BUFFER_SIZE * sizeof(float);
	CHECK_ERROR(MMERR_OSX_BUFFER_ALLOC,
		AudioDeviceSetProperty (gSoundDeviceID, NULL, 0, 0, kAudioDevicePropertyBufferSize,
					sizeof(myBufferByteCount), &myBufferByteCount)
	);

	/* add our audio IO procedure. */
	CHECK_ERROR(MMERR_OSX_ADD_IO_PROC,
		AudioDeviceAddIOProc (gSoundDeviceID, gAudioIOProc, NULL)
	);
	gIOProcIsInstalled = 1;

#if !USE_FILL_THREAD
	/* get the buffer memory */
	if ((gSoundBuffer = (unsigned char *) MikMod_amalloc(gInBufferSize)) == NULL) {
		_mm_errno = MMERR_OUT_OF_MEMORY;
		return 1;
	}
#else
	/* some thread init */
	if (pthread_mutex_init (&gBufferMutex, NULL) ||
	    pthread_cond_init (&gBufferCondition, NULL)) {
		_mm_errno = MMERR_OSX_PTHREAD;
		return 1;
	}

	for (i = 0; i < NUMBER_BACK_BUFFERS; i++) {
		if ((gSoundBackBuffer[i] = (unsigned char *) MikMod_amalloc(gInBufferSize)) == NULL) {
			_mm_errno = MMERR_OUT_OF_MEMORY;
			return 1;
		}
	}

	gCurrentPlayBuffer = 0;
	gCurrentFillBuffer = 0;
#endif /* USE_FILL_THREAD */

	/* init mikmod */
	return VC_Init ();
}

static void OSX_Exit (void)
{
#if USE_FILL_THREAD
	int		i;
#endif

	if (gDeviceHasStarted) {
		/* stop the audio device */
		AudioDeviceStop (gSoundDeviceID, gAudioIOProc);
		gDeviceHasStarted = 0;
#if USE_FILL_THREAD
		/* finish the fill buffer thread off */
		pthread_mutex_lock (&gBufferMutex);
		gExitBufferFillThread = 1;
		pthread_mutex_unlock (&gBufferMutex);
		pthread_cond_signal(&gBufferCondition);
		pthread_join (gBufferFillThread, NULL);
	}

	/* destroy other thread related stuff */
	pthread_mutex_destroy (&gBufferMutex);
	pthread_cond_destroy (&gBufferCondition);
#else
	}
#endif /* USE_FILL_THREAD */

	/* remove the audio IO proc */
	if (gIOProcIsInstalled) {
		AudioDeviceRemoveIOProc (gSoundDeviceID, gAudioIOProc);
		gIOProcIsInstalled = 0;
	}

	if (gHardwareBufferSize) {
		AudioDeviceSetProperty (gSoundDeviceID, NULL, 0, 0, kAudioDevicePropertyBufferSize,
					sizeof(gHardwareBufferSize), &gHardwareBufferSize);
		gHardwareBufferSize = 0;
	}

#if !USE_FILL_THREAD
	/* free up the sound buffer */
	MikMod_afree (gSoundBuffer);
	gSoundBuffer = NULL;
#else
	for ( i = 0; i < NUMBER_BACK_BUFFERS; i++ ) {
		/* free up the back buffer */
		MikMod_afree (gSoundBackBuffer[i]);
		gSoundBackBuffer[i] = NULL;
	}
#endif /* USE_FILL_THREAD */

	VC_Exit ();
}

/* OSX_PlayStart() */
static int OSX_PlayStart (void)
{
	/* start virtch */
	if (VC_PlayStart ()) {
		return 1;
	}

	/* just for security: audio device already playing? */
	if (gDeviceHasStarted) return 0;

#if USE_FILL_THREAD
	/* start the buffer fill thread */
	gExitBufferFillThread = 0;
	if (pthread_create(&gBufferFillThread, NULL, OSX_FillBuffer, NULL)) {
		_mm_errno = MMERR_OSX_PTHREAD;
		return 1;
	}
#endif /* USE_FILL_THREAD */

	/* start the audio IO Proc */
	if (AudioDeviceStart (gSoundDeviceID, gAudioIOProc)) {
		_mm_errno = MMERR_OSX_DEVICE_START;
		return 1;
	}
	gDeviceHasStarted = 1;

	return 0;
}

static void OSX_PlayStop (void)
{
	if (gDeviceHasStarted) {
		/* stop the audio IO Proc */
		AudioDeviceStop (gSoundDeviceID, gAudioIOProc);
		gDeviceHasStarted = 0;

#if USE_FILL_THREAD
		/* finish the fill buffer thread off */
		pthread_mutex_lock (&gBufferMutex);
		gExitBufferFillThread = 1;
		pthread_mutex_unlock (&gBufferMutex);
		pthread_cond_signal (&gBufferCondition);
		pthread_join (gBufferFillThread, NULL);
#endif /* USE_FILL_THREAD */
	}

	/* tell virtch that playback has stopped. */
	VC_PlayStop ();
}

static void OSX_Update (void)
{
	/* do nothing */
}

MIKMODAPI MDRIVER drv_osx={
	NULL,
	"CoreAudio Driver",
	"CoreAudio Driver v2.1",
	0,255,
	"osx",
	NULL,
	NULL,
	OSX_IsPresent,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	OSX_Init,
	OSX_Exit,
	NULL,
	VC_SetNumVoices,
	OSX_PlayStart,
	OSX_PlayStop,
	OSX_Update,
	NULL,
	VC_VoiceSetVolume,
	VC_VoiceGetVolume,
	VC_VoiceSetFrequency,
	VC_VoiceGetFrequency,
	VC_VoiceSetPanning,
	VC_VoiceGetPanning,
	VC_VoicePlay,
	VC_VoiceStop,
	VC_VoiceStopped,
	VC_VoiceGetPosition,
	VC_VoiceRealVolume
};

#else

MISSING(drv_osx);

#endif /* DRV_OSX */
