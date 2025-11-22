tiff2bw
=======

.. program:: tiff2bw

Synopsis
--------

**tiff2bw** [ *options* ] *input.tif* *output.tif*

Description
-----------

:program:`tiff2bw` converts an RGB or Palette color TIFF
image to a greyscale image by
combining percentages of the red, green, and blue channels.
By default, output samples are created by taking
28% of the red channel, 59% of the green channel, and 11% of
the blue channel.  To alter these percentages, the
:option:`-R`, :option:`-G`, and :option:`-B` options may be used.

Options
-------

.. option:: -c compress

  Specify a compression scheme to use when writing image data:
  :command:`-c none` for no compression,
  :command:`-c packbits` for the PackBits compression algorithm,
  :command:`-c zip` for the Deflate compression algorithm,
  :command:`-c g3` for the CCITT Group 3 compression algorithm,
  :command:`-c g4` for the CCITT Group 4 compression algorithm,
  :command:`-c lzw` for Lempel-Ziv & Welch (the default).

.. option:: -r striprows

  Write data with a specified number of rows per strip;
  by default the number of rows/strip is selected so that each strip
  is approximately 8 kilobytes.

.. option:: -R redperc

  Specify the percentage of the red channel to use (default 28).

.. option:: -G greenperc

  Specify the percentage of the green channel to use (default 59).

.. option:: -B blueperc

  Specify the percentage of the blue channel to use (default 11).

See also
--------

:doc:`pal2rgb` (1),
:doc:`tiffinfo` (1),
:doc:`tiffcp` (1),
:doc:`tiffmedian` (1),
:doc:`/functions/libtiff` (3tiff),
