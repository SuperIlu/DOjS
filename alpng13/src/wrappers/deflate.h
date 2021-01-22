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

deflate.h -- Deflate function prototype.

*/

#ifndef DEFLATE_H
#define DEFLATE_H

#ifdef __cplusplus
    extern "C" {
#endif

/* 0 on error, 1 on success */
int alpng_deflate(uint8_t* data, uint32_t data_length, uint8_t** compressed_data, uint32_t* compressed_data_length, char** error_msg);

#ifdef __cplusplus
    }
#endif

#endif
