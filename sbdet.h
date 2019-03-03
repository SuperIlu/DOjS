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

/*************
** typedefs **
*************/
//! sound blaster type enum
typedef enum _blaster_type { SB_NONE, SB_UNKOWN, SBLASTER, SBLASTER_15, SBLASTER_PRO2, SBLASTER_PRO3, SBLASTER_AWE } blaster_type_t;

/************
** structs **
************/
//! sound blaster parameter struct
typedef struct _sblaster {
    unsigned int port;      //!< port
    unsigned int irq;       //!< irq
    unsigned int dma;       //!< low dma channel
    unsigned int dma_high;  //!< high dma channel
    bool bit16;             //!< TRUE if a 16bit card was detected
    blaster_type_t type;    //!< card type description
} sblaster_t;

/*********************
** static functions **
*********************/
extern bool detect_sb(sblaster_t *blaster);

#endif  // __SBDET_H__
