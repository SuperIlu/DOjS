/******************************************************************************
    COOKIE - read and print a witty saying from internet

    By: Jim Martin                      Internet: jim@dorm.rutgers.edu
        Dormitory Networking Project    UUCP: {backbone}!rutgers!jim
        Rutgers University              Phone: (908) 932-3719

    Uses the Waterloo TCP kernel.
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tcp.h>

#define COOKIE_PORT 17

int get_cookie (DWORD host, int all_jars)
{
  udp_Socket sock;
  void      *s = (void*) &sock;
  char       buffer [1460];
  int        len, i, status = 0;

  if (host)
     status = udp_open (s, 0, host, COOKIE_PORT, NULL);
  else
  {
    if (!last_cookie)
    {
      puts ("Sorry, I can't seem to remember where my cookie jars are.\n"\
            "Could you tell me where one is?");
      return (3);
    }
    for (i = 0; i < last_cookie; i++)
    {
      status = udp_open (s, 0, Cookies[i], COOKIE_PORT, NULL);
      if (status && !all_jars)
         break;
    }
  }

  if (!status)
  {
    puts ("None of the cookie jars are open!");
    return 1;
  }

  sock_putc (s, '\n');

  sock_wait_input (s, sock_delay, NULL, &status);
  len = sock_fastread (s, (BYTE*)buffer, sizeof(buffer)-1);
  buffer[len] = 0;
  puts (buffer);

sock_err:
  if (status)
     printf ("%s\n\r", sockerr(s));
  sock_close (s);
  return (0);
}

void Usage (void)
{
  puts ("Quote of the Day (Cookie) - retrieves a witty message\n"\
        "Usage: COOKIE [-ad] [-s server]");
  exit (1);
}

/*----------------------------------------------------------------*/

int MS_CDECL main (int argc, char **argv)
{
  int   ch;
  int   all = 0;
  char *host = NULL;
  DWORD ip;

  while ((ch = getopt(argc, argv, "?das:")) != EOF)
     switch (ch)
     {
       case 'd': dbug_init();
                 break;
       case 'a': all = 1;
                 break;
       case 's': host = optarg;
                 break;
       default : Usage();
     }

  sock_init();

  if (!host)
     return get_cookie (0L, all);

  ip = resolve (host);
  if (ip)
     return get_cookie (ip, 0);

  printf ("`%s' is not a cookie jar\n", host);
  return (0);
}

