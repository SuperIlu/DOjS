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
#include <mujs.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DOjS.h"
#include "file.h"

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
 * new File(filename:string)
 *
 * @param J VM state.
 */
static void new_File(js_State *J) {
    const char *fname = js_tostring(J, 1);

    file_t *f = malloc(sizeof(file_t));
    if (!f) {
        js_error(J, "No memory for file '%s'", fname);
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
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_FILE, f, File_Finalize);
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

    int ch = js_toint16(J, 1);

    if (!f->writeable) {
        js_error(J, "File was opened for reading!");
        return;
    } else {
        fputc((char)ch, f->file);
        fflush(f->file);
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
        fputs(line, f->file);
        fputc('\n', f->file);
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
        fputs(line, f->file);
        fflush(f->file);
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
    // js_getglobal(J, "Object");
    // js_getproperty(J, -1, "prototype");
    js_newobject(J);
    {
        PROTDEF(J, File_ReadByte, TAG_FILE, "ReadByte", 0);
        PROTDEF(J, File_ReadLine, TAG_FILE, "ReadLine", 0);
        PROTDEF(J, File_Close, TAG_FILE, "Close", 0);
        PROTDEF(J, File_WriteByte, TAG_FILE, "WriteByte", 1);
        PROTDEF(J, File_WriteLine, TAG_FILE, "WriteLine", 1);
        PROTDEF(J, File_WriteString, TAG_FILE, "WriteString", 1);
    }
    js_newcconstructor(J, new_File, new_File, TAG_FILE, 2);
    js_defglobal(J, TAG_FILE, JS_DONTENUM);
}
