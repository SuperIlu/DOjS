#
#  GNU Makefile for Waterloo TCP sample applications
#  Gisle Vanem 1997
#
#  Target:
#    djgpp2/GNU-C running under plain DOS or Windows.
#

OBJ_DIR = djgpp.obj

#
# Set to 1 to make DPMI-host (cwsdstub.exe) contained in .exe-file
#
DPMI_STUB ?= 1

#
# Define 'MAKE_MAP = 1' if you like a .map-file
#
MAKE_MAP = 1

#
# Add support for Geo-location in the 'tracert.c' program:
#   GEOIP_LIB = 2 ==> compile with 'ip2location.c'
#   GEOIP_LIB = 1 ==> compile with 'geoip.c'
#   GEOIP_LIB = 0 ==> compile with neither.
#
GEOIP_LIB = 2

INC_DIR = ../inc

WATTLIB = ../lib/libwatt.a

#
# If building on Windows or Linux, '$(DJGPP_PREFIX)-gcc' should become
# something like 'i586-pc-msdosdjgpp-gcc' or a full path like
# 'f:/gv/djgpp/bin/i586-pc-msdosdjgpp-gcc'.
#
# If building on plain old DOS, it is simply 'gcc' which 'make' should
# find on %PATH.
#
ifneq ($(DJGPP_PREFIX),)
  BIN_PREFIX = $(DJGPP_PREFIX)-
else ifeq ($(OS),Windows_NT)
  $(error Define a DJGPP_PREFIX to invoke the djgpp cross compiler).
endif

#
# In case %DJDIR is not set under Windows. Why would it exist?
#
DJDIR ?= e:/djgpp

CC      =  $(BIN_PREFIX)gcc
CFLAGS  = -Wall -W -Wno-sign-compare -g -O2 -I$(INC_DIR)
LDFLAGS = -s

ifeq ($(MAKE_MAP),1)
  LDFLAGS += -Wl,--print-map,--sort-common,--cref
endif

SOURCES = ping.c     popdump.c  rexec.c   tcpinfo.c cookie.c \
          daytime.c  dayserv.c  finger.c  host.c    lpq.c    \
          lpr.c      ntime.c    ph.c      stat.c    htget.c  \
          revip.c    tracert.c  tcptalk.c vlsm.c    whois.c  \
          bping.c    uname.c    blather.c lister.c  wol.c    \
          eth-wake.c ident.c    country.c

OBJECTS  = $(addprefix $(OBJ_DIR)/, $(SOURCES:.c=.o) tiny.o)
PROGRAMS = $(SOURCES:.c=.exe)

all: $(PROGRAMS)
	@echo 'Protected-mode (djgpp2) binaries done.'

$(OBJECTS): $(OBJ_DIR)

$(OBJ_DIR):
	-mkdir $(OBJ_DIR)

dxe_tst.exe: $(OBJ_DIR)/dxe_tst.o ../lib/dxe/libwatt.a
	$(call link_EXE, $@, $^, $(@:.exe=.map))

tracert_obj = $(addprefix $(OBJ_DIR)/, tracert.o geoip.o IP2Location.o tiny.o)

tracert_CFLAGS = -DIS_WATT32 # -DPROBE_PROTOCOL=IPPROTO_TCP

ifeq ($(GEOIP_LIB),1)
  tracert_CFLAGS += -DUSE_GEOIP
else ifeq ($(GEOIP_LIB),2)
  tracert_CFLAGS += -DUSE_IP2LOCATION
endif

$(tracert_obj): CFLAGS += $(tracert_CFLAGS)

$(OBJ_DIR)/%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

tracert.exe: $(tracert_obj)
ifeq ($(DPMI_STUB),1)
	$(call link_EXE, $(OBJ_DIR)/tracert.exe, $^, tracert.map)
	$(call bin_cat_files, $(DJDIR)/bin/cwsdstub.exe, $(OBJ_DIR)/tracert.exe, tracert.exe)
else
	$(call link_EXE, $@, $^, tracert.map)
endif
	rm -f $(OBJ_DIR)/geoip.o $(OBJ_DIR)/IP2Location.o
	@echo


%.exe: $(OBJ_DIR)/%.o $(OBJ_DIR)/tiny.o $(EXTRA_O)
ifeq ($(DPMI_STUB),1)
	$(call link_EXE, $(OBJ_DIR)/$*.exe, $^, $(@:.exe=.map))
	$(call bin_cat_files, $(DJDIR)/bin/cwsdstub.exe, $(OBJ_DIR)/$*.exe, $@)
else
	$(call link_EXE, $@, $^, $(@:.exe=.map))
endif

clean:
	rm -f $(PROGRAMS:.exe=.map) $(OBJ_DIR)/*
	-rmdir $(OBJ_DIR)

vclean realclean: clean
	rm -f $(PROGRAMS)

#
# Make-macro for linking.
#
ifeq ($(MAKE_MAP),1)
  define link_EXE
    $(CC) $(LDFLAGS) -o $(1) $(2) $(WATTLIB) > $(3)
    @echo
  endef
else
  define link_EXE
    $(CC) $(LDFLAGS) -o $(1) $(2) $(WATTLIB)
    @echo
  endef
endif

#
# A concatination of binary files:
#   prefix $(1) to $(2). Result file is in $(3)
#
# If '$OS=Windows_NT', let 'cmd' do the copy. Otherwise
# I assume GNU-make under MSDOS will let command.com do it.
#
# In 'cmd', even with '/y' flags, it's complaing the target $(3)
# exists. So just delete that first.
#
ifeq ($(OS),Windows_NT)
  define bin_cat_files
    rm -f $(3)
    cmd /c copy /y /b $(subst /,\,$(1)) + $(subst /,\,$(2)) $(3)
  endef
else
  define bin_cat_files
    copy /y /b $(1) $(subst /,\,$(1)) + $(subst /,\,$(2)) $(3)
  endef
endif