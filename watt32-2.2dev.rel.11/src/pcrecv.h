/*!\file pcrecv.h
 */
#ifndef _w32_PCRECV_H
#define _w32_PCRECV_H

/*!\struct recv_data
 */
typedef struct recv_data {
        DWORD  recv_sig;
        BYTE  *recv_bufs;
        WORD   recv_bufnum;
      } recv_data;

/*!\struct recv_buf
 */
typedef struct recv_buf {
        DWORD       buf_sig;
        DWORD       buf_hisip;
        long        buf_seqnum;
#if defined(USE_IPV6)
        ip6_address buf_hisip6;
#endif
        WORD        buf_hisport;
        short       buf_len;
        BYTE        buf_data [ETH_MAX]; /* sock_packet_peek() needs 1514 */
      } recv_buf;

#endif
