tiffcmp
=======

.. program:: tiffcmp

Synopsis
--------

**tiffcmp** [ *options* ] *file1.tif* *file2.tif*

Description
-----------

:program:`tiffcmp` compares the tags and data in two files created according
to the Tagged Image File Format, Revision 6.0.
The schemes used for compressing data in each file
are immaterial when data are compared\-data are compared on
a scanline-by-scanline basis after decompression.
Most directory tags are checked; notable exceptions are:
``GrayResponseCurve``, ``ColorResponseCurve``, and ``ColorMap`` tags.
Data will not be compared if any of the ``BitsPerSample``,
``SamplesPerPixel``, or ``ImageWidth`` values are not equal.
By default, :program:`tiffcmp` will terminate if it encounters any difference.

Options
-------

.. option:: -l

  List each byte of image data that differs between the files.

.. option:: -z number

  List specified number of image data bytes that differs between the files.

.. option:: -t

  Ignore any differences in directory tags.

Bugs
----

Tags that are not recognized by the library are not
compared; they may also generate spurious diagnostics.

The image data of tiled files is not compared, since the :c:func:`TIFFReadScanline`
function is used.  An error will be reported for tiled files.

The pixel and/or sample number reported in differences may be off
in some exotic cases. 

See also
--------

:doc:`pal2rgb` (1),
:doc:`tiffinfo` (1),
:doc:`tiffcp` (1),
:doc:`tiffmedian` (1),
:doc:`/functions/libtiff` (3tiff)
