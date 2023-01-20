/*
 * PH.EXE - CSO NameServer Client
 *
 * PH, the predecessor to LDAP:
 *   https://en.wikipedia.org/wiki/CCSO_Nameserver
 *
 * Don Genner (acddon@vm.uoguelph.ca) wrote this clever little CSO
 * client.  You may wish to use it and a DOS batch file to set up a
 * simple phonebook command:
 *
 *  eg.     ph  localserver  "query name=%1"
 *
 * or something much more complex.
 *
 * To get more details on the protocol and local features, use something
 * like:    ph  localserver  "help"
 *
 *
 * Syntax:  Stand-alone:    PH  CSO-NameServer Request
 *                          --------------------------
 *
 *   eg:  PH csaserver.uoguelph.ca "query name=genner return name email"
 *               {Results}
 *
 *        PH csaserver.uoguelph.ca "query name=g*"
 *              {Results}
 *
 *        PH csaserver.uoguelph.ca "query g*"
 *              {Results}
 *
 *
 * Syntax:  Conversational:   PH  CSO-NameServer
 *                            ------------------
 *
 *   or:  PH csaserver.uoguelph.ca
 *            Server : csaserver.uoguelph.ca
 *              {Results}
 *
 *           Request : query genner
 *              {Results}
 *
 *           Request : siteinfo
 *              {Results}
 *
 *           Request : fields
 *              {Results}
 *
 *           Request : quit
 *              {Results}
 *
 *
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tcp.h>

#if defined(__CYGWIN__)
#define strnicmp(s1, s2, len)   strncasecmp (s1, s2, len)
#endif

#define PH_PORT 105

static tcp_Socket  phsock;
static char        buffer[515];

const char *my_fgets (char *buf, size_t sz)
{
  const char *rc = fgets (buf, sz, stdin);

  if (!rc || !*rc || *rc == '\n')
     return (NULL);
  return (rc);
}

static int ph (const char *cmd, DWORD host)
{
  tcp_Socket *s;
  int status;
  int len, prsw, crsw, i;

  s = &phsock;
  if (!tcp_open(s,0,host,PH_PORT,NULL))
  {
    puts (" Sorry, unable to connect to that machine right now!");
    return 1;
  }

  printf (" waiting...");
  fflush (stdout);
  sock_wait_established (s, sock_delay, NULL, &status);

  strcpy (buffer, cmd);
  rip (buffer);
  strcat (buffer, "\r\n");
  sock_puts (s, (BYTE*)&buffer);

  prsw = -1;
  crsw = -1;
  while (prsw)
  {
    sock_wait_input (s, 30, NULL, &status);
    printf ("\r ");
    len = sock_fastread (s, (BYTE*)&buffer, sizeof(buffer));
    for (i = 0; i < len; i++)
    {
      if (crsw && (buffer[i] != '-'))
         putchar (' ');
      if (crsw && (buffer[i] >= '2'))
         prsw = 0;
      putchar (buffer[i]);
      crsw = 0;
      if (buffer[i] == '\n')
         crsw = -1;
    }
  }
  sock_close (s);
  sock_wait_closed (s, sock_delay, NULL, &status);

sock_err:
  if (status == -1)
  {
    const char *err = sockerr(s);
    printf (" \7Error: %s",err);
    if (!strnicmp(err,"timeout",7))
       exit(1);
  }
  puts ("");
  return status;
}


int MS_CDECL main (int argc, char **argv)
{
  const char *cmd, *server;
  char  lcmd[255];
  DWORD host;
  int   status;

  if (argc < 2 || argc > 3)
  {
    puts ("Usage: PH server [request]");
    return 3;
  }

  dbug_init();
  sock_init();
  cmd = NULL;
  server = argv[1];

  host = resolve (server);
  if (host != 0)
     printf (" Server  : `%s'\n\n", server);
  else
  {
    printf ("Could not resolve host `%s'\n", server);
    return 3;
  }

  if (argc == 3)
  {
    cmd = argv[2];
    status = ph (cmd, host);
  }
  else
  {
    while (1)
    {
      printf (" Request : ");
      cmd = my_fgets (lcmd, sizeof(lcmd));
      if (!cmd)
      {
        puts (" No command given\n");
        return 2;
      }
      if (*cmd == '?')
      {
        puts ("  help | query name=.. | siteinfo | fields |"
              " quit | stop | exit");
        continue;
      }
      status = ph (cmd, host);
      if (!strnicmp(cmd,"quit",4) ||
          !strnicmp(cmd,"stop",4) ||
          !strnicmp(cmd,"exit",4))
         break;
    }
  }
  return (status);
}
