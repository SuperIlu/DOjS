/*!\file getprot.c
 *
 *  BSD getprotobyxx() functions.
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
 *  21.aug 1996 - Created
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
#include "pcconfig.h"
#include "get_xby.h"

#if defined(USE_BSD_API)

static struct _protoent *proto0 = NULL;
static char   *protoFname       = NULL;
static FILE   *protoFile        = NULL;
static BOOL    protoClose       = FALSE;

static __inline struct protoent *fill_protoent (const struct _protoent *p)
{
  static struct protoent ret;
  static char   name [30];
  static char  *aliases [MAX_PROTO_ALIASES+1];

  memcpy (&aliases, p->p_aliases, sizeof(aliases));
  ret.p_name    = _strlcpy (name, p->p_name, sizeof(name));
  ret.p_proto   = p->p_proto;
  ret.p_aliases = aliases;
  h_errno       = NETDB_SUCCESS;
  return (&ret);
}

static void W32_CALL _endprotoent (void)
{
  endprotoent();
}

void W32_CALL ReadProtoFile (const char *fname)
{
  static BOOL been_here = FALSE;

  if (!fname || !*fname)
     return;

  if (been_here)  /* loading multiple protocol files */
  {
    free (protoFname);
    FCLOSE (protoFile);
    protoFile = NULL;
  }

  protoFname = strdup (fname);
  if (!protoFname)
     return;

  setprotoent (1);
  if (!protoFile)
     return;

  been_here = TRUE;

  while (1)
  {
    struct  protoent *p = getprotoent();
    struct _protoent *p2;
    int    i;

    if (!p)
       break;

    p2 = calloc (sizeof(*p2), 1);
    if (!p2)
    {
      (*_printf) (_LANG("%s too big!\n"), protoFname);
      break;
    }
    for (i = 0; p->p_aliases[i]; i++)
        p2->p_aliases[i] = strdup (p->p_aliases[i]);
    p2->p_proto = p->p_proto;
    p2->p_name  = strdup (p->p_name);

    if (!p2->p_name)
       break;

    p2->p_next = proto0;
    proto0     = p2;
  }
  rewind (protoFile);
  RUNDOWN_ADD (_endprotoent, 257);

#if 0  /* test */
  {
    const struct _protoent *p;
    int   i;

    printf ("\n%s entries:\n", protoFname);
    for (p = proto0; p; p = p->p_next)
    {
      printf ("proto %3d  name %-10.10s  Aliases:", p->p_proto, p->p_name);
      for (i = 0; p->p_aliases[i]; i++)
          printf (" %s,", p->p_aliases[i]);
      puts ("");
    }
    fflush (stdout);
  }
#endif
}

/*------------------------------------------------------------------*/

const char * W32_CALL GetProtoFile (void)
{
  return (protoFname);
}

void W32_CALL CloseProtoFile (void)
{
  FCLOSE (protoFile);
  protoFile = NULL;
}

void W32_CALL ReopenProtoFile (void)
{
  ReadProtoFile (protoFname);
}

/*------------------------------------------------------------------*/

struct protoent * W32_CALL getprotoent (void)
{
  static struct _protoent p;
  char  *proto, *name;

  if (!netdb_init() || !protoFile)
  {
    h_errno = NO_DATA;
    return (NULL);
  }

  /*  Protocol  Name
   *  ----------------------------------------
   *  0         reserved
   *  1         icmp, internet control message
   *  2         igmp, internet group management
   *  3         ggp, # gateway-gateway protocol
   */

  while (1)
  {
    char buf [2*MAX_NAMELEN], *tok, *tok_buf = NULL;

    if (!fgets(buf,sizeof(buf),protoFile))
    {
      h_errno = NO_DATA;
      return (NULL);
    }

    tok = strltrim (buf);
    if (*tok == '#' || *tok == ';' || *tok == '\n')
       continue;

    proto = strtok_r (tok, " \t", &tok_buf);
    name  = strtok_r (NULL, " ,\t\n", &tok_buf);
    if (proto && name)
       break;
  }

  if (protoClose)
     endprotoent();

  memset (&p, 0, sizeof(p));
  p.p_proto = atoi (proto);
  p.p_name  = name;
  return fill_protoent (&p);
}

/*------------------------------------------------------------------*/

struct protoent * W32_CALL getprotobyname (const char *proto)
{
  struct _protoent fixed;
  const struct _protoent *p;
  int   i;

  memset (&fixed, 0, sizeof(fixed));

  if (!netdb_init() || !proto)
  {
    h_errno = NO_DATA;
    return (NULL);
  }

  /* No chances UDP/TCP protocols are renumbered
   */
  if (!stricmp(proto,"udp"))
  {
    fixed.p_name  = (char*) "udp";
    fixed.p_proto = UDP_PROTO;
    return fill_protoent (&fixed);
  }

  if (!stricmp(proto,"tcp"))
  {
    fixed.p_name  = (char*) "tcp";
    fixed.p_proto = TCP_PROTO;
    return fill_protoent (&fixed);
  }

  for (p = proto0; p; p = p->p_next)
  {
    if (p->p_name && !stricmp(p->p_name,proto))
       return fill_protoent (p);

    /* aliases not supported (no need)
     */
    for (i = 0; p->p_aliases[i] && i < DIM(p->p_aliases); i++)
        if (!stricmp(proto,p->p_aliases[i]))
           return fill_protoent (p);
  }
  h_errno = NO_DATA;
  return (NULL);
}

/*------------------------------------------------------------------*/

struct protoent * W32_CALL getprotobynumber (int proto)
{
  const struct _protoent *p;

  if (!netdb_init())
  {
    h_errno = NO_DATA;
    return (NULL);
  }

  for (p = proto0; p; p = p->p_next)
      if (p->p_proto == proto)
         return fill_protoent (p);
  h_errno = NO_DATA;
  return (NULL);
}

/*------------------------------------------------------------------*/

void W32_CALL setprotoent (int stayopen)
{
  protoClose = (stayopen == 0);

  if (!netdb_init() || !protoFname)
     return;

  if (!protoFile)
       FOPEN_TXT (protoFile, protoFname);
  else rewind (protoFile);
}

/*------------------------------------------------------------------*/

void W32_CALL endprotoent (void)
{
  struct _protoent *p, *next;

  if (_watt_fatal_error)
     return;

  if (protoFname)
     free (protoFname);
  if (protoFile)
     FCLOSE (protoFile);
  protoFname = NULL;
  protoFile  = NULL;

  for (p = proto0; p; p = next)
  {
    int i;
    for (i = 0; p->p_aliases[i] && i < DIM(p->p_aliases); i++)
        free (p->p_aliases[i]);
    next = p->p_next;
    free (p->p_name);
    free (p);
  }
  proto0 = NULL;
  protoClose = TRUE;
}
#endif  /* USE_BSD_API */
