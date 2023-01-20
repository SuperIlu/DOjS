#
# Makefile for Waterloo TCP sample applications
#
# Borland / CBuilder / WIN32 executables.
#
.AUTODEPEND

#
# Set to 1 to link using static wattcpbw.lib.
#
STATIC_LIB = 0

#
# See comments in ../src/Makefile.all (BORLAND section) for this:
#
!if "$(CBUILDER_IS_LLVM_BASED)" == ""
  CC     = $(BCCDIR)\bin\bcc32
  CFLAGS = -WC -v
  C_INCLUDE_PATH =  # Prevent it picking up any MinGW/gcc headers
!else
  CC     = $(BCCDIR)\bin\bcc32c
  CFLAGS = -q -lq -Xdriver -Wno-format-security
!endif

!if "$(STATIC_LIB)" == "1"
  CFLAGS   = $(CFLAGS) -DWATT32_STATIC
  WATT_LIB = ..\lib\wattcpbw.lib
!else
  WATT_LIB = ..\lib\wattcpbw_imp.lib
!endif

CFLAGS = $(CFLAGS) -a- -I..\inc -I..\src

PROGRAMS = ping.exe    popdump.exe rexec.exe   tcpinfo.exe  cookie.exe  \
           daytime.exe dayserv.exe finger.exe  host.exe     lpq.exe     \
           lpr.exe     ntime.exe   ph.exe      stat.exe     htget.exe   \
           tcptalk.exe uname.exe   whois.exe   blather.exe  lister.exe  \
           tracert.exe vlsm.exe    revip.exe   con-test.exe gui-test.exe

all: $(PROGRAMS)
     @echo Borland/Win32 binaries done

$(PROGRAMS): $(WATT_LIB) bcc_win.mak

gui-test.exe: w32-test.c
      $(CC) -o gui-test.exe $(CFLAGS) -DIS_GUI=1 w32-test.c -lsubsystem:windows -l $(WATT_LIB)

con-test.exe: w32-test.c
      $(CC) -o con-test.exe $(CFLAGS) w32-test.c -l $(WATT_LIB)

tracert.exe: tracert.c geoip.c IP2Location.c
      $(CC) -o tracert.exe $(CFLAGS) -DUSE_IP2LOCATION -DIS_WATT32 -l $(WATT_LIB) tracert.c geoip.c IP2Location.c

.c.exe:
     $(CC) -o $*.exe $(CFLAGS) $*.c -l $(WATT_LIB)

clean:
     @del *.obj
     @del *.map
     @del *.tds

