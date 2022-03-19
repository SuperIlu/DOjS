/*
AllegroPNG

Copyright (c) 2006 Michal Molhanec

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented;
     you must not claim that you wrote the original software.
     If you use this software in a product, an acknowledgment
     in the product documentation would be appreciated but
     is not required.

  2. Altered source versions must be plainly marked as such,
     and must not be misrepresented as being the original software.

  3. This notice may not be removed or altered from any
     source distribution.
*/

/*

alpng.h -- Main include file.

*/

#ifndef ALPNG_H
#define ALPNG_H

#ifdef __cplusplus
    extern "C" {
#endif

#if (MAKE_VERSION(4, 2, 0) > MAKE_VERSION(ALLEGRO_VERSION, \
     ALLEGRO_SUB_VERSION, ALLEGRO_WIP_VERSION))
    #error This library was not tested with lower versions of Allegro than 4.2.0!
#endif

extern char* alpng_error_msg;

/* registers PNG extension for load/save_bitmap */
void alpng_init(void);

BITMAP* load_png(AL_CONST char* filename, RGB* pal);
BITMAP* load_png_pf(PACKFILE *f, RGB *pal);
int save_png(AL_CONST char *filename, BITMAP *bmp, AL_CONST RGB *pal);
int save_png_pf(PACKFILE *f, BITMAP *bmp, AL_CONST RGB *pal);

#ifdef __cplusplus
    }
#endif

#endif  /* ALPNG_H */
