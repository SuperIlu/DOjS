#
#  Makefile for Waterloo TCP sample applications
#
#  Digital Mars + WDOSX executables.
#
#  Note: stubit.exe is part of WDOSX
#

CC     = dmc
CFLAGS = -mx -g -Jm -I..\inc

PROGS = ping.exe    popdump.exe rexec.exe  tcpinfo.exe cookie.exe \
        daytime.exe dayserv.exe finger.exe host.exe    lpq.exe    \
        lpr.exe     ntime.exe   ph.exe     stat.exe    htget.exe  \
        revip.exe   tracert.exe uname.exe  vlsm.exe    whois.exe  \
        blather.exe lister.exe


all:  $(PROGS)
      @echo DMC/WDOSX binaries done

.c.exe:
      $(CC) $*.c ..\lib\wattcpdf.lib x32.lib $(CFLAGS) -L/stack:20000
      stubit $*.exe
      del $*.bak

clean:
      @del *.obj
      @del *.map
      @del *.exe

