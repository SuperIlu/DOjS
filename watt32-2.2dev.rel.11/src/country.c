/*!\file country.c
 *
 *  Function for fetching DOS's country info.
 */

/*
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
 *  These funtions are meant to:
 *   1) automatic detection and selection of language for the _LANG()
 *      function.  But not use yet.
 *   2) obtaining the active codepage for IDNA/ACE conversion stuff.
 */

#include <stdio.h>
#include <stdlib.h>

#include "wattcp.h"
#include "wdpmi.h"
#include "x32vm.h"
#include "powerpak.h"
#include "misc.h"

#if defined(__MSDOS__)

static char country_info[35];  /* not used yet */

int GetCountryCode (void)
{
  IREGS reg;

  if (_watt_os_ver < 0x300)
     return (0);

  memset (&reg, 0, sizeof(reg));
  reg.r_ax = 0x3800;

#if (DOSX & DJGPP)
  if (_go32_info_block.size_of_transfer_buffer < sizeof(country_info))
     return (0);
  reg.r_ds = __tb / 16;
  reg.r_dx = __tb & 15;

#elif (DOSX & (PHARLAP|X32VM|POWERPAK))
  if (_watt_dosTbSize < sizeof(country_info))
     return (0);
  reg.r_ds = RP_SEG (_watt_dosTbr);
  reg.r_dx = RP_OFF (_watt_dosTbr);

#elif (DOSX & DOS4GW)
  if (_watt_dosTbSize < sizeof(country_info))
     return (0);
  reg.r_ds = _watt_dosTbSeg;
  reg.r_dx = 0;

#elif (DOSX == 0)
  reg.r_ds = FP_SEG (country_info);
  reg.r_dx = FP_OFF (country_info);

#else
  #error Unexpected target!
#endif

  GEN_INTERRUPT (0x21, &reg);
  if (reg.r_flags & 1)
     return (0);

  memset (&country_info, 0, sizeof(country_info));

#if (DOSX & DJGPP)
  dosmemget (__tb, sizeof(country_info), &country_info);

#elif (DOSX & (PHARLAP|X32VM|POWERPAK))
  ReadRealMem (&country_info, _watt_dosTbr, sizeof(country_info));

#elif (DOSX & DOS4GW)
  memcpy (&country_info, SEG_OFS_TO_LIN(_watt_dosTbSeg,0), sizeof(country_info));
#endif

  return (WORD)reg.r_bx;
}

/*
 * Return active codepage
 */
int GetCodePage (void)
{
  IREGS reg;

  if (_watt_os_ver < 0x303)
     return (0);

  memset (&reg, 0, sizeof(reg));
  reg.r_ax = 0x6601;
  GEN_INTERRUPT (0x21, &reg);
  if (reg.r_flags & CARRY_BIT)
     return (0);
  return (WORD)reg.r_bx;
}
#endif    /* __MSDOS__ */

