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

alpng_common.c -- Anything which doesn't fit anywhere else.

*/

#include <allegro.h>

#include "alpng.h"
#include "alpng_internal.h"

char* alpng_error_msg = "No error.";

unsigned char ALPNG_PNG_HEADER[] = { 137, 80, 78, 71, 13, 10, 26, 10 };
int ALPNG_PNG_HEADER_LEN = sizeof(ALPNG_PNG_HEADER);

static void* load_from_datafile(PACKFILE *f, long size) {
    (void) size;
    return load_png_pf(f, 0);
}

void alpng_init(void) {
    register_bitmap_file_type("png", load_png, save_png, load_png_pf);
    register_datafile_object(DAT_ID('P','N','G',' '), load_from_datafile, (void (*)(void*))destroy_bitmap);
}
