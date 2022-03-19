#
#  GNU make makefile for the MkMake/MkDep/MkLang/MkImp/bin32c
#  utilities used during Watt-32 developement
#
#  DOS versions:   Requires djgpp2 + SLang 1.3+ library installed.
#  Win32 versions: Requires MingW + Slang 1.3+ installed.
#  Linux versions: Requires gcc + Slang 1.3+ installed.

.SUFFIXES: .exe .l .y

#
# Change to suite:
#
SLANG_ROOT_DOS   ?= $(DJDIR)/contrib/slang.210
SLANG_LIB_DOS    ?= $(SLANG_ROOT_DOS)/src/djgobjs/libslang.a

SLANG_ROOT_WIN   ?= $(realpath $(MINGW32))/src/TUI/Slang
SLANG_LIB_WIN    ?= $(SLANG_ROOT_WIN)/src/gw32objs/libslang.a
SLANG_LIB_WIN_CL ?= $(SLANG_ROOT_WIN)/src/mw32objs/wslang32.lib

#
# Cross-compiling Watt-32 on Linux using '../src/configur.sh'.
# We assume S-Lang for Linux is installed in the correct place.
#
SLANG_ROOT_LINUX ?=
SLANG_LIB_LINUX  ?= -lslang

CC     = gcc
CFLAGS = -Wall -g # -s

all: mkmake.exe mkdep.exe mklang.exe mkimp.exe dxegen.exe bin2c.exe

win32: $(addprefix win32/, mkmake.exe mkdep.exe mklang.exe bin2c.exe)

linux: $(addprefix linux/, mkmake mkdep mklang bin2c)

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

mkimp.c: mkimp.l
	flex -o$@ $^

#
# Win32 binaries:
#
win32/mkmake.exe: mkmake.c
	$(CC) -m32 $(CFLAGS) -I$(SLANG_ROOT_WIN)/src -o $*.exe $^ $(SLANG_LIB_WIN)

win32/mkdep.exe: mkdep.c
	$(CC) -m32 $(CFLAGS) -o $*.exe $^

win32/mklang.exe: mklang.c
	$(CC) -m32 $(CFLAGS) -o $*.exe $^

win32/bin2c.exe: bin2c.c
	$(CC) -m32 $(CFLAGS) -o $*.exe $^

#
# MSVC version for testing:
#
msvc/mkmake.exe: mkmake.c
	cl -nologo -MD -Zi -I../inc -Fe$@ -I$(SLANG_ROOT_WIN)/src $^ $(SLANG_LIB_WIN_CL)

#
# Linux binaries:
#
linux/mkmake: mkmake.c
	$(CC) $(CFLAGS) -o $@ $^ $(SLANG_LIB_LINUX)

linux/mkdep: mkdep.c
	$(CC) $(CFLAGS) -o $@ $^

linux/mklang: mklang.c
	$(CC) $(CFLAGS) -o $@ $^

linux/bin2c: bin2c.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@rm -f *.o mk*.exe dxegen.exe bin2c.exe \
               win32/mk*.exe win32/bin2.exe \
               linux/mk*.exe linux/bin2.exe \
               mkimp.c mkimp_gr.c

