# Copyright (c) 2012 Shlomi Fish
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# (This copyright notice applies only to this file)

PROJECT(libmikmod C)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
LIST(APPEND CMAKE_MODULE_PATH "${libmikmod_SOURCE_DIR}/cmake")

SET(LIBMIKMOD_MAJOR_VERSION 3)
SET(LIBMIKMOD_MINOR_VERSION 3)
SET(LIBMIKMOD_MICRO_VERSION 11)

SET(VERSION "${LIBMIKMOD_MAJOR_VERSION}.${LIBMIKMOD_MINOR_VERSION}.${LIBMIKMOD_MICRO_VERSION}")
SET(LIBMIKMOD_VERSION "${LIBMIKMOD_MAJOR_VERSION}.${LIBMIKMOD_MINOR_VERSION}.${LIBMIKMOD_MICRO_VERSION}")
SET(CPACK_PACKAGE_VERSION_MAJOR ${LIBMIKMOD_MAJOR_VERSION})
SET(CPACK_PACKAGE_VERSION_MINOR ${LIBMIKMOD_MINOR_VERSION})
SET(CPACK_PACKAGE_VERSION_PATCH ${LIBMIKMOD_MICRO_VERSION})

# package generation (make package[_source])
SET(CPACK_PACKAGE_NAME "libmikmod")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "libmikmod - a module file playing and sound library")
SET(CPACK_PACKAGE_VENDOR "Shlomi Fish")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING.LESSER")

SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_DESCRIPTION_SUMMARY} ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

SET(base_with_ver "libmikmod-[0-9]+\\\\.[0-9]+\\\\.[0-9]+")
SET(CPACK_SOURCE_IGNORE_FILES
    "/_CPack_Packages/"
    "/CMakeFiles/"
    "/.deps/"
    "^${base_with_ver}(-Source|-Linux)?/"
    "${base_with_ver}.tar\\\\.(gz|bz2|Z|lzma|xz)$"
    "\\\\.o$"
    "~$"
    "/libmikmod\\\\.(a|la)$"
    "\\\\.so(\\\\.[0-9]+)*$"
    "/\\\\.svn/"
    "\\\\.(pdf|fo)$"
    "/CMakeCache\\\\.txt$"
    "/CTestTestfile\\\\.cmake$"
    "/cmake_install\\\\.cmake$"
    "/CPackConfig\\\\.cmake$"
    "/CPackSourceConfig\\\\.cmake$"
    "/tags$"
    "/libmikmod-config$"
    "^config\\\\.h$"
    "/install_manifest\\\\.txt$"
    "/t/"
    "/Testing/"
    "run-tests\\\\.pl"
    "/_Inline/"
    "/(B|build|BUILD)/"
    "/autom4te.cache/"
)

IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
SET (ENABLE_ALSA 1 CACHE BOOL "Include the ALSA driver")
ELSE()
SET (ENABLE_ALSA 0)
ENDIF()

IF(UNIX AND NOT APPLE)
SET (ENABLE_OSS 1 CACHE BOOL "Include the OSS driver")
SET (ENABLE_ESD 0 CACHE BOOL "Include the EsounD (Enlightened Sound Daemon) driver")
SET (ENABLE_PULSE 1 CACHE BOOL "Include the PulseAudio driver")
SET (ENABLE_NAS 0 CACHE BOOL "Include the Network Audio System driver")
SET (ENABLE_AF 0 CACHE BOOL "Include the DEC AudioFile server (AF) driver")
ELSE()
SET (ENABLE_AF 0)
SET (ENABLE_OSS 0)
SET (ENABLE_PULSE 0)
SET (ENABLE_ESD 0)
SET (ENABLE_NAS 0)
ENDIF()

IF(CMAKE_SYSTEM_NAME MATCHES "OpenBSD.*")
SET (ENABLE_SNDIO 1 CACHE BOOL "Include the OpenBSD sndio driver")
ELSE()
SET (ENABLE_SNDIO 0)
ENDIF()

IF(CMAKE_SYSTEM_NAME MATCHES "kNetBSD.*|NetBSD.*|kOpenBSD.*|OpenBSD.*|Solaris.*")
SET (ENABLE_SUNAUDIO 1 CACHE BOOL "Include the SunAudio (or compatibles) driver")
ELSE()
SET (ENABLE_SUNAUDIO 0)
ENDIF()

IF(CMAKE_SYSTEM_NAME MATCHES "AIX.*")
SET (ENABLE_AIX 1 CACHE BOOL "Include the AIX audio driver")
ELSE()
SET (ENABLE_AIX 0)
ENDIF()

IF(CMAKE_SYSTEM_NAME MATCHES "HP-UX.*")
SET (ENABLE_HPUX 1 CACHE BOOL "Include the HP-UX audio driver")
ELSE()
SET (ENABLE_HPUX 0)
ENDIF()

IF(WIN32)
SET (ENABLE_DSOUND 1 CACHE BOOL "Include the DirectSound driver")
SET (ENABLE_MMSOUND 1 CACHE BOOL "Include the Windows MCI driver")
SET (ENABLE_XAUDIO2 0 CACHE BOOL "Include the XAudio2 driver")
SET (ENABLE_XAUDIO28 0 CACHE BOOL "Use XAudio2.8 for Windows8 instead of XAudio2.7")
ELSE()
SET (ENABLE_DSOUND 0)
SET (ENABLE_MMSOUND 0)
SET (ENABLE_XAUDIO2 0)
SET (ENABLE_XAUDIO28 0)
ENDIF()

IF(APPLE)
SET (ENABLE_COREAUDIO 1 CACHE BOOL "Include the CoreAudio driver")
ELSE()
SET (ENABLE_COREAUDIO 0)
ENDIF()

SET (ENABLE_OPENAL 0 CACHE BOOL "Include the OpenAL driver")
SET (ENABLE_SDL 0 CACHE BOOL "Include the SDL driver")

IF(UNIX)
SET (ENABLE_STDOUT 1 CACHE BOOL "Support for output to stdout")
SET (ENABLE_PIPE 1 CACHE BOOL "Support for output via a pipe to another command")
ELSE()
SET (ENABLE_STDOUT 0)
SET (ENABLE_PIPE 0)
ENDIF()

SET (ENABLE_AIFF 1 CACHE BOOL "Support for output to an .aiff file")
SET (ENABLE_WAV 1 CACHE BOOL "Support for output to a .wav file")
SET (ENABLE_RAW 1 CACHE BOOL "Support for output raw pcm data to a file")

IF(NOT WIN32)
SET (ENABLE_DL 0 CACHE BOOL "Load ALSA and EsounD drivers at runtime")
SET (ENABLE_THREADS 1 CACHE BOOL "build a thread-safe version of libmikmod") # only for pthreads.
ELSE()
SET (ENABLE_DL 0)
SET (ENABLE_THREADS 0) # windows builds have it using win32api already.
ENDIF()

SET (DISABLE_HQMIXER 0 CACHE BOOL "Exclude support for high quality mixer")

SET (ENABLE_SIMD 0 CACHE BOOL "Use SIMD (AltiVec or SSE2) optimizations (UNSTABLE!!!)")
SET (ENABLE_RPATH 1 CACHE BOOL "Whether to use an rpath when linking" )
SET (ENABLE_STATIC 1 CACHE BOOL "Whether to build the static library" )

### This is to set the RPATH correctly, so when installed
### under a prefix the executables will find the libraries.
### See:  http://www.cmake.org/Wiki/CMake_RPATH_handling
IF (ENABLE_RPATH)
  # use, i.e. don't skip the full RPATH for the build tree
    SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

  # when building, don't use the install RPATH already
  # (but later on when installing)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

  # the RPATH to be used when installing
    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}")

  # add the automatically determined parts of the RPATH
  # which point to directories outside the build tree to the install RPATH
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
ELSE(ENABLE_RPATH)
#   SET (CMAKE_SKIP_RPATH YES)
ENDIF(ENABLE_RPATH)

INCLUDE(CPack)
include(CheckFunctionExists)
include(CheckSymbolExists)
INCLUDE(CheckCCompilerFlag)
INCLUDE(CheckCSourceCompiles)
include(FindPkgConfig)
include(TestBigEndian)
include(mik_macros)

ADD_SUBDIRECTORY ("dlapi")
ADD_SUBDIRECTORY ("docs")
ADD_SUBDIRECTORY ("drivers")
ADD_SUBDIRECTORY ("include")
ADD_SUBDIRECTORY ("loaders")
ADD_SUBDIRECTORY ("mmio")
ADD_SUBDIRECTORY ("depackers")
ADD_SUBDIRECTORY ("playercode")
ADD_SUBDIRECTORY ("posix")

IF (CMAKE_COMPILER_IS_GNUCC OR (CMAKE_C_COMPILER_ID MATCHES "Clang"))
    ADD_DEFINITIONS("-Wall" "-Wwrite-strings" "-Wshadow" "-Wundef" "-Wcast-qual")
ENDIF()

SET(COMPILER_FLAGS_TO_CHECK "-Werror=implicit-function-declaration")
#LIST(APPEND COMPILER_FLAGS_TO_CHECK "-ffast-math")

IF (APPLE)
    LIST(APPEND COMPILER_FLAGS_TO_CHECK "-fno-common")
ENDIF()

IF (CPU_ARCH)
    LIST(APPEND COMPILER_FLAGS_TO_CHECK "-march=${CPU_ARCH}")
ENDIF()

IF (ENABLE_SIMD)
    IF(CMAKE_SYSTEM_PROCESSOR MATCHES "^i.86$")
        LIST(APPEND COMPILER_FLAGS_TO_CHECK "-msse2")

    # -faltivec is a useful Apple extension. it is not the same as -maltivec.
    ELSEIF(CMAKE_SYSTEM_PROCESSOR MATCHES "^(powerpc|ppc)" AND (CMAKE_SYSTEM_NAME MATCHES "Darwin"))
        LIST(APPEND COMPILER_FLAGS_TO_CHECK "-faltivec")

    ELSEIF(CMAKE_SYSTEM_PROCESSOR MATCHES "^(powerpc|ppc)64" AND (CMAKE_SYSTEM_NAME MATCHES "Darwin"))
        LIST(APPEND COMPILER_FLAGS_TO_CHECK "-faltivec")

    ELSEIF(CMAKE_SYSTEM_PROCESSOR MATCHES "^(powerpc|ppc)64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "^(powerpc|ppc)")
        LIST(APPEND COMPILER_FLAGS_TO_CHECK "-maltivec")

    ENDIF()
ENDIF(ENABLE_SIMD)

SET (IDX 1)
FOREACH (CFLAG_TO_CHECK ${COMPILER_FLAGS_TO_CHECK})
    SET (FLAG_EXISTS_VAR "FLAG_EXISTS_${IDX}")
    MATH (EXPR IDX "${IDX} + 1")
    CHECK_C_COMPILER_FLAG("${CFLAG_TO_CHECK}" ${FLAG_EXISTS_VAR})
    IF (${FLAG_EXISTS_VAR})
        ADD_DEFINITIONS(${CFLAG_TO_CHECK})
    ENDIF()
ENDFOREACH()

# check inline keyword
CHECK_C_SOURCE_COMPILES("static inline int static_foo() {return 0;}
                         int main(void) {return 0;}" HAVE_C_INLINE)
CHECK_C_SOURCE_COMPILES("static __inline__ int static_foo() {return 0;}
                         int main(void) {return 0;}" HAVE_C___INLINE__)
CHECK_C_SOURCE_COMPILES("static __inline int static_foo() {return 0;}
                         int main(void) {return 0;}" HAVE_C___INLINE)

# check for symbol visibility attributes
IF(NOT WIN32 AND NOT CYGWIN)
    SET(OLD_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS}")
    SET(CMAKE_REQUIRED_FLAGS "${OLD_REQUIRED_FLAGS} -Werror")
    CHECK_C_SOURCE_COMPILES("int foo(void) __attribute__((visibility(\"default\")));
                             int main(void) {return 0;}" HAVE_VISIBILITY_DEFAULT)
    IF(HAVE_VISIBILITY_DEFAULT)
        CHECK_C_COMPILER_FLAG(-fvisibility=hidden HAVE_VISIBILITY_HIDDEN)
        IF(HAVE_VISIBILITY_HIDDEN)
            ADD_DEFINITIONS(-fvisibility=hidden)
            ADD_DEFINITIONS("-DSYM_VISIBILITY")
        ENDIF()
    ENDIF()
    SET(CMAKE_REQUIRED_FLAGS "${OLD_REQUIRED_FLAGS}")
ENDIF()

TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

# to find mikmod_internals.h:
INCLUDE_DIRECTORIES(BEFORE ${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(BEFORE "${CMAKE_SOURCE_DIR}/include")

# to find config.h:
INCLUDE_DIRECTORIES(BEFORE ${CMAKE_BINARY_DIR})

CHECK_MULTI_INCLUDE_FILES(
    "dlfcn.h" "fcntl.h" "limits.h" "malloc.h" "memory.h" "sys/ioctl.h" "unistd.h" "windows.h"
)

IF(WIN32 AND NOT HAVE_WINDOWS_H)
    MESSAGE(FATAL_ERROR "windows.h not found.")
ENDIF()

IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    CHECK_MULTI_INCLUDE_FILES("alsa/asoundlib.h" "sys/sam9407.h")
ENDIF()

CHECK_MULTI_FUNCTIONS_EXISTS(
    "posix_memalign" "setenv" "snprintf" "srandom"
)

CHECK_SYMBOL_EXISTS(RTLD_GLOBAL dlfcn.h HAVE_RTLD_GLOBAL)
CHECK_SYMBOL_EXISTS(srandom math.h SRANDOM_IN_MATH_H)

ADD_DEFINITIONS("-DHAVE_CONFIG_H")
# always define MIKMOD_BUILD when building libmikmod
ADD_DEFINITIONS("-DMIKMOD_BUILD")

SET (MIKMOD_LIB_MODULES
    drivers/drv_AF.c
    drivers/drv_aiff.c
    drivers/drv_ahi.c
    drivers/drv_aix.c
    drivers/drv_alsa.c
    drivers/drv_dart.c
    drivers/drv_ds.c
    drivers/drv_esd.c
    drivers/drv_gp32.c
    drivers/drv_hp.c
    drivers/drv_mac.c
    drivers/drv_nas.c
    drivers/drv_nos.c
    drivers/drv_openal.c
    drivers/drv_osles.c
    drivers/drv_oss.c
    drivers/drv_osx.c
    drivers/drv_pipe.c
    drivers/drv_psp.c
    drivers/drv_pulseaudio.c
    drivers/drv_raw.c
    drivers/drv_sam9407.c
    drivers/drv_sdl.c
    drivers/drv_sgi.c
    drivers/drv_sndio.c
    drivers/drv_stdout.c
    drivers/drv_sun.c
    drivers/drv_ultra.c
    drivers/drv_wav.c
    drivers/drv_win.c
    drivers/drv_xaudio2.c
    loaders/load_669.c
    loaders/load_amf.c
    loaders/load_asy.c
    loaders/load_dsm.c
    loaders/load_far.c
    loaders/load_gdm.c
    loaders/load_gt2.c
    loaders/load_imf.c
    loaders/load_it.c
    loaders/load_m15.c
    loaders/load_med.c
    loaders/load_mod.c
    loaders/load_mtm.c
    loaders/load_okt.c
    loaders/load_s3m.c
    loaders/load_stm.c
    loaders/load_stx.c
    loaders/load_ult.c
    loaders/load_umx.c
    loaders/load_uni.c
    loaders/load_xm.c
    mmio/mmalloc.c
    mmio/mmerror.c
    mmio/mmio.c
    depackers/mmcmp.c
    depackers/pp20.c
    depackers/s404.c
    depackers/xpk.c
    posix/strcasecmp.c
    playercode/mdreg.c
    playercode/mdriver.c
    playercode/mdulaw.c
    playercode/mloader.c
    playercode/mlreg.c
    playercode/mlutil.c
    playercode/mplayer.c
    playercode/munitrk.c
    playercode/mwav.c
    playercode/npertab.c
    playercode/sloader.c
    playercode/virtch.c
    playercode/virtch2.c
    playercode/virtch_common.c
)

REPLACE_FUNCTIONS_FROMDIR(MIKMOD_LIB_MODULES
    posix
    memcmp strstr)

SET (LIBMIKMOD_LIBS)

ADD_LIBRARY (mikmod
    SHARED
    ${MIKMOD_LIB_MODULES}
    )

LIST (APPEND LIBMIKMOD_LIBS "mikmod")
IF (ENABLE_STATIC)
    ADD_LIBRARY (mikmod-static
        STATIC
        ${MIKMOD_LIB_MODULES}
    )
    LIST (APPEND LIBMIKMOD_LIBS "mikmod-static")

    SET_TARGET_PROPERTIES(
        mikmod-static
        PROPERTIES OUTPUT_NAME "mikmod"
    )
    SET_TARGET_PROPERTIES(
        ${LIBMIKMOD_LIBS}
        PROPERTIES CLEAN_DIRECT_OUTPUT 1
    )
ENDIF (ENABLE_STATIC)

SET_TARGET_PROPERTIES(mikmod
    PROPERTIES VERSION 3.3.0 SOVERSION 3
)
IF(WIN32)
    SET_TARGET_PROPERTIES(mikmod
        PROPERTIES DEFINE_SYMBOL DLL_EXPORT
    )
ENDIF(WIN32)

IF(DISABLE_HQMIXER)
  SET(NO_HQMIXER 1)
ELSE()
  SET(NO_HQMIXER)
ENDIF()

IF(ENABLE_PIPE)
  SET(DRV_PIPE 1)
ELSE()
  SET(DRV_PIPE)
ENDIF()

IF(ENABLE_STDOUT)
  SET(DRV_STDOUT 1)
ELSE()
  SET(DRV_STDOUT)
ENDIF()

IF(ENABLE_AIFF)
SET(DRV_AIFF 1)
ELSE()
SET(DRV_AIFF)
ENDIF()

IF(ENABLE_WAV)
SET(DRV_WAV 1)
ELSE()
SET(DRV_WAV)
ENDIF()

IF(ENABLE_RAW)
SET(DRV_RAW 1)
ELSE()
SET(DRV_RAW)
ENDIF()

IF(ENABLE_SIMD)
  SET(MIKMOD_SIMD 1)
ELSE()
  SET(MIKMOD_SIMD)
ENDIF()

IF (ENABLE_DL)
  SET(MIKMOD_DYNAMIC 1)
ELSE()
  SET(MIKMOD_DYNAMIC)
ENDIF()

SET(EXTRA_LIBS )

SET(HAVE_PTHREAD)
IF (NOT WIN32)
 INCLUDE(FindThreads)
 IF (CMAKE_USE_PTHREADS_INIT)
  IF (ENABLE_THREADS)
    SET (HAVE_PTHREAD 1)
    IF (CMAKE_THREAD_LIBS_INIT)
        LIST (APPEND EXTRA_LIBS ${CMAKE_THREAD_LIBS_INIT})
    ENDIF()
  ENDIF()
 ENDIF()
ENDIF()

IF (ENABLE_PULSE)
    PKG_CHECK_MODULES(PKG_PULSESIMPLE "libpulse-simple")
    IF(PKG_PULSESIMPLE_FOUND)
        SET(DRV_PULSEAUDIO 1)
        SET(PULSESIMPLE_LIBRARIES ${PKG_PULSESIMPLE_LDFLAGS})
        ADD_DEFINITIONS (${PKG_PULSESIMPLE_CFLAGS})
    ELSE()
        find_path(PULSESIMPLE_INCLUDE_DIR "pulse/simple.h")
        find_library(PULSESIMPLE_LIBRARIES "pulse-simple")
        IF (PULSESIMPLE_LIBRARIES STREQUAL "PULSESIMPLE_LIBRARIES-NOTFOUND")
            MESSAGE(STATUS "Could NOT find PulseAudio.")
            SET(ENABLE_PULSE 0)
            SET(DRV_PULSEAUDIO)
            MESSAGE(WARNING "PulseAudio support is disabled.")
        ELSE()
            MESSAGE(STATUS "Found PulseAudio: ${PULSESIMPLE_LIBRARIES}")
            SET(DRV_PULSEAUDIO 1)
            INCLUDE_DIRECTORIES(${PULSESIMPLE_INCLUDE_DIR})
        ENDIF()
    ENDIF()
    IF(ENABLE_PULSE)
      LIST (APPEND EXTRA_LIBS ${PULSESIMPLE_LIBRARIES})
    ENDIF()
ELSE()
    SET(DRV_PULSEAUDIO)
ENDIF(ENABLE_PULSE)

IF (ENABLE_ESD)
    PKG_CHECK_MODULES(PKG_ESD esound)
    IF(PKG_ESD_FOUND)
        SET(DRV_ESD 1)
        SET(ESD_LIBRARIES ${PKG_ESD_LDFLAGS})
        ADD_DEFINITIONS (${PKG_ESD_CFLAGS})
    ELSE(PKG_ESD_FOUND)
        find_path(ESD_INCLUDE_DIR esd.h)
        find_library(ESD_LIBRARIES esd)
        IF (ESD_LIBRARIES STREQUAL "ESD_LIBRARIES-NOTFOUND")
            MESSAGE(STATUS "Could NOT find EsounD.")
            SET(ENABLE_ESD 0)
            SET(DRV_ESD)
            MESSAGE(WARNING "EsounD support is disabled.")
        ELSE()
            MESSAGE(STATUS "Found EsounD: ${ESD_LIBRARIES}")
            SET(DRV_ESD 1)
            INCLUDE_DIRECTORIES(${ESD_INCLUDE_DIR})
        ENDIF()
    ENDIF(PKG_ESD_FOUND)
    IF(ENABLE_ESD AND NOT ENABLE_DL)
      LIST (APPEND EXTRA_LIBS ${ESD_LIBRARIES})
    ENDIF()
ELSE()
    SET(DRV_ESD)
ENDIF(ENABLE_ESD)

IF (ENABLE_NAS)
  CHECK_INCLUDE_FILE("audio/audiolib.h" HAVE_AUDIO_AUDIOLIB_H)
  find_library(NAS_LIBRARIES audio)
  IF(NOT (NAS_LIBRARIES STREQUAL "NAS_LIBRARIES-NOTFOUND"))
    SET(OLD_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES}")
    SET(CMAKE_REQUIRED_LIBRARIES "audio")
    CHECK_FUNCTION_EXISTS(AuCreateFlow NAS_LIBS_OK)
    SET(CMAKE_REQUIRED_LIBRARIES "${OLD_REQUIRED_LIBRARIES}")
  ENDIF()
  IF((NOT NAS_LIBS_OK) OR (NOT HAVE_AUDIO_AUDIOLIB_H))
    MESSAGE(STATUS "Could not find NAS.")
    SET(ENABLE_NAS 0)
    SET(DRV_NAS)
    MESSAGE(WARNING "NAS (Network Audio System) support is disabled.")
  ELSE()
    MESSAGE(STATUS "Found NAS: ${NAS_LIBRARIES}")
    SET(DRV_NAS 1)
    LIST (APPEND EXTRA_LIBS ${NAS_LIBRARIES})
  ENDIF()
ELSE()
  SET(DRV_NAS)
ENDIF(ENABLE_NAS)

IF (ENABLE_AF)
  CHECK_INCLUDE_FILE("AF/AFlib.h" HAVE_AF_AFLIB_H)
  find_library(AF_LIBRARIES AF)
  IF(NOT (AF_LIBRARIES STREQUAL "AF_LIBRARIES-NOTFOUND"))
    SET(OLD_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES}")
    SET(CMAKE_REQUIRED_LIBRARIES "AF")
    CHECK_FUNCTION_EXISTS(AFOpenAudioConn AF_LIBS_OK)
    SET(CMAKE_REQUIRED_LIBRARIES "${OLD_REQUIRED_LIBRARIES}")
  ENDIF()
  IF((NOT AF_LIBS_OK) OR (NOT HAVE_AF_AFLIB_H))
    MESSAGE(STATUS "Could not find DEC AudioFile server.")
    SET(ENABLE_AF 0)
    SET(DRV_AF)
    MESSAGE(WARNING "DEC AudioFile server support is disabled.")
  ELSE()
    MESSAGE(STATUS "Found DEC AudioFile server: ${AF_LIBRARIES}")
    SET(DRV_AF 1)
    LIST (APPEND EXTRA_LIBS ${AF_LIBRARIES})
  ENDIF()
ELSE()
  SET(DRV_AF)
ENDIF(ENABLE_AF)

IF (ENABLE_OSS)
  CHECK_MULTI_INCLUDE_FILES("sys/soundcard.h" "machine/soundcard.h" "soundcard.h")

  # NetBSD and OpenBSD uses ossaudio emulation layer,
  # otherwise no link library is needed.
  IF(CMAKE_SYSTEM_NAME MATCHES "kNetBSD.*|NetBSD.*|kOpenBSD.*|OpenBSD.*") # AND HAVE_SOUNDCARD_H ??
    FIND_LIBRARY(OSSAUDIO_LIBRARIES "ossaudio")
    IF(OSSAUDIO_LIBRARIES STREQUAL "OSSAUDIO_LIBRARIES-NOTFOUND")
      SET(OSSAUDIO_LIBRARIES)
    ELSE()
      MESSAGE(STATUS "Found libossaudio: ${OSSAUDIO_LIBRARIES}")
    ENDIF()
  ELSE()
    SET(OSSAUDIO_LIBRARIES)
  ENDIF()
  SET(OLD_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES}")
  IF(OSSAUDIO_LIBRARIES)
       SET(CMAKE_REQUIRED_LIBRARIES ${OSSAUDIO_LIBRARIES})
  ENDIF()

  IF(HAVE_SYS_SOUNDCARD_H)
    CHECK_C_SOURCE_COMPILES("#include <sys/ioctl.h>
                             #include <sys/soundcard.h>
                             int main() {return SNDCTL_DSP_RESET;}" HAVE_OSS)
  ELSEIF(HAVE_MACHINE_SOUNDCARD_H)
    CHECK_C_SOURCE_COMPILES("#include <sys/ioctl.h>
                             #include <machine/soundcard.h>
                             int main() {return SNDCTL_DSP_RESET;}" HAVE_OSS)
  ELSEIF(HAVE_SOUNDCARD_H)
    CHECK_C_SOURCE_COMPILES("#include <sys/ioctl.h>
                             #include <soundcard.h>
                             int main() {return SNDCTL_DSP_RESET;}" HAVE_OSS)
  ELSE()
    SET (HAVE_OSS)
  ENDIF()
  SET(CMAKE_REQUIRED_LIBRARIES "${OLD_REQUIRED_LIBRARIES}")

  IF(HAVE_OSS)
    SET (DRV_OSS 1)
    IF(OSSAUDIO_LIBRARIES)
      LIST(APPEND EXTRA_LIBS ${OSSAUDIO_LIBRARIES})
    ENDIF()
  ELSE()
    SET (ENABLE_OSS 0)
    SET (DRV_OSS)
    MESSAGE(WARNING "OSS support is disabled.")
  ENDIF()
ELSE()
  SET (DRV_OSS)
ENDIF (ENABLE_OSS)

IF (ENABLE_ALSA)
  include(FindALSA)
  IF (ALSA_FOUND)
    SET (DRV_ALSA 1)
    IF(NOT ENABLE_DL)
      LIST (APPEND EXTRA_LIBS ${ALSA_LIBRARIES})
    ENDIF()
  ELSE()
    SET (ENABLE_ALSA 0)
    SET (DRV_ALSA)
    MESSAGE(WARNING "ALSA support is disabled.")
  ENDIF()
ELSE()
  SET (DRV_ALSA)
ENDIF(ENABLE_ALSA)

IF (ENABLE_SNDIO)
  CHECK_INCLUDE_FILE("sndio.h" HAVE_SNDIO_H)
  find_library(SNDIO_LIBRARIES sndio)
  IF(NOT (SNDIO_LIBRARIES STREQUAL "SNDIO_LIBRARIES-NOTFOUND"))
    SET(OLD_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES}")
    SET(CMAKE_REQUIRED_LIBRARIES "sndio")
    CHECK_FUNCTION_EXISTS(sio_open SNDIO_LIBS_OK)
    SET(CMAKE_REQUIRED_LIBRARIES "${OLD_REQUIRED_LIBRARIES}")
  ENDIF()
  IF((NOT SNDIO_LIBS_OK) OR (NOT HAVE_SNDIO_H))
    SET (ENABLE_SNDIO 0)
    SET (DRV_SNDIO)
    MESSAGE(WARNING "OpenBSD sndio support is disabled.")
  ELSE()
    SET (DRV_SNDIO 1)
  ENDIF()
ELSE()
  SET (DRV_SNDIO)
ENDIF (ENABLE_SNDIO)

IF (ENABLE_SUNAUDIO)
  CHECK_MULTI_INCLUDE_FILES("sys/audioio.h" "sun/audioio.h")
  IF ("${HAVE_SYS_AUDIOIO_H}" OR "${HAVE_SUN_AUDIOIO_H}")
    SET (DRV_SUN 1)
  ELSE()
    SET (ENABLE_SUNAUDIO 0)
    SET (DRV_SUN)
    MESSAGE(WARNING "SunAudio driver support is disabled.")
  ENDIF()
ELSE()
  SET (DRV_SUN)
ENDIF (ENABLE_SUNAUDIO)

IF (ENABLE_AIX)
  CHECK_MULTI_INCLUDE_FILES("sys/audio.h" "sys/acpa.h")
  IF ("${HAVE_SYS_AUDIO_H}" AND "${HAVE_SYS_ACPA_H}")
    SET (DRV_AIX 1)
  ELSE()
    SET (ENABLE_AIX 0)
    SET (DRV_AIX)
    MESSAGE(WARNING "AIX audio driver support is disabled.")
  ENDIF()
ELSE()
  SET (DRV_AIX)
ENDIF (ENABLE_AIX)

IF (ENABLE_HPUX)
  CHECK_INCLUDE_FILE("sys/audio.h" HAVE_SYS_AUDIO_H)
  IF ("${HAVE_SYS_AUDIO_H}")
    SET (DRV_HPUX 1)
  ELSE()
    SET (ENABLE_HPUX 0)
    SET (DRV_HPUX)
    MESSAGE(WARNING "HP-UX audio driver support is disabled.")
  ENDIF()
ELSE()
  SET (DRV_HPUX)
ENDIF (ENABLE_HPUX)

IF (ENABLE_COREAUDIO)
    find_library(COREAUDIO_LIBS CoreAudio)
    IF (COREAUDIO_LIBS STREQUAL "COREAUDIO_LIBS-NOTFOUND")
        MESSAGE(STATUS "Could NOT find CoreAudio.")
        SET (ENABLE_COREAUDIO 0)
        SET (DRV_OSX)
        MESSAGE(WARNING "CoreAudio driver support is disabled.")
    ELSE()
        MESSAGE(STATUS "Found CoreAudio libs: ${COREAUDIO_LIBS}")
        SET (DRV_OSX 1)
        LIST (APPEND EXTRA_LIBS ${COREAUDIO_LIBS})
    ENDIF()
ELSE()
    SET (DRV_OSX)
ENDIF(ENABLE_COREAUDIO)

IF (ENABLE_MMSOUND)
    SET (DRV_WIN 1)
    LIST (APPEND EXTRA_LIBS "winmm")
ELSE()
    SET (DRV_WIN)
ENDIF(ENABLE_MMSOUND)

IF (ENABLE_DSOUND)
    CHECK_INCLUDE_FILE("dsound.h" HAVE_DSOUND_H)
    IF (HAVE_DSOUND_H)
      SET (DRV_DS 1)
      LIST (APPEND EXTRA_LIBS "dsound")
    ELSE()
      SET (ENABLE_DSOUND 0)
      SET (DRV_DS)
      MESSAGE(WARNING "DirectSound support is disabled.")
    ENDIF()
ELSE()
    SET (DRV_DS)
ENDIF(ENABLE_DSOUND)

IF (ENABLE_XAUDIO2)
    SET(OLD_REQUIRED_DEFINITIONS "${CMAKE_REQUIRED_DEFINITIONS}")
    IF (ENABLE_XAUDIO28)
      SET(CMAKE_REQUIRED_DEFINITIONS "-D_WIN32_WINNT=0x0602")
    ENDIF()
    CHECK_INCLUDE_FILE("xaudio2.h" HAVE_XAUDIO2_H)
    SET(CMAKE_REQUIRED_DEFINITIONS "${OLD_REQUIRED_DEFINITIONS}")
    IF(HAVE_XAUDIO2_H)
      CHECK_SYMBOL_EXISTS(XAUDIO2_VOICE_NOSAMPLESPLAYED xaudio2.h XAUDIO2_7_HEADER)
      IF (XAUDIO2_7_HEADER AND ENABLE_XAUDIO28)
        MESSAGE(WARNING "xaudio2.h <= 2.7 and xaudio2.8 enabled - build will likely fail.")
      ELSEIF((NOT XAUDIO2_7_HEADER) AND (NOT ENABLE_XAUDIO28))
        MESSAGE(WARNING "xaudio2.h >= 2.8 and xaudio2.8 disabled - build will likely fail.")
      ENDIF()
      SET (DRV_XAUDIO2 1)
      LIST (APPEND EXTRA_LIBS "ole32")
      IF (ENABLE_XAUDIO28)
          ADD_DEFINITIONS("-D_WIN32_WINNT=0x0602")
          SET (DRV_XAUDIO28 1)
          LIST (APPEND EXTRA_LIBS "xaudio2_8")
      ELSE()
          SET (DRV_XAUDIO28)
      ENDIF()
    ELSE()
      SET (ENABLE_XAUDIO2 0)
      SET (ENABLE_XAUDIO28 0)
      SET (DRV_XAUDIO2)
      SET (DRV_XAUDIO28)
      MESSAGE(WARNING "XAudio2 support is disabled.")
    ENDIF()
ELSE()
    SET (ENABLE_XAUDIO28 0)
    SET (DRV_XAUDIO2)
    SET (DRV_XAUDIO28)
ENDIF(ENABLE_XAUDIO2)

IF (ENABLE_SDL)
  SET (SDL_BUILDING_LIBRARY TRUE)
  include(FindSDL)
  IF (SDL_FOUND)
    SET (DRV_SDL 1)
    LIST (APPEND EXTRA_LIBS ${SDL_LIBRARY})
  ELSE()
    SET (ENABLE_SDL 0)
    SET (DRV_SDL)
    MESSAGE(WARNING "SDL support is disabled.")
  ENDIF()
ELSE()
  SET (DRV_SDL)
ENDIF(ENABLE_SDL)

IF (ENABLE_OPENAL)
  include(FindOpenAL)
  IF (OPENAL_FOUND)
    SET (DRV_OPENAL 1)
    CHECK_MULTI_INCLUDE_FILES("AL/al.h" "AL/alc.h")
    IF (HAVE_AL_AL_H)
        # alext.h depends on al.h and alc.h, therefore
        # a plain CHECK_INCLUDE_FILE() doesn't work.
        CHECK_C_SOURCE_COMPILES("#include <AL/al.h>
                                 #include <AL/alc.h>
                                 #include <AL/alext.h>
                                 int main() {return 0;}" HAVE_AL_ALEXT_H)
    ENDIF()
    LIST (APPEND EXTRA_LIBS ${OPENAL_LIBRARY})
  ELSE()
    SET (ENABLE_OPENAL 0)
    SET (DRV_OPENAL)
    MESSAGE(WARNING "OpenAL support is disabled.")
  ENDIF()
ELSE()
  SET (DRV_OPENAL)
ENDIF(ENABLE_OPENAL)

IF (ENABLE_DL)
# add -ldl but only if a driver using dlopen() is active
  IF(DRV_ALSA OR DRV_ESD)
    LIST (APPEND EXTRA_LIBS ${CMAKE_DL_LIBS})
  ENDIF()
ENDIF(ENABLE_DL)

IF (UNIX)
    SET(MATH_LIB "m")
ELSE(UNIX)
    SET(MATH_LIB )
ENDIF(UNIX)
LIST (APPEND EXTRA_LIBS ${MATH_LIB})

CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/config.h.cmake" "${CMAKE_BINARY_DIR}/config.h")

SET(prefix ${CMAKE_INSTALL_PREFIX})
SET(exec_prefix "\${prefix}")
SET(libdir "\${exec_prefix}/lib${LIB_SUFFIX}")
SET(includedir "\${prefix}/include")
CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/libmikmod-config.in" "${CMAKE_BINARY_DIR}/libmikmod-config" @ONLY)
CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/libmikmod.pc.in" "${CMAKE_BINARY_DIR}/libmikmod.pc" @ONLY)

FOREACH (TGT ${LIBMIKMOD_LIBS})
    TARGET_LINK_LIBRARIES ("${TGT}"
        ${EXTRA_LIBS})
    INCLUDE_DIRECTORIES(AFTER ${ALSA_INCLUDE_DIRS} ${OPENAL_INCLUDE_DIR} ${SDL_INCLUDE_DIR})
ENDFOREACH (TGT)

INSTALL(
    TARGETS ${LIBMIKMOD_LIBS}
    LIBRARY DESTINATION "lib${LIB_SUFFIX}"
    ARCHIVE DESTINATION "lib${LIB_SUFFIX}"
)

INSTALL(
    FILES "${CMAKE_BINARY_DIR}/libmikmod-config"
    PERMISSIONS GROUP_EXECUTE GROUP_READ OWNER_EXECUTE OWNER_READ OWNER_WRITE WORLD_EXECUTE WORLD_READ
    DESTINATION "bin"
)

INSTALL(
    FILES
        "${CMAKE_SOURCE_DIR}/include/mikmod.h"
    DESTINATION
        "include"
)

INSTALL(
    FILES
        "${CMAKE_BINARY_DIR}/libmikmod.pc"
    DESTINATION
        "lib${LIB_SUFFIX}/pkgconfig"
)
