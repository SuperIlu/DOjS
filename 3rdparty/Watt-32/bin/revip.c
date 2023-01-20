/*
 *  Reverse IP lookup: given a set of IP addresses, resolves
 *  to hostname(s).
 *
 *  G.Vanem <gvanem@yahoo.no>. Created 13-Sep-96
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <tcp.h>
#include <netdb.h>

void usage (void)
{
  printf ("Usage: revip [-d -6] ip-address(es) ..\n");
  exit (0);
}

void lookup4 (const char *ip)
{
  struct hostent *h;
  struct in_addr  addr;

  if (!inet_aton (ip, &addr))
  {
    printf ("`%s' is not a valid IPv4-address\n", ip);
    return;
  }
  printf ("Looking up %s..", ip);
  fflush (stdout);

  h = gethostbyaddr ((char*)&addr.s_addr, sizeof(addr.s_addr), AF_INET);
  printf ("\r%-15.15s: hostname is `%s'\n",
          ip, h ? h->h_name : "* unknown *");
}

void lookup6 (const char *ip)
{
  struct hostent *h;
  struct in6_addr addr;

  if (!inet_pton (AF_INET6,ip,&addr))
  {
    printf ("`%s' is not a valid IPv6-address\n", ip);
    return;
  }
  printf ("Looking up %s..", ip);
  fflush (stdout);

  h = gethostbyaddr ((char*)&addr, sizeof(addr), AF_INET6);
  printf ("\r%-45.45s: hostname is `%s'\n",
          ip, h ? h->h_name : "* unknown *");
}

int MS_CDECL main (int argc, char **argv)
{
  int af = AF_INET;
  int i = 1;
  int ch;

  while ((ch = getopt(argc, argv, "d6h?")) != EOF)
     switch (ch)
     {
       case 'd':
            dbug_init();
            break;
       case '6':
            af = AF_INET6;
            break;
       case 'h':
       case '?':
       default:
            usage();
     }

  argc -= optind;
  argv += optind;
  if (argc < 1)
     usage();

  sock_init();

  for (i = 0; i < argc; i++)
      (af == AF_INET6) ? lookup6 (argv[i]) : lookup4 (argv[i]);
  return (0);
}
