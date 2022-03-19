/*!\file getnet.c
 *
 *  Simple BSD-like network-entry functions.
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
 *  18.aug 1996 - Created
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "wattcp.h"
#include "misc.h"
#include "run.h"
#include "pcconfig.h"
#include "language.h"
#include "strings.h"
#include "get_xby.h"

#if defined(USE_BSD_API)

static struct _netent *network0 = NULL;
static char   *networkFname     = NULL;
static FILE   *networkFile      = NULL;
static BOOL    networkClose     = FALSE;

static __inline struct netent *fill_netent (const struct _netent *n)
{
  static struct netent ret;
  static char   name [MAX_NAMELEN];
  static char  *aliases [MAX_NETENT_ALIASES+1];

  memcpy (&aliases, n->n_aliases, sizeof(aliases));
  ret.n_name     = _strlcpy (name, n->n_name, sizeof(name));
  ret.n_aliases  = aliases;
  ret.n_net      = n->n_net;
  ret.n_addrtype = AF_INET;
  return (&ret);
}

static void W32_CALL _endnetent (void)
{
  endnetent();
}

void W32_CALL ReadNetworksFile (const char *fname)
{
  static BOOL been_here = FALSE;

  if (!fname || !*fname)
     return;

  if (been_here)  /* loading multiple network files */
  {
    free (networkFname);
    FCLOSE (networkFile);
    networkFile = NULL;
  }

  networkFname = strdup (fname);
  if (!networkFname)
     return;

  setnetent (1);
  if (!networkFile)
     return;

  been_here = TRUE;

  while (1)
  {
    struct netent  *n = getnetent();
    struct _netent *n2;
    int    i;

    if (!n)
       break;

    n2 = calloc (sizeof(*n2), 1);
    if (!n2)
    {
      (*_printf) (_LANG("%s too big!\n"), networkFname);
      break;
    }

    for (i = 0; n->n_aliases[i]; i++)
        n2->n_aliases[i] = strdup (n->n_aliases[i]);
    n2->n_net      = n->n_net;
    n2->n_addrtype = n->n_addrtype;
    n2->n_name     = strdup (n->n_name);
    if (!n2->n_name)
       break;
    n2->n_next = network0;
    network0   = n2;
  }
  rewind (networkFile);
  RUNDOWN_ADD (_endnetent, 251);

#if 0  /* test */
  {
    struct _netent *n;

    printf ("\n%s entries:\n", networkFname);

    for (n = network0; n; n = n->n_next)
    {
      int i;
      printf ("net %-15.15s name %-10.10s  Aliases:",
              inet_ntoa(inet_makeaddr(n->n_net,0)), n->n_name);
      for (i = 0; n->n_aliases[i]; i++)
          printf (" %s,", n->n_aliases[i]);
      puts ("");
    }
    fflush (stdout);
  }
#endif
}

/*
 * Return name of networks file
 */
const char * W32_CALL GetNetFile (void)
{
  return (networkFname);
}

/*
 * To prevent running out of file-handles, one should close the
 * 'networks' file before spawning a new shell.
 */
void W32_CALL CloseNetworksFile (void)
{
  FCLOSE (networkFile);
  networkFile = NULL;
}

void W32_CALL ReopenNetworksFile (void)
{
  ReadNetworksFile (networkFname);
}

/*
 * Return the next (non-commented) line from the network-file
 * Format is:
 *   name [=] net [alias..] {\n | # ..}
 *
 * e.g.
 *   loopback     127
 *   arpanet      10   arpa
 */
struct netent * W32_CALL getnetent (void)
{
  struct _netent n;
  char  *name, *net, *alias, *tok_buf = NULL;
  char   buf [2*MAX_NAMELEN], *tok;
  int    i;

  if (!netdb_init() || !networkFile)
     return (NULL);

  while (1)
  {
    if (!fgets(buf,sizeof(buf),networkFile))
       return (NULL);

    tok = strltrim (buf);
    if (*tok == '#' || *tok == ';' || *tok == '\n')
       continue;

    name = strtok_r (tok, " \t", &tok_buf);
    net  = strtok_r (NULL, "= \t\n", &tok_buf);
    if (name && net)
       break;
  }

  if (networkClose)
     endnetent();

  memset (&n, 0, sizeof(n));
  n.n_net  = inet_network (net);
  n.n_name = name;
  alias    = strtok_r (NULL, " \t\n", &tok_buf);

  for (i = 0; alias && i < MAX_NETENT_ALIASES; i++)
  {
    static char aliases [MAX_NETENT_ALIASES][MAX_NAMELEN];

    if (*alias == '#' || *alias == ';')
       break;

    n.n_aliases[i] = _strlcpy (aliases[i], alias, sizeof(aliases[i]));
    alias = strtok_r (NULL, " \t\n", &tok_buf);
  }
  return fill_netent (&n);
}

/*
 * Return a 'netent' structure for network 'name' or
 * NULL if not found.
 */
struct netent * W32_CALL getnetbyname (const char *name)
{
  const struct _netent *n;

  if (!netdb_init() || !name)
     return (NULL);

  for (n = network0; n; n = n->n_next)
  {
    int i;

    if (n->n_name && !stricmp(n->n_name,name))
       return fill_netent (n);

    for (i = 0; n->n_aliases[i]; i++)
        if (!stricmp(name,n->n_aliases[i]))
           return fill_netent (n);
  }
  return (NULL);
}

/*
 * Return a 'netent' structure for network number 'net' or
 * NULL if not found.
 */
struct netent * W32_CALL getnetbyaddr (long net, int type)
{
  const struct _netent *n;

  if (!netdb_init())
     return (NULL);

  for (n = network0; n; n = n->n_next)
      if ((u_long)net == n->n_net && type == n->n_addrtype && n->n_name)
         return fill_netent (n);
  return (NULL);
}

/*
 * Open the network file.
 */
void W32_CALL setnetent (int stayopen)
{
  networkClose = (stayopen == 0);

  if (!netdb_init() || !networkFname)
     return;

  if (!networkFile)
       FOPEN_TXT (networkFile, networkFname);
  else rewind (networkFile);
}

/*
 * Close the network file and free associated memory
 */
void W32_CALL endnetent (void)
{
  struct _netent *n, *next;

  if (_watt_fatal_error)
     return;

  if (networkFname)
     free (networkFname);
  if (networkFile)
     FCLOSE (networkFile);
  networkFname = NULL;
  networkFile  = NULL;

  for (n = network0; n; n = next)
  {
    int i;
    for (i = 0; n->n_aliases[i]; i++)
        free (n->n_aliases[i]);
    next = n->n_next;
    free (n->n_name);
    free (n);
  }
  network0 = NULL;
  networkClose = TRUE;
}
#endif /* USE_BSD_API */
