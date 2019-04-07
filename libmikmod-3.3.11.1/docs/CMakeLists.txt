# Excerpt taken from http://cmake.3232098.n2.nabble.com/Building-Texinfo-with-CMake-td5609561.html
SET(ENABLE_DOC 1 CACHE BOOL "Build the library documentation")

find_program(MAKEINFO_EXECUTABLE makeinfo)

set(info_in ${CMAKE_CURRENT_SOURCE_DIR}/mikmod.texi)
set(info_out ${CMAKE_CURRENT_BINARY_DIR}/mikmod.info)
set(html_out ${CMAKE_CURRENT_BINARY_DIR}/mikmod.html)

IF (ENABLE_DOC)
add_custom_command(OUTPUT ${info_out}
  COMMAND ${MAKEINFO_EXECUTABLE} --no-split -o ${info_out} ${info_in}
  DEPENDS ${info_in}
  COMMENT "Creating Info file ${info_out}"
  VERBATIM)

add_custom_command(OUTPUT ${html_out}
  COMMAND ${MAKEINFO_EXECUTABLE} --no-split --html -o ${html_out} ${info_in}
  DEPENDS ${info_in}
  COMMENT "Creating HTML file ${html_out}"
  VERBATIM)

add_custom_target(info ALL DEPENDS ${info_out} ${html_out})
ENDIF(ENABLE_DOC)
