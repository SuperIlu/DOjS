/*
 * DZcomm : a serial port API.
 * file : lcomm.c
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

int        linux_dzcomm_init(void);
comm_port *linux_comm_port_init(comm_port *port, comm com);
int        linux_comm_port_install_handler(comm_port *port);
void       linux_comm_port_uninstall_handler(comm_port *port);
void       linux_comm_port_delete(comm_port *port);
int        linux_comm_port_in(comm_port *port);
void       linux_comm_port_send_break(comm_port *port, int msec_duration);
int        linux_comm_port_out(comm_port *port, unsigned char *cp);
void       linux_comm_port_flush_input(comm_port *port);
void       linux_comm_port_flush_output(comm_port *port);
int        linux_comm_port_io_buffer_query(comm_port *port, int io, int query);
int        linux_comm_give_line_status(comm_port *port, dzcomm_line line);
int        linux_comm_set_line_status(comm_port *port, dzcomm_line line, int value);

comm_port_func_table linux_comm_port_func_table = {
   linux_dzcomm_init,                   /* Overall initialisation         */
   linux_comm_port_init,                /* Initialise a port description  */
   linux_comm_port_install_handler,     /* Actually make that port run    */
   linux_comm_port_uninstall_handler,   /* Stop it running                */
   linux_comm_port_delete,              /* Lose it for good               */
   NULL,                                /* Overall closedown              */
   linux_comm_port_in,                  /* Get a character that has come  */
   linux_comm_port_send_break,          /* Send a break on the port       */
   linux_comm_port_out,                 /* Output a charcater on the port */
   linux_comm_port_io_buffer_query,     /* Check buffer state             */
   linux_comm_port_flush_output,        /* Flush the output buffer        */
   linux_comm_port_flush_input,         /* Flush the input buffer         */
   linux_comm_give_line_status,         /* Return a control line status   */
   linux_comm_set_line_status           /* Set a control line status      */
};

/*
 * There is no _DZ_DRIVER_INFO for this file because the linux driver is
 * added to the unix list for libraries which are compiled for linux,
 * see src/unix/ucomm.c
 */

/*
 * Global stuff for the Linux parts
 */

/*
 * 1. Linux specific internals
 */


/*----------- LINUX COMM PORT INIT ----------------------------*/

comm_port *linux_comm_port_init(comm_port *port, comm com)
{
   struct serial_struct sio;
   struct termios       tio;
   speed_t              i_speed;
   speed_t              o_speed;
   int                  sio_avail = 1;
   int                  tio_avail = 1;

   /* 
    * Use /dev/ttysN, N is defined by com (1 to 8 becomes 0 to 7)
    *
    * The process is to open it and then to load its current settings
    * into our structure and return. The user then call the _set_ routines
    * which are implemented on the port_install call.
    */
   sprintf(port->szName, "/dev/ttyS%x", (int) com);
   port->fd = open(port->szName, O_RDWR | O_NOCTTY | O_NONBLOCK );
   if (port->fd == -1) return NULL;

   if (tcgetattr(port->fd, &tio) == -1)        tio_avail = 0;
   if (ioctl(port->fd, TIOCGSERIAL, &sio) < 0) sio_avail = 0;

   if (tio_avail == 1) {
      if (tio.c_cflag & CSTOPB) port->nStop = STOP_2;
      else                      port->nStop = STOP_1;

      if      ((tio.c_cflag & CSIZE) == CS5) port->nData = BITS_5;
      else if ((tio.c_cflag & CSIZE) == CS6) port->nData = BITS_6;
      else if ((tio.c_cflag & CSIZE) == CS7) port->nData = BITS_7;
      else if ((tio.c_cflag & CSIZE) == CS8) port->nData = BITS_8;

      if (tio.c_cflag & PARENB) {
	 if (tio.c_cflag & PARODD) port->nParity = ODD_PARITY;
	 else                      port->nParity = EVEN_PARITY;
	 /*
	  * The sun man pages indicate that it is possible to set
	  * space and mark parity as well using the PAREXT flag
	  * (thought unclear on the details) but the linux man pages
	  * make no mention of this. As this is first go I'll leave
	  * them out currently.
	  */
      }
      else port->nParity = NO_PARITY;

      if (tio.c_cflag & CRTSCTS) {
	 port->control_type = RTS_CTS;
      }
      else if (tio.c_iflag & (IXON | IXOFF | IXANY)) {
	 port->control_type = XON_XOFF;
      }
      else {
	 port->control_type = NO_CONTROL;
      }

      i_speed = cfgetispeed(&tio);
      o_speed = cfgetospeed(&tio);
      if (i_speed == 0) i_speed = o_speed;
      switch (i_speed) {
      case     B50: port->nBaud =     _50; break;
      case     B75: port->nBaud =     _75; break;
      case    B110: port->nBaud =    _110; break;
      case    B134: port->nBaud =    _134; break;
      case    B150: port->nBaud =    _150; break;
      case    B200: port->nBaud =    _200; break;
      case    B300: port->nBaud =    _300; break;
      case    B600: port->nBaud =    _600; break;
      case   B1200: port->nBaud =   _1200; break;
      case   B1800: port->nBaud =   _1800; break;
      case   B2400: port->nBaud =   _2400; break;
      case   B4800: port->nBaud =   _4800; break;
      case   B9600: port->nBaud =   _9600; break;
      case  B19200: port->nBaud =  _19200; break;
      case  B38400: port->nBaud =  _38400; break;
      case  B57600: port->nBaud =  _57600; break;
      case B115200: port->nBaud = _115200; break;
      case B0:
      case B230400: return NULL; /* unsupported currently */
      }
   }
   else {
      /*
       * Put some defaults in and assume that the user will sort out
       * the necessary.
       */
      port->nStop        = STOP_1;
      port->nData        = BITS_8;
      port->nParity      = NO_PARITY;
      port->control_type = NO_CONTROL;
      port->nBaud        = _9600;
   }

   if (sio_avail) {
      port->nPort = sio.port;
      port->nIRQ  = sio.irq;
   }
   else {
      /*
       * Put some defaults in and assume that the user will sort out
       * the necessary.
       */
      if (com==_com1||com==_com3||com==_com5||com==_com7) port->nIRQ=4;
      else                                                port->nIRQ=3;
      if (com==_com1||com==_com3||com==_com5||com==_com7) port->nPort=0x3f8;
      else                                                port->nPort=0x2f8;
   }

   port->msr_handler  = NULL;
   port->lsr_handler  = NULL;
   port->installed    = PORT_NOT_INSTALLED;
   return port;
}

/*----------------- LINUX COMM PORT INSTALL HANDLER --------------*/

int linux_comm_port_install_handler(comm_port *port)
{ /* The setserial man page and source has provided a lot of the inspiration for this */
   struct serial_struct sio;
   struct termios       tio;
   speed_t              speed;

   sio.type = (int) PORT_UNKNOWN; /* Want it to work it out */
   sio.flags = ASYNC_CALLOUT_NOHUP | ASYNC_LOW_LATENCY | ASYNC_SKIP_TEST;
   sio.close_delay = 50; /* Hundredths of a sec */

   /* These three will be set by the config routine: */
   /*   sio.line = (int)           ;*/
   /*   sio.xmit_fifo_size = (int) ;*/
   /*   sio.hub6 = (int)           ;*/

   /* According to linux/serial.h, the following are no longer used.
      To be on the safe side I shall set then to seom sensible values
      anyway (see man setserial for more info on what they used to do
    */
   sio.closing_wait  = (unsigned short) ASYNC_CLOSING_WAIT_NONE;
   sio.closing_wait2 = (unsigned short) 3000;

   sio.port      = (int) port->nPort;
   sio.irq       = (int) port->nIRQ;
   sio.baud_base = 115200;
   sio.custom_divisor = sio.baud_base/baud_from_baud_bits[port->nBaud];
   /* The use of the ASYNC_SPD_* flags is deprecated in favour of the method below: */

   tio.c_iflag = 0;
   tio.c_oflag = 0;
   tio.c_cflag = CREAD | CLOCAL;
   tio.c_lflag = 0;
   if (port->control_type == XON_XOFF) tio.c_iflag |= IXON | IXOFF;

   switch (port->nData) {
   case BITS_5: tio.c_cflag |= CS5; break;
   case BITS_6: tio.c_cflag |= CS6; break;
   case BITS_7: tio.c_cflag |= CS7; break;
   case BITS_8: tio.c_cflag |= CS8; break;
   }

   if (port->nStop == STOP_2) tio.c_cflag |= CSTOPB;
   if (port->control_type  == RTS_CTS) tio.c_cflag |= CRTSCTS;
   if (port->nParity != NO_PARITY) {
      tio.c_cflag |= PARENB;
      if (port->nParity == ODD_PARITY) tio.c_cflag |= PARODD;
   }

   switch (port->nBaud) {
   case     _50: speed =     B50; break;
   case     _75: speed =     B75; break;
   case    _110: speed =    B110; break;
   case    _134: speed =    B134; break;
   case    _150: speed =    B150; break;
   case    _200: speed =    B200; break;
   case    _300: speed =    B300; break;
   case    _600: speed =    B600; break;
   case   _1200: speed =   B1200; break;
   case   _1800: speed =   B1800; break;
   case   _2400: speed =   B2400; break;
   case   _4800: speed =   B4800; break;
   case   _9600: speed =   B9600; break;
   case  _19200: speed =  B19200; break;
   case  _38400: speed =  B38400; break;
   case  _57600: speed =  B57600; break;
   case _115200: speed = B115200; break;
   default:      speed =  B38400; break; /* keep gcc happy */
   }

   cfsetospeed(&tio, speed);
   cfsetispeed(&tio, speed);

   if (ioctl(port->fd, TIOCSSERIAL, &sio) < 0) return 0;
   if (ioctl(port->fd, TIOCSERCONFIG) < 0) return 0;
   tcsetattr(port->fd, TCSANOW, &tio);

   return 1;
}

/*-------------- LINUX COMM PORT UNINSTALL HANDLER --------------------------*/

void linux_comm_port_uninstall_handler(comm_port *port)
{
   /* Wait for both our outgoing queue and the kernal's outgoing queue to clear
    * before we actually uninstall the port:
    * tcdrain is blocking in its own right, but it may exit on failure on an
    * interrupt, so we need to block until it returns success
    */
   while (tcdrain(port->fd) < 0);
   /* Don't need to do anything else to uninstall */
}


/*-------------- LINUX COMM PORT DELETE ----------------------------------*/

void linux_comm_port_delete(comm_port *port)
{
close(port->fd);
}

/*-------------- LINUX COMM GIVE LINE STATUS -----------------------------*/
int linux_comm_give_line_status(comm_port *port, dzcomm_line line)
{
   int flags;

   ioctl(port->fd, TIOCMGET, &flags);

   switch (line) {
   case DZCOMM_DTR:
      flags &= TIOCM_DTR;
      break;
   case DZCOMM_RTS:
      flags &= TIOCM_RTS;
      break;
   case DZCOMM_CTS:
      flags &= TIOCM_CTS;
      break;
   case DZCOMM_DSR:
      flags &= TIOCM_DSR;
      break;
   }

  return (flags?1:0);
}

/*-------------- LINUX COMM SET LINE STATUS ------------------------------*/
int linux_comm_set_line_status(comm_port *port, dzcomm_line line, int value)
{
   int flag;

   if ((line == DZCOMM_CTS) || (line == DZCOMM_DSR)) return -1;

   if (line == DZCOMM_DTR) flag = TIOCM_DTR;
   if (line == DZCOMM_RTS) flag = TIOCM_RTS;

   if (value) ioctl(port->fd, TIOCMBIS, flag);
   else       ioctl(port->fd, TIOCMBIC, flag);

   return 1;
}

/*-------------- LINUX COMM PORT IO BUFFER QUERY -------------------------*/
/* Query the state of an input or output buffer returns 1 if the query is 
 * correct, 0 otherwise.
 */

int linux_comm_port_io_buffer_query(comm_port *port, int io, int query)
{
   int          rv = 0;
   int          ival = 1;  /* These two defaults mean that ioctl failure does */
   unsigned int uival = 1; /* imply empty when we simply don't know           */

   
   if (query == DZ_IO_BUFFER_EMPTY) {
      if (io == DZ_IO_BUFFER_IN) {
	 ioctl(port->fd, TIOCINQ, &uival);
	 if (uival == 0) rv = 1;
      }
      else {
	 ioctl(port->fd, TIOCOUTQ, &ival);
	 if (ival == 0) rv = 1;
      }
   }
   /* Otherwise do nothing because, as far as I can tell, and please do
    * correct me, one can't determine FULL, and linux buffers don't get
    * full anyway, they just get longer
    */

   return rv;
}

/*-------------- LINUX COMM PORT IN -----------------------------------*/
int linux_comm_port_in(comm_port *port)
{
   unsigned char c;

   if (read(port->fd, &c, 1) != 1) return -1;

   return ((int)c);
}


/*--------------------- LINUX COMM PORT SEND BREAK ---------------------*/
void linux_comm_port_send_break(comm_port *port, int msec_duration)
{
   tcsendbreak(port->fd, msec_duration/250 + 1);
}


/*----------------------- LINUX COMM PORT OUT --------------------------*/
int linux_comm_port_out(comm_port *port, unsigned char *cp)
{
   if (write(port->fd, cp, 1) != 1) return 0;

   return 1;
}

/*----------------------- LINUX COMM PORT FLUSH INPUT ------------------*/
void linux_comm_port_flush_input(comm_port *port)
{
   tcflush(port->fd, TCIFLUSH);
}

/*----------------------- LINUX COMM PORT FLUSH OUTPUT ------------------*/
void linux_comm_port_flush_output(comm_port *port)
{
   tcflush(port->fd, TCOFLUSH);
}

/*-------------------- LINUX DZCOMM INIT ------------------------*/

int linux_dzcomm_init(void)
{
   return (1);
}
