/*!\file chksum.h
 */
#ifndef _w32_CHECK_SUM_H
#define _w32_CHECK_SUM_H

extern int  do_checksum (const BYTE *buf, BYTE proto, unsigned len);

extern WORD _ip6_checksum (const in6_Header *ip, WORD proto,
                           const void *payload, unsigned payloadlen);

extern int  _ip6_tcp_checksum  (const in6_Header *ip, const tcp_Header *tcp, unsigned len);
extern int  _ip6_udp_checksum  (const in6_Header *ip, const udp_Header *udp, unsigned len);
extern int  _ip6_icmp_checksum (const in6_Header *ip, const void *icmp, unsigned len);

/**
 * \def CHECKSUM
 * All targets now uses this C-version of `in_checksum()`.
 */
#define CHECKSUM(p, len)    in_checksum (p, len)
#endif
