/*!\file arpa/inet.h
 * Address conversions.
 */

/* Modified for emx by hv and em 1994
 *
 * Copyright (c) 1983, 1993
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
 *      @(#)inet.h      8.1 (Berkeley) 6/2/93
 *      From: Id: inet.h,v 8.5 1997/01/29 08:48:09 vixie Exp $
 */

#ifndef __ARPA_INET_H
#define __ARPA_INET_H

#ifndef __SYS_W32API_H
#include <sys/w32api.h>
#endif

#ifndef __SYS_TYPES_H
#include <sys/wtypes.h>
#endif

#ifndef __SYS_CDEFS_H
#include <sys/cdefs.h>
#endif

#ifndef __NETINET_IN_H
#include <netinet/in.h>
#endif


__BEGIN_DECLS

W32_FUNC int            W32_CALL ascii2addr   (int, const char *, void *);
W32_FUNC char          *W32_CALL addr2ascii   (int, const void *, int, char *);

W32_FUNC struct in_addr W32_CALL inet_makeaddr (u_long, u_long);
W32_FUNC u_long         W32_CALL inet_addr     (const char*);
W32_FUNC u_long         W32_CALL inet_lnaof    (struct in_addr);
W32_FUNC u_long         W32_CALL inet_netof    (struct in_addr);
W32_FUNC u_long         W32_CALL inet_network  (const char*);
W32_FUNC int            W32_CALL inet_aton     (const char *s, struct in_addr *adr);
W32_FUNC char*          W32_CALL inet_ntoa     (struct in_addr);
W32_FUNC char*          W32_CALL inet_ntoa_r   (struct in_addr adr, char *buf, int buflen);

W32_FUNC char          *W32_CALL inet_nsap_ntoa(int binlen, const u_char *binary, char *ascii);
W32_FUNC u_int          W32_CALL inet_nsap_addr(const char *ascii, u_char *binary, int maxlen);
W32_FUNC const char    *W32_CALL inet_ntop     (int af, const void *src, char *dst, size_t size);
W32_FUNC int            W32_CALL inet_pton     (int af, const char *src, void *dst);

__END_DECLS

#endif  /* !__ARPA_INET_H_ */
