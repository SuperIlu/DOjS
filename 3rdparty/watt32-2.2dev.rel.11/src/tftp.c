/*!\file tftp.c
 * TFTP client.
 *
 * Boot-ROM-Code to load (any file or) an operating system across
 * a TCP/IP network.
 *
 * Module:  tftp.c
 * Purpose: Get a file with TFTP protocol
 * Entries: tftp_boot_load, tftp_set_server, tftp_set_boot_fname
 *
 **************************************************************************
 *
 * Copyright (C) 1995,1996,1997 Gero Kuhlmann <gero@gkminix.han.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Major changes for Watt-32 by G. Vanem <gvanem@yahoo.no> 1999
 *
 *  This client adheres to RFC-1350 (TFTP v2), but supports only
 *  reading from a remote host.
 *
 *  \todo Support option negotiation and ETFTP (RFC-1986)
 */

#include "socket.h"
#include "pcdns.h"
#include "run.h"
#include "tftp.h"

#if defined(USE_TFTP)

#if defined(USE_DEBUG)
  #define TRACE(x)  (*_printf) x
#else
  #define TRACE(x)  ((void)0)
#endif


/*
 * Error codes private to this file:
 */
#define ERR_INV       1       /* invalid packet size */
#define ERR_ERR       2       /* error packet received */
#define ERR_OP        3       /* invalid opcode */
#define ERR_BLOCK     4       /* invalid block number */
#define ERR_TIMEOUT   5       /* timeout while receiving */
#define ERR_UNKNOWN   6       /* unknown error */

/*
 * Various definitions:
 */
#define TFTP_RETRY      5          /* Maximum number of retries */
#define TFTP_TIMEOUT    8          /* Default 8 seconds timeout */
#define TFTP_HEADSIZE   4          /* th_opcode/th_block size */
#define TFTP_PORT_LOW   1024       /* lowest legal local port */
#define TFTP_PORT_HIGH  USHRT_MAX  /* higest legal local port */
#define OCTET_STR       "octet"    /* name for 8-bit raw format */
#define NETASCII_STR    "netascii" /* name for netascii format */
#define MAIL_STR        "mail"     /* name for mail format */

/*
 * Public data
 */
int (W32_CALL *_tftp_write) (const void*, size_t) = NULL;
int (W32_CALL *_tftp_close) (void)                = NULL;

/*
 * Local variables
 */
static struct tftphdr *inbuf;    /* TFTP input buffer  */
static struct tftphdr *outbuf;   /* TFTP output buffer */
static sock_type      *sock;     /* Socket for UDP recv/xmit */

static DWORD currblock;          /* Current data block           */
static int   blocksize;          /* Server's block size          */
static int   ibuflen;            /* Size of data in input buffer */
static int   isopen;             /* TRUE if connection is open   */

static int   tftp_errno   = 0;
static DWORD tftp_server  = 0;
static int   tftp_timeout = TFTP_TIMEOUT;
static int   tftp_retry   = TFTP_RETRY;
static int   tftp_lport   = 0;

static char  tftp_server_name[MAX_HOSTLEN]  = "";
static char  tftp_xfer_mode  [MAX_VALUELEN] = OCTET_STR;
static char *tftp_boot_remote_file = NULL;
static char *tftp_boot_local_file  = NULL;
static char *tftp_openmode         = NULL;

/*
 * Send a tftp request packet
 */
static void send_req (char request, const char *fname)
{
  char *cp, *mode;
  int   len;
  int   fnamlen = strlen (fname);

  /* The output buffer is setup with the request code, the file name,
   * and the name of the data format.
   */
  memset (outbuf, 0, sizeof(*outbuf));
  outbuf->th_opcode = intel16 (request);
  len = SEGSIZE - sizeof(outbuf->th_opcode) - strlen(tftp_xfer_mode) - 1;
  cp  = (char*) &outbuf->th_stuff[0];

  for ( ; *fname && len > 0 && fnamlen > 0; len--, fnamlen--)
      *cp++ = *fname++;
  *cp++ = '\0';

  for (mode = tftp_xfer_mode; *mode; )
      *cp++ = *mode++;
  *cp++ = '\0';

  /* Finally send the request
   */
  len = (int) (cp - (char*)outbuf);
  sock_fastwrite (sock, (BYTE*)outbuf, len);
}


/*
 * Send a tftp acknowledge packet
 */
static void send_ack (WORD block)
{
  struct tftphdr ack;

  ack.th_opcode = intel16 (ACK);
  ack.th_block  = intel16 (block);
  sock_fastwrite (sock, (BYTE*)&ack, TFTP_HEADSIZE);
}

#if defined(USE_DEBUG)
/*
 * Return error string for 'th_code'
 */
static const char *tftp_strerror (int code)
{
  static const char *err_tab[] = {
                    "EUNDEF",
                    "ENOTFOUND",
                    "EACCESS",
                    "ENOSPACE",
                    "EBADOP",
                    "EBADID",
                    "EEXISTS",
                    "ENOUSER",
                    "EOPTNEG"
                  };
  if (code < 0 || code >= DIM(err_tab))
     return ("?");
  return (err_tab[code]);
}
#endif


/*
 * Watch out for "ICMP port unreachable".
 */
static void udp_callback (_udp_Socket *s, int icmp_type, int icmp_code)
{
  if (s->ip_type == UDP_PROTO && s == (_udp_Socket*)sock &&
      (s->locflags & LF_GOT_ICMP) && icmp_type == ICMP_UNREACH)
  {
    /* In lack of a better way, pretend we got a FIN.
     * This causes sock_wait_input() below to break it's loop.
     */
    s->locflags |= LF_GOT_FIN;
    s->err_msg = icmp_type_str [ICMP_UNREACH];
  }
  ARGSUSED (icmp_code);
}

/*
 * Receive a TFTP data packet
 */
static int recv_packet (DWORD block)
{
  int len, status = 0;

  /* Use a callback since first block sent might cause a "ICMP
   * port unreachable" to be sent back. Note that the normal mechanism
   * of detecting ICMP errors (through _udp_cancel) doesn't work since
   * we did set 'sock->udp.hisaddr = 0'.
   *
   * Note: 'block' is 32-bit, but 16-bit in tftp-header.
   *       We allow the header block-counter to wrap (allowing > 32MB files).
   */
  if (block == 1UL)
       sock->udp.icmp_callb = (icmp_upcall) udp_callback;
  else sock->udp.icmp_callb = NULL;

  /* Read packet with timeout
   */
  sock_wait_input (sock, tftp_timeout, NULL, &status);

  len = sock_fastread (sock, (BYTE*)inbuf, TFTP_HEADSIZE+SEGSIZE);

  /* Check that the packet has a correct length
   */
  len -= TFTP_HEADSIZE;
  if (len < 0 || len > SEGSIZE)
  {
    TRACE (("tftp: Invalid packet, len = %d\n", len));
    tftp_errno = ERR_INV;
    return (-1);
  }

  /* Check if we got an error packet
   */
  if (intel16(inbuf->th_opcode) == ERROR)
  {
#if defined(USE_DEBUG)
    int   code = intel16 (inbuf->th_code);
    const char *str = tftp_strerror (code);

    TRACE (("tftp: Error: %s (%d): %.*s\n",
            str, code, SEGSIZE, inbuf->th_data));
#endif
    tftp_errno = ERR_ERR;
    return (-1);
  }

  /* Check if we got a valid data packet at all
   */
  if (intel16(inbuf->th_opcode) != DATA)
  {
    TRACE (("tftp: Invalid opcode %d\n", intel16(inbuf->th_opcode)));
    tftp_errno = ERR_OP;
    return (-1);
  }

  /* Check if the block number of the data packet is correct
   */
  if (intel16(inbuf->th_block) != (WORD)block)
  {
    TRACE (("tftp: Block %u != %u\n", intel16(inbuf->th_block), (WORD)block));
    tftp_errno = ERR_BLOCK;
    return (-1);
  }

  tftp_errno = 0;
  if (debug_on)
     (*_outch) ('#');  /* Write 1 hash-mark per block */

  return (len);

sock_err:
  if (status == -1)
  {
    if (debug_on)
       (*_outch) ('T');

    tftp_errno = ERR_TIMEOUT;
    return (-1);
  }

  /* most likely "Port unreachable"
   */
  TRACE (("tftp: %s\n", sockerr(sock)));
  tftp_errno = ERR_UNKNOWN;
  return (-1);
}


/*
 * Open a TFTP connection on a random local port (our transaction ID).
 * Send the request, wait for first data block and send the first ACK.
 */
static int tftp_open (DWORD server, const char *fname)
{
  int  retry;
  WORD port = 69;

#if defined(USE_BSD_API)
  struct servent *sp = getservbyname ("tftp", "udp");

  if (sp)
     port = intel16 ((WORD)sp->s_port);
#endif

  currblock = 0UL;
  blocksize = 0;

  for (retry = 0; retry < tftp_retry; retry++)
  {
    WORD our_tid;  /* our transaction ID (local port) */

    if (tftp_lport && tftp_lport < TFTP_PORT_LOW)
       outsnl (_LANG("tftp: Illegal local port."));

    if (tftp_lport >= TFTP_PORT_LOW)
         our_tid = tftp_lport;
    else our_tid = Random (TFTP_PORT_LOW, TFTP_PORT_HIGH);

    /* Try to open a TFTP connection to the server
     */
    if (!udp_open(&sock->udp, our_tid, server, port, NULL))
    {
      TRACE (("tftp: %s\n", sockerr(sock)));
      return (0);
    }
    sock->udp.locflags |= LF_NOCLOSE;  /* don't close socket on timeout */

    /* Send the file request block, and then wait for the first data
     * block. If there is no response to the query, retry it with
     * another transaction ID (local port), so that all old packets get
     * discarded automatically.
     */
    send_req (RRQ, fname);

    /* This hack makes it work because the response is sent back on
     * a source-port different from port 69; i.e. the server TID
     * uses a random port. Force the response packet to match a passive
     * socket in udp_handler().
     */
    sock->udp.hisaddr = 0;

    ibuflen = recv_packet (1);
    if (ibuflen >= 0)
    {
      blocksize = ibuflen;
      isopen = TRUE;
      send_ack (1);
      return (1);
    }

    /* If an error (except timeout) occurred, retries are useless
     */
    if (tftp_errno == ERR_ERR || tftp_errno == ERR_UNKNOWN)
       break;
  }
  return (0);
}

/*
 * Close the TFTP connection
 */
static void tftp_close (void)
{
  if (_tftp_close)
    (*_tftp_close)();

  if (debug_on)
     outs ("\n");

  if (sock)
  {
    sock_close (sock);
    DO_FREE (sock);
  }
  DO_FREE (inbuf);
  DO_FREE (outbuf);
}

/*
 * Set the name of TFTP server
 */
char *tftp_set_server (const char *name, int len)
{
  len = min (len+1, SIZEOF(tftp_server_name));
  return _strlcpy (tftp_server_name, name, len);
}

/*
 * Set the name of remote/local file to load from TFTP server.
 * Format is "tftp.boot_file = remote [local].
 * Note: `remote' name cannot contain spaces.
 */
char *tftp_set_boot_fname (const char *name, int len)
{
  char *p, buf [MAX_PATHLEN];

  len = min (len+1, SIZEOF(buf));
  _strlcpy (buf, name, len);
  tftp_boot_remote_file = strdup (buf);
  tftp_boot_local_file  = tftp_boot_remote_file;

  if (tftp_boot_local_file)
  {
    p = strchr (tftp_boot_local_file, ' ');
    if (p)
    {
      *p++ = '\0';
      tftp_boot_local_file = p;
    }
  }
  return (tftp_boot_remote_file);
}

/*
 * Set the mode used for transfer
 */
static char *tftp_set_xfer_mode (const char *name)
{
  return _strlcpy (tftp_xfer_mode, name, sizeof(tftp_xfer_mode));
}

/*
 * Read the next data packet from a TFTP connection
 */
static int tftp_get_block (const char **buf)
{
  int retry;

  /* Don't do anything if no TFTP connection is active.
   */
  if (!isopen)
     return (0);

  /* If the block number is 0 then we are still dealing with the first
   * data block after opening a connection. If the data size is smaller
   * than 'blocksize' just close the connection again.
   */
  if (currblock == 0UL)
  {
    currblock++;
    if (ibuflen < blocksize)
       isopen = FALSE;
    *buf = (const char*) &inbuf->th_data[0];
    return (ibuflen);
  }

  /* Wait for the next data packet. If no data packet is coming in,
   * resend the ACK for the last packet to restart the sender. Maybe
   * he didn't get our first ACK.
   */
  for (retry = 0; retry < tftp_retry; retry++)
  {
    ibuflen = recv_packet (currblock+1);
    if (ibuflen >= 0)
    {
      currblock++;
      send_ack ((WORD)currblock);
      if (ibuflen < blocksize)  /* last block received */
         isopen = FALSE;
      *buf = (const char*) &inbuf->th_data[0];
      return (ibuflen);
    }
    if (tftp_errno == ERR_ERR || tftp_errno == ERR_UNKNOWN)
       break;

    send_ack ((WORD)currblock);
  }
  isopen = FALSE;
  return (-1);
}

/*
 * Load the BOOT-file from TFTP server
 */
int tftp_boot_load (void)
{
  int rc = 0;

  /* Allocate socket and buffers
   */
  sock   = malloc (sizeof(sock->udp));
  inbuf  = malloc (TFTP_HEADSIZE+SEGSIZE);
  outbuf = malloc (TFTP_HEADSIZE+SEGSIZE);

  if (!sock || !inbuf || !outbuf)
  {
    outsnl (_LANG("No memory for TFTP boot."));
    return (0);
  }

  if (!tftp_boot_remote_file)
  {
    outsnl (_LANG("No remote TFTP boot filename defined."));
    return (0);
  }

  if (tftp_server_name[0] && !tftp_server)
     tftp_server = resolve (tftp_server_name);

  if (!tftp_server)
  {
    outsnl (_LANG("Cannot resolve TFTP-server "));
    return (0);
  }

  if (debug_on)
     outs (_LANG("Doing TFTP boot load..."));

  /* Open connection and request file
   */
  if (!tftp_open(tftp_server, tftp_boot_remote_file))
  {
    tftp_close();
    return (0);
  }

  while (1)
  {
    const char *buf;
    int   size = tftp_get_block (&buf);

    if (size < 0)  /* error in transfer */
    {
      rc = 0;
      break;
    }
    if (size > 0 && (*_tftp_write)(buf,size) < 0)
    {
      rc = -1;     /* writer failed, errno set */
      break;
    }
    if (size < blocksize)    /* got last block */
    {
      rc = 1;
      break;
    }
  }
  tftp_close();
  return (rc);
}

/*
 * Config-file handler for TFTP-client
 */
static void (W32_CALL *prev_hook) (const char*, const char*) = NULL;

static void W32_CALL tftp_cfg_hook (const char *name, const char *value)
{
  static const struct config_table tftp_cfg[] = {
            { "BOOT_FILE", ARG_FUNC,    (void*)tftp_set_boot_fname },
            { "SERVER",    ARG_RESOLVE, (void*)&tftp_server        },
            { "TIMEOUT",   ARG_ATOI,    (void*)&tftp_timeout       },
            { "RETRY",     ARG_ATOI,    (void*)&tftp_retry         },
            { "MODE",      ARG_FUNC,    (void*)tftp_set_xfer_mode  },
            { "OPENMODE",  ARG_STRDUP,  (void*)&tftp_openmode      },
            { "PORT",      ARG_ATOI,    (void*)&tftp_lport         },
            { NULL,        0,           NULL                       }
          };
  if (!parse_config_table(tftp_cfg, "TFTP.", name, value) && prev_hook)
     (*prev_hook) (name, value);
}

/**
 * Free allocated memory.
 */
static void W32_CALL tftp_exit (void)
{
  if (!_watt_fatal_error)
  {
    DO_FREE (tftp_boot_remote_file);
    DO_FREE (tftp_openmode);
  }
}

/**
 * Initialize config-hook for TFTP protocol.
 */
int tftp_init (void)
{
  prev_hook = usr_init;
  usr_init  = tftp_cfg_hook;
  RUNDOWN_ADD (tftp_exit, 261);
  return (TRUE);
}

/*
 * A small test program, for djgpp/Watcom only
 */
#if defined(TEST_PROG)

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "netaddr.h"
#include "pcdbug.h"
#include "pcarp.h"

static FILE  *file;
static char  *fname;
static DWORD  tot_size;
static time_t start;

static int W32_CALL close_func (void)
{
  if (file && fname)
  {
    time_t now = time (NULL);
    double  speed;

    if (now == start)
       now++;
    speed = (double)tot_size / (double)(now - start);
    fprintf (stderr, "closing `%s' (%.2f kB/s)\n", fname, speed/1024.0);
    fclose (file);
    file = NULL;
    return (1);
  }
  return (0);
}

static int W32_CALL write_func (const void *buf, size_t length)
{
  static DWORD block = 0;

  if (debug_on < 2)
      debug_on = 2;

  if (block == 0)
  {
    tot_size = 0UL;
    start = time (NULL);
    fname = tftp_boot_local_file;
    fprintf (stderr, "opening `%s'\n", fname);
    file = fopen (fname, tftp_openmode ? tftp_openmode : "wb");
  }
  if (!file)
  {
    perror (fname);
    return (-1);
  }

#if defined(__DJGPP__) && 0
  /*
   * Look for optional Watt-32 stubinfo in block 4.
   * If .exe file isn't newer, kill the connection
   */
  if (block == 4 && is_exe && check_timestamp(buf) < our_timestamp)
  {
    close_func();
    return (-1);
  }
#endif

  if (fwrite (buf, 1, length, file) < length)
     return (-1);
  tot_size += length;
  block++;
  return (0);
}


void usage (char *argv0)
{
  printf ("Usage: %s [[-d] [-n] [-a] [-h host] [-f file]\n"
          "\t\t [-i ip] [-m mask]] [-t timeout] [-r retry]\n"
          "\t -d enable WATTCP.DBG file\n"
          "\t -n run with no config file\n"
          "\t -a add random MAC address for tftp host\n"
          "\t -h specify tftp host\n"
          "\t -f specify remote file to load\n"
          "\t -i specify ip-address      (default 192.168.0.1)\n"
          "\t -m specify network mask    (default 255.255.0.0)\n"
          "\t -t specify total timeout   (default %d)\n"
          "\t -r specify retry count     (default %d)\n",
          argv0, tftp_timeout, tftp_retry);
  exit (-1);
}

int main (int argc, char **argv)
{
  eth_address eth = { 1,2,3,4,5,6 };
  int a_flag = 0;
  int h_flag = 0;
  int i_flag = 0;
  int f_flag = 0;
  int n_flag = 0;
  int m_flag = 0;
  int d_flag = 0;
  int ch;

  while ((ch = getopt(argc, argv, "adn?h:i:f:m:t:r:")) != EOF)
     switch (ch)
     {
       case 'a':
            a_flag = 1;
            break;
       case 'd':
            d_flag = 1;
            break;
       case 'n':
            n_flag = 1;
            break;
       case 'h':
            h_flag = 1;
            tftp_server = aton (optarg);
            break;
       case 'i':
            i_flag = 1;
            my_ip_addr = aton (optarg);
            break;
       case 'f':
            f_flag = 1;
            tftp_set_boot_fname (optarg, strlen(optarg));
            break;
       case 'm':
            m_flag = 1;
            sin_mask = aton (optarg);
            break;
       case 't':
            tftp_timeout = atoi (optarg);
            break;
       case 'r':
            tftp_retry = atoi (optarg);
            break;
       case '?':
       default:
            usage (argv[0]);
     }

  if (n_flag)  /* Demonstrate running with no config file */
  {
    _watt_no_config = 1;
    dbg_mode_all    = 1;
    dbg_print_stat  = 1;
    debug_on = 3;

    if (!m_flag)
       sin_mask = aton ("255.255.0.0");
    if (!i_flag)
       my_ip_addr = aton ("192.168.0.1");
    if (!h_flag)
       tftp_server = aton ("192.168.0.2");
    if (!f_flag)
       tftp_set_boot_fname ("test.fil", 8);
    if (a_flag)
       _arp_cache_add (tftp_server, (const eth_address*)&eth, FALSE);
  }
  else if (m_flag || i_flag || h_flag || f_flag || a_flag)
  {
    puts ("This option requires the `-n' flag");
    return (-1);
  }

  if (d_flag)
     dbug_init();

  if (n_flag)
     dbug_open();

  /* Must set our hook first
   */
  _tftp_write = write_func;
  _tftp_close = close_func;

  sock_init();

#ifdef WIN32
  Sleep (1000);        /* drain network buffers */
#else
  sleep (1);
#endif
  tcp_tick (NULL);
  return (0);
}
#endif /* TEST_PROG */
#endif /* USE_TFTP */
