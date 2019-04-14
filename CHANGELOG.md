**Version 0.9.1 (the fixed foreboding)**
* Fixed libmikmod creating a WAV file when no soundcard is detected.
* Added logging to another machine using IPX. set `REMOTE_DEBUG=true` and start `jsboot/logview.js` on other machine.
* Added `GetPixel()` which returns the `Color` of an on-screen pixel.
* Added `Bitmap.GetPixel()` which returns the `Color` of an image pixel.
* Fixed/added some stuff in GRX.

**Version 0.9 (the fifth foreboding)**
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

**Version 0.8 (the fourth forecoming)**
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

**Version 0.7 (the third installment)**
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

**Version 0.6 (the 2nd coming)**
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

**Version 0.5 (first release)**
* Initial release
* Binary release no longer available because of the old script format
