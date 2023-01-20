#ifndef __XMODEM_H
#define __XMODEM_H

/*
 * Xmodem Transfer Errors
 */
enum xmodem_errors {
  XERR_OK,                /* No errors */

  XERR_XMIT_FUNC,         /* NULL Transmit function pointer */
  XERR_RCVR_FUNC,         /* NULL Receive function pointer */

  XERR_RCVR_CANCEL,       /* Receiver cancelled the transfer */
  XERR_SEND_CANCEL,       /* Sender cancelled the transfer */
  XERR_USER_CANCEL,       /* User cancelled the transfer */

  XERR_FILE_READ,         /* Error reading the file */
  XERR_FILE_WRITE,        /* Error writing the file */

  XERR_ACK_TIMEOUT,       /* Timed out waiting for data pack ACK */
  XERR_NAK_TIMEOUT,       /* Timed out waiting for initial NAK */
  XERR_SOH_TIMEOUT,       /* Timed out waiting for SOH */
  XERR_DATA_TIMEOUT,      /* Timed out waiting for data */
  XERR_LAST_ACK_TIMEOUT,  /* Timed out waiting for final ACK */

  XERR_INVALID_SOH,       /* Invalid char waiting for SOH */
  XERR_INVALID_BLOCK_NUM, /* Block mismatch in packet header */
  XERR_INVALID_CRC,       /* CRC is incorrect */
  XERR_INVALID_CHECKSUM,  /* Checksum is incorrect */
  XERR_BLOCK_SEQUENCE,    /* Block out of sequence */

  XERR_CHAR_ERROR,        /* Received character error */
  XERR_OFFLINE,           /* Modem not online */

  XERR_BLOCK_NAK,         /* NAK received */
};


/*
 * Number of attempts and timeout for each attempt
 * to transferr using CRC.
 */
#define CRC_RETRY_COUNT		4
#define CRC_TIMEOUT             3L

/*
 * Number of attempts and timeout for each attempt
 * to Negative Acknowledge a bad packet.  This also
 * includes the initial NAK to get the thing started.
 */
#define NAK_RETRY_COUNT		10
#define NAK_TIMEOUT             10L

/*
 * Number of attempts and timeout for each attempt
 * to Acknowledge a good packet.
 */
#define ACK_RETRY_COUNT		10
#define ACK_TIMEOUT             10L

/*
 * Number of consecutive CANs and the timeout between them.
 */
#define CAN_COUNT_ABORT		2
#define CAN_TIMEOUT             2L

/*
 * Timeout between packet data bytes.
 */
#define DATA_TIMEOUT            1L

/*
 * Number of false start characters that the sender
 * can receive before aborting the transmission.
 */
#define START_XMIT_RETRY_COUNT	10
#define START_XMIT_TIMEOUT      60L

/*
 * Maximum number of attempts to send a single block.
 */
#define BLOCK_RETRY_COUNT       10

/*
 * Timeout for block responses.
 */
#define BLOCK_RESP_TIMEOUT      10L

/*
 * Macro to purge all characters from the receive
 * buffer.  This waits until there are no characters
 * received for a 1 second period.
 */
#define PURGE_RECEIVER(c) while ((*(c)) (1000, NULL) != RECV_TIMEOUT)


/*
 * Block sizes for normal and for 1K XMODEM transfers.
 */
#define XMODEM_BLOCK_SIZE	128
#define XMODEM_1K_BLOCK_SIZE	1024

/*
 * Values returned by the receive character serial
 * interface function.
 */
#define RECV_TIMEOUT	-1 /* receiver timed-out */

/*
 * Error bits stored in the receiv character error flag.
 */
#define RE_OVERRUN_	0x01	/* receiver overrun error */
#define RE_PARITY_	0x02	/* receiver parity error */
#define RE_FRAMING_	0x04	/* receiver framing error */

/*
 * Values returned by the transmit character serial
 * interface function.
 */
#define XMIT_OK          0      /* character enqued for transmission */
#define XMIT_OFFLINE	-1	/* modem offline */

/*
 *   ASCII values used by XMODEM
 */
#define SOH             1
#define STX             2
#define EOT             4
#define ACK             6
#define BACKSPACE       8
#define NAK             21
#define CAN             24

#define CPMEOF          26

/*
 * XMODEM block description structure.  This is only
 * used internally by the send and receive routines
 * and should not be visible outside of these.
 */
typedef struct xmodem_block_st {
        long total_block_count;       /* total blocks transferred */
        long total_byte_count;        /* total bytes transferred */

        BYTE start_char;              /* block starting character */
        BYTE block_num;               /* transmission block number */
        BYTE not_block_num;           /* one's complement block number */
        char buffer [XMODEM_1K_BLOCK_SIZE+1];  /* data buffer */
        int  buflen;                  /* buffer length (128 or 1024) */
        BYTE checksum;                /* data checksum */
        WORD crc;                     /* data CRC-16 */

        WORD crc_used : 1;            /* 0=Checksum 1=CRC-16 */
      } xblock;

typedef struct xmodem_func_st {
        int (*transmit) (char);           /* xmit function */
        int (*receive)  (long,            /* recv function */
                         WORD *);
        void (*dispstat)(long,            /* display function */
                         long,
                         const char *);
        int (*check_abort) (void);        /* manual abort function */
      } xfunc;

int xmodem_recv (FILE *f,
                 int (*transmit)(char),
                 int (*receive)(long, WORD *),
                 void (*dispstat)(long, long, const char *),
                 int (*check_abort)(void));

int xmodem_send (int block_size,
                 FILE *f,
                 int (*transmit)(char),
                 int (*receive)(long, WORD *),
                 void (*dispstat)(long, long, const char *),
                 int (*check_abort)(void));

WORD xm_update_CRC    (WORD crc, WORD c);
void xm_no_disp_func  (long a, long b, const char *buf);
int  xm_no_abort_func (void);
int  xm_perror        (int error, xfunc *xmf);
void xm_send_cancel   (int (*transmit)(char));

#endif /* __XMODEM_H */
