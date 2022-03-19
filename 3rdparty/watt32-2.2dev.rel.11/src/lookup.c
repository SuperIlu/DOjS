/*!\file lookup.c
 *  Host lookup function.
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wattcp.h"
#include "strings.h"
#include "language.h"
#include "netaddr.h"
#include "misc.h"
#include "pcdns.h"
#include "ip4_in.h"
#include "pcdbug.h"
#include "pctcp.h"

DWORD W32_CALL lookup_host (const char *host, char *ip_str)
{
  DWORD ip;

  WATT_ASSERT (host);

  if (isaddr(host))
     ip = aton (host);
  else
  {
    outs (_LANG("Resolving "));
    outs (host);
    outs ("...");
    ip = resolve (host);
    if (ip)
    {
      if (dom_cname[0] && debug_on >= 1)
      {
        outs ("CNAME ");
        outs (dom_cname);
        outs (" ");
      }
      outsnl (_inet_ntoa(NULL,ip));
    }
  }

  if (!_ip4_is_loopback_addr(ip) && _ip4_is_multihome_addr(ip))
  {
#if defined(USE_DEBUG)
    (*_printf) ("Cannot connect to ourself\n");
#endif
    return (0);
  }

  if (ip_str)
     _inet_ntoa (ip_str, ip);
  return (ip);
}
