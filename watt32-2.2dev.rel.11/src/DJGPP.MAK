#
# NB! THIS MAKEFILE WAS AUTOMATICALLY GENERATED FROM Makefile.all.
#     DO NOT EDIT. Edit Makefile.all and run "configur.bat" <target> instead.
#
# Makefile for the Watt-32 TCP/IP stack.
#


#ifeq ($(W32_BIN2C_),)
#  $(error 'W32_BIN2C_' is not defined. Try running 'configur.bat <target>' again.)
#endif


ASM_SOURCE = asmpkt.asm chksum0.asm cpumodel.asm

CORE_SOURCE = bsdname.c  btree.c    chksum.c   country.c  crc.c      dynip.c    \
              echo.c     fortify.c  getopt.c   gettod.c   highc.c    idna.c     \
              ip4_frag.c ip4_in.c   ip4_out.c  ip6_in.c   ip6_out.c  language.c \
              lookup.c   loopback.c misc.c     netback.c  oldstuff.c packet32.c \
              pc_cbrk.c  pcarp.c    pcbootp.c  pcbuf.c    pcconfig.c pcdbug.c   \
              pcdhcp.c   pcdns.c    pcicmp.c   pcicmp6.c  pcigmp.c   pcintr.c   \
              pcping.c   pcpkt.c    pcpkt32.c  pcqueue.c  pcrarp.c   pcrecv.c   \
              pcsed.c    pcstat.c   pctcp.c    ports.c    powerpak.c ppp.c      \
              pppoe.c    profile.c  punycode.c qmsg.c     rs232.c    run.c      \
              settod.c   sock_dbu.c sock_in.c  sock_ini.c sock_io.c  sock_prn.c \
              sock_scn.c sock_sel.c split.c    strings.c  swsvpkt.c  tcp_fsm.c  \
              tcp_md5.c  tftp.c     timer.c    udp_rev.c  version.c  wdpmi.c    \
              win_dll.c  winadinf.c winmisc.c  winpkt.c   x32vm.c

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

OBJS = \
       $(OBJDIR)\chksum0.obj  $(OBJDIR)\cpumodel.obj  \
       $(OBJDIR)\accept.obj   $(OBJDIR)\bind.obj      \
       $(OBJDIR)\bsddbug.obj  $(OBJDIR)\bsdname.obj   \
       $(OBJDIR)\btree.obj    $(OBJDIR)\chksum.obj    \
       $(OBJDIR)\close.obj    $(OBJDIR)\connect.obj   \
       $(OBJDIR)\country.obj  $(OBJDIR)\crc.obj       \
       $(OBJDIR)\dynip.obj    $(OBJDIR)\echo.obj      \
       $(OBJDIR)\fcntl.obj    $(OBJDIR)\fortify.obj   \
       $(OBJDIR)\fsext.obj    $(OBJDIR)\get_ai.obj    \
       $(OBJDIR)\get_ip.obj   $(OBJDIR)\get_ni.obj    \
       $(OBJDIR)\get_xbyr.obj $(OBJDIR)\geteth.obj    \
       $(OBJDIR)\gethost.obj  $(OBJDIR)\gethost6.obj  \
       $(OBJDIR)\getname.obj  $(OBJDIR)\getnet.obj    \
       $(OBJDIR)\getopt.obj   $(OBJDIR)\getprot.obj   \
       $(OBJDIR)\getput.obj   $(OBJDIR)\getserv.obj   \
       $(OBJDIR)\gettod.obj   $(OBJDIR)\idna.obj      \
       $(OBJDIR)\ioctl.obj    $(OBJDIR)\ip4_frag.obj  \
       $(OBJDIR)\ip4_in.obj   $(OBJDIR)\ip4_out.obj   \
       $(OBJDIR)\ip6_in.obj   $(OBJDIR)\ip6_out.obj   \
       $(OBJDIR)\language.obj $(OBJDIR)\linkaddr.obj  \
       $(OBJDIR)\listen.obj   $(OBJDIR)\lookup.obj    \
       $(OBJDIR)\loopback.obj $(OBJDIR)\misc.obj      \
       $(OBJDIR)\netaddr.obj  $(OBJDIR)\netback.obj   \
       $(OBJDIR)\neterr.obj   $(OBJDIR)\nettime.obj   \
       $(OBJDIR)\nsapaddr.obj $(OBJDIR)\oldstuff.obj  \
       $(OBJDIR)\packet32.obj $(OBJDIR)\pc_cbrk.obj   \
       $(OBJDIR)\pcarp.obj    $(OBJDIR)\pcbootp.obj   \
       $(OBJDIR)\pcbuf.obj    $(OBJDIR)\pcconfig.obj  \
       $(OBJDIR)\pcdbug.obj   $(OBJDIR)\pcdhcp.obj    \
       $(OBJDIR)\pcdns.obj    $(OBJDIR)\pcicmp.obj    \
       $(OBJDIR)\pcicmp6.obj  $(OBJDIR)\pcigmp.obj    \
       $(OBJDIR)\pcintr.obj   $(OBJDIR)\pcping.obj    \
       $(OBJDIR)\pcpkt.obj    $(OBJDIR)\pcpkt32.obj   \
       $(OBJDIR)\pcqueue.obj  $(OBJDIR)\pcrarp.obj    \
       $(OBJDIR)\pcrecv.obj   $(OBJDIR)\pcsed.obj     \
       $(OBJDIR)\pcstat.obj   $(OBJDIR)\pctcp.obj     \
       $(OBJDIR)\poll.obj     $(OBJDIR)\ports.obj     \
       $(OBJDIR)\powerpak.obj $(OBJDIR)\ppp.obj       \
       $(OBJDIR)\pppoe.obj    $(OBJDIR)\presaddr.obj  \
       $(OBJDIR)\printk.obj   $(OBJDIR)\profile.obj   \
       $(OBJDIR)\punycode.obj $(OBJDIR)\qmsg.obj      \
       $(OBJDIR)\receive.obj  $(OBJDIR)\res_comp.obj  \
       $(OBJDIR)\res_data.obj $(OBJDIR)\res_debu.obj  \
       $(OBJDIR)\res_init.obj $(OBJDIR)\res_loc.obj   \
       $(OBJDIR)\res_mkqu.obj $(OBJDIR)\res_quer.obj  \
       $(OBJDIR)\res_send.obj $(OBJDIR)\rs232.obj     \
       $(OBJDIR)\run.obj      $(OBJDIR)\select.obj    \
       $(OBJDIR)\settod.obj   $(OBJDIR)\shutdown.obj  \
       $(OBJDIR)\signal.obj   $(OBJDIR)\sock_dbu.obj  \
       $(OBJDIR)\sock_in.obj  $(OBJDIR)\sock_ini.obj  \
       $(OBJDIR)\sock_io.obj  $(OBJDIR)\sock_prn.obj  \
       $(OBJDIR)\sock_scn.obj $(OBJDIR)\sock_sel.obj  \
       $(OBJDIR)\socket.obj   $(OBJDIR)\sockopt.obj   \
       $(OBJDIR)\split.obj    $(OBJDIR)\stream.obj    \
       $(OBJDIR)\strings.obj  $(OBJDIR)\swsvpkt.obj   \
       $(OBJDIR)\syslog.obj   $(OBJDIR)\syslog2.obj   \
       $(OBJDIR)\tcp_fsm.obj  $(OBJDIR)\tcp_md5.obj   \
       $(OBJDIR)\tftp.obj     $(OBJDIR)\timer.obj     \
       $(OBJDIR)\transmit.obj $(OBJDIR)\udp_rev.obj   \
       $(OBJDIR)\version.obj  $(OBJDIR)\wdpmi.obj     \
       $(OBJDIR)\win_dll.obj  $(OBJDIR)\winadinf.obj  \
       $(OBJDIR)\winmisc.obj  $(OBJDIR)\winpkt.obj    \
       $(OBJDIR)\x32vm.obj    $(OBJDIR)\zadler32.obj  \
       $(OBJDIR)\zcompres.obj $(OBJDIR)\zcrc32.obj    \
       $(OBJDIR)\zdeflate.obj $(OBJDIR)\zgzio.obj     \
       $(OBJDIR)\zinfback.obj $(OBJDIR)\zinffast.obj  \
       $(OBJDIR)\zinflate.obj $(OBJDIR)\zinftree.obj  \
       $(OBJDIR)\ztrees.obj   $(OBJDIR)\zuncompr.obj  \
       $(OBJDIR)\zutil.obj

#
# This generated file is used for all 32-bit MSDOS targets
# (and when USE_FAST_PKT is defined). This enables a faster real-mode
# callback for the PKTDRVR receiver. Included as an array in pcpkt2.c.
#

########################################################################


ifeq ($(OS),Windows_NT)
  ifneq ($(DJ_PREFIX),)
    #
    # Windows hosted djgpp cross compiler. Get it from:
    #   https://github.com/andrewwutw/build-djgpp/releases
    #
    # Define an env-var 'DJ_PREFIX=f:/gv/djgpp/bin/i586-pc-msdosdjgpp-'
    # Thus the full path to 'gcc' becomes:
    #   $(DJ_PREFIX)gcc.exe
    #
    # If not building on Windows, the '$(BIN_PREFIX)gcc' should simply become
    # 'gcc' and GNU-make should find that on %PATH.
    #
    BIN_PREFIX = $(DJ_PREFIX)

    ifeq ($(wildcard $(BIN_PREFIX)gcc.exe),)
      $(error Failed to find 'i586-pc-msdosdjgpp-gcc.exe'.)
    endif
  endif
else
  BIN_PREFIX = $(DJ_PREFIX)/i586-pc-msdosdjgpp-
endif

CFLAGS = -Os -g -I. -I../inc -W -Wall -Wno-strict-aliasing -DWATT32_BUILD \
       # -fno-strength-reduce -ffast-math \
       # -ffunction-sections -fomit-frame-pointer -gcoff

CC     = $(BIN_PREFIX)gcc
AS     = $(BIN_PREFIX)as
AR     = $(BIN_PREFIX)ar rs
AFLAGS = # --gdwarf2
TARGET = ../lib/libwatt.a
OBJDIR = build/djgpp

OBJS := $(subst \,/,$(OBJS))
OBJS := $(subst .obj,.o,$(OBJS))

all:  $(OBJDIR)/cflags.h $(TARGET)
	@echo All done

$(TARGET): $(OBJS)
	$(AR) $@ $?

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJDIR)/%.o: %.S
	$(CC) -E $(CFLAGS) $< > $(OBJDIR)/$*.iS
	$(AS) $(AFLAGS) $(OBJDIR)/$*.iS -o $@

$(OBJDIR)/chksum0.o:  chksum0.S
$(OBJDIR)/cpumodel.o: cpumodel.S

clean:
	rm -f $(TARGET) $(OBJDIR)/*.o $(OBJDIR)/*.iS $(PKT_STUB) $(OBJDIR)/cflags.h
	@echo Cleaning done

-include $(OBJDIR)/watt32.dep


########################################################################


########################################################################

doxygen:
	doxygen doxyfile

lang.c: lang.l
	flex -8 -t lang.l > lang.c

#
# GNU-Make rules uses shell 'sh' commands:
#
$(OBJDIR)/cflags.h: $(MAKEFILE_LIST)
	echo 'const char *w32_cflags = "$(CFLAGS)";' > $(OBJDIR)/cflags.h
	echo 'const char *w32_cc     = "$(CC)";'    >> $(OBJDIR)/cflags.h



$(OBJDIR)/pcpkt.o: asmpkt.nas

$(PKT_STUB): asmpkt.nas
	$(W32_NASM_) -f bin -l asmpkt.lst -o asmpkt.bin asmpkt.nas
	$(W32_BIN2C_) asmpkt.bin > $@



build\Borland\Win32\watt-32.res: watt-32.rc
	$(BRC32) -DDEBUG=0 -D__BORLANDC__ -r -i..\inc -fo build\Borland\Win32\watt-32.res watt-32.rc

build/clang/32bit/watt-32.res: watt-32.rc
	rc -nologo -DRC_DATE="$(DATE)" -DDEBUG=0 -D__clang__ -DBITS=32 -Fo build/clang/32bit/watt-32.res  watt-32.rc

build/clang/64bit/watt-32.res: watt-32.rc
	rc -nologo -DRC_DATE="$(DATE)" -DDEBUG=0 -D__clang__ -DBITS=64 -Fo build/clang/64bit/watt-32.res watt-32.rc

build\digmars\win32\watt-32.res: watt-32.rc
	rcc -DDEBUG=0 -D__DMC__ -32 -r -obuild\digmars\win32\watt-32.res watt-32.rc

build\lcc\watt-32.res: watt-32.rc
	lrc -DDEBUG=0 -D__LCC__ -i..\inc -obuild\lcc\watt-32.res watt-32.rc

build/MinGW32/watt-32.res: watt-32.rc
	windres -DRC_DATE="$(DATE)" --target=pe-i386 -D__MINGW32__ -DDEBUG=0 -O coff -o $@ -i watt-32.rc

build/MinGW64/32bit/watt-32.res: watt-32.rc
	windres -DRC_DATE="$(DATE)" --target=pe-i386 -D__MINGW64__ -DDEBUG=0 -O coff -o $@ -i watt-32.rc

build/MinGW64/64bit/watt-32.res: watt-32.rc
	windres -DRC_DATE="$(DATE)" -DBITS=$(BITS) --target=pe-x86-64 -D__MINGW64__ -DDEBUG=0 -O coff -o $@ -i watt-32.rc

build/CygWin/32bit/watt-32.res: watt-32.rc
	windres -DRC_DATE="$(DATE)" -DBITS=32 -I../inc -DDEBUG=0 -D__CYGWIN__ -O coff -o $@ -i watt-32.rc

build/CygWin/64bit/watt-32.res: watt-32.rc
	windres -DRC_DATE="$(DATE)" -DBITS=64 -I../inc -DDEBUG=0 -D__CYGWIN__ -O coff -o $@ -i watt-32.rc

build\pellesc\32bit\watt-32.res: watt-32.rc
	porc -DDEBUG=0 -D__POCC__ -DBITS=32 -I..\inc -Fobuild\pellesc\32bit\watt-32.res watt-32.rc

build\pellesc\64bit\watt-32.res: watt-32.rc
	porc -DDEBUG=0 -D__POCC__ -DBITS=64 -I..\inc -Fobuild\pellesc\64bit\watt-32.res watt-32.rc

build\visualc\32bit\release\watt-32.res: watt-32.rc
	rc -nologo -DDEBUG=0 -D_MSC_VER -DBITS=32 -Fo build\visualc\32bit\release\watt-32.res watt-32.rc

build\visualc\32bit\debug\watt-32.res: watt-32.rc
	rc -nologo -DDEBUG=1 -D_MSC_VER -DBITS=32 -Fo build\visualc\32bit\debug\watt-32.res watt-32.rc

build\visualc\64bit\release\watt-32.res: watt-32.rc
	rc -nologo -DDEBUG=0 -D_MSC_VER -DBITS=64 -Fo build\visualc\64bit\release\watt-32.res watt-32.rc

build\visualc\64bit\debug\watt-32.res: watt-32.rc
	rc -nologo -DDEBUG=1 -D_MSC_VER -DBITS=64 -Fo build\visualc\64bit\debug\watt-32.res watt-32.rc

build\watcom\win32\watt-32.res: watt-32.rc
	wrc -dDEBUG=0 -D__WATCOMC__ -q -r -zm -fo=build\watcom\win32\watt-32.res watt-32.rc

#
# Rules for creating cflags_buf.h. A file with a C-array of the CFLAGS used.
# Included in version.c.
#
# $(W32_BIN2C) should be set by .\configur.bat to point to either
# ..\util\bin2.exe or ..\util\win32\bin2c.exe.
#
build\build\borland\small\cflags_buf.h: $(C_ARGS)
	$(W32_BIN2C) $(C_ARGS)                     > build\borland\small\cflags_buf.h

build\borland\large\cflags_buf.h: $(C_ARGS)
	$(W32_BIN2C) $(C_ARGS)                     > build\borland\large\cflags_buf.h

build\borland\flat\cflags_buf.h: $(C_ARGS)
	$(W32_BIN2C) $(C_ARGS)                     > build\borland\flat\cflags_buf.h

build\borland\win32\cflags_buf.h: $(C_ARGS)
	$(W32_BIN2C) $(C_ARGS)                     > build\borland\win32\cflags_buf.h

build/clang/32bit/cflags_buf.h:
	../util/win32/bin2c.exe $(C_ARGS)          > build/clang/32bit/cflags_buf.h

build/clang/64bit/cflags_buf.h:
	../util/win32/bin2c.exe $(C_ARGS)          > build/clang/64bit/cflags_buf.h

build/CygWin/32bit/cflags_buf.h:
	echo $(CFLAGS) | ../util/win32/bin2c.exe - > build/CygWin/32bit/cflags_buf.h

build/CygWin/64bit/cflags_buf.h:
	echo $(CFLAGS) | ../util/win32/bin2c.exe - > build/CygWin/64bit/cflags_buf.h

build\digmars\small\cflags_buf.h: $(C_ARGS)
	$(W32_BIN2C) $(C_ARGS)                     > build\digmars\small\cflags_buf.h

build\digmars\large\cflags_buf.h: $(C_ARGS)
	$(W32_BIN2C) $(C_ARGS)                     > build\digmars\large\cflags_buf.h

build\digmars\x32vm\cflags_buf.h: $(C_ARGS)
	$(W32_BIN2C) $(C_ARGS)                     > build\digmars\x32vm\cflags_buf.h

build\digmars\phar\cflags_buf.h: $(C_ARGS)
	$(W32_BIN2C) $(C_ARGS)                     > build\digmars\phar\cflags_buf.h

build\digmars\win32\cflags_buf.h: $(C_ARGS)
	$(W32_BIN2C) $(C_ARGS)                     > build\digmars\win32\cflags_buf.h

build\highc\cflags_buf.h: $(C_ARGS)
	$(W32_BIN2C) $(C_ARGS)                     > build\highc\cflags_buf.h

build\ladsoft\cflags_buf.h:
	echo $(CFLAGS) | $(W32_BIN2C) -            > build\ladsoft\cflags_buf.h

build\lcc\cflags_buf.h:
	echo $(CFLAGS) | $(W32_BIN2C) -            > build\lcc\cflags_buf.h

build/MinGW32/cflags_buf.h:
	echo $(CFLAGS) | ../util/win32/bin2c.exe - > build/MinGW32/cflags_buf.h

build/MinGW64/32bit/cflags_buf.h:
	echo $(CFLAGS) | ../util/win32/bin2c.exe - > build/MinGW64/32bit/cflags_buf.h

build/MinGW64/64bit/cflags_buf.h:
	echo $(CFLAGS) | ../util/win32/bin2c.exe - > build/MinGW64/64bit/cflags_buf.h

build\pellesc\32bit\cflags_buf.h:
	echo $(CFLAGS) | $(W32_BIN2C) -            > build\pellesc\32bit\cflags_buf.h

build\pellesc\64bit\cflags_buf.h:
	echo $(CFLAGS) | $(W32_BIN2C) -            > build\pellesc\64bit\cflags_buf.h

build\visualc\32bit\release\cflags_buf.h: $(C_ARGS)
	echo $(CFLAGS) $(CL) | $(W32_BIN2C) -      > build\visualc\32bit\release\cflags_buf.h

build\visualc\32bit\debug\cflags_buf.h: $(C_ARGS)
	echo $(CFLAGS) $(CL) | $(W32_BIN2C) -      > build\visualc\32bit\debug\cflags_buf.h

build\visualc\64bit\release\cflags_buf.h: $(C_ARGS)
	echo $(CFLAGS) $(CL) | $(W32_BIN2C) -      > build\visualc\64bit\release\cflags_buf.h

build\visualc\64bit\debug\cflags_buf.h: $(C_ARGS)
	echo $(CFLAGS) $(CL) | $(W32_BIN2C) -      > build\visualc\64bit\debug\cflags_buf.h



REPLACE = sed -e 's/\(.*\)\.o: /\n$$(OBJDIR)\/\1.o: /'


DEP_FILE = $(OBJDIR)/watt-32.dep

depend: $(OBJDIR)/cflags.h
	$(CC) -MM $(CFLAGS) $(C_SOURCE) | $(REPLACE) > $(DEP_FILE)

-include $(DEP_FILE)

