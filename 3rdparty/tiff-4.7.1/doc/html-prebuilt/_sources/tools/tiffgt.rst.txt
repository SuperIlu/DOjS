tiffgt
======

.. program:: tiffgt

Synopsis
--------

**tiffgt** [ *options* ] *input.tif* …

Description
-----------

:program:`tiffgt` displays one or more images stored using the
Tag Image File Format, Revision 6.0.

:program:`tiffgt` correctly handles files with any of the following characteristics:

  .. list-table:: Supported tag values
    :widths: 5 20
    :header-rows: 1

    * - Tag
      - Value

    * - BitsPerSample
      - 1, 2, 4, 8, 16
    * - SamplesPerPixel
      - 1, 3, 4 (the 4th sample is ignored)
    * - PhotometricInterpretation
      - 0 (min-is-white), 1 (min-is-black), 2 (RGB), 3 (palette), 6 (YCbCr)
    * - PlanarConfiguration
      - 1 (contiguous), 2 (separate)
    * - Orientation
      - 1 (top-left), 4 (bottom-left)

Data may be organized as strips or tiles and may be
compressed with any of the compression algorithms supported
by the :program:`libtiff` library.

For palette images (``PhotometricInterpretation=3``), :program:`tiffgt`
inspects the colormap values and assumes either 16-bit
or 8-bit values according to the maximum value.
That is, if no colormap entry greater than 255 is found,
:program:`tiffgt` assumes the colormap has only 8-bit values; otherwise
it assumes 16-bit values.
This inspection is done to handle old images written by
previous (incorrect) versions of
:program:`libtiff`.

:program:`tiffgt` can be used to display multiple images one-at-a-time.
The left mouse button switches the display to the first image in the
*next* file in the list of files specified on the command line.
The right mouse button switches to the first image in the
*previous* file in the list.
The middle mouse button causes the first image in the first file
specified on the command line to be displayed.
In addition the following keyboard commands are recognized:

  .. list-table:: Action keys
    :widths: 2 23
    :header-rows: 1

    * - Key
      - Description

    * - :kbd:`b`
      - Use a ``PhotometricInterpretation``
        of ``MinIsBlack`` in displaying the current image
    * - :kbd:`l`
      - Use a ``FillOrder`` of lsb-to-msb in decoding the current image
    * - :kbd:`m`
      - Use a ``FillOrder`` of msb-to-lsb in decoding the current image
    * - :kbd:`c`
      - Use a colormap visual to display the current image
    * - :kbd:`r`
      - Use a true color (24-bit RGB) visual to display the current image
    * - :kbd:`w`
      - Use a ``PhotometricInterpretation`` of ``MinIsWhite`` in displaying
        the current image
    * - :kbd:`W`
      - Toggle (enable/disable) display of warning messages from the
        TIFF library when decoding images
    * - :kbd:`E`
      - Toggle (enable/disable) display of error messages from the
        TIFF library when decoding images.
    * - :kbd:`z`
      - Reset all parameters to their default settings (``FillOrder``,
        ``PhotometricInterpretation``, handling of warnings and errors)
    * - :kbd:`PageUp`
      - Display the previous image in the current file or the last
        image in the previous file
    * - :kbd:`PageDown`
      - Display the next image in the current file or the first image
        in the next file
    * - :kbd:`Home`
      - Display the first image in the current file
    * - :kbd:`End`
      - Display the last image in the current file (unimplemented)

Options
-------

.. option:: -c

  Force image display in a colormap window.

.. option:: -d

  Specify an image to display by directory number.
  By default the first image in the file is displayed.
  Directories are numbered starting at zero.

.. option:: -e

  Enable reporting of error messages from the TIFF library.
  By default :program:`tiffgt` silently ignores images that cannot be read.

.. option:: -f

  Force :program:`tiffgt` to run as a foreground process.
  By default :program:`tiffgt` will place itself in the background once it
  has opened the requested image file.

.. option:: -l

  Force the presumed bit ordering to be LSB to MSB.

.. option:: -m

  Force the presumed bit ordering to be MSB to LSB.


.. option:: -o

  Specify an image to display by directory offset.
  By default the first image in the file is displayed.
  Directories offsets may be specified using C-style syntax;
  i.e. a leading ``0x`` for hexadecimal and a leading ``0`` for octal.


.. option:: -p

  Override the value of the ``PhotometricInterpretation``
  tag; the parameter may be one of: ``miniswhite`` ,
  ``minisblack``, ``rgb``, ``palette``, ``mask``,
  ``separated``, ``ycbcr``, and ``cielab``.

.. option:: -r

  Force image display in a full color window.

.. option:: -s

  Stop on the first read error.
  By default all errors in the input data are ignored and 
  :program:`tiffgt`
  does it's best to display as much of an image as possible.

.. option:: -w
  
  Enable reporting of warning messages from the TIFF library.
  By default :program:`tiffgt`
  ignores warning messages generated when reading an image.

.. option:: -v

  Place information in the title bar describing
  what type of window (full color or colormap) is being
  used, the name of the input file, and the directory
  index of the image (if non-zero).
  By default, the window type is not shown in the title bar.

Bugs
----

Images wider and taller than the display are silently truncated to avoid
crashing old versions of the window manager.

See also
--------

:doc:`tiffdump` (1),
:doc:`tiffinfo` (1),
:doc:`tiffcp` (1),
:doc:`/functions/libtiff` (3tiff)
