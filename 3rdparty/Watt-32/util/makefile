#
#  GNU make makefile for the MkMake/MkDep/MkLang/MkImp/bin32c
#  utilities used during Watt-32 developement
#
#  DOS versions:   Requires djgpp2 + SLang 1.3+ library installed.
#  Win32 versions: Requires MinGW + Slang 1.3+ installed.
#  Linux versions: Requires gcc + Slang 1.3+ installed.
#
#  Setup notes:
#    Review and setup SLANG_ROOT_.... to point to location
#    where your Slang version is installed
#

.SUFFIXES: .exe .l .y

#
# Change to suite:
#
USE_MSVC         ?= 1
SLANG_ROOT_DOS   ?= $(DJDIR)/contrib/slang.210
SLANG_LIB_DOS    ?= $(SLANG_ROOT_DOS)/src/djgobjs/libslang.a

SLANG_ROOT_WIN   ?= $(realpath $(MINGW32))/src/TUI/Slang
SLANG_LIB_WIN_CL ?= $(SLANG_ROOT_WIN)/src/wslang-$(CPU).lib

SLANG_CFLAGS = $(shell pkg-config --cflags slang)
SLANG_LIBS   = $(shell pkg-config --libs slang)

CC     = gcc
CFLAGS = -Wall -g -save-temps # -s

all: mkmake.exe mkdep.exe mklang.exe mkimp.exe dxegen.exe bin2c.exe

win32: $(addprefix win32/, mkmake.exe mkdep.exe mklang.exe bin2c.exe)

linux: $(addprefix linux/, mkmake mkdep mklang bin2c)

#
# DOS/djgpp binaries:
#
mkdep.exe: mkdep.c
	$(CC) $(CFLAGS) -o $@ $^

mklang.exe: mklang.c
	$(CC) $(CFLAGS) -o $@ $^

mkimp.exe: mkimp.c
	$(CC) $(CFLAGS) -o $@ $^

mkmake.exe: mkmake.c
	$(CC) $(CFLAGS) -I$(SLANG_ROOT_DOS)/src -o $@ $^ $(SLANG_LIB_DOS)

bin2c.exe: bin2c.c
	$(CC) $(CFLAGS) -o $@ $^

dxegen.exe: dxegen.c
	$(CC) $(CFLAGS) -o $@ $^

#
# Win32 binaries:
#
CL_CFLAGS  = -nologo -MD -Zi -DWATT32_STATIC -I../inc
CL_LDFLAGS = "-link -incremental:no -debug:none -ignore:4099"

ifeq ($(USE_MSVC),1)
  win32/mkmake.exe: mkmake.c
	cl $(CL_CFLAGS) -Fe./$@ -I$(SLANG_ROOT_WIN)/src $(SLANG_LIB_WIN_CL) $< $(CL_LDFLAGS)

  win32/mkdep.exe: mkdep.c
	cl $(CL_CFLAGS) -Fe./$@ $< $(CL_LDFLAGS)

  win32/mklang.exe: mklang.c
	cl $(CL_CFLAGS) -Fe./$@ $< $(CL_LDFLAGS)

  win32/bin2c.exe: bin2c.c
	cl $(CL_CFLAGS) -Fe./$@ $< $(CL_LDFLAGS)

else
  win32/mkmake.exe: mkmake.c
	$(CC) -m32 $(CFLAGS) $(SLANG_CFLAGS) -o $@ $< $(SLANG_LIBS)

  win32/mkdep.exe: mkdep.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

  win32/mklang.exe: mklang.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

  win32/bin2c.exe: bin2c.c
	$(CC) -m32 $(CFLAGS) -o $@ $<
endif

#
# Linux binaries:
#
linux/mkmake: mkmake.c
	$(CC) $(CFLAGS) $(SLANG_CFLAGS) -o $@ $< $(SLANG_LIBS)

linux/mkdep: mkdep.c
	$(CC) $(CFLAGS) -o $@ $^

linux/mklang: mklang.c
	$(CC) $(CFLAGS) -o $@ $^

linux/bin2c: bin2c.c
	$(CC) $(CFLAGS) -o $@ $^

mkimp.c: mkimp.l
	flex -o$@ $^

clean:
	@rm -f *.o mk*.exe dxegen.exe bin2c.exe \
               win32/mk*.exe win32/bin2c.exe \
               linux/mk* linux/bin2c \
               mkimp.c mkimp_gr.c

