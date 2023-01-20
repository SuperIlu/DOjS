/*
 *  XMODEM.C
 *
 *  Author    Date       Description
 *  -------------------------------------------
 *  Jon Ward  22 Apr 90  Initial Revision.
 *  Jon Ward  23 Apr 90  Cleanup and modify for
 *                         XMODEM-1K and XMODEM-CRC.
 *  Jon Ward  26 Apr 90  Corrected implementation
 *                         of XMODEM-CRC.
 *  Jon Ward  7 Jun 90   Added more comments and a
 *                         little cleanup.
 */

#include "telnet.h"
#include "xmodem.h"

struct send_n_wait_st {
       char  char_to_send;
       int   retry_count;
       long  ms_timeout;
       BYTE *valid_responses;
       int   num_valid_responses;
     };

STATIC BYTE soh_stx_can[]    =  { SOH, STX, CAN };
STATIC BYTE soh_stx_can_eot[] = { SOH, STX, CAN, EOT };

STATIC struct send_n_wait_st crc_req = {
              'C', CRC_RETRY_COUNT, CRC_TIMEOUT,
              soh_stx_can, sizeof (soh_stx_can)
            };

STATIC struct send_n_wait_st checksum_req = {
              NAK, NAK_RETRY_COUNT, NAK_TIMEOUT,
              soh_stx_can, sizeof (soh_stx_can)
            };

STATIC struct send_n_wait_st pack_nak = {
              NAK, NAK_RETRY_COUNT, NAK_TIMEOUT,
              soh_stx_can_eot, sizeof (soh_stx_can_eot)
            };

STATIC struct send_n_wait_st pack_ack = {
              ACK, ACK_RETRY_COUNT, ACK_TIMEOUT,
              soh_stx_can_eot, sizeof (soh_stx_can_eot)
            };

/*
 * Error messages for error enums.
 */
STATIC char *xmodem_errors [] = {
            "Transmission Successful",
            "NULL Transmit function pointer",
            "NULL Receive function pointer",
            "Receiver cancelled the transfer",
            "Sender cancelled the transfer",
            "User cancelled the transfer",
            "Error reading the file",
            "Error writing the file",
            "Timed out waiting for data pack ACK",
            "Timed out waiting for initial NAK",
            "Timed out waiting for SOH",
            "Timed out waiting for data",
            "Timed out waiting for final ACK",
            "Invalid char waiting for SOH",
            "Block mismatch in packet header",
            "CRC is incorrect",
            "Checksum is incorrect",
            "Block out of sequence",
            "Received character error",
            "Modem is not online",
          };

/*
 * Local Function Prototypes
 */
STATIC int xm_send_n_wait (
           const struct send_n_wait_st *req, /* request structure */
           BYTE  *response,                  /* response from sender */
           xfunc *xmf);                      /* xmodem external functions */

STATIC int xm_block_start (
           xblock *xb,             /* xmodem block data */
           BYTE    block_start,    /* block start char from sender */
           xfunc  *xmf);           /* xmodem external functions */

STATIC int xm_recv_block (
           xblock *xb,             /* xmodem block data */
           xfunc  *xmf);           /* xmodem external functions */

STATIC int xm_send_file (
           FILE   *f,              /* file pointer */
           xblock *xb,             /* pointer to block data */
           xfunc  *xmf);           /* xmodem external functions */

STATIC int xm_send_block (
           xblock *xb,             /* pointer to block data */
           xfunc  *xmf);           /* xmodem external functions */

/*
 * This function receives a file transferred via XMODEM, XMODEM-1K or
 * XMODEM/CRC.  The `f' argument represents the file to receive that has
 * been opened for writing.
 */
int xmodem_recv (
    FILE *f,                                   /* file to write to */
    int (*transmit) (char),                    /* xmit function */
    int (*receive) (long, WORD *),             /* recv function */
    void (*dispstat) (long,long,const char*),  /* display function */
    int (*check_abort) (void))                 /* manual abort function */
{
  int    error;            /* gen purpose error var */
  BYTE   start_char;       /* first char of block */
  BYTE   next_block;       /* next block we expect */
  BYTE   last_block;       /* last successful block */
  xblock xb;               /* xmodem block data */
  xfunc  xmfuncs;          /* xmodem external functions */


  /* Initialize the function pointer structure.
   */
  if ((xmfuncs.dispstat = dispstat) == NULL)
     xmfuncs.dispstat = xm_no_disp_func;

  if ((xmfuncs.check_abort = check_abort) == NULL)
     xmfuncs.check_abort = xm_no_abort_func;

  if ((xmfuncs.transmit = transmit) == NULL)
     return (xm_perror (XERR_XMIT_FUNC, &xmfuncs));

  if ((xmfuncs.receive = receive) == NULL)
     return (xm_perror (XERR_RCVR_FUNC, &xmfuncs));


  /* Initialize data for the first block and purge all data from the
   * receive buffer.  Init the number of bytes and blocks received and
   * display some useful info.
   */
  next_block = last_block = 1;

  xb.total_block_count = 0L;
  xb.total_byte_count = 0L;

  (*xmfuncs.dispstat) (0L, 0L, "");

  PURGE_RECEIVER (receive);


  /* Attempt to transfer using CRC-16 error detection.
   * This involves sending the CRC begin character: 'C'.
   */
  xb.crc_used = 1;
  error = xm_send_n_wait (&crc_req, &start_char, &xmfuncs);


  /* If the sender did not respond to the CRC-16
   * transfer request, then attempt to transfer using
   * checksum error detection.
   */
  if (error == XERR_SOH_TIMEOUT)
  {
    xb.crc_used = 0;
    error = xm_send_n_wait (&checksum_req, &start_char, &xmfuncs);
  }

  /* If begin transfer request failed, return error.
   */
  if (error != XERR_OK)
     return (error);

  /* If the starting character of the next block is an EOT, then we
   * have completed transferring the file and we exit this loop.
   * Otherwise, we init the xmodem packet structure based on the first
   * character of the packet.
   */
  while (start_char != EOT)
  {
    int good_block;		/* NZ if packet was OK */

    error = xm_block_start (&xb, start_char, &xmfuncs);
    if (error != XERR_OK)
      return (error);

    good_block = -1;		/* assume packet will be OK */


    /* Receive the packet.  If there was an error, then
     * NAK it.  Otherwise, the packet was received OK.
     */
    if (xm_recv_block (&xb, &xmfuncs) != XERR_OK)
       good_block = 0;           /* bad block */

    /* If this is the next expected packet, then append it to the file
     * and update the last and next packet vars.
     */
    else if (xb.block_num == next_block)
    {
      int bytes_written;	/* bytes written for this block */

      last_block = next_block;
      next_block = (next_block + 1) % 256;

      bytes_written = fwrite (xb.buffer, 1, xb.buflen, f);

      xb.total_block_count++;
      xb.total_byte_count += bytes_written;

      (*xmfuncs.dispstat) (xb.total_block_count,
                           xb.total_byte_count, NULL);

      if (bytes_written != xb.buflen)
      {
	xm_send_cancel (transmit);
	return (xm_perror (XERR_FILE_WRITE, &xmfuncs));
      }
    }

    /* If this is the previous packet, then the sender did not
     * receive our ACK to that packet and resent it. This is OK.
     * Just ACK the packet.
     *
     * If the block number for this packet is completely out of
     * sequence, cancel the transmission and return an error.
     */
    else if (xb.block_num != last_block)
    {
      xm_send_cancel (transmit);
      return (xm_perror (XERR_BLOCK_SEQUENCE, &xmfuncs));
    }

    /* Here, good_block is non-zero if the block was received and
     * processed with no problems.  If it was a good block, then we
     * send an ACK.  A NAK is sent for bad blocks.
     */
    if (good_block)
    {
      error = xm_send_n_wait (&pack_ack, &start_char, &xmfuncs);
    }
    else
    {
      PURGE_RECEIVER (receive);
      error = xm_send_n_wait (&pack_nak, &start_char, &xmfuncs);
    }

    if (error != XERR_OK)
       return (error);
  }

  /* The whole file has been received, so attempt to
   * send an ACK to the final EOT.
   */
  if ((*transmit)(ACK) != XMIT_OK)
     return (xm_perror (XERR_OFFLINE, &xmfuncs));

  return (xm_perror (XERR_OK, &xmfuncs));
}

/*
 * Dummy function used in case caller did not supply
 * a display function.
 */
void xm_no_disp_func (long a, long b, const char *buf)
{
  ARGSUSED (a);
  ARGSUSED (b);
  ARGSUSED (buf);
}

/*
 * Dummy function used in case caller did not supply
 * a used abort function.
 */
int xm_no_abort_func (void)
{
  return (0);
}   

/*
 * This function transmits a character and waits for a response.
 * The req argument points to a structure containing info about
 * the char to transmit, retry count, timeout, etc.  The response
 * argument points to a storage place for the received character.
 * The xmf argument points to a structure of caller supplied functions.
 *
 * Any errors encountered are returned.
 */
STATIC int xm_send_n_wait (
           const struct send_n_wait_st *req, /* request structure */
           BYTE  *response,                  /* response from sender */
           xfunc *xmf)                       /* xmodem external functions */
{
  int  j;
  WORD rerr;

  for (j = 0; j < req->retry_count; j++)
  {
    int rcvd_char;
    int i;

    /* Check to see if the user aborted the transfer.
     */
    if ((*xmf->check_abort) () != 0)
    {
      xm_send_cancel (xmf->transmit);
      return (xm_perror (XERR_USER_CANCEL, xmf));
    }

    /* Transmit the block response (or block start) character.
     */
    if ((*xmf->transmit) (req->char_to_send) != XMIT_OK)
       return (xm_perror (XERR_OFFLINE, xmf));

    /* Wait for a response.  If there isn't one or if a
     * parity or similar error occurred, continue with
     * next iteration of the retry loop.
     */
    rcvd_char = (*xmf->receive) (req->ms_timeout, &rerr);

    if (rcvd_char == RECV_TIMEOUT)
      continue;

    if (rerr != 0)
      return (xm_perror (XERR_CHAR_ERROR, xmf));

    /* Initialize the response and check to see if it is valid.
     */
    if (response)
       *response = (BYTE) rcvd_char;

    for (i = 0; i < req->num_valid_responses; i++)
      if (rcvd_char == req->valid_responses[i])
	return (XERR_OK);
  }
  return (xm_perror (XERR_SOH_TIMEOUT, xmf));
}

/*
 * This function analyzes valid block start characters to
 * determine block size or in the case of CAN whether to
 * abort the transmission.
 *
 * Any errors encountered are returned.
 */
STATIC int xm_block_start (
           xblock *xb,           /* xmodem block data */
           BYTE    block_start,  /* block start char from sender */
           xfunc  *xmf)          /* xmodem external functions */
{
  switch (block_start)
  {
    case SOH:                /* NORMAL 128-byte block */
	 xb->buflen = XMODEM_BLOCK_SIZE;
	 return (XERR_OK);

    case STX:                /* 1024-byte block */
	 xb->buflen = XMODEM_1K_BLOCK_SIZE;
	 return (XERR_OK);

    case CAN:                /* Abort signal */
	 if ((*xmf->receive) (CAN_TIMEOUT, NULL) == CAN)
	 {
	   xm_send_cancel (xmf->transmit);
	   return (xm_perror (XERR_SEND_CANCEL, xmf));
	 }
	 break;
  }
  return (xm_perror (XERR_INVALID_SOH, xmf));
}
  
/*
 * This function receives the block numbers, block data, and block
 * checksum or CRC.  The received data is stored in the structure
 * pointed to by the xb argument.  The block numbers are compared, and
 * the checksum or CRC is verified.
 *
 * Any errors encountered are returned.
 */
STATIC int xm_recv_block (
           xblock *xb,        /* xmodem block data */
           xfunc  *xmf)       /* xmodem external functions */
{
  int  i;
  WORD rerr;                  /* receive error */

  /* Attempt to receive the block number.  If the receiver times out
   * or if there is a receive error, ignore the rest of the packet.
   */
  if ((i = (*xmf->receive) (DATA_TIMEOUT, &rerr)) == RECV_TIMEOUT)
    return (xm_perror (XERR_DATA_TIMEOUT, xmf));

  if (rerr)
    return (xm_perror (XERR_CHAR_ERROR, xmf));

  xb->block_num = (BYTE)i;

  /* Attempt to receive the one's complement of the block number.
   */
  if ((i = (*xmf->receive) (DATA_TIMEOUT, &rerr)) == RECV_TIMEOUT)
    return (xm_perror (XERR_DATA_TIMEOUT, xmf));

  if (rerr)
    return (xm_perror (XERR_CHAR_ERROR, xmf));

  xb->not_block_num = (BYTE) i;

  /* Make sure that the block number and one's
   * complemented block number agree.
   */
  if ((255 - xb->block_num) != xb->not_block_num)
    return (xm_perror (XERR_INVALID_BLOCK_NUM, xmf));

  /* Clear the CRC and checksum accumulators and receive the data block.
   */
  xb->crc      = 0;
  xb->checksum = 0;

  for (i = 0; i < xb->buflen; i++)
  {
    int rx = (*xmf->receive) (DATA_TIMEOUT, &rerr);

    if (rx == RECV_TIMEOUT)
       return (xm_perror (XERR_DATA_TIMEOUT, xmf));

    if (rerr)
       return (xm_perror (XERR_CHAR_ERROR, xmf));

    xb->buffer[i] = (BYTE)rx;

    if (xb->crc_used)
         xb->crc       = xm_update_CRC (xb->crc, xb->buffer[i]);
    else xb->checksum += xb->buffer[i];
  }

  /* Validate the CRC.
   */
  if (xb->crc_used)
  {
    int rx = (*xmf->receive) (DATA_TIMEOUT, &rerr);

    if (rx == RECV_TIMEOUT)
       return (xm_perror (XERR_DATA_TIMEOUT, xmf));

    if (rerr)
       return (xm_perror (XERR_CHAR_ERROR, xmf));

    if ((BYTE)rx != (BYTE)(xb->crc >> 8))
       return (xm_perror (XERR_INVALID_CRC, xmf));

    rx = (*xmf->receive) (DATA_TIMEOUT, &rerr);
    if (rx == RECV_TIMEOUT)
       return (xm_perror (XERR_DATA_TIMEOUT, xmf));

    if (rerr)
       return (xm_perror (XERR_CHAR_ERROR, xmf));

    if ((BYTE)rx != (BYTE)(xb->crc & 0xFF))
       return (xm_perror (XERR_INVALID_CRC, xmf));
  }

  /* Validate the checksum.
   */
  else
  {
    int rx = (*xmf->receive) (DATA_TIMEOUT, &rerr);

    if (rx == RECV_TIMEOUT)
       return (xm_perror (XERR_DATA_TIMEOUT, xmf));

    if (rerr)
       return (xm_perror (XERR_CHAR_ERROR, xmf));

    if ((BYTE)rx != xb->checksum)
       return (xm_perror (XERR_INVALID_CHECKSUM, xmf));
  }
  return (XERR_OK);
}

/*
 * This function prints an XMODEM status message using the caller
 * supplied display function.  The error argument is returned.
 */
int xm_perror (int    error,   /* error number */
               xfunc *xmf)     /* xmodem external functions */
{
  xmf->dispstat (-1L, -1L, xmodem_errors[error]);
  return (error);
}

/*
 * This function sends a file via XMODEM, XMODEM-1K or XMODEM/CRC.
 * The f argument represents the file to send that has been opened
 * for reading.
 */
int xmodem_send (
    int   block_size,                        /* maximum block size */
    FILE  *f,                                /* file to write to */
    int  (*transmit)(char),                  /* xmit function */
    int  (*receive)(long, WORD*),            /* recv function */
    void (*dispstat)(long,long,const char*), /* display function */
    int  (*check_abort)(void))               /* manual abort function */
{
  xblock xb;			/* block data */
  xfunc  xmfuncs;               /* xmodem external functions */
  int    error_count;           /* counter for errors */
  int    can_count;             /* cancel counter */
  int    error;                 /* gen error var */
  WORD   rerr;                  /* received char error */

  /* Initialize the function pointer structure.
   */
  if ((xmfuncs.dispstat = dispstat) == NULL)
     xmfuncs.dispstat = xm_no_disp_func;

  if ((xmfuncs.check_abort = check_abort) == NULL)
     xmfuncs.check_abort = xm_no_abort_func;

  if ((xmfuncs.transmit = transmit) == NULL)
     return (xm_perror (XERR_XMIT_FUNC, &xmfuncs));

  if ((xmfuncs.receive = receive) == NULL)
     return (xm_perror (XERR_RCVR_FUNC, &xmfuncs));

  xb.total_block_count = 0L;
  xb.total_byte_count = 0L;

  (*xmfuncs.dispstat) (0L, 0L, "");

  PURGE_RECEIVER (receive);

  /* Purge all data from the receive buffer.  Then we wait for 1 full
   * minute.  If we receive no characters during that time, then we return
   * indicating the file was not transferred.  If we receive a NAK, then
   * we begin transmission.  If we receive an excessive number of garbage
   * characters, then we return indicating no file transfer.
   */
  for (error_count = 0, can_count = 0; 1;)
  {
    int rcvd_char;

    if (error_count >= START_XMIT_RETRY_COUNT)
      return (xm_perror (XERR_NAK_TIMEOUT, &xmfuncs));

    /* try for 1 minute
     */
    rcvd_char = (*xmfuncs.receive) (START_XMIT_TIMEOUT, &rerr);
    if (rerr)
    {
      xm_send_cancel (xmfuncs.transmit);
      return (xm_perror (XERR_CHAR_ERROR, &xmfuncs));
    }

    switch (rcvd_char)
    {
      case RECV_TIMEOUT:
	   xm_send_cancel (xmfuncs.transmit);
	   return (xm_perror (XERR_NAK_TIMEOUT, &xmfuncs));

      case 'C':
	   xb.crc_used = 1;
	   break;

      case NAK:
	   xb.crc_used = 0;
	   break;

      case CAN:
	   if (++can_count >= CAN_COUNT_ABORT)
	   {
	     xm_send_cancel (xmfuncs.transmit);
	     return (xm_perror (XERR_RCVR_CANCEL, &xmfuncs));
	   }
	   continue;

      default:
	   can_count = 0;
	   error_count++;
	   continue;
    }
    break;
  }

  /* Setup the block size and the start of block
   * character and send the file.
   */
  if (block_size == XMODEM_1K_BLOCK_SIZE)
  {
    xb.buflen = XMODEM_1K_BLOCK_SIZE;
    xb.start_char = STX;
  }
  else
  {
    xb.buflen = XMODEM_BLOCK_SIZE;
    xb.start_char = SOH;
  }

  if ((error = xm_send_file (f, &xb, &xmfuncs)) != XERR_OK)
     return (error);

  /* Now, we send an EOT to the receiver and we expect to get an
   * ACK within 1 minute.  If we don't then we timeout and report
   * an error.  If we get any character other than an ACK, we
   * retransmit the EOT.  If we get an ACK, then we exit with
   * hopeful success.
   */
  while (1)
  {
    if ((*xmfuncs.transmit) (EOT) != XMIT_OK)
       return (xm_perror (XERR_OFFLINE, &xmfuncs));

    switch ((*receive) (START_XMIT_TIMEOUT, &rerr))
    {
      case RECV_TIMEOUT:
	   return (xm_perror (XERR_LAST_ACK_TIMEOUT, &xmfuncs));

      case ACK:
           if (rerr)
	     continue;
           break;

      default:
	   continue;
    }
    break;
  }
  return (xm_perror (XERR_OK, &xmfuncs));
}
    
/*
 * This function transmits the file associated with the file pointer
 * f according to the requirements of the XMODEM transfer protocol.
 */
STATIC int xm_send_file (FILE   *f,    /* file pointer */
                         xblock *xb,   /* pointer to block data */
                         xfunc  *xmf)  /* xmodem external functions */
{
  int done;

  /* Repeat until we have sent the whole file.
   */
  for (done = 0, xb->block_num = 1; !done; xb->block_num++)
  {
    size_t bytes_read;
    int    error;

    /* Read a block of data.  If it was not a full block, then
     * check for a file error.  If it was a file error, cancel the
     * transfer and return an error.  Otherwise, pad the remainder
     * of the block with CPM EOFs.
     */
    bytes_read = fread (xb->buffer, 1, xb->buflen, f);

    if (bytes_read != xb->buflen)
    {
      int i;

      if (ferror(f))
      {
	xm_send_cancel (xmf->transmit);
	return (xm_perror (XERR_FILE_READ, xmf));
      }
      for (i = bytes_read; i < xb->buflen; i++)
          xb->buffer[i] = CPMEOF;
      done = -1;
    }

    /* Calculate the one's complement block number and send the block.
     */
    xb->not_block_num = (BYTE) (255 - xb->block_num);

    if ((error = xm_send_block (xb, xmf)) != XERR_OK)
       return (error);
  }
  return (XERR_OK);
}

/*
 * This function transmits the block described by the structure
 * pointed to by the xb argument.  The transmit and receive arguments
 * point to functions that transmit and receive data to and from the
 * modem.  A value of 0 is returned to indicate that the block was
 * successfully sent.  A non-zero return value indicates an offline
 * condition.
 */
STATIC int xm_send_block (xblock *xb,   /* pointer to block data */
                          xfunc  *xmf)  /* xmodem external functions */
{
  int error_count = 0;

  while (1)
  {
    int  i;
    int  can_count;        /* number of CANS received */
    int  block_resp;       /* response to block */
    WORD rerr;             /* received char error */

    /* If there were too many errors, then exit with error.
     */
    if (error_count > BLOCK_RETRY_COUNT)
    {
      xm_send_cancel (xmf->transmit);
      return (xm_perror (XERR_ACK_TIMEOUT, xmf));
    }

    /* Check for user abort and process it if applicable.
     */
    if ((*xmf->check_abort) () != 0)
    {
      xm_send_cancel (xmf->transmit);
      return (xm_perror (XERR_USER_CANCEL, xmf));
    }

    /* Transmit the block start character (it's different depending
     * on whether we're sending 1024-BYTE or 128-BYTE packets.
     *
     * Send the block number.
     *
     * Send the one's complement of the block number.
     */
    if ((*xmf->transmit) (xb->start_char) != XMIT_OK)
       return (xm_perror (XERR_OFFLINE, xmf));

    if ((*xmf->transmit) (xb->block_num) != XMIT_OK)
       return (xm_perror (XERR_OFFLINE, xmf));

    if ((*xmf->transmit) (xb->not_block_num) != XMIT_OK)
       return (xm_perror (XERR_OFFLINE, xmf));

    /* Clear the CRC and checksum and send the data
     * block while building the CRC or checksum.
     */
    xb->crc = 0;
    xb->checksum = 0;

    for (i = 0; i < xb->buflen; i++)
    {
      if ((*xmf->transmit) (xb->buffer[i]) != XMIT_OK)
         return (xm_perror (XERR_OFFLINE, xmf));

      if (xb->crc_used != 0)
           xb->crc = xm_update_CRC (xb->crc, xb->buffer[i]);
      else xb->checksum += xb->buffer[i];
    }

    /* Send the CRC or checksum.  If we send the CRC,
     * we must send the High BYTE first.
     */
    if (xb->crc_used == 0)
    {
      if ((*xmf->transmit) (xb->checksum) != XMIT_OK)
         return (xm_perror (XERR_OFFLINE, xmf));
    }
    else
    {
      if ((*xmf->transmit) ((BYTE) (xb->crc >> 8)) != XMIT_OK)
         return (xm_perror (XERR_OFFLINE, xmf));

      if ((*xmf->transmit) ((BYTE) (xb->crc & 0xFF)) != XMIT_OK)
         return (xm_perror (XERR_OFFLINE, xmf));
    }

    /* Wait for the receiver to respond with an ACK or
     * a NAK.  If we timeout waiting, then return an
     * error code.  If we get a NAK, retransmit the
     * block.  If we gat an ACK, then return with
     * status OK.  If we receive 2 consecutive CANs,
     * then we abort the transmission.
     */
    can_count = 0;

  GET_BLOCK_RESPONSE:
    block_resp = (*xmf->receive) (BLOCK_RESP_TIMEOUT, &rerr);
    if (rerr)
    {
      xm_perror (XERR_CHAR_ERROR, xmf);
      goto GET_BLOCK_RESPONSE;
    }

    switch (block_resp)
    {
      case ACK:
	   error_count = 0;
	   xb->total_block_count++;
	   xb->total_byte_count += xb->buflen;

	   (*xmf->dispstat) (xb->total_block_count,
			     xb->total_byte_count,
			     NULL);
	   break;

      case CAN:
	   if (++can_count >= CAN_COUNT_ABORT)
	   {
	     xm_send_cancel (xmf->transmit);
	     return (xm_perror (XERR_RCVR_CANCEL, xmf));
	   }
	   goto GET_BLOCK_RESPONSE;


          /* I have seen 2 ways of handling this problem. One
           * is to return an error indicating that the sender
           * never received a packet ACK.  Another method
           * retransmits the packet just sent in hopes that
           * the receiver will get its ACK together.  The
           * second one is what I did.
           */
      case RECV_TIMEOUT:
	   error_count++;
	   xm_perror (XERR_ACK_TIMEOUT, xmf);
	   continue;

      case NAK:
	   xm_perror (XERR_BLOCK_NAK, xmf);
	   error_count++;
	   continue;

      default:
	   goto GET_BLOCK_RESPONSE;
    }
    break;
  } 
  return (XERR_OK);
}

/*
 * This function updates a CRC accumulator for xmodem
 * CCITT CRC.
 */
WORD xm_update_CRC (WORD crc,  /* current CRC */
                    WORD c)    /* character to add to CRC */
{
  static WORD crc_polynomial = 0x1021;
  int    i;

  c <<= 8;

  for (i = 0; i < 8; i++)
  {
    if ((c ^ crc) & 0x8000)
         crc = (crc << 1) ^ crc_polynomial;
    else crc <<= 1;
    c <<= 1;
  }
  return (crc);
}

/*
 * This function sends 8 CANs and 8 BSs as done by
 * YAM.  This is used to cancel an X or Y Modem send or receive.
 */
void xm_send_cancel (int (*transmit)(char))
{
  int i;

  for (i = 0; i < 8; i++)
    (*transmit) (CAN);

  for (i = 0; i < 8; i++)
    (*transmit) (BACKSPACE);
}

