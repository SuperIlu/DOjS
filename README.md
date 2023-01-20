# DOjS
## A DOS JavaScript Canvas with sound.
[DOjS](https://github.com/SuperIlu/DOjS) is a JavaScript programming environment for systems running MS-DOS, [FreeDOS](http://freedos.org/) or any DOS based Windows (like 95, 98, ME).
It features an integrated editor, graphics & sound output, mouse/keyboard/joystick input and more (see below).
It was inspired by [Processing](https://processing.org/) which is described on [Wikipedia](https://en.wikipedia.org/wiki/Processing_(programming_language)) as:

> Processing is an open-source graphical library and integrated development environment (IDE) / playground built for the electronic arts, new media art, and visual design communities with the purpose of teaching non-programmers the fundamentals of computer programming in a visual context.

It also has a p5js compatibility mode where some of the functions of [p5js](https://p5js.org/) are available and scripts can have a similar structure to Processing sketches.

You can type in a script with the builtin or your favorite text editor and then run it on a DOS command prompt.

DOjS is pronounces like [doge](https://en.wikipedia.org/wiki/Doge_(meme)), but ending with an "s".

DOjS was only possible due to the work of these people/projects:
  * [MuJS](https://mujs.com/) JavaScript interpreter
  * The [Allegro library](https://liballeg.org/) and [Allegro XC](https://github.com/msikma/allegro-4.2.2-xc)
  * [DJGPP](http://www.delorie.com/djgpp/) from DJ Delorie and the [Linux compile scripts](https://github.com/andrewwutw/build-djgpp) by Andrew Wu.
  * The people that contributed to [p5js](https://p5js.org/).
  * The Glide source cleanup of [Ozkan Sezer](https://github.com/sezero/glide).
  * [AllegroPNG](http://alpng.sourceforge.net/) PNG loader for Allegro
  * [DZCOMM](http://dzcomm.sourceforge.net/) COM port library
  * [libcpuid](https://github.com/anrieff/libcpuid) for cpuid module
  * [curl](https://curl.se/) and [OpenSSL](https://www.openssl.org/) for curl module
  * [cylindrix](https://sourceforge.net/projects/cylindrix/) for IPX module
  * [nanosvg](https://github.com/memononen/nanosvg) for SVG loading
  * [noise](https://github.com/stegu/perlin-noise) for perlin noise generation
  * [genann](https://github.com/codeplea/genann) for neural network engine
  * [SQLite](https://www.sqlite.org/index.html) for sqlite module
  * [Watt32](https://github.com/gvanem/Watt-32.git) for TCP/IP networking
  * [zip](https://github.com/kuba--/zip) for ZIP file access
  * [zlib](http://zlib.net/) for compression
  * [stb_image.h](https://github.com/nothings/stb/blob/master/stb_image.h) for JPEG loading
  * [AnimatedGIF](https://github.com/bitbank2/AnimatedGIF/) for rendering GIF animations.
  * [PL_MPEG](https://github.com/phoboslab/pl_mpeg) for mpeg1 decoding
  * [pdfgen](https://github.com/AndreRenaud/PDFGen) for PDF rendering.

## Contact
You can find me on [Twitter](https://twitter.com/dec_hl), [Mastodon](https://mastodon.social/@dec_hl) or in the [DOjS Discord](https://discord.gg/J7MUTap9fM) if you want...

# Download and quick start
**You can find binary releases on the [GitHub release page](https://github.com/SuperIlu/DOjS/releases).** Just extract the contents of the archive and run `DOjS.exe`.

DOjS runs in [Dosbox](https://www.dosbox.com/) and on real hardware or a virtual machine with MS-DOS, [FreeDOS](https://www.freedos.org/) or any DOS based Windows like Windows 95/98/ME. To use 3Dfx/Glide support you need a Voodoo card or [DOSBox-X](https://github.com/joncampbell123/dosbox-x/releases) (see below).

If you run it on real hardware you need at least a **80386 with 4MB**. I recommend a **Pentium class machine (>= 100MHz) with at least 32MB RAM**. The example files run fine on an Athlon 1GHz and with 256MB RAM.

The following hardware/functions are available:
  * 8/16/24 and 32 bit 2D graphics. On 24/32bit display modes alpha channel transparency is available.
  * BMP, PCX, TGA and PNG image reading and writing, JPEG and SVG loading
  * GRX font loading and rendering
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
  * direct io-port access (inb, outb, etc)
  * LPT or parallel port access (bi-directional)
  * COM or serial port access
  * IPX and TCP/IP networking
  * ZIP file access
  * GIF-Animation, FLC/FLI, MPEG1 or OggVorbis playback

## Examples
<img src="https://github.com/SuperIlu/DOjS/raw/master/images/dojs_003.png" alt="DOjS example" width="200">
<img src="https://github.com/SuperIlu/DOjS/raw/master/images/dojs_010.png" alt="DOjS example" width="200">
<img src="https://github.com/SuperIlu/DOjS/raw/master/images/dojs_019.png" alt="DOjS example" width="200">
<img src="https://github.com/SuperIlu/DOjS/raw/master/images/dojs_034.png" alt="DOjS example" width="200">
<img src="https://github.com/SuperIlu/DOjS/raw/master/images/dojs_004.png" alt="DOjS example" width="200">
<img src="https://github.com/SuperIlu/DOjS/raw/master/images/dojs_013.png" alt="DOjS example" width="200">
<img src="https://github.com/SuperIlu/DOjS/raw/master/images/dojs_025.png" alt="DOjS example" width="200">
<img src="https://github.com/SuperIlu/DOjS/raw/master/images/gltixy.png" alt="DOjS example" width="200">
<img src="https://github.com/SuperIlu/DOjS/raw/master/images/dojs_009.png" alt="DOjS example" width="200">
<img src="https://github.com/SuperIlu/DOjS/raw/master/images/dojs_018.png" alt="DOjS example" width="200">
<img src="https://github.com/SuperIlu/DOjS/raw/master/images/dojs_026.png" alt="DOjS example" width="200">
<img src="https://github.com/SuperIlu/DOjS/raw/master/images/ogl02.png" alt="DOjS example" width="200">

See [DOStodon](https://github.com/SuperIlu/DOStodon) for an complex examples. It is a full Mastodon client implemented using DOjS.

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
** This function is called repeatedly until ESC is pressed or Stop() is called.
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
** This function is called repeatedly until ESC is pressed or Stop() is called.
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

## additional packages
DOjS has a very simple integrated package manager (DPM). It can be started with `DPM.BAT`.
A working packet driver is needed to connect to the package index and download packages using HTTPS.
Packages (and the package index) are fetched from the [DOjS/jSH package repository](https://github.com/SuperIlu/DOjSHPackages).
Downloaded packages are put into `JSBOOT.ZIP` in the `PACKAGE/` directory.
Feel free to submit any packages you want to include in that repository using a pull request.
DPM commands:
  * installed - list installed packages.
  * remove    - remove package.
  * fetch     - fetch package index from server.
  * install   - install a package (and its dependencies) from package index.
  * list      - list available packages in index.
  * setindex  - set index URL (HTTP or HTTPS).
  * help      - this help.;
  * quit      - exit dpm.

## 3dfx/Glide support
DOjS supports most of the Glide3 API that was used with [3dfx](https://en.wikipedia.org/wiki/3dfx_Interactive) accelerator cards. The following hardware is supported:
  * Voodoo 1 [tested]
  * Voodoo 2 [tested]
  * Voodoo 3 [tested]
  * Voodoo 4 [tested by [tunguska](https://twitter.com/tunguska82)]
  * Voodoo 5 [tested by [tunguska](https://twitter.com/tunguska82)]
  * Voodoo Rush (all versions) [tested]
  * Voodoo Banshee (PCI and AGP) [tested]

Additionally you can use [DOSBox-X](https://github.com/joncampbell123/dosbox-x/releases) which emulates a Voodoo 1 card. Glide functions can be found in the 3dfx-module in the documentation, Javascript support functions have a "FX" prefix, all native functions are prefixed with "fx". Detailed Glide3-API documentation can be found on the internet, e.g. on [FalconFly Central](http://falconfly.3dfx.pl/reference.htm). **Make sure you grab the Glide3 SDK and not Glide2!**

You can use the included DOS version of `TEXUS.EXE` to convert bitmaps to `3df` texture files that can be loaded as textures.

**!!! Attention !!!**
3dfx/Glide3 support ONLY works in plain DOS, NOT in the DOS/command window of Windows 9x! Be sure to always boot into a pure DOS prompt before trying to use any of the fx-functions!
Before using 3dfx/Glide3 support you need to copy the appropriate `GLIDE3X.DXE` into the same directory as `DOJS.EXE`. You can do so by using the `V_XXX.BAT` scripts in the distribution ZIP archive.

# Compilation
You can compile DOjS on any modern Linux (the instructions below are for Debian based distributions) or on Windows 10 using Windows Subsystem for Linux (WSL).
Setup Windows Subsystem for Linux (WSL) according to [this](https://docs.microsoft.com/en-us/windows/wsl/install-win10) guide (I used Ubuntu 18.04 LTS).

## Preparation
Build and install DJGPP 7.2.0 according to [this](https://github.com/andrewwutw/build-djgpp) guide.
Install [NVM](https://github.com/nvm-sh/nvm#installing-and-updating).
I used the following command lines to update/install my dependencies:
```bash
sudo apt-get update
sudo apt-get dist-upgrade
sudo apt-get install bison flex curl gcc g++ make texinfo zlib1g-dev g++ unzip htop screen git bash-completion build-essential zip dos2unix python3
nvm install node
npm install -g jsdoc
npm install -g better-docs
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

Make sure DJGPP is in your PATH and DJDIR is set (e.g. I have these lines in my ~/.profile):
```
# DJGPP
export PATH=/home/ilu/djgpp/bin/:/home/ilu/djgpp/i586-pc-msdosdjgpp/bin/:$PATH
export DJDIR=/home/ilu/djgpp/
```

If you used Windows-Tools to check out DOjS from git you may need to fix the newlines of the shell scripts by using `make fixnewlines`.

Now you are ready to compile DOjS with `make clean all`. This might take some time as the dependencies are quite large.
`make distclean` will clean dependencies as well. `make zip` will create the distribution ZIP and `make doc` will re-create the HTML help.

# Notes
## 3dfx/Glide3
In order to compile DOjS you need Glide3 includes and binaries. The ones included with the DOjS sources were created using my [glide repository](https://github.com/SuperIlu/glide) on GitHub. 

## GRX Fonts
DOjS comes pre-bundled with all fonts included with GRX (files ending with ".FNT"). If you want/need additional fonts you can find a very simple tool to convert TTF/BDF fonts to GRX format [here](https://github.com/SuperIlu/GrxFntConv). Results may vary...
A minimal version capable of converting BDF fonts to FNT is included with DOjS (`FONTCONV.EXE`).
You can find information on how to convert a TTF to BDF [here](https://learn.adafruit.com/custom-fonts-for-pyportal-circuitpython-display/conversion).

## Live coding
If you run DOjS on a computer with network interface and a matching packet driver you can (sort of) live code using the [VSCode](https://code.visualstudio.com/) extension in `vscode\livedojs-0.0.4.vsix`. You need to start `DOjS -r examples/websvr.js` and then set the IP address in VSCode using the command `DOjS: Set hostname`. Live coding sketches must look like below to work:
```
// livedojs
exports.Setup = function () {
}

exports.Loop = function () {
    ClearScreen(EGA.BLACK);

    FilledBox(10, 10, 70, 20, EGA.GREEN);
}
```
  * The first line must be exactly `// livedojs`
  * The file must end with `.js`
  * Only `Setup()` and `Loop()` are available, `Input()` does not work.
  * p5js compatibility does not work, you must code using DOjS native API
  * If the hostname is set the sketch will be automatically be uploaded on save
  * The sketch can be uploaded using `DOjS: Upload sketch` manually
  * you can access the JSLOG.TXT of the running server by using `DOjS: get logfile`

# History
See the [changelog](/CHANGELOG.md) for the projects history.

# Planed work
  * Stack trace selector in the editor
  * TCP/IP remote logging/debugging.
  * add FFT module
  * add webp decoder (https://github.com/webmproject/libwebp/blob/main/doc/api.md)
  * Add ZIP file functions (e.g. https://libzip.org/users/ or https://github.com/kuba--/zip).
    * Implement 3df file loading from ZIP
  * add/implement some more math functions
    * https://mathjs.org/
    * https://github.com/evanw/lightgl.js
  * Fix bugs!
  * Anything fun...
    * http://www.speech.cs.cmu.edu/flite/
    * http://speect.sourceforge.net/
    * http://espeak.sourceforge.net/

# Licenses
See [LICENSE](LICENSE) file for all licenses.

## DOjS
All code from me is released under **MIT license**.

## MuJS
MuJS is released under **ISC license**. See *COPYING* in the MuJS folder for details.

## Allegro
Allegro 4 is released under the **Giftware license** (https://liballeg.org/license.html).

## GRX fonts
The GRX fonts are under **MIT and other licenses**. See copying.grx in `LICENSE` for details.
The converted fonts from the Linux Font Project are in the public domain.

## IPX and dosbuffer sub-system
This code is taken from the game [Cylindrix](https://www.allegro.cc/depot/Cylindrix) by Hotwarez LLC, Goldtree Enterprises.
It was [released](https://sourceforge.net/projects/cylindrix/) under **GPLv2**.

## CWSDPMI.EXE
[CWSDPMI](http://sandmann.dotster.com/cwsdpmi/) DPMI host is licensed under **GPL**. The documentation states:
> The files in this binary distribution may be redistributed under the GPL (with source) or without the source code provided.

## 3dfx/Glide3
The code is licensed under "3DFX GLIDE Source Code General Public License".
Source code is available at https://github.com/SuperIlu/glide

## DZComm
DZComm serial library is release as **gift-ware**. See *readme.txt* in the dzcomm folder for details.

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

## zlib
[zlib](http://www.zlib.net/) is released under [zlib license](http://www.zlib.net/zlib_license.html).

## alpng
Copyright (c) 2006 Michal Molhanec

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented;
     you must not claim that you wrote the original software.
     If you use this software in a product, an acknowledgment
     in the product documentation would be appreciated but
     is not required.

  2. Altered source versions must be plainly marked as such,
     and must not be misrepresented as being the original software.

  3. This notice may not be removed or altered from any
     source distribution.

## zip code
See UNLICENSE in zip directory.

## WATTCP
See manual.txt in watt32 directory.

## OpenSSL
The OpenSSL toolkit stays under a double license, i.e. both the conditions of
the OpenSSL License and the original SSLeay license apply to the toolkit.

## cURL
See COPYING in curl directory.

## genann
See LICENSE in neural.dxelib.

## sqlite
See LICENSE.md in sqlite.dxelib.

## libcpuid
See COPYING in cpuid.dxelib.

## nanosvg
See LICENSE.txt in nanosvg.dxelib.

## noise
See LICENSE.md in noise.dxelib.

## nanojpeg
See nanojpeg.c in jpeg.dxelib.

## nanojpeg
See LICENSE in gifanim.dxelib.

# Usage
## Command line
```
Usage: DOjS.EXE [<flags>] <script> [script parameters]
    -r             : Do not invoke the editor, just run the script.
    -l             : Use 50-line mode in the editor.
    -w <width>     : Screen width: 320 or 640, Default: 640.
    -b <bpp>       : Bit per pixel:8, 16, 24, 32. Default: 32.
    -s             : No wave sound.
    -f             : No FM sound.
    -a             : Disable alpha (speeds up rendering).
    -x             : Allow raw disk write (CAUTION!)
    -t             : Disable TCP-stack
    -n             : Disable JSLOG.TXT.
    -j <file>      : Redirect JSLOG.TXT to <file>.
```

## dojs.ini
All command line options can also be provided in `dojs.ini` (which is read at startup from the current directory). See the included example for details.

## Editor keys
```
    F1        : Open/Close help
    SHIFT-F1  : Function context help
    F3        : Save script
    F4        : Run script
    F7        : Find text
    F9        : Show/Close logfile
    F10       : Quit

    Shift-F4       : Truncate logfile and run script
    Shift-F7       : Find again
    CTRL-D         : Delete current line
    SHIFT+Movement : Select text, releasing SHIFT deselects
    CTRL-C         : Copy selection
    CTRL-X         : Cut selection
    CTRL-V         : Paste
    CTRL-LEFT      : Previous word
    CTRL-RIGHT     : Next word
    CTRL/PAGE-UP   : One page up
    CTRL/PAGE-DOWN : One page down
    HOME           : Go to start of line
    END            : Go to end of line
    CTRL-HOME      : Go to start of line
    CTRL-END       : Go to end of line
    TAB            : Insert spaces until next TAB-stop at cursor
    SHIFT-TAB      : Reduce indentation of line

    TAB size is 4.
    The help viewer will remember the current position.

    The logfile can be truncated by pressing DEL in the log viewer.
```

## Scripts and resources
Scripts, as well as resources can either be stored in the file system or in ZIP files. To load data from a zip file the format is `<ZIP filename>=<ZIP entry name>` (e.g. `data.zip=mypic.bmp`). DOjS can be started with a script, a script in a ZIP file or no parameters. If the script was loaded from a ZIP file the running script can obtain resources from the same ZIP file by using `ZipPrefix()` to obtain paths refering to that ZIP. If the script was not started from a ZIP `ZipPrefix()` just passes through the file name (thus loading the file from HDD). If no arameters are supplied DOjS will first try to load `<name of the EXE>.ZIP=MAIN.JS` and then `JSBOOT.ZIP=MAIN.JS`.
Examples:
  * `DOJS.EXE -r script.js` will start `script.js` from the HDD, `ZipPrefix("pic.bmp")` will yield `pic.bmp`.
  * `DOJS.EXE -r data.zip=script.js` will start `script.js` from the ZIP file `data.zip`, `ZipPrefix("pic.bmp")` will yield `data.zip=pic.bmp`.
  * `HURTZ.EXE` DOjS was renamed to `HURTZ.EXE`. It will start `MAIN.JS` from the ZIP file `HURTZ.ZIP`, `ZipPrefix("pic.bmp")` will yield `HURTZ.ZIP=pic.bmp`.
  * `DOJS.EXE` The script was added to `JSBOOT.ZIP`. DOjS will start `MAIN.JS` from the ZIP file `JSBOOT.ZIP`, `ZipPrefix("pic.bmp")` will yield `JSBOOT.ZIP=pic.bmp`.

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
