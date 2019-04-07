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

  Driver for SoundBlaster/Pro/16/AWE32 under DOS

==============================================================================*/

/*

	Written by Andrew Zabolotny <bit@eltech.ru>

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DRV_SB

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include "mikmod_internals.h"

#include "dossb.h"

static void SB_CommandLine(const CHAR *cmdline)
{
	char *ptr, *end;

	if ((ptr=MD_GetAtom("port",cmdline,0)) != NULL) {
		sb.port = strtol(ptr, &end, 16);
		MikMod_free(ptr);
	}
	if ((ptr=MD_GetAtom("irq",cmdline,0)) != NULL) {
		sb.irq = strtol(ptr, &end, 10);
		MikMod_free(ptr);
	}
	if ((ptr=MD_GetAtom("dma",cmdline,0)) != NULL) {
		sb.dma8 = strtol(ptr, &end, 10);
		MikMod_free(ptr);
	}
	if ((ptr=MD_GetAtom("hidma",cmdline,0)) != NULL) {
		sb.dma16 = strtol(ptr, &end, 10);
		MikMod_free(ptr);
	}
}

static BOOL SB_IsThere(void)
{
	return sb_detect();
}

static int SB_Init(void)
{
	if (!sb_open()) {
		_mm_errno = MMERR_INVALID_DEVICE;
		return 1;
	}

	/* Adjust md_mode according to sound card capabilities */
	if (!(sb.caps & SBMODE_STEREO))
		md_mode &= ~DMODE_STEREO;
	if (!(sb.caps & SBMODE_16BITS))
		md_mode &= ~DMODE_16BITS;

	if (md_mixfreq < 4000)
		md_mixfreq = 4000;
	if (md_mode & DMODE_STEREO) {
		if (md_mixfreq > sb.maxfreq_stereo)
			md_mixfreq = sb.maxfreq_stereo;
	} else {
		if (md_mixfreq > sb.maxfreq_mono)
			md_mixfreq = sb.maxfreq_mono;
	}

	return VC_Init();
}

static void SB_Exit(void)
{
	VC_Exit();
	sb_close();
}

/* The last buffer byte filled with sound */
static unsigned int buff_tail = 0;

static void SB_Callback(void)
{
	unsigned int dma_size, dma_pos;
	ULONG (*mixer)(SBYTE *buf, ULONG todo);

	sb_query_dma(&dma_size, &dma_pos);
	/* There isn't much sense in filling less than 256 bytes */
	dma_pos &= ~255;

	/* If nothing to mix, quit */
	if (buff_tail == dma_pos)
		return;

	// if (Player_Paused_internal())
	// 	mixer = VC_SilenceBytes;
	// else
		mixer = VC_WriteBytes;	// FIX: this enables to play sample while no module is active

	/* If DMA pointer still didn't wrapped around ... */
	if (dma_pos > buff_tail) {
		buff_tail += mixer ((SBYTE *)(sb.dma_buff->linear + buff_tail), dma_pos - buff_tail);
		/* If we arrived right to the DMA buffer end, jump to the beginning */
		if (buff_tail >= dma_size)
			buff_tail = 0;
	} else {
		/* If wrapped around, fill first to the end of buffer */
		mixer ((SBYTE *)(sb.dma_buff->linear + buff_tail), dma_size - buff_tail);
		/* Now fill from buffer beginning to current DMA pointer */
		buff_tail = mixer ((SBYTE *)sb.dma_buff->linear, dma_pos);
	}
}

static void SB_Update(void)
{
	/* Do nothing: the real update is done during SB interrupts */
}

static int SB_PlayStart (void)
{
	if (VC_PlayStart())
		return 1;

	/* Enable speaker output */
	sb_output(TRUE);

	/* Set our routine to be called during SB IRQs */
	buff_tail = 0;
	sb.timer_callback = SB_Callback;

	/* Start cyclic DMA transfer */
	if (!sb_start_dma(((md_mode & DMODE_16BITS) ? SBMODE_16BITS | SBMODE_SIGNED : 0) |
		((md_mode & DMODE_STEREO) ? SBMODE_STEREO : 0), md_mixfreq))
	{
		_mm_errno = MMERR_DOSSB_STARTDMA;
		return 1;
	}

	return 0;
}

static int SB_Reset(void)
{
	sb_reset();
	VC_Exit();
	return VC_Init();
}

static void SB_PlayStop(void)
{
	sb.timer_callback = NULL;
	sb_output(FALSE);
	sb_stop_dma();
	VC_PlayStop();
}

MDRIVER drv_sb =
{
	NULL,
	"Sound Blaster",
	"Sound Blaster Orig/2.0/Pro/16 v1.0",
	0, 255,
	"sb",
	"port:c:220,230,240,250,260,270,280,32C,530,604,E80,F40,220:Sound Blaster base I/O port\n"
	"irq:c:2,3,5,7,10,5:Sound Blaster IRQ\n"
	"dma:c:0,1,3,1:Sound Blaster 8 bit DMA channel\n"
	"hidma:c:5,6,7,5:Sound Blaster 16 bit DMA channel (SB16/AWE32 only)\n",

	SB_CommandLine,
	SB_IsThere,
	VC_SampleLoad,
	VC_SampleUnload,
	VC_SampleSpace,
	VC_SampleLength,
	SB_Init,
	SB_Exit,
	SB_Reset,
	VC_SetNumVoices,
	SB_PlayStart,
	SB_PlayStop,
	SB_Update,
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

#else /* DRV_SB */

#include "mikmod_internals.h"
MISSING(drv_sb);

#endif

/* ex:set ts=4: */
