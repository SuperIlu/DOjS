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

  Routine for registering all drivers in libmikmod for the current platform.

==============================================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mikmod_internals.h"

static void _mm_registeralldrivers(void)
{
	/* Register network drivers */
#ifdef DRV_AF
	_mm_registerdriver(&drv_AF);
#endif
#ifdef DRV_PULSEAUDIO
	_mm_registerdriver(&drv_pulseaudio);
#endif
#ifdef DRV_ESD
	_mm_registerdriver(&drv_esd);
#endif
#ifdef DRV_NAS
	_mm_registerdriver(&drv_nas);
#endif

	/* Register hardware drivers - hardware mixing */
#ifdef DRV_ULTRA
	_mm_registerdriver(&drv_ultra);
#endif
#ifdef DRV_SAM9407
	_mm_registerdriver(&drv_sam9407);
#endif

	/* Register multi-platform drivers -- software mixing */
#ifdef DRV_SDL
	_mm_registerdriver(&drv_sdl);
#endif
#ifdef DRV_OPENAL
	_mm_registerdriver(&drv_openal);
#endif

	/* Register OS-specific hardware drivers - software mixing */
#ifdef DRV_AHI
	_mm_registerdriver(&drv_ahi);
#endif
#ifdef DRV_AIX
	_mm_registerdriver(&drv_aix);
#endif
#ifdef DRV_ALSA
	_mm_registerdriver(&drv_alsa);
#endif
#ifdef DRV_HP
	_mm_registerdriver(&drv_hp);
#endif
#ifdef DRV_SNDIO
	_mm_registerdriver(&drv_sndio);
#endif
#ifdef DRV_OSS
	_mm_registerdriver(&drv_oss);
#endif
#ifdef DRV_SGI
	_mm_registerdriver(&drv_sgi);
#endif
#ifdef DRV_SUN
	_mm_registerdriver(&drv_sun);
#endif
#ifdef DRV_DART
	_mm_registerdriver(&drv_dart);
#endif
#ifdef DRV_OS2
	_mm_registerdriver(&drv_os2);
#endif
#ifdef DRV_XAUDIO2
	_mm_registerdriver(&drv_xaudio2);
#endif
#ifdef DRV_DS
	_mm_registerdriver(&drv_ds);
#endif
#ifdef DRV_WIN
	_mm_registerdriver(&drv_win);
#endif
#ifdef DRV_MAC
	_mm_registerdriver(&drv_mac);
#endif
#ifdef DRV_OSX
	_mm_registerdriver(&drv_osx);
#endif
#ifdef DRV_DC
	_mm_registerdriver(&drv_dc);
#endif
#ifdef DRV_GP32
	_mm_registerdriver(&drv_gp32);
#endif
#ifdef DRV_PSP
	_mm_registerdriver(&drv_psp);
#endif
#ifdef DRV_OSLES
	_mm_registerdriver(&drv_osles);
#endif

	/* dos drivers - wss first, since some cards emulate sb */
#ifdef DRV_WSS
	_mm_registerdriver(&drv_wss);
#endif
#ifdef DRV_SB
	_mm_registerdriver(&drv_sb);
#endif

	/* Register disk writers */
#ifdef DRV_WAV
	_mm_registerdriver(&drv_wav);
#endif
#ifdef DRV_AIFF
	_mm_registerdriver(&drv_aiff);
#endif
#ifdef DRV_RAW
	_mm_registerdriver(&drv_raw);
#endif

	/* Register other drivers */
#ifdef DRV_PIPE
	_mm_registerdriver(&drv_pipe);
#endif
#if defined(DRV_STDOUT) && !defined(macintosh)
	_mm_registerdriver(&drv_stdout);
#endif

	/* Register 'nosound' driver */
	_mm_registerdriver(&drv_nos);
}

MIKMODAPI void MikMod_RegisterAllDrivers(void)
{
	MUTEX_LOCK(lists);
	_mm_registeralldrivers();
	MUTEX_UNLOCK(lists);
}

/* ex:set ts=4: */
