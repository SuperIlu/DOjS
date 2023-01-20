#
#  Makefile for Waterloo TCP sample applications
#
#  LadSoft cc376 / valx / flat-model executables.
#  Doesn't work yet (LadSoft missed some functions in it's RTL).
#

CC     = cc386
CFLAGS = -I..\inc -I..\src +v -C+N
LINK   = valx
LFLAGS = -32 -map -nci -stack:20000

PROGS = rexec.exe   tcpinfo.exe cookie.exe  \
        daytime.exe dayserv.exe finger.exe  host.exe    \
        lpq.exe     lpr.exe     ntime.exe   ph.exe      \
        stat.exe    htget.exe   revip.exe   tracert.exe \
        uname.exe   vlsm.exe    whois.exe   blather.exe \
        lister.exe  ping.exe    ident.exe
      # popdump.exe

all:  $(PROGS)
      @echo LadSoft binaries done

.c.exe:
      $(CC) -c $(CFLAGS) $*.c
      nasm -s -f obj $*.asm
      del $*.asm
      $(LINK) $(LFLAGS) @&&|
        c0dos.obj $*.obj, $*.exe, $*.map, ..\lib\wattcplf.lib cldos.lib
|

clean:
      @del *.exe
      @del *.obj
      @del *.map

