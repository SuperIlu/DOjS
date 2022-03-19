/*!\file sys/wtypes.h
 * Watt-32 type definitions.
 */

/*-
 * Copyright (c) 1982, 1986, 1991 The Regents of the University of California.
 * All rights reserved.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
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
 * @(#)types.h 7.17 (Berkeley) 5/6/91
 * @(#)wtypes.h     Waterloo TCP/IP
 */

/*
 * the naming <sys/wtypes.h> is required for those compilers that
 * have <sys/types.h> in the usual place but doesn't define
 * the following types. This file is included from <sys/socket.h>,
 * <tcp.h> etc.
 *
 * Our basic types (BYTE, WORD, etc.) are defined in the bottom.
 */

#ifndef __SYS_WTYPES_H
#define __SYS_WTYPES_H

#ifndef __SYS_W32API_H
#include <sys/w32api.h>
#endif

#if defined(__DJGPP__) || defined(__DMC__) || \
    defined(__MINGW32__) || defined(__CYGWIN__) || defined(__POCC__)
  #include <sys/types.h>
#endif

#if defined(__DJGPP__) && !defined(WATT32_DJGPP_MINGW)
  #include <machine/endian.h>
#endif

#if defined(__MINGW32__) || (defined(__DJGPP__) && DJGPP_MINOR >= 4) || \
    (defined(__WATCOMC__) && __WATCOMC__ >= 1230) ||  /* OW 1.3+ */     \
    defined(__POCC__)                                 /* PellesC */
  #undef  W32_HAVE_STDINT_H
  #define W32_HAVE_STDINT_H 1
  #include <stdint.h>   /* doesn't define 'u_char' etc. */
#endif

/* MSVC 16+ do have <stdint.h> with 'int32_t' etc.
 */
#if defined(_MSC_VER) && (_MSC_VER >= 1600)
  #undef  W32_HAVE_STDINT_H
  #define W32_HAVE_STDINT_H 1
  #include <stdint.h>   /* doesn't define 'u_char' etc. */

  #if (_MSC_VER >= 1800)
    #include <inttypes.h>
  #endif
#endif

/* Borland/CodeGearC/CBuilder do have <stdint.h> with 'int32_t' etc.
 * Not sure about the '0x0700' value.
 */
#if defined(__BORLANDC__)
  #undef W32_HAVE_STDINT_H

  #if (__BORLANDC__ >= 0x0700) || defined(__CODEGEARC__)
    #define W32_HAVE_STDINT_H 1
    #include <stdint.h>
  #endif
#endif

#if !defined(HAVE_U_INT64_T) && !defined(u_int64_t)
  #if defined(__CYGWIN__)
    #undef  HAVE_U_INT32_T
    #define HAVE_U_INT32_T

    #undef  HAVE_U_INT64_T
    #define HAVE_U_INT64_T

    #undef  HAVE_CADDR_T
    #define HAVE_CADDR_T

  #elif defined(__HIGHC__) || defined(__GNUC__) || defined(__CCDL__) || \
        defined(__LCC__)   || defined(__POCC__)
    typedef unsigned long long u_int64_t;
    #define HAVE_U_INT64_T

  #elif defined(__DMC__) && (__INTSIZE == 4)
    typedef unsigned long long u_int64_t;
    #define HAVE_U_INT64_T

  #elif defined(__WATCOMC__) && defined(__WATCOM_INT64__) && \
      !(defined(__SMALL__) || defined(__LARGE__))
    typedef unsigned __int64 u_int64_t;
    #define HAVE_U_INT64_T

  #elif defined(_MSC_VER) && (_MSC_VER >= 900)
    typedef unsigned __int64 u_int64_t;
    #define HAVE_U_INT64_T

  #elif defined(__BORLANDC__) && defined(WIN32)
    typedef unsigned __int64 u_int64_t;
    #define HAVE_U_INT64_T
  #endif
#endif

#if !defined(__GLIBC__)
  #if !defined(HAVE_U_CHAR) && !defined(u_char) && !defined(__u_char_defined)
    typedef unsigned char u_char;
    #define HAVE_U_CHAR
    #define __u_char_defined
  #endif

  #if !defined(HAVE_U_SHORT) && !defined(u_short) && !defined(__u_short_defined)
    typedef unsigned short u_short;
    #define HAVE_U_SHORT
    #define __u_short_defined
  #endif

  #if !defined(HAVE_USHORT) && !defined(ushort)
    typedef unsigned short ushort;   /* SysV compatibility */
    #define HAVE_USHORT
  #endif

  #if !defined(HAVE_U_LONG) && !defined(u_long) && !defined(__u_long_defined)
    typedef unsigned long u_long;
    #define HAVE_U_LONG
    #define __u_long_defined
  #endif

  #if !defined(HAVE_U_INT) && !defined(u_int) && !defined(__u_int_defined)
    #if defined(__SMALL__) || defined(__LARGE__)
      typedef unsigned long u_int;  /* too many headers assumes u_int is >=32-bit */
    #else
      typedef unsigned int  u_int;
    #endif
    #define HAVE_U_INT
    #define __u_int_defined
  #endif

  #if !defined(HAVE_CADDR_T) && !defined(caddr_t) && !defined(__caddr_t_defined)
    /* !! typedef unsigned long caddr_t; */
    typedef char *caddr_t;
    #define HAVE_CADDR_T
    #define __caddr_t_defined
  #endif
#endif

#if !defined(HAVE_U_INT8_T) && !defined(u_int8_t)
  typedef unsigned char u_int8_t;
#endif

#if !defined(HAVE_U_INT16_T) && !defined(u_int16_t)
  typedef unsigned short u_int16_t;
#endif

#if !defined(HAVE_U_INT32_T) && !defined(u_int32_t) && !defined(__u_int32_t_defined)
  typedef unsigned long u_int32_t;
#endif

#if !defined(W32_HAVE_STDINT_H)
  #if !defined(HAVE_INT16_T) && !defined(int16_t)
    typedef short int16_t;
    #define HAVE_INT16_T
  #endif

  #if !defined(HAVE_INT32_T) && !defined(int32_t) && !defined(__CYGWIN__)
    typedef long int32_t;
    #define HAVE_INT32_T
  #endif
#endif

#if !defined(HAVE_U_QUAD_T) && !defined(u_quad_t)
  #define HAVE_U_QUAD_T
  #ifdef HAVE_U_INT64_T
    #define u_quad_t  u_int64_t
  #else
    #define u_quad_t  unsigned long
  #endif
#endif

#if !defined(IOVEC_DEFINED)
  #define IOVEC_DEFINED
  struct iovec {
         void *iov_base;
         int   iov_len;
       };
#endif

#if !defined(socklen_t)
  typedef int       socklen_t;
  #define socklen_t socklen_t
#endif

#if !defined(sa_family_t)
  typedef unsigned short sa_family_t;
  #define sa_family_t    sa_family_t
#endif

#if defined(_BSD_SA_FAMILY_T_)
  typedef _BSD_SA_FAMILY_T_ sa_family_t;
  #undef  _BSD_SA_FAMILY_T_
#endif


#ifndef __BIT_TYPES_DEFINED__
#define __BIT_TYPES_DEFINED__
#endif

#if !defined(FD_SET)    /* not djgpp */
  #undef  FD_SETSIZE
  #define FD_SETSIZE    512
  #define FD_SET(n, p)  ( (p)->fd_bits [(n)/8] |=  (1 << ((n) & 7)) )
  #define FD_CLR(n, p)  ( (p)->fd_bits [(n)/8] &= ~(1 << ((n) & 7)) )
  #define FD_ISSET(n,p) ( (p)->fd_bits [(n)/8] &   (1 << ((n) & 7)) )
  #define FD_ZERO(p)    memset ((void*)(p), 0, sizeof(*(p)))

  typedef struct fd_set {
          unsigned char fd_bits [(FD_SETSIZE+7)/8];
        } fd_set;
#endif

#if defined(__CYGWIN__)
  #include <endian.h>
#else

  #ifndef LITTLE_ENDIAN
  #define LITTLE_ENDIAN 1234
  #endif

  #ifndef BIG_ENDIAN
  #define BIG_ENDIAN    4321
  #endif

  #define BYTE_ORDER    LITTLE_ENDIAN
#endif   /* __CYGWIN__ */

/*
 * Pull in Watt-32 basic types.
 */
#if !defined(WATT32_ON_WINDOWS)
  typedef unsigned char   BYTE;    /**<\typedef 8 bits    */
  typedef unsigned short  WORD;    /**<\typedef 16 bits   */
  typedef unsigned long   DWORD;   /**<\typedef 32 bits   */
  typedef unsigned int    UINT;    /**<\typedef 16/32 bit */

  /* Hack for building Lynx; it defines BOOL to BOOLEAN (char)
   * in it's HTUtils.h.
   */
  #if !defined(BOOLEAN_DEFINED) && !defined(BOOL)
    typedef int           BOOL;    /**<\typedef Boolean */
  #endif
#endif

/*
 * Old compatibility
 */
#if !defined(WATT32_NO_OLDIES)
  #ifndef byte
  #define byte  unsigned char
  #endif

  #ifndef word
  #define word  unsigned short
  #endif

  #ifndef dword
  #define dword unsigned long
  #endif

  #ifndef longword
  #define longword unsigned long
  #endif
#endif

/* Link/network-layer address types
 */
typedef BYTE eth_address[6];      /**<\typedef Ether address */
typedef BYTE tok_address[6];      /**<\typedef TokenRing address */
typedef BYTE fddi_address[6];     /**<\typedef FDDI address */
typedef BYTE ax25_address[7];     /**<\typedef AX-25 address */
typedef BYTE arcnet_address;      /**<\typedef ARCNET address */
typedef BYTE ip6_address[16];     /**<\typedef IPv6 address */

#define mac_address eth_address   /* !!fix-me: breaks AX25 drivers */

/*
 * Function-ptr types
 */

/**<\typedef protocol handler callback type.
 */
typedef int (W32_CALL *ProtoHandler) (void *sock, const BYTE *data, unsigned len,
                                      const void *tcp_phdr, const void *udp_hdr);

/**<\typedef UserHandler
 * callback handler type for _ip_delayX() functions.
 */
typedef int (W32_CALL *UserHandler)  (void *sock);

/**<\typedef VoidProc
 * Handler type for addwattcp() daemons, sock_yield() functions.
 */
typedef void (W32_CALL *VoidProc) (void);

/**<\typedef icmp_upcall.
 * Callback type for ICMP event.
 */
typedef int (W32_CALL *icmp_upcall) (void *socket, BYTE icmp_type, BYTE icmp_code);

#endif
