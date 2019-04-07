# toolchain file I use to cross compile on Linux
# targetting Windows (x86/mingw). running:
# cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/Toolchain-MinGW32.cmake ....

SET(CMAKE_SYSTEM_NAME Windows)

SET(CMAKE_C_COMPILER /usr/local/cross-win32/bin/i686-pc-mingw32-gcc)
SET(CMAKE_CXX_COMPILER /usr/local/cross-win32/bin/i686-pc-mingw32-g++)
SET(CMAKE_RC_COMPILER /usr/local/cross-win32/bin/i686-pc-mingw32-windres)

# where is the target environment
SET(CMAKE_FIND_ROOT_PATH /usr/local/cross-win32)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
