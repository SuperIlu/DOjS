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

#include "sound.h"
#include <mikmod.h>
#include <mujs.h>
#include "DOjS.h"

#define VOICES_MOD 32
#define VOICES_WAV 4

/************
** structs **
************/
//! WAV userdata definition
typedef struct __sample {
    SAMPLE *voice;  //! ready to play sound data
    SBYTE voice_num;
} sample_t;

//! MOD userdata definition
typedef struct __module {
    MODULE *module;  //! ready to play sound data
} mod_t;

/*********************
** static functions **
*********************/

/**
 * @brief finalize and free resources.
 *
 * @param J VM state.
 */
static void Module_Finalize(js_State *J, void *data) {
    mod_t *mod = (mod_t *)data;
    Player_Free(mod->module);
    free(mod);
}

/**
 * @brief load a MOD-file and store it as userdata in JS object.
 * new Module(filename:string)
 *
 * @param J VM state.
 */
static void new_Module(js_State *J) {
    const char *fname = js_tostring(J, 1);

    mod_t *mod = malloc(sizeof(mod_t));
    if (!mod) {
        js_error(J, "No memory for module '%s'", fname);
        return;
    }

    if (!(mod->module = Player_Load(fname, VOICES_MOD, 0))) {
        js_error(J, "Can't load module '%s': %s", fname, MikMod_strerror(MikMod_errno));
        free(mod);
        return;
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_MOD, mod, Module_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushstring(J, mod->module->songname);
    js_defproperty(J, -2, "songname", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushstring(J, mod->module->modtype);
    js_defproperty(J, -2, "modtype", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    if (mod->module->comment) {
        js_pushstring(J, mod->module->comment);
    } else {
        js_pushstring(J, "");
    }
    js_defproperty(J, -2, "comment", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, mod->module->numchn);
    js_defproperty(J, -2, "num_channels", JS_READONLY | JS_DONTENUM | JS_DONTCONF);
}

/**
 * @brief play back the module.
 * mod.Play()
 *
 * @param J VM state.
 */
static void Module_play(js_State *J) {
    if (sound_available) {
        mod_t *mod = js_touserdata(J, 0, TAG_MOD);
        Player_Start(mod->module);
    }
}

/**
 * @brief finalize and free resources.
 *
 * @param J VM state.
 */
static void Sample_Finalize(js_State *J, void *data) {
    sample_t *snd = (sample_t *)data;
    Sample_Free(snd->voice);
    free(snd);
}

/**
 * @brief load a WAV and store it as userdata in JS object.
 * new Sample(filename:string)
 *
 * @param J VM state.
 */
static void new_Sample(js_State *J) {
    const char *fname = js_tostring(J, 1);

    sample_t *snd = malloc(sizeof(sample_t));
    if (!snd) {
        js_error(J, "No memory for sample '%s'", fname);
        return;
    }
    snd->voice_num = -1;

    if (!(snd->voice = Sample_Load(fname))) {
        js_error(J, "Can't load sample '%s': %s", fname, MikMod_strerror(MikMod_errno));
        free(snd);
        return;
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_SAMPLE, snd, Sample_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, snd->voice->length);
    js_defproperty(J, -2, "length", JS_READONLY | JS_DONTENUM | JS_DONTCONF);

    js_pushnumber(J, snd->voice->speed);
    js_defproperty(J, -2, "speed", JS_READONLY | JS_DONTENUM | JS_DONTCONF);
}

/**
 * @brief play back the sound.
 * snd.Play()
 *
 * @param J VM state.
 */
static void Sample_play(js_State *J) {
    if (sound_available) {
        ULONG start = 0;
        if (js_isnumber(J, 1)) {
            start = js_toint32(J, 1);
        }

        sample_t *snd = js_touserdata(J, 0, TAG_SAMPLE);
        snd->voice_num = Sample_Play(snd->voice, start, 0);
    }
}

/**
 * @brief stop sound.
 * snd.Stop()
 *
 * @param J VM state.
 */
static void Sample_stop(js_State *J) {
    if (sound_available) {
        sample_t *snd = js_touserdata(J, 0, TAG_SAMPLE);
        if (snd->voice_num != -1) {
            Voice_Stop(snd->voice_num);
            snd->voice_num = -1;
        }
    }
}

/**
 * @brief check if module is still playing.
 * ModuleIsPlaying():boolean
 *
 * @param J VM state.
 */
static void mod_IsPlaying(js_State *J) {
    if (sound_available) {
        js_pushboolean(J, Player_Active());
    } else {
        js_pushboolean(J, false);
    }
}

/**
 * @brief stop all voices for module.
 *
 * @param J
 */
static void mod_Stop(js_State *J) { sound_mod_stop(); }

/***********************
** exported functions **
***********************/
/**
 * @brief stop all voices for module.
 *
 * @param J VM state.
 */
void sound_mod_stop() {
    if (sound_available) {
        for (int i = 0; i < VOICES_MOD; i++) {
            Voice_Stop(i);
        }
        Player_Stop();
    }
}

/**
 * @brief initialize module/sample subsystem.
 *
 * @param J VM state.
 */
bool init_sound(js_State *J) {
    FUNCDEF(J, mod_IsPlaying, "ModuleIsPlaying", 0);
    FUNCDEF(J, mod_Stop, "ModuleStop", 0);

    js_newobject(J);
    { PROTDEF(J, Sample_play, TAG_SAMPLE, "Play", 0); }
    { PROTDEF(J, Sample_stop, TAG_SAMPLE, "Stop", 1); }
    js_newcconstructor(J, new_Sample, new_Sample, TAG_SAMPLE, 1);
    js_defglobal(J, TAG_SAMPLE, JS_DONTENUM);

    js_newobject(J);
    { PROTDEF(J, Module_play, TAG_MOD, "Play", 0); }
    js_newcconstructor(J, new_Module, new_Module, TAG_MOD, 1);
    js_defglobal(J, TAG_MOD, JS_DONTENUM);

    /* initialize MikMod threads */
    MikMod_InitThreads();

    /* register all the drivers */
    MikMod_RegisterAllDrivers();

    /* register all the module loaders */
    MikMod_RegisterAllLoaders();

    md_mode |= DMODE_SOFT_SNDFX;
    if (MikMod_Init("")) {
        LOGF("Could not initialize sound, reason: %s\n", MikMod_strerror(MikMod_errno));
        return false;
    }

    LOGF("Compiled with MikMod Sound Library version %ld.%ld.%ld\n", LIBMIKMOD_VERSION_MAJOR, LIBMIKMOD_VERSION_MINOR, LIBMIKMOD_REVISION);
    LOGF("Using %s\n", md_driver->Version);
    DEBUGF("Available Loaders:\n%s\n", MikMod_InfoLoader());
    DEBUGF("Available Drivers:\n%s\n", MikMod_InfoDriver());

    MikMod_SetNumVoices(VOICES_MOD, VOICES_WAV);
    MikMod_EnableOutput();

    return true;
}

/**
 * @brief shutdown module/sample subsystem.
 */
void shutdown_sound() {
    Player_Stop();
    MikMod_DisableOutput();
    MikMod_Exit();
}
