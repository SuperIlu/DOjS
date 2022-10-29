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

#include <errno.h>
#include <mujs.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DOjS.h"
#include "file.h"
#include "bytearray.h"

/************
** defines **
************/
#define MAX_LINE_LENGTH 4096  //!< read at max 4KiB

/************
** structs **
************/
//! file userdata definition
typedef struct __file {
    FILE *file;      //!< the file pointer
    bool writeable;  //!< indicates the file was opened for writing
} file_t;

/*********************
** static functions **
*********************/
/**
 * @brief finalize a file and free resources.
 *
 * @param J VM state.
 */
static void File_Finalize(js_State *J, void *data) {
    file_t *f = (file_t *)data;
    if (f->file) {
        fclose(f->file);
    }
    free(f);
}

/**
 * @brief open a file and store it as userdata in JS object.
 * new File(filename:string, mode:string)
 *
 * @param J VM state.
 */
static void new_File(js_State *J) {
    NEW_OBJECT_PREP(J);
    const char *fname = js_tostring(J, 1);

    file_t *f = calloc(1, sizeof(file_t));
    if (!f) {
        JS_ENOMEM(J);
        return;
    }

    const char *mode = js_tostring(J, 2);
    if (mode[0] == 'a' || mode[0] == 'w') {
        f->writeable = true;
    } else if (mode[0] == 'r') {
        f->writeable = false;
    } else {
        js_error(J, "Unknown mode for file '%s'", mode);
        free(f);
        return;
    }

    f->file = fopen(fname, mode);
    if (!f->file) {
        js_error(J, "cannot open file '%s': %s", fname, strerror(errno));
        free(f);
        return;
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_FILE, f, File_Finalize);

    // add properties
    js_pushstring(J, fname);
    js_defproperty(J, -2, "filename", JS_READONLY | JS_DONTCONF);

    js_pushstring(J, mode);
    js_defproperty(J, -2, "mode", JS_READONLY | JS_DONTCONF);
}

/**
 * @brief return the next byte from the file as number.
 * file.ReadByte():number
 *
 * @param J VM state.
 */
static void File_ReadByte(js_State *J) {
    file_t *f = js_touserdata(J, 0, TAG_FILE);
    if (!f->file) {
        js_error(J, "File was closed!");
        return;
    }

    if (f->writeable) {
        js_error(J, "File was opened for writing!");
        return;
    } else {
        int ch = getc(f->file);
        if (ch != EOF) {
            js_pushnumber(J, ch);
        } else {
            js_pushnull(J);
        }
    }
}

/**
 * @brief return the (remaining) bytes from the file as number array.
 * file.ReadBytes([num:number]):number[]
 *
 * @param J VM state.
 */
static void File_ReadBytes(js_State *J) {
    file_t *f = js_touserdata(J, 0, TAG_FILE);
    if (!f->file) {
        js_error(J, "File was closed!");
        return;
    }

    // get number of bytes to read
    uint32_t num = 0xFFFFFFFFU;
    if (js_isnumber(J, 1)) {
        num = js_touint32(J, 1);
    }

    if (f->writeable) {
        js_error(J, "File was opened for writing!");
        return;
    } else {
        js_newarray(J);

        uint32_t idx = 0;
        int ch = getc(f->file);
        while (ch != EOF) {
            js_pushnumber(J, ch);
            js_setindex(J, -2, idx);
            idx++;
            if (idx >= num) {
                break;
            }
            ch = getc(f->file);
        }
    }
}

/**
 * @brief return the (remaining) bytes from the file as ByteArray.
 * file.ReadInts([num:number]):ByteArray
 *
 * @param J VM state.
 */
static void File_ReadInts(js_State *J) {
    file_t *f = js_touserdata(J, 0, TAG_FILE);
    if (!f->file) {
        js_error(J, "File was closed!");
        return;
    }

    // get number of bytes to read
    uint32_t num = 0xFFFFFFFFU;
    if (js_isnumber(J, 1)) {
        num = js_touint32(J, 1);
    }

    if (f->writeable) {
        js_error(J, "File was opened for writing!");
        return;
    } else {
        byte_array_t *ba = ByteArray_create();
        if (!ba) {
            JS_ENOMEM(J);
            return;
        }

        int ch = getc(f->file);
        while (ch != EOF) {
            if (ByteArray_push(ba, 0xFF & ch) < 0) {
                ByteArray_destroy(ba);
                JS_ENOMEM(J);
                return;
            }
            num--;
            if (num == 0) {
                break;
            }
            ch = getc(f->file);
        }
        ByteArray_fromStruct(J, ba);
    }
}

/**
 * @brief return the next line from the file as string.
 * file.ReadLine():string
 *
 * @param J VM state.
 */
static void File_ReadLine(js_State *J) {
    file_t *f = js_touserdata(J, 0, TAG_FILE);
    if (!f->file) {
        js_error(J, "File was closed!");
        return;
    }

    if (f->writeable) {
        js_error(J, "File was opened for writing!");
        return;
    } else {
        char line[MAX_LINE_LENGTH + 1];
        char *s = fgets(line, sizeof(line), f->file);
        if (s) {
            js_pushstring(J, line);
        } else {
            js_pushnull(J);
        }
    }
}

/**
 * @brief close the file.
 * file.Close()
 *
 * @param J VM state.
 */
static void File_Close(js_State *J) {
    file_t *f = js_touserdata(J, 0, TAG_FILE);
    if (f->file) {
        fclose(f->file);
        f->file = NULL;
    }
}

/**
 * @brief get size of file.
 * file.GetSize():number
 *
 * @param J VM state.
 */
static void File_GetSize(js_State *J) {
    file_t *f = js_touserdata(J, 0, TAG_FILE);
    if (!f->file) {
        js_error(J, "File was closed!");
        return;
    }

    int old = ftell(f->file);
    fseek(f->file, 0, SEEK_END);
    int size = ftell(f->file);
    fseek(f->file, old, SEEK_SET);

    js_pushnumber(J, size);
}

/**
 * @brief write a byte to a file.
 * file.WriteByte(ch:number)
 *
 * @param J VM state.
 */
static void File_WriteByte(js_State *J) {
    file_t *f = js_touserdata(J, 0, TAG_FILE);
    if (!f->file) {
        js_error(J, "File was closed!");
        return;
    }

    if (!f->writeable) {
        js_error(J, "File was opened for reading!");
        return;
    } else {
        int ch = js_toint16(J, 1);
        int err = fputc((char)ch, f->file);
        if (err == EOF) {
            js_error(J, "Error writing to file!");
            return;
        }
        fflush(f->file);
    }
}

/**
 * @brief write bytes to a file.
 * file.WriteBytes(data:number[], [num:number])
 *
 * @param J VM state.
 */
static void File_WriteBytes(js_State *J) {
    file_t *f = js_touserdata(J, 0, TAG_FILE);
    if (!f->file) {
        js_error(J, "File was closed!");
        return;
    }

    // get number of bytes to read
    uint32_t num = 0xFFFFFFFFU;
    if (js_isnumber(J, 2)) {
        num = js_touint32(J, 2);
    }

    if (!f->writeable) {
        js_error(J, "File was opened for reading!");
        return;
    } else {
        if (js_isarray(J, 1)) {
            int len = js_getlength(J, 1);
            if (num < len) {
                len = num;
            }

            uint8_t *data = malloc(len);
            if (!data) {
                JS_ENOMEM(J);
                return;
            }

            for (int i = 0; i < len; i++) {
                js_getindex(J, 1, i);
                data[i] = (uint8_t)js_toint16(J, -1);
                js_pop(J, 1);
            }
            int err = fwrite(data, 1, len, f->file);
            free(data);
            if (err != len) {
                js_error(J, "Error writing to file!");
                return;
            }
        } else {
            JS_ENOARR(J);
        }
    }
}

/**
 * @brief write bytes to a file.
 * file.WriteInts(data:ByteArray, [num:number])
 *
 * @param J VM state.
 */
static void File_WriteInts(js_State *J) {
    file_t *f = js_touserdata(J, 0, TAG_FILE);
    if (!f->file) {
        js_error(J, "File was closed!");
        return;
    }

    JS_CHECKTYPE(J, 1, TAG_BYTE_ARRAY);

    // get number of bytes to read
    uint32_t num = 0xFFFFFFFFU;
    if (js_isnumber(J, 2)) {
        num = js_touint32(J, 2);
    }

    if (!f->writeable) {
        js_error(J, "File was opened for reading!");
        return;
    } else {
        if (js_isuserdata(J, 1, TAG_BYTE_ARRAY)) {
            byte_array_t *ba = js_touserdata(J, 1, TAG_BYTE_ARRAY);

            int len = ba->size;
            if (num < len) {
                len = num;
            }

            int err = fwrite(ba->data, 1, len, f->file);
            if (err != len) {
                js_error(J, "Error writing to file!");
                return;
            }
        } else {
            JS_ENOARR(J);
        }
    }
}

/**
 * @brief write a string (terminated by a NEWLINE) to a file.
 * file.WriteLine(txt:string)
 *
 * @param J VM state.
 */
static void File_WriteLine(js_State *J) {
    file_t *f = js_touserdata(J, 0, TAG_FILE);
    if (!f->file) {
        js_error(J, "File was closed!");
        return;
    }

    const char *line = js_tostring(J, 1);

    if (!f->writeable) {
        js_error(J, "File was opened for reading!");
        return;
    } else {
        int err = fputs(line, f->file);
        if (err == EOF) {
            js_error(J, "Error writing to file!");
            return;
        }
        err = fputc('\n', f->file);
        if (err == EOF) {
            js_error(J, "Error writing to file!");
            return;
        }
        fflush(f->file);
    }
}

/**
 * @brief write a string to a file.
 * file.WriteString(txt:string)
 *
 * @param J VM state.
 */
static void File_WriteString(js_State *J) {
    file_t *f = js_touserdata(J, 0, TAG_FILE);
    if (!f->file) {
        js_error(J, "File was closed!");
        return;
    }

    const char *line = js_tostring(J, 1);

    if (!f->writeable) {
        js_error(J, "File was opened for reading!");
        return;
    } else {
        int err = fputs(line, f->file);
        if (err == EOF) {
            js_error(J, "Error writing to file!");
            return;
        }
        fflush(f->file);
    }
}

/**
 * @brief get current read/write position.
 * file.Tell():number
 *
 * @param J VM state.
 */
static void File_Tell(js_State *J) {
    file_t *f = js_touserdata(J, 0, TAG_FILE);
    if (!f->file) {
        js_error(J, "File was closed!");
        return;
    }

    long pos = ftell(f->file);
    if (pos < 0) {
        js_error(J, "Error getting file position!");
    } else {
        js_pushnumber(J, pos);
    }
}

/**
 * @brief set current read/write position.
 * file.Seek(offset:number, whence:number):number
 *
 * @param J VM state.
 */
static void File_Seek(js_State *J) {
    file_t *f = js_touserdata(J, 0, TAG_FILE);
    if (!f->file) {
        js_error(J, "File was closed!");
        return;
    }

    int32_t offset = js_toint32(J, 1);
    uint32_t whence = SEEK_SET;

    // get whence if it is specified
    if (js_isnumber(J, 2)) {
        whence = js_touint32(J, 2);
    }

    int newPos = fseek(f->file, offset, whence);
    if (newPos < 0) {
        js_error(J, "Error setting file position!");
    } else {
        js_pushnumber(J, newPos);
    }
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize file subsystem.
 *
 * @param J VM state.
 */
void init_file(js_State *J) {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    js_newobject(J);
    {
        NPROTDEF(J, File, ReadByte, 0);
        NPROTDEF(J, File, ReadBytes, 1);
        NPROTDEF(J, File, ReadInts, 1);
        NPROTDEF(J, File, ReadLine, 0);
        NPROTDEF(J, File, Close, 0);
        NPROTDEF(J, File, WriteByte, 1);
        NPROTDEF(J, File, WriteBytes, 2);
        NPROTDEF(J, File, WriteInts, 2);
        NPROTDEF(J, File, WriteLine, 1);
        NPROTDEF(J, File, WriteString, 1);
        NPROTDEF(J, File, GetSize, 0);
        NPROTDEF(J, File, Tell, 0);
        NPROTDEF(J, File, Seek, 2);
    }
    CTORDEF(J, new_File, TAG_FILE, 2);

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}
