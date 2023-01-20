#ifndef __SSH_H
#define __SSH_H

#define SSH_VERSION  "0.8"

/* SSH constants and protocol structure */

#define SSH_BUF_SIZE                         4096       /* I/O buffer size */

#define SSH_MSG_NONE                            0       /* no message */
#define SSH_MSG_DISCONNECT                      1       /* cause (string) */
#define SSH_SMSG_PUBLIC_KEY                     2       /* ck,msk,srvk,hostk */
#define SSH_CMSG_SESSION_KEY                    3       /* key (MP_INT) */
#define SSH_CMSG_USER                           4       /* user (string) */
#define SSH_CMSG_AUTH_RHOSTS                    5       /* user (string) */
#define SSH_CMSG_AUTH_RSA                       6       /* modulus (MP_INT) */
#define SSH_SMSG_AUTH_RSA_CHALLENGE             7       /* int (MP_INT) */
#define SSH_CMSG_AUTH_RSA_RESPONSE              8       /* int (MP_INT) */
#define SSH_CMSG_AUTH_PASSWORD                  9       /* pass (string) */
#define SSH_CMSG_REQUEST_PTY                    10      /* TERM, tty modes */
#define SSH_CMSG_WINDOW_SIZE                    11      /* row,col,xpix,ypix */
#define SSH_CMSG_EXEC_SHELL                     12      /* */
#define SSH_CMSG_EXEC_CMD                       13      /* cmd (string) */
#define SSH_SMSG_SUCCESS                        14      /* */
#define SSH_SMSG_FAILURE                        15      /* */
#define SSH_CMSG_STDIN_DATA                     16      /* data (string) */
#define SSH_SMSG_STDOUT_DATA                    17      /* data (string) */
#define SSH_SMSG_STDERR_DATA                    18      /* data (string) */
#define SSH_CMSG_EOF                            19      /* */
#define SSH_SMSG_EXITSTATUS                     20      /* status (int) */
#define SSH_MSG_CHANNEL_OPEN_CONFIRMATION       21      /* channel (int) */
#define SSH_MSG_CHANNEL_OPEN_FAILURE            22      /* channel (int) */
#define SSH_MSG_CHANNEL_DATA                    23      /* ch,data (int,str) */
#define SSH_MSG_CHANNEL_CLOSE                   24      /* channel (int) */
#define SSH_MSG_CHANNEL_CLOSE_CONFIRMATION      25      /* channel (int) */

/* new channel protocol */
#define SSH_MSG_CHANNEL_INPUT_EOF               24
#define SSH_MSG_CHANNEL_OUTPUT_CLOSED           25

/*      SSH_CMSG_X11_REQUEST_FORWARDING         26         OBSOLETE */
#define SSH_SMSG_X11_OPEN                       27      /* channel (int) */
#define SSH_CMSG_PORT_FORWARD_REQUEST           28      /* p,host,hp (i,s,i) */
#define SSH_MSG_PORT_OPEN                       29      /* ch,h,p (i,s,i) */
#define SSH_CMSG_AGENT_REQUEST_FORWARDING       30      /* */
#define SSH_SMSG_AGENT_OPEN                     31      /* port (int) */
#define SSH_MSG_IGNORE                          32      /* string */
#define SSH_CMSG_EXIT_CONFIRMATION              33      /* */
#define SSH_CMSG_X11_REQUEST_FORWARDING         34      /* proto,data (s,s) */
#define SSH_CMSG_AUTH_RHOSTS_RSA                35      /* user,mod (s,mpi) */
#define SSH_MSG_DEBUG                           36      /* string */
#define SSH_CMSG_REQUEST_COMPRESSION            37      /* level 1-9 (int) */
#define SSH_CMSG_MAX_PACKET_SIZE                38      /* max_size (int) */

/* Support for TIS authentication server
 * Contributed by Andre April <Andre.April@cediti.be>.
 */
#define SSH_CMSG_AUTH_TIS                       39      /* */
#define SSH_SMSG_AUTH_TIS_CHALLENGE             40      /* string */
#define SSH_CMSG_AUTH_TIS_RESPONSE              41      /* pass (string) */

/* Support for kerberos authentication by Glenn Machin and Dug Song
 * <dugsong@umich.edu>
 */
#define SSH_CMSG_AUTH_KERBEROS                  42      /* string (KTEXT) */
#define SSH_SMSG_AUTH_KERBEROS_RESPONSE         43      /* string (KTEXT) */
#define SSH_CMSG_HAVE_KERBEROS_TGT              44      /* string (credentials) */

/* Reserved for official extensions, do not use these
 */
#define SSH_CMSG_RESERVED_START                 45
#define SSH_CMSG_RESERVED_END                   63

#define EMULATE_OLD_CHANNEL_CODE 0x0001
#define EMULATE_OLD_AGENT_BUG	 0x0002

#define EMULATE_VERSION_OK	       		0
#define EMULATE_MAJOR_VERSION_MISMATCH 		1
#define EMULATE_VERSION_TOO_OLD 		2
#define EMULATE_VERSION_NEWER	 		3

/* Major protocol version.  Different version indicates major incompatiblity
 *  that prevents communication.
 */
#define PROTOCOL_MAJOR          1

/* Minor protocol version.  Different version indicates minor incompatibility
 * that does not prevent interoperation.
 */
#define PROTOCOL_MINOR          5

#define SSH_CIPHER_NONE		0
#define SSH_CIPHER_IDEA		1
#define SSH_CIPHER_DES		2
#define SSH_CIPHER_3DES		3
#define SSH_CIPHER_RC4		5
#define SSH_CIPHER_BLOWFISH	6

/* macros
 */
#define GET_32BIT_LSB_FIRST(cp)         \
        (((DWORD)(BYTE)(cp)[0])       | \
         ((DWORD)(BYTE)(cp)[1] << 8)  | \
         ((DWORD)(BYTE)(cp)[2] << 16) | \
         ((DWORD)(BYTE)(cp)[3] << 24))

#define PUT_32BIT_LSB_FIRST(cp, value) \
        do {                           \
          (cp)[0] = (value);           \
          (cp)[1] = (value) >> 8;      \
          (cp)[2] = (value) >> 16;     \
          (cp)[3] = (value) >> 24;     \
        } while (0)

#define GET_32BIT_MSB_FIRST(cp)         \
        (((DWORD)(BYTE)(cp)[3])       | \
         ((DWORD)(BYTE)(cp)[2] << 8)  | \
         ((DWORD)(BYTE)(cp)[1] << 16) | \
         ((DWORD)(BYTE)(cp)[0] << 24))

#define PUT_32BIT_MSB_FIRST(cp, value) \
        do {                           \
          (cp)[3] = (value);           \
          (cp)[2] = (value) >> 8;      \
          (cp)[1] = (value) >> 16;     \
          (cp)[0] = (value) >> 24;     \
        } while (0)

#ifdef USE_SSL

#include <openssl/md5.h>
#include <openssl/des.h>
#include <openssl/blowfish.h>

#define DESCon               DES_key_schedule
#define des_set_key          DES_set_key

#define BlowfishContext      BF_KEY
#define blowfish_setkey      BF_set_key
#define blowfish_encrypt_cbc

#else

/* DES / 3DES cipher structures and functions */

typedef struct {
        DWORD k0246[16], k1357[16];
        DWORD eiv0, eiv1;
        DWORD div0, div1;
      } DESCon;

void des_set_key (BYTE *key, DESCon *ks);
void des_3cbc_encrypt (BYTE *dest, const BYTE *src,
                       unsigned int len, DESCon *scheds);
void des_3cbc_decrypt (BYTE *dest, const BYTE *src,
                       unsigned int len, DESCon *scheds);

/* Blowfish cipher structures and functions */

typedef struct {
        DWORD S0[256], S1[256], S2[256], S3[256], P[18];
        DWORD biv0, biv1;                  /* for CBC mode */
      } BlowfishContext;

#define SSH_SESSION_KEY_LENGTH	32

void blowfish_setkey (BlowfishContext *ctx, short keybytes, const BYTE *key);
void blowfish_encrypt_cbc (BYTE *blk, int len, BlowfishContext *ctx);
void blowfish_decrypt_cbc (BYTE *blk, int len, BlowfishContext *ctx);

/* RSA key structure */

typedef struct {
        DWORD  bits;
        DWORD  bytes;
        void  *modulus;
        void  *exponent;
      } R_RSAKey;

int  RSAmakekey (BYTE *data, R_RSAKey *result, BYTE **keystr);
void RSAencrypt (BYTE *data, int length, R_RSAKey *key);

/* MD5 hash structures and functions */

typedef struct {
        DWORD  h[4];
      } MD5_Core_State;

typedef struct MD5Context {
        MD5_Core_State core;
        BYTE           block[64];
        int            blkused;
        DWORD          lenhi, lenlo;
      } MD5Context;

void MD5Init (MD5Context *context);
void MD5Update (MD5Context *context, BYTE const *buf, unsigned len);
void MD5Final (BYTE digest[16], MD5Context *context);

#endif /* USE_SSL */

int ConnectSSH (void);

#endif
