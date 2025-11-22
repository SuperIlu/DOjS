# CMake build for libtiff
#
# Copyright © 2015 Open Microscopy Environment / University of Dundee
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

# Windows specific resource preparation
if (WIN32)
  message(STATUS "Include resource version information for TOOLS")
  set(MSVC_RESOURCE_FILE tif_tools_versioninfo.rc)
  source_group("Resource Files" FILES tif_tools_versioninfo.rc)
endif ()


add_executable(fax2ps ../placeholder.h)
target_sources(fax2ps PRIVATE fax2ps.c ${MSVC_RESOURCE_FILE})
set_target_properties(fax2ps PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(fax2ps PRIVATE tiff tiff_port)

add_executable(fax2tiff ../placeholder.h)
target_sources(fax2tiff PRIVATE fax2tiff.c ${MSVC_RESOURCE_FILE})
set_target_properties(fax2tiff PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(fax2tiff PRIVATE tiff tiff_port)

add_executable(pal2rgb ../placeholder.h)
target_sources(pal2rgb PRIVATE pal2rgb.c ${MSVC_RESOURCE_FILE})
set_target_properties(pal2rgb PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(pal2rgb PRIVATE tiff tiff_port)

add_executable(ppm2tiff ../placeholder.h)
target_sources(ppm2tiff PRIVATE ppm2tiff.c ${MSVC_RESOURCE_FILE})
set_target_properties(ppm2tiff PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(ppm2tiff PRIVATE tiff tiff_port)

add_executable(raw2tiff ../placeholder.h)
target_sources(raw2tiff PRIVATE raw2tiff.c ${MSVC_RESOURCE_FILE})
set_target_properties(raw2tiff PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(raw2tiff PRIVATE tiff tiff_port CMath::CMath)

add_executable(rgb2ycbcr ../placeholder.h)
target_sources(rgb2ycbcr PRIVATE rgb2ycbcr.c ${MSVC_RESOURCE_FILE})
set_target_properties(rgb2ycbcr PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(rgb2ycbcr PRIVATE tiff tiff_port)

add_executable(thumbnail ../placeholder.h)
target_sources(thumbnail PRIVATE thumbnail.c ${MSVC_RESOURCE_FILE})
set_target_properties(thumbnail PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(thumbnail PRIVATE tiff tiff_port CMath::CMath)

add_executable(tiff2bw ../placeholder.h)
target_sources(tiff2bw PRIVATE tiff2bw.c ${MSVC_RESOURCE_FILE})
set_target_properties(tiff2bw PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(tiff2bw PRIVATE tiff tiff_port)

add_executable(tiff2pdf ../placeholder.h)
target_sources(tiff2pdf PRIVATE tiff2pdf.c ${MSVC_RESOURCE_FILE})
set_target_properties(tiff2pdf PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(tiff2pdf PRIVATE tiff tiff_port)

add_executable(tiff2ps ../placeholder.h)
target_sources(tiff2ps PRIVATE tiff2ps.c ${MSVC_RESOURCE_FILE})
set_target_properties(tiff2ps PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(tiff2ps PRIVATE tiff tiff_port CMath::CMath)

add_executable(tiff2rgba ../placeholder.h)
target_sources(tiff2rgba PRIVATE tiff2rgba.c ${MSVC_RESOURCE_FILE})
set_target_properties(tiff2rgba PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(tiff2rgba PRIVATE tiff tiff_port)

add_executable(tiffcmp ../placeholder.h)
target_sources(tiffcmp PRIVATE tiffcmp.c ${MSVC_RESOURCE_FILE})
set_target_properties(tiffcmp PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(tiffcmp PRIVATE tiff tiff_port)

add_executable(tiffcp ../placeholder.h)
target_sources(tiffcp PRIVATE tiffcp.c ${MSVC_RESOURCE_FILE})
set_target_properties(tiffcp PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(tiffcp PRIVATE tiff tiff_port)

add_executable(tiffcrop ../placeholder.h)
target_sources(tiffcrop PRIVATE tiffcrop.c ${MSVC_RESOURCE_FILE})
set_target_properties(tiffcrop PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(tiffcrop PRIVATE tiff tiff_port CMath::CMath)

add_executable(tiffdither ../placeholder.h)
target_sources(tiffdither PRIVATE tiffdither.c ${MSVC_RESOURCE_FILE})
set_target_properties(tiffdither PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(tiffdither PRIVATE tiff tiff_port)

add_executable(tiffdump ../placeholder.h)
target_sources(tiffdump PRIVATE tiffdump.c ${MSVC_RESOURCE_FILE})
set_target_properties(tiffdump PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(tiffdump PRIVATE tiff tiff_port)

add_executable(tiffinfo ../placeholder.h)
target_sources(tiffinfo PRIVATE tiffinfo.c ${MSVC_RESOURCE_FILE})
set_target_properties(tiffinfo PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(tiffinfo PRIVATE tiff tiff_port)

add_executable(tiffmedian ../placeholder.h)
target_sources(tiffmedian PRIVATE tiffmedian.c ${MSVC_RESOURCE_FILE})
set_target_properties(tiffmedian PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(tiffmedian PRIVATE tiff tiff_port)

add_executable(tiffset ../placeholder.h)
target_sources(tiffset PRIVATE tiffset.c ${MSVC_RESOURCE_FILE})
set_target_properties(tiffset PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(tiffset PRIVATE tiff tiff_port)

add_executable(tiffsplit ../placeholder.h)
target_sources(tiffsplit PRIVATE tiffsplit.c ${MSVC_RESOURCE_FILE})
set_target_properties(tiffsplit PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(tiffsplit PRIVATE tiff tiff_port)

if(NOT CMAKE_CROSSCOMPILING)
  add_executable(tiff_mkg3states ../placeholder.h)
  target_sources(tiff_mkg3states PRIVATE mkg3states.c ../libtiff/tif_fax3.h)
  set_target_properties(tiff_mkg3states PROPERTIES LINKER_LANGUAGE CXX)
  target_link_libraries(tiff_mkg3states tiff tiff_port)

  add_custom_target(tiff_faxtable
          DEPENDS tiff_mkg3states
          COMMAND ${CMAKE_COMMAND} -E rm "tif_fax3sm.c"
          COMMAND tiff_mkg3states -b -c const "tif_fax3sm.c"
          WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# rgb2ycbcr and thumbnail are intended to *NOT* be installed. They are for
# testing purposes only.
install(TARGETS fax2ps
                fax2tiff
                pal2rgb
                ppm2tiff
                raw2tiff
                tiff2bw
                tiff2pdf
                tiff2ps
                tiff2rgba
                tiffcmp
                tiffcp
                tiffcrop
                tiffdither
                tiffdump
                tiffinfo
                tiffmedian
                tiffset
                tiffsplit
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

if(OPENGL_SUPPORT)
  add_executable(tiffgt ../placeholder.h)
  target_sources(tiffgt PRIVATE tiffgt.c)
  set_target_properties(tiffgt PROPERTIES LINKER_LANGUAGE CXX)
  target_link_libraries(tiffgt PRIVATE tiff tiff_port)
  target_link_libraries(tiffgt PRIVATE OpenGL::GL GLUT::GLUT)

  install(TARGETS tiffgt
          RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

if(WEBP_SUPPORT AND EMSCRIPTEN)
  # Emscripten is pretty finnicky about linker flags.
  # It needs --shared-memory if and only if atomics or bulk-memory is used.
  foreach(target fax2ps
                 fax2tiff
                 pal2rgb
                 ppm2tiff
                 raw2tiff
                 rgb2ycbcr
                 thumbnail
                 tiff2bw
                 tiff2pdf
                 tiff2ps
                 tiff2rgba
                 tiffcmp
                 tiffcp
                 tiffcrop
                 tiffdither
                 tiffdump
                 tiffinfo
                 tiffmedian
                 tiffset
                 tiffsplit)
    target_link_options(${target} PUBLIC "-Wl,--shared-memory")
  endforeach()
endif()
