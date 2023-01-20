#
#  GNU Makefile for some Waterloo TCP sample applications
#  Gisle Vanem 2004 - 2020.
#
#  Target:
#    CygWin, 32-bit or 64-bit selectable by 'CPU=[x86|x64]'.
#
# If '$(CPU)=x64', build 64-bit programs. Otherwise 32-bit programs.
#
ifeq ($(CPU),)
  CPU = x86
endif

#
# GNU-make is case-sensitive
#
ifeq ($(CPU),X64)
  CPU = x64
endif

#
# Set to 1 to link using static ../lib/$(CPU)/libwatt32-cygwin.a
#
STATIC_LIB ?= 0

CC     = gcc
CFLAGS = -g -Wall -W -O2 -I../inc -Wno-address -DWATT32

ifeq ($(CPU),x64)
  CFLAGS += -m64
else
  CFLAGS += -m32
endif

ifeq ($(STATIC_LIB),1)
  CFLAGS  += -DWATT32_STATIC
  LDFLAGS += -Wl,--enable-stdcall-fixup
  WATT_LIB = ../lib/$(CPU)/libwatt32-cygwin.a
else
  WATT_LIB = ../lib/$(CPU)/libwatt32-cygwin.dll.a
endif

PROGS = ping.exe     popdump.exe  rexec.exe   tcpinfo.exe  cookie.exe   \
        daytime.exe  dayserv.exe  finger.exe  host.exe     lpq.exe      \
        lpr.exe      ntime.exe    ph.exe      stat.exe     htget.exe    \
        revip.exe    vlsm.exe     whois.exe   wol.exe      eth-wake.exe \
        ident.exe    country.exe  tracert.exe con-test.exe gui-test.exe

all: $(PROGS)
	@echo CygWin binaries done

tracert.exe: tracert.c geoip.c
	$(CC) $(CFLAGS) $(LDFLAGS) -DUSE_GEOIP tracert.c geoip.c -o $@ $(WATT_LIB) $(MAPFILE)

gui-test.exe: w32-test.c # tmp.res
	$(CC) $(CFLAGS) $(LDFLAGS) -DIS_GUI=1 -Wl,--subsystem,windows -o $@ w32-test.c $(WATT_LIB) $(MAPFILE)

con-test.exe: w32-test.c # tmp.res
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ w32-test.c $(WATT_LIB) $(MAPFILE)

%.exe: %.c $(WATT_LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $*.exe $*.c $(WATT_LIB) $(MAPFILE)

clean:
	rm -f $(PROGS)

#
# These are needed for MSYS' make (sigh)
#
ping.exe:     ping.c
popdump.exe:  popdump.c
rexec.exe:    rexec.c
tcpinfo.exe:  tcpinfo.c
cookie.exe:   cookie.c
daytime.exe:  daytime.c
dayserv.exe:  dayserv.c
finger.exe:   finger.c
host.exe:     host.c
lpq.exe:      lpq.c
lpr.exe:      lpr.c
ntime.exe:    ntime.c
ph.exe:       ph.c
stat.exe:     stat.c
htget.exe:    htget.c
revip.exe:    revip.c
vlsm.exe:     vlsm.c
whois.exe:    whois.c
wol.exe:      wol.c
eth-wake.exe: eth-wake.c
ident.exe:    ident.c
country.exe:  country.c
