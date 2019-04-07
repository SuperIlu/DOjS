/* MikMod sound library (c) 1998-2014 Miodrag Vallat and others -
 * see AUTHORS file for a complete list.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

/* Driver for output to native Amiga AHI device:
 * Written by Szilárd Biró <col.lawrence@gmail.com>, loosely based
 * on an old AOS4 version by Fredrik Wikstrom <fredrik@a500.org>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mikmod_internals.h"

#ifdef DRV_AHI

#ifdef __amigaos4__
#define __USE_INLINE__
#endif

#include <devices/ahi.h>
#include <proto/exec.h>

#define BUFFERSIZE (4 << 10)

static struct MsgPort *AHImp = NULL;
static struct AHIRequest *AHIReq[2] = { NULL, NULL };
static int active = 0;
static SBYTE *AHIBuf[2] = { NULL, NULL };
static BOOL signed8 = 0;

static void closeLibs(void) { /* close ahi */
	if (AHIReq[1]) {
		AHIReq[0]->ahir_Link = NULL; /* in case we are linked to req[0] */
		if (!CheckIO((struct IORequest *) AHIReq[1])) {
			AbortIO((struct IORequest *) AHIReq[1]);
			WaitIO((struct IORequest *) AHIReq[1]);
		}
		FreeVec(AHIReq[1]);
		AHIReq[1] = NULL;
	}

	if (AHIReq[0])
	{
		if (!CheckIO((struct IORequest *) AHIReq[0])) {
			AbortIO((struct IORequest *) AHIReq[0]);
			WaitIO((struct IORequest *) AHIReq[0]);
		}
		if (AHIReq[0]->ahir_Std.io_Device) {
			CloseDevice((struct IORequest *) AHIReq[0]);
			AHIReq[0]->ahir_Std.io_Device = NULL;
		}
		DeleteIORequest((struct IORequest *) AHIReq[0]);
		AHIReq[0] = NULL;
	}

	if (AHImp) {
		DeleteMsgPort(AHImp);
		AHImp = NULL;
	}

	if (AHIBuf[0]) {
		FreeVec(AHIBuf[0]);
		AHIBuf[0] = NULL;
	}

	if (AHIBuf[1]) {
		FreeVec(AHIBuf[1]);
		AHIBuf[1] = NULL;
	}
}

static BOOL AHI_IsThere(void) {
	return 1;
}

static int AHI_Init(void) {
	AHImp = CreateMsgPort();
	if (AHImp) {
		AHIReq[0] = (struct AHIRequest *)CreateIORequest(AHImp, sizeof(struct AHIRequest));
		if (AHIReq[0]) {
			AHIReq[0]->ahir_Version = 4;
			AHIReq[1] = AllocVec(sizeof(struct AHIRequest), MEMF_PUBLIC);
			if (AHIReq[1]) {
				if (!OpenDevice(AHINAME, AHI_DEFAULT_UNIT, (struct IORequest *)AHIReq[0], 0)) {
					/*AHIReq[0]->ahir_Std.io_Message.mn_Node.ln_Pri = 0;*/
					AHIReq[0]->ahir_Std.io_Command = CMD_WRITE;
					AHIReq[0]->ahir_Std.io_Data = NULL;
					AHIReq[0]->ahir_Std.io_Offset = 0;
					AHIReq[0]->ahir_Frequency = md_mixfreq;
					AHIReq[0]->ahir_Type = (md_mode & DMODE_16BITS)?
								((md_mode & DMODE_STEREO)? AHIST_S16S : AHIST_M16S) :
								((md_mode & DMODE_STEREO)? AHIST_S8S  : AHIST_M8S );
					AHIReq[0]->ahir_Volume = 0x10000;
					AHIReq[0]->ahir_Position = 0x8000;

					CopyMem(AHIReq[0], AHIReq[1], sizeof(struct AHIRequest));

					AHIBuf[0] = AllocVec(BUFFERSIZE, MEMF_PUBLIC | MEMF_CLEAR);
					if (AHIBuf[0]) {
						AHIBuf[1] = AllocVec(BUFFERSIZE, MEMF_PUBLIC | MEMF_CLEAR);
						if (AHIBuf[1]) {
							signed8 = (md_mode & DMODE_16BITS)? 0 : 1;
							return VC_Init();
						}
					}
				}
			}
		}
	}

	closeLibs();
	_mm_errno = MMERR_OPENING_AUDIO;
	return 1;
}

static void AHI_Exit(void) {
	VC_Exit();
	closeLibs();
}

static void AHI_Update(void) {
	ULONG numBytes;

	if (AHIReq[active]->ahir_Std.io_Data) {
		WaitIO((struct IORequest *) AHIReq[active]);
		AHIReq[active]->ahir_Std.io_Data = NULL;
	}

	numBytes = VC_WriteBytes(AHIBuf[active], BUFFERSIZE);
	if (signed8) { /* convert u8 data to s8 */
		ULONG i = 0;
		for (; i < numBytes; ++i)
			AHIBuf[active][i] -= 128;
	}

	AHIReq[active]->ahir_Std.io_Data = AHIBuf[active];
	AHIReq[active]->ahir_Std.io_Length = numBytes;
	AHIReq[active]->ahir_Link = !CheckIO((struct IORequest *) AHIReq[active ^ 1]) ? AHIReq[active ^ 1] : NULL;

	SendIO((struct IORequest *)AHIReq[active]);

	active ^= 1;
}

MIKMODAPI MDRIVER drv_ahi = {
	NULL,
	"AHI",
	"Native AHI Amiga Output driver",
	0,255,
	"AHI",
	NULL,
	NULL,
	AHI_IsThere,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	AHI_Init,
	AHI_Exit,
	NULL,
	VC_SetNumVoices,
	VC_PlayStart,
	VC_PlayStop,
	AHI_Update,
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

MISSING(drv_ahi);

#endif

/* ex:set ts=8: */
