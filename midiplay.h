/*
** Licensed under Attribution 4.0 International (CC BY 4.0)
** https://creativecommons.org/licenses/by/4.0/
**
** Code was taken from http://www.shdon.com/dos/sound
** by Steven Don.
** This is a derived/modified version by Andre Seidelt <superilu@yahoo.com>
*/
#ifndef __MIDIPLAY_H__
#define __MIDIPLAY_H__

#include <mujs.h>
#include <stdbool.h>

/************
** defines **
************/
#define TAG_MIDI "Midi"  //!< class name for Midi()

/*********************
** static functions **
*********************/
extern bool init_midi(js_State *J);
extern void shutdown_midi(void);

#endif  // __MIDIPLAY_H__
