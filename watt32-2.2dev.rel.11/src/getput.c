/*!\file getput.c
 *
 *  get/put short/long functions for little-endian platforms.
 */

/*  Copyright (c) 1997-2002 Gisle Vanem <gvanem@yahoo.no>
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

#include <arpa/nameser.h>
#include <resolv.h>

#include "wattcp.h"
#include "chksum.h"
#include "misc.h"

#undef GETSHORT
#undef GETLONG
#undef PUTSHORT
#undef PUTLONG

/*
 * Functions for get/put short/long. Pointer is _NOT_ advanced.
 */
#define GETSHORT(s, cp) {              \
        u_char *t_cp = (u_char*)(cp);  \
        (s) = ((u_short)t_cp[0] << 8)  \
            | ((u_short)t_cp[1]);      \
      }

#define GETLONG(l, cp) {               \
        u_char *t_cp = (u_char*)(cp);  \
        (l) = ((u_long)t_cp[0] << 24)  \
            | ((u_long)t_cp[1] << 16)  \
            | ((u_long)t_cp[2] << 8)   \
            | ((u_long)t_cp[3]);       \
      }

#define PUTSHORT(s, cp) {              \
        u_short t_s  = (u_short)(s);   \
        u_char *t_cp = (u_char*)(cp);  \
        *t_cp++ = (u_char)(t_s >> 8);  \
        *t_cp   = (u_char)t_s;         \
      }

#define PUTLONG(l, cp) {               \
        u_long  t_l  = (u_long)(l);    \
        u_char *t_cp = (u_char*)(cp);  \
        *t_cp++ = (u_char)(t_l >> 24); \
        *t_cp++ = (u_char)(t_l >> 16); \
        *t_cp++ = (u_char)(t_l >> 8);  \
        *t_cp   = (u_char)t_l;         \
      }


u_short W32_CALL _getshort (const u_char *x)   /* in <arpa/nameserv.h> */
{
  u_short res;
  GETSHORT (res, x);
  return (res);
}

u_long W32_CALL _getlong (const u_char *x)   /* in <arpa/nameserv.h> */
{
  u_long res;
  GETLONG (res, x);
  return (res);
}

void W32_CALL __putshort (u_short var, u_char *ptr)   /* in <resolv.h> */
{
  PUTSHORT (var, ptr);
}

void W32_CALL __putlong (u_long var, u_char *ptr)   /* in <resolv.h> */
{
  PUTLONG (var, ptr);
}

/*
 * If compiler/linker doesn't see our defines for htonl() etc.
 */
#undef htonl
#undef ntohl
#undef htons
#undef ntohs

unsigned long W32_CALL htonl (unsigned long val)
{
  return intel (val);
}
unsigned long W32_CALL ntohl (unsigned long val)
{
  return intel (val);
}
unsigned short W32_CALL htons (unsigned short val)
{
  return intel16 (val);
}
unsigned short W32_CALL ntohs (unsigned short val)
{
  return intel16 (val);
}

