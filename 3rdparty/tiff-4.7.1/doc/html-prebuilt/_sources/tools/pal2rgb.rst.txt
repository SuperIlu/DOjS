pal2rgb
=======

.. program:: pal2rgb

Synopsis
--------

**pal2rgb** [ *options* ] *input.tif* *output.tif*

Description
-----------

:program:`pal2rgb` converts a palette color image to a full color image by
applying the colormap of the palette image to each sample to generate a full color
RGB image.

Options
-------

Options that affect the interpretation of input data are:

.. option:: -C numentries

  This option overrides the default behavior of :program:`pal2rgb`
  in determining whether or not
  colormap entries contain 16-bit or 8-bit values.
  By default the colormap is inspected and
  if no colormap entry greater than 255 is found,
  the colormap is assumed to have only 8-bit values; otherwise
  16-bit values (as required by the TIFF
  specification) are assumed.
  The
  :option:`-C`
  option can be used to explicitly specify the number of
  bits for colormap entries:
  :command:`-C 8` for 8-bit values, 
  :command:`-C 16` for 16-bit values.

Options that affect the output file format are:

.. option:: -p planarconfig

  Explicitly select the planar configuration used in organizing
  data samples in the output image:
  :command:`-p contig` for samples packed contiguously, and
  :command:`-p separate` for samples stored separately.
  By default samples are packed.

.. option:: -c compress

  Use the specified compression algorithm to encoded image data
  in the output file:
  :command:`-c packbits` for Macintosh Packbits,
  :command:`-c lzw` for Lempel-Ziv & Welch,
  :command:`-c zip` for Deflate,
  :command:`-c none`
  for no compression.
  If no compression-related option is specified, the input
  file's compression algorithm is used.

.. option:: -r striprows

  Explicitly specify the number of rows in each strip of the
  output file. If the :option:`-r`
  option is not specified, a number is selected such that each
  output strip has approximately 8 kilobytes of data in it.

Bugs
----

Only 8-bit images are handled.

See also
--------

:doc:`tiffinfo` (1),
:doc:`tiffcp` (1),
:doc:`tiffmedian` (1),
:doc:`/functions/libtiff` (3tiff)
