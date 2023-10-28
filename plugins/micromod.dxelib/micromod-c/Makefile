
CC=gcc
CFLAGS=-ansi -pedantic -Wall -Og

all: micromod-sdl

clean:
	rm -f mod2wav micromod-sdl micromod-w32

mod2wav: mod2wav.c micromod.c micromod.h
	$(CC) $(CFLAGS) micromod.c mod2wav.c -o mod2wav

micromod-sdl: sdlplayer.c micromod.c micromod.h
	$(CC) $(CFLAGS) micromod.c sdlplayer.c -o micromod-sdl `sdl-config --cflags --libs`

micromod-w32: w32player.c micromod.c micromod.h
	$(CC) $(CFLAGS) micromod.c w32player.c -o micromod-w32 -lwinmm
