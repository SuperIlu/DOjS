/*!\file pcrarp.h
 */
#ifndef _w32_PCRARP_H
#define _w32_PCRARP_H

extern WORD _rarptimeout;
extern int  _dorarp (void);
extern BOOL _rarp_handler (const rarp_Header *rh, BOOL brdcast);

#endif
