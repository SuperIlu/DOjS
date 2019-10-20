###
# Makefile for cross compiling DOjS for FreeDOS/MS-DOS
# All compilation was done with DJGPP 7.2.0 built from https://github.com/andrewwutw/build-djgpp
###
# enter the path to x-djgpp here
#DJGPP=/Users/iluvatar/tmp/djgpp/bin
DJGPP=/home/ilu/djgpp/bin

MUJS=mujs-1.0.5
ALLEGRO=allegro-4.2.2-xc-master

INCLUDES=-I$(MUJS) -I$(ALLEGRO)/include
LIBS=-lalleg -lmujs -lm -lemu

CFLAGS=-MMD -Wall -Wmissing-prototypes -O2 -march=i386 -mtune=i586 -ffast-math $(INCLUDES) -DPLATFORM_MSDOS -fgnu89-inline #-DDEBUG_ENABLED
LDFLAGS=-L$(MUJS)/build/release -L$(ALLEGRO)/lib/djgpp

EXE=DOJS.EXE
ZIP=DOJS.ZIP

BUILDDIR=build

DOCDIR=doc/html

CROSS=$(DJGPP)/i586-pc-msdosdjgpp
CROSS_PLATFORM=i586-pc-msdosdjgpp-
CC=$(DJGPP)/$(CROSS_PLATFORM)gcc
AR=$(DJGPP)/$(CROSS_PLATFORM)ar
LD=$(DJGPP)/$(CROSS_PLATFORM)ld
STRIP=$(DJGPP)/$(CROSS_PLATFORM)strip
RANLIB=$(DJGPP)/$(CROSS_PLATFORM)ranlib
export

PARTS= \
	$(BUILDDIR)/dosbuff.o \
	$(BUILDDIR)/ipx.o \
	$(BUILDDIR)/edit.o \
	$(BUILDDIR)/dialog.o \
	$(BUILDDIR)/lines.o \
	$(BUILDDIR)/syntax.o \
	$(BUILDDIR)/file.o \
	$(BUILDDIR)/midiplay.o \
	$(BUILDDIR)/bitmap.o \
	$(BUILDDIR)/color.o \
	$(BUILDDIR)/font.o \
	$(BUILDDIR)/DOjS.o \
	$(BUILDDIR)/funcs.o \
	$(BUILDDIR)/gfx.o \
	$(BUILDDIR)/sound.o \
	$(BUILDDIR)/util.o \
	$(BUILDDIR)/a3d.o

all: init libmujs liballegro $(EXE)

libmujs: $(MUJS)/build/release/libmujs.a

liballegro: $(ALLEGRO)/lib/djgpp/liballeg.a

$(MUJS)/build/release/libmujs.a:
	$(MAKE) -C $(MUJS) build/release/libmujs.a

$(ALLEGRO)/lib/djgpp/liballeg.a:
	cd $(ALLEGRO) && ./xmake.sh lib

$(EXE): $(PARTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $@

$(BUILDDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

zip: all doc
	rm -f $(ZIP)
	zip -9 -v -r $(ZIP) $(EXE) CWSDPMI.EXE LICENSE README.md CHANGELOG.md jsboot/ examples/ $(DOCDIR)

doc:
	rm -rf $(DOCDIR)
	mkdir -p $(DOCDIR)
	cd doc && jsdoc -c jsdoc.conf.json -d ../$(DOCDIR)

init:
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)/
	rm -f $(EXE) DOjS.exe $(ZIP) JSLOG.TXT

distclean: clean
	cd $(ALLEGRO) && ./xmake.sh clean
	$(MAKE) -C $(MUJS) clean
	rm -rf $(DOCDIR) TEST.TXT JSLOG.TXT

.PHONY: clean distclean init doc

DEPS := $(wildcard $(BUILDDIR)/*.d)
ifneq ($(DEPS),)
include $(DEPS)
endif
