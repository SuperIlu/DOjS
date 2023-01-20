#
#  GNU Makefile for some Waterloo TCP sample applications
#  Gisle Vanem 2004 - 2020.
#
#  Target:
#    GNU C 3+ (MinGW)
#

.SUFFIXES: .exe

#
# Set to 1 to link using static libwatt32.a
#
STATIC_LIB = 0

#
# Define 'MAKE_MAP = 1' if you like a .map-file
#
MAKE_MAP = 0

CC       = gcc
CFLAGS  = -m32 -g -Wall -W -Wno-sign-compare -O2 -I../inc
LDFLAGS = -m32 # -s

ifeq ($(STATIC_LIB),1)
  CFLAGS  += -DWATT32_STATIC
  LDFLAGS += -Wl,--enable-stdcall-fixup
  WATT_LIB = ../lib/libwatt32.a
else
  WATT_LIB = ../lib/libwatt32.dll.a
endif

CFLAGS += -DIS_WATT32 -DUSE_GEOIP  # Or '-DUSE_IP2LOCATION'

ifeq ($(MAKE_MAP),1)
  MAPFILE = -Wl,--print-map,--sort-common,--cref > $*.map
endif

PROGS = ping.exe     popdump.exe  rexec.exe    tcpinfo.exe  cookie.exe   \
        daytime.exe  dayserv.exe  finger.exe   host.exe     lpq.exe      \
        lpr.exe      ntime.exe    ph.exe       stat.exe     htget.exe    \
        revip.exe    vlsm.exe     whois.exe    wol.exe      eth-wake.exe \
        ident.exe    country.exe  con-test.exe gui-test.exe tracert.exe

all: $(PROGS)
	@echo 'MinGW32 binaries done.'

con-test.exe: w32-test.c $(WATT_LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(MAPFILE)

gui-test.exe: w32-test.c $(WATT_LIB)
	$(CC) -DIS_GUI=1 $(CFLAGS) $(LDFLAGS) -Wl,--subsystem,windows -o $@ $^ $(MAPFILE)
	@echo

tracert.exe: tracert.c geoip.c IP2Location.c $(WATT_LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ tracert.c geoip.c IP2Location.c $(WATT_LIB) $(MAPFILE)
	@echo

%.exe: %.c $(WATT_LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $*.exe $^ $(MAPFILE)
	@echo

clean:
	rm -f $(PROGS)

SOURCES = ping.c    popdump.c rexec.c   tcpinfo.c cookie.c   \
          daytime.c dayserv.c finger.c  host.c    lpq.c      \
          lpr.c     ntime.c   ph.c      stat.c    htget.c    \
          revip.c   vlsm.c    whois.c   wol.c     eth-wake.c \
          ident.c   country.c tracert.c w32-test.c

path_find = $(wildcard $(addsuffix /$(1),$(subst ;, ,$(subst \,/,$(PATH)))))

ifneq ($(call path_find,python.exe),)
  DEP_CFLAGS = -M $(CFLAGS) | python.exe normpath.py -
else
  DEP_CFLAGS = -MM $(CFLAGS)
endif

depend:
	$(CC) $(SOURCES) $(DEP_CFLAGS) > .depend.MinGW32

-include .depend.MinGW32

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
tracert.exe:  tracert.c geoip.c geoip.h IP2Location.c IP2Location.h

