# The programs that are for internal use only and not installed.
set(private_programs "")

# The programs that are to be installed.
set(public_programs "")

# The manual pages that are to be installed.
set(man_pages "")

# The jasper program
add_executable(jasper jasper.c)
target_link_libraries(jasper libjasper ${JAS_LIBJPEG_TARGET} ${HEIF_LIBRARIES}
  ${MATH_LIBRARY} ${THREAD_LIBRARY})
set(public_programs "${public_programs}" jasper)
set(man_pages "${man_pages}" jasper.1)

# The imginfo program
add_executable(imginfo imginfo.c)
target_link_libraries(imginfo libjasper ${JAS_LIBJPEG_TARGET} ${HEIF_LIBRARIES}
  ${MATH_LIBRARY} ${THREAD_LIBRARY})
set(public_programs "${public_programs}" imginfo)
set(man_pages "${man_pages}" imginfo.1)

# The imgcmp program
add_executable(imgcmp imgcmp.c)
target_link_libraries(imgcmp libjasper ${JAS_LIBJPEG_TARGET} ${HEIF_LIBRARIES}
  ${MATH_LIBRARY} ${THREAD_LIBRARY})
set(public_programs "${public_programs}" imgcmp)
set(man_pages "${man_pages}" imgcmp.1)

# The tmrdemo program
add_executable(tmrdemo tmrdemo.c)
target_link_libraries(tmrdemo libjasper ${JAS_LIBJPEG_TARGET} ${HEIF_LIBRARIES}
  ${MATH_LIBRARY} ${THREAD_LIBRARY})
set(private_programs "${private_programs}" tmrdemo)

# The jiv program (which requires OpenGL and GLUT)
if(JAS_HAVE_OPENGL)
	add_executable(jiv jiv.c)
	target_link_libraries(jiv libjasper
	  ${JAS_LIBJPEG_TARGET} ${HEIF_LIBRARIES} ${JAS_OPENGL_TARGETS}
	  ${JAS_OPENGL_TARGETS} ${MATH_LIBRARY} ${THREAD_LIBRARY})
	set(public_programs "${public_programs}" jiv)
	set(man_pages "${man_pages}" jiv.1)
else()
	message("GLUT library not found; skipping build of jiv program")
endif()

if(JAS_THREADS)
	# The multithread program
	add_executable(multithread multithread.c)
	target_link_libraries(multithread libjasper ${JAS_LIBJPEG_TARGET}
	  ${MATH_LIBRARY} ${THREAD_LIBRARY})
	set(private_programs "${private_programs}" multithread)
endif()

# The test_1 program.
add_executable(test_1 test_1.c)
target_link_libraries(test_1 libjasper ${JAS_LIBJPEG_TARGET}
  ${MATH_LIBRARY} ${THREAD_LIBRARY})
set(private_programs "${private_programs}" test_1)

if(JAS_ENABLE_CXX)
	# The test_2 program.
	add_executable(test_2 test_2.cpp)
	target_link_libraries(test_2 libjasper ${JAS_LIBJPEG_TARGET}
	  ${THREAD_LIBRARY})
	set(private_programs "${private_programs}" test_2)
endif()

if(JAS_ENABLE_FUZZER)
	add_executable(fuzz fuzz.c)
	target_link_libraries(fuzz libjasper ${JAS_LIBJPEG_TARGET}
	  ${MATH_LIBRARY} ${THREAD_LIBRARY})
	set(private_programs "${private_programs}" fuzz)
	target_compile_options(fuzz PRIVATE "-fsanitize=fuzzer")
	if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
		target_link_options(fuzz PRIVATE "-fsanitize=fuzzer")
	else()
		set_target_properties(fuzz PROPERTIES LINK_FLAGS "-fsanitize=fuzzer")
	endif()
endif()

# The JasPer include directories in the source and build tree must be included
# before any other directories that may contain the include directory for an
# already installed version of the JasPer library.
include_directories(BEFORE
  "${CMAKE_CURRENT_BINARY_DIR}/../libjasper/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/../libjasper/include"
)

# Install of the the programs that were built.
install(TARGETS ${public_programs} DESTINATION "${CMAKE_INSTALL_BINDIR}")

# Install the manual pages for the programs that were built.
install(FILES ${man_pages} DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")

################################################################################
# Test suite
################################################################################

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/../../test/bin/wrapper.in"
  "${CMAKE_CURRENT_BINARY_DIR}/../../test/bin/wrapper" @ONLY)

if(BASH_PROGRAM AND JAS_HAVE_ALL_NATIVE_CODECS)
	add_test(run_test_imginfo
	  "${BASH_PROGRAM}" "${CMAKE_CURRENT_BINARY_DIR}/../../test/bin/wrapper"
	  "${CMAKE_CURRENT_SOURCE_DIR}/../../test/bin/run_test_imginfo" -v)
	add_test(run_test_imgcmp
	  "${BASH_PROGRAM}" "${CMAKE_CURRENT_BINARY_DIR}/../../test/bin/wrapper"
	  "${CMAKE_CURRENT_SOURCE_DIR}/../../test/bin/run_test_imgcmp" -v)
	add_test(run_test_1
	  "${BASH_PROGRAM}" "${CMAKE_CURRENT_BINARY_DIR}/../../test/bin/wrapper"
	  "${CMAKE_CURRENT_SOURCE_DIR}/../../test/bin/run_test_1" -v)
	add_test(run_test_2
	  "${BASH_PROGRAM}" "${CMAKE_CURRENT_BINARY_DIR}/../../test/bin/wrapper"
	  "${CMAKE_CURRENT_SOURCE_DIR}/../../test/bin/run_test_2" -v)
	add_test(run_test_3
	  "${BASH_PROGRAM}" "${CMAKE_CURRENT_BINARY_DIR}/../../test/bin/wrapper"
	  "${CMAKE_CURRENT_SOURCE_DIR}/../../test/bin/run_test_3" -v)
	if(JAS_ENABLE_CONFORMANCE_TESTS)
		add_test(run_conformance_tests
		  "${BASH_PROGRAM}" "${CMAKE_CURRENT_BINARY_DIR}/../../test/bin/wrapper"
		  "${CMAKE_CURRENT_SOURCE_DIR}/../../test/bin/run_conformance_tests" -v)
	endif()
	add_custom_target(conformance_check
	  COMMAND
	  "${BASH_PROGRAM}" "${CMAKE_CURRENT_BINARY_DIR}/../../test/bin/wrapper"
	  "${CMAKE_CURRENT_SOURCE_DIR}/../../test/bin/run_conformance_tests" -v -a
	  DEPENDS jasper imgcmp imginfo
	)
	if(JAS_THREADS)
		add_test(run_test_5
		  "${BASH_PROGRAM}" "${CMAKE_CURRENT_BINARY_DIR}/../../test/bin/wrapper"
		  "${CMAKE_CURRENT_SOURCE_DIR}/../../test/bin/run_test_5" -v)
		set(private_programs "${private_programs}" multithread)
	endif()
	add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS
	  ${public_programs} ${private_programs}
	  "${CMAKE_CURRENT_BINARY_DIR}/../../test/bin/wrapper") 
else()
	if (NOT JAS_HAVE_ALL_NATIVE_CODECS)
		message(WARNING
		  "The test suite has been disabled "
		  "due to missing native codec support.")
	else()
		message(WARNING "The test suite has been disabled.")
	endif()
endif()
