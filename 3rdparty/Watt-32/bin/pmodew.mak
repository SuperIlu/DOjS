#
#  Makefile for Waterloo TCP sample applications
#
#  Watcom386 + Pmode/W executables.
#

MODEL   = flat
CC      = *wcc386 -3r
CFLAGS  = -bt=dos -mf -oaxt -DWATT32_STATIC
LFLAGS  = system pmodew option stack=50k
LIBRARY = library ../lib/wattcpwf.lib

BUILD_MESSAGE = Watcom386/PmodeW binaries done

!include wccommon.mak
