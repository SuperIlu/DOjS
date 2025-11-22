tiffmedian
==========

.. program:: tiffmedian

Synopsis
--------

**tiffmedian** [ *options* ] *input.tif* *output.tif*

Description
-----------

:program:`tiffmedian` applies the median cut algorithm to an RGB
image in *input.tif* to generate a palette image that is written to
*output.tif*.  The generated colormap has, by default, 256 entries.
The image data is quantized by mapping each pixel to the closest color
values in the colormap.

Options
-------

.. option:: -c

  Specify the compression to use for data written to the output file:
  :command:`-c none` for no compression,
  :command:`-c  packbits` for PackBits compression,
  :command:`-c lzw` for Lempel-Ziv & Welch compression,
  :command:`-c zip` for Deflate compression.

  By default :program:`tiffmedian` will compress data according to the
  value of the ``Compression`` tag found in the source file.

  LZW compression can be specified together with a ``predictor`` value.
  A predictor value of 2 causes each scanline of the output image to
  undergo horizontal differencing before it is encoded; a value of 1
  forces each scanline to be encoded without differencing.
  LZW-specific options are specified by appending a ``:``-separated
  list to the ``lzw`` option; e.g. ``-c lzw:2`` for LZW compression
  with horizontal differencing.

.. option:: -C

  Specify the number of entries to use in the generated colormap.
  By default all 256 entries/colors are used.

.. option:: -f

  Apply Floyd-Steinberg dithering before selecting a colormap entry.

.. option:: -r

  Specify the number of rows (scanlines) in each strip of data
  written to the output file.  By default, :program:`tiffmedian`
  attempts to set the rows/strip that no more than 8 kilobytes of
  data appear in a strip.

Notes
-----

This program is derived from Paul Heckbert's :program:`median` program.

See also
--------

:doc:`pal2rgb` (1),
:doc:`tiffinfo` (1),
:doc:`tiffcp` (1),
:doc:`tiffcmp` (1),
:doc:`/functions/libtiff` (3tiff),

**"Color Image Quantization for Frame Buffer Display"**,
Paul Heckbert, SIGGRAPH proceedings, 1982, pp. 297-307.
