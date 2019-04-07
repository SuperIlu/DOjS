/*  MikMod sound library
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

  libmikmod driver for audio output on SDL-supported platforms.
  Initially written by Paul Spark <sparkynz74@gmail.com>
  Rewrite/major fixes by O. Sezer <sezero@users.sourceforge.net>

  ==============================================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mikmod_internals.h"

#ifdef DRV_SDL

#include <string.h>
#if defined(SDL_FRAMEWORK) || defined(NO_SDL_CONFIG)
#include <SDL/SDL.h>
#else
#include "SDL.h"
#endif

#define NUMSAMPLES 256 /* a fair default for md_mixfreq <= 11025 Hz */
static SDL_AudioSpec spec;
static int enabled = 0;

static void SDLSoundCallback(void *userdata, Uint8 *stream, int len)
{
    if (!enabled) return;
    if (enabled < 0) {
        if (++enabled == 0)
            enabled = 1;
        return;
    }

    MUTEX_LOCK(vars);
    if (Player_Paused_internal()) {
        VC_SilenceBytes((SBYTE *) stream, (ULONG)len);
    }
    else
    {
        int got = (int) VC_WriteBytes((SBYTE *) stream, (ULONG)len);
        if (got < len) {	/* fill the rest with silence, then */
            VC_SilenceBytes((SBYTE *) &stream[got], (ULONG)(len-got));
        }
    }
    MUTEX_UNLOCK(vars);
}

static BOOL SetupSDLAudio(void)
{
    SDL_AudioSpec wanted;

    wanted.freq     = md_mixfreq;
    wanted.format   =
#if (SDL_MAJOR_VERSION >= 2)
                      (md_mode & DMODE_FLOAT) ? AUDIO_F32SYS :
#endif
                      (md_mode & DMODE_16BITS)? AUDIO_S16SYS : AUDIO_U8;
    wanted.channels = (md_mode & DMODE_STEREO)? 2 : 1;
    wanted.samples  = (md_mixfreq <= 11025)? (NUMSAMPLES    ) :
                      (md_mixfreq <= 22050)? (NUMSAMPLES * 2) :
                      (md_mixfreq <= 44100)? (NUMSAMPLES * 4) :
                                             (NUMSAMPLES * 8);
    wanted.userdata = NULL;
    wanted.callback = SDLSoundCallback;

    if (SDL_OpenAudio(&wanted, &spec) < 0) {
        _mm_errno=MMERR_OPENING_AUDIO;
        return 0;
    }

    return 1;
}

static BOOL SDLDrv_IsPresent(void)
{
    if ((SDL_WasInit(SDL_INIT_AUDIO)) == 0) {
        if (SDL_Init(SDL_INIT_AUDIO) < 0)
            return 0;
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
    return 1;
}

static int SDLDrv_Init(void)
{
#if (SDL_MAJOR_VERSION < 2)
    if (md_mode & DMODE_FLOAT) {
        _mm_errno=MMERR_NO_FLOAT32;
        return 1;
    }
#endif
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        _mm_errno=MMERR_OPENING_AUDIO;
        return 1;
    }
    if (!SetupSDLAudio()) {
        return 1;
    }

    md_mode |= DMODE_SOFT_MUSIC;
    if (VC_Init())
        return 1;

    enabled = -2; /* delay the callback 2 frames */
    return 0;
}

static void SDLDrv_Exit(void)
{
    SDL_LockAudio();
    enabled = 0;
    SDL_UnlockAudio();
    SDL_CloseAudio();
    VC_Exit();
}

static int SDLDrv_Reset(void)
{
    SDLDrv_Exit();
    return SDLDrv_Init();
}

static void SDLDrv_Update( void )
{
/* do nothing */
}

static void SDLDrv_PlayStop(void)
{
    SDL_PauseAudio(1);
    VC_PlayStop();
}

static int SDLDrv_PlayStart(void)
{
    if (VC_PlayStart())
        return 1;
    SDL_PauseAudio(0);
    return 0;
}

MIKMODAPI struct MDRIVER drv_sdl =
{
    NULL,
    "SDL",
    "SDL Driver v1.3",
    0,255,
    "sdl",
    NULL,
    NULL,
    SDLDrv_IsPresent,
    VC_SampleLoad,
    VC_SampleUnload,
    VC_SampleSpace,
    VC_SampleLength,
    SDLDrv_Init,
    SDLDrv_Exit,
    SDLDrv_Reset,
    VC_SetNumVoices,
    SDLDrv_PlayStart,
    SDLDrv_PlayStop,
    SDLDrv_Update,
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

MISSING(drv_sdl);

#endif

