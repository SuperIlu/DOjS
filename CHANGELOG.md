# Version 1.5.0 (The neuronal necessity)  / April 2, 2021
* Added SQLite module
* Added module for training/running neuronal networks
* Added libcpuid module
* Added NanoSVG module
* Added global variables `Width`/`Height`
* Added single parameter version of `Color()`
* Added `Console` and `EvalChain` classes.
* Added perlin noise module
* You can now create single color Bitmaps with `new Bitmap(width, height, color)`
* FIXED: small fixes for raw disk access.
* FIXED: p5js `arc()` function is now somewhat useable (was completely broken before).
* FIXED: Updated openssl to 1.1.1k

# Version 1.4.0 (The curly conglomerate)  / Jan 22, 2021
* Replaced loadpng+libpng by alpng to reduce EXE size.
* Added fast integer array class `IntArray`
* Added `ReadInts()` and `WriteInts()` to `File`, `Zip` and `Socket` to speed up data IO.
* Improved type checking for parameters.
* Added cURL and OpenSSL support.
* FIXED: memory leak in editor
* FIXED: editor crashed when deleting selected text
* Added loadable module support during runtime
* IPX, PNG, Allegro3D, COMPort and cURL are loadable modules now

# Version 1.3.0 (The ZIPpy serving)  / Nov 13, 2020
* Added ZIP file access through `Zip()` class
* Added `StringToBytes()` and `BytesToString()`
* Added file name to stack trace for files loaded by `Include()` and `Require()`
* Added loading of images, fonts, samples, midi files and scripts from ZIP files
* `JSBOOT/` can now be replaced by `JSBOOT.ZIP`
* Added autostart feature for ZIP files
* Added more fonts from Linux Font Project
* Fixed crash in context help
* Added `ReadZIP()`, `Rename()`, `MakeDir()`, `RmDir()` and `RmFile()`
* Internal cleanup and removed some duplicate code
* Fixed bug in `DrawArray()` and new `Bitmap()`
* Fixed memory allocation bug where CWSDPMI and MuJS started to use virtual memory because of missing GCs.
* Added raw disk access through `RawRead()` and `RawWrite()` (and other functions)
* Fixed problems with the Makefile and filenames which only happened for fresh checkouts on case sensitive file systems.
* Updated build instructions
* Fixed `ReadBytes()` in `socket.c`
* Renamed `fxGetMemoryTMU()` to `fxGetMemoryTmu()`

# Version 1.2.1 (Bugfix)  / Oct 8, 2020
* Fixed crash in Voodoo detection which only happens on plain DOSBox.

# Version 1.2.0 (Watt is that?)  / Sept 5, 2020
* Added TCP/IP support through WATT32.
* Fixed bug where text mode was not restored after exiting DOjS
* Added `File.GetSize()`, `File.WriteBytes()` and `File.ReadBytes()`
* Added fixnewlines target to Makefile.
* Updated syntax highlighting file.
* Switched JSDoc documentation to better-docs
* Improved editor
	* Quoted strings are now GREEN
	* The cursor positioning at the end of line is more usable.
	* 50-line mode can now be selected on the command line
	* Fixed CTRL-L/Go to line
* Fixed bugs in p5js compatibility layer
	* `endShape()` polygon drawing
	* `lerpColor()`
* Cleaned up startup messages in logfile.
* Re-added PNG loading/writing

# Version 1.1.0 (the COM together)  / July 11, 2020
* Added COM port access through the `COMPort()` object.
* Added RAW LPT port access through LPT-funktions `LPTRawData()`, `LPTRawStatus()` and `LPTRawControl()`.
* Added LPT printer acces through `LPTReset()`, `LPTSend()` and `LPTStatus()`.
* Added LPT/COM port enumeration through `GetParallelPorts()` and `GetSerialPorts()`.
* Added more verbose output about detected 3dfx card and found ports when startig up.

# Version 1.0.1 (the minimal midi modification)  / Internal only
* Added `MidiGetPos()`
* Added example script that implements a simple MIDI player.
* Small cleanups for release building.

# Version 1.0.0 (the major mashup) / Mar 27, 2020
* Fixed memory leaks in TexInfo
* Added dynamic TexInfo generation from Bitmap
* Better memory handling between native and JS code (GC before malloc())
* Documented joystick functions
* Added RandomInt() to API
* Added compile time to startup message
* Added LFB (linear frame buffer) access to the 3dfx cards
* Added POST() function to display debug info using a ISA/PCI POST card
* SHIFT-F1 now opens the online help at the function uder the cursor (does not work for object methods)
* CTRL-DEL and CTRL-BS now delete whole words
* Updated behavior of CTRL-LEFT/CTRL-RIGHT to match the above
* Updated syntax highlighting with new functions and JS functions
* Added 'h' to command line options
* Updated transparency handling, transparency now is also available with 24bbp display modes
* Added System() to call external programs from DOjS.
* Fixed initialization order of Allegro sound card support
* Improved screen drawing code for editor
* You can now create Bitmaps from ARGB integer arrays via `new Bitmap(array, width, height)`
* You can now create Bitmaps from the current screen `new Bitmap(x, y, width, height)`
* You can now create Bitmaps from the current 3dfx screen `new Bitmap(x, y, width, height, buffer)`
* ARGB Arrays can be drawn directly using `DrawArray()`
* You can now save Bitmaps to BMP, PCX and TGA files.
* Optimized unneeded calls to Input() away.
* Added `createImage(w,h)` to p5js compatibility layer.
* Added `Sample.Get()` and `VoiceGetPosition()`, updated `Sample.Play()`.

# Version 0.9.9 (the voodoo vision) / Jan 26, 2020
* Fixed typo in the `GetXRotateMatrix()` functions.
* Sped up several 3D math function by implementing them in C (native JS functions still available by prefixing 'N').
* Added `Bitmap.DrawAdvanced()` for scaled bitmap drawing.
* Fixed CTRL-C handling on FreeDOS, now copy & paste works there as well.
* You can now pass command line parameters to your scripts.
* Added `String.startsWith()` and `String.endsWith()` from MDN.
* Added `HSBColor()` to native API.
* Fixed `SizeX()` and `SizeY()` after using `SetRenderBitmap()`
* Added joystick functions.
* Fixed "new sketch" template.
* Added 3Dfx/Glide support for Voodoo1/2/3/4/5/Banshee/Rush
* Improved error reporting from Allegro
* Added automatic fallback to 24bbp when 32bbp are not available
* Fixed the remote logviewer and some examples

# Version 0.9.8 (the expanded dimension) / Nov 17, 2019
* updated help.txt and README.md
* added sound input functions.
* finished 3D drawing functions, zbuffer functions and 3D math functions.
* You can now render to Bitmaps created by `new Bitmap(width, height)`
* `Color()` now also works with three arguments (alpha is 255 for missing value).
* Fixed missing documentation.

# Version 0.9.7 (the latest lament) / Oct 20, 2019
* fixed `rect()`.
* added `Bitmap.DrawTrans()` for 32bit TGA images w/ alpha.
* Added Cut, Copy & Paste to the editor.
* Added auto indent and backtab handling.
* Started working on allegro 3D fuctions (unfinished, untested)!

# Version 0.9.6 (the crashing creation) / May 26, 2019
* Fixed massive crash when using the builtin font in p5 compatibility layer
* Some API improvements
* Even more examples and a DOjS presentation done with DOjS
* Small fixes for `image()`, `arc()` and `text()` in p5 compatibility layer
* Enabled transparency for 24bpp and 32bpp mode (was only 32bpp before).
* Added useable alpha value to the return value of `GetPixel()`
* As always: more examples
* updated MuJS to latest version from http://git.ghostscript.com/?p=mujs.git;a=summary

# Version 0.9.5 (the changed creation) / May 2, 2019
* Replaced libmikmod, GRX and all sound code by Allegro 4.2.2.
* Transparency/RGBA now works.
* added `TransparencyEnabled()` to switch during runtime.
* Internal API changed quite much due to the switch to Allegro.
* p5js compatibility improved because of the better color handling.
* Added command line switch to disable transparency for speed reasons.
* PNGs are no longer supported, Allegro comes with PCX, TGA and BMP support only.
* Text printing lost a lot of its options due to the switch to Allegro.
* `SHIFT-F4` now clears the logfile before running the script.
* Improved internal debugging features.
* Splitted `func.c` into `gfx.c` and `func.c`.
* Implemented `push()` and `pop()` in p5js compatibility layer.
* Added a test-folder with scripts dedicated to test the API.
* Added `translate()`, `rotate()`, `shearX()`, `shearY()`, `resetMatrix()`, `scale()` and `applyMatrix()` in p5js compatibility layer.

# Version 0.9.1 (the fixed foreboding) / Apr 14, 2019
* Fixed libmikmod creating a WAV file when no soundcard is detected.
* Added logging to another machine using IPX. set `REMOTE_DEBUG=true` and start `jsboot/logview.js` on other machine.
* Added `GetPixel()` which returns the `Color` of an on-screen pixel.
* Added `Bitmap.GetPixel()` which returns the `Color` of an image pixel.
* Fixed/added some stuff in GRX.

# Version 0.9 (the fifth foreboding) / Apr 7, 2019
* Fixed `millis()` in p5js compatibility layer.
* Improved mouse display (still flickering, though).
* Fixed `MouseShowCursor(false)` (sort of).
* Added node discovery and address helpers to IPX module.
* Fixed `mousePressed()` and implemented `mouseDragged()`in p5js compatibility layer.
* Added HSL and HSB color mode to p5js compatibility layer.
* Added very simple syntax highlighting to the editor.
* Added `CustomLine()`, `CustomBox()`, etc to native API
* Added `strokeWeight()` to p5js compatibility layer.
* Renamed *tests/* to *examples/*
* Fixed *Makefile* on OSX.
* Switched to libmikmod for MOD and WAV sound output.

# Version 0.8 (the fourth forecoming) / Mar 27, 2019
* Tuned the compiler flags to pentium class CPUs
* Added command line option to turn off SoundBlaster code
* Added double buffering to video output to reduce the flickering
* Added more examples
* Added `MemoryInfo()`
* Changed `Print()` to `Println()`, added `Print()` w/o NEWLINE.
* Added `Include()` which loads a module into top level context.
* Added `Debug()` and `DEBUG` to support conditional debug output.
* Changed `Require()` to search modules in `jsboot/` and the current directory.
* `File.ReadByte()` now returns `null` at EOF.
* Improved documentation.
* Improved detection of missing SoundBlaster.
* Added Processing/p5js compatibility layer.
* Added some examples from the p5js example library.

# Version 0.7 (the third installment) / Mar 3, 2019
* Wrote a text editor for in-line script editing.
* Added inline-help and logfile viewer.
* Added frame rate limit and  `GetFramerate()` and `SetFramerate()`
* Added `Gc()` which can log to `JSLOG.TXT`
* Made command line override for SBlaster autodetection.
* Added font resizing.
* Added color mask functionality from GRX.
* Added NO_COLOR to predefined colors.
* Hopefully fixed Makefile dependency tracking
* Added PNG loading.
* Added command line parameters to select screen mode.
* Added PNG and BMP saving.
* Added `List()` and `Stat()` for directory traversal.
* Shortened key codes in `jsboot/func.js` and added `CharCode()`
* DOjS now has a mascot.
* Makefile now had ZIP target.
 
# Version 0.6 (the 2nd coming) / Feb 17, 2019
* Added CHANGELOG
* Now linking with DJGPP FPU emulation (`-lemu`)
* Added support for file reading/writing (see `Read()` and `File()`)
* Fixed Makefile.unix
* Reformated the whole code. I didn't notice my notebook had different settings in VSCode when releasing v0.5
* added `Read()` and `Require()`
* Finally some stack traces in the logfile in case of errors
* improved logfile output for detected hardware
* switched to new script format with `Setup()` and `Loop()`
* Added `MouseSetCursorMode()`
* Fixed object creation
* Added MIDI playing
* Added IPX networking

# Version 0.5 (first release) / Feb 11, 2019
* Initial release
* Binary release no longer available because of the old script format
