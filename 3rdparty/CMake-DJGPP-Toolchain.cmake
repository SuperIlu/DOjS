set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          x86)

# Without that flag CMake is not able to pass test compilation check
set(CMAKE_TRY_COMPILE_TARGET_TYPE   STATIC_LIBRARY)

set(CMAKE_AR                        i586-pc-msdosdjgpp-ar${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_ASM_COMPILER              i586-pc-msdosdjgpp-gcc${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_C_COMPILER                i586-pc-msdosdjgpp-gcc${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_CXX_COMPILER              i586-pc-msdosdjgpp-g++${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_LINKER                    i586-pc-msdosdjgpp-ld${CMAKE_EXECUTABLE_SUFFIX})
set(CMAKE_OBJCOPY                   i586-pc-msdosdjgpp-objcopy${CMAKE_EXECUTABLE_SUFFIX} CACHE INTERNAL "")
set(CMAKE_RANLIB                    i586-pc-msdosdjgpp-ranlib${CMAKE_EXECUTABLE_SUFFIX} CACHE INTERNAL "")
set(CMAKE_SIZE                      i586-pc-msdosdjgpp-size${CMAKE_EXECUTABLE_SUFFIX} CACHE INTERNAL "")
set(CMAKE_STRIP                     i586-pc-msdosdjgpp-strip${CMAKE_EXECUTABLE_SUFFIX} CACHE INTERNAL "")

#set(CMAKE_C_FLAGS                   "-MMD -Wall -std=gnu99 -march=i386 -mtune=i586 -ffast-math -fomit-frame-pointer -fgnu89-inline -Wmissing-prototypes" CACHE INTERNAL "")
#set(CMAKE_CXX_FLAGS                 "${CMAKE_C_FLAGS} -fno-exceptions" CACHE INTERNAL "")

#set(CMAKE_C_FLAGS_DEBUG             "-O0 -DDEBUG_ENABLED -g" CACHE INTERNAL "")
#set(CMAKE_C_FLAGS_RELEASE           "-Os -DNDEBUG" CACHE INTERNAL "")
#set(CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_C_FLAGS_DEBUG}" CACHE INTERNAL "")
#set(CMAKE_CXX_FLAGS_RELEASE         "${CMAKE_C_FLAGS_RELEASE}" CACHE INTERNAL "")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
