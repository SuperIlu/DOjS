/*!\file highc.c
 *  Functions for Metaware's High-C.
 *
 *  This file is for Metaware HighC/C++ with Pharlap only.
 *  Handles tracing of entry/exit functions and replaces the buggy
 *  system() routine.
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>

#include "wattcp.h"
#include "strings.h"
#include "pcconfig.h"

#if defined(__HIGHC__) && (DOSX & PHARLAP)

/*
 * HighC's putenv() doesn't modify the inherited environment.
 * We must do it ourself.
 */
static char *setup_environment (void)
{
  char  *copy;
  char  *start;
  char **env  = _environ; /* list of env.var + putenv() vars */
  UINT   size = 0;

  if (!env || !*env)
     return (NULL);

  while (env && *env)  /* find how many bytes we need to malloc */
       size += strlen (*env++) + 1;

  size++;
  copy = malloc (size);
  if (!copy)
     return (NULL);

  start = copy;
  env   = _environ;
  while (env && *env)  /* make a new continuous environment block */
  {
    int len = strlen (*env) + 1;

    memcpy (copy, *env++, len);
    copy += len;
  }
  *copy = '\0';        /* final termination */
  return (start);
}

static int execute (const char *path, const char *args, const char *env)
{
  union  REGS reg;
  EXEC_BLK    blk;
  char        buf[256];
  int         len;
  ULONG       minPages, maxPages;

  len  = strlen (args);
  if (len > sizeof(buf)-2)
     return (0);

  _dx_cmem_usage (0, FALSE, &minPages, &maxPages);

  strcpy (buf+1, args);
  buf [0]   = len++;
  buf [len] = '\r';

  /* if `env' is NULL we'll inherit the environment without any
   * variables added by `putenv()'
   */
  blk.ex_eoff = (UCHAR*)env;    /* pass new environment */
  blk.ex_eseg = SS_DATA;
  blk.ex_coff = (UCHAR*) buf;   /* command line address */
  blk.ex_cseg = SS_DATA;
  reg.x.ax    = 0x4B00;
  reg.x.bx    = (UINT) &blk;
  reg.x.dx    = (UINT) path;
  fflush (stdout);              /* flush buffers now */
  fflush (stderr);

  intdos (&reg, &reg);
  if (reg.x.cflag & 1)          /* carry set, return -1 */
     return (-1);

  reg.x.ax = 0x4D00;
  intdos (&reg, &reg);
  return (reg.h.al);            /* return child exit code */
}

/*
 * This replaces system() in hc386.lib
 * Note: this is aliased in <tcp.h>
 */
int _mw_watt_system (const char *cmd)
{
  char  buf[150];
  const char *env;
  const char *comspec = getenv ("COMSPEC");
  int   rc;

  if (!comspec || access(comspec,0))
  {
    (*_printf) ("Bad COMSPEC variable\n");
    return (0);
  }

  if (cmd && *cmd)
       _bprintf (buf, sizeof(buf), " /c %s", cmd);
  else _bprintf (buf, sizeof(buf), "%s", cmd);

  env = setup_environment();
  rc  = execute (comspec, buf, env);

  if (env)
     free (env);
  return (rc);
}

/*
 * The following functions are called when Call_trace, Prolog_trace
 * and Epilog_trace pragmas are in effect.
 * Library must be compiled with:  -Hon=Call_trace
 *                                 -Hon=Prolog_trace
 *                             and -Hon=Epilog_trace
 */

#pragma Off(Call_trace)      /* no trace of trace itself */
#pragma Off(Prolog_trace)
#pragma Off(Epilog_trace)
#pragma Global_aliasing_convention("%r")

#if defined(USE_DEBUG)

#define MAX_LEVEL 1000


static int   level = 0;
static const char *caller [MAX_LEVEL];

static BOOL is_watt32_func (const char *func) /* incomplete */
{
  if (!strcmp(func,"watt_sock_init"))
     return (TRUE);
  if (!strcmp(func,"sock_init"))
     return (TRUE);
  if (!strcmp(func,"_w32_init_misc"))
     return (TRUE);
  if (!strcmp(func,"setup_dos_xfer_buf"))
     return (TRUE);
  if (!strcmp(func,"RDTSC_enabled"))
     return (TRUE);
  if (!strcmp(func,"_w32_init_timers"))
     return (TRUE);
  if (!strcmp(func,"set_utc_offset"))
     return (TRUE);
  if (!strcmp(func,"get_zone"))
     return (TRUE);
  if (!strcmp(func,"tcp_init"))
     return (TRUE);
  if (!strcmp(func,"_eth_init"))
     return (TRUE);
  return (FALSE);   /**< \todo Fill in the rest (use a generated table?) */
}

void _call_trace (const char *from, const char *module, unsigned line,
                  const char *to)
{
  static const char *last_module = NULL;
  char   where[30];

  if (!ctrace_on || level >= DIM(caller)-1)
     return;

  _bprintf (where, sizeof(where), "%s(%u)", module, line);

  (*_printf) ("%-16s:%*s %s() -> %s()\n", where, level, "", from, to);

  if (caller[level] == NULL ||
      (strcmp(from,caller[level])) && is_watt32_func(to))
     caller[level++] = from;

  else if (last_module && !strcmp(module,last_module) && level > 0)
     level--;

  last_module = module;
}

void _prolog_trace (const char *func)
{
#if 0
  if (ctrace_on)
    (*_printf) ("%s()\n", func);
#endif
  ARGSUSED (func);
}

void _epilog_trace (const char *func)
{
  if (level > 0 && ctrace_on)
    --level;
}
#else     /* !USE_DEBUG */

void _call_trace (const char *from, const char *module, unsigned line, const char *to)
{
  ARGSUSED (from);
  ARGSUSED (module);
  ARGSUSED (line);
  ARGSUSED (to);
}

void _prolog_trace (const char *func)
{
  ARGSUSED (func);
}

void _epilog_trace (const char *func)
{
  ARGSUSED (func);
}
#endif   /* USE_DEBUG */
#endif   /* __HIGHC__ && (DOSX & PHARLAP) */
