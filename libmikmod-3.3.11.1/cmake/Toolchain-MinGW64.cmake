# toolchain file I use to cross compile on Linux
# targetting Windows (x64/mingw-w64). running:
# cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/Toolchain-MinGW64.cmake ....

SET(CMAKE_SYSTEM_NAME Windows)

SET(CMAKE_C_COMPILER /opt/cross_win64/bin/x86_64-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER /opt/cross_win64/bin/x86_64-w64-mingw32-g++)
SET(CMAKE_RC_COMPILER /opt/cross_win64/bin/x86_64-w64-mingw32-windres)

# where is the target environment
SET(CMAKE_FIND_ROOT_PATH /opt/cross_win64 /opt/cross_win64/x86_64-w64-mingw32)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
