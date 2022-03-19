/*!\file bsdname.h
 */
#ifndef _w32_BSDNAME_H
#define _w32_BSDNAME_H

/* privates */

extern int         _get_machine_name (char *buf, int size);
extern void        _sethostid6 (const void *addr);
extern const void *_gethostid6 (void);

#endif
