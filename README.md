# DOjS
## A DOS JavaScript Canvas with sound.
[DOjS](https://github.com/SuperIlu/DOjS) is a JavaScript-able canvas with WAV, MIDI and FM sound support for systems running MS-DOS, [FreeDOS](http://freedos.org/) or any DOS based Windows (like 95, 98, ME).
It was inspired by [Processing](https://processing.org/) which is described on [Wikipedia](https://en.wikipedia.org/wiki/Processing_(programming_language)) as:

> Processing is an open-source graphical library and integrated development environment (IDE) / playground built for the electronic arts, new media art, and visual design communities with the purpose of teaching non-programmers the fundamentals of computer programming in a visual context.

It also has a p5js compatibility mode where some of the functions of [p5js](https://p5js.org/) are available and scripts can have a similar structure to Processing sketches.

You can type in a script with the builtin or your favorite text editor and then run it on a DOS command prompt.

DOjS is pronounces like [doge](https://en.wikipedia.org/wiki/Doge_(meme)), but ending with an "s".

![DOjS logo](/examples/DOjS.png)

DOjS was only possible due to the work of these people/projects:
* [MuJS](https://mujs.com/) JavaScript interpreter
* The [GRX graphics library](http://grx.gnu.de/)
* [libmikmod](http://mikmod.sourceforge.net/) sound library.
* The FM and MIDI code from [Steven Don](http://www.shdon.com/dos/sound)
* [DJGPP](http://www.delorie.com/djgpp/) from DJ Delorie and the [Linux compile scripts](https://github.com/andrewwutw/build-djgpp) by Andrew Wu.
* The people that contributed to [p5js](https://p5js.org/).

You can find me on [Twitter](https://twitter.com/dec_hl) if you want...

# Download and quick start
You can find binary releases on the [GitHub release page](https://github.com/SuperIlu/DOjS/releases). Just extract the contents of the archive and run DOjS.

DOjS run in [Dosbox](https://www.dosbox.com/) and on real hardware or a virtual machine with MS-DOS, [FreeDOS](https://www.freedos.org/) or any DOS based Windows like Windows 95/98/ME.

If you run it on real hardware you need at least a **80386 with 4MB**. I recommend a **Pentium class machine (>= 100MHz) with at least 32MB RAM**. The example files run fine on an Athlon 1GHz and with 256MB RAM.

## A minimal script
You can find the following example in `examples/exampl.js`:
```javascript
/*
** This function is called once when the script is started.
*/
function Setup() {
    pink = new Color(241, 66, 244); // define the color pink
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
    ClearScreen(EGA.BLACK);
    TextXY(SizeX() / 2, SizeY() / 2, "Hello World!", pink, NO_COLOR);

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

## p5js compatibility
If you want to write scripts using the syntax of [p5js](https://p5js.org/) you need to use `Include('p5');` as first line of your script. You can find the following example in `examples/examplp5.js`:

```javascript
Include('p5');

/*
** This function is called once when the script is started.
*/
function setup() {
    pink = color(241, 66, 244); // define the color pink
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function draw() {
    background(EGA.BLACK);
    stroke(pink);
    fill(pink);
    text("Hello World!", width / 2, height / 2);

    stroke(EGA.LIGHT_BLUE);
    fill(EGA.LIGHT_BLUE);
    text("rate=" + getFrameRate(), 10, 10);
}
```

More info can be found at the end of this README in the section **Usage** and in the API documentation. Take a look at the `examples/` as well.

# Compilation
You can compile DOjS on any modern Linux (the instructions below are for Debian based distributions) or on Windows 10 using Windows Subsystem for Linux (WSL).
Setup Windows Subsystem for Linux (WSL) according to [this](https://docs.microsoft.com/en-us/windows/wsl/install-win10) guide (I used Ubuntu 18.04 LTS).

## Preparation
Build and install DJGPP 7.2.0 according to [this](https://github.com/andrewwutw/build-djgpp) guide.
I used the following command lines to update/install my dependencies:
```bash
sudo apt-get update
sudo apt-get dist-upgrade
sudo apt-get install bison flex curl gcc g++ make texinfo zlib1g-dev g++ unzip htop screen git bash-completion build-essential npm
sudo npm install -g jsdoc
sudo npm install -g tui-jsdoc-template
```

And the following commands to build and install DJGPP to `/home/ilu/djgpp`.:
```bash
git clone https://github.com/andrewwutw/build-djgpp.git
cd build-djgpp
export DJGPP_PREFIX=/home/ilu/djgpp
./build-djgpp.sh 7.2.0
```

## Getting & Compiling DOjS
Open a shell/command line in the directory where you want the source to reside.

Checkout DOjS from Github:
```bash
git clone https://github.com/SuperIlu/DOjS.git
```

Open the Makefile in a text editor and change the path to DJGPP according to your installation.

Now you are ready to compile DOjS with `make clean all`. This might take some time as the dependencies are quite a large.
`make distclean` will clean dependencies as well. `make zip` will create the distribution ZIP and `make doc` will re-create the HTML help.

# History
See the [changelog](/CHANGELOG.md) for the projects history.

# Known bugs/limitations
* There seems to be a bug in the FM sound system. Sounds created by DOjS sound different to the ones created by Steven Dons original code.
* Some (nice) functions from GRX are still missing.
* BMPs must be 2, 4, 8 bpp.
* `MouseShowCursor(false)` does not work
* The double buffering (fix for flickering graphics) created visibility issues with the mouse cursor.

# Planed work
* Fix bugs!
* Add more GRX functions to JavaScript API.
* split up `func.c` into `grx.c` and `func.c`.
* Improve help viewer.

# Licenses
## DOjS
All code from me is released under **MIT license**.

## MuJS
MuJS is released under **ISC license**. See *COPYING* in the MuJS folder for details.

## GRX
GRX itself is released under **LGPL**, the fonts are under **MIT and other licenses**. See *copying.grx* in the grx folder for details.

## Mikmod
Mikmod is released under **LGPL**.

## MIDI and FM sound
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

## p5js and examples
p5js is is released under **LGPL**. 

The examples are licensed under a [Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License](https://creativecommons.org/licenses/by-nc-sa/4.0/).

# Usage
## Command line
```
    Usage: DOjS.EXE [-r] [-s <p>:<i>:<d>] <script>
        -r             : Do not invoke the editor, just run the script.
        -w <width>     : Screen width: 320 or 640, Default: 640.
        -b <bbp>       : Bit per pixel:8, 16, 24, 32. Default: 24.
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

## API documentation
You can find the full API doc in the [doc/html/](/doc/html/index.html) directory.

## Script format
Scripts need to provide three functions: `Setup()`, `Loop()` and `Input()`. Scripts are loaded and executed top-own. After that `Setup()` is called once and then `Loop()` repeatedly. `Input()` is called whenever mouse of keyboard input happens.

### Setup()
This function is called once at startup. It can initialize variables, setup hardware, etc.

### Loop()
This function is called after setup repeatedly until `Stop()` is called. After calling `Stop()` the program ends when `Loop()` exits.

### Input(event)
This function is called whenever mouse/keyboard input happens.

## IPX networking
DOjS supports IPX networking. Node addresses are arrays of 6 numbers between 0-255. Default socket number and broadcast address definitions can be found in `jsboot/ipx.js`.

## GRX Drawing functions
Please take a look a the GRX [documentation](http://grx.gnu.de/grx249um.html) for details of the drawing functions.
See `jsboot/func.js` for constants.

## Processing/p5js compatibility layer
Add `Include('p5');` as first line to your script. After that you have (limited) [p5.js](https://p5js.org/reference/) compatibility.
Things that don't work:
* GRX has no transparency when rendering, so all functions that need an alpha channel won't work.
* Anything 3D (objects, lights camera, etc)
* Smoothing is not supported.
* Key release events work different for GRX and are simulated for p5js.
* Only simple vertices are supported.
* no DOM
* no transformations

## Logfile
All output via `Print()` and `Println()` is sent to the file `JSLOG.TXT`. You can use `Debug()` instead and output is only generated when you set the global variable `DEBUG=true`.

## Remote logging/debugging
This feature allows you to debug a running script via IPX networking and a second machine. To use remote logging do the following:
* Put both machines on the same network.
* Run `DOJS.EXE -r JSBOOT\LOGVIEW.JS` on one machine.
* Enable debugging by setting `DEBUG=true` and enable remote debugging by `REMOTE_DEBUG=true`. You can either modify `JSBOOT\FUNC.JS` or change the variables at the very beginning of your script.

This works fine with two instances of DOSBox as well.
