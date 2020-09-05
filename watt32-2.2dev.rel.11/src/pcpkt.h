/*!\file pcpkt.h
 */
#ifndef _w32_PCPKT_H
#define _w32_PCPKT_H

#ifndef _w32_PCQUEUE_H
#include "pcqueue.h"
#endif

#ifndef _w32_PCSED_H
#include "pcsed.h"
#endif


#if defined(WIN32)
  #define PKTDRVR_STR "WinPcap"
#else
  #define PKTDRVR_STR "PKTDRVR"
#endif

/*
 * Common stuff for PKTDRVR and Windows interfaces
 */
#if defined(USE_DEBUG)
  #define ASSERT_PKT_INF(rc) do {                                   \
                               if (!_pkt_inf) {                     \
                                  (*_printf) ("%s (%u): "           \
                                              "_pkt_inf == NULL\n", \
                                              __FILE__, __LINE__);  \
                                  return (rc);                      \
                               }                                    \
                             } while (0)
#else
  #define ASSERT_PKT_INF(rc) do {              \
                               if (!_pkt_inf)  \
                                  return (rc); \
                             } while (0)
#endif

W32_DATA WORD _pktdevclass;       /**< Driver class; \ref <tcp.h> for values */
W32_DATA WORD _pkt_ip_ofs;        /**< Offset from MAC-header to IP-header */
W32_DATA BOOL _pktserial;         /**< Driver is a serial (SLIP/PPP) driver */
W32_DATA char _pktdrvrname[];     /**< Name of PKDRVR */
W32_DATA int  _pkt_rxmode;        /**< Current PKTDRVR/WinPcap Rx-mode */
W32_DATA int  _pkt_rxmode0;       /**< Startup receive mode */
W32_DATA int  _pkt_forced_rxmode; /**< Forced Rx-mode via WATTCP.CFG */
W32_DATA int  _pkt_errno;         /**< Last PKTDRVR error code */

/**\todo Make a structure and an access-function returning all
 *   internal data to the apps. _pktdrvrname[], _pktdrvr_descr[], _pktclass etc.
 *
 */

#if defined(WIN32)

#include "winpkt.h"

#else   /* rest of file */

/* Basic PD Functions.
 */
#define PD_DRIVER_INFO  0x01FF
#define PD_ACCESS       0x0200
#define PD_RELEASE      0x0300
#define PD_SEND         0x0400
#define PD_TERMINATE    0x0500
#define PD_GET_ADDRESS  0x0600
#define PD_RESET        0x0700

/* Extended PD Functions (1.09+)
 */
#define PD_GET_PARAM    0x0A00
#define PD_ASY_SEND_109 0x0B00    /**< old 1.09, not reliable */
#define PD_ASY_SEND     0x0C00    /**< 1.11 function */
#define PD_ASY_DROP     0x0D00    /**< 1.11 function */
#define PD_SET_RCV      0x1400
#define PD_GET_RCV      0x1500
#define PD_SET_MULTI    0x1600
#define PD_GET_MULTI    0x1700
#define PD_GET_STATS    0x1800
#define PD_SET_ADDR     0x1900
#define PD_RAW_SEND     0x1A00    /**< 1.11 function */
#define PD_RAW_FLUSH    0x1B00
#define PD_RAW_GET      0x1C00
#define PD_SIGNAL       0x1D00
#define PD_GET_STRUCT   0x1E00
#define PD_GET_VJSTATS  0x8100    /**< DOS-PPP function */

/* Packet-Driver Error Returns.
 */
#define PDERR_BAD_HANDLE      1
#define PDERR_NO_CLASS        2
#define PDERR_NO_TYPE         3
#define PDERR_NO_NUMBER       4
#define PDERR_BAD_TYPE        5
#define PDERR_NO_MULTICAST    6
#define PDERR_CANT_TERMINATE  7
#define PDERR_BAD_MODE        8
#define PDERR_NO_SPACE        9
#define PDERR_TYPE_INUSE     10
#define PDERR_BAD_COMMAND    11
#define PDERR_CANT_SEND      12
#define PDERR_CANT_SET       13
#define PDERR_BAD_ADDRESS    14
#define PDERR_CANT_RESET     15
#define PDERR_BASIC_DVR      16

enum ReceiveModes {
     RXMODE_OFF        = 1,    /**< turn off receiver              */
     RXMODE_DIRECT     = 2,    /**< receive only to this interface */
     RXMODE_BROADCAST  = 3,    /**< DIRECT + broadcast packets     */
     RXMODE_MULTICAST1 = 4,    /**< BROADCAST + limited multicast  */
     RXMODE_MULTICAST2 = 5,    /**< BROADCAST + all multicast      */
     RXMODE_PROMISCOUS = 6     /**< receive all packets on network */
   };

W32_DATA BYTE _pktdevlevel;      /**< Device level */

#if defined(USE_MULTICAST)
  extern int pkt_get_multicast_list (mac_address *listbuf, int *len);
  extern int pkt_set_multicast_list (const void *listbuf, int len);
#endif

#include <sys/pack_on.h>      /* cstate, slcompress etc. must be packed */

/*\struct cstate
 *
 * Statistics returned from DOS-PPP.
 *
 * "state" data for each active tcp conversation on the wire.  This is
 * basically a copy of the entire IP/TCP header from the last packet
 * we saw from the conversation together with a small identifier
 * the transmit & receive ends of the line use to locate saved header.
 */
struct cstate {
       BYTE       cs_this;       /* connection id number (xmit) */
       WORD       cs_next;       /* next in ring (xmit) */
       in_Header  cs_ip;         /* ip/tcp hdr from most recent packet */
       tcp_Header cs_tcp;
       BYTE       cs_ipopt[40];
       BYTE       cs_tcpopt[40];
     };

/*\struct slcompress
 *
 * Serial line compression statistics.
 */
struct slcompress {
       WORD  cstate_ptr_tstate;  /* transmit connection states (DOSPPP is small model) */
       WORD  cstate_ptr_rstate;  /* receive connection states */

       BYTE  tslot_limit;        /* highest transmit slot id (0-l) */
       BYTE  rslot_limit;        /* highest receive slot id (0-l) */

       BYTE  xmit_oldest;        /* oldest xmit in ring */
       BYTE  xmit_current;       /* most recent xmit id */
       BYTE  recv_current;       /* most recent rcvd id */
       BYTE  flags;

       DWORD sls_o_nontcp;       /* outbound non-TCP packets */
       DWORD sls_o_tcp;          /* outbound TCP packets */
       DWORD sls_o_uncompressed; /* outbound uncompressed packets */
       DWORD sls_o_compressed;   /* outbound compressed packets */
       DWORD sls_o_searches;     /* searches for connection state */
       DWORD sls_o_misses;       /* times couldn't find conn. state */

       DWORD sls_i_uncompressed; /* inbound uncompressed packets */
       DWORD sls_i_compressed;   /* inbound compressed packets */
       DWORD sls_i_error;        /* inbound error packets */
       DWORD sls_i_tossed;       /* inbound packets tossed because of error */

       DWORD sls_i_runt;         /* inbound packets too short */
       DWORD sls_i_badcheck;     /* inbound IP-packets with chksum error */
     };

extern int pkt_get_vjstats (struct slcompress *vjstats);
extern int pkt_get_cstate  (struct cstate *cs, WORD cstate_ofs);


/*\struct async_iocb
 *
 * I/O structure for asynchronous transmit function (v1.11, AH = 12)
 * (not used)
 */
struct async_iocb {
       DWORD buffer;          /* Pointer to xmit buffer */
       WORD  length;          /* Length of buffer */
       BYTE  flagbits;        /* Flag bits */
       BYTE  code;            /* Error code */
       DWORD xmit_func;       /* Transmitter upcall */
       char  reserved[4];     /* Future gather-write data */
       char  private[8];      /* Driver's private data */
     };

#define ASY_DONE   1  /* packet driver is done with this iocb */
#define ASY_UPCALL 2  /* requests an upcall when the buffer is re-usable */

/*\struct pkt_info
 *
 * Placeholder for vital data accessed on packet-driver upcall.
 *
 * Keep it gathered to simplify locking memory at `_pkt_inf'.
 * For DOS4GW targets; this structure \b MUST match same structure
 *                     in \b asmpkt4.asm.
 */
struct pkt_info {
       WORD   rm_sel, rm_seg, rm_ofs; /* real selector, segment, ofs */
       WORD   dos_ds;                 /* copy of _dos_ds */
       WORD   use_near_ptr;           /* near-pointers enabled */
       WORD   handle;                 /* Packet-driver handle */
       WORD   is_serial;              /* duplicated from _pktserial */
       WORD   pkt_ip_ofs;             /* store length of MAC-header */
       WORD   pkt_type_ofs;           /* offset to MAC-type */
       struct pkt_ringbuf pkt_queue;  /* Ring-struct for enqueueing */

#if !defined(USE_FAST_PKT)
       /* USE_FAST_PKT uses a single rx_buf in pcpkt2.c
        */
       struct pkt_rx_element rx_buf [RX_BUFS];
#endif
     };

extern struct pkt_info *_pkt_inf;

#include <sys/pack_off.h>       /* restore default packing */

/*
 * sizeof(*_pkt_inf), __LARGE__
 * 3*2         = 6
 * 16          = 16
 * 5*1524      = 7620
 *             = 7642
 *
 * sizeof(*_pkt_inf), DOS4GW
 * 3*2             = 6
 * 26              = 26
 * 20*(14+1500+10) = 30480
 *                 = 30512
 */

#if (DOSX & (DOS4GW|X32VM|DJGPP))
  extern void *pkt_tx_buf (void);
#endif

#if defined(USE_FAST_PKT)
  extern int  pkt_append_recv (const void *tx, unsigned len);
  extern int  pkt_buffers_used (void);
  extern int  pkt_test_upcall  (void);
  extern void pkt_dump_real_mem(void);

  extern struct pkt_rx_element *pkt_poll_recv (void);
#endif


#define ROUND_UP32(sz)  (4 * (((sz) + 3) / 4))

#if (DOSX & DJGPP)
  extern void __dj_movedata (void); /* called by __movedata() */
  extern void __movedata (unsigned src_sel, unsigned src_ofs,
                          unsigned dst_sel, unsigned dst_ofs,
                          size_t bytes);

  #define DOSMEMGET(ofs,len,buf)  __movedata (_pkt_inf->dos_ds,   \
                                              (unsigned)(ofs),    \
                                              (unsigned)_my_ds(), \
                                              (unsigned)(buf),    \
                                              len)
  #define DOSMEMGETL(ofs,d32,buf) _movedatal (_pkt_inf->dos_ds,   \
                                              (unsigned)(ofs),    \
                                              (unsigned)_my_ds(), \
                                              (unsigned)(buf),    \
                                              d32)
  #define DOSMEMPUT(buf,len,ofs)  __movedata ((unsigned)_my_ds(), \
                                              (unsigned)(buf),    \
                                              _pkt_inf->dos_ds,   \
                                              (unsigned)(ofs),    \
                                              len)
  #define DOSMEMPUTL(buf,d32,ofs) _movedatal ((unsigned)_my_ds(), \
                                              (unsigned)(buf),    \
                                              _pkt_inf->dos_ds,   \
                                              (unsigned)(ofs),    \
                                              d32)

  #define DOSMEMCLR(ofs,len)                      \
          do {                                    \
            int i = ROUND_UP32 (len) - 4;         \
            _farsetsel (_dos_ds); /* FS=dos DS */ \
            for ( ; i >= 0; i -= 4)               \
                _farnspokel (ofs+i, 0L);          \
          } while (0)

#endif  /* (DOSX & DJGPP) */
#endif  /* WIN32 */
#endif

