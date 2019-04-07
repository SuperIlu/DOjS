/*
    Interface for DMA routines on DOS
    Copyright (C) 1999 by Andrew Zabolotny, <bit@eltech.ru>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __DOSDMA_H__
#define __DOSDMA_H__

#include <pc.h>

#define DMA1_BASE		0x00	/* 8 bit slave DMA, channels 0..3 */
#define DMA2_BASE		0xC0	/* 16 bit master DMA, ch 4(=slave input)..7 */

#define DMA1_CMD_REG		0x08	/* command register (w) */
#define DMA1_STAT_REG		0x08	/* status register (r) */
#define DMA1_REQ_REG		0x09	/* request register (w) */
#define DMA1_MASK_REG		0x0A	/* single-channel mask (w) */
#define DMA1_MODE_REG		0x0B	/* mode register (w) */
#define DMA1_CLEAR_FF_REG	0x0C	/* clear pointer flip-flop (w) */
#define DMA1_TEMP_REG		0x0D	/* Temporary Register (r) */
#define DMA1_RESET_REG		0x0D	/* Master Clear (w) */
#define DMA1_CLR_MASK_REG	0x0E	/* Clear Mask */
#define DMA1_MASK_ALL_REG	0x0F	/* all-channels mask (w) */

#define DMA2_CMD_REG		0xD0	/* command register (w) */
#define DMA2_STAT_REG		0xD0	/* status register (r) */
#define DMA2_REQ_REG		0xD2	/* request register (w) */
#define DMA2_MASK_REG		0xD4	/* single-channel mask (w) */
#define DMA2_MODE_REG		0xD6	/* mode register (w) */
#define DMA2_CLEAR_FF_REG	0xD8	/* clear pointer flip-flop (w) */
#define DMA2_TEMP_REG		0xDA	/* Temporary Register (r) */
#define DMA2_RESET_REG		0xDA	/* Master Clear (w) */
#define DMA2_CLR_MASK_REG	0xDC	/* Clear Mask */
#define DMA2_MASK_ALL_REG	0xDE	/* all-channels mask (w) */

#define DMA_ADDR_0      0x00	/* DMA address registers */
#define DMA_ADDR_1      0x02
#define DMA_ADDR_2      0x04
#define DMA_ADDR_3      0x06
#define DMA_ADDR_4      0xC0
#define DMA_ADDR_5      0xC4
#define DMA_ADDR_6      0xC8
#define DMA_ADDR_7      0xCC

#define DMA_SIZE_0		0x01	/* DMA transfer size registers */
#define DMA_SIZE_1		0x03
#define DMA_SIZE_2		0x05
#define DMA_SIZE_3		0x07
#define DMA_SIZE_4		0xC2
#define DMA_SIZE_5		0xC6
#define DMA_SIZE_6		0xCA
#define DMA_SIZE_7		0xCE

#define DMA_PAGE_0      0x87	/* DMA page registers */
#define DMA_PAGE_1      0x83
#define DMA_PAGE_2      0x81
#define DMA_PAGE_3      0x82
#define DMA_PAGE_5      0x8B
#define DMA_PAGE_6      0x89
#define DMA_PAGE_7      0x8A

#define DMA_MODE_AUTOINIT	0x10	/* Auto-init mode bit */
#define DMA_MODE_READ		0x44	/* I/O to memory, no autoinit, increment, single mode */
#define DMA_MODE_WRITE		0x48	/* memory to I/O, no autoinit, increment, single mode */
#define DMA_MODE_CASCADE	0xC0	/* pass thru DREQ->HRQ, DACK<-HLDA only */

/* Indexable specific DMA registers */
typedef struct __dma_regs_s {
	unsigned char addr;			/* DMA transfer address register */
	unsigned char page;			/* DMA page register */
	unsigned char size;			/* DMA transfer size register */
	unsigned char mask;			/* DMA mask/unmask register */
	unsigned char flip;			/* DMA flip-flop reset register */
	unsigned char mode;			/* DMA mode register */
} __dma_regs;

extern __dma_regs dma[8];

/* Enable a specific DMA channel */
static inline void dma_enable(unsigned int channel)
{
	outportb(dma[channel].mask, channel & 3);
}

/* Disable a specific DMA channel */
static inline void dma_disable(unsigned int channel)
{
	outportb(dma[channel].mask, (channel & 3) | 0x04);
}

/* Clear the 'DMA Flip Flop' flag */
static inline void dma_clear_ff(unsigned int channel)
{
	outportb(dma[channel].flip, 0);
}

/* Set mode for a specific DMA channel */
static inline void dma_set_mode(unsigned int channel, char mode)
{
	outportb(dma[channel].mode, mode | (channel & 3));
}

/* Set DMA page register */
static inline void dma_set_page(unsigned int channel, char page)
{
	if (channel > 3)
		page &= 0xfe;
	outportb(dma[channel].page, page);
}

/*
  Set transfer address & page bits for specific DMA channel.
  Assumes dma flipflop is clear.
*/
static inline void dma_set_addr(unsigned int channel, unsigned int address)
{
	unsigned char dma_reg = dma[channel].addr;
	dma_set_page(channel, address >> 16);
	if (channel <= 3) {
		outportb(dma_reg, (address) & 0xff);
		outportb(dma_reg, (address >> 8) & 0xff);
	} else {
		outportb(dma_reg, (address >> 1) & 0xff);
		outportb(dma_reg, (address >> 9) & 0xff);
	}
}

/*
  Set transfer size for a specific DMA channel.
  Assumes dma flip-flop is clear.
*/
static inline void dma_set_count(unsigned int channel, unsigned int count)
{
	unsigned char dma_reg = dma[channel].size;
	count--;					/* number of DMA transfers is bigger by one */
	if (channel > 3)
		count >>= 1;
	outportb(dma_reg, (count) & 0xff);
	outportb(dma_reg, (count >> 8) & 0xff);
}

/*
  Query the number of bytes left to transfer.
  Assumes DMA flip-flop is clear.
*/
static inline int dma_get_count(unsigned int channel)
{
	unsigned char dma_reg = dma[channel].size;

	/* using short to get 16-bit wrap around */
	unsigned short count;
	count = inportb(dma_reg);
	count |= inportb(dma_reg) << 8;
	count++;
	return (channel <= 3) ? count : (count << 1);
}

typedef struct dma_buffer_s {
	unsigned char *linear;		/* Linear address */
	unsigned long physical;		/* Physical address */
	unsigned long size;			/* Buffer size */
	unsigned short selector;	/* The selector assigned to this memory */
	unsigned char channel;		/* The DMA channel */
} dma_buffer;

/* Allocate a block of memory suitable for using as a DMA buffer */
extern dma_buffer *dma_allocate(unsigned int channel, unsigned int size);
/* Deallocate a DMA buffer */
extern void dma_free(dma_buffer * buffer);
/* Start DMA transfer to or from given buffer */
extern void dma_start(dma_buffer * buffer, unsigned long count,
					  unsigned char mode);

#endif /* __DOSDMA_H__ */

/* ex:set ts=4: */
