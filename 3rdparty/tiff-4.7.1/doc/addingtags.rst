Defining New TIFF Tags
======================

Libtiff has built-in knowledge of all the standard TIFF tags, as
well as extensions.  The following describes how to add knowledge of
new tags as builtins to libtiff, or how to application specific tags can
be used by applications without modifying libtiff.

.. _TIFFFieldInfo_Definition:

TIFFFieldInfo
-------------

How libtiff manages specific tags is primarily controlled by the
definition for that tag value stored internally as a TIFFFieldInfo structure.
This structure looks like this:

.. highlight:: c

::

    typedef struct {
      uint32_t       field_tag;        /* field's tag */
      short          field_readcount;  /* read count / TIFF_VARIABLE / TIFF_VARIABLE2 / TIFF_SPP */
      short          field_writecount; /* write count / TIFF_VARIABLE / TIFF_VARIABLE2 */
      TIFFDataType   field_type;       /* type of associated data */
      unsigned short field_bit;        /* bit in fieldsset bit vector */
      unsigned char  field_oktochange; /* if true, can change while writing */
      unsigned char  field_passcount;  /* if true, pass dir count on set */
      char          *field_name;       /* ASCII name */
    } TIFFFieldInfo;


.. c:member:: uint32_t TIFFFieldInfo.field_tag

    The tag number.  For instance 277 for the
    SamplesPerPixel tag.  Builtin tags will generally have a ``#define`` in
    :file:`tiff.h` for each known tag.

.. c:member:: short TIFFFieldInfo.field_readcount

    The number of values which should be read.
    The special value :c:macro:`TIFF_VARIABLE` (-1) indicates that a variable number of
    values may be read.  The special value :c:macro:`TIFFTAG_SPP` (-2) indicates that there
    should be one value for each sample as defined by :c:macro:`TIFFTAG_SAMPLESPERPIXEL`.
    The special value :c:macro:`TIFF_VARIABLE2` (-3) is similar to :c:macro:`TIFF_VARIABLE`
    but the required TIFFGetField() count value must be uint32_t* instead of uint16_t* as
    for :c:macro:`TIFF_VARIABLE` (-1).
    For ASCII fields with variable length, this field is :c:macro:`TIFF_VARIABLE`.

.. c:member:: short TIFFFieldInfo.field_writecount

    The number of values which should be written.
    Generally the same as field_readcount.  A few built-in exceptions exist, but
    I haven't analysed why they differ.

.. c:member:: TIFFDataType TIFFFieldInfo.field_type

    Type of the field.  One of :c:enumerator:`TIFF_BYTE`, :c:enumerator:`TIFF_ASCII`,
    :c:enumerator:`TIFF_SHORT`, :c:enumerator:`TIFF_LONG`,
    :c:enumerator:`TIFF_RATIONAL`, :c:enumerator:`TIFF_SBYTE`,
    :c:enumerator:`TIFF_UNDEFINED`, :c:enumerator:`TIFF_SSHORT`,
    :c:enumerator:`TIFF_SLONG`, :c:enumerator:`TIFF_SRATIONAL`,
    :c:enumerator:`TIFF_FLOAT`, :c:enumerator:`TIFF_DOUBLE` or
    :c:enumerator:`TIFF_IFD`.
    And for BigTIFF :c:enumerator:`TIFF_LONG8`,
    :c:enumerator:`TIFF_SLONG8` and :c:enumerator:`TIFF_IFD8`,
    which are automatically reverted to 4 byte fields in
    ClassicTIFF.

.. c:member:: unsigned short TIFFFieldInfo.field_bit

    Built-in tags stored in special fields in the
    TIFF structure have assigned field numbers to distinguish them (e.g.
    :c:macro:`FIELD_SAMPLESPERPIXEL`).  New tags should generally just use
    :c:macro:`FIELD_CUSTOM` indicating they are stored in the generic tag list.

.. c:member:: unsigned char TIFFFieldInfo.field_oktochange

    TRUE if it is OK to change this tag value
    while an image is being written.  FALSE for stuff that must be set once
    and then left unchanged (like ImageWidth, or PhotometricInterpretation for
    instance).

.. c:member:: unsigned char TIFFFieldInfo.field_passcount

    If TRUE, then the count value must be passed
    in :c:func:`TIFFSetField`, and :c:func:`TIFFGetField`, otherwise the count is not required.
    This should generally be TRUE for non-ascii variable count tags unless
    the count is implicit (such as with the colormap).

.. c:member:: char * TIFFFieldInfo.field_name

    A name for the tag.  Normally mixed case (studly caps)
    like ``StripByteCounts``, and relatively short.

Within :file:`tif_dirinfo.c` file, the built-in TIFF tags are defined with
:c:struct:`TIFFField` structure that has additional parameters defining the var_arg
interface of :c:func:`TIFFSetField` and :c:func:`TIFFGetField`.

Various functions exist for getting the internal :c:struct:`TIFFFieldInfo`
definitions, including :c:func:`_TIFFFindFieldInfo`, and
:c:func:`_TIFFFindFieldInfoByName`.  See
:file:`tif_dirinfo.c` for details.

.. _Tag_Auto_registration:

Default Tag Auto-registration
-----------------------------

In libtiff 3.6.0 a new mechanism was introduced allowing libtiff to
read unrecognised tags automatically.  When an unknown tags is encountered,
it is automatically internally defined with a default name and a type
derived from the tag value in the file.  Applications only need to predefine
application specific tags if they need to be able to set them in a file, or
if particular calling conventions are desired for :c:func:`TIFFSetField` and
:c:func:`TIFFGetField`.

When tags are autodefined like this the :c:member:`field_readcount` and
:c:member:`field_writecount` values are always :c:macro:`TIFF_VARIABLE2` (-3).  The
:c:member:`field_passcount` is always TRUE, and the :c:member:`field_bit` is
:c:macro:`FIELD_CUSTOM`.  The field name will be ``Tag %d`` where the ``%d``
is the tag number.

Thus, to read anonymous auto-registered tags use the following:

::

    uint32_t count;
    void* value;  //has to be a pointer to the expected value type.
    TIFFGetField(tif, TIFFTAG_UNKNOWN_TO_LIBTIFF, &count, &value);


.. _Define_Application_Tags:

Defining Application Tags
-------------------------

For various reasons, it is common for applications to want to define
their own tags to store information outside the core TIFF specification.
This is done by calling :c:func:`TIFFMergeFieldInfo` with one or more
:c:struct:`TIFFFieldInfo`.

The libgeotiff library provides geospatial information extensions within
a TIFF file.  First, an array of :c:struct:`TIFFFieldInfo` is prepared with
information on the new tags:

::

    static const TIFFFieldInfo xtiffFieldInfo[] = {

        /* XXX Insert Your tags here */
        { TIFFTAG_GEOPIXELSCALE,	-1,-1, TIFF_DOUBLE,	FIELD_CUSTOM,
          TRUE,	TRUE,	"GeoPixelScale" },
        { TIFFTAG_GEOTRANSMATRIX,	-1,-1, TIFF_DOUBLE,	FIELD_CUSTOM,
          TRUE,	TRUE,	"GeoTransformationMatrix" },
        { TIFFTAG_GEOTIEPOINTS,	-1,-1, TIFF_DOUBLE,	FIELD_CUSTOM,
          TRUE,	TRUE,	"GeoTiePoints" },
        { TIFFTAG_GEOKEYDIRECTORY, -1,-1, TIFF_SHORT,	FIELD_CUSTOM,
          TRUE,	TRUE,	"GeoKeyDirectory" },
        { TIFFTAG_GEODOUBLEPARAMS,	-1,-1, TIFF_DOUBLE,	FIELD_CUSTOM,
          TRUE,	TRUE,	"GeoDoubleParams" },
        { TIFFTAG_GEOASCIIPARAMS,	-1,-1, TIFF_ASCII,	FIELD_CUSTOM,
          TRUE,	FALSE,	"GeoASCIIParams" }
    };

In order to define the tags, we call :c:func:`TIFFMergeFieldInfo` on the
desired TIFF handle with the list of :c:struct:`TIFFFieldInfo`.

::

    #define	N(a)	(sizeof (a) / sizeof (a[0]))

    /* Install the extended Tag field info */
    TIFFMergeFieldInfo(tif, xtiffFieldInfo, N(xtiffFieldInfo));

The tags need to be defined for each TIFF file opened - and when reading
they should be defined before the tags of the file are read, yet a valid
:c:expr:`TIFF *` is needed to merge the tags against.  In order to get them
registered at the appropriate part of the setup process, it is necessary
to register our merge function as an extender callback with libtiff.
This is done with :c:func:`TIFFSetTagExtender`.  We also keep track of the
previous tag extender (if any) so that we can call it from our extender
allowing a chain of customizations to take effect.

::

    static TIFFExtendProc _ParentExtender = NULL;

    static
    void _XTIFFInitialize(void)
    {
        static int first_time=1;
	
        if (! first_time) return; /* Been there. Done that. */
        first_time = 0;
	
        /* Grab the inherited method and install */
        _ParentExtender = TIFFSetTagExtender(_XTIFFDefaultDirectory);
    }

The extender callback is looks like this.  It merges in our new fields
and then calls the next extender if there is one in effect.

::

    static void
    _XTIFFDefaultDirectory(TIFF *tif)
    {
        /* Install the extended Tag field info */
        TIFFMergeFieldInfo(tif, xtiffFieldInfo, N(xtiffFieldInfo));

        /* Since an XTIFF client module may have overridden
         * the default directory method, we call it now to
         * allow it to set up the rest of its own methods.
         */

        if (_ParentExtender)
            (*_ParentExtender)(tif);
    }

The above approach ensures that our new definitions are used when reading
or writing any TIFF file.  However, since on reading we already have
default definitions for tags, it is usually not critical to pre-define them.
If tag definitions are only required for writing custom tags, you can just
call :c:func:`TIFFMergeFieldInfo` before setting new tags.  The whole extender
architecture can then be avoided.

Adding New Builtin Tags
-----------------------

A similar approach is taken to the above.  However, the :c:struct:`_TIFFField`
information should be added to the :c:expr:`tiffFields[]` list in
:file:`tif_dirinfo.c`. This :c:struct:`_TIFFField` structure is like
TIFFFieldInfo structure but has additional members:

The tags in the :c:expr:`tiffFields[]` list need not to be in sorted
order by the tag number. Sorting is done when setting up the
:c:expr:`TIFFFieldArray` at IFD initialization.

.. highlight:: c

::

    typedef struct _TIFFField {
      uint32_t             field_tag;        /* field's tag */
      short                field_readcount;  /* read count / TIFF_VARIABLE / TIFF_VARIABLE2 / TIFF_SPP */
      short                field_writecount; /* write count / TIFF_VARIABLE / TIFF_VARIABLE2 */
      TIFFDataType         field_type;       /* type of associated data */
      uint32_t             field_anonymous;  /* if true, this is a unknown / anonymous tag */
      TIFFSetGetFieldType  set_field_type;   /* type to be passed to TIFFSetField and TIFFGetField */
      TIFFSetGetFieldType  get_field_type;   /* not used */
      unsigned short       field_bit;        /* bit in fieldsset bit vector */
      unsigned char        field_oktochange; /* if true, can change while writing */
      unsigned char        field_passcount;  /* if true, pass dir count on set */
      char                *field_name;       /* ASCII name */
      TIFFFieldArray      *field_subfields;  /* if field points to child ifds, child ifd field definition array */ 
    };


.. c:member:: uint32_t _TIFFField.field_anonymous

    Used internally to indicate auto-registered tags.
    See :c:func:`TIFFFieldIsAnonymous`.

.. c:member:: TIFFSetGetFieldType _TIFFField.set_field_type

    | The enummeration identifier gives a hint and defines the `vararg`
      parameters passed to :c:func:`TIFFSetField` and :c:func:`TIFFGetField`.
    | For example :c:enum:`TIFF_SETGET_UINT64` defines a single
      ``uint64_t`` parameter to be passed to `TIFFSetField()` and
      `TIFFGetField()`, respectively.
    | To distinguish the three different array types, there are three
      strings before the value type within the enummeration identifier
      (e.g. TIFF_SETGET_C0_UINT64):
    | "**_C0_**" for fixed arrays and thus no count parameter is required
      for TIFFSetField() and TIFFGetField(), respectively.
      The array length is constant and given by `field_readcount` or
      `field_writecount`, respectively.
    | "**_C16_**" and "**_C32_**" are for variable arrays with ``uint16_t``
      or ``uint32_t`` count parameter required for TIFFSetField() and
      TIFFGetField(), respectively. But then, `field_readcount` and
      `field_writecount` has to be set to -1 for _C16_ and -3 for _C32_,
      respectively.

.. c:member:: TIFFSetGetFieldType _TIFFField.get_field_type

    Currently, this parameter is not used for TIFFGetField().
    Nevertheless, it is set as a copy of set_field_type or set to
    `TIFF_SETGET_UNDEFINED`.

Normally new built-in tags should be defined with :c:macro:`FIELD_CUSTOM`
and then only two points need to be done:

#. Define the tag in :file:`tiff.h`.
#. Add an entry in the :c:expr:`tiffFields[]` array list defined at the
   top of :file:`tif_dirinfo.c`.

However, if it is desirable for the tag value to have its own field in
the :c:struct:`TIFFDirectory` structure, then you will also need to
``#define`` a new :c:macro:`FIELD_` value for it, and add appropriate
handling as follows:

#. Add a field to the :c:struct:`TIFFDirectory` structure in :file:`tif_dir.h`
   and define a ``FIELD_*`` bit number (also update the definition of
   :c:macro:`FIELD_CODEC` to reflect your addition).
#. Add entries in :c:func:`_TIFFVSetField` and :c:func:`_TIFFVGetField`
   for the new tag.
#. (*optional*) If the value associated with the tag is not a scalar value
   (e.g. the array for ``TransferFunction``) and requires
   special processing,
   then add the appropriate code to :c:func:`TIFFReadDirectory` and
   :c:func:`TIFFWriteDirectory`.  You're best off finding a similar tag and
   cribbing code.
#. Add support to :c:func:`TIFFPrintDirectory` in :file:`tif_print.c`
   to print the tag's value.

If you want to maintain portability, beware of making assumptions
about data types.  Use the typedefs (:c:type:`uint16_t`, etc. when dealing with
data on disk and ``t*_t`` when stuff is in memory) and be careful about
passing items through printf or similar vararg interfaces.

Adding New Codec-private Tags
-----------------------------

To add tags that are meaningful *only when a particular compression
algorithm is used* follow these steps:

#. Define the tag in :file:`tiff.h`.
#. Allocate storage for the tag values in the private state block of
   the codec.
#. Insure the state block is created when the codec is initialized.
#. At :c:func:`TIFFInitfoo` time override the method pointers in the
   :c:struct:`TIFF` structure for getting, setting and printing tag values.
   For example,

   ::

      sp->vgetparent = tif->tif_vgetfield;
      tif->tif_vgetfield = fooVGetField;	/* hook for codec tags */
      sp->vsetparent = tif->tif_vsetfield;
      tif->tif_vsetfield = fooVSetField;	/* hook for codec tags */
      tif->tif_printdir = fooPrintDir;	/* hook for codec tags */

   (Actually you may decide not to override the
   :c:member:`tif_printdir` method, but rather just specify it).
#. Create a private :c:struct:`TIFFFieldInfo` array for your tags and
   merge them into the core tags at initialization time using
   :c:func:`_TIFFMergeFieldInfo`; e.g.

   ::

       _TIFFMergeFieldInfo(tif, fooFieldInfo, N(fooFieldInfo));

   (where :c:macro:`N` is a macro used liberaly throughout the distributed code).

#. Fill in the get and set routines.  Be sure to call the parent method
   for tags that you are not handled directly.  Also be sure to set the
   ``FIELD_*`` bits for tags that are to be written to the file.  Note that
   you can create "pseudo-tags" by defining tags that are processed
   exclusively in the get/set routines and never written to file (see
   the handling of :c:macro:`TIFFTAG_FAXMODE` in :file:`tif_fax3.c`
   for an example of this).
#. Fill in the print routine, if appropriate.

Note that space has been allocated in the ``FIELD_*`` bit space for
codec-private tags.  Define your bits as ``FIELD_CODEC+<offset>`` to
keep them away from the core tags. If you need more tags than there
is room for, just increase :c:macro:`td_fieldsset` at the top of
:file:`tif_dir.h`.
