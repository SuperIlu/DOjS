/*
MIT License

Copyright (c) 2019-2025 Andre Seidelt <superilu@yahoo.com>

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
#include <mujs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "DOjS.h"
#include "util.h"

#include "libcRSID.h"

void init_crsid(js_State* J);

/************
** defines **
************/
#define TAG_CRSID "cRSID"  //!< pointer tag

#define DEFAULT_SUBTUNE 1  // 1..
#define DEFAULT_SAMPLERATE (44100 / 2)
#define DEFAULT_BUFFERSIZE (4096 * 4)

/************
** structs **
************/
//! file userdata definition
typedef struct __vorbis {
    cRSID_SIDheader* SIDheader;
    cRSID_C64instance* C64;
    uint8_t* siddata;
    size_t sidsize;
    AUDIOSTREAM* stream;
} crsid_t;

/*********************
** static functions **
*********************/

/**
 * @brief free ressources.
 *
 * @param ov the crsid_t with the ressources to free.
 */
static void CRSID_cleanup(crsid_t* ov) {
    if (ov->C64) {
        stop_audio_stream(ov->stream);
        ov->stream = NULL;

        free(ov->siddata);
        ov->siddata = NULL;

        cRSID_free(ov->C64);
        ov->C64 = NULL;
    }
}

/**
 * @brief finalize a file and free resources.
 *
 * @param J VM state.
 */
static void CRSID_Finalize(js_State* J, void* data) {
    crsid_t* ov = (crsid_t*)data;
    CRSID_cleanup(ov);
    free(ov);
}

/**
 * @brief open a SID.
 * s = new cRSID(filename:str, subtune:int)
 *
 * @param J VM state.
 */
static void new_cRSID(js_State* J) {
    NEW_OBJECT_PREP(J);

    crsid_t* ov = calloc(1, sizeof(crsid_t));
    if (!ov) {
        JS_ENOMEM(J);
        return;
    }
    memset(ov, 0, sizeof(crsid_t));

    ov->C64 = cRSID_init(DEFAULT_SAMPLERATE, DEFAULT_BUFFERSIZE);
    if (ov->C64 == NULL) {
        free(ov);
        js_error(J, "Init error");
        return;
    }

    const char* fname = js_tostring(J, 1);
    if (!ut_read_file(fname, (void**)&ov->siddata, &ov->sidsize)) {
        cRSID_free(ov->C64);
        free(ov);
        js_error(J, "Could not open '%s'", fname);
        return;
    }
    ov->SIDheader = cRSID_processSIDfile(ov->C64, ov->siddata, ov->sidsize);
    if (!ov->SIDheader) {
        free(ov->siddata);
        cRSID_free(ov->C64);
        free(ov);
        js_error(J, "'%s' is no SID", fname);
        return;
    }

    int SubTune = DEFAULT_SUBTUNE;
    if (js_isnumber(J, 2)) {
        SubTune = js_toint16(J, 2);
        if (SubTune < DEFAULT_SUBTUNE) {
            SubTune = DEFAULT_SUBTUNE;
        }
    }
    cRSID_initSIDtune(ov->C64, ov->SIDheader, SubTune);

    // allocate stream
    ov->stream = play_audio_stream(DEFAULT_BUFFERSIZE, 16, false, DEFAULT_SAMPLERATE, 255, 128);
    if (!ov->stream) {
        cRSID_free(ov->C64);
        free(ov);
        JS_ENOMEM(J);
        return;
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_CRSID, ov, CRSID_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTCONF);

    js_pushstring(J, ov->SIDheader->Author);
    js_defproperty(J, -2, "author", JS_READONLY | JS_DONTCONF);

    js_pushstring(J, ov->SIDheader->Title);
    js_defproperty(J, -2, "title", JS_READONLY | JS_DONTCONF);

    js_pushstring(J, ov->SIDheader->ReleaseInfo);
    js_defproperty(J, -2, "info", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, ov->C64->LoadAddress);
    js_defproperty(J, -2, "load_address", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, ov->C64->EndAddress);
    js_defproperty(J, -2, "end_address", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, ov->C64->EndAddress - ov->C64->LoadAddress);
    js_defproperty(J, -2, "size", JS_READONLY | JS_DONTCONF);

    js_pushnumber(J, ov->C64->InitAddress);
    js_defproperty(J, -2, "init_address", JS_READONLY | JS_DONTCONF);
}

/**
 * @brief close CRSID.
 * s.Close()
 *
 * @param J VM state.
 */
static void CRSID_Close(js_State* J) {
    crsid_t* ov = js_touserdata(J, 0, TAG_CRSID);
    CRSID_cleanup(ov);
}

/**
 * @brief play stream
 * s.Play()
 *
 * @param J VM state.
 */
static void CRSID_Play(js_State* J) {
    crsid_t* ov = js_touserdata(J, 0, TAG_CRSID);
    if (!ov->C64) {
        js_error(J, "SID is closed");
        return;
    }

    while (true) {
        short* mem_chunk = get_audio_stream_buffer(ov->stream);
        if (mem_chunk != NULL) {
            for (int i = 0; i < DEFAULT_BUFFERSIZE; i++) {
                mem_chunk[i] = cRSID_generateSample(ov->C64) ^ 0x8000;
            }

            free_audio_stream_buffer(ov->stream);
        } else {
            break;
        }
    }
}

/*********************
** public functions **
*********************/
/**
 * @brief initialize neural subsystem.
 *
 * @param J VM state.
 */
void init_crsid(js_State* J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

    js_newobject(J);
    {
        NPROTDEF(J, CRSID, Close, 0);
        NPROTDEF(J, CRSID, Play, 0);
    }
    CTORDEF(J, new_cRSID, TAG_CRSID, 2);
}
