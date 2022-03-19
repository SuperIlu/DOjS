DZCOMM

Watcom-specific information.

See readme.txt for a more general overview.



======================================
============ Watcom notes ============
======================================

   Status: Todo

   This library should work with Watcom C versions 10.6 or 11.0, but I have 
   no way to test anything but 10.6, and only minimal interest in 
   maintaining this port, so I'm afraid you are pretty much on your own if 
   you have problems with any different compiler versions.



===========================================
============ Required software ============
===========================================

   - Watcom C, version 10.6 or 11.0
   - djgpp compiler (djdev*.zip, gcc*b.zip, and bnu*b.zip).
   - GNU make (mak*b.zip).
   - sed (sed*b.zip).
   - Optional: rm (fil*b.zip). Used by the clean and uninstall targets.

   Except for the Watcom compiler itself, all of the above packages can be 
   downloaded from your nearest SimTel mirror site, in the 
   /pub/simtelnet/gnu/djgpp/ directory, or you can use the zip picker on 
   http://www.delorie.com/djgpp/. See the djgpp readme.1st file for 
   information about how to install djgpp.



============================================
============ Installing dzcomm =============
============================================

   This is a source-only distribution, so you will have to compile Allegro 
   before you can use it. To do this you should:

   Type "cd dzcomm", followed by "fixwat.bat", followed by "make". Then go 
   do something interesting while everything compiles. When it finishes 
   compiling, type "make install" to set the library up ready for use.

   The makefile will try to guess whether you are using Watcom 10.6 or 11.0 
   by checking for the presence of wdisasm.exe. If it gets this wrong, you 
   may need to override it by passing WATCOM_VERSION=10.6 or 
   WATCOM_VERSION=11 as arguments to make, or setting the WATCOM_VERSION 
   environment variable.

   If you also want to install a debugging version of the library (highly 
   recommended), now type "make install DEBUGMODE=1". Case is important, so 
   it must be DEBUGMODE, not debugmode!

   If you also want to install a profiling version of the library, now type 
   "make install PROFILEMODE=1".

   If your copy of dzcomm doesn't include the makefile.dep dependency files 
   (unlikely, unless you have run "make veryclean" at some point), you can 
   regenerate them by running "make depend".



=======================================
============ Using Allegro ============
=======================================

   All the Allegro functions, variables, and data structures are defined in 
   allegro.h. You should include this in your programs, and link with either 
   the optimised library alleg.lib, the debugging library alld.lib, or the 
   profiling library allp.lib. Programs that use Allegro must be compiled to 
   use the stack based calling convention (wcl386 option '-5s'), and with 
   stack overflow checks disabled (wcl386 option '-s'). You will also have 
   to increase the stack size from the miserly Watcom default, using a 
   wcl386 option like '-k128k', or a linker command like 'option stack=128k'.

