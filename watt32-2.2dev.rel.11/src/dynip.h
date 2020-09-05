/*!\file dynip.h
 */
#ifndef _w32_DYNIP_H
#define _w32_DYNIP_H

#define dynip_init  W32_NAMESPACE (dynip_init)
#define dynip_exec  W32_NAMESPACE (dynip_exec)

void dynip_init (void);
int  dynip_exec (void);

#endif
