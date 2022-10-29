/*
MIT License

Copyright (c) 2019-2022 Andre Seidelt <superilu@yahoo.com>

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

#include "bytearray.h"

#include <allegro.h>
#include <mujs.h>

#include "DOjS.h"
#include "zipfile.h"

#define BA_DEFAULT_SIZE 1024
#define BA_INC_FACTOR 13
#define BA_FACTOR_SCALE 10

#define BA_UPDATE(j, n, v)       \
    {                            \
        js_pushnumber(j, v);     \
        js_setproperty(j, 0, n); \
    }

/*********************
** static functions **
*********************/
/**
 * @brief finalize
 *
 * @param J VM state.
 */
static void ByteArray_Finalize(js_State *J, void *data) {
    byte_array_t *ba = (byte_array_t *)data;
    ByteArray_destroy(ba);
}

/**
 * @brief create an ByteArray
 * ba = new ByteArray()
 * ba = new ByteArray(s:string)
 * ba = new ByteArray(ar:number[])
 *
 * @param J VM state.
 */
static void new_ByteArray(js_State *J) {
    NEW_OBJECT_PREP(J);

    byte_array_t *ba = ByteArray_create();
    if (!ba) {
        JS_ENOMEM(J);
        return;
    }

    // copy data if anything is provided
    if (js_isdefined(J, 1)) {
        if (js_isstring(J, 1)) {
            // create ByteArray from characters of a string
            const char *str = js_tostring(J, 1);
            while (*str) {
                if (ByteArray_push(ba, 0xFF & (*str)) < 0) {
                    ByteArray_destroy(ba);
                    JS_ENOMEM(J);
                    return;
                }
                str++;
            }
        } else if (js_isarray(J, 1)) {
            // create ByteArray from number[] or char[]
            int len = js_getlength(J, 1);

            for (int i = 0; i < len; i++) {
                js_getindex(J, 1, i);

                BA_TYPE val;
                if (js_isstring(J, -1)) {
                    val = js_tostring(J, -1)[0];
                } else {
                    val = js_toint32(J, -1);
                }

                int res = ByteArray_push(ba, val);
                js_pop(J, 1);
                if (res < 0) {
                    JS_ENOMEM(J);
                    return;
                }
            }
        }
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_BYTE_ARRAY, ba, ByteArray_Finalize);

    // add properties
    js_pushnumber(J, ba->alloc_size);
    js_defproperty(J, -2, "alloc_size", JS_DONTCONF);

    js_pushnumber(J, ba->size);
    js_defproperty(J, -2, "length", JS_DONTCONF);
}

/**
 * @brief get value from specific index
 * ba.Get(idx:number):number
 *
 * @param J VM state.
 */
static void ByteArray_Get(js_State *J) {
    byte_array_t *ba = js_touserdata(J, 0, TAG_BYTE_ARRAY);

    int32_t idx = js_toint32(J, 1);
    if ((idx < ba->size) && (idx >= 0)) {
        js_pushnumber(J, ba->data[idx]);
    } else {
        JS_EIDX(J, idx);
    }
}

/**
 * @brief set value at specific index
 * ba.Set(idx:number, val:number)
 *
 * @param J VM state.
 */
static void ByteArray_Set(js_State *J) {
    byte_array_t *ba = js_touserdata(J, 0, TAG_BYTE_ARRAY);

    int32_t idx = js_toint32(J, 1);
    BA_TYPE val = js_toint32(J, 2);
    if ((idx < ba->size) && (idx >= 0)) {
        ba->data[idx] = val;
    } else {
        JS_EIDX(J, idx);
    }
}

/**
 * @brief get and remove last entry
 * ba.Pop():number
 *
 * @param J VM state.
 */
static void ByteArray_Pop(js_State *J) {
    byte_array_t *ba = js_touserdata(J, 0, TAG_BYTE_ARRAY);

    if (ba->size) {
        ba->size--;
        BA_UPDATE(J, "length", ba->size);

        js_pushnumber(J, ba->data[ba->size]);
    } else {
        js_pushundefined(J);
    }
}

/**
 * @brief get and remove first entry
 * ba.Shift():number
 *
 * @param J VM state.
 */
static void ByteArray_Shift(js_State *J) {
    byte_array_t *ba = js_touserdata(J, 0, TAG_BYTE_ARRAY);

    if (ba->size) {
        ba->size--;
        BA_UPDATE(J, "length", ba->size);

        js_pushnumber(J, ba->data[0]);
        for (int i = 0; i < ba->size; i++) {
            ba->data[i] = ba->data[i + 1];
        }
    } else {
        js_pushundefined(J);
    }
}

/**
 * @brief append value to array.
 * ba.Push(val:number)
 *
 * @param J VM state.
 */
static void ByteArray_Push(js_State *J) {
    byte_array_t *ba = js_touserdata(J, 0, TAG_BYTE_ARRAY);
    BA_TYPE val = js_toint32(J, 1);

    int res = ByteArray_push(ba, val);

    if (res > 0) {
        BA_UPDATE(J, "alloc_size", ba->alloc_size);
    } else if (res < 0) {
        JS_ENOMEM(J);
        return;
    }

    BA_UPDATE(J, "length", ba->size);
}

/**
 * @brief convert to JS array.
 * ba.ToArray():number[]
 *
 * @param J VM state.
 */
static void ByteArray_ToArray(js_State *J) {
    byte_array_t *ba = js_touserdata(J, 0, TAG_BYTE_ARRAY);

    js_newarray(J);
    for (uint32_t i = 0; i < ba->size; i++) {
        js_pushnumber(J, ba->data[i]);
        js_setindex(J, -2, i);
    }
}

/**
 * @brief truncate the array to zero size
 * ba.Clear()
 *
 * @param J VM state.
 */
static void ByteArray_Clear(js_State *J) {
    byte_array_t *ba = js_touserdata(J, 0, TAG_BYTE_ARRAY);

    ba->size = 0;
    BA_UPDATE(J, "length", ba->size);
}

/**
 * @brief convert ByteArray to a string
 * ba.ToString():string
 *
 * @param J VM state.
 */
static void ByteArray_ToString(js_State *J) {
    byte_array_t *ba = js_touserdata(J, 0, TAG_BYTE_ARRAY);

    // get memory
    char *str = malloc(ba->size);
    if (!str) {
        JS_ENOMEM(J);
        return;
    }

    // copy over data
    for (int i = 0; i < ba->size; i++) {
        str[i] = 0xFF & ba->data[i];
    }

    // push string and free buffer
    js_pushlstring(J, str, ba->size);
    free(str);
}

/**
 * @brief append javascript array to ByteArray
 * ba.Append(a:number[])
 *
 * @param J VM state.
 */
static void ByteArray_Append(js_State *J) {
    byte_array_t *ba = js_touserdata(J, 0, TAG_BYTE_ARRAY);

    if (js_isstring(J, 1)) {
        const char *str = js_tostring(J, 1);
        while (*str) {
            if (ByteArray_push(ba, 0xFF & (*str)) < 0) {
                ByteArray_destroy(ba);
                JS_ENOMEM(J);
                return;
            }
            str++;
        }
    } else if (js_isarray(J, 1)) {
        int len = js_getlength(J, 1);

        for (int i = 0; i < len; i++) {
            js_getindex(J, 1, i);

            BA_TYPE val;
            if (js_isstring(J, -1)) {
                val = js_tostring(J, -1)[0];
            } else {
                val = js_toint32(J, -1);
            }

            int res = ByteArray_push(ba, val);
            js_pop(J, 1);
            if (res < 0) {
                JS_ENOMEM(J);
                return;
            }
        }
    } else {
        JS_ENOARR(J);
        return;
    }
    BA_UPDATE(J, "alloc_size", ba->alloc_size);
    BA_UPDATE(J, "length", ba->size);
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize ByteArray class
 *
 * @param J VM state.
 */
void init_bytearray(js_State *J) {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    js_newobject(J);
    {
        NPROTDEF(J, ByteArray, Get, 1);
        NPROTDEF(J, ByteArray, Set, 2);
        NPROTDEF(J, ByteArray, Push, 1);
        NPROTDEF(J, ByteArray, Pop, 0);
        NPROTDEF(J, ByteArray, Shift, 0);
        NPROTDEF(J, ByteArray, ToArray, 0);
        NPROTDEF(J, ByteArray, Clear, 0);
        NPROTDEF(J, ByteArray, ToString, 0);
        NPROTDEF(J, ByteArray, Append, 1);
    }
    CTORDEF(J, new_ByteArray, TAG_BYTE_ARRAY, 0);

    js_newobject(J);
    {
        NPROTDEF(J, ByteArray, Get, 1);
        NPROTDEF(J, ByteArray, Set, 2);
        NPROTDEF(J, ByteArray, Push, 1);
        NPROTDEF(J, ByteArray, Pop, 0);
        NPROTDEF(J, ByteArray, Shift, 0);
        NPROTDEF(J, ByteArray, ToArray, 0);
        NPROTDEF(J, ByteArray, Clear, 0);
        NPROTDEF(J, ByteArray, ToString, 0);
        NPROTDEF(J, ByteArray, Append, 1);
    }
    js_setregistry(J, TAG_BYTE_ARRAY);

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}

/**
 * @brief create an ByteArray from a byte array. The object remains on the stack.
 *
 * @param J VM state.
 * @param data the data (will be copied).
 * @param size size of the data.
 */
void ByteArray_fromBytes(js_State *J, const uint8_t *data, uint32_t size) {
    byte_array_t *ba = calloc(sizeof(byte_array_t), 1);
    if (!ba) {
        JS_ENOMEM(J);
        return;
    }
    ba->data = calloc(size, sizeof(BA_TYPE));
    if (!ba->data) {
        free(ba);
        JS_ENOMEM(J);
        return;
    }
    ba->alloc_size = size;
    ba->size = size;

    for (uint32_t i = 0; i < size; i++) {
        ba->data[i] = data[i];
    }

    js_getregistry(J, TAG_BYTE_ARRAY);
    js_newuserdata(J, TAG_BYTE_ARRAY, ba, ByteArray_Finalize);

    // add properties
    js_pushnumber(J, ba->alloc_size);
    js_defproperty(J, -2, "alloc_size", JS_DONTCONF);

    js_pushnumber(J, ba->size);
    js_defproperty(J, -2, "length", JS_DONTCONF);
}

/**
 * @brief create an ByteArray object from an existing struct. The object remains on the stack.
 *
 * @param J VM state.
 * @param ba pointer to an existing struct.
 */
void ByteArray_fromStruct(js_State *J, byte_array_t *ba) {
    js_getregistry(J, TAG_BYTE_ARRAY);
    js_newuserdata(J, TAG_BYTE_ARRAY, ba, ByteArray_Finalize);

    // add properties
    js_pushnumber(J, ba->alloc_size);
    js_defproperty(J, -2, "alloc_size", JS_DONTCONF);

    js_pushnumber(J, ba->size);
    js_defproperty(J, -2, "length", JS_DONTCONF);
}

/**
 * @brief free resources for ByteArray.
 *
 * @param ba pointer to an existing struct.
 */
void ByteArray_destroy(byte_array_t *ba) {
    if (ba) {
        if (ba->data) {
            free(ba->data);
        }
        free(ba);
    }
}

/**
 * @brief create an empty ByteArray struct.
 *
 * @return byte_array_t* a new struct or NULL for no memory.
 */
byte_array_t *ByteArray_create() {
    byte_array_t *ba = calloc(sizeof(byte_array_t), 1);
    if (!ba) {
        return NULL;
    }
    ba->data = calloc(BA_DEFAULT_SIZE, sizeof(BA_TYPE));
    if (!ba->data) {
        free(ba);
        return NULL;
    }
    ba->alloc_size = BA_DEFAULT_SIZE;
    ba->size = 0;

    return ba;
}

/**
 * @brief push a value to an ByteArray.
 *
 * @param ba pointer to an existing struct.
 * @param val the value to append.
 * @return 0 if the value was pushed, 1 if the array needed to be enlarged, -1 if out of memory.
 */
int ByteArray_push(byte_array_t *ba, BA_TYPE val) {
    int ret = 0;

    if (ba->size >= ba->alloc_size) {
        uint32_t larger_size = (ba->alloc_size * BA_INC_FACTOR) / BA_FACTOR_SCALE;
        BA_TYPE *larger = calloc(larger_size, sizeof(BA_TYPE));
        if (!larger) {
            return -1;
        }

        for (int i = 0; i < ba->size; i++) {
            larger[i] = ba->data[i];
        }

        free(ba->data);
        ba->data = larger;
        ba->alloc_size = larger_size;

        ret = 1;
    }
    ba->data[ba->size] = val;
    ba->size++;

    return ret;
}
