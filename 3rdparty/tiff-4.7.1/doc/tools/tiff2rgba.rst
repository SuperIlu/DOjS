tiff2rgba
=========

.. program:: tiff2rgba

Synopsis
--------

**tiff2rgba** [ *options* ] *input.tif* *output.tif*

Description
-----------

:program:`tiff2rgba` converts a wide variety of TIFF images into an RGBA TIFF image.  This
includes the ability to translate different color spaces and photometric
interpretation into RGBA, support for alpha blending, and translation
of many different bit depths into a 32bit RGBA image.

Internally this program is implemented using the :c:func:`TIFFReadRGBAImage`
function, and it suffers any limitations of that function.  This includes
limited support for > 8 ``BitsPerSample`` images, and flaws with some
esoteric combinations of ``BitsPerSample``, photometric interpretation, 
block organization and planar configuration.

The generated images are stripped images with four samples per pixel 
(red, green, blue and alpha) or if the :option:`-n` flag is used, three samples
per pixel (red, green, and blue).  The resulting images are always planar
configuration contiguous.  For this reason, this program is a useful utility
for transform exotic TIFF files into a form ingestible by almost any TIFF
supporting software. 

Options
-------

.. option:: -c

  Specify a compression scheme to use when writing image data:
  :command:`-c none` for no compression (the default),
  :command:`-c packbits` for the PackBits compression algorithm,
  :command:`-c zip` for the Deflate compression algorithm,
  :command:`-c jpeg` for the JPEG compression algorithm, and
  :command:`-c lzw` for Lempel-Ziv & Welch.

.. option:: -r striprows

  Write data with a specified number of rows per strip;
  by default the number of rows/strip is selected so that each strip
  is approximately 8 kilobytes.


.. option:: -b

  Process the image one block (strip/tile) at a time instead of by reading
  the whole image into memory at once.  This may be necessary for very large
  images on systems with limited RAM.

.. option:: -n

  Drop the alpha component from the output file, producing a pure RGB file.
  Currently this does not work if the :option:`-b` flag is also in effect.

.. option:: -B

  Use this value as the background when doing alpha compositing (dropping the
  alpha component).

.. option:: -M size

  Set maximum memory allocation size (in MiB). The default is 256MiB.
  Set to 0 to disable the limit.

See also
--------

:doc:`tiff2bw` (1),
:doc:`/functions/TIFFReadRGBAImage` (3tiff),
:doc:`/functions/libtiff` (3tiff),
