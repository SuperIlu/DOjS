/****************************************************************************
 *
 * File name   : vpkt_uio.c
 * Function    : SwsVpkt user mode I/O
 * Project     : SwsVpkt
 * Authors     : Lawrence Rust
 * Systems     : ANSI C Win32
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 15-Jan-06, LVR, Created.
 *
 * Aug 2010, GV, changed for Watt-32.
 *
 * Comments:
 * --------
 * This is an example of how to perform user mode network I/O.
 *
 ****************************************************************************
 */

#include "wattcp.h"

#if defined(WIN32)

#include "misc.h"
#include "pcpkt.h"
#include "winpkt.h"
#include "w32_ndis.h"
#include "swsvpkt.h"

#if defined(__LCC__)
  #define QueueUserAPC_4th_ARG  __int64 *
#else
  #define QueueUserAPC_4th_ARG  ULONG_PTR
#endif

static struct {
       uint64  rx_pkts;
       uint64  rx_bytes;
       uint64  rx_errors;
       uint64  tx_pkts;
       uint64  tx_bytes;
       uint64  tx_errors;
     } sws_stat;

static DWORD WINAPI WorkerThread (void *);
static void  WINAPI StartReceivingApc (ULONG_PTR);
static void  WINAPI ReceiveComplete (DWORD, DWORD, OVERLAPPED*);
static void  WINAPI ResubmitRxBufferApc (ULONG_PTR);
static BOOL         QueueBuffer (struct SwsVpktUsr*, struct SRxBuffer*);

/*
 * Open a handle to the packet driver
 */
struct SwsVpktUsr *SwsVpktOpen (const char *szName,
                                const struct SwsVpktOpenInfo *pInfo)
{
  struct SwsVpktUsr *pPacket;
  unsigned           uRxBufs;
  size_t             size;
  DWORD              err;

  winpkt_trace_func = "SwsVpktOpen";
  WINPKT_TRACE ("name = %s\n", szName);

  if (!pInfo || !pInfo->pfnRx)
     return (NULL);

  uRxBufs = pInfo->uRxBufs > 0 ? pInfo->uRxBufs : 16;

  /* Allocate instance data */
  size = sizeof(*pPacket) + uRxBufs * sizeof(pPacket->rxBuffer[0]);
  pPacket = HeapAlloc (GetProcessHeap(), HEAP_ZERO_MEMORY, size);
  if (!pPacket)
  {
    err = GetLastError();
    WINPKT_TRACE ("HeapAlloc() failed; %s\n", win_strerror(err));
    SetLastError (err);
    return (NULL);
  }

  pPacket->hDevice = INVALID_HANDLE_VALUE;
  pPacket->uRxBufs = uRxBufs;
  pPacket->pfnRx   = pInfo->pfnRx;

  /* Create overlapped Tx done event
   */
  pPacket->txOverlap.hEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
  if (!pPacket->txOverlap.hEvent)
  {
    err = GetLastError();
    SwsVpktClose (pPacket);
    SetLastError (err);
    WINPKT_TRACE ("CreateEvent() failed; %s\n", win_strerror(err));
    return (NULL);
  }

  /* Attach to packet driver
   */
  pPacket->hDevice = CreateFileA (szName, GENERIC_READ | GENERIC_WRITE,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,   /* Security */
                                  OPEN_EXISTING,
                                  FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL,
                                  NULL);  /* Template file */
  if (pPacket->hDevice == INVALID_HANDLE_VALUE)
  {
    err = GetLastError();
    SwsVpktClose (pPacket);
    SetLastError (err);
    WINPKT_TRACE ("CreateFileA() failed; %s\n", win_strerror(err));
    return (NULL);
  }

  /* Start the worker thread */
  pPacket->hWorker = CreateThread (NULL, 0, WorkerThread, pPacket,
                                   CREATE_SUSPENDED, NULL);
  if (!pPacket->hWorker)
  {
    err = GetLastError();
    SwsVpktClose (pPacket);
    SetLastError (err);
    WINPKT_TRACE ("CreateThread() failed; %s\n", win_strerror(err));
    return (NULL);
  }

  SetThreadPriority (pPacket->hWorker, THREAD_PRIORITY_ABOVE_NORMAL);
  ResumeThread (pPacket->hWorker);
  QueueUserAPC (StartReceivingApc, pPacket->hWorker,
                (QueueUserAPC_4th_ARG)pPacket);
  return (pPacket);
}

/*
 * Release packet driver resources
 */
BOOL SwsVpktClose (struct SwsVpktUsr *pPacket)
{
  char res[100] = { '\0' };
  BOOL rc = FALSE;

  winpkt_trace_func = "SwsVpktClose";

  /* Signal the worker thread to exit */
  pPacket->iWorkerExit = 1;
  if (pPacket->hWorker)
  {
    if (WaitForSingleObject(pPacket->hWorker, 300) == WAIT_TIMEOUT)
    {
      /* This will occur if the thread is waiting in a critical section
       * Resolve the deadlock by killing the thread
       */
      TerminateThread (pPacket->hWorker, 1);
      strcpy (res, "thread killed, ");
    }
    CloseHandle (pPacket->hWorker);
  }

  if (pPacket->hDevice != INVALID_HANDLE_VALUE)
  {
    CloseHandle (pPacket->hDevice);
    pPacket->hDevice = INVALID_HANDLE_VALUE;
    strcat (res, "device closed, ");
    rc = TRUE;
  }

  if (pPacket->txOverlap.hEvent)
  {
    CloseHandle (pPacket->txOverlap.hEvent);
    strcat (res, "overlap event closed");
  }

  WINPKT_TRACE ("rc: %d, res: %s\n", rc, res);

  HeapFree (GetProcessHeap(), 0, pPacket);
  return (rc);
}

/*
 * Send a frame
 */
UINT SwsVpktSend (struct SwsVpktUsr *pPacket,
                  const void *pvBuf, UINT uLen)
{
  DWORD sent = 0;
  BOOL  bRet, overlapped = FALSE;

  pPacket->txOverlap.Offset = 0;
  pPacket->txOverlap.OffsetHigh = 0;

  bRet = WriteFile (pPacket->hDevice, pvBuf, uLen,
                    &sent, &pPacket->txOverlap);

  if (!bRet && GetLastError() == ERROR_IO_PENDING)
  {
    /* Wait for completion */
    sent = 0;
    GetOverlappedResult (pPacket->hDevice, &pPacket->txOverlap,
                         &sent, TRUE);
    overlapped = TRUE;
  }
  if (sent)
  {
    sws_stat.tx_pkts++;
    sws_stat.tx_bytes += sent;
  }
  else
    sws_stat.tx_errors++;

  winpkt_trace_func = "SwsVpktSend";
  WINPKT_TRACE ("overlapped: %d, bRet %d; %s\n",
                overlapped, bRet, sent == 0 ? win_strerror(GetLastError()) : "okay");
  return (UINT)sent;
}


BOOL SwsVpktGetStatistics (const struct SwsVpktUsr *usr, struct PktStats *stats)
{
  stats->in_packets  = sws_stat.rx_pkts;
  stats->in_bytes    = sws_stat.rx_bytes;
  stats->out_packets = sws_stat.tx_pkts;
  stats->out_bytes   = sws_stat.tx_bytes;
  stats->in_errors   = sws_stat.rx_errors;
  stats->out_errors  = sws_stat.tx_errors;
  ARGSUSED (usr);
  return (TRUE);
}

BOOL SwsVpktGetAdapterState (const struct SwsVpktUsr *usr,
                             struct SwsVpktAdapterState *state)
{
  IOCTL_INFO info;
  BOOL  rc;

  memset (&info, '\0', sizeof(info));

  /* Get adapter info */
  rc = SwsVpktDeviceRequest(
          usr->hDevice,               /* Device handle */
          (DWORD)IOCTL_GETINFO,       /* IOCTL code */
          NULL, 0,                    /* -> input buffer & size */
          &info,                      /* -> output buffer */
          sizeof(info),               /* input buffer size */
          NULL);                      /* Bytes returned */
  if (rc)
  {
    state->isPowerOn        =  info.bPowerOn;
    state->isMediaConnected = !info.bMediaDisconnected;
    state->isWan            =  info.bWan;
    state->isWanDown        =  info.bWanDown;
  }
  winpkt_trace_func = "SwsVpktGetAdapterState";
  WINPKT_TRACE ("rc %d, state: %d,%d,%d,%d\n",
                rc, state->isPowerOn, state->isMediaConnected,
                state->isWan, state->isWanDown);
  return (rc);
}

/*
 * Get the adapter's MAC address
 */
BOOL SwsVpktGetMacAddress (const struct SwsVpktUsr *pPacket, mac_address *pMac)
{
  /* Get the device type etc */
  return SwsVpktDeviceRequest (
              pPacket->hDevice,             /* Device handle */
              (DWORD)IOCTL_GETMACADDR,      /* IOCTL code */
              NULL, 0,                      /* -> input buffer & size */
              pMac,                         /* -> output buffer */
              sizeof(*pMac),                /* output buffer size */
              NULL);                        /* Bytes returned */
}

/*
 * Get the adapter's description.
 * Note: This IOCTL was added in ver 1.0.0.5
 */
BOOL SwsVpktGetDescription (const struct SwsVpktUsr *pPacket, char *descr, size_t max)
{
  return SwsVpktDeviceRequest (
           pPacket->hDevice,             /* Device handle */
           (DWORD)IOCTL_GETDESC,         /* IOCTL code */
           NULL, 0,                      /* -> input buffer & size */
           descr,                        /* -> output buffer */
           max,                          /* output buffer size */
           NULL);                        /* Bytes returned */

}

/*
 * Get the NDIS version.
 */
BOOL SwsVpktGetNDISversion (const struct SwsVpktUsr *pPacket, DWORD *ver)
{
  return SwsVpktDeviceRequest (
           pPacket->hDevice,             /* Device handle */
           OID_GEN_DRIVER_VERSION,       /* IOCTL code */
           NULL, 0,                      /* -> input buffer & size */
           ver,                          /* -> output buffer */
           sizeof(*ver),                 /* output buffer size */
           NULL);                        /* Bytes returned */
}

/*
 * Return swsVpkt.sys file-version.
 */
static char swsVpkt_ver[64] = "?";

const char *SwsVpktGetDriverVersion (void)
{
  if (swsVpkt_ver[0] == '?')
     get_file_version ("drivers\\swsVpkt.sys", swsVpkt_ver, sizeof(swsVpkt_ver));
  return (swsVpkt_ver);
}

/*
 * Worker thread
 */
static DWORD WINAPI WorkerThread (void *pv)
{
  struct SwsVpktUsr *pPacket = (struct SwsVpktUsr*)pv;

  /* Execute IO completion callbacks and APC's until exit */
  while (!pPacket->iWorkerExit)
        SleepEx (100, TRUE);

  /* Cancel all pending reads */
  CancelIo (pPacket->hDevice);
  SleepEx (10, TRUE);
  return (0);
}

/*
 * APC to start receiving
 */
static void WINAPI StartReceivingApc (ULONG_PTR ulContext)
{
  struct SwsVpktUsr *pPacket = (struct SwsVpktUsr*)ulContext;
  UINT   i;

  /* Submit all read buffers */
  for (i = 0; i < pPacket->uRxBufs; i++)
      QueueBuffer (pPacket, &pPacket->rxBuffer[i]);
}

/*
 * Post a receive buffer
 */
static BOOL QueueBuffer (struct SwsVpktUsr *pPacket, struct SRxBuffer *pBuffer)
{
  if (pPacket->iWorkerExit)
     return (FALSE);

  memset (&pBuffer->overlap, 0, sizeof(pBuffer->overlap));
  pBuffer->overlap.hEvent = (HANDLE)pPacket;

  if (!ReadFileEx(pPacket->hDevice, pBuffer->buffer, sizeof(pBuffer->buffer),
                  &pBuffer->overlap, ReceiveComplete))
  {
    DWORD dwErr = GetLastError();

    switch (dwErr)
    {
      case ERROR_OPERATION_ABORTED:
           break;
      default:
           /* Retry later */
           QueueUserAPC (ResubmitRxBufferApc, pPacket->hWorker, (QueueUserAPC_4th_ARG)pBuffer);
           break;
    }
    return (FALSE);
  }
  return (TRUE);
}

/*
 * APC to resubmit an RxBuffer
 */
static void WINAPI ResubmitRxBufferApc (ULONG_PTR ulContext)
{
  struct SRxBuffer  *pBuffer = (struct SRxBuffer*) ulContext;
  struct SwsVpktUsr *pPacket = (struct SwsVpktUsr*) pBuffer->overlap.hEvent;

  /* Resubmit the buffer */
  QueueBuffer (pPacket, pBuffer);
}

/*
 * ReadFileEx completion routine
 */
static void WINAPI ReceiveComplete (
       DWORD       dwErr,                  /* completion code */
       DWORD       dwBytes,                /* number of bytes transferred */
       OVERLAPPED *lpOverlapped)           /* I/O information buffer */
{
  struct SRxBuffer  *pBuffer = OVL2RXBUF (lpOverlapped);
  struct SwsVpktUsr *pPacket = (struct SwsVpktUsr*) pBuffer->overlap.hEvent;

  winpkt_trace_func = "ReceiveComplete";
  WINPKT_TRACE ("dwErr %lu, got %lu bytes\n", (u_long)dwErr, (u_long)dwBytes);

  switch (dwErr)
  {
    case 0:
         (*pPacket->pfnRx) (pPacket, pBuffer->buffer, dwBytes);
         sws_stat.rx_pkts++;
         sws_stat.rx_bytes += dwBytes;
         break;

    case ERROR_OPERATION_ABORTED:
         sws_stat.rx_errors++;
         break;
  }

  /* Resubmit the buffer */
  QueueBuffer (pPacket, pBuffer);
}

static const struct search_list ccode_list[] = {
                  { IOCTL_GETINFO,          "IOCTL_GETINFO" },
                  { IOCTL_GETMACADDR,       "IOCTL_GETMACADDR" },
                  { IOCTL_GETDESC,          "IOCTL_GETDESC" },
                  { OID_GEN_DRIVER_VERSION, "OID_GEN_DRIVER_VERSION" }
                };

/*
 * Make a synchronous DeviceIoControl call to a handle opened
 * using FILE_FLAG_OVERLAPPED
 */
BOOL SwsVpktDeviceRequest2 (HANDLE hDevice, DWORD dwIoControlCode,
                           const void *lpInBuffer, DWORD nInBufferSize,
                           void *lpOutBuffer, DWORD nOutBufferSize,
                           DWORD *lpBytesReturned,
                           const char *file, unsigned line)
{
  BOOL       bResult;
  HANDLE     hEvent;
  OVERLAPPED overlap;
  DWORD      bytesReturned, err;

#if defined(USE_DEBUG)
  winpkt_trace_func = "SwsVpktDeviceRequest";
  winpkt_trace_file = file;
  winpkt_trace_line = line;
#endif

  /* Create a manual reset event for overlapped wait */
  hEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
  if (!hEvent)
  {
    err = GetLastError();
#if defined(USE_DEBUG)
    winpkt_trace ("CreateEvent failed; %s\n", win_strerror(err));
#endif
    SetLastError (err);
    return (FALSE);
  }

  memset (&overlap, 0, sizeof(overlap));
  overlap.hEvent = hEvent;

  if (!lpBytesReturned)
     lpBytesReturned = &bytesReturned;

  bResult = DeviceIoControl (hDevice, dwIoControlCode,
                             (void*)lpInBuffer,
                             nInBufferSize, lpOutBuffer,
                             nOutBufferSize, lpBytesReturned,
                             &overlap);

  if (!bResult && GetLastError() == ERROR_IO_PENDING)
  {
    /* Wait for completion */
    bResult = GetOverlappedResult (hDevice, &overlap, lpBytesReturned, TRUE);
  }

  err = GetLastError();

#if defined(USE_DEBUG)
  winpkt_trace ("%s, len %lu/%lu, bResult %d\n",
                list_lookup(dwIoControlCode,ccode_list,DIM(ccode_list)),
                (u_long)nInBufferSize, (u_long)*lpBytesReturned, bResult);
#endif

  CloseHandle (hEvent);
  SetLastError (err);
  return (bResult);
}
#endif  /* WIN32 */
