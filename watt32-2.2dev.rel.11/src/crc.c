/*!\file crc.c
 *  CRC calculation.
 *
 *  These CRC functions are derived from code in chapter 19 of the book
 *  "C Programmer's Guide to Serial Communications", by Joe Campbell.
 *
 *  Only used by the language module (ref. language.l)
 */

#include <stdio.h>
#include <stdlib.h>

#include "wattcp.h"
#include "misc.h"

#define CRC_BITS    32
#define CRC_HIBIT   ((DWORD) (1L << (CRC_BITS-1)))
#define CRC_SHIFTS  (CRC_BITS-8)

/* Our PRZ's 24-bit CRC generator polynomial. Ref:
 *   http://en.wikipedia.org/wiki/Cyclic_redundancy_check
 *   Section "Commonly used and standardized CRCs"
 */
#define CRC_PRZ     0x864CFBL

DWORD *crc_table;    /* Pre-generated table for speeding up CRC calculations. */

/*
 * mk_crctbl() derives a CRC lookup table from the CRC polynomial.
 * The table is used later by crc_bytes() below.
 * mk_crctbl() only needs to be called once at the dawn of time.
 * I.e. in crc_init().
 *
 * The theory behind mk_crctbl() is that table[i] is initialized
 * with the CRC of i, and this is related to the CRC of `i >> 1',
 * so the CRC of `i >> 1' (pointed to by p) can be used to derive
 * the CRC of i (pointed to by q).
 */
static void mk_crctbl (DWORD poly, DWORD *tab)
{
  DWORD *p = tab;
  DWORD *q = tab;
  int    i;

  *q++ = 0;
  *q++ = poly;
  for (i = 1; i < 128; i++)
  {
    DWORD t = *(++p);

    if (t & CRC_HIBIT)
    {
      t <<= 1;
      *q++ = t ^ poly;
      *q++ = t;
    }
    else
    {
      t <<= 1;
      *q++ = t;
      *q++ = t ^ poly;
    }
  }
}

/*
 * Calculate 32-bit CRC on buffer 'buf' with length 'len'.
 */
DWORD crc_bytes (const char *buf, size_t len)
{
  DWORD accum;

  for (accum = 0; crc_table && len > 0; len--)
      accum = (accum << 8) ^ crc_table[(BYTE)(accum >> CRC_SHIFTS) ^ *buf++];
  return (accum);
}

BOOL crc_init (void)
{
  if (crc_table)
     return (TRUE);

  crc_table = calloc (sizeof(DWORD), 256);
  if (!crc_table)
     return (FALSE);

  mk_crctbl (CRC_PRZ, crc_table);
  return (TRUE);
}

