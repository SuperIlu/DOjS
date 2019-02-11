###
# Makefile for cross compiling DOjS for FreeDOS/MS-DOS
# All compilation was done with DJGPP 7.2.0 built from https://github.com/andrewwutw/build-djgpp
###
# enter the path to x-djgpp here
DJGPP=/home/ilu/djgpp/bin

MUJS=mujs-1.0.5
GRX=grx249

INCLUDES=-I$(MUJS) -I$(GRX)/include -I$(GRX)/src/include -I$(GRX)/addons/bmp
LIBS=-lgrx20 -lmujs -lm

CFLAGS=-Wall -pedantic -O2 $(INCLUDES) -DPLATFORM_MSDOS -DDEBUG_ENABLED
LDFLAGS=-L$(MUJS)/build/release -L$(GRX)/lib/dj2

EXE=DOjS.exe

FONTDIR=jsboot/fonts

CROSS_PLATFORM=i586-pc-msdosdjgpp-
CC=$(DJGPP)/$(CROSS_PLATFORM)gcc
AR=$(DJGPP)/$(CROSS_PLATFORM)ar
LD=$(DJGPP)/$(CROSS_PLATFORM)ld
export

PARTS= \
	DOjS.o \
	funcs.o \
	color.o \
	bitmap.o \
	sbdet.o \
	fmmusic.o \
	sbsound.o \
	font.o

all: $(MUJS)/build/release/libmujs.a $(GRX)/lib/unix/libgrx20.a $(EXE)

$(MUJS)/build/release/libmujs.a:
	$(MAKE) -C $(MUJS) build/release/libmujs.a

$(GRX)/lib/unix/libgrx20.a:
	PATH="$(PATH):$(DJGPP)" $(MAKE) -C $(GRX) -f makefile.dj2 libs
	mkdir -p $(FONTDIR)
	cp $(GRX)/fonts/*.fnt $(FONTDIR)

$(EXE): $(PARTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f *.o
	rm -f $(EXE)

distclean: clean
	$(MAKE) -C $(MUJS) clean
	$(MAKE) -C $(GRX) cleanall -f makefile.dj2
	rm -rf $(FONTDIR)

.PHONY: clean distclean
