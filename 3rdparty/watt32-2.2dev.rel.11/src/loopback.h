/*!\file loopback.h
 */
#ifndef _w32_LOOPBACK_H
#define _w32_LOOPBACK_H

#ifndef IPPORT_ECHO
#define IPPORT_ECHO     7
#endif

#ifndef IPPORT_DISCARD
#define IPPORT_DISCARD  9
#endif

#define LBACK_MODE_ENABLE  0x01
#define LBACK_MODE_WINSOCK 0x10    /* highly experimental */

extern WORD  loopback_mode;
extern int   loopback_device (in_Header *);

#endif
