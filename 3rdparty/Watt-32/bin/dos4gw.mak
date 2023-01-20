#
#  Makefile for Waterloo TCP sample applications
#
#  Watcom386/DOS4GW executables.
#

MODEL   = flat
CC      = *wcc386 -3r
CFLAGS  = -bt=dos -mf -oaxt -DWATT32_STATIC
LFLAGS  = system dos4g option stack=50k
LIBRARY = library ../lib/wattcpwf.lib

BUILD_MESSAGE = Watcom386/DOS4GW binaries done

!include wccommon.mak
