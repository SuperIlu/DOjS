/*
    Implementation of DMA routines on DOS
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

#include "dosdma.h"

#include <go32.h> /* includes sys/version.h (djgpp >= 2.02) */
#include <dos.h>
#include <dpmi.h>
#include <sys/nearptr.h>
#include <malloc.h>
#include "mikmod.h" /* for MikMod_malloc() & co */

/* BUG WARNING:  there is an error in DJGPP libraries <= 2.01:
 * src/libc/dpmi/api/d0102.s loads the selector and allocsize
 * arguments in the wrong order.  DJGPP >= 2.02 have it fixed. */
#if (!defined(__DJGPP_MINOR__) || (__DJGPP_MINOR__+0) < 2)
#warning __dpmi_resize_dos_memory() from DJGPP <= 2.01 is broken!
#endif

__dma_regs dma[8] = {
/* *INDENT-OFF* */
	{DMA_ADDR_0, DMA_PAGE_0, DMA_SIZE_0,
	 DMA1_MASK_REG, DMA1_CLEAR_FF_REG, DMA1_MODE_REG},
	{DMA_ADDR_1, DMA_PAGE_1, DMA_SIZE_1,
	 DMA1_MASK_REG, DMA1_CLEAR_FF_REG, DMA1_MODE_REG},

	{DMA_ADDR_2, DMA_PAGE_2, DMA_SIZE_2,
	 DMA1_MASK_REG, DMA1_CLEAR_FF_REG, DMA1_MODE_REG},
	{DMA_ADDR_3, DMA_PAGE_3, DMA_SIZE_3,
	 DMA1_MASK_REG, DMA1_CLEAR_FF_REG, DMA1_MODE_REG},

	{DMA_ADDR_4,          0, DMA_SIZE_4,
	 DMA2_MASK_REG, DMA2_CLEAR_FF_REG, DMA2_MODE_REG},
	{DMA_ADDR_5, DMA_PAGE_5, DMA_SIZE_5,
	 DMA2_MASK_REG, DMA2_CLEAR_FF_REG, DMA2_MODE_REG},

	{DMA_ADDR_6, DMA_PAGE_6, DMA_SIZE_6,
	 DMA2_MASK_REG, DMA2_CLEAR_FF_REG, DMA2_MODE_REG},
	{DMA_ADDR_7, DMA_PAGE_7, DMA_SIZE_7,
	 DMA2_MASK_REG, DMA2_CLEAR_FF_REG, DMA2_MODE_REG}
/* *INDENT-ON* */
};

static int __initialized = 0;
static int __buffer_count = 0;
static __dpmi_meminfo __locked_data;

int dma_initialize()
{
	if (!__djgpp_nearptr_enable())
		return 0;

	/* Trick: Avoid re-setting DS selector limit on each memory allocation
	   call */
	__djgpp_selector_limit = 0xffffffff;

	__locked_data.address = __djgpp_base_address + (unsigned long)&dma;
	__locked_data.size = sizeof(dma);
	if (__dpmi_lock_linear_region(&__locked_data))
		return 0;

	return (__initialized = 1);
}

void dma_finalize()
{
	if (!__initialized)
		return;
	__dpmi_unlock_linear_region(&__locked_data);
	__djgpp_nearptr_disable();
}

dma_buffer *dma_allocate(unsigned int channel, unsigned int size)
{
	int parsize = (size + 15) >> 4;	/* size in paragraphs */
	int par = 0;				/* Real-mode paragraph */
	int selector = 0;			/* Protected-mode selector */
	int mask = channel <= 3 ? 0xfff : 0x1fff;	/* Alignment mask in para. */
	int allocsize = parsize;	/* Allocated size in paragraphs */
	int count;					/* Try count */
	int bound = 0;				/* Nearest bound address */
	int maxsize;				/* Maximal possible block size */
	dma_buffer *buffer = NULL;
	__dpmi_meminfo buff_info, struct_info;

	if (!dma_initialize())
		return NULL;

	/* Loop until we'll get a properly aligned memory block */
	for (count = 8; count; count--) {
		int resize = (selector != 0);

		/* Try first to resize (possibly previously) allocated block */
		if (resize) {
			maxsize = (bound + parsize) - par;
			if (maxsize > parsize * 2)
				maxsize = parsize * 2;
			if (maxsize == allocsize)
				resize = 0;
			else {
				allocsize = maxsize;
				if (__dpmi_resize_dos_memory(selector, allocsize, &maxsize) !=
					0) resize = 0;
			}
		}

		if (!resize) {
			if (selector)
				__dpmi_free_dos_memory(selector), selector = 0;
			par = __dpmi_allocate_dos_memory(allocsize, &selector);
		}

		if ((par == 0) || (par == -1))
			goto exit;

		/* If memory block contains a properly aligned portion, quit loop */
		bound = (par + mask + 1) & ~mask;
		if (par + parsize <= bound)
			break;
		if (bound + parsize <= par + allocsize) {
			par = bound;
			break;
		}
	}
	if (!count) {
		__dpmi_free_dos_memory(selector);
		goto exit;
	}

	buffer = (dma_buffer *) MikMod_malloc(sizeof(dma_buffer));
	buffer->linear = (unsigned char *)(__djgpp_conventional_base + bound * 16);
	buffer->physical = bound * 16;
	buffer->size = parsize * 16;
	buffer->selector = selector;
	buffer->channel = channel;

	buff_info.address = buffer->physical;
	buff_info.size = buffer->size;
	/*
	   Don't pay attention to return code since under plain DOS it often
	   returns error (at least under HIMEM/CWSDPMI and EMM386/DPMI)
	 */
	__dpmi_lock_linear_region(&buff_info);

	/* Lock the DMA buffer control structure as well */
	struct_info.address = __djgpp_base_address + (unsigned long)buffer;
	struct_info.size = sizeof(dma_buffer);
	if (__dpmi_lock_linear_region(&struct_info)) {
		__dpmi_unlock_linear_region(&buff_info);
		__dpmi_free_dos_memory(selector);
		MikMod_free(buffer);
		buffer = NULL;
		goto exit;
	}

  exit:
	if (buffer)
		__buffer_count++;
	else if (--__buffer_count == 0)
		dma_finalize();
	return buffer;
}

void dma_free(dma_buffer * buffer)
{
	__dpmi_meminfo buff_info;

	if (!buffer)
		return;

	buff_info.address = buffer->physical;
	buff_info.size = buffer->size;
	__dpmi_unlock_linear_region(&buff_info);

	__dpmi_free_dos_memory(buffer->selector);
	MikMod_free(buffer);

	if (--__buffer_count == 0)
		dma_finalize();
}

void dma_start(dma_buffer * buffer, unsigned long count, unsigned char mode)
{
	/* Disable interrupts */
	int old_ints = disable();
	dma_disable(buffer->channel);
	dma_set_mode(buffer->channel, mode);
	dma_clear_ff(buffer->channel);
	dma_set_addr(buffer->channel, buffer->physical);
	dma_clear_ff(buffer->channel);
	dma_set_count(buffer->channel, count);
	dma_enable(buffer->channel);
	/* Re-enable interrupts */
	if (old_ints)
		enable();
}

/* ex:set ts=4: */
