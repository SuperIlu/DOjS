/*
 * TIFF Library
 *
 * The purpose of this test file is to test the correctness of
 * TIFFWriteDirectory() when an already written IFD is extended with additional
 * tags or the IFD data size is increased. The IFD must then be re-written to a
 * different location in the file. On the other hand, the IFD should be
 * overwritten if its size has not grown.
 */

#include "tif_config.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tiffio.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef TIFFmin
#define TIFFmax(A, B) ((A) > (B) ? (A) : (B))
#define TIFFmin(A, B) ((A) < (B) ? (A) : (B))
#endif

/* Global settings -not save to change- */
#define SPP 3 /* samples per pixel */
#define BPS 8 /* bits per sample */

char *modeStrings[] = {"wl", "wb", "w8l", "w8b"};

/* Writes some pixel data as scanline or tiles to file.
 */
int write_image_data(TIFF *tif, uint16_t width, uint16_t length, bool tiled,
                     unsigned int pixval, unsigned char *plastlinedata,
                     unsigned int lastlinebytesmax)
{
    size_t bufLen;
    unsigned char *pbufLine = NULL;
    unsigned int bpsmod = (1 << BPS);
    int tlwidth;
    int tllength;
    tmsize_t tlsize;

    assert(SPP == 3);

    if (tiled)
    {
        int ret = TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tlwidth);
        ret += TIFFGetField(tif, TIFFTAG_TILELENGTH, &tllength);
        if (ret < 2)
        {
            fprintf(stderr,
                    "Can't get TIFFTAG_TILEWIDTH/_LENGTH tags. Testline %d\n",
                    __LINE__);
            return 1;
        }
        /* For tiled mode get size of a tile in bytes */
        tlsize = TIFFTileSize(tif);
        bufLen = (size_t)tlsize;
    }
    else
    {
        /* For strip mode get size of a row in bytes */
        bufLen = (((size_t)width * SPP * BPS) + 7) / 8;
    }

    pbufLine = _TIFFmalloc(bufLen);
    if (pbufLine == NULL)
        return 1;

    /* Fill image buffer. */
    pbufLine[0] = (unsigned char)((pixval * 100) % bpsmod);
    pbufLine[1] = 127 % bpsmod;
    pbufLine[2] = 255 % bpsmod;
    for (size_t x = SPP; x < bufLen; x += SPP)
    {
        pbufLine[x] = 10 % bpsmod;
        pbufLine[x + 1] = (unsigned char)((x * 40) % bpsmod);
        pbufLine[x + 2] = (unsigned char)((x * 70) % bpsmod);
    }

    /* Write dummy pixel data. */
    if (tiled)
    {
        uint32_t numtiles = TIFFNumberOfTiles(tif);
        for (uint32_t i = 0; i < numtiles; i++)
        {
            if (TIFFWriteEncodedTile(tif, i, pbufLine, 0) == -1)
            {
                fprintf(stderr, "Can't write image data tile. Testline %d\n",
                        __LINE__);
                return 1;
            }
            if ((plastlinedata != NULL) && (i == (numtiles - 1)))
            {
                memcpy(plastlinedata, pbufLine,
                       TIFFmin(bufLen, (size_t)lastlinebytesmax));
            }
            /* Change something for next row. */
            pbufLine[0] = (pbufLine[0] + 35) % bpsmod;
        }
    }
    else
    {
        for (int i = 0; i < length; i++)
        {
            if (TIFFWriteScanline(tif, pbufLine, i, 0) == -1)
            {
                fprintf(stderr,
                        "Can't write image data scanline. Testline %d\n",
                        __LINE__);
                return 1;
            }
            if ((plastlinedata != NULL) && (i == (length - 1)))
            {
                memcpy(plastlinedata, pbufLine,
                       TIFFmin(bufLen, (size_t)lastlinebytesmax));
            }
            /* Change something for next row. */
            pbufLine[0] = (pbufLine[0] + 30) % bpsmod;
        }
    }
    _TIFFfree(pbufLine);
    return 0;
} /*-- write_image_data() --*/

/* Fills the active IFD with some default values and writes
 *  an image with given number of lines as strips (scanlines) or tiles to file.
 */
int write_data_to_current_directory(TIFF *tif, uint16_t width, uint16_t length,
                                    bool tiled, int ifd_page_num,
                                    bool write_data,
                                    unsigned char *plastlinedata,
                                    unsigned int lastlinebytesmax)
{
    const uint16_t photometric = PHOTOMETRIC_RGB;
    const uint16_t rows_per_strip = 1;
    const uint16_t planarconfig = PLANARCONFIG_CONTIG;

    char auxString[128];

    if (!tif)
    {
        fprintf(stderr, "Invalid TIFF handle.\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width))
    {
        fprintf(stderr, "Can't set ImageWidth tag.\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_IMAGELENGTH, length))
    {
        fprintf(stderr, "Can't set ImageLength tag.\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, BPS))
    {
        fprintf(stderr, "Can't set BitsPerSample tag.\n");
        return 1;
    }
    if (!TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, SPP))
    {
        fprintf(stderr, "Can't set SamplesPerPixel tag.\n");
        return 1;
    }

    if (tiled)
    {
        /* Tilesizes must be multiple of 16. */
        int ret = TIFFSetField(tif, TIFFTAG_TILEWIDTH, 16);
        ret += TIFFSetField(tif, TIFFTAG_TILELENGTH, 16);
        if (ret < 2)
        {
            fprintf(stderr, "Can't set TIFFTAG_TILEWIDTH/_LENGTH tags.\n");
            return 1;
        }
    }
    else
    {
        if (!TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, rows_per_strip))
        {
            fprintf(stderr, "Can't set SamplesPerPixel tag.\n");
            return 1;
        }
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
    sprintf(auxString, "%d th. IFD", ifd_page_num);
    if (!TIFFSetField(tif, TIFFTAG_PAGENAME, auxString))
    {
        fprintf(stderr, "Can't set TIFFTAG_PAGENAME tag.\n");
        return 1;
    }

    /* Write dummy pixel data. */
    if (write_data)
    {
        if (write_image_data(tif, width, length, tiled, ifd_page_num,
                             plastlinedata, lastlinebytesmax))
        {
            fprintf(stderr, "Can't write image data. Testline %d\n", __LINE__);
            return 1;
        }
    }
    return 0;
} /*-- write_data_to_current_directory() --*/

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

/* Compare 'requested_dir_number' with number written in PageName tag
 * into the IFD to identify that IFD.  */
int is_requested_directory(TIFF *tif, int requested_dir_number,
                           const char *filename)
{
    char *ptr = NULL;
    char *auxStr = NULL;

    if (!TIFFGetField(tif, TIFFTAG_PAGENAME, &ptr))
    {
        fprintf(stderr, "Can't get TIFFTAG_PAGENAME tag. Testline %d\n",
                __LINE__);
        return 0;
    }

    /* Check for reading errors */
    if (ptr != NULL)
        auxStr = strchr(ptr, ' ');

    if (ptr == NULL || auxStr == NULL || strncmp(auxStr, " th.", 4))
    {
        ptr = ptr == NULL ? "(null)" : ptr;
        fprintf(stderr,
                "Error reading IFD directory number from PageName tag: %s. "
                "Testline %d\n",
                ptr, __LINE__);
        return 0;
    }

    /* Retrieve IFD identification number from ASCII string */
    const int nthIFD = atoi(ptr);
    if (nthIFD == requested_dir_number)
    {
        return 1;
    }
    fprintf(
        stderr,
        "Expected directory %d from %s was not loaded but: %s. Testline %d\n",
        requested_dir_number, filename, ptr, __LINE__);

    return 0;
}

/* Checks that IFDs on file can be overwritten if they have not grown and
 * were re-written to an other location if IFD size has been grown.
 */
int test_IFD_enlargement(const char *filename, unsigned int openMode,
                         bool is_strile_array_writing, int numIFDs,
                         uint16_t width, uint16_t length, bool tiled)
{

#define NUMIFDsMAX 4
    uint64_t auxUint64;
    TIFFErrorHandler errHandler = NULL;
    uint64_t offsetBase[NUMIFDsMAX] = {0};
    uint64_t offsetNew[NUMIFDsMAX];
    unsigned char bufLine[NUMIFDsMAX + 1][1024];

    assert(numIFDs <= NUMIFDsMAX);
    assert(openMode < (sizeof(modeStrings) / sizeof(modeStrings[0])));

    /*-- Create a file and write numIFDs IFDs directly to it --*/
    TIFF *tif = TIFFOpen(filename, modeStrings[openMode]);
    if (!tif)
    {
        fprintf(stderr, "Can't create %s. Testline %d\n", filename, __LINE__);
        return 1;
    }

    for (int i = 0; i < numIFDs; i++)
    {
        /* Writing baseline images (IFDs) to file. */
        if (write_data_to_current_directory(tif, width, length, tiled, i,
                                            !is_strile_array_writing,
                                            &bufLine[i][0], sizeof(bufLine[i])))
        {
            fprintf(
                stderr,
                "Can't write data to current directory in %s. Testline %d\n",
                filename, __LINE__);
            goto failure;
        }
        /* xmldata need to be 9 bytes (17 bytes for BigTIFF, respectively) thus
         * not to fit into the entry itself and have space to cover two
         * strip-/tile-offsets when XMLPACKET tag contents is reduced. */
        char xmldata[20];
        if (TIFFIsBigTIFF(tif))
            strcpy(xmldata, "XML-ABCDEABCDEFGH");
        else
            strcpy(xmldata, "XML-ABCDE");
        uint32_t xmlpacketLength = (uint32_t)strlen(xmldata);

        if (!TIFFSetField(tif, TIFFTAG_XMLPACKET, xmlpacketLength, xmldata))
        {
            fprintf(stderr, "Can't set TIFFTAG_XMLPACKET tag. Testline %d\n",
                    __LINE__);
            goto failure;
        }
        if (is_strile_array_writing)
        {
            /* Set flag for deferred strile-array writing. */
            if (!TIFFDeferStrileArrayWriting(tif))
            {
                fprintf(
                    stderr,
                    "TIFFDeferStrileArrayWriting failed for %s. Testline %d\n",
                    filename, __LINE__);
                goto failure;
            }
            /* Set flag, that image writing has started and freeze relevant IFD
             * tags. */
            if (!TIFFWriteCheck(tif, tiled, NULL))
            {
                fprintf(stderr, "TIFFWriteCheck failed for %s. Testline %d\n",
                        filename, __LINE__);
                goto failure;
            }
        }
        /* Write IFD tags to file (and setup new empty IFD). */
        if (!TIFFWriteDirectory(tif))
        {
            fprintf(stderr, "Can't write directory to %s. Testline %d\n",
                    filename, __LINE__);
            goto failure;
        }
    } /*-- for (numIFDs) --*/

    /* Retrieve IFD offset values to read IFDs before image data has been
     * written when is_strile_array_writing is true. */
    for (int i = numIFDs - 1; i >= 0; i--)
    {
        TIFFSetDirectory(tif, i);
        offsetBase[i] = TIFFCurrentDirOffset(tif);
    }
    /* For TIFFDeferStrileArrayWriting write now image data to file.*/
    if (is_strile_array_writing)
    {
        for (int i = 0; i < numIFDs; i++)
        {
            /* Reset to written IFD to write strile arrays with dummy content to
             * file and update StripOffset tag within IFD on file.
             * First TIFFForceStrileArrayWriting() prepares internal flags and
             * memory for next writing after switching back to the already
             * written IFD. */
            TIFFSetDirectory(tif, i);
            if (!TIFFForceStrileArrayWriting(tif))
            {
                fprintf(
                    stderr,
                    "TIFFForceStrileArrayWriting failed for %s. Testline %d\n",
                    filename, __LINE__);
                goto failure;
            }
            /* Then write image pixels. */
            if (write_image_data(tif, width, length, tiled, 5 * i,
                                 &bufLine[i][0], sizeof(bufLine[i])))
            {
                fprintf(stderr, "Can't write image data. Testline %d\n",
                        __LINE__);
                return 1;
            }
            /* Last image data need to be flushed to file. */
            if (!TIFFFlushData(tif))
            {
                fprintf(stderr, "TIFFFlushData failed for %s. Testline %d\n",
                        filename, __LINE__);
                goto failure;
            }
            /* Fill (overwrite) strile array with final offsets after image data
             * has been written. Would also be called by TIFFClose(). */
            if (!TIFFForceStrileArrayWriting(tif))
            {
                fprintf(
                    stderr,
                    "TIFFForceStrileArrayWriting failed for %s. Testline %d\n",
                    filename, __LINE__);
                goto failure;
            }
        }
    }
    /* Retrieve IFD offset values for later checking. Should not be changed. */
    for (int i = numIFDs - 1; i >= 0; i--)
    {
        TIFFSetDirectory(tif, i);
        auxUint64 = TIFFCurrentDirOffset(tif);
        if (offsetBase[i] != auxUint64)
        {
            fprintf(stderr, "Unexpected change of offset IFD %d. Testline %d\n",
                    i, __LINE__);
            goto failure;
        }
    }

    /* Test over-writing with less bytes.
     * This leaves some bytes of the larger previous IFD at EOF or before the
     * next IFD. */
    TIFFSetDirectory(tif, 0);
    char xmldata0[] = "XMLb";
    uint32_t xmlpacketLength = (uint32_t)strlen(xmldata0);

    if (!TIFFSetField(tif, TIFFTAG_XMLPACKET, xmlpacketLength, xmldata0))
    {
        fprintf(stderr, "Can't set TIFFTAG_XMLPACKET tag. Testline %d\n",
                __LINE__);
        goto failure;
    }
    if (!TIFFWriteDirectory(tif))
    {
        fprintf(stderr, "Can't overwrite directory to %s. Testline %d\n",
                filename, __LINE__);
        goto failure;
    }

    /* Re-read IFD and check its offset, which should not have been changed at
     * this point, except for strile_array_writing because then the stripoffsets
     * are now written within IFD space. However, for only one IFD (numIFDs=1)
     * the strile array data is written straight behind the IFD and thus can be
     * overwritten.
     * Furthermore, strile_array size of <= 9 (<=17 for BigTIFF) can also be
     * written within the IFD space, because 9 (17 for BigTIFF) byte are gained
     * by downsizing the XMLPACKET. */
    TIFFSetDirectory(tif, 0);
    auxUint64 = TIFFCurrentDirOffset(tif);
    if (!is_strile_array_writing && offsetBase[0] != auxUint64)
    {
        fprintf(stderr,
                "Failure: Directory 0 offset has changed from 0x%" PRIx64
                " (%" PRIu64 ") to 0x %" PRIx64 " (%" PRIu64
                "). Testline %d\n ",
                offsetBase[0], offsetBase[0], auxUint64, auxUint64, __LINE__);
        goto failure;
    }
    else if (is_strile_array_writing && !tiled)
    {
        if (numIFDs > 1 && TIFFNumberOfStrips(tif) > 2)
        {
            if (offsetBase[0] == auxUint64)
            {
                fprintf(stderr,
                        "Failure with strile array writing: Directory 0 offset "
                        "has now "
                        "NOT changed from 0x%" PRIx64 " (%" PRIu64
                        "). Testline %d\n ",
                        offsetBase[0], offsetBase[0], __LINE__);
                goto failure;
            }
        }
        else if (offsetBase[0] != auxUint64)
        {
            fprintf(stderr,
                    "Failure with strile array writing: Directory 0 offset has "
                    "changed from 0x%" PRIx64 " (%" PRIu64 ") to 0x %" PRIx64
                    " (%" PRIu64 "). Testline %d\n ",
                    offsetBase[0], offsetBase[0], auxUint64, auxUint64,
                    __LINE__);
            goto failure;
        }
    }
    else if (is_strile_array_writing && tiled)
    {
        if (numIFDs > 1 && TIFFNumberOfTiles(tif) > 2)
        {
            if (offsetBase[0] == auxUint64)
            {
                fprintf(stderr,
                        "Failure with tiled strile array writing: Directory 0 "
                        "offset has now NOT changed from 0x%" PRIx64
                        " (%" PRIu64 "). Testline %d\n ",
                        offsetBase[0], offsetBase[0], __LINE__);
                goto failure;
            }
        }
        else if (offsetBase[0] != auxUint64)
        {
            fprintf(stderr,
                    "Failure with tiled strile array writing: Directory 0 "
                    "offset has "
                    "changed from 0x%" PRIx64 " (%" PRIu64 ") to 0x %" PRIx64
                    " (%" PRIu64 "). Testline %d\n ",
                    offsetBase[0], offsetBase[0], auxUint64, auxUint64,
                    __LINE__);
            goto failure;
        }
    }

    /*-- Enlarge IFD 0 with enlarged tag data and write it again. --*/
    char xmldata1[] = "XML data package";
    xmlpacketLength = (uint32_t)strlen(xmldata1);

    if (!TIFFSetField(tif, TIFFTAG_XMLPACKET, xmlpacketLength, xmldata1))
    {
        fprintf(stderr, "Can't set TIFFTAG_XMLPACKET tag. Testline %d\n",
                __LINE__);
        goto failure;
    }
    if (!TIFFWriteDirectory(tif))
    {
        fprintf(stderr, "Can't re-write directory 0 to %s. Testline %d\n",
                filename, __LINE__);
        goto failure;
    }

    /* Check IFD 0 offset, which should have been changed.
     * (Because of unused bytes before EOF generated by downsizing of XMLPACKET
     * before enlarging IFD will be re-written even for numIFDs==1) */
    if (!TIFFSetDirectory(tif, 0))
    {
        fprintf(stderr, "Can't set directory 0 of %s. Testline %d\n", filename,
                __LINE__);
        goto failure;
    }
    offsetNew[0] = TIFFCurrentDirOffset(tif);
    if (offsetBase[0] == offsetNew[0])
    {
        fprintf(
            stderr,
            "Failure: Directory 0 offset has now NOT changed from 0x%" PRIx64
            " (%" PRIu64 ").Testline %d\n ",
            offsetNew[0], offsetNew[0], __LINE__);
        goto failure;
    }

    if (numIFDs > 1)
    {
        /*-- Read next directory IFD=1 and re-write that. --*/
        if (!TIFFReadDirectory(tif))
        {
            fprintf(stderr, "Can't read directory from %s. Testline %d\n",
                    filename, __LINE__);
            goto failure;
        }
        /* Enlarge IFD 1 by adding new tag. */
        if (!TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION,
                          "WGS 84 / UTM zone 1N|WGS 84|"))
        {
            fprintf(stderr,
                    "Can't set TIFFTAG_IMAGEDESCRIPTION tag. Testline %d\n",
                    __LINE__);
            goto failure;
        }
        auxUint64 = TIFFCurrentDirOffset(tif);
        if (!TIFFWriteDirectory(tif))
        {
            fprintf(stderr, "Can't re-write directory 1 to %s. Testline %d\n",
                    filename, __LINE__);
            goto failure;
        }
        TIFFSetDirectory(tif, 1);
        offsetNew[1] = TIFFCurrentDirOffset(tif);
        if (auxUint64 == offsetNew[1])
        {
            fprintf(stderr,
                    "Failure: Directory 1 offset has now NOT changed from "
                    "0x%" PRIx64 " (%" PRIu64 ").Testline %d\n ",
                    offsetNew[1], offsetNew[1], __LINE__);
            goto failure;
        }
    }

    /* Use TIFFSetSubDirectory() for old IFD 0 and just overwrite it.
     * Just overwriting invalid directory works. For strile array writing it
     * shall fail, because now the striles are tried to be written within the
     * IFD space. However, for only one IFD (numIFDs=1) the strile array data is
     * written straight behind the IFD and thus can be overwritten.
     * The same is true for tiled images, however if the number of tiles
     * is one (TIFFNumberOfTiles==1), the tile offset fits into the tag entry
     * and thus can also be overwritten.
     */
    if (!TIFFSetSubDirectory(tif, offsetBase[0]))
    {
        fprintf(stderr,
                "Can't set sub-directory at offset 0x%" PRIx64 " (%" PRIu64
                ") within %s. Testline %d\n",
                offsetBase[0], offsetBase[0], filename, __LINE__);
        goto failure;
    }
    if (!tiled)
    {
        if (!is_strile_array_writing || numIFDs <= 1)
        {
            if (!TIFFWriteDirectory(
                    tif)) /* just overwriting invalid directory works!!*/
            {
                fprintf(stderr,
                        "Can't overwrite directory to %s. Testline %d\n",
                        filename, __LINE__);
                goto failure;
            }
        }
        else
        {
            /* Because XMLPACKET is 9 byte reduced, small strilearrays can be
             * overwritten. */
            if (TIFFNumberOfStrips(tif) > 2)
                /* Suppress error message from 'TIFFRewriteDirectory()' like
                 * 'Error fetching directory link' or 'Sanity check...'. */
                errHandler = TIFFSetErrorHandler(NULL);
            if (TIFFWriteDirectory(tif) && TIFFNumberOfStrips(tif) > 2)
            {
                fprintf(stderr,
                        "Was expected to fail but succeeded. %s. Testline %d\n",
                        filename, __LINE__);
                goto failure;
            }
            if (TIFFNumberOfStrips(tif) > 2)
                TIFFSetErrorHandler(errHandler);
        }
    }
    else
    {
        if (!is_strile_array_writing || numIFDs <= 1 ||
            TIFFNumberOfTiles(tif) <= 1)
        {
            if (!TIFFWriteDirectory(
                    tif)) /* just overwriting invalid directory works!!*/
            {
                fprintf(stderr,
                        "Can't overwrite directory to %s. Testline %d\n",
                        filename, __LINE__);
                goto failure;
            }
        }
        else
        {
            /* Because XMLPACKET is 9 byte reduced, small strilearrays can be
             * overwritten. */
            if (TIFFNumberOfTiles(tif) > 2)
                /* Suppress error message from 'TIFFRewriteDirectory()' like
                 * 'Error fetching directory link' or 'Sanity check...'. */
                errHandler = TIFFSetErrorHandler(NULL);
            if (TIFFWriteDirectory(tif) && TIFFNumberOfTiles(tif) > 2)
            {
                fprintf(stderr,
                        "Was expected to fail but succeeded. %s. Testline %d\n",
                        filename, __LINE__);
                goto failure;
            }
            if (TIFFNumberOfTiles(tif) > 2)
                TIFFSetErrorHandler(errHandler);
        }
    }

    /* Re-read the already re-written and thus invalidated IFD 0 with
     * TIFFSetSubDirectory() and try to re-write it.
     * Re-writing an invalid IFD should fail. */
    if (!TIFFSetSubDirectory(tif, offsetBase[0]))
    {
        fprintf(stderr,
                "Can't set sub-directory at offset 0x%" PRIx64 " (%" PRIu64
                ") within %s. Testline %d\n",
                offsetBase[0], offsetBase[0], filename, __LINE__);
        goto failure;
    }
    if (!TIFFSetField(tif, TIFFTAG_XMLPACKET, (uint32_t)strlen(xmldata1),
                      xmldata1))
    {
        fprintf(stderr, "Can't set TIFFTAG_XMLPACKET tag. Testline %d\n",
                __LINE__);
        goto failure;
    }
    /* Suppress error message from 'TIFFRewriteDirectory()' like
     * 'Error fetching directory link' or 'Sanity check...'. */
    errHandler = TIFFSetErrorHandler(NULL);
    if (TIFFWriteDirectory(tif))
    {
        fprintf(stderr, "Was expected to fail but succeeded. %s. Testline %d\n",
                filename, __LINE__);
        goto failure;
    }
    TIFFSetErrorHandler(errHandler);

    /*-- Now with a valid IFD: Re-read IFD with TIFFSetSubDirectory() and
     * re-write it. If it is at the end of the file, it will be overwritten.
     * This is the case for numIFD==1. */
    if (!TIFFSetSubDirectory(tif, offsetNew[0]))
    {
        fprintf(stderr,
                "Can't set sub-directory at offset 0x%" PRIx64 " (%" PRIu64
                ") within %s. Testline %d\n",
                offsetNew[0], offsetNew[0], filename, __LINE__);
        goto failure;
    }
    char xmldata2[] = "XML data package even longer";
    if (!TIFFSetField(tif, TIFFTAG_XMLPACKET, (uint32_t)strlen(xmldata2),
                      xmldata2))
    {
        fprintf(stderr, "Can't set TIFFTAG_XMLPACKET tag.. Testline %d\n",
                __LINE__);
        goto failure;
    }
    auxUint64 = TIFFCurrentDirOffset(tif);
    if (!TIFFWriteDirectory(tif))
    {
        fprintf(stderr, "Can't re-write directory 0 to %s. Testline %d\n",
                filename, __LINE__);
        goto failure;
    }
    TIFFSetDirectory(tif, 0);
    offsetNew[0] = TIFFCurrentDirOffset(tif);
    if (numIFDs > 1 && auxUint64 == offsetNew[0])
    {
        fprintf(stderr,
                "Failure: Directory %d offset has NOT changed from 0x%" PRIx64
                " (%" PRIu64 ").Testline %d\n ",
                0, offsetNew[0], offsetNew[0], __LINE__);
        goto failure;
    }

    /*-- Make a little check. --*/
    /* Reopen prepared testfile */
    TIFFClose(tif);
    tif = TIFFOpen(filename, "r+");
    if (!tif)
    {
        fprintf(stderr, "Can't open %s\n", filename);
        goto failure;
    }
    for (int i = 0; i < numIFDs; i++)
    {

        if (!TIFFSetDirectory(tif, i))
        {
            fprintf(stderr, "Can't set directory %d of %s. Testline %d\n", i,
                    filename, __LINE__);
            goto failure;
        }
        if (i < 2)
        {
            auxUint64 = TIFFCurrentDirOffset(tif);
            if (auxUint64 != offsetNew[i])
            {
                fprintf(
                    stderr,
                    "Failure: Directory 0 offset has changed from 0x%" PRIx64
                    " (%" PRIu64 ") to 0x %" PRIx64 " (%" PRIu64
                    ").Testline %d\n ",
                    offsetNew[i], offsetNew[i], auxUint64, auxUint64, __LINE__);
                goto failure;
            }
        }
        if (!is_requested_directory(tif, i, filename))
        {
            goto failure;
        }
        /* Check last image line or last tile. */
        tmsize_t bytesRead;
        if (tiled)
        {
            uint32_t numtiles = TIFFNumberOfTiles(tif);
            uint64_t tileSize = TIFFTileSize64(tif);
            if (tileSize > sizeof(bufLine[NUMIFDsMAX]))
            {
                fprintf(
                    stderr,
                    "Failure: Read buffer for tile too small.Testline %d\n ",
                    __LINE__);
                goto failure;
            }
            bytesRead = TIFFReadEncodedTile(
                tif, numtiles - 1, &bufLine[NUMIFDsMAX][0], sizeof(bufLine[0]));
        }
        else
        {
            uint64_t scanlineSize = TIFFScanlineSize64(tif);
            if (scanlineSize > sizeof(bufLine[NUMIFDsMAX]))
            {
                fprintf(stderr,
                        "Failure: Read buffer for scanline too small.Testline "
                        "%d\n ",
                        __LINE__);
                goto failure;
            }
            bytesRead =
                TIFFReadScanline(tif, &bufLine[NUMIFDsMAX][0], length - 1, 0);
        }
        if (bytesRead > 0)
        {
            for (int k = 0; k < bytesRead; k++)
            {
                if (bufLine[NUMIFDsMAX][k] != bufLine[i][k])
                {
                    fprintf(
                        stderr,
                        "Failure: Read IFD %d last line pixel %d value %d is "
                        "different to written value %d .Testline %d\n ",
                        i, k, bufLine[i][k], bufLine[NUMIFDsMAX][k], __LINE__);
                    goto failure;
                }
            }
        }
        else
        {
            fprintf(stderr,
                    "Failure: Reading IFD %d last line %d .Testline %d\n ", i,
                    length - 1, __LINE__);
            goto failure;
        }
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
} /*-- test_IFD_enlargement --*/

/* Width and length for the next test functions can be adapted here. */
#define WIDTH 1
#define LENGTH 2

/* Checks that Custom-IFDs like EXIF-IFD in file can be overwritten if they have
 * not grown and were re-written to an other location if IFD size has been
 * grown.
 */
int test_EXIF_enlargement(const char *filename, bool is_big_tiff)
{
    char auxString[128];
    uint64_t offsetEXIFBase[2];
    uint64_t offsetEXIFNew[2];

    /*-- Create a file and write two EXIF-IFDs directly to it --*/
    TIFF *tif = TIFFOpen(filename, is_big_tiff ? "w8" : "w");
    if (!tif)
    {
        fprintf(stderr, "Can't create %s. Testline %d\n", filename, __LINE__);
        return 1;
    }

    /* Write baseline test image. */
    for (int i = 0; i < 2; i++)
    {
        /* Writing baseline images (IFDs) and EXIF-IFDs to file. */
        if (write_data_to_current_directory(tif, WIDTH, LENGTH, FALSE, i, true,
                                            NULL, 0))
        {
            fprintf(
                stderr,
                "Can't write data to current directory in %s. Testline %d\n",
                filename, __LINE__);
            goto failure;
        }
        /* Set EXIFIFD tag with dummy value to reserve space in IFD. */
        if (!TIFFSetField(tif, TIFFTAG_EXIFIFD, (uint64_t)0))
        {
            fprintf(stderr, "Can't set TIFFTAG_EXIFIFD tag. Testline %d\n",
                    __LINE__);
            goto failure;
        }
        if (!TIFFWriteDirectory(tif))
        {
            fprintf(stderr, "Can't write directory to %s. Testline %d\n",
                    filename, __LINE__);
            goto failure;
        }

        if (write_EXIF_data_to_current_directory(tif, i, &offsetEXIFBase[i]))
        {
            fprintf(stderr,
                    "Can't write EXIF data to current directory in %s. "
                    "Testline %d\n",
                    filename, __LINE__);
            goto failure;
        }
        /* Go back to current main-IFD and update EXIF-IFD offset. */
        if (!TIFFSetDirectory(tif, i))
        {
            fprintf(stderr, "Can't set directory %d of %s. Testline %d\n", i,
                    filename, __LINE__);
            goto failure;
        }
        if (!TIFFSetField(tif, TIFFTAG_EXIFIFD, offsetEXIFBase[i]))
        {
            fprintf(stderr, "Can't set TIFFTAG_EXIFIFD tag. Testline %d\n",
                    __LINE__);
            goto failure;
        }
        if (!TIFFWriteDirectory(tif))
        {
            fprintf(stderr, "Can't write directory to %s. Testline %d\n",
                    filename, __LINE__);
            goto failure;
        }
    }

    /* Test over-writing the EXIF directory without change or with less bytes.
     */
    int retCode;
    char *pAscii0;
    char *pAscii1;
    if (!TIFFReadEXIFDirectory(tif, offsetEXIFBase[0]))
    {
        fprintf(stderr,
                "Can't read EXIF directory at offset 0x%" PRIx64 " (%" PRIu64
                ")  within %s. Testline %d\n",
                offsetEXIFBase[0], offsetEXIFBase[0], filename, __LINE__);
        goto failure;
    }

    retCode = TIFFGetField(tif, EXIFTAG_EXIFVERSION, &pAscii0);
    retCode += TIFFGetField(tif, EXIFTAG_SUBSECTIMEORIGINAL, &pAscii1);
    if (retCode != 2)
    {
        fprintf(stderr, "Can't read EXIF tags. Testline %d\n", __LINE__);
        return 1;
    }

    /* Use shorter string thus EXIF-IFD should be overwritten. */
    sprintf(auxString, "EXIF-%d-XX", 0);
    if (!TIFFSetField(tif, EXIFTAG_SUBSECTIMEORIGINAL, auxString))
    {
        fprintf(stderr,
                "Can't set EXIFTAG_SUBSECTIMEORIGINAL tag. Testline %d\n",
                __LINE__);
        return 1;
    }
    if (!TIFFWriteCustomDirectory(tif, &offsetEXIFNew[0]))
    {
        fprintf(stderr,
                "TIFFWriteCustomDirectory() with EXIF failed. Testline %d\n",
                __LINE__);
        return 1;
    }
    /* Check for overwriting. */
    if (offsetEXIFBase[0] != offsetEXIFNew[0])
    {
        fprintf(stderr,
                "Failure: EXIF IFD 0 offset has changed from 0x%" PRIx64
                " (%" PRIu64 ") to 0x %" PRIx64 " (%" PRIu64 ").Testline %d\n ",
                offsetEXIFBase[0], offsetEXIFBase[0], offsetEXIFNew[0],
                offsetEXIFNew[0], __LINE__);
        goto failure;
    }
    else
    {
        /*EXIF has been overwritten. Check that. */
        if (!TIFFReadEXIFDirectory(tif, offsetEXIFNew[0]))
        {
            fprintf(stderr,
                    "Can't read EXIF directory at offset 0x%" PRIx64
                    " (%" PRIu64 ")  within %s. Testline %d\n",
                    offsetEXIFBase[0], offsetEXIFBase[0], filename, __LINE__);
            goto failure;
        }
        if (!TIFFGetField(tif, EXIFTAG_SUBSECTIMEORIGINAL, &pAscii1))
        {
            fprintf(stderr,
                    "Can't read tag EXIFTAG_SUBSECTIMEORIGINAL of %s. Testline "
                    "%d\n",
                    filename, __LINE__);
            goto failure;
        }
        if (strcmp(pAscii1, auxString))
        {
            fprintf(stderr,
                    "Read contents of EXIFTAG_SUBSECTIMEORIGINAL %s is not as "
                    "expected %s of %s. Testline "
                    "%d\n",
                    pAscii1, auxString, filename, __LINE__);
            goto failure;
        }
    }

    /* Enlarging and re-writing EXIF IFD 1. Since EXIF IFD 1 is at EOF,
     * even an enlargement should overwrite it. */
    if (!TIFFReadEXIFDirectory(tif, offsetEXIFBase[1]))
    {
        fprintf(stderr,
                "Can't read EXIF directory 1 at offset 0x%" PRIx64 " (%" PRIu64
                ")  within %s. Testline %d\n",
                offsetEXIFBase[1], offsetEXIFBase[1], filename, __LINE__);
        goto failure;
    }

    retCode = TIFFGetField(tif, EXIFTAG_EXIFVERSION, &pAscii0);
    retCode += TIFFGetField(tif, EXIFTAG_SUBSECTIMEORIGINAL, &pAscii1);
    if (retCode != 2)
    {
        fprintf(stderr, "Can't read EXIF tags. Testline %d\n", __LINE__);
        return 1;
    }

    /* Enlarge EXIF IFD 1 with additional tag to provoke re-writing. */
    if (!TIFFSetField(tif, EXIFTAG_FLASH, 44))
    {
        fprintf(stderr, "Can't set EXIFTAG_FLASH tag. Testline %d\n", __LINE__);
        goto failure;
    }
    if (!TIFFWriteCustomDirectory(tif, &offsetEXIFNew[1]))
    {
        fprintf(stderr,
                "TIFFWriteCustomDirectory() with EXIF failed. Testline %d\n",
                __LINE__);
        goto failure;
    }
    /* Check: EXIF IFD 1 should be overwritten because should be at EOF. */
    if (offsetEXIFBase[1] != offsetEXIFNew[1])
    {
        fprintf(stderr,
                "Failure: EXIF IFD 1 offset has changed from 0x%" PRIx64
                " (%" PRIu64 ") to 0x %" PRIx64 " (%" PRIu64 ").Testline %d\n ",
                offsetEXIFBase[1], offsetEXIFBase[1], offsetEXIFNew[1],
                offsetEXIFNew[1], __LINE__);
        goto failure;
    }

    /* Now enlarge and re-write EXIF IFD 0. */
    if (!TIFFReadEXIFDirectory(tif, offsetEXIFBase[0]))
    {
        fprintf(stderr,
                "Can't read EXIF directory at offset 0x%" PRIx64 " (%" PRIu64
                ")  within %s. Testline %d\n",
                offsetEXIFBase[0], offsetEXIFBase[0], filename, __LINE__);
        goto failure;
    }
    /* Enlarge EXIF IFD 0 with additional tag to provoke re-writing. */
    if (!TIFFSetField(tif, EXIFTAG_SPECTRALSENSITIVITY,
                      "EXIF-0-enlargement-EXIF"))
    {
        fprintf(stderr,
                "Can't set EXIFTAG_SPECTRALSENSITIVITY tag. Testline %d\n",
                __LINE__);
        goto failure;
    }
    if (!TIFFWriteCustomDirectory(tif, &offsetEXIFNew[0]))
    {
        fprintf(stderr,
                "TIFFWriteCustomDirectory() with EXIF failed. Testline %d\n",
                __LINE__);
        goto failure;
    }

    /* Check: EXIF-IFD 0 shall now be re-written. */
    if (offsetEXIFBase[0] == offsetEXIFNew[0])
    {
        fprintf(stderr,
                "Failure: EXIF IFD 0 offset has NOT changed from 0x%" PRIx64
                " (%" PRIu64 "). Testline %d\n ",
                offsetEXIFBase[0], offsetEXIFBase[0], __LINE__);
        goto failure;
    }
    else
    {
        /* EXIF-IFD has been re-written, thus offset in main-IFD needs
         * update. */
        if (!TIFFSetDirectory(tif, 1))
        {
            fprintf(stderr, "Can't set directory %d of %s. Testline %d\n", 1,
                    filename, __LINE__);
            goto failure;
        }
        if (!TIFFSetField(tif, TIFFTAG_EXIFIFD, offsetEXIFNew[0]))
        {
            fprintf(stderr, "Can't set TIFFTAG_EXIFIFD tag. Testline %d\n",
                    __LINE__);
            goto failure;
        }
        if (!TIFFWriteDirectory(tif))
        {
            fprintf(stderr, "Can't write directory to %s. Testline %d\n",
                    filename, __LINE__);
            goto failure;
        }
    }

    /*-- Make a little check. --*/
    if (!TIFFSetDirectory(tif, 1))
    {
        fprintf(stderr, "Can't set directory 1 of %s. Testline %d\n", filename,
                __LINE__);
        goto failure;
    }
    if (!is_requested_directory(tif, 1, filename))
    {
        goto failure;
    }
    if (!TIFFSetDirectory(tif, 0))
    {
        fprintf(stderr, "Can't set directory 0 of %s. Line %d\n", filename,
                __LINE__);
        goto failure;
    }
    if (!is_requested_directory(tif, 0, filename))
    {
        goto failure;
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
} /*-- test_EXIF_enlargement --*/

/* Checks that SubIFDs on file can be overwritten if they have not grown.
 * But SubIFDs cannot be re-written to another location in the file,
 * because TIFFWriteDirectory() cannot determine from which main-IFD the SubIFD
 * offset was taken for TIFFSetSubDirectory(). This shall result in an error
 * return of TIFFWriteDirectory().
 */
int test_SubIFD_enlargement(const char *filename, bool is_big_tiff)
{

    /* Define the number of sub-IFDs you are going to write */
#define NUMBER_OF_DIRS 2
#define NUMBER_OF_SUBIFDs 3
    uint16_t number_of_sub_IFDs = NUMBER_OF_SUBIFDs;
    toff_t sub_IFDs_offsets[NUMBER_OF_SUBIFDs] = {
        0UL}; /* array for SubIFD tag */

    int i;
    int blnWriteSubIFD = 0;
    int iIFD = 0, iSubIFD = 0;
    TIFFErrorHandler errHandler = NULL;

    uint64_t offsetBase[NUMBER_OF_DIRS];

    /*-- Create a file and write NUMBER_OF_DIRS IFDs with
     * NUMBER_OF_SUBIFDs SubIFDs to this file. --*/
    TIFF *tif = TIFFOpen(filename, is_big_tiff ? "w8" : "w");
    if (!tif)
    {
        fprintf(stderr, "Can't create %s. Testline %d\n", filename, __LINE__);
        return 1;
    }

    for (i = 0; i < NUMBER_OF_DIRS + NUMBER_OF_SUBIFDs; i++)
    {
        if (write_data_to_current_directory(
                tif, WIDTH, LENGTH, FALSE,
                blnWriteSubIFD ? 200 + iSubIFD++ : iIFD++, true, NULL, 0))
        {
            fprintf(
                stderr,
                "Can't write data to current directory in %s. Testline %d\n",
                filename, __LINE__);
            goto failure;
        }
        if (blnWriteSubIFD)
        {
            /* SUBFILETYPE tag is not mandatory for SubIFD writing, but a
             * good idea to indicate thumbnails */
            if (!TIFFSetField(tif, TIFFTAG_SUBFILETYPE, FILETYPE_REDUCEDIMAGE))
                goto failure;
        }

        /* For the first multi-page image, trigger TIFFWriteDirectory() to
         * switch for the next number_of_sub_IFDs calls to add those as SubIFDs
         * e.g. for thumbnails */
        if (0 == i)
        {
            blnWriteSubIFD = 1;
            /* Now here is the trick: the next n directories written
             * will be sub-IFDs of the main-IFD (where n is number_of_sub_IFDs
             * specified when you set the TIFFTAG_SUBIFD field.
             * The SubIFD offset array sub_IFDs_offsets is filled automatically
             * with the proper offset values by the following number_of_sub_IFDs
             * TIFFWriteDirectory() calls and are updated in the related
             * main-IFD with the last call.
             */
            if (!TIFFSetField(tif, TIFFTAG_SUBIFD, number_of_sub_IFDs,
                              sub_IFDs_offsets))
                goto failure;
        }
        if (!TIFFWriteDirectory(tif))
        {
            fprintf(stderr, "Can't write directory to %s. Testline %d\n",
                    filename, __LINE__);
            goto failure;
        }
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
    if (numberOfMainIFDs !=
        (tdir_t)(NUMBER_OF_DIRS + NUMBER_OF_SUBIFDs) - number_of_sub_IFDs)
    {
        fprintf(stderr,
                "Unexpected number of directories in %s. Expected %i, "
                "found %i.\n",
                filename, NUMBER_OF_DIRS - number_of_sub_IFDs,
                numberOfMainIFDs);
        goto failure;
    }

    /* Retrieve IFD offset values for later checking. */
    TIFFSetDirectory(tif, 1);
    offsetBase[1] = TIFFCurrentDirOffset(tif);
    TIFFSetDirectory(tif, 0);
    offsetBase[0] = TIFFCurrentDirOffset(tif);

    /* The first directory is now read. */
    /* Check if there are SubIFD subfiles */
    void *ptr;
    if (TIFFGetField(tif, TIFFTAG_SUBIFD, &number_of_sub_IFDs, &ptr))
    {
        /* Copy SubIFD array from pointer */
        memcpy(sub_IFDs_offsets, ptr,
               number_of_sub_IFDs * sizeof(sub_IFDs_offsets[0]));
    }
    /* Read first SubIFD directory */
    if (!TIFFSetSubDirectory(tif, sub_IFDs_offsets[0]))
        goto failure;
    if (!is_requested_directory(tif, 200 + 0, filename))
    {
        goto failure;
    }
    /* Just overwrite SubIFD with smaller text. */
    if (!TIFFSetField(tif, TIFFTAG_PAGENAME, "99 th. X"))
    {
        fprintf(stderr, "Can't set TIFFTAG_PAGENAME tag. Testline %d\n",
                __LINE__);
        goto failure;
    }
    if (!TIFFWriteDirectory(tif))
    {
        fprintf(stderr, "Can't write directory to %s. Testline %d\n", filename,
                __LINE__);
        goto failure;
    }

    /* Enlarging and re-writing SubIFD 1 shall fail.
     * TIFFWriteDirectory() cannot determine from which main-IFD the SubIFD
     * offset was taken for TIFFSetSubDirectory().
     * Therefore, SubIFD can be overwritten but cannot be re-written to another
     * location in the file.
     */
    if (!TIFFSetSubDirectory(tif, sub_IFDs_offsets[1]))
        goto failure;
    if (!TIFFSetField(tif, TIFFTAG_PAGENAME,
                      "Long Pagename to trigger re-writing"))
    {
        fprintf(stderr, "Can't set TIFFTAG_PAGENAME tag. Testline %d\n",
                __LINE__);
        goto failure;
    }
    /* Suppress error message from 'TIFFRewriteDirectory()' like
     * 'Error fetching directory link' or 'Sanity check...'. */
    errHandler = TIFFSetErrorHandler(NULL);
    if (TIFFWriteDirectory(tif))
    {
        fprintf(stderr, "Was expected to fail but succeeded. %s. Testline %d\n",
                filename, __LINE__);
        goto failure;
    }
    TIFFSetErrorHandler(errHandler);

    /*-- Make a little check. --*/
    if (!TIFFSetDirectory(tif, 1))
    {
        fprintf(stderr, "Can't set directory 1 of %s. Testline %d\n", filename,
                __LINE__);
        goto failure;
    }
    if (!is_requested_directory(tif, 1, filename))
    {
        goto failure;
    }
    if (offsetBase[1] != TIFFCurrentDirOffset(tif))
    {
        fprintf(stderr, "IFD 1 offset has moved. %s. Line %d\n", filename,
                __LINE__);
        goto failure;
    }

    if (!TIFFSetDirectory(tif, 0))
    {
        fprintf(stderr, "Can't set directory 0 of %s. Line %d\n", filename,
                __LINE__);
        goto failure;
    }
    if (!is_requested_directory(tif, 0, filename))
    {
        goto failure;
    }
    if (offsetBase[0] != TIFFCurrentDirOffset(tif))
    {
        fprintf(stderr, "IFD 0 offset has moved. %s. Line %d\n", filename,
                __LINE__);
        goto failure;
    }

    /* Check SubIFDs */
    for (i = 0; i < NUMBER_OF_SUBIFDs; i++)
    {
        if (!TIFFSetSubDirectory(tif, sub_IFDs_offsets[i]))
            goto failure;
        int k = (i == 0 ? 99 : 200 + i);
        if (!is_requested_directory(tif, k, filename))
        {
            fprintf(stderr, "Can't read SubIFD %d of %s correctly. Line %d\n",
                    k, filename, __LINE__);
            goto failure;
        }
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
} /*-- test_SubIFD_enlargement --*/

/* Checks that sequences that use TIFFCheckpointDirectory() to store the current
 * IFD in a file overwrite it if it has not grown, and write it to another
 * location if the IFD size has grown.
 */
int test_CheckpointDirectory(const char *filename, bool is_big_tiff)
{

    uint64_t auxUint64;
    uint64_t offsetBase[2];
    uint64_t offsetNew[2];
    char auxString[128];

    /*-- Create a file and write two IFDs directly to it --*/
    TIFF *tif = TIFFOpen(filename, is_big_tiff ? "w8" : "w");
    if (!tif)
    {
        fprintf(stderr, "Can't create %s. Testline %d\n", filename, __LINE__);
        return 1;
    }

    for (int i = 0; i < 2; i++)
    {
        /* Writing baseline images (IFDs) to file. */
        if (write_data_to_current_directory(tif, WIDTH, LENGTH, FALSE, i, true,
                                            NULL, 0))
        {
            fprintf(
                stderr,
                "Can't write data to current directory in %s. Testline %d\n",
                filename, __LINE__);
            goto failure;
        }
        /* CheckpointDirectory testing */
        if (!TIFFCheckpointDirectory(tif))
        {
            fprintf(stderr,
                    "Can't checkpoint directory %d of %s. Testline %d\n", i,
                    filename, __LINE__);
            goto failure;
        }
        offsetBase[i] = offsetNew[i] = TIFFCurrentDirOffset(tif);
        if (i == 1)
        {
            /* Provoke checkpointed IFD to be re-written by TIFFWriteDirectory()
             * below. */
            sprintf(auxString, "%d th. IMG", i);
            if (!TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, auxString))
            {
                fprintf(stderr,
                        "Can't set TIFFTAG_IMAGEDESCRIPTION tag. Testline %d\n",
                        __LINE__);
                goto failure;
            }
        }
        if (!TIFFWriteDirectory(tif))
        {
            fprintf(stderr, "Can't write directory to %s. Testline %d\n",
                    filename, __LINE__);
            goto failure;
        }
        /* Check IFD offsets:
         * First IFD shall be overwritten (same offset) and second IFD shall be
         * re-written (different offset). */
        if (!TIFFSetDirectory(tif, i))
        {
            fprintf(stderr, "Can't set directory %d of %s. Testline %d\n", i,
                    filename, __LINE__);
            goto failure;
        }
        auxUint64 = TIFFCurrentDirOffset(tif);
        if ((i == 0) && (auxUint64 != offsetBase[i]))
        {
            fprintf(stderr,
                    "Checkpointed offset 0x%" PRIx64
                    " is different to finally written offset 0x%" PRIx64
                    " Testline %d\n",
                    auxUint64, offsetBase[i], __LINE__);
            goto failure;
        }
        else if ((i == 1) && (auxUint64 == offsetBase[i]))
        {
            fprintf(stderr,
                    "Checkpointed offset 0x%" PRIx64
                    " is not different to finally written offset 0x%" PRIx64
                    " Testline %d\n",
                    auxUint64, offsetBase[i], __LINE__);
            goto failure;
        }
        /* Update offsetNew for later use. */
        offsetNew[i] = TIFFCurrentDirOffset(tif);
        /* Because of TIFFSetDirectory() prepare now new empty IFD for next
         * loop. */
        TIFFCreateDirectory(tif);
    }
    TIFFClose(tif);

    /* Reopen prepared testfile */
    tif = TIFFOpen(filename, "r+");
    if (!tif)
    {
        fprintf(stderr, "Can't create %s\n", filename);
        goto failure;
    }

    /* First IFD is read. Change something and checkpoint it several times. */
    /* Change IFD identification number string of PageName tag. */
    sprintf(auxString, "%d th. IFD", 9);
    if (!TIFFSetField(tif, TIFFTAG_PAGENAME, auxString))
    {
        fprintf(stderr, "Can't set TIFFTAG_PAGENAME tag.\n");
        return 1;
    }
    if (!TIFFCheckpointDirectory(tif))
    {
        fprintf(stderr, "Can't checkpoint directory %d of %s. Testline %d\n", 0,
                filename, __LINE__);
        goto failure;
    }
    auxUint64 = TIFFCurrentDirOffset(tif);
    if (auxUint64 != offsetBase[0])
    {
        fprintf(stderr,
                "Checkpointed offset 0x%" PRIx64
                " is different to finally written offset 0x%" PRIx64
                " Testline %d\n",
                auxUint64, offsetBase[0], __LINE__);
        goto failure;
    }
    /* Provoke checkpointed IFD to be re-written due to additional tag in IFD 0.
     */
    sprintf(auxString, "%d th. IMG YYY", 9);
    if (!TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, auxString))
    {
        fprintf(stderr, "Can't set TIFFTAG_IMAGEDESCRIPTION tag. Testline %d\n",
                __LINE__);
        goto failure;
    }
    if (!TIFFCheckpointDirectory(tif))
    {
        fprintf(stderr, "Can't checkpoint directory %d of %s. Testline %d\n", 0,
                filename, __LINE__);
        goto failure;
    }
    auxUint64 = TIFFCurrentDirOffset(tif);
    if (auxUint64 == offsetBase[0])
    {
        fprintf(stderr,
                "Checkpointed offset 0x%" PRIx64
                " is NOT different to finally written offset 0x%" PRIx64
                " Testline %d\n",
                auxUint64, offsetBase[0], __LINE__);
        goto failure;
    }
    offsetNew[0] = TIFFCurrentDirOffset(tif);

    if (!TIFFWriteDirectory(tif))
    {
        fprintf(stderr, "Can't write directory to %s. Testline %d\n", filename,
                __LINE__);
        goto failure;
    }

    /* Close and reopen prepared testfile */
    TIFFClose(tif);
    tif = TIFFOpen(filename, "r+");
    if (!tif)
    {
        fprintf(stderr, "Can't create %s\n", filename);
        goto failure;
    }

    /*-- Make a little check. --*/
    if (!TIFFSetDirectory(tif, 1))
    {
        fprintf(stderr, "Can't set directory 1 of %s. Testline %d\n", filename,
                __LINE__);
        goto failure;
    }
    if (!is_requested_directory(tif, 1, filename))
    {
        goto failure;
    }
    if (!TIFFSetDirectory(tif, 0))
    {
        fprintf(stderr, "Can't set directory 0 of %s. Line %d\n", filename,
                __LINE__);
        goto failure;
    }
    if (!is_requested_directory(tif, 9, filename))
    {
        goto failure;
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
} /*-- test_CheckpointDirectory --*/

/* LibTIFF increments IFD write-pointer always to even addresses, thus leaving 1
 * byte gaps in the data space for data outside IFD entries. This routine
 * checks correct handling for that.
 */
int test_OddDataSizes(const char *filename, unsigned int openMode)
{

    uint64_t auxUint64;
    uint64_t offsetBase;

    assert(openMode < (sizeof(modeStrings) / sizeof(modeStrings[0])));

    /*-- Create a file and write IFDs directly to it --*/
    TIFF *tif = TIFFOpen(filename, modeStrings[openMode]);
    if (!tif)
    {
        fprintf(stderr, "Can't create %s. Testline %d\n", filename, __LINE__);
        return 1;
    }

    /* Write 15 byte field (14 byte + zero).
     * Set it before other tags, otherwise it will be directly at EOF,
     * because custom tag data are written in the sequence they are set.
     * At EOF no gap will be inserted!*/
    if (!TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, "123456789ABCDE"))
    {
        fprintf(stderr, "Can't set TIFFTAG_IMAGEDESCRIPTION tag. Testline %d\n",
                __LINE__);
        goto failure;
    }
    /* Writing baseline images (IFDs) to file. */
    if (write_data_to_current_directory(tif, WIDTH, LENGTH, FALSE, 0, TRUE,
                                        NULL, 0))
    {
        fprintf(stderr,
                "Can't write data to current directory in %s. Testline %d\n",
                filename, __LINE__);
        goto failure;
    }
    if (!TIFFWriteDirectory(tif))
    {
        fprintf(stderr, "Can't write directory to %s. Testline %d\n", filename,
                __LINE__);
        goto failure;
    }
    /* Now write some data so that IFD 0 ends not at EOF. */
    if (write_data_to_current_directory(tif, WIDTH, LENGTH, FALSE, 1, TRUE,
                                        NULL, 0))
    {
        fprintf(stderr,
                "Can't write data to current directory in %s. Testline %d\n",
                filename, __LINE__);
        goto failure;
    }

    /* Now start odd / even data size testing with IFD 0 ...*/
    if (!TIFFSetDirectory(tif, 0))
    {
        fprintf(stderr, "Can't set directory %d of %s. Testline %d\n", 0,
                filename, __LINE__);
        goto failure;
    }
    offsetBase = TIFFCurrentDirOffset(tif);

    /* Write now 16 byte field (15 byte + zero), which shall be overwritten,
     * because 1 byte "gap" could be used. */
    if (!TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, "123456789ABCDEF"))
    {
        fprintf(stderr, "Can't set TIFFTAG_IMAGEDESCRIPTION tag. Testline %d\n",
                __LINE__);
        goto failure;
    }
    if (!TIFFWriteDirectory(tif))
    {
        fprintf(stderr, "Can't write directory to %s. Testline %d\n", filename,
                __LINE__);
        goto failure;
    }
    /* Check IFD offset: IFD shall be overwritten (same offset). */
    if (!TIFFSetDirectory(tif, 0))
    {
        fprintf(stderr, "Can't set directory %d of %s. Testline %d\n", 0,
                filename, __LINE__);
        goto failure;
    }
    auxUint64 = TIFFCurrentDirOffset(tif);

    if (auxUint64 != offsetBase)
    {
        fprintf(stderr,
                "Odd to even data size: Offset 0x%" PRIx64
                " is different to first written offset 0x%" PRIx64
                " Testline %d\n",
                auxUint64, offsetBase, __LINE__);
        goto failure;
    }

    /* Write now 17 byte field (16 byte + zero), which shall be re-written. */
    if (!TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, "123456789ABCDEFX"))
    {
        fprintf(stderr, "Can't set TIFFTAG_IMAGEDESCRIPTION tag. Testline %d\n",
                __LINE__);
        goto failure;
    }
    if (!TIFFWriteDirectory(tif))
    {
        fprintf(stderr, "Can't write directory to %s. Testline %d\n", filename,
                __LINE__);
        goto failure;
    }
    /* Check IFD offset: IFD shall be re-written (different offset). */
    if (!TIFFSetDirectory(tif, 0))
    {
        fprintf(stderr, "Can't set directory %d of %s. Testline %d\n", 0,
                filename, __LINE__);
        goto failure;
    }
    auxUint64 = TIFFCurrentDirOffset(tif);

    if (auxUint64 == offsetBase)
    {
        fprintf(stderr,
                "Odd to even data size: Offset 0x%" PRIx64 " has NOT changed."
                " Testline %d\n",
                auxUint64, __LINE__);
        goto failure;
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
} /*-- test_OddDataSizes --*/

int main()
{

    int retval = 0;
    int retvalLast = 0;
    int ntest = 0;
    char filename[128] = {0};

    /* clang-format off */
    fprintf(stderr,"==== Testing IFD enlargement (overwriting or re-writing of IFD). ====\n");
    for (int tiled = 0; tiled <= 1; tiled++)
    {
        for (int strileWriting = 0; strileWriting <= 1; strileWriting++)
        {
            for (unsigned int openMode = 0; openMode < 4; openMode++)
            {
                sprintf(filename, "test_ifd_enlargement_%02d_%s_%s%s.tif", ntest,
                        modeStrings[openMode], (tiled ? "TL" : "ST"),
                        (strileWriting ? "_strileW" : ""));
                /* clang-format off */
                retval += test_IFD_enlargement(filename, openMode, strileWriting, 1, 1, 2, tiled);  ntest++;
                if (retval != retvalLast) { fprintf(stderr, "    >>>> Test %d FAILED  (openMode %s; strileWriting=%d; tiled=%d). <<<<\n\n", ntest, modeStrings[openMode], strileWriting, tiled); retvalLast = retval; }
                retval += test_IFD_enlargement(filename, openMode, strileWriting, 1, 6, 12, tiled);  ntest++;
                if (retval != retvalLast) { fprintf(stderr, "    >>>> Test %d FAILED  (openMode %s; strileWriting=%d; tiled=%d). <<<<\n\n", ntest, modeStrings[openMode], strileWriting, tiled); retvalLast = retval; }
                retval += test_IFD_enlargement(filename, openMode, strileWriting, 2, 1, 2, tiled);  ntest++;
                if (retval != retvalLast) { fprintf(stderr, "    >>>> Test %d FAILED  (openMode %s; strileWriting=%d; tiled=%d). <<<<\n\n", ntest, modeStrings[openMode], strileWriting, tiled); retvalLast = retval; }
                retval += test_IFD_enlargement(filename, openMode, strileWriting, 2, 6, 12, tiled);  ntest++;
                if (retval != retvalLast) { fprintf(stderr, "    >>>> Test %d FAILED  (openMode %s; strileWriting=%d; tiled=%d). <<<<\n\n", ntest, modeStrings[openMode], strileWriting, tiled); retvalLast = retval; }
                retval += test_IFD_enlargement(filename, openMode, strileWriting, 3, 1, 2, tiled);  ntest++;
                if (retval != retvalLast) { fprintf(stderr, "    >>>> Test %d FAILED  (openMode %s; strileWriting=%d; tiled=%d). <<<<\n\n", ntest, modeStrings[openMode], strileWriting, tiled); retvalLast = retval; }
                retval += test_IFD_enlargement(filename, openMode, strileWriting, 3, 6, 12, tiled);  ntest++;
                if (retval != retvalLast) { fprintf(stderr, "    >>>> Test %d FAILED  (openMode %s; strileWriting=%d; tiled=%d). <<<<\n\n", ntest, modeStrings[openMode], strileWriting, tiled); retvalLast = retval; }
                retval += test_IFD_enlargement(filename, openMode, strileWriting, 3, 40, 22, tiled);  ntest++;
                if (retval != retvalLast) { fprintf(stderr, "    >>>> Test %d FAILED  (openMode %s; strileWriting=%d; tiled=%d). <<<<\n\n", ntest, modeStrings[openMode], strileWriting, tiled); retvalLast = retval; }
                retval += test_IFD_enlargement(filename, openMode, strileWriting, 1, 20, 10, tiled);  ntest++;
                if (retval != retvalLast) { fprintf(stderr, "    >>>> Test %d FAILED  (openMode %s; strileWriting=%d; tiled=%d). <<<<\n\n", ntest, modeStrings[openMode], strileWriting, tiled); retvalLast = retval; }
                retval += test_IFD_enlargement(filename, openMode, strileWriting, 2, 20, 10, tiled);  ntest++;
                if (retval != retvalLast) { fprintf(stderr, "    >>>> Test %d FAILED  (openMode %s; strileWriting=%d; tiled=%d). <<<<\n\n", ntest, modeStrings[openMode], strileWriting, tiled); retvalLast = retval; }
                retval += test_IFD_enlargement(filename, openMode, strileWriting, 3, 20, 10, tiled);  ntest++;
                if (retval != retvalLast) { fprintf(stderr, "    >>>> Test %d FAILED  (openMode %s; strileWriting=%d; tiled=%d). <<<<\n\n", ntest, modeStrings[openMode], strileWriting, tiled); retvalLast = retval; }
                /* clang-format on */
            }
        }
    }
    /* clang-format off */

    /* test EXIF IFD, SubIFD and IFD checkpointing */
    retval += test_EXIF_enlargement("test_ifd_EXIF_enlargement.tif", FALSE);  ntest++;
    if (retval != retvalLast) { fprintf(stderr, "    >>>> Test %d FAILED. <<<<\n\n", ntest); retvalLast = retval; }
    retval += test_EXIF_enlargement("test_ifd_EXIF_enlargementBig.tif", TRUE);  ntest++;
    if (retval != retvalLast) { fprintf(stderr, "    >>>> Test %d FAILED. <<<<\n\n", ntest); retvalLast = retval; }

    retval += test_SubIFD_enlargement("test_SubIFD_enlargement.tif", FALSE);  ntest++;
    if (retval != retvalLast) { fprintf(stderr, "    >>>> Test %d FAILED. <<<<\n\n", ntest); retvalLast = retval; }
    retval += test_SubIFD_enlargement("test_SubIFD_enlargementBig.tif", TRUE);  ntest++;
    if (retval != retvalLast) { fprintf(stderr, "    >>>> Test %d FAILED. <<<<\n\n", ntest); retvalLast = retval; }

    retval += test_CheckpointDirectory("test_CheckpointDirectory.tif", FALSE);  ntest++;
    if (retval != retvalLast) { fprintf(stderr, "    >>>> Test %d FAILED. <<<<\n\n", ntest); retvalLast = retval; }
    retval += test_CheckpointDirectory("test_CheckpointDirectoryBig.tif", TRUE);  ntest++;
    if (retval != retvalLast) { fprintf(stderr, "    >>>> Test %d FAILED. <<<<\n\n", ntest); retvalLast = retval; }

    retval += test_OddDataSizes("test_OddDataSizes.tif", 0);  ntest++;
    if (retval != retvalLast) { fprintf(stderr, "    >>>> Test %d FAILED. <<<<\n\n", ntest); retvalLast = retval; }

    if (!retval)
    {
        fprintf(stderr, "==== Testing IFD enlargement finished OK. ====\n");
    }
    else
    {
        fprintf(stderr, "==== Testing IFD enlargement finished with ERROR. ====\n");
    }

    return retval;
}
