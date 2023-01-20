/******************************************************************************

  REXEC - remotely execute a UNIX command

  Copyright (C) 1991, University of Waterloo

  This program is free software; you can redistribute it and/or modify
  it, but you may not sell it.

  This program is distributed in the hope that it will be useful,
  but without any warranty; without even the implied warranty of
  merchantability or fitness for a particular purpose.

      Erick Engelke                   or via E-Mail
      Faculty of Engineering
      University of Waterloo          Erick@development.watstar.uwaterloo.ca
      200 University Ave.,
      Waterloo, Ont., Canada
      N2L 3G1

******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <conio.h>
#include <tcp.h>

#define EXEC_PORT 512

char *getpass (const char *prompt);

char cmdbuf [2048];

char lname [64];               /* local copies if none supplied */
char lpass [64];
char lcmd [255];

int makecmdbuf (const char *name, const char *pass, const char *cmd)
{
  return sprintf (cmdbuf, "%c%s%s%s", 0, name, pass, cmd);
}

const char *my_fgets (char *buf, size_t sz)
{
  const char *rc = fgets (buf, sz, stdin);

  if (!rc || !*rc || *rc == '\n')
     return (NULL);
  return (rc);
}

int exec (const char *hostname, WORD port, const char *name, char *pass, const char *cmd)
{
  WORD   lport, jqpublic, count;
  DWORD  host;
  int    status, len;
  char   buffer [1024];
  static tcp_Socket rsh_sock;

  srand ((unsigned int)time(NULL));
  lport = (rand() & 512) + 512;       /* return 511 < port < 1024 */

  host = lookup_host (hostname, NULL);
  if (host == 0)
  {
    printf ("%s", dom_strerror(dom_errno));
    return (2);
  }

  jqpublic = 0;
  if (!name)
  {
    printf (" Userid   : ");
    name = my_fgets (lname, sizeof(lname));
    if (!name)
    {
      name = "JQPUBLIC";
      printf ("%s", name);
      jqpublic = 1;
    }
  }
  if (!pass)
  {
    if (jqpublic)
         pass = "";
    else strcpy (pass = lpass, getpass(" Password : "));
       /* copy for neatness since getpass overwrites */
  }
  if (!cmd)
  {
    printf (" Command  : ");
    cmd = my_fgets (lcmd, sizeof(lcmd));
    if (!cmd)
    {
      puts ("No command given\n");
      return (2);
    }
  }

  if (!tcp_open(&rsh_sock, lport, host, port, NULL))
  {
    printf ("Remote host unaccessible");
    return (1);
  }
  printf ("waiting for remote host to connect...\r");
  sock_wait_established (&rsh_sock, sock_delay, NULL, &status);

  printf ("remote host connected, waiting verification...\r");

  len = makecmdbuf (name, pass, cmd);
  sock_write (&rsh_sock, (const BYTE*)cmdbuf, len);

  while (1)
  {
    sock_tick (&rsh_sock, &status);
    if (!sock_dataready(&rsh_sock))
       continue;
    sock_fastread (&rsh_sock, (BYTE*)buffer, 1);
    printf ("                                              \r");
    if (*buffer == 1)
       printf ("RSH failed...\n\r");
    break;
  }

  while (1)
  {
    if (watt_kbhit())
       sock_putc (&rsh_sock, getch());
    sock_tick (&rsh_sock, &status);
    if (sock_dataready(&rsh_sock))
    {
      count = sock_fastread (&rsh_sock, (BYTE*)buffer, sizeof(buffer));
      fwrite (buffer, count, 1, stdout);
    }
  }

sock_err:
  switch (status)
  {
    case 1 : puts ("\nConnection closed");
             break;
    case-1 : printf ("ERROR: %s\n", sockerr(&rsh_sock));
             break;
  }
  return (status == 1 ? 0 : 1);
}

void help (void)
{
  puts ("REXEC [-d] host [username [password]] cmdstring");
  puts ("The values for cmdstring should be placed inside quotes");
  exit (3);
}

int MS_CDECL main (int argc, char **argv)
{
  char *hostname = NULL;
  char *name     = NULL;
  char *pass     = NULL;
  char *cmd      = NULL;

  if (argc > 1 && !strcmp("-d",argv[1]))
  {
    dbug_init();
    argc--;
    argv++;
  }

  switch (argc)
  {
    case 5:
         pass = argv[3];
    case 4:
         name = argv[2];
    case 3:
         cmd = argv[argc-1];
    case 2:
         break;
    default:
         help();
  }

  sock_init();
  hostname = argv[1];

  return exec (hostname, EXEC_PORT, name, pass, cmd);
}


#if defined(__HIGHC__) || defined(_MSC_VER) || defined(__WATCOMC__) || \
    defined(__DMC__) || defined(__MINGW32__)
char *getpass (const char *prompt)
{
  static char pass[21];
  char *p = pass;

  printf ("%s", prompt);
  fflush (stdout);

  while (1)
  {
    int ch = getch();

    if (isprint(ch))
    {
      *p++ = (char)ch;
      putch ('*');
      continue;
    }
    if (ch == '\r')
      break;

    if (ch == '\b' && p != pass)
    {
      p--;
      putch ('\b');
      putch (' ');
      putch ('\b');
    }
    if (p == pass+sizeof(pass)-1)
       break;
  }
  *p = '\0';
  return (pass);
}
#endif

