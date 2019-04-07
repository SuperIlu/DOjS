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

  Driver for output on win32 platforms using XAudio2

==============================================================================*/

/*
	Originally written by 'honza.c' <honzac@users.sourceforge.net>
	Fixes, C-only conversion and float support by O.Sezer
					<sezero@users.sourceforge.net>
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DRV_XAUDIO2

#ifndef _WIN32_WINNT
/* 0x0501 for WinXP, 0x0602 for Win8 */
#define _WIN32_WINNT 0x0501
#endif
#if defined(DRV_XAUDIO28) && (_WIN32_WINNT < 0x0602)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0602
#endif
#include "mikmod_internals.h"

#define INITGUID
#include <xaudio2.h>

#if defined(_MSC_VER)
#ifdef DRV_XAUDIO28
#pragma comment(lib,"xaudio2.lib")
#endif
#pragma comment(lib,"ole32.lib")
#endif

/* PF_XMMI64_INSTRUCTIONS_AVAILABLE not in all SDKs */
#ifndef PF_XMMI64_INSTRUCTIONS_AVAILABLE
#define PF_XMMI64_INSTRUCTIONS_AVAILABLE 10
#endif

#ifndef WAVE_FORMAT_IEEE_FLOAT
#define WAVE_FORMAT_IEEE_FLOAT 0x0003
#endif


#define XAUDIO2_NUM_BUFFERS	4
#define XAUDIO2_BUFFER_SIZE	32768

#if defined(MIKMOD_DEBUG)
# define dbgprintf			fprintf
#elif defined (__GNUC__) && !(defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
# define dbgprintf(f, fmt, args...)	do {} while (0)
#else
# define dbgprintf(f, ...)		do {} while (0)
#endif

#ifndef __cplusplus
/* doing things C-only .. */
static HANDLE hBufferEvent;

static void STDMETHODCALLTYPE cb_OnVoiceProcessPassStart(IXAudio2VoiceCallback* p,
							 UINT32 SamplesRequired) {
	dbgprintf(stderr, "\n>XAudio2: OnVoiceProcessingPassStart<\n");
}
static void STDMETHODCALLTYPE cb_OnVoiceProcessPassEnd(IXAudio2VoiceCallback* p) {
	dbgprintf(stderr, "\n>XAudio2: OnVoiceProcessingPassEnd<\n");
}
static void STDMETHODCALLTYPE cb_OnStreamEnd(IXAudio2VoiceCallback* p) {
	dbgprintf(stderr, "\n>XAudio2: OnStreamEnd<\n");
}
static void STDMETHODCALLTYPE cb_OnBufferStart(IXAudio2VoiceCallback* p,
						 void* pBufferContext) {
	dbgprintf(stderr, "\n>XAudio2: OnBufferStart<\n");
}
static void STDMETHODCALLTYPE cb_OnBufferEnd(IXAudio2VoiceCallback* p,
						 void* pBufferContext) {
	SetEvent(hBufferEvent);
	dbgprintf(stderr, "\n>XAudio2: OnBufferEnd<\n");
}
static void STDMETHODCALLTYPE cb_OnLoopEnd(IXAudio2VoiceCallback* p,
						 void* pBufferContext) {
	dbgprintf(stderr, "\n>XAudio2: OnLoopEnd<\n");
}
static void STDMETHODCALLTYPE cb_OnVoiceError(IXAudio2VoiceCallback* p,
						 void* pBufferContext,
						 HRESULT Error) {
	dbgprintf(stderr, "\n>XAudio2: OnVoiceError: %ld <\n", Error);
}
static IXAudio2VoiceCallbackVtbl cbVoice_vtbl = {
	cb_OnVoiceProcessPassStart,
	cb_OnVoiceProcessPassEnd,
	cb_OnStreamEnd,
	cb_OnBufferStart,
	cb_OnBufferEnd,
	cb_OnLoopEnd,
	cb_OnVoiceError
};
static IXAudio2VoiceCallback cbVoice = {
	&cbVoice_vtbl
};
#define pcbVoice		&cbVoice

#else /* __cplusplus: */
class XAudio2VoiceCallback: public IXAudio2VoiceCallback {
public:
	HANDLE hBufferEvent;
public:
	XAudio2VoiceCallback(): hBufferEvent(CreateEvent(NULL, FALSE, FALSE, TEXT("libmikmod XAudio2 Driver buffer Event"))) { }
	virtual ~XAudio2VoiceCallback() { CloseHandle(hBufferEvent); }
	STDMETHOD_(void, cb_OnVoiceProcessPassStart)(UINT32 SamplesRequired) {
		dbgprintf(stderr, "\n>XAudio2: OnVoiceProcessingPassStart<\n");
	}
	STDMETHOD_(void, cb_OnVoiceProcessPassEnd)() {
		dbgprintf(stderr, "\n>XAudio2: OnVoiceProcessingPassEnd<\n");
	}
	STDMETHOD_(void, cb_OnStreamEnd)() {
		dbgprintf(stderr, "\n>XAudio2: OnStreamEnd<\n");
	}
	STDMETHOD_(void, cb_OnBufferStart)(void* pBufferContext) {
		dbgprintf(stderr, "\n>XAudio2: OnBufferStart<\n");
	}
	STDMETHOD_(void, cb_OnBufferEnd)(void* pBufferContext) {
		SetEvent(hBufferEvent);
		dbgprintf(stderr, "\n>XAudio2: OnBufferStart<\n");
	}
	STDMETHOD_(void, cb_OnLoopEnd)(void* pBufferContext) {
		dbgprintf(stderr, "\n>XAudio2: OnLoopEnd<\n");
	}
	STDMETHOD_(void, cb_OnVoiceError)(void* pBufferContext, HRESULT Error) {
		dbgprintf(stderr, "\n>XAudio2: OnVoiceError: %ld <\n", Error);
	}
};
static XAudio2VoiceCallback *pcbVoice = NULL;
#define hBufferEvent	pcbVoice->hBufferEvent
#define IXAudio2_Release(p)				((p)->Release())
#ifndef DRV_XAUDIO28
#define IXAudio2_CreateMasteringVoice(p,a,b,c,d,e,f)	((p)->CreateMasteringVoice(a,b,c,d,e,f))
#define IXAudio2SourceVoice_GetState(p,a)		((p)->GetState(a))
#else
#define IXAudio2_CreateMasteringVoice(p,a,b,c,d,e,f,g)	((p)->CreateMasteringVoice(a,b,c,d,e,f,g))
#define IXAudio2SourceVoice_GetState(p,a,b)		((p)->GetState(a,b))
#endif
#define IXAudio2SourceVoice_SubmitSourceBuffer(p,a,b)	((p)->SubmitSourceBuffer(a,b))
#define IXAudio2_CreateSourceVoice(p,a,b,c,d,e,f,g)	((p)->CreateSourceVoice(a,b,c,d,e,f,g))
#define IXAudio2SourceVoice_DestroyVoice(p)		((p)->DestroyVoice())
#define IXAudio2MasteringVoice_DestroyVoice(p)		((p)->DestroyVoice())
#define IXAudio2SourceVoice_Start(p,a,b)		((p)->Start(a,b))
#define IXAudio2SourceVoice_Stop( p,a,b)		((p)->Stop(a,b))
#endif /* __cplusplus */

static IXAudio2* pXAudio2 = NULL;
static IXAudio2MasteringVoice* pMasterVoice = NULL;
static IXAudio2SourceVoice* pSourceVoice = NULL;
static HANDLE UpdateBufferHandle = NULL;
static BOOL threadInUse = FALSE;
static BYTE buffers[XAUDIO2_NUM_BUFFERS][XAUDIO2_BUFFER_SIZE];
static DWORD current_buf = 0;


static DWORD WINAPI UpdateBufferProc(LPVOID lpParameter) {
	while (threadInUse) {
		while (1) {
			XAUDIO2_VOICE_STATE state;
			XAUDIO2_BUFFER audio_buf;

			#ifndef DRV_XAUDIO28
			IXAudio2SourceVoice_GetState(pSourceVoice, &state);
			#else
			IXAudio2SourceVoice_GetState(pSourceVoice, &state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
			#endif
			if (state.BuffersQueued >= XAUDIO2_NUM_BUFFERS - 1)
				break;
			MUTEX_LOCK(vars);
			if (Player_Paused_internal())
				VC_SilenceBytes((SBYTE *) buffers[current_buf], (ULONG) XAUDIO2_BUFFER_SIZE);
			else
				VC_WriteBytes((SBYTE *) buffers[current_buf], (ULONG) XAUDIO2_BUFFER_SIZE);
			MUTEX_UNLOCK(vars);
			memset(&audio_buf, 0, sizeof(XAUDIO2_BUFFER));
			audio_buf.AudioBytes = XAUDIO2_BUFFER_SIZE;
			audio_buf.pAudioData = buffers[current_buf];
			IXAudio2SourceVoice_SubmitSourceBuffer(pSourceVoice, &audio_buf, NULL);
			current_buf++;
			current_buf %= XAUDIO2_NUM_BUFFERS;
		}
		WaitForSingleObject(hBufferEvent, INFINITE);
	}
	return 0;
}

static void XAudio2_CommandLine(const CHAR *cmdline) {
/* no options */
}

static BOOL XAudio2_IsPresent(void) {
	HRESULT r;

	if (pXAudio2 == NULL) {
#ifndef _XBOX
		CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif
		r = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
		if (pXAudio2) {
			IXAudio2_Release(pXAudio2);
			pXAudio2 = NULL;
		}
#ifndef _XBOX
		CoUninitialize();
#endif
		if (FAILED(r))
			return 0;
	}
	return 1;
}

static int XAudio2_Init(void) {
	UINT32 flags;
	DWORD thread_id;
	WAVEFORMATEX wfmt;

	memset(&wfmt, 0, sizeof(WAVEFORMATEX));
	wfmt.wFormatTag= (md_mode & DMODE_FLOAT)? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM;
	wfmt.nChannels = (md_mode & DMODE_STEREO)? 2: 1;
	wfmt.nSamplesPerSec = md_mixfreq;
	wfmt.wBitsPerSample = (md_mode & DMODE_FLOAT)? 32: (md_mode & DMODE_16BITS)? 16: 8;
	wfmt.nBlockAlign = (wfmt.wBitsPerSample * wfmt.nChannels) / 8;
	wfmt.nAvgBytesPerSec = wfmt.nSamplesPerSec * wfmt.nBlockAlign;
	if (wfmt.nSamplesPerSec < XAUDIO2_MIN_SAMPLE_RATE ||
	    wfmt.nSamplesPerSec > XAUDIO2_MAX_SAMPLE_RATE ||
	    wfmt.nChannels > XAUDIO2_MAX_AUDIO_CHANNELS) {
		return 1;
	}

	current_buf = 0;
	flags = 0;
#if defined(_DEBUG) && !defined(DRV_XAUDIO28)
/*	flags |= XAUDIO2_DEBUG_ENGINE;*/
#endif
#ifndef _XBOX
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif
	if (FAILED(XAudio2Create(&pXAudio2, flags, XAUDIO2_DEFAULT_PROCESSOR))) {
		goto fail;
	}
#ifdef DRV_XAUDIO28
	if (FAILED(IXAudio2_CreateMasteringVoice(pXAudio2, &pMasterVoice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE,
						 0, NULL, NULL, AudioCategory_Other))) {
		goto fail;
	}
#else
	if (FAILED(IXAudio2_CreateMasteringVoice(pXAudio2, &pMasterVoice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, NULL))) {
		goto fail;
	}
#endif
	if (FAILED(IXAudio2_CreateSourceVoice(pXAudio2, &pSourceVoice, &wfmt, 0, 1.0f, pcbVoice, NULL, NULL))) {
		goto fail;
	}
#ifndef __cplusplus
	if ((hBufferEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("libmikmod XAudio2 Driver buffer Event"))) == NULL) {
		goto fail;
	}
#endif
	if ((UpdateBufferHandle = CreateThread(NULL, 0, UpdateBufferProc, NULL, CREATE_SUSPENDED, &thread_id)) == NULL) {
		goto fail;
	}
#if defined HAVE_SSE2
	/* this test only works on Windows XP or later */
	if (IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE)) {
		md_mode|=DMODE_SIMDMIXER;
	}
#endif
	return VC_Init();

fail:
	if (pSourceVoice) {
		IXAudio2SourceVoice_DestroyVoice(pSourceVoice);
		pSourceVoice = NULL;
	}
	if (pMasterVoice) {
		IXAudio2MasteringVoice_DestroyVoice(pMasterVoice);
		pMasterVoice = NULL;
	}
	if (pXAudio2) {
		IXAudio2_Release(pXAudio2);
		pXAudio2 = NULL;
	}
#ifndef _XBOX
	CoUninitialize();
#endif
	return 1;
}

static void XAudio2_Exit(void) {
	if (UpdateBufferHandle != NULL) {
		/* signal thread to exit and wait for the exit */
		if (threadInUse) {
			threadInUse = 0;
			MUTEX_UNLOCK(vars);
			SetEvent(hBufferEvent);
			WaitForSingleObject(UpdateBufferHandle, INFINITE);
			MUTEX_LOCK(vars);
		}
		CloseHandle(UpdateBufferHandle);
		UpdateBufferHandle = NULL;
	}
	IXAudio2SourceVoice_Stop(pSourceVoice, 0, 0);
	if (pSourceVoice) {
		IXAudio2SourceVoice_DestroyVoice(pSourceVoice);
		pSourceVoice = NULL;
	}
	if (pMasterVoice) {
		IXAudio2MasteringVoice_DestroyVoice(pMasterVoice);
		pMasterVoice = NULL;
	}
	if (pXAudio2) {
		IXAudio2_Release(pXAudio2);
		pXAudio2 = NULL;
	}
#ifndef __cplusplus
	if (hBufferEvent != NULL) {
		CloseHandle(hBufferEvent);
		hBufferEvent = NULL;
	}
#endif
#ifndef _XBOX
	CoUninitialize();
#endif
	VC_Exit();
}

static BOOL do_update = 0;

static void XAudio2_Update(void) {
	if (do_update && pSourceVoice) {
		do_update = 0;

		while (1) {
			XAUDIO2_VOICE_STATE state;
			XAUDIO2_BUFFER audio_buf;

			#ifndef DRV_XAUDIO28
			IXAudio2SourceVoice_GetState(pSourceVoice, &state);
			#else
			IXAudio2SourceVoice_GetState(pSourceVoice, &state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
			#endif
			if (state.BuffersQueued > 0)
				break;
			current_buf %= XAUDIO2_NUM_BUFFERS;
			if (Player_Paused_internal())
				VC_SilenceBytes((SBYTE *) buffers[current_buf], (ULONG) XAUDIO2_BUFFER_SIZE);
			else
				VC_WriteBytes((SBYTE *) buffers[current_buf], (ULONG) XAUDIO2_BUFFER_SIZE);
			memset(&audio_buf, 0, sizeof(XAUDIO2_BUFFER));
			audio_buf.AudioBytes = XAUDIO2_BUFFER_SIZE;
			audio_buf.pAudioData = buffers[current_buf];
			IXAudio2SourceVoice_SubmitSourceBuffer(pSourceVoice, &audio_buf, NULL);
			current_buf++;
			current_buf %= XAUDIO2_NUM_BUFFERS;
		}
		IXAudio2SourceVoice_Start(pSourceVoice, 0, 0);
		threadInUse = 1;
		ResumeThread(UpdateBufferHandle);
	}
}

static void XAudio2_PlayStop(void) {
	do_update = 0;
	if (pSourceVoice)
		IXAudio2SourceVoice_Stop(pSourceVoice, 0, 0);
	VC_PlayStop();
}

static int XAudio2_PlayStart(void) {
	do_update = 1;
	return VC_PlayStart();
}

MIKMODAPI MDRIVER drv_xaudio2 = {
	NULL,
	"XAudio2",
	"DirectX XAudio2 Driver v0.3",
	0,255,
	"xaudio2",
	"",
	XAudio2_CommandLine,
	XAudio2_IsPresent,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	XAudio2_Init,
	XAudio2_Exit,
	NULL,
	VC_SetNumVoices,
	XAudio2_PlayStart,
	XAudio2_PlayStop,
	XAudio2_Update,
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

#include "mikmod_internals.h"
MISSING(drv_xaudio2);

#endif

/* ex:set ts=8: */
