thumbnail
=========

.. program:: thumbnail

Synopsis
--------

**thumbnail** [ *options* ] *input.tif* *output.tif*

Description
-----------

:program:`thumbnail` is a program written to show how one might use the
``SubIFD`` tag (#330) to store thumbnail images.
:program:`thumbnail`
copies a TIFF Class F facsimile file to the output file
and for each image an 8-bit greyscale "thumbnail sketch" is created.
The output file contains the thumbnail image with the associated
full-resolution page linked below with the SubIFD tag.

By default, thumbnail images are 216 pixels wide by 274 pixels high.
Pixels are calculated by sampling and filtering the input image
with each pixel value passed through a contrast curve.

Options
-------

.. option:: -w width

  Specify the width of thumbnail images in pixels.

.. option:: -h height

  Specify the height of thumbnail images in pixels.

.. option:: -c contrast

  Specify a contrast curve to apply in generating the thumbnail images.
  By default pixels values are passed through a linear contrast curve
  that simply maps the pixel value ranges.  Alternative curves are:

  ======  ======================
  Curve   Description
  ======  ======================
  exp50   50% exponential curve
  exp60   60% exponential curve
  exp70   70% exponential curve
  exp80   80% exponential curve
  exp90   90% exponential curve
  exp     pure exponential curve
  linear  linear curve
  ======  ======================

Bugs
----

There are no options to control the format of the saved thumbnail images.

See also
--------

:doc:`tiffdump` (1),
:doc:`tiffgt` (1),
:doc:`tiffinfo` (1),
:doc:`/functions/libtiff` (3tiff),
