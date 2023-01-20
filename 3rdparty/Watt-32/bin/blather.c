/*
 * blather - a program to generate vat control messages
 *
 * Usage: blather ["ID string"] [ttl] [multicast address] [port]
 *
 *   Blather is a primarly useless program that I hacked together to
 * prove that my multicast code was sending correctly. It will send out
 * a ID string to the specified multicast address/port at 5 second 
 * intervals. By default it will send a silly ID out on MBone Audio with
 * a ttl of 32. Each time an update is sent out, a message will be 
 * displayed. To quit, hit any key (be patient, it can take up to 5 secs)
 *
 * Jim Martin
 * jim@noc.rutgers.edu
 * 11/10/93
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <conio.h>
#include <dos.h>
#include <tcp.h>

#ifdef __DJGPP__
#include <unistd.h>
#endif          

struct CtrlMsg {
       BYTE  version : 2;
       BYTE  flags   : 6;
       BYTE  type;
       WORD  confid;
       char  data;
     };

void help (void)
{
  puts ("blather [-?] [id] [ttl] [host] [port]");
  puts ("  ttl: 1 - same subnet");
  puts ("      32 - same site");
  puts ("      64 - same region");
  puts ("     128 - same continent");
  puts ("     255 - unresticted");
}

int main (int argc, char **argv)
{
  static udp_Socket sock;
  udp_Socket *s = &sock;
  DWORD  host;
  int    ttl;
  WORD   port;
  char   id [512];
  struct CtrlMsg *header;

  if (argc > 1 && argv[1][1] == '?')
     help();

  /* override defaults with command line if available */
  if (argc > 1)
       strcpy (id, argv[1]);
  else strcpy (id, "A random PC blathering for no apparent reason");

  tcp_set_debug_state (2);
  dbug_init();

  _multicast_on = 1;
  sock_init();

  ttl  = (argc > 2) ? atoi   (argv[2]) : 32;
  host = (argc > 3) ? resolve(argv[3]) : resolve("224.2.0.1");
  port = (argc > 4) ? atoi   (argv[4]) : 3457;

  /* First initialize the header structure */
  header = calloc (sizeof(struct CtrlMsg)+strlen(id), 1);
  header->type = 1;
  strcpy (&header->data,id);

  /* Open the UDP connection */
  udp_open (s, port, host, port, NULL);

  /* Join the multicast group */
  if (!join_mcast_group(host))
     printf ("Unable to join group!!!\n");

  /* Set the TTL */
  udp_SetTTL (s, ttl);

  /* And loop until we get a keypress */
  while (!watt_kbhit())
  {
    tcp_tick (NULL);
    sock_write (s, (char*)header, sizeof(struct CtrlMsg)+strlen(id));
    printf ("Sent an ID of: %s \n",id);
    sleep (5);
  }
  sock_close (s);

  /* Leave the multicast group */
  if (!leave_mcast_group(host))
     printf ("Unable to leave group!!!\n");

  return (0);
}


