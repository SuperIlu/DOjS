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

/* MMCMP ("ziRCONia") decompression support
 *
 * Based on the public domain version from the libmodplug library by
 * Olivier Lapicque <olivierl@jps.net> (sezero's fork of libmodplug
 * at github: http://github.com/sezero/libmodplug/tree/sezero )
 *
 * Rewritten for libmikmod by O. Sezer <sezero@users.sourceforge.net>
 * with some extra ideas from the libxmp version by Claudio Matsuoka.
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

/*#define MMCMP_DEBUG*/

typedef struct MMCMPFILEHDR
{
	UBYTE id[8]; /* string 'ziRCONia' */
	UWORD hdrsize; /* sizeof MMCMPHEADER */
} MMCMPFILEHDR; /* 10 bytes */

typedef struct MMCMPHEADER
{
	UWORD version;
	UWORD nblocks;
	ULONG filesize;
	ULONG blktable;
	UBYTE glb_comp;
	UBYTE fmt_comp;
} MMCMPHEADER; /* 14 bytes */

typedef struct MMCMPBLOCK
{
	ULONG unpk_size;
	ULONG pk_size;
	ULONG xor_chk;
	UWORD sub_blk;
	UWORD flags;
	UWORD tt_entries;
	UWORD num_bits;
} MMCMPBLOCK; /* 20 bytes */

typedef struct MMCMPSUBBLOCK
{
	ULONG unpk_pos;
	ULONG unpk_size;
} MMCMPSUBBLOCK; /* 8 bytes */

#define MMCMP_COMP	0x0001
#define MMCMP_DELTA	0x0002
#define MMCMP_16BIT	0x0004
#define MMCMP_STEREO	0x0100
#define MMCMP_ABS16	0x0200
#define MMCMP_ENDIAN	0x0400


typedef struct MMCMPBITBUFFER
{
	ULONG bitcount;
	ULONG bitbuffer;
	const UBYTE *start;
	const UBYTE *end;
} MMCMPBITBUFFER;

static ULONG MMCMP_GetBits(MMCMPBITBUFFER* b, ULONG nBits)
{
	ULONG d;
	if (!nBits) return 0;
	while (b->bitcount < 24)
	{
		b->bitbuffer |= ((b->start < b->end) ? *b->start++ : 0) << b->bitcount;
		b->bitcount += 8;
	}
	d = b->bitbuffer & ((1 << nBits) - 1);
	b->bitbuffer >>= nBits;
	b->bitcount -= nBits;
	return d;
}

static const ULONG MMCMP8BitCommands[8] =
{
	0x01, 0x03,	0x07, 0x0F,	0x1E, 0x3C,	0x78, 0xF8
};

static const ULONG MMCMP8BitFetch[8] =
{
	3, 3, 3, 3, 2, 1, 0, 0
};

static const ULONG MMCMP16BitCommands[16] =
{
	0x01, 0x03,	0x07, 0x0F,	0x1E, 0x3C,	0x78, 0xF0,
	0x1F0, 0x3F0, 0x7F0, 0xFF0, 0x1FF0, 0x3FF0, 0x7FF0, 0xFFF0
};

static const ULONG MMCMP16BitFetch[16] =
{
	4, 4, 4, 4, 3, 2, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};


BOOL MMCMP_Unpack(MREADER* reader, void** out, long* outlen)
{
	ULONG srclen, destlen;
	UBYTE *destbuf, *destptr, *destend;
	MMCMPHEADER mmh;
	ULONG *pblk_table;
	MMCMPSUBBLOCK *subblocks;
	ULONG i, blockidx, numsubs;
	UBYTE *buf;  ULONG bufsize;

	_mm_fseek(reader,0,SEEK_END);
	srclen = _mm_ftell(reader);
	if (srclen < 256) return 0;

	_mm_rewind(reader);
	if (_mm_read_I_ULONG(reader) != 0x4352697A)	/* 'ziRC' */
		return 0;
	if (_mm_read_I_ULONG(reader) != 0x61694e4f)	/* 'ONia' */
		return 0;
	if (_mm_read_I_UWORD(reader) != 14)		/* header size */
		return 0;

	mmh.version = _mm_read_I_UWORD(reader);
	mmh.nblocks = _mm_read_I_UWORD(reader);
	mmh.filesize = _mm_read_I_ULONG(reader);
	mmh.blktable = _mm_read_I_ULONG(reader);
	mmh.glb_comp = _mm_read_UBYTE(reader);
	mmh.fmt_comp = _mm_read_UBYTE(reader);

	if ((!mmh.nblocks) || (mmh.filesize < 16) || (mmh.filesize > 0x8000000) ||
	    (mmh.blktable >= srclen) || (mmh.blktable + 4*mmh.nblocks > srclen)) {
		return 0;
	}
	destlen = mmh.filesize;
	numsubs = 32;
	bufsize = 65536;

	destbuf = (UBYTE*)MikMod_malloc(destlen);
	buf = (UBYTE*)MikMod_malloc(bufsize);
	pblk_table = (ULONG*)MikMod_malloc(mmh.nblocks*4);
	subblocks = (MMCMPSUBBLOCK*)MikMod_malloc(numsubs*sizeof(MMCMPSUBBLOCK));
	if (!destbuf || !buf || !pblk_table || !subblocks)
		goto err;
	destend = destbuf + destlen;

	_mm_fseek(reader,mmh.blktable,SEEK_SET);
	for (blockidx = 0; blockidx < mmh.nblocks; blockidx++) {
		pblk_table[blockidx] = _mm_read_I_ULONG(reader);
	}

	for (blockidx = 0; blockidx < mmh.nblocks; blockidx++)
	{
		ULONG srcpos = pblk_table[blockidx];
		MMCMPBLOCK block;

		if (srcpos + 20 >= srclen) goto err;

		_mm_fseek(reader,srcpos,SEEK_SET);
		block.unpk_size = _mm_read_I_ULONG(reader);
		block.pk_size = _mm_read_I_ULONG(reader);
		block.xor_chk = _mm_read_I_ULONG(reader);
		block.sub_blk = _mm_read_I_UWORD(reader);
		block.flags = _mm_read_I_UWORD(reader);
		block.tt_entries = _mm_read_I_UWORD(reader);
		block.num_bits = _mm_read_I_UWORD(reader);

		if (!block.unpk_size || !block.pk_size || !block.sub_blk)
			goto err;
		if (block.pk_size <= block.tt_entries)
			goto err;
		if (block.flags & MMCMP_COMP) {
			if (block.flags & MMCMP_16BIT) {
				if (block.num_bits >= 16)
					goto err;
			}
			else {
				if (block.num_bits >=  8)
					goto err;
			}
		}

		srcpos += 20 + block.sub_blk*8;
		if (srcpos >= srclen) goto err;

		if (numsubs < block.sub_blk) {
			numsubs = (block.sub_blk + 31) & ~31;
			MikMod_free(subblocks);
			subblocks = (MMCMPSUBBLOCK*)MikMod_malloc(numsubs*sizeof(MMCMPSUBBLOCK));
			if (!subblocks) goto err;
		}
		for (i = 0; i < block.sub_blk; i++) {
			subblocks[i].unpk_pos = _mm_read_I_ULONG(reader);
			subblocks[i].unpk_size = _mm_read_I_ULONG(reader);
			if (subblocks[i].unpk_pos >= destlen) goto err;
			if (subblocks[i].unpk_size > destlen - subblocks[i].unpk_pos) goto err;
		}

#ifdef MMCMP_DEBUG
		fprintf(stderr, "block %u: flags=%04X sub_blocks=%u",
				  blockidx, (unsigned)block.flags, (unsigned)block.sub_blk);
		fprintf(stderr, " pksize=%u unpksize=%u", block.pk_size, block.unpk_size);
		fprintf(stderr, " tt_entries=%u num_bits=%u\n", block.tt_entries, block.num_bits);
#endif
		if (!(block.flags & MMCMP_COMP))
		{ /* Data is not packed */
			_mm_fseek(reader,srcpos,SEEK_SET);
			destptr = destbuf + subblocks[0].unpk_pos;
			i = 0;

			while (1) {
#ifdef MMCMP_DEBUG
				fprintf(stderr, "  Unpacked sub-block %u: offset %u, size=%u\n",
						i, subblocks[i].unpk_pos, subblocks[i].unpk_size);
#endif
				_mm_read_UBYTES(destptr,subblocks[i].unpk_size,reader);
				destptr += subblocks[i].unpk_size;
				if (++i == block.sub_blk) break;
			}
		}
		else if (block.flags & MMCMP_16BIT)
		{ /* Data is 16-bit packed */
			MMCMPBITBUFFER bb;
			ULONG size;
			ULONG pos = 0;
			ULONG numbits = block.num_bits;
			ULONG oldval = 0;

#ifdef MMCMP_DEBUG
			fprintf(stderr, "  16-bit block: pos=%u size=%u ",
					subblocks[0].unpk_pos, subblocks[0].unpk_size);
			if (block.flags & MMCMP_DELTA) fprintf(stderr, "DELTA ");
			if (block.flags & MMCMP_ABS16) fprintf(stderr, "ABS16 ");
			fprintf(stderr, "\n");
#endif
			size = block.pk_size - block.tt_entries;
			if (bufsize < size) {
				while (bufsize < size) bufsize += 65536;
				MikMod_free(buf);
				if (!(buf = (UBYTE*)MikMod_malloc(bufsize)))
					goto err;
			}

			bb.bitcount = 0;
			bb.bitbuffer = 0;
			bb.start = buf;
			bb.end = buf + size;

			_mm_fseek(reader,srcpos+block.tt_entries,SEEK_SET);
			_mm_read_UBYTES(buf,size,reader);
			destptr = destbuf + subblocks[0].unpk_pos;
			size = subblocks[0].unpk_size;
			i = 0;

			while (1)
			{
				ULONG newval = 0x10000;
				ULONG d = MMCMP_GetBits(&bb, numbits+1);

				if (d >= MMCMP16BitCommands[numbits])
				{
					ULONG nFetch = MMCMP16BitFetch[numbits];
					ULONG newbits = MMCMP_GetBits(&bb, nFetch) + ((d - MMCMP16BitCommands[numbits]) << nFetch);
					if (newbits != numbits)
					{
						numbits = newbits & 0x0F;
					} else
					{
						if ((d = MMCMP_GetBits(&bb, 4)) == 0x0F)
						{
							if (MMCMP_GetBits(&bb, 1)) break;
							newval = 0xFFFF;
						} else
						{
							newval = 0xFFF0 + d;
						}
					}
				} else
				{
					newval = d;
				}
				if (newval < 0x10000)
				{
					newval = (newval & 1) ? (ULONG)(-(SLONG)((newval+1) >> 1)) : (ULONG)(newval >> 1);
					if (block.flags & MMCMP_DELTA)
					{
						newval += oldval;
						oldval = newval;
					} else
					if (!(block.flags & MMCMP_ABS16))
					{
						newval ^= 0x8000;
					}
					if (destend - destptr < 2) goto err;
					pos += 2;
					*destptr++ = (UBYTE) (((UWORD)newval) & 0xff);
					*destptr++ = (UBYTE) (((UWORD)newval) >> 8);
				}
				if (pos >= size)
				{
					if (++i == block.sub_blk) break;
					size = subblocks[i].unpk_size;
					destptr = destbuf + subblocks[i].unpk_pos;
					pos = 0;
				}
			}
		}
		else
		{ /* Data is 8-bit packed */
			MMCMPBITBUFFER bb;
			ULONG size;
			ULONG pos = 0;
			ULONG numbits = block.num_bits;
			ULONG oldval = 0;
			UBYTE ptable[0x100];

			size = block.pk_size - block.tt_entries;
			if (bufsize < size) {
				while (bufsize < size) bufsize += 65536;
				MikMod_free(buf);
				if (!(buf = (UBYTE*)MikMod_malloc(bufsize)))
					goto err;
			}

			bb.bitcount = 0;
			bb.bitbuffer = 0;
			bb.start = buf;
			bb.end = buf + size;

			_mm_read_UBYTES(ptable,0x100,reader);
			_mm_fseek(reader,srcpos+block.tt_entries,SEEK_SET);
			_mm_read_UBYTES(buf,size,reader);
			destptr = destbuf + subblocks[0].unpk_pos;
			size = subblocks[0].unpk_size;
			i = 0;

			while (1)
			{
				ULONG newval = 0x100;
				ULONG d = MMCMP_GetBits(&bb, numbits+1);

				if (d >= MMCMP8BitCommands[numbits])
				{
					ULONG nFetch = MMCMP8BitFetch[numbits];
					ULONG newbits = MMCMP_GetBits(&bb, nFetch) + ((d - MMCMP8BitCommands[numbits]) << nFetch);
					if (newbits != numbits)
					{
						numbits = newbits & 0x07;
					} else
					{
						if ((d = MMCMP_GetBits(&bb, 3)) == 7)
						{
							if (MMCMP_GetBits(&bb, 1)) break;
							newval = 0xFF;
						} else
						{
							newval = 0xF8 + d;
						}
					}
				} else
				{
					newval = d;
				}
				if (newval < 0x100)
				{
					int n = ptable[newval];
					if (block.flags & MMCMP_DELTA)
					{
						n += oldval;
						oldval = n;
					}
					destptr[pos++] = (UBYTE)n;
				}
				if (pos >= size)
				{
					if (++i == block.sub_blk) break;
					size = subblocks[i].unpk_size;
					destptr = destbuf + subblocks[i].unpk_pos;
					pos = 0;
				}
			}
		}
	}

	MikMod_free(buf);
	MikMod_free(pblk_table);
	MikMod_free(subblocks);
	*out = destbuf;
	*outlen = destlen;
	return 1;

  err:
	MikMod_free(buf);
	MikMod_free(pblk_table);
	MikMod_free(subblocks);
	MikMod_free(destbuf);
	return 0;
}
