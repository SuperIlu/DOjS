/*
 * Lister - a program to display vat control messages
 *
 * Usage: lister [multicast address] [port]
 *
 *   Lister is a primarly useless program that I hacked together to
 * prove that my multicast code was receiving correctly. It will list
 * the contents of all of the vat control messages on the address/port
 * pair specified. The default is for MBone Audio, but using the
 * command line arguments, you can specify any session. FYI: The port that
 * is specified on the vat command line or in the options panel is the
 * data port. The control port, the port lister takes, is the one immediately
 * above that. Any keypress will cause lister to exit.
 *
 * Jim Martin
 * jim@noc.rutgers.edu
 * 11/10/93
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <conio.h>
#include <tcp.h>

/* as taken from Van's message in rem-conf on Wed July 8, 1992
 */
struct CtrlMsgHdr {
       BYTE  version : 2;
       BYTE  flags   : 6;
       BYTE  type;
       WORD  confid;
     };

int main (int argc, char **argv)
{
  struct CtrlMsgHdr *control;

  udp_Socket *s = malloc (sizeof(*s));
  char  *buffer = malloc (8192);
  char  *spare  = malloc (1500);
  DWORD  host;
  WORD   port;

  if (!s || !buffer || !spare)
  {
    printf ("No memory for buffer/socket\n");
    return (-1);
  }

  tcp_set_debug_state (2);
  dbug_init();

  _multicast_on = 1;
  sock_init();

  /* override defaults with command line if available
   */
  if (argc > 1)
       host = resolve (argv[1]);
  else host = resolve ("224.2.0.1");

  if (!IN_MULTICAST(host))
  {
    printf ("%s is not a class-D address\n", _inet_ntoa(buffer,host));
    return (-1);
  }

  if (argc > 2)
       port = atoi (argv[2]);
  else port = 3457;

  /* Open the udp connection
   */
  if (!udp_open(s, port, host, port, NULL))
  {
    printf ("Unable to open udp connection\n");
    free (s);
    return (1);
  }

  /* Join the multicast group
   */
  if (!join_mcast_group(host))
     printf ("Unable to join group!!!\n");

  if (sock_recv_init(s,buffer,sizeof(buffer)))
  {
    printf ("Could not enable large buffers!\n");
    free (s);
    return (3);
  }

  /* turn off UDP checksums
   */
  sock_mode (s, UDP_MODE_NOCHK);

  /* loop through until someone whacks a key
   */
  while (!watt_kbhit())
  {
    tcp_tick (NULL);

    if (sock_recv(s,spare,sizeof(spare)))
    {
      control = (struct CtrlMsgHdr *)spare;
      printf ("Vers: %u Flags: %u Type: %u ConfID: %u ID: %s \n",
              control->version, control->flags,
              control->type, ntohs(control->confid),
              spare + sizeof(struct CtrlMsgHdr));
    }
  }

  sock_close (s);

  if (!leave_mcast_group(host))
     printf ("Unable to leave group!!!\n");

  free (s);
  return (0);
}

