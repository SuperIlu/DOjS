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

#include <errno.h>
#include <string.h>

#include "util.h"

/***********************
** exported functions **
***********************/
/**
 * @brief check if the file exists.
 *
 * @param fname file name.
 *
 * @return check_file_t one of the enums.
 */
check_file_t ut_check_file(char *fname) {
    FILE *f = fopen(fname, "r");
    if (f) {
        fclose(f);
        return CF_YES;
    } else {
        if (errno == ENOENT) {
            return CF_NO;
        } else {
            return CF_ERROR;
        }
    }
}

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
