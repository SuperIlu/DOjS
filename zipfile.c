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

#include "zipfile.h"

#include <errno.h>
#include <mujs.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DOjS.h"
#include "zip.h"
#include "intarray.h"

/************
** defines **
************/
#define MAX_LINE_LENGTH 4096  //!< read at max 4KiB

/************
** structs **
************/
//! file userdata definition
typedef struct __file {
    struct zip_t *zip;  //!< the zip pointer
    bool writeable;     //!< indicates the zip was opened for writing
} jszip_t;

/*********************
** static functions **
*********************/
/**
 * @brief finalize a zip and free resources.
 *
 * @param J VM state.
 */
static void Zip_Finalize(js_State *J, void *data) {
    jszip_t *z = (jszip_t *)data;
    if (z->zip) {
        zip_close(z->zip);
    }
    free(z);
}

/**
 * @brief open a zip and store it as userdata in JS object.
 * new Zip(filename:string, mode:string, [compression:number])
 *
 * @param J VM state.
 */
static void new_Zip(js_State *J) {
    NEW_OBJECT_PREP(J);
    const char *fname = js_tostring(J, 1);

    jszip_t *z = malloc(sizeof(jszip_t));
    if (!z) {
        JS_ENOMEM(J);
        return;
    }

    const char *mode = js_tostring(J, 2);
    if (mode[0] == 'a' || mode[0] == 'w') {
        z->writeable = true;
    } else if (mode[0] == 'r') {
        z->writeable = false;
    } else {
        js_error(J, "Unknown mode for ZIP '%s'", mode);
        free(z);
        return;
    }

    int level;
    if (js_isnumber(J, 3)) {
        level = js_toint16(J, 3);
        if (level < 0) {
            level = 0;
        }
        if (level > 9) {
            level = 9;
        }
    } else {
        level = ZIP_DEFAULT_COMPRESSION_LEVEL;
    }

    z->zip = zip_open(fname, level, mode[0]);
    if (!z->zip) {
        js_error(J, "cannot open ZIP '%s'", fname);
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_ZIP, z, Zip_Finalize);
}

/**
 * @brief close the zip.
 * zip.Close()
 *
 * @param J VM state.
 */
static void Zip_Close(js_State *J) {
    jszip_t *z = js_touserdata(J, 0, TAG_ZIP);
    if (z->zip) {
        zip_close(z->zip);
        z->zip = NULL;
    }
}

/**
 * @brief get number of entries in ZIP
 * zip.NumEntries():number
 *
 * @param J VM state.
 */
static void Zip_NumEntries(js_State *J) {
    jszip_t *z = js_touserdata(J, 0, TAG_ZIP);
    if (!z->zip) {
        js_error(J, "ZIP was closed!");
        return;
    }
    int num_ent = zip_total_entries(z->zip);
    if (num_ent >= 0) {
        js_pushnumber(J, num_ent);
    } else {
        js_error(J, "ZIP error!");
        return;
    }
}

/**
 * @brief get entry listing.
 * zip.GetEntries():Array
 * [
 *      [name, is_directory, size, crc32],
 *      ...
 * ]
 *
 * @param J VM state.
 */
static void Zip_GetEntries(js_State *J) {
    jszip_t *z = js_touserdata(J, 0, TAG_ZIP);
    if (!z->zip) {
        js_error(J, "ZIP was closed!");
        return;
    }

    js_newarray(J);
    int n = zip_total_entries(z->zip);
    for (int i = 0; i < n; i++) {
        zip_entry_openbyindex(z->zip, i);

        js_newobject(J);
        {
            js_pushstring(J, zip_entry_name(z->zip));
            js_setproperty(J, -2, "name");
            js_pushboolean(J, zip_entry_isdir(z->zip));
            js_setproperty(J, -2, "is_directory");
            js_pushnumber(J, zip_entry_size(z->zip));
            js_setproperty(J, -2, "size");
            js_pushnumber(J, zip_entry_crc32(z->zip));
            js_setproperty(J, -2, "crc32");
        }
        js_setindex(J, -2, i);
    }
}

/**
 * @brief add file to ZIP.
 * zip.AddFile(entry_path:string, hdd_path:string)
 *
 * @param J VM state.
 */
static void Zip_AddFile(js_State *J) {
    jszip_t *z = js_touserdata(J, 0, TAG_ZIP);
    if (!z->zip) {
        js_error(J, "ZIP was closed!");
        return;
    }
    if (!z->writeable) {
        js_error(J, "ZIP was opened for reading!");
        return;
    } else {
        const char *zip_name = js_tostring(J, 1);
        const char *hdd_name = js_tostring(J, 2);

        if (zip_entry_open(z->zip, zip_name) < 0) {
            js_error(J, "Could not add '%s' as '%s' to ZIP (zip_entry_fwrite)!", hdd_name, zip_name);
            return;
        }
        if (zip_entry_fwrite(z->zip, hdd_name) < 0) {
            js_error(J, "Could not add '%s' as '%s' to ZIP (zip_entry_fwrite)!", hdd_name, zip_name);
            return;
        }
        if (zip_entry_close(z->zip) < 0) {
            js_error(J, "Could not add '%s' as '%s' to ZIP (zip_entry_fwrite)!", hdd_name, zip_name);
            return;
        }
    }
}

/**
 * @brief extract file from ZIP.
 * zip.ExtractFile(entry_path:string, hdd_path:string)
 *
 * @param J VM state.
 */
static void Zip_ExtractFile(js_State *J) {
    jszip_t *z = js_touserdata(J, 0, TAG_ZIP);
    if (!z->zip) {
        js_error(J, "ZIP was closed!");
        return;
    }

    const char *zip_name = js_tostring(J, 1);
    const char *hdd_name = js_tostring(J, 2);

    if (zip_entry_open(z->zip, zip_name) < 0) {
        js_error(J, "Could not extract '%s' into '%s' from ZIP (zip_entry_open)!", zip_name, hdd_name);
        return;
    }
    if (zip_entry_fread(z->zip, hdd_name) < 0) {
        js_error(J, "Could not extract '%s' into '%s' from ZIP (zip_entry_fread)!", zip_name, hdd_name);
        return;
    }
    if (zip_entry_close(z->zip) < 0) {
        js_error(J, "Could not extract '%s' into '%s' from ZIP (zip_entry_close)!", zip_name, hdd_name);
        return;
    }
}

/**
 * @brief return the bytes from the zip entry as number array.
 * zip.ReadBytes(entry_name:string):number[]
 *
 * @param J VM state.
 */
static void Zip_ReadBytes(js_State *J) {
    jszip_t *z = js_touserdata(J, 0, TAG_ZIP);
    if (!z->zip) {
        js_error(J, "ZIP was closed!");
        return;
    }

    if (z->writeable) {
        js_error(J, "ZIP was opened for writing!");
        return;
    } else {
        uint8_t *buf = NULL;
        size_t bufsize;

        const char *zip_name = js_tostring(J, 1);

        if (zip_entry_open(z->zip, zip_name) < 0) {
            js_error(J, "Could not extract entry '%s' from ZIP (zip_entry_open)!", zip_name);
            return;
        }
        if (zip_entry_read(z->zip, (void **)&buf, &bufsize) < 0) {
            js_error(J, "Could not extract entry '%s' from ZIP (zip_entry_open)!", zip_name);
            return;
        }
        if (zip_entry_close(z->zip) < 0) {
            js_error(J, "Could not extract entry '%s' from ZIP (zip_entry_close)!", zip_name);
            free(buf);
            return;
        }

        js_newarray(J);
        for (int i = 0; i < bufsize; i++) {
            js_pushnumber(J, buf[i]);
            js_setindex(J, -2, i);
        }
        free(buf);
    }
}

/**
 * @brief return the bytes from the zip entry as IntArray.
 * zip.ReadBytes(entry_name:string):IntArray
 *
 * @param J VM state.
 */
static void Zip_ReadInts(js_State *J) {
    jszip_t *z = js_touserdata(J, 0, TAG_ZIP);
    if (!z->zip) {
        js_error(J, "ZIP was closed!");
        return;
    }

    if (z->writeable) {
        js_error(J, "ZIP was opened for writing!");
        return;
    } else {
        uint8_t *buf = NULL;
        size_t bufsize;

        const char *zip_name = js_tostring(J, 1);

        if (zip_entry_open(z->zip, zip_name) < 0) {
            js_error(J, "Could not extract entry '%s' from ZIP (zip_entry_open)!", zip_name);
            return;
        }
        if (zip_entry_read(z->zip, (void **)&buf, &bufsize) < 0) {
            js_error(J, "Could not extract entry '%s' from ZIP (zip_entry_open)!", zip_name);
            return;
        }
        if (zip_entry_close(z->zip) < 0) {
            js_error(J, "Could not extract entry '%s' from ZIP (zip_entry_close)!", zip_name);
            free(buf);
            return;
        }
        IntArray_fromBytes(J, buf, bufsize);
        free(buf);
    }
}

/**
 * @brief write a bytes to a zip entry.
 * zip.WriteBytes(entry_name:string, data:number[])
 *
 * @param J VM state.
 */
static void Zip_WriteBytes(js_State *J) {
    jszip_t *z = js_touserdata(J, 0, TAG_ZIP);
    if (!z->zip) {
        js_error(J, "ZIP was closed!");
        return;
    }

    if (!z->writeable) {
        js_error(J, "ZIP was opened for reading!");
        return;
    } else {
        if (js_isarray(J, 2)) {
            int len = js_getlength(J, 2);

            const char *zip_name = js_tostring(J, 1);
            if (zip_entry_open(z->zip, zip_name) < 0) {
                js_error(J, "Could create '%s' in ZIP (zip_entry_open)!", zip_name);
                return;
            }

            uint8_t *data = malloc(len);
            if (!data) {
                JS_ENOMEM(J);
                return;
            }

            for (int i = 0; i < len; i++) {
                js_getindex(J, 2, i);
                data[i] = (uint8_t)js_toint16(J, -1);
                js_pop(J, 1);
            }
            if (zip_entry_write(z->zip, data, len) < 0) {
                js_error(J, "Could create '%s' in ZIP (zip_entry_write)!", zip_name);
                free(data);
                return;
            }
            if (zip_entry_close(z->zip) < 0) {
                js_error(J, "Could create '%s' in ZIP (zip_entry_close)!", zip_name);
                free(data);
                return;
            }

            free(data);
        } else {
            JS_ENOARR(J);
        }
    }
}

/**
 * @brief write a bytes to a zip entry.
 * zip.WriteBytes(entry_name:string, data:IntArray)
 *
 * @param J VM state.
 */
static void Zip_WriteInts(js_State *J) {
    jszip_t *z = js_touserdata(J, 0, TAG_ZIP);
    if (!z->zip) {
        js_error(J, "ZIP was closed!");
        return;
    }

    if (!z->writeable) {
        js_error(J, "ZIP was opened for reading!");
        return;
    } else {
        if (js_isuserdata(J, 2, TAG_INT_ARRAY)) {
            int_array_t *ia = js_touserdata(J, 2, TAG_INT_ARRAY);

            const char *zip_name = js_tostring(J, 1);
            if (zip_entry_open(z->zip, zip_name) < 0) {
                js_error(J, "Could create '%s' in ZIP (zip_entry_open)!", zip_name);
                return;
            }

            uint8_t *data = malloc(ia->size);
            if (!data) {
                JS_ENOMEM(J);
                return;
            }

            for (int i = 0; i < ia->size; i++) {
                data[i] = ia->data[i];
            }
            if (zip_entry_write(z->zip, data, ia->size) < 0) {
                js_error(J, "Could create '%s' in ZIP (zip_entry_write)!", zip_name);
                free(data);
                return;
            }
            if (zip_entry_close(z->zip) < 0) {
                js_error(J, "Could create '%s' in ZIP (zip_entry_close)!", zip_name);
                free(data);
                return;
            }

            free(data);
        } else {
            JS_ENOARR(J);
        }
    }
}

/*----------------------------------------------------------------------*/
/*                memory vtable                                         */
/*----------------------------------------------------------------------*/

/* The packfile data for our memory reader. */
typedef struct MEMREAD_INFO {
    AL_CONST unsigned char *block;
    long unsigned int length;
    long unsigned int offset;
} MEMREAD_INFO;

static int memread_getc(void *userdata) {
    MEMREAD_INFO *info = userdata;
    ASSERT(info);
    ASSERT(info->offset <= info->length);

    if (info->offset == info->length) {
        return EOF;
    } else {
        return info->block[info->offset++];
    }
}

static int memread_ungetc(int c, void *userdata) {
    MEMREAD_INFO *info = userdata;
    unsigned char ch = c;

    if ((info->offset > 0) && (info->block[info->offset - 1] == ch)) {
        return ch;
    } else {
        return EOF;
    }
}

static int memread_putc(int c, void *userdata) { return EOF; }

static long memread_fread(void *p, long n, void *userdata) {
    MEMREAD_INFO *info = userdata;
    size_t actual;
    ASSERT(info);
    ASSERT(info->offset <= info->length);

    actual = MIN(n, info->length - info->offset);

    memcpy(p, info->block + info->offset, actual);
    info->offset += actual;

    ASSERT(info->offset <= info->length);

    return actual;
}

static long memread_fwrite(AL_CONST void *p, long n, void *userdata) { return 0; }

static int memread_seek(void *userdata, int offset) {
    MEMREAD_INFO *info = userdata;
    long actual;
    ASSERT(info);
    ASSERT(info->offset <= info->length);

    actual = MIN(offset, info->length - info->offset);

    info->offset += actual;

    ASSERT(info->offset <= info->length);

    if (offset == actual) {
        return 0;
    } else {
        return -1;
    }
}

static int memread_fclose(void *userdata) {
    MEMREAD_INFO *info = userdata;
    free((void *)info->block);
    free(info);
    return 0;
}

static int memread_feof(void *userdata) {
    MEMREAD_INFO *info = userdata;

    return info->offset >= info->length;
}

static int memread_ferror(void *userdata) {
    (void)userdata;

    return FALSE;
}

/* The actual vtable. Note that writing is not supported, the functions for
 * writing above are only placeholders.
 */
static PACKFILE_VTABLE memread_vtable = {memread_fclose, memread_getc, memread_ungetc, memread_fread, memread_putc, memread_fwrite, memread_seek, memread_feof, memread_ferror};

/***********************
** exported functions **
***********************/
/**
 * @brief initialize zip subsystem.
 *
 * @param J VM state.
 */
void init_zipfile(js_State *J) {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    js_newobject(J);
    {
        NPROTDEF(J, Zip, Close, 0);
        NPROTDEF(J, Zip, NumEntries, 0);
        NPROTDEF(J, Zip, GetEntries, 0);
        NPROTDEF(J, Zip, AddFile, 2);
        NPROTDEF(J, Zip, ExtractFile, 2);
        NPROTDEF(J, Zip, WriteBytes, 2);
        NPROTDEF(J, Zip, ReadBytes, 1);
        NPROTDEF(J, Zip, WriteInts, 2);
        NPROTDEF(J, Zip, ReadInts, 1);
    }
    CTORDEF(J, new_Zip, TAG_ZIP, 3);

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}

/**
 * @brief read an entry from a ZIP file into memory and provide its contents as a PACKFILE.
 *
 * @param zname name of the ZIP file.
 * @param ename name of the entry.
 *
 * @return PACKFILE* on success, NULL on failure.
 */
PACKFILE *open_zipfile2(const char *zname, const char *ename) {
    MEMREAD_INFO *info = malloc(sizeof(MEMREAD_INFO));
    if (!info) {
        return NULL;
    }
    info->offset = 0;

    struct zip_t *zip = zip_open(zname, 0, 'r');
    if (!zip) {
        free(info);
        return NULL;
    }

    if (zip_entry_open(zip, ename) < 0) {
        zip_close(zip);
        free(info);
        return NULL;
    }
    if (zip_entry_read(zip, (void **)&info->block, &info->length) < 0) {
        zip_close(zip);
        free(info);
        return NULL;
    }
    zip_entry_close(zip);
    zip_close(zip);

    return pack_fopen_vtable(&memread_vtable, info);
}

/**
 * @brief read an entry from a ZIP file into memory and provide its contents as a PACKFILE.
 *
 * @param fname a filename in the format of "<zip file>=<entry name>".
 *
 * @return PACKFILE* on success, NULL on failure.
 */
PACKFILE *open_zipfile1(const char *fname) {
    char *delim = strchr(fname, ZIP_DELIM);

    if (!delim) {
        return NULL;
    }

    // get memory for a copy of the filename
    int flen = strlen(fname) + 1;
    char *zname = malloc(flen);
    if (!zname) {
        return NULL;
    }
    memcpy(zname, fname, flen);
    int idx = delim - fname;
    zname[idx] = 0;
    char *ename = &zname[idx + 1];

    PACKFILE *pf = open_zipfile2(zname, ename);
    if (!pf) {
        free(zname);
        return NULL;
    }
    free(zname);
    return pf;
}

/**
 * @brief read an entry from a ZIP file into memory. The read file is always null terminated, but 'size' always represents the original file size.
 *
 * @param fname a filename in the format of "<zip file>=<entry name>".
 * @param buf pointer to the read data. must be freed by caller.
 * @param size the real file size (without terminating null byte)
 *
 * @return true if the file could be read, buf and size will return valid values
 * @return false if an error occured, buf and size will be 0.
 */
bool read_zipfile1(const char *fname, void **buf, size_t *size) {
    char *delim = strchr(fname, ZIP_DELIM);

    if (!delim) {
        return NULL;
    }

    // get memory for a copy of the filename
    int flen = strlen(fname) + 1;
    char *zname = malloc(flen);
    if (!zname) {
        return NULL;
    }
    memcpy(zname, fname, flen);
    int idx = delim - fname;
    zname[idx] = 0;
    char *ename = &zname[idx + 1];

    bool ret = read_zipfile2(zname, ename, buf, size);

    free(zname);
    return ret;
}

/**
 * @brief read an entry from a ZIP file into memory. The read file is always null terminated, but 'size' always represents the original file size.
 *
 * @param zname name of the ZIP file.
 * @param ename name of the entry.
 * @param fname a filename in the format of "<zip file>=<entry name>".
 * @param buf pointer to the read data. must be freed by caller.
 * @param size the real file size (without terminating null byte)
 *
 * @return true if the file could be read, buf and size will return valid values
 * @return false if an error occured, buf and size will be 0.
 */
bool read_zipfile2(const char *zname, const char *ename, void **buf, size_t *size) {
    struct zip_t *zip = zip_open(zname, 0, 'r');
    if (!zip) {
        *size = 0;
        *buf = NULL;
        return false;
    }

    if (zip_entry_open(zip, ename) < 0) {
        zip_close(zip);
        *size = 0;
        *buf = NULL;
        return false;
    }
    *size = zip_entry_size(zip);
    *buf = malloc(*size + 1);
    char *b = *buf;
    if (!*buf) {
        zip_close(zip);
        *size = 0;
        *buf = NULL;
        return false;
    }
    if (zip_entry_noallocread(zip, *buf, *size) < 0) {
        zip_close(zip);
        free(*buf);
        *size = 0;
        *buf = NULL;
        return false;
    }
    b[*size] = 0;  // always null-terminate data
    zip_entry_close(zip);
    zip_close(zip);

    return true;
}

/**
 * @brief check if a ZIP file entry exists.
 *
 * @param fname a filename in the format of "<zip file>=<entry name>".
 *
 * @return true if the ZIP file and the entry could be found, else false.
 */
bool check_zipfile1(const char *fname) {
    char *delim = strchr(fname, ZIP_DELIM);

    if (!delim) {
        return NULL;
    }

    // get memory for a copy of the filename
    int flen = strlen(fname) + 1;
    char *zname = malloc(flen);
    if (!zname) {
        return NULL;
    }
    memcpy(zname, fname, flen);
    int idx = delim - fname;
    zname[idx] = 0;
    char *ename = &zname[idx + 1];

    bool ret = check_zipfile2(zname, ename);

    free(zname);
    return ret;
}

/**
 * @brief check if a ZIP file entry exists.
 *
 * @param zname name of the ZIP file.
 * @param ename name of the entry.
 *
 * @return true if the ZIP file and the entry could be found, else false.
 */
bool check_zipfile2(const char *zname, const char *ename) {
    struct zip_t *zip = zip_open(zname, 0, 'r');
    if (!zip) {
        return false;
    }

    if (zip_entry_open(zip, ename) < 0) {
        zip_close(zip);
        return false;
    }
    zip_entry_close(zip);
    zip_close(zip);

    return true;
}
