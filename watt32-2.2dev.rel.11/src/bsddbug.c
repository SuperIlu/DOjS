/*!\file bsddbug.c
 * BSD socket debugging.
 */

/*  BSD sockets functionality for Watt-32 TCP/IP
 *
 *  Copyright (c) 1997-2002 Gisle Vanem <gvanem@yahoo.no>
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
 *        Debug functions moved from socket.c
 */

#include "socket.h"
#include "run.h"
#include "get_xby.h"

#if defined(USE_DEBUG) && defined(USE_BSD_API)  /* Whole file */

#define USE_PRINTK   0
#define SCOPE_INDENT 2    /* How much to indent printout at each new scope */

static int sk_scope = 0;  /*!< scope-level (indent printout) */

static void (W32_CALL *prev_hook) (const char*, const char*);

static BOOL   dbg_use_ods  = FALSE;
static FILE  *dbg_file     = NULL;
static char   dbg_omode[5] = "w+";           /* max "w+tc" */
static char   dbg_fname[MAX_PATHLEN+1] = "";

static void W32_CALL bsddbug_exit (void);
static void          bsddbug_close (void);

#if defined(WIN32)
static int ods_printf (const char *fmt, va_list arg);
#endif

int MS_CDECL _sock_debugf (const char *fmt, ...)
{
  int     len = 0, rc = 0;
  char    buf[256];
  va_list arg;
  int     err, errno_save = errno;

  if (!debug_xmit || !fmt)  /* dbug_init() not called */
     return (0);

#if defined(WIN32)
  if (dbg_use_ods)
  {
    va_start (arg, fmt);
    rc = ods_printf (fmt, arg);
    va_end (arg);
    return (rc);
  }
#endif

  if (!dbg_file)
     return (rc);

  va_start (arg, fmt);

  if (*fmt == '\n')
  {
#if (USE_PRINTK)
    len = _printk ("\n%9s: ", elapsed_str(set_timeout(0)));
    if (sk_scope > 0)
       len += _printk ("%*s", SCOPE_INDENT*sk_scope, "");
#else
    len = fprintf (dbg_file, "\n%9s: ", elapsed_str(set_timeout(0)));
    if (sk_scope > 0)
       len += fprintf (dbg_file, "%*s", SCOPE_INDENT*sk_scope, "");
#endif
    fmt++;
    if (*fmt == '\0')
       return (len);
  }

#if (USE_PRINTK)
  _vsnprintk (buf, sizeof(buf), fmt, arg);
  rc  = _fputsk (buf, dbg_file);
#else
  vsprintf (buf, fmt, arg);
  rc  = fputs (buf, dbg_file);
#endif

  err = errno;
  _sock_dbug_flush();

  if (rc == EOF)
  {
    TCP_CONSOLE_MSG (1, ("%s (%d): write failed; %s\n",
                     __FILE__, __LINE__, strerror(err)));
    bsddbug_close();
  }
  else
    len += rc;

  errno = errno_save;
  va_end (arg);
  return (len);
}

BOOL _sock_dbug_active (void)
{
  return (dbg_file != NULL || dbg_use_ods);
}

void _sock_dbug_flush (void)
{
  if (!dbg_file)
     return;

#if (USE_PRINTK)
  _printk_safe = TRUE;
  _printk_flush();
#else
  fflush (dbg_file);
#endif
}

/*
 * Called from sock_ini.c if 'debug_xmit' != NULL.
 */
void _sock_dbug_open (void)
{
  if (dbg_file == stdout || dbg_file == stderr || dbg_use_ods)
     return;

#if (USE_PRINTK)
  if (_printk_init(2000, dbg_fname))
  {
    dbg_file = _printk_file;
    RUNDOWN_ADD (bsddbug_exit, 150);
  }
#else
  if (!dbg_fname[0])
     return;

  errno = 0;
  dbg_file = fopen_excl (dbg_fname, dbg_omode);
  if (dbg_file)
  {
#if !defined(__BORLANDC__)  /* buggy output with this */
    static char buf [1024];
    setvbuf (dbg_file, buf, _IOLBF, sizeof(buf));
#endif
    RUNDOWN_ADD (bsddbug_exit, 150);
  }
#endif  /* USE_PRINTK */

  TCP_CONSOLE_MSG (2, ("%s (%d): dbg_file `%s', handle %d, errno %d\n",
                   __FILE__, __LINE__, dbg_fname,
                   dbg_file ? fileno(dbg_file) : -1, errno));
}

/*
 * Callbacks for config-table parser.
 */
static void set_dbg_fname (const char *value)
{
  _strlcpy (dbg_fname, value, sizeof(dbg_fname));

  if (!stricmp(dbg_fname,"stderr"))
     dbg_file = stderr;
  if (!stricmp(dbg_fname,"stdout") || !stricmp(dbg_fname,"con"))
     dbg_file = stdout;
#if defined(WIN32)
  if (!stricmp(dbg_fname,"$ods"))
     dbg_use_ods = TRUE;
#endif
}

static void set_dbg_openmode (const char *value)
{
  _strlcpy (dbg_omode, value, sizeof(dbg_omode));
}

/*
 * Config-file hook routine called from parser.
 * (set_values() in pcconfig.c and via ourinit() in pcdbug.c)
 *
 * Parses keywords:
 *   SK_DEBUG.DEVICE   = <file>
 *   SK_DEBUG.OPENMODE = <mode>
 */
static void W32_CALL dbug_parse (const char *name, const char *value)
{
  static const struct config_table cfg[] = {
            { "DEVICE",   ARG_FUNC, (void*)set_dbg_fname    },
            { "OPENMODE", ARG_FUNC, (void*)set_dbg_openmode },
            { NULL,       0,        NULL                    }
          };

  if (!parse_config_table(&cfg[0], "SK_DEBUG.", name, value) && prev_hook)
     (*prev_hook) (name, value);
}

/*
 * Normally called from dbug_init() (in pcdbug.c) if USE_DEBUG is defined.
 * Must be called before watt_sock_init() and any other socket functions.
 */
void _sock_dbug_init (void)
{
  prev_hook = usr_init;
  usr_init  = dbug_parse;
}

static void show_local_ports_inuse (void)
{
  WORD port, num = 0;

  if (!dbg_file)
     return;

#if (USE_PRINTK)
  _printk ("\nLocal ports still in use:\n");
#else
  fprintf (dbg_file, "\nLocal ports still in use:\n");
#endif

  for (port = 1025; port < USHRT_MAX; port++)
  {
    int rc = reuse_localport (port);

    if (rc < 0)
       break;
    if (rc > 0)
    {
      num++;
#if (USE_PRINTK)
      _printk ("%5u%c", port, num % 12 ? ',' : '\n');
#else
      fprintf (dbg_file, "%5u%c", port, (num % 12) ? ',' : '\n');
#endif
    }
  }
}

void bsd_fortify_print (const char *buf)
{
  if (dbg_file)
  {
#if (USE_PRINTK)
    _fputsk (buf, dbg_file);
#else
    fputs (buf, dbg_file);
#endif
  }
}

#if defined(WIN32)
/**
 * \todo: Needs to be rewritten to handle line-buffering. !!
 */
static int ods_printf (const char *fmt, va_list arg)
{
  char   buf1[300];
  char   buf2[400];
  char  *p1   = buf1;
  char  *p2   = buf2;
  size_t left = sizeof(buf1);

  if (*fmt == '\n')
  {
    p1   += SNPRINTF (p1, left, "\n%9s: ", elapsed_str(set_timeout(0)));
    left -= p1 - buf1;
    fmt++;
    if (sk_scope > 0)
    {
      p1   += SNPRINTF (p1, left, "%*s", SCOPE_INDENT*sk_scope, "");
      left -= p1 - buf1;
    }
  }

  VSNPRINTF (p1, left, fmt, arg);

  for (p1 = buf1, p2 = buf2; *p1 && p2 < buf2 + sizeof(buf2); p1++, p2++)
  {
    if (p1[0] == '\n' && p1[-1] != '\r')
        *p2++ = '\r';
    *p2 = *p1;
  }

#if 0
  if (p2[-2] != '\r' || p2[-1] != '\n')
     strcpy (p2, "\r\n"), p2 += 2;
#endif

  *p2++ = '\0';
  OutputDebugStringA (buf2);

  /* Because of clang:
   *   warning: Value stored to 'p2' during its initialization is never read
   */
  ARGSUSED (p2);
  return (p2 - buf2);
}
#endif  /* WIN32 */


#include "nochkstk.h"

static void bsddbug_close (void)
{
#if (USE_PRINTK == 0)
  if (dbg_file && dbg_file != stdout && dbg_file != stderr)
     FCLOSE (dbg_file);
#endif
  dbg_file = NULL;
}

static void W32_CALL bsddbug_exit (void)
{
  if (!_watt_fatal_error && dbg_file)
  {
    DumpEthersCache();
    DumpHostsCache();
#if defined(USE_IPV6)
    DumpHosts6Cache();
#endif
    show_local_ports_inuse();
    _sock_dbug_flush();
  }
  bsddbug_close();
  set_dbg_openmode ("at");  /* append next time we open it */
}

/*
 * These two functions are meant to increase/decrease the suspension
 * depth of a "task". I.e. _sock_enter_scope() should awake other
 * sleeping "tasks" waiting to be signalled. Ref. sk_block in socket.c.
 * Now they're only used to make the sock-trace prettier.
 */
void _sock_enter_scope (void)
{
  if (sk_scope < INT_MAX)
     sk_scope++;
}

void _sock_leave_scope (void)
{
  if (sk_scope > 0)
      sk_scope--;
}
#endif  /* USE_DEBUG && USE_BSD_API */

