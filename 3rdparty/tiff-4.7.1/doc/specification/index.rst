TIFF File Format Specification
==============================

.. image:: ../images/jim.gif
    :width: 139
    :alt: jim

| A copy of the 6.0 specification is available from the libtiff
  ftp site at `<https://download.osgeo.org/libtiff/doc/TIFF6.pdf>`_.
| Other places are `<https://www.itu.int/itudoc/itu-t/com16/tiff-fx/docs/tiff6.pdf>`_
  and `<https://www.loc.gov/preservation/digital/formats/fdd/fdd000022.shtml>`_

Draft :doc:`technote2` covers problems
with the TIFF 6.0 design for embedding JPEG-compressed data in TIFF, and
describes an alternative.

The LibTIFF coverage of the TIFF 6.0 specification is detailed in :doc:`coverage`.

A design for a TIFF variation supporting files larger than 4GB is detailed in :doc:`bigtiff`.

.. toctree::
    :maxdepth: 1
    :titlesonly:

    technote2
    coverage
    bigtiff
    coverage-bigtiff
