/*!\file syslog2.c
 * Config-parser for syslog().
 *
 *  Simple syslog handler for Watt-32 & DOS.
 *  This module contain data and config-parser only.
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
#include <stdarg.h>
#include <sys/syslog.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#include "wattcp.h"
#include "printk.h"
#include "pcconfig.h"
#include "strings.h"
#include "syslog2.h"

#if defined(USE_BSD_API)

static void (W32_CALL *prev_hook) (const char*, const char*) = NULL;

char syslog_file_name [MAX_NAMELEN] = "";  /* name of logfile */
char syslog_host_name [MAX_HOSTLEN] = "";  /* name of loghost */
WORD syslog_port = 514;                    /* udp port to use */
int  syslog_mask = LOG_UPTO(-1);           /* log everything */

static int get_log_mask (const char *value)
{
  int mask = 0;

  if (!strcmp(value, "all"))
     return (LOG_PRIMASK); /* LOG_UPTO (LOG_EMERG); */

  if (strstr(value, "emerg"))  mask |= LOG_EMERG;
  if (strstr(value, "alert"))  mask |= LOG_ALERT;
  if (strstr(value, "crit"))   mask |= LOG_CRIT;
  if (strstr(value, "error"))  mask |= LOG_ERR;
  if (strstr(value, "warn"))   mask |= LOG_WARNING;
  if (strstr(value, "notice")) mask |= LOG_NOTICE;
  if (strstr(value, "info"))   mask |= LOG_INFO;
  if (strstr(value, "debug"))  mask |= LOG_DEBUG;
  return (mask);
}

static void set_syslog_mask (const char *value)
{
  char val[100];

  _strlcpy (val, value, sizeof(val));
  strlwr (val);
  syslog_mask = get_log_mask (val);
}

static void W32_CALL syslog2_init (const char *name, const char *value)
{
  static const struct config_table syslog_cfg[] = {
            { "FILE",  ARG_STRCPY, (void*)&syslog_file_name },
            { "HOST",  ARG_STRCPY, (void*)&syslog_host_name },
            { "PORT",  ARG_ATOI,   (void*)&syslog_port      },
            { "LEVEL", ARG_FUNC,   (void*)set_syslog_mask   },
            { NULL,    0,          NULL                     }
          };
  if (!parse_config_table(&syslog_cfg[0], "SYSLOG.", name, value) && prev_hook)
     (*prev_hook) (name, value);
}

void syslog_init (void)
{
  prev_hook = usr_init;
  usr_init  = syslog2_init;
}
#endif /* USE_BSD_API */
