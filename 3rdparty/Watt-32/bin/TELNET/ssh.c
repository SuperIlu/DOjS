/*
 * $Date: 2001/04/02 13:27:30 $
 * $Revision: 1.5 $
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 * Modified from DOS-SSH files (protocol.c, negotiat.c and sshdos.c)
 */

#include "telnet.h"
#include "config.h"
#include "screen.h"
#include "ssh.h"

struct Packet {
       long   length;
       DWORD  type;
       DWORD  crc;
       BYTE  *data;
       BYTE  *body;
       long   maxlen;
     };

static void  ssh_gotdata (BYTE *data);
static int   packet_read_block (void);
static void  packet_read_expect (DWORD type);
static void  write_packet (void);
static void  create_header (int type, int len);
static DWORD crc32 (const void *buf, size_t len);

static struct Packet pktin  = { 0, 0, 0, NULL, NULL, 0 };
static struct Packet pktout = { 0, 0, 0, NULL, NULL, 0 };

static int   cipher      = 0;                /* cipher on or off */
static int   cipher_type = SSH_CIPHER_3DES;  /* type of cipher */
static char  username[20];
static char  password[20];

static BlowfishContext ectx, dctx;
static DESCon          keys[3];


/* check_emulation: take the remote party's version number as
 * arguments and return our possibly modified version number back
 * (relevant only for clients).
 *
 * Return values:
 * EMULATE_VERSION_OK means we can work together
 *
 * EMULATE_VERSION_TOO_OLD if the other party has too old version
 * which we cannot emulate,
 *
 * EMULATE_MAJOR_VERSION_MISMATCH if the other party has different
 * major version and thus will probably not understand anything we
 * say, and
 *
 * EMULATE_VERSION_NEWER if the other party has never code than we
 * have.
 *
 */
static int check_emulation (int remote_major, int remote_minor,
                            int *return_major, int *return_minor)
{
  if (return_major)
     *return_major = PROTOCOL_MAJOR;
  if (return_minor)
  {
    if (remote_minor < PROTOCOL_MINOR)
         *return_minor = remote_minor;
    else *return_minor = PROTOCOL_MINOR;
  }

  if (remote_major < PROTOCOL_MAJOR)
     return (EMULATE_MAJOR_VERSION_MISMATCH);

  if (remote_major == 1 && remote_minor == 0)
     return (EMULATE_VERSION_TOO_OLD); /* We no longer support 1.0. */

  if (remote_major > PROTOCOL_MAJOR ||
      (remote_major == PROTOCOL_MAJOR && remote_minor > PROTOCOL_MINOR))
  {
    /* The remote software is newer than we. If we are the client,
     * no matter - the server will decide. If we are the server, we
     * cannot emulate a newer client and decide to stop.
     */
    return (EMULATE_VERSION_NEWER);
  }
  return (EMULATE_VERSION_OK);
}

/*
 * SSH version string exchange
 */
static void exchange_identification (void)
{
  char buf[256], remote_version[256];
  int  remote_major, remote_minor, i;
  int  my_major, my_minor;
  int  status;

  sock_wait_input (&sock, cfg.timeout, NULL, &status);

  /* Read other side's version identification.
   */
  i = sock_gets (&sock, (BYTE*)&buf, sizeof(buf));
  if (i == 0)
     fatal ("sock_gets: %s", sockerr(&sock));
  buf[i] = '\n';

  if (verbose)
     cprintf ("Remote version: %s\r", buf);

  /* Check that the versions match.  In future this might accept several
   * versions and set appropriate flags to handle them.
   */
  if (sscanf (buf, "SSH-%d.%d-%[^\n]\n", &remote_major, &remote_minor, remote_version) != 3)
     fatal ("Bad remote protocol version identification");

  switch (check_emulation (remote_major, remote_minor, &my_major, &my_minor))
  {
    case EMULATE_VERSION_TOO_OLD:
         fatal ("Remote machine has too old SSH software version");
    case EMULATE_MAJOR_VERSION_MISMATCH:
         fatal ("Major protocol versions incompatible");
    case EMULATE_VERSION_NEWER:
         /* We will emulate the old version. */
         break;
    case EMULATE_VERSION_OK:
         break;
    default:
         fatal ("Unexpected return value from check_emulation");
  }

  sprintf (buf, "SSH-%d.%d-%s\n", my_major, my_minor, SSH_VERSION);
  if (verbose)
     cprintf ("Local version: %s\r", buf);
  sock_write (&sock, (const BYTE*)&buf, strlen(buf));
  return;

sock_err:
  fatal ("sock_wait_input: %s", sockerr(&sock));
}

/*
 * create session key and ID
 */
static void create_ssh_keys (void)
{
  size_t i, len;
  BYTE   session_key[32];
  BYTE   session_id[16];
  BYTE   cookie[8];
  BYTE  *RSAblock, *keystr1, *keystr2;

  R_RSAKey   servkey, hostkey;
  MD5Context md5c;

  memcpy (cookie, pktin.body, sizeof(cookie));
  if (verbose)
     cputs ("Extracting server keys...");

  i = RSAmakekey (pktin.body+8, &servkey, &keystr1);
  RSAmakekey (pktin.body+8+i, &hostkey, &keystr2);
  if (verbose)
     cputs ("Done\n\r");

  if (verbose)
     cputs ("Generating session ID...");

  MD5Init (&md5c);
  MD5Update (&md5c, keystr2, hostkey.bytes);
  MD5Update (&md5c, keystr1, servkey.bytes);
  MD5Update (&md5c, cookie, 8);
  MD5Final (session_id, &md5c);
  if (verbose)
     cputs ("Done\n\r");

  for (i = 0; i < sizeof(session_key); i++)
      session_key[i] = rand() % 256;

  len = (hostkey.bytes > servkey.bytes ? hostkey.bytes : servkey.bytes);

  RSAblock = calloc (len, 1);
  if (!RSAblock)
     fatal ("Out of memory (1)");

  for (i = 0; i < sizeof(session_key); i++)
  {
    RSAblock[i] = session_key[i];
    if (i < sizeof(session_id))
       RSAblock[i] ^= session_id[i];
  }

  if (verbose)
     cputs ("Encrypting session key...");

  if (hostkey.bytes > servkey.bytes)
  {
    RSAencrypt (RSAblock, 32, &servkey);
    RSAencrypt (RSAblock, servkey.bytes, &hostkey);
  }
  else
  {
    RSAencrypt (RSAblock, 32, &hostkey);
    RSAencrypt (RSAblock, hostkey.bytes, &servkey);
  }
  if (verbose)
     cputs ("Done\n\r");

  create_header (SSH_CMSG_SESSION_KEY, len+15);
  pktout.body[0] = cipher_type;
  memcpy (pktout.body+1, cookie, 8);
  *(WORD*) &pktout.body[9] = intel16 (len * 8);
  memcpy (pktout.body+11, RSAblock, len);
  pktout.body[len+11] = 0;
  pktout.body[len+12] = 0;
  pktout.body[len+13] = 0;
  pktout.body[len+14] = 0;

  if (verbose)
     cputs ("Sending encrypted session key\r\n");

  write_packet();
  free (RSAblock);

  switch (cipher_type)
  {
    case SSH_CIPHER_3DES:
         des_set_key (session_key, &keys[0]);
         des_set_key (session_key+8, &keys[1]);
         des_set_key (session_key+16, &keys[2]);
         break;

    case SSH_CIPHER_BLOWFISH:
         blowfish_setkey (&ectx, SSH_SESSION_KEY_LENGTH, session_key);
         ectx.biv0 = 0;
         ectx.biv1 = 0;
         dctx = ectx;
         break;
  }
  cipher = 1;
}

/*
 * Send username
 */
static void send_username (const char *user)
{
  int i = strlen (user);

  create_header (SSH_CMSG_USER, i+4);
  pktout.body[0] = 0;
  pktout.body[1] = 0;
  pktout.body[2] = 0;
  pktout.body[3] = i;
  memcpy (pktout.body+4, user, i);
  write_packet();
}

/*
 * get username
 */
static void get_username (void)
{
  short len = 0;
  int   ch  = 0;

  cputs ("\r\nUsername: ");
  while (1)
  {
    ch = getche();
    if (ch == '\r' || len == sizeof(username))
       break;

    if (ch == '\b' && len > 0)
       username[--len] = '\0';
    else if (ch != '\b')
       username[len++] = ch;
  }
  username[len] = '\0';
  putch ('\n');
}

/*
 * get password
 */
static void get_password (void)
{
  short len = 0;
  int   ch  = 0;

  cputs ("Password: ");
  while (1)
  {
    ch = getch();
    if (ch == '\r' || len == sizeof(password))
       break;

    if (ch == '\b' && len > 0)
       password[--len] = '\0';
    else if (ch != '\b')
    {
      password[len++] = ch;
      putch ('*');
    }
  }
  password[len] = '\0';
  cprintf ("\r\n");

  create_header (SSH_CMSG_AUTH_PASSWORD, len+4);
  pktout.body[0] = 0;
  pktout.body[1] = 0;
  pktout.body[2] = 0;
  pktout.body[3] = len;
  memcpy (pktout.body+4, password, len);
  write_packet();
}

static void send_packet_size (void)
{
  int size = SSH_BUF_SIZE;

  if (verbose)
     cputs ("Setting maximum packet size...");

  create_header (SSH_CMSG_MAX_PACKET_SIZE, 4);
  pktout.body[0] = 0;
  pktout.body[1] = 0;
  *(WORD*)&pktout.body[2] = intel16 (size);
  write_packet();
  packet_read_block();
  if (verbose)
  {
    if (pktin.type == SSH_SMSG_SUCCESS)
         cputs ("success");
    else cputs ("failed");
  }
}

/*
 * allocate a pseudo terminal
 */
static void request_pty (const char *termtype, int max_row, int max_col)
{
  int len = strlen (termtype);

  create_header (SSH_CMSG_REQUEST_PTY, len+5*4+1);

  *(DWORD*) &pktout.body[0] = intel (len);
  memcpy (pktout.body+4, termtype, len);
  memset (pktout.body+4+len, 0, 17);
  pktout.body[ 7+len] = max_row;
  pktout.body[11+len] = max_col;
  write_packet();
}

static void ssh_session_recv (void)
{
  BYTE inbuf [SSH_BUF_SIZE];
  int  i, len = sock_fastread (&sock, (BYTE*)&inbuf, 4);

  if (!len)
     return;

  len = intel (*(DWORD*)inbuf);
  sock_read (&sock, (BYTE*) &inbuf[4], len+8 - (len % 8));
  ssh_gotdata (inbuf);

  switch (pktin.type)
  {
    case SSH_SMSG_STDOUT_DATA:
    case SSH_SMSG_STDERR_DATA:
         for (i = 0; i < pktin.length-5; i++)
             VT_Process (pktin.body[i+4]);
         return;

    case SSH_SMSG_EXITSTATUS:
         create_header (SSH_CMSG_EXIT_CONFIRMATION, 0);
         write_packet();
         quit = 1;
         return;

    case SSH_MSG_DISCONNECT:
         /* Connection reset by other party */
         pktin.body[pktin.length + 4] = 0;
         SCR_PutString ((const char*)pktin.body+4);
         quit = 1;
         return;

    case SSH_SMSG_SUCCESS:
    case SSH_MSG_IGNORE:
         return;

    default:
         if (dbug_mode || verbose)
            cprintf ("Unsupported packet received. Type: %ld\n\r", pktin.type);
         return;
  }
}

static void ssh_session_xmit (void)
{
  char str[20];
  int  len = GetKeyTN (str, sizeof(str));

  if (len > 0)
  {
    create_header (SSH_CMSG_STDIN_DATA, 4+len);
    pktout.body[0] = 0;
    pktout.body[1] = 0;
    pktout.body[2] = 0;
    pktout.body[3] = len;
    memcpy (&pktout.body[4], str, len);
    write_packet();
  }
}

int ConnectSSH (void)
{
  get_username();

  /* Start negotiation on network
   */
  if (verbose)
     cputs ("Identification Exchange\n\r");

  exchange_identification();

  /* Wait for a public key packet from the server.
   */
  if (verbose)
     cputs ("Wait for public key\n\r");

  packet_read_expect (SSH_SMSG_PUBLIC_KEY);

  /* Create SSH keys
   */
  create_ssh_keys();

  /* Wait for key confirmation
   */
  if (verbose)
     cputs ("Waiting for first encrypted ACK\n\r");

  packet_read_expect (SSH_SMSG_SUCCESS);

  /* Send username
   */
  send_username (username);
  packet_read_block();
  switch (pktin.type)
  {
    int n;

    case SSH_SMSG_SUCCESS:  /* no authentication needed */
         break;
    case SSH_SMSG_FAILURE:  /* get password */
         n = 3;             /* 3 chances */
         while (1)
         {
           get_password();
           packet_read_block();
           if (pktin.type == SSH_SMSG_SUCCESS)
              break;
           n--;
           if (n == 0)
                fatal ("Invalid password");
           else cputs ("Invalid password\n\r");
         }
         break;
    default:
         fatal ("Invalid packet received");
  }

  /* Set maximum packet size
   */
  send_packet_size();

  /* Request a remote pseudo-terminal
   */
  request_pty (VT_GetMode(), SCR_GetScrHeight(), SCR_GetScrWidth());
  packet_read_expect (SSH_SMSG_SUCCESS);

  /* Start remote shell
   */
  create_header (SSH_CMSG_EXEC_SHELL, 0);
  write_packet();

  recvState = ssh_session_recv;
  xmitState = ssh_session_xmit;
  return (1);
}

/*
 * create header for raw outgoing packet
 */
static void create_header (int type, int len)
{
  int pad, biglen;

  len   += 5;                     /* type and CRC */
  pad    = 8 - (len % 8);
  biglen = len + pad;

  pktout.length = len - 5;
  if (pktout.maxlen < biglen)
  {
    pktout.maxlen = biglen;
    if (!pktout.data)
       pktout.data = malloc (SSH_BUF_SIZE);
    if (!pktout.data)
       fatal ("Out of memory (2)");
  }
  pktout.type = type;
  pktout.body = pktout.data + 4 + pad + 1;
}

/*
 * create outgoing packet
 */
static void write_packet (void)
{
  int   pad, len, biglen, i;
  DWORD crc;

  len    = pktout.length + 5;      /* type and CRC */
  pad    = 8 - (len % 8);
  biglen = len + pad;

  pktout.body[-1] = pktout.type;
  for (i = 0; i < pad; i++)
      pktout.data[i+4] = rand() % 256;

  crc = crc32 (pktout.data+4, biglen-4);

  *(DWORD*) &pktout.data [biglen] = intel (crc);
  *(DWORD*) &pktout.data [0]      = intel (len);

  if (dbug_mode)
     tel_dump_hex ("SENT plaintext:", pktout.data, biglen+4);

  if (cipher)
  {
    switch (cipher_type)
    {
      case SSH_CIPHER_3DES:
           des_3cbc_encrypt (pktout.data+4, pktout.data+4, biglen, keys);
           break;

      case SSH_CIPHER_BLOWFISH:
           blowfish_encrypt_cbc (pktout.data+4, biglen, &ectx);
           break;
    }
    if (dbug_mode)
       tel_dump_hex ("SENT ciphertext: ", pktout.data, biglen+4);
  }
  if (sock_write (&sock, (const BYTE*)pktout.data, biglen+4) != biglen+4)
     fatal ("sock_write: %s", sockerr(&sock));
}

/*
 * convert raw, encrypted packet to readable structure
 */
static void ssh_gotdata (BYTE *data)
{
  long len, biglen;
  int  pad;

  len    = intel (*(DWORD*)data);
  pad    = 8 - (len % 8);
  biglen = len + pad;

  if (!pktin.data)
     pktin.data = malloc (SSH_BUF_SIZE);

  if (!pktin.data)
     fatal ("Out of memory (3)");

  pktin.length = biglen - 4 - pad;
  len = min (SSH_BUF_SIZE, biglen+4);

  if (dbug_mode || verbose)
  {
    if (cipher)
         tel_dump_hex ("RECEIVED ciphertext: ", data, len);
    else tel_dump_hex ("RECEIVED plaintext: ", data, len);
  }

  if (cipher)
  {
    switch (cipher_type)
    {
      case SSH_CIPHER_3DES:
           des_3cbc_decrypt (data+4, data+4, len-4, keys);
           break;

      case SSH_CIPHER_BLOWFISH:
           blowfish_decrypt_cbc (data+4, len-4, &dctx);
           break;
    }
    if (dbug_mode)
       tel_dump_hex ("RECEIVED plaintext: ", data, len);
  }
  memcpy (pktin.data, data+4, len-4);
  pktin.type = pktin.data[pad];
  pktin.body = pktin.data + pad + 1;
}

/*
 * get a packet with blocking
 */
static int packet_read_block (void)
{
  while (1)
  {
    BYTE inbuf [SSH_BUF_SIZE];
    int  status;

    sock_wait_input (&sock, sock_delay, NULL, &status);
    sock_fastread (&sock, (BYTE*)&inbuf, sizeof(inbuf));
    ssh_gotdata (inbuf);

    if (pktin.type == SSH_MSG_DEBUG)
    {
      if (dbug_mode || verbose)
      {
        int i = pktin.body[3];
        pktin.body[4+i] = 0;
        cprintf ("%s\r\n", pktin.body+4);
        tel_dump ("SSH in: %s\r\n", pktin.body+4);
      }
      continue;
    }
    if (pktin.type == SSH_MSG_IGNORE)
       continue;

    if (pktin.type == SSH_MSG_DISCONNECT)
    {
      int i = pktin.body[3];
      pktin.body[4+i] = 0;
      fatal ("DISCONNECT: %s", pktin.body+4);
    }
    break;
  }
  return (0);

sock_err:
  fatal ("Socket error: %s", sockerr(&sock));
  return (-1);
}

/*
 * expect a packet type
 */
static void packet_read_expect (DWORD type)
{
  packet_read_block();
  if (pktin.type == SSH_MSG_DISCONNECT)
  {
    int i = pktin.body[3];
    pktin.body[4+i] = '\0';
    fatal ("\nDISCONNECT: %s", pktin.body+4);
  }
  if (pktin.type != type)
     fatal ("Invalid answer from server");
}

/*
 * CRC32 implementation.
 *
 * The basic concept of a CRC is that you treat your bit-string
 * abcdefg... as a ludicrously long polynomial M=a+bx+cx^2+dx^3+...
 * over Z[2]. You then take a modulus polynomial P, and compute the
 * remainder of M on division by P. Thus, an erroneous message N
 * will only have the same CRC if the difference E = M-N is an
 * exact multiple of P. (Note that as we are working over Z[2], M-N
 * = N-M = M+N; but that's not very important.)
 *
 * What makes the CRC good is choosing P to have good properties:
 *
 *  - If its first and last terms are both nonzero then it cannot
 *    be a factor of any single term x^i. Therefore if M and N
 *    differ by exactly one bit their CRCs will guaranteeably
 *    be distinct.
 *
 *  - If it has a prime (irreducible) factor with three terms then
 *    it cannot divide a polynomial of the form x^i(1+x^j).
 *    Therefore if M and N differ by exactly _two_ bits they will
 *    have different CRCs.
 *
 *  - If it has a factor (x+1) then it cannot divide a polynomial
 *    with an odd number of terms. Therefore if M and N differ by
 *    _any odd_ number of bits they will have different CRCs.
 *
 *  - If the error term E is of the form x^i*B(x) where B(x) has
 *    order less than P (i.e. a short _burst_ of errors) then P
 *    cannot divide E (since no polynomial can divide a shorter
 *    one), so any such error burst will be spotted.
 *
 * The CRC32 standard polynomial is
 *   x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x^1+x^0
 *
 * In fact, we don't compute M mod P; we compute M*x^32 mod P.
 *
 * The concrete implementation of the CRC is this: we maintain at
 * all times a 32-bit word which is the current remainder of the
 * polynomial mod P. Whenever we receive an extra bit, we multiply
 * the existing remainder by x, add (XOR) the x^32 term thus
 * generated to the new x^32 term caused by the incoming bit, and
 * remove the resulting combined x^32 term if present by replacing
 * it with (P-x^32).
 *
 * Bit 0 of the word is the x^31 term and bit 31 is the x^0 term.
 * Thus, multiplying by x means shifting right. So the actual
 * algorithm goes like this:
 *
 *   x32term = (crcword & 1) ^ newbit;
 *   crcword = (crcword >> 1) ^ (x32term * 0xEDB88320);
 *
 * In practice, we pre-compute what will happen to crcword on any
 * given sequence of eight incoming bits, and store that in a table
 * which we then use at run-time to do the job:
 *
 *   outgoingplusnew = (crcword & 0xFF) ^ newbyte;
 *   crcword = (crcword >> 8) ^ table[outgoingplusnew];
 *
 * where table[outgoingplusnew] is computed by setting crcword=0
 * and then iterating the first code fragment eight times (taking
 * the incoming byte low bit first).
 *
 * Note that all shifts are rightward and thus no assumption is
 * made about exact word length! (Although word length must be at
 * _least_ 32 bits, but ANSI C guarantees this for `unsigned long'
 * anyway.)
 */

/* ----------------------------------------------------------------------
 * Multi-function module. Can be compiled three ways.
 *
 *  - Compile with no special #defines. Will generate a table
 *    that's already initialised at compile time, and one function
 *    crc32(buf,len) that uses it. Normal usage.
 *
 *  - Compile with INITFUNC defined. Will generate an uninitialised
 *    array as the table, and as well as crc32(buf,len) it will
 *    also generate void crc32_init(void) which sets up the table
 *    at run time. Useful if binary size is important.
 *
 *  - Compile with GENPROGRAM defined. Will create a standalone
 *    program that does the initialisation and outputs the table as
 *    C code.
 */

/*
 * This variant of the code has the data already prepared.
 */
static const DWORD crc32_table[256] = {
  0x00000000L, 0x77073096L, 0xEE0E612CL, 0x990951BAL,
  0x076DC419L, 0x706AF48FL, 0xE963A535L, 0x9E6495A3L,
  0x0EDB8832L, 0x79DCB8A4L, 0xE0D5E91EL, 0x97D2D988L,
  0x09B64C2BL, 0x7EB17CBDL, 0xE7B82D07L, 0x90BF1D91L,
  0x1DB71064L, 0x6AB020F2L, 0xF3B97148L, 0x84BE41DEL,
  0x1ADAD47DL, 0x6DDDE4EBL, 0xF4D4B551L, 0x83D385C7L,
  0x136C9856L, 0x646BA8C0L, 0xFD62F97AL, 0x8A65C9ECL,
  0x14015C4FL, 0x63066CD9L, 0xFA0F3D63L, 0x8D080DF5L,
  0x3B6E20C8L, 0x4C69105EL, 0xD56041E4L, 0xA2677172L,
  0x3C03E4D1L, 0x4B04D447L, 0xD20D85FDL, 0xA50AB56BL,
  0x35B5A8FAL, 0x42B2986CL, 0xDBBBC9D6L, 0xACBCF940L,
  0x32D86CE3L, 0x45DF5C75L, 0xDCD60DCFL, 0xABD13D59L,
  0x26D930ACL, 0x51DE003AL, 0xC8D75180L, 0xBFD06116L,
  0x21B4F4B5L, 0x56B3C423L, 0xCFBA9599L, 0xB8BDA50FL,
  0x2802B89EL, 0x5F058808L, 0xC60CD9B2L, 0xB10BE924L,
  0x2F6F7C87L, 0x58684C11L, 0xC1611DABL, 0xB6662D3DL,
  0x76DC4190L, 0x01DB7106L, 0x98D220BCL, 0xEFD5102AL,
  0x71B18589L, 0x06B6B51FL, 0x9FBFE4A5L, 0xE8B8D433L,
  0x7807C9A2L, 0x0F00F934L, 0x9609A88EL, 0xE10E9818L,
  0x7F6A0DBBL, 0x086D3D2DL, 0x91646C97L, 0xE6635C01L,
  0x6B6B51F4L, 0x1C6C6162L, 0x856530D8L, 0xF262004EL,
  0x6C0695EDL, 0x1B01A57BL, 0x8208F4C1L, 0xF50FC457L,
  0x65B0D9C6L, 0x12B7E950L, 0x8BBEB8EAL, 0xFCB9887CL,
  0x62DD1DDFL, 0x15DA2D49L, 0x8CD37CF3L, 0xFBD44C65L,
  0x4DB26158L, 0x3AB551CEL, 0xA3BC0074L, 0xD4BB30E2L,
  0x4ADFA541L, 0x3DD895D7L, 0xA4D1C46DL, 0xD3D6F4FBL,
  0x4369E96AL, 0x346ED9FCL, 0xAD678846L, 0xDA60B8D0L,
  0x44042D73L, 0x33031DE5L, 0xAA0A4C5FL, 0xDD0D7CC9L,
  0x5005713CL, 0x270241AAL, 0xBE0B1010L, 0xC90C2086L,
  0x5768B525L, 0x206F85B3L, 0xB966D409L, 0xCE61E49FL,
  0x5EDEF90EL, 0x29D9C998L, 0xB0D09822L, 0xC7D7A8B4L,
  0x59B33D17L, 0x2EB40D81L, 0xB7BD5C3BL, 0xC0BA6CADL,
  0xEDB88320L, 0x9ABFB3B6L, 0x03B6E20CL, 0x74B1D29AL,
  0xEAD54739L, 0x9DD277AFL, 0x04DB2615L, 0x73DC1683L,
  0xE3630B12L, 0x94643B84L, 0x0D6D6A3EL, 0x7A6A5AA8L,
  0xE40ECF0BL, 0x9309FF9DL, 0x0A00AE27L, 0x7D079EB1L,
  0xF00F9344L, 0x8708A3D2L, 0x1E01F268L, 0x6906C2FEL,
  0xF762575DL, 0x806567CBL, 0x196C3671L, 0x6E6B06E7L,
  0xFED41B76L, 0x89D32BE0L, 0x10DA7A5AL, 0x67DD4ACCL,
  0xF9B9DF6FL, 0x8EBEEFF9L, 0x17B7BE43L, 0x60B08ED5L,
  0xD6D6A3E8L, 0xA1D1937EL, 0x38D8C2C4L, 0x4FDFF252L,
  0xD1BB67F1L, 0xA6BC5767L, 0x3FB506DDL, 0x48B2364BL,
  0xD80D2BDAL, 0xAF0A1B4CL, 0x36034AF6L, 0x41047A60L,
  0xDF60EFC3L, 0xA867DF55L, 0x316E8EEFL, 0x4669BE79L,
  0xCB61B38CL, 0xBC66831AL, 0x256FD2A0L, 0x5268E236L,
  0xCC0C7795L, 0xBB0B4703L, 0x220216B9L, 0x5505262FL,
  0xC5BA3BBEL, 0xB2BD0B28L, 0x2BB45A92L, 0x5CB36A04L,
  0xC2D7FFA7L, 0xB5D0CF31L, 0x2CD99E8BL, 0x5BDEAE1DL,
  0x9B64C2B0L, 0xEC63F226L, 0x756AA39CL, 0x026D930AL,
  0x9C0906A9L, 0xEB0E363FL, 0x72076785L, 0x05005713L,
  0x95BF4A82L, 0xE2B87A14L, 0x7BB12BAEL, 0x0CB61B38L,
  0x92D28E9BL, 0xE5D5BE0DL, 0x7CDCEFB7L, 0x0BDBDF21L,
  0x86D3D2D4L, 0xF1D4E242L, 0x68DDB3F8L, 0x1FDA836EL,
  0x81BE16CDL, 0xF6B9265BL, 0x6FB077E1L, 0x18B74777L,
  0x88085AE6L, 0xFF0F6A70L, 0x66063BCAL, 0x11010B5CL,
  0x8F659EFFL, 0xF862AE69L, 0x616BFFD3L, 0x166CCF45L,
  0xA00AE278L, 0xD70DD2EEL, 0x4E048354L, 0x3903B3C2L,
  0xA7672661L, 0xD06016F7L, 0x4969474DL, 0x3E6E77DBL,
  0xAED16A4AL, 0xD9D65ADCL, 0x40DF0B66L, 0x37D83BF0L,
  0xA9BCAE53L, 0xDEBB9EC5L, 0x47B2CF7FL, 0x30B5FFE9L,
  0xBDBDF21CL, 0xCABAC28AL, 0x53B39330L, 0x24B4A3A6L,
  0xBAD03605L, 0xCDD70693L, 0x54DE5729L, 0x23D967BFL,
  0xB3667A2EL, 0xC4614AB8L, 0x5D681B02L, 0x2A6F2B94L,
  0xB40BBE37L, 0xC30C8EA1L, 0x5A05DF1BL, 0x2D02EF8DL
};

DWORD crc32 (const void *buf, size_t len)
{
  DWORD crcword = 0L;
  const BYTE *p = (const BYTE*) buf;

  while (len--)
  {
    DWORD newbyte = *p++;

    newbyte ^= crcword & 0xFFL;
    crcword = (crcword >> 8) ^ crc32_table[newbyte];
  }
  return crcword;
}

