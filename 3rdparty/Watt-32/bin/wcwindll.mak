#
#  Makefile for Waterloo TCP sample applications
#
#  Watcom Win32 executables.
#
#  Usage: 'wmake -h -f wcwindll.mak'
#

MODEL   = win32
CC      = *wcc386 -3r
CFLAGS  = -bt=nt -mf -oaxt -DWIN32
LFLAGS  = system nt
LIBRARY = library ../lib/wattcpww_imp.lib

BUILD_MESSAGE = Watcom/Win32 binaries (DLL) done

!include wccommon.mak
