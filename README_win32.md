# Win32 version
Since DOjS v1.13 it is possible to compile parts of DOjS for Win32 as well.
It will create a native Win32 binary based on Allegro 4.3.3.1 and without any of the hardware support available for DOS.
The EXE should run on Windows 98 or never.

The win32 version has the additional `-u` command line parameter which will switch DOjS to fullscreen when running.
See the [common README](README.md) for command line parameters and more information.

Please note the this feature is not thoroughly tested.
The following functionality should work:
- The editor
- All Allegro 2D graphics functions and the p5js emulation
- PCM/Midi sound (sound input is untested)
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

# Building
Just run `make -f Makefile.win32 zip` to create the distribution archive.

## Building mingw for i486 on Debian/Ubuntu
See [here](https://gist.github.com/SuperIlu/1f0ed930d907442c0fcb2566f7e63ae9) for the script I used to create my i486-w64-mingw32 toolchain.
The script was derived from [here](https://sourceforge.net/p/mingw-w64/wiki2/Build%20a%20native%20Windows%2064-bit%20gcc%20from%20Linux%20%28including%20cross-compiler%29/).
