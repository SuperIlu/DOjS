#
# GNU Makefile for checking sources with LC-lint.
# djgpp 2.03+ required.
#

.SUFFIXES: .lnt

path_find = $(wildcard $(addsuffix /$(1),$(subst ;, ,$(subst \,/,$(PATH)))))

ifeq ($(call path_find,lclint.exe),)
  LINT = e:/djgpp/contrib/lclint.24/bin/lclint.exe
else
  LINT = lclint
endif

LFLAGS = +standard -I../inc -I$(DJDIR)/include -sysdirs $(DJDIR)/include \
         -Dlint -D__DJGPP__=2 -D__GNUC__=3 -D__GNUC_MINOR__=1 -DDOSX=2   \
         -warnposix -linelen 80 -nullassign -noeffect -mustfree          \
         -duplicatequals +ignoresigns -predboolothers -predboolint       \
         -boolops +boolint +charint -exportlocal -nullpass -nullret      \
         -unqualifiedtrans -onlytrans -branchstate -globstate -type      \
         -retvalbool -retvalother -retvalint -compmempass -temptrans     \
         -nullstate -compdef -exitarg -nestedextern -statictrans         \
         -immediatetrans -castfcnptr -mayaliasunique -modobserver        \
         -uniondef -usereleased -fullinitblock -macromatchname

SOURCES = accept.c   adr2asc.c  asc2adr.c  bind.c     bsddbug.c  bsdname.c  \
          btree.c    chksum.c   close.c    connect.c  country.c  crc.c      \
          echo.c     fcntl.c    fortify.c  fsext.c    get_ai.c   get_ni.c   \
          geteth.c   gethost.c  gethost6.c getname.c  getnet.c   getopt.c   \
          getprot.c  getput.c   getserv.c  gettod.c   highc.c    ioctl.c    \
          ip4_frag.c ip4_in.c   ip4_out.c  ip6_in.c   ip6_out.c  language.c \
          linkaddr.c listen.c   lookup.c   loopback.c misc.c     netaddr.c  \
          netback.c  neterr.c   nettime.c  nsapaddr.c oldstuff.c pc_cbrk.c  \
          pcarp.c    pcbootp.c  pcbsd.c    pcbuf.c    pcconfig.c pcdbug.c   \
          pcdhcp.c   pcicmp.c   pcicmp6.c  pcintr.c   pcmulti.c  pcping.c   \
          pcpkt.c    pcpkt32.c  pcqueue.c  pcrarp.c   pcrecv.c   pcsed.c    \
          pcslip.c   pcstat.c   pctcp.c    poll.c     ports.c    ppp.c      \
          pppoe.c    presaddr.c printk.c   qmsg.c     receive.c  res_comp.c \
          res_data.c res_debu.c res_init.c res_loc.c  res_mkqu.c res_quer.c \
          res_send.c select.c   settod.c   shutdown.c signal.c   sock_dbu.c \
          sock_in.c  sock_ini.c sock_io.c  sock_prn.c sock_scn.c sock_sel.c \
          socket.c   sockopt.c  split.c    stream.c   strings.c  syslog.c   \
          syslog2.c  tcp_fsm.c  tftp.c     timer.c    transmit.c udp_dom.c  \
          udp_nds.c  udp_rev.c  version.c  w32pcap.c  wdpmi.c    x32vm.c    \
          rs232.c

LFILES = $(SOURCES:.c=.lnt)

all: msg $(LFILES)

msg:
	@echo 'Generating *.lnt files...'

.c.lnt:
	@echo $<
	@$(LINT) $(LFLAGS) $< > $@

clean:
	rm -f $(LFILES)

