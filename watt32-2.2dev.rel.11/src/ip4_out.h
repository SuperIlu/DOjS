/*!\file ip4_out.h
 */
#ifndef _w32_IP_OUT_H
#define _w32_IP_OUT_H

extern BOOL _ip4_dont_frag;     /*!< Enable the IP_DF bit on output */
extern int  _ip4_id_increment;  /*!< The IP identifier increment */
extern int  _default_ttl;       /*!< The default time-to-live for IP */
extern BYTE _default_tos;       /*!< The default type-of-service for IP */

extern WORD _get_ip4_id (void);
extern WORD _get_this_ip4_id (void);

extern int _ip4_output (in_Header *ip, DWORD src_ip, DWORD dst_ip,
                        BYTE protocol, BYTE ttl, BYTE tos, WORD ip_id,
                        int data_len, const void *sock,
                        const char *file, unsigned line);

#define IP4_OUTPUT(ip, src, dst, proto, ttl, tos, ip_id, data_len, sock) \
       _ip4_output(ip, src, dst, proto, ttl, tos, ip_id, data_len, sock, \
                   __FILE__, __LINE__)


#if defined(USE_FRAGMENTS)
  int _ip4_send_fragments (sock_type *sk, BYTE proto, DWORD dest,
                           const void *buf, unsigned len, const char *file,
                           unsigned line);

  #define _IP4_SEND_FRAGMENTS(sk, proto, dest, buf, len) \
          _ip4_send_fragments(sk, proto, dest, buf, len, __FILE__, __LINE__)
#endif

#endif
