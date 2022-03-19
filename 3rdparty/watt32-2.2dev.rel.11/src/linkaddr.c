/*!\file linkaddr.c
 *
 */

/* Copyright (c) 1990, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *  $Id: linkaddr.c,v 1.2 1998/08/20 21:56:24 joel Exp $
 */

#include "socket.h"

#if defined(USE_BSD_API)

/* States
 */
#define NAMING  0
#define GOTONE  1
#define GOTTWO  2
#define RESET   3

/* Inputs
 */
#define DIGIT   (4*0)
#define END     (4*1)
#define DELIM   (4*2)
#define LETTER  (4*3)

void W32_CALL link_addr (const char *addr, struct sockaddr_dl *sdl)
{
  char *cp    = sdl->sdl_data;
  char *cplim = sdl->sdl_len + (char*) sdl;
  int   Byte  = 0;
  int   state = NAMING;
  int   New   = 0;

  memset (&sdl->sdl_family, 0, sdl->sdl_len - 1);
  sdl->sdl_family = AF_LINK;

  do
  {
    state &= ~LETTER;
    if (*addr >= '0' && *addr <= '9')
       New = *addr - '0';

    else if (*addr >= 'a' && *addr <= 'f')
       New = *addr - 'a' + 10;

    else if (*addr >= 'A' && *addr <= 'F')
       New = *addr - 'A' + 10;

    else if (*addr == 0)
       state |= END;

    else if (state == NAMING &&
             ((*addr >= 'A' && *addr <= 'Z') ||
              (*addr >= 'a' && *addr <= 'z')))
       state |= LETTER;

    else
       state |= DELIM;

    addr++;
    switch (state)
    {
      case NAMING | DIGIT:
      case NAMING | LETTER:
           *cp++ = addr[-1];
           continue;
      case NAMING | DELIM:
           state = RESET;
           sdl->sdl_nlen = (u_char)(cp - sdl->sdl_data);
           continue;
      case GOTTWO | DIGIT:
           *cp++ = Byte;
           /* FALLTHROUGH */
      case RESET | DIGIT:
           state = GOTONE;
           Byte = New;
           continue;
      case GOTONE | DIGIT:
           state = GOTTWO;
           Byte = New + (Byte << 4);
           continue;
      default:               /* | DELIM */
           state = RESET;
           *cp++ = Byte;
           Byte = 0;
           continue;
      case GOTONE | END:
      case GOTTWO | END:
           *cp++ = Byte;
           /* FALLTHROUGH */
      case RESET | END:
           break;
    }
    break;
  }
  while (cp < cplim);

  sdl->sdl_alen = (u_char)(cp - LLADDR(sdl));
  New = (int) (cp - (char*)sdl);
  if (New > (int)sizeof(*sdl))
     sdl->sdl_len = New;
}

char * W32_CALL link_ntoa (const struct sockaddr_dl *sdl)
{
  static char obuf[64];
  char  *out        = obuf;
  const BYTE *in    = (const BYTE*) LLADDR (sdl);
  const BYTE *inlim = in + sdl->sdl_alen;
  BOOL  firsttime = TRUE;

  if (sdl->sdl_nlen)
  {
    memcpy (obuf, sdl->sdl_data, sdl->sdl_nlen);
    out += sdl->sdl_nlen;
    if (sdl->sdl_alen)
       *out++ = ':';
  }
  while (in < inlim && out < obuf+sizeof(obuf)-3)
  {
    int Byte = *in++;

    if (firsttime)
         firsttime = FALSE;
    else *out++ = '.';
    if (Byte > 0xF)
    {
      *out++ = hex_chars_lower [Byte >> 4];
      *out++ = hex_chars_lower [Byte & 0xf];
    }
    else
      *out++ = hex_chars_lower [Byte];
  }
  *out = '\0';
  return (obuf);
}
#endif /* USE_BSD_API */

