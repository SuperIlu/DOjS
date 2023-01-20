/*
taken from https://github.com/woltapp/blurhash

MIT License

Copyright (c) 2018 Wolt Enterprises

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#ifndef __BLURHASH_DECODE_H__
#define __BLURHASH_DECODE_H__

#include <stdint.h>
#include <stdbool.h>

/*
        decodeToArray : Decodes the blurhash and copies the pixels to pixelArray,
                                        This method is suggested if you use an external memory allocator for pixelArray.
                                        pixelArray should be of size : width * height * nChannels
        Parameters :
                blurhash : A string representing the blurhash to be decoded.
                width : Width of the resulting image
                height : Height of the resulting image
                punch : The factor to improve the contrast, default = 1
                nChannels : Number of channels in the resulting image array, 3 = RGB, 4 = RGBA
                pixelArray : Pointer to memory region where pixels needs to be copied.
        Returns : int, -1 if error 0 if successful
*/
int decodeToArray(const char* blurhash, int width, int height, int punch, int nChannels, uint8_t* pixelArray);

/*
        isValidBlurhash : Checks if the Blurhash is valid or not.
        Parameters :
                blurhash : A string representing the blurhash
        Returns : bool (true if it is a valid blurhash, else false)
*/
bool isValidBlurhash(const char* blurhash);

#endif
