#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include <tcp.h>

#ifdef __GNUC__
#include <unistd.h>
#endif

#ifdef __HIGHC__
#include <mw/conio.h>
#endif

#define TALK_PORT 23

struct text_info ti;
tcp_Socket  s;

static void init (void)
{
  gettextinfo (&ti);
  puts ("\7\7");
  textbackground (MAGENTA);
  textcolor (WHITE);
  clrscr();
  cputs ("  messages:");
  gotoxy (1,ti.screenheight-1);
  cputs ("  current line:");

  window (2,2,ti.screenwidth-1,ti.screenheight-2);
  textbackground (DARKGRAY);
  clrscr();
  window (1,ti.screenheight-1,ti.screenwidth,ti.screenheight);
  clrscr();
  gotoxy (1,2);
}

static void add_msg (char *source, char *msg)
{
  window (2,2,ti.screenwidth-1,ti.screenheight-2);
  gotoxy (1,1);
  delline();
  gotoxy (1,21);
  cprintf ("%8.8s : %s", source, msg);
  window (1,ti.screenheight-1,ti.screenwidth,ti.screenheight);
  gotoxy (1,2);
}

int main (int argc, char **argv)
{
  DWORD remoteip;
  char  *host;
  char  dummyuser [80], dummyruser[80], dummyhost[80];
  char  buffer [80], rbuffer [80], ch;
  WORD  position;
  WORD  sendit, who_closed = 0;
  char  *user, *remoteuser;
  int   status;

  sock_init();

  clrscr();
  cputs ("TCPTALK : (experimental version)\n\n\r");

  user = "me";        /* default */

  if (argc < 2)
  {
    cputs ("TCPTALK  [remote_user_id@]remote_host  [my_user_id]");
    cputs ("\n\r\nWaiting for an incomming call\n\r\n");
    cprintf ("(My address is [%s]\n\r", _inet_ntoa(buffer,_gethostid()));
    tcp_listen (&s,TALK_PORT,0,0,NULL,0);
    sock_mode (&s, TCP_MODE_ASCII);
    sock_wait_established (&s, 0, NULL, &status);
    cprintf ("Connection established\r");
    sock_wait_input (&s, sock_delay, NULL, &status);
    sock_gets (&s, dummyhost, sizeof(dummyhost));
    sock_wait_input (&s, sock_delay, NULL, &status);
    sock_gets (&s, remoteuser = dummyruser, sizeof(dummyruser));
    sock_puts (&s, "ok");
    sound (1000);
    cputs ("\n\rPress any to go to TALK session.\n\r");
    getch();
    nosound();
    sock_puts (&s, "<answering your call>");
    init();
  }
  else
  {
    remoteuser = "other";
    if ((host = strchr(argv[1],'@')) != NULL)
    {
      *host++ = 0;
      remoteuser = argv[1];
    }
    else
      host = argv[1];

    if ((remoteip = resolve(host)) == 0)
    {
      textcolor (RED);
      cprintf ("\n\rUnable to resolve `%s'\n\r", host);
      return 3;
    }

    if (argc < 3)
    {
      cputs ("Userid to assume:");
      user = gets (dummyuser);
      puts ("\n\r");
    }
    else
      cprintf ("Using `%s' as local userid\n\r", user);

    if (!tcp_open(&s, 0, remoteip, TALK_PORT, NULL))
    {
      cputs ("Unable to open connection.");
      return 1;
    }
    sock_mode (&s, TCP_MODE_ASCII);
    sock_wait_established (&s, sock_delay,NULL, &status);
    sock_puts (&s, _inet_ntoa(buffer,_gethostid()));
    sock_puts (&s, user);
    sock_wait_input (&s, sock_delay, NULL, &status);

    sock_gets (&s, buffer, sizeof(buffer));
    if (stricmp(buffer, "ok"))
    {
      sock_close (&s);
      cputs ("Remote side did not wish to connect");
      cprintf ("MSG: %s\n", buffer);
      sock_wait_closed (&s, sock_delay, NULL, &status);
      return 1;
    }
    init();
    add_msg (remoteuser, "< remote user has not answerred yet, waiting...>");
    sock_wait_input (&s,0,NULL,&status);
  }

  /* we are connected */

  add_msg (remoteuser, "connected");
  *buffer = position = sendit = 0;

  while (tcp_tick(&s))
  {
    if (watt_kbhit())
    {
      if ((ch = getch()) == 27)
      {
        sock_close (&s);
        who_closed = 1;
        break;
      }
      switch (ch)
      {
        case '\r': sendit = 1;
                   break;
        case '\b': buffer [--position] = 0;
                   delline();
                   cputs (buffer);
                   break;
        case '\t': ch = ' ';
        default  : buffer [position++] = ch;
                   buffer [position] = 0;
                   gotoxy(1,2);
                   clreol();
                   cputs(buffer);
      }
      if (sendit)
      {
        sock_puts (&s, buffer);
        add_msg (user, buffer);
        delline();
        sendit = 0;
        position = *buffer = 0;
      }
    }

    if (sock_dataready(&s))
    {
      sock_gets (&s, rbuffer, sizeof(rbuffer));
      add_msg (remoteuser, rbuffer);
    }
  }

  delline();
  textcolor (RED);
  if (who_closed == 1)
  {
    cputs (" *** YOU CLOSED CONNECTION *** ");
    sock_wait_closed (&s,sock_delay,NULL,&status);
  }
  else
    cputs (" *** OTHER PERSON CLOSED CONNECTION *** ");

  sleep (1);
  return 0;

sock_err:
  switch (status)
  {
    case 1 : cputs ("Connection closed");
             break;
    case -1: cputs ("REMOTE HOST CLOSED CONNECTION");
             break;
  }
  return 0;
}

