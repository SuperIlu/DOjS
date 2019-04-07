/*	MikMod sound library
	(c) 1998-2005 Miodrag Vallat and others - see file AUTHORS for
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

  Driver for output on win32 platforms using DirectSound

==============================================================================*/

/*

	Written by Brian McKinney <Brian.McKinney@colorado.edu>

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mikmod_internals.h"

#ifdef DRV_DS

#include <memory.h>
#include <string.h>

#define INITGUID
#if !defined(__cplusplus) && !defined(CINTERFACE)
#define CINTERFACE
#endif
#include <dsound.h>

#ifdef __WATCOMC__
/* If you encounter build failures from Open Watcom's dsound.h,
 * see: https://github.com/open-watcom/open-watcom-v2/pull/313
 */
/* Open Watcom has broken __cdecl (leading underscore) name mangling for Windows
 * internal var names. It is fixed in Open Watcom V2 fork as of May/2014:
 * https://github.com/open-watcom/open-watcom-v2/commit/961ef1ff756f3ec5a7248cefcae00a6ecaa97ff4
 * Therefore, we define and use a local copy of IID_IDirectSoundNotify here.
 */
#include <guiddef.h>
DEFINE_GUID(IID_IDirectSoundNotify,0xB0210783,0x89cd,0x11d0,0xAF,0x08,0x00,0xA0,0xC9,0x25,0xCD,0x16);
#endif

/* PF_XMMI64_INSTRUCTIONS_AVAILABLE not in all SDKs */
#ifndef PF_XMMI64_INSTRUCTIONS_AVAILABLE
#define PF_XMMI64_INSTRUCTIONS_AVAILABLE 10
#endif

/* DSBCAPS_CTRLALL is not defined anymore with DirectX 7. Of course DirectSound
   is a coherent, backwards compatible API... */
#ifndef DSBCAPS_CTRLALL
#define DSBCAPS_CTRLALL ( DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_CTRLVOLUME | \
						  DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY | \
						  DSBCAPS_CTRL3D )
#endif

#ifndef WAVE_FORMAT_IEEE_FLOAT
#define WAVE_FORMAT_IEEE_FLOAT 0x0003
#endif

/* size of each buffer */
#define FRAGSIZE 16
/* buffer count */
#define UPDATES  2

static LPDIRECTSOUND pSoundCard = NULL;
static LPDIRECTSOUNDBUFFER pPrimarySoundBuffer = NULL, pSoundBuffer = NULL;
static LPDIRECTSOUNDNOTIFY pSoundBufferNotify = NULL;

static HANDLE notifyUpdateHandle = NULL, updateBufferHandle = NULL;
static BOOL threadInUse = FALSE;
static int fragsize=1<<FRAGSIZE;
static DWORD controlflags = DSBCAPS_CTRLALL & ~DSBCAPS_GLOBALFOCUS;

static DWORD WINAPI updateBufferProc(LPVOID lpParameter)
{
	LPVOID pBlock1 = NULL, pBlock2 = NULL;
	DWORD soundBufferCurrentPosition, blockBytes1, blockBytes2;
	DWORD start;

	while (threadInUse) {
		if (WaitForSingleObject(notifyUpdateHandle,INFINITE)==WAIT_OBJECT_0) {

			if (!threadInUse) break;

			IDirectSoundBuffer_GetCurrentPosition
						(pSoundBuffer,&soundBufferCurrentPosition,NULL);

			if (soundBufferCurrentPosition < fragsize)
				start = fragsize;
			else
				start = 0;

			if (IDirectSoundBuffer_Lock
						(pSoundBuffer,start,fragsize,&pBlock1,&blockBytes1,
						 &pBlock2,&blockBytes2,0)==DSERR_BUFFERLOST) {
				IDirectSoundBuffer_Restore(pSoundBuffer);
				IDirectSoundBuffer_Lock
						(pSoundBuffer,start,fragsize,&pBlock1,&blockBytes1,
						 &pBlock2,&blockBytes2,0);
			}

			MUTEX_LOCK(vars);
			if (Player_Paused_internal()) {
				VC_SilenceBytes((SBYTE*)pBlock1,(ULONG)blockBytes1);
				if (pBlock2)
					VC_SilenceBytes((SBYTE*)pBlock2,(ULONG)blockBytes2);
			} else {
				VC_WriteBytes((SBYTE*)pBlock1,(ULONG)blockBytes1);
				if (pBlock2)
					VC_WriteBytes((SBYTE*)pBlock2,(ULONG)blockBytes2);
			}
			MUTEX_UNLOCK(vars);

			IDirectSoundBuffer_Unlock
						(pSoundBuffer,pBlock1,blockBytes1,pBlock2,blockBytes2);
		}
	}
	return 0;
}

static void DS_CommandLine(const CHAR *cmdline)
{
	CHAR *ptr=MD_GetAtom("buffer",cmdline,0);

	if (ptr) {
		int buf=atoi(ptr);

		if ((buf<12)||(buf>19)) buf=FRAGSIZE;
		fragsize=1<<buf;

		MikMod_free(ptr);
	}

	ptr=MD_GetAtom("globalfocus",cmdline,1);
	if (ptr) {
		controlflags |= DSBCAPS_GLOBALFOCUS;
		MikMod_free(ptr);
	} else
		controlflags &= ~DSBCAPS_GLOBALFOCUS;
}

static BOOL DS_IsPresent(void)
{
	if(DirectSoundCreate(NULL,&pSoundCard,NULL)!=DS_OK)
		return 0;
	if (pSoundCard) {
		IDirectSound_Release(pSoundCard);
		pSoundCard = NULL;
	}
	return 1;
}

static int DS_Init(void)
{
	DSBUFFERDESC soundBufferFormat;
	WAVEFORMATEX pcmwf;
	DSBPOSITIONNOTIFY positionNotifications[2];
	DWORD updateBufferThreadID;
	LPVOID p = NULL;

	if (DirectSoundCreate(NULL,&pSoundCard,NULL)!=DS_OK) {
		_mm_errno=MMERR_OPENING_AUDIO;
		return 1;
	}

	if (IDirectSound_SetCooperativeLevel
				(pSoundCard,GetForegroundWindow(),DSSCL_PRIORITY)!=DS_OK) {
		_mm_errno=MMERR_DS_PRIORITY;
		return 1;
	}

	memset(&soundBufferFormat,0,sizeof(DSBUFFERDESC));
	soundBufferFormat.dwSize = sizeof(DSBUFFERDESC);
	soundBufferFormat.dwFlags = DSBCAPS_PRIMARYBUFFER;
	soundBufferFormat.dwBufferBytes = 0;
	soundBufferFormat.lpwfxFormat = NULL;

	if (IDirectSound_CreateSoundBuffer
				(pSoundCard,&soundBufferFormat,&pPrimarySoundBuffer,NULL)!=DS_OK) {
		_mm_errno=MMERR_DS_BUFFER;
		return 1;
	}

	memset(&pcmwf,0,sizeof(WAVEFORMATEX));
	pcmwf.wFormatTag     =(md_mode&DMODE_FLOAT)? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM;
	pcmwf.nChannels      =(md_mode&DMODE_STEREO)?2:1;
	pcmwf.nSamplesPerSec =md_mixfreq;
	pcmwf.wBitsPerSample =(md_mode&DMODE_FLOAT)?32:(md_mode&DMODE_16BITS)?16:8;
	pcmwf.nBlockAlign    =(pcmwf.wBitsPerSample * pcmwf.nChannels) / 8;
	pcmwf.nAvgBytesPerSec=pcmwf.nSamplesPerSec*pcmwf.nBlockAlign;

	if (IDirectSoundBuffer_SetFormat(pPrimarySoundBuffer,&pcmwf)!=DS_OK) {
		_mm_errno=MMERR_DS_FORMAT;
		return 1;
	}
	IDirectSoundBuffer_Play(pPrimarySoundBuffer,0,0,DSBPLAY_LOOPING);

	memset(&soundBufferFormat,0,sizeof(DSBUFFERDESC));
	soundBufferFormat.dwSize	=sizeof(DSBUFFERDESC);
	soundBufferFormat.dwFlags	=controlflags|DSBCAPS_GETCURRENTPOSITION2 ;
	soundBufferFormat.dwBufferBytes =fragsize*UPDATES;
	soundBufferFormat.lpwfxFormat	=&pcmwf;

	if (IDirectSound_CreateSoundBuffer
				(pSoundCard,&soundBufferFormat,&pSoundBuffer,NULL)!=DS_OK) {
		_mm_errno=MMERR_DS_BUFFER;
		return 1;
	}

#ifdef __cplusplus
	IDirectSoundBuffer_QueryInterface(pSoundBuffer, IID_IDirectSoundNotify,&p);
#else
	IDirectSoundBuffer_QueryInterface(pSoundBuffer,&IID_IDirectSoundNotify,&p);
#endif
	if (!p) {
		_mm_errno=MMERR_DS_NOTIFY;
		return 1;
	}
	pSoundBufferNotify = (LPDIRECTSOUNDNOTIFY) p;

	notifyUpdateHandle=CreateEvent
				(NULL,FALSE,FALSE,TEXT("libmikmod DirectSound Driver positionNotify Event"));
	if (!notifyUpdateHandle) {
		_mm_errno=MMERR_DS_EVENT;
		return 1;
	}

	updateBufferHandle=CreateThread
				(NULL,0,updateBufferProc,NULL,CREATE_SUSPENDED,&updateBufferThreadID);
	if (!updateBufferHandle) {
		_mm_errno=MMERR_DS_THREAD;
		return 1;
	}

	memset(positionNotifications,0,2*sizeof(DSBPOSITIONNOTIFY));
	positionNotifications[0].dwOffset    =0;
	positionNotifications[0].hEventNotify=notifyUpdateHandle;
	positionNotifications[1].dwOffset    =fragsize;
	positionNotifications[1].hEventNotify=notifyUpdateHandle;
	if (IDirectSoundNotify_SetNotificationPositions
				(pSoundBufferNotify,2,positionNotifications) != DS_OK) {
		_mm_errno=MMERR_DS_UPDATE;
		return 1;
	}

#if defined HAVE_SSE2
	/* this test only works on Windows XP or later */
	if (IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE)) {
		md_mode|=DMODE_SIMDMIXER;
	}
#endif
	return VC_Init();
}

static void DS_Exit(void)
{
	DWORD statusInfo;

	if(updateBufferHandle) {
		/* Signal thread to exit and wait for the exit */
		if (threadInUse) {
			threadInUse = 0;
			MUTEX_UNLOCK(vars);
			SetEvent (notifyUpdateHandle);
			WaitForSingleObject (updateBufferHandle, INFINITE);
			MUTEX_LOCK(vars);
		}

		CloseHandle(updateBufferHandle),
		updateBufferHandle = NULL;
	}
	if (notifyUpdateHandle) {
		CloseHandle(notifyUpdateHandle),
		notifyUpdateHandle = NULL;
	}

	if (pSoundBufferNotify) {
		IDirectSoundNotify_Release(pSoundBufferNotify);
		pSoundBufferNotify = NULL;
	}
	if(pSoundBuffer) {
		if(IDirectSoundBuffer_GetStatus(pSoundBuffer,&statusInfo)==DS_OK)
			if(statusInfo&DSBSTATUS_PLAYING)
				IDirectSoundBuffer_Stop(pSoundBuffer);
		IDirectSoundBuffer_Release(pSoundBuffer);
		pSoundBuffer = NULL;
	}

	if(pPrimarySoundBuffer) {
		if(IDirectSoundBuffer_GetStatus(pPrimarySoundBuffer,&statusInfo)==DS_OK)
			if(statusInfo&DSBSTATUS_PLAYING)
				IDirectSoundBuffer_Stop(pPrimarySoundBuffer);
		IDirectSoundBuffer_Release(pPrimarySoundBuffer);
		pPrimarySoundBuffer = NULL;
	}

	if (pSoundCard) {
		IDirectSound_Release(pSoundCard);
		pSoundCard = NULL;
	}

	VC_Exit();
}

static BOOL do_update = 0;

static void DS_Update(void)
{
	LPVOID block;
	DWORD bBytes;

	/* Do first update in DS_Update() to be consistent with other
	   non threaded drivers. */
	if (do_update && pSoundBuffer) {
		do_update = 0;

		if (IDirectSoundBuffer_Lock(pSoundBuffer, 0, fragsize, &block, &bBytes, NULL, NULL, 0)
											== DSERR_BUFFERLOST) {
			IDirectSoundBuffer_Restore (pSoundBuffer);
			IDirectSoundBuffer_Lock (pSoundBuffer, 0, fragsize, &block, &bBytes, NULL, NULL, 0);
		}

		if (Player_Paused_internal()) {
			VC_SilenceBytes ((SBYTE *)block, (ULONG)bBytes);
		} else {
			VC_WriteBytes ((SBYTE *)block, (ULONG)bBytes);
		}

		IDirectSoundBuffer_Unlock (pSoundBuffer, block, bBytes, NULL, 0);

		IDirectSoundBuffer_SetCurrentPosition(pSoundBuffer, 0);
		IDirectSoundBuffer_Play(pSoundBuffer, 0, 0, DSBPLAY_LOOPING);

		threadInUse=1;
		ResumeThread (updateBufferHandle);
	}
}

static void DS_PlayStop(void)
{
	do_update = 0;
	if (pSoundBuffer)
		IDirectSoundBuffer_Stop(pSoundBuffer);
	VC_PlayStop();
}

static int DS_PlayStart(void)
{
	do_update = 1;
	return VC_PlayStart();
}

MIKMODAPI MDRIVER drv_ds=
{
	NULL,
	"DirectSound",
	"DirectSound Driver (DX6+) v0.6",
	0,255,
	"ds",
	"buffer:r:12,19,16:Audio buffer log2 size\n"
		"globalfocus:b:0:Play if window does not have the focus\n",
	DS_CommandLine,
	DS_IsPresent,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	DS_Init,
	DS_Exit,
	NULL,
	VC_SetNumVoices,
	DS_PlayStart,
	DS_PlayStop,
	DS_Update,
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

MISSING(drv_ds);

#endif

/* ex:set ts=4: */
