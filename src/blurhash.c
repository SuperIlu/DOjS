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
#define _USE_MATH_DEFINES
#include <math.h>

#include <stdlib.h>
#include <string.h>

#include "blurhash.h"

extern float fmaxf(float x, float y);
extern float fminf(float x, float y);

static char chars[83] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz#$%*+,-.:;=?@[]^_{|}~";

static inline uint8_t clampToUByte(int *src) {
    if (*src >= 0 && *src <= 255) return *src;
    return (*src < 0) ? 0 : 255;
}

static inline uint8_t *createByteArray(int size) { return (uint8_t *)malloc(size * sizeof(uint8_t)); }

static inline int linearTosRGB(float value) {
    float v = fmaxf(0, fminf(1, value));
    if (v <= 0.0031308)
        return v * 12.92 * 255 + 0.5;
    else
        return (1.055 * powf(v, 1 / 2.4) - 0.055) * 255 + 0.5;
}

static inline float sRGBToLinear(int value) {
    float v = (float)value / 255;
    if (v <= 0.04045)
        return v / 12.92;
    else
        return powf((v + 0.055) / 1.055, 2.4);
}

static inline float signPow(float value, float exp) { return copysignf(powf(fabsf(value), exp), value); }

static int decodeToInt(const char *string, int start, int end) {
    int value = 0, iter1 = 0, iter2 = 0;
    for (iter1 = start; iter1 < end; iter1++) {
        int index = -1;
        for (iter2 = 0; iter2 < 83; iter2++) {
            if (chars[iter2] == string[iter1]) {
                index = iter2;
                break;
            }
        }
        if (index == -1) return -1;
        value = value * 83 + index;
    }
    return value;
}

static void decodeDC(int value, float *r, float *g, float *b) {
    *r = sRGBToLinear(value >> 16);         // R-component
    *g = sRGBToLinear((value >> 8) & 255);  // G-Component
    *b = sRGBToLinear(value & 255);         // B-Component
}

static void decodeAC(int value, float maximumValue, float *r, float *g, float *b) {
    int quantR = (int)floorf(value / (19 * 19));
    int quantG = (int)floorf(value / 19) % 19;
    int quantB = (int)value % 19;

    *r = signPow(((float)quantR - 9) / 9, 2.0) * maximumValue;
    *g = signPow(((float)quantG - 9) / 9, 2.0) * maximumValue;
    *b = signPow(((float)quantB - 9) / 9, 2.0) * maximumValue;
}

bool isValidBlurhash(const char *blurhash) {
    const int hashLength = strlen(blurhash);

    if (!blurhash || strlen(blurhash) < 6) return false;

    int sizeFlag = decodeToInt(blurhash, 0, 1);  // Get size from first character
    int numY = (int)floorf(sizeFlag / 9) + 1;
    int numX = (sizeFlag % 9) + 1;

    if (hashLength != 4 + 2 * numX * numY) return false;
    return true;
}

int decodeToArray(const char *blurhash, int width, int height, int punch, int nChannels, uint8_t *pixelArray) {
    if (!isValidBlurhash(blurhash)) return -1;
    if (punch < 1) punch = 1;

    int sizeFlag = decodeToInt(blurhash, 0, 1);
    int numY = (int)floorf(sizeFlag / 9) + 1;
    int numX = (sizeFlag % 9) + 1;
    int iter = 0;

    float r = 0, g = 0, b = 0;
    int quantizedMaxValue = decodeToInt(blurhash, 1, 2);
    if (quantizedMaxValue == -1) return -1;

    float maxValue = ((float)(quantizedMaxValue + 1)) / 166;

    int colors_size = numX * numY;
    float colors[colors_size][3];

    for (iter = 0; iter < colors_size; iter++) {
        if (iter == 0) {
            int value = decodeToInt(blurhash, 2, 6);
            if (value == -1) return -1;
            decodeDC(value, &r, &g, &b);
            colors[iter][0] = r;
            colors[iter][1] = g;
            colors[iter][2] = b;

        } else {
            int value = decodeToInt(blurhash, 4 + iter * 2, 6 + iter * 2);
            if (value == -1) return -1;
            decodeAC(value, maxValue * punch, &r, &g, &b);
            colors[iter][0] = r;
            colors[iter][1] = g;
            colors[iter][2] = b;
        }
    }

    int bytesPerRow = width * nChannels;
    int x = 0, y = 0, i = 0, j = 0;
    int intR = 0, intG = 0, intB = 0;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            float r = 0, g = 0, b = 0;

            for (j = 0; j < numY; j++) {
                for (i = 0; i < numX; i++) {
                    float basics = cos((M_PI * x * i) / width) * cos((M_PI * y * j) / height);
                    int idx = i + j * numX;
                    r += colors[idx][0] * basics;
                    g += colors[idx][1] * basics;
                    b += colors[idx][2] * basics;
                }
            }

            intR = linearTosRGB(r);
            intG = linearTosRGB(g);
            intB = linearTosRGB(b);

            pixelArray[nChannels * x + 0 + y * bytesPerRow] = clampToUByte(&intR);
            pixelArray[nChannels * x + 1 + y * bytesPerRow] = clampToUByte(&intG);
            pixelArray[nChannels * x + 2 + y * bytesPerRow] = clampToUByte(&intB);

            if (nChannels == 4) pixelArray[nChannels * x + 3 + y * bytesPerRow] = 255;  // If nChannels=4, treat each pixel as RGBA instead of RGB
        }
    }

    return 0;
}
