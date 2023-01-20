/*!\file pcpkt.c
 *
 *  Packet Driver interface for WatTCP/Watt-32.
 *
 *  Heavily modified and extended for DOSX by
 *  G.Vanem <gvanem@yahoo.no>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>

#include "copyrigh.h"
#include "wattcp.h"

/* All this is for MS-DOS only. For Win32, refer winpkt.c instead.
 */
#if defined(__MSDOS__)

#if defined(__DJGPP__)
  #include <sys/exceptn.h>
  #include <sys/nearptr.h>
  #include <crt0.h>
#endif

#include "wdpmi.h"
#include "x32vm.h"
#include "powerpak.h"
#include "asmpkt.h"
#include "language.h"
#include "sock_ini.h"
#include "cpumodel.h"
#include "misc.h"
#include "misc_str.h"
#include "timer.h"
#include "profile.h"
#include "pcsed.h"
#include "pcstat.h"
#include "pcconfig.h"
#include "pcdbug.h"
#include "pcigmp.h"
#include "pcqueue.h"
#include "pcpkt.h"
#include "pcpkt32.h"

WORD _pktdevclass  = PDCLASS_UNKNOWN;  /**< Ethernet, Token, FDDI etc.       */
WORD _pkt_ip_ofs   = 0;                /**< ofs from link-layer head to ip   */
WORD _pkt_type_ofs = 0;                /**< ofs from link-layer head to type */
BOOL _pktserial    = FALSE;            /**< using serial driver, SLIP/PPP    */
BYTE _pktdevlevel  = 1;                /**< basic unless otherwise specified */
int  _pkt_rxmode   = RXMODE_BROADCAST; /**< active receive mode              */
int  _pkt_rxmode0  = -1;               /**< startup receive mode             */
int  _pkt_errno    = 0;                /**< error code set in pcpkt.c API    */
const char *pkt_error = NULL;          /**< Last pkt error string            */

int  _pkt_forced_rxmode = -1;
char _pktdrvrname[20]   = "unknown";

struct pkt_info *_pkt_inf = NULL;    /**< module data that will be locked */

static char  pkt_sign[]      = "PKT DRVR";
static WORD  pkt_interrupt   = 0;
static DWORD pkt_drop_cnt    = 0;
static BYTE  pkt_txretries   = 2;
static int   pkt_txwait      = 0;       /* # msec to wait if Tx fails */
static BOOL  pkt_use_near    = FALSE;   /* Use near-ptr if enabled */
static BOOL  pkt_do_reset    = FALSE;   /* reset handle at exit */
static int   pkt_drvr_ver    = 0;       /* Driver internal version */
static int   pkt_num_rx_bufs = RX_BUFS;

#if defined(USE_STATISTICS)
  static struct PktStats init_stats;
  static void get_init_stats (void);
#endif

static struct PktParameters pkt_params;
static BOOL   got_params   = FALSE;

int pkt_release_handle (WORD handle);
int pkt_reset_handle (WORD handle);

/**
 * First and last vector to search for a driver.
 * The 1.11 spec allows drivers to be in the 0x20-0xFF range.
 */
#define PKT_FIRST_VEC    0x60
#define PKT_LAST_VEC     0x80
#define PKT_FIRST_VEC_11 0x20
#define PKT_LAST_VEC_11  0xFF


#if (DOSX & PHARLAP)
  #include <mw/exc.h>

  REALPTR rm_base;

  /*
   * real-data is organised as follows: PKT_TMP at rm_base + 0x40
   *                                    TX_BUF  at PKT_TMP + PKT_TMP_SIZE
   *                                    RX_BUF  at TX_BUF + RX_SIZE
   */
  #define PKT_TMP()     (WORD) 0x40
  #define TX_BUF()      (PKT_TMP() + PKT_TMP_SIZE)  /* DWORD boundary */
  #define RX_BUF()      (TX_BUF() + RX_SIZE)
  #define RDATA_SIZE    (RX_BUF() + RX_SIZE)
  #define RP_SEGM()     RP_SEG (rm_base)

#elif (DOSX & X32VM)    /* X32VM is very similar to Pharlap except that it */
  REALPTR rm_base;      /* automatically maps DOS memory into our DS */

  #define PKT_TMP()     (WORD) 0x40
  #define TX_BUF()      (PKT_TMP() + PKT_TMP_SIZE)
  #define RX_BUF()      (TX_BUF() + RX_SIZE)
  #define RDATA_SIZE    (RX_BUF() + RX_SIZE)
  #define RP_SEGM()     RP_SEG (rm_base)

#elif (DOSX & DJGPP)
  #if !defined(USE_FAST_PKT)
    #define RMCB_STK_SIZE 2048
    #include "gormcb_c.inc"

    static _go32_dpmi_seginfo rm_cb;
  #endif

  static DWORD rm_base;

  /*
   * real-data is organised as follows: PKT_TMP at rm_base + 0
   *                                    TX_BUF  at PKT_TMP + PKT_TMP_SIZE
   *                                    RX_BUF  at TX_BUF  + RX_SIZE
   */
  #define PKT_TMP()     0
  #define TX_BUF()      (PKT_TMP() + PKT_TMP_SIZE)
  #define RX_BUF()      (TX_BUF() + RX_SIZE)
  #define RDATA_SIZE    (RX_BUF() + RX_SIZE)
  #define RP_SEGM()     _pkt_inf->rm_seg

#elif (DOSX & DOS4GW)         /* All DOS4GW type extenders */

  static DWORD rm_base;       /* Linear address (in DOS) for allocated area */

  #if !defined(USE_FAST_PKT)
    static WORD  pkt_inf_sel; /* selector returned from `_pkt_inf' allocation */
  #endif

  /*
   * The DOS-area (0-1MB) is automagically mapped into application
   * data.  This eases communication with packet-driver, but clean
   * crashes can not be expected. i.e. bugs (dereferencing null-ptr)
   * in application will most likely hang the machine.
   *
   * Real-mode code/data is organised like this:
   *   pkt_receiver4_start() copied to allocated area at rm_base
   *   PKT_TMP   at rm_base + (pkt_receiver4_end() - pkt_receiver4_start())
   *   TX_BUF    at PKT_TMP + PKT_TMP_SIZE
   *   end area  at TX_BUF + RX_SIZE
   *   RX_BUF is in DOS-allocated `_pkt_inf' structure.
   */
  #define PKT_TMP()     ((DWORD)pkt_receiver4_end -  \
                         (DWORD)pkt_receiver4_start)
  #define TX_BUF()      (PKT_TMP() + PKT_TMP_SIZE)
  #define RCV_OFS()     ((DWORD)pkt_receiver4_rm - \
                         (DWORD)pkt_receiver4_start)
  #define RP_SEGM()     _pkt_inf->rm_seg

#elif (DOSX & POWERPAK)
  static REALPTR         rm_base;

  #if !defined(USE_FAST_PKT)
    static DPMI_callback rm_cb;
  #endif

  #define PKT_TMP()     0
  #define TX_BUF()      (PKT_TMP() + PKT_TMP_SIZE)
  #define RX_BUF()      (TX_BUF() + RX_SIZE)
  #define RDATA_SIZE    (RX_BUF() + RX_SIZE)
  #define RP_SEGM()     _pkt_inf->rm_seg

#elif (DOSX == 0)
  void (__cdecl _far * pkt_enque_ptr) (BYTE _far *buf, WORD len);
  void (__cdecl _far *_pkt_enque_ptr) (BYTE _far *buf, WORD len);

#else
  #error Help me!
#endif


#if defined(USE_FAST_PKT)
  #include "pcpkt2.c"

#elif (DOSX)
  static int setup_rmode_callback (void);
  static int lock_code_and_data   (void);
  static int unlock_code_and_data (void);
#endif

static int  find_vector    (int first, int num);
static int  setup_pkt_inf  (void);
static void get_rmode_data (void *dest, unsigned size, WORD seg, WORD ofs);
static BOOL pkt_api_entry  (IREGS *reg, unsigned called_from_line);

#define PKT_API(reg)       pkt_api_entry (reg, __LINE__)
#define CARRY_BIT          1   /* carry bit in (e)flags register */

#define PKT_ERR(str,code)  do {                                     \
                             const char *err = pkt_strerror (code); \
                             outs (_LANG(str));                     \
                             outsnl (err);                          \
                           } while (0)

/**
 * Return textual error representing error-code.
 */
const char * W32_CALL pkt_strerror (int code)
{
  static char  buf[50];
  static const char *errors[] = {
              __LANG("Unknown driver error (0x--)"),
    /* 1 */   __LANG("Invalid handle number"),
              __LANG("No interfaces of specified class found"),
    /* 3 */   __LANG("No interfaces of specified type found"),
              __LANG("No interfaces of specified number found"),
    /* 5 */   __LANG("Bad packet type specified"),
              __LANG("This interface does not support multicast"),
    /* 7 */   __LANG("This packet driver cannot terminate"),
              __LANG("An invalid receiver mode was specified"),
    /* 9 */   __LANG("Operation failed because of insufficient space"),
              __LANG("Type previously accessed, and not released"),
    /* 11 */  __LANG("The command was out of range, or not implemented"),
              __LANG("Cannot send packet (hardware error)"),
    /* 13 */  __LANG("Cannot change hardware address"),
              __LANG("Hardware address has bad length/format"),
    /* 15 */  __LANG("Could not reset interface"),
              __LANG("Extended driver needed")
            };
  const char *rc;
  char       *p;

  if (code > 0 && code < DIM(errors))
     return (_LANG(errors[code]));

  rc = _strlcpy (buf, _LANG(errors[0]), sizeof(buf));
  p  = strchr (rc, '(');
  if (p && strlen(p) >= 5)
  {
    code &= 0xFF;
    p[3] = hex_chars_upper [code >> 4];
    p[4] = hex_chars_upper [code & 15];
  }
  return (buf);
}

/* Return TRUE is adapter is up.
 */
BOOL W32_CALL pkt_is_init (void)
{
  return (_pkt_inf != NULL);
}

/*
 * Only for testing asmpkt4.asm
 */
#if (DOSX & DOS4GW) && 0
static void dump_asm4 (void)
{
  const BYTE *p = (const BYTE*) rm_base;
  int   i;

  printf ("PKT_INF %08lX, HEAD %08lX, QUEUE %08lX, INDEX %04X, XYPOS %d\n",
          *(DWORD*)p, *(DWORD*)(p+4), *(DWORD*)(p+8), *(WORD*)(p+12),
          *(WORD*)(p+14));

  printf ("rm_base %08lX, handle %04X, is_serial %d\n",
          rm_base, _pkt_inf->handle, _pkt_inf->is_serial);

  printf ("VAR_1-14: ");
  for (i = 16; i <= 42; i += 2)
      printf ("%04X ", *(WORD*)(p+i));

#if 0
  printf ("PKT_RECV: ");
  for (i = RCV_OFS(); i < RCV_OFS()+10; i++)
      printf ("%02X ", p[i]);

  printf ("\nPKT_TMP:  ");
  for (i = PKT_TMP(); i < PKT_TMP()+PKT_TMP_SIZE; i++)
      printf ("%02X ", p[i]);
#endif
  fflush (stdout);
}
#endif


/**
 * Return address of DOS memory Tx-buffer.
 * For DOS4GW/X32VM (and djgpp with near-ptrs) targets, return near
 * address of transmit buffer located in DOS memory.
 */
#if (DOSX & DOS4GW)
void *pkt_tx_buf (void)
{
  return (void*)(rm_base + TX_BUF());
}

#elif (DOSX & DJGPP)
void *pkt_tx_buf (void)
{
  if (_pkt_inf && _pkt_inf->use_near_ptr)
     return (void*)(rm_base + TX_BUF() + __djgpp_conventional_base);
  fprintf (stderr, "%s (%d): wrong usage of pkt_tx_buf()\n",
           __FILE__, __LINE__);
  exit (-1);
  return (NULL);
}

#elif (DOSX & X32VM)
void *pkt_tx_buf (void)
{
  return (void*) ((RP_SEG(rm_base) << 4) + RP_OFF(rm_base) +
                  (DWORD)_x386_zero_base_ptr + TX_BUF());
}
#endif

/**
 * Setup the receiver upcall handler. The handler is called for
 * all packets matching current receive mode. In mode 3 all broadcast
 * and directed packets.
 */
static int pkt_set_access (void)
{
  IREGS regs;

  ASSERT_PKT_INF (0);

  memset (&regs, 0, sizeof(regs));

  regs.r_ax  = PD_ACCESS | _pktdevclass;
  regs.r_bx  = 0xFFFF;        /* any type          */
  regs.r_dx  = 0;             /* generic interface */
  regs.r_cx  = 0;             /* receive any proto */

#if defined(USE_FAST_PKT)
  regs.r_es = _pkt_inf->rm_seg;
  regs.r_di = PktReceiver;

#elif (DOSX & DJGPP)
  regs.r_es = rm_cb.rm_segment;
  regs.r_di = rm_cb.rm_offset;

#elif (DOSX & (PHARLAP|X32VM))
  regs.r_es = RP_SEGM();
  regs.r_di = 0;              /* RMCB aligned at para address */

#elif (DOSX & POWERPAK)
  regs.r_es = RP_SEG (rm_cb.rm_addr);
  regs.r_di = RP_OFF (rm_cb.rm_addr);

#elif (DOSX & DOS4GW)
  regs.r_es = RP_SEGM();
  regs.r_di = RCV_OFS();

#else /* real-mode targets */
  regs.r_es = FP_SEG (pkt_receiver_rm);  /* = this CS */
  regs.r_di = FP_OFF (pkt_receiver_rm);
#endif

  if (!PKT_API(&regs))
  {
    PKT_ERR ("Error allocating handle: ", _pkt_errno);
    return (0);
  }
  _pkt_inf->handle = (WORD) regs.r_ax;
  return (1);
}

/**
 * Fetch the PKTDRVR information. Device level, class and name.
 */
static int pkt_drvr_info (void)
{
  IREGS regs;
  int   rc = 1;  /* assume okay */

  ASSERT_PKT_INF (0);

  /* Lets find out about the driver
   */
  memset (&regs, 0, sizeof(regs));
  regs.r_ax = PD_DRIVER_INFO;

  if (!PKT_API(&regs))
  {
    TRACE_CONSOLE (1, "Warning: old-type PKTDRVR\n");
    _pktdevclass  = PDCLASS_ETHER;      /* assume Ethernet */
    _pkt_ip_ofs   = sizeof(eth_Header);
    _pkt_type_ofs = offsetof (struct eth_Header, type);
    _pkt_errno    = 0;
  }
  else          /* new Packet-Driver (1.09+) */
  {
    _pktdevlevel = loBYTE (regs.r_ax);
    _pktdevclass = hiBYTE (regs.r_cx);
    pkt_drvr_ver = regs.r_bx & 0xFFFF;

    switch (_pktdevclass)
    {
      case PDCLASS_TOKEN:
      case PDCLASS_TOKEN_RIF:  /* !! fix me */
           _pkt_ip_ofs   = sizeof(tok_Header);
           _pkt_type_ofs = offsetof (struct tok_Header, type);
           break;

      case PDCLASS_ETHER:
           _pkt_ip_ofs   = sizeof(eth_Header);
           _pkt_type_ofs = offsetof (struct eth_Header, type);
           break;

      case PDCLASS_FDDI:
           _pkt_ip_ofs   = sizeof(fddi_Header);
           _pkt_type_ofs = offsetof (struct fddi_Header, type);
           break;

      case PDCLASS_ARCNET:
           _pkt_ip_ofs   = ARC_HDRLEN;
           _pkt_type_ofs = ARC_TYPE_OFS;
           break;

      case PDCLASS_SLIP:
      case PDCLASS_PPP:
      case PDCLASS_AX25:    /* !! for now */
           _pkt_ip_ofs   = 0;
           _pkt_type_ofs = 0;
           break;

      default:
           outs (_LANG("PKT-ERROR: Unsupported driver class "));
           outhex ((char)_pktdevclass);
           outsnl ("h");
           _pkt_errno = PDERR_NO_CLASS;
           rc = 0;
    }
    get_rmode_data (_pktdrvrname, sizeof(_pktdrvrname), regs.r_ds, regs.r_si);
    _pktdrvrname [sizeof(_pktdrvrname)-1] = '\0';

   if (pkt_txwait == 0 &&
       !strcmp(_pktdrvrname,"NDIS3PKT"))
      pkt_txwait = 1;    /* This helps when using NDIS3PKT */
  }

  _pktserial = (_pktdevclass == PDCLASS_SLIP ||
                _pktdevclass == PDCLASS_PPP  ||
                _pktdevclass == PDCLASS_AX25);

  _pkt_inf->is_serial    = _pktserial;
  _pkt_inf->pkt_ip_ofs   = _pkt_ip_ofs;
  _pkt_inf->pkt_type_ofs = _pkt_type_ofs;
  return (rc);
}

/**
 * Get PKTDRVR parameters (MTU, version etc.)
 */
int pkt_get_params (struct PktParameters *params)
{
  IREGS regs;
  int   rc = 0;

  memset (&regs, 0, sizeof(regs));
  regs.r_ax = PD_GET_PARAM;  /* get driver parameters */

  DISABLE();
  if (PKT_API(&regs))
  {
    get_rmode_data (params, sizeof(*params), regs.r_es, regs.r_di);
    got_params = TRUE;
    rc = 1;
  }
  ENABLE();
  return (rc);
}

/**
 * Return PKTDRVR maximum-transmit-units (MTU).
 * \note This includes length of MAC-header.
 */
int pkt_get_mtu (void)
{
  if (_pktdevclass == PDCLASS_ARCNET)
     return (ARCNET_MAX_DATA - 8);

  if (got_params)
     return (pkt_params.mtu);
  return (-1);
}

#ifdef NOT_USED
/**
 * Return length of MAC address.
 * \retval 6 For Ethernet.
 */
int pkt_get_mac_len (void)
{
  if (_pktdevclass == PDCLASS_ARCNET)
     return (sizeof(arcnet_address));
  if (got_params)
     return (pkt_params.addr_len);
  return (-1);
}
#endif

/**
 * Return version of spec. this PKTDRVR conforms to.
 * \retval -1 on error.
 * Return major version in upper 8 bit, minor in lower.
 */
int pkt_get_api_ver (WORD *ver)
{
  if (got_params)
  {
    *ver = ((pkt_params.major_rev << 8) + pkt_params.minor_rev);
    return (1);
  }
  return (0);
}

/**
 * Return version of PKTDRVR.
 */
int pkt_get_drvr_ver (WORD *major, WORD *minor, WORD *unused1, WORD *unused2)
{
  if (!pkt_drvr_ver)
     return (0);
  *major = pkt_drvr_ver >> 8;
  *minor = pkt_drvr_ver & 255;
  ARGSUSED (unused1);
  ARGSUSED (unused2);
  return (1);
}

/**
 * Return PKTDRVR vector.
 */
W32_FUNC int W32_CALL pkt_get_vector (void)
{
  return (pkt_interrupt);
}

/*
 * Return driver class
 */
W32_FUNC WORD W32_CALL pkt_get_drvr_class (void)
{
  return (_pktdevclass);
}

/*
 * Returns low-level PktDriver name.
 */
const char * W32_CALL pkt_get_device_name (void)
{
  return (_pktdrvrname);
}

/**
 * Called from pkt_eth_init() to search for PKTDRVR.
 *  - Checks register struct size (for non-DOSX targets).
 *  - Searches for the PKTDRVR interrupt.
 *  - Allocates '_pkt_inf' structure.
 *  - Sets up a real-mode callback and lock code/data (DOSX targets).
 *  - Fetches PKTDRVR information.
 *  - Sets up driver to call our real-mode callback.
 *  - Fetches local MAC address.
 *  - Fetches parameters and initialise statistics.
 */
static int pkt16_drvr_init (mac_address *mac_addr)
{
  /* If interrupt specified in config-file ("PKT.VECTOR = 0xNN"),
   * check a single vector. Else, search for the 1st driver in range
   * 0x60-0x80.
   */
  if (pkt_interrupt)
       pkt_interrupt = find_vector (pkt_interrupt, 1);
  else pkt_interrupt = find_vector (PKT_FIRST_VEC,
                                    PKT_LAST_VEC - PKT_FIRST_VEC + 1);

  if (pkt_interrupt == 0)
  {
    outsnl (_LANG("NO PACKET DRIVER FOUND."));
    return (WERR_NO_DRIVER);
  }

  if (!setup_pkt_inf())
  {
    outsnl (_LANG("Failed to allocate PACKET DRIVER data."));
    return (WERR_NO_MEM);
  }

#if defined(USE_FAST_PKT)
  switch (setup_rmode_receiver())
  {
    case -1:
         outsnl (_LANG("Error in PACKET DRIVER stub code."));
         return (WERR_PKT_ERROR);
  }
#elif (DOSX)
  switch (setup_rmode_callback())
  {
    case -1:
         outsnl (_LANG("Failed to allocate callback for PACKET DRIVER."));
         return (WERR_NO_MEM);
    case -2:
         outsnl (_LANG("Failed to allocate DOS-mem for PACKET DRIVER."));
    case -3:    /* msg already printed */
         return (WERR_NO_MEM);
  }

  if (lock_code_and_data() < 0)
  {
    outsnl (_LANG("Failed to lock code/data for PACKET DRIVER."));
    return (WERR_PKT_ERROR);
  }
#endif   /* USE_FAST_PKT */

  if (!pkt_drvr_info())         /* get device class etc. of driver */
     return (WERR_PKT_ERROR);

  if (!pkt_set_access())        /* set upcall receive handler */
  {
    pkt_release();
    return (WERR_PKT_ERROR);
  }

  if (!pkt_get_addr(mac_addr))  /* get our MAC address */
  {
    pkt_release();
    return (WERR_PKT_ERROR);
  }

  memset (&pkt_params, 0, sizeof(pkt_params));
  got_params = FALSE;

  if (_pktdevlevel >= 5 && _pktdevlevel < 255) /* high-performance driver */
  {
    if (!pkt_get_params(&pkt_params))
       TRACE_CONSOLE (1, "Failed to get PKTDRVR params; %s\n",
                      pkt_strerror(_pkt_errno));
  }

#if defined(USE_STATISTICS)
  if (_pktdevlevel >= 2 && _pktdevlevel < 255) /* extended driver */
     get_init_stats();
#endif

  if (_pktserial)
     return (0);  /* success */

  /* Set Rx-mode forced via config.
   */
  if (_pkt_forced_rxmode != -1 &&
      (_pkt_forced_rxmode < -1 ||
       _pkt_forced_rxmode > RXMODE_PROMISCOUS))
  {
    TRACE_CONSOLE (0, "Illegal Rx-mode (%d) specified\n",
                   _pkt_forced_rxmode);
    _pkt_forced_rxmode = -1;
  }

  if (pkt_get_rcv_mode())
     _pkt_rxmode0 = _pkt_rxmode;   /* remember startup mode */

  if (_pkt_forced_rxmode != -1 &&
      _pkt_forced_rxmode != _pkt_rxmode0)
     pkt_set_rcv_mode (_pkt_forced_rxmode);

  /* else hope _pkt_rxmode0 is >= RXMODE_BROADCAST */

  return (0);  /* success */
}

/*
 * The following functions (ending at '_pkt_end()') are called
 * at interrupt time (or asynchronously). 'pkt_release()' may be
 * called from SIGSEGV handler. Therefore don't assume anything
 * about the state of our stack (except hope it's large enough).
 * And don't use large local variables here. Hope we will not be
 * reentered since not all functions below are reentrant.
 */

/* Disable stack-checking here
 */
#if defined(__HIGHC__)
  #pragma off(Call_trace)
  #pragma off(Prolog_trace)
  #pragma off(Epilog_trace)
#endif

#if defined(WATCOM386)
 /* #pragma option -zu */                      /* assume SS != DS (doesn't work) */
 /* #pragma aux pkt_release __modify[__ss] */  /* !! fix-me (doesn't work) */
#endif

#include "nochkstk.h"


/**
 * Release the allocated protocol handle.
 */
int pkt_release_handle (WORD handle)
{
  static IREGS regs;  /* `static' because the stack could be too small */

  memset (&regs, 0, sizeof(regs));
  regs.r_ax = PD_RELEASE;
  regs.r_bx = handle;

  if (!PKT_API(&regs) && !_watt_fatal_error)
     PKT_ERR ("Error releasing handle: ", _pkt_errno);
  return (1);
}

/**
 * Reset the driver-state associated with handle.
 */
int pkt_reset_handle (WORD handle)
{
  static IREGS regs;  /* `static' because the stack could be too small */

  memset (&regs, 0, sizeof(regs));
  regs.r_ax = PD_RESET;
  regs.r_bx = handle;

  if (!PKT_API(&regs))
     PKT_ERR ("Error resetting handle: ", _pkt_errno);
  return (1);
}

#if (DOSX) && !defined(USE_FAST_PKT)
static void release_callback (void)
{
#if (DOSX & (PHARLAP|X32VM))
  if (rm_base)
     _dx_free_rmode_wrapper (rm_base);
  rm_base = 0;

#elif (DOSX & POWERPAK)
  if (rm_cb.pm_func)
     dpmi_free_callback_retf (&rm_cb);
  rm_cb.pm_func = 0;

#elif (DOSX & DJGPP)
  if (rm_cb.pm_offset)
     _pkt_dpmi_free_real_mode_callback (&rm_cb);
  rm_cb.pm_offset = 0;
#endif
}
#endif  /* DOSX && !USE_FAST_PKT */


/**
 * Release allocated DOS memory.
 */
#if (DOSX) && defined(USE_FAST_PKT)
static void release_real_mem (void)
{
#if (DOSX & PHARLAP)
  if (_pkt_inf && _pkt_inf->rm_seg)
     _dx_real_free (_pkt_inf->rm_seg);
  rm_base = 0;

#elif (DOSX & X32VM)
  if (rm_base)
     _x32_real_free (rm_base);
  rm_base = 0;

#elif (DOSX & POWERPAK)
  if (_pkt_inf && _pkt_inf->rm_sel)
     dpmi_real_free (_pkt_inf->rm_sel);

#elif (DOSX & DJGPP)
  if (_pkt_inf && _pkt_inf->rm_sel)
  {
    __dpmi_error = 0;
    __dpmi_free_dos_memory (_pkt_inf->rm_sel);  /* DPMI fn 101h */
    if (__dpmi_error)
    {
      (*_printf) ("%s (%u): DPMI/DOS error %04Xh\n",
                  __FILE__, __LINE__, __dpmi_error);
      TRACE_FILE ("release_real_mem: dpmi_error %u\n", __dpmi_error);
    }
  }
  rm_base = (DWORD)-1;  /* crash if used after freed */

#elif (DOSX & DOS4GW)
  if (!_watt_fatal_error && _pkt_inf)
  {
    if (_pkt_inf->rm_sel)
       dpmi_real_free (_pkt_inf->rm_sel);

#if !defined(USE_FAST_PKT)
    if (pkt_inf_sel)
       dpmi_real_free (pkt_inf_sel);
    pkt_inf_sel = 0;
#endif
  }
#endif

  if (_pkt_inf)
     _pkt_inf->rm_sel = _pkt_inf->rm_seg = 0;

#if !defined(USE_FAST_PKT)
  _pkt_inf = NULL;  /* don't free this ptr below */
#endif
}
#endif  /* DOSX && USE_FAST_PKT */


/**
 * Release the pkt-driver.
 *
 * Might be called from exception/signal handler.
 * To make sure the driver is released, use a destructor in addition
 * to rundown function and sock_exit(). This will be called even if
 * _exit() is called.
 */
#ifdef __WATCOMC__
#pragma aux pkt_release loadds;
#endif

#if defined(__GNUC__)
#define DTOR __attribute__((destructor))
#else
#define DTOR
#endif

int DTOR pkt_release (void)
{
  DISABLE();

  if (_pkt_inf)
  {
    /* Don't do this by default. It may cause some (RealTek)
     * packet-drivers to fail all further upcalls.
     */
    if (pkt_do_reset)
       pkt_reset_handle (_pkt_inf->handle);
    pkt_release_handle (_pkt_inf->handle);
  }

/* !! pkt_interrupt = 0; */

  /**
   * \todo We might be called between 1st and 2nd packet-driver
   *       upcall. Need to wait for 2nd upcall to finish or else
   *       freeing the RMCB too early could cause a crash or a
   *       stuck PKTDRVR.
   */

#if (DOSX)
  #if defined(USE_FAST_PKT)
    release_real_mem();
  #else
    unlock_code_and_data();   /* unlock before freeing */
    release_callback();
  #endif
#endif

  if (_pkt_inf && !_watt_fatal_error)
     free (_pkt_inf);

  _pkt_inf = NULL;  /* drop anything still in the queue */

  ENABLE();
  return (1);
}

#if defined(__BORLANDC__)
  #pragma exit pkt_release 100
#elif defined(__CCDL__)
  #pragma rundown pkt_release 110
#endif


#if !defined(USE_FAST_PKT) && !(DOSX & DOS4GW)

/**
 *  Enqueue a received packet into '_pkt_inf->pkt_queue'.
 *
 *  This routine is called from pkt_receiver_rm/_pm().
 *  The packet has been copied to rx_buffer (in DOS memory) by the
 *  packet-driver. We now must copy it to correct queue.
 *  Interrupts are disabled on entry.
 *
 *  Note 1: For real-mode targets SS and SP have been setup to a small
 *          work stack in asmpkt.asm (SS = CS). The stack can only take
 *          64 pushes, hence use few local variables here.
 *
 *  Note 2: The C-compiler must be told NOT to use register calling
 *          for this routine (MUST use __cdecl) because it's called from
 *          asmpkt.asm via `pkt_enque_ptr' function pointer.
 *
 *  Note 3: Watcom/DOS4GW targets doesn't use real->prot mode upcall (RMCB),
 *          but does the following in asmpkt4.asm instead.
 *
 *  Note 4: For DOSX targets, all code from pkt_enqueue() down to _pkt_end()
 *          must be locked in memory.
 *
 *  HACK: For real-mode targets this routine is called via the
 *        `pkt_enque_ptr' function pointer. This was the only way
 *        I could avoid a fixup error for small-model programs.
 */

#if (DOSX)
static void pkt_enqueue (unsigned rxBuf, WORD rxLen)
#else
static void __cdecl _far pkt_enqueue (BYTE _far *rxBuf, WORD rxLen)
#endif
{
  struct pkt_ringbuf *q = &_pkt_inf->pkt_queue;
  int    index;

  /* don't use pktq_in_index() and pktq_in_buf() because they
   * are not in locked code area.
   */
  index = q->in_index + 1;
  if (index >= q->num_buf)
      index = 0;

  if (index != q->out_index)
  {
    char *head   = (char*)q->buf_start + (q->buf_size * q->in_index);
    int   padLen = q->buf_size - 4 - rxLen;

    if (rxLen > q->buf_size - 4)  /* don't overwrite marker */
    {
      rxLen  = q->buf_size - 4;
      padLen = 0;
    }

#if (DOSX & (PHARLAP|X32VM|POWERPAK))
    ReadRealMem (head, rm_base + rxBuf, rxLen);

#elif (DOSX & DJGPP)
    if (_pkt_inf->use_near_ptr)
         memcpy (head, (void*)(rm_base+rxBuf+__djgpp_conventional_base), rxLen);
    else DOSMEMGETL (rm_base + rxBuf, (rxLen+3)/4, head);

#else  /* real-mode targets */
    _fmemcpy (head, rxBuf, rxLen);
#endif

    /* Zero-fill remaining old data in this buffer.
     */
    head += rxLen;
    while (padLen--)
       *head++ = '\0';
    q->in_index = index;   /* update buffer tail-index */
  }
  else
    q->num_drop++;         /* no room, increment drop count */
}


/*
 * We have allocated a real-mode callback (RMCB) to gain control
 * here when the packet-driver makes an upcall.
 *
 * Entry AL = 0; driver requests a buffer. We return ES:DI of real-mode buffer
 *       BX = handle (IP, ARP or RARP)
 *       CX = length of packet
 *
 * Entry AL = 1; driver has put the data in buffer, we then enqueues the buffer
 *       BX = handle (IP, ARP or RARP)
 *       CX = length of packet (same as CX above)
 *       DS:SI = *must* be same as ES:DI returned above.
 *       ES:DI = should be same as ES:DI returned above (but don't count on it).
 *
 * Interrupts are disabled on entry.
 * Note: This function MUST be declared cdecl (not register called).
 */
#if (DOSX & (PHARLAP|X32VM))
  static void cdecl pkt_receiver_pm (SWI_REGS *r)
  {
    PUSHF_CLI();

    if ((BYTE)r->r_ax == 0)         /* AL == 0; rx-buffer request */
    {
      if (!_pkt_inf || (WORD)r->r_cx > ETH_MAX)
      {
        r->r_es = 0;
        r->r_di = 0;
      }
      else
      {
        r->r_es = RP_SEGM();
        r->r_di = RX_BUF();
      }
    }
    else if ((WORD)r->r_si && _pkt_inf)   /* AL != 0; rx-buffer filled */
    {
      pkt_enqueue (RX_BUF(), (WORD)r->r_cx);
    }
    POPF();
  }

#elif (DOSX & DJGPP)
  static void cdecl pkt_receiver_pm (__dpmi_regs *r)
  {
    WORD fs, gs;

    PUSHF_CLI();      /* "cld" already done in RMCB wrapper */

    fs = get_fs_reg();
    gs = get_gs_reg();

    if (r->h.al == 0)
    {
      if (!_pkt_inf || r->x.cx > ETH_MAX)
      {
        r->x.es = 0;
        r->x.di = 0;
      }
      else
      {
        r->x.es = RP_SEGM();
        r->x.di = RX_BUF();
      }
    }
    else if (r->x.si && _pkt_inf)
    {
      pkt_enqueue (RX_BUF(), r->x.cx);
    }

    set_fs_reg (fs);
    set_gs_reg (gs);

    POPF();
  }

#elif (DOSX & POWERPAK)
  static void cdecl pkt_receiver_pm (void)
  {
    REAL_regs *r;

    PUSHF_CLI();
    r = &rm_cb.rm_regs;

    if ((BYTE)r->r_ax == 0)
    {
      if (!_pkt_inf || (WORD)r->r_cx > ETH_MAX)
      {
        r->r_es = 0;
        r->r_di = 0;
      }
      else
      {
        r->r_es = RP_SEGM();
        r->r_di = RX_BUF();
      }
    }
    else if (r->r_si && _pkt_inf)
    {
      pkt_enqueue (RX_BUF(), (WORD)r->r_cx);
    }
    POPF();
  }
#endif

void _pkt_end (void) {}

#if defined(__WATCOMC__)
#pragma alloc_text (pkt_lock_TEXT, pkt_enqueue, pkt_receiver_pm, _pkt_end);
#endif

#endif  /* !(DOSX & DOS4GW) && !USE_FAST_PKT */


/**
 * Send a link-layer frame. For PPP/SLIP 'tx' contains no MAC-header.
 * For EtherNet, Token-Ring, ARCNET and FDDI, 'tx' contains the
 * complete frame.
 *
 * \todo Change this so that we doesn't retry if the driver was
 *       stuck for "too long". I've seen drivers using approx.
 *       100 msec on each iteration below. Retrying if the driver is
 *       stuck is not worth the time.
 */
int pkt_send (const void *tx, int length)
{
  IREGS  regs;
  WORD   tx_seg, tx_ofs;
  BOOL   fail = FALSE;
  int    tx_cnt;

  if (!_pkt_inf)
     return (0);

  PROFILE_START ("pkt_send");

  memset (&regs, 0, sizeof(regs));

#if (DOSX & (PHARLAP|POWERPAK))
  tx_seg = RP_SEGM();
  tx_ofs = TX_BUF();
  WriteRealMem (rm_base + tx_ofs, (void*)tx, length);

#elif (DOSX & (DOS4GW|X32VM))
  tx_seg = RP_SEGM();
  tx_ofs = TX_BUF();
  WATT_ASSERT (tx == pkt_tx_buf());  /* no need to copy anything */

#elif (DOSX & DJGPP)
  tx_seg = RP_SEGM();
  tx_ofs = TX_BUF();
  if (_pkt_inf->use_near_ptr)
       WATT_ASSERT (tx == pkt_tx_buf());
  else DOSMEMPUTL (tx, (length+3)/4, rm_base + tx_ofs);

#else
  tx_seg = FP_SEG (tx);
  tx_ofs = FP_OFF (tx);
#endif

  for (tx_cnt = 0; tx_cnt <= pkt_txretries; tx_cnt++)
  {
    regs.r_cx = length;
    regs.r_ax = PD_SEND;
    regs.r_ds = tx_seg;
    regs.r_si = tx_ofs;

    fail = FALSE;

    if (PKT_API(&regs))   /* success */
       break;

    fail = TRUE;
    if (pkt_txwait > 0)
       Wait (pkt_txwait);
  }

  if (tx_cnt > 0)
     STAT (macstats.num_tx_retry += tx_cnt-1);

  if (fail)
     length = 0;

  if (length == 0)
     STAT (macstats.num_tx_err++);

  PROFILE_STOP();
  return (length);
}

/**
 * Return the MAC address.
 */
int pkt_get_addr (mac_address *mac)
{
  IREGS regs;
  WORD  seg, ofs;
  int   rc;

  ASSERT_PKT_INF (0);

  PROFILE_START ("pkt_get_addr");

  regs.r_ax = PD_GET_ADDRESS;
  regs.r_bx = _pkt_inf->handle;
  regs.r_cx = sizeof (*mac);

#if (DOSX)
  regs.r_es = seg = RP_SEGM();
  regs.r_di = ofs = PKT_TMP();
#else
  regs.r_es = seg = FP_SEG (mac);
  regs.r_di = ofs = FP_OFF (mac);
#endif

  if (!PKT_API(&regs))
  {
    PKT_ERR ("Cannot read own MAC address: ", _pkt_errno);
    rc = 0;
  }
  else
  {
    get_rmode_data (mac, sizeof(*mac), seg, ofs);
    rc = 1;
  }
  PROFILE_STOP();
  return (rc);
}

/**
 * Set a new MAC source address.
 */
int pkt_set_addr (const void *addr)
{
  IREGS regs;

  ASSERT_PKT_INF (0);

  regs.r_ax = PD_SET_ADDR;
  regs.r_cx = sizeof (mac_address);

#if (DOSX & DJGPP)
  DOSMEMPUT (addr, sizeof(mac_address), rm_base + PKT_TMP());
  regs.r_es = RP_SEGM();
  regs.r_di = PKT_TMP();

#elif (DOSX & DOS4GW)
  memcpy ((void*)(rm_base + PKT_TMP()), addr, sizeof(mac_address));
  regs.r_es = RP_SEGM();
  regs.r_di = PKT_TMP();

#elif (DOSX & (PHARLAP|X32VM|POWERPAK))
  WriteRealMem (rm_base + PKT_TMP(), (void*)addr, sizeof(mac_address));
  regs.r_es = RP_SEGM();
  regs.r_di = PKT_TMP();

#else
  {
    mac_address tmp;
    memcpy (&tmp, addr, sizeof(tmp)); /* prevent driver modifying '*addr' */
    regs.r_es = FP_SEG (&tmp);
    regs.r_di = FP_OFF (&tmp);
  }
#endif

  if (!PKT_API(&regs))
     return (0);
  return (1);
}

#if defined(USE_STATISTICS)
/**
 * Get PKTDRVR statistics.
 *
 * Return both 'stats' for current session and 'total' statistics
 * since driver loaded.
 */
int pkt_get_stats (struct PktStats *stats, struct PktStats *total)
{
  IREGS regs;
  int   rc = 0;

  PROFILE_START ("pkt_get_stats");
  DISABLE();

  /* Handle in BX is optional for 1.11 drivers
   */
  regs.r_bx = _pkt_inf ? _pkt_inf->handle : 0;
  regs.r_ax = PD_GET_STATS;

  if (!PKT_API(&regs))
     goto fail;

  rc = 1;
  get_rmode_data (stats, sizeof(*stats), regs.r_ds, regs.r_si);

  if (total)
     *total = *stats;

  stats->in_packets  -= init_stats.in_packets;
  stats->out_packets -= init_stats.out_packets;
  stats->in_bytes    -= init_stats.in_bytes;
  stats->out_bytes   -= init_stats.out_bytes;
  stats->in_errors   -= init_stats.in_errors;
  stats->out_errors  -= init_stats.out_errors;
  stats->lost        -= init_stats.lost;
fail:
  ENABLE();
  PROFILE_STOP();
  return (rc);
}

static void get_init_stats (void)
{
  IREGS regs;

  regs.r_ax = PD_GET_STATS;
  regs.r_bx = _pkt_inf->handle;
  memset (&init_stats, 0, sizeof(init_stats));

  DISABLE();
  if (PKT_API(&regs))
     get_rmode_data (&init_stats, sizeof(init_stats), regs.r_ds, regs.r_si);
  ENABLE();
}


/**
 * Get VJ-compression statistics from pkt-driver. Only DOS-PPP
 * ver 0.6+ supports the PD_GET_VJSTATS (0x81) call.
 */
static WORD cstate_dseg = 0;

int pkt_get_vjstats (struct slcompress *vjstats)
{
  IREGS regs;
  int   rc = 0;

  regs.r_ax = PD_GET_VJSTATS;
  regs.r_bx = sizeof (*vjstats);

  DISABLE();
  if (PKT_API(&regs))
  {
    get_rmode_data (vjstats, sizeof(*vjstats), regs.r_ds, regs.r_si);
    cstate_dseg = regs.r_ds;
    rc = 1;
  }
  ENABLE();
  return (rc);
}

int pkt_get_cstate (struct cstate *cs, WORD cstate_ofs)
{
  if (cstate_dseg == 0 || cstate_ofs == 0)
     return (0);

  DISABLE();
  get_rmode_data (cs, sizeof(*cs), cstate_dseg, cstate_ofs);
  ENABLE();
  return (1);
}
#endif  /* USE_STATISTICS */


/**
 * Clear the receive queue.
 */
int pkt_buf_wipe (void)
{
#if defined(USE_FAST_PKT)
  pktq_clear (NULL);
#else
  ASSERT_PKT_INF (0);
  pktq_clear (&_pkt_inf->pkt_queue);
#endif
  return (1);
}

/**
 * Release a packet from the receive queue.
 * If USE_FAST_PKT, pkt has already been free'd. Just update drop count.
 */
void pkt_free_pkt (const void *pkt)
{
#if !defined(USE_FAST_PKT)  /* packet already copied in pcsed.c */
  struct pkt_ringbuf *q;

  if (!_pkt_inf || !pkt)
     return;

  q = &_pkt_inf->pkt_queue;
  pkt_drop_cnt = q->num_drop;

  if (pkt != (const void*) (pktq_out_buf(q) + _pkt_ip_ofs))
  {
    TRACE_CONSOLE (0, "%s: freeing illegal packet 0x%p.\n", __FILE__, pkt);
    pktq_clear (q);
  }
  else
    pktq_inc_out (q);
#else
  if (rm_base && rm_base < (DWORD)-1)
     pkt_drop_cnt = FAR_PEEK_DWORD (struct pkt_info, pkt_queue.num_drop);
  ARGSUSED (pkt);
#endif
}


/**
 * Return number of packets waiting in queue.
 */
int pkt_waiting (void)
{
#if defined(USE_FAST_PKT)
  if (rm_base && rm_base < (DWORD)-1)
     return pkt_buffers_used();
  return (-1);
#else
  if (_pkt_inf)
     return pktq_queued (&_pkt_inf->pkt_queue);
  return (-1);
#endif
}

/**
 * Return number of packets dropped.
 */
DWORD pkt_dropped (void)
{
#if defined(USE_FAST_PKT)
  if (rm_base && rm_base < (DWORD)-1)
     return pkt_rx_dropped();
#else
  if (_pkt_inf)
     return (_pkt_inf->pkt_queue.num_drop);
#endif
  return (pkt_drop_cnt);    /* return last known value */
}

    /**
 * Search WATTCP.CFG file for "PKT.VECTOR = 0x??" etc.
 * Accept "0x20 - 0xFF".
 */
static int parse_config_pass_1 (void)
{
  static int    vect = 0;
  static const struct config_table pkt_init_cfg[] = {
       { "PKT.VECTOR",    ARG_ATOX_B, (void*)&vect },
       { "PKT.TXRETRIES", ARG_ATOB,   (void*)&pkt_txretries },
       { "PKT.TXWAIT",    ARG_ATOI,   (void*)&pkt_txwait },
       { "PKT.RXMODE",    ARG_ATOI,   (void*)&_pkt_forced_rxmode },
       { "PKT.RXBUFS",    ARG_ATOI,   (void*)&pkt_num_rx_bufs },
       { "PKT.NEAR_PTR",  ARG_ATOI,   (void*)&pkt_use_near },
       { "PKT.RESET",     ARG_ATOI,   (void*)&pkt_do_reset },
       { NULL,            0,          NULL }
     };
  const struct config_table *cfg_save = watt_init_cfg;
  void (W32_CALL *init_save) (const char*, const char*) = usr_init;
  int    rc;

  watt_init_cfg = pkt_init_cfg;
  usr_init      = NULL;    /* only pkt_init_cfg[] gets parsed */
  rc = tcp_config (NULL);
  usr_init      = init_save;
  watt_init_cfg = cfg_save;
  if (!rc)
     return (0);
  return (vect);
}

/**
 * Initialise Packet driver interface.
 *
 * First determine vector to use;
 *   if WATTCP.CFG specifies a "PKT.VECTOR = 0xNN", use that vector,
 *   else search for PKTDRVR between PKT_FIRST_VEC and PKT_LAST_VEC
 *   (0x60 - 0x80). If the driver is outside this range, user must
 *   specify one with "PKT.VECTOR = 0x??".
 *
 * If DOSX, probe and initialise protected-mode driver (not operational).
 * Call pkt16_drvr_init() to initialise API.
 */
int pkt_eth_init (mac_address *addr)
{
  int vector, rc = -1;

  if (_watt_no_config && !_watt_user_config_fn)
       vector = 0;
  else vector = parse_config_pass_1();

  if (vector)
  {
    pkt_interrupt = vector;
    if (pkt_interrupt < PKT_FIRST_VEC_11 || /* 0x20-0xFF range for 1.11 drvr */
        pkt_interrupt > PKT_LAST_VEC_11)
        pkt_interrupt = 0;                  /* discard illegal value */
  }
  else
    pkt_interrupt = 0;

  {
#if (DOSX)
    int pm_driver = pkt32_drvr_probe (pm_driver_list);

    if (pm_driver > -1)
    {
      rc = pkt32_drvr_init (pm_driver, addr);
      if (rc == 0)
         TRACE_CONSOLE (2, "Using Pmode `%s' driver at %08lX\n",
                        pkt32_drvr_name(pm_driver), (DWORD)_pkt32_drvr);
    }
    if (rc)  /* pmode driver failed, try real-mode driver */
#endif
       rc = pkt16_drvr_init (addr);

    if (rc)
       return (rc);  /* no suitable driver found */
  }

#if defined(USE_FAST_PKT)
  FAR_POKE_WORD (struct pkt_info, handle, _pkt_inf->handle);
  FAR_POKE_WORD (struct pkt_info, is_serial, _pkt_inf->is_serial);
  FAR_POKE_WORD (struct pkt_info, pkt_ip_ofs, _pkt_inf->pkt_ip_ofs);
  FAR_POKE_WORD (struct pkt_info, pkt_type_ofs, _pkt_inf->pkt_type_ofs);

#if (DOSX & X32VM) && 0
  if (!pkt_test_upcall())
  {
    pkt_release();
    return (-1);
  }
#endif
#endif

  return (0);
}

/**
 * Check a single interrupt vector for signature string "PKT DRVR"
 * at offset 3 into intr-handler.
 */
#if (DOSX & (PHARLAP|X32VM))
  static BOOL check_intr_num (WORD intr_num)
  {
    REALPTR rp;
    char    temp[16];

    _dx_rmiv_get (intr_num, &rp);
    if (rp)
    {
      ReadRealMem (&temp, rp, sizeof(temp));
      if (!memcmp(temp+3, pkt_sign, strlen(pkt_sign)))
         return (TRUE);
    }
    return (FALSE);
  }

#elif (DOSX & DJGPP)
  static BOOL check_intr_num (WORD intr_num)
  {
    char  temp[16];
    DWORD rp;
    __dpmi_raddr realAdr;

    __dpmi_get_real_mode_interrupt_vector (intr_num, &realAdr);
    rp = (realAdr.segment << 4) + realAdr.offset16;
    if (rp)
    {
      dosmemget (rp, sizeof(temp), &temp);
      if (!memcmp(temp+3, pkt_sign, strlen(pkt_sign)))
         return (TRUE);
    }
    return (FALSE);
  }

#elif (DOSX & DOS4GW)
  static BOOL check_intr_num (WORD intr_num)
  {
    const BYTE *addr = (const BYTE*) dpmi_get_real_vector (intr_num);

    if (addr && !memcmp(addr+3, pkt_sign, strlen(pkt_sign)))
       return (TRUE);
    return (FALSE);
  }

#elif (DOSX & POWERPAK)
  static BOOL check_intr_num (WORD intr_num)
  {
    char    temp[16];
    DWORD   addr = (DWORD) dpmi_get_real_vector (intr_num);
    REALPTR rp;

    RP_SET (rp, addr & 0xF, addr >> 4);
    ReadRealMem (&temp, rp, sizeof(temp));
    if (!memcmp(temp+3, pkt_sign, strlen(pkt_sign)))
       return (TRUE);
    return (FALSE);
  }

#else       /* real-mode version */
  static BOOL check_intr_num (WORD intr_num)
  {
    char _far *addr = (char _far*) getvect (intr_num);

#if !defined(USE_FAST_PKT)
    pkt_enque_ptr  = pkt_enqueue;
    _pkt_enque_ptr = pkt_enqueue;
#endif

    if (addr && !_fmemcmp(addr+3, pkt_sign, strlen(pkt_sign)))
       return (TRUE);
    return (FALSE);
  }
#endif

/*
 * Find the interrupt-number for the PKTDRVR by searching interrupt
 * handler entries (at vector+3) for signature string "PKT DRVR".
 */
static int find_vector (int first, int num)
{
  int inum;

  for (inum = first; inum < first + num; inum++)
  {
    if (inum != 0x67 &&     /* can never be at EMS vector */
        check_intr_num(inum))
       return (inum);
  }
  return (0);
}

#if !defined(USE_FAST_PKT)
/*
 * DOS-extender functions for allocation a real-mode callback that
 * the real-mode PKTDRVR will call when a packet is received.
 * Lock down all code and data that is touched in this callback.
 */
#if (DOSX & (PHARLAP|X32VM))
  static int setup_rmode_callback (void)
  {
    rm_base = _dx_alloc_rmode_wrapper_retf (pkt_receiver_pm, NULL,
                                            RDATA_SIZE, 1024);
    if (!rm_base)
       return (-1);
    return (0);
  }

  static int lock_code_and_data (void) /* Needed for 386|VMM only (?) */
  {
    UINT size = (UINT)_pkt_end - (UINT)pkt_enqueue;

    _dx_lock_pgsn ((void*)pkt_enqueue, size);
    _dx_lock_pgsn ((void*)ReadRealMem, 100);
    _dx_lock_pgsn ((void*)_pkt_inf, sizeof(*_pkt_inf));
    _dx_lock_pgsn ((void*)&_pkt_inf, sizeof(_pkt_inf));

  #if (DOSX & X32VM)
    _dx_lock_pgsn ((void*)memcpy, 200);
  #endif
    return (0);
  }

  static int unlock_code_and_data (void)
  {
    UINT size = (UINT)_pkt_end - (UINT)pkt_enqueue;

    _dx_ulock_pgsn ((void*)pkt_enqueue, size);
    _dx_ulock_pgsn ((void*)ReadRealMem, 100);
    _dx_ulock_pgsn ((void*)_pkt_inf, sizeof(*_pkt_inf));
    _dx_ulock_pgsn ((void*)&_pkt_inf, sizeof(_pkt_inf));

  #if (DOSX & X32VM)
    _dx_ulock_pgsn ((void*)memcpy, 200);
  #endif
    return (0);
  }

#elif (DOSX & DJGPP)
  static int setup_rmode_callback (void)
  {
    static __dpmi_regs rm_reg;
    int    seg = __dpmi_allocate_dos_memory ((RDATA_SIZE + 15) / 16,
                                             (int*)&_pkt_inf->rm_sel);
    if (seg < 0)
       return (-2);

    _pkt_inf->rm_seg = seg;
    rm_cb.pm_offset = (DWORD) pkt_receiver_pm;

    memset (&rm_reg, 0, sizeof(rm_reg));
    if (_pkt_dpmi_allocate_real_mode_callback_retf (&rm_cb,&rm_reg))
       return (-1);

    rm_base = (_pkt_inf->rm_seg << 4);

    DOSMEMCLR (rm_base, RDATA_SIZE);
    return (0);
  }

  static int lock_code_and_data (void)
  {
    DWORD size = (DWORD)_pkt_end - (DWORD)pkt_enqueue;

    WATT_ASSERT ((DWORD)_pkt_end > (DWORD)pkt_enqueue);

    if (_go32_dpmi_lock_code((void*)pkt_enqueue, size)    ||
        _go32_dpmi_lock_code((void*)__movedata, 30)       ||
        _go32_dpmi_lock_code((void*)_movedatal, 30)       ||
        _go32_dpmi_lock_code((void*)__dj_movedata, 100)   ||
        _go32_dpmi_lock_data(&_pkt_inf, sizeof(_pkt_inf)) ||
        _go32_dpmi_lock_data(_pkt_inf, sizeof(*_pkt_inf)))
       return (-1);
    /* rm_reg is already locked */
    return (0);
  }

  static void _unlock (const void *addr, DWORD size)
  {
    __dpmi_meminfo mem;
    DWORD base = 0;

    __dpmi_get_segment_base_address (_my_ds(), &base);
    mem.address = base + (DWORD)addr;
    mem.size    = size;
    __dpmi_unlock_linear_region (&mem);
  }

  static int unlock_code_and_data (void)
  {
    _unlock ((void*)pkt_enqueue, (DWORD)_pkt_end - (DWORD)pkt_enqueue);
    _unlock ((void*)__movedata, 30);
    _unlock ((void*)_movedatal, 30);
    _unlock ((void*)__dj_movedata, 100);
    _unlock (_pkt_inf, sizeof(*_pkt_inf));
    _unlock (&_pkt_inf, sizeof(_pkt_inf));
    return (0);
  }

#elif (DOSX & DOS4GW)
  static int setup_rmode_callback (void)
  {
    int length;

    /* test for asmpkt4.asm/pcpkt.h mismatch
     */
    if (asmpkt_size_chk != sizeof(*_pkt_inf))
    {
#ifdef USE_DEBUG
      fprintf (stderr,
               "%s (%u): Development error:\n"
               "  sizeof(pkt_info) = %u pcpkt.h\n"
               "  sizeof(pkt_info) = %u asmpkt4.asm, (diff %d)\n",
               __FILE__, __LINE__,
               sizeof(*_pkt_inf), asmpkt_size_chk,
               sizeof(*_pkt_inf) - asmpkt_size_chk);
#endif
      return (-3);
    }

    /* Allocate DOS-memory for pkt_receiver4_rm() and temp/Tx buffers.
     */
    length = TX_BUF() + RX_SIZE;
    _pkt_inf->rm_seg = dpmi_real_malloc (length, &_pkt_inf->rm_sel);
    if (!_pkt_inf->rm_seg)
       return (-2);

    rm_base = ((DWORD)_pkt_inf->rm_seg) << 4;

    /* Clear DOS area and copy code down into it.
     */
    memset ((void*)rm_base, 0, length);
    length = PKT_TMP() - 1;
    memcpy ((void*)rm_base, pkt_receiver4_start, length);
    return (0);
  }

  /* No need (?) to lock anything.
   */
  static int lock_code_and_data (void)
  {
    return (0);
  }

  static int unlock_code_and_data (void)
  {
    return (0);
  }

#elif (DOSX & POWERPAK)
  static int setup_rmode_callback (void)
  {
    /* Allocate DOS-memory for temp/Tx buffers.
     */
    _pkt_inf->rm_seg = dpmi_real_malloc (RDATA_SIZE, &_pkt_inf->rm_sel);
    if (!_pkt_inf->rm_seg)
       return (-2);

    rm_base = (REALPTR) ((DWORD)_pkt_inf->rm_seg << 16);

    rm_cb.pm_func = pkt_receiver_pm;
    if (!dpmi_alloc_callback_retf(&rm_cb))
       return (-1);
    return (0);
  }

  static int lock_code_and_data (void)
  {
    unsigned size = (unsigned)_pkt_end - (unsigned)pkt_enqueue;

    if (!dpmi_lock_region ((void*)pkt_enqueue, size) ||
        !dpmi_lock_region ((void*)ReadRealMem, 100)  ||
        !dpmi_lock_region ((void*)_pkt_inf, sizeof(*_pkt_inf)) ||
        !dpmi_lock_region ((void*)&_pkt_inf, sizeof(_pkt_inf)))
       return (-1);
    return (0);
  }

  static int unlock_code_and_data (void)
  {
    unsigned size = (unsigned)_pkt_end - (unsigned)pkt_enqueue;

    if (!dpmi_unlock_region ((void*)pkt_enqueue, size) ||
        !dpmi_unlock_region ((void*)ReadRealMem, 100)  ||
        !dpmi_unlock_region ((void*)_pkt_inf, sizeof(*_pkt_inf)) ||
        !dpmi_unlock_region ((void*)&_pkt_inf, sizeof(_pkt_inf)))
       return (-1);
    return (0);
  }
#endif
#endif  /* !USE_FAST_PKT */


/*
 * For DOS4GW targets, allocate the '_pkt_inf' structure
 * from DOS memory. All others allocate from heap.
 */
static int setup_pkt_inf (void)
{
  /** \todo Make dynamic allocation based on 'pkt_num_rx_bufs'.
   * Allocate multiple 64kB blocks if needed (would involve messy
   * 16-bit coding in asmpkt.nas).
   */
  int size = sizeof (*_pkt_inf);

#if (DOSX & DOS4GW) && !defined(USE_FAST_PKT)
  DWORD seg;

  WATT_ASSERT (size < USHRT_MAX); /* max alloc from DOS is 64k */

  seg = dpmi_real_malloc (size, &pkt_inf_sel);

  asmpkt_inf = (struct pkt_info*) (seg << 16); /* run-time location */
  _pkt_inf   = (struct pkt_info*) (seg << 4);

#else
  _pkt_inf = malloc (size);
#endif

  if (!_pkt_inf)   /* Allocation failed */
     return (0);

  memset (_pkt_inf, 0, size);   /* Clear area */

#if defined(USE_FAST_PKT)
  rm_base = setup_pkt_inf_fast();
  if (!rm_base)
     return (0);

#else
  pktq_init (&_pkt_inf->pkt_queue,
             sizeof(_pkt_inf->rx_buf[0]),  /* RX_SIZE */
             DIM(_pkt_inf->rx_buf),        /* RX_BUFS */
             (char*)&_pkt_inf->rx_buf);
#endif

#if (DOSX & DOS4GW) && !defined(USE_FAST_PKT)
  _pkt_inf->pkt_queue.dos_ofs = offsetof (struct pkt_info, rx_buf[0]);
#elif (DOSX & DJGPP)
  _pkt_inf->dos_ds = _dos_ds;
#endif

#if (DOSX & DJGPP)
  if (pkt_use_near)
  {
    if (!(_crt0_startup_flags & _CRT0_FLAG_NEARPTR))
       TRACE_CONSOLE (1, "Near-pointers not enabled\n");
    else
    {
      _pkt_inf->use_near_ptr = TRUE;
      TRACE_CONSOLE (1, "Near-pointers enabled\n");
    }
  }
  else if (_crt0_startup_flags & _CRT0_FLAG_NEARPTR)
       TRACE_CONSOLE (1, "Near-pointers on, but \"PKT.NEAR_PTR = 0\"\n");
#endif

  return (1);
}


/**
 * Return PKTDRVR data at seg:ofs, Copy to 'dest'.
 */
static void get_rmode_data (void *dest, unsigned size, WORD seg, WORD ofs)
{
#if (DOSX & (PHARLAP|X32VM|POWERPAK))
  REALPTR rp;
  RP_SET (rp, ofs, seg);
  ReadRealMem (dest, rp, size);

#elif (DOSX & DOS4GW)
  memcpy (dest, SEG_OFS_TO_LIN(seg,ofs), size);

#elif (DOSX & DJGPP)
  dosmemget (ofs + (seg << 4), size, dest);

#else
  _fmemcpy (dest, MK_FP(seg,ofs), size);
#endif
}


/**
 * The API entry to the network link-driver. Use either protected mode
 * interface via a call to dynamically loaded module (not yet) or issue
 * an interrupt for the real-mode PKTDRVR.
 *
 * Returns TRUE if CARRY is clear, otherwise set '_pkt_errno' from DH
 * register and return FALSE.
 */
static BOOL pkt_api_entry (IREGS *reg, unsigned line)
{
  _pkt_errno = 0;
  reg->r_flags = 0;

#if (DOSX)
  /** \todo Use 32-bit API; accessing card via pmode driver
   */
  if (_pkt32_drvr)
  {
    if (!(*_pkt32_drvr)(reg))   /* call the pmode interface */
    {
      /* _pkt_errno should be set above */
      reg->r_flags |= CARRY_BIT;
      return (FALSE);
    }
    return (TRUE);
  }
#endif

  if (!pkt_interrupt)
  {
#if defined(USE_DEBUG)
    fprintf (stderr, "%s (%d): API called after deinit.\n", __FILE__, line);
#endif
    reg->r_flags |= CARRY_BIT;
    ARGSUSED (line);
    _pkt_errno = PDERR_BAD_COMMAND;
    return (FALSE);
  }

  /* Use the 16-bit real-mode PKTDRVR API.
   */
  GEN_INTERRUPT (pkt_interrupt, reg);

  if (reg->r_flags & CARRY_BIT)
  {
    _pkt_errno = hiBYTE (reg->r_dx);  /* DH has error-code */
    return (FALSE);
  }
  return (TRUE);
}


/**
 * Sets the receive mode of the interface.
 *
 * mode is one of the following modes:
 *  - 1 - turn off receiver
 *  - 2 - receive only packets sent to this interface
 *  - 3 - mode 2 plus broadcast packets (default)
 *  - 4 - mode 3 plus limited multicast packets
 *  - 5 - mode 3 plus all multicast packets
 *  - 6 - all packets (AKA promiscuous mode)
 *
 * \retval 0 - upon error - _pkt_errno is set
 * \retval 1 - if the mode was set successfully
 */
int pkt_set_rcv_mode (int mode)
{
  IREGS regs;

  if (!_pkt_inf)
  {
    _pkt_errno = PDERR_BAD_HANDLE;
    return (0);
  }

  regs.r_ax = PD_SET_RCV;
  regs.r_bx = _pkt_inf->handle;
  regs.r_cx = mode;

  /* This needs an Extended driver. SLIP/PPP is point-to-point.
   */
  if (_pktdevlevel < 2 || _pktserial)
  {
    _pkt_errno = PDERR_CANT_SET;
    return (0);
  }

  if (!PKT_API(&regs))
  {
    PKT_ERR ("Error setting receiver mode: ", _pkt_errno);
    return (0);
  }
  _pkt_rxmode = mode;
  return (1);
}

/**
 * Gets the receive mode of the interface (can never be mode 0).
 *  \retval !0 Okay  - _pkt_errno is 0, _pkt_rxmode and retval is current mode.
 *  \retval 0  Error - _pkt_errno is set.
 *
 *  _pkt_rxmode is one of the following modes: (upon successful return)
 *   - 1 - turn off receiver
 *   - 2 - receive only packets sent to this interface
 *   - 3 - mode 2 plus broadcast packets (default)
 *   - 4 - mode 3 plus limited multicast packets
 *   - 5 - mode 3 plus all multicast packets
 *   - 6 - all packets (a.k.a promiscuous mode)
 */
int pkt_get_rcv_mode (void)
{
  IREGS regs;

  ASSERT_PKT_INF (-1);

  regs.r_ax = PD_GET_RCV;
  regs.r_bx = _pkt_inf->handle;

  /* This needs an Extended driver (not SLIP/PPP)
   */
  if (_pktdevlevel < 2 || _pktserial)
  {
    _pkt_errno = PDERR_BAD_COMMAND;
    return (0);
  }

  if (!PKT_API(&regs))
  {
    PKT_ERR ("Error getting receive mode: ", _pkt_errno);
    return (0);
  }
  _pkt_rxmode = regs.r_ax;
  return (regs.r_ax);
}


#if defined(USE_MULTICAST)
/**
 * Gets the current list of multicast addresses from the PKTDRVR.
 *
 * \arg listbuf is the buffer into which the list is placed
 * \arg len     on input:  length of listbuf
 *              on output: length of list returned
 *
 * \retval 0   upon error - _pkt_errno is set
 * \retval 1  if retrieval was successful,
 */
int pkt_get_multicast_list (mac_address *listbuf, int *lenp)
{
  IREGS regs;
  int   len = *lenp;

  ASSERT_PKT_INF (0);

  regs.r_ax  = PD_GET_MULTI;
  _pkt_errno = PDERR_NO_MULTICAST;

  /* Basic drivers don't support multicast
   */
  if (_pktdevlevel < 2 || _pktserial ||
      (got_params && pkt_params.multicast_avail == 0) ||
      !PKT_API(&regs))
  {
    if (debug_on > 0)
       PKT_ERR ("Error getting multicast list: ", _pkt_errno);
    return (0);
  }

  if ((WORD)regs.r_cx == 0)  /* no MC addresses, okay */
  {
    memset (listbuf, 0, len);
    *lenp = 0;
    return (1);
  }

  /* move it into the caller's buffer
   */
  DISABLE();
  len = min (len, (WORD)regs.r_cx);
  get_rmode_data (listbuf, len, regs.r_es, regs.r_di);
  *lenp = len;
  ENABLE();
  return (1);
}

/**
 * Sets the list of multicast addresses for which the PKTDRVR is responsible.
 *
 * \arg listbuf is the buffer containing the list
 * \arg len     is the length of listbuf
 *
 * \retval  0  upon error - _pkt_errno is set
 * \retval  1  if set was successful
 */
int pkt_set_multicast_list (const void *listbuf, int len)
{
  IREGS regs;
  WORD  seg, ofs;

  ASSERT_PKT_INF (0);

  /* Basic drivers don't support multicast
   */
  if (_pktdevlevel < 2 || _pktserial ||
      (got_params && pkt_params.multicast_avail == 0)) /* no point in trying */
  {
    _pkt_errno = PDERR_NO_MULTICAST;
    return (0);
  }

#if (DOSX)
  if (len > PKT_TMP_SIZE)
  {
    _pkt_errno = PDERR_NO_SPACE;
    return (0);
  }
#endif

  DISABLE();

#if (DOSX & (PHARLAP|X32VM|POWERPAK))
  seg = RP_SEGM();
  ofs = PKT_TMP();
  WriteRealMem (rm_base + PKT_TMP(), (void*)listbuf, len);

#elif (DOSX & DOS4GW)
  seg = RP_SEGM();
  ofs = PKT_TMP();
  memcpy ((void*)(rm_base + PKT_TMP()), listbuf, len);

#elif (DOSX & DJGPP)
  seg = RP_SEGM();
  ofs = PKT_TMP();
  DOSMEMPUT (listbuf, len, rm_base + PKT_TMP());

#else
  seg = FP_SEG (listbuf);
  ofs = FP_OFF (listbuf);
#endif

  ENABLE();

  regs.r_ax = PD_SET_MULTI;
  regs.r_cx = len;
  regs.r_es = seg;
  regs.r_di = ofs;

  if (!PKT_API(&regs))
  {
    if (debug_on > 0)
       PKT_ERR ("Error setting multicast list: ", _pkt_errno);
    return (0);
  }
  return (1);
}
#endif /* USE_MULTICAST */
#endif /* __MSDOS__ */

