/*
** Licensed under Attribution 4.0 International (CC BY 4.0)
** https://creativecommons.org/licenses/by/4.0/
**
** Code was taken from http://www.shdon.com/dos/sound
** by Steven Don.
** This is a derived/modified version by Andre Seidelt <superilu@yahoo.com>
*/

#ifndef __FMMUSIC_H__
#define __FMMUSIC_H__

#include "DOjS.h"
#include <mujs.h>

#define JSINC_FMMUSIC                                                          \
  BOOT_DIR "fmmusic.js" //!< boot script for fm music subsystem

extern bool init_fmmusic(js_State *J);
extern void shutdown_fmmusic();

#endif // __FMMUSIC_H__
