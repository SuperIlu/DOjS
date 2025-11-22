fax2tiff
========

.. program:: fax2tiff

Synopsis
--------

**fax2tiff** [ *options* ] [ **-o** *output.tif* ] *input.raw*

Description
-----------

:program:`fax2tiff` creates a TIFF file containing  CCITT
Group 3 or Group 4 encoded data from one or more files containing "raw"
Group 3 or Group 4 encoded data (typically obtained directly from a fax modem).
By default, each row of data in the resultant TIFF
file is 1-dimensionally encoded and
padded or truncated to 1728 pixels, as needed.
The resultant image is a set of low resolution (98 lines/inch)
or medium resolution (196 lines/inch)
pages, each of which is a single strip of data.
The generated file conforms to the TIFF
Class F (FAX) specification for storing facsimile data.
This means, in particular, that each page of the data does
**not** include the trailing
*"return to control"* (``RTC``) code; as required
for transmission by the CCITT Group 3 specifications.
The old, "classic", format is created if the
:option:`-c` option is used.
(The Class F format can also be requested with the
:option:`-f` option.)

The default name of the output image is :file:`fax.tif`;
this can be changed with the :option:`-o` option.
Each input file is assumed to be a separate page of facsimile data
from the same document.
The order in which input files are specified on the command
line is the order in which the resultant pages appear in the
output file.

Options
-------

Options that affect the interpretation of input data are:

.. option:: -3

  Assume input data is CCITT Group 3 encoded (default).

.. option:: -4

  Assume input data is CCITT Group 4 encoded.

.. option:: -U

  Assume input data is uncompressed (Group 3 or Group 4).

.. option:: -1

  Assume input data is encoded with the 1-dimensional version of the CCITT
  Group 3 Huffman encoding algorithm (default).

.. option:: -2

  Assume input data is 2-dimensional version of the CCITT
  Group 3 Huffman encoding algorithm.

.. option:: -P

  Assume input data is **not**
  EOL-aligned (default). This option has effect with Group 3 encoded input only.

.. option:: -A

  Assume input data is EOL-aligned. This option has effect with Group 3
  encoded input only.

.. option:: -M

  Treat input data as having bits filled from most significant bit (``MSB``) to most least bit (``LSB``).

.. option:: -L

  Treat input data as having bits filled from least significant bit (``LSB``) to most significant bit
  (``MSB``) (default).

.. option:: -B

  Assume input data was encoded with black as 0 and white as 1.

.. option:: -W

  Assume input data was encoded with black as 1 and white as 0 (default).

.. option:: -R

  Specify the vertical resolution, in lines/inch, of the input images.
  By default input are assumed to have a vertical resolution of 196 lines/inch.
  If images are low resolution facsimile, a value of 98 lines/inch should
  be specified.

.. option:: -X

  Specify the width, in pixels, of the input images.
  By default input are assumed to have a width of 1728 pixels.

Options that affect the output file format are:

.. option:: -o

  Specify the name of the output file.


.. option:: -7

  Force output to be compressed with the CCITT
  Group 3 Huffman encoding algorithm (default).

.. option:: -8

  Force output to be compressed with the CCITT
  Group 4 Huffman encoding.

.. option:: -u

  Force output to be uncompressed (Group 3 or Group 4).

.. option:: -5

  Force output to be encoded with the 1-dimensional version of the CCITT
  Group 3 Huffman encoding algorithm.

.. option:: -6

  Force output to be encoded with the 2-dimensional version of the CCITT
  Group 3 Huffman encoding algorithm (default).

.. option:: -a

  Force the last bit of each *"End Of Line"* (``EOL``)
  code to land on a byte boundary (default). This "zero padding" will
  be reflected in the contents of the ``Group3Options``
  tag of the resultant TIFF file. This option has effect with Group 3 encoded output only.

.. option:: -p

  Do not EOL-align output. This option has effect with Group 3 encoded
  output only.

.. option:: -c

  Generate "classic" Group 3 TIFF format.

.. option:: -f

  Generate TIFF Class F (TIFF/F) format (default).

.. option:: -m

  Force output data to have bits filled from most significant bit (``MSB``)
  to most least bit (``LSB``).

.. option:: -l

  Force output data to have bits filled from least significant bit (``LSB``)
  to most significant bit (``MSB``) (default).

.. option:: -r

  Specify the number of rows (scanlines) in each strip of data
  written to the output file.
  By default (or when value ``0`` is specified), :program:`tiffcp`
  attempts to set the rows/strip
  that no more than 8 kilobytes of data appear in a strip (with except of G3/G4
  compression schemes). If you specify special value ``-1``
  it will results in infinite number of the rows per strip. The entire image
  will be the one strip in that case. This is default in case of G3/G4 output
  compression schemes.

.. option:: -s

  Stretch the input image vertically by writing each input row of
  data twice to the output file.

.. option:: -v

  Force :program:`fax2tiff`
  to print the number of rows of data it retrieved from the input file.

.. option:: -z

  Force output to be compressed with the LZW encoding.

Diagnostics
-----------

The following warnings and errors come from the decoding
routines in the library.

**"Warning, %s: Premature EOL at scanline %d (x %d).\en"**:

  The input data had a row that was shorter than the expected width.
  The row is padded with white.

**"%s: Premature EOF at scanline %d (x %d).\en"**:

  The decoder ran out of data in the middle of a scanline.
  The resultant row is padded with white.

**"%s: Bad code word at row %d, x %d\en"**:

  An invalid Group 3 ``code``
  was encountered while decoding the input file.
  The row number and horizontal position is given.
  The remainder of the input row is discarded, while
  the corresponding output row is padded with white.

**"%s: Bad 2D code word at scanline %d.\en"**:

  An invalid Group 4 or 2D Group 3 ``code``
  was encountered while decoding the input file.
  The row number and horizontal position is given.
  The remainder of the input row is discarded, while
  the corresponding output row is padded with white.

Bugs
----

Input data are assumed to have a a "top left" orientation;
it should be possible to override this assumption
from the command line.

See also
--------

"CCITT Recommendation T.4"
(Standardization of Group 3 Facsimile Apparatus for Document Transmission).

"The Spirit of TIFF Class F",
an appendix to the TIFF 5.0 specification prepared by Cygnet Technologies.

:doc:`tiffinfo` (1),
:doc:`tiffdither` (1),
:doc:`tiffgt` (1),
:doc:`/functions/libtiff` (3tiff)
