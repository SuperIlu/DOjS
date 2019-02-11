/*
** Licensed under Attribution 4.0 International (CC BY 4.0)
** https://creativecommons.org/licenses/by/4.0/
**
** Code was taken from http://www.shdon.com/dos/sound
** by Steven Don.
** This is a derived/modified version by Andre Seidelt <superilu@yahoo.com>
*/

#ifndef __SBDET_H__
#define __SBDET_H__

#include <stdbool.h>

//! sound blaster type enum
typedef enum _blaster_type {
  SB_NONE,
  SB_UNKOWN,
  SBLASTER,
  SBLASTER_15,
  SBLASTER_PRO2,
  SBLASTER_PRO3,
  SBLASTER_AWE
} blaster_type_t;

//! sound blaster parameter struct
typedef struct _sblaster {
  unsigned int port;
  unsigned int irq;
  unsigned int dma;
  unsigned int dma_high;
  bool bit16;
  blaster_type_t type;
} sblaster_t;

extern bool detect_sb(sblaster_t *blaster);

#endif // __SBDET_H__
