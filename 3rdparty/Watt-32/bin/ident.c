#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <tcp.h>
#include <netdb.h>

#if (defined(__SMALL__) && !defined(DOS386)) || defined(__LARGE__)
  #define SAVE_SPACE
#endif

static int W32_CALL wait_handler (void *sock)
{
  (void) sock;
  return watt_kbhit();
}

int ident (DWORD host, WORD client_port, WORD server_port)
{
  static tcp_Socket sock;
  int    status    = 0;
  WORD   localport = _w32_Random (1100, 5555);
  WORD   identport = 113;
  char   buf[512];
  char   osName[50];
  char   userId[50];
  int    len, num;

#ifndef SAVE_SPACE
  struct servent *serv;

  if ((serv = getservbyname("auth", "tcp")) != NULL ||
      (serv = getservbyname("ident","tcp")) != NULL)
     identport = intel16 (serv->s_port);
#endif

  if (!tcp_open(&sock,localport,host,identport,NULL))
  {
    printf ("Unable to connect; %s", sockerr(&sock));
    return (-1);
  }
#ifdef __DJGPP__
  tcp_md5_secret (&sock, "Hi there");
#endif

  printf ("waiting...");
  fflush (stdout);
  sock_wait_established (&sock, sock_delay, NULL, &status);

  sock_printf (&sock, "%d,%d\r\n", server_port, client_port);

  sock_wait_input (&sock, 10, NULL, &status);

  /*
   * A typical response:
   *   0,0:USERID:OS-name:User-name
   */
  len = sock_fastread (&sock, (BYTE*)&buf, sizeof(buf)-1);
  buf[len] = '\0';
  printf ("\nRaw data: %s\n", buf);

  osName[0] = userId[0] = '\0';
  num = sscanf (buf, "%*d,%*d:USERID:%50[^:]:%[ a-zA-Z0-9]50s",
                osName, userId);
  printf ("Parsed (%d): OS-name `%s', User `%s'\n", num, osName, userId);

  printf ("closing...");
  fflush (stdout);
  sock_close (&sock);
  sock_wait_closed (&sock, 0, wait_handler, &status);
  return (0);

sock_err:
  switch (status)
  {
    case 1:
         puts ("\nHost closed");
         status = 0;
         break;
    case -1:
         printf ("\n%s\n", sockerr(&sock));
         break;
  }
  sock_abort (&sock);
  return (status);
}

/*---------------------------------------------------------------*/

void usage (void)
{
  puts ("Simple RFC-1413 ident client\n"
        "Usage: ident [-v] [-d] [-t] server <server-client-port-pair>\n"
        "       -v  - print version\n"
        "       -d  - enable Watt-32 debug\n"
        "       -t  - use timer ISR for timeouts\n"
        "E.g \"ident some.host.com 23,1030\" queries telnet connection "
        "at port 1030");
  exit (0);
}

int main (int argc, char **argv)
{
  char  *server = "localhost";
  DWORD  host;
  WORD   cli_port = 0;
  WORD   srv_port = 0;
  int    timer_isr = 0;
  int    ch;

  while ((ch = getopt(argc,argv,"?vdt")) != EOF)
     switch(ch)
     {
       case 'v':
            puts (wattcpVersion());
            break;
       case 'd':
            dbug_init();
            break;
       case 't':
            timer_isr = 1;
            break;
       case '?':
       default:
            usage();
     }

  argc -= optind;
  argv += optind;

  if (argc < 1 || (server = *argv) == NULL)
     usage();

  if (argc > 1)
     sscanf (argv[1], "%hu,%hu", &srv_port, &cli_port);

  sock_init();
  if (timer_isr)
  {
#if !defined(WIN32) && !defined(_WIN32) && !defined(__DPMI32__)  /* Win32/PowerPak */
  #if 0
    wintr_init();
    wintr_enable();
  #else
    init_timer_isr();
  #endif
#endif
  }

  host = lookup_host (server, NULL);
  if (host == 0L)
  {
    printf ("%s", dom_strerror(dom_errno));
    return (3);
  }
  return ident (host, cli_port, srv_port);
}


