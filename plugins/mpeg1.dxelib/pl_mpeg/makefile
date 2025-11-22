# By default, this makefile builds 3 example applications for pl_mpeg
# - pl_mpeg_extract_frames: a command line tool that dumps all frames into BMPs
# - pl_mpeg_player_gl: a video player using SDL2 and OpenGL
# - pl_mpeg_player_sdl: a video player using SDL2 and it's built-in 2d renderer

# The players require the SDL2 library to be installed. Please note that the 
# linker flags for the GL version have only been tested on Linux. PRs welcome!

# To build individually:
# make extract
# make player_sdl
# make player_gl


CC ?= gcc
CFLAGS ?= -std=gnu99 -O3
LFLAGS ?= 

TARGET_EXTRACT ?= pl_mpeg_extract_frames
TARGET_SDL ?= pl_mpeg_player_sdl
TARGET_GL ?= pl_mpeg_player_gl

SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LFLAGS = $(shell sdl2-config --libs)


# Determine the right opengl libs for this platform
UNAME_S := $(shell uname -s)

# macOS
ifeq ($(UNAME_S), Darwin)
	GL_LFLAGS = -GLU -framework OpenGL

# Linux
else ifeq ($(UNAME_S), Linux)
	GL_LFLAGS = -lOpenGL -lGLEW

# Windows Msys
else ifeq ($(shell uname -o), Msys)
	GL_LFLAGS = -lopengl32 -lglew32
endif

all: $(TARGET_EXTRACT) $(TARGET_SDL) $(TARGET_GL)
extract: $(TARGET_EXTRACT)
player_sdl: $(TARGET_SDL)
player_gl: $(TARGET_GL)

$(TARGET_EXTRACT):pl_mpeg_extract_frames.c pl_mpeg.h
	$(CC) $(CFLAGS) pl_mpeg_extract_frames.c -o $(TARGET_EXTRACT) $(LFLAGS)

$(TARGET_SDL):pl_mpeg_player_sdl.c pl_mpeg.h
	$(CC) $(CFLAGS) $(SDL_CFLAGS) pl_mpeg_player_sdl.c -o $(TARGET_SDL) $(LFLAGS) $(SDL_LFLAGS)

$(TARGET_GL):pl_mpeg_player_gl.c pl_mpeg.h
	$(CC) $(CFLAGS) $(SDL_CFLAGS) pl_mpeg_player_gl.c -o $(TARGET_GL) $(LFLAGS) $(GL_LFLAGS) $(SDL_LFLAGS)

.PHONY: clean
clean:
	$(RM) $(TARGET_EXTRACT) $(TARGET_SDL) $(TARGET_GL)
