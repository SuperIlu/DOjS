/*
 * DZcomm : a serial port API.
 * file : djirq.c
 *
 * From the allegro library, see below for functionality
 *
 */
/*         ______   ___    ___
 *        /\  _  \ /\_ \  /\_ \
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      Interrupt wrapper functions for djgpp. Unlike the libc
 *      _go32_dpmi_* wrapper functions, these can deal with
 *      reentrant interrupts.
 *
 *      By Shawn Hargreaves.
 *
 *      See readme.txt for copyright information.
 */

/* This #define tells dzcomm.h that this is source file for the library and
 * not to include things the are only for the users code.
 */
#define DZCOMM_LIB_SRC_FILE

#include "../i386/asmdefs.inc"
#include "dzcomm.h"
#include "dzcomm/dzintern.h"

#ifndef DZCOMM_DOS
#error something is wrong with the makefile
#endif

#define MAX_IRQS 8      /* timer + keyboard + soundcard + spares */
#define STACK_SIZE 8192 /* an 8k stack should be plenty */

_DZ_IRQ_HANDLER _dzirq_handler[MAX_IRQS];

unsigned char *_dzirq_stack[IRQ_STACKS];

int _irq_init_called = 0;

extern void _dzirq_wrapper_0(), _dzirq_wrapper_1(), _dzirq_wrapper_2(), _dzirq_wrapper_3(), _dzirq_wrapper_4(), _dzirq_wrapper_5(), _dzirq_wrapper_6(), _dzirq_wrapper_7(),
    _dzirq_wrapper_0_end();

/* _dos_irq_exit:
 *  Routine for freeing the interrupt handler stacks.
 */
void _dzdos_irq_exit() {
    int c;

    if (!_irq_init_called) return;

    for (c = 0; c < IRQ_STACKS; c++) {
        if (_dzirq_stack[c]) {
            _dzirq_stack[c] -= STACK_SIZE - 32;
            free(_dzirq_stack[c]);
            _dzirq_stack[c] = NULL;
        }
    }

    _irq_init_called = 0;
}

/* _dzdos_irq_init:
 *  Initialises this module.
 */
void _dzdos_irq_init() {
    int c;

    if (_irq_init_called) return;

    LOCK_VARIABLE(_dzirq_handler);
    LOCK_VARIABLE(_dzirq_stack);
    LOCK_FUNCTION(_dzirq_wrapper_0);

    for (c = 0; c < MAX_IRQS; c++) {
        _dzirq_handler[c].handler = NULL;
        _dzirq_handler[c].number = 0;
    }

    for (c = 0; c < IRQ_STACKS; c++) {
        _dzirq_stack[c] = malloc(STACK_SIZE);
        if (_dzirq_stack[c]) {
            LOCK_DATA(_dzirq_stack[c], STACK_SIZE);
            _dzirq_stack[c] += STACK_SIZE - 32; /* stacks grow downwards */
        }
    }

    atexit(_dzdos_irq_exit);
    _irq_init_called = 1;
}

/* _install_irq:
 *  Installs a hardware interrupt handler for the specified irq, allocating
 *  an asm wrapper function which will save registers and handle the stack
 *  switching. The C function should return zero to exit the interrupt with
 *  an iret instruction, and non-zero to chain to the old handler.
 */
int _dzdos_real_install_irq(int num, int (*handler)()) {
    __dpmi_paddr addr;
    int c;

    if (!_irq_init_called) _dzdos_irq_init();

    for (c = 0; c < MAX_IRQS; c++) {
        if (_dzirq_handler[c].handler == NULL) {
            addr.selector = _my_cs();

            switch (c) {
                case 0:
                    addr.offset32 = (long)_dzirq_wrapper_0;
                    break;
                case 1:
                    addr.offset32 = (long)_dzirq_wrapper_1;
                    break;
                case 2:
                    addr.offset32 = (long)_dzirq_wrapper_2;
                    break;
                case 3:
                    addr.offset32 = (long)_dzirq_wrapper_3;
                    break;
                case 4:
                    addr.offset32 = (long)_dzirq_wrapper_4;
                    break;
                case 5:
                    addr.offset32 = (long)_dzirq_wrapper_5;
                    break;
                case 6:
                    addr.offset32 = (long)_dzirq_wrapper_6;
                    break;
                case 7:
                    addr.offset32 = (long)_dzirq_wrapper_7;
                    break;
                default:
                    return -1;
            }

            _dzirq_handler[c].handler = handler;
            _dzirq_handler[c].number = num;

            __dpmi_get_protected_mode_interrupt_vector(num, &_dzirq_handler[c].old_vector);
            __dpmi_set_protected_mode_interrupt_vector(num, &addr);

            return 0;
        }
    }

    return -1;
}

/* _remove_irq:
 *  Removes a hardware interrupt handler, restoring the old vector.
 */
void _dzdos_real_remove_irq(int num) {
    int c;

    for (c = 0; c < MAX_IRQS; c++) {
        if (_dzirq_handler[c].number == num) {
            __dpmi_set_protected_mode_interrupt_vector(num, &_dzirq_handler[c].old_vector);
            _dzirq_handler[c].number = 0;
            _dzirq_handler[c].handler = NULL;
            break;
        }
    }
}
