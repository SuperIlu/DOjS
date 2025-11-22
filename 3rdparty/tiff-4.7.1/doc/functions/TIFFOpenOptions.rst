TIFFOpenOptions
===============

Synopsis
--------

.. highlight:: c

::

    #include <tiffio.h>

.. c:type:: TIFFOpenOptions TIFFOpenOptions

.. c:function:: TIFFOpenOptions* TIFFOpenOptionsAlloc(void)

.. c:function:: void TIFFOpenOptionsFree(TIFFOpenOptions*)

.. c:function:: void TIFFOpenOptionsSetMaxSingleMemAlloc(TIFFOpenOptions* opts, tmsize_t max_single_mem_alloc)

.. c:function:: void TIFFOpenOptionsSetMaxCumulatedMemAlloc(TIFFOpenOptions* opts, tmsize_t max_cumulated_mem_alloc)

.. c:function:: void TIFFOpenOptionsSetErrorHandlerExtR(TIFFOpenOptions* opts, TIFFErrorHandlerExtR handler, void* errorhandler_user_data)

.. c:function:: void TIFFOpenOptionsSetWarningHandlerExtR(TIFFOpenOptions* opts, TIFFErrorHandlerExtR handler, void* warnhandler_user_data)

.. c:function:: void TIFFOpenOptionsSetWarnAboutUnknownTags(TIFFOpenOptions *opts, int warn_about_unknown_tags)

Description
-----------

:c:type:`TIFFOpenOptions` is an opaque structure which can be passed
to the TIFF open"Ext" functions to define some ``libtiff`` internal settings.
The settings are the maximum single memory allocation limit and 
per-TIFF handle (re-entrant) error handler and warning handler functions.
For those handler a pointer to a **custom defined data structure** *user_data* 
can be given along.

:c:func:`TIFFOpenOptionsAlloc` allocates memory for the :c:type:`TIFFOpenOptions`
opaque structure and returns a :c:type:`TIFFOpenOptions` pointer. 

:c:func:`TIFFOpenOptionsFree` releases the allocated memory for
:c:type:`TIFFOpenOptions`. The allocated memory for :c:type:`TIFFOpenOptions`
can be released straight after successful execution of the related
TIFFOpen"Ext" functions like :c:func:`TIFFOpenExt`.

:c:func:`TIFFOpenOptionsSetMaxSingleMemAlloc` (added in libtiff 4.5.0) sets
the value for the maximum single memory limit in byte that ``libtiff`` internal
memory allocation functions are allowed to request per call.

.. note::
    However, the ``libtiff`` external functions :c:func:`_TIFFmalloc`
    and :c:func:`_TIFFrealloc` **do not apply** this internal memory
    allocation limit set by :c:func:`TIFFOpenOptionsSetMaxSingleMemAlloc`!

:c:func:`TIFFOpenOptionsSetMaxCumulatedMemAlloc` (added in libtiff 4.6.1) sets
the maximum cumulated memory allocations in byte, for a given TIFF handle,
that ``libtiff`` internal memory allocation functions are allowed.

.. note::
    However, the ``libtiff`` external functions :c:func:`_TIFFmalloc`
    and :c:func:`_TIFFrealloc` **do not apply** this internal memory
    allocation limit set by :c:func:`TIFFOpenOptionsSetMaxCumulatedMemAlloc`!

:c:func:`TIFFOpenOptionsSetErrorHandlerExtR` sets the function pointer to
an application-specific and per-TIFF handle (re-entrant) error handler.
Furthermore, a pointer to a **custom defined data structure** *errorhandler_user_data* 
can be passed. This error handler is invoked through :c:func:`TIFFErrorExtR`
and the *errorhandler_user_data* pointer is given along.
The *errorhandler_user_data* argument may be NULL.

:c:func:`TIFFOpenOptionsSetWarningHandlerExtR` works like
:c:func:`TIFFOpenOptionsSetErrorHandlerExtR` but for the warning handler,
which is invoked through  :c:func:`TIFFWarningExtR`

:c:func:`TIFFOpenOptionsSetWarnAboutUnknownTags` sets whether libtiff should
emit a warning when encountering a unknown tag. This function has been added in
libtiff 4.7.1 and the default value is FALSE (change of behaviour compared to
earlier versions).

Example
-------

::

    #include "tiffio.h"

    typedef struct MyErrorHandlerUserDataStruct
    {
        /* ... any user data structure ... */
    } MyErrorHandlerUserDataStruct;

    static int myErrorHandler(TIFF *tiff, void *user_data, const char *module,
                          const char *fmt, va_list ap)
    {
        MyErrorHandlerUserDataStruct *errorhandler_user_data =
            (MyErrorHandlerUserDataStruct *)user_data;
        /*... code of myErrorHandler ...*/
        return 1;
    }


    main()
    {
        tmsize_t limit = (256 * 1024 * 1024);
        MyErrorHandlerUserDataStruct user_data = { /* ... any data ... */};

        TIFFOpenOptions *opts = TIFFOpenOptionsAlloc();
        TIFFOpenOptionsSetMaxSingleMemAlloc(opts, limit);
        TIFFOpenOptionsSetErrorHandlerExtR(opts, myErrorHandler, &user_data);
        TIFF *tif = TIFFOpenExt("foo.tif", "r", opts);
        TIFFOpenOptionsFree(opts);
        /* ... go on here ... */

        TIFFClose(tif);
    }

Note
----

This functionality was introduced with libtiff 4.5.

See also
--------

:doc:`libtiff` (3tiff),
:doc:`TIFFOpen` (3tiff),
:doc:`TIFFError` (3tiff),
:doc:`TIFFWarning` (3tiff)
