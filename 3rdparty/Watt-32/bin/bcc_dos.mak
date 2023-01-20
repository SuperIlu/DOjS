#
#  Makefile for Waterloo TCP sample applications
#
#  Borland (real-mode) executables.
#

.AUTODEPEND
.SWAP

#
# Change this to 's' for small (good luck linking that).
#          or to 'f' for flat. Also change 'bcc' to 'bcc32' below.
#
MODEL  = l
CFLAGS = -m$(MODEL) -v -ls -f87 -I..\inc -I..\src -L ..\lib\wattcpb$(MODEL).lib

PROGS  = ping.exe    popdump.exe rexec.exe   tcpinfo.exe cookie.exe  \
         daytime.exe dayserv.exe finger.exe  host.exe    lpq.exe     \
         lpr.exe     ntime.exe   ph.exe      stat.exe    htget.exe   \
         tcptalk.exe uname.exe   whois.exe   tcpport.exe             \
       # blather.exe lister.exe  tracert.exe vlsm.exe    revip.exe

all:  $(PROGS)
      @echo Real-mode binaries done

.c.exe:
      bcc $(CFLAGS) $*.c

clean:
        @del *.obj
        @del *.map
love:
        @echo not war!
