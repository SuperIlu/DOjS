#
# Common make rules for Watt-32 programs except djgpp versions.
# Used only by Makefiles under .\bin\ (not in .\bin\ itself).
# These Makefiles requires a Borland compatible make tool
# (maker.exe for CBuilder or plain old Borland works best).
#
# For djgpp targets look at "djcommon.mak"
#
# For Watcom386 DOS-targets one of these DOSX-extenders must be defined:
#
#WDOSX_EXTENDER    = 1
#PHARLAP_EXTENDER  = 1
#PMODEW_EXTENDER   = 1
#DOS4GW_EXTENDER   = 1
#CAUSEWAY_EXTENDER = 1
 DOS32A_EXTENDER   = 1
#ZRDX_EXTENDER     = 1

#
# To build using BCC32 and PowerPak, define "POWERPAK=1" before
# including this file. You need bcc32 v4.5+ to use PowerPak.
#

#
# Watcom call convention; either '-3/4/5/6r' (register calls) or
# '-3/4/5/6s' (stack convention). 'r' or 's' must match option that
# wattcpwf.lib was built with.
#
!if !$d(CALLING_CONVENTION)
CALLING_CONVENTION = -3r
!endif

!if !$d(SRC)
!error SRC not defined. Define SRC = C-files.. before including COMMON.MAK
!endif

!if !$d(BORLAND_EXE)
!error BORLAND_EXE not defined.
!endif

!if !$d(PHARLAP_EXP)
!error PHARLAP_EXP not defined.
!endif

!if !$d(WATCOM_EXE)
!error WATCOM_EXE not defined.
!endif

#########################################################################

.AUTODEPEND
.SWAP

MMODEL        = l
BCC_CFLAGS    = -c -m$(MMODEL) -v -O -f87 -H=$(TEMP)\bcc.sym
BCC32_CFLAGS  = -c -WX -v -ls -RT- -O -w-aus -w-stu
WCC386_CFLAGS = -mf -w5 -zc -zq -zm -fr=nul -bt=dos -d2 -fpi -oilrt $(CALLING_CONVENTION)
HC386_CFLAGS  = -w4 -c -g -O2 -586 -Hsuffix=.o32 -Hnocopyr -Hturboerr \
                -Hpragma=stack_size_warn(5000) -Hpragma=Offwarn(572)

INC = $(WATT_ROOT)\inc
LIB = $(WATT_ROOT)\lib

#
# PHARLAP is only used for targets using Pharlap DOS-extender.
# You MUST define this in your environment. Set to root of Pharlap
# installation.
#
# e.g. "set PHARLAP=c:\pharlap"

BORLAND_OBJ = $(SRC:.c=.obj)
HIGHC_OBJ   = $(SRC:.c=.o32)
WATCOM_OBJ  = $(SRC:.c=.wo)

EXES = $(BORLAND_EXE) $(PHARLAP_EXP) $(WATCOM_EXE)

all:  $(EXES)


#
# Borland C (large/flat model) target
#
!if "$(POWERPAK)" == "1"
$(BORLAND_EXE): bcc32.arg $(BORLAND_OBJ) $(LIB)\wattcpbf.lib
                tlink32 @&&|
                  -s -v -c -ax c0x32.obj $(BORLAND_OBJ), $*.exe, $*.map, \
                  $(LIB)\wattcpbf.lib dpmi32.lib cw32.lib
|
!else
$(BORLAND_EXE): bcc.arg $(BORLAND_OBJ) $(LIB)\wattcpb$(MMODEL).lib
                bcc -e$(BORLAND_EXE) @&&|
                  -ml -v -ls -L $(LIB)\wattcpb$(MMODEL).lib $(BORLAND_OBJ)
|
!endif

#
# Metaware High-C / Pharlap target
#
$(PHARLAP_EXP): hc386.arg $(HIGHC_OBJ) $(LIB)\wattcphf.lib
                386link -exe $(PHARLAP_EXP) @&&|
                  $(HIGHC_OBJ)
                  -lib $(LIB)\wattcphf.lib $(EXTRA_HCLIB)
                  -lib hc386, hc387, hcna, dosx32, dos32, exc_hc
                  -symbols -offset 1000h -stack 50000h -386 -twocase
                  -nostub -unprivileged -fullwarn -maxdata 0 -fullseg
                  -publist both -purge none *
                  -mapnames 30 -mapwidth 132 -pack
                # -attributes class code er
                # -attributes class data rw
                # -attributes class stack rw
|


#
# Watcom386 / DOS4GW target
#
$(WATCOM_EXE): wcc386.arg $(WATCOM_OBJ) wlink.arg $(LIB)\wattcpwf.lib
               wlink @wlink.arg
!if "$(WDOSX_EXTENDER)" == "1"
               stubit $(WATCOM_EXE)
!endif

wlink.arg: Makefile $(WATT_ROOT)\bin\common.mak
           @copy &&|
!if "$(DOS4GW_EXTENDER)" == "1"
             system dos4g
!endif
!if "$(CAUSEWAY_EXTENDER)" == "1"
             system causeway
!endif
!if "$(ZRDX_EXTENDER)" == "1"
             system zrdx
!endif
!if "$(DOS32A_EXTENDER)" == "1"
             system dos32a
             option stub=$(WATT_ROOT)\bin\dos32a.exe
!endif
!if "$(PMODEW_EXTENDER)" == "1"
             system pmodew
             option stub=$(WATT_ROOT)\bin\pmodew.exe
!endif
             option quiet, map, verbose, caseexact, stack=100k, eliminate
             debug codeview all
             file { $(WATCOM_OBJ) }
!if "$(PHARLAP_EXTENDER)" == "1"
             system pharlap
             option offset=0x1000, stub=$(PHARLAP)\bin\gotnt.exe
             runtime unpriv, minreal=0x1000
             libpath $(PHARLAP)\lib
             library dosx32, exc_wc, $(LIB)\wattcpwf
!else
             library $(LIB)\wattcpwf.lib

!endif
!if "$(EXTRA_WCLIB)" != ""
             library $(EXTRA_WCLIB)
!endif
             name $(WATCOM_EXE)
| $<



#
# .c -> object rules
#

!if "$(POWERPAK)" == "1"
.c.obj:
           bcc32 @bcc32.arg $*.c
!else
.c.obj:
           bcc @bcc.arg $*.c
!endif

.c.o32:
           hc386 @hc386.arg $*.c

.c.wo:
           wcc386 @wcc386.arg $*.c -fo=$*.wo

bcc.arg:   $(PREREQUISITES) Makefile $(WATT_ROOT)\bin\common.mak
           @copy &&|
             $(COMMON_DEFS) -DWATT32 -I$(INC)
             $(BCC_CFLAGS)
             $(BCC_DEFS)
| $<

bcc32.arg: $(PREREQUISITES) Makefile $(WATT_ROOT)\bin\common.mak
           @copy &&|
             $(COMMON_DEFS) -DWATT32 -I$(INC)
             $(BCC32_CFLAGS)
             $(BCC_DEFS)
| $<

hc386.arg: $(PREREQUISITES) Makefile $(WATT_ROOT)\bin\common.mak
           @copy &&|
             $(COMMON_DEFS) -DWATT32 -I$(INC)
             $(HC386_CFLAGS)
             $(HC386_DEFS)
| $<

wcc386.arg: $(PREREQUISITES) Makefile $(WATT_ROOT)\bin\common.mak
           @copy &&|
             $(COMMON_DEFS) -DWATT32 -I$(INC)
             $(WCC386_CFLAGS)
             $(WCC386_DEFS)
!if "$(PHARLAP_EXTENDER)" == "1"
             -I$(PHARLAP)\inc
!endif
| $<

clean:
           @del *.obj
           @del *.o32
           @del *.wo
           @del *.map
           @del bcc.arg
           @del bcc32.arg
           @del hc386.arg
           @del wcc386.arg
           @del wlink.arg
           @del make*.@@@

vclean veryclean scrub: clean
           @del $(BORLAND_EXE)
           @del $(PHARLAP_EXP)
           @del $(WATCOM_EXE)


