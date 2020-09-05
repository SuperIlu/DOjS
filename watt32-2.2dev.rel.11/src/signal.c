/*!\file signal.c
 * Signal handlers for BSD socket API.
 */

/*  BSD sockets functionality for Watt-32 TCP/IP
 *
 *  Copyright (c) 1997-2007 Gisle Vanem <gvanem@yahoo.no>
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. All advertising materials mentioning features or use of this software
 *     must display the following acknowledgement:
 *       This product includes software developed by Gisle Vanem
 *       Bergen, Norway.
 *
 *  THIS SOFTWARE IS PROVIDED BY ME (Gisle Vanem) AND CONTRIBUTORS ``AS IS''
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL I OR CONTRIBUTORS BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  Version
 *
 *  0.5 : Dec 18, 1997 : G. Vanem - created
 */

#include "socket.h"

#if defined(USE_BSD_API)

#include "nochkstk.h"

#if defined(__HIGHC__)  /* Metaware's HighC have SIGBREAK == SIGQUIT */
  #undef SIGBREAK
#elif defined(__DMC__)  /* Digital Mars have SIGBREAK == SIGINT */
  #undef SIGBREAK
#endif

/*
 * I'm not sure what the benefit of using sigprocmask() on djgpp is.
 * So leave it alone for the moment.
 */
#if defined(__DJGPP__)
  #define USE_SIGMASK  0
#else
  #define USE_SIGMASK  0
#endif

struct signal_table {
       void (MS_CDECL *old)(int);
       int             sig_num;
       const char     *sig_name;
       BOOL            caught;  /* we caught this signal */
       BOOL            exit;    /* exit if app doesn't have a signal-handler */
     };

static struct signal_table sig_tab[] = {
    { NULL, SIGINT,   "SIGINT",  FALSE, TRUE },
#if defined(SIGBREAK)
    { NULL, SIGBREAK, "SIGBREAK",FALSE, TRUE }, /* Watcom */
#endif
#if defined(SIGQUIT)
    { NULL, SIGQUIT,  "SIGQUIT", FALSE, TRUE },  /* should we trap this? */
#endif
#if defined(SIGPIPE)
    { NULL, SIGPIPE,  "SIGPIPE", FALSE, FALSE }, /* djgpp */
#endif
#if defined(SIGALRM) && 0  /* Should we hook this? */
    { NULL, SIGALRM,  "SIGALRM", FALSE, FALSE }, /* djgpp */
#endif
  };

static int signal_depth = 0; /* Our running signal "depth" */

/*
 * We block all signals from being raised while running our
 * loops (connect etc.). Postpone all original signal-handlers until
 * we're done.
 */
#if (USE_SIGMASK == 0)
/*
 * Our signal catcher. Set 'caught' flag.
 */
static void MS_CDECL sig_catch (int sig)
{
  int i;

  for (i = 0; i < DIM(sig_tab); i++)
      if (sig == sig_tab[i].sig_num)
      {
        sig_tab[i].caught = TRUE;
        break;
      }
  signal (sig, SIG_IGN);    /* ignore until we restore it */
}
#endif

/*
 * Signal handling for most loops where we can be interrupted
 * by user pressing ^C/^Break (generating SIGINT/SIGBREAK).
 */
int _sock_sig_setup (void)
{
#if (USE_SIGMASK)
  static sigset_t old_mask, new_mask;
#endif
  int i;

  if (signal_depth > 0 ||
      ++signal_depth > 1)
     return (0);

  _sock_start_timer();

#if (USE_SIGMASK)
  sigemptyset (&new_mask);
#endif

  for (i = 0; i < DIM(sig_tab); i++)
  {
    struct signal_table *sig = sig_tab + i;

    sig->caught = FALSE;

#if (USE_SIGMASK)
    sigaddset (&new_mask, sig->sig_num);
    sigprocmask (SIG_SETMASK, &new_mask, &old_mask);
#else
    sig->old = signal (sig->sig_num, sig_catch);
#endif
  }
  return (0);
}

/*
 * Return number of blocked signal that are pending/caught.
 */
int _sock_sig_pending (void)
{
  int i, num = 0;

#if (USE_SIGMASK)
  sigset_t pending;

  sigpending (&pending);
  for (i = 0; i < DIM(sig_tab); i++)
      if (sigismember(&pending, sig_tab[i].sig_num))
         num++;
#else
  for (i = 0; i < DIM(sig_tab); i++)
      if (sig_tab[i].caught)
         num++;
#endif
  return (num);
}

/*
 * Unhook signal-handlers and raise() signals we caught.
 */
int _sock_sig_restore (void)
{
  int i;

  if (signal_depth == 0 ||
      --signal_depth > 0)
     return (0);

  _sock_stop_timer();

  for (i = 0; i < DIM(sig_tab); i++)
  {
    struct signal_table *sig = sig_tab + i;

#if (USE_SIGMASK)
    sigset_t pending, new_set;

    if (sigpending(&pending) == 0 &&
        sigismember(&pending,sig->sig_num))
    {
      /* The signal became pending while we blocked it.
       * I.e. ^C was pressed, so act on it.
       */
      sigemptyset (&new_set);
      sigaddset (&new_set, sig->sig_num);

      SOCK_DEBUGF ((", raising %s", sig->sig_name));
      SOCK_ENTER_SCOPE();
      sigprocmask (SIG_UNBLOCK, &new_set, NULL);
      SOCK_LEAVE_SCOPE();
    }
#else
    signal (sig->sig_num, sig->old);
    if (!sig->caught)
       continue;

    sig->caught = FALSE;
    if (sig->exit &&
        (sig->old == SIG_DFL ||
         sig->old == SIG_IGN ||
         sig->old == sig_handler_watt))
    {
      char buf[30];

      strcpy (buf, "\nTerminating on ");
      strcat (buf, sig->sig_name);
      sock_sig_exit (buf, sig->sig_num);
      /* no return */
    }
    SOCK_DEBUGF ((", raising %s", sig->sig_name));
    SOCK_ENTER_SCOPE();
    raise (sig->sig_num);
    SOCK_LEAVE_SCOPE();
    return (-1);
#endif
  }
  return (0);
}

/*
 * Raise SIGPIPE/SIGURG if signal defined (not needed yet).
 * Return -1 with errno = EPIPE.
 */
int _sock_sig_epipe (const Socket *sock)
{
#if defined(SIGPIPE)
  if (!sock->msg_nosig)
     raise (SIGPIPE);
#endif
  SOCK_ERRNO (EPIPE);
  ARGSUSED (sock);
  return (-1);
}
#endif  /* USE_BSD_API */
