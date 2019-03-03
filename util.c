/*
MIT License

Copyright (c) 2019 Andre Seidelt <superilu@yahoo.com>

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

#include <errno.h>
#include <grx20.h>
#include <string.h>

#include "util.h"

/***********************
** exported functions **
***********************/
/**
 * @brief get a readable string describing the video adapter.
 */
const char *ut_getAdapterString() {
    switch (GrAdapterType()) {
        case GR_VGA:
            return ("GR_VGA");
        case GR_EGA:
            return ("GR_EGA");
        case GR_HERC:
            return ("GR_HERC");
        case GR_8514A:
            return ("GR_8514A");
        case GR_S3:
            return ("GR_S3");
        case GR_XWIN:
            return ("GR_XWIN");
        case GR_WIN32:
            return ("GR_WIN32");
        case GR_LNXFB:
            return ("GR_LNXFB");
        case GR_SDL:
            return ("GR_SDL");
        case GR_MEM:
            return ("GR_MEM");
        default:
            return ("Unknown");
    }
}

/**
 * @brief get a readable string describing the video mode.
 */
const char *ut_getModeString() {
    switch (GrCurrentMode()) {
        case GR_80_25_text:
            return ("GR_80_25_text");
        case GR_default_text:
            return ("GR_default_text");
        case GR_width_height_text:
            return ("GR_width_height_text");
        case GR_biggest_text:
            return ("GR_biggest_text");
        case GR_320_200_graphics:
            return ("GR_320_200_graphics");
        case GR_default_graphics:
            return ("GR_default_graphics");
        case GR_width_height_graphics:
            return ("GR_width_height_graphics");
        case GR_biggest_noninterlaced_graphics:
            return ("GR_biggest_noninterlaced_graphics");
        case GR_biggest_graphics:
            return ("GR_biggest_graphics");
        case GR_width_height_color_graphics:
            return ("GR_width_height_color_graphics");
        case GR_width_height_color_text:
            return ("GR_width_height_color_text");
        case GR_custom_graphics:
            return ("GR_custom_graphics");
        case GR_NC_80_25_text:
            return ("GR_NC_80_25_text");
        case GR_NC_default_text:
            return ("GR_NC_default_text");
        case GR_NC_width_height_text:
            return ("GR_NC_width_height_text");
        case GR_NC_biggest_text:
            return ("GR_NC_biggest_text");
        case GR_NC_320_200_graphics:
            return ("GR_NC_320_200_graphics");
        case GR_NC_default_graphics:
            return ("GR_NC_default_graphics");
        case GR_NC_width_height_graphics:
            return ("GR_NC_width_height_graphics");
        case GR_NC_biggest_noninterlaced_graphics:
            return ("GR_NC_biggest_noninterlaced_graphics");
        case GR_NC_biggest_graphics:
            return ("GR_NC_biggest_graphics");
        case GR_NC_width_height_color_graphics:
            return ("GR_NC_width_height_color_graphics");
        case GR_NC_width_height_color_text:
            return ("GR_NC_width_height_color_text");
        case GR_NC_custom_graphics:
            return ("GR_NC_custom_graphics");
        case GR_width_height_bpp_graphics:
            return ("GR_width_height_bpp_graphics");
        case GR_width_height_bpp_text:
            return ("GR_width_height_bpp_text");
        case GR_custom_bpp_graphics:
            return ("GR_custom_bpp_graphics");
        case GR_NC_width_height_bpp_graphics:
            return ("GR_NC_width_height_bpp_graphics");
        case GR_NC_width_height_bpp_text:
            return ("GR_NC_width_height_bpp_text");
        case GR_NC_custom_bpp_graphics:
            return ("case GR_NC_custom_bpp_graphics");
        default:
            return ("Unknown");
    }
}

/**
 * @brief check if the file exists.
 *
 * @param fname file name.
 *
 * @return check_file_t one of the enums.
 */
check_file_t ut_check_file(char *fname) {
    FILE *f = fopen(fname, "r");
    if (f) {
        fclose(f);
        return CF_YES;
    } else {
        if (errno == ENOENT) {
            return CF_NO;
        } else {
            return CF_ERROR;
        }
    }
}

/**
 * @brief check if string ends with suffix.
 *
 * @param str the string to check.
 * @param suffix the suffix to check for.
 *
 * @return true if the string ends with suffix
 * @return false if the string does not end with suffix
 */
bool ut_endsWith(const char *str, const char *suffix) {
    if (!str || !suffix) return false;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix > lenstr) return false;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

/**
 * @brief convert a GrFrameMode into a readable string.
 *
 * @param fm One of the GrFrameModes.
 *
 * @return a readable string
 */
const char *ut_getFrameModeString(GrFrameMode fm) {
    switch (fm) {
        case GR_frameUndef:
            return "GR_frameUndef";
        case GR_frameText:
            return "GR_frameText";
        case GR_frameHERC1:
            return "GR_frameHERC1";
        case GR_frameEGAVGA1:
            return "GR_frameEGAVGA1";
        case GR_frameEGA4:
            return "GR_frameEGA4";
        case GR_frameSVGA4:
            return "GR_frameSVGA4";
        case GR_frameSVGA8:
            return "GR_frameSVGA8";
        case GR_frameVGA8X:
            return "GR_frameVGA8X";
        case GR_frameSVGA16:
            return "GR_frameSVGA16";
        case GR_frameSVGA24:
            return "GR_frameSVGA24";
        case GR_frameSVGA32L:
            return "GR_frameSVGA32L";
        case GR_frameSVGA32H:
            return "GR_frameSVGA32H";
        case GR_frameSVGA8_LFB:
            return "GR_frameSVGA8_LFB";
        case GR_frameSVGA16_LFB:
            return "GR_frameSVGA16_LFB";
        case GR_frameSVGA24_LFB:
            return "GR_frameSVGA24_LFB";
        case GR_frameSVGA32L_LFB:
            return "GR_frameSVGA32L_LFB";
        case GR_frameSVGA32H_LFB:
            return "GR_frameSVGA32H_LFB";
        case GR_frameRAM1:
            return "GR_frameRAM1";
        case GR_frameRAM4:
            return "GR_frameRAM4";
        case GR_frameRAM8:
            return "GR_frameRAM8";
        case GR_frameRAM16:
            return "GR_frameRAM16";
        case GR_frameRAM24:
            return "GR_frameRAM24";
        case GR_frameRAM32L:
            return "GR_frameRAM32L";
        case GR_frameRAM32H:
            return "GR_frameRAM32H";
        case GR_frameRAM3x8:
            return "GR_frameRAM3x8";
        default:
            return "Unknown";
    }
}

/**
 * @brief dump all videomodes to logfile
 */
void ut_dumpVideoModes() {
    GrFrameMode fm;
    const GrVideoMode *mp;
    for (fm = GR_firstGraphicsFrameMode; fm <= GR_lastGraphicsFrameMode; fm++) {
        LOGF("%s:\n", ut_getFrameModeString(fm));
        mp = GrFirstVideoMode(fm);
        while (mp != NULL) {
            LOGF("  Mode %dx%d/%d (%02X)\n", mp->width, mp->height, mp->bpp, mp->mode);

            mp = GrNextVideoMode(mp);
        }
    }
}