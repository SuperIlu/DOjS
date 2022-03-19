/*!\file ip4_frag.c
 * IP4 de-fragmenter.
 */

/*
 *  Packet de-fragmentation code for WATTCP
 *  Written by and COPYRIGHT (c)1993 to Quentin Smart
 *                               smart@actrix.gen.nz
 *  all rights reserved.
 *
 *    This software is distributed in the hope that it will be useful,
 *    but without any warranty; without even the implied warranty of
 *    merchantability or fitness for a particular purpose.
 *
 *    You may freely distribute this source code, but if distributed for
 *    financial gain then only executables derived from the source may be
 *    sold.
 *
 *  Murf = Murf@perftech.com
 *  other fragfix = mdurkin@tsoft.net
 *
 *  Based on RFC815
 *
 *  Code used to use pktbuf[] as reassembly buffer. It now allocates
 *  a "bucket" dynamically at startup. There are 'MAX_IP_FRAGS' buckets
 *  to handle at the same time.
 *
 *  G.Vanem 1998 <gvanem@yahoo.no>
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <arpa/inet.h>

#include "wattcp.h"
#include "strings.h"
#include "language.h"
#include "misc.h"
#include "timer.h"
#include "chksum.h"
#include "pcconfig.h"
#include "pcqueue.h"
#include "pcstat.h"
#include "pcpkt.h"
#include "pcicmp.h"
#include "pctcp.h"
#include "pcdbug.h"
#include "netaddr.h"
#include "run.h"
#include "ip4_in.h"
#include "ip4_out.h"
#include "ip4_frag.h"

#define MAX_IP_FRAGS     2   /* max # of fragmented IP-packets */
#define MAX_IP_HOLDTIME  15  /* time (in sec) to hold before discarding */

int _ip4_frag_reasm = MAX_IP_HOLDTIME;  /* configurable; pcconfig.c */

#if defined(USE_FRAGMENTS)

#undef TRACE_MSG

#if defined(TEST_PROG)
  #define TRACE_MSG(color, args)                     \
          do {                                       \
            SET_ATTR (color);                        \
            printf ("%s(%u): ", __FILE__, __LINE__); \
            printf args ;                            \
            NORM_TEXT();                             \
          } while (0)

#else
  #define TRACE_MSG(color, args)  TCP_TRACE_MSG (args)
#endif

#if 0
  #undef  MAX_FRAGMENTS
  #define MAX_FRAGMENTS   20U   /* !! test */
  #define MAX_FRAG_SIZE   (MAX_FRAGMENTS * MAX_IP4_DATA) /* 29600 */
#endif

#define BUCKET_MARKER   0xDEAFABBA

/*
 * Number of 'fd_set' required to hold MAX_SOCKETS.
 */
#define NUM_FD_SETS  ((MAX_FRAG_SIZE+sizeof(fd_set)-1) / sizeof(fd_set))

typedef fd_set  used_map [NUM_FD_SETS];

struct huge_ip {
       in_Header hdr;
       BYTE      data [MAX_FRAG_SIZE];   /* 66600 for DOSX */
       DWORD     marker;
     };

struct frag_key {
       DWORD  source;
       DWORD  destin;
       WORD   ident;
       BYTE   proto;
     };

struct frag_bucket {
       BOOL            used;     /* this bucket is taken */
       BOOL            got_ofs0; /* we've received ofs-0 fragment */
       int             active;   /* # of active fragments */
       used_map        map;      /* map of bits received */
       mac_address     mac_src;  /* remember for icmp_send_timexceed() */
       struct frag_key key;      /* key for matching new fragments */
       struct huge_ip *ip;       /* calloc'ed on startup */
     };

struct hole {                   /* structure for missing chunks */
       struct hole *next;
       long         start;
       long         end;
     };

struct frag_ctrl {
       BOOL         used;       /* this position in table is in use */
       DWORD        timer;      /* expiry for this fragment */
       struct hole *hole_first;
       in_Header   *ip;
       BYTE        *data_offset;
     };

static struct frag_ctrl   frag_control [MAX_IP_FRAGS][MAX_FRAGMENTS];
static struct frag_bucket frag_buckets [MAX_IP_FRAGS];

static long  data_start;  /* NB! global data; not reentrant */
static long  data_end;
static long  data_length;

enum frag_bits {
     IS_FRAG  = 0x01,
     IS_LAST  = 0x02,
     IS_FIRST = 0x04
   };

/*
   frag_bucket[]:  Holds the fragments with offsets (0-MAX_FRAG_SIZE)
                   ------------------------------------------------------
   0:              |                 struct huge_ip                     |
                   ------------------------------------------------------
                   .                                                    .
                   .                                                    .
                   ------------------------------------------------------
   MAX_IP_FRAGS-1: |                                                    |
                   ------------------------------------------------------
                   ^                                                    ^
                   |                                                    |
                   0                                              MAX_FRAG_SIZE

 */

static enum frag_bits fbits;     /* IPv4 fragment bits */

#if defined(TEST_PROG)
static void dump_frag_holes (const char *where, const struct frag_ctrl *fc, const struct hole *hole)
{
  const struct hole *h;

  if (!fc)
  {
    TRACE_MSG (6, ("%s, No frag!\n", where));
    return;
  }

  TRACE_MSG (15, ("%s: fc->used: %d, fc->data_offset: %" ADDR_FMT "\n",
                  where, fc->used, ADDR_CAST(fc->data_offset) ));

  for (h = hole; h; h = h->next)
  {
    TRACE_MSG (15, ("hole: %" ADDR_FMT ", h->start: %ld, h->end: %ld\n",
                    ADDR_CAST(h), h->start, h->end));
  }
}
#endif

#if defined(USE_DEBUG)
static const char *decode_fbits (int fb)
{
  static char buf[50];
  char  *end;

  buf[0] = '\0';
  if (fb & IS_FRAG)
     strcat (buf, "IS_FRAG+");

  if (fb & IS_LAST)
     strcat (buf, "IS_LAST+");

  if (fb & IS_FIRST)
     strcat (buf, "IS_FIRST+");

  end = strrchr (buf, '+');
  if (end)
     *end = '\0';
  return (buf);
}
#endif


/*
 * Check if this packet is part of a fragment-chain. It might be
 * a last fragment (MF=0) with ofs 0. This would be normal for
 * fragments sent from Linux. Look in 'frag_buckets' for a match.
 *
 * If the 'fk' was not found, set '*free_bucket' to next free bucket
 * and return FALSE.
 *
 * If found, set '*bucket' to matching bucket and '*slot' to free
 * slot in '*bucket' and return TRUE.
 */
static BOOL match_frag (const in_Header *ip, int *slot, int *bucket, int *free_bucket)
{
  struct frag_key fk;
  int    i, b;

  *bucket = -1;
  *slot   = -1;
  *free_bucket = -1;

  fk.source = ip->source;
  fk.destin = ip->destination;
  fk.ident  = ip->identification;
  fk.proto  = ip->proto;

  for (b = 0; b < DIM(frag_buckets); b++)
  {
    const struct frag_bucket *fb = frag_buckets + b;
    const struct frag_ctrl   *fc = &frag_control[b][0];

    if (!fb->used)
    {
      if (*free_bucket == -1)   /* get 1st vacant bucket */
          *free_bucket = b;
      continue;
    }

    if (memcmp(&fk,&fb->key,sizeof(fk)))
       continue;

    for (i = 0; i < (int)MAX_FRAGMENTS; i++, fc++)
    {
      if (!fc->used)
      {
        *bucket = b;
        return (TRUE);
      }
      *slot = i;  /* This frag-control is used. Return it's index */
    }
  }
  return (FALSE);
}

/*
 * Check and report if fragment data 'ofs' and 'end' are okay
 */
static __inline BOOL check_frag_ofs (const in_Header *ip, DWORD ofs, DWORD end)
{
  if (ofs + end <= MAX_FRAG_SIZE &&   /* fragment offset okay, < 65528 */
      ofs + end <= USHRT_MAX-8)       /* must not wrap around 64kB */
     return (TRUE);

  TCP_CONSOLE_MSG (2, (_LANG("Bad frag-ofs: %lu, frag-end: %lu, ip-prot %u (%s -> %s)\n"),
                   ofs, end, ip->proto,
                   _inet_ntoa(NULL,intel(ip->source)),
                   _inet_ntoa(NULL,intel(ip->destination))));
  ARGSUSED (ip);
  return (FALSE);
}

static __inline void set_fbits (DWORD offset, WORD flags)
{
  fbits = 0;

  if (flags & IP_MF)
  {
    fbits |= IS_FRAG;
    if (offset == 0)
       fbits |= IS_FIRST;
  }
  else if (offset)
          fbits = (IS_FRAG | IS_LAST);
}

/*
 * Prepare and setup for reassembly
 */
static BOOL setup_first_frag (const in_Header *ip, int idx)
{
  struct frag_ctrl   *fc;
  struct frag_bucket *fb  = frag_buckets + idx; /* do fragment hanlding in this bucket */
  struct huge_ip     *hip = fb->ip;
  struct hole        *hole;
  unsigned            i;

  /* Marker destroyed!
   */
  if (hip->marker != BUCKET_MARKER)
     TCP_CONSOLE_MSG (0, ("frag_buckets[%d] destroyed\n!", idx));

  TRACE_MSG (7, ("frag_bucket[%d] = %" ADDR_FMT "\n", idx, ADDR_CAST(hip)));

  /* Remember MAC source address
   */
  if (_pktserial)
       memset (&fb->mac_src, 0, sizeof(mac_address));
  else memcpy (&fb->mac_src, MAC_SRC(ip), sizeof(mac_address));

  WATT_ASSERT (fb->used == 0);

  fb->used     = TRUE;
  fb->got_ofs0 = FALSE;

  /* Find first empty slot
   */
  fc = &frag_control[idx][0];
  for (i = 0; i < MAX_FRAGMENTS; i++, fc++)
      if (!fc->used)
         break;

  if (i == MAX_FRAGMENTS)
     return (FALSE);

  fc->used = TRUE;      /* mark as used */
  fb->active++;         /* inc active frags counter */

  WATT_ASSERT (fb->active == 1);
  TRACE_MSG (7, ("bucket=%d, active=%u, i=%u\n", idx, fb->active, i));

  /* Setup frag header data, first packet
   */
  fb->key.proto  = ip->proto;
  fb->key.source = ip->source;
  fb->key.destin = ip->destination;
  fb->key.ident  = ip->identification;

  fc->ip    = &fb->ip->hdr;
  fc->timer = set_timeout (1000 * min(_ip4_frag_reasm, ip->ttl));

  /* Set pointers to beginning of IP packet data
   */
  fc->data_offset = (BYTE*)fc->ip + in_GetHdrLen(ip);

  /* Setup initial hole-list.
   */
  if (data_start == 0)  /* 1st fragment sent is 1st fragment received */
  {
    WORD  ip_len = intel16 (ip->length);
    BYTE *dst    = (BYTE*) fc->ip;

    ip_len = min (ip_len, _mtu);
    memcpy (dst, ip, ip_len);
    hole = (struct hole*) (dst + ip_len + 1);
    fc->hole_first = hole;
    fb->got_ofs0   = TRUE;
  }
  else
  {
    /* !!fix-me: assumes header length of this fragment is same as
     *           in reassembled IP packet (may have IP-options)
     */
    BYTE *src = (BYTE*)ip + in_GetHdrLen(ip);
    BYTE *dst = fc->data_offset + data_start;

    memcpy (dst, src, (size_t)data_length);

    /* Bracket beginning of data
     */
    hole        = fc->hole_first = (struct hole*)fc->data_offset;
    hole->start = 0;
    hole->end   = data_start - 1;

    if (!(fbits & IS_LAST))
    {
      hole->next = (struct hole*) (fc->data_offset + data_length + 1);
      hole = hole->next;
    }
    else
    {
      hole = fc->hole_first->next = NULL;
      /* Adjust length */
      fc->ip->length = intel16 ((WORD)(data_end + in_GetHdrLen(ip)));
    }
  }

  if (hole)
  {
    hole->start = data_length;
    hole->end   = MAX_FRAG_SIZE;
    hole->next  = NULL;

    TRACE_MSG (7, ("hole %" ADDR_FMT ", hole->start %lu, hole->end %lu\n",
                   ADDR_CAST(hole), hole->start, hole->end));
  }

#if defined(TEST_PROG)
  dump_frag_holes ("setup_first_frag", fc, hole);
#endif

  return (TRUE);
}

/*
 * ip4_defragment() is called if '*ip_ptr' is part of a new or existing
 * fragment chain.
 *
 * IP header already checked in _ip4_handler().
 * Return the reassembled segment (ICMP, UDP or TCP) in '*ip_ptr'.
 * We assume MAC-header is the same on all fragments.
 * We return a packet with the MAC-header of the first
 * fragment received.
 */
int ip4_defragment (const in_Header **ip_ptr, DWORD offset, WORD flags)
{
  struct frag_ctrl   *fc        = NULL;
  struct frag_bucket *fb        = NULL;
  struct hole        *hole      = NULL;
  struct hole        *prev_hole = NULL;
  const in_Header    *ip        = *ip_ptr;

  BOOL  found    = FALSE;
  BOOL  got_hole = FALSE;
  int   slot, bucket, free_bucket;

  /* Not a fragment (or part of a chain) if offset is 0 and !IP_MF.
   */
  if (offset == 0 && !(flags & IP_MF))
     return (1);

  set_fbits (offset, flags);

  /* Check if part of an existing frag-chain or a new fragment
   */
  if (!match_frag(ip,&slot,&bucket,&free_bucket))
  {
    if (slot == MAX_FRAGMENTS-1)   /* found but no slots free */
    {
      STAT (ip4stats.ips_fragments++);
      goto drop_frag;
    }

#if 0
    if (!(fbits & IS_FRAG)) /* A normal non-fragmented IP-packet */
       return (1);
#endif

    /* Not in frag-list, but ofs > 0 or MF set. Must be a part of
     * a (possibly new) fragment chain.
     */
  }

  STAT (ip4stats.ips_fragments++);

  /* Calculate where data should go
   */
  data_start  = (long) offset;
  data_length = (long) intel16 (ip->length) - in_GetHdrLen (ip);
  data_end    = data_start + data_length;

  TRACE_MSG (7, ("\nip4_defrag: %s -> %s, id 0x%04X, len %ld, ofs %ld, fbits %s\n",
                 _inet_ntoa(NULL,intel(ip->source)),
                 _inet_ntoa(NULL,intel(ip->destination)),
                 intel16(ip->identification), data_length, data_start,
                 decode_fbits(fbits)));

  if ((flags & IP_MF) && data_length == 0 && offset != 0)
  {
    TRACE_MSG (7, ("No data.\n"));
    goto drop_frag;
  }

  if ((flags & IP_MF) && (data_length & 7) != 0)
  {
    TRACE_MSG (7, ("Frag-data not multiple of 8.\n"));
    goto drop_frag;
  }

  if (!check_frag_ofs(ip,data_start,data_end))
     goto drop_frag;

  found = (bucket > -1);

  if (!found)
  {
    TRACE_MSG (7, ("bucket=%d, i=%d, key not found\n", free_bucket, slot));
    bucket = free_bucket;
    fb     = frag_buckets + bucket;

    /* Can't handle any new frags, drop packet
     */
    if (bucket == -1 || fb->active == MAX_FRAGMENTS)
       goto drop_frag;

    if (!setup_first_frag (ip, bucket))   /* Setup first fragment received */
       goto drop_frag;

    /* Okay so far. Wait for next IP and continue defragmentation.
     */
    DEBUG_RX (NULL, ip);
    return (0);
  }

  fc = &frag_control [bucket][slot];
  fb = frag_buckets + bucket;

  TRACE_MSG (7, ("bucket=%d, slot=%d key found, fbits: %s, active=%d\n",
                 bucket, slot, decode_fbits(fbits), fb->active));

  if (fbits & IS_LAST)           /* Adjust length  */
     fc->ip->length = intel16 ((WORD)(data_end + in_GetHdrLen(ip)));

  if (offset == 0)
     fb->got_ofs0 = TRUE;

  hole = fc->hole_first;   /* Hole handling */

  do
  {
#if defined(TEST_PROG)
    dump_frag_holes ("ip4_defragment(1)", fc, hole);
#endif

    if (hole && (data_start <= hole->end) &&   /* We've found the spot */
        (data_end >= hole->start))
    {
      long last_end = hole->end;    /* Pick up old hole end for later */

      got_hole = TRUE;

      /* Find where to insert fragment.
       * Check if there's a hole before the new frag
       */
      if (data_start > hole->start)
      {
        hole->end = data_start - 1;
        prev_hole = hole;  /* We have a new prev */
      }
      else
      {
        /* No, delete current hole
         */
        if (!prev_hole)
             fc->hole_first  = hole->next;
        else prev_hole->next = hole->next;
      }

      /* Is there a hole after the current fragment?
       * Only if we're not last and more to come
       */
      if (data_end < hole->end)
      {
        hole = (struct hole*) (data_end + 1 + fc->data_offset);
        hole->start = data_end + 1;
        hole->end   = last_end;

        /* prev_hole = NULL if first
         */
        if (!prev_hole)
        {
          hole->next = fc->hole_first;
          fc->hole_first = hole;
        }
        else
        {
          hole->next = prev_hole->next;
          prev_hole->next = hole;
        }
      }
    }
    prev_hole = hole;
    hole = hole->next;

#if defined(TEST_PROG)
    dump_frag_holes ("ip4_defragment(2)", fc, hole);
#endif
  }
  while (hole);          /* Until we got to the end or found */


  /* Thats all setup so copy in the data
   */
  if (got_hole)
     memcpy (fc->data_offset + data_start,
             (BYTE*)ip + in_GetHdrLen(ip), (size_t)data_length);

  TRACE_MSG (7, ("got_hole %d, fc->hole_first %" ADDR_FMT "\n",
                 got_hole, ADDR_CAST(fc->hole_first)));

  if (fc->hole_first == NULL)  /* Now we have all the parts */
  {
    if (fb->active >= 1)
        fb->active--;

    /* Redo checksum as we've changed the length in the header
     */
    fc->ip->frag_ofs = 0;      /* no MF or frag-ofs */
    fc->ip->checksum = 0;
    fc->ip->checksum = ~CHECKSUM (fc->ip, sizeof(in_Header));

    STAT (ip4stats.ips_reassembled++);
    *ip_ptr = fc->ip;        /* MAC-header is in front of IP */
    return (1);
  }

  STAT (ip4stats.ips_fragdropped--);

drop_frag:
  STAT (ip4stats.ips_fragdropped++);
  DEBUG_RX (NULL, ip);
  return (0);
}

/*
 * Release a reassembled IP-packet.
 * Or mark a frag_bucket as inactive because of reassembly timeout.
 */
int ip4_free_fragment (const in_Header *ip)
{
  unsigned i, j;

  for (j = 0; j < DIM(frag_buckets); j++)
  {
    struct frag_bucket *fb  = frag_buckets + j;
    struct huge_ip     *hip = fb->ip;

    if (hip->marker != BUCKET_MARKER)
       TCP_CONSOLE_MSG (0, ("frag_buckets[%d] destroyed\n!", j));

    if (fb->used && ip == &hip->hdr)
    {
      TRACE_MSG (7, ("ip4_free_fragment(%" ADDR_FMT "), bucket=%d, active=%d\n",
                     ADDR_CAST(ip), j, fb->active));

      fb->used   = FALSE;
      fb->active = 0;
      for (i = 0; i < MAX_FRAGMENTS; i++)
          frag_control[j][i].used = FALSE;
      return (1);
    }
  }
  return (0);
}

/*
 * Check if any fragment chains has timed-out
 */
static void W32_CALL chk_timeout_frags (void)
{
  unsigned i, j;

  for (j = 0; j < DIM(frag_buckets); j++)
  {
    struct frag_bucket *fb = frag_buckets + j;
    struct frag_ctrl   *fc = &frag_control[j][0];

    TRACE_MSG (14, ("chk_timeout_frags(), bucket %u, active %d, used %d\n",
                    j, fb->active, fb->used));
    if (!fb->active)
       continue;

    for (i = 0; i < MAX_FRAGMENTS; i++, fc++)
    {
      if (!fc->used || !chk_timeout(fc->timer))
         continue;

      if (fb->got_ofs0)
      {
        struct in_Header ip;

        memset (&ip, 0, sizeof(ip));
        ip.identification = fb->key.ident;
        ip.proto          = fb->key.proto;
        ip.source         = fb->key.source;
        ip.destination    = fb->key.destin;

        /* send an ICMP_TIMXCEED (code 1)
         */
        icmp_send_timexceed (&ip, (const void*)&fb->mac_src);
      }

      STAT (ip4stats.ips_fragtimeout++);

      TRACE_MSG (14, ("chk_timeout_frags(), bucket %u, slot %d, id %04X\n",
                      j, i, intel16(fb->key.ident)));

      if (ip4_free_fragment(&fb->ip->hdr))
         return;
    }
  }
}

/*
 * Free memeory allocated at startup by ip4_frag_init().
 */
static void W32_CALL free_frag_buckets (void)
{
  int i;

  for (i = 0; i < DIM(frag_buckets); i++)
  {
    struct frag_bucket *fb  = frag_buckets + i;
    struct huge_ip     *hip = fb->ip;

    if (hip && hip->marker == BUCKET_MARKER)
    {
      hip->marker = 0;
      free (hip);
    }
  }
}

/*
 * Allocate 'frag_buckets[]::ip' for doing reassembly.
 */
void ip4_frag_init (void)
{
  int i;

  for (i = 0; i < DIM(frag_buckets); i++)
  {
    struct frag_bucket *fb  = frag_buckets + i;
    struct huge_ip     *hip = calloc (sizeof(*hip), 1);

    if (!hip)
       break;

    memset (fb, '\0', sizeof(*fb));
    fb->ip = hip;
    hip->marker = BUCKET_MARKER;
  }

  /* Add a daemon to check for IPv4-fragment time-outs.
   */
  DAEMON_ADD (chk_timeout_frags);
  RUNDOWN_ADD (free_frag_buckets, 260);
}

/*----------------------------------------------------------------------*/

#if defined(TEST_PROG)  /* a small test program */
#undef FP_OFF
#undef enable
#undef disable

#ifndef _MSC_VER
#include <unistd.h>
#endif

#ifdef WIN32
#define sleep(sec)  Sleep (1000*(sec))
#endif

#include "sock_ini.h"
#include "loopback.h"
#include "pcarp.h"

static DWORD to_host   = 0;
static WORD  frag_ofs  = 0;
static int   max_frags = 5;
static int   frag_size = 1000;
static int   rand_frag = 0;
static int   rev_order = 0;
static int   time_frag = 0;

#ifdef WIN32

/* Put this somewhere else */

static struct {
    const char *col_name;
    int         col_value;
  } colors[] = {
    { "black",        0 },
    { "blue",         FOREGROUND_BLUE                    },
    { "green",        FOREGROUND_GREEN                   },
    { "cyan",         FOREGROUND_BLUE | FOREGROUND_GREEN },
    { "red",          FOREGROUND_RED                     },
    { "magenta",      FOREGROUND_BLUE | FOREGROUND_RED   },
    { "brown",        FOREGROUND_RED  | FOREGROUND_GREEN },
    { "lightgray",    FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED },
    { "darkgray",     FOREGROUND_INTENSITY | 0 },
    { "lightblue",    FOREGROUND_INTENSITY | FOREGROUND_BLUE   },
    { "lightgreen",   FOREGROUND_INTENSITY | FOREGROUND_GREEN  },
    { "lightcyan",    FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN },
    { "lightred",     FOREGROUND_INTENSITY | FOREGROUND_RED                     },
    { "lightmagenta", FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_RED   },
    { "yellow",       FOREGROUND_INTENSITY | FOREGROUND_RED  | FOREGROUND_GREEN },
    { "white",        FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED }
  };

void test_colors (void)
{
  int i;

  for (i = 0; i < DIM(colors); i++)
  {
    SetConsoleTextAttribute (stdout_hnd, (console_info.wAttributes & ~7) | colors[i].col_value);
    printf ("Color %2d: %2d = %s\n", i, colors[i].col_value, colors[i].col_name);
    SetConsoleTextAttribute (stdout_hnd, console_info.wAttributes);
  }
}
#endif

void usage (const char *argv0)
{
  printf ("%s [-n num] [-s size] [-h ip] [-r] [-R] [-t]\n"
          "Send fragmented ICMP Echo Request (ping)\n\n"
          "options:\n"
          "  -n  number of fragments to send     (default %d)\n"
          "  -s  size of each fragment           (default %d)\n"
          "  -h  specify destination IP          (default 127.0.0.1)\n"
          "  -r  send fragments in random order  (default %s)\n"
          "  -R  send fragments in reverse order (default %s)\n"
          "  -t  simulate fragment timeout       (default %s)\n",
          argv0, max_frags, frag_size,
          rand_frag ? "yes" : "no",
          rev_order ? "yes" : "no",
          time_frag ? "yes" : "no");
  exit (0);
}

BYTE *init_frag (int argc, char **argv)
{
  int   i, ch;
  BYTE *data;

  while ((ch = getopt(argc, argv, "h:n:s:rRt?")) != EOF)
     switch (ch)
     {
       case 'h': to_host = inet_addr (optarg);
                 if (to_host == INADDR_NONE)
                 {
                   printf ("Illegal IP-address\n");
                   exit (-1);
                 }
                 break;
       case 'n': max_frags = atoi (optarg);
                 break;
       case 's': frag_size = atoi (optarg) / 8;  /* multiples of 8 */
                 frag_size <<= 3;
                 break;
       case 'r': rand_frag = 1;
                 break;
       case 'R': rev_order = 1;
                 break;
       case 't': time_frag = 1;   /** \todo Simulate fragment timeout */
                 break;
       case '?':
       default : usage (argv[0]);
     }

  if (max_frags < 1 || max_frags > FD_SETSIZE)
  {
    printf ("# of fragments is 1 - %d\n", FD_SETSIZE);
    exit (-1);
  }

  if (frag_size < 8 || frag_size > (int)MAX_IP4_DATA)
  {
    printf ("Fragsize range is 8 - %lu\n", (unsigned long)MAX_IP4_DATA);
    exit (-1);
  }

  if ((unsigned)(frag_size * max_frags) > USHRT_MAX)
  {
    printf ("Total fragsize must be < 64kB\n");
    exit (-1);
  }

  data = calloc (frag_size * max_frags, 1);
  if (!data)
  {
    printf ("no memory\n");
    exit (-1);
  }

  for (i = 0; i < max_frags; i++)
     memset (data + i*frag_size, 'a'+i, frag_size);

  loopback_mode |= LBACK_MODE_ENABLE;
  dbug_init();
  sock_init();

  if (!to_host)
     to_host = htonl (INADDR_LOOPBACK);
  return (data);
}

/*----------------------------------------------------------------------*/

int rand_packet (fd_set *fd, int max)
{
  int count = 0;

  while (1)
  {
    int i = Random (0, max);
    if (i < max && !FD_ISSET(i,fd))
    {
      FD_SET (i, fd);
      return (i);
    }
    if (++count == 10*max)
       return (-1);
  }
}

/*----------------------------------------------------------------------*/

int main (int argc, char **argv)
{
  fd_set      is_sent;
  int         i;
  eth_address eth;
  in_Header  *ip;
  ICMP_PKT   *icmp;
  WORD        frag_flag;
  BYTE       *data = init_frag (argc, argv);

  if (!_arp_resolve(ntohl(to_host), &eth))
  {
    printf ("ARP failed\n");
    return (-1);
  }

  _ip4_frag_reasm = 3;

  ip   = (in_Header*) _eth_formatpacket (&eth, IP4_TYPE);
  icmp = (ICMP_PKT*) data;

  ip->hdrlen         = sizeof(*ip)/4;
  ip->ver            = 4;
  ip->tos            = _default_tos;
  ip->identification = _get_ip4_id();
  ip->ttl            = 15;           /* max 15sec reassembly time */
  ip->proto          = ICMP_PROTO;

  icmp->echo.type       = ICMP_ECHO;
  icmp->echo.code       = 0;
  icmp->echo.identifier = 0;
  icmp->echo.index      = 1;
  icmp->echo.sequence   = set_timeout (1) & 0xFFFF;  /* "random" id */
  icmp->echo.checksum   = 0;
  icmp->echo.checksum   = ~CHECKSUM (icmp, max_frags*frag_size);

  FD_ZERO (&is_sent);

#if 0  /* test random generation */
  if (rand_frag)
     for (i = 0; i < max_frags; i++)
     {
       int j = rand_packet (&is_sent, max_frags);
       printf ("index %d\n", j);
     }
  exit (0);
#endif

  for (i = 0; i < max_frags; i++)
  {
    int j;

    if (rand_frag)
    {
      j = rand_packet (&is_sent, max_frags);
      if (j < 0)
         break;
    }
    if (rev_order)
    {
      j = max_frags - i - 1;
      frag_flag = (j > 0) ? IP_MF : 0;
    }
    else
    {
      j = i;
      frag_flag = (j == max_frags-1) ? 0 : IP_MF;
    }

    frag_ofs = (j * frag_size);
    memcpy ((BYTE*)(ip+1), data+frag_ofs, frag_size);

    /* The loopback device swaps src/dest IP; hence we must set them
     * on each iteration of the loop
     */
    ip->source      = intel (my_ip_addr);
    ip->destination = to_host;
    ip->frag_ofs    = intel16 (frag_ofs/8 | frag_flag);
    ip->length      = intel16 (frag_size + sizeof(*ip));
    ip->checksum    = 0;
    ip->checksum    = ~CHECKSUM (ip, sizeof(*ip));

    _eth_send (frag_size+sizeof(*ip), NULL, __FILE__, __LINE__);

    STAT (ip4stats.ips_ofragments++);
  }

  puts ("Calling tcp_tick(). Press a key to quit.");
  debug_on = 2;

  while (!kbhit())
  {
    tcp_tick (NULL);
    sleep (1);
  }

  free (data);
  return (0);
}
#endif  /* TEST_PROG */
#endif  /* USE_FRAGMENTS */

