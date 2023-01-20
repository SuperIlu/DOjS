#
# Common definitions and rules for all Watcom *.mak files in
# this directory; they contains a '!include wccommon.mak'.
#
#       exe             win32   flat    small32 large   small
!inject ping.exe        win32   flat    small32 large
!inject popdump.exe     win32   flat    small32 large
!inject rexec.exe       win32   flat    small32 large
!inject tcpinfo.exe     win32   flat    small32 large
!inject cookie.exe      win32   flat    small32 large   small
!inject daytime.exe     win32   flat    small32 large   small
!inject dayserv.exe     win32   flat    small32 large   small
!inject finger.exe      win32   flat    small32 large
!inject host.exe        win32   flat    small32 large   small
!inject lpq.exe         win32   flat    small32 large
!inject lpr.exe         win32   flat    small32 large
!inject ntime.exe       win32   flat    small32 large
!inject ph.exe          win32   flat    small32 large
!inject stat.exe        win32   flat    small32 large   small
!inject htget.exe       win32   flat    small32 large
!inject revip.exe       win32   flat    small32
!inject tcpport.exe                             large
!inject uname.exe       win32   flat    small32 large   small
!inject tracert.exe     win32   flat    small32
!inject whois.exe       win32   flat    small32 large
!inject blather.exe     win32   flat    small32
!inject lister.exe      win32   flat    small32
!inject ident.exe       win32
!inject vlsm.exe        win32   flat    small32
!inject con-test.exe    win32
!inject gui-test.exe    win32

PROGS = $+$($(MODEL))$-

!ifdef __UNIX__
RM = rm -f
!else
RM = del
!endif

COMMON_CFLAGS = -zq -wx -I"../inc" -fo=.obj -zm -s
DBG_CFLAGS = -d1

#
# Turn off these:
# wlink  Warning! W1027: file clib3r.lib(strerror.c): redefinition of strerror_ ignored
#
COMMON_LFLAGS = debug all option quiet, map, eliminate disable 1027

LINK = *wlink

all:  $(EXTRA_OBJ) $(PROGS) .SYMBOLIC
      @echo $(BUILD_MESSAGE)

.c.obj: .PRECIOUS
      $(CC) $(COMMON_CFLAGS) $(CFLAGS) $(DBG_CFLAGS) $[@

.c.exe: .PRECIOUS
      $(CC) $(COMMON_CFLAGS) $(CFLAGS) $(DBG_CFLAGS) $[@
      $(LINK) $(COMMON_LFLAGS) $(LFLAGS) name $^@ file {$[&.obj $(EXTRA_OBJ)} $(LIBRARY)
!ifdef EXTRA_EXE
      $(EXTRA_EXE)
!endif

con-test.exe: w32-test.c .PRECIOUS
      $(CC) $(COMMON_CFLAGS) $(CFLAGS) $(DBG_CFLAGS) $[@
      $(LINK) $(COMMON_LFLAGS) $(LFLAGS) runtime console name $^@ file $[&.obj $(LIBRARY)

gui-test.exe: w32-test.c .PRECIOUS
      $(CC) $(COMMON_CFLAGS) $(CFLAGS) -DIS_GUI $(DBG_CFLAGS) $[@
      $(LINK) $(COMMON_LFLAGS) $(LFLAGS) runtime windows name $^@ file $[&.obj $(LIBRARY)

clean: .SYMBOLIC
      @$(RM) *.obj
      @$(RM) *.map
      @$(RM) $(PROGS)
