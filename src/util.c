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

#include "util.h"

#include <errno.h>
#include <string.h>

/***********************
** exported functions **
***********************/
/**
 * make a persistent copy of a string
 *
 * @param str the string to copy.
 *
 * @return a newly malloced() string or NULL if out of memory.
 */
char *ut_clone_string(const char *str) {
    size_t len = strlen(str) + 1;
    char *ret = malloc(len);
    if (!ret) {
        return NULL;
    }
    memcpy(ret, str, len);
    return ret;
}

/**
 * @brief check if string ends with suffix (case insensitiv).
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
    return strncasecmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

/**
 * @brief extract the file extension.
 *
 * @param filename a filename.
 *
 * @return everything after the last "." or an empty string if the was no dot.
 */
const char *ut_getFilenameExt(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

/**
 * @brief check if a file exists for reading.
 *
 * @param filename the name of the file.
 * @return true if the file exists and can be opened for reading, else false.
 */
bool ut_file_exists(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (f) {
        fclose(f);
        return true;
    } else {
        return false;
    }
}

/**
 * @brief read a file into memory. The read file is always null terminated, but 'size' always represents the original file size.
 *
 * @param fname a filename.
 * @param buf pointer to the read data. must be freed by caller.
 * @param size the real file size (without terminating null byte)
 *
 * @return true if the file could be read, buf and size will return valid values
 * @return false if an error occured, buf and size will be 0.
 */
bool ut_read_file(const char *fname, void **buf, size_t *size) {
    FILE *f;
    char *s;
    int n, t;

    *buf = NULL;
    *size = 0;

    f = fopen(fname, "rb");
    if (!f) {
        DEBUGF("Can't open file '%s': %s\n", fname, strerror(errno));
        return false;
    }

    if (fseek(f, 0, SEEK_END) < 0) {
        fclose(f);
        DEBUGF("Can't seek in file '%s': %s\n", fname, strerror(errno));
        return false;
    }

    n = ftell(f);
    if (n < 0) {
        fclose(f);
        DEBUGF("Can't tell in file '%s': %s\n", fname, strerror(errno));
        return false;
    }

    if (fseek(f, 0, SEEK_SET) < 0) {
        fclose(f);
        DEBUGF("Can't seek in file '%s': %s\n", fname, strerror(errno));
        return false;
    }

    s = malloc(n + 1);
    if (!s) {
        fclose(f);
        return false;
    }

    t = fread(s, 1, n, f);
    if (t != n) {
        free(s);
        fclose(f);
        DEBUGF("Can't read data from file '%s': %s\n", fname, strerror(errno));
        return false;
    }
    s[n] = 0;
    fclose(f);

    *buf = s;
    *size = n;
    return true;
}

/**
 * @brief try to determine image format in a ByteArray.
 *
 * @param ba the ByteArray
 *
 * @return file extension for that format or NULL for 'unknown'
 */
char *ut_getBitmapType(byte_array_t *ba) {
    if (ba->size < 12) {
        return NULL;
    }

    if ((ba->data[0] == 0xFF) && (ba->data[1] == 0xD8) && (ba->data[2] == 0xFF)) {
        return "JPG";
    } else if ((ba->data[0] == 'R') && (ba->data[1] == 'I') && (ba->data[2] == 'F') && (ba->data[3] == 'F') && (ba->data[8] == 'W') && (ba->data[9] == 'E') &&
               (ba->data[10] == 'B') && (ba->data[11] == 'P')) {
        return "WEP";  // WEBP
    } else if ((ba->data[0] == 'G') && (ba->data[1] == 'I') && (ba->data[2] == 'F') && (ba->data[3] == '8') && (ba->data[5] == 'a')) {
        return "GIF";
    } else if ((ba->data[0] == 0x89) && (ba->data[1] == 0x50) && (ba->data[2] == 0x4E) && (ba->data[3] == 0x47) && (ba->data[4] == 0x0D) && (ba->data[5] == 0x0A) &&
               (ba->data[6] == 0x1A) && (ba->data[7] == 0x0A)) {
        return "PNG";
    } else if ((ba->data[0] == 'q') && (ba->data[1] == 'o') && (ba->data[2] == 'i') && (ba->data[3] == 'f')) {
        return "QOI";
    } else if ((ba->data[0] == 'B') && (ba->data[1] == 'M')) {
        return "BMP";
    } else if (ba->data[0] == 0x0A) {
        return "PCX";
    } else {
        // try to figure out if TGA
        if (((ba->data[1] == 0) || (ba->data[1] == 1)) &&                                                   // Color map type must be 0 or 1
            (((ba->data[2] >= 1) && (ba->data[2] <= 3)) || ((ba->data[2] >= 9) && (ba->data[2] <= 11)))) {  // image type must be 1, 2, 3, 9, 10 or 11
            return "TGA";
        }
    }
    return NULL;
}
