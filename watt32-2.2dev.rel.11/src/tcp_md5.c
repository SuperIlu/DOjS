/*!\file tcp_md5.c
 *
 *  Handling of TCP MD5 fingrprint option.
 *
 *  Copyright (c) 1997-2002 Gisle Vanem <gvanem@yahoo.no>
 *  Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All rights
 *  reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. All advertising materials mentioning features or use of this software
 *     must display the following acknowledgement:
 *       This product includes software developed by Gisle Vanem
 *       Bergen, Norway.
 *
 *  THIS SOFTWARE IS PROVIDED BY ME (Gisle Vanem) AND CONTRIBUTORS ``AS IS''
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL I OR CONTRIBUTORS BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  Version
 *
 *  0.1 : Mar 24, 2004 : G. Vanem - created
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "wattcp.h"
#include "chksum.h"
#include "strings.h"
#include "misc.h"
#include "pctcp.h"

#if defined(USE_TCP_MD5)

/* MD5 context.
 */
typedef struct {
        DWORD  state [4];    /* state (ABCD) */
        DWORD  count [2];    /* number of bits, modulo 2^64 (LSB first) */
        BYTE   buffer [64];  /* input buffer */
      } MD5_CTX;

static void MD5_Init   (MD5_CTX *context);
static void MD5_Update (MD5_CTX *context, const void *buffer, size_t length);
static void MD5_Final  (BYTE result[16], MD5_CTX *context);

const char *W32_CALL tcp_md5_secret (_tcp_Socket *sock, const char *secret)
{
  char *old = sock->secret;

  if (old)
     free (old);
  sock->secret = secret ? strdup (secret) : NULL;
  return (old);
}

/*
 * Used to insert the MD5 signature *after* we've built the complete
 * TCP packet.
 */
void W32_CALL make_md5_signature (const in_Header *ip, const tcp_Header *tcp,
                                  WORD datalen, const char *secret, BYTE *buf)
{
  struct tcp_PseudoHeader ph;
  struct tcp_Header       tcp_copy;
  const BYTE *data = (const BYTE*)tcp + (tcp->offset << 2);
  MD5_CTX ctx;

  MD5_Init (&ctx);

  /* Step 1: Update MD5 hash with IP pseudo-header.
   */
  memset (&ph, 0, sizeof(ph));
  ph.src      = ip->source;
  ph.dst      = ip->destination;
  ph.protocol = ip->proto;
  ph.length   = intel16 (datalen);

  MD5_Update (&ctx, &ph.src, sizeof(ph.src));
  MD5_Update (&ctx, &ph.dst, sizeof(ph.dst));
  MD5_Update (&ctx, &ph.mbz, sizeof(ph.mbz));
  MD5_Update (&ctx, &ph.protocol, sizeof(ph.protocol));
  MD5_Update (&ctx, &ph.length, sizeof(ph.length));

  /* Step 2: Update MD5 hash with TCP header, excluding options.
   * The TCP checksum must be set to zero.
   */
  memcpy (&tcp_copy, tcp, sizeof(tcp_copy));
  tcp_copy.checksum = 0;
  MD5_Update (&ctx, &tcp_copy, sizeof(tcp_copy));

  /* Step 3: Update MD5 hash with TCP segment data, if present.
   */
  if (datalen > 0)
     MD5_Update (&ctx, data, datalen);

  /* Step 4: Update MD5 hash with shared secret.
   */
  MD5_Update (&ctx, secret, strlen(secret));
  MD5_Final (buf, &ctx);
}

/*
 * Check incoming TCP segment for MD5 option.
 */
BOOL W32_CALL check_md5_signature (const _tcp_Socket *s, const in_Header *ip)
{
  const struct tcp_Header *tcp;
  const BYTE  *recv_sign, *data, *opt;
  struct tcp_PseudoHeader ph;
  struct tcp_Header       tcp_copy;

  BYTE    signature [16];
  MD5_CTX ctx;
  WORD    len;

  if (!s->secret)
     return (FALSE);

  WATT_ASSERT (ip->ver == 4);

  tcp = (tcp_Header*) ((BYTE*)ip + in_GetHdrLen (ip));

  len = intel16 (ip->length) - in_GetHdrLen (ip);
  len -= (tcp->offset << 2);
  data = (const BYTE*)tcp + (tcp->offset << 2);

  /* find the TCPOPT_SIGNATURE signature
   */
  recv_sign = NULL;
  for (opt = (const BYTE*)(tcp+1); opt < data; )
  {
    if (*opt == TCPOPT_SIGNATURE)
    {
      recv_sign = opt + 2;
      break;
    }
    if (*opt == TCPOPT_EOL)
       break;

    if (*opt == TCPOPT_NOP)
         opt++;
    else opt += *(opt+1);
  }

  if (!recv_sign || opt[1] != TCPOPT_SIGN_LEN)
     return (FALSE);

  MD5_Init (&ctx);

  /* Step 1: Update MD5 hash with IP pseudo-header.
   */
  memset (&ph, 0, sizeof(ph));
  ph.src      = ip->source;
  ph.dst      = ip->destination;
  ph.protocol = ip->proto;
  ph.length   = intel16 (len);

  MD5_Update (&ctx, &ph.src, sizeof(ph.src));
  MD5_Update (&ctx, &ph.dst, sizeof(ph.dst));
  MD5_Update (&ctx, &ph.mbz, sizeof(ph.mbz));
  MD5_Update (&ctx, &ph.protocol, sizeof(ph.protocol));
  MD5_Update (&ctx, &ph.length, sizeof(ph.length));

  /* Step 2: Update MD5 hash with TCP header, excluding options.
   * The TCP checksum must be set to zero.
   */
  memcpy (&tcp_copy, tcp, sizeof(tcp_copy));
  tcp_copy.checksum = 0;
  MD5_Update (&ctx, &tcp_copy, sizeof(tcp_copy));

  /* Step 3: Update MD5 hash with TCP segment data, if present.
   */
  if (len > 0)
     MD5_Update (&ctx, data, len);

  /* Step 4: Update MD5 hash with shared secret.
   */
  MD5_Update (&ctx, s->secret, strlen(s->secret));
  MD5_Final (signature, &ctx);

  if (!memcmp(recv_sign, signature, sizeof(signature)))
     return (TRUE);
  return (FALSE);
}

/*
 * taken from RFC-1321/Appendix A.3
 * MD5C.C - RSA Data Security, Inc., MD5 message-digest algorithm
 */

/*
 * Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All rights
 * reserved.
 *
 * License to copy and use this software is granted provided that it is
 * identified as the "RSA Data Security, Inc. MD5 Message-Digest Algorithm"
 * in all material mentioning or referencing this software or this function.
 *
 * License is also granted to make and use derivative works provided that such
 * works are identified as "derived from the RSA Data Security, Inc. MD5
 * Message-Digest Algorithm" in all material mentioning or referencing the
 * derived work.
 *
 * RSA Data Security, Inc. makes no representations concerning either the
 * merchantability of this software or the suitability of this software for
 * any particular purpose. It is provided "as is" without express or implied
 * warranty of any kind.
 *
 * These notices must be retained in any copies of any part of this
 * documentation and/or software.
 */

/*
 * Constants for MD5Transform routine.
 */
#define S11    7
#define S12   12
#define S13   17
#define S14   22
#define S21    5
#define S22    9
#define S23   14
#define S24   20
#define S31    4
#define S32   11
#define S33   16
#define S34   23
#define S41    6
#define S42   10
#define S43   15
#define S44   21

static void Transform (DWORD[4], const BYTE *);
static void Encode    (BYTE *, const DWORD *, size_t);
static void Decode    (DWORD *, const BYTE *, size_t);

static BYTE padding[64] = {
       0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0
     };

/*
 * F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/*
 * ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/*
 * FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4. Rotation is
 * separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) {                    \
        (a) += F ((b), (c), (d)) + (x) + (DWORD)(ac); \
        (a) = ROTATE_LEFT ((a), (s));                 \
        (a) += (b);                                   \
      }
#define GG(a, b, c, d, x, s, ac) {                    \
        (a) += G ((b), (c), (d)) + (x) + (DWORD)(ac); \
        (a) = ROTATE_LEFT ((a), (s));                 \
        (a) += (b);                                   \
      }
#define HH(a, b, c, d, x, s, ac) {                    \
        (a) += H ((b), (c), (d)) + (x) + (DWORD)(ac); \
        (a) = ROTATE_LEFT ((a), (s));                 \
        (a) += (b);                                   \
      }
#define II(a, b, c, d, x, s, ac) {                    \
        (a) += I ((b), (c), (d)) + (x) + (DWORD)(ac); \
        (a) = ROTATE_LEFT ((a), (s));                 \
        (a) += (b);                                   \
      }

/*
 * MD5 initialization. Begins an MD5 operation, writing a new context.
 */
static void MD5_Init (MD5_CTX *context)
{
  context->count[0] = context->count[1] = 0;

  /* Load magic initialization constants.
   */
  context->state[0] = 0x67452301;
  context->state[1] = 0xefcdab89;
  context->state[2] = 0x98badcfe;
  context->state[3] = 0x10325476;
}

/*
 * MD5 block update operation. Continues an MD5 message-digest operation,
 * processing another message block, and updating the context.
 */
static void MD5_Update (MD5_CTX    *context,    /* context */
                        const void *input,      /* input block */
                        size_t      inputLen)   /* length of input block */
{
  DWORD i, index, partLen;

  /* Compute number of bytes mod 64 */
  index = (DWORD) ((context->count[0] >> 3) & 0x3F);

  /* Update number of bits */
  if ((context->count[0] += ((DWORD)inputLen << 3)) < ((DWORD)inputLen << 3))
       context->count[1]++;
  context->count[1] += ((DWORD)inputLen >> 29);

  partLen = 64 - index;

  /* Transform as many times as possible.
   */
  if (inputLen >= partLen)
  {
    memcpy (&context->buffer[index], input, partLen);
    Transform (context->state, context->buffer);

    for (i = partLen; i + 63 < inputLen; i += 64)
        Transform (context->state, (const BYTE*)input+i);
    index = 0;
  }
  else
    i = 0;

  /* Buffer remaining input */
  memcpy (&context->buffer[index], (const char*)input+i, inputLen-i);
}

/*
 * MD5 finalization. Ends an MD5 message-digest operation, writing the
 * message digest and zeroizing the context.
 */
static void MD5_Final (BYTE     digest[16],  /* message digest */
                       MD5_CTX *context)     /* context */
{
  BYTE  bits[8];
  DWORD index, padLen;

  /* Save number of bits */
  Encode (bits, context->count, 8);

  /* Pad out to 56 mod 64. */
  index  = (DWORD) ((context->count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  MD5_Update (context, padding, padLen);

  /* Append length (before padding) */
  MD5_Update (context, bits, 8);

  /* Store state in digest */
  Encode (digest, context->state, 16);

  /* Zeroize sensitive information. */
  memset (context, 0, sizeof(*context));
}

/*
 * MD5 basic transformation. Transforms state based on block.
 */
static void Transform (DWORD state[4], const BYTE *block)
{
  DWORD a = state[0];
  DWORD b = state[1];
  DWORD c = state[2];
  DWORD d = state[3];
  DWORD x[16];

  Decode (x, block, 64);

  /* Round 1 */
  FF (a, b, c, d, x[0],  S11, 0xd76aa478); /* 1  */
  FF (d, a, b, c, x[1],  S12, 0xe8c7b756); /* 2  */
  FF (c, d, a, b, x[2],  S13, 0x242070db); /* 3  */
  FF (b, c, d, a, x[3],  S14, 0xc1bdceee); /* 4  */
  FF (a, b, c, d, x[4],  S11, 0xf57c0faf); /* 5  */
  FF (d, a, b, c, x[5],  S12, 0x4787c62a); /* 6  */
  FF (c, d, a, b, x[6],  S13, 0xa8304613); /* 7  */
  FF (b, c, d, a, x[7],  S14, 0xfd469501); /* 8  */
  FF (a, b, c, d, x[8],  S11, 0x698098d8); /* 9  */
  FF (d, a, b, c, x[9],  S12, 0x8b44f7af); /* 10 */
  FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
  FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
  FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
  FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
  FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
  FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

  /* Round 2 */
  GG (a, b, c, d, x[1],  S21, 0xf61e2562); /* 17 */
  GG (d, a, b, c, x[6],  S22, 0xc040b340); /* 18 */
  GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
  GG (b, c, d, a, x[0],  S24, 0xe9b6c7aa); /* 20 */
  GG (a, b, c, d, x[5],  S21, 0xd62f105d); /* 21 */
  GG (d, a, b, c, x[10], S22, 0x2441453);  /* 22 */
  GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
  GG (b, c, d, a, x[4],  S24, 0xe7d3fbc8); /* 24 */
  GG (a, b, c, d, x[9],  S21, 0x21e1cde6); /* 25 */
  GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
  GG (c, d, a, b, x[3],  S23, 0xf4d50d87); /* 27 */
  GG (b, c, d, a, x[8],  S24, 0x455a14ed); /* 28 */
  GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
  GG (d, a, b, c, x[2],  S22, 0xfcefa3f8); /* 30 */
  GG (c, d, a, b, x[7],  S23, 0x676f02d9); /* 31 */
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

  /* Round 3 */
  HH (a, b, c, d, x[5],  S31, 0xfffa3942); /* 33 */
  HH (d, a, b, c, x[8],  S32, 0x8771f681); /* 34 */
  HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
  HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
  HH (a, b, c, d, x[1],  S31, 0xa4beea44); /* 37 */
  HH (d, a, b, c, x[4],  S32, 0x4bdecfa9); /* 38 */
  HH (c, d, a, b, x[7],  S33, 0xf6bb4b60); /* 39 */
  HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
  HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
  HH (d, a, b, c, x[0],  S32, 0xeaa127fa); /* 42 */
  HH (c, d, a, b, x[3],  S33, 0xd4ef3085); /* 43 */
  HH (b, c, d, a, x[6],  S34, 0x4881d05);  /* 44 */
  HH (a, b, c, d, x[9],  S31, 0xd9d4d039); /* 45 */
  HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
  HH (b, c, d, a, x[2],  S34, 0xc4ac5665); /* 48 */

  /* Round 4 */
  II (a, b, c, d, x[0],  S41, 0xf4292244); /* 49 */
  II (d, a, b, c, x[7],  S42, 0x432aff97); /* 50 */
  II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
  II (b, c, d, a, x[5],  S44, 0xfc93a039); /* 52 */
  II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
  II (d, a, b, c, x[3],  S42, 0x8f0ccc92); /* 54 */
  II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
  II (b, c, d, a, x[1],  S44, 0x85845dd1); /* 56 */
  II (a, b, c, d, x[8],  S41, 0x6fa87e4f); /* 57 */
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
  II (c, d, a, b, x[6],  S43, 0xa3014314); /* 59 */
  II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
  II (a, b, c, d, x[4],  S41, 0xf7537e82); /* 61 */
  II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
  II (c, d, a, b, x[2],  S43, 0x2ad7d2bb); /* 63 */
  II (b, c, d, a, x[9],  S44, 0xeb86d391); /* 64 */

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;

  /* Zeroize sensitive information. */
  memset (&x, 0, sizeof(x));
}

/*
 * Encodes input (DWORD) into output (BYTE). Assumes len is a
 * multiple of 4.
 */
static void Encode (BYTE *output, const DWORD *input, size_t len)
{
  size_t i, j;

  for (i = 0, j = 0; j < len; i++, j += 4)
  {
#if 0
    output[j]   = (BYTE) (input[i] & 0xff);
    output[j+1] = (BYTE) ((input[i] >> 8) & 0xff);
    output[j+2] = (BYTE) ((input[i] >> 16) & 0xff);
    output[j+3] = (BYTE) ((input[i] >> 24) & 0xff);
#else
    *(DWORD*) (output+j) = intel (input[i]);
#endif
  }
}

/*
 * Decodes input (BYTE) into output (DWORD). Assumes len is a
 * multiple of 4.
 */
static void Decode (DWORD *output, const BYTE *input, size_t len)
{
  size_t i, j;

  for (i = 0, j = 0; j < len; i++, j += 4)
#if 0
      output[i] = ((DWORD)input[j]) |
                  (((DWORD)input[j+1]) << 8) |
                  (((DWORD)input[j+2]) << 16) |
                  (((DWORD)input[j+3]) << 24);
#else
      output[i] = intel (*(DWORD*)(input+j));
#endif
}

#endif  /* USE_TCP_MD5 */
