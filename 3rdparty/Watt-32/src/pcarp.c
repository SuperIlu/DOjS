/*!\file pcarp.c
 *
 * Address Resolution Protocol.
 *
 * 2002-09 Gundolf von Bachhaus:
 *   90% rewrite - Optional non-blocking ARP lookup & redirect handling
 *   Gateway / Route / ARP data stored & accessed separately
 *   Module encapsulated - no global variables.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "copyrigh.h"
#include "wattcp.h"
#include "language.h"
#include "netaddr.h"
#include "misc.h"
#include "misc_str.h"
#include "run.h"
#include "timer.h"
#include "ip4_in.h"
#include "ip4_out.h"
#include "sock_ini.h"
#include "chksum.h"
#include "pcdbug.h"
#include "pctcp.h"
#include "pcsed.h"
#include "pcconfig.h"
#include "pcqueue.h"
#include "pcstat.h"
#include "pcicmp.h"
#include "pcdhcp.h"
#include "pcpkt.h"
#include "pcarp.h"

#define DO_SORT_GATEWAYS 1

#if defined(USE_DEBUG)
  #define TRACE(args)   do {                                               \
                          if (arp_trace_level > 0) {                       \
                            if (arp_trace_level > 1)                       \
                              (*_printf) ("%s (%u): ", __FILE__,__LINE__); \
                            (*_printf) ("  ");                             \
                            (*_printf) args;                               \
                          }                                                \
                        } while (0)

  static const char *get_ARP_flags (WORD flg);
  static const char *get_route_flags (WORD flg);
#else
  #define TRACE(args)   ((void)0)
#endif

#define INET_NTOA(ip) _inet_ntoa (NULL, ip)

/**
 * Check if two address are on the same network.
 */
#define ON_SAME_NETWORK(addr1, addr2, mask) ((addr1 & mask) == (addr2 & mask))


/* Local parameters.
 */
static int  arp_trace_level = 0;      /* trace level for this code (>1 include file/line) */
static int  arp_timeout     = 2;      /* 2 seconds ARP timeout    */
static int  arp_alive       = 300;    /* 5 min ARP cache lifespan */
static int  arp_rexmit_to   = 250;    /* 250 milliseconds per try */
static BOOL dead_gw_detect  = FALSE;  /* Enable Dead Gateway detection */
static BOOL arp_gratiotous  = FALSE;

static BOOL LAN_lookup (DWORD ip, eth_address *eth);
static BOOL is_on_LAN  (DWORD ip);

static BOOL          arp_check_common (DWORD ip, eth_address *eth);
static void W32_CALL arp_daemon (void);

/**
 * GATEWAY HANDLING.
 */
#define DEAD_GW_TIMEOUT    10000   /* 10 sec */
#define GATE_TOP_OF_CACHE  8

static int gate_top = 0;     /* index into gate_list */

static struct gate_entry gate_list [GATE_TOP_OF_CACHE];

/**
 * Return start of gate_list[].
 *
 * \note Only 'gate_top' gateways are valid in result.
 */
int W32_CALL _arp_gateways_get (const struct gate_entry **rc)
{
  *rc = gate_list;
  return (gate_top);
}

#if defined(USE_DEAD_GWD)
/**
 * Send a ping (with TTL=1) to a gateway.
 * Ref. RFC-1122.
 */
static WORD icmp_id  = 0;
static WORD icmp_seq = 0;

static int ping_gateway (DWORD host, void *eth)
{
  struct ping_pkt  *pkt;
  struct ICMP_echo *icmp;
  struct in_Header *ip;
  int    len;

  pkt  = (struct ping_pkt*) _eth_formatpacket (eth, IP4_TYPE);
  ip   = &pkt->in;
  icmp = &pkt->icmp;
  len  = sizeof (*icmp);
  icmp_id = (WORD) set_timeout (0);  /* "random" id */

  TRACE (("ping_gateway (%s)\n", INET_NTOA(host)));

  icmp->type       = ICMP_ECHO;
  icmp->code       = 0;
  icmp->index      = 1;
  icmp->identifier = icmp_id;
  icmp->sequence   = icmp_seq++;
  icmp->checksum   = 0;
  icmp->checksum   = ~CHECKSUM (icmp, len);

  return IP4_OUTPUT (ip, 0, intel(host), ICMP_PROTO, 1,
                     (BYTE)_default_tos, 0, len, NULL);
}

/*
 * This function is called once each second.
 */
static void check_dead_gw (void)
{
  static int i = 0;

  if (i >= gate_top)
      i = 0;

  for ( ; i < gate_top; i++)
  {
    struct gate_entry *gw = gate_list + i;
    eth_address        eth;

    TRACE (("check_dead_gw (1), i %d\n", i));

    if (!is_on_LAN(gw->gate_ip))
       continue;

    if (!LAN_lookup(gw->gate_ip,&eth))
    {
      TRACE (("check_dead_gw (2), IP %s\n", INET_NTOA(gw->gate_ip)));
      continue;
    }

    if (gw->chk_timer == 0UL || !chk_timeout(gw->chk_timer))
    {
      gw->chk_timer = set_timeout (DEAD_GW_TIMEOUT);
      continue;
    }

    if (gw->echo_pending &&
        _chk_ping(gw->gate_ip,NULL) == (DWORD)-1)
    {
      TRACE (("check_dead_gw (3), i %d\n", i));

      gw->is_dead      = TRUE;
      gw->echo_pending = FALSE;
      TRACE (("Dead default GW %s (%d) detected\n",
              INET_NTOA(gw->gate_ip), i));
    }

    if (ping_gateway(gw->gate_ip, eth))
       gw->echo_pending = TRUE;

    gw->chk_timer = set_timeout (DEAD_GW_TIMEOUT);
    return;  /* only one ping per interval */
  }
}
#endif   /* USE_DEAD_GWD */

/*
 * Check if a mask is legal for specified address.
 */
static BOOL legal_mask (DWORD mask, DWORD addr)
{
#if 1
  ARGSUSED (addr);
  return check_mask (mask);
#else
  if (addr & ~mask)      /* E.g. '10.0.0.5' AND '0.0.0.255' */
    return (FALSE);
  mask = intel (~mask);
  if (mask & (mask+1))
    return (FALSE);
  return (TRUE);
#endif
}

#if DO_SORT_GATEWAYS
/**
 * Compare two entries in gateway-list.
 * Sort such that entries with the longest netmasks (32-mask_len())
 * are put first. Thus the default route (mask 0.0.0.0) is matched
 * last in _route_destin().
 *
 * From http://www.linuxfoundation.org/collaborate/workgroups/networking/networkoverview:
 *   LPM (Longest Prefix Match) is the lookup algorithm.
 *   The route with the longest netmask is the one chosen.
 *   Netmask 0, which is the shortest netmask, is for the default gateway.
 */
static int MS_CDECL compare (const struct gate_entry *a,
                             const struct gate_entry *b)
{
  return (int)(a->mask - b->mask);
}
#endif

/**
 * Add a gateway to the routing table.
 *
 * The format of 'config_string' is:
 *   gateway [,subnet], mask]]
 *
 * If 'config_string' is NULL, simply add 'ip' with zero
 * mask and subnet.
 *
 * eg. "gateway = 129.97.176.1"  -> becomes the default route
 * eg. "gateway = 129.97.176.2, 129.97.0.0, 255.255.0.0"
 *
 * The  first  example  shows  how a  default  gateway  is
 * created.  A default gateway is used if no other choices
 * exist.
 *
 * The second example shows how to specify a gateway for a
 * particular subnet.  In this example, whenever the 'top'
 * 16 bits of the destination are 129.97.*.*, that gateway
 * will be used.
 *
 * \todo Move to route.c and rename to route_add().
 */
BOOL W32_CALL _arp_add_gateway (const char *config_string, DWORD ip)
{
  struct gate_entry *gw;
  DWORD  subnet = 0UL;
  DWORD  mask   = 0UL;
  int    i;

  if (config_string)
  {
    const char *subnetp, *maskp;

    /* Get gateway IP address from string
     */
    ip = aton (config_string);
    if (ip == 0)
       return (FALSE);   /* invalid */

    /* Check if optional subnet was supplied
     */
    subnetp = strchr (config_string, ',');
    if (subnetp)
    {
      /* NB: atoi (used in aton) stops conversion when the first non-number
       * is hit, so there is no need to manually null-terminate the string.
       * i.e. aton ("123.123.123.123,blabla") = 123.123.123.123
       */
      subnet = aton (++subnetp);

      /* Check if optional mask was supplied
       */
      maskp = strchr (subnetp, ',');
      if (maskp)
      {
        mask = aton (++maskp);
      }
      else /* No mask was supplied, we derive it from the supplied subnet */
      {
        switch (subnet >> 30)
        {
          case 0:
          case 1:
               mask = CLASS_A_ADDR;
               break;
          case 2:
               mask = CLASS_B_ADDR;
               break;
          case 3:
          default:
               mask = CLASS_C_ADDR;
               break;
        }
      }
    }
  }
  else /* (config_string == NULL) */
  {
    if (ip == 0UL)
    {
      outsnl ("_arp_add_gateway(): Illegal router");
      return (FALSE);   /* args invalid */
    }
  }

  TRACE (("_arp_add_gateway (%s)\n"
          "   ip: %s, mask: %s, subnet: %s\n",
          config_string ? config_string : "<none>",
          INET_NTOA(ip), INET_NTOA(mask), INET_NTOA(subnet)));

  if (!legal_mask(mask,subnet))
  {
    outs ("Illegal mask "); outs (INET_NTOA(mask));
    outs (" for subnet ");  outsnl (INET_NTOA(subnet));
    return (FALSE);
  }

  /* Figure out where to put our new gateway
   */
  gw = NULL;

  /* Check if gateway is already is in list
   */
  for (i = 0; i < gate_top; i++)
      if (gate_list[i].gate_ip == ip)
      {
        gw = gate_list + i;
        break;
      }

  /* If a 'new' gateway, we check if we have enough room and simply
   * add it to the end of the list.
   *
   * There is not much point in sorting the list, as the 'whole' list is
   * scanned when a gateway is needed. Usually there will only be 1 anyway.
   * If we do sort, we would use plain old insert-sort and NOT quicksort,
   * as quicksort goes O(n^2) on an already sorted list, plus it has a high
   * overhead that especially hurts on a tiny list like this.
   */
  if (gw == NULL)
  {
    if (gate_top == DIM(gate_list)-1)
    {
      outsnl (_LANG("Gateway table full"));
      return (FALSE);  /* no more room */
    }
    gw = gate_list + gate_top;
    gate_top++;
  }

  /* Fill in new (or reused) entry
   */
  memset (gw, 0, sizeof(*gw));
  gw->gate_ip = ip;
  gw->mask    = mask;
  gw->subnet  = subnet;

#if DO_SORT_GATEWAYS
  qsort ((void*)&gate_list, gate_top, sizeof(gate_list[0]),
         (CmpFunc)compare);
#endif

  return (TRUE);
}

/**
 * Delete all gateways.
 */
void W32_CALL _arp_kill_gateways (void)
{
  TRACE (("gate_top=%d, _arp_kill_gateways(), gate_top=0\n", gate_top));
  gate_top = 0;
}

/**
 * Check if we have at least one default gateway.
 */
BOOL W32_CALL _arp_have_default_gw (void)
{
  int i, num = 0;

  for (i = 0; i < gate_top; i++)
      if (gate_list[i].subnet == 0UL)
         num++;
  TRACE (("_arp_have_default_gw(), num: %d\n", num));
  return (num > 0);
}


/**
 * ARP HANDLING.
 */

/**
 * ARP cache table internal structure:
 * \verbatim
 *
 * Index pointers       ARP cache table
 * --------------       ---------------
 *
 * (top_of_cache) ->
 *  (= top_pending)   ----------------------
 *                    | ...
 *                    | PENDING ARP ENTRIES (requested/ing - no reply yet)
 * first_pending ->   | ... grow downwards (into free slots)
 *  (= top_free)      ----------------------
 *                    | ...
 *                    | ...
 *                    | FREE SLOTS
 *                    | ...
 * first_free ---->   | ...
 *  (= top_dynamic)   ----------------------
 *                    | ... grow upwards (into free slots)
 *                    | ...
 *                    | "DYNAMIC" ARP ENTRIES
 * first_dynamic ->   | ...
 *  (= top_fixed)     ----------------------
 *                    | ... grow upwards (rolling dynamic entries upwards)
 *                    | FIXED ARP ENTRIES
 *  (first_dynamic=0) | ...
 *                    ----------------------
 * \endverbatim
 *
 * \note
 *  - "Top" means last entry + 1.
 *  - The entries inside each section are not ordered in any way.
 *  - The ARP cache only holds entries of hosts that are on our LAN.
 *  - Connections to hosts outside of our LAN are done over a gateway, and
 *    the IP of the gateway used (per host) is stored in the "route cache"
 *    (below).
 */

#define ARP_TOP_OF_CACHE    64 /**< would need to be a variable if ARP
                                *   table were auto-resizing
                                */
#define ARP_TOP_PENDING     ARP_TOP_OF_CACHE
#define ARP_TOP_FREE        arp_first_pending
#define ARP_TOP_DYNAMIC     arp_first_free
#define ARP_TOP_FIXED       arp_first_dynamic
#define ARP_FIRST_FIXED     0

static int arp_first_pending = ARP_TOP_PENDING;
static int arp_first_free    = 0;
static int arp_first_dynamic = 0;

static struct arp_entry arp_list [ARP_TOP_OF_CACHE];

/**
 * Low-level ARP send function.
 */
static BOOL arp_send (const arp_Header *arp, unsigned line)
{
  TRACE (("_arp_send()\n"));
  ARGSUSED (arp);
  return _eth_send (sizeof(*arp), NULL, __FILE__, line);
}


/**
 * Send broadcast ARP request.
 */
static BOOL arp_send_request (DWORD ip)
{
  arp_Header *arp = (arp_Header*) _eth_formatpacket (&_eth_brdcast[0], ARP_TYPE);

  TRACE (("_arp_send_request (%s)\n", INET_NTOA(ip)));

  arp->hwType       = intel16 (_eth_get_hwtype(NULL,NULL));
  arp->protType     = IP4_TYPE;
  arp->hwAddrLen    = sizeof (eth_address);
  arp->protoAddrLen = sizeof (ip);
  arp->opcode       = ARP_REQUEST;
  arp->srcIPAddr    = intel (my_ip_addr);
  arp->dstIPAddr    = intel (ip);
  memcpy (arp->srcEthAddr, _eth_addr, sizeof(arp->srcEthAddr));
  memset (arp->dstEthAddr, 0, sizeof(arp->dstEthAddr));
  return arp_send (arp, __LINE__);
}


/**
 * Send unicast/broadcast ARP reply.
 * 'src_ip' and 'dst_ip' on host order.
 */
BOOL W32_CALL _arp_reply (const void *e_dst, DWORD src_ip, DWORD dst_ip)
{
  arp_Header *arp;

  if (!e_dst)
     e_dst = &_eth_brdcast;

  TRACE (("Sending ARP reply (%s [%s] -> %s [%s])\n",
          INET_NTOA(src_ip), MAC_address(&_eth_addr),
          INET_NTOA(dst_ip), MAC_address(e_dst)));

  arp = (arp_Header*) _eth_formatpacket (e_dst, ARP_TYPE);
  arp->hwType       = intel16 (_eth_get_hwtype(NULL,NULL));
  arp->protType     = IP4_TYPE;
  arp->hwAddrLen    = sizeof (mac_address);
  arp->protoAddrLen = sizeof (dst_ip);
  arp->opcode       = ARP_REPLY;
  arp->srcIPAddr    = intel (src_ip);
  arp->dstIPAddr    = intel (dst_ip);

  memcpy (arp->srcEthAddr, _eth_addr, sizeof(mac_address));
  memcpy (arp->dstEthAddr, e_dst, sizeof(mac_address));
  return arp_send (arp, __LINE__);
}

/**
 * Move an ARP entry from \b from_index to \b to_index.
 */
static void arp_move_entry (int to_index, int from_index)
{
  memcpy (&arp_list[to_index], &arp_list[from_index], sizeof(struct arp_entry));
}

/**
 * Return start of arp_list[].
 *
 * \note Called need to inspect the '*rc->flags' for validity.
 */
int W32_CALL _arp_cache_get (const struct arp_entry **rc)
{
  *rc = arp_list;
  return DIM(arp_list);
}


/**
 * Start a host lookup on the LAN.
 *
 * This is called by 'higher' routines to start an
 * ARP request. This function is non-blocking, i.e. returns 'immediately'.
 */
static BOOL LAN_start_lookup (DWORD ip)
{
  struct arp_entry *ae;
  int    i;

  TRACE (("LAN_start_lookup (%s)\n", INET_NTOA(ip)));

  /* Ignore if IP is already in any list section (pending, fixed, dynamic)
   */
  for (i = arp_first_pending; i < ARP_TOP_PENDING; i++)
      if (arp_list[i].ip == ip)
          return (TRUE);

  for (i = arp_first_dynamic; i < ARP_TOP_DYNAMIC; i++)
      if (arp_list[i].ip == ip)
      {
        STAT (cache_stats.num_arp_hits++);
        return (TRUE);
      }

  for (i = ARP_FIRST_FIXED; i < ARP_TOP_FIXED; i++)
      if (arp_list[i].ip == ip)
      {
        STAT (cache_stats.num_arp_hits++);
        return (TRUE);
      }

  /* Figure out where to put the new guy
   */
  if (arp_first_free < ARP_TOP_FREE) /* Do we have any free slots? */
  {
    /* yes, ok! */
  }
  else if (arp_first_dynamic < ARP_TOP_DYNAMIC) /* any dynamic entries? */
  {
    /* This new request is probably more important than an existing
     * dynamic entry, so we sacrifice the top dynamic entry. It might be
     * neater to kill the oldest entry, but all this shouldn't happen anyway.
     * NB: Table size reallocation would go here.
     */
    --ARP_TOP_DYNAMIC; /* nuke top entry */
    STAT (cache_stats.num_arp_overflow++);
    outsnl (_LANG("ARP table overflow"));
  }
  else /* No more room - table is full with pending + fixed entries. */
  {
    outsnl (_LANG("ARP table full"));
    STAT (cache_stats.num_arp_overflow++);
    return (FALSE);       /* failed, nothing we can do right now. */
  }

  /* Fill new slot, send out ARP request
   */
  --arp_first_pending;
  ae = arp_list + arp_first_pending;
  ae->ip     = ip;
  ae->expiry = set_timeout (1000UL * arp_timeout);
  ae->flags  = (ARP_FLG_INUSE | ARP_FLG_PENDING);
  ae->retransmit_to = set_timeout (arp_rexmit_to);

  STAT (cache_stats.num_arp_misses++);

  /* If request fails, we try again a little sooner
   */
  if (!arp_send_request(ip))
       ae->retransmit_to = set_timeout (arp_rexmit_to / 4);
  else ae->retransmit_to = set_timeout (arp_rexmit_to);

  return (TRUE);      /* ok, new request logged */
}

/**
 * Lookup host in fixed/dynamic list.
 */
static BOOL LAN_lookup (DWORD ip, eth_address *eth)
{
  int  i;
  BOOL rc = FALSE;

  /* Check in dynamic + fixed list section
   */
  for (i = ARP_FIRST_FIXED; i < ARP_TOP_DYNAMIC; i++)
  {
    if (arp_list[i].ip != ip)
       continue;
    if (eth)
       memcpy (eth, arp_list[i].hardware, sizeof(*eth));
    rc = TRUE;
    break;
  }
  TRACE (("LAN_lookup (%s), i: %d, flg: %s, rc: %d\n",
          INET_NTOA(ip), i, rc ? get_ARP_flags(arp_list[i].flags) : "<n/a>", rc));
  return (rc);
}

/**
 * Lookup host in pending list.
 */
static BOOL LAN_lookup_pending (DWORD ip)
{
  BOOL rc = FALSE;
  int  i;

  /* Scan pending list section
   */
  for (i = arp_first_pending; i < ARP_TOP_PENDING; i++)
      if (arp_list[i].ip == ip)
      {
        rc = TRUE;
        break;
      }

  TRACE (("LAN_lookup_pending (%s), i: %d, flg: %s, rc: %d\n",
          INET_NTOA(ip), i, rc ? get_ARP_flags(arp_list[i].flags) : "<n/a>",
          rc));
  return (rc);
}

/**
 * Check ARP entries for timeout.
 *
 * This function runs through the 'pending' section of the ARP table and
 * checks for entries that have either expired or require a re-send.
 * The 'dynamic' entries are checked for expiry if requested.
 */
static void arp_check_timeouts (BOOL check_dynamic_entries)
{
  struct arp_entry *ae;
  int    i, num = 0;

  /* Check pending entries for retansmit & expiry
   */
  for (i = arp_first_pending; i < ARP_TOP_PENDING; i++)
  {
    ae = arp_list + i;

    /* If entry has expired (without being resolved): kill it
     */
    if (chk_timeout(ae->expiry))
    {
      num++;
      if (i > arp_first_pending)
      {
        ae->flags &= ~ARP_FLG_INUSE;
        arp_move_entry (i--, arp_first_pending);  /* fill hole */
        arp_first_pending++;
        continue;   /* backed 'i' up a step, now re-check 'new' current entry */
      }
      ++arp_first_pending;
    }
    /* If time for a retransmission: do it & restart timeout
     */
    else if (chk_timeout(ae->retransmit_to))
    {
      /* If request fails, we try again a little sooner
       */
      ae->retransmit_to = set_timeout (arp_send_request(ae->ip) ?
                                       arp_rexmit_to : arp_rexmit_to / 4);
    }
  }

  /* Check dynamic entries for expiry
   */
  if (!check_dynamic_entries)
     goto quit;

  for (i = arp_first_dynamic; i < ARP_TOP_DYNAMIC; i++)
  {
    ae = arp_list + i;

    if (chk_timeout(ae->expiry))  /* entry has expired: kill it */
    {
      if (i < --ARP_TOP_DYNAMIC)
      {
        ae->flags &= ~ARP_FLG_INUSE;
        num++;
        arp_move_entry (i--, ARP_TOP_DYNAMIC);  /* fill hole */
        /* backed 'i' up a step, now re-check 'new' current entry */
      }
    }
  }

quit:
  TRACE (("arp_check_timeouts (%d), num expired/rechecked: %d\n", check_dynamic_entries, num));
}


/**
 * ROUTE (& REDIRECT) HANDLING.
 */

/* Routing table internal structure:
 * \verbatim
 *
 * Index pointers       Route cache table
 * --------------       -----------------
 *
 * (top_of_cache) ->
 *  (= top_pending)   ----------------------
 *                    | ...
 *                    | PENDING ROUTE ENTRIES (ARPing gateway - no reply yet)
 * first_pending ->   | ... grow downwards (into free slots)
 *  (= top_free)      ----------------------
 *                    | ...
 *                    | ...
 *                    | FREE SLOTS
 *                    | ...
 * first_free ---->   | ...
 *  (= top_dynamic)   ----------------------
 *                    | ... grow upwards (into free slots)
 *                    | ...
 *                    | "DYNAMIC" ROUTE ENTRIES
 * first_dynamic ->   | ...
 *                    ----------------------
 * \endverbatim
 *
 * \note
 *  - "Top" means last entry + 1.
 *  - The entries inside each section are not ordered in any way.
 *  - The route cache only holds entries of hosts that are OUTSIDE of our LAN.
 *  - The entries time-out when the gateways ARP cache entry times out.
 *
 * \note
 *  Although we don't support multiple physical interfaces, we support
 *  multiple gateways connected to our interface.
 */

#define ROUTE_TOP_OF_CACHE    32
#define ROUTE_TOP_PENDING     ROUTE_TOP_OF_CACHE
#define route_top_free        route_first_pending
#define route_top_dynamic     route_first_free
#define ROUTE_FIRST_DYNAMIC   0

static int route_first_pending = ROUTE_TOP_PENDING;
static int route_first_free    = 0;

static struct route_entry route_list [ROUTE_TOP_OF_CACHE];

static void route_move_entry (int to_index, int from_index)
{
  memcpy (&route_list[to_index], &route_list[from_index],  /* never overlapping mem-area */
          sizeof(route_list[0]));
}

static BOOL route_make_new_slot (DWORD host_ip, DWORD gate_ip, DWORD mask)
{
  struct route_entry *re;

  /* We assume IP was already checked for, otherwise we would add it twice.
   * Check where we have room.
   */
  if (route_first_free < route_top_free)
  {
    /* Ok, free slots available */
  }
  else if (ROUTE_FIRST_DYNAMIC < route_top_dynamic)
  {
    /* Slaughter first dynamic entry, as new entry probably is more important
     */
    if (ROUTE_FIRST_DYNAMIC < --route_top_dynamic)
       route_move_entry (ROUTE_FIRST_DYNAMIC, route_top_dynamic);
    outsnl (_LANG("Route table overflow"));
  }
  else
  {
    outsnl (_LANG("Route table full"));
    return (FALSE); /* Nothing we can do - list is full of pending entries */
  }

  /* Put the new entry in
   */
  --route_first_pending;
  re = route_list + route_first_pending;
  re->host_ip = host_ip;    /* when connection to this host ... */
  re->gate_ip = gate_ip;    /* ... use this gateway */
  re->mask    = mask;       /* remember the mask */
  re->flags   = (ROUTE_FLG_USED | ROUTE_FLG_PENDING);

  TRACE (("route_make_new_slot(): added host %s, GW %s, mask %s at index %d\n",
          INET_NTOA(host_ip), INET_NTOA(gate_ip), INET_NTOA(mask), route_first_pending));
  return (TRUE);
}

/**
 * This should probably go into route.c
 */
static BOOL is_on_LAN (DWORD ip)
{
#if 1
  return (((ip ^ my_ip_addr) & sin_mask) == 0);
#else
  return (ip && sin_mask && ((ip ^ my_ip_addr) & sin_mask) == 0);
#endif

}

/**
 * Register a new host as gateway. Called on ICMP-redirects.
 *
 * Redirects are sent from a gateway/router, telling us that a
 * different gateway is better suited to connect to the specified
 * host than the one we were using.
 */
BOOL W32_CALL _arp_register (DWORD use_this_gateway_ip, DWORD for_this_host_ip)
{
  int i;

  TRACE (("_arp_register (%s, %s)\n",
          INET_NTOA(use_this_gateway_ip),
          INET_NTOA(for_this_host_ip)));

  /* Only makes sense if ("old") host is outside of our LAN,
   * and ("new") gateway is on LAN.
   */
  if (!is_on_LAN(use_this_gateway_ip) || is_on_LAN(for_this_host_ip))
     return (FALSE);

  /* See if this guy is in our dynamic table
   */
  for (i = ROUTE_FIRST_DYNAMIC; i < route_top_dynamic; i++)
  {
    struct route_entry *re = route_list + i;

    if (re->host_ip != for_this_host_ip)
       continue;

    if (re->gate_ip == use_this_gateway_ip)
       return (TRUE); /* Already done */

    if (LAN_lookup(use_this_gateway_ip, NULL))
    {
      re->gate_ip = use_this_gateway_ip;
      return (TRUE);  /* New gateway is already in ARP cache, done */
    }

    if (!LAN_start_lookup(use_this_gateway_ip))
    {
      outsnl (_LANG ("Unable to add redirect to ARP cache"));
      return (FALSE); /* ARP table full */
    }

    /* Kill 'old' dynamic entry, fill hole
     */
    if (i < --route_top_dynamic)
       route_move_entry (i, route_top_dynamic);

    /* Add new request, the new dynamic slot will be created when the
     * gateway ARP reply comes
     */
    return route_make_new_slot (use_this_gateway_ip, for_this_host_ip, sin_mask);
  }

  /* Note: We do not check the pending section, as the gateway sending
   * the redirect could not really know the best route to a host that
   * we have not yet even started to connect to. Redirects referring
   * to a pending entry could be some sort of redirect attack.
   */
  return (FALSE);
}

/**
 * Gets MAC of gateway needed to reach the given host.
 */
static BOOL route_lookup (DWORD ip, eth_address *eth)
{
  BOOL rc = FALSE;
  int  i;

  TRACE (("route_lookup (%s), ROUTE_FIRST_DYNAMIC: %d, route_top_dynamic: %d\n",
          INET_NTOA(ip), ROUTE_FIRST_DYNAMIC, route_top_dynamic));

  /* 1st, we need to find the gateway entry for the specified host
   * in our route table
   */
  for (i = ROUTE_FIRST_DYNAMIC; i < route_top_dynamic; i++)
  {
    struct route_entry *re = route_list + i;
    BOOL   same_net = ON_SAME_NETWORK (ip, re->host_ip, re->mask);

    TRACE (("  route_list[%d]: gate_ip %s, host_ip: %s, ON_SAME_NETWORK: %d\n",
            i, INET_NTOA(re->gate_ip), INET_NTOA(re->host_ip), same_net));

    if (re->host_ip != ip && !same_net)   // !!
       continue;

    /* 2nd, the gateway needs to be in the ARP table
     */
    re->flags |= ROUTE_FLG_PENDING;
    rc = LAN_lookup (re->gate_ip, eth);
    if (rc)
       re->flags &= ~ROUTE_FLG_PENDING;
    break;
  }
  TRACE (("  rc: %d\n", rc));
  return (rc);
}

/**
 * Returns TRUE if the lookup of the gateway (assigned to the
 * supplied ip) is still pending.
 */
static BOOL route_lookup_pending (DWORD ip)
{
  const struct route_entry *re = NULL;
  BOOL  rc = FALSE;
  int   i = 0;

  /* Scan our pending list for the supplied IP
   */
  for (i = route_first_pending; i < ROUTE_TOP_PENDING; i++)
  {
    re = route_list + i;
    if (re->host_ip == ip)
    {
      rc = TRUE;
      break;
    }
  }
  TRACE (("route_lookup_pending (%s), idx: %d, rc: %d, re->flags: %s\n",
          INET_NTOA(ip), i, rc, rc ? get_route_flags(re->flags) : "<n/a>"));
  return (rc);
}

/**
 * Start a route lookup.
 */
static BOOL route_start_lookup (DWORD ip)
{
  DWORD first_gate_ip;
  DWORD first_gate_mask;
  int   i;

  TRACE (("route_start_lookup (%s), gate_top: %d\n", INET_NTOA(ip), gate_top));

  /* Check if we already have an entry anywhere for this host
   */
  for (i = route_first_pending; i < ROUTE_TOP_PENDING; i++)
      if (route_list[i].host_ip == ip)
         return (TRUE);   /* Already here */

  for (i = ROUTE_FIRST_DYNAMIC; i < route_top_dynamic; i++)
      if (route_list[i].host_ip == ip)
         return (TRUE);   /* Already here */

  /* Abort if we don't have any gateways
   */
  if (gate_top <= 0)
  {
    outsnl (_LANG ("No gateways defined."));
    return (FALSE);
  }

  /* Find the first 'fitting' gateway
   */
  first_gate_ip = 0;    /* we remember the first gateway IP that fits */
  first_gate_mask = 0;

  for (i = 0; i < gate_top; i++)
  {
    const struct gate_entry *gw = gate_list + i;

    TRACE (("  i %d, gw->gate_ip %s, gw->subnet %s, gw->mask %s, gw->is_dead %d\n", i,
            INET_NTOA(gw->gate_ip), INET_NTOA(gw->subnet),
            INET_NTOA(gw->mask), gw->is_dead));

    if (/* sin_mask != IP_BCAST_ADDR && */ !is_on_LAN(gw->gate_ip))
       continue;

    if ((gw->mask & ip) != gw->subnet)   /* IP not on same subnet as gw->gate_ip */
       continue;

    if (gw->is_dead)
       continue;

    if (!LAN_start_lookup(gw->gate_ip))
    {
      outsnl (_LANG ("Unable to add gateway to ARP cache"));
      return (FALSE); /* ARP table full, no point in going on right now */
    }
    first_gate_ip   = gw->gate_ip;   /* We start with this guy */
    first_gate_mask = gw->mask;
    break;
  }

  /* Abort if we didn't find anybody at all to ARP, or all ARPs failed
   */
  if (first_gate_ip == 0)
  {
    outsnl (_LANG ("No matching gateway"));
    return (FALSE);
  }

  /* Create a new route cache slot with our guy
   */
  route_make_new_slot (ip, first_gate_ip, first_gate_mask);
  return (TRUE);
}


/**
 * Periodic route checker.
 * Run through all pending entries and check if an attempt to
 * reach a gateway was successfull, or has timed-out.
 * If the attempt timed-out, we try the next fitting gateway.
 * If there are no more fitting gateways to try, the connection has failed.
 */
static void route_check_timeouts (BOOL check_dynamic_entries)
{
  struct route_entry *re, temp;
  int    i, j;

  /* Check our pending entries
   */
  for (i = route_first_pending; i < ROUTE_TOP_PENDING; i++)
  {
    re = route_list + i;

    /* Was the ARP lookup able to resolve the gateway IP?
     */
    if (LAN_lookup(re->gate_ip, NULL))
    {
      /* Success - move route entry from pending to dynamic list
       */
      temp = *re;    /* Make a copy so we can safely delete the pending entry */
      if (i > route_first_pending)
         route_move_entry (i--, route_first_pending);    /* fill hole */
         /* (i-- to "re"check new current entry) */

      temp.flags &= ~ROUTE_FLG_PENDING;
      ++route_first_pending;                 /* remove from pending list */
      route_list [route_top_dynamic] = temp; /* add to dynamic list */
      ++route_top_dynamic;
    }
    /* Is the ARP lookup still pending? -> Keep waiting
     */
    else if (LAN_lookup_pending(re->gate_ip))
    {
      /* Do nothing */
    }
    /* The ARP lookup failed, we try the next possible gateway
     */
    else
    {
      /* Find the gateway that was tried last (the one that just timed out)
       */
      BOOL found_last_gw = FALSE;
      BOOL found_next_gw = FALSE;

      for (j = 0; j < gate_top; j++)
      {
        if (gate_list[j].gate_ip != re->gate_ip)
           continue;
        found_last_gw = TRUE;
        break;
      }

      if (!found_last_gw)
         j = -1; /* If search failed, we try the first gateway "again". */

      /* Now we look for the next gateway that could be used
       */
      while (++j < gate_top)
      {
        const struct gate_entry *gw = gate_list + j;

        if (/* sin_mask != IP_BCAST_ADDR && */ !is_on_LAN(gw->gate_ip))
           continue;

        if ((gw->mask & re->host_ip) != gw->subnet)
           continue;

        if (!LAN_start_lookup(gw->gate_ip))
           break;                  /* No room in ARP table, fail */

        re->gate_ip = gw->gate_ip; /* Ok, now we try this gateway */
        found_next_gw = TRUE;
        break;
      }

      /* No more gateways to try, hence lookup failed, kill entry
       */
      if (!found_next_gw)
      {
        if (i > route_first_pending)
           route_move_entry (i--, route_first_pending); /* fill hole */
        ++route_first_pending;
      }
    }
  }

  if (!check_dynamic_entries)
     return;

  /* Check our dynamic list for expired entries
   */
  for (i = ROUTE_FIRST_DYNAMIC; i < route_top_dynamic; i++)
  {
    re = route_list + i;
    if (LAN_lookup(re->gate_ip, NULL))
       continue;   /* Still in ARP cache - ok */

    /* ARP entry expired. Remove from list.
     */
    if (i < --route_top_dynamic)
    {
      route_move_entry (i--, route_top_dynamic); /* fill hole */
      /* (i backed up a step so 'new' slot is rechecked) */
      re = route_list + i;
      re->flags &= ~ROUTE_FLG_USED;
    }
  }
}


/**
 * "PUBLIC" ARP/ROUTE FUNCTIONS.
 *
 * These are the main functions visible from outside the module.
 * Most of them simply check if the destination host IP is on the
 * LAN or needs to be routed over a gateway, and then relay on to
 * the apropriate LAN_...() or route_...() function.
 */

#if !defined(USE_UDP_ONLY)
/**
 * Initialise an ARP lookup.
 * This is called by 'higher' routines to start an ARP request.
 * This function is non-blocking, i.e. returns 'immediately'.
 */
BOOL W32_CALL _arp_start_lookup (DWORD ip)
{
  TRACE (("_arp_start_lookup (%s)\n", INET_NTOA(ip)));

  if (arp_check_common(ip,NULL))
     return (TRUE);

  if (is_on_LAN(ip))
     return LAN_start_lookup (ip);
  return route_start_lookup (ip);
}


/**
 * Lookup MAC-address of 'ip'.
 * \retval TRUE MAC for 'ip' is known.
 */
BOOL W32_CALL _arp_lookup (DWORD ip, eth_address *eth)
{
  TRACE (("_arp_lookup (%s), is_on_LAN(): %d\n",
          INET_NTOA(ip), is_on_LAN(ip)));

  if (arp_check_common(ip,eth))
     return (TRUE);

  if (is_on_LAN(ip))
     return LAN_lookup (ip, eth);
  return route_lookup (ip, eth);
}


/**
 * An ARP-lookup timeout check.
 * \retval TRUE  The lookup is currently underway ("pending").
 * \retval FALSE The IP has either been resolved (_arp_lookup() == TRUE),
 *         or the lookup has timed out, i.e. the host is unreachable.
 */
BOOL W32_CALL _arp_lookup_pending (DWORD ip)
{
  TRACE (("_arp_lookup_pending (%s)\n", INET_NTOA(ip)));

  if (is_on_LAN(ip))
     return LAN_lookup_pending (ip);
  return route_lookup_pending (ip);
}

/**
 * Lookup fixed MAC-address of 'ip'.
 * \retval TRUE Supplied 'ip' has a fixed MAC entry.
 */
BOOL W32_CALL _arp_lookup_fixed (DWORD ip, eth_address *eth)
{
  int i;

  TRACE (("_arp_lookup_fixed (%s)\n", INET_NTOA(ip)));

  if (arp_check_common(ip,eth))
     return (TRUE);

  if (!is_on_LAN(ip))  /* We only have/need a LAN version */
     return (FALSE);

  /* Scan fixed table section
   */
  for (i = ARP_FIRST_FIXED; i < ARP_TOP_FIXED; i++)
  {
    if (arp_list[i].ip != ip)
       continue;
    if (eth)
       memcpy (eth, arp_list[i].hardware, sizeof(*eth));
    return (TRUE);
  }
  return (FALSE);
}
#endif  /* USE_UDP_ONLY */

/**
 * Common stuff for _arp_resolve() and _arp_lookup()
 */
static BOOL arp_check_common (DWORD ip, eth_address *eth)
{
#if defined(USE_DEBUG)
  if (arp_trace_level >= 4)
  {
    _arp_cache_dump();
    _arp_gateways_dump();
    _arp_routes_dump();
  }
#endif

  STAT (cache_stats.num_arp_search++);

  if (_pktserial) /* A serial driver uses a null MAC */
  {
    if (eth)
       memset (eth, 0, sizeof(*eth));
    return (TRUE);
  }
  if (_ip4_is_local_addr(ip)) /* A local address uses own MAC */
  {
    if (eth)
       memcpy (eth, _eth_addr, sizeof(*eth));
    return (TRUE);
  }
  return (FALSE);
}

/**
 * The blocking lookup function visible to higher functions.
 */
BOOL W32_CALL _arp_resolve (DWORD ip, eth_address *eth)
{
  BOOL (*lookup) (DWORD ip, eth_address *eth);
  BOOL (*start_lookup) (DWORD ip);
  BOOL (*pending_lookup) (DWORD ip);
  BOOL   rc = FALSE;
  WORD   brk_mode;

  TRACE (("_arp_resolve (%s)\n", INET_NTOA(ip)));

  if (arp_check_common(ip,eth))
     return (TRUE);

  /* Quick check if we have this guy in our cache.
   */
  if (is_on_LAN(ip))
  {
    lookup         = LAN_lookup;
    start_lookup   = LAN_start_lookup;
    pending_lookup = LAN_lookup_pending;
  }
  else
  {
    lookup         = route_lookup;
    start_lookup   = route_start_lookup;
    pending_lookup = route_lookup_pending;
  }

  if ((*lookup)(ip, eth))
     return (TRUE);         /* Ok, done */

  /* Put out the request for the MAC
   */
  if (!(*start_lookup)(ip))
  {
    TRACE (("%s() failed\n", (start_lookup == LAN_start_lookup) ?
            "LAN_start_lookup" : "route_start_lookup"));
    return (FALSE);  /* Request failed, resolve doomed */
  }

  NEW_BREAK_MODE (brk_mode, 1);

  /* Now busy-wait until reply is here or timeout (or Ctrl-C)
   */
  do
  {
    tcp_tick (NULL);   /* will call our daemon to timeout/retran */
    arp_daemon();      /* added for faster lookup */

    if ((*lookup)(ip, eth))
    {
      rc = TRUE;
      break;
    }
    WATT_YIELD();

    if (_watt_cbroke)
    {
      _watt_cbroke = 0;
      break;
    }
  }
  while ((*pending_lookup)(ip));

  OLD_BREAK_MODE (brk_mode);
  return (rc);
}


/**
 * Add given IP/Ether address to ARP-cache.
 * \note 'ip' is on host order.
 */
BOOL W32_CALL _arp_cache_add (DWORD ip, const void *eth, BOOL expires)
{
  struct arp_entry *ae;

  TRACE (("_arp_cache_add (%s), expires: %d\n", INET_NTOA(ip), expires));

  if (!my_ip_addr && !expires)
  {
   /* If called from e.g. pcconfig, my_ip_addr may be 0 when using
    * DHCP. Allow adding fixed entries.
    */
  }
  else if (!is_on_LAN (ip))  /* Only makes sense if on our LAN. */
     return (FALSE);

  _arp_cache_del (ip);       /* Kill it if already here somewhere */

  /* Now add to correct list
   */
  if (expires) /* dynamic list */
  {
    if (arp_first_free >= ARP_TOP_FREE)  /* No free dynamic slots */
       return (FALSE);

    /* Fill new slot data
     */
    ae = arp_list + ARP_TOP_DYNAMIC;
    ARP_TOP_DYNAMIC++;
    ae->ip = ip;
    memcpy (&ae->hardware, eth, sizeof(ae->hardware));
    ae->expiry = set_timeout (1000UL * arp_alive);
    ae->flags  = (ARP_FLG_INUSE | ARP_FLG_DYNAMIC);
  }
  else   /* fixed list */
  {
    /* Check if we have any free slots; make room if possible
     */
    if (arp_first_free >= ARP_TOP_FREE) /* No more fixed slots? */
    {
      if (arp_first_dynamic >= ARP_TOP_DYNAMIC)
         return (FALSE);   /* No free AND no dynamic slots! */
      --ARP_TOP_DYNAMIC;   /* Kill the top dynamic slot to make room */
    }

    /* Roll dynamic entires up one slot to make room for the new fixed entry
     */
    if (arp_first_dynamic < ARP_TOP_DYNAMIC)
       arp_move_entry (ARP_TOP_DYNAMIC, arp_first_dynamic);
    ++ARP_TOP_DYNAMIC;

    /* Fill new slot data
     */
    ae = arp_list + ARP_TOP_FIXED;    /* implies ++arp_first_dynamic! */
    ARP_TOP_FIXED++;
    ae->ip    = ip;
    ae->flags = (ARP_FLG_INUSE | ARP_FLG_FIXED);
    memcpy (&ae->hardware, eth, sizeof(ae->hardware));

    TRACE_CONSOLE (4, "_arp_cache_add(): ip: %s, eth: %s, ARP_TOP_FIXED: %d, flags: %02X\n",
                   _inet_ntoa(NULL,ip), MAC_address(eth), ARP_TOP_FIXED-1, ae->flags);
  }
  return (TRUE);
}


/**
 * Delete given 'ip' address from ARP-cache (dynamic, fixed or pending).
 * \note 'ip' is on host order.
 */
BOOL W32_CALL _arp_cache_del (DWORD ip)
{
  struct arp_entry *ae;
  BOOL   rc = FALSE;
  int    i;

  /* Remove from dynamic list if present
   */
  for (i = arp_first_dynamic; i < ARP_TOP_DYNAMIC; i++)
  {
    ae = arp_list + i;
    if (ae->ip != ip)
       continue;

    if (i < --ARP_TOP_DYNAMIC)
       arp_move_entry (i, ARP_TOP_DYNAMIC);   /* fill hole */
    ae->flags &= ~ARP_FLG_INUSE;
    rc = TRUE;
    goto quit;
  }

  /* Remove from fixed list if present
   */
  for (i = ARP_FIRST_FIXED; i < ARP_TOP_FIXED; i++)
  {
    ae = arp_list + i;
    if (ae->ip != ip)
       continue;

    if (i < --ARP_TOP_FIXED)
       arp_move_entry (i, ARP_TOP_FIXED);   /* fill hole */

    /* Do we have any dynamic entries we need to roll down one slot?
     * arp_first_dynamic same as ARP_TOP_FIXED, already implicity
     * decremented above!
     */
    if (arp_first_dynamic < --ARP_TOP_DYNAMIC)
       arp_move_entry (arp_first_dynamic, ARP_TOP_DYNAMIC);
    ae->flags &= ~ARP_FLG_INUSE;
    rc = TRUE;
    goto quit;
  }

  /* Remove from pending list if present
   */
  for (i = arp_first_pending; i < ARP_TOP_PENDING; i++)
  {
    ae = arp_list + i;
    if (ae->ip != ip)
       continue;

    if (i > arp_first_pending)
       arp_move_entry (i, arp_first_pending);   /* fill hole */
    ++arp_first_pending;
    ae->flags &= ~ARP_FLG_INUSE;
    rc = TRUE;
    goto quit;
  }

  /* Didn't have it in cache */

quit:
  TRACE (("_arp_cache_del (%s). i: %d, rc: %d\n", INET_NTOA(ip), i, rc));
  return (rc);
}

/**
 * ARP background daemon.
 * Calls timeout-checks / retransmitters.
 *
 * We don't check the dynamic entries for a timeout on every call,
 * once a second is plenty enough.
 */
static void W32_CALL arp_daemon (void)
{
  static BOOL  check_dynamic       = TRUE;
  static DWORD check_dynamic_timer = 0UL;

  arp_check_timeouts   (check_dynamic);
  route_check_timeouts (check_dynamic);

  if (check_dynamic)
  {
#if defined(USE_DEAD_GWD) /* check dead gateways if we have >1 default GW */
    if (dead_gw_detect)
    {
      if (_arp_check_gateways() <= 1)
           dead_gw_detect = FALSE;
      else check_dead_gw();
    }
#endif

    /* Preset check_dynamic for next call (1 sec)
     */
    check_dynamic_timer = set_timeout (1000UL);
    check_dynamic = FALSE;
  }
  else if (chk_timeout(check_dynamic_timer))
  {
    check_dynamic = TRUE;
  }
}

/**
 * Parser for "\c ARP.xx" keywords in "\c WATTCP.CFG".
 */
static void (W32_CALL *prev_cfg_hook) (const char*, const char*);

static void W32_CALL arp_parse (const char *name, const char *value)
{
  static const struct config_table arp_cfg[] = {
         { "ALIVE",         ARG_ATOI, (void*)&arp_alive       },
         { "DEAD_GW_DETECT",ARG_ATOI, (void*)&dead_gw_detect  },
         { "GRATIOTOUS",    ARG_ATOI, (void*)&arp_gratiotous  },
         { "RETRANS_TO",    ARG_ATOI, (void*)&arp_rexmit_to   },
         { "TIMEOUT",       ARG_ATOI, (void*)&arp_timeout     },
         { "TRACE",         ARG_ATOI, (void*)&arp_trace_level },
         { NULL,            0,        NULL                    }
       };
  if (!parse_config_table(&arp_cfg[0], "ARP.", name, value) && prev_cfg_hook)
     (*prev_cfg_hook) (name, value);
}

/**
 * Setup config-table parse function and add background ARP deamon.
 */
void W32_CALL _arp_init (void)
{
  DAEMON_ADD (arp_daemon);

  memset (&arp_list, 0, sizeof(arp_list));
  prev_cfg_hook = usr_init;
  usr_init      = arp_parse;
}

#if defined(WIN32)
  #define BROADCAST_MODE() (_pkt_rxmode & RXMODE_BROADCAST)
#else
  #define BROADCAST_MODE() (_pkt_rxmode <= RXMODE_BROADCAST)
#endif

/**
 * Receive ARP handler.
 * This processes incoming 'raw' ARP packets supplied by tcp_tick().
 */
BOOL W32_CALL _arp_handler (const arp_Header *ah, BOOL brdcast)
{
  struct arp_entry  *ae;
  const eth_address *e_src, *e_dst;   /* src/dest eth-addresses */
  DWORD src, dst;                     /* src/dest IP-addresses */
  WORD  hw_needed = intel16 (_eth_get_hwtype(NULL,NULL));
  BOOL  to_us, do_reply = FALSE;
  int   i;

  DEBUG_RX (NULL, ah);

  if (ah->hwType   != hw_needed ||  /* wrong hardware type, */
      ah->protType != IP4_TYPE)     /* or not IPv4-protocol */
  {
    TRACE (("Bogus ARP-header; hwType: %04X, protoType: %04X.\n",
            ah->hwAddrLen, ah->protoAddrLen));
    return (FALSE);
  }

  if (ah->hwAddrLen    != sizeof(mac_address) ||
      ah->protoAddrLen != sizeof(src))
  {
    TRACE (("Bogus ARP-header; hwAddrLen: %d, protoAddrLen: %d.\n",
            ah->hwAddrLen, ah->protoAddrLen));
    return (FALSE);
  }

  src = intel (ah->srcIPAddr);
  dst = intel (ah->dstIPAddr);

  e_src = &ah->srcEthAddr;
  e_dst = &ah->dstEthAddr;

  to_us = !memcmp(&_eth_addr,MAC_DST(ah),_eth_mac_len) ||
          !memcmp(&_eth_addr,e_dst,_eth_mac_len);

  /* Does someone else want our Ethernet address?
   */
  if (ah->opcode == ARP_REQUEST)
  {
    if (_ip4_is_local_addr(dst) &&
        !_ip4_is_multicast(dst) &&
        !_ip4_is_loopback_addr(dst))
       do_reply = TRUE;

    if (!to_us && !BROADCAST_MODE())
    {
      if (src == my_ip_addr)
         TRACE (("Address conflict from %s [%s]\n",
                 INET_NTOA(src), MAC_address(e_src)));

      /* Prevent anti-sniffers detecting us if we're not in normal rx-mode
       */
      if (memcmp(MAC_DST(ah),_eth_brdcast,sizeof(_eth_brdcast)))  /* not bcast */
         do_reply = FALSE;
    }

    if (do_reply)
       _arp_reply (e_src, dst, src);
  }

  /* See if the senders IP & MAC is anything we can use
   */

  /* Is this the awaited reply to a pending entry?
   */
  for (i = arp_first_pending; i < ARP_TOP_PENDING; i++)
  {
    ae = arp_list + i;
    if (ah->opcode != ARP_REPLY || ae->ip != src)
       continue;

    /* Remove from pending list
     */
    if (i > arp_first_pending)
       arp_move_entry (i, arp_first_pending); /* fill 'hole' */
    ++arp_first_pending;

    ae->flags &= ~(ARP_FLG_INUSE | ARP_FLG_PENDING);

    /* fill new dynamic entry
     * (at least one slot is free because we just freed a pending slot)
     */
#if 1
    _arp_cache_add (src, e_src, TRUE);
#else
    ae = arp_list + ARP_TOP_DYNAMIC;
    ARP_TOP_DYNAMIC++;
    ae->ip    = src;
    ae->flags = (ARP_FLG_INUSE | ARP_FLG_DYNAMIC);
    memcpy (&ae->hardware, e_src, sizeof(*e_src));
    ae->expiry = set_timeout (1000UL * arp_alive);
#endif

    TRACE (("Got ARP-reply from %s [%s]. No longer pending.\n",
            INET_NTOA(src), MAC_address(e_src)));
    return (TRUE);
  }

  /* Or is this a 'refresher' of a dynamic entry?
   * We'll use both ARP_REQUEST and ARP_REPLY to refresh.
   */
  if (ah->opcode != ARP_REQUEST && ah->opcode != ARP_REPLY)
     goto quit;

  for (i = arp_first_dynamic; i < ARP_TOP_DYNAMIC; i++)
  {
    BOOL  equal;
    DWORD timeout;

    ae = arp_list + i;
    if (ae->ip != src)
       continue;

    /* This could also be an 'ARP poisoning attack', where an attacker is
     * trying to slip us a fake MAC address.
     * Knowing that, we check if the MAC address has changed, and if so
     * prematurely expire the entry. We will re-request it when we need it.
     * If the MAC address is 'still' the same, we just restart the timeout.
     */
    equal = (memcmp(&ae->hardware, e_src, sizeof(*e_src)) == 0);

    /* if poisoned, we give the 'real guy' 500 ms grace to reclaim his MAC ;)
     */
    timeout = (equal ? (1000UL * arp_alive) : 500UL);
    ae->expiry = set_timeout (timeout);
    TRACE (("Got refreshed (%sequal) entry for %s [%s].\n",
            equal ? "" : "not ", INET_NTOA(src), MAC_address(e_src)));
    return (TRUE);
  }

  /* Add to cache if we replied.
   */
  if (do_reply)
  {
    _arp_cache_add (src, e_src, TRUE);
    TRACE (("Added entry for %s [%s]\n", INET_NTOA(src), MAC_address(e_src)));
    return (TRUE);
  }

  /* Most TCP stacks add any 'sniffed' ARP replies to their cache in case
   * they're needed later. But what are the odds? :)
   * Anyway, this could quickly fill the table with 'junk' entries on
   * heavily populated LANs; plus it makes us vulnerable to ARP-flooding
   * attacks ... so it's probably wiser to just ignore them.
   */

quit:
  if (src != my_ip_addr)
     TRACE (("ARP-packet from %s [%s] not handled.\n",
             INET_NTOA(src), MAC_address(e_src)));
  ARGSUSED (brdcast);
  return (FALSE);
}


#if defined(USE_DHCP)
/**
 * Used by DHCP initialisation.
 * Do an ARP resolve on our own IP address to check if someone
 * else is replying. Return non-zero if someone replied.
 * \note Blocks waiting for reply or timeout.
 */
BOOL W32_CALL _arp_check_own_ip (eth_address *other_guy)
{
  DWORD save = my_ip_addr;
  BOOL  rc;

  TRACE (("_arp_check_own_ip()\n"));

  my_ip_addr = 0;
  memset (other_guy, 0, sizeof(*other_guy));
  rc = _arp_resolve (save, other_guy);
  my_ip_addr = save;

  if (rc && memcmp(other_guy,(const void*)"\0\0\0\0\0\0",sizeof(*other_guy)))
     return (FALSE);
  return (TRUE);
}
#else
/* \todo: Add an empty stub */
#endif

/**
 * Return number of default gateways.
 */
int W32_CALL _arp_check_gateways (void)
{
  int i, num = 0;

  TRACE (("_arp_check_gateways()\n"));

  /* Send a gratiotous ARP. Don't if already done DHCP_arp_check().
   */
#if defined(USE_DHCP)
  if (DHCP_did_gratuitous_arp)
     arp_gratiotous = FALSE;
#endif

  if (arp_gratiotous)
     _arp_reply (NULL, my_ip_addr, IP_BCAST_ADDR);

  for (i = 0; i < gate_top; i++)
      if (gate_list[i].subnet == 0UL)
         num++;
  return (num);
}

#if defined(USE_DEBUG)
void W32_CALL _arp_cache_dump (void)
{
  const struct arp_entry *ae;
  DWORD  now = set_timeout (0);
  int    i, num = 0, max = _arp_cache_get (&ae);

  (*_printf) ("\nARP-cache:\n");

  for (i = 0; i < max; i++, ae++)
  {
    const char *remain;

    if (!(ae->flags & ARP_FLG_INUSE))
       continue;

    num++;
    if (ae->expiry)
    {
      if (ae->expiry > now)
      {
        char buf[30];
        sprintf (buf, "expires in %ss", time_str(ae->expiry-now));
        remain = buf;
      }
      else
        remain = "timed out";
    }
    else
      remain = "no expiry";

    (*_printf) ("      IP: %-15s -> %s, %s %s\n",
                INET_NTOA(ae->ip), MAC_address(&ae->hardware),
                (ae->flags & ARP_FLG_FIXED)   ? "Fixed,  " :
                (ae->flags & ARP_FLG_DYNAMIC) ? "Dynamic," :
                (ae->flags & ARP_FLG_PENDING) ? "Pending," : "??,     ",
                remain);
  }
  if (num == 0)
     (*_printf) ("      <Empty>\n");
}

void W32_CALL _arp_routes_dump (void)
{
  const struct route_entry *re = route_list;
  int   i;

  (*_printf) ("\nRoutes:\n"
              "   # GATEWAY         HOST            MASK            FLAGS\n");

  for (i = 0; i < DIM(route_list); i++, re++)
  {
    if (!(re->flags & ROUTE_FLG_USED))
       continue;
    (*_printf) ("  %2d %-15s %-15s %-15s %s\n",
                i, INET_NTOA(re->gate_ip), INET_NTOA(re->host_ip),
                INET_NTOA(re->mask), get_route_flags(re->flags));
  }
}

#if defined(USE_BSD_API)
  /*
   * In ioctl.c
   */
  extern void __get_ifname (char *if_name);
#else
  /*
   * Just fake.
   */
  #define __get_ifname(var) strcpy (var, "eth0")
#endif


void W32_CALL _arp_gateways_dump (void)
{
  const struct gate_entry *gw = gate_list;
  int   i;

  (*_printf) ("\nGateways:\n");
  if (gate_top <= 0)
  {
    (*_printf) ("NONE\n");
    return;
  }

#if 0
  (*_printf) ("     GATEWAY'S IP     SUBNET           SUBNET MASK\n");

  for (i = 0 ; i < gate_top; i++, gw++)
  {
    char gate[20], subnet[20], mask[20];

    strcpy (gate, INET_NTOA(gw->gate_ip));

    if (gw->subnet)
         strcpy (subnet, INET_NTOA(gw->subnet));
    else strcpy (subnet, "0.0.0.0 (def)");

    if (gw->mask)
         strcpy (mask, INET_NTOA(gw->mask));
    else strcpy (mask, INET_NTOA(sin_mask)), strcat (mask," (def)");

    (*_printf) ("     %-15s  %-15s  %-15s\n", gate, subnet, mask);
  }
#else
/*
   Print it like "nmap --iflist" does:

    **************************ROUTES**************************
    DST/MASK           DEV  GATEWAY
    255.255.255.255/32 eth1 10.0.0.6      my_ip
    10.0.0.6/32        lo0  127.0.0.1     loopback network
    10.255.255.255/32  eth1 10.0.0.6      all 1 broadcast
    255.255.255.255/32 eth1 10.0.0.6      directed broadcast
    10.0.0.0/24        eth1 10.0.0.6
    127.0.0.0/8        lo0  127.0.0.1
    224.0.0.0/4        eth1 10.0.0.6
    0.0.0.0/0          eth1 10.0.0.1
 */
  (*_printf) ("     DEST/MASK        DEV   GATEWAY         (MASK)\n");

  for (i = 0 ; i < gate_top; i++, gw++)
  {
    DWORD mask = gw->mask; // ? gw->mask : sin_mask;
    char *subnet = INET_NTOA(gw->subnet);
    char  ifname[10];
    int   padding, mlen = mask_len(mask);

    __get_ifname (ifname);
    padding = 14 - strlen(subnet);
    if (mlen > 9)
       padding--;

    (*_printf) ("     %s/%d%*s %s  %-15s (%s)\n",
                subnet, mlen, padding, "", ifname,
                INET_NTOA(gw->gate_ip), INET_NTOA(mask));
  }
#endif
}

/**
 * Debug-dump of configured gateways, route table and ARP cache.
 * Redone + moved here from pcdbug.c for encapsulation reasons
 * GvB 2002-09
 */
void W32_CALL _arp_debug_dump (void)
{
  const struct arp_entry   *ae;
  const struct gate_entry  *gw;
  const struct route_entry *re;
  DWORD now = set_timeout (0UL);
  DWORD mask;
  int   i;

  /* Gateways
   */
  if (!dbug_printf ("Gateway list:\n"))
     return;

  if (gate_top == 0)
  {
    dbug_printf ("  --none--\n");
  }
  else for (i = 0; i < gate_top; i++)
  {
    gw = gate_list + i;
    mask = gw->mask ? gw->mask : sin_mask;
    dbug_printf ("  #%03d: %-15s ", i, INET_NTOA(gw->gate_ip));
    dbug_printf ("(network: %-15s ", INET_NTOA(gw->subnet));
    dbug_printf ("mask: %s)\n"   , INET_NTOA(mask));
  }

  /* Route table
   */
  dbug_printf ("\nRouting cache:\n"
               "------- top of cache -----------------------------------------------\n"
               "  (%03d) top of pending slots ---------------------------------------\n",
               ROUTE_TOP_PENDING);

  if (route_first_pending == ROUTE_TOP_PENDING)
  {
    dbug_printf ("        --none--\n");
  }
  else if (route_first_pending > ROUTE_TOP_PENDING)
  {
    dbug_printf ("  INDEX ERROR!\n");
  }
  else for (i = ROUTE_TOP_PENDING-1; i >= route_first_pending; i--)
  {
    re = route_list + i;
    dbug_printf ("  #%03d: IP: %-15s -> gateway IP %-15s\n",
                 i, INET_NTOA(re->host_ip), INET_NTOA(re->gate_ip));
  }

  dbug_printf ("- (%03d) top of free slots ------------------------------------------\n",
               route_top_free);

  if (route_first_free == route_top_free)
  {
    dbug_printf ("  --none--\n");
  }
  else if (route_first_free > route_top_free)
  {
    dbug_printf ("  INDEX ERROR!\n");
  }
  else if (route_top_free - route_first_free <= 3)
  {
    for (i = route_top_free-1; i >= route_first_free; i--)
        dbug_printf ("  #%03d: (free)\n", i);
  }
  else
  {
    dbug_printf ("  #%03d: (free)\n", route_top_free-1);
    dbug_printf ("   ...  (free)\n");
    dbug_printf ("  #%03d: (free)\n", route_first_free);
  }

  dbug_printf ("- (%03d) top of dynamic slots ---------------------------------------\n",
               route_top_dynamic);
  if (ROUTE_FIRST_DYNAMIC == route_top_dynamic)
  {
    dbug_printf ("        --none--\n");
  }
  else if (ROUTE_FIRST_DYNAMIC > route_top_dynamic)
  {
    dbug_printf ("  INDEX ERROR!\n");
  }
  else for (i = route_top_dynamic-1; i >= ROUTE_FIRST_DYNAMIC; i--)
  {
    re = route_list + i;
    dbug_printf ("  #%03d: IP: %-15s -> gateway IP %-15s\n",
                 i, INET_NTOA(re->host_ip), INET_NTOA(re->gate_ip));
  }
  dbug_printf ("------- bottom of cache --------------------------------------------\n");

  /* ARP table
   */
  dbug_printf ("\nARP cache:\n"
               "------- top of cache -----------------------------------------------\n"
               "  (%03d) top of pending slots ---------------------------------------\n",
               ARP_TOP_PENDING);

  if (arp_first_pending == ARP_TOP_PENDING)
  {
    dbug_printf ("        --none--\n");
  }
  else if (arp_first_pending > ARP_TOP_PENDING)
  {
    dbug_printf ("  INDEX ERROR!\n");
  }
  else for (i = ARP_TOP_PENDING-1; i >= arp_first_pending; i--)
  {
    ae = arp_list + i;
    dbug_printf ("  #%03d: IP: %-15s -> ??:??:??:??:??:??  expires in %ss\n",
                 i, INET_NTOA(ae->ip), time_str(ae->expiry - now));
  }

  dbug_printf ("- (%03d) top of free slots ------------------------------------------\n",
               ARP_TOP_FREE);
  if (arp_first_free == ARP_TOP_FREE)
  {
    dbug_printf ("  --none--\n");
  }
  else if (arp_first_free > ARP_TOP_FREE)
  {
    dbug_printf ("  INDEX ERROR!\n");
  }
  else if (ARP_TOP_FREE - arp_first_free <= 3)
  {
    for (i = ARP_TOP_FREE-1; i >= arp_first_free; i--)
        dbug_printf ("  #%03d: (free)\n", i);
  }
  else
  {
    dbug_printf ("  #%03d: (free)\n", ARP_TOP_FREE-1);
    dbug_printf ("   ...  (free)\n");
    dbug_printf ("  #%03d: (free)\n", arp_first_free);
  }

  dbug_printf ("- (%03d) top of dynamic slots ---------------------------------------\n",
               ARP_TOP_DYNAMIC);
  if (arp_first_dynamic == ARP_TOP_DYNAMIC)
  {
    dbug_printf ("        --none--\n");
  }
  else if (arp_first_dynamic > ARP_TOP_DYNAMIC)
  {
    dbug_printf ("  INDEX ERROR!\n");
  }
  else for (i = ARP_TOP_DYNAMIC-1; i >= arp_first_dynamic; i--)
  {
    ae = arp_list + i;
    dbug_printf ("  #%03d: IP: %-15s -> %s  expires in %ss\n",
                 i, INET_NTOA(ae->ip), MAC_address(&ae->hardware),
                 time_str(ae->expiry - now));
  }

  dbug_printf ("- (%03d) top of fixed slots -----------------------------------------\n",
               ARP_TOP_FIXED);
  if (ARP_FIRST_FIXED == ARP_TOP_FIXED)
  {
    dbug_printf ("        --none--\n");
  }
  else if (ARP_FIRST_FIXED > ARP_TOP_FIXED)
  {
    dbug_printf ("  INDEX ERROR!\n");
  }
  else for (i = ARP_TOP_FIXED-1; i >= ARP_FIRST_FIXED; i--)
  {
    ae = arp_list + i;
    dbug_printf ("  #%03d: IP: %-15s -> %s\n",
                 i, INET_NTOA(ae->ip), MAC_address(&ae->hardware));
  }
  dbug_printf ("------- bottom of cache --------------------------------------------\n");
}

static const char *get_ARP_flags (WORD flg)
{
  static char buf[50];
  char  *p;

  buf[0] = '\0';
  if (flg & ARP_FLG_INUSE)
     strcat (buf, "INUSE,");
  if (flg & ARP_FLG_PENDING)
     strcat (buf, "PEND,");
  if (flg & ARP_FLG_DYNAMIC)
     strcat (buf, "DYN,");
  if (flg & ARP_FLG_FIXED)
     strcat (buf, "FIXED,");
  p = strrchr (buf,',');
  if (p)
     *p = '\0';
  return (buf);
}

static const char *get_route_flags (WORD flg)
{
  static char buf[50];
  char  *p;

  buf[0] = '\0';
  if (flg & ROUTE_FLG_USED)
     strcat (buf, "INUSE, ");
  if (flg & ROUTE_FLG_PENDING)
     strcat (buf, "PEND, ");
  if (flg & ROUTE_FLG_DYNAMIC)
     strcat (buf, "DYN, ");
  if (flg & ROUTE_FLG_FIXED)
     strcat (buf, "FIXED,");
  p = strrchr (buf,',');
  if (p)
     *p = '\0';
  return (buf);
}

#else
  W32_CALL void W32_CALL _arp_cache_dump (void) {}
  W32_CALL void W32_CALL _arp_gateways_dump (void) {}
  W32_CALL void W32_CALL _arp_routes_dump (void) {}
  W32_CALL void W32_CALL _arp_debug_dump (void) {}
#endif  /* USE_DEBUG */


#if defined(TEST_PROG)

#if defined(__WATCOMC__)
  #pragma disable_message (202)
#endif

/*
 * Find the best 'fitting' gateway for destination IP.
 * Return INADDR_ANY if 'ip' is directly reachable.
 */
static DWORD _route_destin (DWORD ip)
{
  DWORD rc = 0;
  int   i;

  TRACE (("_route_destin (%s)\n", INET_NTOA(ip)));

  for (i = 0; i < gate_top; i++)
  {
    const struct gate_entry *gw = gate_list + i;

    TRACE (("  i %d, gw->gate_ip %s, gw->subnet %s, gw->mask %s\n"
            "       ON_SAME_NETWORK: %d\n", i,
            INET_NTOA(gw->gate_ip), INET_NTOA(gw->subnet),
            INET_NTOA(gw->mask), ON_SAME_NETWORK(gw->subnet,ip,gw->mask)));
#if 0
    if (sin_mask != IP_BCAST_ADDR && !is_on_LAN(gw->gate_ip))
       continue;
    if (!ON_SAME_NETWORK(gw->subnet,ip,gw->mask))
       continue;
#else
    if (((ip ^ gw->subnet) & gw->mask) == 0)  /* IP on same subnet as gw->gate_ip */
       return (gw->gate_ip);
#endif
  }
//  if (rc)
       return (rc);
//  return (is_on_LAN(ip) ? INADDR_ANY : INADDR_NONE);
}


#include "pcdns.h"
#include "pcbuf.h"

#if !defined(__DJGPP__)
  W32_GCC_PRAGMA (GCC diagnostic ignored "-Wunused-variable")
  W32_GCC_PRAGMA (GCC diagnostic ignored "-Wunused-function")

  static int num_okay = 0;
  static int num_fail = 0;
#endif

#define TEST(func, args, expect) do {                                   \
                                   HIGH_TEXT();                         \
                                   (*_printf) ("%s() ", #func);         \
                                   NORM_TEXT();                         \
                                   if (func args == expect) {           \
                                     num_okay++;                        \
                                     YELLOW_TEXT();                     \
                                     (*_printf) ("OK\n");               \
                                   } else {                             \
                                     num_fail++;                        \
                                     RED_TEXT();                        \
                                     (*_printf) ("FAIL\n");             \
                                   }                                    \
                                   NORM_TEXT();                         \
                                 } while (0)

#define TEST_EQUAL(func, args, expect) do {                             \
                                   int rc;                              \
                                   HIGH_TEXT();                         \
                                   (*_printf) ("%s() ", #func);         \
                                   NORM_TEXT();                         \
                                   rc = func args;                      \
                                   if (rc == expect) {                  \
                                     num_okay++;                        \
                                     (*_printf) ("OK\n");               \
                                   } else {                             \
                                     num_fail++;                        \
                                    (*_printf) ("FAIL. Got %d\n", rc);  \
                                   }                                    \
                                 } while (0)

#define TEST_ROUTE(dest) do {                                       \
                           DWORD ip = _route_destin (aton(dest));   \
                           HIGH_TEXT();                             \
                           (*_printf) ("Best route for %s is %s\n", \
                                       dest, INET_NTOA(ip));        \
                           NORM_TEXT();                             \
                         } while (0)

static void W32_CALL callback (void)
{
  static char fan[] = "\\|/-";
  static int  idx = 0;

  if (_watt_cbroke)
  {
    RED_TEXT(); (*_printf) ("^C/^Break detected\n");
    NORM_TEXT();
    exit (-1);
  }
  (*_outch) (fan[idx++]);
  (*_outch) ('\b');
  idx &= 3;
}

int main (int argc, char **argv)
{
  sock_type   sock;
  const char *host = "www.google.com";
  int         status = 0;
  DWORD       ip;
  char        buf[1000];
  int         len;
  mac_address eth1, eth2, eth3;

  debug_on = 0;
  dbug_init();
  sock_init();

#if 0
// arp_trace_level = 0;
  _arp_kill_gateways();

  /*                        Gateway    Subnet     Mask     */
  TEST (_arp_add_gateway, ("127.0.0.1, 127.0.0.0, 255.0.0.0",           0UL), TRUE);
  TEST (_arp_add_gateway, ("10.0.0.10,  10.0.0.2, 255.255.255.255",     0UL), TRUE);   /* special LAN rule */
  TEST (_arp_add_gateway, ("10.0.0.1",                                  0UL), TRUE);   /* default route (Class A-mask) */
  TEST (_arp_add_gateway, ("10.0.0.4,  162.100.102.0, 255.255.255.248", 0UL), TRUE);   /* special route */

  arp_gateways_dump();

  TEST (legal_mask, (aton("255.255.255.0"), aton("127.0.0.0")), FALSE /*TRUE*/);
  TEST (legal_mask, (aton("128.255.255.0"), aton("127.0.0.0")), FALSE);

  TEST (check_mask2, ("255.255.255.128"), 1);
  TEST (check_mask2, ("255.255.255.127"), 0);
  TEST (check_mask2, ("0.0.0.1"),         0);
  TEST (check_mask2, ("255.255.255.2"),   0);
  TEST (check_mask2, ("255.255.255.254"), 1);
  TEST (check_mask2, ("127.255.255.255"), 0);

  TEST_EQUAL (mask_len, (aton("255.255.255.0")), 8);
  TEST_EQUAL (mask_len, (aton("255.255.0.0")),  16);
  TEST_EQUAL (mask_len, (aton("255.0.0.0")),    24);
  TEST_EQUAL (mask_len, (aton("0.0.0.0")),      32);

  TEST_ROUTE ("10.0.0.1");
  TEST_ROUTE ("10.0.0.2");
  TEST_ROUTE ("10.0.0.3");
  TEST_ROUTE ("11.0.0.1");
  TEST_ROUTE ("100.101.102.103");
  TEST_ROUTE ("162.100.102.1");
  TEST_ROUTE ("127.22.0.5");

//arp_trace_level = 4;
  TEST (_arp_resolve, (aton("10.0.0.1"), &eth1), TRUE);
  (*_printf) (" -> %s\n", MAC_address(&eth1));

  TEST (_arp_resolve, (def_nameservers[0], &eth2), TRUE);
  (*_printf) (" -> %s\n", MAC_address(&eth2));

  TEST (_arp_resolve, (aton("55.44.33.22"), &eth3), TRUE);
  (*_printf) (" -> %s\n", MAC_address(&eth3));

  TEST (memcmp, (&eth1, &eth2, sizeof(eth1)), 0);
  TEST (memcmp, (&eth1, &eth3, sizeof(eth1)), 0);

  (*_printf) ("\nTEST RESULT: %d okay, %d fail\n", num_okay, num_fail);

  _arp_cache_dump();

#else
  ip = lookup_host (host, NULL);
  if (!ip)
  {
    printf ("%s", dom_strerror(dom_errno));
    return (1);
  }
  if (tcp_open(&sock.tcp, 0, ip, 80, NULL))
  {
    sock_yield (&sock.tcp, callback);
    sock_wait_established (&sock, sock_delay, NULL, &status);
    printf ("Connected. Awaiting response...");
    fflush (stdout);
    sock_puts (&sock, (const BYTE*)
                      "GET /index.html HTTP/1.0\r\n"
                      "User-Agent: pcarp test\r\n\r\n");

    while ((len = sock_read(&sock,(BYTE*)buf,sizeof(buf))) > 0)
         fwrite (buf, len, 1, stdout);
  }

sock_err:
  if (status == -1)
    printf ("Cannot connect to %s: %s\n", host, sockerr(&sock));

  ARGSUSED (eth1);
  ARGSUSED (eth2);
  ARGSUSED (eth3);
#endif

  ARGSUSED (argc);
  ARGSUSED (argv);

  return (status);
}
#endif /* TEST_PROG */
