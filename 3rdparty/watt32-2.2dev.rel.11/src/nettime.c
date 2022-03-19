/*!\file nettime.c
 * Counts user/system time.
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
 *
 *  14.may 1999 (GV)  - Created
 *
 *  This module implements time counters for "user" and "system"
 *  calls. NOTE: "system" time is counted only when signals are
 *  trapped. i.e. around select_s(), connect() loops.
 *
 *  Based on djgpp version of times().
 *
 */

#include "socket.h"

#if defined(USE_BSD_API)

static DWORD sock_sys_time;
static DWORD start;

void _sock_start_timer (void)
{
  start = set_timeout (0);
}

void _sock_stop_timer (void)
{
  DWORD now = set_timeout (0);

  if (now > start)
       sock_sys_time += (now - start);
  else sock_sys_time += (start - now); /* msec/tick counter wrapped */
}

static DWORD sys_time (void)
{
  return (sock_sys_time);
}

static DWORD usr_time (void)
{
  return (set_timeout(0) - start_time - sock_sys_time);
}

unsigned long W32_CALL net_times (struct tms *tms)
{
  if (!tms)
  {
    SOCK_ERRNO (EINVAL);
    return (unsigned long)(-1);
  }
  memset (tms, 0, sizeof(*tms));
  if (_watt_is_init)
  {
#ifdef WIN32
    DWORD divisor = 1000;
#else
    DWORD divisor = (has_8254 || has_rdtsc) ? 1000 : 18;
#endif

    tms->tms_utime = (CLOCKS_PER_SEC * usr_time()) / divisor;
    tms->tms_stime = (CLOCKS_PER_SEC * sys_time()) / divisor;
  }
  return (tms->tms_utime);
}
#endif  /* USE_BSD_API */

