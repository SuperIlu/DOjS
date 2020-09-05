/*!\file dynip.c
 *
 * Simple dynamic IP/hostname update client.
 * Only updating using the DynDNS2 protocol is supported.
 *
 * This code is based on the ddclient Perl-script by Paul Burry:
 * http://members.rogers.com/ddclient/pub/ddclient.tar.gz
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <netdb.h>

#include "wattcp.h"
#include "strings.h"
#include "language.h"
#include "misc.h"
#include "run.h"
#include "pcconfig.h"
#include "pcbuf.h"
#include "pctcp.h"
#include "pcdns.h"
#include "netaddr.h"
#include "dynip.h"

#if defined(USE_DYNIP_CLI)

#if defined(SNPRINTF)
  #define BUF(buf)   buf, sizeof(buf)
  #define _SNPRINTF  SNPRINTF

#else
  #define BUF(buf)   buf
  #define _SNPRINTF  sprintf
#endif

#if defined(USE_DEBUG)
  #define TRACE(level,arg)                              \
          do {                                          \
            if (trace_level >= level && dynip_enable) { \
               (*_printf) arg;                          \
               fflush (stdout);                         \
            }                                           \
          } while (0)
#else
  #define TRACE(level,arg)  ((void)0)
#endif

struct URL {
       char *host;
       char *get_req;
       WORD  port;
       BOOL  on_heap;
     };

static char dyndns_user    [MAX_VALUELEN+1] = "";
static char dyndns_passwd  [MAX_VALUELEN+1] = "";
static char dyn_myhostname [MAX_VALUELEN+1] = "";
static char dyn_myip [20]  = "";
static BOOL dynip_enable   = FALSE;
static int  dyndns_refresh = 3600;
static int  trace_level    = 0;
static char config_file [MAX_PATHLEN+1] = "$(TEMP)\\W32DYNIP.TMP";

static struct URL dyndns = { (char*) "members.dyndns.com",
                             (char*) "/nic/update?system=dyndns&hostname=%s",
                             80, FALSE
                           };
static struct URL chkip  = { NULL, NULL, 0, FALSE };

static char   resp_buf [2048];
static void (W32_CALL *prev_hook) (const char*, const char*);

static int get_url (const char *host, int port, const char *path,
                    const char *user_pass);

static const char *parse_myip_address (const char *buf);
static int         parse_dyndns_response (const char *buf);
static int         chkip_method (const char *value);
static int         dyndns_params (const char *value);
static void        url_free (struct URL *url);
static BOOL        url_parse (struct URL *res, const char *url);


/**
 * Parser for DYNIP configuration.
 * Matches all "\c DYNIP.xx" values from WATTCP.CFG file and
 * make appropriate actions.
 */
static void W32_CALL dynip_config (const char *name, const char *value)
{
  static const struct config_table dynip_cfg[] = {
       { "ENABLE",       ARG_ATOI,   (void*) &dynip_enable   },
       { "MY_IPADDRESS", ARG_FUNC,   (void*) chkip_method    },
       { "MY_HOSTNAME",  ARG_STRCPY, (void*) &dyn_myhostname },
       { "UPDATE",       ARG_FUNC,   (void*) dyndns_params   },
       { "USER",         ARG_STRCPY, (void*) &dyndns_user    },
       { "PASSWD",       ARG_STRCPY, (void*) &dyndns_passwd  },
       { "REFRESH",      ARG_ATOI,   (void*) &dyndns_refresh },
       { "TRACE",        ARG_ATOI,   (void*) &trace_level    },
       { "CONFIG",       ARG_STRCPY, (void*) &config_file    }, /* \todo */
       { NULL,           0,          NULL                    }
     };

  if (!parse_config_table(&dynip_cfg[0], "DYNIP.", name, value) && prev_hook)
     (*prev_hook) (name, value);
}

/**
 * Free allocated memory.
 */
static void W32_CALL dynip_exit (void)
{
  if (_watt_fatal_error)
     return;

  url_free (&chkip);
  url_free (&dyndns);
}

void dynip_init (void)
{
  prev_hook = usr_init;
  usr_init  = dynip_config;
  RUNDOWN_ADD (dynip_exit, 262);
}

/*
 * Do this at most once each hour:
 *
 * 1. Get our WAN ip (<wan-ip> below) by fetching:
 *    http://checkip.dyndns.com/  or  http://ipdetect.dnspark.com/
 *
 * 2. Send HTTP request to members.dyndns.com:
 *    GET /nic/update?system=dyndns&hostname=<host-name>&myip=<wan-ip> HTTP/1.1
 *    Host: members.dyndns.com
 *    Authorization: Basic <user-name>:<password>  base64 encoded
 *    Connection: close
 *
 * 3. Get result: "nohost"         means <host-name> isn't registered in DynDNS.
 *                "noauth"         means wrong <user-name> or <passord>.
 *                "notfqdn"        means invalid <host-name> etc.
 *                "nochg <wan-ip>" means no update needed.
 *                "good <wan-ip>"  means the update was acccepted.
 *
 * \todo: this should be rewritten as a non-blocking Watt-32 daemon.
 *        Use daemon_add (dynip_exec).
 */
int dynip_exec (void)
{
  char auth_buf [512] = "";
  char request [512], *p;
  int  num_arg, rc = 0;

  if (!dynip_enable)
     return (0);

  if (!dyn_myip[0] && chkip.host)  /* Not already entered as dotted quad */
  {
    const char *myip;

    rc = get_url (chkip.host, chkip.port, chkip.get_req, NULL);
    if (rc <= 0)
       return (0);

    myip = parse_myip_address (resp_buf);

    TRACE (2, ("Got %d bytes HTML, Found myip: `%s'\r\n", rc, myip));
    if (!myip)
       return (0);
    _strlcpy (dyn_myip, myip, sizeof(dyn_myip));
  }

  if (!dyn_myhostname[0] || !dyndns.get_req)
     return (0);

  for (num_arg = 0, p = strstr(dyndns.get_req,"%s"); p;
       p = strstr(p+1,"%s"))
      num_arg++;

  if (num_arg != 1 && num_arg != 2)
  {
    TRACE (1, ("Update URL (%s) must contain 1 or 2 \"%%s\" parameters\n",
           dyndns.get_req));
    return (0);
  }

  if (dyndns_user[0] && dyndns_passwd[0])
     _SNPRINTF (BUF(auth_buf), "%s:%s", dyndns_user, dyndns_passwd);
  else if (dyndns_user[0])
     _SNPRINTF (BUF(auth_buf), "%s", dyndns_user);

  _SNPRINTF (BUF(request), dyndns.get_req, dyn_myhostname, dyn_myip);

  if (!dyn_myip[0])
  {
    p = strrchr (request, '&');  /* chop of "&myip.." */
    if (p)
       *p = '\0';
  }
  rc = get_url (dyndns.host, dyndns.port, request,
                *auth_buf ? auth_buf : NULL);
  if (rc <= 0)
     return (0);
  return parse_dyndns_response (resp_buf);
}

/**
 * Parse response buffer and find the first IP-address.
 * 'buf' must be 0-terminated.
 */
static const char *parse_myip_address (const char *orig)
{
  const char *body, *end, *ret, *p;
  char  *buf;
  static char res[20];

  buf = strdup (orig);
  if (!buf)
  {
    TRACE (1, ("No memory\n"));
    return (NULL);
  }

  strlwr (buf);

  /* Accept "<body>" or "<body bgcolor..>" etc.
   */
  body = strstr (buf, "<body");
  if (!body)
  {
    TRACE (1, ("No <body>\n"));
    free (buf);
    return (NULL);
  }

  p   = body + sizeof("<body")-1;
  end = strstr (p, "</body>");
  if (!end)
  {
    TRACE (1, ("No </body>\n"));
    free (buf);
    return (NULL);
  }

  /* Find an IP-address between 'p' and 'end'
   */
  TRACE (2, ("p: `%s', len %d\n", p, (int)(end-p)));
  ret = NULL;

  for ( ; *p && p < end; p++)
      if (isdigit((int)*p) && aton(p))
      {
        ret = _strlcpy (res, p, min(end-p+1,SIZEOF(res)));
        break;
      }
  free (buf);
  return (ret);
}

/**
 * Parse the status response from DynDNS.
 */
static int parse_dyndns_response (const char *buf)
{
  TRACE (1, ("DynDNS resp: `%s'\n", buf));

#if 0
  if (!strncmp(buf,"nohost",6))
     ;
  if (!strncmp(buf,"noauth",6))
     ;
  if (!strncmp(buf,"notfqdn",7))
     ;
  if (!strncmp(buf,"nochg",5))
     ;
  if (!strncmp(buf,"good",4))
     ;
  if (!strncmp(buf,"!yours",6))
     ;
  if (!strncmp(buf,"abuse",5))
     ;
  if (!strncmp(buf,"numhost",7))
     ;
  if (!strncmp(buf,"dnserr",6))
     ;
#endif
  ARGSUSED (buf);
  return (1);
}

/**
 * Simple URL parser; accepts only "http://" prefixes (optional).
 * Extracts host, port and request parts ("/" if not present).
 */
static BOOL url_parse (struct URL *res, const char *url)
{
  char  scheme [20+1];
  char  buf [256+1];
  char *p;
  const char *orig_url = url;
  int   num;

  res->port    = 80;    /* default values */
  res->host    = NULL;
  res->get_req = NULL;
  res->on_heap = FALSE;

  p = strstr (url, "://");
  if (p)
  {
    _strlcpy (scheme, url, min(SIZEOF(scheme), p-url+1));
    if (stricmp(scheme,"http"))
    {
      TRACE (1, ("Unsupported scheme `%s'\n", scheme));
      return (FALSE);
    }
    url = p + 3;
  }

  num = sscanf (url, "%256[^/]/%*s", buf);

  p = strrchr (url, '/'); /* workaround for djgpp 2.04 sscanf() bug */
  if (p && !*(p+1))
     num = 1;

  if (num >= 1)
  {
    p = strchr (buf, ':');
    if (p)
    {
      res->port = atoi (p+1);
      *p = '\0';
    }
    res->host = strdup (buf);
    res->on_heap = TRUE;
    p = strchr (url, '/');
    res->get_req = p ? strdup (p) : strdup ("/");
    TRACE (2, ("url_parse: host `%s', port %u, req `%s'\r\n",
               res->host, res->port, res->get_req));
    return (TRUE);
  }
  TRACE (1, ("Malformed URL `%s'\r\n", orig_url));
  ARGSUSED (orig_url);
  return (FALSE);
}

/**
 * Free the URL.
 */
static void url_free (struct URL *url)
{
  if (url->on_heap)
  {
    DO_FREE (url->get_req);
    DO_FREE (url->host);
  }
  url->on_heap = FALSE;
}

/**
 * Select method of getting your public IP (WAN-side) address.
 * Either a dotted IPv4 address or an URL to return your address.
 */
static int chkip_method (const char *value)
{
  if (isaddr(value))
     _strlcpy (dyn_myip, value, sizeof(dyn_myip));
  else
  {
    url_free (&chkip);
    url_parse (&chkip, value);
  }
  return (1);
}

/**
 * Extracts the host, port and request from the value. Used for
 * sending the DynDNS IP/host update.
 */
static int dyndns_params (const char *value)
{
  url_free (&dyndns);
  url_parse (&dyndns, value);
  return (1);
}

/*
 * Taken from:
 *
 *   HTGET
 *
 *   Get a document via HTTP
 *
 *   This program was compiled under Borland C++ 3.1 in C mode for the small
 *   model. You will require the WATTCP libraries. A copy is included in the
 *   distribution. For the sources of WATTCP, ask archie where the archives
 *   are.
 *
 *   Please send bug fixes, ports and enhancements to the author for
 *   incorporation in newer versions.
 *
 *   Copyright (C) 1996  Ken Yap (ken@syd.dit.csiro.au)
 *
 *   This program is free software; you can redistribute it and/or modify it
 *   under the terms of the Artistic License, a copy of which is included
 *   with this distribution.
 *
 *   THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
 *   IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 *   WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#define USER_AGENT  "Watt-32/DynIP-client 0.5"
#define AUTHOR      "Copyright (C) 1996 Ken Yap (ken@syd.dit.csiro.au)"

#define HTTPVER     "HTTP/1.0"   /* HTTP version to use */
#define HTTPVER_10  "HTTP/1.0"
#define HTTPVER_11  "HTTP/1.1"

#if defined(__CYGWIN__)
  #define STARTS_WITH(s, what)  (strncasecmp (s, what, sizeof(what)-1) == 0)
#else
  #define STARTS_WITH(s, what)  (strnicmp (s, what, sizeof(what)-1) == 0)
#endif

static int base64encode (const char *in, char *out, size_t out_len)
{
  int    c1, c2, c3;
  static char basis_64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"\
                           "abcdefghijklmnopqrstuvwxyz0123456789+/";
  while (*in)
  {
    c1 = *in++;
    if (*in == '\0')
       c2 = c3 = 0;
    else
    {
      c2 = *in++;
      if (*in == '\0')
           c3 = 0;
      else c3 = *in++;
    }
    out_len -= 4;
    if (out_len <= 4)
       return (-1);

    *out++ = basis_64 [c1 >> 2];
    *out++ = basis_64 [((c1 & 0x3) << 4) | ((c2 & 0xF0) >> 4)];
    if (c2 == 0 && c3 == 0)
    {
      *out++ = '=';
      *out++ = '=';
    }
    else if (c3 == 0)
    {
      *out++ = basis_64 [((c2 & 0xF) << 2) | ((c3 & 0xC0) >> 6)];
      *out++ = '=';
    }
    else
    {
      *out++ = basis_64 [((c2 & 0xF) << 2) | ((c3 & 0xC0) >> 6)];
      *out++ = basis_64 [c3 & 0x3F];
    }
  }
  *out = '\0';
  return (0);
}

/**
 * Receive and parse the HYTTP header.
 * Return FALSE if header isn't correct.
 * Return TRUE and extract "Content-Length" field.
 */
static BOOL get_header (void *sock, long *cont_len_ptr)
{
  int    len, response;
  long   content_length;
  size_t i;
  char   buf [512];
  char  *s;

  *cont_len_ptr = 0L;

  len = sock_gets (sock, (BYTE*)buf, sizeof(buf));
  if (len <= 0)
  {
    TRACE (1, ("EOF from server\n"));
    return (FALSE);
  }
  TRACE (1, ("HTTP: `%.*s'\r\n", len, buf));

  if (!STARTS_WITH(buf,HTTPVER_10) &&  /* no HTTP/1.[01]? */
      !STARTS_WITH(buf,HTTPVER_11))
  {
    TRACE (1, ("Not a HTTP/1.[01] server\n"));
    return (FALSE);
  }

  s = buf + sizeof(HTTPVER_10)-1;
  i = strspn (s, " \t");
  if (i == 0)        /* no whitespace? */
  {
    TRACE (1, ("Malformed HTTP/1.[01] line\n"));
    return (FALSE);
  }

  s += i;
  response = 500;
  sscanf (s, "%3d", &response);
  if (response == 401)
  {
    TRACE (1, ("Authorisation failed\n"));
    return (FALSE);
  }
  if (response != 200 && response != 301 &&
      response != 302 && response != 304)
  {
    TRACE (1, ("Unexpected response %s\n", s));
    return (FALSE);
  }

  /* Eat up the other header lines here.
   * Set to LONG_MAX, in case no "Content-length" header.
   */
  content_length = LONG_MAX;

  while ((len = sock_gets(sock, (BYTE*)buf, sizeof(buf))) > 0)
  {
    TRACE (1, ("HTTP: `%.*s'\r\n", len, buf));
    s = buf;
    if (STARTS_WITH(s,"Content-Length:"))
    {
      s += sizeof("Content-Length:") - 1;
      content_length = atol (s);
    }
#if 0
    else if (STARTS_WITH(s,"Transfer-encoding: chunked"))
    {
      TRACE (1, ("Chunked encoding not supported\n"));
      return (FALSE);
    }
#endif
    else if (STARTS_WITH(buf, "Location:"))
    {
      if (response == 301 || response == 302)
         TRACE (1, ("Location at %s\n", buf));
    }
    else if (strchr(" \t", buf[0]))
            TRACE (1, ("Warning: continuation line encountered\n"));
  }
  *cont_len_ptr = content_length;
  return (TRUE);
}

/**
 * Fetch a single URL optionally with authentication.
 */
static int get_url (const char *host, int port, const char *path,
                    const char *user_pass)
{
  DWORD addr;
  int   status = 0;
  int   length;
  long  content_length = 0L;
  char  req_buf [512], *p;
  sock_type *sock;

  if (!host)
  {
    TRACE (1, ("get_url: NULL host\n"));
    return (-1);
  }

  addr = resolve (host);
  if (!addr)
  {
    TRACE (1, ("%s\n", dom_strerror(dom_errno)));
    return (-1);
  }

  sock = malloc (sizeof(*sock));
  if (!sock)
  {
    puts ("get_url: No memory");
    return (-1);
  }

  if (!tcp_open(&sock->tcp, 0, addr, port, NULL))
  {
    printf ("Cannot connect to `%s'\n", host);
    free (sock);
    return (-1);
  }

  length = 0;

  sock_wait_established (sock, 10, NULL, &status);
  if (!tcp_tick(sock))   /* in case they sent reset */
  {
    status = -1;
    goto sock_err;
  }

  p = req_buf;
  p += _SNPRINTF (BUF(req_buf),
                  "GET %s %s\r\n"
                  "Host: %s\r\n"
                  "User-Agent: %s\r\n",
                  path, HTTPVER, host, USER_AGENT);
  if (user_pass)
  {
    char encoded_auth [256];

    if (base64encode(user_pass, encoded_auth, sizeof(encoded_auth)) < 0)
       goto sock_err;
    p += sprintf (p, "Authorization: Basic %s\r\n", encoded_auth);
  }

  p += sprintf (p, "Connection: close\r\n\r\n");

  TRACE (2, ("Sent: %s\n", req_buf));

  sock_write (sock, (BYTE*)req_buf, (int)(p - req_buf));
  sock_wait_input (sock, 10, NULL, &status);

  if (get_header (sock, &content_length))
     TRACE (2, ("Content-length %ld, remaining %u\n",
            content_length, sock_dataready(sock)));

  if (content_length >= 0L)
  {
    while (sock_dataready(sock) > 0)
    {
      int i = sock_gets (sock, (BYTE*)(resp_buf+length),
                         sizeof(resp_buf)-1-length);
      TRACE (1, ("%s", resp_buf+length));
      length += i;
      if (length > SIZEOF(resp_buf))
      {
        length = SIZEOF(resp_buf);
        break;
      }
    }
    if (content_length != LONG_MAX && length != content_length)
       TRACE (1, ("Warning, actual length = %d, content length = %ld\n",
              length, content_length));
  }

sock_err:
  sock_abort (sock);  /* Kill 'sock' from the '_tcp_allsocs' list. */

  if (status == -1)
     TRACE (1, ("get_url(): %s, %s, ", host, sockerr(sock)));
  TRACE (2, ("got %d bytes\n", length));
  free (sock);
  return (length);
}
#endif  /* USE_DYNIP_CLI */

