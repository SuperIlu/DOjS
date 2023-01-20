/*!\file winpkt.h
 *
 * Only included on Win32
 */
#ifndef _w32_WINPKT_H
#define _w32_WINPKT_H

#if !defined(_w32_PCPKT_H)
#error Include this file inside or after pcpkt.h only.
#endif

/**
 * Make WinPcap/npf.sys behave like a Packet-driver.
 * Errors below 0 are WinPkt.c errors. Others come from GetLastError().
 * \todo Add some npf/NDIS specific errors?
 */
#define PDERR_GEN_FAIL      -1
#define PDERR_NO_DRIVER     -2
#define PDERR_NO_CLASS      -3
#define PDERR_NO_MULTICAST  -4
#define PDERR_NO_SPACE      -5

enum ReceiveModes {
     RXMODE_OFF        = 0x00,    /**< possible to turn off? */
     RXMODE_DIRECT     = 0x01,    /**< NDIS_PACKET_TYPE_DIRECTED */
     RXMODE_MULTICAST1 = 0x02,    /**< NDIS_PACKET_TYPE_MULTICAST */
     RXMODE_MULTICAST2 = 0x04,    /**< NDIS_PACKET_TYPE_ALL_MULTICAST */
     RXMODE_BROADCAST  = 0x08,    /**< NDIS_PACKET_TYPE_BROADCAST */
     RXMODE_PROMISCOUS = 0x20,    /**< NDIS_PACKET_TYPE_PROMISCUOUS */
     RXMODE_ALL_LOCAL  = 0x80     /**< NDIS_PACKET_TYPE_ALL_LOCAL (direct+bc+mc1) */
     /* we have no use for the other NDIS_PACKET_TYPE defs */
   };

/* NPF defaults to mode 0 after openening adapter.
 */
#define RXMODE_DEFAULT  (RXMODE_DIRECT | RXMODE_BROADCAST)

extern struct pkt_rx_element *pkt_poll_recv (void);

#if defined(USE_MULTICAST)
  extern int pkt_get_multicast_list (mac_address *listbuf, int *len);
  extern int pkt_set_multicast_list (const void *listbuf, int len);
#endif

typedef BOOL        (*func_init)        (void);
typedef void       *(*func_open)        (const char *name, ...);
typedef BOOL        (*func_close)       (void *a);
typedef UINT        (*func_send)        (const void *a, const void *buf, UINT buf_len); /* returns length of buf sent */
typedef BOOL        (*func_get_mac)     (const void *a, const void *mac);
typedef BOOL        (*func_get_stats)   (const void *a, struct PktStats *stats, struct PktStats *total);
typedef BOOL        (*func_get_if_stat) (const void *a, BOOL *up);
typedef BOOL        (*func_get_if_type) (const void *a, WORD *type);
typedef BOOL        (*func_get_if_mtu)  (const void *a, DWORD *Mbit_s);
typedef BOOL        (*func_get_if_speed)(const void *a, DWORD *Mbit_s);
typedef BOOL        (*func_get_descr)   (const void *a, char *descr, size_t max);
typedef const char *(*func_get_drv_ver) (void);

/* This func-ptr is used in 'winpcap_recv_thread()' outside of the
 * critical region. Hence we force a 'cdecl' on MSVC since registers
 * doesn't always seems to be preserved across thread-switches.
 * See '\note' in 'winpcap_recv_thread()' docs.
 */
typedef UINT (MS_CDECL *func_recv) (const void *a, void *buf, UINT buf_len);       /* returns length of buf received */

/**\struct pkt_info
 *
 * Placeholder for vital data accessed by capture thread.
 */
struct pkt_info {
       const void   *adapter_info;     /* opaque ADAPTER_INFO object (WinPcap only) */
       const void   *adapter;          /* opaque ADAPTER or SwsVpktUsr object */
       void         *npf_buf;          /* WinPcap: buffer for ReadFile() */
       int           npf_buf_size;     /* WinPcap: size of above buffer */
       HANDLE        recv_thread;      /* WinPcap: thread for capturing */
       WORD          pkt_ip_ofs;       /* store length of MAC-header */
       WORD          pkt_type_ofs;     /* offset to MAC-type */
       volatile BOOL stop_thread;      /* signal thread to stop */
       volatile BOOL thread_stopped;   /* did it stop gracefully? */

       const char *api_name;           /* "WinPcap", "SwsVpkt" etc. */
       const char *sys_drvr_name;      /* "NPF.sys", "SwsVpkt.sys" etc. */

       /* Function pointers to transmit(), recv() etc. Depending
        * on the type of low-level driver we use (NPF/SwsVpkt/WanPacket).
        */
       func_init         init_op;
       func_open         open_op;
       func_close        close_op;
       func_send         send_op;
       func_recv         recv_op;
       func_get_mac      get_mac_op;
       func_get_stats    get_stats_op;
       func_get_if_stat  get_if_stat_op;
       func_get_if_type  get_if_type_op;
       func_get_if_mtu   get_if_mtu_op;
       func_get_if_speed get_if_speed_op;
       func_get_descr    get_descr_op;
       func_get_drv_ver  get_drv_ver_op;

       const char   *error;              /* Last error (NULL if none) */
       struct pkt_ringbuf    pkt_queue;  /* Ring-struct for enqueueing */
       struct pkt_rx_element rx_buf [RX_BUFS]; /* the receive buffer */
     };

extern char _pktdrvr_descr[];

extern struct pkt_info *_pkt_inf;

extern const char *winpkt_trace_func;
extern const char *winpkt_trace_file;
extern UINT        winpkt_trace_line;
extern UINT        winpkt_trace_level;

#if defined(USE_DEBUG)
  /*
   * Trace to file specified by 'winpkt.dumpfile'.
   * I.e. above 'winpkt_trace_file'.
   */
  #define WINPKT_TRACE(args, ...)                \
          do {                                   \
            winpkt_trace_file = __FILE__;        \
            winpkt_trace_line = __LINE__;        \
            winpkt_trace (args, ## __VA_ARGS__); \
          } while (0)

  extern void winpkt_trace (const char *fmt, ...) W32_ATTR_PRINTF (1,2);
  extern void winpkt_trace_fclose (void);

#else
  #define WINPKT_TRACE(args, ...)  ((void)0)
#endif  /* USE_DEBUG */

#endif  /* _w32_WINPKT_H */
