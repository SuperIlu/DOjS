/*!\file sock_prn.c
 * sock_type printing to network
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "copyrigh.h"
#include "wattcp.h"
#include "strings.h"
#include "language.h"
#include "pcconfig.h"
#include "pctcp.h"

int MS_CDECL sock_printf (sock_type *s, const char *fmt, ...)
{
  char    buf [tcp_MaxBufSize];
  int     len;
  va_list args;

  va_start (args, fmt);

#if defined(VSNPRINTF)
  len = VSNPRINTF (buf, sizeof(buf)-1, fmt, args);

  /* Some versions of snprintf return -1 if they'd truncate
   * the output. Others return <buf_size> or greater.
   */
  if (len < 0 || len >= SIZEOF(buf)-1)
  {
    outsnl (_LANG("ERROR: sock_printf() overrun"));
    len = sizeof(buf)-1;
    buf [len] = '\0';
  }
#else
  len = vsprintf (buf, fmt, args);
  if (len > SIZEOF(buf))
  {
    outsnl (_LANG("ERROR: sock_printf() overrun"));
    return (0);
  }
#endif

  sock_puts (s, (const BYTE*)buf);

  va_end (args);
  return (len);
}

