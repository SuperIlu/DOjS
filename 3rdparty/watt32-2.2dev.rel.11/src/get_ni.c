/*!\file get_ni.c
 * BSD getnameinfo(),
 */

/* Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Issues to be discussed:
 * - Thread safe-ness must be checked
 * - Return values.  There seems to be no standard for return value (RFC2553)
 *   but INRIA implementation returns EAI_xxx defined for getaddrinfo().
 * - RFC2553 says that we should raise error on short buffer.  X/Open says
 *   we need to truncate the result.  We obey RFC2553 (and X/Open should be
 *   modified).
 *
 * Adapted for Watt-32 tcp/ip stack by G. Vanem <gvanem@yahoo.no>, Aug 2002.
 */

#include "socket.h"

#if defined(USE_BSD_API)

#define NI_SUCCESS 0
#define ANY        0
#define YES        1
#define NO         0

#define ENI_NOSOCKET    EINVAL
#define ENI_NOHOSTNAME  EINVAL
#define ENI_MEMORY      ENOMEM
#define ENI_SALEN       EINVAL
#define ENI_FAMILY      EAFNOSUPPORT
#define ENI_SYSTEM      -1

#define GI_RESULT(err)  gi_result (err, #err, __LINE__)

static struct afd {
       int  a_af;
       int  a_addrlen;
       int  a_socklen;
       int  a_off;
     } afd_list [3];

struct sockinet {
       BYTE  si_len;
       BYTE  si_family;
       WORD  si_port;
     };

/*
 * Initialise afd_list[] with IPv6 protocol at highest priority.
 */
static void afd_list_init (BOOL ip6_prio)
{
  int i = (ip6_prio ? 0 : 1);

  memset (&afd_list, 0, sizeof(afd_list));
  afd_list[i].a_af      = AF_INET6;
  afd_list[i].a_addrlen = sizeof (struct in6_addr);
  afd_list[i].a_socklen = sizeof (struct sockaddr_in6);
  afd_list[i].a_off     = offsetof (struct sockaddr_in6, sin6_addr);

  i ^= 1;
  afd_list[i].a_af      = AF_INET;
  afd_list[i].a_addrlen = sizeof (struct in_addr);
  afd_list[i].a_socklen = sizeof (struct sockaddr_in);
  afd_list[i].a_off     = offsetof (struct sockaddr_in, sin_addr);
}

static int gi_result (int err, const char *str, unsigned line)
{
  SOCK_LEAVE_SCOPE();
  if (err)
  {
    SOCK_DEBUGF ((", %s (%d) at line %u", str, err, line));
    if (err > 0)
       SOCK_ERRNO (err);
  }
  else
    SOCK_DEBUGF ((", okay"));

  ARGSUSED (str);
  ARGSUSED (line);
  return (err);
}

int W32_CALL getnameinfo (const struct sockaddr *sa, socklen_t salen,
                          char *host, socklen_t hostlen,
                          char *serv, socklen_t servlen, int flags)
{
  static BOOL init = FALSE;
  struct afd *afd;
  sa_family_t family = -1;
  DWORD       v4a;
  WORD        port;
  char        numserv[512];
  char        numaddr[512];
  char       *addr, *p;
  int         i;

  const struct servent      *sp;
  const struct hostent      *hp;
  const struct sockaddr_in  *a4 = (const struct sockaddr_in *) sa;
  const struct sockaddr_in6 *a6 = (const struct sockaddr_in6*) sa;

  if (!init)
     afd_list_init (TRUE);
  init = TRUE;

  SOCK_DEBUGF (("\ngetnameinfo: "));

  if (!sa)
     return GI_RESULT (ENI_NOSOCKET);

  family = sa->sa_family;

  SOCK_DEBUGF (("name %s, serv %.10s, flags %04X",
                family == AF_INET  ?  inet_ntoa(a4->sin_addr) :
                family == AF_INET6 ? _inet6_ntoa(&a6->sin6_addr) : "??",
                ((flags & NI_NUMERICSERV) || flags == 0) ? "<get>" : serv,
                flags));

  for (i = 0; afd_list[i].a_af; i++)
      if (afd_list[i].a_af == family)
      {
        afd = &afd_list[i];
        goto found;
      }
  return GI_RESULT (ENI_FAMILY);

found:
  if (salen != afd->a_socklen)
     return GI_RESULT (ENI_SALEN);

  port = ((struct sockinet*)sa)->si_port; /* network byte order */
  addr = (char*) sa + afd->a_off;

  SOCK_ENTER_SCOPE();

  if (!serv || servlen == 0)
  {
    /* do nothing in this case.
     * in case you are wondering if "&&" is more correct than
     * "||" here: RFC2553 says that serv == NULL OR servlen == 0
     * means that the caller does not want the result.
     */
  }
  else
  {
    if (flags & NI_NUMERICSERV)
         sp = NULL;
    else sp = getservbyport (port, (flags & NI_DGRAM) ? "udp" : "tcp");
    if (sp)
    {
      if ((int)strlen(sp->s_name) + 1 > servlen)
         return GI_RESULT (ENI_MEMORY);
      strcpy (serv, sp->s_name);
    }
    else
    {
      itoa (ntohs(port), numserv, 10);
      if ((int)strlen(numserv) + 1 > servlen)
         return GI_RESULT (ENI_MEMORY);
      strcpy (serv, numserv);
    }
  }

  switch (sa->sa_family)
  {
    case AF_INET:
         v4a = ntohl (((struct sockaddr_in*)sa)->sin_addr.s_addr);
         if (IN_MULTICAST (v4a) || IN_EXPERIMENTAL (v4a))
            flags |= NI_NUMERICHOST;
         v4a >>= IN_CLASSA_NSHIFT;
         if (v4a == 0)
            flags |= NI_NUMERICHOST;
         break;

    case AF_INET6:
         switch (a6->sin6_addr.s6_addr[0])
         {
           case 0x00:
                if (IN6_IS_ADDR_V4MAPPED(&a6->sin6_addr))
                   ;
                else if (IN6_IS_ADDR_LOOPBACK(&a6->sin6_addr))
                   ;
                else
                  flags |= NI_NUMERICHOST;
                break;
           default:
                if (IN6_IS_ADDR_LINKLOCAL(&a6->sin6_addr))
                   flags |= NI_NUMERICHOST;
                else if (IN6_IS_ADDR_MULTICAST(&a6->sin6_addr))
                   flags |= NI_NUMERICHOST;
                break;
         }
         break;
  }

  if (!host || hostlen == 0)
  {
    /* do nothing in this case.
     * in case you are wondering if "&&" is more correct than
     * "||" here: RFC2553 says that host == NULL OR hostlen == 0
     * means that the caller does not want the result.
     */
  }
  else if (flags & NI_NUMERICHOST)
  {
    /* NUMERICHOST and NAMEREQD conflicts with each other
     */
    if (flags & NI_NAMEREQD)
       return GI_RESULT (ENI_NOHOSTNAME);

    if (inet_ntop (afd->a_af, addr, numaddr, sizeof(numaddr)) == NULL)
       return GI_RESULT (ENI_SYSTEM);

    if ((int)strlen(numaddr) + 1 > hostlen)
       return GI_RESULT (ENI_MEMORY);

    strcpy (host, numaddr);

    if (afd->a_af == AF_INET6 &&
        (IN6_IS_ADDR_LINKLOCAL(addr) ||
         IN6_IS_ADDR_MULTICAST(addr)) &&
         ((struct sockaddr_in6*)sa)->sin6_scope_id)
    {
#if !defined(ALWAYS_WITHSCOPE)
      if (flags & NI_WITHSCOPEID)
#endif
      {
        char *ep    = strchr (host, '\0');
        int   id    = ((struct sockaddr_in6*)sa)->sin6_scope_id;
        int   scope = __scope_id_to_ascii (id);  /* '1' ... N */

        *ep++ = SCOPE_DELIMITER;
        *ep++ = scope ? scope : '?';
        *ep = '\0';
      }
    }
  }
  else
  {
    hp = gethostbyaddr (addr, afd->a_addrlen, afd->a_af);
    if (hp)
    {
      if (flags & NI_NOFQDN)
      {
        p = strchr (hp->h_name, '.');
        if (p)
           *p = '\0';
      }
      if ((int)strlen(hp->h_name) + 1 > hostlen)
         return GI_RESULT (ENI_MEMORY);
      strcpy (host, hp->h_name);
    }
    else
    {
      if (flags & NI_NAMEREQD)
         return GI_RESULT (ENI_NOHOSTNAME);

      if (inet_ntop (afd->a_af, addr, numaddr, sizeof(numaddr)) == NULL)
         return GI_RESULT (ENI_NOHOSTNAME);

      if ((int)strlen(numaddr) + 1 > hostlen)
         return GI_RESULT (ENI_MEMORY);

      strcpy (host, numaddr);
    }
  }
  ARGSUSED (a4);
  return GI_RESULT (NI_SUCCESS);
}


#if defined(TEST_PROG)

#undef ni_flags  /* <netinet/icmp6.> */

static int ni_flags = NI_NAMEREQD; // NI_NUMERICHOST

static int ip4_resolve (const char *name)
{
  struct sockaddr_in a4;
  char   buf[40];
  int    rc;

  printf ("getnameinfo(): ");
  fflush (stdout);

  a4.sin_family = AF_INET;
  inet_pton (AF_INET, "203.178.141.194", &a4.sin_addr);
  rc = getnameinfo ((const struct sockaddr*)&a4, sizeof(a4),
                    buf, sizeof(buf), NULL, 0, ni_flags);
  if (rc)
       perror (NULL);
  else puts (buf);
  ARGSUSED (name);
  return (rc);
}

static int ip6_resolve (const char *name)
{
  struct sockaddr_in6 a6;
  char   buf[40];
  int    rc;

  printf ("getnameinfo(): ");
  fflush (stdout);

  a6.sin6_family = AF_INET6;  /* www.kame.net */
  inet_pton (AF_INET6, "2001:200:DFF:FFF1:216:3EFF:FEB1:44D7", &a6.sin6_addr);
  rc = getnameinfo ((const struct sockaddr*)&a6, sizeof(a6),
                    buf, sizeof(buf), NULL, 0, ni_flags);
  if (rc)
       puts (hstrerror(h_errno));
  else puts (buf);
  ARGSUSED (name);
  return (rc);
}

int main (int argc, char **argv)
{
  int af = -1;

  if (argc < 2)
     goto usage;

  dbug_init();
  sock_init();

  if (!stricmp(argv[1],"AF_INET"))
     af = AF_INET;

  else if (!stricmp(argv[1],"AF_INET6"))
     af = AF_INET6;

  if (af == AF_INET6)
     return ip6_resolve ("www.kame.net");

  if (af == AF_INET)
     return ip4_resolve ("www.kame.net");

usage:
  printf ("Usage: <AF_INET | AF_INET6>");
  return (1);
}
#endif /* TEST_PROG */
#endif /* USE_BSD_API && USE_IP6 */

