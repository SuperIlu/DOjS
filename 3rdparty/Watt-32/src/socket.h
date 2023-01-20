/*!\file src/socket.h
 */

/*
 *  BSD sockets functionality for Waterloo TCP/IP.
 *
 *  by G. Vanem 1997
 */

#ifndef _w32_SOCKET_H
#define _w32_SOCKET_H

#if defined(__TURBOC__) && (__TURBOC__ <= 0x301)
  /*
   * Prevent tcc <= 2.01 from even looking at this.
   */
  #define BOOL int
#else  /* rest of file */

#if defined(__CYGWIN__)
  #include "wattcp.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>
#include <fcntl.h>
#include <io.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/syslog.h>

#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_arp.h>
#include <net/if_ether.h>
#include <net/if_packe.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/in_pcb.h>
#include <netinet/in_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>
#include <netinet/ip6.h>
#include <netinet/tcp_time.h>

#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <arpa/tftp.h>
#include <netdb.h>
#include <resolv.h>

#include "wattcp.h"
#include "chksum.h"
#include "wdpmi.h"
#include "misc.h"
#include "misc_str.h"
#include "run.h"
#include "timer.h"
#include "sock_ini.h"
#include "language.h"
#include "pcconfig.h"
#include "pcqueue.h"
#include "pcdbug.h"
#include "pcsed.h"
#include "pcpkt.h"
#include "pcstat.h"
#include "pcigmp.h"
#include "pctcp.h"
#include "pcbuf.h"
#include "pcicmp.h"
#include "pcarp.h"
#include "pcrecv.h"
#include "printk.h"
#include "netaddr.h"
#include "ip4_in.h"
#include "ip4_out.h"
#include "ip6_in.h"
#include "ip6_out.h"
#include "bsddbug.h"
#include "bsdname.h"
#include "gettod.h"

#if defined(__DJGPP__)
  #include <sys/resource.h>
  #include <sys/fsext.h>
  #include <unistd.h>
  #if (DJGPP_MINOR >= 4)
  #include <libc/fd_props.h>
  #endif

  /* in fsext.c */
  extern int _fsext_demux (__FSEXT_Fnumber func, int *rv, va_list _args);

#elif defined(__CYGWIN__)
  #include <cygwin/version.h>

  extern long _get_osfhandle (int);   /* in cygwin1.dll (no prototype) */
#endif

/*
 * Various sizes
 */
#if defined (TARGET_IS_32BIT)
  #define MAX_DGRAMS        5              /* # of datagrams for broadcast */
  #define MAX_RAW_BUFS      5              /* # of _raw_Socket in list */
  #define MAX_RAW6_BUFS     5              /* # of _raw6_Socket in list */
  #define MAX_PACKET_BUFS   10
  #define MAX_SOCKETS       5000           /* # of sockets to handle */
  #define MAX_TCP_RECV_BUF  (1024*1024-1)  /* Max size for SO_RCVBUF */
#else
  #define MAX_DGRAMS        2
  #define MAX_RAW_BUFS      2
  #define MAX_RAW6_BUFS     2
  #define MAX_PACKET_BUFS   5
  #define MAX_SOCKETS       512
  #define MAX_TCP_RECV_BUF  (USHRT_MAX-1)
#endif

#define MAX_TCP_SEND_BUF    (USHRT_MAX-1)   /* for SO_SNDLOWAT/SO_RCVLOWAT */
#define MAX_UDP_RECV_BUF    (USHRT_MAX-1)
#define MAX_UDP_SEND_BUF    (USHRT_MAX-1)
#define MAX_RAW_RECV_BUF    (USHRT_MAX-1)
#define MAX_RAW_SEND_BUF    (USHRT_MAX-1)

#define DEFAULT_SEND_LOWAT  0
#define DEFAULT_RECV_LOWAT  0
#define DEFAULT_UDP_SIZE   (8*1024)


/* Buffer types for AF_PACKET sockets
 */
struct sock_packet_buf {
       WORD rx_len;
       char rx_buf [ETH_MAX+4];   /* add room for end marker */
     };

typedef struct sock_packet_pool {
        struct pkt_ringbuf     queue;
        struct sock_packet_buf buf [MAX_PACKET_BUFS];
      } sock_packet_pool;

typedef struct Socket {
        int                 fd;
        struct Socket      *next;
        struct sockaddr_in *local_addr;
        struct sockaddr_in *remote_addr;

        int                 timeout;
        int                 inp_flags;   /* misc. IP-protocol options */
        BYTE                ip_tos;
        BYTE                ip_ttl;

        struct ip_opts     *ip_opt;      /* for setsockopt(s,IP_OPTION,..) */
        unsigned            ip_opt_len;  /* output IP-option length */

        int                 so_options;  /* SO_KEEPALIVE, SO_ACCEPTCONN etc. */
        int                 so_proto;    /* IPPROTO_UDP, IPPROTO_TCP etc. */
        int                 so_type;     /* SOCK_STREAM, SOCK_DGRAM etc. */
        int                 so_family;   /* address family AF_INET/AF_INET6 */
        int                 so_state;    /* internal state, ref SS_* below */
        int                 so_error;    /* internal socket errno */
        time_t              close_time;  /* we closed at (SOCK_STREAM) */
        BOOL                linger_on;   /* user specified lingering */
        unsigned            linger_time; /* linger-time (SOCK_STREAM) */
        unsigned            fd_duped;    /* FSEXT reference counting */
        DWORD               keepalive;   /* keepalive timeout */
        DWORD               nb_timer;    /* non-block timer */
        _udp_Socket        *udp_sock;    /* actual state and Rx/Tx data is in */
        _tcp_Socket        *tcp_sock;    /*  one of these pointers */
        _raw_Socket        *raw_sock;    /* points to 1st in linked-list */
        _raw6_Socket       *raw6_sock;
        recv_buf          **bcast_pool;  /* buffers for INADDR_ANY sockets */
        unsigned            pool_size;   /* size of above buffer */

        /* For SOCK_PACKET sockets.
         */
        sock_packet_pool   *packet_pool;
        int      (W32_CALL *old_eth_peek) (void *pkt);

        /* listen-queue for incoming tcp connections
         */
        int                 backlog;
        _tcp_Socket        *listen_queue [SOMAXCONN];
        DWORD               syn_timestamp[SOMAXCONN]; /* got SYN at [msec] */

        unsigned            send_lowat;  /* low-water Tx marks */
        unsigned            recv_lowat;  /* low-water Rx marks */
        BOOL                msg_nosig;   /* don't raise SIGPIPE */
        DWORD               cookie;      /* memory cookie / marker */

      } Socket;


/*
 * Let first socket start at 3 in order not to
 * confuse sockets with stdin/stdout/stderr handles.
 * First socket will always be >= 3 on djgpp thanks to FS-extensions.
 */
#define SK_FIRST  3

/*
 * Number of 'fd_set' required to hold MAX_SOCKETS.
 */
#define NUM_SOCK_FDSETS  ((MAX_SOCKETS+sizeof(fd_set)-1) / sizeof(fd_set))

/*
 * Misc. defines
 */
#ifndef SIGIO       /* for asynchronous I/O support (not yet) */
#define SIGIO       SIGUSR1
#endif

#ifndef IPPORT_ANY
#define IPPORT_ANY  0
#endif

/*
 * Socket state bits (in so_state).
 */
#define SS_NOFDREF         0x0001          /* no file table ref any more */
#define SS_UNCONNECTED     SS_NOFDREF      /* or just created socket */
#define SS_ISCONNECTED     0x0002          /* socket connected to a peer */
#define SS_ISCONNECTING    0x0004          /* in process of connecting */
#define SS_ISDISCONNECTING 0x0008          /* in process of disconnecting */
#define SS_CANTSENDMORE    0x0010          /* can't send more data */
#define SS_CANTRCVMORE     0x0020          /* can't receive more data */
#define SS_RCVATMARK       0x0040          /* at mark on input (no used) */

#define SS_PRIV            0x0080          /* privileged for broadcast */
#define SS_NBIO            0x0100          /* non-blocking operations */
#define SS_ASYNC           0x0200          /* async I/O notify (not used) */
#define SS_ISCONFIRMING    0x0400          /* accepting connection req */
#define SS_ISLISTENING     SS_ISCONFIRMING /* non standard */

#define SS_LOCAL_ADDR      0x0800          /* has local address/port (not used) */
#define SS_REMOTE_ADDR     0x1000          /* has remote address/port (not used) */
#define SS_CONN_REFUSED    0x2000          /* connection refused (ICMP_UNREACH etc) */


/*
 * Socket macros
 */
#define SOCK_CALLOC(sz)  _sock_calloc (sz, __FILE__, __LINE__)
#define SOCK_DEL_FD(fd)  _sock_del_fd (fd, __FILE__, __LINE__)

/*
 * Prologue code starting most BSD-socket functions
 */
#define SOCK_PROLOGUE(socket, fmt, fd)           \
        do {                                     \
          SOCK_DEBUGF ((fmt, fd));               \
          if (!socket) {                         \
             if (_sock_dos_fd(fd)) {             \
                SOCK_DEBUGF ((", ENOTSOCK"));    \
                SOCK_ERRNO (ENOTSOCK);           \
                return (-1);                     \
             }                                   \
             SOCK_DEBUGF ((", EBADF"));          \
             SOCK_ERRNO (EBADF);                 \
             return (-1);                        \
          }                                      \
        } while (0)

#if defined(TARGET_IS_32BIT)
  #define VERIFY_RW(ptr,len)                    \
          do {                                  \
            if (!valid_addr(ptr,len)) {         \
               SOCK_DEBUGF ((", EFAULT "        \
                 "(buf %" ADDR_FMT ", len %d)", \
                 ADDR_CAST(ptr), (int)(len)));  \
               SOCK_ERRNO (EFAULT);             \
               return (-1);                     \
            }                                   \
          } while (0)
#else
  #define VERIFY_RW(ptr,len) ((void)0)
#endif

/*
 * Print error and exit at critical places. Should not be used
 * in "production quality" code.
 */
#if defined(USE_BSD_FATAL)
  #define SOCK_FATAL(arg)  do {                        \
                             (*_printf) arg;           \
                             fflush (stdout);          \
                          /* _watt_fatal_error = 1; */ \
                             exit (-1);                \
                           } while (0)
#else
  #define SOCK_FATAL(arg)  ((void)0)
#endif

/*
 * Only djgpp/DMC have (some of) the <fcntl.h> F_?? commands.
 * For others, we define some dummy values.
 */
#define W32_FCNTL_BASE 100

#ifndef F_DUPFD
#define F_DUPFD   (W32_FCNTL_BASE+0)
#endif

#ifndef F_GETFL
#define F_GETFL   (W32_FCNTL_BASE+1)
#endif

#ifndef F_SETFL
#define F_SETFL   (W32_FCNTL_BASE+2)
#endif

#ifndef F_GETFD
#define F_GETFD   (W32_FCNTL_BASE+3)
#endif

#ifndef F_SETFD
#define F_SETFD   (W32_FCNTL_BASE+4)
#endif

#ifndef F_GETLK
#define F_GETLK   (W32_FCNTL_BASE+5)
#endif

#ifndef F_SETLK
#define F_SETLK   (W32_FCNTL_BASE+6)
#endif

#ifndef F_SETLKW
#define F_SETLKW  (W32_FCNTL_BASE+7)
#endif

#ifndef F_GETOWN
#define F_GETOWN  (W32_FCNTL_BASE+8)
#endif

#ifndef F_SETOWN
#define F_SETOWN  (W32_FCNTL_BASE+9)
#endif

/*
 * Setup trapping of signals and critical sections around loops.
 */
extern int  _sock_sig_setup   (void);
extern int  _sock_sig_restore (void);
extern int  _sock_sig_pending (void);
extern int  _sock_sig_epipe   (const Socket *s);

extern void _sock_crit_start  (void);
extern void _sock_crit_stop   (void);

/*
 * Timing of "kernel" times for some socket calls.
 */
extern void _sock_start_timer (void);
extern void _sock_stop_timer  (void);

/*
 * Things for SOCK_PACKET sockets.
 */
extern unsigned sock_packet_transmit (Socket *sock, const void *buf,
                                      unsigned len, const struct sockaddr *to,
                                      int tolen);

extern unsigned sock_packet_receive (Socket *sock, void *buf, unsigned len,
                                     struct sockaddr *from, size_t *fromlen);

extern unsigned sock_packet_rbused (Socket *sock);

/*
 * Allocation, `sk_list' stuff etc.
 */
extern void   *_sock_calloc    (size_t size, const char *file, unsigned line);
extern Socket *_sock_del_fd    (int sock, const char *file, unsigned line);
extern Socket *_socklist_find  (int s);
extern BOOL    _sock_dos_fd    (int s);
extern int     _sock_half_open (const _tcp_Socket *tcp);
extern int     _sock_append    (_tcp_Socket **tcp);
extern int     _sock_set_rcv_buf  (sock_type *s, size_t len);
extern void    _sock_free_rcv_buf (sock_type *s);
extern void    _sock_set_syn_hook (int (*func)(_tcp_Socket **));
extern BOOL    _sock_set_promisc_rx_mode (void);
extern BOOL    _sock_set_mcast_rx_mode   (void);
extern BOOL    _sock_set_normal_rx_mode  (const Socket *_this);

/*
 * Interface naming stuff.
 */
extern const sock_type *__get_sock_from_s (int s, int proto);
extern void             __get_ifname (char *if_name);
extern void             __set_ifname (const char *if_name);
extern int              __scope_ascii_to_id (const char *str);
extern int              __scope_id_to_ascii (int scope);


/*
 * Check `sockaddr*' passed to bind()/connect().
 */
extern int _sock_chk_sockaddr (Socket *socket, const struct sockaddr *sa, int len);

/*
 * Handle UDP/TCP connect()/listen() calls.
 */
extern int _TCP_open   (Socket *socket, struct in_addr host, WORD loc_port, WORD rem_port);
extern int _UDP_open   (Socket *socket, struct in_addr host, WORD loc_port, WORD rem_port);
extern int _TCP_listen (Socket *socket, struct in_addr host, WORD loc_port);
extern int _UDP_listen (Socket *socket, struct in_addr host, WORD port);

extern int _TCP6_open  (Socket *socket, const void *host, WORD loc_port, WORD rem_port);
extern int _UDP6_open  (Socket *socket, const void *host, WORD loc_port, WORD rem_port);
extern int _TCP6_listen(Socket *socket, const void *host, WORD loc_port);
extern int _UDP6_listen(Socket *socket, const void *host, WORD port);

#endif  /* old __TURBOC__ */
#endif  /* _w32_SOCKET_H */

