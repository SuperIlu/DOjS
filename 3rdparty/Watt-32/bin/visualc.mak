#
#  Makefile for Waterloo TCP sample applications
#
#  Microsoft Visual-C / Win32 executables.
#

#
# Build using static library (1) wattcpvc.lib or use the watt-32.dll
# via the import library wattcpvc_imp.lib (0).
#
STATIC_LIB = 0

#
# Add support for Geo-location in the 'tracert.c' program:
#   GEOIP_LIB = 2 ==> compile with 'ip2location.c'
#   GEOIP_LIB = 1 ==> compile with 'geoip.c'
#   GEOIP_LIB = 0 ==> compile with neither.
#
GEOIP_LIB = 2

#
# Debug-mode or release-mode libraries are used. Make sure wattcp*.lib
# was compiled with the same configuration. If Watt-32 libs where built
# with "-M?d" and "-D_DEBUG" you should set DEBUG_MODE = 1.
#
# These configurations are possible:
#
#  Option Runtime      Thread-safe Debug/release
#  -------------------------------------------------
#  -MD    msvcrt.dll   Yes         release (normal)
#  -MDd   msvcrtd.dll  Yes         debug
#  -ML    libc.lib     No          release
#  -MLd   libcd.lib    No          debug
#  -MT    libcmt.lib   Yes         release
#  -MTd   libcmtd.lib  Yes         debug
#
# The below  '$(CPU)' can be set on cmd-line:
#   nmake CPU=x64 visualc.mak
#
# to force the 64-bit version of the Watt-32 library.
#
!if "$(CPU)." == "."
CPU = x86
!endif

# Build with DEBUG_MODE=1 if you want to profile the program with
# .\vcprof.bat.
#
!if "$(DEBUG_MODE)." == "."
DEBUG_MODE = 0
!endif

CC     = cl
CFLAGS = -nologo -DWIN32 -EHsc -W3 -Gy -Zi -I..\inc \
         -D_CRT_NONSTDC_NO_WARNINGS -D_CRT_OBSOLETE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS \
       # -DUNICODE -D_UNICODE

LDFLAGS = -nologo -fixed:no -mapinfo:exports -incremental:no -machine:$(CPU)

!if $(DEBUG_MODE) == 1
DEBUG   = _d
CFLAGS  = $(CFLAGS) -MDd -Os -D_DEBUG -RTCc -GS -GF # -Gr
LDFLAGS = $(LDFLAGS) -debug
!else
DEBUG   =
CFLAGS  = $(CFLAGS) -MD -Ot -GS # -Gr
LDFLAGS = $(LDFLAGS) -debug     #-release
!endif

!if $(STATIC_LIB) == 1
CFLAGS  = $(CFLAGS) -DWATT32_STATIC
WATTLIB = ..\lib\$(CPU)\wattcpvc$(DEBUG).lib
LIBS    = $(WATTLIB) advapi32.lib user32.lib

!else
WATTLIB = ..\lib\$(CPU)\wattcpvc_imp$(DEBUG).lib
LIBS    = $(WATTLIB)
!endif

!if "$(CPU)." == "x64."
CFLAGS = $(CFLAGS) -wd4244 -wd4267
!endif

PROGRAMS = popdump.exe  rexec.exe    tcpinfo.exe cookie.exe  \
           daytime.exe  dayserv.exe  finger.exe  host.exe    \
           lpq.exe      lpr.exe      ntime.exe   ph.exe      \
           stat.exe     htget.exe    revip.exe   vlsm.exe    \
           whois.exe    ping.exe     ident.exe   country.exe \
           tracert.exe  con-test.exe gui-test.exe

all:  $(PROGRAMS)
!if $(DEBUG_MODE) == 1
      @echo Visual-C binaries (debug) done
!else
      @echo Visual-C binaries (release) done
!endif

$(PROGRAMS): $(WATTLIB) visualc.mak

gui-test.exe: w32-test.c # tmp.res
      $(CC) -c $(CFLAGS) -DIS_GUI=1 -Fogui-test.obj w32-test.c
      link $(LDFLAGS) -subsystem:windows -map:$*.map -out:$*.exe gui-test.obj $(LIBS)

con-test.exe: w32-test.c # tmp.res
      $(CC) -c $(CFLAGS) -Focon-test.obj w32-test.c
      link $(LDFLAGS) -subsystem:console -map:$*.map -out:$*.exe con-test.obj $(LIBS)

TRACERT_CFLAGS = $(CFLAGS) -DIS_WATT32 # -DPROBE_PROTOCOL=IPPROTO_TCP

TRACERT_CFLAGS = $(TRACERT_CFLAGS) -DUNICODE -D_UNICODE

!if "$(GEOIP_LIB)" == "1"
TRACERT_CFLAGS = $(TRACERT_CFLAGS) -DUSE_GEOIP
!endif

!if "$(GEOIP_LIB)" == "2"
TRACERT_CFLAGS = $(TRACERT_CFLAGS) -DUSE_IP2LOCATION
!endif

tracert.exe: tracert.c geoip.c geoip.h ip2location.c ip2location.h # tmp.res
      $(CC) -c $(TRACERT_CFLAGS) tracert.c geoip.c ip2location.c
      link $(LDFLAGS) -verbose -map:$*.map -out:$*.exe tracert.obj geoip.obj IP2Location.obj $(LIBS) > link.tmp
      cat link.tmp >> tracert.map

.c.exe: # tmp.res
      $(CC) -c $(CFLAGS) $*.c
      link $(LDFLAGS) -map:$*.map -out:$*.exe $*.obj $(LIBS)

.c.i:
      $(CC) -E -c $(CFLAGS) $*.c > $*.i

tmp.res: tmp.rc
      rc -nologo -DDEBUG=0 -D_MSC_VER -DBITS=32 -Fo tmp-32.res tmp.rc

# tmp.rc:

clean:
      - @del *.exe
      - @del *.ilk
      - @del *.map
      - @del *.obj
      - @del *.pdb
      - @del *.pbo
      - @del *.pbt
      - @del *.pbi
      - @del *._xe

