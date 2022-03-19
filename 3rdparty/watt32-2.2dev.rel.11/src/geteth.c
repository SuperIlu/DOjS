/*!\file geteth.c
 *
 *  `/etc/ethers' file functions for Watt-32.
 */

/*  Copyright (c) 1997-2002 Gisle Vanem <gvanem@yahoo.no>
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. All advertising materials mentioning features or use of this software
 *     must display the following acknowledgement:
 *       This product includes software developed by Gisle Vanem
 *       Bergen, Norway.
 *
 *  THIS SOFTWARE IS PROVIDED BY ME (Gisle Vanem) AND CONTRIBUTORS ``AS IS''
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL I OR CONTRIBUTORS BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 *  28.apr 2000 - Created
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wattcp.h"
#include "misc.h"
#include "run.h"
#include "pcarp.h"
#include "pcconfig.h"
#include "pcdns.h"
#include "pcdbug.h"
#include "strings.h"
#include "netaddr.h"
#include "bsddbug.h"
#include "get_xby.h"

#if defined(USE_BSD_API)

#include <net/if.h>
#include <net/if_arp.h>
#include <net/if_dl.h>
#include <net/if_ether.h>
#include <net/route.h>

#if defined(TEST_PROG)
  #undef  SOCK_DEBUGF
  #define SOCK_DEBUGF(args)  printf args
#endif

#if !defined(USE_BUFFERED_IO)
  #error This file needs USE_BUFFERED_IO
#endif

struct ethent {
       eth_address    eth_addr;  /* ether/MAC address of host */
       DWORD          ip_addr;   /* IP-address (host order) */
       char          *name;      /* host-name for IP-address */
       struct ethent *next;
     };

static struct ethent *eth0 = NULL;
static int    num_entries  = -1;
static char   ethersFname [MAX_PATHLEN] = "";

static void W32_CALL end_ether_entries (void);
static int           get_ether_entry (char *in_buf, eth_address *e,
                                      char *out_buf, size_t out_buf_size);

void W32_CALL InitEthersFile (const char *fname)
{
  if (fname && *fname)
     _strlcpy (ethersFname, fname, sizeof(ethersFname));
}

const char * W32_CALL GetEthersFile (void)
{
  return (ethersFname[0] ? ethersFname : NULL);
}

int W32_CALL NumEtherEntries (void)
{
  return (num_entries);
}

/**
 * Read the /etc/ethers file. This MUST be called after any /etc/hosts
 * file has been read.
 * \todo: Assert that.
 */
void W32_CALL ReadEthersFile (void)
{
  FILE *file;
  char  buf [2*MAX_HOSTLEN];

  if (!ethersFname[0])
     return;

  if (!FOPEN_TXT(file, ethersFname))
  {
    netdb_warn (ethersFname);
    return;
  }

  while (fgets(buf,sizeof(buf),file))
  {
    struct ethent  *e;
    struct hostent *h;
    char   host_ip [MAX_HOSTLEN];
    int    save;
    eth_address eth;

    if (!get_ether_entry(buf,&eth,host_ip,sizeof(host_ip)))
       continue;

    if (num_entries == -1)
       num_entries = 0;
    num_entries++;

    save = called_from_resolve;
    called_from_resolve = TRUE;   /* prevent a DNS lookup */
    h = gethostbyname (host_ip);
    called_from_resolve = save;

    /* If 'h == NULL': means 'host_ip' contained a host-name that cannot be resolved
     *                 at this moment. gethostbyname()+resolve() cannot be called at startup.
     * If 'h != NULL': means 'host_ip' is simply an IPv4-address or a host-name (or alias)
     *                 that is in '/etc/hosts' file.
     */
    if (!h)
    {
      CONSOLE_MSG (4, ("ReadEthersFile(): gethostbyname() failed\n"));
      continue;
    }
    CONSOLE_MSG (4, ("\n"));

    e = calloc (sizeof(*e), 1);
    if (!e)
       break;

    e->ip_addr = ntohl (*(DWORD*)h->h_addr);
    memcpy (&e->eth_addr, &eth, sizeof(e->eth_addr));

    /* Add this to the permanent ARP-cache.
     */
    _arp_cache_add (e->ip_addr, &e->eth_addr, FALSE);

    if (h->h_name)
    {
      e->name = strdup (h->h_name);
      if (!e->name)
         break;
    }
    e->next = eth0;
    eth0 = e;
  }

  FCLOSE (file);
  RUNDOWN_ADD (end_ether_entries, 250);
}

/*
 * Parse a string of text containing an ethernet address and
 * hostname/IP-address and separate it into its component parts.
 * E.g.
 *   in_buf -> "88-87-17-17-5a-3e  10.0.0.4"
 *   in_buf -> "E0-CA-94-3D-74-F0  printer"
 */
#define MIN_LEN  sizeof("0:0:0:0:0:0 a.b.c.d")

static int get_ether_entry (char *in_buf, eth_address *e,
                            char *name, size_t name_max)
{
  size_t   len, i;
  unsigned eth [sizeof(*e)];
  BOOL     ok, colon;
  char    *token = strltrim (in_buf);
  char    *ip_name;
  char    *tok_buf = NULL;

  if (*token == '#' || *token == ';' || *token == '\n' || strlen(token) < MIN_LEN)
     return (0);

  colon = (token[2] == ':');

  ok = colon ? (sscanf(token, "%02x:%02x:%02x:%02x:%02x:%02x",
                       &eth[0], &eth[1], &eth[2],
                       &eth[3], &eth[4], &eth[5]) == DIM(eth)) :
               (sscanf(token, "%02x-%02x-%02x-%02x-%02x-%02x",
                       &eth[0], &eth[1], &eth[2],
                       &eth[3], &eth[4], &eth[5]) == DIM(eth));
  if (!ok)
  {
    CONSOLE_MSG (1, ("get_ether_entry(): sscanf() failed\n"));
    return (0);
  }

  token   = strtok_r (token, " \t", &tok_buf);
  ip_name = strtok_r (NULL, " #\t\n", &tok_buf);

  if (!token || !ip_name || (len = strlen(ip_name)) < 1 || len > name_max)
  {
    CONSOLE_MSG (1, ("get_ether_entry(): short line or malformed ip_name '%s'\n", ip_name));
    return (0);
  }

  ip_name = (char*) expand_var_str (ip_name);

  _strlcpy (name, ip_name, name_max);

  for (i = 0; i < sizeof(*e); i++)
      ((BYTE*)e)[i] = eth[i];

  CONSOLE_MSG (4, ("get_ether_entry(): ip: %s, eth: %02X:%02X:%02X:%02X:%02X:%02X\n",
                   ip_name, eth[0],eth[1],eth[2],eth[3],eth[4],eth[5]));

  return (1);
}

/*
 * Free memory in 'eth0' array
 */
static void W32_CALL end_ether_entries (void)
{
  struct ethent *e, *next;

  if (_watt_fatal_error)
     return;

  for (e = eth0; e; e = next)
  {
    DO_FREE (e->name);
    next = e->next;
    free (e);
  }
  eth0 = NULL;
  num_entries = 0;
}

void W32_CALL DumpEthersCache (void)
{
  const struct ethent *e;

  if (!ethersFname[0])
  {
    SOCK_DEBUGF (("No ETHERS line found in WATTCP.CFG\n"));
    return;
  }

  SOCK_DEBUGF (("\n%s entries:\n", ethersFname));
  for (e = eth0; e; e = e->next)
      SOCK_DEBUGF (("  %s = %-15.15s (%s)\n",
                    MAC_address(&e->eth_addr), _inet_ntoa(NULL,e->ip_addr),
                    e->name ? e->name : "none"));
}
#endif  /* USE_BSD_API */

/*
 * Find the name associated with ether-addr 'eth'.
 * Returns null if not found in 'eth0' list.
 */
const char * W32_CALL GetEtherName (const eth_address *eth)
{
#if defined(USE_BSD_API)
  const struct ethent *e;

  for (e = eth0; e; e = e->next)
     if (!memcmp(eth, e->eth_addr, sizeof(*eth)))
        return (e->name);
#endif
  ARGSUSED (eth);
  return  (NULL);
}

#if defined(TEST_PROG)

#include "pcdbug.h"
#include "sock_ini.h"

int main (void)
{
#if defined(USE_BSD_API)
  debug_on = 2;
  dbug_init();
  sock_init();
  DumpEthersCache();

#else
  puts ("\"USE_BSD_API\" not defined");
#endif /* USE_BSD_API */
  return (0);
}
#endif /* TEST_PROG */
