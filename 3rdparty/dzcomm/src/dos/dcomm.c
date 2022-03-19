/*
 * DZcomm : a serial port API.
 * file : dcomm.c
 *
 * Low level DOS implementation bits.
 *
 * v1.0 By Dim Zegebart and others (see AUTHORS file)
 *
 * See readme.txt for copyright information.
 */

/* This #define tells dzcomm.h that this is source file for the library and
 * not to include things the are only for the users code.
 */
#define DZCOMM_LIB_SRC_FILE

#include "dzcomm.h"
#include "dzcomm/dzintern.h"

#include <signal.h>

/*
 * Stuff to tell the generic routines how to run with DOS/DJGPP
 */

int dos_dzcomm_init(void);
comm_port *dos_comm_port_init(comm_port *port, comm com);
int dos_comm_port_install_handler(comm_port *port);
void dos_comm_port_uninstall_handler(comm_port *port);
void dos_dzcomm_closedown(void);
int dos_comm_port_in(comm_port *port);
void dos_comm_port_send_break(comm_port *port, int msec_duration);
int dos_comm_port_out(comm_port *port, unsigned char *cp);
int dos_comm_port_io_buffer_query(comm_port *port, int io, int query);
int dos_comm_give_line_status(comm_port *port, dzcomm_line line);
int dos_comm_set_line_status(comm_port *port, dzcomm_line line, int value);

comm_port_func_table dos_comm_port_func_table = {
    /* dzcomm_init       */ dos_dzcomm_init,
    /* init              */ dos_comm_port_init,
    /* install_handler   */ dos_comm_port_install_handler,
    /* uninstall_handler */ dos_comm_port_uninstall_handler,
    /* delete            */ NULL,
    /* closedown         */ dos_dzcomm_closedown,
    /* in                */ dos_comm_port_in,
    /* send_break        */ dos_comm_port_send_break,
    /* out               */ dos_comm_port_out,
    /* io_buffer_query   */ dos_comm_port_io_buffer_query,
    /* flush_output      */ NULL,
    /* flush_input       */ NULL,
    /* give_line_status  */ dos_comm_give_line_status,
    /* set_line_status   */ dos_comm_set_line_status};

_DZ_DRIVER_INFO _comm_port_driver_list[] = {{COMM_PORT_DOS_DRIVER, &dos_comm_port_func_table, TRUE}, {0, NULL, 0}};

/*
 * Global stuff for the DOS/DJGPP parts
 */

static int (*com_irq_wrapper[16])();
comm_port *irq_top_com_port[16] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static comm_port *irq_bot_com_port[16] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
volatile int dz_interrupt_depth = 0;

/*
 * 1. DOS specific internals
 */

/* Masks for particular interrupts in the UART */
#define THREINT 0x02

/* Masks for bits in some registers */
/* Lines we contorl */
#define DTR_IN_MCR 0x01
#define RTS_IN_MCR 0x02
/* Lines we read */
#define CTS_CHANGED_IN_MSR 0x01
#define DSR_CHANGED_IN_MSR 0x02
#define CTS_IN_MSR 0x10 /* equivalent to RTS during loopback */
#define DSR_IN_MSR 0x20 /* equivalent to DTR during loopback */

/* If building debug library put some markers in the data flow */
#ifdef DEBUGMODE
inline void DEBUG_MARKER(fifo_queue *q, unsigned char x) { queue_put_(q, &x); }
#else
#define DEBUG_MARKER(q, x)
#endif

/* previous signal handlers */
static void *old_sig_abrt = NULL;
static void *old_sig_fpe = NULL;
static void *old_sig_ill = NULL;
static void *old_sig_segv = NULL;
static void *old_sig_term = NULL;
static void *old_sig_int = NULL;

#ifdef SIGKILL
static void *old_sig_kill = NULL;
#endif

#ifdef SIGQUIT
static void *old_sig_quit = NULL;
#endif

#ifdef SIGTRAP
static void *old_sig_trap = NULL;
#endif

/*
 * The interupt handing routine must return zero or non-zero. Non-zero
 * will result in interupt chaining, Zero will stop chaining. Chaining
 * can (albeit rarely) cause system problems, and should be used with
 * care when it doesn't, it's an easy may to get your knickers in a twist.
 * So, use the following flag at compile time to determine behavior:
 */

#ifdef DZCOMM_INTERRUPT_CHAINING
#define IRQ_FUNCTION_RETURN_VALUE 1
#else
#define IRQ_FUNCTION_RETURN_VALUE 0
#endif /* DZCOMM_INTERRUPT_CHAINING */

/*----------- DOS COMM PORT INIT ----------------------------*/

comm_port *dos_comm_port_init(comm_port *port, comm com) {
    /* IRQ numbers - standard */
    if (com == _com1 || com == _com3 || com == _com5 || com == _com7)
        port->nIRQ = 4;
    else
        port->nIRQ = 3; /* com2 or com4 */

    port->nPort = 0;
    /* getting comport base address:                                       */
    /* 0x400+com*2 - memory address where stored default comm base address */
    dosmemget(0x400 + com * 2, 2, &port->nPort);
    if (port->nPort == 0) { /* Above standard didn't work! Try defaults ... */
        if (com == _com1 || com == _com3 || com == _com5 || com == _com7)
            port->nPort = 0x3f8;
        else
            port->nPort = 0x2f8;
    }

    strcpy(port->szName, "COM ");

    port->szName[3] = com + 49;
    port->installed = PORT_NOT_INSTALLED;
    port->nBaud = _2400;
    port->nData = BITS_8;
    port->nStop = STOP_1;
    port->nParity = NO_PARITY;
    port->control_type = NO_CONTROL;
    port->msr_handler = NULL;
    port->lsr_handler = NULL;

    _go32_dpmi_lock_data(port, sizeof(comm_port));

    return (port);
}

/*----------------- DOS COMM PORT INSTALL HANDLER --------------*/

int dos_comm_port_install_handler(comm_port *port) {
    int i;
    int divisor;
    unsigned char divlow;
    unsigned char divhigh;
    unsigned char comm_param;
    const unsigned char IMR_8259_0 = 0x21;
    const unsigned char IMR_8259_1 = 0xa1;
    const unsigned char ISR_8259_0 = 0x20;
    const unsigned char ISR_8259_1 = 0xa0;
    const unsigned char IERALL = 0x0f; /* mask for all communications interrupts */
    const unsigned char MCRALL = 0x0f; /* DTR,RTS,OUT1,OUT2=1                    */

    /* Set up variables in port making it easy to see which i/o address is which */
    port->THR = port->nPort;
    port->RDR = port->nPort;
    port->BRDL = port->nPort;
    port->BRDH = 1 + port->nPort;
    port->IER = 1 + port->nPort;
    port->IIR = 2 + port->nPort;
    port->FCR = 2 + port->nPort;
    port->LCR = 3 + port->nPort;
    port->MCR = 4 + port->nPort;
    port->LSR = 5 + port->nPort;
    port->MSR = 6 + port->nPort;
    port->SCR = 7 + port->nPort;

    /* set communication parametrs */
    divisor = 115200 / baud_from_baud_bits[port->nBaud];
    divlow = divisor & 0x000000ff;
    divhigh = (divisor >> 8) & 0x000000ff;
    switch (port->nParity) {
        default:
        case NO_PARITY:
            comm_param = 0x00;
            break;
        case ODD_PARITY:
            comm_param = 0x08;
            break;
        case EVEN_PARITY:
            comm_param = 0x18;
            break;
        case MARK_PARITY:
            comm_param = 0x28;
            break;
        case SPACE_PARITY:
            comm_param = 0x38;
            break;
    }
    switch (port->nData) {
        case BITS_5:
            comm_param |= 0x00;
            break;
        case BITS_6:
            comm_param |= 0x01;
            break;
        case BITS_7:
            comm_param |= 0x02;
            break;
        default:
        case BITS_8:
            comm_param |= 0x03;
            break;
    }
    if (port->nStop == STOP_2) comm_param |= 0x04;

    /* Reset UART into known state - Thanks to Bradley Town    */
    outportb(port->LCR, 0x00); /* Access THR/RBR/IER           */
    outportb(port->IER, 0x00); /* Disable interrupts from UART */
    outportb(port->MCR, 0x04); /* Enable some multi-port cards */

    /* Code based on stuff from SVAsync lib.
     * Clear UART Status and data registers
     * setting up FIFO if possible
     */
    i = inportb(port->SCR);
    outportb(port->SCR, 0x55);
    if (inportb(port->SCR) == 0x55) {
        /* On the off chance that SCR is actually hardwired to 0x55,
         * do the same check with a different value.
         */
        outportb(port->SCR, 0xAA);
        if (inportb(port->SCR) == 0xAA) {
            /* The chip is better than an 8250 - it has a scratch pad */
            outportb(port->SCR, i); /* Set SCR back to what it was before */
            inportb(port->SCR);     /* Give slow motherboards a chance    */

            /* Is there a FIFO ? - go through twice for slow motherboards */
            outportb(port->FCR, 0x01);
            i = inportb(port->FCR);
            outportb(port->FCR, 0x01);
            i = inportb(port->FCR);

            /* Some old stuff relies on this (no idea why) */
            outportb(port->FCR, 0x00);
            inportb(port->FCR); /* Give slow motherboards a chance */

            if ((i & 0x80) == 0) /* It's a 16450 or 8250 with scratch pad */
                ;
            else if ((i & 0x40) == 0) /* It's a 16550 - broken FIFO */
                ;
            else {
                /* It's a 16450A series : try and start the FIFO.
                 * It appears that some chips need a two call protocol, but
                 * those that don't seem to work even if you do start it
                 * twice. The first call is simply to start it, the second
                 * starts it and sets an 8 byte FIFO trigger level.
                 */
                outportb(port->FCR, 0x01);
                inportb(port->FCR); /* Give slow motherboards a chance */
                outportb(port->FCR, 0x87);
                inportb(port->FCR); /* Give slow motherboards a chance */

                /* Check that the FIFO initialised */
                if ((inportb(port->IIR) & 0xc0) != 0xc0) {
                    /*
                     * It didn't so we assume it isn't there but disable it to
                     * be on the safe side.
                     */
                    outportb(port->IIR, 0xfe);
                    inportb(port->IIR); /* Give slow motherboards a chance */
                }
            }
        }
    }
    /* end of (modified) SVAsync code */

    /* set interrupt parametrs */
    /* calculating mask for 8259 controller's IMR         */
    /* and number of interrupt handler for given irq level */
    if (port->nIRQ <= 7) { /* if 0<=irq<=7 first IMR address used */
        port->interrupt_enable_mask = ~(0x01 << port->nIRQ);
        port->nIRQVector = port->nIRQ + 8;
        port->IMR_8259 = IMR_8259_0;
        port->ISR_8259 = ISR_8259_0;
    } else {
        port->interrupt_enable_mask = ~(0x01 << (port->nIRQ % 8));
        port->nIRQVector = 0x70 + (port->nIRQ - 8);
        port->IMR_8259 = IMR_8259_1;
        port->ISR_8259 = ISR_8259_1;
    }

    /* Set Port Toggle to BRDL/BRDH registers to set baud rate */
    outportb(port->LCR, comm_param | 0x80);
    outportb(port->BRDL, divlow);
    outportb(port->BRDH, divhigh);
    /* reset LCR and Port Toggleout */
    outportb(port->LCR, comm_param & 0x7F);

    port->next_port = NULL;
    port->last_port = irq_bot_com_port[port->nIRQ];

    dz_disable_interrupts(); /* Stop interrupts for a few critical operations */
    if (irq_top_com_port[port->nIRQ] == NULL)
        irq_top_com_port[port->nIRQ] = port;
    else
        irq_bot_com_port[port->nIRQ]->next_port = port;
    irq_bot_com_port[port->nIRQ] = port;
    dz_enable_interrupts(); /* And allow interrupts again, hairy bit done */

    /* If the first port on this IRQ install a handler */
    if (port->last_port == NULL) {
        dz_disable_interrupts(); /* Stop interrupts for a few operations */
        if (_dzdos_install_irq(port->nIRQVector, com_irq_wrapper[port->nIRQ]) != 0) {
            dz_enable_interrupts(); /* Enable interrupts again before returning failure */
            dz_make_comm_err("Unable to set up an interrupt for a comm port.");
            return 0;
        }
        /* enable interrupt port->nIRQ level */
        outportb(port->IMR_8259, inportb(port->IMR_8259) & port->interrupt_enable_mask);
        dz_enable_interrupts(); /* And allow interrupts again */
    }

    /* Get the initial CTS state and set our internal flag to represent it. Note
     * that a low (physical) line corresponds to it being OK for us to transmit.
     * This corresponds to a high in the relevant bit in the MCR register.
     */
    dz_disable_interrupts(); /* Stop interrupts for a few critical operations */
    if (inportb(port->MSR) & CTS_IN_MSR)
        port->cts = CTS_ON;
    else
        port->cts = CTS_OFF;
    dz_enable_interrupts(); /* And allow interrupts again */

    /* We are able to receive to start with and so set the internal state
     * representation to indicate that we are accepting currently
     */
    port->rts = 1;
    /*
     * We are ready to transmit (even if we have nothing) to start with,
     * so set out internal representation to that.
     */
    port->dtr = 1;

    /* Set the DTR, RTS, OUT1 and OUT2 flags to start level (see above) */
    outportb(port->MCR, MCRALL);

    /* enable all communication's interrupts */
    outportb(port->IER, IERALL);

    return 1;
}

/*-------------- DOS COMM PORT UNINSTALL HANDLER --------------------------*/

void dos_comm_port_uninstall_handler(comm_port *port) {
    comm_port *i_port;

    /* Get any outstanding stuff to send sent */
    while (queue_empty(port->OutBuf) != DZQ_EMPTY)
        ; /* Is the queue empty? */
    while (!(inportb(port->LSR) & 0x20))
        ; /* Is the chip empty? */

    /*
     * Although the easy solution is to close down interrupts for all the rest
     * of this routine, this appears to cause hangs on a particular
     * motherboard/UART combination. So, we only disable interrupts when we
     * absolutely have to.
     * Effectively we are handling the case where the UART we are removing
     * issues an interrupt after the global disable but before it is
     * specifically disabled. If we do not allow this interrupt and go
     * straight on to removing the UART's port entry when we reallow interupts
     * globally we will be unable to service the interrupt, which is a bit
     * unfortunate ...
     */

    /* Turn of interrupts from this port */
    outportb(port->IER, 0x00);
    inportb(port->IER); /* Give slow motherboards a chance */

    /* Reset the modem control register (inc CTS/RTS flow control) */
    outportb(port->MCR, 0x00);
    inportb(port->MCR); /* Give slow motherboards a chance */
    /* And align the internal representation */
    port->rts = 0;
    port->dtr = 0;

    /* If we're the last one using the IRQ then don't get called any more */

    /* And we used to then disable this interrupt entirely.
     * But what if someone else is chaining ?
     */
    /* outportb(port->IMR_8259,inportb(port->IMR_8259)&~(port->interrupt_enable_mask)); */

    if (port == irq_top_com_port[port->nIRQ]) {
        /* We're removing the top one from the chain, so get the second one */
        i_port = (comm_port *)irq_top_com_port[port->nIRQ]->next_port;

        /* If there is a second one on the list, then it's last port is no longer there */
        dz_disable_interrupts();
        if (i_port != NULL)
            i_port->last_port = NULL;
        else {
            /* There is no second port, so remove the handler */
            _dzdos_remove_irq(port->nIRQVector);
        }
        irq_top_com_port[port->nIRQ] = i_port;
        dz_enable_interrupts();
    } else if (port == irq_bot_com_port[port->nIRQ]) {
        /* We're removing the bottom one from the chain */
        i_port = (comm_port *)port->last_port;
        /* As we weren't removing the first one, there must be a previous one that should no longer point on to this one */
        dz_disable_interrupts();
        i_port->next_port = NULL;
        irq_bot_com_port[port->nIRQ] = i_port;
        dz_enable_interrupts();
    } else {
        /* There must be a port either side, as this is neither a top nor bottom port */
        dz_disable_interrupts();
        ((comm_port *)port->last_port)->next_port = port->next_port;
        ((comm_port *)port->next_port)->last_port = port->last_port;
        dz_enable_interrupts();
    }
}

/*------------------------- DOS INTERRUPT ON -----------------------------*/

static inline void dos_interrupt_on(comm_port *port, unsigned char _interrupt_) {
    unsigned char i = inportb(port->IER);
    if (!(i & _interrupt_)) outportb(port->IER, i | _interrupt_);
}
END_OF_FUNCTION(dos_interrupt_on);

/*------------------------- DOS INTERRUPT OFF -----------------------------*/

static inline void dos_interrupt_off(comm_port *port, unsigned char _interrupt_) {
    unsigned char i = inportb(port->IER);
    if (i & _interrupt_) outportb(port->IER, i & ~_interrupt_);
}
END_OF_FUNCTION(dos_interrupt_off);

/* ----------- DOS COMM PORT FORCE MCR BIT ----------------------- */
/*
 * Low level function for setting the state of any given MCR bit.
 * Replaces comm_port_hand which was never documented anyway.
 */

inline void dos_comm_port_force_mcr_bit(comm_port *port, int bit /* 0 to 7 */, int new_state) {
    int c = inportb(port->MCR);
    int m = 1 << bit;

    if (new_state)
        outportb(port->MCR, c | m);
    else
        outportb(port->MCR, c & (~m));
}

/*-------------------- DOS COMM GIVE LINE STATUS ------------------------*/

int dos_comm_give_line_status(comm_port *port, dzcomm_line line) {
    int v = -1;

    switch (line) {
        case DZCOMM_DTR:
            v = port->dtr;
            break;
        case DZCOMM_RTS:
            v = port->rts;
            break;
        case DZCOMM_CTS:
            v = inportb(port->MSR) & CTS_IN_MSR;
            break;
        case DZCOMM_DSR:
            v = inportb(port->MSR) & DSR_IN_MSR;
            break;
    }

    return (v ? 1 : 0);
}
END_OF_FUNCTION(dos_comm_give_line_status);

/*-------------------- DOS DZCOMM SET LINE STATUS ------------------------*/

int dos_comm_set_line_status(comm_port *port, dzcomm_line line, int value) {
    int v = -1;

    switch (line) {
        case DZCOMM_DTR:
            if (value != port->dtr) {
                v = inportb(port->MCR);
                if (!value)
                    v &= ~DTR_IN_MCR;
                else
                    v |= DTR_IN_MCR;
                outportb(port->MCR, v);
                port->dtr = value;
            }
            v = 1;
            break;
        case DZCOMM_RTS:
            if (value != port->rts) {
                v = inportb(port->MCR);
                if (!value)
                    v &= ~RTS_IN_MCR;
                else
                    v |= RTS_IN_MCR;
                outportb(port->MCR, v);
                port->rts = value;
            }
            v = 1;
            break;
        case DZCOMM_CTS:
        case DZCOMM_DSR:
            v = -1;
            break;
    }

    return v;
}
END_OF_FUNCTION(dos_comm_set_line_status);

/*---------------------- COMM PORT HAND ---------------------------*/
/* The next two (send_xon/xoff) are obviously low
 * level implementation functions but they aren't documented and aren't
 * called internally. So I've commented them out until I know whether I
 * need them or they can go - and they may useful references in the future
 */
/*
inline int dos_comm_port_send_xoff(comm_port *port)
{
  if (port->xonxoff_send != XOFF_SENT) {
     port->xoff=1;
     dos_interrupt_on(port, THREINT);
  }
  return(1);
}
*/
/*
inline int dos_comm_port_send_xon(comm_port *port)
{
  if (port->xonxoff_send!=XON_SENT)
   { port->xon=1;
     dos_interrupt_on(port,THREINT);
   }
  return(1);
}
*/

/*----------------------- DOS COMM PORT RDR ------------------------*/

static inline void dos_comm_port_rdr(comm_port *port) {
    unsigned char c = (unsigned char)inportb(port->RDR);
    int n;

    c &= port->valid_bit_mask;

    switch (port->control_type) {
        case NO_CONTROL:
            queue_put_(port->InBuf, &c); /* We ignore the return value - if the queue's full so be it */
            port->in_cnt++;
            break;
        case XON_XOFF:
            if (c == XON_ASCII) {
                port->xonxoff_rcvd = XON_RCVD;
                dos_interrupt_on(port, THREINT);
                return;
            }
            if (c == XOFF_ASCII) {
                port->xonxoff_rcvd = XOFF_RCVD;
                dos_interrupt_off(port, THREINT);
                return;
            }
            n = queue_put_(port->InBuf, &c);
            port->in_cnt++;
            if (n != DZQ_NOT_NEARLY_FULL) {
                /* buffer is near full */
                if (port->xonxoff_send != XOFF_SENT) {
                    port->xoff = 1;
                    dos_interrupt_on(port, THREINT);
                }
            }
            break;
        case RTS_CTS:
            n = queue_put_(port->InBuf, &c);
            port->in_cnt++;
            if (n != DZQ_NOT_NEARLY_FULL) {
                /* buffer is near full and so we don't wish to receive any more data.
                 * By setting the RTS bit in the MCR register to low we make the
                 * (physical) RTS line high which should stop the other end
                 * transmitting.
                 */
                /* This code is identical to the code in dos_comm_set_line_status
                 * but is explicitly here to minimise CPU wastage.
                 */
                outportb(port->MCR, (inportb(port->MCR) & ~RTS_IN_MCR));
                port->rts = 0;
            }
            break;
        default:
            break;
    }
}
static END_OF_FUNCTION(dos_comm_port_rdr)

    /*----------------------- DOS COMM PORT TRX ------------------------*/

    static inline void dos_comm_port_trx(comm_port *port) {
    unsigned char uc;

    switch (port->control_type) {
        case NO_CONTROL:
            if (queue_empty(port->OutBuf) == DZQ_EMPTY) {
                /* queue empty, nothig to send */
                dos_interrupt_off(port, THREINT);
                return;
            }
            queue_get_(port->OutBuf, &uc);
            port->out_cnt++;
            outportb(port->THR, (int)uc);
            break;
        case XON_XOFF:
            if (port->xoff == 1) {
                outportb(port->THR, XOFF_ASCII);
                port->xoff = 0;
                port->xonxoff_send = XOFF_SENT;
            } else if (port->xon == 1) {
                outportb(port->THR, XON_ASCII);
                port->xon = 0;
                port->xonxoff_send = XON_SENT;
                return;
            }
            if (queue_empty(port->OutBuf) == DZQ_EMPTY) {
                /* queue empty, nothig to send */
                dos_interrupt_off(port, THREINT);
                return;
            }
            queue_get_(port->OutBuf, &uc);
            outportb(port->THR, (int)uc);
            port->out_cnt++;
            break;
        case RTS_CTS:
            if (queue_empty(port->OutBuf) == DZQ_EMPTY) {
                /* queue empty, nothing to send */
                dos_interrupt_off(port, THREINT);
                return;
            }
            if (port->cts == CTS_OFF) {
                /* In theory, this should never happen. However, I'm not convinced
                 * that there isn't a race condition in which it might not and so
                 * I'm checking for it and battening down the hatches just to be safe.
                 */
                dos_interrupt_off(port, THREINT);
                return;
            }
            queue_get_(port->OutBuf, &uc);
            outportb(port->THR, (int)uc);
            port->out_cnt++;
            break;
        default:
            break;
    }
}
static END_OF_FUNCTION(dos_comm_port_trx);

/*------------------ DZ COMM PORT INTERRUPT HANDLER -------------------*/

/* A very solid method of handling the UART interrupts that avoids
 * all possible int failures has been suggested by Richard Clayton,
 * and I recommend it as well. Let your interrupt handler do the
 * following:
 *  1. Disarm the UART interrupts by masking them in the IMR of the ICU.
 *  2. Send a specific or an unspecific EOI to the ICU (first slave, then
 *     master, if you're using channels above 7).
 *  3. Enable CPU interrupts (STI) to allow high priority ints to be processed.
 *  4. Read IIR and follow its contents until bit 0 is set.
 *  5. Check if transmission is to be kicked (when XON received or if CTS
 *     goes high); if yes, call tx interrupt handler manually.
 *  6. Disable CPU interrupts (CLI).
 *  7. Rearm the UART interrupts by unmasking them in the IMR of the ICU.
 *  8. Return from interrupt.
 */

/* Note that the 'clever' dz_interrupt_{en,dis}able routines are NOT
 * used here because (1) we want speed and (2) we are in control and
 * all key stuff is done after an ENABLE and before the final DISABLE.
 * This also stops us accidentally messing up the depth counter for
 * everyone else.
 */

/* always return zero - chaining or not chaining handled by the wrapper */
int dos_comm_port_interrupt_handler(comm_port *port) {
    int int_id;
    int c;
    comm_port *c_port = port;

    DEBUG_MARKER(port->InBuf, '[');
    DEBUG_MARKER(port->InBuf, inportb(0x2c0) + '0');

    /* 1 */
    outportb(port->IMR_8259, inportb(port->IMR_8259) | ~port->interrupt_enable_mask);

    /* 2 */
    if (port->nIRQ > 7) {
        outportb(0xA0, 0x60 + (port->nIRQ & 0x07));
        outportb(0x20, 0x62);
    } else
        outportb(0x20, 0x60 + port->nIRQ);

    /* 3 */
    ENABLE();

    /* Point 4 above: */
    do {
        port = c_port;

        /* a. Decide which UART has issued the interrupt */
        int_id = inportb(port->IIR);
        while ((int_id & 0x01) && (int_id > 0)) {
            /* This UART did not call */
            if (port->next_port == NULL)
                int_id = -1; /* Nothing (left) to service - Thanks Mike */
            else {
                port = port->next_port;
                int_id = inportb(port->IIR);
            }
        }

        if (int_id > -1) {
            /* b. service whatever requests the calling UART may have.
             * The top 4 bits are either unused or indicate the presence
             * of a functioning FIFO, which we don't need to know. So
             * trim them off to simplify the switch statement below.
             */
            int_id &= 0x0f;
            do {
                switch (int_id) {
                    case 0x0c: /* Timeout */
                        /* Called when FIFO not up to trigger level but no activity
                         * for a while. Handled exactly as RDAINT, see below for
                         * description.
                         */
                        DEBUG_MARKER(port->InBuf, 'C');
                        do {
                            dos_comm_port_rdr(port);
                        } while (inportb(port->LSR) & 0x01);
                        break;
                    case 0x06: /* LSINT */
                        DEBUG_MARKER(port->InBuf, '6');
                        c = inportb(port->LSR);
                        if (port->lsr_handler != NULL) port->lsr_handler(port, c);
                        break;
                    case 0x04: /* RDAINT */
                        DEBUG_MARKER(port->InBuf, '4');
                        /* The IIR flag tested above stops when the FIFO is
                         * below the trigger level rather than empty, whereas
                         * this flag allows one to empty it:
                         * (do loop because there must be at least one to read
                         *  by virtue of having got here.)
                         */
                        do {
                            dos_comm_port_rdr(port);
                        } while (inportb(port->LSR) & 0x01);
                        break;
                    case 0x02: /* THREINT */
                        DEBUG_MARKER(port->InBuf, '2');
                        dos_comm_port_trx(port);
                        break;
                    case 0x00: /* MSINT */
                        DEBUG_MARKER(port->InBuf, '0');
                        c = inportb(port->MSR);
                        /* This interrupt may have resulted from a CTS state change. We could
                         * check this by verifying that CTS_CHANGED_IN_MSR is high as well in
                         * this next if statement but by why not verify the state more often?
                         */
                        if (port->control_type == RTS_CTS) {
                            /* If the (physical) line is low then the CTS bit in the MCR
                             * is high. This tells us that we can transmit and so set our
                             * state as such and turn transmit interrupts on if there is
                             * something to transmit.
                             */
                            if ((port->cts == CTS_OFF) && (c & CTS_IN_MSR)) {
                                port->cts = CTS_ON;
                                if (queue_empty(port->OutBuf) == DZQ_NOT_EMPTY) {
                                    dos_interrupt_on(port, THREINT);
                                }
                            }
                            /* On the other hand, if the (physical) line is high then the
                             * CTS bit in the MCR is low. This tells us that we should not
                             * transmit and so set our state as such and ensure that the
                             * transmit interrupts off.
                             */
                            else if ((port->cts == CTS_ON) && !(c & CTS_IN_MSR)) {
                                port->cts = CTS_OFF;
                                dos_interrupt_off(port, THREINT);
                            }
                            /* And call any routine the user has set up for the occasion */
                            if (port->msr_handler != NULL) port->msr_handler(port, c);
                        }
                        break;
                } /* end switch */

                /* Point 5 - This is a kick for UARTS that need it even though the enabled
                 * THRE interrupt should render it unnecessary */
                if (inportb(port->LSR) & 0x60) dos_comm_port_trx(port);

                /* Get the next instruction, trimming as above */
                int_id = inportb(port->IIR) & 0x0f;
            } while (!(int_id & 0x01)); /* loop if more instructions */

        } /* end if */

    } while (int_id != -1);

    /* Point 6 */
    DISABLE();

    /* Point 7 */
    outportb(c_port->IMR_8259, inportb(c_port->IMR_8259) & c_port->interrupt_enable_mask);

    /* Point 8 */
    return (0);
}
END_OF_FUNCTION(dos_comm_port_interrupt_handler);

/*----------------------INTERRUPT HANDLER WRAPPERS --------------------*/

#define com_irq_wrapper_(N)                                   \
    static int com##N##_irq_wrapper(void) {                   \
        DISABLE();                                            \
        dos_comm_port_interrupt_handler(irq_top_com_port[N]); \
        return (IRQ_FUNCTION_RETURN_VALUE);                   \
    }                                                         \
    END_OF_FUNCTION(com##N##_irq_wrapper);

com_irq_wrapper_(0);
com_irq_wrapper_(1);
com_irq_wrapper_(2);
com_irq_wrapper_(3);
com_irq_wrapper_(4);
com_irq_wrapper_(5);
com_irq_wrapper_(6);
com_irq_wrapper_(7);
com_irq_wrapper_(8);
com_irq_wrapper_(9);
com_irq_wrapper_(10);
com_irq_wrapper_(11);
com_irq_wrapper_(12);
com_irq_wrapper_(13);
com_irq_wrapper_(14);
com_irq_wrapper_(15);

/*-------------- DOS COMM PORT TEST FLOW BIT ---------------------------*/

void dos_comm_port_test_flow_bit(comm_port *port) {
    switch (port->control_type) {
        case NO_CONTROL:
            break;
        case XON_XOFF:  // XON/XOFF
            if ((queue_nearly_full(port->InBuf) == DZQ_NOT_NEARLY_FULL) && (port->xonxoff_send != XON_SENT)) {
                port->xon = 1;
                dos_interrupt_on(port, THREINT);
            }
            break;
        case RTS_CTS:
            if ((queue_nearly_full(port->InBuf) == DZQ_NOT_NEARLY_FULL) && (port->rts == 0)) {
                /* The buffer now has spare capacity and we had inhibited transmission
                 * and so we can now set the RTS bit in the MCR register high which sets
                 * the (physical) line low which should tell the other end it can transmit
                 * again.
                 */
                /* This code is the same as the code in dos_comm_set_line_status but
                 * is here explicitly to cut down in CPU wastage here.
                 */
                outportb(port->MCR, (inportb(port->MCR) | RTS_IN_MCR));  // setting RTS
                port->rts = 1;
            }
            break;
        default:
            break;
    }
}

/*--------------------- DOS COMM PORT START BREAK --------------------*/
void dos_comm_port_start_break(comm_port *port) { outportb(port->LCR, inportb(port->LCR) | 0x40); }
END_OF_FUNCTION(dos_comm_port_start_break);

/*--------------------- DOS COMM PORT STOP BREAK ---------------------*/
void dos_comm_port_stop_break(comm_port *port) { outportb(port->LCR, inportb(port->LCR) & 0xbf); }
END_OF_FUNCTION(dos_comm_port_stop_break);

/*--------------------- DOS COMM PORT EOB INT ------------------------*/
void dos_comm_port_eob_int(void *p) {
    dos_comm_port_stop_break((comm_port *)p);
    dzdos_remove_param_int(dos_comm_port_eob_int, p);
}
END_OF_FUNCTION(dos_comm_port_eob_int);

/*--------------------- DOS COMM PORT SEND BREAK ---------------------*/
void dos_comm_port_send_break(comm_port *port, int msec_duration) {
    /* The timer needs to be installed */
    if (!_dzdos_timer_installed) dzdos_install_timer();

    /* Start the break signal */
    dos_comm_port_start_break(port);

    /* Set up an interrupt to stop the interrupt in msec_durations time */
    dzdos_install_param_int(dos_comm_port_eob_int, (void *)port, msec_duration);
}

/*--------------------- DOS COMM PORT IO BUFFER QUERY ----------------*/
int dos_comm_port_io_buffer_query(comm_port *port, int io, int query) {
    fifo_queue *q;
    int rv = 0;

    if (io == DZ_IO_BUFFER_IN) {
        /* Do flow implementation */
        dos_comm_port_test_flow_bit(port);
        q = port->InBuf;
    } else
        q = port->OutBuf;

    switch (query) {
        case DZ_IO_BUFFER_EMPTY:
            if (queue_empty(q) == DZQ_EMPTY) rv = 1;
            break;
        case DZ_IO_BUFFER_FULL:
            if (queue_full(q) == DZQ_EMPTY) rv = 1;
    }

    return rv;
}

/*----------------------- DOS COMM PORT IN ---------------------------*/
int dos_comm_port_in(comm_port *port) {
    unsigned char c;

    /* Get the next character from the buffer, note that this routine
     * is only called having checked that the is (at least) one
     * to get
     */
    queue_get_(port->InBuf, &c);
    /* Having emptied the buffer a bit, make any flow control changes
     * that are necessary.
     */
    dos_comm_port_test_flow_bit(port);

    return ((int)c);
}

/*----------------------- DOS COMM PORT OUT --------------------------*/
int dos_comm_port_out(comm_port *port, unsigned char *cp) {
    if (queue_put_(port->OutBuf, cp) < 0) return 0;

    switch (port->control_type) {
        case NO_CONTROL:
            dos_interrupt_on(port, THREINT);
            break;
        case XON_XOFF:
            if (port->xonxoff_rcvd != XOFF_RCVD) {
                dos_interrupt_on(port, THREINT);
            }
            break;
        case RTS_CTS:
            if (port->cts == CTS_ON) { /* modem is 'clear to send' */
                dos_interrupt_on(port, THREINT);
            }
            break;
        default:
            break;
    }

    return 1;
}

/*-------------------- DOS DZCOMM CLOSEDOWN--------------------*/

void dos_dzcomm_closedown(void) {
    /* Set the signal routines back to their previous values */
    signal(SIGABRT, old_sig_abrt);
    signal(SIGFPE, old_sig_fpe);
    signal(SIGILL, old_sig_ill);
    signal(SIGSEGV, old_sig_segv);
    signal(SIGTERM, old_sig_term);
    signal(SIGINT, old_sig_int);

#ifdef SIGKILL
    signal(SIGKILL, old_sig_kill);
#endif

#ifdef SIGQUIT
    signal(SIGQUIT, old_sig_quit);
#endif

#ifdef SIGTRAP
    signal(SIGTRAP, old_sig_trap);
#endif

    /* Should probably do the opposite of below but I'm not sure about unlocking yet */
}

/*-------------------- DOS DZCOMM SIGNAL HANDLER --------------*/

/*  Used to trap various signals, to make sure things get shut down cleanly.
 */
static void dos_dzcomm_signal_handler(int num) {
    dzcomm_closedown(); /* Restore the previous signal handler and closedown */
    raise(num);         /* And so this will chain to the previous handler */
}

/*-------------------- DOS DZCOMM INIT ------------------------*/

int dos_dzcomm_init(void) {
    LOCK_FUNCTION(dos_comm_port_interrupt_handler);
    LOCK_FUNCTION(dos_comm_port_rdr);
    LOCK_FUNCTION(dos_comm_port_trx);
    LOCK_FUNCTION(dos_interrupt_on);
    LOCK_FUNCTION(dos_interrupt_off);
    LOCK_FUNCTION(dos_comm_give_line_status);
    LOCK_FUNCTION(dos_comm_set_line_status);
    LOCK_FUNCTION(dos_comm_port_stop_break);
    LOCK_FUNCTION(dos_comm_port_start_break);
    LOCK_FUNCTION(dos_comm_port_eob_int);
    LOCK_FUNCTION(com0_irq_wrapper);
    LOCK_FUNCTION(com1_irq_wrapper);
    LOCK_FUNCTION(com2_irq_wrapper);
    LOCK_FUNCTION(com3_irq_wrapper);
    LOCK_FUNCTION(com4_irq_wrapper);
    LOCK_FUNCTION(com5_irq_wrapper);
    LOCK_FUNCTION(com6_irq_wrapper);
    LOCK_FUNCTION(com7_irq_wrapper);
    LOCK_FUNCTION(com8_irq_wrapper);
    LOCK_FUNCTION(com9_irq_wrapper);
    LOCK_FUNCTION(com10_irq_wrapper);
    LOCK_FUNCTION(com11_irq_wrapper);
    LOCK_FUNCTION(com12_irq_wrapper);
    LOCK_FUNCTION(com13_irq_wrapper);
    LOCK_FUNCTION(com14_irq_wrapper);
    LOCK_FUNCTION(com15_irq_wrapper);
    LOCK_VARIABLE(com_irq_wrapper);
    LOCK_VARIABLE(irq_top_com_port);
    LOCK_VARIABLE(irq_bot_com_port);

    com_irq_wrapper[0] = com0_irq_wrapper;
    com_irq_wrapper[1] = com1_irq_wrapper;
    com_irq_wrapper[2] = com2_irq_wrapper;
    com_irq_wrapper[3] = com3_irq_wrapper;
    com_irq_wrapper[4] = com4_irq_wrapper;
    com_irq_wrapper[5] = com5_irq_wrapper;
    com_irq_wrapper[6] = com6_irq_wrapper;
    com_irq_wrapper[7] = com7_irq_wrapper;
    com_irq_wrapper[8] = com8_irq_wrapper;
    com_irq_wrapper[9] = com9_irq_wrapper;
    com_irq_wrapper[10] = com10_irq_wrapper;
    com_irq_wrapper[11] = com11_irq_wrapper;
    com_irq_wrapper[12] = com12_irq_wrapper;
    com_irq_wrapper[13] = com13_irq_wrapper;
    com_irq_wrapper[14] = com14_irq_wrapper;
    com_irq_wrapper[15] = com15_irq_wrapper;

    /* install emergency-exit signal handlers */
    old_sig_abrt = signal(SIGABRT, dos_dzcomm_signal_handler);
    old_sig_fpe = signal(SIGFPE, dos_dzcomm_signal_handler);
    old_sig_ill = signal(SIGILL, dos_dzcomm_signal_handler);
    old_sig_segv = signal(SIGSEGV, dos_dzcomm_signal_handler);
    old_sig_term = signal(SIGTERM, dos_dzcomm_signal_handler);
    old_sig_int = signal(SIGINT, dos_dzcomm_signal_handler);

#ifdef SIGKILL
    old_sig_kill = signal(SIGKILL, dos_dzcomm_signal_handler);
#endif

#ifdef SIGQUIT
    old_sig_quit = signal(SIGQUIT, dos_dzcomm_signal_handler);
#endif

#ifdef SIGTRAP
    old_sig_trap = signal(SIGTRAP, dos_dzcomm_signal_handler);
#endif

    return (1);
}
