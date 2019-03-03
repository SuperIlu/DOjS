###
# Makefile for cross compiling DOjS for FreeDOS/MS-DOS
# All compilation was done with DJGPP 7.2.0 built from https://github.com/andrewwutw/build-djgpp
###
# enter the path to x-djgpp here
#DJGPP=/Users/iluvatar/tmp/djgpp/bin
DJGPP=/home/ilu/djgpp/bin

MUJS=mujs-1.0.5
GRX=grx249
ZLIB=zlib-1.2.11
PNG=lpng1636

INCLUDES=-I$(MUJS) -I$(GRX)/include -I$(GRX)/src/include -I$(GRX)/addons/bmp -I$(ZLIB) -I$(PNG)
LIBS=-lgrx20 -lpng -lz -lmujs -lm -lemu

CFLAGS=-MMD -Wall -pedantic -O2 $(INCLUDES) -DPLATFORM_MSDOS -DDEBUG_ENABLED
LDFLAGS=-L$(MUJS)/build/release -L$(GRX)/lib/dj2 -L$(ZLIB) -L$(PNG)

EXE=DOjS.EXE

FONTDIR=jsboot/fonts
BUILDDIR=build

CROSS_PLATFORM=i586-pc-msdosdjgpp-
CC=$(DJGPP)/$(CROSS_PLATFORM)gcc
AR=$(DJGPP)/$(CROSS_PLATFORM)ar
LD=$(DJGPP)/$(CROSS_PLATFORM)ld
STRIP=$(DJGPP)/$(CROSS_PLATFORM)strip
RANLIB=$(DJGPP)/$(CROSS_PLATFORM)ranlib
export

PARTS= \
	$(BUILDDIR)/DOjS.o \
	$(BUILDDIR)/funcs.o \
	$(BUILDDIR)/color.o \
	$(BUILDDIR)/bitmap.o \
	$(BUILDDIR)/sbdet.o \
	$(BUILDDIR)/fmmusic.o \
	$(BUILDDIR)/sbsound.o \
	$(BUILDDIR)/font.o \
	$(BUILDDIR)/file.o \
	$(BUILDDIR)/midiplay.o \
	$(BUILDDIR)/dosbuff.o \
	$(BUILDDIR)/ipx.o \
	$(BUILDDIR)/edit.o \
	$(BUILDDIR)/dialog.o \
	$(BUILDDIR)/lines.o \
	$(BUILDDIR)/util.o

all: init libmujs libgrx $(EXE)

libz: $(ZLIB)/msdos/libz.a

libpng: libz $(PNG)/scripts/libpng.a

libgrx: libpng $(GRX)/lib/unix/libgrx20.a

libmujs: $(MUJS)/build/release/libmujs.a

$(MUJS)/build/release/libmujs.a:
	$(MAKE) -C $(MUJS) build/release/libmujs.a

$(GRX)/lib/unix/libgrx20.a:
	PATH="$(PATH):$(DJGPP)" $(MAKE) -C $(GRX) -f makefile.dj2 libs
	mkdir -p $(FONTDIR)
	cp $(GRX)/fonts/*.fnt $(FONTDIR)

$(ZLIB)/msdos/libz.a:
	$(MAKE) -C $(ZLIB) -f Makefile.dojs

$(PNG)/scripts/libpng.a:
	$(MAKE) -C $(PNG) -f makefile.dojs libpng.a

$(EXE): $(PARTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $@

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

zip: all
	zip -9 -v -r DOJS.ZIP DOjS.EXE CWSDPMI.EXE LICENSE README.md CHANGELOG.md jsboot/ tests/

init:
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)/
	rm -f $(EXE) $(KEX) DOJS.ZIP

distclean: clean
	$(MAKE) -C $(MUJS) clean
	$(MAKE) -C $(GRX) cleanall -f makefile.dj2
	$(MAKE) -C $(ZLIB) -f Makefile.dojs clean
	$(MAKE) -C $(PNG) -f makefile.dojs clean
	rm -rf $(FONTDIR) TEST.TXT JSLOG.TXT

.PHONY: clean distclean init

DEPS := $(wildcard $(BUILDDIR)/*.d)
ifneq ($(DEPS),)
include $(DEPS)
endif
