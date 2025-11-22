/*
 * Copyright (c) 2022, Su Laus  @Su_Laus
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * the author may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of the author.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

/* ===========  Purpose ===============================================
 * Tests the following points:
 *  - Handling of signed tags
 *  - Definition of additional, user-defined tags
 *  - Specification of field name strings or with field_name = NULL
 *  - Prevent reading anonymous tags by specifying them as FIELD_IGNORE
 *    (see https://gitlab.com/libtiff/libtiff/-/issues/532)
 *  - Immediate clearing of the memory for the definition of the additional tags
 *    (allocate memory for TIFFFieldInfo structure and free that memory
 *     immediately after calling TIFFMergeFieldInfo().
 */

#include <memory.h> /* necessary for linux compiler (memset) */
#include <stdio.h>
#include <stdlib.h> /* necessary for linux compiler */

#include "tif_config.h" /* necessary for linux compiler to get HAVE_UNISTD_H */
#ifdef HAVE_UNISTD_H
#include <unistd.h> /* for unlink() on linux */
#endif

#include <tiffio.h>

#define FAULT_RETURN 1
#define OK_RETURN 0
#define GOTOFAILURE goto failure;

#define N(a) (sizeof(a) / sizeof(a[0]))

#define FIELD_IGNORE 0 /* same as FIELD_PSEUDO */

enum
{
    SINT8 = 65100,
    SINT16,
    SINT32,
    SINT64,
    C0_SINT8,
    C0_SINT16,
    C0_SINT32,
    C0_SINT64,
    C16_SINT8,
    C16_SINT16,
    C16_SINT32,
    C16_SINT64,
    C32_SINT8,
    C32_SINT16,
    C32_SINT32,
    C32_SINT64,
    C32_SINT64NULL,
};

static const TIFFFieldInfo tiff_field_info[] = {
    {SINT8, 1, 1, TIFF_SBYTE, FIELD_CUSTOM, 0, 0, "SINT8"},
    {SINT16, 1, 1, TIFF_SSHORT, FIELD_CUSTOM, 0, 0, "SINT16"},
    {SINT32, 1, 1, TIFF_SLONG, FIELD_CUSTOM, 0, 0, "SINT32"},
    {SINT64, 1, 1, TIFF_SLONG8, FIELD_CUSTOM, 0, 0, "SINT64"},
    {C0_SINT8, 6, 6, TIFF_SBYTE, FIELD_CUSTOM, 0, 0, "C0_SINT8"},
    {C0_SINT16, 6, 6, TIFF_SSHORT, FIELD_CUSTOM, 0, 0, "C0_SINT16"},
    {C0_SINT32, 6, 6, TIFF_SLONG, FIELD_CUSTOM, 0, 0, "C0_SINT32"},
    {C0_SINT64, 6, 6, TIFF_SLONG8, FIELD_CUSTOM, 0, 0, "C0_SINT64"},
    {C16_SINT8, TIFF_VARIABLE, TIFF_VARIABLE, TIFF_SBYTE, FIELD_CUSTOM, 0, 1,
     "C16_SINT8"},
    {C16_SINT16, TIFF_VARIABLE, TIFF_VARIABLE, TIFF_SSHORT, FIELD_CUSTOM, 0, 1,
     "C16_SINT16"},
    {C16_SINT32, TIFF_VARIABLE, TIFF_VARIABLE, TIFF_SLONG, FIELD_CUSTOM, 0, 1,
     "C16_SINT32"},
    {C16_SINT64, TIFF_VARIABLE, TIFF_VARIABLE, TIFF_SLONG8, FIELD_CUSTOM, 0, 1,
     "C16_SINT64"},
    {C32_SINT8, TIFF_VARIABLE2, TIFF_VARIABLE2, TIFF_SBYTE, FIELD_CUSTOM, 0, 1,
     "C32_SINT8"},
    {C32_SINT16, TIFF_VARIABLE2, TIFF_VARIABLE2, TIFF_SSHORT, FIELD_CUSTOM, 0,
     1, "C32_SINT16"},
    {C32_SINT32, TIFF_VARIABLE2, TIFF_VARIABLE2, TIFF_SLONG, FIELD_CUSTOM, 0, 1,
     "C32_SINT32"},
    {C32_SINT64, TIFF_VARIABLE2, TIFF_VARIABLE2, TIFF_SLONG8, FIELD_CUSTOM, 0,
     1, "C32_SINT64"},
    /* Test field_name=NULL in static const array, which is now possible because
     * handled within TIFFMergeFieldInfo(). */
    {C32_SINT64NULL, TIFF_VARIABLE2, TIFF_VARIABLE2, TIFF_SLONG8, FIELD_CUSTOM,
     0, 1, NULL},
};

/* Global parameter for the field array to be passed to extender, which can be
 * changed during runtime. */
static TIFFFieldInfo *p_tiff_field_info = (TIFFFieldInfo *)tiff_field_info;
static uint32_t N_tiff_field_info =
    sizeof(tiff_field_info) / sizeof(tiff_field_info[0]);

static TIFFExtendProc parent = NULL;

static void extender(TIFF *tif)
{
    if (p_tiff_field_info != NULL)
    {
        TIFFMergeFieldInfo(tif, p_tiff_field_info, N_tiff_field_info);
        if (parent)
        {
            (*parent)(tif);
        }
    }
    else
    {
        TIFFErrorExtR(tif, "field_info_extender",
                      "Pointer to tiff_field_info array is NULL.");
    }
}

/*-- Global test fields --*/
int8_t s8[] = {-8, -9, -10, -11, INT8_MAX, INT8_MIN};
int16_t s16[] = {-16, -17, -18, -19, INT16_MAX, INT16_MIN};
int32_t s32[] = {-32, -33, -34, -35, INT32_MAX, INT32_MIN};
int64_t s64[] = {-64, -65, -66, -67, INT64_MAX, INT64_MIN};

const uint32_t idxSingle = 0;

static int writeTestTiff(const char *szFileName, int isBigTiff)
{
    int ret;
    TIFF *tif;
    int retcode = FAULT_RETURN;

    unlink(szFileName);
    if (isBigTiff)
    {
        fprintf(stdout, "\n-- Writing signed values to BigTIFF...\n");
        tif = TIFFOpen(szFileName, "w8");
    }
    else
    {
        fprintf(stdout, "\n-- Writing signed values to ClassicTIFF...\n");
        tif = TIFFOpen(szFileName, "w");
    }
    if (!tif)
    {
        fprintf(stdout, "Can't create test TIFF file %s.\n", szFileName);
        return (FAULT_RETURN);
    }

    ret = TIFFSetField(tif, SINT8, s8[idxSingle]);
    if (ret != 1)
    {
        fprintf(stdout, "Error writing SINT8: ret=%d\n", ret);
        GOTOFAILURE;
    }
    ret = TIFFSetField(tif, SINT16, s16[idxSingle]);
    if (ret != 1)
    {
        fprintf(stdout, "Error writing SINT16: ret=%d\n", ret);
        GOTOFAILURE;
    }
    ret = TIFFSetField(tif, SINT32, s32[idxSingle]);
    if (ret != 1)
    {
        fprintf(stdout, "Error writing SINT32: ret=%d\n", ret);
        GOTOFAILURE;
    }

    TIFFSetField(tif, C0_SINT8, &s8);
    TIFFSetField(tif, C0_SINT16, &s16);
    TIFFSetField(tif, C0_SINT32, &s32);

    TIFFSetField(tif, C16_SINT8, 6, &s8);
    TIFFSetField(tif, C16_SINT16, 6, &s16);
    TIFFSetField(tif, C16_SINT32, 6, &s32);

    TIFFSetField(tif, C16_SINT8, 6, &s8);
    TIFFSetField(tif, C16_SINT16, 6, &s16);
    TIFFSetField(tif, C16_SINT32, 6, &s32);

    TIFFSetField(tif, C32_SINT8, 6, &s8);
    TIFFSetField(tif, C32_SINT16, 6, &s16);
    TIFFSetField(tif, C32_SINT32, 6, &s32);

    if (isBigTiff)
    {
        ret = TIFFSetField(tif, SINT64, s64[0]);
        if (ret != 1)
        {
            fprintf(stdout, "Error writing SINT64: ret=%d\n", ret);
            GOTOFAILURE;
        }
        ret = TIFFSetField(tif, C0_SINT64, &s64);
        if (ret != 1)
        {
            fprintf(stdout, "Error writing C0_SINT64: ret=%d\n", ret);
            GOTOFAILURE;
        }
        ret = TIFFSetField(tif, C16_SINT64, N(s64), &s64);
        if (ret != 1)
        {
            fprintf(stdout, "Error writing C16_SINT64: ret=%d\n", ret);
            GOTOFAILURE;
        }
        ret = TIFFSetField(tif, C32_SINT64, N(s64), &s64);
        if (ret != 1)
        {
            fprintf(stdout, "Error writing C32_SINT64: ret=%d\n", ret);
            GOTOFAILURE;
        }
        ret = TIFFSetField(tif, C32_SINT64NULL, N(s64), &s64);
        if (ret != 1)
        {
            fprintf(stdout, "Error writing C32_SINT64NULL: ret=%d\n", ret);
            GOTOFAILURE;
        }
    }

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, 1);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, 1);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
    ret = (int)TIFFWriteEncodedStrip(tif, 0, "\0", 1);
    if (ret != 1)
    {
        fprintf(stdout, "Error TIFFWriteEncodedStrip: ret=%d\n", ret);
        GOTOFAILURE;
    }

    retcode = OK_RETURN;
failure:
    TIFFClose(tif);
    return (retcode);
}

static int readTestTiff(const char *szFileName, int isBigTiff)
{
    int ret;
    int i;
    int8_t s8l, *s8p;
    int16_t s16l, *s16p;
    int32_t s32l, *s32p;
    int64_t s64l, *s64p;
    uint16_t count;
    uint32_t count32;
    int retcode = FAULT_RETURN;

    fprintf(stdout, "-- Reading signed values ...\n");
    TIFF *tif = TIFFOpen(szFileName, "r");
    if (!tif)
    {
        fprintf(stdout, "Can't open test TIFF file %s.\n", szFileName);
        return (FAULT_RETURN);
    }

    ret = TIFFGetField(tif, SINT8, &s8l);
    if (ret != 1)
    {
        fprintf(stdout, "Error reading SINT8: ret=%d\n", ret);
        GOTOFAILURE
    }
    else
    {
        if (s8l != s8[idxSingle])
        {
            fprintf(stdout,
                    "Read value of SINT8  %d differs from set value %d\n", s8l,
                    s8[idxSingle]);
            GOTOFAILURE
        }
    }
    ret = TIFFGetField(tif, SINT16, &s16l);
    if (ret != 1)
    {
        fprintf(stdout, "Error reading SINT16: ret=%d\n", ret);
        GOTOFAILURE
    }
    else
    {
        if (s16l != s16[idxSingle])
        {
            fprintf(stdout,
                    "Read value of SINT16  %d differs from set value %d\n",
                    s16l, s16[idxSingle]);
            GOTOFAILURE
        }
    }
    ret = TIFFGetField(tif, SINT32, &s32l);
    if (ret != 1)
    {
        fprintf(stdout, "Error reading SINT32: ret=%d\n", ret);
        GOTOFAILURE
    }
    else
    {
        if (s32l != s32[idxSingle])
        {
            fprintf(stdout,
                    "Read value of SINT32  %d differs from set value %d\n",
                    s32l, s32[idxSingle]);
            GOTOFAILURE
        }
    }

    ret = TIFFGetField(tif, C0_SINT8, &s8p);
    if (ret != 1)
    {
        fprintf(stdout, "Error reading C0_SINT8: ret=%d\n", ret);
        GOTOFAILURE
    }
    count = N(s8);
    for (i = 0; i < count; i++)
    {
        if (s8p[i] != s8[i])
        {
            fprintf(stdout,
                    "Read value %d of C0_SINT8-Array %d differs from set value "
                    "%d\n",
                    i, s8p[i], s8[i]);
            GOTOFAILURE
        }
    }

    ret = TIFFGetField(tif, C0_SINT16, &s16p);
    if (ret != 1)
    {
        fprintf(stdout, "Error reading C0_SINT16: ret=%d\n", ret);
        GOTOFAILURE
    }
    count = N(s16);
    for (i = 0; i < count; i++)
    {
        if (s16p[i] != s16[i])
        {
            fprintf(stdout,
                    "Read value %d of C0_SINT16-Array %d differs from set "
                    "value %d\n",
                    i, s16p[i], s16[i]);
            GOTOFAILURE
        }
    }

    ret = TIFFGetField(tif, C0_SINT32, &s32p);
    if (ret != 1)
    {
        fprintf(stdout, "Error reading C0_SINT32: ret=%d\n", ret);
        GOTOFAILURE
    }
    count = N(s32);
    for (i = 0; i < count; i++)
    {
        if (s32p[i] != s32[i])
        {
            fprintf(stdout,
                    "Read value %d of C0_SINT32-Array %d differs from set "
                    "value %d\n",
                    i, s32p[i], s32[i]);
            GOTOFAILURE
        }
    }

    s8p = NULL;
    ret = TIFFGetField(tif, C16_SINT8, &count, &s8p);
    if (ret != 1 || s8p == NULL)
    {
        fprintf(stdout,
                "Error reading C16_SINT8: ret=%d; count=%d; pointer=%p\n", ret,
                count, s8p);
        GOTOFAILURE
    }
    else
    {
        for (i = 0; i < count; i++)
        {
            if (s8p[i] != s8[i])
            {
                fprintf(
                    stdout,
                    "Read value %d of s8-Array %d differs from set value %d\n",
                    i, s8p[i], s8[i]);
                GOTOFAILURE
            }
        }
    }

    s16p = NULL;
    ret = TIFFGetField(tif, C16_SINT16, &count, &s16p);
    if (ret != 1 || s16p == NULL)
    {
        fprintf(stdout,
                "Error reading C16_SINT16: ret=%d; count=%d; pointer=%p\n", ret,
                count, s16p);
        GOTOFAILURE
    }
    else
    {
        for (i = 0; i < count; i++)
        {
            if (s16p[i] != s16[i])
            {
                fprintf(stdout,
                        "Read value %d of C16_SINT16-Array %d differs from set "
                        "value %d\n",
                        i, s16p[i], s16[i]);
                GOTOFAILURE
            }
        }
    }

    s32p = NULL;
    ret = TIFFGetField(tif, C16_SINT32, &count, &s32p);
    if (ret != 1 || s32p == NULL)
    {
        fprintf(stdout,
                "Error reading C16_SINT32: ret=%d; count=%d; pointer=%p\n", ret,
                count, s32p);
        GOTOFAILURE
    }
    else
    {
        for (i = 0; i < count; i++)
        {
            if (s32p[i] != s32[i])
            {
                fprintf(stdout,
                        "Read value %d of C16_SINT32-Array %d differs from set "
                        "value %d\n",
                        i, s32p[i], s32[i]);
                GOTOFAILURE
            }
        }
    }

    if (isBigTiff)
    {
        ret = TIFFGetField(tif, SINT64, &s64l);
        if (ret != 1)
        {
            fprintf(stdout, "Error reading SINT64: ret=%d\n", ret);
            GOTOFAILURE
        }
        else
        {
            if (s64l != s64[idxSingle])
            {
                fprintf(stdout,
                        "Read value of SINT64  %" PRIi64
                        " differs from set value %" PRIi64 "\n",
                        s64l, s64[idxSingle]);
                GOTOFAILURE
            }
        }

        s64p = NULL;
        ret = TIFFGetField(tif, C0_SINT64, &s64p);
        count = N(s64);
        if (ret != 1)
        {
            fprintf(stdout, "Error reading C0_SINT64: ret=%d\n", ret);
            GOTOFAILURE
        }
        else
        {
            for (i = 0; i < count; i++)
            {
                if (s64p[i] != s64[i])
                {
                    fprintf(stdout,
                            "Read value %d of C0_SINT64-Array %" PRIi64
                            " differs from set value %" PRIi64 "\n",
                            i, s64p[i], s64[i]);
                    GOTOFAILURE
                }
            }
        }

        s64p = NULL;
        ret = TIFFGetField(tif, C16_SINT64, &count, &s64p);
        if (ret != 1 || s64p == NULL)
        {
            fprintf(stdout,
                    "Error reading C16_SINT64: ret=%d; count=%d; pointer=%p\n",
                    ret, count, s64p);
            GOTOFAILURE
        }
        else
        {
            for (i = 0; i < count; i++)
            {
                if (s64p[i] != s64[i])
                {
                    fprintf(stdout,
                            "Read value %d of C16_SINT64-Array %" PRIi64
                            " differs from set value %" PRIi64 "\n",
                            i, s64p[i], s64[i]);
                    GOTOFAILURE
                }
            }
        }

        s64p = NULL;
        ret = TIFFGetField(tif, C32_SINT64, &count32, &s64p);
        if (ret != 1 || s64p == NULL)
        {
            fprintf(stdout,
                    "Error reading C32_SINT64: ret=%d; count=%d; pointer=%p\n",
                    ret, count, s64p);
            GOTOFAILURE
        }
        else
        {
            for (i = 0; i < (int)count32; i++)
            {
                if (s64p[i] != s64[i])
                {
                    fprintf(stdout,
                            "Read value %d of C32_SINT64-Array %" PRIi64
                            " differs from set value %" PRIi64 "\n",
                            i, s64p[i], s64[i]);
                    GOTOFAILURE
                }
            }
        }
    } /*-- if(isBigTiff) --*/

    retcode = OK_RETURN;
failure:

    fprintf(stdout, "-- End of test. Closing TIFF file. --\n");
    TIFFClose(tif);
    return (retcode);
}
/*-- readTestTiff() --*/

static int readTestTiff_ignore_some_tags(const char *szFileName)
{
    int ret;
    int8_t s8l;
    int16_t s16l;
    int32_t s32l;
    int retcode = FAULT_RETURN;

    /* There is a use case, where LibTIFF shall be prevented from reading
     * unknown tags that are present in the file as anonymous tags. This can be
     * achieved by defining these tags with ".field_bit = FIELD_IGNORE". */

    /* Copy const array to be manipulated and freed just after TIFFMergeFields()
     * within the "extender()" called by TIFFOpen(). */
    TIFFFieldInfo *tiff_field_info2;
    tiff_field_info2 = (TIFFFieldInfo *)malloc(sizeof(tiff_field_info));
    if (tiff_field_info2 == (TIFFFieldInfo *)NULL)
    {
        fprintf(stdout,
                "Can't allocate memoy for tiff_field_info2 structure.\n");
        return (FAULT_RETURN);
    }
    memcpy(tiff_field_info2, tiff_field_info, sizeof(tiff_field_info));
    /* Switch field array for extender callback. */
    p_tiff_field_info = tiff_field_info2;

    /*-- Adapt tiff_field_info array for ignoring unknown tags to LibTIFF, which
     * have been written to file before. --*/
    /* a.) Just set field_bit to FIELD_IGNORE = 0 */
    tiff_field_info2[2].field_bit = FIELD_IGNORE;
    /* b.) Usecase with all field array infos zero but the tag value. */
    ttag_t tag = tiff_field_info2[4].field_tag;
    memset(&tiff_field_info2[4], 0, sizeof(tiff_field_info2[4]));
    tiff_field_info2[4].field_tag = tag;

    fprintf(stdout, "\n-- Reading file with unknown tags to be ignored ...\n");
    TIFF *tif = TIFFOpen(szFileName, "r");

    /* tiff_field_info2 should not be needed anymore, as long as the still
     * active extender() is not called again. Therefore, the extender callback
     * should be disabled by resetting it to the saved one. */
    free(tiff_field_info2);
    tiff_field_info2 = NULL;
    TIFFSetTagExtender(parent);

    if (!tif)
    {
        fprintf(stdout, "Can't open test TIFF file %s.\n", szFileName);
        return (FAULT_RETURN);
    }

    /* Read the first two known tags for testing */
    ret = TIFFGetField(tif, SINT8, &s8l);
    if (ret != 1)
    {
        fprintf(stdout, "Error reading SINT8: ret=%d\n", ret);
        GOTOFAILURE
    }
    else
    {
        if (s8l != s8[idxSingle])
        {
            fprintf(stdout,
                    "Read value of SINT8  %d differs from set value %d\n", s8l,
                    s8[idxSingle]);
            GOTOFAILURE
        }
    }
    ret = TIFFGetField(tif, SINT16, &s16l);
    if (ret != 1)
    {
        fprintf(stdout, "Error reading SINT16: ret=%d\n", ret);
        GOTOFAILURE
    }
    else
    {
        if (s16l != s16[idxSingle])
        {
            fprintf(stdout,
                    "Read value of SINT16  %d differs from set value %d\n",
                    s16l, s16[idxSingle]);
            GOTOFAILURE
        }
    }

    /* The two ignored tags shall not be present. */
    ret = TIFFGetField(tif, tiff_field_info[2].field_tag, &s32l);
    if (ret != 0)
    {
        fprintf(stdout,
                "Error: Tag %d, set to be ignored, has been read from file.\n",
                tiff_field_info[2].field_tag);
        GOTOFAILURE
    }

    ret = TIFFGetField(tif, tiff_field_info[4].field_tag, &s32l);
    if (ret != 0)
    {
        fprintf(stdout,
                "Error: Tag %d, set to be ignored, has been read from file.\n",
                tiff_field_info[4].field_tag);
        GOTOFAILURE
    }

    retcode = OK_RETURN;
failure:

    fprintf(stdout,
            "-- End of test for ignored unknown tags. Closing TIFF file. --\n");
    TIFFClose(tif);
    return (retcode);
}
/*-- readTestTiff_ignore_some_tags() --*/

int main(void)
{
    /*-- Signed tags test --*/
    parent = TIFFSetTagExtender(&extender);
    if (writeTestTiff("temp.tif", 0) != OK_RETURN)
        return (-1);
    if (readTestTiff("temp.tif", 0) != OK_RETURN)
        return (-1);

    if (writeTestTiff("tempBig.tif", 1) != OK_RETURN)
        return (-1);
    if (readTestTiff("tempBig.tif", 1) != OK_RETURN)
        return (-1);
    unlink("tempBig.tif");
    fprintf(stdout, "---------- Signed tag test finished OK -----------\n");

    /*-- Adapt tiff_field_info array for ignoring unknown tags to LibTIFF, which
     * have been written to file. --*/
    if (readTestTiff_ignore_some_tags("temp.tif") != OK_RETURN)
        return (-1);
    unlink("temp.tif");
    fprintf(stdout,
            "---------- Ignoring unknown tag test finished OK -----------\n");

    return 0;
}
