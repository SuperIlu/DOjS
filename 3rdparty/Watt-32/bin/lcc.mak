#
#  Makefile for Waterloo TCP sample applications
#
#  lcc-win32 executables.
#

CC      = lcc
CFLAGS  = -g2 -A -O -I..\inc -I..\src -DWATT32_STATIC
WATTLIB = ..\lib\wattcp_lcc.lib
LIBS    = $(WATTLIB) advapi32.lib

PROGRAMS = popdump.exe rexec.exe   tcpinfo.exe cookie.exe \
           daytime.exe dayserv.exe finger.exe  host.exe   \
           lpq.exe     lpr.exe     ntime.exe   ph.exe     \
           stat.exe    htget.exe   revip.exe   vlsm.exe   \
           whois.exe   ping.exe    ident.exe   country.exe

all:  $(PROGRAMS)
      @echo lcc-win32 binaries done

$(PROGRAMS): $(WATTLIB)

.c.exe:
      $(CC) -c $(CFLAGS) $*.c
      lcclnk -map $*.map -o $*.exe $*.obj $(LIBS)

clean:
      - @del *.exe
      - @del *.map
      - @del *.obj

