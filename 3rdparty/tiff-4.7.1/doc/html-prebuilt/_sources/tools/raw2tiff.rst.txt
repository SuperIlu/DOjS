raw2tiff
========

.. program:: raw2tiff


Synopsis
--------

**raw2tiff** [ *options ] *input.raw* *output.tif*

Description
-----------

:program:`raw2tiff` converts a raw byte sequence into TIFF.
By default, the TIFF image is created with data samples packed
(``PlanarConfiguration=1``), compressed with the PackBits algorithm
(``Compression=32773``), and with each strip no more than 8 kilobytes.
These characteristics can overridden, or explicitly specified
with the options described below.

Options
-------

.. option:: -H number

  Size of input image file header in bytes (0 by default). This amount of data
  just will be skipped from the start of file while reading.

.. option:: -w number

  Width of input image in pixels (can be guessed, see :ref:`raw-guess` below).

.. option: -l number

  length of input image in lines (can be guessed, see :ref:`raw-guess` below).

:option: -b number

  number of bands in input image (1 by default).

.. option:: -d datatype

  type of samples in input image, where *datatype* may be one of:

  =========  ===================================
  Data type  Description
  =========  ===================================
  byte       8-bit unsigned integer (default)
  short      16-bit unsigned integer
  long       32-bit unsigned integer
  sbyte      8-bit signed integer
  sshort     16-bit signed integer
  slong      32-bit signed integer
  float      32-bit IEEE floating point
  double     64-bit IEEE floating point
  =========  ===================================

.. option:: -i config

  type of sample interleaving in input image, where *config* may be one of:

  =============  ================================
  Configuration  Description
  =============  ================================
  pixel          pixel interleaved data (default)
  band           band interleaved data.
  =============  ================================

.. option:: -p  photo

  photometric interpretation (color space) of the input image, where *photo* may
  be one of:

  ===========  ==============================================
  Photometric  Description
  ===========  ==============================================
  miniswhite   white color represented with 0 value
  minisblack   black color represented with 0 value (default)
  rgb          image has RGB color model
  cmyk         image has CMYK (separated) color model
  ycbcr        image has YCbCr color model
  cielab       image has CIE L*a*b color model
  icclab       image has ICC L*a*b color model
  itulab       image has ITU L*a*b color model
  ===========  ==============================================

.. option:: -s

  swap bytes fetched from the input file.

.. option:: -L

  input data has LSB2MSB bit order (default).

.. option:: -M

  input data has MSB2LSB bit order.

.. option:: -c

  Specify a compression scheme to use when writing image data:
  :command:`-c none` for no compression,
  :command:`-c packbits` for the PackBits compression algorithm (the default),
  :command:`-c jpeg` for the baseline JPEG compression algorithm,
  :command:`-c zip` for the Deflate compression algorithm,
  :command:`\-c lzw` for Lempel-Ziv & Welch.

.. option -r striprows

  Write data with a specified number of rows per strip;
  by default the number of rows/strip is selected so that each strip
  is approximately 8 kilobytes.

.. _raw-guess:

Guessing the image geometry
---------------------------

:program:`raw2tif` can guess image width and height in case one or both of these parameters are
not specified. If you omit one of those parameters, the complementary one will
be calculated based on the file size (taking into account header size, number
of bands and data type). If you omit both parameters, the statistical approach
will be used. Utility will compute correlation coefficient between two lines
at the image center using several appropriate line sizes and the highest
absolute value of the coefficient will indicate the right line size. That is
why you should be cautious with the very large images, because guessing
process may take a while (depending on your system performance). Of course, the
utility can't guess the header size, number of bands and data type, so it
should be specified manually. If you don't know anything about your image,
just try with the several combinations of those options.

There is no magic, it is just a mathematical statistics, so it can be wrong
in some cases. But for most ordinary images guessing method will work fine.

See also
--------

:doc:`pal2rgb` (1),
:doc:`tiffinfo` (1),
:doc:`tiffcp` (1),
:doc:`tiffmedian` (1),
:doc:`/functions/libtiff` (3tiff)
