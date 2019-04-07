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

include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckFunctionExists)

# Taken from http://www.cmake.org/pipermail/cmake/2007-March/013060.html
MACRO(REPLACE_FUNCTIONS sources)
  FOREACH(name ${ARGN})
    STRING(TOUPPER have_${name} SYMBOL_NAME)
    CHECK_FUNCTION_EXISTS(${name} ${SYMBOL_NAME})
    IF(NOT ${SYMBOL_NAME})
      SET(${sources} ${${sources}} ${name}.c)
    ENDIF(NOT ${SYMBOL_NAME})
  ENDFOREACH(name)
ENDMACRO(REPLACE_FUNCTIONS)

MACRO(REPLACE_FUNCTIONS_FROMDIR sources dir)
  FOREACH(name ${ARGN})
    STRING(TOUPPER have_${name} SYMBOL_NAME)
    CHECK_FUNCTION_EXISTS(${name} ${SYMBOL_NAME})
    IF(NOT ${SYMBOL_NAME})
      SET(${sources} ${${sources}} ${dir}/${name}.c)
    ENDIF(NOT ${SYMBOL_NAME})
  ENDFOREACH(name)
ENDMACRO(REPLACE_FUNCTIONS_FROMDIR)

MACRO(CHECK_MULTI_INCLUDE_FILES)
  FOREACH(name ${ARGN})
    STRING(TOUPPER have_${name} SYMBOL_NAME)
    STRING(REGEX REPLACE "\\." "_" SYMBOL_NAME ${SYMBOL_NAME})
    STRING(REGEX REPLACE "/" "_" SYMBOL_NAME ${SYMBOL_NAME})
    CHECK_INCLUDE_FILE(${name} ${SYMBOL_NAME})
  ENDFOREACH(name)
ENDMACRO(CHECK_MULTI_INCLUDE_FILES)

MACRO(CHECK_MULTI_FUNCTIONS_EXISTS)
  FOREACH(name ${ARGN})
    STRING(TOUPPER have_${name} SYMBOL_NAME)
    CHECK_FUNCTION_EXISTS(${name} ${SYMBOL_NAME})
  ENDFOREACH(name)
ENDMACRO(CHECK_MULTI_FUNCTIONS_EXISTS)
