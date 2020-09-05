/*!\file rs232.h
 *
 * Execution trace via a serial port, GvB 2002-09.
 *
 * Activated from USER CODE by calling trace2com_init(...) with the
 * port number (either 1-4, or the UARTs hardware address) and the baudrate.
 * Recommend AFAP = 115200. Parameters are always 8,N,1.
 *
 * Any string then passed to __trace2com(...) is sent out over the COM port.
 * (Even after the CPU has crashed and gone down in flames ;)
 *
 * Not for BCC32 since it doesn't have native I/O instructions (only
 * via the macros in IOPORT.H)
 */
#ifndef _w32_RS232_H
#define _w32_RS232_H

#if defined(USE_RS232_DBG)
  #define SIO_TRACE(p)  __trace2com p
#else
  #define SIO_TRACE(p)  ((void)0)
#endif
#endif

