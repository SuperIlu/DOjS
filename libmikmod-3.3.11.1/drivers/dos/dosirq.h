/*
    Interface for IRQ routines on DOS
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

#ifndef __DOSIRQ_H__
#define __DOSIRQ_H__

#include <pc.h>

#define PIC1_BASE	0x20		/* PIC1 base */
#define PIC2_BASE	0xA0		/* PIC2 base */

struct irq_handle {
	void (*c_handler) ();		/* The real interrupt handler */
	unsigned long handler_size;	/* The size of interrupt handler */
	unsigned long handler;		/* Interrupt wrapper address */
	unsigned long prev_selector;	/* Selector of previous handler */
	unsigned long prev_offset;	/* Offset of previous handler */
	unsigned char irq_num;		/* IRQ number */
	unsigned char int_num;		/* Interrupt number */
	unsigned char pic_base;		/* PIC base (0x20 or 0xA0) */
	unsigned char pic_mask;		/* Old PIC mask state */
};

/* Return the enabled state for specific IRQ */
static inline unsigned char irq_state(struct irq_handle * irq)
{
	return ((~inportb(irq->pic_base + 1)) & (0x01 << (irq->irq_num & 7)));
}

/* Acknowledge the end of interrupt */
static inline void _irq_ack(int irqno)
{
	outportb(irqno > 7 ? PIC2_BASE : PIC1_BASE, 0x60 | (irqno & 7));
	/* For second controller we also should acknowledge first controller */
	if (irqno > 7)
		outportb(PIC1_BASE, 0x20);	/* 0x20, 0x62? */
}

/* Acknowledge the end of interrupt */
static inline void irq_ack(struct irq_handle * irq)
{
	outportb(irq->pic_base, 0x60 | (irq->irq_num & 7));
	/* For second controller we also should acknowledge first controller */
	if (irq->pic_base != PIC1_BASE)
		outportb(PIC1_BASE, 0x20);	/* 0x20, 0x62? */
}

/* Mask (disable) the particular IRQ given his ordinal */
static inline void _irq_disable(int irqno)
{
	unsigned int port_no = (irqno < 8 ? PIC1_BASE : PIC2_BASE) + 1;
	outportb(port_no, inportb(port_no) | (1 << (irqno & 7)));
}

/* Unmask (enable) the particular IRQ given its ordinal */
static inline void _irq_enable(int irqno)
{
	unsigned int port_no = (irqno < 8 ? PIC1_BASE : PIC2_BASE) + 1;
	outportb(port_no, inportb(port_no) & ~(1 << (irqno & 7)));
}

/* Mask (disable) the particular IRQ given its irq_handle structure */
static inline void irq_disable(struct irq_handle * irq)
{
	outportb(irq->pic_base + 1,
                 inportb(irq->pic_base + 1) | (1 << (irq->irq_num & 7)));
}

/* Unmask (enable) the particular IRQ given its irq_handle structure */
static inline void irq_enable(struct irq_handle * irq)
{
	outportb(irq->pic_base + 1,
                 inportb(irq->pic_base + 1) & ~(1 << (irq->irq_num & 7)));
}

/* Check if a specific IRQ is pending: return 0 is no */
static inline int irq_check(struct irq_handle * irq)
{
	outportb(irq->pic_base, 0x0B);	/* Read IRR vector */
	return (inportb(irq->pic_base) & (1 << (irq->irq_num & 7)));
}

/* Hook a specific IRQ; NOTE: IRQ is disabled upon return, irq_enable() it */
extern struct irq_handle *irq_hook(int irqno, void (*handler)(),
                                   unsigned long size);
/* Unhook a previously hooked IRQ */
extern void irq_unhook(struct irq_handle * irq);
/* Start IRQ detection process (IRQ list is given with irq mask) */
/* irq_confirm should return "1" if the IRQ really comes from the device */
extern void irq_detect_start(unsigned int irqs,
                             int (*irq_confirm) (int irqno));
/* Finish IRQ detection process */
extern void irq_detect_end();
/* Get the count of specific irqno that happened */
extern int irq_detect_get(int irqno, unsigned int *irqmask);
/* Clear IRQ counters */
extern void irq_detect_clear();

/* The size of interrupt stack */
extern unsigned int __irq_stack_size;
/* The number of nested interrupts that can be handled */
extern unsigned int __irq_stack_count;

#endif /* __DOSIRQ_H__ */

/* ex:set ts=4: */
