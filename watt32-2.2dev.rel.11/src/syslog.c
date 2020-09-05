/*!\file syslog.c
 *
 *  Simple syslog handler for Watt-32.
 *
 *  Loosely based on BSD-version.
 *    by Gisle Vanem <gvanem@yahoo.no>  Jun-99
 *
 *  This module really belongs to the application layer,
 *  but is included in Watt-32 for convenience.
 *
 *  \note syslog_init() must be called in watt_sock_init() (or prior to
 *        that). openlog() and syslog() cannot be used before watt_sock_init()
 *        is called.
 *
 *  \todo configure variables in syslog2.c when calling openlog() thus
 *        making syslog() independent of watt_sock_init().
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/syslog.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <time.h>
#include <netdb.h>

#include "wattcp.h"
#include "misc.h"
#include "run.h"
#include "printk.h"
#include "pctcp.h"
#include "pcsed.h"
#include "pcstat.h"
#include "pcbuf.h"
#include "pcdbug.h"
#include "pcconfig.h"
#include "netaddr.h"
#include "sock_ini.h"
#include "strings.h"
#include "syslog2.h"

#if defined(USE_BSD_API)

#if defined(__CCDL__) && !defined(getpid)
#define getpid()  0
#endif

#if defined(__LCCDL__)
#define getpid()  _getpid()
#endif

#define INTERNALLOG  (LOG_ERR|LOG_CONS|LOG_PERROR|LOG_PID)

static sock_type *log_sock   = NULL;     /* UDP socket for log */
static FILE      *log_file   = NULL;     /* FILE* for log */
static DWORD      log_host   = 0UL;      /* IP-address of host (host order) */
static BOOL       log_opened = FALSE;    /* have called openlog() (error or not) */
static int        log_stat   = 0;        /* status bits, set by openlog() */
static char      *log_tag    = NULL;     /* string to tag the entry with */
static int        log_mask   = LOG_INFO; /* default is LOG_EMERG - LOGINFO */
static DWORD      log_facil  = LOG_USER; /* default facility */

/* These arrays used to be pulled in from <sys/syslog.h> when SYSLOG_NAMES
 * was defined, but caused "multiple defined" problems.
 */
CODE prioritynames[] = {
        { LOG_ALERT,      "alert"   },
        { LOG_CRIT,       "crit"    },
        { LOG_DEBUG,      "debug"   },
        { LOG_EMERG,      "emerg"   },
        { LOG_ERR,        "err"     },
        { LOG_ERR,        "error"   },      /* DEPRECATED */
        { LOG_INFO,       "info"    },
        { INTERNAL_NOPRI, "none"    },      /* INTERNAL */
        { LOG_NOTICE,     "notice"  },
        { LOG_EMERG,      "panic"   },      /* DEPRECATED */
        { LOG_WARNING,    "warn"    },      /* DEPRECATED */
        { LOG_WARNING,    "warning" },
        { -1,             NULL      }
      };

CODE facilitynames[] = {
        { LOG_AUTH,      "auth"     },
        { LOG_AUTHPRIV,  "authpriv" },
        { LOG_CRON,      "cron"     },
        { LOG_DAEMON,    "daemon"   },
        { LOG_FTP,       "ftp"      },
        { LOG_KERN,      "kern"     },
        { LOG_LPR,       "lpr"      },
        { LOG_MAIL,      "mail"     },
        { INTERNAL_MARK, "mark"     },      /* INTERNAL */
        { LOG_NEWS,      "news"     },
        { LOG_NTP,       "ntp"      },
        { LOG_AUTH,      "security" },      /* DEPRECATED */
        { LOG_SYSLOG,    "syslog"   },
        { LOG_USER,      "user"     },
        { LOG_UUCP,      "uucp"     },
        { LOG_LOCAL0,    "local0"   },
        { LOG_LOCAL1,    "local1"   },
        { LOG_LOCAL2,    "local2"   },
        { LOG_LOCAL3,    "local3"   },
        { LOG_LOCAL4,    "local4"   },
        { LOG_LOCAL5,    "local5"   },
        { LOG_LOCAL6,    "local6"   },
        { LOG_LOCAL7,    "local7"   },
        { -1,            NULL       }
      };

/*
 * syslog, vsyslog --
 *   print message on log file; output is intended for syslogd(8).
 */
void W32_CDECL syslog (int pri, const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  vsyslog (pri, fmt, args);
  va_end (args);
}

void W32_CALL vsyslog (int pri, const char *fmt, va_list ap)
{
#if defined(USE_DEBUG)
  const char *pri_name = "?";
  const char *fac_name = "?";
#endif
  const char *pri_end;
  char  *p, ct_buf[30];
  char   tbuffer [2048];
  int    left = sizeof(tbuffer);
  int    saved_errno = errno;
  time_t t;

#if 0
  /* Check for invalid bits
   */
  if (pri & ~(LOG_PRIMASK|LOG_FACMASK))
  {
    syslog (INTERNALLOG, "syslog: unknown priority: %d", pri);
    pri &= (LOG_PRIMASK | LOG_FACMASK);
  }
#endif

  /* Set default facility if none specified
   */
  if (!(pri & LOG_FACMASK))
     pri |= log_facil;

#if defined(USE_DEBUG)
  pri_name = list_lookup (LOG_PRI(pri),
                          (const struct search_list*)prioritynames,
                          DIM(prioritynames)-1);

  fac_name = list_lookup (LOG_FACMASK & pri,
                          (const struct search_list*)facilitynames,
                          DIM(facilitynames)-1);
#endif

  /* Check priority against setlogmask() values.
   * Note: higher priorities are lower values !!
   */
  if (LOG_PRI(pri) > LOG_PRI(log_mask))
  {
#if defined(USE_DEBUG)
    if (debug_on >= 2)
       (*_printf) ("syslog: Dropping %s (%d) priority message, "
                   "facility %s (%d), pri 0x%04X\n",
                   pri_name, LOG_PRI(pri),
                   fac_name, LOG_FAC(pri), pri);
#endif
    return;
  }

#if defined(USE_DEBUG)
  if (debug_on >= 2)
     (*_printf) ("syslog: Accepting %s (%d) priority message, "
                 "facility %s (%d), pri 0x%04X\n",
                 pri_name, LOG_PRI(pri),
                 fac_name, LOG_FAC(pri), pri);
#endif

  /* Build the message
   */
  time (&t);
  p  = tbuffer;
  p += _snprintk (p, left, "<%3d>%.15s ", pri, ctime_r(&t,ct_buf)+4);
  left -= p - tbuffer;
  pri_end = 1 + strchr (tbuffer, '>');

  if (log_tag)
  {
    p += _snprintk (p, left, "%s", log_tag);
    left -= p - tbuffer;
  }
  if (log_stat & LOG_PID)
  {
    p += _snprintk (p, left, "[%d]", getpid());
    left -= p - tbuffer;
  }
  if (log_tag)
  {
    p += _snprintk (p, left, ": ");
    left -= p - tbuffer;
  }

  SOCK_ERRNO (saved_errno);
  p += _vsnprintk (p, left, fmt, ap);
  if (*(p-1) != '\n')
  {
    *p++ = '\n';
    *p = '\0';
  }

  if (!log_opened)
     openlog (log_tag, log_stat | LOG_NDELAY, log_facil);

  if (log_file)
     _fputsk (pri_end, log_file);

  if ((log_stat & LOG_PERROR) ||
      ((pri & LOG_PRIMASK) == LOG_ERR && (log_stat & LOG_CONS)))
     _fputsk (pri_end, stdout);

  if (log_sock)
  {
    int   len = strlen (tbuffer);
    char *err;

    len = sock_write (log_sock, (const BYTE*)tbuffer, len);
    err = (char*) sockerr (log_sock);

    if (len <= 0 || err)
    {
      err = strdup (err);
      sock_close (log_sock);
      free (log_sock);
      log_sock = NULL;
      syslog (log_stat | LOG_ERR, "UDP write failed: %s\n", err);
      if (err)
         free (err);
    }
  }
}


static const char *getlogname (void)
{
  const char *name = get_argv0();
  char       *dot;
  static char buf [MAX_PATHLEN];

  if (name && name[0])
  {
    name = _strlcpy (buf, name, sizeof(buf));
    dot  = strrchr (name, '.');
    if (dot && (!strnicmp(dot,".exe",4) || !strnicmp(dot,".exp",4)))
    {
      strcpy (dot, ".log");
      return (name);
    }
  }
  return ("$unknown.log");
}

static int openloghost (char **errbuf)
{
  struct servent *sp;
  struct hostent *hp;
  static char buf [80];

  *errbuf = NULL;

  if (log_sock)    /* reopen UDP connection */
  {
    sock_close (log_sock);
    log_sock = NULL;
  }

  sp = getservbyname ("syslog", "udp");
  if (sp)
     syslog_port = htons (sp->s_port);

  log_host = _inet_addr (syslog_host_name);
  if (!log_host)
  {
    hp = gethostbyname (syslog_host_name);
    if (!hp)
    {
      sprintf (buf, "unknown host `%s'", syslog_host_name);
      *errbuf = buf;
      return (0);
    }
    log_host = ntohl (*(DWORD*)hp->h_addr);
  }

  if (!log_sock)
  {
    log_sock = (sock_type*) malloc (sizeof(_udp_Socket));
    if (!log_sock)
    {
      _strlcpy (buf, strerror(errno), sizeof(buf));
      *errbuf = buf;
      return (0);
    }
  }

  if (!udp_open(&log_sock->udp, 0, log_host, syslog_port, NULL))
  {
    _strlcpy (buf, sockerr(log_sock), sizeof(buf));
    *errbuf = buf;
    sock_close (log_sock);
    free (log_sock);
    log_sock = NULL;
    return (0);
  }
  return (1);
}

static void W32_CALL _closelog (void)
{
  closelog();
}

void W32_CALL openlog (const char *ident, int logstat, int logfac)
{
  static BOOL done = FALSE;
  BOOL   fail  = FALSE;
  char  *error = NULL;

  if (ident)
     log_tag = (char*)ident;
  log_stat = logstat;

  if (logfac && !(logfac & ~LOG_FACMASK))  /* legal facility */
     log_facil = logfac;

  if (!syslog_file_name[0])          /* not set in wattcp.cfg */
     _strlcpy (syslog_file_name, getlogname(), sizeof(syslog_file_name));

  if (syslog_mask)
  {
    TCP_CONSOLE_MSG (2, ("setlogmask(0x%04X)\n", syslog_mask));
    setlogmask (syslog_mask);
  }

  /* Open immediately
   * !! fix-me: the LOG_NDELAY flag is really for the network connection
   */
  if (syslog_file_name[0] && (log_stat & LOG_NDELAY))
  {
    if (log_file)                  /* reopen */
       fclose (log_file);

    log_opened = TRUE;

    log_file = fopen (syslog_file_name, "at");
    if (!log_file || fputs("\n",log_file) == EOF)
    {
      log_file = NULL;
      error = strerror (errno);
      fail  = TRUE;
    }
#if !defined(__BORLANDC__)  /* buggy output with this */
    else
    {
      static char fbuf [256];
      setvbuf (log_file, fbuf, _IOLBF, sizeof(fbuf));  /* line-buffered */
    }
#endif
  }

  if (syslog_host_name[0] && !openloghost(&error))
     fail = TRUE;

  _printk_file = log_file;

  if (!done)
  {
    RUNDOWN_ADD (_closelog, 110);
    _printk_init (2000, NULL);
    done = TRUE;
    if (log_sock)
    {
      syslog (LOG_INFO, "syslog client at %I started", htonl(my_ip_addr));
      if (_inet_addr(syslog_host_name))
           syslog (LOG_INFO, "Logging to host %s", syslog_host_name);
      else syslog (LOG_INFO, "Logging to host %s (%I)",
                   syslog_host_name, htonl(log_host));
    }
  }

  if (fail && (log_stat & LOG_CONS))
     fprintf (stdout, "syslog: %s\n", error);
}


int W32_CALL setlogmask (int new_mask)
{
  int old = log_mask;
  if (new_mask)
     log_mask = new_mask;
  return (old);
}


char * W32_CALL setlogtag (char *new_tag)
{
  char *old = log_tag;
  log_tag = new_tag;
  return (old);
}

#include "nochkstk.h"

void W32_CALL closelog (void)
{
  if (!_watt_fatal_error)
  {
    if (log_file)
       fclose (log_file);

    if (log_sock)
    {
      sock_close (log_sock);
      free (log_sock);
    }
  }
  log_sock   = NULL;
  log_file   = NULL;
  log_opened = FALSE;
}
#endif /* USE_BSD_API */


#if defined(TEST_PROG)

#include "sock_ini.h"
#include "pcdbug.h"

int main (void)
{
  dbug_init();
  sock_init();
  debug_on = 2;

  openlog ("test", LOG_PID, LOG_LOCAL2);

  syslog (LOG_NOTICE, "NOTICE:  time now %t. file %s at line %d\n",
          __FILE__, __LINE__);

  SOCK_ERRNO (ENOMEM);
  syslog (LOG_ERR | LOG_LOCAL0, "ERR:     Allocation failed; %m");

  syslog (LOG_DEBUG, "DEBUG:   my_ip_addr = %I", htonl(my_ip_addr));

  SOCK_ERRNO (ENETDOWN);
  syslog (LOG_ALERT, "ALERT:   Connection failed; %m");

  syslog (LOG_EMERG, "EMERG:   Coffee mug empty");

  syslog (LOG_CRIT|LOG_KERN, "CRIT:    Kernel message: General error reading disk.");

  SOCK_ERRNO (EILSEQ);
  syslog (LOG_WARNING, "WARNING: Multibyte error; %m");

  syslog (LOG_INFO, "INFO:    Leaving main()");

  printf ("Done. ");
  if (log_file)
     printf ("Look at `%s'", syslog_file_name);

  closelog();
  return (0);
}
#endif /* TEST_PROG */
