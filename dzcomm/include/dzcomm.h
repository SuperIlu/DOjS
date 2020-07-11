/*
 * DZcomm : a serial port API.
 * file : dzcomm.h
 *
 * Main include file
 *
 * See readme.txt for copyright information.
 */

#ifndef DZCOMM_H
#define DZCOMM_H

#ifdef __cplusplus
extern "C" {
#endif

#define DZCOMM_VERSION 0
#define DZCOMM_SUB_VERSION 9
#define DZCOMM_WIP_VERSION 9
#define DZCOMM_VERSION_STR "0.9.9e (WIP)"
#define DZCOMM_DATE_STR "2002"
#define DZCOMM_DATE 20030326 /* yyyymmdd */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dzcomm/dzconfig.h"
#include "dzcomm/dzqueue.h"

#define not_commented(_s_) ((_s_)[-1] != '/' && (_s_)[-2] != '/')

/*-------------------------- SOME GLOBAL STUFF --------------------------*/

#ifndef TRUE
#define TRUE -1
#define FALSE 0
#endif

/* this driver structure and the way it is used is lifted from allergo */
typedef struct _DZ_DRIVER_INFO /* info about a particular driver */
{
    int id;         /* integer ID */
    void *driver;   /* the driver structure */
    int autodetect; /* set to allow autodetection */
} _DZ_DRIVER_INFO;

DZ_ARRAY(char, dz_empty_string);

#ifndef ALLEGRO_H

#define TIMERS_PER_SECOND 1193181L
#define SECS_TO_TIMER(x) ((long)(x)*TIMERS_PER_SECOND)
#define MSEC_TO_TIMER(x) ((long)(x) * (TIMERS_PER_SECOND / 1000))
#define BPS_TO_TIMER(x) (TIMERS_PER_SECOND / (long)(x))
#define BPM_TO_TIMER(x) ((60 * TIMERS_PER_SECOND) / (long)(x))

#undef MIN
#undef MAX
#undef MID

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MID(x, y, z) MAX((x), MIN((y), (z)))

#define EMPTY_STRING "\0\0\0"

#endif

#define scan_(_key_) ({ ((_key_) >> 8) & 0xff; })        /* extract scan part  */
#define ascii_(_key_) ({ (_key_) & 0xff; })              /* extract ascii part */
#define key_shifts_(_key_) ({ ((_key_) >> 16) & 0xff; }) /* extract shifts keys status */

#define data_(_comm_data_) ({ (_comm_data_) & 0xff; })          /* extract data byte */
#define q_reg_(_comm_data_) ({ ((_comm_data_) >> 16) & 0xff; }) /* extract status registers byte */

/* Determine if Ctrl-... pressed
 * 1 - if yes. For example if(ctrl_,'C') {exit(1);}
 * 0 - if not
 */
#define ctrl_(_key_, c) ({ ((key_shifts & KB_CTRL_FLAG) && ((ascii_(_key_) + 'A' - 1) == (c))); })

/*---------------------------- COMM_PORT --------------------------*/

typedef enum { _com1, _com2, _com3, _com4, _com5, _com6, _com7, _com8 } comm;
typedef enum { DZCOMM_DTR, DZCOMM_RTS, DZCOMM_CTS, DZCOMM_DSR } dzcomm_line;

typedef enum { NO_PARITY, ODD_PARITY, EVEN_PARITY, MARK_PARITY, SPACE_PARITY } parity_bits;
typedef enum { STOP_1, STOP_2 } stop_bits;
typedef enum { NO_CONTROL, XON_XOFF, RTS_CTS } flow_control_type;
typedef enum { BITS_5, BITS_6, BITS_7, BITS_8 } data_bits;
typedef enum { _50, _75, _110, _134, _150, _200, _300, _600, _1200, _1800, _2400, _4800, _9600, _19200, _38400, _57600, _115200 } baud_bits;

#define DZ_MIN_PARITY NO_PARITY
#define DZ_MAX_PARITY SPACE_PARITY
#define DZ_MIN_CONTROL NO_CONTROL
#define DZ_MAX_CONTROL RTS_CTS
#define DZ_MIN_STOP STOP_1
#define DZ_MAX_STOP STOP_2
#define DZ_MIN_DATA BITS_5
#define DZ_MAX_DATA BITS_8
#define DZ_MIN_BAUD _50
#define DZ_MAX_BAUD _115200

DZ_ARRAY(int, baud_from_baud_bits);
DZ_ARRAY(int, num_stop_bits);
DZ_ARRAY(int, num_data_bits);
DZ_ARRAY(char *, flow_control_desc);
DZ_ARRAY(char *, parity_desc);

DZ_ARRAY(char, szDZCommErr);

#define DZCOMM_MAX_PORT_NAME_LENGTH 200

/* Comm port. */
typedef struct {                                  /* General and communication parametrs
                                                   * Alter these eight fields after comm_port_new() (where defaults
                                                   * values are set) before calling comm_port_install_handler to
                                                   * get the comm line going. The prefered way to access them is to
                                                   * use the access functions below.
                                                   */
    char szName[DZCOMM_MAX_PORT_NAME_LENGTH + 1]; /* name of comm port. */
    int fd;                                       /* File descriptor for OSes that use one for this */
    unsigned short int nPort;                     /* comm port address eg. 0x3f8    */
    unsigned char nIRQ;                           /* comm IRQ eg. 3                 */
    baud_bits nBaud;                              /* baud rate                      */
    data_bits nData;                              /* data length                    */
    stop_bits nStop;                              /* stop bits                      */
    parity_bits nParity;                          /* parity                         */
    flow_control_type control_type;               /* control type                   */

    /* The next two fields are for setting up functions which are called
     * (if set). The code _must_ be in locked memory and _must_ not demand
     * much CPU. They are called when the UART returns particular conditions
     * in either MSR or LSR registers. The registers are polled and a call
     * to the handlers issued if they are set (ie. non NULL).
     */
    int (*msr_handler)(/* comm_port *port, int msr_contents */);
    int (*lsr_handler)(/* comm_port *port, int lsr_contents */);

    /* Internal flags used for flow control and bit trimming. */
    unsigned char xon;
    unsigned char xoff;
    unsigned char xonxoff_send;
    unsigned char xonxoff_rcvd;
    unsigned char rts;
    unsigned char dtr;
    unsigned char cts;
    unsigned char valid_bit_mask;

    /* next two fields are for byte counting */
    unsigned int in_cnt;   // counter for input data
    unsigned int out_cnt;  // counter for output data

    /* Next two fields used for setting up the IRQ routine and
     * (un)masking the interrupt in certain circumstances.
     */
    unsigned short int nIRQVector;
    unsigned char interrupt_enable_mask;

    /* Internal field to know the state of the comm port and for fitting it
     * into a list.
     */
    enum { PORT_INSTALLED, PORT_NOT_INSTALLED } installed;
    unsigned char keep_chaining;
    void *next_port;
    void *last_port;

    /* input and output queues */
    fifo_queue *InBuf;  /* pointer to read buffer  */
    fifo_queue *OutBuf; /* pointer to write buffer */

    /* This lot are set up to minimise CPU time where accessing the comm
     * port's registers.
     */
    unsigned short int THR;      /* Transmitter Holding Register     */
    unsigned short int RDR;      /* Reciever Data Register           */
    unsigned short int BRDL;     /* Baud Rate Divisor, Low usint     */
    unsigned short int BRDH;     /* Baud Rate Divisor, High Byte     */
    unsigned short int IER;      /* Interupt Enable Register         */
    unsigned short int IIR;      /* Interupt Identification Register */
    unsigned short int FCR;      /* FIFO Control Register            */
    unsigned short int LCR;      /* Line Control Register            */
    unsigned short int MCR;      /* Modem Control Register           */
    unsigned short int LSR;      /* Line Status Register             */
    unsigned short int MSR;      /* Modem Status Register            */
    unsigned short int SCR;      /* SCR Register                     */
    unsigned short int ISR_8259; /* interrupt service register       */
    unsigned short int IMR_8259; /* interrupt mask register          */
} comm_port;

typedef DZ_METHOD(int, dz_ll_handler, (comm_port *, int));

/* System and port initialisation and closing functions */
DZ_FUNC(int, dzcomm_init, (void));
DZ_FUNC(comm_port *, comm_port_init, (comm com));
DZ_FUNC(void, comm_port_delete, (comm_port * port));
DZ_FUNC(void, dzcomm_closedown, (void));

/* Calls for defining the configuration of the port */
DZ_FUNC(void, comm_port_set_name, (comm_port * port, char *name));
DZ_FUNC(void, comm_port_set_port_address, (comm_port * port, unsigned short int port_address));
DZ_FUNC(void, comm_port_set_irq_num, (comm_port * port, unsigned char irq_num));
DZ_FUNC(void, comm_port_set_baud_rate, (comm_port * port, baud_bits baud_rate));
DZ_FUNC(void, comm_port_set_data_bits, (comm_port * port, data_bits num_data_bits));
DZ_FUNC(void, comm_port_set_stop_bits, (comm_port * port, stop_bits num_stop_bits));
DZ_FUNC(void, comm_port_set_parity, (comm_port * port, parity_bits parity_option));
DZ_FUNC(void, comm_port_set_flow_control, (comm_port * port, flow_control_type flow_option));
/* The next two are for setting up functions which are called (if set).
 * The code _must_ be in locked memory and _must_ not demand too much
 * CPU. They are called when the UART returns particular conditions in
 * either MSR or LSR registers. The registers are polled and a call to
 * the handlers issued if they are set (ie. non NULL).
 */
DZ_FUNC(void, comm_port_set_msr_handler, (comm_port * port, dz_ll_handler msr_handler));
DZ_FUNC(void, comm_port_set_lsr_handler, (comm_port * port, dz_ll_handler lsr_handler));

/* Calls for retrieving the configuration of the port */
DZ_FUNC(char *, comm_port_get_name, (comm_port * port, char *name, int max_chars));
DZ_FUNC(unsigned short int, comm_port_get_port_address, (comm_port * port));
DZ_FUNC(unsigned char, comm_port_get_irq_num, (comm_port * port));
DZ_FUNC(baud_bits, comm_port_get_baud_rate, (comm_port * port));
DZ_FUNC(data_bits, comm_port_get_data_bits, (comm_port * port));
DZ_FUNC(stop_bits, comm_port_get_stop_bits, (comm_port * port));
DZ_FUNC(parity_bits, comm_port_get_parity, (comm_port * port));
DZ_FUNC(flow_control_type, comm_port_get_flow_control, (comm_port * port));
DZ_FUNC(dz_ll_handler, comm_port_get_msr_handler, (comm_port * port));
DZ_FUNC(dz_ll_handler, comm_port_get_lsr_handler, (comm_port * port));

/* Calls for putting the configuration into action (or stopping it) */
DZ_FUNC(int, comm_port_install_handler, (comm_port * port));
DZ_FUNC(int, comm_port_reinstall, (comm_port * port));
DZ_FUNC(void, comm_port_uninstall, (comm_port * port));

/* Port running calls */
DZ_FUNC(void, comm_port_flush_input, (comm_port * port));
DZ_FUNC(void, comm_port_flush_output, (comm_port * port));
DZ_FUNC(int, comm_port_out, (comm_port * port, unsigned char c));
DZ_FUNC(int, comm_port_out_empty, (comm_port * port));
DZ_FUNC(int, comm_port_in_empty, (comm_port * port));
DZ_FUNC(int, comm_port_out_full, (comm_port * port));
DZ_FUNC(int, comm_port_in_full, (comm_port * port));
DZ_FUNC(int, comm_port_test, (comm_port * port));
DZ_FUNC(void, comm_port_delete, (comm_port * port));
DZ_FUNC(int, comm_port_string_send, (comm_port * port, const char *s));
DZ_FUNC(int, comm_port_command_send, (comm_port * port, char *s));
DZ_FUNC(void, comm_port_send_break, (comm_port * port, int msec_duration));
DZ_FUNC(int, comm_port_load_settings, (comm_port * port, char *ini_name));
DZ_FUNC(int, modem_hangup, (comm_port * port));
DZ_FUNC(int, comm_port_give_line_status, (comm_port * port, dzcomm_line line));
DZ_FUNC(int, comm_port_set_line_status, (comm_port * port, dzcomm_line line, int value));

/*****************************************/
/************ Helper includes ************/
/*****************************************/

#include "dzcomm/dzinline.h"

#ifdef DZCOMM_EXTRA_HEADER
#include DZCOMM_EXTRA_HEADER
#endif

#ifdef __cplusplus
}
#endif

#endif /* DZCOMM_H */
