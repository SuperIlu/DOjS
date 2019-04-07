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

/*
 * StoneCracker S404 algorithm data decompression routine
 * (c) 2006 Jouni 'Mr.Spiv' Korhonen. The code is in public domain.
 *
 * modified for xmp by Claudio Matsuoka, Jan 2010
 * modified for libmikmod by O. Sezer -- May 2015
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
/*#include <assert.h>*/

#include "mikmod_internals.h"

#ifdef SUNOS
extern int fprintf(FILE *, const char *, ...);
#endif


/*#define STC_DEBUG*/

static UWORD readmem16b(UBYTE *m)
{
  ULONG a, b;
  a = m[0];
  b = m[1];
  return (a << 8) | b;
}

struct bitstream {
  /* bit buffer for rolling data bit by bit from the compressed file */
  ULONG word;

  /* bits left in the bit buffer */
  int left;

  /* compressed data source */
  UWORD *src;
  UBYTE *orgsrc;
};

static int initGetb(struct bitstream *bs, UBYTE *src, ULONG src_length)
{
  int eff;

  bs->src = (UWORD *) (src + src_length);
  bs->orgsrc = src;

  bs->left = readmem16b((UBYTE *)bs->src); /* bit counter */
#ifdef STC_DEBUG
  if (bs->left & (~0xf))
    fprintf(stderr, "Worked around an ancient stc bug\n");
#endif
  /* mask off any corrupt bits */
  bs->left &= 0x000f;
  bs->src--;

  /* get the first 16-bits of the compressed stream */
  bs->word = readmem16b((UBYTE *)bs->src);
  bs->src--;

  eff = readmem16b((UBYTE *)bs->src); /* efficiency */
  bs->src--;

  return eff;
}

/* get nbits from the compressed stream */
static int getb(struct bitstream *bs, int nbits)
{
  bs->word &= 0x0000ffff;

  /* If not enough bits in the bit buffer, get more */
  if (bs->left < nbits) {
    bs->word <<= bs->left;
    /*assert((bs->word & 0x0000ffffU) == 0);*/

    /* Check that we don't go out of bounds */
    if (bs->orgsrc > (UBYTE *)bs->src) {
       return -1;
    }

    bs->word |= readmem16b((UBYTE *)bs->src);
    bs->src--;

    nbits -= bs->left;
    bs->left = 16; /* 16 unused (and some used) bits left in the word */
  }

  /* Shift nbits off the word and return them */
  bs->left -= nbits;
  bs->word <<= nbits;
  return bs->word >> 16;
}

static int decompressS404(UBYTE *src, UBYTE *orgdst,
			  SLONG dst_length, SLONG src_length)
{
  UWORD w;
  SLONG eff;
  SLONG n;
  UBYTE *dst;
  SLONG oLen = dst_length;
  struct bitstream bs;
  int x;

  dst = orgdst + oLen;

  eff = initGetb(&bs, src, src_length);

  while (oLen > 0) {
    x = getb(&bs, 9);
    /* Sanity check */
    if (x < 0) {
      return -1;
    }

    w = x;

    if (w < 0x100) {
      if (orgdst >= dst) {
        return -1;
      }
      *--dst = w;
      oLen--;
    } else if (w == 0x13e || w == 0x13f) {
      w <<= 4;
      x = getb(&bs, 4);
      /* Sanity check */
      if (x < 0) {
        return -1;
      }
      w |= x;

      n = (w & 0x1f) + 14;
      oLen -= n;
      while (n-- > 0) {
        x = getb(&bs, 8);
        /* Sanity check */
        if (x < 0) {
          return -1;
        }
        w = x;

        if (orgdst >= dst) {
          return -1;
        }

        *--dst = w;
      }
    } else {
      if (w >= 0x180) {
        /* copy 2-3 */
        n = w & 0x40 ? 3 : 2;

        if (w & 0x20) {
          /* dist 545 -> */
          w = (w & 0x1f) << (eff - 5);
          x = getb(&bs, eff - 5);
          /* Sanity check */
          if (x < 0) {
            return -1;
          }
          w |= x;
          w += 544;
        } else if (w & 0x30) {
          /* dist 1 -> 32 */
          w = (w & 0x0f) << 1;
          x = getb(&bs, 1);
          /* Sanity check */
          if (x < 0) {
            return -1;
          }
          w |= x;
        } else {
          /* dist 33 -> 544 */
          w = (w & 0x0f) << 5;
          x = getb(&bs, 5);
          /* Sanity check */
          if (x < 0) {
            return -1;
          }
          w |= x;
          w += 32;
        }
      } else if (w >= 0x140) {
        /* copy 4-7 */
        n = ((w & 0x30) >> 4) + 4;

        if (w & 0x08) {
          /* dist 545 -> */
          w = (w & 0x07) << (eff - 3);
          x = getb(&bs, eff - 3);
          /* Sanity check */
          if (x < 0) {
            return -1;
          }
          w |= x;
          w += 544;
        } else if (w & 0x0c) {
          /* dist 1 -> 32 */
          w = (w & 0x03) << 3;
          x = getb(&bs, 3);
          /* Sanity check */
          if (x < 0) {
            return -1;
          }
          w |= x;
        } else {
          /* dist 33 -> 544 */
          w = (w & 0x03) << 7;
          x = getb(&bs, 7);
          /* Sanity check */
          if (x < 0) {
            return -1;
          }
          w |= x;
          w += 32;
        }
      } else if (w >= 0x120) {
        /* copy 8-22 */
        n = ((w & 0x1e) >> 1) + 8;

        if (w & 0x01) {
          /* dist 545 -> */
          x = getb(&bs, eff);
          /* Sanity check */
          if (x < 0) {
            return -1;
          }
          w = x;
          w += 544;
        } else {
          x = getb(&bs, 6);
          /* Sanity check */
          if (x < 0) {
            return -1;
          }
          w = x;

          if (w & 0x20) {
            /* dist 1 -> 32 */
            w &= 0x1f;
          } else {
            /* dist 33 -> 544 */
            w <<= 4;
            x = getb(&bs, 4);
            /* Sanity check */
            if (x < 0) {
              return -1;
            }
            w |= x;
            w += 32;
          }
        }
      } else {
        w = (w & 0x1f) << 3;
        x = getb(&bs, 3);
        /* Sanity check */
        if (x < 0) {
          return -1;
        }
        w |= x;
        n = 23;

        while (w == 0xff) {
          n += w;
          x = getb(&bs, 8);
          /* Sanity check */
          if (x < 0) {
            return -1;
          }
          w = x;
        }
        n += w;

        x = getb(&bs, 7);
        w = x;

        if (w & 0x40) {
          /* dist 545 -> */
          w = (w & 0x3f) << (eff - 6);
          x = getb(&bs, eff - 6);
          /* Sanity check */
          if (x < 0) {
            return -1;
          }
          w |= x;
          w += 544;
        } else if (w & 0x20) {
          /* dist 1 -> 32 */
          w &= 0x1f;
        } else {
          /* dist 33 -> 544; */
          w <<= 4;
          x = getb(&bs, 4);
          /* Sanity check */
          if (x < 0) {
            return -1;
          }
          w |= x;
          w += 32;
        }
      }

      oLen -= n;

      while (n-- > 0) {
        dst--;
        if (dst < orgdst || (dst + w + 1) >= (orgdst + dst_length))
            return -1;
        *dst = dst[w + 1];
      }
    }
  }

  return 0;
}

BOOL S404_Unpack(MREADER *reader, void **out, long *outlen)
{
	SLONG iLen, sLen, oLen, pLen;
	UBYTE *src, *dst = NULL;
	int err;

	_mm_fseek(reader,0,SEEK_END);
	iLen = _mm_ftell(reader);
	if (iLen <= 16) return 0;

	_mm_rewind(reader);
	if (_mm_read_M_ULONG(reader) != 0x53343034) /* S404 */
		return 0;

	sLen = _mm_read_M_SLONG(reader); /* Security length */
	oLen = _mm_read_M_SLONG(reader); /* Depacked length */
	pLen = _mm_read_M_SLONG(reader); /* Packed length */
#ifdef STC_DEBUG
	fprintf(stderr,"S404: iLen= %d, sLen= %d, pLen= %d, oLen= %d\n",
		iLen, sLen, pLen, oLen);
#endif
	if (sLen < 0 || oLen <= 0 || pLen <= 0) return 0;
	if (pLen + 16 >= iLen) return 0; /* sanity check */

	if (!(src = (UBYTE*) MikMod_malloc(iLen - 16)))
		return 0;
	if (!(dst = (UBYTE*) MikMod_malloc(oLen))) {
		MikMod_free(src);
		return 0;
	}

	_mm_read_UBYTES(src, iLen - 16, reader);
	err = decompressS404(src, dst, oLen, pLen);
	MikMod_free(src);

	if (!err) {
		*out = dst;
		*outlen = oLen;
		return 1;
	}

	MikMod_free(dst);
	return 0;
}
