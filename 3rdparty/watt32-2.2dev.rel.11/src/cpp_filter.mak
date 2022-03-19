#
# A GNU-makefile to output a C preprocessed file of
# any .c-file in ./src or ./src/test.
#
# Example usage:
#   To C-preprocess using MSVC compiler, use:
#     make -f cpp_filter.mak MSVC_CHECK=1 socket.i
#
#   To C-preprocess using PelleC compiler, use:
#     make -f cpp_filter.mak POCC_CHECK=1 socket.i
#
#   To C-preprocess using djgpp/gcc compiler, use:
#     make -f cpp_filter.mak DJGPP_CHECK=1 socket.i
#
#   To C-preprocess using your default gcc compiler (1st on PATH), use:
#     make -f cpp_filter.mak socket.i
#
# This will produce socket.i from socket.c which you can inspect to see
# what the C-compiler is really given to compile.
#
# Requires Python 2+ and GNU-indent (unless USE_INDENT=0).
#
USE_INDENT   ?= 1
CPU_BITS     ?= 32
MSVC_CHECK   ?= 1
CLANG_CHECK  ?= 0
POCC_CHECK   ?= 0
DJGPP_CHECK  ?= 0
CYGWIN_CHECK ?= 0
PYTHON       ?= f:/ProgramFiler/Python36/python

WATT_ROOT := $(realpath $(WATT_ROOT))

THIS_FILE     = $(firstword $(MAKEFILE_LIST))
CPP_FILTER_PY = $(realpath $(dir $(THIS_FILE))cpp_filter.py)

ifeq ($(DJGPP_CHECK),1)
  ifeq ($(OS),Windows_NT)
    #
    # Windows hosted djgpp cross compiler. Get it from:
    #   https://github.com/andrewwutw/build-djgpp/releases
    #
    # Define an env-var 'DJ_PREFIX=f:/gv/djgpp/bin/i586-pc-msdosdjgpp'
    # Thus the full path to 'gcc' becomes:
    #   $(DJ_PREFIX)-gcc.exe
    #
    ifeq ($(DJ_PREFIX),)
      $(error Define a $(DJ_PREFIX) to point to the ROOT of the djgpp cross compiler.)
    endif

    ifeq ($(wildcard $(DJ_PREFIX)gcc.exe),)
      $(error Failed to find 'i586-pc-msdosdjgpp-gcc.exe'.)
    endif

    CC = $(DJ_PREFIX)gcc
  else
    CC = gcc
  endif

  CFLAGS = -O2 -Wall

else ifeq ($(MSVC_CHECK),1)
  CC     = cl
  CFLAGS = -nologo -W3 -D_WIN32_WINNT=0x0601

else ifeq ($(CLANG_CHECK),1)
  CC     = clang-cl
  CFLAGS = -nologo -Wall
  CL=
  export CL

else ifeq ($(POCC_CHECK),1)
  # USE_INDENT = 0
  CC     = pocc
  CFLAGS = -Go -X -Tx86-coff -D_M_IX86=1 -W2 \
           -I$(realpath $(PELLESC)\Include) -I$(realpath $(PELLESC)\Include\win)

else  # MinGW, CygWin
  CC     = gcc
  CFLAGS = -O2 -Wall

  ifeq ($(CYGWIN_CHECK),1)
    CFLAGS += -DWIN32 -D_WIN32
  endif
endif

CFLAGS += -DWATT32_BUILD -I$(WATT_ROOT)/inc -I$(WATT_ROOT)/src -I.

CFLAGS += -DW32_NAMESPACE= # -DWATT32_NO_NAMESPACE

#
# So this makefile can be used from ./Python too.
#
ifeq (0,1)
  CFLAGS += -I$(realpath $(PYTHONHOME))/include
endif

PREPROCESS_CMD = $(CC) -E $(CFLAGS) $< | $(PYTHON) $(CPP_FILTER_PY)

ifeq ($(USE_INDENT),1)
  PREPROCESS_CMD += | indent -st
endif

all: $(CPP_FILTER_PY) $(MAKECMDGOALS)

%.i: %.c FORCE $(CPP_FILTER_PY)
	$(PREPROCESS_CMD) > $@
	@echo ''
	@echo 'Look at "$@" for the preprosessed results.'

test:
	@echo 'I am  $$(THIS_FILE):     "$(THIS_FILE)".'
	@echo 'I am  $$(CPP_FILTER_PY): "$(CPP_FILTER_PY)".'
	@echo 'Goals $$(MAKECMDGOALS):  "$(MAKECMDGOALS)".'
	@echo '$$(CURDIR): "$(CURDIR)".'

FORCE:

#
# Create 'cpp_filter.py' in the directory of $(THIS_FILE)
#
$(CPP_FILTER_PY): $(THIS_FILE)
	@echo 'Generating $@...'
	$(file >  $@,#!/usr/env/python)
	$(file >> $@,# DO NOT EDIT! This file ($@) was generated automatically)
	$(file >> $@,# from $(realpath $(THIS_FILE)). Edit that file instead.)
	$(file >> $@,#)
	$(file >> $@,from __future__ import print_function)
	$(file >> $@,if 1:)
	$(file >> $@,$(_CPP_FILTER_PY))

define _CPP_FILTER_PY
  import sys, os

  try:
    import ntpath
  except ImportError as e:
    print ("Failed to import ntpath: %s" % e)
    sys.exit(1)

  def _win32_abspath (path):
    path = ntpath.abspath (path)
    return path.replace ('\\', '/')

  def skip_cwd (s1, s2):
    ''' Skip the leading part that is in common with s1 and s2
    '''
    i = 0
    while i < len(s1) and s1[i] == s2[i]:
       i += 1
    return s2[i:]

  #
  # Return long lines into multiple lines breaking at a space.
  # Add indent on 2nd and following lines.
  #
  def wrap_long_line (s, indent = " "):
    res = ''
    remaining = 0
    max_col = 120   # !to-do: figure out the screen width
    i = 0
    for word in s.split(" "):
      if remaining < len(word):
        if i > 0:
          res += "\n" + indent
        remaining = max_col - len(indent)
      res += word + " "
      remaining -= len(word) + 1
      i += 1
    return res

  ####################################################################

  cwd = _win32_abspath (os.getcwd()) + '/'

  last_line  = '??'
  last_fname = '??'
  empty_lines = 0

  while True:
    line = sys.stdin.readline()
    if not line:
      break
    if line.startswith('\n') or line.startswith('\r'):
      empty_lines += 1
      continue

    # print ("orig: \"%s\"" % line)

    line = line.replace ("\\\\", "/")
    fname = None
    quote = line.find ('\"')

    if line.startswith("#line ") and quote > 0:
      fname = _win32_abspath (line[quote:])
      last_fname = fname

    if line.strip() != '' and last_line != '':
      if fname is None or fname != last_fname:
        if line.find("__declspec(deprecated("):
          line = wrap_long_line (line)
        print (line, end="")

        if line.strip() == '}':  # Print a newline after a function
          print ("")

    last_line = line

  if empty_lines > 0:
    sys.stderr.write ("Removed %d empty lines." % empty_lines)

endef



