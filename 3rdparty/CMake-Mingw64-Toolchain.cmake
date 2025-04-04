set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          x86)
set(TOOLCHAIN_PREFIX                "i486-w64-mingw32")

# Without that flag CMake is not able to pass test compilation check
set(CMAKE_TRY_COMPILE_TARGET_TYPE   STATIC_LIBRARY)

set(CMAKE_AR                        "${TOOLCHAIN_PREFIX}-ar${CMAKE_EXECUTABLE_SUFFIX}")
set(CMAKE_ASM_COMPILER              "${TOOLCHAIN_PREFIX}-gcc${CMAKE_EXECUTABLE_SUFFIX}")
set(CMAKE_C_COMPILER                "${TOOLCHAIN_PREFIX}-gcc${CMAKE_EXECUTABLE_SUFFIX}")
set(CMAKE_CXX_COMPILER              "${TOOLCHAIN_PREFIX}-g++${CMAKE_EXECUTABLE_SUFFIX}")
set(CMAKE_LINKER                    "${TOOLCHAIN_PREFIX}-ld${CMAKE_EXECUTABLE_SUFFIX}")
set(CMAKE_OBJCOPY                   "${TOOLCHAIN_PREFIX}-objcopy${CMAKE_EXECUTABLE_SUFFIX}" CACHE INTERNAL "")
set(CMAKE_RANLIB                    "${TOOLCHAIN_PREFIX}-ranlib${CMAKE_EXECUTABLE_SUFFIX}" CACHE INTERNAL "")
set(CMAKE_SIZE                      "${TOOLCHAIN_PREFIX}-size${CMAKE_EXECUTABLE_SUFFIX}" CACHE INTERNAL "")
set(CMAKE_STRIP                     "${TOOLCHAIN_PREFIX}-strip${CMAKE_EXECUTABLE_SUFFIX}" CACHE INTERNAL "")

set(CMAKE_FIND_ROOT_PATH "$ENV{MINGWDIR}")

#set(CMAKE_C_FLAGS                   "-MMD -Wall -std=gnu99 -march=i486 -mtune=i586 -ffast-math -fomit-frame-pointer -fgnu89-inline -Wmissing-prototypes" CACHE INTERNAL "")
#set(CMAKE_CXX_FLAGS                 "${CMAKE_C_FLAGS} -fno-exceptions" CACHE INTERNAL "")

#set(CMAKE_C_FLAGS_DEBUG             "-O0 -DDEBUG_ENABLED -g" CACHE INTERNAL "")
#set(CMAKE_C_FLAGS_RELEASE           "-Os -DNDEBUG" CACHE INTERNAL "")
#set(CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_C_FLAGS_DEBUG}" CACHE INTERNAL "")
#set(CMAKE_CXX_FLAGS_RELEASE         "${CMAKE_C_FLAGS_RELEASE}" CACHE INTERNAL "")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
