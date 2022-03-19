/*!\file pcqueue.h
 */
#ifndef _w32_PCQUEUE_H
#define _w32_PCQUEUE_H

#include <stddef.h>

#ifndef __SYS_WTIME_H
#include <sys/wtime.h>
#endif

#ifndef _w32_PCSED_H
#include "pcsed.h"
#endif

/**
 * Define number of receive buffers.
 */
#if defined(WIN32)
  /*
   * \todo Make RX_BUFS (i.e. size of thread queue) configurable.
   * Only the NPF kernel-buffer size is configurable now (pkt_num_rx_bufs).
   */
  #define RX_BUFS    50

#elif defined(USE_FAST_PKT)
  #define RX_BUFS    40    /* max # that will fit in 64 kB */

#elif (DOSX)
  #if (DOSX & DOS4GW)      /* allocation from DOS is limited to 64kB */
    #define RX_BUFS  30
  #else
    #define RX_BUFS  50    /* # of receive buffers to use in queue */
  #endif                   /* should be greater than MAX_FRAGMENT */

#else
  #define RX_BUFS    5
#endif

#if (DOSX)
  #define PKT_TMP_SIZE 64  /* scratch buffer only needed for 32-bit DOS */
#endif

#define PKTQ_MARKER  0xDEAFBABE

/*
 * asmpkt4.asm depends on these structs beeing packed
 */
#if (DOSX & DOS4GW) || defined(USE_FAST_PKT)
#include <sys/pack_on.h>
#endif

#if defined(WIN32)
  struct pkt_rx_element {
         uint64 tstamp_put;          /* timestamp from recv-thread */
         uint64 tstamp_get;          /* timestamp when polled */
         WORD   rx_length;           /* length from PacketReceivePacket() */
         BYTE   rx_buf [ETH_MAX+10]; /* add some margin */
       };
#else
  struct pkt_rx_element {
         DWORD  tstamp_put[2];       /* RDTSC timestamp on 1st upcall */
         DWORD  tstamp_get[2];       /* RDTSC timestamp when polled */
         WORD   handle;              /* Packet-driver handle of upcall */
         WORD   rx_length_1;         /* length on 1st upcall */
         WORD   rx_length_2;         /* length on 2nd upcall */
         WORD   filler;              /* 4 byte alignment */
         BYTE   rx_buf [ETH_MAX+10]; /* add some margin */
       };  /* = 1548 bytes */
#endif

/* DOS: 1524+24=1548
 */
#define RX_SIZE               sizeof(struct pkt_rx_element)
#define RX_ELEMENT_HEAD_SIZE  offsetof (struct pkt_rx_element, rx_buf[0])

/*!\struct pkt_ringbuf
 * FIFO style ring-buffer.
 */
struct pkt_ringbuf {
       volatile WORD  in_index;   /* index *ahead* of last buffer put */
       WORD           out_index;  /* index of buffer pulled out */
       WORD           buf_size;   /* size of each buffer */
       WORD           num_buf;    /* number of buffers */
       volatile DWORD num_drop;   /* number of dropped pkts */
       DWORD_PTR      buf_start;  /* start of buffer pool (linear addr) */
#if (DOSX & (DOS4GW|POWERPAK)) || defined(USE_FAST_PKT)
       WORD           dos_ofs;    /* offset of pool, used by rmode stub */
#endif                            /* total size = 26 for DOS4GW/POWERPAK */
     };

#if (DOSX & DOS4GW) || defined(USE_FAST_PKT)
#include <sys/pack_off.h>
#endif

extern int   pktq_inc_in  (struct pkt_ringbuf *q);
extern int   pktq_inc_out (struct pkt_ringbuf *q);
extern int   pktq_check   (struct pkt_ringbuf *q);
extern void  pktq_clear   (struct pkt_ringbuf *q);

extern int   pktq_init     (struct pkt_ringbuf *q, int size, int num, char *buf);
extern int   pktq_in_index (struct pkt_ringbuf *q);
extern char *pktq_in_buf   (struct pkt_ringbuf *q);
extern char *pktq_out_buf  (struct pkt_ringbuf *q);
extern int   pktq_queued   (struct pkt_ringbuf *q);

#if defined(USE_FAST_PKT) && defined(__MSDOS__)
  extern DWORD asmpkt_rm_base;

  /* macros to access runtime location of 'asmpkt_inf'
   */
  #define FAR_PEEK_WORD(struc,mbr)    PEEKW (0, asmpkt_rm_base + offsetof(struc,mbr))
  #define FAR_PEEK_DWORD(struc,mbr)   PEEKL (0, asmpkt_rm_base + offsetof(struc,mbr))

  #define FAR_POKE_WORD(struc,mbr,x)  POKEW (0, asmpkt_rm_base + offsetof(struc,mbr), x)
  #define FAR_POKE_DWORD(struc,mbr,x) POKEL (0, asmpkt_rm_base + offsetof(struc,mbr), x)

  extern int pktq_far_init   (int size, int num, DWORD asmpkt_real_base);
  extern int pktq_far_queued (void);
  extern int pktq_far_check  (struct pkt_ringbuf *q);
#endif

#endif
