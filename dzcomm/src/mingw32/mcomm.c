/*
 * DZcomm : a serial port API.
 * file : mcomm.c
 *
 * Linux low level bits.
 * 
 * v1.0 By Mystery man ...
 *
 * See readme.txt for copyright information.
 */

/* This #define tells dzcomm.h that this is source file for the library and
 * not to include things the are only for the users code.
 */
#define DZCOMM_LIB_SRC_FILE

#include "dzcomm.h"
#include "dzcomm/dzintern.h"

/*
 * Stuff to tell the generic routines how to run with Linux
 */

int        mingw32_dzcomm_init(void);
comm_port *mingw32_comm_port_init(comm_port *port, comm com);
int        mingw32_comm_port_install_handler(comm_port *port);
void       mingw32_comm_port_uninstall_handler(comm_port *port);
void       mingw32_comm_port_delete(comm_port *port);
int        mingw32_comm_port_in(comm_port *port);
void       mingw32_comm_port_send_break(comm_port *port, int msec_duration);
int        mingw32_comm_port_out(comm_port *port, unsigned char *cp);
void       mingw32_comm_port_flush_input(comm_port *port);
void       mingw32_comm_port_flush_output(comm_port *port);
int        mingw32_comm_port_io_buffer_query(comm_port *port, int io, int query);
int        mingw32_comm_give_line_status(comm_port *port, dzcomm_line line);
int        mingw32_comm_set_line_status(comm_port *port, dzcomm_line line, int value);

comm_port_func_table mingw32_comm_port_func_table = {
   mingw32_dzcomm_init,                   /* Overall initialisation         */
   mingw32_comm_port_init,                /* Initialise a port description  */
   mingw32_comm_port_install_handler,     /* Actually make that port run    */
   mingw32_comm_port_uninstall_handler,   /* Stop it running                */
   mingw32_comm_port_delete,              /* Lose it for good               */
   NULL,								  /* Overall closedown              */
   mingw32_comm_port_in,                  /* Get a character that has come  */
   mingw32_comm_port_send_break,          /* Send a break on the port       */
   mingw32_comm_port_out,                 /* Output a charcater on the port */
   mingw32_comm_port_io_buffer_query,     /* Check buffer state             */
   mingw32_comm_port_flush_output,        /* Flush the output buffer        */
   mingw32_comm_port_flush_input,         /* Flush the input buffer         */
   mingw32_comm_give_line_status,
   mingw32_comm_set_line_status
};

#ifdef DZCOMM_LINUX
DZ_VAR(comm_port_func_table, linux_comm_port_func_table);
#endif

_DZ_DRIVER_INFO _comm_port_driver_list[] = 
{
   { COMM_PORT_MINGW32_DRIVER,  &mingw32_comm_port_func_table,  TRUE },
   { 0,                       NULL,                       0 }
};

/*
 * Global stuff for the mingw32 parts
 */


/*
 * 1. mingw32 specific internals
 */


/*----------- mingw32 COMM PORT INIT ----------------------------*/

comm_port *mingw32_comm_port_init(comm_port *port, comm com)
{
/* TODO */
   port->msr_handler  = NULL;
   port->lsr_handler  = NULL;
   port->installed    = PORT_NOT_INSTALLED;
   return port;
}

/*----------------- mingw32 COMM PORT INSTALL HANDLER --------------*/

int mingw32_comm_port_install_handler(comm_port *port)
{
/* TODO */
    return 1;
}

/*-------------- mingw32 COMM PORT UNINSTALL HANDLER --------------------------*/

void mingw32_comm_port_uninstall_handler(comm_port *port)
{
/* TODO */
}


/*-------------- mingw32 COMM PORT DELETE ----------------------------------*/

void mingw32_comm_port_delete(comm_port *port)
{
/* TODO */
}

/*-------------- mingw32 COMM PORT IO BUFFER QUERY -------------------------*/
/* Query the state of an input or output buffer returns 1 if the query is 
 * correct, 0 otherwise.
 */

int mingw32_comm_port_io_buffer_query(comm_port *port, int io, int query)
{
/* TODO */

   return 0;
}

/*-------------- mingw32 COMM PORT IN -----------------------------------*/
int mingw32_comm_port_in(comm_port *port)
{
/* TODO */
    
   return -1;
}


/*--------------------- mingw32 COMM PORT SEND BREAK ---------------------*/
void mingw32_comm_port_send_break(comm_port *port, int msec_duration)
{
/* TODO */
}


/*----------------------- mingw32 COMM PORT OUT --------------------------*/
int mingw32_comm_port_out(comm_port *port, unsigned char *cp)
{
/* TODO */
    return 1;
}

/*----------------------- mingw32 COMM PORT FLUSH INPUT ------------------*/
void mingw32_comm_port_flush_input(comm_port *port)
{
/* TODO */
}

/*----------------------- mingw32 COMM PORT FLUSH OUTPUT ------------------*/
void mingw32_comm_port_flush_output(comm_port *port)
{
/* TODO */
}

/*-------------------- mingw32 DZCOMM INIT ------------------------*/

int mingw32_dzcomm_init(void)
{
/* TODO */
    return (1);
}

/*-------------------- mingw32 DZCOMM CTS STATUS ------------------------*/

int mingw32_comm_give_line_status(comm_port *port, dzcomm_line line)
{
	/* TODO */
	return -1;
}

/*-------------------- mingw32 DZCOMM DSR STATUS ------------------------*/

int mingw32_comm_set_line_status(comm_port *port, dzcomm_line line, int value)
{
	/* TODO */
	return -1;
}
