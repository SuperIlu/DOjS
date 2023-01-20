#
#  Makefile for Waterloo TCP sample applications
#
#  Watcom386 + WDOSX executables
#
#  Note: stubit.exe is part of WDOSX
#

MODEL   = flat
CC      = *wcc386 -3r
CFLAGS  = -bt=dos -mf -oaxt -DWATT32_STATIC
LFLAGS  = system wdosx option stack=50k
LIBRARY = library ../lib/wattcpwf.lib
EXTRA_EXE = @%make bind_exe

BUILD_MESSAGE = Watcom386/WDOSX binaries done

!include wccommon.mak

bind_exe: .procedure
      stubit $^@
      del $^&.bak
