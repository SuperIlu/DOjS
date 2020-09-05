#
# A GNU Makefile for _watt32.pyd - A simple Python 2.x module
# for Watt-32 library.
#
# Supports MSVC or MinGW (MinGW little tested).
# Needs SWIG and off-course Python to build and test.
#

define USAGE
  Usage: "$(MAKE) CC=[cl | gcc] [all | clean | vclean]"
  Specify CC=cl  - use MSVC
  Specify CC=gcc - use MinGW
endef

DEBUG_MODE = 0
STATIC     = 0

ifneq ($(PYTHON_INCLUDE_DIR),)
  PYTHON_ROOT = $(realpath $(PYTHON_INCLUDE_DIR)/..)
else
  PYTHON_ROOT = $(shell python -c "import sys; print(sys.prefix)")
endif

PYTHON_VER = $(shell $(PYTHON_ROOT)/python -c 'import os,sys; os.write(1,sys.version[0]); os.write(1,sys.version[2])')

ifeq ($(CC),cl)
  CFLAGS  = -nologo -Ox -EHsc -Zi -W3
  LDFLAGS = -nologo -debug -dll -map -verbose -incremental:no
  EX_LIBS = python$(PYTHON_VER).lib   # MUST be in %LIB-path

  ifeq ($(DEBUG_MODE),1)
    DEBUG   = _d
    CFLAGS += -MDd
  else
    CFLAGS += -MD
  endif

else ifeq ($(CC),gcc)
  CFLAGS =  -m32 -Wall
  LDFLAGS = -m32

  ifeq ($(DEBUG_MODE),1)
    CFLAGS += -O0 -ggdb
  else
    CFLAGS  += -O2
    LDFLAGS += -s
  endif

  LDFLAGS += -shared -Wl,--print-map,--sort-common,--cref
  EX_LIBS  = -lpython$(PYTHON_VER) # MUST be in %LIBRARY_PATH-path

else
  $(error $(USAGE))
endif

CFLAGS += # -DSWIG_LINK_RUNTIME -DSWIG_PYTHON_SILENT_MEMLEAK -DSWIGRUNTIME_DEBUG

CFLAGS += -DWATT32_BUILD -DWATT32_NO_NAMESPACE

CFLAGS += -I.. -I../../inc -I$(PYTHON_ROOT)/include

ifeq ($(STATIC),1)
  CFLAGS += -DWATT32_STATIC
  ifeq ($(CC),cl)
    EX_LIBS += ../../lib/x86/wattcpvc$(DEBUG).lib user32.lib advapi32.lib
  else
    EX_LIBS += ../../lib/x86/libwatt32.a -luser32 -ladvapi32
  endif
else
  ifeq ($(CC),cl)
    EX_LIBS += ../../lib/x86/wattcpvc_imp$(DEBUG).lib
  else
    EX_LIBS += ../../lib/x86/libwatt32.dll.a
  endif
endif

SWIG       = swig
SWIG_FLAGS = -Wall -python $(filter -D% -I%, $(CFLAGS)) # -nodefaultctor -nodefaultdtor # -debug-symbols

GENERATED = watt32_swig_wrap.c watt32.py

all: _watt32.pyd
	python ./runme.py -d www.google.com

test_stuff:
	@echo '$$(PYTHON_ROOT): $(PYTHON_ROOT).'
	@echo '$$(PYTHON_VER):  $(PYTHON_VER).'

install: _watt32.pyd watt32.py
	cp --update _watt32.pyd $(PYTHON_ROOT)/DLLs
	cp --update watt32.py   $(PYTHON_ROOT)/Lib/site-packages

uninstall:
	rm -f $(PYTHON_ROOT)/dlls/_watt32.pyd
	rm -f $(PYTHON_ROOT)/Lib/site-packages/watt32.py

ifeq ($(CC),cl)

_watt32.pyd: watt32_swig_wrap.obj _watt32.res
	link $(LDFLAGS) $< -out:$@ $(EX_LIBS) > link.tmp
	cat link.tmp >> _watt32.map
	rm -f link.tmp _watt32.lib _watt32.exp watt32.pyc

_watt32.res: _watt32.rc
	rc -nologo -DDEBUG=$(DEBUG_MODE) -I../../inc -D_MSC_VER -Fo $@ _watt32.rc

else

_watt32.pyd: watt32_swig_wrap.o _watt32.res
	$(CC) $(LDFLAGS) $< -o $@ $(EX_LIBS) > _watt32.map
	rm -f _watt32.pyc

_watt32.res: _watt32.rc
	windres -I../../inc -D__MINGW32__ -O COFF -o $@ _watt32.rc

endif

watt32.py: watt32_swig_wrap.c

watt32_swig_wrap.c: watt32_swig.i
	$(SWIG) $(SWIG_FLAGS) -o $@ watt32_swig.i

#
# The dependency must be updated by hand.
#
watt32_swig_wrap.c: ../wattcp.h ../sock_ini.h ../pcdbug.h \
                    ../pcdns.h  ../pctcp.h    ../misc.h

%.o: %.c
	$(CC) -DSWIG $(CFLAGS) -c $<
	@echo

%.obj: %.c
	$(CC) -DSWIG $(CFLAGS) -c $<
	@echo

clean:
	rm -f *.obj *.o *.pdb *.pyc *.res vc1*.pdb $(GENERATED)

vclean realclean: clean
	rm -f _watt32.pyd _watt32.res _watt32.exp _watt32.lib _watt32.map watt32.pyo
