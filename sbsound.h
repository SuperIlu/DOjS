/*
** Licensed under Attribution 4.0 International (CC BY 4.0)
** https://creativecommons.org/licenses/by/4.0/
**
** Code was taken from http://www.shdon.com/dos/sound
** by Steven Don.
** This is a derived/modified version by Andre Seidelt <superilu@yahoo.com>
*/
#ifndef __SBSOUND_H__
#define __SBSOUND_H__

#include <mujs.h>
#include <stdbool.h>

extern bool init_sbsound(js_State *J);
extern void shutdown_sbsound(void);
#endif // __SBSOUND_H__
