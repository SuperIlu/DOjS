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

input.h -- Data input handling.

*/

#ifndef INPUT_H
#define INPUT_H

#ifdef __cplusplus
    extern "C" {
#endif

/*
    Represents input data.
*/
struct input_data {
    unsigned char* data; /* array of raw data */
    unsigned long length; /* length of data */
    unsigned char current; /* current byte, for per-bit access */
    unsigned long i;  /* index in data to the next byte which will be read */
    int si; /* subindex in byte for per-bit access;
               values 0-8 where 8 mean: read next byte and reset si to 0 */
    int error; /* it is set to 1 if last reading function was unsuccessful
                  (ie. read after end the of data). you have to set it to 0
                  manually */
};

/* Reads one bit from input. Returns 0 or 1.*/
extern int input_read_bit(struct input_data* data);

/* Reads count bits from input. count of 0 returns 0 */
extern int input_read_bits(struct input_data* data, int count);

/* Read one byte from input. If the input data is in the middle of some byte,
   ignores rest of the byte. */
extern unsigned char input_read_byte(struct input_data* data);

/* Reads 0-4 bytes. For 0 returns 0. */
extern unsigned long input_read_bytes(struct input_data* data, int count);

#ifdef __cplusplus
    }
#endif

#endif /* INPUT_H */
