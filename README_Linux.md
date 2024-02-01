# Linux version
Since DOjS v1.11 it is possible to compile parts of DOjS for Linux as well.
It will create a native Linux binary based on Allegro 4.3.3.1 and without any of the hardware support available for DOS.

The linux version has the additional `-u` command line parameter which will switch DOjS to fullscreen when running.
**Beware:** Keyboard input did not work for me on WLS2/Ubuntu when running in fullscreen mode.

Please note the this feature is not thoroughly tested.
The following functionality should work:
- The editor
- All Allegro 2D graphics functions and the p5js emulation
- PCM sound (no FM sound and no sound input!)
- cURL including HTTPS
- FileIO and ZIP files
- SQLite3
- JPG, PNG, WEBP and QOI
- Neural
- Noise
- MPEG1
- Nanosvg
- PDFGen
- Vorbis
- GIFAnim
- Sockets

Work in progress
- OpenGL (DOjS segfaults when exiting)

## Compilation
You need to install the following packages on a debian based system to compile DOjS for Linux
```
sudo apt-get install \
	libcurl4-openssl-dev \
	zlib1g-dev \
	libx11-dev \
	libxcursor-dev \
	libasound2-dev \
	build-essential \
	cmake \
	libpng-dev \
	freeglut3-dev \
	mesa-common-dev \
	libglfw3-dev \
	libxxf86vm-dev \
	libxpm-dev \
	libglew-dev
```

You should the be able to compile it with `make -f Makefile.linux`.
This will create `dojs`, `JSBOOT.ZIP` and download `cacerts.pem`.

Do not mix DOS and Linux builds! Always run `make distclean` using the appropriate Makefile before compiling for the other platform.
