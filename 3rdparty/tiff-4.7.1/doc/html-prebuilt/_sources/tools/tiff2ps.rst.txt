tiff2ps
========

.. program:: tiff2ps

Synopsis
--------

**tiff2ps** [ *options* ] *input.tif* …

Description
-----------

:program:`tiff2ps` reads TIFF images and writes PostScript™ or Encapsulated
PostScript™ (EPS) on the standard output. By default :program:`tiff2ps`
writes Encapsulated PostScript™ for the first image in the specified
TIFF image file.

By default, :program:`tiff2ps` will generate PostScript™ that fills a
printed area specified by the TIFF tags in the input file.
If the file does not contain ``XResolution`` or ``YResolution``
tags, then the printed area is set according to the image dimensions.
The :option:`-w` and :option:`-h` options (see below)
can be used to set the dimensions of the printed area in inches;
overriding any relevant TIFF tags.

The PostScript™ generated for RGB, palette, and CMYK images uses the
``colorimage`` operator.  The PostScript™ generated for greyscale and
bilevel images uses the ``image`` operator.  When the ``colorimage``
operator is used, PostScript™ code to emulate this operator
on older PostScript™ printers is also generated.
Note that this emulation code can be very slow.

Color images with associated alpha data are composited over
a white background.

Options
-------

.. option:: -1

  Generate PostScript™ Level 1 (the default).

.. option:: -2

  Generate PostScript™ Level 2.

.. option:: -3

  Generate PostScript™ Level 3. It basically allows one to use the ```/flateDecode``
  filter for ZIP compressed TIFF images.

.. option:: -8

  Disable use of ASCII85 encoding with PostScript™ Level 2/3.

.. option:: -a

  Generate output for all IFDs (pages) in the input file.

.. option:: -b margin

  Specify the bottom margin for the output (in inches). This does not affect
  the height of the printed image.

.. option:: -c

  Center the image in the output. This option only shows an effect if both
  the :option:`-w` and the :option:`-h` option are given.

.. option:: -C name

  Specify the document creator name.

.. option:: -d dir

  Set the initial TIFF
  directory to the specified directory number.
  (NB: Directories are numbered starting at zero.)
  This option is useful for selecting individual pages in a
  multi-page (e.g. facsimile) file.

.. option:: -D

  Enable duplex printing (two pages per sheet of paper).

.. option:: -e

  Force the generation of Encapsulated PostScript™ (implies
  :option:`-z`).

.. option:: -h size

  Specify the vertical size of the printed area (in inches).

.. option:: -H

  Specify the maximum height of image (in inches). Images with larger sizes will
  be split in several pages. Option :option:`-L`
  may be used for specifying size of split images overlapping.

.. option:: -i

  Enable/disable pixel interpolation.  This option requires a
  single numeric value: zero to disable pixel interpolation and
  non-zero to enable.  The default is enabled.

.. option:: -L

  Specify the size of overlapping for split images (in inches). Used in
  conjunction with :option:`-H` and :option:`-W` options.

.. option:: -l

  Specify the left margin for the output (in inches). This does not affect
  the width of the printed image.

.. option:: -M size

  Set maximum memory allocation size (in MiB). The default is 256MiB.
  Set to 0 to disable the limit.


.. option:: -m

  Where possible render using the ``imagemask`` PostScript™ operator instead of the
  ``image`` operator.  When this option is specified :program:`tiff2ps` will use
  ``imagemask`` for rendering 1 bit deep images.  If this option is not specified
  or if the image depth is greater than 1 then the ``image`` operator is used.

.. option:: -o ifdoffset

  Set the initial TIFF directory to the IFD at the specified file offset.
  This option is useful for selecting thumbnail images and the
  like which are hidden using the ``SubIFD`` tag.

.. option:: -O

  Write PostScript™ to specified file instead of standard output.

.. option:: -p

  Force the generation of (non-Encapsulated) PostScript™.

.. option:: -P L|P

  Set optional PageOrientation DSC comment to Landscape or Portrait.

.. option:: -r 90|180|270|auto

  Rotate image by 90, 180, 270 degrees or auto.  Auto picks the best
  fit for the image on the specified paper size (eg portrait
  or landscape) if :option:`-h` or :option:`-w` is specified. Rotation is in degrees 
  counterclockwise. Auto rotates 90 degrees ccw to produce landscape.

.. option:: -s

  Generate output for a single IFD (page) in the input file.

.. option:: -t title

  Specify the document title string.

.. option:: -T

  Print pages for top edge binding.

.. option:: -w size

  Specify the horizontal size of the printed area (in inches).

.. option:: -W size

  Specify the maximum width of image (in inches). Images with larger sizes will
  be split in several pages. Options :option:`-L` and :option:`-W`
  are mutually exclusive.

.. option:: -x

  Override resolution units specified in the TIFF as centimeters.

.. option:: -y

  Override resolution units specified in the TIFF as inches.

.. option:: -z

  When generating PostScript™ Level 2, data is scaled so that it does not
  image into the ``deadzone`` on a page (the outer margin that the printing device
  is unable to mark).  This option suppresses this behavior.
  When PostScript™ Level 1 is generated, data is imaged to the entire printed
  page and this option has no affect.

Examples
--------

The following generates PostScript™ Level 2 for all pages of a facsimile:

.. highlight:: shell

::

    tiff2ps \-a2 fax.tif | lpr

Note also that if you have version 2.6.1 or newer of Ghostscript then you
can efficiently preview facsimile generated with the above command.

To generate Encapsulated PostScript™ for a the image at directory 2
of an image use:

::

    tiff2ps \-d 1 foo.tif

(Notice that directories are numbered starting at zero.)

If you have a long image, it may be split in several pages:

::

    tiff2ps \-h11 \-w8.5 \-H14 \-L.5 foo.tif > foo.ps

The page size is set to 8.5x11 by :option:`-w` and :option:`-h`
options. We will accept a small amount of vertical compression, so
:option:`-H` set to 14. Any pages between 11 and 14 inches will be fit onto one page.
Pages longer than 14 inches are cut off at 11 and continued on the next
page. The :command:`-L.5`
option says to repeat a half inch on the next page (to improve readability).

Bugs
----

Because PostScript™ does not support the notion of a colormap,
8-bit palette images produce 24-bit PostScript™ images.
This conversion results in output that is six times
bigger than the original image and which takes a long time
to send to a printer over a serial line.
Matters are even worse for 4-, 2-, and 1-bit palette images.

Does not handle tiled images when generating PostScript™ Level I output.

See also
--------

:doc:`pal2rgb` (1),
:doc:`tiffinfo` (1),
:doc:`tiffcp` (1),
:doc:`tiffgt` (1),
:doc:`tiffmedian` (1),
:doc:`tiff2bw` (1),
:doc:`/functions/libtiff` (3tiff),
