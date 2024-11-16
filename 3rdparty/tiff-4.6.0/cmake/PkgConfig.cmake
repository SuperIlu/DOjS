# pkg-config support
#
# Copyright © 2015 Open Microscopy Environment / University of Dundee
# Copyright © 2021 Roger Leigh <rleigh@codelibre.net>
# Written by Roger Leigh <rleigh@codelibre.net>
#
# Permission to use, copy, modify, distribute, and sell this software and
# its documentation for any purpose is hereby granted without fee, provided
# that (i) the above copyright notices and this permission notice appear in
# all copies of the software and related documentation, and (ii) the names of
# Sam Leffler and Silicon Graphics may not be used in any advertising or
# publicity relating to the software without the specific, prior written
# permission of Sam Leffler and Silicon Graphics.
#
# THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
# EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
# WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
#
# IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
# ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
# OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
# LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
# OF THIS SOFTWARE.

# Post-process lib files into linker flags
function(set_libs_private out_var)
    set(tiff_libs_private "")
    foreach(lib IN LISTS ARGN)
        get_filename_component(name "${lib}" NAME)
        foreach(prefix IN LISTS CMAKE_FIND_LIBRARY_PREFIXES)
            if(NOT("${prefix}" STREQUAL "") AND name MATCHES "^${prefix}")
                string(REGEX REPLACE "^${prefix}" "" name "${name}")
                break()
            endif()
        endforeach()
        foreach(suffix IN LISTS CMAKE_FIND_LIBRARY_SUFFIXES)
            if(NOT("${suffix}" STREQUAL "") AND name MATCHES "${suffix}$")
                string(REGEX REPLACE "${suffix}$" "" name "${name}")
                break()
            endif()
        endforeach()
        string(APPEND tiff_libs_private " -l${name}")
    endforeach()
    set(${out_var} "${tiff_libs_private}" PARENT_SCOPE)
endfunction()

# Generate pkg-config file
set(prefix "${CMAKE_INSTALL_PREFIX}")
set(exec_prefix "\${prefix}")
if(IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}")
    set(libdir "${CMAKE_INSTALL_LIBDIR}")
else()
    set(libdir "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
endif()
if(IS_ABSOLUTE "${CMAKE_INSTALL_INCLUDEDIR}")
    set(includedir "${CMAKE_INSTALL_INCLUDEDIR}")
else()
    set(includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
endif()
set_libs_private(tiff_libs_private ${tiff_libs_private_list})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libtiff-4.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/libtiff-4.pc @ONLY)

# Install pkg-config file
if (tiff-install)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libtiff-4.pc
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
endif()
