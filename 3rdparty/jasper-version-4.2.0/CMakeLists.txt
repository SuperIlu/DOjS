cmake_minimum_required(VERSION 3.20)
# Version 3.20 needed for cmake_path.
# Version 3.19 needed for file(REAL_PATH ...).
# Version 3.12 needed for FindJPEG imported targets.
# Version 3.10 needed for FindOpenGL imported targets.
# Version 3.1 needed for FindGLUT imported targets.

################################################################################
# Specify software and shared library version information.
################################################################################

# The major, minor, and micro version numbers of the project.
set(JAS_VERSION_MAJOR 4)
set(JAS_VERSION_MINOR 2)
set(JAS_VERSION_PATCH 0)

# The shared library versioning information.
# Guidelines on how to change this information can be found below.
# The shared library version was set to 7.0.0 for JasPer version 4.0.0.
set(JAS_SO_VERSION 7)
set(JAS_SO_MINOR 0)
set(JAS_SO_RELEASE 0)

# Guidelines for Updating the Shared Library Versioning Information
# =================================================================
#
# If the code did not change (e.g., only documentation was updated):
#     do not change the shared library versioning information.
# If the code changed and the binary interface for the library did not change
# from the previous release (e.g., most bug fixes):
#     increment JAS_SO_RELEASE.
# If the binary interface changed, but remains compatible with the previous
# release (e.g., only new functions were added):
#     - increment JAS_SO_MINOR; and
#     - set JAS_SO_RELEASE to 0.
# If the binary interface changed in a way that breaks compatibility with the
# previous release (e.g., a function was deleted):
#     - increment JAS_SO_VERSION; and
#     - set both JAS_SO_MINOR and JAS_SO_RELEASE to 0.

# Generate the project version.
set(JAS_VERSION
	"${JAS_VERSION_MAJOR}.${JAS_VERSION_MINOR}.${JAS_VERSION_PATCH}")

# Generate the shared library build version.
set(JAS_SO_NAME "${JAS_SO_VERSION}.${JAS_SO_MINOR}.${JAS_SO_RELEASE}")

message("Software version: ${JAS_VERSION}")
message("Shared library ABI version: ${JAS_SO_VERSION}")
message("Shared library build version: ${JAS_SO_NAME}")

message("CMAKE_VERSION: ${CMAKE_VERSION}")

project(JasPer VERSION ${JAS_VERSION} LANGUAGES C)

message("CMAKE_C_COMPILER_ID: ${CMAKE_C_COMPILER_ID}")
message("CMAKE_C_COMPILER_VERISON: ${CMAKE_C_COMPILER_VERSION}")

################################################################################
# Include modules and set policies.
################################################################################

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/build/cmake/modules)

# This include should be placed as early as possible.
include(InSourceBuild)

include(CheckCCompilerFlag)
include(EnableAssertions)

# Adhere to GNU filesystem layout conventions.
include(GNUInstallDirs)

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckTypeSize)
include(CheckCSourceCompiles)
include(CTest)
include(Sanitizers)
include(GenerateExportHeader)
include(JasGetIncludes)

# Note: JasGetStdcVersion uses try_run.
include(JasGetStdcVersion)
include(JasCheckType)

# Save some typing below.
function(add_c_compile_options args)
	foreach(arg IN LISTS ARGV)
		add_compile_options($<$<COMPILE_LANGUAGE:C>:${arg}>)
	endforeach()
endfunction()

################################################################################
# Define options.
################################################################################

# Note: The variable JAS_CROSSCOMPILING can be defined on the command line
# in order to override the value of CMAKE_CROSSCOMPILING (if necessary).

if(DEFINED BUILD_SHARED_LIBS)
	option(JAS_ENABLE_SHARED "Enable building of shared library"
	  ${BUILD_SHARED_LIBS})
else()
	option(JAS_ENABLE_SHARED "Enable building of shared library" ON)
endif()

option(JAS_PACKAGING "Enable packaging mode (e.g., disable RPATH)" OFF)

option(JAS_ENABLE_PIC "Enable position-independent code" ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ${JAS_ENABLE_PIC})

option(JAS_ENABLE_HIDDEN "Hide internal symbols" ON)
option(JAS_ENABLE_32BIT "Use a word size of 32 bits (instead of 64 bits) for fixed-point representations" OFF)
option(JAS_ENABLE_LIBJPEG "Enable the use of the JPEG Library" ON)
option(JAS_ENABLE_LIBHEIF "Enable the use of the HEIF Library" ON)
option(JAS_ENABLE_OPENGL "Enable the use of the OpenGL/GLUT Library" ON)
option(JAS_ENABLE_DOC "Enable building of the documentation" ON)
option(JAS_ENABLE_LATEX "Allow the use of LaTeX if it is available" ON)
option(JAS_ENABLE_PROGRAMS "Enable building of the programs" ON)
option(JAS_ENABLE_MULTITHREADING_SUPPORT "Enable multithreading support" ON)
option(JAS_PREFER_PTHREAD
  "Prefer POSIX Threads Library (over C11) for threading support" ON)
option(JAS_PREFER_PTHREAD_TSS
  "Prefer POSIX Threads Library thread-specific storage (over _Thread_local)"
  OFF)
option(JAS_STRICT "Treat compiler warnings as errors" OFF)

# Options controlling which codec support is built into the JasPer library.
option(JAS_INCLUDE_BMP_CODEC "Include (native) BMP codec support" ON)
option(JAS_INCLUDE_JP2_CODEC "Include (native) JP2 codec support" ON)
option(JAS_INCLUDE_JPC_CODEC "Include (native) JPC codec support" ON)
option(JAS_INCLUDE_JPG_CODEC
  "Include JPG codec support (via IJG JPEG Library)" ON)
option(JAS_INCLUDE_HEIC_CODEC
  "Include experimental HEIC codec support (via Libheif Library)" ON)
option(JAS_INCLUDE_MIF_CODEC "Include (native) MIF codec support" ON)
option(JAS_INCLUDE_PGX_CODEC "Include (native) PGX codec support" ON)
option(JAS_INCLUDE_PNM_CODEC "Include (native) PNM codec support" ON)
option(JAS_INCLUDE_RAS_CODEC "Include (native) RAS codec support" ON)

# Options controlling which codec support that is built into the JasPer
# library is enabled by default.
option(JAS_ENABLE_BMP_CODEC "Enable (native) BMP codec support" ON)
option(JAS_ENABLE_JP2_CODEC "Enable (native) JP2 codec support" ON)
option(JAS_ENABLE_JPC_CODEC "Enable (native) JPC codec support" ON)
option(JAS_ENABLE_JPG_CODEC
  "Enable JPG codec support (via IJG JPEG Library)" ON)
option(JAS_ENABLE_HEIC_CODEC
  "Enable experimental HEIC codec support (via Libheif Library)" OFF)
option(JAS_ENABLE_MIF_CODEC "Enable (native) MIF codec support" OFF)
option(JAS_ENABLE_PGX_CODEC "Enable (native) PGX codec support" ON)
option(JAS_ENABLE_PNM_CODEC "Enable (native) PNM codec support" ON)
option(JAS_ENABLE_RAS_CODEC "Enable (native) RAS codec support" ON)

set(JAS_DEFAULT_MAX_MEM_USAGE "1073741824" CACHE STRING
  "Set memory usage limit in bytes")

# The following options are for testing/debugging and should normally only
# be changed by the developers of JasPer.
option(JAS_USE_JAS_INIT
  "Use jas_init instead of jas_init_custom for initialization (should be false, except for testing)"
  OFF)
option(JAS_ENABLE_NON_THREAD_SAFE_DEBUGGING
  "Enable non-thread safe debug code" OFF)

if(JAS_USE_JAS_INIT)
	add_compile_definitions(JAS_USE_JAS_INIT)
endif()

# To override the default setting of max_align_t, something like what
# appears in the body of the if-statement following this comment can be used.
# WARNING: Do not do this unless you know what you are doing.
if(FALSE)
	add_compile_definitions("JAS_NO_SET_MAX_ALIGN_T")
	add_compile_definitions("max_align_t=__INSERT_APPROPRIATE_TYPE_HERE__")
endif()

# To override the default setting of ssize_t, something like what
# appears in the body of the if-statement following this comment can be used.
# WARNING: Do not do this unless you know what you are doing.
if(FALSE)
	add_compile_definitions("JAS_NO_SET_SSIZE_T")
	add_compile_definitions("ssize_t=__INSERT_APPROPRIATE_TYPE_HERE__")
endif()

# WARNING: THE FOLLOWING OPTION SHOULD ONLY BE ENABLED BY THE JASPER SOFTWARE
# DEVELOPMENT TEAM.  IT IS NECESSARY FOR INTERNAL TESTING PURPOSES AND POSES
# SIGNIFICANT SECURITY RISKS IF ENABLED.
option(JAS_ENABLE_DANGEROUS_INTERNAL_TESTING_MODE
  "DO NOT ENABLE FOR SECURITY REASONS! (For internal use by JasPer development team only"
  OFF)

option(JAS_ENABLE_CXX "Enable C++ code (for testing)." OFF)
option(JAS_ENABLE_CONFORMANCE_TESTS "Enable conformance tests." OFF)

option(JAS_WASM "Enable WebAssembly mode" OFF)
if(JAS_WASM)
	set(JAS_CROSSCOMPILING 1)
	set(JAS_STDC_VERSION "201112L")
	set(JAS_ENABLE_MULTITHREADING_SUPPORT 0)
	set(JAS_ENABLE_SHARED 0)
	set(JAS_USE_JAS_INIT 0)
	add_compile_definitions(JAS_WASI_LIBC)
endif()

################################################################################
#
################################################################################

if(JAS_ENABLE_CXX)
	message("Enabling C++")
	enable_language(CXX)
	message("CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
	message("CMAKE_CXX_COMPILER_VERSION: ${CMAKE_CXX_COMPILER_VERSION}")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# It is strongly recommended that the JasPer software be built using a
# compiler that supports the C11 (or later) version of the C standard, if
# such a compiler is available.  The use of compilers supporting older
# versions of the language standard, such as C99, should only be used when
# no C11 support is available.
#set(CMAKE_C_STANDARD 99) # This should work but is not recommended.
set(CMAKE_C_STANDARD 11)

# Enable extensions so that POSIX support is visible (e.g., ssize_t) without
# having to manually define _POSIX_C_SOURCE.
# Setting this to OFF will break things for at least some versions of MacOS
# (e.g, due to u_char, u_short, and u_int being undefined).
# In any case, it might be a bad idea to set this to OFF due to issues
# related to max_align_t and ssize_t.
set(CMAKE_C_EXTENSIONS ON)

# Ensure that that the try_compile command (and the commands that use it)
# use the compiler option for the language standard being used.
# This policy was added in CMake 3.8.
cmake_policy(SET CMP0067 NEW)

if(JAS_ENABLE_CXX)
	set(CMAKE_CXX_STANDARD 11)
	set(CMAKE_CXX_EXTENSIONS ON)
endif()

# Eliminate a CMake warning.
# Note: This policy was added in CMake 3.20.  Do not add this for now,
# since it will cause older versions of CMake to barf.
#cmake_policy(SET CMP0115 NEW)

if(APPLE AND CMAKE_SYSTEM_NAME MATCHES "Darwin")
	set(MACOS TRUE)
else()
	set(MACOS FALSE)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(LINUX TRUE)
else()
	set(LINUX FALSE)
endif()

if((CMAKE_GENERATOR MATCHES "Xcode") OR
  (CMAKE_GENERATOR MATCHES "Visual Studio") OR
  (CMAKE_GENERATOR STREQUAL "Ninja Multi-Config"))
	set(JAS_MULTICONFIGURATION_GENERATOR TRUE)
	message("JAS_MULTICONFIGURATION_GENERATOR ${JAS_MULTICONFIGURATION_GENERATOR}")
else()
	set(JAS_MULTICONFIGURATION_GENERATOR FALSE)
endif()
message("JAS_MULTICONFIGURATION_GENERATOR ${JAS_MULTICONFIGURATION_GENERATOR}")

# If a multiconfiguration generator is used, ensure that various output
# files are not placed in subdirectories (such as Debug and Release)
# as this will cause the CTest test suite to fail.
if(JAS_MULTICONFIGURATION_GENERATOR)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY .)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY .)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY .)
	foreach(config ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER "${config}" config)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config} .)
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${config} .)
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${config} .)
	endforeach()
endif()

include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_NAME "${CMAKE_PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${JAS_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "JasPer Image Processing Tool Kit")
set(CPACK_PACKAGE_VENDOR "Michael Adams")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${JAS_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${JAS_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${JAS_VERSION_PATCH}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY
  "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
set(CPACK_PACKAGE_FILE_NAME
  "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_GENERATOR "TGZ")
include(CPack)

if(CMAKE_BUILD_TYPE)
	message("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
elseif(CMAKE_CONFIGURATION_TYPES)
	message("CMAKE_CONFIGURATION_TYPES: ${CMAKE_CONFIGURATION_TYPES}")
endif()
message("CMAKE_C_COMPILER_ID: ${CMAKE_C_COMPILER_ID}")
message("CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message("CMAKE_LINKER: ${CMAKE_LINKER}")

# Customize the compiler flags for warning messages.
if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
	check_c_compiler_flag("-Wno-stringop-truncation"
	  JAS_GCC_WARN_NO_STRINGOP_TRUNCATION)
	check_c_compiler_flag("-Wno-stringop-overflow"
	  JAS_GCC_WARN_NO_STRINGOP_OVERFLOW)
	check_c_compiler_flag("-Wno-tautological-constant-out-of-range-compare"
	  JAS_CLANG_WARN_NO_OUT_OF_RANGE_COMPARE)
	add_c_compile_options(-Wall)

	# Note: Use CMAKE_C_FLAGS (instead of add_compile_options) so that the
	# flags are also used for compile checks.
	#add_c_compile_options(-pedantic)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")

	add_c_compile_options(-W)
	add_c_compile_options(-Wno-sign-compare)
	add_c_compile_options(-Wformat)
	add_c_compile_options(-Wmissing-prototypes)
	add_c_compile_options(-Wstrict-prototypes)
	add_c_compile_options(-Wno-type-limits)
	if (JAS_CLANG_WARN_NO_OUT_OF_RANGE_COMPARE)
		add_c_compile_options(-Wno-tautological-constant-out-of-range-compare)
	endif()
	if(JAS_GCC_WARN_NO_STRINGOP_TRUNCATION)
		add_c_compile_options(-Wno-stringop-truncation)
	endif()
	if(JAS_GCC_WARN_NO_STRINGOP_OVERFLOW)
		# Note: Sometimes spurious warnings that have nothing to do with
		# string are generated.
		add_c_compile_options(-Wno-stringop-overflow)
	endif()
	add_c_compile_options(-Wmissing-declarations)
	if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
		add_c_compile_options(-Wno-unused-command-line-argument)
	endif()
	if(JAS_STRICT)
		# Treat all compiler warnings as errors.
		# Note: Use CMAKE_C_FLAGS (instead of add_compile_options) so that the
		# flags are also used for compile checks.
		#add_c_compile_options(-pedantic-errors)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic-errors")
		#add_c_compile_options(-Werror)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
	endif()
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
	# possibly lossy conversion
	add_c_compile_options(/wd4244)
	# different const qualifiers
	add_c_compile_options(/wd4090)
	# signed/unsigned mismatch
	add_c_compile_options(/wd4018)
	# function may be unsafe (e.g., sprintf, strncat)
	add_c_compile_options(/wd4996)
	# allow some kinds of narrowing conversions
	add_c_compile_options(/wd4267)
	if(JAS_STRICT)
		# Treat all compiler warnings as errors.
		add_c_compile_options(/WX)
	endif()
endif()

if(JAS_ENABLE_HIDDEN)
	set(CMAKE_C_VISIBILITY_PRESET hidden)
	set(CMAKE_VISIBILITY_INLINES_HIDDEN 0)
	if(NOT (CMAKE_BUILD_TYPE MATCHES "Debug"))
		if(LINUX)
			check_c_compiler_flag("-ffunction-sections" HAVE_FUNCTION_SECTIONS)
			check_c_compiler_flag("-fdata-sections" HAVE_DATA_SECTIONS)
			check_c_compiler_flag("-Wl,--gc-sections" HAVE_GC_SECTIONS)
			if(HAVE_FUNCTION_SECTIONS AND HAVE_DATA_SECTIONS AND
			  HAVE_GC_SECTIONS)
				add_c_compile_options(-ffunction-sections)
				add_c_compile_options(-fdata-sections)
				add_c_compile_options(-Wl,--gc-sections)
			endif()
		endif()
	endif()
endif()

message("CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
message("CMAKE_C_FLAGS_RELEASE: ${CMAKE_C_FLAGS_RELEASE}")
message("CMAKE_C_FLAGS_DEBUG: ${CMAKE_C_FLAGS_DEBUG}")

################################################################################
# Perform platform checks.
################################################################################

#unset(JAS_CMAKE_REQUIRED_FLAGS)
#unset(JAS_CMAKE_REQUIRED_DEFINITIONS)
#if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
#	set(JAS_CMAKE_REQUIRED_FLAGS ${JAS_CMAKE_REQUIRED_FLAGS}
#	  -Wpedantic -pedantic-errors -Werror)
#	#set(JAS_CMAKE_REQUIRED_DEFINITIONS ${JAS_CMAKE_REQUIRED_DEFINITIONS}
#	#  "_POSIX_C_SOURCE=200809L")
#endif()
#set(CMAKE_REQUIRED_FLAGS ${JAS_CMAKE_REQUIRED_FLAGS})
#set(JAS_CMAKE_REQUIRED_DEFINITIONS ${JAS_CMAKE_REQUIRED_DEFINITIONS})

# Get the value of the __STDC_VERSION__ preprocessor define.
if((DEFINED JAS_CROSSCOMPILING AND JAS_CROSSCOMPILING) OR
  (NOT DEFINED JAS_CROSSCOMPILING AND CMAKE_CROSSCOMPILING))
	message("A cross-compiling environment has been detected.")
	# Note: The variable JAS_STDC_VERSION should be set to the value of the
	# preprocessor symbol __STDC_VERSION__ on the target platform.
	# Since CMake is unable to run a cross-compiled program, the value of
	# __STDC_VERSION__ cannot be automatically determined when
	# cross-compiling.  If cross-compiling, the value of JAS_STDC_VERSION
	# will need to be set manually from the command line
	# (e.g., using -DJAS_STDC_VERSION=YYYYMML) or by changing the line below.
if(NOT JAS_WASM)
	set(JAS_STDC_VERSION "0L" CACHE INTERNAL "The value of __STDC_VERSION__.")
	if (JAS_STDC_VERSION STREQUAL "0L")
		message(FATAL_ERROR
		  "The value of __STDC_VERSION__ cannot be automatically determined "
		  "when cross-compiling.  Please set JAS_STDC_VERSION to the value "
		  "of __STDC_VERSION__ when invoking CMake (e.g., by using the "
		  "option -DJAS_STDC_VERSION=...) or modify the CMakeLists.txt "
		  "appropriately.")
	endif()
endif()
else()
	jas_get_stdc_version(status JAS_STDC_VERSION)
	if(NOT status)
		message(FATAL_ERROR "Cannot determine the value of __STDC_VERSION__.")
	endif()
endif()
message("__STDC_VERSION__: ${JAS_STDC_VERSION}")

find_program(BASH_PROGRAM bash)

check_include_files(fcntl.h JAS_HAVE_FCNTL_H)
check_include_files(io.h JAS_HAVE_IO_H)
check_include_files(unistd.h JAS_HAVE_UNISTD_H)
check_include_files(sys/time.h JAS_HAVE_SYS_TIME_H)
check_include_files(sys/types.h JAS_HAVE_SYS_TYPES_H)

check_type_size(max_align_t MAX_ALIGN_T)
if(HAVE_MAX_ALIGN_T)
	set(JAS_HAVE_MAX_ALIGN_T 1 CACHE INTERNAL "Test JAS_HAVE_MAX_ALIGN_T")
else()
	set(JAS_HAVE_MAX_ALIGN_T 0 CACHE INTERNAL "Test JAS_HAVE_MAX_ALIGN_T")
endif()
message("JAS_HAVE_MAX_ALIGN_T: ${JAS_HAVE_MAX_ALIGN_T}")

jas_check_type("char" JAS_HAVE_CHAR SIZE JAS_SIZEOF_CHAR)
message("JAS_SIZEOF_CHAR: ${JAS_SIZEOF_CHAR}")

jas_check_type("int" JAS_HAVE_INT SIZE JAS_SIZEOF_INT)
message("JAS_SIZEOF_INT: ${JAS_SIZEOF_INT}")

jas_check_type("long" JAS_HAVE_LONG SIZE JAS_SIZEOF_LONG)
message("JAS_SIZEOF_LONG: ${JAS_SIZEOF_LONG}")

jas_check_type("long long" JAS_HAVE_LLONG SIZE JAS_SIZEOF_LLONG)
message("JAS_SIZEOF_LLONG: ${JAS_SIZEOF_LLONG}")

jas_check_type("size_t" JAS_HAVE_SIZE_T SIZE JAS_SIZEOF_SIZE_T)
message("JAS_SIZEOF_SIZE_T: ${JAS_SIZEOF_SIZE_T}")

jas_check_type("ssize_t" JAS_HAVE_SSIZE_T SIZE JAS_SIZEOF_SSIZE_T)
message("JAS_HAVE_SSIZE_T: ${JAS_HAVE_SSIZE_T}")
if(JAS_HAVE_SSIZE_T)
	message("JAS_SIZEOF_SSIZE_T: ${JAS_SIZEOF_SSIZE_T}")
endif()

jas_check_type("int_fast32_t" JAS_HAVE_INT_FAST32_T SIZE
  JAS_SIZEOF_INT_FAST32_T)
message("JAS_SIZEOF_INT_FAST32_T: ${JAS_SIZEOF_INT_FAST32_T}")

jas_check_type("int_least32_t" JAS_HAVE_INT_LEAST32_T SIZE
  JAS_SIZEOF_INT_LEAST32_T)
message("JAS_SIZEOF_INT_LEAST32_T: ${JAS_SIZEOF_INT_LEAST32_T}")

jas_check_type("int_fast64_t" JAS_HAVE_INT_FAST64_T SIZE
  JAS_SIZEOF_INT_FAST64_T)
message("JAS_SIZEOF_INT_FAST64_T: ${JAS_SIZEOF_INT_FAST64_T}")

jas_check_type("int_least64_t" JAS_HAVE_INT_LEAST64_T SIZE
  JAS_SIZEOF_INT_LEAST64_T)
message("JAS_SIZEOF_INT_LEAST64_T: ${JAS_SIZEOF_INT_LEAST64_T}")

jas_check_type("__int128_t" JAS_HAVE_INT128_T SIZE
  JAS_SIZEOF_INT128_T)
message("JAS_SIZEOF_INT128_T: ${JAS_SIZEOF_INT128_T}")

#check_type_size(ssize_t SSIZE_T)
#if(HAVE_SSIZE_T)
#	set(JAS_HAVE_SSIZE_T 1 CACHE INTERNAL "Test JAS_HAVE_SSIZE_T")
#else()
#	set(JAS_HAVE_SSIZE_T 0 CACHE INTERNAL "Test JAS_HAVE_SSIZE_T")
#endif()
#message("JAS_HAVE_SSIZE_T: ${JAS_HAVE_SSIZE_T}")

#check_type_size(uintmax_t UINTMAX_T)
#if(HAVE_UINTMAX_T)
#	set(JAS_HAVE_UINTMAX_T 1 CACHE INTERNAL "Test JAS_HAVE_UINTMAX_T")
#else()
#	set(JAS_HAVE_UINTMAX_T 0 CACHE INTERNAL "Test JAS_HAVE_UINTMAX_T")
#endif()
#message("JAS_HAVE_UINTMAX_T: ${JAS_HAVE_UINTMAX_T}")

#check_type_size(intmax_t INTMAX_T)
#if(HAVE_INTMAX_T)
#	set(JAS_HAVE_INTMAX_T 1 CACHE INTERNAL "Test JAS_HAVE_INTMAX_T")
#else()
#	set(JAS_HAVE_INTMAX_T 0 CACHE INTERNAL "Test JAS_HAVE_INTMAX_T")
#endif()
#message("JAS_HAVE_INTMAX_T: ${JAS_HAVE_INTMAX_T}")

check_function_exists(nanosleep JAS_HAVE_NANOSLEEP)
check_function_exists(gettimeofday JAS_HAVE_GETTIMEOFDAY)
check_function_exists(getrusage JAS_HAVE_GETRUSAGE)
check_function_exists(mkostemp JAS_HAVE_MKOSTEMP)
if(MSVC)
	check_symbol_exists(snprintf stdio.h JAS_HAVE_SNPRINTF)
else()
	check_function_exists(snprintf JAS_HAVE_SNPRINTF)
endif()

################################################################################
# Check for invalid codec combinations.
################################################################################

if(NOT JAS_ENABLE_DANGEROUS_INTERNAL_TESTING_MODE AND JAS_ENABLE_MIF_CODEC)
	message(FATAL_ERROR
	  "For reasons of security, it is not recommended that the MIF codec "
	  "be enabled by default."
	  )
	set(JAS_ENABLE_MIF_CODEC OFF)
endif()
message("JAS_ENABLE_MIF_CODEC: ${JAS_ENABLE_MIF_CODEC}")

if (JAS_INCLUDE_BMP_CODEC AND
  JAS_INCLUDE_JP2_CODEC AND
  JAS_INCLUDE_JPC_CODEC AND
  JAS_INCLUDE_PGX_CODEC AND
  JAS_INCLUDE_PNM_CODEC AND
  JAS_INCLUDE_MIF_CODEC AND
  JAS_INCLUDE_RAS_CODEC)
	message("Support for all native codecs has been included.")
	set(JAS_HAVE_ALL_NATIVE_CODECS 1)
else()
	message(WARNING
	  "Support for one or more native codecs has been excluded.  "
	  "The inclusion of support for all native codecs is "
	  "STRONGLY RECOMMENDED.")
	set(JAS_HAVE_ALL_NATIVE_CODECS 0)
endif()

if(JAS_INCLUDE_JP2_CODEC AND NOT JAS_INCLUDE_JPC_CODEC)
	message(FATAL_ERROR
	  "The JPC codec must be included in order to include the JP2 codec.")
endif()

# Prepare variable for pkgconfig file
set(JAS_PKGCONFIG_REQUIRES)

################################################################################
# Configure threading support
################################################################################

find_library(PTHREAD_LIBRARY pthread)
if(NOT PTHREAD_LIBRARY)
	set(PTHREAD_LIBRARY "")
endif()

set(JAS_THREADS FALSE)
set(JAS_THREADS_C11 FALSE)
set(JAS_THREADS_PTHREAD FALSE)
set(JAS_THREADS_WIN32 FALSE)
set(JAS_HAVE_THREAD_LOCAL FALSE)
set(JAS_HAVE_C11_THREADS FALSE)
set(JAS_HAVE_PTHREAD_THREADS FALSE)
set(JAS_HAVE_WIN32_THREADS FALSE)
if(JAS_ENABLE_MULTITHREADING_SUPPORT)
	check_c_source_compiles("
	#include <threads.h>
	#include <stdatomic.h>
	int main() {
		thrd_t thread;
		mtx_t mutex;
		tss_t tss;
	}
	" JAS_TRY_COMPILE_C11_THREADS)
	check_c_source_compiles("
	typedef struct {int x;} foo_t;
	/* Use a pointer to a struct to entice a warning from GCC
	in C99 mode with -Wpedantic. */
	extern _Thread_local foo_t *foo;
	int main() {
	}
	" JAS_TRY_COMPILE_THREAD_LOCAL)
	check_include_files(pthread.h JAS_HAVE_PTHREAD_H)

	# Check for support for various threading libraries.
	if(MACOS)
		set(THREADS_PREFER_PTHREAD_FLAG TRUE)
		set(CMAKE_HAVE_THREADS_LIBRARY 1)
		set(CMAKE_THREAD_LIBS_INIT "-lpthread")
		set(CMAKE_USE_WIN32_THREADS_INIT 0)
		set(CMAKE_USE_PTHREADS_INIT 1)
		set(Threads_FOUND TRUE)
	else()
		set(THREADS_PREFER_PTHREAD_FLAG TRUE)
		find_package(Threads)
	endif()
	if(Threads_FOUND)
		if(CMAKE_USE_PTHREADS_INIT AND JAS_HAVE_PTHREAD_H)
			set(JAS_HAVE_PTHREAD_THREADS TRUE)
		endif()
		if(CMAKE_USE_WIN32_THREADS_INIT)
			set(JAS_HAVE_WIN32_THREADS TRUE)
		endif()
		set(THREAD_LIBRARY "${CMAKE_THREAD_LIBS_INIT}")
		if(JAS_TRY_COMPILE_C11_THREADS)
			set(JAS_HAVE_C11_THREADS TRUE)
		endif()
		if(JAS_TRY_COMPILE_THREAD_LOCAL AND NOT JAS_PREFER_PTHREAD_TSS)
			set(JAS_HAVE_THREAD_LOCAL TRUE)
		endif()
	endif()

	# TODO: Is this really needed?  Is this due to a TSan issue?
	if(MACOS)
		set(JAS_PREFER_PTHREAD FALSE)
	endif()

	# Apply preferences to select one particular threading library.
	if(JAS_HAVE_PTHREAD_THREADS AND JAS_PREFER_PTHREAD)
		set(JAS_THREADS_PTHREAD TRUE)
	elseif(JAS_HAVE_C11_THREADS)
		set(JAS_THREADS_C11 TRUE)
	elseif(JAS_HAVE_WIN32_THREADS)
		set(JAS_THREADS_WIN32 TRUE)
	elseif(JAS_HAVE_PTHREAD_THREADS)
		set(JAS_THREADS_PTHREAD TRUE)
	endif()
	if(JAS_THREADS_C11 OR JAS_THREADS_PTHREAD OR JAS_THREADS_WIN32)
		set(JAS_THREADS TRUE)
		message("Threading support enabled.")
	else()
		message(WARNING "No threading support found.")
	endif()

	# TODO/NOTE: This is an attempt at a CMake/Ubuntu bug workaround.
	if(LINUX AND JAS_THREADS_PTHREAD)
		set(THREAD_LIBRARY ${THREAD_LIBRARY} ${PTHREAD_LIBRARY})
	endif()

	message("Threads_FOUND: ${Threads_FOUND}")
	message("Pthread library: ${PTHREAD_LIBRARY}")
	message("Prefer POSIX threading support: ${JAS_PREFER_PTHREAD}")
	message("Threading support found: ${JAS_THREADS}")
	message("C11 threading support detected: ${JAS_HAVE_C11_THREADS}")
	message("POSIX threading support detected: ${JAS_HAVE_PTHREAD_THREADS}")
	message("WIN32 threading support detected: ${JAS_HAVE_WIN32_THREADS}")
	message("C11 threading support selected: ${JAS_THREADS_C11}")
	message("POSIX threading support selected: ${JAS_THREADS_PTHREAD}")
	message("Win32 threading support selected: ${JAS_THREADS_WIN32}")
	message("Threading library: ${THREAD_LIBRARY}")
	message("_Thread_local: ${JAS_HAVE_THREAD_LOCAL}")
else()
	message("Threading support has been disabled.")
endif()

if(JAS_ENABLE_MULTITHREADING_SUPPORT AND NOT JAS_THREADS)
	message(FATAL_ERROR "Threading support is not available.")
endif()

if(JAS_THREADS_C11 AND JAS_ENABLE_TSAN)
	message(FATAL_ERROR
	  "TSAN does not have interceptors for C11 concurrency primitives.  "
	  "Using C11 threads and TSan will likely not end well.  "
	  "See https://github.com/google/sanitizers/issues/1195")
endif()

################################################################################
# Check for the OpenGL and GLUT libraries.
################################################################################

set(JAS_HAVE_OPENGL 0)
set(JAS_OPENGL_TARGETS)
if(JAS_ENABLE_OPENGL)
	set(OpenGL_GL_PREFERENCE GLVND)
	find_package(OpenGL)
	find_package(GLUT)
	if((TARGET OpenGL::GL) AND (TARGET GLUT::GLUT))
		jas_get_includes_from_targets(CMAKE_REQUIRE_INCLUDES
		  OpenGL::GL OpenGL::GLU GLUT::GLUT)
		message("CMAKE_REQUIRE_INCLUDES ${CMAKE_REQUIRE_INCLUDES}")
		check_include_files(GL/glut.h JAS_HAVE_GL_GLUT_H)
		check_include_files(GLUT/glut.h JAS_HAVE_GLUT_GLUT_H)
		check_include_files(glut.h JAS_HAVE_GLUT_H)
		if(JAS_HAVE_GL_GLUT_H OR JAS_HAVE_GLUT_GLUT_H OR JAS_HAVE_GLUT_H)
			set(JAS_OPENGL_TARGETS GLUT::GLUT OpenGL::GL)
			if (TARGET OpenGL::GLU)
				set(JAS_OPENGL_TARGETS ${JAS_OPENGL_TARGETS} OpenGL::GLU)
			endif()
			set(JAS_HAVE_OPENGL 1)
		else()
			message(WARNING "The glut.h header file is missing.")
			message(WARNING "Disabling OpenGL.")
		endif()
	endif()
endif()

if(JAS_HAVE_OPENGL AND MACOS)
	add_compile_definitions(GL_SILENCE_DEPRECATION)
endif()

################################################################################
# Check for the JPEG library.
################################################################################

# The following is needed to prevent the incorrect version of the JPEG library
# from being found.
if(MACOS)
	set(CMAKE_FIND_FRAMEWORK LAST)
endif()

set(JAS_HAVE_LIBJPEG 0)
set(JAS_LIBJPEG_TARGET)
if(JAS_ENABLE_LIBJPEG)
	find_package(JPEG)
	if (TARGET JPEG::JPEG)
		# In some versions of the JPEG library, the header file jpeglib.h
		# does not include some of the header files upon which it depends
		# (e.g., stdio.h and stdint.h).  So, we cannot reliably use
		# check_include_file here.
		jas_get_includes_from_targets(CMAKE_REQUIRED_INCLUDES JPEG::JPEG)
		check_c_source_compiles("
			#include <stdio.h>
			#include <stdint.h>
			#include <jpeglib.h>
			int main() {}
		" JAS_HAVE_JPEGLIB_H)
		if(JAS_HAVE_JPEGLIB_H)
			set(JAS_HAVE_LIBJPEG 1)
			set(JAS_LIBJPEG_TARGET JPEG::JPEG)
			list(APPEND JAS_PKGCONFIG_REQUIRES libjpeg)
		else()
			message(WARNING "The header file jpeglib.h is missing.")
			message(WARNING "Disabling LIBJPEG.")
		endif()
	endif()
endif()

if(NOT JAS_HAVE_LIBJPEG)
	set(JAS_INCLUDE_JPG_CODEC 0)
endif()
message("JAS_INCLUDE_JPG_CODEC: ${JAS_INCLUDE_JPG_CODEC}")

################################################################################
# Check for the HEIF library.
################################################################################

set(JAS_HAVE_LIBHEIF 0)
if(JAS_ENABLE_LIBHEIF)
	find_library(HEIF_LIBRARY heif)
	check_include_files(libheif/heif.h JAS_HAVE_HEIF_H)
	if(HEIF_LIBRARY AND JAS_HAVE_HEIF_H)
		set(JAS_HAVE_LIBHEIF 1)
		set(HEIF_LIBRARIES "${HEIF_LIBRARY}")
		list(APPEND JAS_PKGCONFIG_REQUIRES libheif)
	endif()
endif()
if(NOT JAS_HAVE_LIBHEIF)
	set(JAS_INCLUDE_HEIC_CODEC 0)
endif()
message("JAS_INCLUDE_HEIC_CODEC: ${JAS_INCLUDE_HEIC_CODEC}")

################################################################################
# Check for the Math library.
################################################################################

find_library(MATH_LIBRARY m)
if(NOT MATH_LIBRARY)
	set(MATH_LIBRARY "")
endif()

################################################################################
# Perform shared library setup.
################################################################################

if(JAS_ENABLE_SHARED AND NOT JAS_PACKAGING)

	if(MACOS)
		# On MacOS, the target is located at runtime using rpaths.
		set(CMAKE_MACOSX_RPATH TRUE)
	endif()

	# Adjust the runtime search path (rpath) for binaries in the build tree
	# (i.e., do not skip build rpaths).
	set(CMAKE_SKIP_BUILD_RPATH FALSE)

	# Do not use the install rpath for binaries in the build tree.
	# We want to use the build rpath (not install rpath) for binaries in the
	# build tree.
	set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

	# Append to the rpath of installed binaries any directories outside the
	# project that are in the linker search path or contain linked library
	# files.
	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

	# Specify the rpath for the installed targets.
	# We only want to include directories in the installed rpath if they
	# will not be considered implicitly.
	list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
	  "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" jas_is_system_dir)
	if(jas_is_system_dir EQUAL -1)
	   set(CMAKE_INSTALL_RPATH
		  "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
	endif()

endif()

################################################################################
# Miscellany.
################################################################################

add_subdirectory(src/libjasper)
if(JAS_ENABLE_PROGRAMS)
    add_subdirectory(src/app)
endif()
if(JAS_ENABLE_DOC)
	add_subdirectory(doc)
endif()

# The package configuation (pc) file should be installed in
# ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig.
string(JOIN " " JAS_PKGCONFIG_REQUIRES ${JAS_PKGCONFIG_REQUIRES})
# Handle both relative and absolute paths (e.g. NixOS) for a relocatable package
if(IS_ABSOLUTE "${CMAKE_INSTALL_INCLUDEDIR}")
    set(JAS_PC_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}")
else()
    set(JAS_PC_INCLUDEDIR "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
endif()
if(IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}")
    set(JAS_PC_LIBDIR "${CMAKE_INSTALL_LIBDIR}")
else()
    set(JAS_PC_LIBDIR "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
endif()
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/build/pkgconfig/jasper.pc.in"
  "${CMAKE_CURRENT_BINARY_DIR}/build/pkgconfig/jasper.pc" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/build/pkgconfig/jasper.pc"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

install(FILES "README.md" DESTINATION "${CMAKE_INSTALL_DOCDIR}")
