		Printing from GRX  

This is source of beta version of printing procedures for GRX.
These procedures are based on sources of printer BGI drivers
for Borland C++ and Pascal compilers. This BGI driver was 
developed by Ullrich von Bassevitz (see copying.uz).

Only part of sources of printer BGI driver are used. I didn't port 
drawing functions from BGI driver as they are already implemented in GRX.
I took only printing part which is now rather heavily modified to get 
rid of Borland C++ specific features (e.g. inline assembler).

Current version is tested with DJGPP and Linux versions of GRX only. 
I didn't even try to compile it with Borland C++ for real mode as 
I think it is useless due to lack of memory needed for buffer where
to create image. To print from GRX under Linux one should install
printer filter that allows to send PCL output to printer.

Only some modes are tested:
	Epson LQ printer : 180x180 dpi
        LaserJet 4L : 300x300 dpi (with and without compression)
	
I also tried DeskJet 500C mode (300x300 dpi with separate black)
on DeskJet 690C and it worked.

Printing code is linked into executable only when it is really required.
 
Currently it's included as addon to GRX. 

--------------------  Files  -------------------------------------------
grxprint.c	- main sources of printing code
grxprint.h      - interface definitions for user
prndata.c       - printer definitions 
grxprn00.h      - definitions used internally by grxprint only
printest.c      - test example
copying.uz      - original copyright notice from Ullrich von Bassevitz
printer.doc     - original docs on printer BGI driver
------------------------------------------------------------------------

NOTE:   Ullrich von Bassevitz is no more maintaining printer BGI driver.
        Addresses mentioned in printer.doc are NO MORE USABLE


Andris Pavenis        
e-mail: pavenis@latnet.lv
