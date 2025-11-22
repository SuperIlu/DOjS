BigTIFF Design
==============

The classic TIFF file format uses 32bit offsets and, as such, is limited to 4 gigabytes.
The **BigTIFF** design details a 64-bit (larger than 4GB) TIFF format specification.
The design is based on a proposal by Steve Carlsen of Adobe, with input
from various other parties.

Overview of BigTIFF Extension
-----------------------------

* The Version ID, in header bytes 2-3, formerly decimal 42, changes to **43** for BigTIFF.
* Header bytes 4-5 contain the decimal number **8**.

  * If there is some other number here, a reader should give up.
  * This is to provide a nice way to move to 16-byte pointers some day.

* Header bytes 6-7 are reserved and must be **zero**.

  * If they're not, a reader should give up.

* Header bytes 8-15 contain the 8-byte offset to the first IFD.
* Value/Offset fields are 8 bytes long, and take up bytes 8-15 in an IFD entry.

  * If the value is ≤ 8 bytes, it must be stored in the field.
  * All values must begin at an 8-byte-aligned address.

* 8-byte offset to the Next_IFD, at the end of an IFD.
* To keep IFD entries 8-byte-aligned, we begin with an 8-byte
  (instead of 2-byte) count of the number of directory entries.
* Add ``TIFFDataTypes`` of ``TIFF_LONG8`` (= 16), an 8-byte unsigned int,
  and ``TIFF_SLONG8`` (= 17), an 8-byte signed int.
* Add ``TIFFDataType`` ``TIFF_IFD8`` (=18) an 8-byte IFD offset.
* ``StripOffsets`` and ``TileOffsets`` and ``ByteCounts`` may be ``TIFF_LONG8``
  or the traditionally allowed ``TIFF_LONG`` or ``TIFF_SHORT``.

* The proposed extension is :file:`.tf8` or :file:`.btf`, and call it "BigTIFF".

Otherwise, it's just like "original TIFF" or "classic TIFF".

BigTIFF Structures compared to Classic TIFF
-------------------------------------------

TIFF and BigTIFF File Header
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Here's the classic TIFF file header ...

.. list-table:: Classic File Header
  :widths: auto
  :header-rows: 1
  :align: left
  :width: 90%

  * - Offset
    - Datatype
    - Value
  * - 0
    - UInt16
    - Byte order indication
  * - 2
    - UInt16
    - Version number (always 42)
  * - 4
    - UInt32
    - Offset to first IFD

And this is the **BigTIFF** file header ...

.. list-table:: BigTIFF File Header
  :widths: auto
  :header-rows: 1
  :align: left
  :width: 90%

  * - Offset
    - Datatype
    - Value
  * - 0
    - UInt16
    - Byte order indication
  * - 2
    - UInt16
    - Version number, always **43** for BigTIFF
  * - 4
    - UInt16
    - | Bytesize of offsets.
      | Always 8 in BigTIFF, it provides a nice way to move to 16-byte pointers some day.
      | If there is some other value here, a reader should give up.
  * - 6
    - UInt16
    - | Reserved, always 0.
      | If there is some other value here, a reader should give up.
  * - 8
    - UInt64
    - Offset to first IFD

The last members in both variants of the structure point to the first IFD
(Image File Directory).
This IFD can be located anywhere in the file. Every 'page' in a multi-page TIFF,
classic TIFF or BigTIFF, is represented by exactly one IFD.

TIFF and BigTIFF Image File Directory Structure
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Here's a more detailed view of the classic IFD ...

.. list-table:: Classic IFD
  :widths: auto
  :header-rows: 1
  :align: left

  * - Offset
    - Datatype
    - Value
  * - 0
    - UInt16
    - Number of tags in IFD
  * - 2 + x * 12
    - Tag structure
    - Tag data
  * - 2 + (Number of tags in IFD) * 12
    - UInt32
    - | Offset to next IFD, if there is a next IFD,
      | 0 otherwise.

And this is how the IFD looks like in the **BigTIFF** file format ...

.. list-table:: BigTIFF IFD
  :widths: auto
  :header-rows: 1
  :align: left

  * - Offset
    - Datatype
    - Value
  * - 0
    - UInt64
    - Number of tags in IFD
  * - 8 + x * 20
    - Tag structure
    - Tag data
  * - 8 + (Number of tags in IFD) * 20
    - UInt64
    - | Offset to next IFD, if there is a next IFD,
      | 0 otherwise.

TIFF and BigTIFF Tag Structure
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The tags in this IFD should be sorted by code, in both classic TIFF and BigTIFF.
Every tag takes up exactly 12 bytes in classic TIFF, and looks like this ...

.. list-table:: Classic Tag Structure
  :widths: auto
  :header-rows: 1
  :align: left

  * - Offset
    - Datatype
    -	Value
  * - 0
    - UInt16
    - Tag identifying code
  * - 2
    - UInt16
    - Datatype of tag data
  * - 4
    - UInt32
    - Number of values
  * - 8
    - | x * tag data datatype
      | or UInt32 offset
    - | Tag data
      | or offset to tag data

This same tag structure, in **BigTIFF**, takes up 20 bytes, and looks like this ...

.. list-table:: BigTIFF Tag Structure
  :widths: auto
  :header-rows: 1
  :align: left

  * - Offset
    - Datatype
    - Value
  * - 0
    - UInt16
    - Tag identifying code
  * - 2
    - UInt16
    - Datatype of tag data
  * - 4
    - UInt64
    - Number of values
  * - 12
    - | x * tag data datatype
      | or UInt64 offset
    - | Tag data
      | or offset to tag data

The same rule for 'inlining' the tag data applies to both classic TIFF and BigTIFF,
only the threshold size differs.
In classic TIFF, the tag data was written inside the tag structure, in the IFD,
if its size was smaller than or equal to 4 bytes. Otherwise, it's written elsewhere,
and pointed to. 
In BigTIFF, the tag data is written inside the tag structure, in the IFD,
if its size is smaller than or equal to 8 bytes.

Other miscellaneous details
---------------------------

Amongst the suggested file extensions are 'tif', 'tf8' and 'btf'.

Three datatypes are added to classic TIFF:

- ``TIFF_LONG8`` = 16, being unsigned 8-byte (64-bit) integer
- ``TIFF_SLONG8`` = 17, being signed 8-byte (64-bit) integer
- ``TIFF_IFD8`` = 18, being a new unsigned 8-byte (64-bit) IFD offset.

The ``StripOffsets``, ``StripByteCounts``, ``TileOffsets``, and ``TileByteCounts`` tags are allowed
to have the datatype ``TIFF_LONG8`` in BigTIFF. Old datatypes ``TIFF_LONG``, and ``TIFF_SHORT``
where allowed in the TIFF 6.0 specification, are still valid in BigTIFF, too.

Likewise, tags that point to other IFDs, like e.g. the SubIFDs tag, are now allowed
to have the datatype ``TIFF_IFD8`` in BigTIFF. Again, the old datatypes TIFF_IFD,
and the hardly recommendable ``TIFF_LONG``, are still valid, too.

Samples
-------
BigTIFF sample file BigTIFFSamples.zip contains a series of 8 different BigTIFF files.
Especially note the BigTIFFSamples.html file in this zip, documenting their characteristics.

That sample file from Joris Van Damme can still be found at the Wayback Machine:
`BigTIFF Example files <https://web.archive.org/web/20240329145231/https://www.awaresystems.be/imaging/tiff/bigtiff/BigTIFFSamples.zip>`_

