/*!\file pcbuf.h
 */
#ifndef _w32_PCBUF_H
#define _w32_PCBUF_H

#if 0
#define VALID_UDP  1
#define VALID_TCP  2
#define VALID_IP4  3
#define VALID_IP6  4
#endif

extern void _sock_check_tcp_buffers (const _tcp_Socket *tcp);
extern void _sock_check_udp_buffers (const _udp_Socket *udp);

extern const _raw_Socket  *find_oldest_raw  (const _raw_Socket *raw);

#if defined(USE_IPV6)
extern const _raw6_Socket *find_oldest_raw6 (const _raw6_Socket *raw);
#endif

#endif

