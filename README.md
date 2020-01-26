# DOjS
## A DOS JavaScript Canvas with sound.
[DOjS](https://github.com/SuperIlu/DOjS) is a JavaScript-able canvas with WAV and MIDI sound support for systems running MS-DOS, [FreeDOS](http://freedos.org/) or any DOS based Windows (like 95, 98, ME).
It was inspired by [Processing](https://processing.org/) which is described on [Wikipedia](https://en.wikipedia.org/wiki/Processing_(programming_language)) as:

> Processing is an open-source graphical library and integrated development environment (IDE) / playground built for the electronic arts, new media art, and visual design communities with the purpose of teaching non-programmers the fundamentals of computer programming in a visual context.

It also has a p5js compatibility mode where some of the functions of [p5js](https://p5js.org/) are available and scripts can have a similar structure to Processing sketches.

You can type in a script with the builtin or your favorite text editor and then run it on a DOS command prompt.

DOjS is pronounces like [doge](https://en.wikipedia.org/wiki/Doge_(meme)), but ending with an "s".

DOjS was only possible due to the work of these people/projects:
* [MuJS](https://mujs.com/) JavaScript interpreter
* The [Allegro library](https://liballeg.org/)
* [DJGPP](http://www.delorie.com/djgpp/) from DJ Delorie and the [Linux compile scripts](https://github.com/andrewwutw/build-djgpp) by Andrew Wu.
* The people that contributed to [p5js](https://p5js.org/).
* The Glide source cleanup of [Ozkan Sezer](https://github.com/sezero/glide).

You can find me on [Twitter](https://twitter.com/dec_hl) if you want...

# Download and quick start
**You can find binary releases on the [GitHub release page](https://github.com/SuperIlu/DOjS/releases).** Just extract the contents of the archive and run DOjS.

DOjS runs in [Dosbox](https://www.dosbox.com/) and on real hardware or a virtual machine with MS-DOS, [FreeDOS](https://www.freedos.org/) or any DOS based Windows like Windows 95/98/ME. To use 3Dfx/Glide support you need a Voodoo card or [DOSBox-X](https://github.com/joncampbell123/dosbox-x/releases) (see below).

If you run it on real hardware you need at least a **80386 with 4MB**. I recommend a **Pentium class machine (>= 100MHz) with at least 32MB RAM**. The example files run fine on an Athlon 1GHz and with 256MB RAM.

The following hardware/functions are available:
* 8/16/24 and 32 bit 2D graphics. On 32bit display modes alpha channel transparency is available.
* Keyboard input
* Mouse input
* Joystick/Joyport input
* File IO
* MIDI output
* WAV output
* Audio input/sampling
* Allegro 3D rendering (software)
* 3dfx/Glide3 3D rendering output (hardware)
* p5js compatibility

## A minimal script
You can find the following example in `examples/exampl.js`:
```javascript
/*
** This function is called once when the script is started.
*/
function Setup() {
    pink = new Color(241, 66, 244, 255); // define the color pink
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
Open this script with `DOjS.EXE examples\exampl.js` or use `DOjS.EXE -r examples\exampl.js` to run it without starting the integrated editor first. If the script does not exist the editor loads the template for a new script.

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
    text("Hello p5js World!", width / 2, height / 2);

    stroke(EGA.LIGHT_BLUE);
    fill(EGA.LIGHT_BLUE);
    text("rate=" + getFrameRate(), 10, 10);
}
```

More info can be found at the end of this README in the section **Usage** and in the API documentation. Take a look at the `examples/` as well.

## 3dfx/Glide support
DOjS supports most of the Glide3 API that was used with [3dfx](https://en.wikipedia.org/wiki/3dfx_Interactive) accelerator cards. The following hardware is supported:
* Voodoo 1 [tested]
* Voodoo 2 [tested]
* Voodoo 3 [tested]
* Voodoo 4 [untested, I don't own one]
* Voodoo 5 [untested, I don't own one]
* Voodoo Rush (all versions) [tested]
* Voodoo Banshee (PCI and AGP) [tested]

Additionally you can use [DOSBox-X](https://github.com/joncampbell123/dosbox-x/releases) which emulates a Voodoo 1 card. Glide functions can be found in the 3dfx-module in the documentation, Javascript support functions have a "FX" prefix, all native functions are prefixed with "fx". Detailed Glide3-API documentation can be found on the internet, e.g. on [FalconFly Central](http://falconfly.3dfx.pl/reference.htm). Make sure you grab the Glide3 SDK and not Glide2!
You can use the included DOS version of `TEXUS.EXE` to convert bitmaps to `3df` texture files that can be loaded as textures.
**!!! Attention !!!**
3dfx/Glide3 support ONLY works in plain DOS, NOT in the DOS/command window of Windows 9x! Be sure to always boot into a pure DOS prompt before trying to use any of the fx-functions!
Before using 3dfx/Glide3 support you need to copy the appropriate `GLIDE3X.DXE` into the same directory as `DOJS.EXE`. You can do so by using the `V_XXX.BAT` scripts in the distribution ZIP archive.

# Compilation
You can compile DOjS on any modern Linux (the instructions below are for Debian based distributions) or on Windows 10 using Windows Subsystem for Linux (WSL).
Setup Windows Subsystem for Linux (WSL) according to [this](https://docs.microsoft.com/en-us/windows/wsl/install-win10) guide (I used Ubuntu 18.04 LTS).

## Preparation
Build and install DJGPP 7.2.0 according to [this](https://github.com/andrewwutw/build-djgpp) guide.
I used the following command lines to update/install my dependencies:
```bash
sudo apt-get update
sudo apt-get dist-upgrade
sudo apt-get install bison flex curl gcc g++ make texinfo zlib1g-dev g++ unzip htop screen git bash-completion build-essential npm zip
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

## 3dfx/Glide3
In order to compile DOjS you need Glide3 includes and binaries. The ones included with the DOjS sources were created using my [glide repository](https://github.com/SuperIlu/glide) on GitHub. 

# History
See the [changelog](/CHANGELOG.md) for the projects history.

# Planed work
* Fix bugs!
* Improve help viewer (context help?).
* Anything fun...
* Switch to Duktape JS runtime
* Add COM port access
* Enable runtime creation of 3dfx/glide textures

# Licenses
## DOjS
All code from me is released under **MIT license**.

## MuJS
MuJS is released under **ISC license**. See *COPYING* in the MuJS folder for details.

## Allegro
Allegro 4 is released under the **Giftware license** (https://liballeg.org/license.html).

## GRX
The GRX fonts are under **MIT and other licenses**. See copying.grx in `LICENSE` for details.

## IPX and dosbuffer sub-system
This code is taken from the game [Cylindrix](https://www.allegro.cc/depot/Cylindrix) by Hotwarez LLC, Goldtree Enterprises.
It was [released](https://sourceforge.net/projects/cylindrix/) under **GPLv2**.

## CWSDPMI.EXE
[CWSDPMI](http://sandmann.dotster.com/cwsdpmi/) DPMI host is licensed under **GPL**. The documentation states:
> The files in this binary distribution may be redistributed under the GPL (with source) or without the source code provided.

## 3dfx/Glide3
The code is licensed under "3DFX GLIDE Source Code General Public License".
Source code is available at https://github.com/SuperIlu/glide

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

## p5js and examples
p5js is is released under **LGPL**. 

The examples are licensed under a [Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License](https://creativecommons.org/licenses/by-nc-sa/4.0/).

# Usage
## Command line
```
Usage: DOjS.EXE [-r] [-s] [-f] [-a] <script> [script parameters]
    -r             : Do not invoke the editor, just run the script.
    -w <width>     : Screen width: 320 or 640, Default: 640.
    -b <bpp>       : Bit per pixel:8, 16, 24, 32. Default: 32.
    -s             : No wave sound.
    -f             : No FM sound.
    -a             : Disable alpha (speeds up rendering).
```

## Editor keys
```
    F1  : Open/Close help
    F3  : Save script
    F4  : Run script
    F7  : Find text
    F9  : Show/Close logfile
    F10 : Quit

    Shift-F4       : Truncate logfile and run script
    Shift-F7       : Find again
    CTRL-D         : Delete current line
    SHIFT+Movement : Select text, releasing SHIFT deselects
    CTRL-C         : Copy selection
    CTRL-X         : Cut selection
    CTRL-V         : Paste
    CTRL-LEFT      : Previous word
    CTRL-RIGHT     : Next word
    PAGE-UP        : One page up.
    PAGE-DOWN      : One page down.
    HOME           : Go to start of line
    END            : Go to end of line
    CTRL-HOME      : Go to start of line
    CTRL-END       : Go to end of line
    TAB            : Insert spaces until next TAB-stop at cursor
    SHIFT-TAB      : Reduce indentation of line.

    TAB size is 4.
    The help viewer will remember the current position.

    The logfile can be truncated by pressing DEL in the log viewer.
```

## API documentation
You can find the full API doc in the [doc/html/](http://htmlpreview.github.io/?https://github.com/SuperIlu/DOjS/blob/master/doc/html/index.html) directory.
Go to the p5.js hompage for [p5.js reference](https://p5js.org/reference/).

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

## Drawing functions
See API doc for details.

## Processing/p5js compatibility layer
Add `Include('p5');` as first line to your script. After that you have (limited) [p5.js](https://p5js.org/reference/) compatibility.
Things that don't work:
* Anything 3D (objects, lights camera, etc)
* Key release events work different for Allegro and are simulated for p5js.
* Only simple vertices are supported.
* no DOM

## Logfile
All output via `Print()` and `Println()` is sent to the file `JSLOG.TXT`. You can use `Debug()` instead and output is only generated when you set the global variable `DEBUG=true`.

## Remote logging/debugging
This feature allows you to debug a running script via IPX networking and a second machine. To use remote logging do the following:
* Put both machines on the same network.
* Run `DOJS.EXE -r JSBOOT\LOGVIEW.JS` on one machine.
* Enable debugging by setting `DEBUG=true` and enable remote debugging by `REMOTE_DEBUG=true`. You can either modify `JSBOOT\FUNC.JS` or change the variables at the very beginning of your script.

This works fine with two instances of DOSBox as well.
Please note that if the log messages are transmitted to fast the receiving instance of DOJS might skip some of these when displaying.
