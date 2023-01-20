#
#  Makefile for Waterloo TCP sample applications
#
#  PellesC Win32/Win64 executables.
#

# The below  '$(CPU)' can be set on cmd-line:
#   pomake CPU=x64 pellesc.mak
#
# to force the 64-bit version of the Watt-32 library.
#
!if "$(CPU)." == "."
CPU = x86
!endif

#
# Set to 1 to link using static ..\lib\$(CPU)\wattcppo.lib.
#
STATIC_LIB = 0

CC     = pocc
CFLAGS = -MT -Ox -Ze -Zi -W1 -Go -T$(CPU)-coff -X -DWIN32 \
         -I..\inc -I..\src -I$(PELLESC)\include -I$(PELLESC)\include\win

!if "$(CPU)." == "x64."
WINLIB   = $(PELLESC)\lib\Win64
OLDNAMES = oldnames64.lib
!else
WINLIB   = $(PELLESC)\lib\Win
OLDNAMES = oldnames.lib
!endif

!if "$(STATIC_LIB)" == "1"
CFLAGS = $(CFLAGS) -DWATT32_STATIC
LIBS   = ..\lib\$(CPU)\wattcppo.lib user32.lib kernel32.lib advapi32.lib
!else
LIBS   = ..\lib\$(CPU)\wattcppo_imp.lib
!endif

LINK    = polink
LDFLAGS = -map -libpath:$(PELLESC)\lib -libpath:$(WINLIB) \
          -machine:$(CPU) -subsystem:console -debug -verbose

LIBS = $(OLDNAMES) $(LIBS)

PROGS = rexec.exe   tcpinfo.exe cookie.exe  \
        daytime.exe dayserv.exe finger.exe  host.exe    \
        lpq.exe     lpr.exe     ntime.exe   ph.exe      \
        stat.exe    htget.exe   revip.exe   tracert.exe \
        uname.exe   vlsm.exe    whois.exe   blather.exe \
        lister.exe  ping.exe    ident.exe
      # popdump.exe

all:  $(PROGS)
	@del link.tmp
	@echo PellesC binaries done ($$(CPU)=$(CPU)).

.c.exe:
	$(CC) -c $(CFLAGS) $*.c
	$(LINK) $(LDFLAGS) -out:$*.exe -map:$*.map $*.obj $(LIBS) > link.tmp
	type link.tmp >> $*.map

.c.i:
	$(CC) -P $(CFLAGS) $*.c

clean:
	@del *.exe
	@del *.obj
	@del *.map

