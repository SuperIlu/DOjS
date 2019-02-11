##################
## Include Directories

!ifndef %INCLUDE
## DEFAULT
WATCOM_HEADER_DIR=C:\WATCOM\H
!else
WATCOM_HEADER_DIR=$(%INCLUDE)
!endif


##################
## Locations

GRX_LIB_SUBDIR=watcom32
GRX_BIN_SUBDIR=bin

##################
## Binaries

CC = wcc386
LIB = wlib
LINK = wlink

##################
## Binary Flags and Options

!ifdef DEBUG

##################
## DEBUG FLAGS
CC_OPTS = -i=$(WATCOM_HEADER_DIR);.\include;.\src\include;.\src;.\addons\print;.\addons\bmp -w4 &
-e25 -d__MSDOS__ -dSMALL_STACK -dLFB_BY_NEAR_POINTER -dUSE_WATCOM386_ASM -dDEBUG=0x7800 -zq -od -d2 -5r &
-bt=dos -mf
LIB_OPTS = -b -c -n -q -p=512
LINK_OPTS = d all SYS dos4g op inc op m op maxe=25 op q op symf

!else

##################
## RELEASE FLAGS
CC_OPTS = -i=$(WATCOM_HEADER_DIR);.\include;.\src\include;.\src;.\addons\print;.\addons\bmp -w4 &
-e25 -d__MSDOS__ -dSMALL_STACK -dLFB_BY_NEAR_POINTER -dUSE_WATCOM386_ASM -zq -otexan -d1 -5r &
-bt=dos -mf
LIB_OPTS = -b -c -n -q -p=512
LINK_OPTS = SYS dos4g op inc op m op maxe=25 op q op symf

!endif

##################
## Targets

GRXVERSION = 229
GRXLIB = lib\$(GRX_LIB_SUBDIR)\grx$(GRXVERSION).lib
GRXLINK = wat32mak.lb1
GRXTESTS = $(GRX_BIN_SUBDIR)\modetest.exe &
$(GRX_BIN_SUBDIR)\arctest.exe &
$(GRX_BIN_SUBDIR)\blittest.exe &
$(GRX_BIN_SUBDIR)\circtest.exe &
$(GRX_BIN_SUBDIR)\cliptest.exe &
$(GRX_BIN_SUBDIR)\colorops.exe &
$(GRX_BIN_SUBDIR)\curstest.exe &
$(GRX_BIN_SUBDIR)\fonttest.exe &
$(GRX_BIN_SUBDIR)\imgtest.exe &
$(GRX_BIN_SUBDIR)\fnt2c.exe &
$(GRX_BIN_SUBDIR)\fnt2text.exe &
$(GRX_BIN_SUBDIR)\keys.exe &
$(GRX_BIN_SUBDIR)\life.exe &
$(GRX_BIN_SUBDIR)\linetest.exe &
$(GRX_BIN_SUBDIR)\mousetst.exe &
$(GRX_BIN_SUBDIR)\pcirctst.exe &
$(GRX_BIN_SUBDIR)\polytest.exe &
$(GRX_BIN_SUBDIR)\rgbtest.exe &
$(GRX_BIN_SUBDIR)\scroltst.exe &
$(GRX_BIN_SUBDIR)\speedtst.exe &
$(GRX_BIN_SUBDIR)\textpatt.exe &
$(GRX_BIN_SUBDIR)\winclip.exe &
$(GRX_BIN_SUBDIR)\wintest.exe &
$(GRX_BIN_SUBDIR)\arctest.dat &
$(GRX_BIN_SUBDIR)\polytest.dat



##################
## Rules

all : lib tests .SYMBOLIC

lib : $(GRXLIB) .SYMBOLIC

tests : $(GRXTESTS) .SYMBOLIC


clean : .SYMBOLIC
 @del *.obj
 @del *.err
 @del *.lb1
 @del *.lk1
 @del *.map
 @del $(GRX_BIN_SUBDIR)\*.ilk
 @del $(GRX_BIN_SUBDIR)\*.sym

!include .\src\makefile.wat

!include .\test\makefile.wat
