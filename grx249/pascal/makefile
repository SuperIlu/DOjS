# --------------------------------
# Check, if the paths are correct 
# --------------------------------

# Set the library-path to libgrx
GRXLIB = -L/usr/X11R6/lib

# Set the library-path to libjpeg and libtiff
GRAPHICSLIB = -L/usr/lib

# Compiler and options on your system  
COMPILER = $(PC) --automake -Wall

# --------------------------------------------
# I Think, you don't need to change from here
# --------------------------------------------

all: blt_test modetest keytest txt_test vir_test polytest jpgtest colortst imgview

blt_test:blt_test.pas
	$(COMPILER) $(GRXLIB) $(GRAPHICSLIB) blt_test.pas -oblt_test

modetest:modetest.pas
	$(COMPILER) $(GRXLIB) $(GRAPHICSLIB) modetest.pas -omodetest

keytest:	keytest.pas
	$(COMPILER) $(GRXLIB) $(GRAPHICSLIB) keytest.pas -okeytest

txt_test:txt_test.pas
	$(COMPILER) $(GRXLIB) $(GRAPHICSLIB) txt_test.pas -otxt_test

vir_test:vir_test.pas
	$(COMPILER) $(GRXLIB) $(GRAPHICSLIB) vir_test.pas -ovir_test

polytest:polytest.pas
	$(COMPILER) $(GRXLIB) $(GRAPHICSLIB) polytest.pas -opolytest

jpgtest:jpgtest.pas
	$(COMPILER) $(GRXLIB) $(GRAPHICSLIB) jpgtest.pas -ojpgtest

colortst:colortst.pas
	$(COMPILER) $(GRXLIB) $(GRAPHICSLIB) colortst.pas -ocolortst

imgview:imgview.pas
	$(COMPILER) $(GRXLIB) $(GRAPHICSLIB) imgview.pas -oimgview

clean:
	rm -f *.o *.gpi blt_test modetest keytest txt_test vir_test polytest jpgtest colortst imgview


