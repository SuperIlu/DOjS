# DOjS
## A DOS JavaScript Canvas with sound.
DOjS is a JavaScript-able canvas with WAV, MIDI and FM sound support for systems running MS-DOS, [FreeDOS](http://freedos.org/) or any DOS based Windows (like 95, 98, ME).
It was inspired by [Processing](https://processing.org/) which is described on [Wikipedia](https://en.wikipedia.org/wiki/Processing_(programming_language)) as:

> Processing is an open-source graphical library and integrated development environment (IDE) / playground built for the electronic arts, new media art, and visual design communities with the purpose of teaching non-programmers the fundamentals of computer programming in a visual context.

You can type in a script with the builtin or your favorite text editor and then run it on a DOS command prompt.

DOjS is pronounces like [doge](https://en.wikipedia.org/wiki/Doge_(meme)), but ending with an "s".
![DOjS logo](/tests/DOjS.png)

DOjS was only possible due to the work of these people/projects:
* [MuJS](https://mujs.com/) JavaScript interpreter
* The [GRX graphics library](http://grx.gnu.de/)
* The SoundBlaster example code from [Steven Don](http://www.shdon.com/dos/sound)
* [DJGPP](http://www.delorie.com/djgpp/) from DJ Delorie and the [Linux compile scripts](https://github.com/andrewwutw/build-djgpp) by Andrew Wu.

You can find me on [Twitter](https://twitter.com/dec_hl) if you want...

# Compilation
## Preparation
Setup Windows Subsystem for Linux (WSL) according to [this](https://docs.microsoft.com/en-us/windows/wsl/install-win10) guide (I used Ubuntu 18.04 LTS).

Build and install DJGPP 7.2.0 according to [this](https://github.com/andrewwutw/build-djgpp) guide.
I used the following command lines to update/install my dependencies:
```bash
sudo apt-get update
sudo apt-get dist-upgrade
sudo apt-get install bison flex curl gcc g++ make texinfo zlib1g-dev g++ unzip htop screen git bash-completion build-essential
```

And the following commands to build and install DJGPP to `/home/ilu/djgpp`.:
```bash
git clone https://github.com/andrewwutw/build-djgpp.git
cd build-djgpp
export DJGPP_PREFIX=/home/ilu/djgpp
./build-djgpp.sh 7.2.0
```

## Getting & Compiling DOjS
Open a PowerShell command line in the directory where you want the source to reside.
Run WLS to start the Linux Subsystem.

Checkout DOjS from Github:
```bash
git clone https://github.com/SuperIlu/DOjS.git
```

Open the Makefile in a text editor and change the path to DJGPP according to your installation.

Now you are ready to compile DOjS with `make clean all`. This might take some time as the dependencies are quite a large.
`make distclean` will clean dependencies as well.

## A minimal script
You can find the following example in `tests/exampl.js`:
```javascript
/*
** This function is called once when the script is started.
*/
function Setup() {
    str = "";
    pink = new Color(241, 66, 244); // define the color pink
    SetFramerate(30);
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
    ClearScreen(EGA.BLACK);
    TextXY(SizeX() / 2, SizeY() / 2, "Hello World!", pink, NO_COLOR);

    TextXY(2, SizeY() / 2 + 20, str, EGA.WHITE, NO_COLOR);

    TextXY(10, 10, "rate=" + GetFramerate(), EGA.BLACK, EGA.LIGHT_BLUE);
}

/*
** This function is called on any input.
*/
function Input(event) {
    str = JSON.stringify(event);
}
```
Open this script with `DOjS.EXE tests\exampl.js` or use `DOjS.EXE -r tests\exampl.js` to run it without starting the integrated editor first. If the script does not exist the editor loads the template for a new script.

# Known bugs/limitations
* SoundBlaster IRQ detection only works for IRQs 3, 5 and 7. I could not figure out why the system locks up when 2, 10 and 11 are tried as well.
* There seems to be a bug in the FM sound system. Sounds created by DOjS sound different to the ones created by Steven Dons original code.
* Some (nice) functions from GRX are still missing.
* WAV files need to be 11025Hz, 8bit, mono. Different sample rates produce a warning in the logfile (and sound weird). Different formats are outright refused.
* BMPs must be 2, 4, 8 bpp.
* `MouseShowCursor(false)` does not work
* ~~The Makefile does not honor the include files for dependencies and is a mess.~~
* ~~The JavaScript API is not stable yet. The current naming scheme is inconsistent to say the least...~~
* ~~All script-code has to be in a single file, no `include()` function yet.~~
* ~~Error handling is a nightmare. So far I could neither find a way to display a propper stack trace nor the offending line!~~
~~* Scripts are responsible to provide a way to exit DOjS, no general exit mechanism like with Processing is implemented yet.~~

# Planed work
* Fix bugs!
* Add more GRX functions to JavaScript API.
* split up `func.c` into `grx.c` and `func.c`.
* Improve sound loading/output.
* Add libmikmod?
* Add double buffering?
* Improve help viewer.
* ~~Add text-file reading/writing.~~
* ~~Add including other JS files.~~
* ~~Straighten/cleanup the JavaScript API.~~
* ~~Rework the way scripts are started to the `setup()/loop()` solution known from Processing/Arduino.~~
* ~~Include an in-system text editor like [KeXted](https://github.com/abelidze/KeXted) to change the scripts.~~
* ~~Make scripts restartable.~~
* ~~Include PNG loading.~~
* ~~Add IXP network code.~~

# Licenses
## DOjS
All code from me is released under **MIT license**.

## MuJS
MuJS is released under **ISC license**. See *COPYING* in the MuJS folder for details.

## GRX
GRX itself is released under **LGPL**, the fonts are under **MIT and other licenses**. See *copying.grx* in the grx folder for details.

## SoundBlaster MIDI, DMA, FM sound and SoundBlaster detection code
This code (modified by me) is (c) bei [Steven Don](http://www.shdon.com/). It is licensed under [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/) with kind permission of Steven. `FM.DAT` is released under [WTFPL](http://www.wtfpl.net/).

## IPX and dosbuffer sub-system
This code is taken from the game [Cylindrix](https://www.allegro.cc/depot/Cylindrix) by Hotwarez LLC, Goldtree Enterprises.
It was [released](https://sourceforge.net/projects/cylindrix/) under **GPLv2**.


## CWSDPMI.EXE
[CWSDPMI](http://sandmann.dotster.com/cwsdpmi/) DPMI host is licensed under **GPL**. The documentation states:
> The files in this binary distribution may be redistributed under the GPL (with source) or without the source code provided.

## Logo
The DOjS logo dog was downloaded from [Pexels](https://www.pexels.com/photo/animal-cachorro-doge-pug-1753686/) and kindly provided by Iago Garcia Garcia.

The logo font is [Comic relief](https://www.fontsquirrel.com/fonts/comic-relief) by Jeff Davis provided under **SIL OPEN FONT LICENSE Version 1.1**.

## WAVs
All WAV files were downloaded from [BigSoundBank](https://bigsoundbank.com/) and are licensed under [WTFPL](https://bigsoundbank.com/droit.html)
* https://bigsoundbank.com/detail-0283-song-of-rooster.html
* https://bigsoundbank.com/detail-1102-bell-bike-5.html
* https://bigsoundbank.com/detail-1023-explosion-far-away.html

## MIDs
The MIDI files were downloaded from the [FreeDOOM](https://github.com/freedoom/freedoom) project and are licensed under [this](https://github.com/freedoom/freedoom/blob/master/COPYING.adoc) license.

## libpng
[libpng](http://www.libpng.org/pub/png/libpng.html) is released under [PNG Reference Library License version 2](http://www.libpng.org/pub/png/src/libpng-LICENSE.txt)

## zlib
[zlib](http://www.zlib.net/) is released under [zlib license](http://www.zlib.net/zlib_license.html).


# Usage
## Command line
```
    Usage: DOjS.EXE [-r] [-s <p>:<i>:<d>] <script>
        -r             : Do not invoke the editor, just run the script.
        -w <width>     : Screen width: 320 or 640, Default: 640.
        -b <bbp>       : Bit per pixel:8, 16, 24, 32. Default: 24.
        -s <p>:<i>:<d> : Override sound card auto detection with given values.
                         p := Port (220h - 280h).
                         i := IRQ  (2 - 11).
                         d := DMA  (0 - 7).
                         Example: -s 220:5:1
```

## Editor keys
```
    F1  : Open/Close help
    F3  : Save script
    F4  : Run script
    F7  : Find text
    F9  : Show/Close logfile
    F10 : Quit

    Shift-F7   : Find again
    CTRL-D     : Delete current line
    CTRL-LEFT  : Previous word
    CTRL-RIGHT : Next word
    PAGE-UP    : One page up.
    PAGE-DOWN  : One page down.
    HOME       : Go to start of line
    END        : Go to end of line
    CTRL-HOME  : Go to start of line
    CTRL-END   : Go to end of line

    TAB size is 4.
    The help viewer will remember the current position.

    The logfile can be truncated by pressing DEL in the log viewer.
```

# JS API description
A very minimal API documentation.

* [JS API description](#js-api-description)
	* [Script format](#script-format)
		* [Setup()](#setup)
		* [Loop()](#loop)
		* [Input(event: {x:number, y:number, flags:number, buttons:number, key:number, kbstat:number, dtime:number})](#inputevent-xnumber-ynumber-flagsnumber-buttonsnumber-keynumber-kbstatnumber-dtimenumber)
	* [File](#file)
		* [f = new File(filename:string, mode:string)](#f-new-filefilenamestring-modestring)
		* [f.ReadByte():number](#freadbytenumber)
		* [f.WriteByte(ch:number)](#fwritebytechnumber)
		* [f.ReadLine():string](#freadlinestring)
		* [f.WriteLine(txt:string)](#fwritelinetxtstring)
		* [f.WriteString(txt:string)](#fwritestringtxtstring)
		* [f.Close()](#fclose)
	* [IPX networking](#ipx-networking)
		* [IpxSocketOpen(num:number)](#ipxsocketopennumnumber)
		* [IpxSocketClose()](#ipxsocketclose)
		* [IpxSendPacket(data:string, dest:array[6:number])](#ipxsendpacketdatastring-destarray6number)
		* [IpxCheckPacket():boolean](#ipxcheckpacketboolean)
		* [IpxGetPacket()](#ipxgetpacket)
		* [IpxGetLocalAddress():array[6:number]](#ipxgetlocaladdressarray6number)
	* [Color](#color)
		* [c = new Color(red:number, green:number, blue:number[, mask:number])](#c-new-colorrednumber-greennumber-bluenumber-masknumber)
		* [c.value](#cvalue)
		* [c.GetRed()](#cgetred)
		* [c.GetGreen()](#cgetgreen)
		* [c.GetBlue()](#cgetblue)
	* [Bitmap](#bitmap)
		* [bm = new Bitmap(filename:string)](#bm-new-bitmapfilenamestring)
		* [bm.filename](#bmfilename)
		* [bm.width](#bmwidth)
		* [bm.height](#bmheight)
		* [bm.Draw(x:number, y:number)](#bmdrawxnumber-ynumber)
	* [Font](#font)
		* [f = new Font(filename:string)](#f-new-fontfilenamestring)
		* [f.filename](#ffilename)
		* [f.minwidth](#fminwidth)
		* [f.maxwidth](#fmaxwidth)
		* [f.DrawString(x:number, y:number, text:string, foreground:Color, background: Color, direction:number, alignX:number, alignY:number)](#fdrawstringxnumber-ynumber-textstring-foregroundcolor-background-color-directionnumber-alignxnumber-alignynumber)
		* [f.StringWidth(text:string):number](#fstringwidthtextstringnumber)
		* [f.StringHeight(text:string):number](#fstringheighttextstringnumber)
		* [f.Resize(w:number, h:number)](#fresizewnumber-hnumber)
	* [Midi](#midi)
		* [mid = new Midi(filename:string)](#mid-new-midifilenamestring)
		* [mid.Play()](#midplay)
		* [MidiIsPlaying():boolean](#midiisplayingboolean)
		* [MidiStop()](#midistop)
	* [Sound](#sound)
		* [snd = new Sound(filename:string)](#snd-new-soundfilenamestring)
		* [snd.filename](#sndfilename)
		* [snd.length](#sndlength)
		* [snd.format](#sndformat)
		* [snd.channels](#sndchannels)
		* [snd.rate](#sndrate)
		* [snd.num_bits](#sndnum_bits)
		* [snd.Play()](#sndplay)
	* [FM Music](#fm-music)
		* [SetInstrument(voice:number, instrument:object)](#setinstrumentvoicenumber-instrumentobject)
		* [NoteOn(voice:number, note:number, octave: number)](#noteonvoicenumber-notenumber-octave-number)
		* [NoteOff(voice:number, note:number, octave: number)](#noteoffvoicenumber-notenumber-octave-number)
	* [GRX Drawing functions](#grx-drawing-functions)
		* [NumColors():number](#numcolorsnumber)
		* [NumFreeColors():number](#numfreecolorsnumber)
		* [SizeX():number](#sizexnumber)
		* [SizeY():number](#sizeynumber)
		* [MaxX():number](#maxxnumber)
		* [MaxY():number](#maxynumber)
		* [ClearScreen(c:Color)](#clearscreenccolor)
		* [Plot(x:number, y:number, c:Color)](#plotxnumber-ynumber-ccolor)
		* [Line(x1:number, y1:number, x2:number, y2:number, c:Color)](#linex1number-y1number-x2number-y2number-ccolor)
		* [Box(x1:number, y1:number, x2:number, y2:number, c:Color)](#boxx1number-y1number-x2number-y2number-ccolor)
		* [Circle(x1:number, y1:number, r:number, c:Color)](#circlex1number-y1number-rnumber-ccolor)
		* [Ellipse(xc:number, yc:number, xa:number, ya:number, c:Color)](#ellipsexcnumber-ycnumber-xanumber-yanumber-ccolor)
		* [CircleArc(x:number, y:number, r:number, start:number, end:number, style:number, c:Color)](#circlearcxnumber-ynumber-rnumber-startnumber-endnumber-stylenumber-ccolor)
		* [EllipseArc(xc:number, yc:number, xa:number, start:number, end:number, style:number, c:Color)](#ellipsearcxcnumber-ycnumber-xanumber-startnumber-endnumber-stylenumber-ccolor)
		* [FilledBox(x1:number, y1:number, x2:number, y2:number, c:Color)](#filledboxx1number-y1number-x2number-y2number-ccolor)
		* [FramedBox(x1:number, y1:number, x2:number, y2:number, [intcolor:Color, topcolor:Color, rightcolor:Color, bottomcolor:Color, leftcolor:Color])](#framedboxx1number-y1number-x2number-y2number-intcolorcolor-topcolorcolor-rightcolorcolor-bottomcolorcolor-leftcolorcolor)
		* [FilledCircle(x1:number, y1:number, r:number, c:Color)](#filledcirclex1number-y1number-rnumber-ccolor)
		* [FilledEllipse(xc:number, yc:number, xa:number, ya:number, c:Color)](#filledellipsexcnumber-ycnumber-xanumber-yanumber-ccolor)
		* [FilledCircleArc(x:number, y:number, r:number, start:number, end:number, style:number, c:Color)](#filledcirclearcxnumber-ynumber-rnumber-startnumber-endnumber-stylenumber-ccolor)
		* [FilledEllipseArc(xc:number, yc:number, xa:number, start:number, end:number, style:number, c:Color)](#filledellipsearcxcnumber-ycnumber-xanumber-startnumber-endnumber-stylenumber-ccolor)
		* [FloodFill(x:number, y:number, border:Color, c:Color)](#floodfillxnumber-ynumber-bordercolor-ccolor)
		* [FloodSpill(x1:number, y1:number, x2:number, y2:number, border:Color, c:Color)](#floodspillx1number-y1number-x2number-y2number-bordercolor-ccolor)
		* [FloodSpill2(x1:number, y1:number, x2:number, y2:number, old1:Color, new1:Color, old2:Color, new2:Color)](#floodspill2x1number-y1number-x2number-y2number-old1color-new1color-old2color-new2color)
		* [PolyLine(c:Color, [[x1, x2], [..], [xN, yN]])](#polylineccolor-x1-x2-xn-yn)
		* [Polygon(c:Color, [[x1, x2], [..], [xN, yN]])](#polygonccolor-x1-x2-xn-yn)
		* [FilledPolygon(c:Color, [[x1, x2], [..], [xN, yN]])](#filledpolygonccolor-x1-x2-xn-yn)
		* [FilledConvexPolygon(c:Color, [[x1, x2], [..], [xN, yN]])](#filledconvexpolygonccolor-x1-x2-xn-yn)
		* [MouseSetSpeed(spmul:number, spdiv:number)](#mousesetspeedspmulnumber-spdivnumber)
		* [MouseSetAccel(thresh:number, accel:number)](#mousesetaccelthreshnumber-accelnumber)
		* [MouseSetLimits(x1:number, y1:number, x2:number, y2:number)](#mousesetlimitsx1number-y1number-x2number-y2number)
		* [MouseGetLimits()](#mousegetlimits)
		* [MouseWarp(x:number, y:number)](#mousewarpxnumber-ynumber)
		* [MouseShowCursor(b:boolean)](#mouseshowcursorbboolean)
		* [MouseCursorIsDisplayed():boolean](#mousecursorisdisplayedboolean)
		* [MouseSetColors(fg:Color, bg:Color)](#mousesetcolorsfgcolor-bgcolor)
	* [MouseSetCursorMode(mode:int, ...)](#mousesetcursormodemodeint)
		* [TextXY(x:number, y:number, text:string, fg:Color, bg:Color)](#textxyxnumber-ynumber-textstring-fgcolor-bgcolor)
		* [SaveBmpImage(fname:string)](#savebmpimagefnamestring)
		* [SavePngImage(fname:string)](#savepngimagefnamestring)
	* [Other functions/properties](#other-functionsproperties)
		* [SOUND_AVAILABLE: boolean](#sound_available-boolean)
		* [SYNTH_AVAILABLE: boolean](#synth_available-boolean)
		* [MOUSE_AVAILABLE: boolean](#mouse_available-boolean)
		* [MIDI_AVAILABLE: boolean](#midi_available-boolean)
		* [Print(a, ...)](#printa)
		* [Stop()](#stop)
		* [Sleep(ms:number)](#sleepmsnumber)
		* [Read(filename:string):string](#readfilenamestringstring)
		* [List(dname:string):[f1:string, f1:string, ...]](#listdnamestringf1string-f1string)
		* [Stat(name:string)](#statnamestring)
		* [Require(filename:string):module](#requirefilenamestringmodule)
		* [Gc(info:boolean)](#gcinfoboolean)
		* [SetFramerate(rate:number)](#setframerateratenumber)
		* [GetFramerate():number](#getframeratenumber)
		* [SetExitKey(key:number)](#setexitkeykeynumber)

## Script format
Scripts need to provide three functions: `Setup()`, `Loop()` and `Input()`. Scripts are loaded and executed top-own. After that `Setup()` is called once and then `Loop()` repeatedly. `Input()` is called whenever mouse of keyboard input happens.

### Setup()
This function is called once at startup. It can initialize variables, setup hardware, etc.

### Loop()
This function is called after setup repeatedly until `Stop()` is called. After calling `Stop()` the program ends when `Loop()` exits.

### Input(event: {x:number, y:number, flags:number, buttons:number, key:number, kbstat:number, dtime:number})
This function is called whenever mouse/keyboard input happens. The parameter is an event object with the following fields: `{x:number, y:number, flags:number, buttons:number, key:number, kbstat:number, dtime:number}`. The definitions for the `flags` and `key` field can be found in `jsboot/func.js`.

## File
### f = new File(filename:string, mode:string)
Open a file, for file modes see `jsboot/file.js`. Files can only either be read or written, never both. Writing to a closed file throws an exception.

### f.ReadByte():number
Read a single byte from file and return it as number.

### f.WriteByte(ch:number)
Write a single byte to a file.

### f.ReadLine():string
Read a line of text from file. The maximum line length is 4096 byte.

### f.WriteLine(txt:string)
Write a NEWLINE terminated string to a file.

### f.WriteString(txt:string)
Write a string to a file.

### f.Close()
Close the file.

## IPX networking
DOjS supports IPX networking. Node addresses are arrays of 6 numbers between 0-255. Default socket number and broadcast address definitions can be found in `jsboot/ipx.js`.

### IpxSocketOpen(num:number)
Open an IPX socket.

### IpxSocketClose()
Close IPX socket (if any).

### IpxSendPacket(data:string, dest:array[6:number])
Send packet via IPX. Max length 79 byte.

### IpxCheckPacket():boolean
Check for packet in receive buffer.

### IpxGetPacket():{data:string, source:array[6:number]}
Get packet from receive buffer (or NULL).

### IpxGetLocalAddress():array[6:number]
Get the local address.

## Color
See `jsboot/color.js` for predefined EGA colors. Use `NO_COLOR` for the transparent color.

### c = new Color(red:number, green:number, blue:number[, mask:number])
Create a RGB color with optional mask (see `jsboot/color.js`).

### c.value
24bit value with the actual color

### c.GetRed()
get the red part of a color.

### c.GetGreen()
get the green part of a color.

### c.GetBlue()
get the blue part of a color.

## Bitmap
### bm = new Bitmap(filename:string)
Load a BMP or PNG image.

### bm.filename
Name of the file.

### bm.width
Width in pixels

### bm.height
Height in pixels

### bm.Draw(x:number, y:number)
Draw the image to the canvas at given coordinates.

## Font
See `jsboot/font.js` for direction and alignment values.

### f = new Font(filename:string)
Load a .FNT file for GRX.

### f.filename
Name of the FNT file.

### f.minwidth
Width of smallest character

### f.maxwidth
Width of widest character

### f.DrawString(x:number, y:number, text:string, foreground:Color, background: Color, direction:number, alignX:number, alignY:number)
Draw a string to the canvas.

### f.StringWidth(text:string):number
Calculate string width for this font.

### f.StringHeight(text:string):number
Calculate string height for this font.

### f.Resize(w:number, h:number)
Resize font to new width/height.

## Midi
### mid = new Midi(filename:string)
Load a midi file.

### mid.Play()
Play the midi file.

### MidiIsPlaying():boolean
Check if the file is still playing

### MidiStop()
Stop playing midi.

## Sound
### snd = new Sound(filename:string)
Load a WAV (11025Hz, 8bit, mono). Different sample rates are accepted (and create a warning in the log), but are not converted and will sound weird.

### snd.filename
Name of the WAV

### snd.length
Sound length.

### snd.format
Sound format.

### snd.channels
Number of channels.

### snd.rate
Sampling rate.

### snd.num_bits
Number of bits/sample.

### snd.Play()
Play the WAV once.

## FM Music
See `jsboot/fmmusic.js` for pre-defined instruments and notes.

### SetInstrument(voice:number, instrument:object)
Convert the contents of a JS object into an instrument and set the parameters to the given voice.

### NoteOn(voice:number, note:number, octave: number)
Start playing a note.

### NoteOff(voice:number, note:number, octave: number)
Stop playing a note.

## GRX Drawing functions
Please take a look a the GRX [documentation](http://grx.gnu.de/grx249um.html) for details of the drawing functions.
See `jsboot/func.js` for constants.

### NumColors():number
Get number of possible colors.

### NumFreeColors():number
Get number of remaining free colors.

### SizeX():number
get the width of the drawing area.

### SizeY():number
get the height of the drawing area.

### MaxX():number
get the max X coordinate on the drawing area.

### MaxY():number
get the max Y coordinate on the drawing area.

### ClearScreen(c:Color)
clear the screen with given color.

### Plot(x:number, y:number, c:Color)
draw a point.

### Line(x1:number, y1:number, x2:number, y2:number, c:Color)
draw a line.

### Box(x1:number, y1:number, x2:number, y2:number, c:Color)
draw a box.

### Circle(x1:number, y1:number, r:number, c:Color)
draw a circle.

### Ellipse(xc:number, yc:number, xa:number, ya:number, c:Color)
draw an ellipse.

### CircleArc(x:number, y:number, r:number, start:number, end:number, style:number, c:Color):{"centerX":XXX,"centerY":XXX,"endX":XXX,"endY":XXX,"startX":XXX,"startY":XXX}
Draw a circle arc. Returns an object with coordinates of the drawn arc: `{"centerX":XXX,"centerY":XXX,"endX":XXX,"endY":XXX,"startX":XXX,"startY":XXX}`.

### EllipseArc(xc:number, yc:number, xa:number, start:number, end:number, style:number, c:Color):{"centerX":XXX,"centerY":XXX,"endX":XXX,"endY":XXX,"startX":XXX,"startY":XXX}
Draw an ellipse arc. Returns an object with coordinates of the drawn arc: `{"centerX":XXX,"centerY":XXX,"endX":XXX,"endY":XXX,"startX":XXX,"startY":XXX}`.

### FilledBox(x1:number, y1:number, x2:number, y2:number, c:Color)
draw a filled box.

### FramedBox(x1:number, y1:number, x2:number, y2:number, [intcolor:Color, topcolor:Color, rightcolor:Color, bottomcolor:Color, leftcolor:Color])
draw a framed box (3D effect box). The last parameter is an array with the colors to use for the borders.

### FilledCircle(x1:number, y1:number, r:number, c:Color)
draw a filled circle.

### FilledEllipse(xc:number, yc:number, xa:number, ya:number, c:Color)
draw an ellipse.

### FilledCircleArc(x:number, y:number, r:number, start:number, end:number, style:number, c:Color):{"centerX":XXX,"centerY":XXX,"endX":XXX,"endY":XXX,"startX":XXX,"startY":XXX}
Draw a filled circle arc. Returns an object with coordinates of the drawn arc: `{"centerX":XXX,"centerY":XXX,"endX":XXX,"endY":XXX,"startX":XXX,"startY":XXX}`.

### FilledEllipseArc(xc:number, yc:number, xa:number, start:number, end:number, style:number, c:Color):{"centerX":XXX,"centerY":XXX,"endX":XXX,"endY":XXX,"startX":XXX,"startY":XXX}
Draw a filled ellipse arc. Returns an object with coordinates of the drawn arc: `{"centerX":XXX,"centerY":XXX,"endX":XXX,"endY":XXX,"startX":XXX,"startY":XXX}`.

### FloodFill(x:number, y:number, border:Color, c:Color)
do a flood fill.

### FloodSpill(x1:number, y1:number, x2:number, y2:number, border:Color, c:Color)
do a flood spill.

### FloodSpill2(x1:number, y1:number, x2:number, y2:number, old1:Color, new1:Color, old2:Color, new2:Color)
do a flood spill2.

### PolyLine(c:Color, [[x1, x2], [..], [xN, yN]])
draw a polyline.

### Polygon(c:Color, [[x1, x2], [..], [xN, yN]])
draw a polygon.

### FilledPolygon(c:Color, [[x1, x2], [..], [xN, yN]])
draw a filled polygon.

### FilledConvexPolygon(c:Color, [[x1, x2], [..], [xN, yN]])
draw a filled convex polygon.

### MouseSetSpeed(spmul:number, spdiv:number)
set mouse speed

### MouseSetAccel(thresh:number, accel:number)
set mouse acceleration

### MouseSetLimits(x1:number, y1:number, x2:number, y2:number)
set mouse limits

### MouseGetLimits():{"x1":XXX, "y1":XXX, "x2":XXX, "y2":XXX}
get mouse limits

### MouseWarp(x:number, y:number)
move mouse cursor

### MouseShowCursor(b:boolean)
show hide mouse cursor

### MouseCursorIsDisplayed():boolean
check if the cursor is visible

### MouseSetColors(fg:Color, bg:Color)
Set mouse pointer colors.

## MouseSetCursorMode(mode:int, ...)
change mode of the cursor.
`MouseSetCursorMode(MOUSE.Mode.NORMAL)` or
`MouseSetCursorMode(MOUSE.Mode.RUBBER,xanchor,yanchor,GrColor)` or
`MouseSetCursorMode(MOUSE.Mode.LINE,xanchor,yanchor,GrColor)` or
`MouseSetCursorMode(MOUSE.Mode.BOX,dx1,dy1,dx2,dy2,GrColor)`

### TextXY(x:number, y:number, text:string, fg:Color, bg:Color)
Draw a text with the default font.

### SaveBmpImage(fname:string)
Save current screen to file.

### SavePngImage(fname:string)
Save current screen to file.

## Other functions/properties
### SOUND_AVAILABLE: boolean
`true` if WAV sound is available.

### SYNTH_AVAILABLE: boolean
`true` if FM sound is available.

### MOUSE_AVAILABLE: boolean
`true` if mouse is available.

### MIDI_AVAILABLE: boolean
`true` if midi is available.

### Print(a, ...)
Write data to `JSLOG.TXT` logfile.

### Stop()
DOjS will exit after the current call to `Loop()`.

### Sleep(ms:number)
Sleep for the given number of ms.

### Read(filename:string):string
Load the contents of a file into a string.

### List(dname:string):[f1:string, f1:string, ...]
Get directory listing.

### Stat(name:string):{"atime":string,"blksize":number,"ctime":string,"drive":string,"is_blockdev":bool,"is_chardev":bool,"is_directory":bool,"is_regular":bool,"mtime":string,"nlink":number,"size":number}
Get information about a file/directory.

### Require(filename:string):module
Used to load a module. The functions exported from this module can be accessed using the returned value.

### Gc(info:boolean)
Run garbage collector, print statistics to logfile if `info==true`.

### SetFramerate(rate:number)
Set maximum frame rate. If `Loop()` takes longer than `1/rate` seconds then the framerate will not be reached.

### GetFramerate():number
Current frame rate.

### SetExitKey(key:number)
Change the exit key from ESCAPE to any other keycode from `jsboot/func.js`.

### CharCode(c:string)
Can be used to obtain the key code for `SetExitKey()` for a character, e.g. `SetExitKey(CharCode('q'))`.
