/*
Inflate implementation

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

inflate.h -- Main inflate algorithm.

*/

#ifndef INFLATE_H
#define INFLATE_H

#include "input.h"

#ifdef __cplusplus
    extern "C" {
#endif

/* Unpacks data. Returns 0 on error, unpacked data length otherwise.
   error_msg is assigned to the text of error message (text "OK" on success).
 */
extern unsigned int alpng_inflate(struct input_data* data, unsigned char* unpacked_array, unsigned int unpacked_length, char** error_msg);

#ifdef __cplusplus
    }
#endif

#endif /* INFLATE_H */
