#
# Check all/most .h-files for error using 'clang-cl'.
#
# Does not work with '_KERNEL' defined.
#
USE_CPP_MODE ?= 1

export CL=

CFLAGS = -Wall -I.                \
         -ferror-limit=5          \
         -DSTRUCT_IFPREFIX_NEEDED \
         -DBSD=199103             \
       # -DKERNEL                 \
       # -D_KERNEL

ifeq ($(USE_CPP_MODE),1)
  CFLAGS += -TC
endif

all: check_hdr.c
	clang-cl -c $(CFLAGS) check_hdr.c
	@rm -f check_hdr.obj

check_hdr.c: check_hdr.mak
	$(info Generating $@)
	$(file  > $@, /* Generated file. NO NOT EDIT!)
	$(file >> $@,  */)
	$(file >> $@,$(check_hdr_c))

clean:
	rm -f check_hdr.obj check_hdr.c

#
# The meat of 'check_hdr.c':
#
define check_hdr_c
  #if defined(__clang__)
    #pragma clang diagnostic ignored "-Wreserved-id-macro"
    #pragma clang diagnostic ignored "-Wstrict-prototypes"
//  #pragma clang diagnostic ignored "-Wzero-length-array"
    #pragma clang diagnostic ignored "-Wnonportable-system-include-path"

    #if (__clang_major__ >= 13)
    #pragma clang diagnostic ignored "-Wreserved-identifier"
    #endif
  #endif

  #include <tcp.h>

  W32_GCC_PRAGMA (clang diagnostic ignored  "-Wvisibility")
  W32_GCC_PRAGMA (clang diagnostic ignored  "-Wpragma-pack")

  #include <sys/wtypes.h>
  #include <sys/werrno.h>
  #include <sys/whide.h>
  #include <sys/wtime.h>

  #include <err.h>
  #include <netdb.h>
  #include <poll.h>
  #include <resolv.h>
  #include <syslog.h>

  #include <arpa/ftp.h>
  #include <arpa/inet.h>
  #include <arpa/nameser.h>
  #include <arpa/telnet.h>
  #include <arpa/tftp.h>

  #include <net/bpf.h>
  #include <net/bpfdesc.h>
  #include <net/ethertyp.h>
  #include <net/if.h>
  #include <net/if_arc.h>
  #include <net/if_arp.h>
  #include <net/if_atm.h>
  #include <net/if_dl.h>
  #include <net/if_ether.h>
  #include <net/if_fddi.h>
  #include <net/if_llc.h>
  #include <net/if_media.h>
  #include <net/if_packe.h>
  #include <net/if_packet.h>
  #include <net/if_ppp.h>
  #include <net/if_pppva.h>
  #include <net/if_slvar.h>
  #include <net/if_strip.h>
  #include <net/if_tun.h>
  #include <net/if_types.h>
  #include <net/netisr.h>
  #include <net/pfil.h>
  #include <net/ppp-comp.h>
  #include <net/ppp_defs.h>
  #include <net/radix.h>
  #include <net/raw_cb.h>
  #include <net/route.h>
  #include <net/slcompre.h>
  #include <net/slip.h>

  #include <netinet/icmp6.h>
  #include <netinet/icmp_var.h>
  #include <netinet/if_ether.h>
  #include <netinet/if_fddi.h>
  #include <netinet/igmp.h>
  #include <netinet/igmp_var.h>
  #include <netinet/ip.h>
  #include <netinet/in.h>
  #include <netinet/in_pcb.h>
  #include <netinet/in_systm.h>
  #include <netinet/in_var.h>
  #include <netinet/ip6.h>
  #include <netinet/ipv6.h>
  #include <netinet/ip_fw.h>
  #include <netinet/ip_icmp.h>
  #include <netinet/ip_mrout.h>
  #include <netinet/ip_var.h>
  #include <netinet/tcp.h>
  #include <netinet/tcpip.h>
  #include <netinet/tcp_debu.h>
  #include <netinet/tcp_fsm.h>
  #include <netinet/tcp_scor.h>
  #include <netinet/tcp_seq.h>
  #include <netinet/tcp_time.h>
  #include <netinet/tcp_var.h>
  #include <netinet/udp.h>
  #include <netinet/udp_var.h>

  #include <netinet6/ipsec.h>
  #include <netinet6/ipsec6.h>

  #include <netinet6/ah.h>
  #include <netinet6/ah6.h>
  #include <netinet6/esp.h>
  #include <netinet6/esp6.h>
  #include <netinet6/esp_rijn.h>
  #include <netinet6/in6.h>
  #include <netinet6/in6_gif.h>
  #include <netinet6/in6_ifat.h>
  #include <netinet6/in6_pcb.h>
  #include <netinet6/in6_pref.h>
  #include <netinet6/in6_var.h>
  #include <netinet6/ip6proto.h>
  #include <netinet6/ip6_ecn.h>
  #include <netinet6/ip6_fw.h>
  #include <netinet6/ip6_mrou.h>
  #include <netinet6/ip6_var.h>
  #include <netinet6/ipcomp.h>
  #include <netinet6/ipcomp6.h>
  #include <netinet6/mld6_var.h>
  #include <netinet6/nd6.h>
  #include <netinet6/pim6.h>
  #include <netinet6/pim6_var.h>
  #include <netinet6/raw_ip6.h>
  #include <netinet6/scope6_v.h>
  #include <netinet6/tcp6_var.h>
  #include <netinet6/udp6_var.h>

  #include <protocol/dumprest.h>
  #include <protocol/routed.h>
  #include <protocol/rwhod.h>
  #include <protocol/talkd.h>
  #include <protocol/timed.h>

  #include <rpc/auth.h>
  #include <rpc/auth_des.h>
  #include <rpc/auth_uni.h>
  #include <rpc/clnt.h>
  #include <rpc/key_prot.h>
  #include <rpc/pmap_cln.h>
  #include <rpc/pmap_pro.h>
  #include <rpc/pmap_rmt.h>
  #include <rpc/rpc.h>
  #include <rpc/rpc_msg.h>
  #include <rpc/svc.h>
  #include <rpc/svc_auth.h>
  #include <rpc/types.h>
  #include <rpc/xdr.h>
  #include <rpcsvc/ypclnt.h>
  #include <rpcsvc/yp_prot.h>

  #include <sys/cdefs.h>
  #include <sys/errno.h>
  #include <sys/ioctl.h>
  #include <sys/mbuf.h>
  #include <sys/param.h>
  #include <sys/poll.h>
  #include <sys/queue.h>
  #include <sys/select.h>
  #include <sys/socket.h>
  #include <sys/so_ioctl.h>
  #include <sys/swap.h>
  #include <sys/syslog.h>
  #include <sys/uio.h>
  #include <sys/un.h>
  #include <sys/w32api.h>

  #include <w32-fakes/winsock.h>
  #include <w32-fakes/winsock2.h>
  #include <w32-fakes/ws2tcpip.h>
endef
