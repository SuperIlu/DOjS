tiff2pdf
========

.. program:: tiff2pdf

Synopsis
--------

**tiff2pdf** [ *options* ] *input.tiff*

Description
-----------

:program:`tiff2pdf` opens a TIFF image and writes a PDF document to standard output.

The program converts one TIFF file to one PDF file, including multiple page
TIFF files, tiled TIFF files, black and white. grayscale, and color TIFF 
files that contain data of TIFF photometric interpretations of bilevel, 
grayscale, RGB, YCbCr, CMYK separation, and ICC L*a*b* as supported by 
:program:`libtiff` and PDF.

If you have multiple TIFF files to convert into one PDF file then use :program:`tiffcp`
or other program to concatenate the files into a multiple page TIFF file.  
If the input TIFF file is of huge dimensions (greater than 10000 pixels height
or width) convert the input image to a tiled TIFF if it is not already.

The standard output is standard output.  Set the output file name with the
:option:`-o` option.

All black and white files are compressed into a single strip CCITT G4 Fax
compressed PDF, unless tiled, where tiled black and white images are
compressed into tiled CCITT G4 Fax compressed PDF, 
:program:`libtiff` CCITT support is assumed.

Color and grayscale data can be compressed using either JPEG compression,
ITU-T T.81, or Zip/Deflate LZ77 compression.  Set the compression type using
the :option:`-j` or :option:`-z` options.  JPEG compression support
requires that :program:`libtiff` be configured with JPEG support, and
Zip/Deflate compression support requires that :program:`libtiff`
be configured with Zip support.  Use only one or the other of
:option:`-j` and :option:`-z`.

If the input TIFF contains single strip CCITT G4 Fax compressed information,
then that is written to the PDF file without transcoding, unless the options 
of no compression and no passthrough are set, :option:`-d` and :option:`-n`.

If the input TIFF contains JPEG or single strip Zip/Deflate compressed
information, and they are configured, then that is written to the PDF file 
without transcoding, unless the options of no compression and no passthrough 
are set.

The default page size upon which the TIFF image is placed is determined by
the resolution and extent of the image data.  Default values for the TIFF 
image resolution can be set using the :option:`-x` and :option:`-y`
options.  The page size can be set using the :option:`-p`
option for paper size, or :option:`-w` and :option:`-l`
for paper width and length, then each page of the TIFF image is centered on
its page.  The distance unit for default resolution and page width and
length can be set by the :option:`-u` option, the default unit is inch.

Various items of the output document information can be set with the
:option:`-e`, :option:`-c`, :option:`-a`, :option:`-t`, :option:`-s`,
and :option:`-k` options.  Setting the argument of the option to ``""`` for these
tags causes the relevant document information field to be not written.  Some 
of the document information values otherwise get their information from the 
input TIFF image, the software, author, document name, and image description.

The Portable Document Format (PDF) specification is copyrighted by Adobe
Systems, Incorporated.

Options
-------

.. option:: -o output-file

  Set the output to go to file *output-file*

.. option:: -j

  Compress with JPEG (requires :program:`libjpeg` configured with
  :program:`libtiff`).

.. option:: -z

  Compress with Zip/Deflate (requires :program`zlib` configured with
  :program:`libtiff`).

.. option:: -q quality

  Set the compression quality, 1-100 for JPEG.

.. option:: -n

  Do not allow data to be converted without uncompressing, no compressed
  data passthrough.

.. option:: -b

  Set PDF ``Interpolate`` user preference.

.. option:: -d

  Do not compress (decompress) - except monochrome to CCITT Group 4.

.. option:: -i

  Invert colors.

.. option:: -p paper-size

  Set paper size, e.g., ``letter``,  ``legal``, ``A4``.

.. option:: -F

  Cause the tiff to fill the PDF page.

.. option:: -u [ i | m ]

  Set distance unit, ``i`` for inch, ``m`` for centimeter.

.. option:: -w width

  Set width in units.

.. option:: -l length

  Set length in units.

.. option:: -x xres

  Set x/width resolution default.

.. option:: -y yres

  Set y/length resolution default.

.. option:: -r [ d | o ]

  Set ``d`` for resolution default for images without resolution,
  ``o`` for resolution override for all images.

.. option:: -f

  Set PDF ``Fit Window`` user preference.

.. option:: -e YYYYMMDDHHMMSS

  Set document information date, overrides image or current date/time default,
  ``YYYYMMDDHHMMSS``.

.. option:: -c creator

  Set document information creator, overrides image software default.

.. option:: -a author

  Set document information author, overrides image artist default.

.. option:: -t title

  Set document information title, overrides image document name default.

.. option:: -s subject

  Set document information subject, overrides image image description default.

.. option:: -k keywords

  Set document information keywords.

.. option:: -m size

  Set memory allocation limit (in MiB). Default is 256MiB. Set to 0 to disable the limit.

.. option:: -h

  List usage reminder to stderr and exit.

Examples
--------

The following example would generate the file output.pdf from input.tiff:

.. highlight:: shell

::

    tiff2pdf -o output.pdf input.tiff

The following example would generate PDF output from input.tiff and write it 
to standard output:

::

    tiff2pdf input.tiff

The following example would generate the file output.pdf from input.tiff, 
putting the image pages on a letter sized page, compressing the output 
with JPEG, with JPEG quality 75, setting the title to ``Document``, and setting 
the ``Fit Window`` option:

::

    tiff2pdf -p letter -j -q 75 -t "Document" -f -o output.pdf input.tiff

See also
--------

:doc:`tiffcp` (1),
:doc:`tiff2ps` (1),
:doc:`/functions/libtiff` (3tiff),
