/*
MIT License

Copyright (c) 2015 Jamie Owen
Copyright (c) 2019-2021 Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "blender.h"

/***********
** macros **
***********/
#define bALPHA() int a = (src >> 24) & 0xFF;

#define bRGB()                    \
    int r1 = (src >> 16) & 0xFF;  \
    int g1 = (src >> 8) & 0xFF;   \
    int b1 = src & 0xFF;          \
    int r2 = (dest >> 16) & 0xFF; \
    int g2 = (dest >> 8) & 0xFF;  \
    int b2 = dest & 0xFF;

#define bRET(r, g, b) return 0xFF000000UL | ((r << 16) & 0xFF0000UL) | ((g << 8) & 0x00FF00UL) | (b & 0x0000FFUL);

#define bOPAQUE(x, y) (a >= 254) ? (x) : (((x)*a + y * (255 - a)) >> 8)

/***********************
** exported functions **
***********************/
/**
 * @brief alpha calculation function used with transparent colors.
 * see https://www.gamedev.net/forums/topic/34688-alpha-blend-formula/
 *
 * @param src the new color with the alpha information.
 * @param dest the existing color in the BITMAP.
 * @param n ignored.
 * @return unsigned long the color to put into the BITMAP.
 */
unsigned long blender_alpha(unsigned long src, unsigned long dest, unsigned long n) {
    bALPHA();
    if (a >= 254) {
        return src;  // no alpha, just return new color
    } else {
        bRGB();

        int r = ((a * (r1 - r2)) >> 8) + r2;
        int g = ((a * (g1 - g2)) >> 8) + g2;
        int b = ((a * (b1 - b2)) >> 8) + b2;

        bRET(r, g, b);
    }
}

/**
 * @brief ADD blender
 * min(base+blend,1.0)
 * see https://github.com/jamieowen/glsl-blend
 *
 * @param src the new color with the alpha information.
 * @param dest the existing color in the BITMAP.
 * @param n ignored.
 * @return unsigned long the color to put into the BITMAP.
 */
unsigned long blender_add(unsigned long src, unsigned long dest, unsigned long n) {
#define bSAT(x)    \
    if (x > 255) { \
        x = 255;   \
    }

    bALPHA();
    bRGB();

    int r = bOPAQUE(r1 + r2, r2);
    int g = bOPAQUE(g1 + g2, g2);
    int b = bOPAQUE(b1 + b2, b2);

    bSAT(r);
    bSAT(g);
    bSAT(b);

    bRET(r, g, b);
#undef bSAT
}

/**
 * @brief DARKEST blender
 * min(blend,base)
 * see https://github.com/jamieowen/glsl-blend
 *
 * @param src the new color with the alpha information.
 * @param dest the existing color in the BITMAP.
 * @param n ignored.
 * @return unsigned long the color to put into the BITMAP.
 */
unsigned long blender_darkest(unsigned long src, unsigned long dest, unsigned long n) {
    bALPHA();
    bRGB();

    int r = bOPAQUE(r1 < r2 ? r1 : r2, r2);
    int g = bOPAQUE(g1 < g2 ? g1 : g2, g2);
    int b = bOPAQUE(b1 < b2 ? b1 : b2, b2);

    bRET(r, g, b);
}

/**
 * @brief LIGHTEST lightest
 * max(blend,base)
 * see https://github.com/jamieowen/glsl-blend
 *
 * @param src the new color with the alpha information.
 * @param dest the existing color in the BITMAP.
 * @param n ignored.
 * @return unsigned long the color to put into the BITMAP.
 */
unsigned long blender_lightest(unsigned long src, unsigned long dest, unsigned long n) {
    bALPHA();
    bRGB();

    int r = bOPAQUE(r1 > r2 ? r1 : r2, r2);
    int g = bOPAQUE(g1 > g2 ? g1 : g2, g2);
    int b = bOPAQUE(b1 > b2 ? b1 : b2, b2);

    bRET(r, g, b);
#undef bMAX
}

/**
 * @brief DIFFERENCE blender
 * abs(base-blend)
 * see https://github.com/jamieowen/glsl-blend
 *
 * @param src the new color with the alpha information.
 * @param dest the existing color in the BITMAP.
 * @param n ignored.
 * @return unsigned long the color to put into the BITMAP.
 */
unsigned long blender_difference(unsigned long src, unsigned long dest, unsigned long n) {
#define bABS(x) ((x) >= 0 ? (x) : ((x) * -1))

    bALPHA();
    bRGB();

    int r = bOPAQUE(bABS(r1 - r2), r2);
    int g = bOPAQUE(bABS(g1 - g2), g2);
    int b = bOPAQUE(bABS(b1 - b2), b2);

    bRET(r, g, b);
#undef bABS
}

/**
 * @brief EXCLUSION blender
 * base+blend-2.0*base*blend
 * see https://github.com/jamieowen/glsl-blend
 *
 * @param src the new color with the alpha information.
 * @param dest the existing color in the BITMAP.
 * @param n ignored.
 * @return unsigned long the color to put into the BITMAP.
 */
unsigned long blender_exclusion(unsigned long src, unsigned long dest, unsigned long n) {
#define bSAT(x)           \
    if (x < 0) {          \
        x = 0;            \
    } else if (x > 255) { \
        x = 255;          \
    }

    bALPHA();
    bRGB();

    int r = bOPAQUE(r1 + r2 - ((2 * r1 * r2) >> 8), r2);
    int g = bOPAQUE(g1 + g2 - ((2 * g1 * g2) >> 8), g2);
    int b = bOPAQUE(b1 + b2 - ((2 * b1 * b2) >> 8), b2);

    bSAT(r);
    bSAT(g);
    bSAT(b);

    bRET(r, g, b);
#undef bSAT
}

/**
 * @brief MULTIPLY blender
 * base*blend
 * see https://github.com/jamieowen/glsl-blend
 *
 * @param src the new color with the alpha information.
 * @param dest the existing color in the BITMAP.
 * @param n ignored.
 * @return unsigned long the color to put into the BITMAP.
 */
unsigned long blender_multiply(unsigned long src, unsigned long dest, unsigned long n) {
    bALPHA();
    bRGB();

    int r = bOPAQUE((r1 * r2) >> 8, r2);
    int g = bOPAQUE((g1 * g2) >> 8, g2);
    int b = bOPAQUE((b1 * b2) >> 8, b2);

    bRET(r, g, b);
}

/**
 * @brief SCREEN blender
 * 1.0-((1.0-base)*(1.0-blend))
 * see https://github.com/jamieowen/glsl-blend
 *
 * @param src the new color with the alpha information.
 * @param dest the existing color in the BITMAP.
 * @param n ignored.
 * @return unsigned long the color to put into the BITMAP.
 */
unsigned long blender_screen(unsigned long src, unsigned long dest, unsigned long n) {
    bALPHA();
    bRGB();

    int r = bOPAQUE(255 - (((255 - r1) * (255 - r2)) >> 8), r2);
    int g = bOPAQUE(255 - (((255 - g1) * (255 - g2)) >> 8), g2);
    int b = bOPAQUE(255 - (((255 - b1) * (255 - b2)) >> 8), b2);

    bRET(r, g, b);
}

/**
 * @brief OVERLAY blender
 * base<0.5?(2.0*base*blend):(1.0-2.0*(1.0-base)*(1.0-blend))
 * see https://github.com/jamieowen/glsl-blend
 *
 * @param src the new color with the alpha information.
 * @param dest the existing color in the BITMAP.
 * @param n ignored.
 * @return unsigned long the color to put into the BITMAP.
 */
unsigned long blender_overlay(unsigned long src, unsigned long dest, unsigned long n) {
    bALPHA();
    bRGB();

    int r = bOPAQUE(r2 < 128 ? ((2 * r1 * r2) >> 8) : (255 - ((2 * (255 - r1) * (255 - r2)) >> 8)), r2);
    int g = bOPAQUE(g2 < 128 ? ((2 * g1 * g2) >> 8) : (255 - ((2 * (255 - g1) * (255 - g2)) >> 8)), g2);
    int b = bOPAQUE(b2 < 128 ? ((2 * b1 * b2) >> 8) : (255 - ((2 * (255 - b1) * (255 - b2)) >> 8)), b2);

    bRET(r, g, b);
}

/**
 * @brief HARD LIGHT blender
 * inverted overlay
 * see https://github.com/jamieowen/glsl-blend
 *
 * @param src the new color with the alpha information.
 * @param dest the existing color in the BITMAP.
 * @param n ignored.
 * @return unsigned long the color to put into the BITMAP.
 */
unsigned long blender_hardlight(unsigned long src, unsigned long dest, unsigned long n) {
    bALPHA();
    bRGB();

    int r = bOPAQUE(r1 < 128 ? ((2 * r1 * r2) >> 8) : (255 - ((2 * (255 - r1) * (255 - r2)) >> 8)), r2);
    int g = bOPAQUE(g1 < 128 ? ((2 * g1 * g2) >> 8) : (255 - ((2 * (255 - g1) * (255 - g2)) >> 8)), g2);
    int b = bOPAQUE(b1 < 128 ? ((2 * b1 * b2) >> 8) : (255 - ((2 * (255 - b1) * (255 - b2)) >> 8)), b2);

    bRET(r, g, b);
}

/**
 * @brief DOGE blender
 * (blend==1.0)?blend:min(base/(1.0-blend),1.0)
 * see https://github.com/jamieowen/glsl-blend
 *
 * @param src the new color with the alpha information.
 * @param dest the existing color in the BITMAP.
 * @param n ignored.
 * @return unsigned long the color to put into the BITMAP.
 */
unsigned long blender_doge(unsigned long src, unsigned long dest, unsigned long n) {
#define bMIN(x, y) (x < y ? (x) : (y))
    bALPHA();
    bRGB();

    int r = bOPAQUE((r1 >= 254) ? r1 : bMIN((r2 << 8) / (255 - r1), 255), r2);
    int g = bOPAQUE((g1 >= 254) ? g1 : bMIN((g2 << 8) / (255 - g1), 255), g2);
    int b = bOPAQUE((b1 >= 254) ? b1 : bMIN((b2 << 8) / (255 - b1), 255), b2);

    bRET(r, g, b);
#undef bMIN
}

/**
 * @brief BURN blender
 * (blend==0.0)?blend:max((1.0-((1.0-base)/blend)),0.0)
 * see https://github.com/jamieowen/glsl-blend
 *
 * @param src the new color with the alpha information.
 * @param dest the existing color in the BITMAP.
 * @param n ignored.
 * @return unsigned long the color to put into the BITMAP.
 */
unsigned long blender_burn(unsigned long src, unsigned long dest, unsigned long n) {
#define bMAX(x, y) (x > y ? (x) : (y))
    bALPHA();
    bRGB();

    int r = bOPAQUE((r1 == 0) ? r1 : bMAX((255 - (((255 - r2) << 8) / r1)), 0), r2);
    int g = bOPAQUE((g1 == 0) ? g1 : bMAX((255 - (((255 - g2) << 8) / g1)), 0), g2);
    int b = bOPAQUE((b1 == 0) ? b1 : bMAX((255 - (((255 - b2) << 8) / b1)), 0), b2);

    bRET(r, g, b);
#undef bMAX
}

/**
 * @brief SUBSTRACT blender
 * max(base+blend-1.0,0.0)
 * see https://github.com/jamieowen/glsl-blend
 *
 * @param src the new color with the alpha information.
 * @param dest the existing color in the BITMAP.
 * @param n ignored.
 * @return unsigned long the color to put into the BITMAP.
 */
unsigned long blender_substract(unsigned long src, unsigned long dest, unsigned long n) {
#define bSAT(x)  \
    if (x < 0) { \
        x = 0;   \
    }

    bALPHA();
    bRGB();

    int r = bOPAQUE(r1 + r2 - 255, r2);
    int g = bOPAQUE(g1 + g2 - 255, g2);
    int b = bOPAQUE(b1 + b2 - 255, b2);

    bSAT(r);
    bSAT(g);
    bSAT(b);

    bRET(r, g, b);
}
