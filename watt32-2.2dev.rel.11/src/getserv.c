 /*!\file getserv.c
 *
 *  Simple BSD-like services functions.
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
 *  20.aug 1996 - Created
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "wattcp.h"
#include "strings.h"
#include "misc.h"
#include "run.h"
#include "language.h"
#include "netaddr.h"
#include "pcconfig.h"
#include "get_xby.h"

#if defined(USE_BSD_API)

#include <sys/socket.h>
#include <netdb.h>

static struct _servent *serv0 = NULL;
static char   *servFname      = NULL;
static FILE   *servFile       = NULL;
static BOOL    servClose      = FALSE;

/* For DOSX targets we use a small hash table to speed-up searches.
 */
#if (DOSX)
  #define PORT_HASH_SIZE  256   /* must be 2^n */
  #define GET_HASH(p)     port_hash [(p) & (PORT_HASH_SIZE-1)]
  #define SET_HASH(se)    port_hash [se->s_port & (PORT_HASH_SIZE-1)] = se
  static struct _servent *port_hash [PORT_HASH_SIZE];
#else
  #define GET_HASH(p)     serv0
  #define SET_HASH(p)     ((void)0)
#endif

#define MAX_SERVLEN  20
#define MAX_PROTLEN  20

static __inline struct servent *fill_servent (const struct _servent *s)
{
  static struct servent ret;
  static char  name [MAX_SERVLEN];
  static char  prot [MAX_PROTLEN];
  static char *aliases [MAX_SERV_ALIASES+1];

  memcpy (&aliases, s->s_aliases, sizeof(aliases));
  ret.s_name    = _strlcpy (name, s->s_name, sizeof(name));
  ret.s_proto   = _strlcpy (prot, s->s_proto, sizeof(prot));
  ret.s_aliases = aliases;
  ret.s_port    = s->s_port;
  h_errno       = NETDB_SUCCESS;
  return (&ret);
}

static void W32_CALL _endservent (void)
{
  endservent();
}

/*
 * Read the \i services file and build linked-list.
 */
void W32_CALL ReadServFile (const char *fname)
{
  static BOOL been_here = FALSE;

  if (!fname || !*fname)
     return;

  if (been_here)  /* loading multiple services files */
  {
    free (servFname);
    FCLOSE (servFile);
    servFile = NULL;
  }
  servFname = strdup (fname);
  if (!servFname)
     return;

  setservent (1);
  if (!servFile)
     return;

  been_here = TRUE;

  while (1)
  {
    struct  servent *s = getservent();
    struct _servent *s2;
    int    i;

    if (!s)
       break;

    s2 = calloc (sizeof(*s2), 1);
    if (!s2)
    {
      (*_printf) (_LANG("%s too big!\n"), servFname);
      return;
    }
    for (i = 0; s->s_aliases[i]; i++)
        s2->s_aliases[i] = strdup (s->s_aliases[i]);
    s2->s_port  = s->s_port;
    s2->s_name  = strdup (s->s_name);
    s2->s_proto = strdup (s->s_proto);

    if (!s2->s_name || !s2->s_proto)
       break;

    s2->s_next = serv0;
    serv0      = s2;
    SET_HASH (s2);
  }
  rewind (servFile);
  RUNDOWN_ADD (_endservent, 255);
}

/*
 * Return name of \i /etc/services file.
 */
const char * W32_CALL GetServFile (void)
{
  return (servFname);
}

/*
 * Close the \i /etc/services file.
 */
void W32_CALL CloseServFile (void)
{
  FCLOSE (servFile);
  servFile = NULL;
}

/*
 * Reopen the \i /etc/services file.
 */
void W32_CALL ReopenServFile (void)
{
  ReadServFile (servFname);
}

/*------------------------------------------------------------------*/

struct servent * W32_CALL getservent (void)
{
  static struct _servent s;
  char  *name, *proto, *alias, *tok_buf = NULL;
  WORD   port;
  int    i;

  if (!netdb_init() || !servFile)
  {
    h_errno = NO_DATA;
    return (NULL);
  }

  while (1)
  {
    char buf[2*MAX_NAMELEN], *tok;

    if (!fgets(buf,sizeof(buf),servFile))
    {
      h_errno = NO_DATA;
      return (NULL);
    }

    tok = strltrim (buf);
    if (*tok == '#' || *tok == ';' || *tok == '\n')
       continue;

    /*  # Service         port/prot       alias(es)
     *  #-------------------------------------------
     *    rtmp            1/ddp   <- we only support "udp"/"tcp"
     *    echo            7/tcp
     *    echo            7/udp
     *    discard         9/tcp           sink null
     *    discard         9/udp           sink null
     */

    name = strtok_r (tok, " \t", &tok_buf);
    tok  = strtok_r (NULL, "/ \t\n", &tok_buf);
    if (!tok)
       continue;
    port  = intel16 (atoi(tok));
    proto = strtok_r (NULL, " \t\n", &tok_buf);
    if (name && port && proto &&
        (!stricmp(proto,"udp") || !stricmp(proto,"tcp")))
       break;
  }

  if (servClose)
     endservent();

  memset (&s, 0, sizeof(s));
  s.s_name  = name;
  s.s_proto = proto;
  s.s_port  = port;

  alias = strtok_r (NULL, " \t\n", &tok_buf);

  for (i = 0; alias && i < MAX_SERV_ALIASES; i++)
  {
    static char aliases [MAX_SERV_ALIASES][MAX_SERVLEN];

    if (*alias == '#' || *alias == ';')
       break;

    s.s_aliases[i] = _strlcpy (aliases[i], alias, sizeof(aliases[i]));
    alias = strtok_r (NULL, " \t\n", &tok_buf);
  }
  return fill_servent (&s);
}

/*------------------------------------------------------------------*/

struct servent * W32_CALL getservbyname (const char *serv, const char *proto)
{
  const struct _servent *s;
  int   i;

  if (!netdb_init() || !serv) /* proto == NULL is okay */
  {
    h_errno = NO_DATA;
    return (NULL);
  }

  for (s = serv0; s; s = s->s_next)
  {
    BOOL chk_prot = FALSE;

    if (s->s_name && !stricmp(s->s_name,serv))
       chk_prot = TRUE;

    else for (i = 0; s->s_aliases[i]; i++)
             if (!stricmp(serv,s->s_aliases[i]))
             {
               chk_prot = TRUE;
               break;
             }
    if (chk_prot && (!proto || !stricmp(proto,s->s_proto)))
       return fill_servent (s);
  }
  h_errno = NO_DATA;
  return (NULL);
}

/*------------------------------------------------------------------*/

struct servent * W32_CALL getservbyport (int port, const char *proto)
{
  const struct _servent *s;

  if (!netdb_init() || !proto)  /* proto == NULL is okay */
  {
    h_errno = NO_DATA;
    return (NULL);
  }

  for (s = GET_HASH(port); s; s = s->s_next)
      if (s->s_port == port &&
          (!proto || !stricmp(s->s_proto,proto)))
         return fill_servent (s);
  h_errno = NO_DATA;
  return (NULL);
}

/*------------------------------------------------------------------*/

void W32_CALL setservent (int stayopen)
{
  servClose = (stayopen == 0);
  if (!netdb_init() || !servFname)
     return;

  if (!servFile)
       FOPEN_TXT (servFile, servFname);
  else rewind (servFile);
}

/*------------------------------------------------------------------*/

void W32_CALL endservent (void)
{
  struct _servent *s, *next;

  if (_watt_fatal_error)
     return;

  if (servFname)
     free (servFname);
  if (servFile)
     FCLOSE (servFile);
  servFname = NULL;
  servFile  = NULL;

  for (s = serv0; s; s = next)
  {
    int i;
    for (i = 0; s->s_aliases[i]; i++)
        free (s->s_aliases[i]);
    next = s->s_next;
    free (s->s_name);
    free (s->s_proto);
    free (s);
  }
  serv0 = NULL;
  servClose = TRUE;
}
#endif /* USE_BSD_API */

#if defined(TEST_PROG)

#include "pcdbug.h"
#include "sock_ini.h"

int main (void)
{
  const struct _servent *s;
  int   i;

  dbug_init();
  sock_init();

  printf ("\n%s entries:\n", servFname);

  for (s = serv0; s; s = s->s_next)
  {
    printf ("proto %-6.6s  port %4d  name %-10.10s  Aliases:",
            s->s_proto, intel16(s->s_port), s->s_name);
    for (i = 0; s->s_aliases[i]; i++)
         printf (" %s", s->s_aliases[i]);
    puts ("");
  }
  fflush (stdout);
  return (0);
}
#endif /* TEST_PROG */


