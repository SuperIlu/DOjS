#
#  Makefile for Waterloo TCP sample applications
#
#  Watcom Win32 executables.
#
#  Usage: 'wmake -h -f wc_win.mak'
#
STATIC  = 0
MODEL   = win32
CC      = *wcc386 -3r
CFLAGS  = -bt=nt -mf -oaxt -DWIN32
LFLAGS  = system nt

!if "$(STATIC)" == "1"
CFLAGS += -DWATT32_STATIC
LIBRARY = library ../lib/wattcpww.lib
!else
LIBRARY = library ../lib/wattcpww_imp.lib
!endif

BUILD_MESSAGE = Watcom/Win32 binaries done

!include wccommon.mak
