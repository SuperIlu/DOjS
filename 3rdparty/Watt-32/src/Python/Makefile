#
# A GNU Makefile for _watt32.pyd - A simple Python module
# for Watt-32 library.
#
# Supports MSVC, clang-cl or MinGW (MinGW little tested).
# Needs SWIG and off-course Python 2/3 to build and test.
#
define Usage

  Usage:
    make CC=[cl | clang-cl | gcc] <PYTHON="py -2.x" | "py -3.x"> <WATT32_STATIC=1> [all | clean | vclean]'
      CC=gcc      - use MinGW
      CC=cl       - use MSVC
      CC=clang-cl - use clang-cl
endef

#
# Using the Python Launcher argument 'PYTHON="py -x.y"', in a 'make py_info'
# invokations, things could look like this:
#
#   %WATT_ROOT%\src\Python> make CC=cl PYTHON="py -2" py_info
#   PY_ROOT:      F:/ProgramFiler/Python27.
#   PY_MAJOR_VER: 2.
#   PY_MINOR_VER: 7.
#
#   %WATT_ROOT%\src\Python> make CC=cl PYTHON="py -3.6" py_info
#   PY_ROOT:      f:/ProgramFiler/Python36.
#   PY_MAJOR_VER: 3.
#   PY_MINOR_VER: 6.
#
#   %WATT_ROOT%\src\Python> make CC=cl PYTHON="py -3.9" py_info
#   PY_ROOT:      f:/ProgramFiler/Python36.
#   PY_MAJOR_VER: 3.
#   PY_MINOR_VER: 9.
#
#   %WATT_ROOT%\src\Python> make CC=cl PYTHON="py -3" py_info
#   PY_ROOT:      F:/ProgramFiler/Python310.
#   PY_MAJOR_VER: 3.
#   PY_MINOR_VER: 10.
#
# A 'PYTHON="py -3"', reporting Python 3.9 just means that the default
# Python version 3 is '3.9'. Allthough one have a version 3.6 installed
# as non-default.
#

#
# This assumes you have a MSys/Cygwin's 'echo.exe' program with ANSI colour support.
#
BRIGHT_GREEN = \e[1;32m
BRIGHT_WHITE = \e[1;37m
green_msg    = @echo -e "$(BRIGHT_GREEN)$(1)\e[0m"

export CL=

#
# The 'CPU' MUST match the bitness of chosen 'PYTHON'.
# E.g. if you run 'make CC=cl PYTHON="py -2"' and your Python 2.x is 32-bit,
# call the 'vcvarsall.bat x86' first.
#
CPU ?= x86

#
# The only option at the moment:
#
WATT32_STATIC ?= 1

#
# Could be set in envronment of on the cmd-line:
#
PYTHON ?= py -3

PY_ROOT      = $(realpath  $(shell $(PYTHON) -c "import sys; print (sys.prefix)"))
PY_MAJOR_VER = $(shell $(PYTHON) -c "import sys; print(sys.version_info.major)" )
PY_MINOR_VER = $(shell $(PYTHON) -c "import sys; print(sys.version_info.minor)" )

ifeq ($(CC),cl)
  CFLAGS  = -nologo -MD -Ox -Zi -W3
  RCFLAGS = -nologo -D_MSC_VER
  LDFLAGS = -nologo -debug -dll -map -verbose -incremental:no -nodefaultlib:uuid.lib -libpath:$(PY_ROOT)/libs
  EX_LIBS = $(wildcard $(PY_ROOT)/libs/python$(PY_MAJOR_VER).lib)

else ifeq ($(CC),clang-cl)
  CFLAGS  = -nologo -MD -Ox -Zi -W3
  RCFLAGS = -nologo -D__clang__
  LDFLAGS = -nologo -debug -dll -map -verbose -incremental:no -nodefaultlib:uuid.lib -libpath:$(PY_ROOT)/libs
  EX_LIBS = $(wildcard $(PY_ROOT)/libs/python$(PY_MAJOR_VER).lib)

else ifeq ($(CC),gcc)
  CFLAGS =  -m32 -Wall -O2 -s
  RCFLAGS = -O COFF --target=pe-i386 -D__MINGW32__
  LDFLAGS = -m32 -s -shared -static-libgcc -Wl,--print-map
  EX_LIBS  = $(wildcard $(PY_ROOT)/libs/libpython$(PY_MAJOR_VER).a)

else
  $(error $(Usage))
endif

RCFLAGS += -I../../inc \
           -I$(PY_ROOT)/include

CFLAGS  += -I.. -I../../inc     \
           -I$(PY_ROOT)/include \
           -DPy_ENABLE_SHARED

# CFLAGS += -DSWIG_LINK_RUNTIME \
#           -DSWIGRUNTIME_DEBUG

#
# Without a '-DSWIG_PYTHON_SILENT_MEMLEAK', there is this exception:
#   TypeError: in method '_ping', argument 2 of type 'DWORD'
#   swig/python detected a memory leak of type 'DWORD *', no destructor found.
#
CFLAGS += -DSWIG_PYTHON_SILENT_MEMLEAK

ifneq ($(CC),cl)
  CFLAGS += -Wno-unused-function \
            -Wno-visibility
endif

ifeq ($(WATT32_STATIC),1)
  CFLAGS += -DWATT32_STATIC

  ifeq ($(CC),cl)
    EX_LIBS += ../../lib/$(CPU)/wattcpvc.lib
  else ifeq ($(CC),clang-cl)
    EX_LIBS += ../../lib/$(CPU)/wattcp_clang.lib
  else
    EX_LIBS += ../../lib/$(CPU)/libwatt32.a
  endif

  ifeq ($(CC),gcc)
    SYS_LIBS = -luser32 -ladvapi32
  else
    SYS_LIBS = user32.lib advapi32.lib
  endif

else
  CFLAGS  += -DWATT32_BUILD
  SYS_LIBS =

  ifeq ($(CC),cl)
    EX_LIBS += ../../lib/$(CPU)/wattcpvc_imp.lib
  else ifeq ($(CC),clang-cl)
    EX_LIBS += ../../lib/$(CPU)/wattcp_clang_imp.lib
  else
    EX_LIBS += ../../lib/$(CPU)/libwatt32.dll.a
  endif
endif

SWIG_FLAGS = -macroerrors -python $(filter -D% -I%, $(CFLAGS)) # -nodefaultctor -nodefaultdtor -debug-symbols
SWIG_FLAGS += -w451 -w454

GENERATED = watt32_swig_wrap.c watt32.py check-for-unused-libs.py

all: py_info _watt32.pyd epilogue

epilogue:
	$(call green_msg,Do a $(BRIGHT_WHITE)$(PYTHON) ./runme.py -d www.google.com $(BRIGHT_GREEN)to test it.)
	$(call green_msg,Do a $(BRIGHT_WHITE)make CC=$(CC) install $(BRIGHT_GREEN)to install it.)

py_info:
	$(call green_msg,Detected Python variables:)
	$(call green_msg,  PY_ROOT:      $(BRIGHT_WHITE)$(PY_ROOT).)
	$(call green_msg,  PY_MAJOR_VER: $(BRIGHT_WHITE)$(PY_MAJOR_VER).)
	$(call green_msg,  PY_MINOR_VER: $(BRIGHT_WHITE)$(PY_MINOR_VER).)

ifeq ($(CC),gcc)
  _watt32.pyd: watt32_swig_wrap.o _watt32.res $(EX_LIBS)
	$(call green_msg,Linking $@)
	$(CC) -o $@ $(LDFLAGS) $^ $(SYS_LIBS) > _watt32.map
	@echo

  _watt32.res: _watt32.rc
	windres -o $@ $(RCFLAGS) $<
	@echo

  install: _watt32.pyd watt32.py
	cp --update _watt32.pyd  $(PY_ROOT)/DLLs
	cp --update watt32.py    $(PY_ROOT)/Lib/site-packages

else
  _watt32.pyd: watt32_swig_wrap.obj _watt32.res  $(EX_LIBS) | check-for-unused-libs.py
	$(call green_msg,Linking $@)
	link $(LDFLAGS) $^ -out:$@ $(SYS_LIBS) > link.tmp
	@cat link.tmp >> _watt32.map
	@rm -f _watt32.{exp,lib}
	@$(PYTHON) check-for-unused-libs.py link.tmp

  _watt32.res: _watt32.rc
	rc -Fo $@ $(RCFLAGS) $<
	@echo

  install: _watt32.pyd watt32.py
	cp --update _watt32.{pyd,pdb} $(PY_ROOT)/DLLs
	cp --update watt32.py         $(PY_ROOT)/Lib/site-packages
endif

uninstall:
	rm -f $(PY_ROOT)/dlls/_watt32.{pyd,pdb}
	rm -f $(PY_ROOT)/Lib/site-packages/watt32.py

watt32.py: watt32_swig_wrap.c

watt32_swig_wrap.c: watt32_swig.i
	$(call green_msg,Generating Swig wrapper $@)
	swig $(SWIG_FLAGS) -o $@ watt32_swig.i
	@echo

#
# The dependency must be updated by hand.
#
watt32_swig_wrap.c: ../../inc/sys/w32api.h ../wattcp.h ../sock_ini.h ../pcdbug.h ../pcicmp.h \
                    ../pcdns.h ../pctcp.h ../misc.h ../misc.c

%.o: %.c
	$(CC) $(CFLAGS) -DSWIG -c $<
	@echo

%.obj: %.c
	$(CC) $(CFLAGS) -DSWIG -c $<
	@echo

clean:
	rm -f *.obj *.o *.pdb *.pyc *.res vc1*.pdb link.tmp $(GENERATED)
	rm -fr __pycache__

vclean realclean: clean
	rm -f _watt32.pyd _watt32.res _watt32.exp _watt32.lib _watt32.map watt32.pyo _watt32.pyc

check-for-unused-libs.py: Makefile
	$(call green_msg,Generating $@)
	$(file >  $@,#!/usr/env/python)
	$(file >> $@,if 1:)
	$(file >> $@,$(check_for_unused_libs_PY))

define check_for_unused_libs_PY
  #
  # Check a MSVC .map-file for lines after a 'Unused libraries:'
  #
  import os, sys

  map_file    = sys.argv[1]
  ignore_libs = [ ]

  class State():
    IDLE   = 0
    UNUSED = 1

  class Color():
    RED = WHITE = RESET = ""

  try:
    from colorama import init, Fore, Style
    init()
    Color.RED   = Fore.RED + Style.BRIGHT
    Color.WHITE = Fore.WHITE + Style.BRIGHT
    Color.RESET = Style.RESET_ALL
  except:
    pass

  def report (unused):
    num = len(unused)
    plural = [ "library", "libraries" ]
    if num > 0:
       print ("%s%d unused %s in %s:%s" % (Color.RED, num, plural[num > 1], map_file, Color.RESET))
       for u in sorted(unused):
           print ("  " + u)
    print ("%sDone%s\n" % (Color.WHITE, Color.RESET))

  def process_map (state):
    unused_libs = []
    f = open (map_file, "rt")
    lines = f.readlines()
    f.close()
    for l in lines:
      l = l.strip()
      if l == "Unused libraries:":
         state = State.UNUSED
         continue
      if state == State.UNUSED:
         if l == "":
            break
         if os.path.basename (l).lower() not in ignore_libs:
            unused_libs.append (l)
    return unused_libs

  report (process_map(State.IDLE))
endef

