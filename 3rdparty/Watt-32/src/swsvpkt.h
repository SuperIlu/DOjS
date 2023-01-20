/*!\file swsvpkt.h
 *
 * Only included on Win32
 */
#ifndef _w32_SWSVPKT_H
#define _w32_SWSVPKT_H

#include <winioctl.h>

/* Win32 accessible device. Adapters have 0..31 appended */
#define IOCTL_DEVICE "SwsVpkt"

/* Device type */
#define IOCTL_TYPE   0xA50C  /* Microsoft uses 0 - 0x7FFF, OEMs use 0x8000 - 0xFFFF */
#define IOCTL_FCMIN  0x900   /* Microsoft uses function codes 0-0x7FF, OEM's use 0x800 - 0xFFF */

#define IOCTL_GETMACADDR \
        CTL_CODE (IOCTL_TYPE, (IOCTL_FCMIN + 0), METHOD_BUFFERED, FILE_READ_ACCESS)
        /* lpvOutBuffer -> 6 BYTE MAC address */

#define IOCTL_SETMULTICASTLIST \
        CTL_CODE (IOCTL_TYPE, (IOCTL_FCMIN + 1), METHOD_BUFFERED, FILE_WRITE_ACCESS)
        /* lpvInBuffer -> Multicast list */

#define IOCTL_SETPROMISCUITY \
        CTL_CODE (IOCTL_TYPE, (IOCTL_FCMIN + 2), METHOD_BUFFERED, FILE_WRITE_ACCESS)
        /* lpvInBuffer -> BYTE, !0= enable promiscuous mode */

#define IOCTL_WRITE_SCATTER \
        CTL_CODE (IOCTL_TYPE, (IOCTL_FCMIN + 3), METHOD_OUT_DIRECT, FILE_WRITE_ACCESS)
        /* lpvInBuffer -> array of IOCTL_BUFFER's */

typedef struct IOCTL_BUFFER {
        void  *pv;
        DWORD  dwLen;
      } IOCTL_BUFFER;

#define IOCTL_GETINFO \
        CTL_CODE (IOCTL_TYPE, (IOCTL_FCMIN + 4), METHOD_BUFFERED, FILE_READ_ACCESS)
        /* lpvOutBuffer -> IOCTL_INFO */

typedef struct IOCTL_INFO {
        BOOL bPowerOn;
        BOOL bMediaDisconnected;
        BOOL bWan;               /* Adapter is a WAN */
        BOOL bWanDown;           /* WAN is not connected */
      } IOCTL_INFO;

#define IOCTL_GETMULTICASTLIST \
        CTL_CODE (IOCTL_TYPE, (IOCTL_FCMIN + 5), METHOD_BUFFERED, FILE_READ_ACCESS)
        /* lpvOutBuffer -> Multicast list */

#define IOCTL_GETDESC \
        CTL_CODE (IOCTL_TYPE, (IOCTL_FCMIN + 6), METHOD_BUFFERED, FILE_READ_ACCESS)
        /* lpvOutBuffer -> ASCIIZ string */

/* Get a ptr to the start of a struct given a ptr to a field within it
 */
#define SWS_CONTAINER(ptr, type, field) \
        (type*) ((char*)(ptr) - offsetof(type, field))


/* Receive buffer
 */
struct SRxBuffer {
       OVERLAPPED overlap;
#define OVL2RXBUF(p) SWS_CONTAINER (p, struct SRxBuffer, overlap)
       unsigned char buffer [1514];
     };

struct SwsVpktUsr;

/* Rx frame callback */
typedef void SwsVpktRxFn (
        struct SwsVpktUsr *,     /* IN: Handle */
        const void *,            /* IN: Frame data */
        unsigned);               /* IN:  Frame length */

/* Interface instance
 */
struct SwsVpktUsr {
       HANDLE           hDevice;      /* Packet protocol driver */
       OVERLAPPED       txOverlap;    /* For write requests */
       HANDLE           hWorker;      /* Worker thread handle */
       int              iWorkerExit;  /* Worker thread is exiting */
       SwsVpktRxFn     *pfnRx;        /* Rx frame callback */
       unsigned         uRxBufs;
       struct SRxBuffer rxBuffer[1];  /* VLA of Rx buffers */
     };

struct SwsVpktOpenInfo {
       SwsVpktRxFn *pfnRx;        /* Rx frame callback */
       unsigned     uRxBufs;      /* No. receive buffers */
     };

struct SwsVpktAdapterState {
       unsigned int isPowerOn        : 1;
       unsigned int isMediaConnected : 1;
       unsigned int isWan            : 1;
       unsigned int isWanDown        : 1;
     };

extern struct SwsVpktUsr *SwsVpktOpen (  /* retuns NULL on error */
       const char *,                     /* IN: Adapter name; \\.\SwsVpkt0..31 */
       const struct SwsVpktOpenInfo *);  /* IN: Open info */

extern BOOL SwsVpktClose (struct SwsVpktUsr *usr);

extern BOOL SwsVpktGetAdapterState (const struct SwsVpktUsr    *usr,
                                    struct SwsVpktAdapterState *state);

extern BOOL SwsVpktGetStatistics (const struct SwsVpktUsr *usr,
                                  struct PktStats         *stats);

extern UINT SwsVpktSend (struct SwsVpktUsr *usr,
                         const void        *buf,
                         UINT               buf_len);

extern BOOL SwsVpktGetMacAddress (const struct SwsVpktUsr *usr,
                                  mac_address             *addr);

extern BOOL SwsVpktGetDescription (const struct SwsVpktUsr *usr,
                                   char *descr, size_t descr_len);

extern BOOL SwsVpktGetNDISversion (const struct SwsVpktUsr *usr,
                                   DWORD *ver);

extern const char *SwsVpktGetDriverVersion (void);

extern BOOL SwsVpktDeviceRequest2 (HANDLE hDevice,
                                   DWORD dwIoControlCode,
                                   const void *lpInBuffer,
                                   DWORD nInBufferSize,
                                   void *lpOutBuffer,
                                   DWORD nOutBufferSize,
                                   DWORD *lpBytesReturned,
                                   const char *file,
                                   unsigned line);

#define SwsVpktDeviceRequest(dev,code,in_buf,in_size,out_buf,out_size,bytes_ret) \
        SwsVpktDeviceRequest2 (dev,code,in_buf,in_size,out_buf,out_size,bytes_ret, \
                               __FILE__, __LINE__)

#endif

