/*!\file chksum.h
 */
#ifndef _w32_CHECK_SUM_H
#define _w32_CHECK_SUM_H

#define in_checksum_fast W32_NAMESPACE (in_checksum_fast)
#define do_checksum      W32_NAMESPACE (do_checksum)

extern int do_checksum (const BYTE *buf, BYTE proto, unsigned len);

extern WORD _ip6_checksum (const in6_Header *ip, WORD proto,
                           const void *payload, unsigned payloadlen);

extern int  _ip6_tcp_checksum  (const in6_Header *ip, const tcp_Header *tcp, unsigned len);
extern int  _ip6_udp_checksum  (const in6_Header *ip, const udp_Header *udp, unsigned len);
extern int  _ip6_icmp_checksum (const in6_Header *ip, const void *icmp, unsigned len);

/*
 * In chksum0.asm / chksum0.s
 * Fast asm-version doesn't work with Watcom (don't know why).
 * LCC-win32's linker doesn't handle tasm's object files. So use
 * the slow C-version.
 */
#if (DOSX)
  extern WORD cdecl in_checksum_fast (const void *ptr, unsigned len);

  #if defined(__WATCOMC__)
    /* No decoration. Args on stack. All regs preserved */
    #pragma aux (cdecl) _w32_in_checksum_fast "_*" parm caller[];
  #elif defined(__HIGHC__)
    #pragma alias (_w32_in_checksum_fast, "_w32_in_checksum_fast")
  #endif

  #if defined(__WATCOMC__) || defined(__LCC__) || !defined(__i386__)
    #define CHECKSUM(p, len)  in_checksum (p, len)
  #else
    #define HAVE_IN_CHECKSUM_FAST
    #define CHECKSUM(p, len)  in_checksum_fast (p, len)
  #endif
#else
   /* No .asm version for in_checksum().
    */
  #define CHECKSUM(p, len)    in_checksum (p, len)
#endif

#endif
