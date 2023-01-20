/*
 * This is mainly to test if sockets might be used with djgpp's
 * fdopen() and stdio oriented calls (fgets, fprintf). It doesn't
 * seems it's implemented as of djgpp 2.03.
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "sysdep.h"

#if !defined(__DJGPP__)
#error "For '__DJGPP__' only."
#endif

int main (void)
{
#ifdef NOT_USED
  struct linger      linger;
#endif
  struct sockaddr_in sin;
  int    s, on = 1;
  FILE  *fil;

  dbug_init();

  s = socket (AF_INET, SOCK_DGRAM, 0);
  if (s < 0)
  {
    perror ("socket");
    return (-1);
  }

  fil = fdopen (s, "r+");
  if (!fil)
  {
    perror ("fdopen");
    return (-1);
  }

  memset (&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port   = htons (6543);

  if (bind(s, (struct sockaddr*)&sin, sizeof(sin)) < 0)
  {
    perror ("bind");
    return (-1);
  }

  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0 )
  {
    perror ("setsockopt");
    return (-1);
  }

  while (!kbhit())
  {
    char msg [50];
    char buf [50];

    gethostname (buf, sizeof(buf));
    if (fprintf(fil, "HELO %s\r\n", buf) < 0)
    {
      perror ("fprintf");
      break;
    }
    if (fgets(msg, sizeof(msg), fil) == NULL)
    {
      perror ("fgets");
      break;
    }
    fputs (msg, stderr);
  }

#ifdef NOT_USED
  linger.l_linger = 500;  /* 5sec */
  linger.l_onoff  = 1;
  if (setsockopt(s, SOL_SOCKET, SO_LINGER, (void*)&linger, sizeof(linger)) < 0)
  {
    perror ("setsockopt");
    return (-1);
  }
#endif

  if (close(s) < 0)
  {
    perror ("close");
    return (-1);
  }
  return (0);
}

