/*!\file pcsarp.h
 *
 * Secure ARP handler module.
 */
#ifndef _w32_PCSARP_H
#define _w32_PCSARP_H

#include <sys/pack_on.h>

struct sarp_Auth {
       DWORD  magic;
       BYTE   type;
       BYTE   siglen;
       WORD   datalen;
       DWORD  timestamp;
     };

struct sarp_Packet {
       struct arp_Header arp;
       struct sarp_Auth  auth;
     };

#include <sys/pack_off.h>

int sarp_init (void);

#endif

