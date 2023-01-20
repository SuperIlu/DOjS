BIN_PREFIX = i586-pc-msdosdjgpp-
W32_BIN2C_ = ../util/linux/bin2c
W32_NASM_ ?= nasm
#
# NB! THIS MAKEFILE WAS AUTOMATICALLY GENERATED FROM Makefile.all.
#     DO NOT EDIT. Edit Makefile.all and run "configur.bat" <target> instead.
#
# Makefile for the Watt-32 TCP/IP stack.
#


ifeq ($(W32_BIN2C_),)
  $(error 'W32_BIN2C_' is not defined. Try running 'configur.bat <target>' again.)
endif




ASM_SOURCE = asmpkt.asm cpumodel.asm

CORE_SOURCE = bsdname.c  btree.c    chksum.c   country.c  crc.c      dynip.c    \
              echo.c     fortify.c  getopt.c   gettod.c   highc.c    idna.c     \
              ip4_frag.c ip4_in.c   ip4_out.c  ip6_in.c   ip6_out.c  language.c \
              lookup.c   loopback.c misc.c     netback.c  oldstuff.c packet32.c \
              pc_cbrk.c  pcarp.c    pcbootp.c  pcbuf.c    pcconfig.c pcdbug.c   \
              pcdhcp.c   pcdns.c    pcicmp.c   pcicmp6.c  pcigmp.c   pcintr.c   \
              pcping.c   pcpkt.c    pcpkt32.c  pcqueue.c  pcrarp.c   pcrecv.c   \
              pcsed.c    pcstat.c   pctcp.c    ports.c    powerpak.c ppp.c      \
              pppoe.c    profile.c  punycode.c qmsg.c     run.c      settod.c   \
              sock_dbu.c sock_in.c  sock_ini.c sock_io.c  sock_prn.c sock_scn.c \
              sock_sel.c split.c    misc_str.c swsvpkt.c  tcp_fsm.c  tcp_md5.c  \
              tftp.c     timer.c    udp_rev.c  version.c  wdpmi.c    win_dll.c  \
              winadinf.c winmisc.c  winpkt.c   x32vm.c

BSD_SOURCE =  accept.c   bind.c     bsddbug.c  close.c    connect.c  fcntl.c    \
              fsext.c    get_ai.c   get_ip.c   get_ni.c   get_xbyr.c geteth.c   \
              gethost.c  gethost6.c getname.c  getnet.c   getprot.c  getput.c   \
              getserv.c  ioctl.c    linkaddr.c listen.c   netaddr.c  neterr.c   \
              nettime.c  nsapaddr.c poll.c     presaddr.c printk.c   receive.c  \
              select.c   shutdown.c signal.c   socket.c   sockopt.c  stream.c   \
              syslog.c   syslog2.c  transmit.c

BIND_SOURCE = res_comp.c res_data.c res_debu.c res_init.c res_loc.c res_mkqu.c \
              res_quer.c res_send.c

ZLIB_SOURCE = zadler32.c  zcompres.c zcrc32.c   zgzio.c \
              zuncompr.c  zdeflate.c ztrees.c   zutil.c \
              zinflate.c  zinfback.c zinftree.c zinffast.c

C_SOURCE = $(CORE_SOURCE) $(BSD_SOURCE) $(BIND_SOURCE) $(ZLIB_SOURCE)

COMMON_OBJS = \
       $(OBJPATH)cpumodel.obj  \
       $(OBJPATH)accept.obj   $(OBJPATH)bind.obj      \
       $(OBJPATH)bsddbug.obj  $(OBJPATH)bsdname.obj   \
       $(OBJPATH)btree.obj    $(OBJPATH)chksum.obj    \
       $(OBJPATH)close.obj    $(OBJPATH)connect.obj   \
       $(OBJPATH)crc.obj      $(OBJPATH)dynip.obj     \
       $(OBJPATH)echo.obj     $(OBJPATH)fcntl.obj     \
       $(OBJPATH)fortify.obj  $(OBJPATH)get_ai.obj    \
       $(OBJPATH)get_ip.obj   $(OBJPATH)get_ni.obj    \
       $(OBJPATH)get_xbyr.obj $(OBJPATH)geteth.obj    \
       $(OBJPATH)gethost.obj  $(OBJPATH)gethost6.obj  \
       $(OBJPATH)getname.obj  $(OBJPATH)getnet.obj    \
       $(OBJPATH)getopt.obj   $(OBJPATH)getprot.obj   \
       $(OBJPATH)getput.obj   $(OBJPATH)getserv.obj   \
       $(OBJPATH)gettod.obj   $(OBJPATH)idna.obj      \
       $(OBJPATH)ioctl.obj    $(OBJPATH)ip4_frag.obj  \
       $(OBJPATH)ip4_in.obj   $(OBJPATH)ip4_out.obj   \
       $(OBJPATH)ip6_in.obj   $(OBJPATH)ip6_out.obj   \
       $(OBJPATH)language.obj $(OBJPATH)linkaddr.obj  \
       $(OBJPATH)listen.obj   $(OBJPATH)lookup.obj    \
       $(OBJPATH)loopback.obj $(OBJPATH)misc.obj      \
       $(OBJPATH)netaddr.obj  $(OBJPATH)netback.obj   \
       $(OBJPATH)neterr.obj   $(OBJPATH)nettime.obj   \
       $(OBJPATH)nsapaddr.obj $(OBJPATH)oldstuff.obj  \
       $(OBJPATH)packet32.obj $(OBJPATH)pc_cbrk.obj   \
       $(OBJPATH)pcarp.obj    $(OBJPATH)pcbootp.obj   \
       $(OBJPATH)pcbuf.obj    $(OBJPATH)pcconfig.obj  \
       $(OBJPATH)pcdbug.obj   $(OBJPATH)pcdhcp.obj    \
       $(OBJPATH)pcdns.obj    $(OBJPATH)pcicmp.obj    \
       $(OBJPATH)pcicmp6.obj  $(OBJPATH)pcigmp.obj    \
       $(OBJPATH)pcping.obj   $(OBJPATH)pcqueue.obj   \
       $(OBJPATH)pcrarp.obj   $(OBJPATH)pcrecv.obj    \
       $(OBJPATH)pcsed.obj    $(OBJPATH)pcstat.obj    \
       $(OBJPATH)pctcp.obj    $(OBJPATH)poll.obj      \
       $(OBJPATH)ports.obj    $(OBJPATH)ppp.obj       \
       $(OBJPATH)pppoe.obj    $(OBJPATH)presaddr.obj  \
       $(OBJPATH)printk.obj   $(OBJPATH)profile.obj   \
       $(OBJPATH)punycode.obj $(OBJPATH)receive.obj   \
       $(OBJPATH)res_comp.obj $(OBJPATH)res_data.obj  \
       $(OBJPATH)res_debu.obj $(OBJPATH)res_init.obj  \
       $(OBJPATH)res_loc.obj  $(OBJPATH)res_mkqu.obj  \
       $(OBJPATH)res_quer.obj $(OBJPATH)res_send.obj  \
       $(OBJPATH)run.obj      $(OBJPATH)select.obj    \
       $(OBJPATH)settod.obj   $(OBJPATH)shutdown.obj  \
       $(OBJPATH)signal.obj   $(OBJPATH)sock_dbu.obj  \
       $(OBJPATH)sock_in.obj  $(OBJPATH)sock_ini.obj  \
       $(OBJPATH)sock_io.obj  $(OBJPATH)sock_prn.obj  \
       $(OBJPATH)sock_scn.obj $(OBJPATH)sock_sel.obj  \
       $(OBJPATH)socket.obj   $(OBJPATH)sockopt.obj   \
       $(OBJPATH)split.obj    $(OBJPATH)stream.obj    \
       $(OBJPATH)misc_str.obj $(OBJPATH)swsvpkt.obj   \
       $(OBJPATH)syslog.obj   $(OBJPATH)syslog2.obj   \
       $(OBJPATH)tcp_fsm.obj  $(OBJPATH)tcp_md5.obj   \
       $(OBJPATH)tftp.obj     $(OBJPATH)timer.obj     \
       $(OBJPATH)transmit.obj $(OBJPATH)udp_rev.obj   \
       $(OBJPATH)version.obj  $(OBJPATH)zadler32.obj  \
       $(OBJPATH)zcompres.obj $(OBJPATH)zcrc32.obj    \
       $(OBJPATH)zdeflate.obj $(OBJPATH)zgzio.obj     \
       $(OBJPATH)zinfback.obj $(OBJPATH)zinffast.obj  \
       $(OBJPATH)zinflate.obj $(OBJPATH)zinftree.obj  \
       $(OBJPATH)ztrees.obj   $(OBJPATH)zuncompr.obj  \
       $(OBJPATH)zutil.obj

WINDOWS_OBJS = $(OBJPATH)win_dll.obj  \
               $(OBJPATH)winadinf.obj \
               $(OBJPATH)winmisc.obj  \
               $(OBJPATH)winpkt.obj

#
# These object are only possible for DOS (16/32-bit).
# So never add them to Library programs for Windows targets.
#
# This is to prevent warnings like these from MSVC's link:
#   qmsg.obj : warning LNK4221: This object file does not define any previously undefined public
#              symbols, so it will not be used by any link operation that consumes this library
#

DOS_OBJS = \
           $(OBJPATH)country.obj  \
           $(OBJPATH)fsext.obj    \
           $(OBJPATH)pcpkt32.obj  \
           $(OBJPATH)pcpkt.obj    \
           $(OBJPATH)pcintr.obj   \
           $(OBJPATH)powerpak.obj \
           $(OBJPATH)qmsg.obj     \
           $(OBJPATH)wdpmi.obj    \
           $(OBJPATH)x32vm.obj

#
# CPU, bit-width and suffix.
# Not used for any DOS targets.
#
CPU    = x86
BITS   = 32
SUFFIX =

#
# For all 16/32-bit DOS targets:
#
OBJS = $(COMMON_OBJS) $(DOS_OBJS)

#
# This generated file is used for all 32-bit MSDOS targets
# (and when USE_FAST_PKT is defined). This enables a faster real-mode
# callback for the PKTDRVR receiver. Included as an array in pcpkt2.c.
#
PKT_STUB = pkt_stub.h

########################################################################

#
# Only used by 'make -f djgpp install':
#
prefix = /dev/env/DJDIR/net/watt

CFLAGS = -O3 -g -I. -I../inc -DWATT32_BUILD -W -Wall -Wno-strict-aliasing \
         -march=i386 -mtune=i586

STAT_LIB = ../lib/libwatt.a
OBJDIR   = build/djgpp
OBJPATH  = $(OBJDIR)/

CC     = $(BIN_PREFIX)gcc
AR     = $(BIN_PREFIX)ar rs
AS     = $(BIN_PREFIX)as
AFLAGS = # --gdwarf2

ifeq ($(filter 2 3 4,$(word 3, $(shell true | $(CC) -E -dD -x c - | grep 'define\ *__GNUC__'))),)
  #
  # We have gcc >= 5.x and we must ensure that always traditional
  # GNU extern inline semantics are used (aka -fgnu89-inline) even
  # if ISO C99 semantics have been specified.
  #
  CFLAGS += -fgnu89-inline
endif

OBJS := $(subst .obj,.o,$(OBJS))

C_ARGS   = $(OBJPATH)gcc.arg
LIB_ARGS = $(OBJPATH)ar.arg

TARGETS = $(STAT_LIB)

all: $(PKT_STUB) $(OBJPATH)cflags.h $(TARGETS)
	@echo All done

$(STAT_LIB): $(OBJS) $(LIB_ARGS)
	$(AR) $@ @$(LIB_ARGS)

$(OBJPATH)%.o: %.c $(C_ARGS)
	$(CC) @$(C_ARGS) -o $@ $<

$(OBJPATH)%.o: %.S $(C_ARGS)
	$(CC) -E @$(C_ARGS) $< > $(OBJPATH)$*.iS
	$(AS) $(AFLAGS) $(OBJPATH)$*.iS -o $@

$(OBJPATH)cpumodel.o: cpumodel.S

install: all
	- mkdir -p "$(prefix)/inc"
	- mkdir -p "$(prefix)/lib"
	cp -fr ../inc "$(prefix)"
	cp -fr ../lib "$(prefix)"
	@echo Install to $(prefix) done

clean:
	rm -f $(TARGETS) $(OBJPATH)*.o $(OBJPATH)*.iS $(OBJPATH)*.arg $(PKT_STUB) $(OBJPATH)cflags.h
	@echo Cleaning done

-include build/djgpp/watt32.dep

########################################################################


########################################################################

doxygen:
	doxygen doxyfile

lang.c: lang.l
	flex --8bit --stdout lang.l > lang.c

  #
  # All these Windows targets uses GNU-make. Hence it should be safe to
  # assume the 'date' program is available.
  #
  DATE = $(shell date +%d-%B-%Y)

  #
  # Extract the GNU-make major version number:
  #
  MAKE_MAJOR_VER = $(word 1, $(subst ., ,$(MAKE_VERSION)))

  ifeq ($(MAKE_MAJOR_VER),4)
    #
    # Create a response file $(1) for GNU-make ver 4.x.
    # One word from $(2) per line into $(1).
    #
    define create_response_file
      $(file > $(1))
      $(foreach f, $(2), $(file >> $(1),$(strip $(f))) )
    endef

    else
    #
    # For a buggy (?) GNU-make. E.g. on MacOS:
    #
    define create_response_file
      rm -f $(1) ; $(foreach f, $(2), echo $(strip $(f)) >> $(1) ;)
    endef
  endif

$(C_ARGS): $(MAKEFILE_LIST)
	$(call create_response_file, $@, -c $(CFLAGS))

$(LIB_ARGS): $(OBJS) $(MAKEFILE_LIST)
	$(call create_response_file, $@, $(OBJS))

$(LINK_ARGS): $(OBJS) $(MAKEFILE_LIST)
	$(call create_response_file, $@, $(OBJS))

  #
  # GNU-Make rules uses shell 'sh' commands:
  #
$(OBJPATH)cflags.h: $(MAKEFILE_LIST)
	echo 'const char *w32_cflags = "$(CFLAGS)";' > $(OBJPATH)cflags.h
	echo 'const char *w32_cc     = "$(CC)";'    >> $(OBJPATH)cflags.h


$(OBJPATH)pcpkt.obj: asmpkt.nas
$(OBJPATH)pcpkt.o:   asmpkt.nas

$(PKT_STUB): asmpkt.nas
	$(W32_NASM_) -f bin -l asmpkt.lst -o asmpkt.bin asmpkt.nas
	$(W32_BIN2C_) asmpkt.bin > $@



#
# Rules for creating 'cflagsbf.h'. A file with a C-array of the 'CFLAGS' used.
# Included in 'version.c'.
#
# '$(W32_BIN2C)' should be set by '.\configur.bat' to point to either
# '..\util\bin2c.exe or '..\util\win32\bin2c.exe'.
#
# And for GNU-make, $(W32_BIN2C_) should be set by '.\configur.bat' to point to either
# '../util/bin2c.exe' or '../util/win32/bin2c.exe'.
#
# PS. 'cflagsbf.h' was previously named 'cflags_buf.h'. But that may cause troubles
#     on plain DOS with only 8+3 files.
#


#
# Preprocess a .c-file and pipe through 'Astyle' to beautify it.
#
%.i: %.c
	@echo "Preprocessed result of $< with these CFLAGS:"    > $@
	cat $(C_ARGS)                                          >> $@
	@echo "----------------------------------------------" >> $@
	$(CC) -E @$(C_ARGS) $< | astyle >> $@
	@echo

DEP_REPLACE = sed -e 's/\(.*\)\.o: /\n$$(OBJPATH)\1.o: /' \
                  -e 's@/cygdrive/@:@'

DEP_FILE = $(OBJPATH)watt-32.dep

depend: $(OBJPATH)cflags.h
	$(CC) -MM $(CFLAGS) $(C_SOURCE) | $(DEP_REPLACE) > $(DEP_FILE)

#
# This is not the file generated by 'configur.bat', but the above 'depend' rule.
#
-include $(DEP_FILE)

