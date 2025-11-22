Allegro 4.2.2 (DJGPP/DOS cross compiling)
=========================================

A fork of the old Allegro 4.2.2 release from July 2007, with a few minor modifications to the source to facilitate cross compiling with DJGPP.

This fork exists so that you can use Allegro to write DOS games with a version of DJGPP for a non-DOS, non-Windows system. *If you don't intend specifically to build DOS games, you should use the [latest release](http://liballeg.org/) instead!*

Originally, this version of Allegro was intended to be used with GCC 2.91.N. Some modifications have been made to make it work with GCC 5.2.0 as provided by DJGPP.

If you must compile this for a target other than DJGPP, you must edit [/include/allegro/platform/alplatf.h](https://github.com/msikma/allegro-4.2.2-xc/blob/master/include/allegro/platform/alplatf.h) and add the correct platform setting.

Usage
-----

The first step is to get a working DJGPP compiler, as per [andrewwutw's build-djgpp](https://github.com/andrewwutw/build-djgpp) instructions. This should work for Mac OS X, Linux and FreeBSD, but I've only tested it with Mac OS X. Please let me know if you can verify that this works for other OSes.

### Compiling

If you're on Mac OS X, and you've set up DJGPP correctly, it should work out of the box. On other platforms, you'll need to edit `xmake.sh` to set the correct paths.

Run `./xmake.sh lib` to compile the library. No standard make command will work.

If all goes well, a `lib/djgpp/liballeg.a` file will be generated that you can link with.

### Targeting Windows

If you're targeting Windows, you should probably not use this. Just use MinGW instead. It should be [easy to compile something that works on both Windows 10 and DOS](https://twitter.com/Sosowski/status/730563851389964293).

Patches
-------

Aside from the modifications for easier cross-compiling, the following changes were made:

* [#1 - Fix broken 8-bit Sound Blaster volume](https://github.com/msikma/allegro-4.2.2-xc/pull/1) - this was a longstanding bug with the 4.2.2 source
* [#2 - Add `-fgnu89-inline` to `asmdef.exe` target to facilitate GCC 5+](https://github.com/msikma/allegro-4.2.2-xc/pull/2)

Example
-------

There's an [Allegro DOS example](https://github.com/msikma/allegro-dos-example) repository available with a small Hello World program.

Copyright
---------

Allegro 4.2.2 is [giftware licensed](http://liballeg.org/license.html). My own modifications to the source are public domain. This repository was forked off the [liballeg/allegro5](https://github.com/liballeg/allegro5) repository, with all commit history past 2007-07-22 removed.
