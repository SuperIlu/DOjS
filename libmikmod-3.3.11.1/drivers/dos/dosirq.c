/*
    Implementation of IRQ routines on DOS
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

#include "dosirq.h"

#include <dpmi.h>
#include <go32.h>
#include <dos.h>
#include <sys/nearptr.h>
#include <malloc.h>
#include <string.h>
#include "mikmod.h" /* for MikMod_malloc() & co */

unsigned int __irq_stack_size = 0x4000;
unsigned int __irq_stack_count = 1;

static void __int_stub_template (void)
{
/* *INDENT-OFF* */
	asm("	pushal\n"
		"	pushl	%ds\n"
		"	pushl	%es\n"
		"	pushl	%fs\n"
		"	pushl	%gs\n"
		"	movw	$0x1234,%ax\n"		/* Get DPMI data selector */
		"	movw	%ax,%ds\n"			/* Set DS and ES to data selector */
		"	movw	%ax,%es\n"
		"	movl	$0x12345678,%ebx\n"	/* Interrupt stack top */
		"	movl	(%ebx),%ecx\n"
		"	movl	%ecx,%edx\n"
		"	subl	$0x12345678,%ecx\n"	/* Subtract irq_stack_count */
		"	movl	%ecx,(%ebx)\n"
		"	movw	%ss,%si\n"			/* Save old SS:ESP */
		"	movl	%esp,%edi\n"
		"	movl	%edx,%esp\n"		/* Set SS:ESP to interrupt stack */
		"	movw	%ax,%ss\n"
		"	pushl	%esi\n"
		"	pushl	%edi\n"
		"	pushl	%ebx\n"
		"	pushl	%edx\n"
		"	call	1f\n"				/* Call user interrupt handler */
		"1:	popl	%edx\n"
		"	popl	%ebx\n"
		"	movl	%edx,(%ebx)\n"
		"	popl	%edi\n"
		"	popl	%esi\n"
		"	movl	%edi,%esp\n"		/* Restore old SS:ESP */
		"	movw	%si,%ss\n"
		"	popl	%gs\n"
		"	popl	%fs\n"
		"	popl	%es\n"
		"	popl	%ds\n"
		"	popal\n"
		"	iret\n");
/* *INDENT-ON* */
}

#include <stdio.h>

static int _allocate_iret_wrapper(_go32_dpmi_seginfo * info)
{
	unsigned char *irqtpl = (unsigned char *)__int_stub_template;
	unsigned char *irqend, *irqwrapper, *tmp;
	__dpmi_meminfo handler_info;
	unsigned int wrappersize;

	/* First, skip until pushal */
	while (*irqtpl != 0x60)
		irqtpl++;
	/* Now find the iret */
	irqend = irqtpl;
	while (*irqend++ != 0xcf);

	wrappersize = 4 + __irq_stack_size * __irq_stack_count + 4 +
	  ((long)irqend - (long)irqtpl);
	irqwrapper = (unsigned char *) MikMod_malloc(wrappersize);
	/* Lock the wrapper */
	handler_info.address = __djgpp_base_address + (unsigned long)irqwrapper;
	handler_info.size = wrappersize;
	if (__dpmi_lock_linear_region(&handler_info)) {
		MikMod_free(irqwrapper);
		return -1;
	}

	/* First comes the interrupt wrapper size */
	*(unsigned long *)irqwrapper = wrappersize;

	/* Next comes the interrupt stack */
	tmp = irqwrapper + 4 + __irq_stack_size * __irq_stack_count;

	/* The following dword is interrupt stack pointer */
	*((void **)tmp) = tmp;
	tmp += 4;

	/* Now comes the interrupt wrapper itself */
	memcpy(tmp, irqtpl, irqend - irqtpl);
	*(unsigned short *)(tmp + 9) = _my_ds();
	*(unsigned long *)(tmp + 16) = (unsigned long)tmp - 4;
	*(unsigned long *)(tmp + 26) = __irq_stack_size;
	*(unsigned long *)(tmp + 46) =
	  info->pm_offset - (unsigned long)(tmp + 50);

	info->pm_offset = (unsigned long)tmp;
	info->pm_selector = _my_cs();

	return 0;
}

static void _free_iret_wrapper(_go32_dpmi_seginfo * info)
{
	__dpmi_meminfo handler_info;

	info->pm_offset -= 4 + __irq_stack_size * __irq_stack_count + 4;

	handler_info.address = __djgpp_base_address + info->pm_offset;
	handler_info.size = *(unsigned long *)info->pm_offset;
	__dpmi_unlock_linear_region(&handler_info);

	MikMod_free((void *)info->pm_offset);
}

struct irq_handle *irq_hook(int irqno, void (*handler)(), unsigned long size)
{
	int interrupt;
	struct irq_handle *irq;
	__dpmi_version_ret version;
	__dpmi_meminfo handler_info, struct_info;
	_go32_dpmi_seginfo info;
	unsigned long old_sel, old_ofs;

	__dpmi_get_version(&version);
	if (irqno < 8)
		interrupt = version.master_pic + irqno;
	else
		interrupt = version.slave_pic + (irqno - 8);

	if (_go32_dpmi_get_protected_mode_interrupt_vector(interrupt, &info))
		return NULL;

	old_sel = info.pm_selector;
	old_ofs = info.pm_offset;

	info.pm_offset = (unsigned long)handler;
	if (_allocate_iret_wrapper(&info))
		return NULL;

	/* Lock the interrupt handler in memory */
	handler_info.address = __djgpp_base_address + (unsigned long)handler;
	handler_info.size = size;
	if (__dpmi_lock_linear_region(&handler_info)) {
		_free_iret_wrapper(&info);
		return NULL;
	}

	irq = (struct irq_handle *) MikMod_malloc(sizeof(struct irq_handle));
	irq->c_handler = handler;
	irq->handler_size = size;
	irq->handler = info.pm_offset;
	irq->prev_selector = old_sel;
	irq->prev_offset = old_ofs;
	irq->int_num = interrupt;
	irq->irq_num = irqno;
	irq->pic_base = irqno < 8 ? PIC1_BASE : PIC2_BASE;

	struct_info.address = __djgpp_base_address + (unsigned long)irq;
	struct_info.size = sizeof(struct irq_handle);
	if (__dpmi_lock_linear_region(&struct_info)) {
		MikMod_free(irq);
		__dpmi_unlock_linear_region(&handler_info);
		_free_iret_wrapper(&info);
		return NULL;
	}

	_go32_dpmi_set_protected_mode_interrupt_vector(interrupt, &info);

	irq->pic_mask = irq_state(irq);
	return irq;
}

void irq_unhook(struct irq_handle *irq)
{
	_go32_dpmi_seginfo info;
	__dpmi_meminfo mem_info;

	if (!irq)
		return;

	/* Restore the interrupt vector */
	irq_disable(irq);
	info.pm_offset = irq->prev_offset;
	info.pm_selector = irq->prev_selector;
	_go32_dpmi_set_protected_mode_interrupt_vector(irq->int_num, &info);

	/* Unlock the interrupt handler */
	mem_info.address = __djgpp_base_address + (unsigned long)irq->c_handler;
	mem_info.size = irq->handler_size;
	__dpmi_unlock_linear_region(&mem_info);

	/* Unlock the irq_handle structure */
	mem_info.address = __djgpp_base_address + (unsigned long)irq;
	mem_info.size = sizeof(struct irq_handle);
	__dpmi_unlock_linear_region(&mem_info);

	info.pm_offset = irq->handler;
	_free_iret_wrapper(&info);

	/* If IRQ was enabled before we hooked, restore enabled state */
	if (irq->pic_mask)
		irq_enable(irq);
	else
		irq_disable(irq);

	MikMod_free(irq);
}

/*---------------------------------------------- IRQ detection mechanism -----*/
static struct irq_handle *__irqs[16];
static int (*__irq_confirm) (int irqno);
static volatile unsigned int __irq_mask;
static volatile unsigned int __irq_count[16];

#define DECLARE_IRQ_HANDLER(irqno)							\
static void __irq##irqno##_handler ()						\
{															\
  if (irq_check (__irqs [irqno]) && __irq_confirm (irqno))	\
  {															\
    __irq_count [irqno]++;									\
    __irq_mask |= (1 << irqno);								\
  }															\
  irq_ack (__irqs [irqno]);									\
}

/* *INDENT-OFF* */
DECLARE_IRQ_HANDLER(0)
DECLARE_IRQ_HANDLER(1)
DECLARE_IRQ_HANDLER(2)
DECLARE_IRQ_HANDLER(3)
DECLARE_IRQ_HANDLER(4)
DECLARE_IRQ_HANDLER(5)
DECLARE_IRQ_HANDLER(6)
DECLARE_IRQ_HANDLER(7)
DECLARE_IRQ_HANDLER(8)
DECLARE_IRQ_HANDLER(9)
DECLARE_IRQ_HANDLER(10)
DECLARE_IRQ_HANDLER(11)
DECLARE_IRQ_HANDLER(12)
DECLARE_IRQ_HANDLER(13)
DECLARE_IRQ_HANDLER(14)
DECLARE_IRQ_HANDLER(15)
/* *INDENT-ON* */

static void (*__irq_handlers[16]) () = {
	__irq0_handler, __irq1_handler, __irq2_handler, __irq3_handler,
	  __irq4_handler, __irq5_handler, __irq6_handler, __irq7_handler,
	  __irq8_handler, __irq9_handler, __irq10_handler, __irq11_handler,
	  __irq12_handler, __irq13_handler, __irq14_handler, __irq15_handler};

void irq_detect_start(unsigned int irqs, int (*irq_confirm) (int irqno))
{
	int i;

	__irq_mask = 0;
	__irq_confirm = irq_confirm;
	memset(&__irqs, 0, sizeof(__irqs));
	memset((void *) &__irq_count, 0, sizeof(__irq_count));

	/* Hook all specified IRQs */
	for (i = 1; i <= 15; i++)
		if (irqs & (1 << i)) {
			__irqs[i] = irq_hook(i, __irq_handlers[i], 200);
			/* Enable the interrupt */
			irq_enable(__irqs[i]);
		}
	/* Enable IRQ2 if we need at least one IRQ above 7 */
	if (irqs & 0xff00)
		_irq_enable(2);
}

void irq_detect_end()
{
	int i;
	for (i = 15; i >= 1; i--)
		if (__irqs[i])
			irq_unhook(__irqs[i]);
}

int irq_detect_get(int irqno, unsigned int *irqmask)
{
	int oldirq = disable();
	int count = __irq_count[irqno];
	*irqmask = __irq_mask;
	__irq_mask = 0;
	if (oldirq)
		enable();
	return count;
}

void irq_detect_clear()
{
	int oldirq = disable();
	memset((void *) &__irq_count, 0, sizeof(__irq_count));
	__irq_mask = 0;
	if (oldirq)
		enable();
}

/* ex:set ts=4: */
