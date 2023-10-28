
CC=gcc
CC_CROSS=i586-pc-msdosdjgpp-gcc
CFLAGS=-ansi -pedantic -Wall -Og

all: xm2wav

clean:
	rm -f xm2wav ibxm-sdl

xm2wav: xm2wav.c ibxm.c ibxm.h
	$(CC) $(CFLAGS) xm2wav.c ibxm.c -o xm2wav

dos_xm2wav: xm2wav.c ibxm.c ibxm.h
	$(CC_CROSS) $(CFLAGS) xm2wav.c ibxm.c -o xm2wav

ibxm-sdl: sdlplayer.c patterndisplay.c patterndisplay.h topaz8.c ibxm.c ibxm.h
	$(CC) $(CFLAGS) sdlplayer.c patterndisplay.c topaz8.c ibxm.c -o ibxm-sdl `sdl2-config --cflags --libs`
