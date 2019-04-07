/* MikMod sound library (c) 2003-2015 Raphael Assenat and others -
 * see AUTHORS file for a complete list.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

/* amiga Powerpack PP20 decompression support
 *
 * Code from Heikki Orsila's amigadepack 0.02
 * based on code by Stuart Caie <kyzer@4u.net>
 * This software is in the Public Domain
 *
 * Modified for xmp by Claudio Matsuoka, 08/2007
 * - merged mld's checks from the old depack sources. Original credits:
 *   - corrupt file and data detection
 *     (thanks to Don Adan and Dirk Stoecker for help and infos)
 *   - implemeted "efficiency" checks
 *   - further detection based on code by Georg Hoermann
 *
 * Modified for xmp by Claudio Matsuoka, 05/2013
 * - decryption code removed
 *
 * Modified for libmikmod by O. Sezer, Apr. 2015, with a few extra bits
 * from the libmodplug library by Olivier Lapicque.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#include <string.h>

#include "mikmod_internals.h"

#ifdef SUNOS
extern int fprintf(FILE *, const char *, ...);
#endif

#define PP_READ_BITS(nbits, var) do {                          \
  bit_cnt = (nbits);                                           \
  while (bits_left < bit_cnt) {                                \
    if (buf_src < src) return 0; /* out of source bits */      \
    bit_buffer |= (*--buf_src << bits_left);                   \
    bits_left += 8;                                            \
  }                                                            \
  (var) = 0;                                                   \
  bits_left -= bit_cnt;                                        \
  while (bit_cnt--) {                                          \
    (var) = ((var) << 1) | (bit_buffer & 1);                   \
    bit_buffer >>= 1;                                          \
  }                                                            \
} while(0)

#define PP_BYTE_OUT(byte) do {                                 \
  if (out <= dest) return 0; /* output overflow */             \
  *--out = (byte);                                             \
  written++;                                                   \
} while (0)

static BOOL ppDecrunch(const UBYTE *src, UBYTE *dest,
                       const UBYTE *offset_lens,
                       ULONG src_len, ULONG dest_len,
                       UBYTE skip_bits)
{
  ULONG bit_buffer, x, todo, offbits, offset, written;
  const UBYTE *buf_src;
  UBYTE *out, *dest_end, bits_left, bit_cnt;

  /* set up input and output pointers */
  buf_src = src + src_len;
  out = dest_end = dest + dest_len;

  written = 0;
  bit_buffer = 0;
  bits_left = 0;

  /* skip the first few bits */
  PP_READ_BITS(skip_bits, x);

  /* while there are input bits left */
  while (written < dest_len) {
    PP_READ_BITS(1, x);
    if (x == 0) {
      /* 1bit==0: literal, then match. 1bit==1: just match */
      todo = 1; do { PP_READ_BITS(2, x); todo += x; } while (x == 3);
      while (todo--) { PP_READ_BITS(8, x); PP_BYTE_OUT(x); }

      /* should we end decoding on a literal, break out of the main loop */
      if (written == dest_len) break;
    }

    /* match: read 2 bits for initial offset bitlength / match length */
    PP_READ_BITS(2, x);
    offbits = offset_lens[x];
    todo = x+2;
    if (x == 3) {
      PP_READ_BITS(1, x);
      if (x==0) offbits = 7;
      PP_READ_BITS(offbits, offset);
      do { PP_READ_BITS(3, x); todo += x; } while (x == 7);
    }
    else {
      PP_READ_BITS(offbits, offset);
    }
    if ((out + offset) >= dest_end) return 0; /* match overflow */
    while (todo--) { x = out[offset]; PP_BYTE_OUT(x); }
  }

  /* all output bytes written without error */
  return 1;
  /* return (src == buf_src) ? 1 : 0; */
}

BOOL PP20_Unpack(MREADER* reader, void** out, long* outlen)
{
	ULONG srclen, destlen;
	UBYTE *destbuf, *srcbuf;
	UBYTE tmp[4], skip;
	BOOL ret;

	/* PP FORMAT:
	 *  1 longword identifier       'PP20' or 'PX20'
	 * [1 word checksum (if 'PX20') $ssss]
	 *  1 longword efficiency       $eeeeeeee
	 *  X longwords crunched file   $cccccccc,$cccccccc,...
	 *  1 longword decrunch info    'decrlen' << 8 | '8 bits other info'
	 */

	_mm_fseek(reader,0,SEEK_END);
	srclen = _mm_ftell(reader);
	if (srclen < 256) return 0;
	/* file length should be a multiple of 4 */
	if (srclen & 3) return 0;

	_mm_rewind(reader);
	if (_mm_read_I_ULONG(reader) != 0x30325050)	/* 'PP20' */
		return 0;

	_mm_fseek(reader,srclen-4,SEEK_SET);
	_mm_read_UBYTES(tmp,4,reader);
	destlen = tmp[0] << 16;
	destlen |= tmp[1] << 8;
	destlen |= tmp[2];
	skip = tmp[3];

	_mm_fseek(reader,4,SEEK_SET);
	_mm_read_UBYTES(tmp,4,reader);

	/* original pp20 only support efficiency
	 * from 9 9 9 9 up to 9 10 12 13, afaik,
	 * but the xfd detection code says this...
	 *
	 * move.l 4(a0),d0
	 * cmp.b #9,d0
	 * blo.b .Exit
	 * and.l #$f0f0f0f0,d0
	 * bne.s .Exit
	 */
	if ((tmp[0] < 9) || (tmp[0] & 0xf0)) return 0;
	if ((tmp[1] < 9) || (tmp[1] & 0xf0)) return 0;
	if ((tmp[2] < 9) || (tmp[2] & 0xf0)) return 0;
	if ((tmp[3] < 9) || (tmp[3] & 0xf0)) return 0;

	if ((destlen < 512) || (destlen > 0x400000) || (destlen > 16*srclen))
		return 0;
	if ((destbuf = (UBYTE*)MikMod_malloc(destlen)) == NULL)
		return 0;

	srclen -= 12;
	if ((srcbuf = (UBYTE*)MikMod_malloc(srclen)) == NULL) {
		MikMod_free(destbuf);
		return 0;
	}
	_mm_read_UBYTES(srcbuf,srclen,reader);

	ret = ppDecrunch(srcbuf, destbuf, tmp, srclen, destlen, skip);
	MikMod_free(srcbuf);

	if (!ret) {
		MikMod_free(destbuf);
	}
	else {
		*out = destbuf;
		*outlen = destlen;
	}
	return ret;
}
