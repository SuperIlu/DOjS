#
#  Makefile for Waterloo TCP sample applications
#
#  Borland's bcc32 + WDOSX executables.
#
#  Note 1: c0x32.obj and import32.lib is part of PowerPak installation.
#          Should be in <bcc-root>\lib.
#
#  Note 2: stubit.exe is part of WDOSX.
#

COMPILE = bcc32 -c -v -O -a -d -f- -Z -Tt -I..\inc

LINK    = tlink32
LFLAGS  = -Tpe -m -Gm -c -s -v -n -aa

PROGS = ping.exe    popdump.exe rexec.exe  tcpinfo.exe cookie.exe \
        daytime.exe dayserv.exe finger.exe host.exe    lpq.exe    \
        lpr.exe     ntime.exe   ph.exe     stat.exe    htget.exe  \
        revip.exe   tracert.exe uname.exe  vlsm.exe    whois.exe  \
        blather.exe lister.exe


all:  $(PROGS)
      @echo BCC32/WDOSX binaries done

.c.exe:
      $(COMPILE) $*.c
      $(LINK) @&&|
        c0x32.obj $*.obj, $*.exe, $*.map
        ..\lib\wattcpbf.lib import32.lib cw32.lib $(LFLAGS)
|
      stubit $*.exe
      del $*.bak

clean:
      @del *.obj
      @del *.map

