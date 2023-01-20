/*
 * TCPPORT - make tcp connections from virtual serial ports
 *
 * Copyright (C) 1989, 1990, 1991, University of Waterloo
 * Portions Copyright (C) 1991, Trustees of Columbia University
 *    in the City of New York.  Permission is granted to any
 *    individual or institution to use, copy, or redistribute
 *    this software as long as it is not sold for profit, provided
 *    this copyright notice is retained.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but without any warranty; without even the implied warranty of
 *   merchantability or fitness for a particular purpose.
 *
 * Authors:
 * Erick Engelke (erick@development.watstar.uwaterloo.ca),
 *      Engineering Computing, University of Waterloo.
 * Bruce Campbell (bruce@development.watstar.uwaterloo.ca),
 *      Engineering Computing, University of Waterloo
 * Frank da Cruz (fdc@columbia.edu, FDCCU@CUVMA.BITNET),
 *      Columbia University Center for Computing Activities.
 *
 *   1.00 - May 13, 1991 : E. Engelke - stole negotiations from Frank's
 *                       & F. Da Cruz   telnet portion of C-KERMIT
 *   0.04 - May  7, 1991 : E. Engelke - got echo/no echo working
 *   0.03 - Apr 24, 1991 : E. Engelke - hacked terminal negotiation
 *   0.02 - Mar 24, 1991 : E. Engelke - convert \r to \n for UNIX compatibility
 *   0.01 - Feb   , 1991 : E. Engelke - converted Bruce's program to TCP
 * - 1.00 -              : B. Campbell- created original program
 *
 *  To force a particular terminal type, set the environment variable
 *  tcpterm to something like vt102, vt100, etc.
 *
 *  To decrease the size of the executable I have used outs rather than
 *  printf.  outs is used by the Waterloo TCP kernal for displaying error
 *  messages through DOS fn 2.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <dos.h>
#include <tcp.h>

#define TELCMDS
#define TELOPTS
#include <arpa/telnet.h>

#define OLD 1       /* new changes not complete */


#if !defined(__SMALL__) && !defined(__LARGE__)
#error This is a 16-bit real-mode program
#endif

#if defined(__WATCOMC__)
  #define __INLINE_FUNCTIONS__
  #include <i86.h>
  #define enable()      _enable()
  #define disable()     _disable()
  #define getvect(a)    _dos_getvect(a)
  #define setvect(a,b)  _dos_setvect(a,b)
#endif

int  tn_sttyp        (void);
int  do_transmission (void);
void do_reception    (void);
int  ttinc           (int fast);
int  sytek_int       (WORD ax);


char termtype [32];          /* from the environment, used in negotiations */
int  echo = 1;               /* default on, but hate it                    */


#define TRANSMIT_TICKS    1       /* do the transmit after this many ticks */
#define TRANSMIT_BUF_SIZE 2048    /* must be 2^n bytes                     */
#define TRANSMIT_MAX      256     /* most to transmit at a time            */

#define RECEIVE_TICKS     1
#define RECEIVE_BUF_SIZE  2048
#define STACK_SIZE        2048

BYTE *transmit_buffer;
BYTE *receive_buffer;
WORD  tran_in  = 0;
WORD  tran_out = 0;
WORD  rec_in   = 0;
WORD  rec_out  = 0;
int   transmit_clock = TRANSMIT_TICKS;
int   receive_clock  = RECEIVE_TICKS;

void interrupt far (*old14)(void);
void interrupt far (*old8) (void);

#if (OLD == 0)
char bigbuf[8192];   /* used to speed up tcp recvs */
#endif

tcp_Socket *s;

int   moved_vectors = 0;
int   sock_status   = 0;
DWORD host;
DWORD recvtimeout;

#define dmsg W32_NAMESPACE(dmsg)
extern void dmsg (char *str);

/* TCP/IP Telnet negotiation support code */

static int sgaflg = 0;                      /* telnet SGA flag */

/*
 * send_iac - send interupt character and pertanent stuff
 *          - return 0 on success
 */
int send_iac (char cmd, char opt)
{
  BYTE io_data[3];
  io_data[0] = IAC;
  io_data[1] = cmd;
  io_data[2] = opt;
  sock_fastwrite (s, io_data, sizeof(io_data));
  return (!tcp_tick(s));
}


/*
 * Initialize a telnet connection.
 * Returns -1 on error, 0 is ok
 */
int tn_ini (void)
{
  sgaflg = 0;                         /* SGA flag starts out this way. */
  if (send_iac(WILL,TELOPT_TTYPE))
     return (-1);

  if (send_iac(DO,  TELOPT_SGA))
     return (-1);

/*
 *  The ECHO negotiations are not necessary for talking to full-duplex
 *  systems, and they don't seem to do any good when sent to half-duplex
 *  ones -- they still refuse to echo, and what's worse, they get into
 *  prolonged negotiation loops.  Real telnet sends only the two above
 *  at the beginning of a connection.
 */

  if (send_iac(WONT,TELOPT_ECHO)) return (-1);  /* I won't echo.     */
  if (send_iac(DO,  TELOPT_ECHO)) return (-1);  /* Please, you echo. */
  return (0);
}

/*
 * Process in-band Telnet negotiation characters from the remote host.
 * Call with the telnet IAC character and the current duplex setting
 * (0 = remote echo, 1 = local echo).
 * Returns:
 *   0 on success
 *  -1 on failure (= internal or i/o error)
 */

#define TSBUFSIZ 41
char sb[TSBUFSIZ];                      /* Buffer for subnegotiations */

int tn_doop (int c)
{
  int x, y, n, flag;

  x = ttinc(0) & 0xFF;                /* Read command character */

  switch (x)
  {
    case TELOPT_ECHO:                 /* ECHO negotiation. */
      if (c == WILL)                  /* Host says it will echo. */
      {
        if (echo)                     /* Only reply if change required */
        {
          echo = 0;
          if (send_iac(DO,x))         /* Please do. */
             return (-1);
        }
        return (0);
      }

      if (c == WONT)                  /* Host says it won't echo. */
      {
        if (!echo)                    /* If I'm not echoing already */
        {
          if (send_iac(DONT,x))       /* agree to echo. */
             return (-1);
          echo = 1;
        }
        return (0);
      }

      if (c == DO)                    /* Host wants me to echo */
      {
        if (send_iac(WONT,x))         /* I say I won't, */
           return (-1);
        if (send_iac(DO,x))           /* and ask the host to echo. */
           return (-1);
        echo = 0;
        return (0);
      }

      if (c == DONT)                  /* Host wants me not to echo */
      {
        if (send_iac(WONT,x))         /* I say I won't. */
           return (-1);
        echo = 0;
        return (0);
      }
      return (0);

    case TELOPT_SGA:                  /* Suppress Go-Ahead */
      if (c == WONT)                  /* Host says it won't. */
      {
        sgaflg = 1;                   /* Remember. */
        if (!echo)                    /* If we're not echoing, */
        {
          if (send_iac(DONT,x))       /* acknowledge, */
             return (-1);
          echo = 1;                   /* and switch to local echo. */
        }
      }

      if (c == WILL)                  /* Host says it will. */
      {
        sgaflg = 0;                   /* Remember. */
        if (echo)                     /* If I'm echoing now, */
        {
          if (send_iac(DO,x))         /* this is a change, so ACK. */
             return (-1);
          if (send_iac(DO,TELOPT_ECHO)) /* Request remote echo */
             return (-1);
        }
      }
      return (0);

    case TELOPT_TTYPE:                /* Terminal Type */
      switch (c)
      {
        case DO:                      /* DO terminal type. */
          if (send_iac(WILL,x))       /* Say I'll send it if asked. */
             return (-1);
          return (0);

        /* enter subnegociations */
        case SB:
          n = flag = 0;               /* Flag for when done reading SB */
          while (n < TSBUFSIZ)        /* Loop looking for IAC SE */
          {
            if ((y = ttinc(0)) < 0)
               return (-1);
            y &= 0xff;                /* Make sure it's just 8 bits. */
            sb[n++] = y;              /* Save what we got in buffer. */

            if (y == IAC)             /* If this is an IAC */
               flag = 1;              /* set the flag. */
            else                      /* Otherwise, */
            {
              if (flag && y == SE)    /* if this is SE which immediately */
                 break;               /* follows IAC, we're done. */
              flag = 0;               /* Otherwise turn off flag. */
            }
          }
          if (!flag)
             return (-1);       /* Make sure we got a valid SB */

          if (*sb == 1 && tn_sttyp())
             return (-1);

        default:                      /* Others, ignore */
          return (0);
      }

    default:                          /* All others: refuse */
      switch (c)
      {
        case WILL:                    /* You will? */
          if (send_iac(DONT,x))       /* Please don't. */
             return (-1);
          break;
        case DO:                      /* You want me to? */
          if (send_iac(WONT,x))       /* I won't. */
             return (-1);
          if (send_iac(DONT,x))       /* Don't you either. */
             return (-1);
          break;
        case DONT:                    /* (fall thru...) */
          if (send_iac(WONT,x))       /* I won't. */
             return (-1);
        case WONT:                    /* You won't? */
          break;                      /* Good. */
      }
      return (0);
  }
}


/*
 * serial port portion
 *
 */


/* Telnet send terminal type */
/* Returns -1 on error, 0 on success */

int tn_sttyp (void)
{                            /* Send telnet terminal type. */
  char *ttn;
  int   ttl;                 /* Name & length of terminal type. */

  ttn = termtype;            /* we already got this from environment */
  if ((*ttn == 0) || ((ttl = strlen(ttn)) >= TSBUFSIZ))
  {
    ttn = "UNKNOWN";
    ttl = 7;
  }
  ttn = strcpy (&sb[1], ttn);         /* Copy to subnegotiation buffer */
  ttn = strchr (strupr(ttn), 0);

  *sb    = 0;                         /* 'is'... */
  *ttn++ = IAC;
  *ttn   = SE;

  if (send_iac(SB,TELOPT_TTYPE))      /* Send: Terminal Type */
     return (-1);

  sock_fastwrite (s, sb, ttl+3);
  return (0);
}


/*
 * ttinc   - destructively read a character from our buffer
 *         - if fast = 0, never times out
 */
int ttinc (int fast)
{
  char ch;

  /* organized to reduce number of set_timeouts when data waiting
   */
  while (rec_in == rec_out)
  {
    if (!tcp_tick(s))
    {
      sock_status = 0;
      s = NULL;
      return (-1);
    }

    /* do processing */
    kbhit();
    do_transmission();
    do_reception();

    if (fast)
       break;
  }
  ch = receive_buffer [rec_out];
  if (++rec_out >= RECEIVE_BUF_SIZE)
     rec_out = 0;
  return ((WORD)ch & 0x00FF);
}


#ifdef __WATCOMC__
  #pragma off(check_stack)

  extern void stackset (void far *stack);
  #pragma aux stackset = \
          "mov  ax, ss"  \
          "mov  bx, sp"  \
          "mov  ss, dx"  \
          "mov  sp, si"  \
          "push ax"      \
          "push bx"      \
          __parm [__dx __si] \
          __modify [__ax __bx];

  extern void stackrestore (void);
  #pragma aux stackrestore = \
          "pop bx"     \
          "pop ax"     \
          "mov ss, ax" \
          "mov sp, bx" \
          __modify [__ax __bx];

  void interrupt ourhandler (union INTPACK r)
  {
    static WORD stack [STACK_SIZE];

    stackset (&stack[STACK_SIZE-1]);
    r.w.ax = sytek_int (r.w.ax);
    stackrestore();
  }
#endif /* __WATCOMC__ */


#ifdef __TURBOC__
  #pragma argsused
  #pragma option -N

  void interrupt ourhandler (bp,di,si,ds,es,dx,cx,bx,ax,ip,cs,flgs)
  {
    static WORD old_sp, old_ss;
    static WORD stack [STACK_SIZE];
    static struct {
           WORD bp,di,si,ds,es,dx,cx,bx,ax,ip,cs,flgs;
         } far *stkptr;

    (int*)stkptr = &bp;
    old_ss = _SS;
    old_sp = _SP;
    _SS = FP_SEG (stack);
    _SP = FP_OFF (&stack[STACK_SIZE-1]);
    stkptr->ax = sytek_int (stkptr->ax);
    _SS = old_ss;
    _SP = old_sp;
  }
#endif /* __TURBOC__ */


void stuff_tx_char (char ch)
{
  transmit_buffer [tran_in] = ch;
  tran_in = (tran_in + 1) % TRANSMIT_BUF_SIZE;
}

void stuff_rx_char (char ch)
{
  receive_buffer [rec_in] = ch;
  rec_in = (rec_in + 1) % RECEIVE_BUF_SIZE;
}

int sytek_int (WORD ax)
{
  BYTE ah, ch;
  WORD status;

  if (!s)
     return (0x1000);   /* timeout */

  if (chk_timeout(recvtimeout))
  {
    do_reception();
    recvtimeout = set_timeout (10);
  }

  ah = ax >> 8;

  if (ah == 1)                        /* send char in AL */
  {
    ch = ax & 0x0FF;
    if (((tran_in + 1) % TRANSMIT_BUF_SIZE) == tran_out)
    {
      dmsg ("tx buf full?");
      status = 0x8000 | ch;
    }
    else
    {
      if ((ch == '\r') && echo)
           stuff_tx_char ('\n');
      else stuff_tx_char (ch);

      status = 0x6000 | ch;

      /* local echoing if requested */
      if (echo)
      {
        stuff_rx_char (ch);
        if (ch == '\r')
           stuff_rx_char ('\n');
      }
    }
  }

  else if (ah == 2)              /* receive char into AL */
  {
    do
    {
      ch = 0;
      if (rec_in == rec_out)     /* RX queue empty */
         status = 0x8000;
      else
      {
        status = ch = (ttinc(0) & 0xFF);
        if (ch == IAC)
        {
          /* process this stuff */
          ch = ttinc (0);
          if (ch == IAC)
               ch = 0;      /* let it pass through */
          else tn_doop (ch);
        }
      }
    }
    while (ch == IAC);
    /* status = 0x0800; timeout */
  }

  else if (ah == 3)                 /* get status */
  {
    if (rec_in == rec_out)
         status = 0x6010;
    else status = 0x6110;

  }

  else if (ah == 0)              /* init port */
  {
    status = 0x06010;
  }
  else
  {
    status = ax;
    dmsg ("command err?");
  }

  /* here we do the io */
  if (transmit_clock <= 0)
  {
    do_transmission();
    disable();
    transmit_clock = TRANSMIT_TICKS;
    enable();
  }

  return (status);
}

void interrupt tcpport_tick (void)
{
  (*old8)();
  if (transmit_clock) transmit_clock--;
  if (receive_clock)  receive_clock--;
}

void do_reception (void)
{
  int status;
  WORD maxtransfer;

  sock_tick (s,&status);

#if OLD
  if (sock_dataready(s))
  {
    if (rec_out > rec_in)
    {
      /* we can fill intermediate portion of buffer */
      maxtransfer = rec_out - rec_in;
    }
    else
    {
      /* we fill end of buffer and leave start for next attempt */
      maxtransfer = RECEIVE_BUF_SIZE - rec_in;
    }
    if (maxtransfer)
    {
      rec_in += sock_fastread (s, receive_buffer+rec_in, maxtransfer);
      if (rec_in >= RECEIVE_BUF_SIZE)
          rec_in -= RECEIVE_BUF_SIZE;
    }
  }
#else
  maxtransfer = RECEIVE_BUF_SIZE - rec_in;
  if (rec_out > rec_in)
     maxtransfer = rec_out - rec_in;
  maxtransfer = sock_recv (s, receive_buffer+rec_in, maxtransfer);
  if (maxtransfer)
     rec_in = (rec_in + maxtransfer) % RECEIVE_BUF_SIZE;
#endif
  return;

sock_err:
  switch (status)
  {
    case 1:  dmsg ("\7Host closed connection?\r\n");
             break;
    case -1: dmsg ("\7Host reset connection?\r\n");
             break;
  }
  s = NULL;
  sock_status = 0;
}

int do_transmission (void)
{
  WORD send_chars;

  if (tran_in == tran_out)
     return (0);

  if (tran_in > tran_out)
       send_chars = tran_in - tran_out;
  else send_chars = TRANSMIT_BUF_SIZE - tran_out;

  if (send_chars > TRANSMIT_MAX)
      send_chars = TRANSMIT_MAX;

  sock_flushnext (s);
  send_chars = sock_fastwrite (s, transmit_buffer+tran_out, send_chars);

  /* this only changes it by the number of bytes we have emptied out */
  tran_out = (tran_out + send_chars) % TRANSMIT_BUF_SIZE;

  return (0);
}


int main (int argc, char **argv)
{
  int   status = 0;
  char *temp;

  if (argc < 4)
  {
    outsnl ("TCPPORT host port program options..");
    return (1);
  }

  transmit_buffer = calloc (TRANSMIT_BUF_SIZE, 1);
  receive_buffer  = calloc (RECEIVE_BUF_SIZE, 1);
  s               = calloc (sizeof(*s), 1);

  if (!transmit_buffer || !receive_buffer || !s)
  {
    outsnl ("Not enough memory");
    return (1);
  }

  sock_init();

  if ((host = resolve(argv[1])) == 0)
  {
    outsnl ("Unknown host");
    return (1);
  }

  if ((temp = getenv("TCPTERM")) != NULL)
  {
    /* deal with strncpy limitation
     */
    memcpy (&termtype, &temp, sizeof(termtype));
    termtype [sizeof(termtype)-1] = 0;
    outs ("TERMINAL EMULATION: ");
    outsnl (termtype);
  }
  else
    strcpy (termtype,"UNKNOWN");

  if (host == my_ip_addr)
  {
    outsnl ("Incomming sessions not supported...");
    sock_wait_established (s, 0, NULL, &status);
    return (-3);
  }

  if (!tcp_open(s,0,host,atoi(argv[2]),NULL))
  {
#if (OLD == 0)
    sock_recv_init (s, bigbuf, sizeof(bigbuf));
#endif
    outsnl ("Unable to open");
    return (1);
  }

  sock_wait_established (s, sock_delay, NULL, &status);
  sock_mode (s, TCP_MODE_NAGLE);
  sock_status = 1;            /* allow interrupts */

  /* move vectors */
  moved_vectors = 1;
  old8  = getvect (0x08);
  old14 = getvect (0x14);

  setvect (0x08,tcpport_tick);
  setvect (0x14,ourhandler);
  recvtimeout = set_timeout (50);

  outs ("Running...");
  outsnl (argv[3]);
  system (argv[argc-1]);

  outsnl ("Done, now closing session");

  setvect (0x14, old14);
  setvect (0x08, old8);
  moved_vectors = 0;

  if (s)
  {
    sock_close (s);
    sock_wait_closed (s, sock_delay, NULL, &status);
  }

sock_err:
  switch (status)
  {
    case 1 : outsnl ("Done.");
             break;
    case -1: outsnl ("Remote host reset connection.");
             break;
  }
  if (moved_vectors)
  {
    setvect (0x14, old14);
    setvect (0x08, old8);
  }
  return (status ? 2 : 0);
}

