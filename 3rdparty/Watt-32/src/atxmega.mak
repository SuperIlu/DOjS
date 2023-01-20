#
# Experimental makefile for microcontroller targets:
#  ATXmega
#
# This file is currently NOT generated.
#
AVR_TOOLS_ROOT ?= f:/gv/dx-radio/Embedded/arduino-1.8.16/hardware/tools/avr
AVR_TOOLS_INC   = $(AVR_TOOLS_ROOT)/avr/include
AVR_PREFIX      = $(AVR_TOOLS_ROOT)/bin/avr-

export C_INCLUDE_PATH=
export CPLUS_INCLUDE_PATH=

CORE_SOURCE = bsdname.c  btree.c    chksum.c   crc.c      \
              dynip.c    echo.c     fortify.c  getopt.c   \
              gettod.c   idna.c     ip4_frag.c ip4_in.c   \
              ip4_out.c  ip6_in.c   ip6_out.c  language.c \
              lookup.c   loopback.c misc.c     netback.c  \
              oldstuff.c pc_cbrk.c  pcarp.c    pcbootp.c  \
              pcbuf.c    pcconfig.c pcdbug.c   pcdhcp.c   \
              pcdns.c    pcicmp.c   pcicmp6.c  pcigmp.c   \
              pcping.c   pcrarp.c   pcrecv.c   pcsed.c    \
              pcstat.c   pctcp.c    ports.c    ppp.c      \
              pppoe.c    profile.c  punycode.c qmsg.c     \
              run.c      settod.c   sock_dbu.c sock_in.c  \
              sock_ini.c sock_io.c  sock_prn.c sock_scn.c \
              sock_sel.c split.c    misc_str.c tcp_fsm.c  \
              tcp_md5.c  tftp.c     timer.c    udp_rev.c  \
              version.c

BSD_SOURCE =  accept.c   bind.c     bsddbug.c  close.c    connect.c  fcntl.c   \
              fsext.c    get_ai.c   get_ip.c   get_ni.c   get_xbyr.c geteth.c  \
              gethost.c  gethost6.c getname.c  getnet.c   getprot.c  getput.c  \
              getserv.c  ioctl.c    linkaddr.c listen.c   netaddr.c  neterr.c  \
              nettime.c  nsapaddr.c poll.c     presaddr.c printk.c   receive.c \
              select.c   shutdown.c signal.c   socket.c   sockopt.c  stream.c  \
              syslog.c   syslog2.c  transmit.c

BIND_SOURCE = res_comp.c res_data.c res_debu.c res_init.c res_loc.c res_mkqu.c \
              res_quer.c res_send.c

ZLIB_SOURCE = zadler32.c  zcompres.c zcrc32.c   zgzio.c \
              zuncompr.c  zdeflate.c ztrees.c   zutil.c \
              zinflate.c  zinfback.c zinftree.c zinffast.c

#
# To be defined: depends on hardware.
#
MCU_SOURCE =

ALL_SOURCES = $(CORE_SOURCE) $(BSD_SOURCE) $(BIND_SOURCE) $(MCU_SOURCE) $(ZLIB_SOURCE)

ALL_OBJS = $(addprefix $(OBJPATH)/, $(ALL_SOURCES:.c=.o))

CFLAGS = -O3 -g -I. -I../inc -DWATT32_BUILD \
         -W -Wno-cpp -Wno-strict-aliasing   \
         -fgnu89-inline                     \
         -I$(AVR_TOOLS_INC)                 \
         -I$(AVR_TOOLS_INC)/avr

#
# Select target MCU:
#
# CFLAGS += -mmcu=atxmega128a1u
CFLAGS += -mmcu=atmega16hva2

STAT_LIB = ../lib/libwatt-atxmega.a
OBJDIR   = build/atxmega
OBJPATH  = $(OBJDIR)

CC     = $(AVR_PREFIX)gcc
AR     = $(AVR_PREFIX)ar rs
AS     = $(AVR_PREFIX)as
AFLAGS = # --gdwarf2

C_ARGS   = $(OBJPATH)/gcc.arg
LIB_ARGS = $(OBJPATH)/ar.arg

all: $(OBJPATH) $(C_ARGS) $(OBJPATH)/cflags.h $(STAT_LIB)
	@echo "All done"

$(OBJPATH):
	mkdir $@
	@echo

$(STAT_LIB): $(ALL_OBJS) $(LIB_ARGS)
	$(AR) $@ @$(LIB_ARGS)
	@echo

$(OBJPATH)/%.o: %.c
	$(CC) -c @$(C_ARGS) -o $@ $<

$(OBJPATH)/%.o: %.S
	$(CC) -E @$(C_ARGS) $< > $(OBJPATH)/$*.iS
	$(AS) $(AFLAGS) $(OBJPATH)/$*.iS -o $@

clean:
	rm -f $(STAT_LIB)
	rm -fr $(OBJPATH)
	@echo Cleaning done

lang.c: lang.l
	flex --8bit --stdout lang.l > lang.c
	@echo

#
# Create a response file $(1).
# One word from $(2) per line into $(1).
#
define create_response_file
  $(file > $(1))
  $(foreach f, $(2), $(file >> $(1),$(strip $(f))) )
endef

$(C_ARGS): $(MAKEFILE_LIST)
	$(call create_response_file, $@, $(CFLAGS))

$(LIB_ARGS): $(ALL_OBJS) $(MAKEFILE_LIST)
	$(call create_response_file, $@, $(ALL_OBJS))

$(OBJPATH)/cflags.h: $(MAKEFILE_LIST)
	echo 'const char *w32_cflags = "$(CFLAGS)";' > $(OBJPATH)/cflags.h
	echo 'const char *w32_cc     = "$(CC)";'    >> $(OBJPATH)/cflags.h

#
# Print the AVR built-ins:
#
built-ins:
	$(CC) $(CFLAGS) -E -dM - < /dev/null | sort


#
# Preprocess a .c-file and pipe through 'Astyle' to beautify it.
#
%.i: %.c
	@echo "Preprocessed result of $< with these CFLAGS:"    > $@
	cat $(C_ARGS)                                          >> $@
	@echo "----------------------------------------------" >> $@
	$(CC) -E @$(C_ARGS) $< | astyle >> $@
	@echo

DEP_REPLACE = sed -e 's/\(.*\)\.o: /\n$$(OBJPATH)\/\1.o: /' \
                  -e 's@/cygdrive/@:@'

DEP_FILE = $(OBJPATH)/watt-32.dep

depend: $(OBJPATH)/cflags.h
	$(CC) -MM $(CFLAGS) $(ALL_SOURCES) | $(DEP_REPLACE) > $(DEP_FILE)

-include $(DEP_FILE)

