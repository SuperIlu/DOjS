/*
 * TIFF Library
 *
 * The purpose of this test suite is to test the correctness of
 * TIFFWriteDirectory() when appending multiple directories to an open file.
 *
 * Currently, there is an optimization where the TIFF data structure stores the
 * offset of the last written directory in order to avoid having to traverse the
 * entire directory list each time a new one is added. The offset is not stored
 * in the file itself, only in the in-memory data structure. This means we still
 * go through the entire list the first time a directory is appended to a
 * newly-opened file, and the shortcut is taken for subsequent directory writes.
 *
 * In order to test the correctness of the optimization, the
 * `test_lastdir_offset` function writes 10 directories to two different files.
 * For the first file we use the optimization, by simply calling
 * TIFFWriteDirectory() repeatedly on an open TIFF handle. For the second file,
 * we avoid the optimization by closing the file after each call to
 * TIFFWriteDirectory(), which means the next directory write will traverse the
 * entire list.
 *
 * Finally, the two files are compared to check that the number of directories
 * written is the same, and that their offsets match. The test is then repeated
 * using BigTIFF files.
 *
 * Furthermore, arbitrary directory loading using TIFFSetDirectory() is checked,
 * especially after the update with "relative" movement possibility. Also
 * TIFFUnlinkDirectory() is tested.
 *
 * Furthermore, tests are performed with big-endian, little-endian and BigTIFF
 * images.
 *
 * Furthermore the correctness of tif_curdir = TIFFCurrentDirectory() when
 * moving though a multi-IFD file and creating new IFDs, overwriting or
 * re-writing IFDs as well for reading and writing SubIFDs is tested.
 *
 */

#include "tif_config.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tiffio.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/* Global settings for the test image. It is not save to change. */
#define SPP 3            /* Samples per pixel */
#define N_DIRECTORIES 10 /* Number of directories to write */
const uint16_t width = 1;
const uint16_t length = 1;
const uint16_t bps = 8;
const uint16_t photometric = PHOTOMETRIC_RGB;
const uint16_t rows_per_strip = 1;
const uint16_t planarconfig = PLANARCONFIG_CONTIG;

char *openModeStrings[] = {"wl", "wb", "w8l", "w8b"};
char *openModeText[] = {"non-BigTIFF and LE", "non-BigTIFF and BE",
                        "BigTIFF and LE", "BigTIFF and BE"};

/* Some functions and macros to get more readable test code. */
int CheckCurDirNum(TIFF *tif, tdir_t expected_dirnum, int line)
{

    tdir_t curdir = TIFFCurrentDirectory(tif);
    if (curdir != expected_dirnum)
    {
        fprintf(stderr,
                "Error: curdir %d is different to expected one %d at line %d\n",
                curdir, expected_dirnum, line);
        return 1;
    }
    return 0;
}

#define TIFFWriteDirectory_M(tif, filename, line)                              \
    if (!TIFFWriteDirectory(tif))                                              \
    {                                                                          \
        fprintf(stderr, "Can't write directory to %s at line %d\n", filename,  \
                line);                                                         \
        goto failure;                                                          \
    }

#define TIFFCheckpointDirectory_M(tif, dirnum, filename, line)                 \
    if (!TIFFCheckpointDirectory(tif))                                         \
    {                                                                          \
        fprintf(stderr, "Can't checkpoint directory %d of %s at line %d\n",    \
                dirnum, filename, line);                                       \
        goto failure;                                                          \
    }

#define TIFFSetDirectory_M(tif, dirnum, filename, line)                        \
    if (!TIFFSetDirectory(tif, dirnum))                                        \
    {                                                                          \
        fprintf(stderr, "Can't set directory %d of %s at line %d\n", dirnum,   \
                filename, line);                                               \
        goto failure;                                                          \
    }

#define CHECKCURDIRNUM_M(tif, x, line)                                         \
    if (CheckCurDirNum(tif, x, line))                                          \
    {                                                                          \
        goto failure;                                                          \
    }

/* Writes basic tags to current directory (IFD) as well one pixel to the file.
 * For is_corrupted = TRUE a corrupted IFD (missing image width tag) is
 * generated. */
int write_data_to_current_directory(TIFF *tif, int i, bool is_corrupted)
{
    unsigned char buf[SPP] = {0, 127, 255};
    char auxString[128];

    if (!tif)
    {
        fprintf(stderr, "Invalid TIFF handle.\n");
        return 1;
    }
    if (!is_corrupted)
    {
        if (!TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width))
        {
            fprintf(stderr, "Can't set ImageWidth tag.\n");
            return 1;
        }
    }
    if (!TIFFSetField(tif, TIFFTAG_IMAGELENGTH, length))
    {
        fprintf(stderr, "Can't set ImageLength tag.\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bps))
    {
        fprintf(stderr, "Can't set BitsPerSample tag.\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, SPP))
    {
        fprintf(stderr, "Can't set SamplesPerPixel tag.\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, rows_per_strip))
    {
        fprintf(stderr, "Can't set SamplesPerPixel tag.\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_PLANARCONFIG, planarconfig))
    {
        fprintf(stderr, "Can't set PlanarConfiguration tag.\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, photometric))
    {
        fprintf(stderr, "Can't set PhotometricInterpretation tag.\n");
        return 1;
    }
    /* Write IFD identification number to ASCII string of PageName tag. */
    sprintf(auxString, "%d th. IFD", i);
    if (!TIFFSetField(tif, TIFFTAG_PAGENAME, auxString))
    {
        fprintf(stderr, "Can't set TIFFTAG_PAGENAME tag.\n");
        return 1;
    }

    /* Write dummy pixel data. */
    if (TIFFWriteScanline(tif, buf, 0, 0) == -1 && !is_corrupted)
    {
        fprintf(stderr, "Can't write image data.\n");
        return 1;
    }

    return 0;
}

/* Fills a new IFD and appends it to a file by opening a file and closing it
 * again after writing. */
int write_directory_to_closed_file(const char *filename, unsigned int openMode,
                                   int i)
{
    TIFF *tif;
    /* Replace 'w' for write by 'a' for append. */
    char strAux[8] = {0};
    strncpy(strAux, openModeStrings[openMode], sizeof(strAux) - 1);
    strAux[0] = 'a';
    tif = TIFFOpen(filename, strAux);
    if (!tif)
    {
        fprintf(stderr, "Can't create/open %s\n", filename);
        return 1;
    }

    if (write_data_to_current_directory(tif, i, false))
    {
        fprintf(stderr, "Can't write data to directory %d of %s.\n", i,
                filename);
        TIFFClose(tif);
        return 1;
    }

    if (!TIFFWriteDirectory(tif))
    {
        fprintf(stderr, "TIFFWriteDirectory() failed for directory %d of %s.\n",
                i, filename);
        TIFFClose(tif);
        return 1;
    }

    TIFFClose(tif);
    return 0;
}

/* Opens a file and counts its directories. */
int count_directories(const char *filename, int *count)
{
    TIFF *tif;
    *count = 0;

    tif = TIFFOpen(filename, "r");
    if (!tif)
    {
        fprintf(stderr, "Can't read %s\n", filename);
        return 1;
    }

    do
    {
        CHECKCURDIRNUM_M(tif, (tdir_t)(*count), __LINE__);
        (*count)++;
    } while (TIFFReadDirectory(tif));
failure:
    TIFFClose(tif);
    return 0;
}

/* Compare 'requested_dir_number' with number written in PageName tag
 * into the IFD to identify that IFD.  */
int is_requested_directory(TIFF *tif, int requested_dir_number,
                           const char *filename)
{
    char *ptr = NULL;
    char *auxStr = NULL;

    if (!TIFFGetField(tif, TIFFTAG_PAGENAME, &ptr))
    {
        fprintf(stderr, "Can't get TIFFTAG_PAGENAME tag.\n");
        return 0;
    }

    /* Check for reading errors */
    if (ptr != NULL)
        auxStr = strchr(ptr, ' ');

    if (ptr == NULL || auxStr == NULL || strncmp(auxStr, " th.", 4))
    {
        ptr = ptr == NULL ? "(null)" : ptr;
        fprintf(stderr,
                "Error reading IFD directory number from PageName tag: %s\n",
                ptr);
        return 0;
    }

    /* Retrieve IFD identification number from ASCII string */
    const int nthIFD = atoi(ptr);
    if (nthIFD == requested_dir_number)
    {
        return 1;
    }
    fprintf(stderr, "Expected directory %d from %s was not loaded but: %s\n",
            requested_dir_number, filename, ptr);

    return 0;
}

enum DirWalkMode
{
    DirWalkMode_ReadDirectory,
    DirWalkMode_SetDirectory,
    DirWalkMode_SetDirectory_Reverse,
};
/* Gets a list of the directory offsets in a file. Assumes the file has at least
 * N_DIRECTORIES directories.
 * There are three methods to test walking through the IFD chain.
 * This tests the optimization of faster SetDirectory(). */
int get_dir_offsets(const char *filename, uint64_t *offsets,
                    enum DirWalkMode dirWalkMode)
{
    TIFF *tif;
    int i;

    tif = TIFFOpen(filename, "r");
    if (!tif)
    {
        fprintf(stderr, "Can't read %s\n", filename);
        return 1;
    }

    for (i = 0; i < N_DIRECTORIES; i++)
    {
        tdir_t dirn = (dirWalkMode == DirWalkMode_SetDirectory_Reverse)
                          ? (N_DIRECTORIES - i - 1)
                          : i;

        if (dirWalkMode != DirWalkMode_ReadDirectory &&
            !TIFFSetDirectory(tif, dirn) && dirn < (N_DIRECTORIES - 1))
        {
            fprintf(stderr, "Can't set %d.th directory from %s\n", i, filename);
            TIFFClose(tif);
            return 3;
        }

        offsets[dirn] = TIFFCurrentDirOffset(tif);

        /* Check, if dirn is requested directory */
        if (!is_requested_directory(tif, dirn, filename))
        {
            TIFFClose(tif);
            return 4;
        }

        if (dirWalkMode == DirWalkMode_ReadDirectory &&
            !TIFFReadDirectory(tif) && i < (N_DIRECTORIES - 1))
        {
            fprintf(stderr, "Can't read %d.th directory from %s\n", i,
                    filename);
            TIFFClose(tif);
            return 2;
        }
    }

    TIFFClose(tif);
    return 0;
}

/* Checks that TIFFSetDirectory() work well after update for relative seeking
 * to following directories.
 *
 * There are several issues especially when SubIFDs and custom directories are
 * involved. There are no real directory number for those and TIFFSetDirectory()
 * cannot be used. However, TIFFSetDirectory() is needed to switch back to the
 * main-IFD chain. Furthermore, IFD-loop handling needs to be supported in any
 * cases.
 * Also the case where directly after TIFFWriteDirectory() that directory
 * is re-read using TIFFSetDirectory() is tested.
 */
int test_arbitrary_directrory_loading(unsigned int openMode)
{
    char filename[128] = {0};
    TIFF *tif;
    uint64_t offsets_base[N_DIRECTORIES];
    int expected_original_dirnumber;
    tdir_t expected_curdir = (tdir_t)(-1);

    if (openMode >= (sizeof(openModeStrings) / sizeof(openModeStrings[0])))
    {
        fprintf(stderr, "Index %d for openMode parameter out of range.\n",
                openMode);
        return 1;
    }

    /* Get individual filenames and delete existent ones. */
    sprintf(filename, "test_arbitrary_directrory_loading_%s.tif",
            openModeStrings[openMode]);
    unlink(filename);

    /* Create a file and write N_DIRECTORIES (10) directories to it. */
    tif = TIFFOpen(filename, openModeStrings[openMode]);
    if (!tif)
    {
        fprintf(stderr, "Can't create %s\n", filename);
        return 1;
    }
    TIFFSetDirectory(tif, 0);
    CHECKCURDIRNUM_M(tif, expected_curdir, __LINE__);
    for (int i = 0; i < N_DIRECTORIES; i++)
    {
        if (write_data_to_current_directory(tif, i, false))
        {
            fprintf(stderr, "Can't write data to current directory in %s\n",
                    filename);
            goto failure;
        }
        if (!TIFFWriteDirectory(tif))
        {
            fprintf(stderr, "Can't write directory to %s\n", filename);
            goto failure;
        }
        expected_curdir++;
        /* Fails in 4.6.0 */
        CHECKCURDIRNUM_M(tif, expected_curdir, __LINE__);
        if (i >= 2 && i <= 4)
        {
            if (i == 3)
            {
                /* Invalidate directory - TIFFSetSubDirectory() will fail.
                 * The next TIFFSetDirectory(i) will revoke this action.*/
                if (TIFFSetSubDirectory(tif, 0))
                {
                    fprintf(stderr,
                            "Unexpected return at invalidate directory %d "
                            "within %s.\n",
                            i, filename);
                    goto failure;
                }
            }
            /* Test jump back to just written directory. */
            if (!TIFFSetDirectory(tif, i))
            {
                fprintf(stderr,
                        "Can't set directory %d within %s  "
                        "right after TIFFWriteDirectory().\n",
                        i, filename);
                goto failure;
            }
            if (i == 4)
            {
                /* Invalidate directory - TIFFSetSubDirectory() will fail.
                 * This time, tif_curdir keeps set to -1 and counts now again
                 * from 0, despite the directory number in the file is equal
                 * "i". */
                if (TIFFSetSubDirectory(tif, 0))
                {
                    fprintf(stderr,
                            "Unexpected return at invalidate directory %d "
                            "within %s.\n",
                            i, filename);
                    goto failure;
                }
            }
            /*Check if correct directory is loaded */
            if (!is_requested_directory(tif, i, filename))
            {
                goto failure;
            }
            /* Reset to next directory to go on with writing. */
            TIFFCreateDirectory(tif);
        }
    }
    TIFFClose(tif);

    /* Reopen prepared testfile */
    tif = TIFFOpen(filename, "r+");
    if (!tif)
    {
        fprintf(stderr, "Can't create %s\n", filename);
        return 1;
    }

    /* Get directory offsets */
    if (get_dir_offsets(filename, offsets_base, DirWalkMode_ReadDirectory))
    {
        fprintf(stderr, "Error reading directory offsets from %s.\n", filename);
        goto failure;
    }

    /* Set last directory and then one after, which should fail. */
    if (!TIFFSetDirectory(tif, N_DIRECTORIES - 1))
    {
        fprintf(stderr, "Can't set last directory %d within %s\n",
                N_DIRECTORIES - 1, filename);
        goto failure;
    }
    if (TIFFSetDirectory(tif, N_DIRECTORIES + 1))
    {
        fprintf(stderr,
                "End of IFD chain not detected. Set non existing directory %d "
                "within %s\n",
                N_DIRECTORIES + 1, filename);
        goto failure;
    }
    CHECKCURDIRNUM_M(tif, N_DIRECTORIES - 1, __LINE__);

    /* Test very fast  TIFFSetDirectory() using IFD loop directory list.
     * First populate IFD loop directory list and then go through directories in
     * reverse order. Within between read after end of IFDs using
     * TIFFReadDirectory() where IFD loop directory list should be kept. */
    for (int i = 0; i < N_DIRECTORIES; i++)
    {
        if (!TIFFSetDirectory(tif, i))
        {
            fprintf(stderr, "Can't set %d.th directory from %s\n", i, filename);
            goto failure;
        }
        CHECKCURDIRNUM_M(tif, i, __LINE__);
    }
    TIFFReadDirectory(tif);
    CHECKCURDIRNUM_M(tif, N_DIRECTORIES - 1, __LINE__);
    for (int i = N_DIRECTORIES - 1; i >= 0; i--)
    {
        if (!TIFFSetDirectory(tif, i))
        {
            fprintf(stderr, "Can't set %d.th directory from %s\n", i, filename);
            goto failure;
        }
        if (!is_requested_directory(tif, i, filename))
        {
            goto failure;
        }
        CHECKCURDIRNUM_M(tif, i, __LINE__);
    }

    /* Test not existing directory number */
    if (TIFFSetDirectory(tif, N_DIRECTORIES))
    {
        fprintf(stderr,
                "No expected fail for accessing not existent directory number "
                "%d in file %s\n",
                N_DIRECTORIES, filename);
        goto failure;
    }
    CHECKCURDIRNUM_M(tif, 0, __LINE__);

    /* Close and Reopen prepared testfile */
    TIFFClose(tif);
    tif = TIFFOpen(filename, "r+");
    if (!tif)
    {
        fprintf(stderr, "Can't create %s\n", filename);
        return 1;
    }

    /* Step through directories just using TIFFSetSubDirectory() */
    for (int i = N_DIRECTORIES - 1; i >= 0; i--)
    {
        if (!TIFFSetSubDirectory(tif, offsets_base[i]))
        {
            fprintf(stderr, "Can't set %d.th directory from %s\n", i, filename);
            goto failure;
        }
        if (!is_requested_directory(tif, i, filename))
        {
            goto failure;
        }
        CHECKCURDIRNUM_M(tif, i, __LINE__);
    }

    /* More specialized test cases for relative seeking within TIFFSetDirectory.
     * However, with using IFD loop directory list, most of this test cases will
     * never be reached! */
    if (!TIFFSetDirectory(tif, 2))
    {
        fprintf(stderr, "Can't set directory %d within %s\n", 2, filename);
        goto failure;
    }
    CHECKCURDIRNUM_M(tif, 2, __LINE__);
    uint64_t off2 = TIFFCurrentDirOffset(tif);
    /* Note that dirnum = 2 is deleted here since TIFFUnlinkDirectory()
     * starts with 1 instead of 0. */
    if (!TIFFUnlinkDirectory(tif, 3))
    {
        fprintf(stderr, "Can't unlink directory %d within %s\n", 3, filename);
        goto failure;
    }
    /* Move to the unlinked IFD. This sets tif_curdir=0 because unlinked IFD
     * offset is not in the IFD loop list. This indicates a SubIFD chain. */
    if (!TIFFSetSubDirectory(tif, off2))
    {
        fprintf(stderr,
                "Can't set sub-directory at offset 0x%" PRIx64 " (%" PRIu64
                ") within %s\n",
                off2, off2, filename);
        goto failure;
    }
    /*Check if correct directory is loaded */
    CHECKCURDIRNUM_M(tif, 0, __LINE__);
    expected_original_dirnumber = 2;
    if (!is_requested_directory(tif, expected_original_dirnumber, filename))
    {
        goto failure;
    }
    /* This should move back to the main-IFD chain and load the third
     * directory which has IFD number 4, due to the deleted third IFD. */
    if (!TIFFSetDirectory(tif, 3))
    {
        fprintf(stderr, "Can't set new directory %d within %s\n", 3, filename);
        goto failure;
    }
    CHECKCURDIRNUM_M(tif, 3, __LINE__);
    expected_original_dirnumber = 4;
    if (!is_requested_directory(tif, expected_original_dirnumber, filename))
    {
        goto failure;
    }
    /* Test backwards jump. */
    if (!TIFFSetDirectory(tif, 2))
    {
        fprintf(stderr, "Can't set new directory %d within %s\n", 2, filename);
        goto failure;
    }
    expected_original_dirnumber = 3;
    if (!is_requested_directory(tif, expected_original_dirnumber, filename))
    {
        goto failure;
    }

    /* Second UnlinkDirectory -> two IFDs are missing in the main-IFD chain
     * then, original dirnum 2 and 3 */
    if (!TIFFUnlinkDirectory(tif, 3))
    {
        fprintf(stderr, "Can't unlink directory %d within %s\n", 3, filename);
        goto failure;
    }
    if (!TIFFSetDirectory(tif, 2))
    {
        fprintf(stderr,
                "Can't set new directory %d after second "
                "TIFFUnlinkDirectory(3) within %s\n",
                2, filename);
        goto failure;
    }
    CHECKCURDIRNUM_M(tif, 2, __LINE__);
    /* which should now be the previous dir-3. */
    expected_original_dirnumber = 4;
    if (!is_requested_directory(tif, expected_original_dirnumber, filename))
    {
        goto failure;
    }

    /* Check, if third original directory, which is unlinked, could be loaded
     * and the following, still chained one. This is like for a SubIFD.
     * tif_curdir will be set to 0 because deleted IFD is not in main sequence
     * and then incremented to 1 when reading the still chained next IFD using
     * TIFFReadDirectory(). */
    if (!TIFFSetSubDirectory(tif, offsets_base[2]))
    {
        fprintf(stderr,
                "Can't set sub-directory at offset 0x%" PRIx64 " (%" PRIu64
                ") within %s\n",
                offsets_base[2], offsets_base[2], filename);
        goto failure;
    }
    CHECKCURDIRNUM_M(tif, 0, __LINE__);
    if (!TIFFReadDirectory(tif))
    {
        fprintf(stderr,
                "Can't read directory after directory at offset 0x%" PRIx64
                " (%" PRIu64 ") within %s\n",
                offsets_base[2], offsets_base[2], filename);
        goto failure;
    }
    /* Check if correct directory is loaded, which was unlinked the second
     * time.
     */
    CHECKCURDIRNUM_M(tif, 1, __LINE__);
    expected_original_dirnumber = 3;
    if (!is_requested_directory(tif, expected_original_dirnumber, filename))
    {
        goto failure;
    }

    /* Load unlinked directory like a SubIFD and then go back to the
     * main-IFD chain using TIFFSetDirectory(). Also check two loads of the
     * same directory. */
    if (!TIFFSetSubDirectory(tif, offsets_base[2]))
    {
        fprintf(stderr,
                "Can't set sub-directory at offset 0x%" PRIx64 " (%" PRIu64
                ") within %s\n",
                offsets_base[2], offsets_base[2], filename);
        goto failure;
    }
    CHECKCURDIRNUM_M(tif, 0, __LINE__);
    if (!TIFFSetDirectory(tif, 3))
    {
        fprintf(stderr, "Can't set new directory %d within %s\n", 3, filename);
        goto failure;
    }
    if (!TIFFSetDirectory(tif, 3))
    {
        fprintf(stderr, "Can't set new directory %d a second time within %s\n",
                3, filename);
        goto failure;
    }
    CHECKCURDIRNUM_M(tif, 3, __LINE__);
    /*Check if correct directory is loaded. Because two original IFDs are
     * unlinked / missing, the original dirnumber is now 5. */
    expected_original_dirnumber = 5;
    if (!is_requested_directory(tif, expected_original_dirnumber, filename))
    {
        goto failure;
    }

    /* Another sequence for testing. */
    if (!TIFFSetDirectory(tif, 2))
    {
        fprintf(stderr, "Can't set new directory %d a second time within %s\n",
                2, filename);
        goto failure;
    }
    if (!TIFFSetDirectory(tif, 3))
    {
        fprintf(stderr, "Can't set new directory %d a second time within %s\n",
                3, filename);
        goto failure;
    }
    if (!TIFFSetSubDirectory(tif, offsets_base[2]))
    {
        fprintf(stderr,
                "Can't set sub-directory at offset 0x%" PRIx64 " (%" PRIu64
                ") within %s\n",
                offsets_base[2], offsets_base[2], filename);
        goto failure;
    }
    expected_original_dirnumber = 2;
    if (!is_requested_directory(tif, expected_original_dirnumber, filename))
    {
        goto failure;
    }
    if (!TIFFSetDirectory(tif, 3))
    {
        fprintf(stderr, "Can't set new directory %d a second time within %s\n",
                3, filename);
        goto failure;
    }
    /*Check if correct directory is loaded. Because two original IFDs are
     * unlinked / missing, the original dirnumber is now 5. */
    expected_original_dirnumber = 5;
    if (!is_requested_directory(tif, expected_original_dirnumber, filename))
    {
        goto failure;
    }

    /* Third UnlinkDirectory -> three IFDs are missing in the main-IFD chain
     * then, original dirnum 0, 2 and 3
     * Furthermore, this test checks that TIFFUnlinkDirectory() can unlink
     * the first directory dirnum = 0 and a following TIFFSetDirectory(0)
     * does not load the unlinked directory. */
    CHECKCURDIRNUM_M(tif, 3, __LINE__);
    if (!TIFFUnlinkDirectory(tif, 1))
    {
        fprintf(stderr, "Can't unlink directory %d within %s\n", 0, filename);
        goto failure;
    }
    CHECKCURDIRNUM_M(tif, (tdir_t)(-1), __LINE__);
    /* Now three directories are missing (0,2,3) and thus directory 0 is
     * original directory 1 and directory 2 is original directory 5. */
    if (!TIFFSetDirectory(tif, 0))
    {
        fprintf(stderr,
                "Can't set new directory %d after third "
                "TIFFUnlinkDirectory(1) within %s\n",
                0, filename);
        goto failure;
    }
    CHECKCURDIRNUM_M(tif, 0, __LINE__);
    expected_original_dirnumber = 1;
    if (!is_requested_directory(tif, expected_original_dirnumber, filename))
    {
        goto failure;
    }
    if (!TIFFSetDirectory(tif, 2))
    {
        fprintf(stderr,
                "Can't set new directory %d after third "
                "TIFFUnlinkDirectory(1) within %s\n",
                2, filename);
        goto failure;
    }
    expected_original_dirnumber = 5;
    if (!is_requested_directory(tif, expected_original_dirnumber, filename))
    {
        goto failure;
    }

    /* TIFFUnlinkDirectory(0) is not allowed, because dirnum starts for
     * this function with 1 instead of 0.
     * An error return is expected here. */
    CHECKCURDIRNUM_M(tif, 2, __LINE__);
    fprintf(stderr, "----- Expect error messages about 'TIFFUnlinkDirectory() "
                    "first directory starts with number 1 and not 0.' -----\n");
    if (TIFFUnlinkDirectory(tif, 0))
    {
        fprintf(stderr, "TIFFUnlinkDirectory(0) did not return an error.\n");
        goto failure;
    }
    CHECKCURDIRNUM_M(tif, 2, __LINE__);

    TIFFClose(tif);
    unlink(filename);
    return 0;

failure:
    if (tif)
    {
        TIFFClose(tif);
    }
    return 1;
} /*-- test_arbitrary_directrory_loading() --*/

/* Tests SubIFD writing and reading
 *
 *
 */
int test_SubIFD_directrory_handling(unsigned int openMode)
{
    char filename[128] = {0};

/* Define the number of sub-IFDs you are going to write */
#define NUMBER_OF_SUBIFDs 4
    uint16_t number_of_sub_IFDs = NUMBER_OF_SUBIFDs;
    toff_t sub_IFDs_offsets[NUMBER_OF_SUBIFDs] = {
        0UL}; /* array for SubIFD tag */
    int blnWriteSubIFD = 0;
    int i;
    int iIFD = 0, iSubIFD = 0;
    TIFF *tif;
    int expected_original_dirnumber;

    if (openMode >= (sizeof(openModeStrings) / sizeof(openModeStrings[0])))
    {
        fprintf(stderr, "Index %d for openMode parameter out of range.\n",
                openMode);
        return 1;
    }

    /* Get individual filenames and delete existent ones. */
    sprintf(filename, "test_SubIFD_directrory_handling_%s.tif",
            openModeStrings[openMode]);
    unlink(filename);

    /* Create a file and write N_DIRECTORIES (10) directories to it */
    tif = TIFFOpen(filename, openModeStrings[openMode]);
    if (!tif)
    {
        fprintf(stderr, "Can't create %s\n", filename);
        return 1;
    }
    for (i = 0; i < N_DIRECTORIES; i++)
    {
        if (write_data_to_current_directory(
                tif, blnWriteSubIFD ? 200 + iSubIFD++ : iIFD++, false))
        {
            fprintf(stderr, "Can't write data to current directory in %s\n",
                    filename);
            goto failure;
        }
        if (blnWriteSubIFD)
        {
            /* SUBFILETYPE tag is not mandatory for SubIFD writing, but a
             * good idea to indicate thumbnails */
            if (!TIFFSetField(tif, TIFFTAG_SUBFILETYPE, FILETYPE_REDUCEDIMAGE))
                goto failure;
        }

        expected_original_dirnumber = blnWriteSubIFD ? 0 : iIFD - 1;

        /* For the second multi-page image, trigger TIFFWriteDirectory() to
         * switch for the next number_of_sub_IFDs calls to add those as SubIFDs
         * e.g. for thumbnails */
        if (1 == i)
        {
            blnWriteSubIFD = 1;
            /* Now here is the trick: the next n directories written
             * will be sub-IFDs of the main-IFD (where n is number_of_sub_IFDs
             * specified when you set the TIFFTAG_SUBIFD field.
             * The contents of sub_IFDs_offsets is initially copied to the
             * internal SubIFD offset array and written to the main-IFD. Those
             * SubIFD offsets in the main-IFD are then patched by the following
             * number_of_sub_IFDs TIFFWriteDirectory() calls which write the
             * sub-IFDs to file.
             */
            if (!TIFFSetField(tif, TIFFTAG_SUBIFD, number_of_sub_IFDs,
                              sub_IFDs_offsets))
                goto failure;
        }

        if (!TIFFWriteDirectory(tif))
        {
            fprintf(stderr, "Can't write directory to %s\n", filename);
            goto failure;
        }
        /* Fails in 4.6.0 */
        CHECKCURDIRNUM_M(tif, expected_original_dirnumber, __LINE__);

        if (iSubIFD >= number_of_sub_IFDs)
            blnWriteSubIFD = 0;
    }
    TIFFClose(tif);

    /* Reopen prepared testfile */
    tif = TIFFOpen(filename, "r+");
    if (!tif)
    {
        fprintf(stderr, "Can't create %s\n", filename);
        goto failure;
    }

    tdir_t numberOfMainIFDs = TIFFNumberOfDirectories(tif);
    CHECKCURDIRNUM_M(tif, 0, __LINE__);
    if (numberOfMainIFDs != (tdir_t)N_DIRECTORIES - number_of_sub_IFDs)
    {
        fprintf(stderr,
                "Unexpected number of directories in %s. Expected %i, "
                "found %i.\n",
                filename, N_DIRECTORIES - number_of_sub_IFDs, numberOfMainIFDs);
        goto failure;
    }

    tdir_t currentDirNumber = TIFFCurrentDirectory(tif);

    /* The first directory is already read through TIFFOpen() */
    int blnRead = 0;
    expected_original_dirnumber = 1;
    do
    {
        /* Check if there are SubIFD subfiles */
        void *ptr;
        if (TIFFGetField(tif, TIFFTAG_SUBIFD, &number_of_sub_IFDs, &ptr))
        {
            /* Copy SubIFD array from pointer */
            memcpy(sub_IFDs_offsets, ptr,
                   number_of_sub_IFDs * sizeof(sub_IFDs_offsets[0]));

            for (i = 0; i < number_of_sub_IFDs; i++)
            {
                /* Read first SubIFD directory */
                if (!TIFFSetSubDirectory(tif, sub_IFDs_offsets[i]))
                    goto failure;
                if (!is_requested_directory(tif, 200 + i, filename))
                {
                    goto failure;
                }
                /* For SubIFDs, curdir is always set to 0 for the first SubIFD
                 * read.
                 * When SubIFDs are not concatenated, curdir is 0 for all
                 * SubIFDs read.
                 * When the SubIFDs are concatenated the following
                 * ones after the first read one get incremented curdir numbers.
                 * The curdir sequence gets updated when a previous one before
                 * the firstly read one is read. I.e. curdir behaves in the
                 * SubIFD chain like in the main IFD chain to support IFD loop
                 * checking.
                 */
                /* Check if there is a SubIFD chain behind the first one from
                 * the array, as specified by Adobe */
                int n = 0;
                /* subdirnum starts with 1 because first SubIFD is already
                 * read with TIFFSetSubDirectory() */
                CHECKCURDIRNUM_M(tif, 0, __LINE__);
                tdir_t subdirnum = 1;
                /* analyse SubIFD chain */
                while (TIFFReadDirectory(tif))
                {
                    if (!is_requested_directory(tif, 201 + i + n++, filename))
                        goto failure;
                    /* Fails in 4.6.0 when SubIFD chaining is enabled. */
                    CHECKCURDIRNUM_M(tif, subdirnum, __LINE__);
                    subdirnum++;
                }
            }
            /*-- Go through SubIFDs in reverse order --*/
            /* Activate main-IFD to reset SubIFD loop list from above. */
            TIFFSetDirectory(tif, 0);
            for (i = number_of_sub_IFDs - 1; i >= 0; i--)
            {
                if (!TIFFSetSubDirectory(tif, sub_IFDs_offsets[i]))
                    goto failure;
                if (!is_requested_directory(tif, 200 + i, filename))
                {
                    goto failure;
                }
                int n = 0;
                tdir_t subdirnum = 1;
                CHECKCURDIRNUM_M(tif, 0, __LINE__);
                /* analyse SubIFD chain */
                while (TIFFReadDirectory(tif))
                {
                    if (!is_requested_directory(tif, 201 + i + n++, filename))
                        goto failure;
                    /* Fails in 4.6.0 when SubIFD chaining is enabled. */
                    CHECKCURDIRNUM_M(tif, subdirnum, __LINE__);
                    subdirnum++;
                }
            }
            /* Go back to main-IFD chain and re-read that main-IFD directory */
            if (!TIFFSetDirectory(tif, currentDirNumber))
                goto failure;
        }
        /* Read next main-IFD directory (subfile) */
        blnRead = TIFFReadDirectory(tif);
        currentDirNumber = TIFFCurrentDirectory(tif);
        if (blnRead)
            CHECKCURDIRNUM_M(tif, expected_original_dirnumber, __LINE__);
        if (blnRead && !is_requested_directory(
                           tif, expected_original_dirnumber++, filename))
            goto failure;
    } while (blnRead);

    /*--- Now test arbitrary directory loading with SubIFDs ---*/
    if (!TIFFSetSubDirectory(tif, sub_IFDs_offsets[1]))
        goto failure;
    if (!is_requested_directory(tif, 201, filename))
    {
        goto failure;
    }
    CHECKCURDIRNUM_M(tif, 0, __LINE__);

    /*-- Test TIFFCheckpointDirectory() with SubIFDs --
     * However, SubIFDs cannot be re-written to another location because
     * TIFFRewriteDirectory() does not support SubIFDs. Overwriting works. */
    tdir_t expected_curdir = TIFFNumberOfDirectories(tif);
    TIFFCreateDirectory(tif);
    if (write_data_to_current_directory(tif, expected_curdir, false))
    {
        fprintf(stderr,
                "Can't write data to current directory in %s at line %d\n",
                filename, __LINE__);
        goto failure;
    }
    /* prepare to write next directory as SubIFD */
    number_of_sub_IFDs = 1;
    if (!TIFFSetField(tif, TIFFTAG_SUBIFD, number_of_sub_IFDs,
                      sub_IFDs_offsets))
        goto failure;
    TIFFWriteDirectory_M(tif, filename, __LINE__);
    /* Fails in 4.6.0 */
    CHECKCURDIRNUM_M(tif, expected_curdir, __LINE__);
    /* write SubIFD */
    if (write_data_to_current_directory(tif, 700, false))
    {
        fprintf(stderr, "Can't write data to current directory in %s\n",
                filename);
        goto failure;
    }
    /* SUBFILETYPE tag is not mandatory for SubIFD writing, but a
     * good idea to indicate thumbnails */
    if (!TIFFSetField(tif, TIFFTAG_SUBFILETYPE, FILETYPE_REDUCEDIMAGE))
        goto failure;

    /* Checkpoint SubIFD test */
    TIFFCheckpointDirectory_M(tif, expected_curdir, filename, __LINE__);
    /* The next four CHECKKURDIRNUM_M fail in 4.6.0 */
    CHECKCURDIRNUM_M(tif, 0, __LINE__);
    /* Overwriting works but re-writing will not. */
    TIFFWriteDirectory_M(tif, filename, __LINE__);
    CHECKCURDIRNUM_M(tif, 0, __LINE__);
    /* Write another main-IFD */
    expected_curdir++;
    if (write_data_to_current_directory(tif, expected_curdir, false))
    {
        fprintf(stderr, "Can't write data to current directory in %s\n",
                filename);
        goto failure;
    }
    TIFFCheckpointDirectory_M(tif, expected_curdir, filename, __LINE__);
    CHECKCURDIRNUM_M(tif, expected_curdir, __LINE__);
    TIFFWriteDirectory_M(tif, filename, __LINE__);
    CHECKCURDIRNUM_M(tif, expected_curdir, __LINE__);

    TIFFClose(tif);
    unlink(filename);
    return 0;

failure:
    if (tif)
    {
        TIFFClose(tif);
    }
    return 1;
} /*--- test_SubIFD_directrory_handling() ---*/

/* Test failure in TIFFSetSubDirectory() (see issue #618 and MR !543)
 *
 * If int TIFFSetSubDirectory(TIFF *tif, uint64_t diroff) fails due to
 * TIFFReadDirectory() error, subsequent calls to TIFFSetDirectory() cannot
 * recover a consistent state. This is reproducible in master, 4.6, 4.5.1.
 * This issue surfaced around the time when relative seeking
 * support / IFD hash map optimiazions were introduced. This can be reproduced
 * by opening an invalid SubIFD directory (e.g. w/o required ImageLength tag).
 * The issue was fixed by MR !543.
 */
int test_SetSubDirectory_failure(unsigned int openMode)
{
    char filename[128] = {0};

/* Define the number of sub-IFDs you are going to write */
#define NUMBER_OF_SUBIFDs2 1
    uint16_t number_of_sub_IFDs = NUMBER_OF_SUBIFDs2;
    toff_t sub_IFDs_offsets[NUMBER_OF_SUBIFDs2] = {
        0UL}; /* array for SubIFD tag */
    TIFF *tif;
    tdir_t expected_curdir = 0;

    if (openMode >= (sizeof(openModeStrings) / sizeof(openModeStrings[0])))
    {
        fprintf(stderr, "Index %d for openMode parameter out of range.\n",
                openMode);
        return 1;
    }

    /* Get individual filenames and delete existent ones. */
    sprintf(filename, "test_SetSubDirectory_failure_%s.tif",
            openModeStrings[openMode]);
    unlink(filename);

    /* Create a file and write one directory with one corrupted sub-directory to
     * it */
    tif = TIFFOpen(filename, openModeStrings[openMode]);
    if (!tif)
    {
        fprintf(stderr, "Can't create %s\n", filename);
        return 1;
    }

    if (write_data_to_current_directory(tif, 300, false))
    {
        fprintf(stderr, "Can't write data to current directory in %s\n",
                filename);
        goto failure;
    }
    /* prepare to write next directory as SubIFD */
    if (!TIFFSetField(tif, TIFFTAG_SUBIFD, number_of_sub_IFDs,
                      sub_IFDs_offsets))
        goto failure;
    if (!TIFFWriteDirectory(tif))
    {
        fprintf(stderr, "Can't write directory to %s\n", filename);
        goto failure;
    }
    CHECKCURDIRNUM_M(tif, expected_curdir, __LINE__);
    /* write corrupted SubIFD */
    fprintf(stderr, "----- Expect some error messages about 'scanline size is "
                    "zero' -----\n");
    if (write_data_to_current_directory(tif, 310, true))
    {
        fprintf(stderr, "Can't write data to current directory in %s\n",
                filename);
        goto failure;
    }
    /* SUBFILETYPE tag is not mandatory for SubIFD writing, but a
     * good idea to indicate thumbnails */
    if (!TIFFSetField(tif, TIFFTAG_SUBFILETYPE, FILETYPE_REDUCEDIMAGE))
        goto failure;

    if (!TIFFWriteDirectory(tif))
    {
        fprintf(stderr, "Can't write directory to %s\n", filename);
        goto failure;
    }
    CHECKCURDIRNUM_M(tif, expected_curdir, __LINE__);
    TIFFClose(tif);

    /* Reopen prepared testfile */
    tif = TIFFOpen(filename, "r");
    if (!tif)
    {
        fprintf(stderr, "Can't open %s\n", filename);
        goto failure;
    }
    /* The first directory is already read through TIFFOpen() */
    if (!is_requested_directory(tif, 300, filename))
    {
        goto failure;
    }
    CHECKCURDIRNUM_M(tif, expected_curdir, __LINE__);

    /* Check if there are SubIFD subfiles */
    void *ptr;
    if (TIFFGetField(tif, TIFFTAG_SUBIFD, &number_of_sub_IFDs, &ptr))
    {
        /* Copy SubIFD array from pointer */
        memcpy(sub_IFDs_offsets, ptr,
               number_of_sub_IFDs * sizeof(sub_IFDs_offsets[0]));

        /* Read first SubIFD directory */
        if (!TIFFSetSubDirectory(tif, sub_IFDs_offsets[0]))
        {
            /* Attempt to set back to the main directory.
             * Would fail if absolute seeking is not forced independent of
             * TIFFReadDirectory success. */
            if (!TIFFSetDirectory(tif, 0))
            {
                fprintf(stderr,
                        "Failed to reset from not valid SubIFD back to "
                        "main directory. %s\n",
                        filename);
                goto failure;
            }
            if (!is_requested_directory(tif, 300, filename))
            {
                goto failure;
            }
        }
        else
        {
            if (!is_requested_directory(tif, 310, filename))
            {
                goto failure;
            }
        }
        /* Go back to main-IFD chain and re-read that main-IFD directory */
        if (!TIFFSetDirectory(tif, 0))
            goto failure;
        CHECKCURDIRNUM_M(tif, 0, __LINE__);
    }

    TIFFClose(tif);
    unlink(filename);
    return 0;

failure:
    if (tif)
    {
        TIFFClose(tif);
    }
    return 1;
} /*--- test_SetSubDirectory_failure() ---*/

/* Checks that rewriting a directory does not break the directory linked
 * list.
 *
 * This could happen because TIFFRewriteDirectory() relies on the traversal of
 * the directory linked list in order to move the rewritten directory to the
 * end of the file. This means the `lastdir_offset` optimization should be
 * skipped, otherwise the linked list will be broken at the point where it
 * connected to the rewritten directory, resulting in the loss of the
 * directories that come after it.
 * Rewriting the first directory requires an additional test, because it is
 * treated differently from the directories that have a predecessor in the list.
 */
int test_rewrite_lastdir_offset(unsigned int openMode)
{
    char filename[128] = {0};
    int i, count;
    TIFF *tif;

    if (openMode >= (sizeof(openModeStrings) / sizeof(openModeStrings[0])))
    {
        fprintf(stderr, "Index %d for openMode parameter out of range.\n",
                openMode);
        return 1;
    }

    /* Get individual filenames and delete existent ones. */
    sprintf(filename, "test_directory_rewrite_%s.tif",
            openModeStrings[openMode]);
    unlink(filename);

    /* Create a file and write N_DIRECTORIES (10) directories to it. */
    tif = TIFFOpen(filename, openModeStrings[openMode]);
    if (!tif)
    {
        fprintf(stderr, "Can't create %s\n", filename);
        return 1;
    }
    for (i = 0; i < N_DIRECTORIES; i++)
    {
        if (write_data_to_current_directory(tif, i, false))
        {
            fprintf(stderr, "Can't write data to current directory in %s\n",
                    filename);
            goto failure;
        }
        if (!TIFFWriteDirectory(tif))
        {
            fprintf(stderr, "Can't write directory to %s\n", filename);
            goto failure;
        }
    }

    /* Without closing the file, go to the fifth directory
     * and check, if dirn is requested directory. */
    TIFFSetDirectory(tif, 4);
    if (!is_requested_directory(tif, 4, filename))
    {
        TIFFClose(tif);
        return 4;
    }

    /* Rewrite the fifth directory by calling TIFFRewriteDirectory
     * and check, if the offset of IFD loaded by TIFFSetDirectory() is
     * different. Then, the IFD loop directory list is correctly maintained for
     * speed up of TIFFSetDirectory() with directly getting the offset that
     * list.
     */
    uint64_t off1 = TIFFCurrentDirOffset(tif);
    if (write_data_to_current_directory(tif, 4, false))
    {
        fprintf(stderr, "Can't write data to fifth directory in %s\n",
                filename);
        goto failure;
    }
    if (!TIFFRewriteDirectory(tif))
    {
        fprintf(stderr, "Can't rewrite fifth directory to %s\n", filename);
        goto failure;
    }
    i = 4;
    if (!TIFFSetDirectory(tif, i))
    {
        fprintf(stderr, "Can't set %d.th directory from %s\n", i, filename);
        goto failure;
    }
    uint64_t off2 = TIFFCurrentDirOffset(tif);
    if (!is_requested_directory(tif, i, filename))
    {
        goto failure;
    }
    if (off1 == off2)
    {
        fprintf(stderr,
                "Rewritten directory %d was not correctly accessed by "
                "TIFFSetDirectory() in file %s\n",
                i, filename);
        goto failure;
    }

    /* Now, perform the test for the first directory */
    TIFFSetDirectory(tif, 0);
    if (!is_requested_directory(tif, 0, filename))
    {
        TIFFClose(tif);
        return 5;
    }
    off1 = TIFFCurrentDirOffset(tif);
    if (write_data_to_current_directory(tif, 0, false))
    {
        fprintf(stderr, "Can't write data to first directory in %s\n",
                filename);
        goto failure;
    }
    if (!TIFFRewriteDirectory(tif))
    {
        fprintf(stderr, "Can't rewrite first directory to %s\n", filename);
        goto failure;
    }
    i = 0;
    if (!TIFFSetDirectory(tif, i))
    {
        fprintf(stderr, "Can't set %d.th directory from %s\n", i, filename);
        goto failure;
    }
    off2 = TIFFCurrentDirOffset(tif);
    if (!is_requested_directory(tif, i, filename))
    {
        goto failure;
    }
    if (off1 == off2)
    {
        fprintf(stderr,
                "Rewritten directory %d was not correctly accessed by "
                "TIFFSetDirectory() in file %s\n",
                i, filename);
        goto failure;
    }

    TIFFClose(tif);
    tif = NULL;

    /* Check that the file has the expected number of directories*/
    if (count_directories(filename, &count))
    {
        fprintf(stderr, "Error counting directories in file %s.\n", filename);
        goto failure;
    }
    if (count != N_DIRECTORIES)
    {
        fprintf(stderr,
                "Unexpected number of directories in %s. Expected %i, "
                "found %i.\n",
                filename, N_DIRECTORIES, count);
        goto failure;
    }
    /* hint: file was closed by count_directories() */
    unlink(filename);
    return 0;

failure:
    if (tif)
    {
        TIFFClose(tif);
    }
    return 1;
} /*-- test_rewrite_lastdir_offset() --*/

/* Compares multi-directory files written with and without the lastdir
 * optimization */
int test_lastdir_offset(unsigned int openMode)
{
    char filename_optimized[128] = {0};
    char filename_non_optimized[128] = {0};
    int i, count_optimized, count_non_optimized;
    uint64_t offsets_base[N_DIRECTORIES];
    uint64_t offsets_comparison[N_DIRECTORIES];
    TIFF *tif;

    if (openMode >= (sizeof(openModeStrings) / sizeof(openModeStrings[0])))
    {
        fprintf(stderr, "Index %d for openMode parameter out of range.\n",
                openMode);
        return 1;
    }

    /* Get individual filenames and delete existent ones. */
    sprintf(filename_optimized, "test_directory_optimized_%s.tif",
            openModeStrings[openMode]);
    sprintf(filename_non_optimized, "test_directory_non_optimized_%s.tif",
            openModeStrings[openMode]);
    unlink(filename_optimized);
    unlink(filename_non_optimized);

    /* First file: open it and add multiple directories. This uses the
     * lastdir optimization. */
    tif = TIFFOpen(filename_optimized, openModeStrings[openMode]);
    if (!tif)
    {
        fprintf(stderr, "Can't create %s\n", filename_optimized);
        return 1;
    }
    for (i = 0; i < N_DIRECTORIES; i++)
    {
        if (write_data_to_current_directory(tif, i, false))
        {
            fprintf(stderr, "Can't write data to current directory in %s\n",
                    filename_optimized);
            goto failure;
        }
        if (!TIFFWriteDirectory(tif))
        {
            fprintf(stderr, "Can't write directory to %s\n",
                    filename_optimized);
            goto failure;
        }
    }
    TIFFClose(tif);
    tif = NULL;

    /* Second file: close it after adding each directory. This avoids the
     * lastdir optimization. */
    for (i = 0; i < N_DIRECTORIES; i++)
    {
        if (write_directory_to_closed_file(filename_non_optimized, openMode, i))
        {
            fprintf(stderr, "Can't write directory to %s\n",
                    filename_non_optimized);
            goto failure;
        }
    }

    /* Check that both files have the expected number of directories */
    if (count_directories(filename_optimized, &count_optimized))
    {
        fprintf(stderr, "Error counting directories in file %s.\n",
                filename_optimized);
        goto failure;
    }
    if (count_optimized != N_DIRECTORIES)
    {
        fprintf(stderr,
                "Unexpected number of directories in %s. Expected %i, "
                "found %i.\n",
                filename_optimized, N_DIRECTORIES, count_optimized);
        goto failure;
    }
    if (count_directories(filename_non_optimized, &count_non_optimized))
    {
        fprintf(stderr, "Error counting directories in file %s.\n",
                filename_non_optimized);
        goto failure;
    }
    if (count_non_optimized != N_DIRECTORIES)
    {
        fprintf(stderr,
                "Unexpected number of directories in %s. Expected %i, "
                "found %i.\n",
                filename_non_optimized, N_DIRECTORIES, count_non_optimized);
        goto failure;
    }

    /* Check that both files have the same directory offsets */
    /* In parallel of comparing offsets between optimized base file and
     * non-optimized file, test also three methods of walking through the
     * IFD chain within get_dir_offsets(). This tests the optimization of
     * faster SetDirectory(). */
    if (get_dir_offsets(filename_optimized, offsets_base,
                        DirWalkMode_ReadDirectory))
    {
        fprintf(stderr, "Error reading directory offsets from %s.\n",
                filename_optimized);
        goto failure;
    }
    for (int file_i = 0; file_i < 2; ++file_i)
    {
        const char *filename =
            (file_i == 0) ? filename_optimized : filename_non_optimized;

        for (enum DirWalkMode mode = DirWalkMode_ReadDirectory;
             mode <= DirWalkMode_SetDirectory_Reverse; ++mode)
        {
            if (get_dir_offsets(filename, offsets_comparison, mode))
            {
                fprintf(stderr,
                        "Error reading directory offsets from %s in mode %d.\n",
                        filename, mode);
                goto failure;
            }
            for (i = 0; i < N_DIRECTORIES; i++)
            {
                if (offsets_base[i] != offsets_comparison[i])
                {
                    fprintf(stderr,
                            "Unexpected directory offset for directory %i, "
                            "expected "
                            "offset %" PRIu64 " but got %" PRIu64 ".\n",
                            i, offsets_base[i], offsets_comparison[i]);
                    goto failure;
                }
            }
        }
    }
    /* hint: files are always closed by count_directories() and
     * get_dir_offsets() */
    unlink(filename_optimized);
    unlink(filename_non_optimized);
    return 0;

failure:
    if (tif)
    {
        TIFFClose(tif);
    }
    return 1;
} /*-- test_lastdir_offset() --*/

/* Adds an EXIF IFD with some default values to the active IFD.
 */
int write_EXIF_data_to_current_directory(TIFF *tif, int i,
                                         uint64_t *dir_offset_EXIF)
{
    char auxString[128];

    if (!tif)
    {
        fprintf(stderr, "Invalid TIFF handle.\n");
        return 1;
    }

    /*-- Now create an EXIF directory. */
    if (TIFFCreateEXIFDirectory(tif) != 0)
    {
        fprintf(stderr, "TIFFCreateEXIFDirectory() failed.\n");
        return 1;
    }

    if (!TIFFSetField(tif, EXIFTAG_EXIFVERSION, "0231"))
    {
        fprintf(stderr, "Can't write EXIFTAG_EXIFVERSION\n");
        return 1;
    }

    /* Write IFD identification number to ASCII string of SUBSECTIMEORIGINAL
     * tag. */
    sprintf(auxString, "EXIF-%d-EXIF", i);
    if (!TIFFSetField(tif, EXIFTAG_SUBSECTIMEORIGINAL, auxString))
    {
        fprintf(stderr, "Can't set EXIFTAG_SUBSECTIMEORIGINAL tag.\n");
        return 1;
    }

    if (!TIFFWriteCustomDirectory(tif, dir_offset_EXIF))
    {
        fprintf(stderr, "TIFFWriteCustomDirectory() with EXIF failed.\n");
        return 1;
    }

    return 0;
} /*-- write_EXIF_data_to_current_directory() --*/

/* Test correct incrementing of tif_curdir TIFFCurrentDirectory()
 *
 * Up to 4.6.0 there are cases where tif_curdir is set inconsistently or
 * incorrectly, depending on the previous history. This function shows some of
 * these cases and tests them after the improvements.
 */
int test_current_dirnum_incrementing(int testcase, unsigned int openMode)
{
#define N_DIRECTORIES_2 2
    char filename[128] = {0};
    tdir_t lastdir;
    tdir_t expected_curdir = (tdir_t)(-1);

    if (openMode >= (sizeof(openModeStrings) / sizeof(openModeStrings[0])))
    {
        fprintf(stderr, "Index %d for openMode parameter out of range.\n",
                openMode);
        return 1;
    }

    /* Get individual filenames and delete existent ones. */
    sprintf(filename, "test_current_dirnum_incrementing_%s.tif",
            openModeStrings[openMode]);
    unlink(filename);

    /* Create a file and write N_DIRECTORIES_2 directories to it. */
    TIFF *tif = TIFFOpen(filename, openModeStrings[openMode]);
    if (!tif)
    {
        fprintf(stderr, "Can't create %s\n", filename);
        return 1;
    }

    /*-- Standard case for writing sequential IFDs. --*/
    for (int i = 0; i < N_DIRECTORIES_2; i++)
    {
        if (write_data_to_current_directory(tif, i, false))
        {
            fprintf(stderr,
                    "Can't write data to current directory in %s at %d\n",
                    filename, __LINE__);
            goto failure;
        }
        TIFFWriteDirectory_M(tif, filename, __LINE__);
        expected_curdir++;
        CHECKCURDIRNUM_M(tif, expected_curdir, __LINE__);
    }

    /*-- Standard case for sequential reading up to the end. --*/
    expected_curdir = 0;
    TIFFSetDirectory_M(tif, expected_curdir, filename, __LINE__);
    while (TIFFReadDirectory(tif))
    {
        expected_curdir++;
        CHECKCURDIRNUM_M(tif, expected_curdir, __LINE__);
    }
    /* Last read for next_offset 0 shall not increment tif_curdir. */
    CHECKCURDIRNUM_M(tif, expected_curdir, __LINE__);

    /* Now, go back to IFD 0 and overwrite it in the first loop and rewrite it
     * in the second loop (due to "i" having two digits then).
     * tif_curdir should stay to that IFD-number.  */
    expected_curdir = 0;
    for (unsigned int i = 9; i < 11; i++)
    {
        TIFFSetDirectory_M(tif, expected_curdir, filename, __LINE__);
        if (write_data_to_current_directory(tif, i, false))
        {
            fprintf(stderr,
                    "Can't write data to current directory in %s at %d\n",
                    filename, __LINE__);
            goto failure;
        }
        TIFFWriteDirectory_M(tif, filename, __LINE__);
        /* Fails in 4.6.0 */
        CHECKCURDIRNUM_M(tif, expected_curdir, __LINE__);
    }

    /* Now, go back to IFD 1 and overwrite it in the first loop and rewrite it
     * in the second loop (due to "i" having two digits then).
     * tif_curdir should stay to that IFD-number.  */
    expected_curdir = 1;
    for (unsigned int i = 8; i < 13; i += 4)
    {
        TIFFSetDirectory_M(tif, expected_curdir, filename, __LINE__);
        if (write_data_to_current_directory(tif, i, false))
        {
            fprintf(stderr,
                    "Can't write data to current directory in %s at %d\n",
                    filename, __LINE__);
            goto failure;
        }
        TIFFWriteDirectory_M(tif, filename, __LINE__);
        /* Fails in 4.6.0 */
        CHECKCURDIRNUM_M(tif, expected_curdir, __LINE__);
    }

    /* Set IFD 0 and append new IFDs.  */
    if (testcase == 0)
    {
        expected_curdir = TIFFNumberOfDirectories(tif);
    }
    else if (testcase == 1)
    {
        TIFFClose(tif);
        tif = TIFFOpen(filename, openModeStrings[openMode]);
        expected_curdir = 0;
    }
    else if (testcase == 2)
    {
        expected_curdir = TIFFNumberOfDirectories(tif);
        TIFFClose(tif);
        /* Replace 'w' for write by 'a' for append. */
        char strAux[8] = {0};
        strncpy(strAux, openModeStrings[openMode], sizeof(strAux) - 1);
        strAux[0] = 'a';
        tif = TIFFOpen(filename, strAux);
    }
    for (unsigned int setdir = 0; setdir < 3; setdir += 2)
    {
        for (unsigned int i = 0; i < 2; i++)
        {
            if (testcase == 0)
            {
                TIFFSetDirectory_M(tif, setdir, filename, __LINE__);
                TIFFCreateDirectory(tif);
            }
            if (write_data_to_current_directory(tif, expected_curdir, false))
            {
                fprintf(
                    stderr,
                    "Can't write data to current directory in %s at line %d\n",
                    filename, __LINE__);
                goto failure;
            }
            TIFFWriteDirectory_M(tif, filename, __LINE__);
            /* Fails in 4.6.0, except for testcase TIFFOpen("w") */
            CHECKCURDIRNUM_M(tif, expected_curdir, __LINE__);
            expected_curdir++;
        }
    }

    /*-- Several CheckpointDirectory() testing --*/
    /* Set state such as if the last directory would have been just written.
     * - read last IFD from file and then
     * - create and activate a newly initialized IFD, not written yet. */
    if (testcase == 0)
    {
        lastdir = TIFFNumberOfDirectories(tif) - 1;
        expected_curdir = lastdir + 1;
        TIFFSetDirectory(tif, lastdir);
        TIFFCreateDirectory(tif);
    }
    else if (testcase == 1)
    {
        TIFFClose(tif);
        tif = TIFFOpen(filename, openModeStrings[openMode]);
        expected_curdir = 0;
    }
    /* Fill the new IFD and checkpoint several times before writing. */
    if (write_data_to_current_directory(tif, expected_curdir, false))
    {
        fprintf(stderr, "Can't write data to current directory in %s at %d\n",
                filename, __LINE__);
        goto failure;
    }
    for (unsigned int i = 0; i < 2; i++)
    {
        TIFFCheckpointDirectory_M(tif, expected_curdir, filename, __LINE__);
        /* Fails in 4.6.0 only in second loop */
        CHECKCURDIRNUM_M(tif, expected_curdir, __LINE__);
        if (i == 1)
        {
            /* In last loop force IFD to be rewritten. */
            TIFFSetField(tif, TIFFTAG_PAGENAME, "6.th IFD rewritten");
        }
        TIFFCheckpointDirectory_M(tif, expected_curdir, filename, __LINE__);
        /* Fails in 4.6.0 */
        CHECKCURDIRNUM_M(tif, expected_curdir, __LINE__);
        TIFFWriteDirectory_M(tif, filename, __LINE__);
        /* Fails in 4.6.0 */
        CHECKCURDIRNUM_M(tif, expected_curdir, __LINE__);
        /* Prepare checkpointing of same IFD in next loop after reading it from
         * file. */
        TIFFSetDirectory_M(tif, expected_curdir, filename, __LINE__);
    }

    /*-- RewriteDirectory() testing --*/
    /* RewriteDirectory() forces to write the IFD to another location
     * and set-up a new, empty IFD. */
    /* Set state such as if the last directory would have been just written.
     * - read last IFD from file and then
     * - create and activate a newly initialized IFD, not written yet. */
    if (testcase == 0 || testcase == 2)
    {
        lastdir = TIFFNumberOfDirectories(tif) - 1;
    }
    else
    {
        lastdir = expected_curdir;
    }
    TIFFSetDirectory(tif, lastdir);
    TIFFCreateDirectory(tif);
    expected_curdir = lastdir + 1;
    /* Write a new IFD using TIFFRewriteDirectory, read it and rewrite it again.
     * First with curdir set to the last IFD and then set to the first one. */
    for (unsigned int i = 0; i < 2; i++)
    {
        if (write_data_to_current_directory(tif, expected_curdir, false))
        {
            fprintf(stderr,
                    "Can't write data to current directory in %s at %d\n",
                    filename, __LINE__);
            goto failure;
        }
        if (!TIFFRewriteDirectory(tif))
        {
            fprintf(stderr, "Can't rewrite directory to %s at line %d\n",
                    filename, __LINE__);
            goto failure;
        }
        /* Fails in 4.6.0 only in second loop */
        CHECKCURDIRNUM_M(tif, expected_curdir, __LINE__);
        /* Read IFD and write it to another location. */
        TIFFSetDirectory_M(tif, expected_curdir, filename, __LINE__);
        if (!TIFFRewriteDirectory(tif))
        {
            fprintf(stderr, "Can't rewrite directory to %s at line %d\n",
                    filename, __LINE__);
            goto failure;
        }
        /* Fails in 4.6.0 */
        CHECKCURDIRNUM_M(tif, expected_curdir, __LINE__);
        expected_curdir++;
        /* Second loop starts with IFD 0 read from file. */
        TIFFSetDirectory_M(tif, 0, filename, __LINE__);
        TIFFCreateDirectory(tif);
    }

    /*-- CustomDirectory testing --*/
    /* Set state such as if the last directory would have been just written.
     * - Read last IFD from file and then
     * - create and activate a newly initialized IFD, not written yet and
     *   attach an EXIF IFD to it with forcing a re-write of the IFD by
     *   adding the EXIF-IFD tag.
     * - Do that two times:
     *     - first  set last  IFD and append a new one,
     *     - second set first IFD and append a new one. */
    if (testcase == 0 || testcase == 1)
    {
        lastdir = TIFFNumberOfDirectories(tif) - 1;
    }
    else
    {
        lastdir = expected_curdir - 1;
    }
    TIFFSetDirectory(tif, lastdir);
    TIFFCreateDirectory(tif);
    expected_curdir = lastdir + 1;
    uint64_t offsetEXIFBase[2];
    for (unsigned int i = 0; i < 2; i++)
    {
        if (write_data_to_current_directory(tif, expected_curdir, false))
        {
            fprintf(stderr,
                    "Can't write data to current directory in %s at %d\n",
                    filename, __LINE__);
            goto failure;
        }
        TIFFWriteDirectory_M(tif, filename, __LINE__);
        /* Fails in 4.6.0 only in second loop */
        CHECKCURDIRNUM_M(tif, expected_curdir, __LINE__);
        if (write_EXIF_data_to_current_directory(tif, i, &offsetEXIFBase[i]))
        {
            fprintf(stderr,
                    "Can't write EXIF data to current directory in %s. "
                    "Testline %d\n",
                    filename, __LINE__);
            goto failure;
        }
        /* CustomDirectory invalidates directory number. */
        CHECKCURDIRNUM_M(tif, (tdir_t)(-1), __LINE__);
        /* Go back to current main-IFD and update EXIF-IFD offset. */
        TIFFSetDirectory_M(tif, expected_curdir, filename, __LINE__);
        if (!TIFFSetField(tif, TIFFTAG_EXIFIFD, offsetEXIFBase[i]))
        {
            fprintf(stderr, "Can't set TIFFTAG_EXIFIFD tag. Testline %d\n",
                    __LINE__);
            goto failure;
        }
        TIFFWriteDirectory_M(tif, filename, __LINE__);
        /* Fails in 4.6.0 */
        CHECKCURDIRNUM_M(tif, expected_curdir, __LINE__);
        expected_curdir++;

        /* Second loop starts with IFD 0 read from file. */
        TIFFSetDirectory_M(tif, 0, filename, __LINE__);
        TIFFCreateDirectory(tif);
    }

    /*-- Read EXIF Custom-Directory --*/
    for (unsigned int i = 0; i < 2; i++)
    {
        if (!TIFFReadEXIFDirectory(tif, offsetEXIFBase[i]))
        {
            fprintf(stderr,
                    "TIFFReadEXIFDirectory(%" PRIu64 ") failed for %s "
                    "at %d\n",
                    offsetEXIFBase[i], filename, __LINE__);
            goto failure;
        }
        CHECKCURDIRNUM_M(tif, 0, __LINE__);
    }

    /*-- Test for fail of ReadDirectory() and fail of SetDirectory() --*/
    /*-- Writing IFD0, IFD1 as valid IFD and IFD2 as corrupted. --*/
    uint64_t offsetBase[4];
    TIFFClose(tif);
    tif = TIFFOpen(filename, openModeStrings[openMode]);
    expected_curdir = 0;
    fprintf(stderr, "----- Expect some error messages about 'scanline size is "
                    "zero' -----\n");
    for (int i = 0; i < 3; i++)
    {
        if (write_data_to_current_directory(tif, i, (i == 2 ? true : false)))
        {
            fprintf(stderr,
                    "Can't write data to current directory in %s at %d\n",
                    filename, __LINE__);
            goto failure;
        }
        TIFFWriteDirectory_M(tif, filename, __LINE__);
        /* Expect offsetBase{0,0,0} here, because TIFFWriteDirectory() returns
         * a new and empty IFD after writing. */
        offsetBase[i] = TIFFCurrentDirOffset(tif);
        CHECKCURDIRNUM_M(tif, expected_curdir, __LINE__);
        expected_curdir++;
    }
    TIFFSetDirectory_M(tif, 0, filename, __LINE__);
    offsetBase[0] = TIFFCurrentDirOffset(tif);
    CHECKCURDIRNUM_M(tif, 0, __LINE__);
    TIFFSetDirectory_M(tif, 1, filename, __LINE__);
    offsetBase[1] = TIFFCurrentDirOffset(tif);
    CHECKCURDIRNUM_M(tif, 1, __LINE__);
    /* TIFFReadDirectory() shall fail but curdir shall be incremented,
     * because parts of corrupt IFD2 are read in. */
    if (TIFFReadDirectory(tif))
    {
        fprintf(stderr,
                "TIFFReadDirectory() for IFD2 was expected to fail but "
                "succeeded for %s "
                "at %d\n",
                filename, __LINE__);
        goto failure;
    }
    offsetBase[2] = TIFFCurrentDirOffset(tif);
    CHECKCURDIRNUM_M(tif, 2, __LINE__);
    TIFFSetDirectory_M(tif, 0, filename, __LINE__);
    CHECKCURDIRNUM_M(tif, 0, __LINE__);
    /* TIFFSetDirectory() shall fail on IFD2 but curdir shall be incremented,
     * because parts of corrupt IFD2 are read in. */
    if (TIFFSetDirectory(tif, 2))
    {
        fprintf(stderr,
                "TIFFSetDirectory(2) was expected to fail but succeeded for %s "
                "at %d\n",
                filename, __LINE__);
        goto failure;
    }
    CHECKCURDIRNUM_M(tif, 2, __LINE__);
    /* Also TIFFSetSubDirectory() shall fail for IFD2 and curdir shall be
     * set correctly to 2. Therefore, set it back to IFD0 first. */
    TIFFSetDirectory_M(tif, 0, filename, __LINE__);
    CHECKCURDIRNUM_M(tif, 0, __LINE__);
    if (TIFFSetSubDirectory(tif, offsetBase[2]))
    {
        fprintf(stderr,
                "TIFFSetSubDirectory(%" PRIu64
                ") was expected to fail but succeeded for %s "
                "at %d\n",
                offsetBase[2], filename, __LINE__);
        goto failure;
    }
    /* Fails in 4.6.0 */
    CHECKCURDIRNUM_M(tif, 2, __LINE__);

    /* However, TIFFSetSubDirectory() on a non valid IFD "offset" shall fail,
     * and tif_curdir shall be set to "unknown" (-1). */
    fprintf(stderr, "----- Expect error messages about 'Sanity check AND "
                    "Failed to read directory' -----\n");
    if (TIFFSetSubDirectory(tif, 10))
    {
        fprintf(stderr,
                "TIFFSetSubDirectory(%" PRIu64
                ") was expected to fail but succeeded for %s "
                "at %d\n",
                offsetBase[1], filename, __LINE__);
        goto failure;
    }
    /* Fails in 4.6.0 */
    CHECKCURDIRNUM_M(tif, (tdir_t)(-1), __LINE__);
    /* Set IFD0 for next test */
    TIFFSetSubDirectory(tif, offsetBase[0]);
    CHECKCURDIRNUM_M(tif, 0, __LINE__);

    /* TIFFSetSubDirectory(0) shall invalidate the active IFD read from file by
     * setting tif_nextdiroff and tif_diroff=0 */
    TIFFSetSubDirectory(tif, 0);
    CHECKCURDIRNUM_M(tif, (tdir_t)(-1), __LINE__);

    /*-- Patch offset of IFD2 to not existing IFD3 without entries.
     *   Thus TIFFFetchDirectory() will fail. --*/
#define TIFFReadFile_M(tif, buf, size)                                         \
    ((*TIFFGetReadProc(tif))(TIFFClientdata(tif), (buf), (size)));
#define TIFFWriteFile_M(tif, buf, size)                                        \
    ((*TIFFGetWriteProc(tif))(TIFFClientdata(tif), (buf), (size)));
#define TIFFSeekFile_M(tif, off, whence)                                       \
    ((*TIFFGetSeekProc(tif))(TIFFClientdata(tif), (off), (whence)));

    /* ---------------------------------------------------------------------
     * Test IFD index incrementing in case the functions return with certain
     * errors. To provoke that errors, the file is patched by writing bytes
     * directly into the file. Therefore, code below does only handle
     * Classic-TIFF and little-endian files.
     * The code works also on big endian machines, which have to swap some
     * directly read/written values.
     * --------------------------------------------------------------------- */
    if (!(TIFFIsBigEndian(tif) || TIFFIsBigTIFF(tif)))
    {
        /* Patch nextIFDOffset of IFD2, which is 0, with offset to itself.
         * This generates an IFD3 without any elements at the end of file.
         * Reading IFD3 should provoke reading error. */
        uint64_t ss = TIFFSeekFile_M(tif, offsetBase[2], 0);
        uint16_t cnt = 0;
        uint64_t rr = TIFFReadFile_M(tif, &cnt, 2);
        if (TIFFIsByteSwapped(tif))
            TIFFSwabShort(&cnt);
        ss = TIFFSeekFile_M(tif, offsetBase[2] + cnt * 12 + 2, 0);
        uint32_t wt = (uint32_t)ss;
        if (TIFFIsByteSwapped(tif))
            TIFFSwabLong(&wt);
        rr = TIFFWriteFile_M(tif, &wt, 4);
        (void)rr;

        /* Now there are offsets to four IFDs in the file, where the last one is
         * not existing and has a non-valid dircount and entries behind EOF.
         * (dircount is 458 (as offset) */
        fprintf(stderr, "----- Expect error messages about 'Error fetching "
                        "directory link.' -----\n");
        /* TIFFNumberOfDirectories() returns 3 and omits the invalid fourth IFD.
         */
        lastdir = TIFFNumberOfDirectories(tif);
        TIFFSetDirectory(tif, 0);
        CHECKCURDIRNUM_M(tif, 0, __LINE__);

        /* TIFFSetDirectory(3) fails with error messages:
         *   TIFFFetchDirectory: test_current_dirnum_incrementing_wl.tif:
         *   Can not read TIFF directory.
         *   TIFFReadDirectory: Failed to read directory at offset 458. */
        fprintf(stderr, "----- Expect error messages about 'Cannot read TIFF "
                        "directory.' -----\n");
        if (TIFFSetDirectory(tif, 3))
        {
            fprintf(stderr,
                    "TIFFSetDirectory(3) for IFD3 was expected to fail but "
                    "succeeded for %s "
                    "at %d\n",
                    filename, __LINE__);
            goto failure;
        }

        /* Fails in 4.6.0 */
        /* Reading invalid IFD 3 leads to an error and was not read in.
         * Therefore, curdir shall be 65535 (non-existing directory) */
        CHECKCURDIRNUM_M(tif, (tdir_t)(-1), __LINE__);
        offsetBase[3] = TIFFCurrentDirOffset(tif);

        /* Point IFD3 to a location within the file, where it has now for
         * little-endian TIFF files a non-valid dircount=0, which leads also to
         * an error and the IFD is not read in. */
        ss = TIFFSeekFile_M(tif, offsetBase[2] + cnt * 12 + 2, 0);
        wt = (uint32_t)(offsetBase[1] + 8);
        // wt = (uint32_t)(ss + 400);
        if (TIFFIsByteSwapped(tif))
            TIFFSwabLong(&wt);
        rr = TIFFWriteFile_M(tif, &wt, 4);

        fprintf(stderr, "----- Expect error messages about 'Error fetching "
                        "directory link.' -----\n");
        /* TIFFNumberOfDirectories() returns now 4, because for an IFD linked
         * list dircount=0 is not treated as an error and there is an offset
         * (=1) to a next IFD. Then, at the fifth IFD a link error occurs. */
        lastdir = TIFFNumberOfDirectories(tif);
        TIFFSetDirectory(tif, 0);
        CHECKCURDIRNUM_M(tif, 0, __LINE__);

        /* TIFFSetDirectory(3) fails with error messages:
         *    test_current_dirnum_incrementing_wl.tif: Failed to allocate
         *    memory for to read TIFF directory (0 elements of 12 bytes each).
         *    TIFFReadDirectory: Failed to read directory at offset 178.
         * The IFD 3 is not read in and curdir is set to 65535 (non-existing
         * directory).
         */
        fprintf(stderr,
                "----- Expect error messages about 'Failed to allocate "
                "memory for to read TIFF directory.' AND 'Failed to read "
                "directory ..' -----\n");
        if (TIFFSetDirectory(tif, 3))
        {
            fprintf(stderr,
                    "TIFFSetDirectory(3) for IFD3 was expected to fail but "
                    "succeeded for %s "
                    "at %d\n",
                    filename, __LINE__);
            goto failure;
        }

        /* Fails in 4.6.0 */
        CHECKCURDIRNUM_M(tif, (tdir_t)(-1), __LINE__);
    }

    TIFFClose(tif);
    unlink(filename);
    return 0;

failure:
    if (tif)
    {
        TIFFClose(tif);
    }
    return 1;
} /*-- test_current_dirnum_incrementing() --*/

/* Test correct setting of tif_curdircount.
 *
 */
int test_curdircount_setting(unsigned int openMode)
{
#define N_DIRECTORIES_2 2
    char filename[128] = {0};
    tdir_t expected_curdir = (tdir_t)(-1);

    if (openMode >= (sizeof(openModeStrings) / sizeof(openModeStrings[0])))
    {
        fprintf(stderr, "Index %d for openMode parameter out of range.\n",
                openMode);
        return 1;
    }

    /* Get individual filenames and delete existent ones. */
    sprintf(filename, "test_dircount_%s.tif", openModeStrings[openMode]);
    unlink(filename);

    /* Create a file and write N_DIRECTORIES_2 directories to it. */
    TIFF *tif = TIFFOpen(filename, openModeStrings[openMode]);
    if (!tif)
    {
        fprintf(stderr, "Can't create %s\n", filename);
        return 1;
    }

    /*-- Standard case for writing sequential IFDs. --*/
    for (int i = 0; i < N_DIRECTORIES_2; i++)
    {
        if (write_data_to_current_directory(tif, i, false))
        {
            fprintf(stderr,
                    "Can't write data to current directory in %s at %d\n",
                    filename, __LINE__);
            goto failure;
        }
        TIFFWriteDirectory_M(tif, filename, __LINE__);
        expected_curdir++;
        CHECKCURDIRNUM_M(tif, expected_curdir, __LINE__);
    }
    TIFFClose(tif);

    /* Simulate case for trap in TIFFLinkDirectory() where tif_lastdiroff is set
     * but tif_curdircount is not.
     * https://gitlab.com/libtiff/libtiff/-/merge_requests/634#note_1990250515
     */
    tif = TIFFOpen(filename, "r+");
    CHECKCURDIRNUM_M(tif, 0, __LINE__);
    TIFFRewriteDirectory(tif);
    CHECKCURDIRNUM_M(tif, 0, __LINE__);
    TIFFCreateDirectory(tif);
    TIFFWriteDirectory(tif);
    CHECKCURDIRNUM_M(tif, 2, __LINE__);
    tdir_t numdir = TIFFNumberOfDirectories(tif);
    TIFFClose(tif);

    /* Testcase iswrittentofile=0 for SetSubDirectory(0). */
    tif = TIFFOpen(filename, "r+");
    TIFFSetSubDirectory(tif, 0);
    /* After TIFFSetSubDirectory(tif, 0) the read IFD is re-written as a
     * new IFD and tif_curdir and tif_curdircount shall be updated.
     * The latter was not the case before resetting iswrittentofile=0 at
     * SetSubDirectory(0). Even an IFD-Loop was then wrongly assumed at the next
     * TIFFNumberOfDirectories() call. */
    TIFFWriteDirectory(tif);
    CHECKCURDIRNUM_M(tif, numdir, __LINE__);

    /* Unlink all directories. */
    numdir = TIFFNumberOfDirectories(tif);
    for (int i = numdir; 0 < i; i--)
    {
        TIFFUnlinkDirectory(tif, i);
        CHECKCURDIRNUM_M(tif, (tdir_t)(-1), __LINE__);
    }

    TIFFClose(tif);
    unlink(filename);
    return 0;

failure:
    TIFFClose(tif);
    return 1;
} /*-- test_curdircount_setting() --*/

/* Test writing and reading of a custom directory only (solitary) without having
 * touched a normal directory.
 * Reproduces issue https://gitlab.com/libtiff/libtiff/-/issues/643.
 * Error analysis of issue 643:
 *   Opening a file as read with "h" option does not initialize a default
 *   directory and thus, the pointer tif->tif_tagmethods.vsetfield and
 *   tif->tif_tagmethods.vgetfield are not initialized. This is done in
 *   TIFFDefaultDirectory(). In standard read functions, that default directory
 *   is set up but not for reading custom directories. Any call to
 *   TIFFSetField() or TIFFGetField() within TIFFReadCustomDirectory() will then
 *   fail or cause a SegFault.
 */
int test_solitary_custom_directory(unsigned int openMode)
{
    char filename[128] = {0};

    if (openMode >= (sizeof(openModeStrings) / sizeof(openModeStrings[0])))
    {
        fprintf(stderr, "Index %d for openMode parameter out of range.\n",
                openMode);
        return 1;
    }

    /* Get individual filenames and delete existent ones. */
    sprintf(filename, "test_solitary_custom_directory_%s.tif",
            openModeStrings[openMode]);
    unlink(filename);

    /* Create a dummy file without IFD. */
    TIFF *tif = TIFFOpen(filename, openModeStrings[openMode]);
    if (!tif)
    {
        fprintf(stderr, "Can't create %s\n", filename);
        return 1;
    }
    /* A TIFFSetFiled() here would work. */
    TIFFSetField(tif, TIFFTAG_DOCUMENTNAME, "DocName");
    TIFFClose(tif);

    /* Open file without reading a directory using option "h".
     * Option "h" suppresses to load the directory. Thus reading functions are
     * hot set. Read TIFF header only, do not load the first image directory.
     * That could be useful in case of the broken first directory. We can open
     * the file and proceed to the other directories.
     */
    tif = TIFFOpen(filename, "r+h");
    if (!tif)
    {
        fprintf(stderr, "Can't open %s\n", filename);
        return 1;
    }
    /* A TIFFSetField() here would fail or cause a SegFault, because of the "h"
     * option before fix of issue 643. */
    TIFFSetField(tif, TIFFTAG_DOCUMENTNAME, "DocName");

    /* Write a custom directory. */
    uint64_t offsetEXIFBase;
    if (write_EXIF_data_to_current_directory(tif, 0, &offsetEXIFBase))
    {
        fprintf(stderr,
                "Can't write solitary EXIF data into %s. "
                "Testline %d\n",
                filename, __LINE__);
        goto failure;
    }
    TIFFClose(tif);

    /* Open and read custom directory without touching another IFD. */
    tif = TIFFOpen(filename, "rh");
    if (!tif)
    {
        fprintf(stderr, "Can't open %s\n", filename);
        return 1;
    }
    /* This custom directory reading will cause a SegFault due to the "h"
     * option before fix of issue 643. */
    if (!TIFFReadEXIFDirectory(tif, offsetEXIFBase))
    {
        fprintf(stderr,
                "Can't read solitary EXIF data from %s. "
                "Testline %d\n",
                filename, __LINE__);
        goto failure;
    }

    TIFFClose(tif);
    unlink(filename);
    return 0;

failure:
    TIFFClose(tif);
    return 1;
} /*-- test_solitary_custom_directory() --*/

int main()
{

    unsigned int openModeMax =
        (sizeof(openModeStrings) / sizeof(openModeStrings[0]));
    for (unsigned int openMode = 0; openMode < openModeMax; openMode++)
    {
        fprintf(stderr, "\n--- Test for %s open option. ---\n",
                openModeText[openMode]);
        if (test_curdircount_setting(openMode))
        {
            fprintf(stderr, "Failed during %s test_curdircount_setting test.\n",
                    openModeText[openMode]);
            return 1;
        }

        if (test_lastdir_offset(openMode))
        {
            fprintf(stderr, "Failed during %s WriteDirectory test.\n",
                    openModeText[openMode]);
            return 1;
        }

        if (test_rewrite_lastdir_offset(openMode))
        {
            fprintf(stderr, "Failed during %s RewriteDirectory test.\n",
                    openModeText[openMode]);
            return 1;
        }

        /* Test arbitrary directory loading */
        if (test_arbitrary_directrory_loading(openMode))
        {
            fprintf(stderr,
                    "Failed during %s ArbitraryDirectoryLoading test.\n",
                    openModeText[openMode]);
            return 1;
        }

        /* Test SubIFD writing and reading */
        if (test_SubIFD_directrory_handling(openMode))
        {
            fprintf(stderr,
                    "Failed during %s SubIFD_directrory_handling test.\n",
                    openModeText[openMode]);
            return 1;
        }
        /* Test for failure in TIFFSetSubDirectory() */
        if (test_SetSubDirectory_failure(openMode))
        {
            fprintf(stderr, "Failed during %s SetSubDirectory_failure test.\n",
                    openModeText[openMode]);
            return 1;
        }
    }

    /* Test for correct "current directory number" incrementing.*/
    for (unsigned int openMode = 0; openMode < openModeMax; openMode++)
    {
        for (int i = 0; i <= 2; i++)
        {
            fprintf(stderr,
                    "\n--- Test for test_current_dirnum_incrementing %s "
                    "testcase %d. "
                    "---\n",
                    openModeText[openMode], i);
            if (test_current_dirnum_incrementing(i, openMode))
            {
                fprintf(stderr,
                        "Failed during Current Directory Number Incrementing "
                        "test #%d in %s.\n",
                        i, openModeText[openMode]);
                return 1;
            }
        }
    }

    /* Test for reading / writing custom directory (e.g. EXIF) without having
     * touched a normal directory. */
    for (unsigned int openMode = 0; openMode < openModeMax; openMode++)
    {
        fprintf(stderr,
                "\n--- Test for test_solitary_custom_directory %s ---\n",
                openModeText[openMode]);
        if (test_solitary_custom_directory(openMode))
        {
            fprintf(stderr,
                    "Failed during test_solitary_custom_directory in %s.\n",
                    openModeText[openMode]);
            return 1;
        }
    }

    return 0;
}
