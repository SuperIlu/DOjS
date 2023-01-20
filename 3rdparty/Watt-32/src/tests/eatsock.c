/*
 * A test for how Watt-32 handles creating many sockets.
 * Study the wattcp.sk file (and/or MSVC CRTDBG report)
 * for mem-leaks.
 */
#include <assert.h>

#include "socket.h"
#include "pcdbug.h"
#include "profile.h"

WORD  test_port = 1234;
int   max_socks = 0;
int   s_type;
int  *socks = NULL;

void setup (void)
{
  printf ("eatsock: %s\n", wattcpVersion());
  printf ("CFLAGS: %s\n", wattcpBuildCflags());
  dbug_init();
  sock_init();

#if defined(USE_PROFILER)
  if (!profile_enable)
  {
    profile_enable = 1;
    if (!profile_init())
       exit (-1);
  }
#endif
}

/*--------------------------------------------------------------------------*/

static int get_sock_type (const char *type)
{
  if (!stricmp(type,"SOCK_RAW"))
     return (SOCK_RAW);
  if (!stricmp(type,"SOCK_PACKET"))
     return (SOCK_PACKET);
  if (!stricmp(type,"SOCK_DGRAM"))
     return (SOCK_DGRAM);
  if (!stricmp(type,"SOCK_STREAM"))
     return (SOCK_STREAM);
  return (-1);
}

void Usage (const char *argv0)
{
  printf ("Usage: %s <num-sockets> <SOCK_DGRAM | SOCK_STREAM | SOCK_RAW | SOCK_PACKET>",
          argv0);
  exit (0);
}

int main (int argc, char **argv)
{
  struct sockaddr_in sa;
  int    rc, i;

  if (argc != 3)
     Usage (argv[0]);

  max_socks = atoi (argv[1]);
  if (max_socks == 0)
     Usage (argv[0]);

  s_type = get_sock_type (argv[2]);
  if (s_type == -1)
  {
    printf ("Illegal sock-type '%s'.\n", argv[2]);
    Usage (argv[0]);
  }

  if (s_type == SOCK_STREAM)
     puts ("Testing SOCK_STEAM may report mem-leaks. It's normal\n"
           "(unless you wait >= 3min below).");

  setup();
  socks = calloc (max_socks, sizeof(int));
  assert (socks != NULL);

  for (i = 0; i < max_socks; i++)
  {
    PROFILE_START ("socket");
    socks[i] = socket (AF_INET, s_type, 0);
    PROFILE_STOP();
    if (socks[i] < 0)
    {
      perror ("socket");
      max_socks = i;
      break;
    }
  }

  for (i = 0; i < max_socks; i++)
  {
    sa.sin_family      = AF_INET;
    sa.sin_port        = htons (test_port);
    sa.sin_addr.s_addr = INADDR_ANY;
    PROFILE_START ("bind");
    rc = bind (socks[i], (struct sockaddr*)&sa, sizeof(sa));
    PROFILE_STOP();
    if (rc < 0)
    {
      perror ("bind");
      break;
    }
  }

  for (i = 0; i < max_socks; i++)
  {
    PROFILE_START ("close_s");
    close_s (socks[i]);
    PROFILE_STOP();
  }
  free (socks);

  puts ("Waiting for SOCK_STREAM sockets to die. Press any key to exit.");
  while (!kbhit())
  {
    Wait (100);
    tcp_tick (NULL);
    putchar ('.');
    fflush (stdout);
  }
  return (0);
}

