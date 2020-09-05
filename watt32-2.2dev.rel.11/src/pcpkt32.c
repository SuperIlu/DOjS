/*!\file pcpkt32.c
 *
 *  32-bit PKTDRVR interface.
 *
 *  Skeleton (not finished) for probing for the presence of network cards.
 *  All drivers should be put in a dynamically loaded module (DLL/DXE).
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

#include "wattcp.h"

#if defined(__MSDOS__) && DOSX

#include "pcconfig.h"
#include "wdpmi.h"
#include "x32vm.h"
#include "misc.h"
#include "pcsed.h"
#include "pcpkt.h"
#include "pcpkt32.h"

int (*_pkt32_drvr)(IREGS*) = NULL;

const struct PM_driver pm_driver_list[] = {
                 { PM_DRVR_3C501,   "3c501"   },
                 { PM_DRVR_3C503,   "3c503"   },
                 { PM_DRVR_3C505,   "3c505"   },
                 { PM_DRVR_3C507,   "3c507"   },
                 { PM_DRVR_3C5x9,   "3c5x9"   },
                 { PM_DRVR_3C59x,   "3c59x"   },
                 { PM_DRVR_NE2000,  "NE2000"  },
                 { PM_DRVR_EEXPR,   "EthExp"  },
                 { PM_DRVR_RTL8139, "RTL8139" },
                 { 0,               NULL      }
               };

int pkt32_drvr_probe (const PM_driver *drivers)
{
  ARGSUSED (drivers);
  return (-1);
}

int pkt32_drvr_init (int driver, mac_address *addr)
{
  ARGSUSED (driver);
  ARGSUSED (addr);
  return (-1);
}

const char *pkt32_drvr_name (int driver)
{
  int i;
  for (i = 0; pm_driver_list[i].type; i++)
      if (pm_driver_list[i].type == driver)
         return (pm_driver_list[i].name);

  return ("unknown");
}
#endif  /* __MSDOS__ && DOSX */
