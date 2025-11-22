####################################
LibTIFF - TIFF Library and Utilities
####################################

.. table:: References
    :widths: auto

    =====================  =====================================
    Name                   Reference
    =====================  =====================================
    Home Page #1           http://www.simplesystems.org/libtiff/
    Home Page #2           https://libtiff.gitlab.io/libtiff/
    Latest Stable Release  v4.7.1
    Primary Download Site  `download.osgeo.org <https://download.osgeo.org/libtiff/>`_
    Mailing List           `tiff@lists.osgeo.org <tiff@lists.osgeo.org>`_
    List subscription      http://lists.osgeo.org/mailman/listinfo/tiff/
    Source repository      https://gitlab.com/libtiff/libtiff
    =====================  =====================================

The LibTIFF software provides support for the *Tag Image File Format* (TIFF),
a widely used format for storing image data.  The latest version of
the TIFF specification is :doc:`specification/index`.

Included in this software distribution are:

* a library, :file:`libtiff`, for reading and writing TIFF images
* a small collection of tools for performing simple manipulations of TIFF images
* documentation for the library and tools.

The :file:`libtiff` library, along with associated tool programs, should handle most of
your needs for reading and writing TIFF images.

LibTIFF is portable software, supported on various operating systems including
UNIX (Linux, BSD, MacOS X) and Windows.

.. note::

    At libtiff v4.6.0, the source code for most TIFF tools (except tiffinfo,
    tiffdump, tiffcp and tiffset) has been moved to archive/ directory
    and was not built.
    tiff2ps and tiff2pdf source code has been moved in a unsupported category,
    no longer built by default, but were still part of the the source
    distribution.

    With libtiff v4.7.0 those tools were restored.


The following sections are included in this documentation:

.. toctree::
    :maxdepth: 2
    :titlesonly:

    specification/index
    images
    build
    terms
    libtiff
    multi_page
    functions
    internals
    addingtags
    tools
    contrib
    rfcs/index
    project/index
    releases/index
