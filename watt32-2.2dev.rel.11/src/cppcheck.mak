#
# GNU Makefile for checking Watt-32 sources with CppCheck.
# Get it at http://cppcheck.sourceforge.net/
#
# NOT FINISHED.
#

.PHONY: chk_cppcheck check_one_file check_all_files

path_search = $(firstword $(wildcard $(addsuffix /$(1),$(subst ;, ,$(PATH)))))

ifeq ($(call path_search,cppcheck.exe),)
chk_cppcheck:
	@echo 'cppcheck.exe not found on $$(PATH).'
endif

ARGS = --platform=win32A --enable=style --enable=all \
       -I ../inc -I $(MINGW32)/include -i $(MINGW32)/include --relative-paths \
       --std=posix --std=c99 -D_WINDOWS_H   # pretend we've included <windows.h>

ARGS += -D__GNUC__=4 -D__GNUC_MINOR__=7 -v

ARGS += --suppress=nonreentrantFunctionsgethostbyaddr  \
        --suppress=nonreentrantFunctionsgethostbyname  \
        --suppress=nonreentrantFunctionsgethostbyname2 \
        --suppress=nonreentrantFunctionsgethostent     \
        --suppress=nonreentrantFunctionsgetnetbyaddr   \
        --suppress=nonreentrantFunctionsgetnetbyname   \
        --suppress=nonreentrantFunctionsgetprotobyname \
        --suppress=nonreentrantFunctionsgetservbyname  \
        --suppress=nonreentrantFunctionsgetservbyport  \
        --suppress=nonreentrantFunctionsgetservent     \
        --suppress=nonreentrantFunctionsstrtok         \
        --suppress=nonreentrantFunctionsecvt           \
        --suppress=nonreentrantFunctionsfcvt           \
        --suppress=nonreentrantFunctionsgcvt

ARGS += -DCPPCHECK_RUN -DWATT32_BUILD -DWIN32 \
        -D__MINGW32__     \
      # -DUSE_DEBUG       \
      # -DUSE_BOOTP       \
      # -DUSE_DHCP        \
      # -DUSE_LANGUAGE    \
      # -DUSE_FRAGMENTS   \
      # -DUSE_STATISTICS  \
      # -DUSE_BIND        \
      # -DUSE_BSD_API     \
      # -DUSE_BSD_FATAL   \
      # -DUSE_LOOPBACK    \
      # -DUSE_BUFFERED_IO \
      # -DUSE_TFTP        \
      # -DUSE_RARP        \
      # -DUSE_MULTICAST   \
      # -DUSE_GZIP        \
      # -UUSE_UDP_ONLY

CORE_SOURCE = bsdname.c  btree.c    chksum.c   country.c  crc.c      \
              echo.c     fortify.c  getopt.c   gettod.c   highc.c    \
              ip4_frag.c ip4_in.c   ip4_out.c  ip6_in.c   ip6_out.c  \
              language.c lookup.c   loopback.c misc.c     netback.c  \
              oldstuff.c pc_cbrk.c  pcarp.c    pcbootp.c  powerpak.c \
              pcbuf.c    pcconfig.c pcdbug.c   pcdhcp.c   pcicmp.c   \
              pcicmp6.c  pcintr.c   pcigmp.c   pcping.c   pcpkt.c    \
              pcpkt32.c  pcqueue.c  pcrarp.c   pcrecv.c   pcsed.c    \
              pcstat.c   pctcp.c    ports.c    ppp.c      pppoe.c    \
              qmsg.c     rs232.c    settod.c   sock_dbu.c sock_in.c  \
              sock_ini.c sock_io.c  sock_prn.c sock_scn.c sock_sel.c \
              split.c    strings.c  tcp_fsm.c  tftp.c     timer.c    \
              pcdns.c    udp_rev.c  version.c  wdpmi.c    x32vm.c    \
              idna.c     punycode.c tcp_md5.c  dynip.c    winadinf.c \
              win_dll.c  winmisc.c  winpkt.c   packet32.c profile.c  \
              swsvpkt.c

BSD_SOURCE = accept.c   adr2asc.c  asc2adr.c  bind.c     bsddbug.c  \
             close.c    connect.c  fcntl.c    fsext.c    get_ai.c   \
             get_ni.c   get_ip.c   geteth.c   gethost.c  gethost6.c \
             getname.c  getnet.c   getprot.c  getput.c   getserv.c  \
             get_xbyr.c ioctl.c    linkaddr.c listen.c   netaddr.c  \
             neterr.c   nettime.c  nsapaddr.c poll.c     presaddr.c \
             printk.c   receive.c  select.c   shutdown.c signal.c   \
             socket.c   sockopt.c  stream.c   syslog.c   syslog2.c  \
             transmit.c

BIND_SOURCE = res_comp.c res_data.c res_debu.c res_init.c res_loc.c \
              res_mkqu.c res_quer.c res_send.c

ZLIB_SOURCE = zadler32.c  zcompress.c zcrc32.c zgzio.c    zuncompr.c \
              zdeflate.c  ztrees.c    zutil.c  zinflate.c zinfback.c \
              zinftrees.c zinffast.c

SOURCES = $(CORE_SOURCE) $(BSD_SOURCE) $(BIND_SOURCE) $(ZLIB_SOURCE)

REPORT = CppCheck-report.txt

all: check_all_files
	@echo 'Done. Look at $(REPORT) for results.'

$(SOURCES): check_one_file

check_one_file:
	@echo 'Generating $(REPORT)...'
	rm -f $(REPORT)
	echo $(MAKECMDGOALS): >> $(REPORT)
	cppcheck $(ARGS) $(MAKECMDGOALS) 2>> $(REPORT)

check_one_file_2: cppcheck.args
	@echo 'Generating $(REPORT)...'
	rm -f $(REPORT)
	echo $(MAKECMDGOALS): >> $(REPORT)
	cppcheck @cppcheck.args $(MAKECMDGOALS) 2>> $(REPORT)


check_all_files:
	@echo 'Generating $(REPORT)...'
	rm -f $(REPORT)
	@for f in $(SOURCES) ; do \
    	echo $$f: >> $(REPORT) ; \
    	cppcheck $(ARGS) $$f 2>> $(REPORT) ; \
    done

#
# CppCheck doesn't support a response file yet.
#
cppcheck.args: $(MAKEFILES)
	echo 'Generating $@.'
	@echo $(ARGS) --force > $@

clean:
	rm -f $(REPORT)

