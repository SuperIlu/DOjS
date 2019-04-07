/*	MikMod sound library
	(c) 1998, 1999 Miodrag Vallat and others - see file AUTHORS for
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

  Driver for Windows Sound System under DOS

==============================================================================*/

/*

	Written by Andrew Zabolotny <bit@eltech.ru>

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DRV_WSS

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include "mikmod_internals.h"

#include "doswss.h"

static void WSS_CommandLine(const CHAR *cmdline)
{
	char *ptr, *end;

	if ((ptr=MD_GetAtom("port",cmdline,0)) != NULL) {
		wss.port = strtol(ptr, &end, 16);
		MikMod_free(ptr);
	}
	if ((ptr=MD_GetAtom("irq",cmdline,0)) != NULL) {
		wss.irq = strtol(ptr, &end, 10);
		MikMod_free(ptr);
	}
	if ((ptr=MD_GetAtom("dma",cmdline,0)) != NULL) {
		wss.dma = strtol(ptr, &end, 10);
		MikMod_free(ptr);
	}
}

static BOOL WSS_IsThere(void)
{
	return wss_detect();
}

static int WSS_Init(void)
{
	if (!wss_open()) {
		_mm_errno = MMERR_INVALID_DEVICE;
		return 1;
	}

	/* Adjust mixing frequency according to card capabilities */
	md_mixfreq = wss_adjust_freq(md_mixfreq);

	return VC_Init();
}

static void WSS_Exit(void)
{
	VC_Exit();
	wss_close();
}

/* The last buffer byte filled with sound */
static unsigned int buff_tail = 0;

static void WSS_Callback(void)
{
	unsigned int dma_size, dma_pos;
	ULONG (*mixer)(SBYTE *buf, ULONG todo);

	wss_query_dma(&dma_size, &dma_pos);
	/* There isn't much sense in filling less than 256 bytes */
	dma_pos &= ~255;

	/* If nothing to mix, quit */
	if (buff_tail == dma_pos)
		return;

    // if (Player_Paused_internal())
    // 	mixer = VC_SilenceBytes;
    // else
    mixer = VC_WriteBytes;  // FIX: this enables to play sample while no module is active

	/* If DMA pointer still didn't wrapped around ... */
	if (dma_pos > buff_tail) {
		buff_tail += mixer ((SBYTE *)(wss.dma_buff->linear + buff_tail), dma_pos - buff_tail);
		/* If we arrived right to the DMA buffer end, jump to the beginning */
		if (buff_tail >= dma_size)
			buff_tail = 0;
	} else {
		/* If wrapped around, fill first to the end of buffer */
		mixer ((SBYTE *)(wss.dma_buff->linear + buff_tail), dma_size - buff_tail);
		/* Now fill from buffer beginning to current DMA pointer */
		buff_tail = mixer ((SBYTE *)wss.dma_buff->linear, dma_pos);
	}
}

static void WSS_Update(void)
{
	/* Do nothing: the real update is done during SB interrupts */
}

static int WSS_PlayStart(void)
{
	if (VC_PlayStart())
		return 1;

	/* Set our routine to be called during WSS IRQs */
	buff_tail = 0;
	wss.timer_callback = WSS_Callback;

	/* Start cyclic DMA transfer */
	if (!wss_start_dma(((md_mode & DMODE_16BITS) ? WSSMODE_16BITS | WSSMODE_SIGNED : 0) |
		((md_mode & DMODE_STEREO) ? WSSMODE_STEREO : 0), md_mixfreq))
	{
		_mm_errno = MMERR_DOSWSS_STARTDMA;
		return 1;
	}

	/* Enable speaker output */
	wss_output(TRUE);

	return 0;
}

static int WSS_Reset(void)
{
	wss_reset();
	VC_Exit();
	return VC_Init();
}

static void WSS_PlayStop(void)
{
	wss.timer_callback = NULL;
	wss_output(FALSE);
	wss_stop_dma();
	VC_PlayStop();
}

MDRIVER drv_wss =
{
	NULL,
	"Windows Sound System",
	"Windows Sound System (CS423*,ESS*) v1.0",
	0, 255,
	"wss",
	"port:c:32C,530,604,E80,F40,530:Windows Sound System base I/O port\n"
	"irq:c:2,3,5,7,10,5:Windows Sound System IRQ\n"
	"dma:c:0,1,3,0:Windows Sound System DMA channel\n",

	WSS_CommandLine,
	WSS_IsThere,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	WSS_Init,
	WSS_Exit,
	WSS_Reset,
	VC_SetNumVoices,
	WSS_PlayStart,
	WSS_PlayStop,
	WSS_Update,
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

#else /* ifdef DRV_WSS */

#include "mikmod_internals.h"
MISSING(drv_wss);

#endif

/* ex:set ts=4: */
