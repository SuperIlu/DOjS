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

  Driver for output to the Macintosh Sound Manager

==============================================================================*/

/*
	Written by Anders F Bjoerklund <afb@algonet.se>

	Based on free code by:
	- Antoine Rosset <RossetAntoine@bluewin.ch> (author of PlayerPRO)
	- John Stiles <stiles@emulation.net>
	- Pierre-Olivier Latour <pol@french-touch.net>

	This code uses two different ways of filling the buffers:
	- Classic code uses SndPlayDoubleBuffer callbacks
	- Carbon code uses SndCallBacks with Deferred Tasks

	Updated by Axel Wefers <awe@fruitz-of-dojo.de>:
	- changed code for compatibility with ProjectBuilder/OSX:
	- "NewSndCallBackProc()" to "NewSndCallBackUPP()".
	- "NewDeferredTaskProc()" to "NewDeferredTaskUPP()".
	- added some conditionals to avoid compiler warnings.

	Updated again in 2004 by afb, to fix some bugs:
	- deadlock in Player_Paused, when using HAVE_PTHREAD
	  (since it is now using the global "vars" MUTEX too)
	- playback was wrong speed when running under CarbonLib
	  (due to Deferred Tasks having lame latencies there)
	- proper playing of partially filled buffers too
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mikmod_internals.h"

#ifdef DRV_MAC

#if defined(__APPLE__) && defined(__MACH__)

#include <Carbon/Carbon.h>

#elif TARGET_API_MAC_CARBON && (UNIVERSAL_INTERFACES_VERSION >= 0x0335)

#include <Carbon.h>

#else

#include <Sound.h>
#include <OSUtils.h>
#include <Gestalt.h>

#endif

#ifndef TARGET_API_MAC_CARBON
#define TARGET_API_MAC_CARBON	TARGET_CARBON
#endif

#ifndef TARGET_CPU_68K
#define TARGET_CPU_68K	GENERATING68K
#endif

#if TARGET_API_MAC_CARBON
#define USE_SNDDOUBLEBUFFER		0
#else
#define USE_SNDDOUBLEBUFFER		1
#endif

#if TARGET_API_MAC_CARBON
#define USE_DEFERREDTASKS		0
#else
#define USE_DEFERREDTASKS		1
#endif

#define SOUND_BUFFER_SIZE		4096L

static SndChannelPtr			soundChannel = NULL;	/* pointer to a sound channel */

#if USE_SNDDOUBLEBUFFER
static SndDoubleBufferHeader	doubleHeader;			/* pointer to double buffers  */
#else
static SndCallBackUPP 			sndCallBack = NULL;
static ExtSoundHeader			sndHeader;		/* a sound manager bufferCmd header */

static Ptr				sndBuffer1 = NULL;
static Ptr				sndBuffer2 = NULL;
static Ptr				currentBuffer;
static long				currentFrames;

#if USE_DEFERREDTASKS
static DeferredTask			dtask;			/* deferred task record */
static volatile Boolean			deferredTaskFired = true;
static volatile Boolean			deferredTaskDone = true;
#endif

#endif /* USE_SNDDOUBLEBUFFER */

#define FILL_BUFFER(_buffer_data,_buffer_size,_bytes) \
	MUTEX_LOCK(vars); \
	if (Player_Paused_internal()) \
		_bytes=VC_SilenceBytes((SBYTE*)_buffer_data,(ULONG)_buffer_size); \
	else	_bytes=VC_WriteBytes((SBYTE*)_buffer_data,(ULONG)_buffer_size); \
	MUTEX_UNLOCK(vars);


#if USE_SNDDOUBLEBUFFER

/* DoubleBackProc, called at interrupt time */
static pascal void MyDoubleBackProc(SndChannelPtr channel,SndDoubleBufferPtr doubleBuffer)
{
#ifndef GCC
#pragma unused(channel)
#endif
	long written;
#if TARGET_CPU_68K
	long oldA5=SetA5(doubleBuffer->dbUserInfo[0]);
#endif

	FILL_BUFFER(doubleBuffer->dbSoundData, SOUND_BUFFER_SIZE, written)

	if (doubleHeader.dbhNumChannels==2) written>>=1;
	if (doubleHeader.dbhSampleSize==16) written>>=1;

	doubleBuffer->dbNumFrames=written;
	doubleBuffer->dbFlags|=dbBufferReady;

#if TARGET_CPU_68K
	SetA5(oldA5);
#endif
}

#else /* USE_SNDDOUBLEBUFFER */

#if USE_DEFERREDTASKS
/* DeferredTask, called at almost-interrupt time (not for 68K - doesn't set A5) */
static pascal void DeferredTaskCallback(long param)
{
	long written;

	deferredTaskFired = true;

	FILL_BUFFER(param, SOUND_BUFFER_SIZE, written)

	deferredTaskDone = true;
}
#endif /* USE_DEFERREDTASKS */

/* SoundCallback, called at interrupt time (not for 68K - doesn't set A5)  */
static pascal void SoundCallback(SndChannelPtr channel, SndCommand *command)
{
#ifndef GCC
#pragma unused(channel,command)
#endif
	SndCommand buffer   = { bufferCmd, 0, (long) &sndHeader };
	SndCommand callback = { callBackCmd, 0, 0 };

	/* Install current buffer */
	sndHeader.samplePtr = currentBuffer;
	sndHeader.numFrames = currentFrames;
	SndDoImmediate(soundChannel, &buffer);

#if USE_DEFERREDTASKS
	/* Setup deferred task to fill next buffer */
	if(deferredTaskFired)
	{
		currentBuffer = (currentBuffer == sndBuffer1) ? sndBuffer2 : sndBuffer1;
		if (currentBuffer != NULL)
		{
			deferredTaskFired = false;
			deferredTaskDone = false;
			dtask.dtParam = (long) currentBuffer;
			DTInstall((DeferredTaskPtr) &dtask);
		}
	}
#else
	{
		long	bytes;

		currentBuffer = (currentBuffer == sndBuffer1) ? sndBuffer2 : sndBuffer1;
		FILL_BUFFER(currentBuffer, SOUND_BUFFER_SIZE, bytes)

		if (sndHeader.numChannels == 2) bytes >>= 1;
		if (sndHeader.sampleSize == 16) bytes >>= 1;

		currentFrames = bytes;
	}
#endif /* USE_DEFERREDTASKS */

	/* Queue next callback */
	SndDoCommand(soundChannel, &callback, true);
}

#endif /* USE_SNDDOUBLEBUFFER */

static BOOL MAC_IsThere(void)
{
	NumVersion nVers;

	nVers=SndSoundManagerVersion();
	if (nVers.majorRev>=2)
		return 1; /* need SoundManager 2.0+ */
	else
		return 0;
}

static int MAC_Init(void)
{
	OSErr err,iErr;
#if USE_SNDDOUBLEBUFFER
	int i;
	SndDoubleBufferPtr doubleBuffer;
#endif
	long rate,maxrate,maxbits;
	long gestaltAnswer;
	NumVersion nVers;
	Boolean Stereo,StereoMixing,Audio16;
	Boolean NewSoundManager,NewSoundManager31;

	NewSoundManager31=NewSoundManager=false;

	nVers=SndSoundManagerVersion();
	if (nVers.majorRev>=3) {
		NewSoundManager=true;
		if (nVers.minorAndBugRev>=0x10)
			NewSoundManager31=true;
	} else
	  if (nVers.majorRev<2)
		return 1; /* failure, need SoundManager 2.0+ */

	iErr=Gestalt(gestaltSoundAttr,&gestaltAnswer);
	if (iErr==noErr) {
		Stereo=(gestaltAnswer & (1<<gestaltStereoCapability))!=0;
		StereoMixing=(gestaltAnswer & (1<<gestaltStereoMixing))!=0;
		Audio16=(gestaltAnswer & (1<<gestalt16BitSoundIO))!=0;
	} else {
		/* failure, couldn't get any sound info at all ? */
		Stereo=StereoMixing=Audio16=false;
	}

#if !TARGET_CPU_68K || !TARGET_RT_MAC_CFM
	if (NewSoundManager31) {
		iErr=GetSoundOutputInfo(0L,siSampleRate,(void*)&maxrate);
		if (iErr==noErr)
			iErr=GetSoundOutputInfo(0L,siSampleSize,(void*)&maxbits);
	}

	if (iErr!=noErr) {
#endif
		maxrate=rate22khz;

		if (NewSoundManager && Audio16)
			maxbits=16;
		else
			maxbits=8;
#if !TARGET_CPU_68K || !TARGET_RT_MAC_CFM
	}
#endif

	switch (md_mixfreq) {
		case 48000:rate=rate48khz;break;
		case 44100:rate=rate44khz;break;
		case 22254:rate=rate22khz;break;
		case 22050:rate=rate22050hz;break;
		case 11127:rate=rate11khz;break;
		case 11025:rate=rate11025hz;break;
		default:   rate=0;break;
	}

	if (!rate) {
		_mm_errno=MMERR_MAC_SPEED;
		return 1;
	}

	md_mode|=DMODE_SOFT_MUSIC|DMODE_SOFT_SNDFX;

	if ((md_mode&DMODE_16BITS)&&(maxbits<16))
		md_mode&=~DMODE_16BITS;

	if (!Stereo || !StereoMixing)
		md_mode&=~DMODE_STEREO;

	if (rate>maxrate)
		rate=maxrate;
	if (md_mixfreq>(maxrate>>16))
		md_mixfreq=maxrate>>16;

#if USE_SNDDOUBLEBUFFER
	err=SndNewChannel(&soundChannel,sampledSynth,
			  (md_mode&DMODE_STEREO)?initStereo:initMono, NULL);
	if(err!=noErr) {
		_mm_errno=MMERR_OPENING_AUDIO;
		return 1;
	}

	doubleHeader.dbhCompressionID=0;
	doubleHeader.dbhPacketSize   =0;
	doubleHeader.dbhSampleRate   =rate;
	doubleHeader.dbhSampleSize   =(md_mode&DMODE_16BITS)?16:8;
	doubleHeader.dbhNumChannels  =(md_mode&DMODE_STEREO)?2:1;
	doubleHeader.dbhDoubleBack   =NewSndDoubleBackProc(&MyDoubleBackProc);

	for(i=0;i<2;i++) {
		doubleBuffer=(SndDoubleBufferPtr)NewPtrClear(sizeof(SndDoubleBuffer)+
							     SOUND_BUFFER_SIZE);
		if(!doubleBuffer) {
			_mm_errno=MMERR_OUT_OF_MEMORY;
			return 1;
		}

		doubleBuffer->dbNumFrames=0;
		doubleBuffer->dbFlags=0;
		doubleBuffer->dbUserInfo[0]=SetCurrentA5();
		doubleBuffer->dbUserInfo[1]=0;

		doubleHeader.dbhBufferPtr[i]=doubleBuffer;
	}

#else /* USE_SNDDOUBLEBUFFER */
	if(sndCallBack == NULL)
		sndCallBack = NewSndCallBackUPP(SoundCallback); /* <AWE> was "NewSndCallBackProc()" */

	err=SndNewChannel(&soundChannel,sampledSynth,
			  (md_mode&DMODE_STEREO)?initStereo:initMono, sndCallBack);
	if(err!=noErr) {
		_mm_errno=MMERR_OPENING_AUDIO;
		return 1;
	}

	sndBuffer1 = NewPtrClear(SOUND_BUFFER_SIZE);
	sndBuffer2 = NewPtrClear(SOUND_BUFFER_SIZE);
	if (sndBuffer1 == NULL || sndBuffer2 == NULL) {
		_mm_errno=MMERR_OUT_OF_MEMORY;
		return 1;
	}
	currentBuffer = sndBuffer1;

	/* Setup sound header */
	memset(&sndHeader, 0, sizeof(sndHeader));
	sndHeader.numChannels = (md_mode&DMODE_STEREO)? 2: 1;
	sndHeader.sampleRate = rate;
	sndHeader.encode = extSH;
	sndHeader.baseFrequency = kMiddleC;
	sndHeader.numFrames = SOUND_BUFFER_SIZE >> (((md_mode&DMODE_STEREO)? 1: 0) + ((md_mode&DMODE_16BITS)?1: 0));
	sndHeader.sampleSize = (md_mode&DMODE_16BITS)? 16: 8;
	sndHeader.samplePtr = currentBuffer;

#if USE_DEFERREDTASKS
	/* Setup deferred task record */
	memset(&dtask, 0, sizeof(dtask));
	dtask.qType = dtQType;
	dtask.dtFlags = 0;
	dtask.dtAddr = NewDeferredTaskUPP(DeferredTaskCallback); /* <AWE> was "NewDeferredTaskProc()" */
	dtask.dtReserved = 0;
	deferredTaskFired = true;
#endif /* USE_DEFERREDTASKS */

#endif /* USE_SNDDOUBLEBUFFER */

	return VC_Init();
}

static void MAC_Exit(void)
{
#if USE_SNDDOUBLEBUFFER
	int i;
#else
	Ptr	temp1,temp2;
#endif

	if (soundChannel != NULL)
	{
		SndDisposeChannel(soundChannel,true); /* "true" means to flush and quiet */
		soundChannel=NULL;
	}

#if USE_SNDDOUBLEBUFFER
	DisposeRoutineDescriptor((UniversalProcPtr)doubleHeader.dbhDoubleBack);
	doubleHeader.dbhDoubleBack=NULL;

	for(i=0;i<doubleHeader.dbhNumChannels;i++) {
		DisposePtr((Ptr)doubleHeader.dbhBufferPtr[i]);
		doubleHeader.dbhBufferPtr[i]=NULL;
	}

#else /* USE_SNDDOUBLEBUFFER */
	if (sndCallBack != NULL)
	{
		DisposeSndCallBackUPP(sndCallBack);
		sndCallBack = NULL;
	}

	temp1 = sndBuffer1;
	sndBuffer1 = NULL;
	temp2 = sndBuffer2;
	sndBuffer2 = NULL;

#if USE_DEFERREDTASKS
	/* <afb> we can't dispose of the buffers until the DT is done with them */
	while (!deferredTaskDone)
		;
#endif
	DisposePtr(temp1);
	DisposePtr(temp2);
#endif /* USE_SNDDOUBLEBUFFER */

	VC_Exit();
}

static int MAC_PlayStart(void)
{
	OSErr err;

#if USE_SNDDOUBLEBUFFER
	MyDoubleBackProc(soundChannel,doubleHeader.dbhBufferPtr[0]);
	MyDoubleBackProc(soundChannel,doubleHeader.dbhBufferPtr[1]);

	err=SndPlayDoubleBuffer(soundChannel,&doubleHeader);
	if(err!=noErr) {
		_mm_errno=MMERR_MAC_START;
		return 1;
	}

#else /* USE_SNDDOUBLEBUFFER */
	SndCommand callback = { callBackCmd, 0, 0 };

	err=SndDoCommand(soundChannel, &callback, true);
	if(err!=noErr) {
		_mm_errno=MMERR_MAC_START;
		return 1;
	}
#endif /* USE_SNDDOUBLEBUFFER */

	return VC_PlayStart();
}

static void MAC_PlayStop(void)
{
	SndCommand flush = { flushCmd, 0, 0 };
	SndCommand quiet = { quietCmd, 0, 0 };

	/* <afb> IM:Sound says we should issue the flushCmd before the quietCmd. */
	SndDoImmediate(soundChannel,&flush);
	SndDoImmediate(soundChannel,&quiet);

	VC_PlayStop();
}

static void MAC_Update(void)
{
	return;
}

MIKMODAPI MDRIVER drv_mac={
    NULL,
    "Mac Driver (Carbonized)",
    "Macintosh Sound Manager Driver v2.1",
    0,255,
    "mac",
    NULL,
    NULL,
    MAC_IsThere,
    VC_SampleLoad,
    VC_SampleUnload,
    VC_SampleSpace,
    VC_SampleLength,
    MAC_Init,
    MAC_Exit,
    NULL,
    VC_SetNumVoices,
    MAC_PlayStart,
    MAC_PlayStop,
    MAC_Update,
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

#else /* ifdef DRV_MAC */

MISSING(drv_mac);

#endif

/* ex:set ts=4: */

