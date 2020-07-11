                DZCOMM - Version 0.9.9 (WIP)

   A serial port add on to the Allegro game programming library.

               By Neil Townsend, Aug 9th, 2000.

		See the AUTHORS file for a
	       complete list of contributors.


#include <std_disclaimer.h>

   "I do not accept responsibility for any effects, adverse or otherwise, 
    that this code may have on you, your computer, your sanity, your dog, 
    and anything else that you can think of. Use it at your own risk."


======================================
============ Introduction ============
======================================


   Dzcomm is a cross-platform library intended for extending the
   Allegro graphics and gaming library to have easy access to RS-232
   ports. It was motivated by the availability of the wonderful djgpp
   compiler for DOS. Release 1.0 should mark dzcomm as multi-platform
   in line with the steps taken by Allegro.

   To find out more about Allergo, go to its website:
   http://www.talula.demon.co.uk/allegro/.

   Dzcomm was originally written by a chap called Dim whose initials
   (DZ) give us the name for this communications library.


=============================================
============ Supported platforms ============
=============================================

   For instructions on how to install Allegro, how to link your programs 
   with it, and any additional information specific to each of the supported 
   platforms, see one of the files:

   DOS/djgpp         - see readme.dj
   DOS/Watcom        - see readme.wat
   Windows/MSVC      - see readme.vc
   Windows/Mingw32   - see readme.mgw
   Windows/RSXNT     - see readme.rsx
   Linux             - see readme.lnx
   Unix              - see readme.uni
   BeOS              - see readme.be

   General API information can be found in allegro.txt, which is also 
   available in HTML, TexInfo, and RTF format in the docs directory.


==================================
============ Features ============
==================================

   Cross-platform support for DOS, Windows, Unix, and BeOS systems.
   BEING DONE



===================================
============ Copyright ============
===================================

   Dzcomm is gift-ware. It was created by a number of people working in 
   cooperation, and is given to you freely as a gift. You may use, modify, 
   redistribute, and generally hack it about in any way you like, and you do 
   not have to give us anything in return. However, if you like this product 
   you are encouraged to thank us by making a return gift to the dzcomm 
   community. This could be by writing an add-on package, providing a useful 
   bug report, making an improvement to the library, or perhaps just 
   releasing the sources of your program so that other people can learn from 
   them. If you redistribute parts of this code or make something using it,
   it would be nice if you mentioned dzcomm somewhere in the credits, but you 
   are not required to do this. We trust you not to abuse our generosity.


=======================================
============ Configuration ============
=======================================

   By default, dzcomm will assume that it is going to run alongside the allegro
   library. However, this version will still work without the allegro library.
   It is possible to compile a version which assumes that allegro is not going
   to be there. However, if you use this version alongside allegro you can
   expect problems. Details of how to configure this are given in the readme
   files listed above.


======================================
============ Contact info ============
======================================

   The latest version of dzcomm can always be found on the dzcomm 
   homepage, http://dzcomm.sourceforge.net/

   My personal address is neil@users.sourceforge.net, feel free to contact
   me about dzcomm issues.

   For the ever-shrinking minority of people without net access, my snail 
   address is 4 rue Sedillot, 45200, Montargis, France. Should
   using the post office be impossible, find your way to Montargis in France
   and head due south from the church in the central square down rue Gambetta.
   That road becomes my road.
