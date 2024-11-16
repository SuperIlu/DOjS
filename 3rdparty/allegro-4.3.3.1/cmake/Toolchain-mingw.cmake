# Use this command to build the Windows port of Allegro
# with a mingw cross compiler:
#
#   cmake -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain-mingw.cmake .
#
# or for out of source:
#
#   cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-mingw.cmake ..
#
# You will need at least CMake 2.6.0.
#
# Adjust the following paths to suit your environment.
#
# You might want to set MINGDIR to prevent make install making a mess
# in your normal directories.
#
# This file was based on http://www.cmake.org/Wiki/CmakeMingw

set(TOOLCHAIN_PREFIX "i486-w64-mingw32")

# the name of the target operating system
set(CMAKE_SYSTEM_NAME Windows)

# Assume the target architecture.
# XXX for some reason the value set here gets cleared before we reach the
# main CMakeLists.txt; see that file for a workaround.
set(CMAKE_SYSTEM_PROCESSOR i486)
add_compile_options(-march=i486)
add_link_options(-static-libgcc)

set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}-g++")
set(CMAKE_C_COMPILER "${TOOLCHAIN_PREFIX}-gcc")
set(CMAKE_OBJCOPY "${TOOLCHAIN_PREFIX}-objcopy")
set(CMAKE_STRIP "${TOOLCHAIN_PREFIX}-strip")
set(CMAKE_SIZE "${TOOLCHAIN_PREFIX}-size")
set(CMAKE_AR "${TOOLCHAIN_PREFIX}-ar")
set(ASSEMBLER "${TOOLCHAIN_PREFIX}-as")
set(CMAKE_RC_COMPILER "${TOOLCHAIN_PREFIX}-windres")

set(CMAKE_FIND_ROOT_PATH "$ENV{MINGWDIR}")

# Adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Tell pkg-config not to look at the target environment's .pc files.
# Setting PKG_CONFIG_LIBDIR sets the default search directory, but we have to
# set PKG_CONFIG_PATH as well to prevent pkg-config falling back to the host's
# path.
set(ENV{PKG_CONFIG_LIBDIR} ${CMAKE_FIND_ROOT_PATH}/lib/pkgconfig)
set(ENV{PKG_CONFIG_PATH} ${CMAKE_FIND_ROOT_PATH}/lib/pkgconfig)

set(INSTALL_PREFIX ${CMAKE_FIND_ROOT_PATH})
if(ENV{MINGDIR} STREQUAL "")
    set(ENV{MINGDIR} ${CMAKE_FIND_ROOT_PATH})
endif()
