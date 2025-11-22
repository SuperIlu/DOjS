rgb2ycbcr
=========

.. program:: rgb2ycbcr

Synopsis
--------

**rgb2ycbcr** [ *options* ] *src1.tif src2.tif … dst.tif*

Description
-----------

:program:`rgb2ycbcr` converts RGB color, greyscale, or bi-level TIFF
images to YCbCr images by transforming and sampling pixel data. If multiple
files are specified on the command line each source file is converted to a
separate directory in the destination file.

By default, chrominance samples are created by sampling
2 by 2 blocks of luminance values; this can be changed with the
:option:`-h` and :option:`-v` options.
Output data are compressed with the ``PackBits``
compression scheme, by default; an alternate scheme can be selected with the
:option:`-c` option.
By default, output data are compressed in strips with
the number of rows in each strip selected so that the
size of a strip is never more than 8 kilobytes;
the :option:`-r`
option can be used to explicitly set the number of
rows per strip.

Options
-------

.. option:: -c compress

  Specify a compression scheme to use when writing image data:
  :command:`-c none` for no compression,
  :command:`-c packbits` for the PackBits compression algorithm (the default),
  :command:`-c jpeg` for the JPEG compression algorithm,
  :command:`\-c zip` for the deflate compression algorithm, and
  :command:`-c lzw` for Lempel-Ziv & Welch.

.. option:: -h size

  Set the horizontal sampling dimension to one of: 1, 2 (default), or 4.

.. option:: -r striprows

  Write data with a specified number of rows per strip;
  by default the number of rows/strip is selected so that each strip
  is approximately 8 kilobytes.

.. option:: -v size

  Set the vertical sampling dimension to one of: 1, 2 (default), or 4.

See also
--------

:doc:`tiffinfo` (1),
:doc:`tiffcp` (1),
:doc:`/functions/libtiff` (3tiff)
