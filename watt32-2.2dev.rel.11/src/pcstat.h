/*!\file pcstat.h
 */
#ifndef _w32_PCSTAT_H
#define _w32_PCSTAT_H

#ifndef ICMP_MAXTYPE
#define ICMP_MAXTYPE 18
#endif

#include <sys/socket.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_ether.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/in_pcb.h>
#include <netinet/tcp.h>
#include <netinet/tcp_time.h>
#include <netinet/tcpip.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netinet/tcp_var.h>
#include <netinet/icmp_var.h>
#include <netinet/igmp_var.h>
#include <netinet/icmp6.h>
#include <netinet6/ip6_var.h>

/*!\struct macstat
 *
 * MAC-layer statistics.
 */
struct macstat {
       DWORD  non_ip_recv;            /* # of non-IP packets received */
       DWORD  non_ip_sent;            /*                     sent */
       DWORD  num_tx_err;             /* # of transmission errors */
       DWORD  num_tx_retry;           /* # of transmission retries */
       DWORD  num_tx_timeout;         /* # of async transmission timeouts */
       DWORD  num_unk_type;           /* # of unhandled types */
       DWORD  num_llc_frames;         /* # of LLC frames received */
       DWORD  num_wrong_handle;       /* # upcalls with wrong handle */
       DWORD  num_bad_sync;           /* # upcalls with different lengths */
       DWORD  num_too_large;          /* # upcalls with > ETH_MAX pkts */
       DWORD  num_too_small;          /* # upcalls with < ETH_MIN pkts */
       DWORD  num_mac_loop;           /* # of pkts looped by NDIS */
       DWORD  num_ip_recurse;         /* # of IP-pkts blocked due to recursion */

       /*!\struct arp */
       struct {
         DWORD request_recv;          /* # of ARP requests received */
         DWORD request_sent;          /*                   sent */
         DWORD reply_recv;            /* # of ARP replies received */
         DWORD reply_sent;            /*                  sent */
       } arp;

       /*!\struct rarp */
       struct {
         DWORD request_recv;          /* # of RARP requests received */
         DWORD request_sent;          /*                    sent */
         DWORD reply_recv;            /* # of RARP replies received */
         DWORD reply_sent;            /*                   sent */
       } rarp;
     };

/*!\struct pppoestat
 *
 * PPP-over-Ethernet statistics.
 */
struct pppoestat {
       DWORD  num_disc_sent;
       DWORD  num_disc_recv;
       DWORD  num_sess_sent;
       DWORD  num_sess_recv;
     };

/*!\struct cache_stat
 *
 * Statistics for various cache hits/misses.
 */
struct cache_stat {
       DWORD  num_arp_search;
       DWORD  num_arp_hits;
       DWORD  num_arp_misses;
       DWORD  num_arp_overflow;
       DWORD  num_route_hits;
       DWORD  num_route_misses;
     };

/*
 * IP statistics for all in/out messages, <netinet/ip_var.h>
 *
 * ips_total;           - total packets received
 * ips_badsum;          - checksum bad
 * ips_tooshort;        - packet too short
 * ips_toosmall;        - not enough data
 * ips_badhlen;         - ip header length < data size
 * ips_badlen;          - ip length < ip header length
 * ips_fragments;       - fragments received
 * ips_fragdropped;     - frags dropped (dups, out of space)
 * ips_fragtimeout;     - fragments timed out
 * ips_forward;         - packets forwarded
 * ips_cantforward;     - packets rcvd for unreachable dest
 * ips_redirectsent;    - packets forwarded on same net
 * ips_noproto;         - unknown or unsupported protocol
 * ips_delivered;       - datagrams delivered to upper level
 * ips_localout;        - total ip packets generated here
 * ips_odropped;        - lost out packets due to nobufs, etc.
 * ips_idropped;        - lost in packets due to nobufs, etc.  !! added
 * ips_reassembled;     - total packets reassembled ok
 * ips_fragmented;      - datagrams successfully fragmented
 * ips_ofragments;      - output fragments created
 * ips_cantfrag;        - don't fragment flag was set, etc.
 * ips_badoptions;      - error in option processing
 * ips_noroute;         - packets discarded due to no route
 * ips_badvers;         - ip version != 4
 * ips_rawout;          - total raw ip packets generated
 * ips_toolong;         - ip length > max ip packet size
 *
 *
 * UDP statistics, <netinet/udp_var.h>
 *
 * udps_ipackets        - total input packets
 * udps_hdrops          - packet shorter than header
 * udps_badsum          - checksum error
 * udps_badlen          - data length larger than packet
 * udps_noport          - no socket on port
 * udps_noportbcast     - as above, arrived as broadcast
 * udps_fullsock        - not delivered, input socket full
 * udpps_pcbcachemiss   - input packets missing pcb cache
 * udpps_pcbhashmiss    - input packets not for hashed pcb
 * udps_opackets        - total output packets
 *
 *
 * TCP statistics for all connections, <netinet/tcp_var.h>
 *
 * tcps_connattempt     - connections initiated         (output)
 * tcps_accepts         - connections accepted          (input)
 * tcps_connects        - connections established       (input)
 * tcps_drops           - connections dropped           (output)
 * tcps_conndrops       - embryonic connections dropped (output)
 * tcps_closed          - conn. closed (includes drops)
 * tcps_segstimed       - segs where we tried to get rtt
 * tcps_rttupdated      - times we succeeded
 * tcps_delack          - delayed acks sent
 * tcps_timeoutdrop     - conn. dropped in rxmt timeout
 * tcps_rexmttimeo      - retransmit timeouts
 * tcps_persisttimeo    - persist timeouts
 * tcps_keeptimeo       - keepalive timeouts
 * tcps_keepprobe       - keepalive probes sent
 * tcps_keepdrops       - connections dropped in keepalive
 *
 * tcps_sndtotal        - total packets sent
 * tcps_sndpack         - data packets sent
 * tcps_sndbyte         - data bytes sent
 * tcps_sndrexmitpack   - data packets retransmitted
 * tcps_sndrexmitbyte   - data bytes retransmitted
 * tcps_sndacks         - ack-only packets sent
 * tcps_sndprobe        - window probes sent
 * tcps_sndurg          - packets sent with URG only
 * tcps_sndwinup        - window update  -only packets sent
 * tcps_sndctrl         - control (SYN|FIN|RST) packets sent
 *
 * tcps_rcvtotal        - total packets received
 * tcps_rcvpack         - packets received in sequence
 * tcps_rcvbyte         - bytes received in sequence
 * tcps_rcvbadsum       - packets received with ccksum errs
 * tcps_rcvbadoff       - packets received with bad offset
 * tcps_rcvshort        - packets received too short
 * tcps_rcvduppack      - duplicate-only packets received
 * tcps_rcvdupbyte      - duplicate-only bytes received
 * tcps_rcvpartduppack  - packets with some duplicate data
 * tcps_rcvpartdupbyte  - dup. bytes in part-dup. packets
 * tcps_rcvoopack       - out-of-order packets received
 * tcps_rcvoobyte       - out-of-order bytes received
 * tcps_rcvpackafterwin - packets with data after window
 * tcps_rcvbyteafterwin - bytes rcvd after window
 * tcps_rcvafterclose   - packets rcvd after "close"
 * tcps_rcvwinprobe     - rcvd window probe packets
 * tcps_rcvdupack       - rcvd duplicate acks
 * tcps_rcvacktoomuch   - rcvd acks for unsent data
 * tcps_rcvackpack      - rcvd ack packets
 * tcps_rcvackbyte      - bytes acked by rcvd acks
 * tcps_rcvwinupd       - rcvd window update packets
 * tcps_pawsdrop        - segments dropped due to PAWS (protected against wrapped segments)
 * tcps_predack         - times hdr predict ok for acks
 * tcps_preddat         - times hdr predict ok for data pkts
 * tcps_pcbcachemiss    - times missed cache for input segment
 * tcps_cachedrtt       - times cached RTT in route updated
 * tcps_cachedrttvar    - times cached rttvar updated
 * tcps_cachedssthresh  - times cached ssthresh updated
 * tcps_usedrtt         - times RTT initialized from route
 * tcps_usedrttvar      - times RTTVAR initialized from rt
 * tcps_usedssthresh    - times ssthresh initialized from rt
 * tcps_persistdrop     - timeout in persist state
 * tcps_badsyn          - bogus SYN, e.g. premature ACK
 * tcps_mturesent       - resends due to MTU discovery
 * tcps_listendrop      - listen queue overflows
 * #ifdef TCP_FACK
 * tcps_fack_recovery;     - recovery episodes
 * tcps_fack_sndpack;      - data packets sent
 * tcps_fack_sndbyte;      - data bytes sent
 * tcps_fack_sndrexmitpack - data packets retransmitted
 * tcps_fack_sndrexmitbyte - data bytes retransmitted
 * #endif
 *
 *
 * ICMP statistics, in <netinet/icmp_var.h>
 *
 * icps_error           - # of calls to icmp_error
 * icps_oldshort        - no error 'cuz old ip too short
 * icps_oldicmp         - no error 'cuz old was icmp
 * icps_outhist[18+1]   - output counters for ICMP_xx
 * icps_badcode         - icmp_code out of range
 * icps_tooshort        - packet < ICMP_MINLEN
 * icps_checksum        - bad checksum
 * icps_badlen          - calculated bound mismatch
 * icps_reflect         - number of responses
 * icps_inhist[18+1]    - input counters for ICMP_xx
 *
 *
 * IGMP statistics, <netinet/igmp_var.h>
 *
 * igps_rcv_total       - total IGMP messages received
 * igps_rcv_tooshort    - received with too few bytes
 * igps_rcv_badsum      - received with bad checksum
 * igps_rcv_queries     - received membership queries
 * igps_rcv_badqueries  - received invalid queries
 * igps_rcv_reports     - received membership reports
 * igps_rcv_badreports  - received invalid reports
 * igps_rcv_ourreports  - received reports for our groups
 * igps_snd_reports     - sent membership reports
 */

#if defined(USE_STATISTICS)
  #define macstats     W32_NAMESPACE (macstats)
  #define ip4stats     W32_NAMESPACE (ip4stats)
  #define ip6stats     W32_NAMESPACE (ip6stats)
  #define udpstats     W32_NAMESPACE (udpstats)
  #define tcpstats     W32_NAMESPACE (tcpstats)
  #define icmpstats    W32_NAMESPACE (icmpstats)
  #define icmp6stats   W32_NAMESPACE (icmp6stats)
  #define igmpstats    W32_NAMESPACE (igmpstats)
  #define pppoestats   W32_NAMESPACE (pppoestats)
  #define cache_stats  W32_NAMESPACE (cache_stats)

  extern struct macstat    macstats;
  extern struct ipstat     ip4stats;
  extern struct ip6stat    ip6stats;
  extern struct udpstat    udpstats;
  extern struct tcpstat    tcpstats;
  extern struct icmpstat   icmpstats;
  extern struct icmp6stat  icmp6stats;
  extern struct igmpstat   igmpstats;
  extern struct pppoestat  pppoestats;
  extern struct cache_stat cache_stats;

  extern void update_in_stat  (void);
  extern void update_out_stat (void);

  #define STAT(x) x

#else
  #define STAT(x)  ((void)0)
#endif

#endif /* !_w32_PCSTAT_H */

