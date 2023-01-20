#include <stdarg.h>
#include <ctype.h>
#include <setjmp.h>

#include "telnet.h"
#include "screen.h"
#include "config.h"
#include "keyb.h"
#include "ssh.h"

static char   subbuffer [256]; /* buffer for sub-options */
static int    sga_flag  = 0;
static int    echo_mode = 1;
static int    enter_key = '\n';
static time_t start_time;
static WORD   dest_port;

static void SendIAC (BYTE cmd, BYTE opt)
{
  if (connected)
  {
    BYTE data[3];
    data[0] = IAC;
    data[1] = cmd;
    data[2] = opt;
    sock_fastwrite (&sock, data, 3); // !! was sock_write
  }
}

int PutStringTN (const char *fmt, ...)
{
  char    buf[2048];
  int     len = -1;
  va_list args;

  if (connected)
  {
    va_start (args, fmt);
    len = VSPRINTF (buf, fmt, args);
    len = sock_write (&sock, (const BYTE*)&buf, len);
    va_end (args);
  }
  return (len);
}

static void ProcessIAC (void)
{
  int      cmd, opt, y, flag;
  unsigned n;

  if (GetCharTN(&cmd) <= 0)
     return;

  if (cmd == IAC)
     return;

  if (GetCharTN(&opt) <= 0)
     return;

  tel_dump ("%c%c%c", IAC, cmd, opt);

  switch (opt)
  {
    case TELOPT_ECHO:
         if (cmd == WILL)
         {
           if (echo_mode)
           {
             echo_mode = 0;
             SendIAC (DO, TELOPT_ECHO);
           }
         }
         else if (cmd == WONT)
         {
           if (!echo_mode)
           {
             echo_mode = 1;
             SendIAC (DONT, TELOPT_ECHO);
           }
         }
         else if (cmd == DO)
         {
           echo_mode = 0;
           SendIAC (WONT, TELOPT_ECHO);
           SendIAC (DO, TELOPT_ECHO);
         }
         else if (cmd == DONT)
         {
           echo_mode = 0;
           SendIAC (WONT, TELOPT_ECHO);
         }
         break;

    case TELOPT_SGA:
         if (cmd == WONT)
         {
           sga_flag = 1;
           if (!echo_mode)
           {
             SendIAC (DONT, TELOPT_SGA);
             echo_mode = 1;
           }
         }
         else if (cmd == WILL)
         {
           sga_flag = 0;
           if (echo_mode)
           {
             SendIAC (DO, TELOPT_SGA);
             SendIAC (DO, TELOPT_ECHO);
           }
         }
         break;

    case TELOPT_TTYPE:
         if (cmd == DO)
            SendIAC (WILL, TELOPT_TTYPE);
         else if (cmd == SB)
         {
           n = flag = 0;
           while (n < sizeof(subbuffer))
           {
             if (GetCharTN(&y) <= 0)
                break;
             subbuffer[n++] = y;
             tel_dump ("%c", y);
             if (y == IAC)
                flag = 1;
             else
             {
               if (flag && y == SE)
                  break;
               flag = 0;
             }
           }
           if (flag && subbuffer[0] == 1)
              PutStringTN ("%c%c%c%c%s%c%c", IAC, SB, TELOPT_TTYPE,
                           TELQUAL_IS, VT_GetMode(), IAC, SE);
         }
         break;

    default:
         if (cmd == WILL)
            SendIAC (DONT, opt);
         else if (cmd == DO)
         {
           SendIAC (WONT, opt);
           SendIAC (DONT, opt);
         }
         else if (cmd == DONT)
           SendIAC (WONT, opt);
         break;
  }
}

/*
 * Poll for keypress and return translation for key
 */
int GetKeyTN (char *str, size_t max)
{
  const char *trans = NULL;
  int   key = KeyGetKey();

  switch (key)
  {
    case Key_ALTX:
         quit = 1;
         return (0);
    case Key_ALTR:
         VT_Init();
         return (0);
    case 0:
         return (0);   /* no key pressed */
    default:
         break;
  }

  if ((key == Key_CTRL_M || key == Key_CTRL_J) && enter_key == '\r')
       key = Key_CTRL_J;    /* <Enter> -> '\r' */
  else trans = KeyTranslate (key);

  if (trans)
  {
    strncpy (str, trans, max);
    str[max-1] = '\0';
    tel_dump ("\r\nTx: `%s'\r\n", trans);
    return strlen (str);
  }
  *str++ = key;
  *str   = '\0';
  tel_dump ("\r\nTx: %c\r\n", str);
  return (1);
}

/*
 * Telnet background job; print current time and time online
 */
static char *TimeOnline (DWORD t)
{
  static char buf[20];
  int    hour = t / 3600;
  int    mins = t / 60 - 60*hour;
  int    secs = t % 60;

  snprintf (buf, sizeof(buf), "%02d:%02d:%02d", hour, mins, secs);
  return (buf);
}

static void UpdateTime (time_t start)
{
  time_t now = time (NULL);
  DWORD  onl = (DWORD) difftime (now, start);

  /* "Mon Apr 29 13:14:52 1997" */
  /*             ^ +11          */
  SCR_StatusLine (CurrentTime,"%.8s", ctime(&now)+11);
  SCR_StatusLine (OnlineTime, "%s",   TimeOnline(onl));
}

int BackGroundTN (void)
{
  static DWORD sec_timer = 0UL;

  if (sec_timer == 0)
      sec_timer = set_timeout (900);

  else if (chk_timeout(sec_timer))
  {
    UpdateTime (start_time);
    sec_timer = set_timeout (900);
  }
  return (1);
}

/*
 * Our main input handler
 */
void TelnetRecv (void)
{
  int c;

  while (GetCharTN(&c) > 0)
  {
    if (c == IAC)
         ProcessIAC();
    else VT_Process (c);
  }
}

/*
 * Telnet output handler
 */
void TelnetSend (void)
{
  while (connected && sock_tbleft(&sock) > 2)
  {
    char str[20];
    int  len = GetKeyTN (str, sizeof(str));

    if (!len)
       break;
    sock_write (&sock, (const BYTE*)&str, len);
  }
}

int ConnectTN (const char *host, WORD port)
{
  int   status = 0;
  DWORD ip_adr = lookup_host (host, NULL);

  if (ip_adr == 0L)
  {
    cprintf ("Cannot resolve `%s'\r\n", host);
    goto no_ip;
  }

  tel_log ("Connecting to `%s' (port %d)...", host, port);
  cprintf ("Connecting to `%s'...", host);
  time (&start_time);

  if (!tcp_open(&sock, 0, ip_adr, port, NULL))
     goto sock_err;

  sock_wait_established (&sock, cfg.timeout, NULL, &status);
  sock_yield (&sock, (void(*)())BackGroundTN);

  strcpy (hostname, host);
  dest_port = port;
  cprintf ("Connected.\r\n");
  SCR_StatusLine (HostName, host);
  SetColour (DataText);
  connected = 1;

  if (port == IPPORT_TELNET)
  {
    SendIAC (WILL, TELOPT_TTYPE);
    SendIAC (DO,   TELOPT_SGA);
    SendIAC (WONT, TELOPT_ECHO);
    SendIAC (DO,   TELOPT_ECHO);
  }
  else if (port == IPPORT_SSH)
  {
    if (!ConnectSSH())
       goto sock_err;
  }
  else
  {
    enter_key = '\r';
    tel_log ("Using untranslated <Enter> key");
  }

  sock_flush (&sock);
  return (1);

sock_err:
  tel_log ("Connection failed: %s", sockerr(&sock));

no_ip:
  cprintf ("failed.\r\n<Press any key>..");
  getch();
  return (0);
}

void CloseTN (void)
{
  if (connected)
     sock_close (&sock);
  connected = 0;
}

int GetCharTN (int *ch)
{
  if (!connected || !sock_established(&sock))
  {
    connected = 0;
    return (-1);
  }
  if (sock_dataready(&sock))
  {
    BYTE c;

    sock_fastread (&sock, (BYTE*)&c, 1);
    *ch = c;
    return (1);
  }
  return (0);
}

void TelnetExit (void)
{
  const char *err;

  window (1, 1, cfg.scr_width, cfg.scr_height);
  gotoxy (1, cfg.scr_height);
  nosound();
  putchar ('\r');
  err = sockerr (&sock);
  if (err)
     puts (err);
  exit (0);
}
