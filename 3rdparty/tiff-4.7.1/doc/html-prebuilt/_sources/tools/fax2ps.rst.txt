fax2ps
======

.. program:: fax2ps

Synopsis
--------

**fax2ps** [ *options* ] [ *file …* ]

Description
-----------

:program:`fax2ps` reads one or more TIFF
facsimile image files and prints a compressed form of
PostScript on the standard output that is suitable for printing.

By default, each page is scaled to reflect the
image dimensions and resolutions stored in the file.
The :option:`-x` and :option:`-y`
options can be used to specify the horizontal and vertical
image resolutions (lines/inch), respectively.
If the :option:`-S`
option is specified, each page is scaled to fill an output page.
The default output page is 8.5 by 11 inches.
Alternate page dimensions can be specified in inches with the
:option:`-W` and :option:`-H`
options.

By default :program:`fax2ps`
generates PostScript for all pages in the file.
The :option:`-p`
option can be used to select one or more pages from
a multi-page document.


:program:`fax2ps`
generates a compressed form of PostScript that is
optimized for sending pages of text to a PostScript
printer attached to a host through a low-speed link (such
as a serial line).
Each output page is filled with white and then only
the black areas are drawn.
The PostScript specification of the black drawing operations
is optimized by using a special font that encodes the
move-draw operations required to fill
the black regions on the page.
This compression scheme typically results in a substantially
reduced PostScript description, relative to the straightforward
imaging of the page with a PostScript
``image``
operator.
This algorithm can, however, be ineffective
for continuous-tone and white-on-black images.
For these images, it sometimes is more efficient to send
the raster bitmap image directly; see
.BR tiff2ps (1).

Options
-------

.. option:: -p number

  Print only the indicated page.
  Multiple pages may be printed by specifying
  this option more than once.

.. option::  -x resolution

  Use *resolution*
  as the horizontal resolution, in dots/inch, of the image data.
  By default this value is taken from the file.

.. option:: -y resolution

  Use *resolution*
  as the vertical resolution, in lines/inch, of the image data.
  By default this value is taken from the file.

.. option:: -S

  Scale each page of image data to fill the output page dimensions.
  By default images are presented according to the dimension
  information recorded in the TIFF file.

.. option:: -W width

  Use *width*
  as the width, in inches, of the output page.

.. option:: -H height

  Use *height*
  as the height, in inches, of the output page.

Diagnostics
-----------

Some messages about malformed TIFF images come from the
TIFF library.

Various messages about badly formatted facsimile images
may be generated due to transmission errors in received
facsimile.
:program:`fax2ps`
attempts to recover from such data errors by resynchronizing
decoding at the end of the current scanline.
This can result in long horizontal black lines in the resultant
PostScript image.

Notes
-----

If the destination printer supports PostScript Level II then
it is always faster to just send the encoded bitmap generated
by the :program:`tiff2ps` program.

Bugs
----

:program:`fax2ps`
should probably figure out when it is doing a poor
job of compressing the output and just generate 
PostScript to image the bitmap raster instead.

See also
--------

:doc`tiff2ps` (1), :doc:`/functions/libtiff` (3tiff)
