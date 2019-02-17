# DOjS
## A DOS JavaScript Canvas with sound.
DOjS is a JavaScrip-able canvas with WAV and FM sound support for systems running MS-DOS, [FreeDOS](http://freedos.org/) or any DOS based Windows (like 95, 98, ME).
It was inspired by [Processing](https://processing.org/) which is described on [Wikipedia](https://en.wikipedia.org/wiki/Processing_(programming_language)) as:

> Processing is an open-source graphical library and integrated development environment (IDE) / playground built for the electronic arts, new media art, and visual design communities with the purpose of teaching non-programmers the fundamentals of computer programming in a visual context.

For now you can type in a script with your favorite text editor and then run it on a DOS command prompt.

DOjS was only possible due to the work of these people/projects:
* [MuJS](https://mujs.com/) JavaScript interpreter
* The [GRX graphics library](http://grx.gnu.de/)
* The SoundBlaster example code from [Steven Don](http://www.shdon.com/dos/sound)
* [DJGPP](http://www.delorie.com/djgpp/) from DJ Delorie and the [Linux compile scripts](https://github.com/andrewwutw/build-djgpp) by Andrew Wu.

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

Now you are ready to compile DOjS with `make clean all`. This might take some time as GRX is quite a large project.
`make distclean` will clean MuJS and GRX as well.

## A minimal script
You can find the following example in `tests/exampl.js`:
```javascript
function Setup() {
	pink = new Color(241, 66, 244);	// define the color pink
}

function Loop() {
	TextXY(SizeX(), SizeY(), "Hello World", pink);
}
```
Run this with `DOjS.EXE tests\exampl.js`.

# Known bugs/limitations
* SoundBlaster IRQ detection only works for IRQs 3, 5 and 7. I could not figure out why the system locks up when 2, 10 and 11 are tried as well.
* There seems to be a bug in the FM sound system. Sounds created by DOjS sound different to the ones created by Steven Dons original code.
* ~~The JavaScript API is not stable yet. The current naming scheme is inconsistent to say the least...~~
* ~~All script-code has to be in a single file, no `include()` function yet.~~
* Some (nice) functions from GRX are still missing.
* Scripts are responsible to provide a way to exit DOjS, no general exit mechanism like with Processing is implemented yet.
* ~~Error handling is a nightmare. So far I could neither find a way to display a propper stack trace nor the offending line!~~
* WAV files need to be 11025Hz, 8bit, mono. Different sample rates produce a warning in the logfile (and sound weird). Different formats are outright refused.
* BMPs must be read 2, 4, 8 bpp.
* The Makefile does not honor the include files for dependencies.
* `MouseShowCursor(false)` does not work

# Planed work
* Fix bugs!
* ~~Add text-file reading/writing.~~
* ~~Add including other JS files.~~
* Add more GRX functions to JavaScript API.
* split up `func.c` into `grx.c` and `func.c`.
* ~~Straighten/cleanup the JavaScript API.~~
* ~~Rework the way scripts are started to the `setup()/loop()` solution known from Processing/Arduino.~~
* Include an in-system text editor like [KeXted](https://github.com/abelidze/KeXted) to change the scripts.
* Make scripts restartable.
* Include PNG loading.
* Improve sound loading/output.
* Add IXP network code.

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
It was [released](https://sourceforge.net/projects/cylindrix/) under GPLv2.


## CWSDPMI.EXE
CWSDPMI(http://sandmann.dotster.com/cwsdpmi/) DPMI host is licensed under GPL. The documentation states:
> The files in this binary distribution may be redistributed under the GPL (with source) or without the source code provided.

# JS API description
A very minimal API documentation.

## Script format
Scripts need to provide two functions: `Setup()` and `Loop()`. Scripts are loaded and executed top-own. After that `Setup()` is called once and then `Loop()` repeatedly.

### Setup()
This function is called once at startup. It can initialize variables, setup hardware, etc.

### Loop()
This function is called after setup repeatedly until `Stop()` is called. After calling `Stop()` the program ends when `Loop()` exits.

## File
### f = new File(filename:string, mode:string)
Open a file and store it as userdata in JS object. For file modes see `jsboot/file.js`. Files can only either be read or written, never both. Writing to a closed file throws an exception.

### f.ReadByte():number
Read a single byte from file and return it as number.

### f.WriteByte(ch:number)
Write a single byte to

### f.ReadLine():string
Read a line of text from file. The maximum line length is 4096 byte.

### f.WriteLine(txt:string)
Write a NEWLINE terminated string to a file.

### f.WriteString(txt:string)
Write a string to a file.

### f.Close()
Close the file.

## IPX networking
DOjS supports IPX networking.

### IpxSocketOpen(num:number)
Open an IPX socket.

### IpxSocketClose()
Close IPX socket (if any).

### IpxSendPacket(data:string, dest:array[6:number])
Send packet via IPX. Max length 79 byte.

### IpxCheckPacket():boolean
Check for packet in receive buffer

### IpxGetPacket():{data:string, source:array[6:number]}
Get packet from receive buffer (or NULL).

### IpxGetLocalAddress():array[6:number]
Get the local address.

## Color
See *jsboot/color.js* for predefined EGA colors.

### c = new Color(red:number, green:number, blue:number)
Create a color

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
Load a BMP image.

### bm.filename
Name of the BMP file.

### bm.width
Width in pixels

### bm.height
Height in pixels

### bm.Draw(x:number, y:number)
Draw the image to the canvas at given coordinates.

## Font
See *jsboot/font.js* for direction and alignment values.

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
Load a WAV (11025Hz, 8bit, mono).

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
See *jsboot/fmmusic.js* for pre-defined instruments and notes.

### SetInstrument(voice:number, instrument:object)
Convert the contents of a JS object into an instrument and set the parameters to the given voice.

### NoteOn(voice:number, note:number, octave: number)
Start playing a note.

### NoteOff(voice:number, note:number, octave: number)
Stop playing a note.

## GRX Drawing functions
Please take a look a the GRX [documentation](http://grx.gnu.de/grx249um.html) for details of the drawing functions.
See *jsboot/func.js* for constants.

### NumColors():number
Get number of possible colors.

### NumFreeColors():number
Get number of remaining free colors.

### KeyRead():number
wait for keypress and return the keycode.

### KeyPressed():boolean
check for keypress.

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
draw a plot.

### Plot(x1:number, y1:number, x2:number, y2:number, c:Color)
draw a line.

### Box(x1:number, y1:number, x2:number, y2:number, c:Color)
draw a box.

### Circle(x1:number, y1:number, r:number, c:Color)
draw a circle.

### Ellipse(xc:number, yc:number, xa:number, ya:number, c:Color)
draw an ellipse.

### CircleArc(x:number, y:number, r:number, start:number, end:number, style:number, c:Color):{"centerX":XXX,"centerY":XXX,"endX":XXX,"endY":XXX,"startX":XXX,"startY":XXX}
Draw a circle arc.

### EllipseArc(xc:number, yc:number, xa:number, start:number, end:number, style:number, c:Color):{"centerX":XXX,"centerY":XXX,"endX":XXX,"endY":XXX,"startX":XXX,"startY":XXX}
Draw an ellipse arc.

### FilledBox(x1:number, y1:number, x2:number, y2:number, c:Color)
draw a filled box.

### FramedBox(x1:number, y1:number, x2:number, y2:number, [intcolor:Color, topcolor:Color, rightcolor:Color, bottomcolor:Color, leftcolor:Color])
draw a framed box.

### FilledCircle(x1:number, y1:number, r:number, c:Color)
draw a filled circle.

### FilledEllipse(xc:number, yc:number, xa:number, ya:number, c:Color)
draw an ellipse.

### FilledCircleArc(x:number, y:number, r:number, start:number, end:number, style:number, c:Color):{"centerX":XXX,"centerY":XXX,"endX":XXX,"endY":XXX,"startX":XXX,"startY":XXX}
Draw a filled circle arc.

### FilledEllipseArc(xc:number, yc:number, xa:number, start:number, end:number, style:number, c:Color):{"centerX":XXX,"centerY":XXX,"endX":XXX,"endY":XXX,"startX":XXX,"startY":XXX}
Draw a filled ellipse arc.

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
mose mouse cursor

### MouseShowCursor(b:boolean)
show hide mouse cursor

### MouseCursorIsDisplayed():boolean
check if the cursor is visible

### MousePendingEvent():boolean
Check if there are pending events

### MouseGetEvent(flags:number):{x:number, y:number, flags:number, buttons:number, key:number, kbstat:number, dtime:number}
Get the next event filtered by flags.

### MouseEventEnable(kb:boolean, ms:boolean)
Enable/disable mouse/keyboard events

### MouseSetColors(fg:Color, bg:Color)
Set mouse pointer colors.

## MouseSetCursorMode(mode:int, ...)
change mode of the cursor.
MouseSetCursorMode(MOUSE.Mode.NORMAL) or
MouseSetCursorMode(MOUSE.Mode.RUBBER,xanchor,yanchor,GrColor) or
MouseSetCursorMode(MOUSE.Mode.LINE,xanchor,yanchor,GrColor) or
MouseSetCursorMode(MOUSE.Mode.BOX,dx1,dy1,dx2,dy2,GrColor) or

### TextXY(x:number, y:number, text:string, fg:Color, bg:Color)
Draw a text with the default font.

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

### Require(filename:string):module
Used to load a module. The functions from this module can be accessed using the returned value.

