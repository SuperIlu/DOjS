/*!\file rs232.c
 *
 * Simple serial (RS232) debug-dump, by GvB 2002-09.
 * Debug strings can be dumped to the set-up serial COM port.
 * I/O port works bady with Borland's bcc32. Hence not supported.
 *
 * MSDOS only.
 */

#include <stdio.h>
#include <stdarg.h>

#include "wattcp.h"
#include "language.h"
#include "ioport.h"
#include "misc.h"
#include "misc_str.h"
#include "timer.h"
#include "rs232.h"

#if defined(USE_RS232_DBG)  /* Rest of file */

/* Offsets from base address 'trace2com_base'
 */
#define TXRX_REG      0        /* Transmit/Receive Register */
#define IER_REG       1        /* Interrupt Enable Register */
#define BAUD_LSB_REG  0        /* LSB baud-rate when LCR bit 7 is set */
#define BAUD_MSB_REG  1        /* MSB baud-rate when LCR bit 7 is set */
#define IIR_REG       2        /* Interrupt Identification Register */
#define FIFO_REG      2        /* 16550 FIFO control Register */

#define LCR_REG       3        /* Line Control Register */
#define MCR_REG       4        /* Modem Control Register */
#define LSR_REG       5        /* Line Status Register */
  #define LSR_THRE    0x20     /* Tx holding reg. empty */

static DWORD trace2com_speed      = 115200UL;
static int   trace2com_base       = 0;
static int   trace2com_fifoSize_1 = 0;
static int   trace2com_stdPorts [4] = { 0x3F8, 0x2F8, 0x3E8, 0x2E8 };

/*
 * ___trace2com() - Strings passed here are dumped to the set-up serial
 * COM port. If the string is < 16 chars long, it will fit into the
 * 16550 UARTs FIFO and will not cause any significant delay.
 *
 * No interrupts are used, so this will not interfere with anything
 * else that might be happening.
 */
int MS_CDECL __trace2com (const char *fmt, ...)
{
  int     fifo_left = 0; /* Assume TX FIFO is full on first round -> force check */
  char    buf [256];
  int     len, i;
  va_list args;

  if (trace2com_base <= 0)   /* Not yet initialized */
     return (0);

  va_start (args, fmt);

#if defined(VSNPRINTF)
  len = VSNPRINTF (buf, sizeof(buf)-1, fmt, args);
  if (len < 0 || len >= SIZEOF(buf)-1)
  {
    outsnl (_LANG("ERROR: __trace2com() overrun"));
    len = sizeof(buf)-1;
    buf [len] = '\0';
  }
#else
  len = vsprintf (buf, fmt, args);
  if (len > SIZEOF(buf))  /* harm already done, but better than no test */
  {
    outsnl (_LANG("ERROR: __trace2com() overrun"));
    return (0);
  }
#endif

  for (i = 0; i < len; i++)
  {
    DWORD to = set_timeout (400000/trace2com_speed); /* 10ms at 38kB/s */

    if (--fifo_left < 0) /* Is the TX FIFO full? */
    {
      /* Wait until THRE or TX FIFO empty
       */
      while (!(_inportb (trace2com_base+LSR_REG) & LSR_THRE))
      {
        if (chk_timeout(to))
           return (i);
      }
      fifo_left = trace2com_fifoSize_1; /* Now we can fill it up again */
    }
    _outportb (trace2com_base+TXRX_REG, buf[i]);
  }

  if (--fifo_left < 0)
  {
    DWORD to = set_timeout (400000/trace2com_speed);

    /* Wait until THRE or TX FIFO empty
     */
    while (!(_inportb (trace2com_base+LSR_REG) & LSR_THRE))
    {
      if (chk_timeout(to))
         return (len);
    }
  }
  _outportb (trace2com_base+TXRX_REG, '\r');
  _outportb (trace2com_base+TXRX_REG, '\n');
  return (len+2);
}

/*
 * trace2com_init() - Public initialisation
 */
int trace2com_init (WORD port_addr, DWORD baud_rate)
{
  BYTE  Lsb, Msb;
  WORD  base;
  DWORD div;

  /* Check/get UART address
   */
  if (port_addr < 1 || port_addr > 4)
     return (0);

  base = trace2com_stdPorts [port_addr-1];

  /* See if the chip is actually there and ready
   */
  if (_inportb (base+IER_REG) == 0xFF)    /* Nothing here */
     return (0);

  if ((_inportb (base+IER_REG) & 0x0F) != 0x00)
     return (0);    /* UART is already in use by another program */

  _outportb (base+IER_REG, 0);  /* disable all interrupts */
  if (_inportb (base+IER_REG) != 0)
     return (0);                /* Whatever is here is not an UART */

  /* Set up UARTs registers
   */
  div = 115200UL / baud_rate;
  Msb = div >> 8;
  Lsb = (div << 8) >> 8;

  _outportb (base+LCR_REG, 0x80);       /* Turn address latch on */
  _outportb (base+BAUD_LSB_REG, Lsb);
  _outportb (base+BAUD_MSB_REG, Msb);
  _outportb (base+LCR_REG, 0x00);       /* Turn address latch off (again) */

  _outportb (base+IER_REG, 0x00);       /* No interrupts, we use polling */
  _outportb (base+FIFO_REG, 0x01|0x08); /* Activate FIFO @ MODE 2 */
  _outportb (base+LCR_REG , 0x03);      /* 8, N, 1 */
  _outportb (base+MCR_REG, 0x03);       /* DTR + DTS on */

  /* Check chip type, gives us the FIFO size
   */
  _outportb (base + FIFO_REG, 0x11);
  Lsb = _inportb (base + FIFO_REG);

  /* If we have a 16750 or 16550 the FIFO size is 16, otherwise we assume 1
   */
  trace2com_fifoSize_1 = ((Lsb & 0x20) != 0x20 ||
                          (Lsb & 0xC0) == 0xC0) ? 15 : 0;

  /* We are now officially open for business
   */
  trace2com_speed = baud_rate;
  trace2com_base  = base;

  SIO_TRACE ((_LANG("Watt-32 COM%d trace started"), port_addr));
  return (1);
}
#endif  /* USE_RS232_DBG */

