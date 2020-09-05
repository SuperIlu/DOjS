/*!\file pcsed.h
 *
 * Link-layer (DIX Ethernet, Token-Ring, ARCnet, FDDI and PPP/SLIP) Interface.
 */

#ifndef _w32_PCSED_H
#define _w32_PCSED_H

/**
 * Hardware (MAC) address numbers (host order).
 */
#define HW_TYPE_ETHER      1   /* Ether >= 10MB */
#define HW_TYPE_ETHER_3MB  2   /* Ether = 3MB */
#define HW_TYPE_AX25       3   /* Amateur packet-radio */
#define HW_TYPE_TOKEN      6   /* IEEE 802 Networks */
#define HW_TYPE_ARCNET     7   /* 2.5 MBit ARCNET */
#define HW_TYPE_APPLETALK  8   /* Not at all supported */
#define HW_TYPE_FDDI       10  /* Not really supported */

W32_CLANG_PACK_WARN_OFF()

#include <sys/pack_on.h>  /* align structs on byte boundaries */

/*!\struct eth_Header
 *
 * The DIX Ethernet header.
 */
typedef struct eth_Header {
        eth_address  destination;
        eth_address  source;
        WORD         type;
      } eth_Header;

/*!\struct eth_Packet
 *
 * The Ethernet header with data.
 */
typedef struct eth_Packet {
        eth_Header   head;
        BYTE         data [ETH_MAX_DATA];
      } eth_Packet;


/*!\struct vlan_Header
 *
 * The 802.1Q VLAN header (on Ethernet).
 */
typedef struct vlan_Header {
        eth_address  destination;
        eth_address  source;
        WORD         proto;        /* 0x8100 */
        WORD         tci;
        WORD         encap_proto;
      } vlan_Header;

/*!\struct vlan_Packet
 *
 * The 802.1Q VLAN header with data.
 */
typedef struct vlan_Packet {
        vlan_Header head;                  /* 18 */
        BYTE        data [VLAN_MAX_DATA];  /* 1496 */
      } vlan_Packet;


/*!\struct tok_Header
 *
 * Token-Ring header, refer RFC-1042, pg. 8.
 */
typedef struct tok_Header {
        /* MAC header.
         */
        BYTE        accessCtrl;
        BYTE        frameCtrl;
        mac_address destination;
        mac_address source;

        /* 2-18 bytes of Routing Information (RIF) may be present here.
         * We discard it anyway (see fix_tok_head()).
         */

        /* LLC header.
         */
        BYTE        DSAP;
        BYTE        SSAP;
        BYTE        ctrl;

        /* SNAP extension.
         */
        BYTE        org[3];
        WORD        type;
      } tok_Header;

/*!\struct tok_Packet
 *
 * Token-Ring header with data.
 */
typedef struct tok_Packet {
        tok_Header head;
        BYTE       data [TOK_MAX_DATA];
      } tok_Packet;

/*!\struct llc_Header
 *
 * LLC header format.
 */
typedef struct llc_Header {
        BYTE  DSAP;
        BYTE  SSAP;
        union {
          BYTE  u_ctl;
          WORD  is_ctl;
          struct {
            BYTE  snap_ui;
            BYTE  snap_pi[5];
          } snap;
          struct {
            BYTE  snap_ui;
            BYTE  snap_orgcode[3];
            BYTE  snap_ethertype[2];
          } snap_ether;
        } ctl;
      } llc_Header;

/**
 * From Xinu, tr.h
 */
#define TR_AC       0x10    /* Access Control; Frame bit (not token) */
#define TR_FC       0x40    /* Frame Control;  LLC header follows */
#define TR_DSAP     0xAA    /* DSAP field; SNAP follows LLC */
#define TR_SSAP     0xAA    /* SSAP field; SNAP follows LLC */
#define TR_CTRL     0x03    /* Unnumbered Information */
#define TR_ORG      0x00    /* Organisation Code or Protocol Id */


/*!\struct fddi_Header
 *
 * The FDDI header.
 */
typedef struct fddi_Header {
        BYTE        frameCtrl;
        eth_address destination;
        eth_address source;
        BYTE        DSAP;
        BYTE        SSAP;
        BYTE        ctrl;
        BYTE        org[3];
        WORD        type;
      } fddi_Header;

/*!\struct fddi_Packet
 *
 * The FDDI header with data.
 */
typedef struct fddi_Packet {
        fddi_Header head;
        BYTE        data [FDDI_MAX_DATA];
      } fddi_Packet;

#define FDDI_FC     0x40   /* See above */
#define FDDI_DSAP   0xAA
#define FDDI_SSAP   0xAA
#define FDDI_CTRL   0x03
#define FDDI_ORG    0x00


/*!\struct arcnet_Header
 *
 * The ARCNET header.
 */
typedef struct arcnet_Header {
        BYTE   source;
        BYTE   destination;
        BYTE   type;
        BYTE   flags;
        WORD   sequence;

       /* Only present in exception packets (flags == 0xFF)
        */
        BYTE   type2;     /* same as 'type' */
        BYTE   flags2;
        WORD   sequence2;
      } arcnet_Header;

#define ARC_HDRLEN    6  /* normal header length */
#define ARC_TYPE_OFS  2

/*!\struct arcnet_Packet
 *
 * The ARCNET header with data.
 */
typedef struct arcnet_Packet {
        arcnet_Header head;
        BYTE          data [ARCNET_MAX_DATA];
      } arcnet_Packet;


/*!\union link_Packet
 *
 * The union of all above MAC-headers.
 */
typedef union link_Packet {
        struct eth_Packet    eth;   /* _pktdevclass = PDCLASS_ETHER */
        struct tok_Packet    tok;   /* _pktdevclass = PDCLASS_TOKEN (_RIF) */
        struct fddi_Packet   fddi;  /* _pktdevclass = PDCLASS_FDDI  */
        struct arcnet_Packet arc;   /* _pktdevclass = PDCLASS_ARCNET */
        struct ip_Packet     ip;    /* _pktdevclass = PDCLASS_PPP/PDCLASS_SLIP */
        struct vlan_Packet   vlan;  /* not supported */
      } link_Packet;

#include <sys/pack_off.h>           /* restore default packing */

W32_CLANG_PACK_WARN_DEF()

struct _eth_last_info {
       struct {
         unsigned          size;    /* frame-size of last Rx */
         struct ulong_long tstamp;  /* timestamp of last Tx */
       } tx;
       struct {
         unsigned          size;    /* frame-size of last Rx */
         struct ulong_long tstamp;  /* timestamp of last Rx 1st upcall */
       } rx;
     };

extern struct _eth_last_info _eth_last;

extern BOOL        _ip_recursion, _eth_is_init;
extern BOOL        _eth_ndis3pkt, _eth_winpcap, _eth_npcap, _eth_win10pcap, _eth_airpcap, _eth_SwsVpkt, _eth_wanpacket;
extern BYTE        _eth_mac_len;
extern const char *_eth_not_init;

extern mac_address _eth_addr;        /* Current MAC-address (not AX-25) */
extern mac_address _eth_real_addr;   /* MAC-addr before _eth_set_addr() */
extern mac_address _eth_loop_addr;   /* CF:00:00:00:00:00 */
extern mac_address _eth_brdcast;     /* FF:FF:FF:FF:FF:FF */

#if defined(USE_MULTICAST)
  #include "pcigmp.h"

  BOOL _eth_join_mcast_group  (const struct MultiCast *mc);
  BOOL _eth_leave_mcast_group (const struct MultiCast *mc);
#endif

/*
 * Return pointer to hardware source address of
 * an IP packet. For Ethernet:
 *
 *     struct eth_Packet {
 *            BYTE  dest [6];    <-  -14
 *            BYTE  src  [6];    <-   -8
 *            WORD  type;
 *            BYTE  data [1500]; <-  ip
 *          };
 *
 * For Token-Ring:
 *     struct tok_Packet {
 *            BYTE  AC, FC;            <-  -22
 *            BYTE  dest [6];
 *            BYTE  src  [6];          <-  -14
 *            BYTE  DSAP, SSAP, ctrl;
 *            BYTE  org [3];
 *            WORD  type;
 *            BYTE  data [1500];       <-  ip
 *
 * These macros and functions should never be called for serial protocols
 * except that it doesn't hurt to use MAC_SRC() for all driver classes.
 */

#if defined(NOT_USED)
  /* Slower, but safer method. The ATTR_NORETURN() is for the cases
   * where these used functions are used wrongly. In which case we exit().
   */
  extern void *_eth_mac_hdr (const in_Header *ip)  ATTR_NORETURN();
  extern void *_eth_mac_dst (const in_Header *ip)  ATTR_NORETURN();
  extern void *_eth_mac_src (const in_Header *ip)  ATTR_NORETURN();
  extern WORD  _eth_mac_typ (const in_Header *ip)  ATTR_NORETURN();

  #define MAC_HDR(ip) _eth_mac_hdr(ip)
  #define MAC_DST(ip) _eth_mac_dst(ip)
  #define MAC_SRC(ip) _eth_mac_src(ip)
  #define MAC_TYP(ip) _eth_mac_typ(ip)
#else
  #define MAC_HDR(ip) (void*) ((BYTE*)(ip) - _pkt_ip_ofs)

  #define MAC_DST(ip) (void*) ((BYTE*)(ip) -                              \
                        (_pktdevclass == PDCLASS_TOKEN  ? _pkt_ip_ofs-2 : \
                         _pktdevclass == PDCLASS_ARCNET ? _pkt_ip_ofs-1 : \
                         _pktdevclass == PDCLASS_FDDI   ? _pkt_ip_ofs-5 : \
                         _pkt_ip_ofs))

  #define MAC_SRC(ip) (void*) ((BYTE*)(ip) -                              \
                        (_pktdevclass == PDCLASS_TOKEN  ? _pkt_ip_ofs-8 : \
                         _pktdevclass == PDCLASS_ARCNET ? _pkt_ip_ofs   : \
                         _pktdevclass == PDCLASS_FDDI   ? _pkt_ip_ofs-7 : \
                         _pkt_ip_ofs-6))

  #define MAC_TYP(ip) (*(WORD*) ((BYTE*)(ip) - \
                      (_pktdevclass == PDCLASS_ARCNET ? _pkt_ip_ofs-2 : 2)))
#endif

/*
 * Macros to set or clear peer's source MAC-address stored in
 * the _udp_Socket / _tcp_Socket.
 * Only used on passive (listening) udp/tcp sockets.
 * We don't use arp_resolve() because of reentrancy problems.
 * But if inbound packets come from a different gateway than
 * outbound packets, then we're toast..
 */
#define SET_PEER_MAC_ADDR(tcb,ip) do {                        \
          if (!_pktserial)                                    \
             memcpy (&(tcb)->his_ethaddr[0], MAC_SRC(ip), 6); \
        } while (0)

#define CLR_PEER_MAC_ADDR(tcb) do {                 \
          if (!_pktserial)                          \
             memset (&(tcb)->his_ethaddr[0], 0, 6); \
        } while (0)

#endif
