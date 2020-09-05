/*!\file pcigmp.h
 */
#ifndef _w32_PCMULTI_H
#define _w32_PCMULTI_H

/*!\struct IGMPv0_packet
 *
 * Internet Group Management Protocol packet version 0.
 */
typedef struct IGMPv0_packet {
        BYTE    type;
        BYTE    code;
        WORD    checksum;
        WORD    ident;
        DWORD   address;
        DWORD   access_key;
      } IGMPv0_packet;

/*!\struct IGMPv1_packet
 *
 * Internet Group Management Protocol packet version 1.
 */
typedef struct IGMPv1_packet {
        int    type    : 4;
        int    version : 4;
        BYTE   unused;
        WORD   checksum;
        DWORD  address;
      } IGMPv1_packet;

/*!\struct IGMPv2_packet
 *
 * Internet Group Management Protocol packet version 2.
 */
typedef struct IGMPv2_packet {
        BYTE   type;
        BYTE   max_resp;
        WORD   checksum;
        DWORD  address;
      } IGMPv2_packet;

/*!\struct IGMPv3_packet
 *
 * Internet Group Management Protocol packet version 3.
 */
typedef struct IGMPv3_packet {
        BYTE   type;
        BYTE   max_resp;
        WORD   checksum;
        DWORD  address;
        int    QRV   : 3;
        int    s_bit : 1;
        int    resv  : 4;
        BYTE   QQIC;
        WORD   num_src;
        DWORD  src_addr;  /* \todo: Fixme; array of IPs */
      } IGMPv3_packet;


#define IGMP_VERSION_1         1
#define IGMP_VERSION_2         2

/* 'type' field for IGMP v0 */
#define IGMPv0_CG_REQUEST      1
#define IGMPv0_CG_REPORT       2
#define IGMPv0_JG_REQUEST      3
#define IGMPv0_JG_REPLY        4
#define IGMPv0_LG_REQUEST      5
#define IGMPv0_LG_REPLY        6
#define IGMPv0_CONFIRM_REQUEST 7
#define IGMPv0_CONFIRM_REPLY   8

/* 'type' field for IGMP v1 */
#define IGMPv1_QUERY           1
#define IGMPv1_REPORT          2
#define IGMPv1_DVMRP           3

/* 'type' field for IGMP v2 */
#define IGMPv2_MEMB_QUERY      0x11
#define IGMPv2_MEMB_v1_REPORT  0x12
#define IGMPv2_DVMRP           0x13
#define IGMPv2_PIMv1           0x14

/* etc, etc... */

/* 'type' field for IGMP v3 */
#define IGMPv3_QUERY           0x11
#define IGMPv3_REPORT          0x22
#define IGMPv3_MEMB_v1_REPORT  0x12
#define IGMPv3_MEMB_v2_REPORT  0x16
#define IGMPv3_LEAVE           0x17

#if defined(USE_MULTICAST)
  /**
   * Stuff for Multicast Support - JRM 6/7/93.
   */
  #define IPMULTI_SIZE    20            /**< the size of the ipmulti table     */
  #define MCAST_ALL_SYST  0xE0000001UL  /**< the default mcast addr 224.0.0.1  */

  /**\struct MultiCast
   *
   * Multicast internal structure.
   */
  struct MultiCast {
         DWORD       ip;           /**< IP address of group */
         eth_address ethaddr;      /**< Ethernet address of group */
         BYTE        processes;    /**< number of interested processes */
         DWORD       reply_timer;  /**< IGMP query reply timer */
         BOOL        active;       /**< is this an active entry */
       };

  extern void igmp_handler (const in_Header *ip, BOOL brdcast);
#endif
#endif
