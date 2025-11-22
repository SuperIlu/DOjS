ppm2tiff
========

.. program:: ppm2tiff

Synopsis

**ppm2tiff** [ *options* ] [ *input.ppm* ] *output.tif*

Description
-----------

:program:`ppm2tiff` converts a file in the PPM, PGM and PBM image formats to
TIFF.  By default, the TIFF image is created with data samples packed
(``PlanarConfiguration=1``), compressed with the Packbits algorithm
(``Compression=32773``), and with each strip no more than 8 kilobytes.
These characteristics can be overridden, or explicitly specified with the
options described below.

If the PPM file contains greyscale data, then the ``PhotometricInterpretation``
tag is set to 1 (min-is-black), otherwise it is set to 2 (RGB).

If no PPM file is specified on the command line, :program:`ppm2tiff`
will read from the standard input.

Options
-------

.. option:: -c compress

   Specify a compression scheme to use when writing image data:
   :command:`-c none` for no compression,
   :command:`-c packbits` for PackBits compression (will be used by default),
   :command:`-c lzw` for Lempel-Ziv & Welch compression,
   :command:`-c jpeg` for baseline JPEG compression,
   :command:`-c zip` for Deflate compression,
   :command:`-c g3` for CCITT Group 3 (T.4) compression, and
   :command:`-c g4` for CCITT Group 4 (T.6) compression.

.. option:: -r striprows

  Write data with a specified number of rows per strip; by default the number of
  rows/strip is selected so that each strip is approximately 8 kilobytes.

.. option:: -R resolution

  Mark the resultant image to have the specified X and Y resolution (in
  dots/inch).

See also
--------

:doc:`tiffinfo` (1),
:doc:`tiffcp` (1),
:doc:`tiffmedian` (1),
:doc:`/functions/libtiff` (3tiff)
