/*
MIT License

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

#include "intarray.h"

#include <allegro.h>
#if WINDOWS==1
#include <winalleg.h>
#endif
#include <mujs.h>

#include "DOjS.h"
#include "zipfile.h"

#define IA_DEFAULT_SIZE 1024
#define IA_INC_FACTOR 13
#define IA_FACTOR_SCALE 10

#define IA_UPDATE(j, n, v)       \
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
static void IntArray_Finalize(js_State *J, void *data) {
    int_array_t *ia = (int_array_t *)data;
    IntArray_destroy(ia);
}

/**
 * @brief create an IntArray
 * ia = new IntARray()
 * ia = new IntARray(s:string)
 * ia = new IntARray(ar:number[])
 *
 * @param J VM state.
 */
static void new_IntArray(js_State *J) {
    NEW_OBJECT_PREP(J);

    int_array_t *ia = IntArray_create();
    if (!ia) {
        JS_ENOMEM(J);
        return;
    }

    // copy data if anything is provided
    if (js_isdefined(J, 1)) {
        if (js_isstring(J, 1)) {
            // create IntArray from characters of a string
            const char *str = js_tostring(J, 1);
            while (*str) {
                if (IntArray_push(ia, 0xFF & (*str)) < 0) {
                    IntArray_destroy(ia);
                    JS_ENOMEM(J);
                    return;
                }
                str++;
            }
        } else if (js_isarray(J, 1)) {
            // create IntArray from number[] or char[]
            int len = js_getlength(J, 1);

            for (int i = 0; i < len; i++) {
                js_getindex(J, 1, i);

                IA_TYPE val;
                if (js_isstring(J, -1)) {
                    val = js_tostring(J, -1)[0];
                } else {
                    val = js_toint32(J, -1);
                }

                int res = IntArray_push(ia, val);
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
    js_newuserdata(J, TAG_INT_ARRAY, ia, IntArray_Finalize);

    // add properties
    js_pushnumber(J, ia->alloc_size);
    js_defproperty(J, -2, "alloc_size", JS_DONTCONF);

    js_pushnumber(J, ia->size);
    js_defproperty(J, -2, "length", JS_DONTCONF);
}

/**
 * @brief get value from specific index
 * ia.Get(idx:number):number
 *
 * @param J VM state.
 */
static void IntArray_Get(js_State *J) {
    int_array_t *ia = js_touserdata(J, 0, TAG_INT_ARRAY);

    int32_t idx = js_toint32(J, 1);
    if ((idx < ia->size) && (idx >= 0)) {
        js_pushnumber(J, ia->data[idx]);
    } else {
        JS_EIDX(J, idx);
    }
}

/**
 * @brief set value at specific index
 * ia.Set(idx:number, val:number)
 *
 * @param J VM state.
 */
static void IntArray_Set(js_State *J) {
    int_array_t *ia = js_touserdata(J, 0, TAG_INT_ARRAY);

    int32_t idx = js_toint32(J, 1);
    IA_TYPE val = js_toint32(J, 2);
    if ((idx < ia->size) && (idx >= 0)) {
        ia->data[idx] = val;
    } else {
        JS_EIDX(J, idx);
    }
}

/**
 * @brief get and remove last entry
 * ia.Pop():number
 *
 * @param J VM state.
 */
static void IntArray_Pop(js_State *J) {
    int_array_t *ia = js_touserdata(J, 0, TAG_INT_ARRAY);

    if (ia->size) {
        ia->size--;
        IA_UPDATE(J, "length", ia->size);

        js_pushnumber(J, ia->data[ia->size]);
    } else {
        js_pushundefined(J);
    }
}

/**
 * @brief get and remove first entry
 * ia.Shift():number
 *
 * @param J VM state.
 */
static void IntArray_Shift(js_State *J) {
    int_array_t *ia = js_touserdata(J, 0, TAG_INT_ARRAY);

    if (ia->size) {
        ia->size--;
        IA_UPDATE(J, "length", ia->size);

        js_pushnumber(J, ia->data[0]);
        for (int i = 0; i < ia->size; i++) {
            ia->data[i] = ia->data[i + 1];
        }
    } else {
        js_pushundefined(J);
    }
}

/**
 * @brief append value to array.
 * ia.Push(val:number)
 *
 * @param J VM state.
 */
static void IntArray_Push(js_State *J) {
    int_array_t *ia = js_touserdata(J, 0, TAG_INT_ARRAY);
    IA_TYPE val = js_toint32(J, 1);

    int res = IntArray_push(ia, val);

    if (res > 0) {
        IA_UPDATE(J, "alloc_size", ia->alloc_size);
    } else if (res < 0) {
        JS_ENOMEM(J);
        return;
    }

    IA_UPDATE(J, "length", ia->size);
}

/**
 * @brief convert to JS array.
 * ia.ToArray():number[]
 *
 * @param J VM state.
 */
static void IntArray_ToArray(js_State *J) {
    int_array_t *ia = js_touserdata(J, 0, TAG_INT_ARRAY);

    js_newarray(J);
    for (uint32_t i = 0; i < ia->size; i++) {
        js_pushnumber(J, ia->data[i]);
        js_setindex(J, -2, i);
    }
}

/**
 * @brief truncate the array to zero size
 * ia.Clear()
 *
 * @param J VM state.
 */
static void IntArray_Clear(js_State *J) {
    int_array_t *ia = js_touserdata(J, 0, TAG_INT_ARRAY);

    ia->size = 0;
    IA_UPDATE(J, "length", ia->size);
}

/**
 * @brief convert IntArray to a string
 * ia.ToString():string
 *
 * @param J VM state.
 */
static void IntArray_ToString(js_State *J) {
    int_array_t *ia = js_touserdata(J, 0, TAG_INT_ARRAY);

    // get memory
    char *str = malloc(ia->size);
    if (!str) {
        JS_ENOMEM(J);
        return;
    }

    // copy over data
    for (int i = 0; i < ia->size; i++) {
        str[i] = 0xFF & ia->data[i];
    }

    // push string and free buffer
    js_pushlstring(J, str, ia->size);
    free(str);
}

/**
 * @brief append javascript array to IntArray
 * ia.Append(a:number[])
 *
 * @param J VM state.
 */
static void IntArray_Append(js_State *J) {
    int_array_t *ia = js_touserdata(J, 0, TAG_INT_ARRAY);

    if (js_isstring(J, 1)) {
        const char *str = js_tostring(J, 1);
        while (*str) {
            if (IntArray_push(ia, 0xFF & (*str)) < 0) {
                IntArray_destroy(ia);
                JS_ENOMEM(J);
                return;
            }
            str++;
        }
    } else if (js_isarray(J, 1)) {
        int len = js_getlength(J, 1);

        for (int i = 0; i < len; i++) {
            js_getindex(J, 1, i);

            IA_TYPE val;
            if (js_isstring(J, -1)) {
                val = js_tostring(J, -1)[0];
            } else {
                val = js_toint32(J, -1);
            }

            int res = IntArray_push(ia, val);
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
    IA_UPDATE(J, "alloc_size", ia->alloc_size);
    IA_UPDATE(J, "length", ia->size);
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize IntArray class
 *
 * @param J VM state.
 */
void init_intarray(js_State *J) {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    js_newobject(J);
    {
        NPROTDEF(J, IntArray, Get, 1);
        NPROTDEF(J, IntArray, Set, 2);
        NPROTDEF(J, IntArray, Push, 1);
        NPROTDEF(J, IntArray, Pop, 0);
        NPROTDEF(J, IntArray, Shift, 0);
        NPROTDEF(J, IntArray, ToArray, 0);
        NPROTDEF(J, IntArray, Clear, 0);
        NPROTDEF(J, IntArray, ToString, 0);
        NPROTDEF(J, IntArray, Append, 1);
    }
    CTORDEF(J, new_IntArray, TAG_INT_ARRAY, 0);

    js_newobject(J);
    {
        NPROTDEF(J, IntArray, Get, 1);
        NPROTDEF(J, IntArray, Set, 2);
        NPROTDEF(J, IntArray, Push, 1);
        NPROTDEF(J, IntArray, Pop, 0);
        NPROTDEF(J, IntArray, Shift, 0);
        NPROTDEF(J, IntArray, ToArray, 0);
        NPROTDEF(J, IntArray, Clear, 0);
        NPROTDEF(J, IntArray, ToString, 0);
        NPROTDEF(J, IntArray, Append, 1);
    }
    js_setregistry(J, TAG_INT_ARRAY);

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}

/**
 * @brief create an IntArray from a byte array. The object remains on the stack.
 *
 * @param J VM state.
 * @param data the data (will be copied).
 * @param size size of the data.
 */
void IntArray_fromBytes(js_State *J, const uint8_t *data, uint32_t size) {
    int_array_t *ia = calloc(sizeof(int_array_t), 1);
    if (!ia) {
        JS_ENOMEM(J);
        return;
    }
    ia->data = calloc(size, sizeof(IA_TYPE));
    if (!ia->data) {
        free(ia);
        JS_ENOMEM(J);
        return;
    }
    ia->alloc_size = size;
    ia->size = size;

    for (uint32_t i = 0; i < size; i++) {
        ia->data[i] = data[i];
    }

    js_getregistry(J, TAG_INT_ARRAY);
    js_newuserdata(J, TAG_INT_ARRAY, ia, IntArray_Finalize);

    // add properties
    js_pushnumber(J, ia->alloc_size);
    js_defproperty(J, -2, "alloc_size", JS_DONTCONF);

    js_pushnumber(J, ia->size);
    js_defproperty(J, -2, "length", JS_DONTCONF);
}

/**
 * @brief create an IntArray object from an existing struct. The object remains on the stack.
 *
 * @param J VM state.
 * @param ia pointer to an existing struct.
 */
void IntArray_fromStruct(js_State *J, int_array_t *ia) {
    js_getregistry(J, TAG_INT_ARRAY);
    js_newuserdata(J, TAG_INT_ARRAY, ia, IntArray_Finalize);

    // add properties
    js_pushnumber(J, ia->alloc_size);
    js_defproperty(J, -2, "alloc_size", JS_DONTCONF);

    js_pushnumber(J, ia->size);
    js_defproperty(J, -2, "length", JS_DONTCONF);
}

/**
 * @brief free resources for IntArray.
 *
 * @param ia pointer to an existing struct.
 */
void IntArray_destroy(int_array_t *ia) {
    if (ia) {
        if (ia->data) {
            free(ia->data);
        }
        free(ia);
    }
}

/**
 * @brief create an empty IntArray struct.
 *
 * @return int_array_t* a new struct or NULL for no memory.
 */
int_array_t *IntArray_create() {
    int_array_t *ia = calloc(sizeof(int_array_t), 1);
    if (!ia) {
        return NULL;
    }
    ia->data = calloc(IA_DEFAULT_SIZE, sizeof(IA_TYPE));
    if (!ia->data) {
        free(ia);
        return NULL;
    }
    ia->alloc_size = IA_DEFAULT_SIZE;
    ia->size = 0;

    return ia;
}

/**
 * @brief push a value to an IntArray.
 *
 * @param ia pointer to an existing struct.
 * @param val the value to append.
 * @return 0 if the value was pushed, 1 if the array needed to be enlarged, -1 if out of memory.
 */
int IntArray_push(int_array_t *ia, IA_TYPE val) {
    int ret = 0;

    if (ia->size >= ia->alloc_size) {
        uint32_t larger_size = (ia->alloc_size * IA_INC_FACTOR) / IA_FACTOR_SCALE;
        IA_TYPE *larger = calloc(larger_size, sizeof(IA_TYPE));
        if (!larger) {
            return -1;
        }

        for (int i = 0; i < ia->size; i++) {
            larger[i] = ia->data[i];
        }

        free(ia->data);
        ia->data = larger;
        ia->alloc_size = larger_size;

        ret = 1;
    }
    ia->data[ia->size] = val;
    ia->size++;

    return ret;
}
