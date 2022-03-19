/*!\file get_xbyr.c
 * Reentrant getXbyX() functions.
 */

/*  BSD/XOpen-like functions:
 *    Reentrant (MT-safe) version of some
 *    getXbyY function from <netdb.h>
 *
 *  Copyright (c) 1997-2002 Gisle Vanem <gvanem@yahoo.no>
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
 *  18.aug 1998 (GV)  - Created
 *
 *  NOT USED YET
 */

#include "resolver.h"

#if defined(USE_BSD_API)

/*
 * NB! This file isn't part of Watt-32 yet as I don't have
 *     proper documentation on these `_r' functions.
 *     Anybody got a clue?
 */

/*
 * gethostbyY_r..
 */
int W32_CALL gethostbyaddr_r (const char *addr, int addr_len, int addr_type,
                              struct hostent *result,
                              struct hostent *buffer,
                              int buf_len, int *p_errno)
{
  struct hostent *he;

  if (!result || !buffer)
  {
    *p_errno = EINVAL;
    return (-1);
  }

  he = gethostbyaddr (addr, addr_len, addr_type);
  *p_errno = h_errno;
  if (!he)
     return (-1);

  memcpy (result, he, sizeof(*result));
  ARGSUSED (buf_len);
  return (0);
}

struct hostent * W32_CALL gethostbyname_r (const char *name,
                                           struct hostent *result,
                                           char *buffer, int buf_len,
                                           int *p_errno)
{
  UNFINISHED();
  *p_errno = ERANGE;
  SOCK_ERRNO (ERANGE);
  ARGSUSED (name);
  ARGSUSED (result);
  ARGSUSED (buffer);
  ARGSUSED (buf_len);
  return (NULL);
}

int W32_CALL gethostent_r (struct hostent *result,
                           struct hostent *buffer)
{
  UNFINISHED();
  ARGSUSED (result);
  ARGSUSED (buffer);
  return (-1);
}

int W32_CALL sethostent_r (int stayopen, struct hostent *buffer)
{
  UNFINISHED();
  ARGSUSED (stayopen);
  ARGSUSED (buffer);
  return (-1);
}

int W32_CALL endhostent_r (struct hostent *buffer)
{
  UNFINISHED();
  ARGSUSED (buffer);
  return (-1);
}


/*
 * getprotobyY_r..
 */
int W32_CALL getprotobynumber_r (int proto,
                                 struct protoent *result,
                                 struct protoent *buffer)
{
  UNFINISHED();
  ARGSUSED (proto);
  ARGSUSED (result);
  ARGSUSED (buffer);
  return (-1);
}

int W32_CALL getprotobyname_r (const char *name,
                               struct protoent *result,
                               struct protoent *buffer)
{
  UNFINISHED();
  ARGSUSED (name);
  ARGSUSED (result);
  ARGSUSED (buffer);
  return (-1);
}

int W32_CALL getprotoent_r (struct protoent *result,
                            struct protoent *buffer)
{
  UNFINISHED();
  ARGSUSED (result);
  ARGSUSED (buffer);
  return (-1);
}

int W32_CALL setprotoent_r (int stayopen, struct protoent *buffer)
{
  UNFINISHED();
  ARGSUSED (stayopen);
  ARGSUSED (buffer);
  return (-1);
}

int W32_CALL endprotoent_r (struct protoent *buffer)
{
  UNFINISHED();
  ARGSUSED (buffer);
  return (-1);
}

/*
 * getservbyY_r..
 */
int W32_CALL getservbyname_r (const char *name, const char *proto,
                              struct servent *result,
                              struct servent *buffer)
{
  UNFINISHED();
  ARGSUSED (name);
  ARGSUSED (proto);
  ARGSUSED (result);
  ARGSUSED (buffer);
  return (-1);
}

int W32_CALL getservbyport_r (int port, const char *proto,
                              struct servent *result,
                              struct servent *buffer)
{
  UNFINISHED();
  ARGSUSED (port);
  ARGSUSED (proto);
  ARGSUSED (result);
  ARGSUSED (buffer);
  return (-1);
}
#endif /* USE_BSD_API */
