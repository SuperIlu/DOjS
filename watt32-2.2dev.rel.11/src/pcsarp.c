/*!\file pcsarp.c
 *
 * Secure ARP client.
 *
 * March 2004-09: created. Partly based on Linux version by
 *                Albero Ornaghi <alor@blackhats.it>
 *                http://alor.antifork.org/projects/s-arp/
 *                http://www.acsac.org/2003/papers/111.pdf
 *
 * This is mostly an excersise in OpenSSL programming. Don't expect
 * it to make sense since S-ARP is completely non-standard (no RFC).
 * Only Linux has a kernel patch to use S-ARP.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wattcp.h"
#include "strings.h"
#include "netaddr.h"
#include "misc.h"
#include "pcdbug.h"
#include "pcconfig.h"
#include "pctcp.h"
#include "pcsed.h"
#include "pcpkt.h"
#include "pcarp.h"
#include "pcsarp.h"


#if defined(USE_SECURE_ARP)
/*
 * OpenSSL include path should be in %INCLUDE% or %C_INCLUDE_PATH%.
 */
#include <openssl/dsa.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#if defined(USE_DEBUG)
  #define SARP_DEBUG(level, args)      \
          do {                         \
            if (sarp_debug >= level) { \
               (*_printf) args;        \
               fflush (stdout);        \
            }                          \
          } while (0)
#else
  #define SARP_DEBUG(args, level)  ((void)0)
#endif

struct host_list {
       DWORD             ip;     /* on host order */
       eth_address       mac;
       BOOL              secure;
       DSA              *dsa;
       struct host_list *next;
     };

static void (W32_CALL *prev_cfg_hook) (const char*, const char*);
static void  W32_CALL  sarp_parse (const char *name, const char *value);

static char  *ca_keyfile   = NULL;
static char  *priv_keyfile = NULL;
static int    sarp_debug   = 0;

static struct host_list *known_hosts  = NULL;
static struct host_list *secure_hosts = NULL;

static int  sarp_receive  (const struct sarp_Packet *sarp);
static int  sarp_transmit (struct sarp_Packet *sarp);

/**
 * Application must call this *before* sock_init() so we don't link
 * in OpenSSL libs by default.
 */
int sarp_init (void)
{
  const char *rand = getenv ("RANDFILE");

  prev_cfg_hook   = usr_init;
  usr_init        = sarp_parse;
  _sarp_recv_hook = sarp_receive;
  _sarp_xmit_hook = sarp_transmit;

  if (rand && FILE_EXIST(rand))
       RAND_load_file (rand, -1);
  else SARP_DEBUG (0, ("Warning: No random seed file found\n"));
  return (0);
}

static int read_ca_keyfile (const char *value)
{
  DSA *dsa;

  DSA_free (dsa);
}

static int read_priv_keyfile (const char *value)
{
  FILE *fil;

  ca_keyfile = strdup (value);
  if (!ca_keyfile)
     return (0);

  fil = fopen (ca_keyfile, "rt");
  if (!fil)
     return (0);

#if 0
  priv = d2i_RSAPrivateKey_fp (fil, NULL);
#endif
  fclose (fil);
  return (1);
}

static BOOL crypto_load_sarp_file (const char *pem_file, DSA **dsa)
{
  FILE *fil = fopen (pem_file, "rt");

  *dsa = d2i_DSAPublicKey (NULL, raw, len);

  if (!crypto_verify_sign(message,strlen(message),sig,siglen,*dsa))
  {
    SARP_DEBUG (0, ("%s has illegal signature\n", pem_file));
    DSA_free (*dsa);
    return (FALSE);
  }
}

/*
 * Parse "SECURE_HOST" 'value' "<IP>, <MAC>, <public-key-file>"
 */
static int set_secure_host (const char *value)
{
  struct host_list *host;
  char   ip[16], mac[21], pem_file[256];
  DWORD  ip_addr;
  DSA   *dsa;

  if (sscanf(value,"%15[^,],%20[^,],%255[^\r\n]", ip, mac, pem_file) != 3)
     return (0);

  ip_addr = _inet_addr (ip);
  if (!ip_addr)
     return (0);

  host = calloc (sizeof(*host), 1);
  if (!host)
     return (0);

  if (!_inet_atoeth(mac, &host->mac) ||
      !crypto_load_sarp_file(pem_file, &dsa))
  {
    free (host);
    return (0);
  }

  host->secure = TRUE;
  host->ip     = ip_addr;
  host->dsa    = dsa;
  host->next   = secure_hosts;
  secure_hosts = host;
  return (1);
}

/*
 * Parse "KNOWN_HOST" 'value' "<IP>, <MAC>"
 */
static int set_known_host (const char *value, int value_len)
{
  struct host_list *host;
  char   ip[16], mac[21];

  if (sscanf(value,"%15[^,],%20[^\r\n]", ip, mac) != 2)
     return (0);

  host = calloc (sizeof(*host), 1);
  if (!host)
     return (0);

  if (!_inet_atoeth (mac, &host->mac))
  {
    free (host);
    return (0);
  }

  host->secure = FALSE;
  host->ip     = _inet_addr (ip);
  host->next  =  known_hosts;
  known_hosts = host;
  return (1);
}


/**
 * Parser for "SEC_ARP.xx" keywords in WATTCP.CFG (or SARP.CFG).
 */
static void W32_CALL sarp_parse (const char *name, const char *value)
{
  static const struct config_table sarp_cfg[] = {
         { "CA_KEY",      ARG_FUNC, (void*)read_ca_keyfile   },
         { "PRIV_KEY",    ARG_FUNC, (void*)read_priv_keyfile },
         { "SECURE_HOST", ARG_FUNC, (void*)set_secure_host   },
         { "KNOWN_HOST",  ARG_FUNC, (void*)set_known_host    },
         { NULL,          0,        NULL                     }
       };

  if (!parse_config_table(&sarp_cfg[0], "SEC_ARP.", name, value) &&
      prev_cfg_hook)
     (*prev_cfg_hook) (name, value);
}

/**
 * Check if host should use secure ARP.
 */
static __inline BOOL is_secure_host (DWORD ip)
{
  const struct host_list *host;

  for (host = secure_hosts; host; host = host->next)
      if (host->ip == ip)
         return (host->dsa != NULL);
  return (FALSE);
}

/**
 * Check if host is among the known hosts.
 */
static __inline BOOL is_known_host (DWORD ip)
{
  const struct host_list *host;

  for (host = known_hosts; host; host = host->next)
      if (host->ip == ip)
         return (TRUE);
  return (FALSE);
}

static int make_auth_Header (struct sarp_Packet *sarp)
{
  ARGSUSED (sarp);
  return (0);
}

/*
 * Called from _arp_handler() after HW-type and protocol (IPv4) is
 * verified. Must completely replace _arp_handler().
 */
static int sarp_receive (const struct sarp_Packet *sarp)
{
  DWORD src_ip = ntohl (sarp->arp.srcIPAddr);

  if (!is_secure_host(src_ip) && !is_known_host(src_ip))
     return (0);   /* drop it */

  /** \todo */
  return (1);
}

/*
 * Called from arp_send(). Add any authentication header as needed.
 * Return total length of what's need to be transmitted.
 */
static int sarp_transmit (struct sarp_Packet *sarp)
{
  DWORD dst_ip = ntohl (sarp->arp.dstIPAddr);
  WORD  len    = sizeof (sarp->arp);

  if (!is_secure_host(dst_ip))
     return (len);   /* send as-is */

  memset (&sarp->auth, 0, sizeof(sarp->auth));
  len += make_auth_Header (sarp);
  return (len);
}

/**
 * Secure-ARP debug dump
 */
void _sarp_debug_dump (void)
{
}
#endif  /* USE_SECURE_ARP */
