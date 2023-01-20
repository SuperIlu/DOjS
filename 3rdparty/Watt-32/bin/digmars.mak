#
#  Makefile for Waterloo TCP sample applications.
#  NB! Requires Borland's make tool
#
#  Digital Mars Compiler. Large or flat-model (Pharlap/X32VM) executables.
#  Set MODEL to 's'mall, 'l'large, 'x' for extended with X32VM extender or
#  'p' for Pharlap extended.
#

MODEL  = l
CC     = dmc
CFLAGS = -m$(MODEL) -I$(DIGMARS)\include -I..\inc -Jm -r -s

!if "$(MODEL)" == "s"
LFLAGS = -L/stack:25000 -L/map:full -L/DE
LIBS   = ..\lib\wattcpds.lib
!endif

!if "$(MODEL)" == "l"
LFLAGS = -L/stack:25000 -L/map:full -L/DE
LIBS   = ..\lib\wattcpdl.lib
!endif

!if "$(MODEL)" == "x"
LFLAGS = -L/stack:25000 -L/map:full -L/DE
LIBS   = ..\lib\wattcpdx.lib x32.lib
!endif

!if "$(MODEL)" == "p"
LIBS = ..\lib\wattcpdp.lib dosx32.lib #exc_dmc.lib
!endif

PROGS = ping.exe    popdump.exe rexec.exe   tcpinfo.exe \
        cookie.exe  daytime.exe dayserv.exe finger.exe  \
        host.exe    lpq.exe     lpr.exe     ntime.exe   \
        ph.exe      stat.exe    htget.exe   revip.exe   \
        tracert.exe uname.exe   vlsm.exe    whois.exe   \
        blather.exe lister.exe

all:  $(PROGS)
      @echo Digital Mars binaries done


!if "$(MODEL)" == "p"
.c.exe:
      $(CC) -c $*.c $(CFLAGS)
      386link $*.obj -exe $*.exe @&&|
        -lib $(LIBS) $(LFLAGS)
|

!else

.c.exe:
      $(CC) $*.c $(LIBS) @&&|
       $(CFLAGS) $(LIBS) $(LFLAGS)
|
!endif

clean:
      @del *.exe
      @del *.obj

