/*
MIT License

Copyright (c) 2019-2020 Andre Seidelt <superilu@yahoo.com>

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
 * @brief check if string ends with suffix.
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
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
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
