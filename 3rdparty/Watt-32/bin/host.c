/*
 * domain name server protocol
 *
 * This portion of the code needs some major work.  I ported it (read STOLE IT)
 * from NCSA and lost about half the code somewhere in the process.
 *
 * Note, this is a user level process.
 *
 *  0.2 : Apr 24, 1991 - use substring portions of domain
 *  0.1 : Mar 18, 1991 - improved the trailing domain list
 *  0.0 : Feb 19, 1991 - pirated by Erick Engelke
 * -1.0 :              - NCSA code
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tcp.h>

#if defined(__CYGWIN__)
  #define kbhit()  ((void)0)
#else
  #include <conio.h>
#endif

static DWORD timeoutwhen;
static udp_Socket *dom_sock;

#define DOMSIZE       512    /* maximum domain message size to mess with */
#define DOM_SRC_PORT  1415   /* old 997 didn't pass some firewalls */
#define DOM_DST_PORT  53     /* standard domain destination port */

/*
 * Header for the DOMAIN queries
 * ALL OF THESE ARE BYTE SWAPPED QUANTITIES!
 * We are the poor slobs who are incompatible with the world's BYTE order
 */
struct dhead {
       WORD  ident;          /* unique identifier */
       WORD  flags;
       WORD  qdcount;        /* question section, # of entries */
       WORD  ancount;        /* answers, how many              */
       WORD  nscount;        /* count of name server RRs       */
       WORD  arcount;        /* number of "additional" records */
     };

/* flag masks for the flags field of the DOMAIN header
 */
#define DQR      0x8000        /* query = 0, response = 1  */
#define DOPCODE  0x7100        /* opcode, see below        */
#define DAA      0x0400        /* Authoritative answer     */
#define DTC      0x0200        /* Truncation, response was cut off at 512 */
#define DRD      0x0100        /* Recursion desired        */
#define DRA      0x0080        /* Recursion available      */
#define DRCODE   0x000F        /* response code, see below */

/* opcode possible values:
 */
#define DOPQUERY 0             /* a standard query                   */
#define DOPIQ    1             /* an inverse query                   */
#define DOPCQM   2             /* a completion query, multiple reply */
#define DOPCQU   3             /* a completion query, single reply   */

/* the rest reserved for future
 *
 * legal response codes:
 */
#define DROK     0             /* okay response                */
#define DRFORM   1             /* format error                 */
#define DRFAIL   2             /* their problem, server failed */
#define DRNAME   3             /* name error, we know name doesn't exist */
#define DRNOPE   4             /* no can do request                  */
#define DRNOWAY  5             /* name server refusing to do request */

#define DTYPEA   1             /* host address resource record (RR)  */
#define DTYPEPTR 12            /* a domain name ptr                  */

#define DIN      1             /* ARPA internet class */
#define DWILD    255           /* wildcard for several of the classifications */

/*
 * A resource record is made up of a compressed domain name followed by
 * this structure.  All of these ints need to be byteswapped before use.
 */
struct rrpart {
       WORD   rtype;           /* resource record type = DTYPEA    */
       WORD   rclass;          /* RR class = DIN                   */
       DWORD  ttl;             /* time-to-live, changed to 32 bits */
       WORD   rdlength;        /* length of next field             */
       BYTE   rdata[DOMSIZE];  /* data field                       */
     };

/*
 * Data for domain name lookup
 */
static struct useek {
       struct dhead h;
       BYTE   x [DOMSIZE];
     } *question;

static const char *loc_domain;

/*
 * Packdom()
 *   pack a regular text string into a packed domain name, suitable
 *   for the name server.
 *
 *   returns length
 */
static int Packdom (char *dst, char *src)
{
  char *p, *q, *savedst;
  int  i, dotflag, defflag;

  p = src;
  dotflag = defflag = 0;
  savedst = dst;

  do                         /* copy whole string */
  {
    *dst = 0;
    q = dst + 1;
    while (*p && (*p != '.'))
        *q++ = *p++;

    i = p - src;
    if (i > 0x3f)
       return (-1);
    *dst = i;
    *q = 0;

    if (*p)                /* update pointers */
    {
      dotflag = 1;
      src = ++p;
      dst = q;
    }
    else if (!dotflag && !defflag && loc_domain)
    {
      p = (char*)loc_domain;      /* continue packing with default */
      defflag = 1;
      src = p;
      dst = q;
    }
  }
  while (*p);
  q++;
  return (q - savedst);           /* length of packed string */
}

/*
 * Unpackdom()
 *   Unpack a compressed domain name that we have received from another
 *   host.  Handles pointers to continuation domain names -- buf is used
 *   as the base for the offset of any pointer which is present.
 *   returns the number of bytes at src which should be skipped over.
 *   Includes the NULL terminator in its length count.
 */
static int Unpackdom (char *dst, size_t dst_size, char *src, char *buf)
{
  char *savesrc = src;
  int   i, j, retval = 0;

  while (*src && dst < dst+dst_size)
  {
    j = *src;

    while ((j & 0xC0) == 0xC0)
    {
      if (!retval)
         retval = src - savesrc + 2;
      src++;
      src = &buf[(j & 0x3f)*256+*src];       /* pointer dereference */
      j = *src;
    }

    src++;
    for (i = 0; i < (j & 0x3F); i++)
        *dst++ = *src++;

    *dst++ = '.';
  }

  *(--dst) = '\0';           /* add terminator */
  src++;                     /* account for terminator on src */

  if (!retval)
     retval = src - savesrc;

  return (retval);
}

/*
 * SendDom()
 *   put together a domain lookup packet and send it
 *   uses port 53
 *       num is used as identifier
 */
static WORD SendDom (char *s, DWORD towho, WORD num)
{
  WORD  i, ulen;
  BYTE *pSave, *p;

  pSave = (BYTE*) &question->x;
  i     = Packdom ((char*)pSave,s);

  p = (BYTE*) &question->x[i];
  *p++ = 0;                    /* high BYTE of qtype  */
  *p++ = DTYPEA;               /* number is < 256, so we know high byte=0 */
  *p++ = 0;                    /* high BYTE of qclass */
  *p++ = DIN;                  /* qtype is < 256      */

  question->h.ident = intel16 (num);
  ulen = sizeof(struct dhead) + (p - pSave);

  udp_open (dom_sock, DOM_SRC_PORT, towho, DOM_DST_PORT, NULL);

  sock_write (dom_sock, (const BYTE*)question, ulen);
  return (ulen);
}

/*
 * SendRDom()
 *   put together a reverse domain lookup packet and send it
 *   uses port 53
 *       num is used as identifier
 */
static WORD SendRDom (char *s, DWORD towho, WORD num)
{
  WORD  i, ulen;
  BYTE *pSave, *p;

  pSave = (BYTE*) &question->x;
  i = Packdom ((char*)&question->x, s);

  p = &question->x[i];
  *p++ = 0;                           /* high BYTE of qtype */
  *p++ = DTYPEA;                      /* want host address */
  *p++ = 0;                           /* high BYTE of qclass */
  *p++ = DIN;                         /* qtype is < 256 */

  question->h.ident = intel16 (num);
  ulen = sizeof(struct dhead) + (p - pSave);

  udp_open (dom_sock, DOM_SRC_PORT, towho, DOM_DST_PORT, NULL);

  sock_write (dom_sock, (const BYTE*)question, ulen);
  return (ulen);
}

static int Countpaths (const char *pathstring)
{
  int   count = 0;
  const char *p;

  for (p = pathstring; *p || *(p+1); p++)
  {
    if (*p == 0)
       count++;
  }
  return (++count);
}

static const char *
Getpath (const char *pathstring, /* the path list to search        */
         int   whichone)         /* which path to get, starts at 1 */
{
  const char *retval;

  if (whichone > Countpaths(pathstring))
     return (NULL);

  whichone--;
  for (retval = pathstring; whichone; retval++)
  {
    if (*retval == 0)
       whichone--;
  }
  return (retval);
}

/*
 * Ddextract()
 *  extract the ip number from a response message.
 *  returns the appropriate status code and if the ip number is available,
 *  copies it into mip
 */
static DWORD Ddextract (struct useek *qp, BYTE *mip)
{
  struct rrpart *rrp;
  WORD   i, j = 0, nans, rcode;
  char  *p, space[260];

  nans  = intel16 (qp->h.ancount);         /* number of answers */
  rcode = DRCODE & intel16(qp->h.flags);   /* return code for this msg */
  if (rcode > 0)
     return (rcode);

  if (nans > 0 &&                          /* at least one answer   */
      (intel16(qp->h.flags) & DQR))        /* response flag is set  */
  {
    p = (char*)&qp->x;                     /* where question starts */
    i = Unpackdom (space, sizeof(space), p, (char*)qp);

    /* spec defines name then  QTYPE + QCLASS = 4 bytes
     */
    p += i + 4;
    printf (" NAME : ? %s\n", space);

    /* At this point, there may be several answers.  We will take the first
     *  one which has an IP number.  There may be other types of answers that
     *  we want to support later.
     */
    while (nans-- > 0)                     /* look at each answer   */
    {
      i = Unpackdom (space, sizeof(space), p, (char*)qp);
      p += i;                              /* account for string    */
      rrp = (struct rrpart*) p;            /* resource record here  */

      /* check things which might not align on 68000 chip one BYTE at a time
       */
      if (!*p && *(p+1) == DTYPEA &&       /* correct type and class */
          !*(p+2) && *(p+3) == DIN)
      {
        memmove (rrp->rdata, mip, 4);      /* save IP #            */
        return (0);                        /* successful return    */
      }
      memmove (&rrp->rdlength, &j, 2);
      p += 10 + intel16 (j);               /* length of rest of RR */
    }
  }

  return (-1);                            /* generic failed to parse */
}

/*
 * Udpdom()
 *   Look at the results to see if our DOMAIN request is ready.
 *   It may be a timeout, which requires another query.
 */
static DWORD Udpdom (void)
{
  int   i, uret;
  DWORD desired = 0;

  uret = sock_fastread (dom_sock, (BYTE*)question, sizeof(struct useek));

  if (uret < 0)    /* this does not happen */
     return (-1);

  /* check to see if the necessary information was in the UDP response
   */
  i = Ddextract (question, (BYTE*)&desired);
  switch (i)
  {
    case 3:  return (0);              /* name does not exist    */
    case 0:  return (intel(desired)); /* we found the IP number */
    case -1: return (0);              /* strange return code from ddextract */
    default: return (0);              /* dunno */
  }
}

/*
 * Sdomain()
 *   DOMAIN based name lookup
 *   query a domain name server to get an IP number
 *   Returns the machine number of the machine record for future reference.
 *   Events generated will have this number tagged with them.
 *   Returns various negative numbers on error conditions.
 *
 *   if add_dom is non-zero, add default domain
 */
static DWORD Sdomain (const char *name, int add_dom, DWORD nameserver, int *timedout)
{
  char  namebuff[512];
  int   i, is_a_name;
  DWORD response;

  is_a_name = !isaddr (name);
  response  = 0;
  *timedout = 1;

  if (!nameserver)   /* no nameserver, give up now */
  {
    outs ("No nameserver defined!\n\r");
    return (0);
  }

  question->h.flags   = intel16 (DRD);   /* use recursion */
  question->h.qdcount = 0;
  question->h.ancount = 0;
  question->h.nscount = 0;
  question->h.arcount = 0;

  strncpy (namebuff, name, sizeof(namebuff)-1);

  if (is_a_name)
  {
    /* forward lookup
     * only add these things if we are doing a real name
     */
    question->h.qdcount = intel16(1);
    if (add_dom)
    {
      if (namebuff[strlen(namebuff)-1] != '.') /* if no trailing dot */
      {
        if (loc_domain)           /* there is a search list */
        {
          strcat (namebuff,".");
          strcat (namebuff,Getpath(loc_domain,1));
        }
      }
      else
        namebuff [strlen(namebuff)-1] = 0;  /* kill trailing dot */
    }
  }
  else
    question->h.ancount = intel16(1);    /* reverse lookup */

  /*
   * This is not terribly good, but it attempts to use a binary
   * exponentially increasing delays.
   */

   for (i = 2; i < 17; i *= 2)
   {
     if (is_a_name)
          SendDom (namebuff, nameserver, 0xF001);    /* try UDP */
     else SendRDom (namebuff, nameserver, 0xF001);

     ip_timer_init (dom_sock, i);
     do
     {
       kbhit();
       tcp_tick (dom_sock);
       if (ip_timer_expired(dom_sock))
          break;
       if (_watt_cbroke)
          break;
       if (chk_timeout(timeoutwhen))
          break;
       if (sock_dataready(dom_sock))
          *timedout = 0;
     }
     while (*timedout);

     if (!*timedout) /* got an answer */
        break;
  }

  if (!*timedout)
      response = Udpdom();    /* process the received data */

  sock_close (dom_sock);
  return (response);
}

/*
 * NextDomain():
 *   given domain and count = 0,1,2,..., return next larger
 *   domain or NULL when no more are available
 */
static char *NextDomain (char *domain, int count)
{
  char *p;
  int i;

  p = domain;

  for (i = 0; i < count; ++i)
  {
    p = strchr( p, '.' );
    if (!p)
       return (NULL);
    ++p;
  }
  return (p);
}

/*
 * new_resolve():
 *   convert domain name -> address resolution.
 *   returns 0 if name is unresolvable right now
 */
DWORD new_resolve (const char *name)
{
  DWORD      ip_address = 0L;
  WORD       old_hndlcbrk;
  int        count, i;
  int        timeout [MAX_NAMESERVERS];
  struct     useek qp;
  udp_Socket ds;

  question = &qp;
  dom_sock = &ds;

  if (!dns_timeout)
       dns_timeout = sock_delay << 2;
  timeoutwhen = set_timeout (1000 * dns_timeout);

  count = 0;
  memset (&timeout, 0, sizeof(timeout));

  old_hndlcbrk = _watt_handle_cbreak;
  _watt_handle_cbreak = 1;     /* enable special interrupt mode */
  _watt_cbroke = 0;
  do
  {
    loc_domain = NextDomain (def_domain, count);
    if (!loc_domain)
       count = -1;     /* use default name */

    for (i = 0; i < last_nameserver; ++i)
        if (!timeout[i])
        {
           ip_address = Sdomain (name, count != -1,
                                 def_nameservers[i], &timeout[i]);
           if (ip_address)
              break;      /* got name, bail out of loop */
        }

    if (count == -1)
       break;
    count++;
  }
  while (!ip_address);
  _watt_handle_cbreak = old_hndlcbrk;
  return (ip_address);
}


int W32_CDECL main (int argc, char **argv)
{
  char *name;
  DWORD host;
  char  buffer [20];

  if (argc < 2)
  {
    puts("HOST hostname");
    return (3);
  }

  dbug_init();
  sock_init();
  name = strdup (argv[1]);
  printf ("resolving \"%s\"...", name);
  host = new_resolve (name);

  if (host)
  {
    puts ("");
    printf ("%s : %s\n", name, _inet_ntoa(buffer,host));
  }
  free (name);
  return (host ? 0 : 1);
}

