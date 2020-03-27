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

#include "joystick.h"
#include <mujs.h>
#include "DOjS.h"

#include <allegro.h>

/*********************
** static functions **
*********************/
/**
 * @brief save calibration data.
 * JoystickSaveData(file:string)
 *
 * @param J VM state.
 */
static void joy_save_data(js_State *J) {
    if (DOjS.joystick_available) {
        const char *fname = js_tostring(J, 1);
        if (save_joystick_data(fname) != 0) {
            js_error(J, "Can't save joystick data '%s'", allegro_error);
        }
    }
}

/**
 * @brief load calibration data.
 * JoystickLoadData(file:string)
 *
 * @param J VM state.
 */
static void joy_load_data(js_State *J) {
    if (DOjS.joystick_available) {
        const char *fname = js_tostring(J, 1);
        if (load_joystick_data(fname) != 0) {
            js_error(J, "Can't load joystick data '%s'", allegro_error);
        }
    }
}

/**
 * @brief push joystick flags as properties into current object.
 *
 * @param J VM state.
 * @param flags the flags
 */
static void joy_push_flags(js_State *J, int flags) {
    js_pushboolean(J, flags & JOYFLAG_DIGITAL);
    js_setproperty(J, -2, "digital");
    js_pushboolean(J, flags & JOYFLAG_ANALOGUE);
    js_setproperty(J, -2, "analogue");
    js_pushboolean(J, flags & JOYFLAG_CALIB_DIGITAL);
    js_setproperty(J, -2, "calib_digital");
    js_pushboolean(J, flags & JOYFLAG_CALIB_ANALOGUE);
    js_setproperty(J, -2, "calib_analogue");
    js_pushboolean(J, flags & JOYFLAG_CALIBRATE);
    js_setproperty(J, -2, "calibrate");
    js_pushboolean(J, flags & JOYFLAG_SIGNED);
    js_setproperty(J, -2, "signed");
    js_pushboolean(J, flags & JOYFLAG_UNSIGNED);
    js_setproperty(J, -2, "unsigned");
}

/**
 * @brief JoystickPoll(idx:number):joy_object
 *
 * @param J VM state.
 */
static void joy_poll(js_State *J) {
    if (DOjS.joystick_available) {
        int num = js_touint32(J, 1);
        if (num > num_joysticks) {
            js_error(J, "Joystick number out of range (%d > %d).", num, num_joysticks);
            return;
        }
        poll_joystick();

        js_newobject(J);
        {
            joy_push_flags(J, joy[num].flags);

            js_newarray(J);
            for (int j = 0; j < joy[num].num_buttons; j++) {
                js_newobject(J);
                {
                    js_pushstring(J, joy[num].button[j].name);
                    js_setproperty(J, -2, "name");
                    js_pushboolean(J, joy[num].button[j].b);
                    js_setproperty(J, -2, "state");
                }
                js_setindex(J, -2, j);
            }
            js_setproperty(J, -2, "buttons");

            js_newarray(J);
            for (int j = 0; j < joy[num].num_sticks; j++) {
                js_newobject(J);
                {
                    joy_push_flags(J, joy[num].stick[j].flags);

                    js_pushstring(J, joy[num].stick[j].name);
                    js_setproperty(J, -2, "name");
                    js_newarray(J);
                    for (int i = 0; i < joy[num].stick[j].num_axis; i++) {
                        js_newobject(J);
                        {
                            js_pushnumber(J, joy[num].stick[j].axis[i].pos);
                            js_setproperty(J, -2, "pos");
                            js_pushnumber(J, joy[num].stick[j].axis[i].d1);
                            js_setproperty(J, -2, "d1");
                            js_pushnumber(J, joy[num].stick[j].axis[i].d2);
                            js_setproperty(J, -2, "d2");
                            js_pushstring(J, joy[num].stick[j].axis[i].name);
                            js_setproperty(J, -2, "name");
                        }
                        js_setindex(J, -2, i);
                    }
                    js_setproperty(J, -2, "axis");
                }
                js_setindex(J, -2, j);
            }
            js_setproperty(J, -2, "sticks");
        }
    }
}

/**
 * @brief Pass the number of the joystick you want to calibrate as the parameter.
 * Return value: Returns a text description for the next type of calibration that will be done on the specified joystick, or NULL if no more calibration is required.
 * JoystickCalibrateName(i:number):string
 *
 * @param J VM state.
 */
static void joy_calibrate_name(js_State *J) {
    if (DOjS.joystick_available) {
        int num = js_touint32(J, 1);
        if (num > num_joysticks) {
            js_error(J, "Joystick number out of range (%d > %d).", num, num_joysticks);
            return;
        }
        const char *str = calibrate_joystick_name(num);
        if (str) {
            js_pushstring(J, str);
        } else {
            js_pushnull(J);
        }
    }
}

/**
 * @brief This function performs the next operation in the calibration series for the specified stick, assuming that the joystick has been positioned in the manner described by a
 * previous call to calibrate_joystick_name().
 *
 * @param J VM state.
 */
static void joy_calibrate(js_State *J) {
    if (DOjS.joystick_available) {
        int num = js_touint32(J, 1);
        if (num > num_joysticks) {
            js_error(J, "Joystick number out of range (%d > %d).", num, num_joysticks);
            return;
        }
        if (calibrate_joystick(num) != 0) {
            js_error(J, "Can't calibrate joystick '%s'", allegro_error);
        }
    }
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize joystick subsystem.
 *
 * @param J VM state.
 */
void init_joystick(js_State *J) {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    FUNCDEF(J, joy_save_data, "JoystickSaveData", 1);
    FUNCDEF(J, joy_load_data, "JoystickLoadData", 1);
    FUNCDEF(J, joy_poll, "JoystickPoll", 1);
    FUNCDEF(J, joy_calibrate_name, "JoystickCalibrateName", 1);
    FUNCDEF(J, joy_calibrate, "JoystickCalibrate", 1);

    DOjS.joystick_available = install_joystick(JOY_TYPE_AUTODETECT) == 0;
    if (!DOjS.joystick_available) {
        LOGF("No joystick found: %s\n", allegro_error);
    }
    PROPDEF_B(J, DOjS.joystick_available, "JOYSTICK_AVAILABLE");
    PROPDEF_N(J, num_joysticks, "NUM_JOYSTICKS");

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}

/**
 * @brief shutdown joystick subsystem.
 */
void shutdown_joystick() {
    if (DOjS.joystick_available) {
        remove_joystick();
    }
}
