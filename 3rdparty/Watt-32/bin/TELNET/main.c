#include <stdarg.h>
#include <setjmp.h>
#include <signal.h>
#include <netdb.h>

#include "telnet.h"
#include "config.h"
#include "keyb.h"
#include "screen.h"
#include "script.h"

#ifdef __DJGPP__
#include <sys/exceptn.h>
#endif

char progPath [_MAX_PATH] = ".\\";
char progName [_MAX_PATH] = "telnet";

tcp_Socket sock;

char hostname [MAX_HOSTNAME];
int  quit       = 0;
int  dbug_mode  = 0;
int  verbose    = 0;
int  connected  = 0;

void (*recvState)(void) = TelnetRecv;
void (*xmitState)(void) = TelnetSend;

static void InitMain    (char *argv0);
static void Usage       (void);
static void sig_handler (int sig);

/*
 * Set screen emulation mode
 */
static int SetEmulation (char *emul)
{
  if (!stricmp (emul,"vt52"))
     return VT_SetMode (VT52);

  if (!stricmp (emul,"heath19"))
     return VT_SetMode (HEATH19);

  if (!strnicmp(emul,"vt100",5))
     return VT_SetMode (VT100);

  if (!strnicmp(emul,"vt102",5))
     return VT_SetMode (VT102);

  if (!stricmp (emul,"vt200"))
     return VT_SetMode (VT200);

  if (!stricmp (emul,"ansi"))
     return VT_SetMode (ANSI);

  if (!stricmp (emul,"linux"))
     return VT_SetMode (LINUX);

  printf ("Unknown emulation: `%s'\n", emul);
  return (0);
}

/*
 * Return port for service at 'str'
 */
static WORD GetService (const char *str)
{
  struct servent *se;
  WORD   val = atoi (str);

  if (val > 0)
     return (val);

  se = getservbyname (str, "tcp");
  if (se)
     return ntohs (se->s_port);

  fprintf (stderr, "Illegal port/service `%s'\n", str);
  Usage();
  /* not reached */
  return (0);
}

/*
 * Print a short program usage/description.
 */
static void Usage (void)
{
  fprintf (stderr,
           "Usage: %s [-Vvd] [-K keydriver] [-E emulation] [-s script] host [port]",
           progName);
  exit (0);
}

int main (int argc, char **argv)
{
  WORD  port;
  int   ch;
  char *host   = NULL;
  char *kbd    = "ANSI-AT.KBD";
  char *script = NULL;

  InitMain (argv[0]);

  if (!OpenIniFile("telnet.ini"))
     return (1);

  while ((ch = getopt(argc,argv,"?VvdK:E:s:")) != EOF)
     switch (ch)
     {
       case 'V': puts (wattcpVersion());
                 return (0);

       case 'v': verbose++;
                 break;

       case 'd': dbug_init();
                 dbug_mode = 1;
                 break;

       case 'K': kbd = optarg;
                 break;

       case 'E': if (!SetEmulation(optarg))
                    return (1);
                 break;

       case 's': script = optarg;
                 break;
       case '?':
       case 'h':
       default:  Usage();
                 break;
     }

  argc -= optind;
  argv += optind;

  if (argc < 1)
     Usage();

  tel_log_init();
  tel_dump_init();

  if (!KeyDriver(kbd))
     return (0);

  sock_init();

  signal (SIGINT, sig_handler);

#ifdef DJGPP
  __djgpp_set_sigquit_key (0x082D); /* Bind ALT-X to SIGQUIT */
  signal (SIGQUIT, sig_handler);
#endif

  host = argv[0];

  if (argc >= 2)
       port = GetService (argv[1]);
  else port = IPPORT_TELNET;

  SCR_Init (1);

  if (!ConnectTN(host,port))
     TelnetExit();

  if (script)
     ChatMain (script);

  while (!quit && connected)
  {
    tcp_tick (&sock);
    (*recvState)();
    (*xmitState)();
    BackGroundTN();
  }
  CloseTN();
  TelnetExit();
  return (0);
}

/*--------------------------------------------------------------------*/

static void InitMain (char *argv0)
{
#ifdef __HIGHC__
  FARPTR iv, ex;
  int    _386debug = 0;  /* 386|Debug doesn't give us correct argv[] */

  _dx_pmiv_get (13, &iv);
  _dx_excep_get(13, &ex);
  if (FP_SEL(iv) != FP_SEL(ex)) /* separate exc-13 gates? */
     _386debug = 1;             /* yes, debugger active   */

  InstallExcHandler (NULL);
  if (!_386debug)
#endif
  {
    char *s, *p = strdup (argv0);

    if (p && (s = strrchr (p, SLASH)) != NULL)
    {
      *s = '\0';
      strcpy (progPath, p);
      strcpy (progName, s+1);
    }
    else
    {
      strcpy (progPath, ".");
      strcpy (progName, argv0);
    }
    strlwr (progPath);
    strlwr (progName);
  }
}

/*--------------------------------------------------------------------*/

#include "nochkstk.h"

#ifdef __HIGHC__          /* disable stack-checking and tracing here */
  #pragma Off(call_trace)
  #pragma Off(prolog_trace)
  #pragma Off(epilog_trace)
  extern void  _mwinit_stack_margin (void);
  extern DWORD _mwstack_limit[2];   /* stack-check limits */
#endif

static int sig_counter = 0;

static void sig_handler (int sig)
{
#if defined(DJGPP)
  if (sig == SIGQUIT)
  {
    quit = 1;
    return;
  }
#endif

  if (connected && sig_counter++ == 0)  /* got first ^C */
  {
    sig_counter = 0;
    KeyUngetKey (Key_CTRL_C);
    signal (SIGINT, sig_handler);
  }
  else
  {
    tel_log ("SIGINT caught. Shutting down");
    TelnetExit();
  }
}

void assert_fail (const char *what, const char *file, unsigned line)
{
  char buf[512];

  sprintf (buf, "%s (%u): Assertion `%s' failed!", file, line, what);
  tel_log (buf);
  fprintf (stderr, "Fatal: %s\n", buf);
  TelnetExit();
}

void fatal (const char *fmt, ...)
{
  va_list ap;
  char    buf[512];

  va_start (ap, fmt);
  vsprintf (buf, fmt, ap);
  va_end (ap);
  cprintf ("%s\r\n", buf);
  TelnetExit();
}

