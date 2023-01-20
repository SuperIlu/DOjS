#
#  Makefile for Waterloo TCP sample applications
#
#  Borland PowerPak (32-bit prot-mode) executables.
#

.AUTODEPEND
.SWAP

DEBUG = -v
PACK  = 0

PROGS = ping.exe    popdump.exe rexec.exe   tcpinfo.exe cookie.exe  \
        daytime.exe dayserv.exe finger.exe  host.exe    lpq.exe     \
        lpr.exe     ntime.exe   ph.exe      stat.exe    htget.exe   \
        revip.exe   tracert.exe tcptalk.exe uname.exe   vlsm.exe    \
        whois.exe   blather.exe lister.exe

all:  $(PROGS)
      @echo PowerPak binaries done

.c.exe:
      bcc32 -c -WX -v -ls -RT- -O -I..\inc -I..\src $*.c
      tlink32 $(DEBUG) @&&|
         -s -c -ax -Tpe c0x32.obj $*.obj, $*.exe, $*.map, \
         ..\lib\wattcpbf.lib dpmi32.lib cw32.lib
|
!if "$(PACK)" == "1"
      tdstrp32 $*.exe
      upx -9 $*.exe
!endif
