#
#  A 'wmake' makefile for Waterloo TCP sample applications
#
#  Watcom386/X32VM executables.
#
SYSTEM = x32vm
SYSTEM = x32sv

COMPILE = *wcc386 -mf -3s -w5 -d2 -zq -zm -of -I..\inc -fr=nul -bt=dos
LINK    = *wlink option quiet, map, verbose, eliminate, caseexact, stack=50k &
             debug all system $(SYSTEM)

LINK    += option stub=$(%DIGMARS)\lib\zlx.lod

COMPILE += -D_NEED__x32_stacksize

#
# Allthough 'Digital Mars' is no longer supported, it's 'x32.lib' is needed here.
#
LIBRARY = library ..\lib\wattcpwf.lib, $(%DIGMARS)\lib\x32.lib

PROGS = ping.exe    popdump.exe rexec.exe   tcpinfo.exe cookie.exe  &
        daytime.exe dayserv.exe finger.exe  host.exe    lpq.exe     &
        lpr.exe     ntime.exe   ph.exe      stat.exe    htget.exe   &
        revip.exe   tracert.exe uname.exe   vlsm.exe    whois.exe   &
        blather.exe lister.exe


all: $(PROGS) .SYMBOLIC
     @echo Watcom386/X32VM binaries done

.c.exe: .PRECIOUS
     $(COMPILE) $*.c
     $(COMPILE) x32_stk.c
     $(LINK) name $*.exe file $*.obj, x32_stk.obj $(LIBRARY)

clean: .SYMBOLIC
     - rm -f *.obj
     - rm -f $(PROGS:.exe=.map)

vclean: clean .SYMBOLIC
     - rm -f $(PROGS)


