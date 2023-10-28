/*
MIT License

Copyright (c) 2019-2023 Andre Seidelt <superilu@yahoo.com>

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

VGM specs are at https://vgmrips.net/wiki/VGM_Specification
*/

#include "DOjS.h"
#include "socket.h"
#include "zipfile.h"
#include "util.h"
#include "vgm.h"

#define VGM_RESOLUTION 44100
#define VGM_FACTOR 44
#define VGM_TIMER_FREQ BPS_TO_TIMER(VGM_RESOLUTION / VGM_FACTOR)
#define VGM_OPL_ADDR 0x388
#define VGM_OPL_DATA 0x389

// #define VGM_DUMP

typedef struct {
    uint32_t Vgmident;
    uint32_t EoFoffset;
    uint32_t Version;
    uint32_t SN76489clock;
    uint32_t YM2413clock;
    uint32_t GD3offset;
    uint32_t Totalsamples;
    uint32_t Loopoffset;
    uint32_t Loopsamples;
    uint32_t Rate;
    uint32_t SNFBSNWSF;
    uint32_t YM2612clock;
    uint32_t YM2151clock;
    uint32_t VGMdataoffset;
    uint32_t SegaPCMclock;
    uint32_t SPCMInterface;
    uint32_t RF5C68clock;
    uint32_t YM2203clock;
    uint32_t YM2608clock;
    uint32_t YM2610Bclock;
    uint32_t YM3812clock;
    uint32_t YM3526clock;
    uint32_t Y8950clock;
    uint32_t YMF262clock;
    uint32_t YMF278Bclock;
    uint32_t YMF271clock;
    uint32_t YMZ280Bclock;
    uint32_t RF5C164clock;
    uint32_t PWMclock;
    uint32_t AY8910clock;
    uint32_t AYTAYFlags;
    uint32_t VMLBLM;
    uint32_t GBDMGclock;
    uint32_t NESAPUclock;
    uint32_t MultiPCMclock;
    uint32_t uPD7759clock;
    uint32_t OKIM6258clock;
    uint32_t OFKFCF;
    uint32_t OKIM6295clock;
    uint32_t K051649clock;
    uint32_t K054539clock;
    uint32_t HuC6280clock;
    uint32_t C140clock;
    uint32_t K053260clock;
    uint32_t Pokeyclock;
    uint32_t QSoundclock;
    uint32_t SCSPclock;
    uint32_t ExtraHdrofs;
    uint32_t WonderSwanclock;
    uint32_t VSUclock;
    uint32_t SAA1099clock;
    uint32_t ES5503clock;
    uint32_t ES5506clock;
    uint32_t ESchnsCD;
    uint32_t X1010clock;
    uint32_t C352clock;
    uint32_t GA20clock;
    uint32_t unused1;
    uint32_t unused2;
    uint32_t unused3;
    uint32_t unused4;
    uint32_t unused5;
    uint32_t unused6;
    uint32_t unused7;
} vgm_t;

static volatile uint8_t *vgm = NULL;
static volatile vgm_t *vgm_header = NULL;
static volatile uint8_t *vgm_data = NULL;
static volatile uint32_t vgm_pos = 0;
static volatile uint32_t vgm_end = 0;
static volatile int32_t vgm_wait = 0;

/**
 * Send the given byte of data to the given register of the OPL2 chip.
 */
static void vgm_opl_write(uint8_t reg, uint8_t val) {
    // Select register
    outp(VGM_OPL_ADDR, reg);

    // Wait for card to accept value
    for (int i = 1; i < 25; i++) {
        inp(VGM_OPL_ADDR);
    }

    // Send value
    outp(VGM_OPL_DATA, val);

    // Wait for card to accept value
    for (int i = 1; i < 100; i++) {
        inp(VGM_OPL_ADDR);
    }
}
END_OF_FUNCTION(vgm_opl_write);

static void vgm_int() {
    if (vgm_wait > 0) {
        vgm_wait -= VGM_FACTOR;
    } else {
        while (vgm_pos < vgm_end) {
            uint8_t cmd = vgm_data[vgm_pos];

            if (cmd >= 0x70 && cmd < 0x80) {
                // wait n+1 samples, n can range from 0 to 15.
                vgm_wait = 1 + (cmd & 0x0F);
                vgm_pos++;
            } else if (cmd == 0x5a) {
                // YM2413, write value dd to register aa
                uint8_t aa = vgm_data[vgm_pos + 1];
                uint8_t dd = vgm_data[vgm_pos + 2];
                vgm_opl_write(aa, dd);
                vgm_pos += 3;
            } else if (cmd == 0x61) {
                // Wait n samples, n can range from 0 to 65535 (approx 1.49 seconds). Longer pauses than this are represented by multiple wait commands.
                vgm_wait = *((uint16_t *)(&vgm_data[vgm_pos + 1]));
                vgm_pos += 3;
            } else if (cmd == 0x62) {
                // wait 735 samples (60th of a second), a shortcut for 0x61 0xdf 0x02
                vgm_wait = 735;
                vgm_pos++;
            } else if (cmd == 0x63) {
                // wait 882 samples (50th of a second), a shortcut for 0x61 0x72 0x03
                vgm_wait = 882;
                vgm_pos++;
            } else if (cmd == 0x66) {
                // end of sound data --> loop
                vgm_pos = 0;
                break;
            } else if (cmd == 0x67) {
                // data block: see below
                vgm_pos += 3;                                    // cmd 0x66 and type
                vgm_pos += *((uint32_t *)(&vgm_data[vgm_pos]));  // add size
                vgm_pos += 4;                                    // add 4 bytes because of size
            } else {
                // unknown cmd
            }

            // only wait when the waiting time is longer than our IRQ interval
            if (vgm_wait > VGM_FACTOR) {
                break;
            } else {
                vgm_wait = 0;
            }
        }
    }
}
END_OF_FUNCTION(vgm_int);

/**
 * detect the presence of a FM card.
 */
static bool vgm_opl_detect() {
    uint8_t A, B;

    vgm_opl_write(1, 0);
    vgm_opl_write(4, 0x60);
    vgm_opl_write(4, 0x80);
    A = inp(VGM_OPL_ADDR);
    vgm_opl_write(2, 0xFF);
    vgm_opl_write(4, 0x21);
    rest_callback(80, tick_socket);
    B = inp(VGM_OPL_ADDR);
    vgm_opl_write(4, 0x60);
    vgm_opl_write(4, 0x80);
    if ((A & 0xE0) == 0 && (B & 0xE0) == 0xC0) {
        return true;
    } else {
        return false;
    }
}

/**
 * Hard reset the OPL2 chip. This should be done before sending any register data to the chip.
 */
static void vgm_opl_reset() {
    for (int i = 0; i < 256; i++) {
        vgm_opl_write(i, 0x00);
    }
}

#ifdef VGM_DUMP
static void vgm_dump() {
    LOGF("VGM version %08X\n", vgm_header->Version);

    int pos = 0;
    while (pos < vgm_end) {
        uint8_t cmd = vgm_data[pos];

        if (cmd >= 0x70 && cmd < 0x80) {
            // wait n+1 samples, n can range from 0 to 15.
            LOGF("WAIT_7x %d\n", 1 + (cmd & 0x0F));
            pos++;
        } else if (cmd == 0x5a) {
            // YM2413, write value dd to register aa
            uint8_t aa = vgm_data[pos + 1];
            uint8_t dd = vgm_data[pos + 2];
            LOGF("WRITE 0x%02X 0x%02X\n", aa, dd);
            pos += 3;
        } else if (cmd == 0x61) {
            // Wait n samples, n can range from 0 to 65535 (approx 1.49 seconds). Longer pauses than this are represented by multiple wait commands.
            LOGF("WAIT_62 %d\n", *((uint16_t *)(&vgm_data[pos + 1])));
            pos += 3;
        } else if (cmd == 0x62) {
            // wait 735 samples (60th of a second), a shortcut for 0x61 0xdf 0x02
            LOG("WAIT_62 735\n");
            pos++;
        } else if (cmd == 0x63) {
            // wait 882 samples (50th of a second), a shortcut for 0x61 0x72 0x03
            LOG("WAIT_63 882\n");
            pos++;
        } else if (cmd == 0x66) {
            // end of sound data
            LOG("EOS\n");
            break;
        } else if (cmd == 0x67) {
            // data block: see below
            vgm_pos += 3;  // cmd 0x66 and type
            LOGF("DATA size=%ld\n", *((uint32_t *)(&vgm_data[vgm_pos])));
            vgm_pos += *((uint32_t *)(&vgm_data[vgm_pos]));  // add size
            vgm_pos += 4;                                    // add 4 bytes because of size
        } else {
            LOGF("UNKNOWN 0x%02X\n", cmd);
        }
    }
}
#endif

static void f_VgmPlay(js_State *J) {
    if (vgm_data) {
        vgm_pos = 0;
        vgm_wait = 0;

        if (install_int_ex(vgm_int, VGM_TIMER_FREQ) != 0) {
            js_error(J, "Failed to install VGM timer");
        } else {
            // LOGF("VGM playback started\n");
        }
    }
}

static void f_VgmPos(js_State *J) { js_pushnumber(J, vgm_pos); }

static void f_VgmStop(js_State *J) {
    remove_int(vgm_int);

    vgm_opl_reset();

    // LOGF("VGM playback stopped\n");
}

static void f_VgmDiscard(js_State *J) {
    f_VgmStop(J);
    if (vgm) {
        free((void *)vgm);
    }
    vgm = NULL;
    vgm_pos = 0;
    vgm_end = 0;
    vgm_wait = 0;
    vgm_header = NULL;
    vgm_data = NULL;
}

static void f_VgmLoad(js_State *J) {
    if (DOjS.midi_available) {
        js_error(J, "VGM OPL2 does only work when Allegro FM sound is disabled (-f on command line or in DOjS.INI)");
        return;
    }

    if (!vgm_opl_detect()) {
        js_error(J, "OPL2 not detected!");
        return;
    }

    f_VgmDiscard(J);

    const char *fname = js_tostring(J, 1);
    char *delim = strchr(fname, ZIP_DELIM);

    size_t size;
    if (!delim) {
        if (!ut_read_file(fname, (void **)&vgm, &size)) {
            js_error(J, "Could not read VGM '%s'", fname);
            return;
        }
    } else {
        if (!read_zipfile1(fname, (void **)&vgm, &size)) {
            js_error(J, "Could not read VGM '%s'", fname);
            return;
        }
    }

    if (memcmp((const void *)vgm, "Vgm ", 4) != 0) {
        js_error(J, "VGM header error.");
        free((void *)vgm);
        return;
    }

    vgm_header = (vgm_t *)vgm;
    if (vgm_header->EoFoffset != size - 4) {
        js_error(J, "VGM format error.");
        free((void *)vgm);
        return;
    }

    if (vgm_header->Version < 0x00000151) {
        js_error(J, "only VGM >= 1.51 is supported.");
        free((void *)vgm);
        return;
    }

    // set pointers
    vgm_data = ((uint8_t *)&vgm_header->VGMdataoffset) + vgm_header->VGMdataoffset;
    vgm_end = size - (vgm_data - vgm);

#ifdef VGM_DUMP
    LOGF("Loaded VGM %s of size %d @ 0x%08X, data @ 0x%08X, end index %d\n", fname, size, vgm, vgm_data, vgm_end);
    LOGF("cmd offset is 0x%02X\n", vgm_header->VGMdataoffset);
    vgm_dump();
#endif
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize vgm subsystem.
 *
 * @param J VM state.
 */
void init_vgm(js_State *J) {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    LOGF("VGM playback resolution is %dHz\n", (VGM_RESOLUTION / VGM_FACTOR));

    LOCK_VARIABLE(vgm_pos);
    LOCK_VARIABLE(vgm_data);
    LOCK_VARIABLE(vgm_wait);
    LOCK_FUNCTION(vgm_int);
    LOCK_FUNCTION(vgm_opl_write);

    NFUNCDEF(J, VgmPos, 0);
    NFUNCDEF(J, VgmPlay, 0);
    NFUNCDEF(J, VgmStop, 0);
    NFUNCDEF(J, VgmLoad, 1);
    NFUNCDEF(J, VgmDiscard, 0);

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}

/**
 * @brief shutdown VGM subsystem.
 */
void shutdown_vgm() {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);
    f_VgmStop(NULL);
    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}
