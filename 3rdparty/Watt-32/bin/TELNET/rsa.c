/*
 * RSA implementation just sufficient for ssh client-side
 * initialisation step
 *
 * Rewritten for more speed by Joris van Rantwijk, Jun 1999.
 */

#include "telnet.h"
#include "ssh.h"

#undef byte

typedef WORD Bignum;

static Bignum *newbn (int length)
{
  Bignum *b = malloc ((length+1) * sizeof(Bignum));

  if (!b)
     fatal ("malloc");
  b[0] = length;
  return (b);
}

static void freebn (Bignum *b)
{
  free (b);
}

/*
 * Compute c = a * b.
 * Input is in the first len words of a and b.
 * Result is returned in the first 2*len words of c.
 */
static void bigmul (WORD *a, WORD *b, WORD *c, int len)
{
  int   i, j;
  DWORD ai, t;

  for (j = len-1; j >= 0; j--)
     c[j+len] = 0;

  for (i = len-1; i >= 0; i--)
  {
    ai = a[i];
    t  = 0;
    for (j = len-1; j >= 0; j--)
    {
      t += ai * (DWORD)b[j];
      t += (DWORD)c[i+j+1];
      c[i+j+1] = (WORD)t;
      t = t >> 16;
    }
    c[i] = (WORD)t;
  }
}

/*
 * Compute a = a % m.
 * Input in first 2*len words of a and first len words of m.
 * Output in first 2*len words of a (of which first len words will be zero).
 * The MSW of m MUST have its high bit set.
 */
static void bigmod (WORD *a, WORD *m, int len)
{
  WORD     m0, m1;
  unsigned int h;
  int      i, k;

  /* Special case for len == 1 */
  if (len == 1)
  {
    a[1] = (((long)a[0] << 16) + a[1]) % m[0];
    a[0] = 0;
    return;
  }

  m0 = m[0];
  m1 = m[1];

  for (i = 0; i <= len; i++)
  {
    DWORD t;
    unsigned int q, r, c;

    if (i == 0)
    {
      h = 0;
    }
    else
    {
      h = a[i-1];
      a[i-1] = 0;
    }

    /* Find q = h:a[i] / m0 */
    t = ((DWORD)h << 16) + a[i];
    q = t / m0;
    r = t % m0;

    /* Refine our estimate of q by looking at
     * h:a[i]:a[i+1] / m0:m1
     */
    t = (long)m1 * (long)q;

    if (t > ((DWORD)r << 16) + a[i+1])
    {
      q--;
      t -= m1;
      r  = (r + m0) & 0xffff;    /* overflow? */
      if (r >= (DWORD)m0 && t > ((DWORD)r << 16) + a[i+1])
         q--;
    }

    /* Substract q * m from a[i...]
     */
    c = 0;
    for (k = len-1; k >= 0; k--)
    {
      t  = (long)q * (long)m[k];
      t += c;
      c  = t >> 16;
      if ((WORD)t > a[i+k])
         c++;
      a[i+k] -= (WORD)t;
    }

    /* Add back m in case of borrow
     */
    if (c != h)
    {
      t = 0;
      for (k = len-1; k >= 0; k--)
      {
        t += m[k];
        t += a[i+k];
        a[i+k] = (WORD)t;
        t = t >> 16;
      }
    }
  }
}

/*
 * Compute (base ^ exp) % mod.
 * The base MUST be smaller than the modulus.
 * The most significant word of mod MUST be non-zero.
 * We assume that the result array is the same size as the mod array.
 */
static void modpow (Bignum *base, Bignum *exp, Bignum *mod, Bignum *result)
{
  WORD *a, *b, *n, *m;
  int   mshift;
  int   mlen, i, j;

  /* Allocate m of size mlen, copy mod to m
   * We use big endian internally
   */
  mlen = mod[0];
  m = malloc (mlen * sizeof (WORD));

  for (j = 0; j < mlen; j++)
      m[j] = mod[mod[0]-j];

  /* Shift m left to make msb bit set
   */
  for (mshift = 0; mshift < 15; mshift++)
      if ((m[0] << mshift) & 0x8000)
         break;

  if (mshift)
  {
    for (i = 0; i < mlen-1; i++)
       m[i] = (m[i] << mshift) | (m[i+1] >> (16 - mshift));
    m[mlen-1] = m[mlen-1] << mshift;
  }

  /* Allocate n of size mlen, copy base to n
   */
  n = malloc (mlen * sizeof (WORD));

  i = mlen - base[0];
  for (j = 0; j < i; j++)
      n[j] = 0;
  for (j = 0; j < (int)base[0]; j++)
      n[i+j] = base [base[0]-j];

  /* Allocate a and b of size 2*mlen. Set a = 1
   */
  a = malloc (2 * mlen * sizeof (WORD));
  b = malloc (2 * mlen * sizeof (WORD));

  for (i = 0; i < 2 * mlen; i++)
      a[i] = 0;
  a[2*mlen-1] = 1;

  /* Skip leading zero bits of exp.
   */
  i = 0;
  j = 15;
  while (i < exp[0] && (exp[(int)(exp[0]-i)] & (1 << j)) == 0)
  {
    j--;
    if (j < 0)
    {
      i++;
      j = 15;
    }
  }

  /* Main computation
   */
  while (i < (int)exp[0])
  {
    while (j >= 0)
    {
      bigmul (a+mlen, a+mlen, b, mlen);
      bigmod (b, m, mlen);
      if ((exp[exp[0]-i] & (1 << j)) != 0)
      {
        bigmul (b+mlen, n, a, mlen);
        bigmod (a, m, mlen);
      }
      else
      {
        WORD *t;

        t = a;
        a = b;
        b = t;
      }
      j--;
    }
    i++;
    j = 15;
  }

  /* Fixup result in case the modulus was shifted
   */
  if (mshift)
  {
    for (i = mlen-1; i < 2*mlen - 1; i++)
        a[i] = (a[i] << mshift) | (a[i+1] >> (16 - mshift));
    a[2*mlen-1] = a[2*mlen-1] << mshift;
    bigmod (a, m, mlen);
    for (i = 2*mlen-1; i >= mlen; i--)
        a[i] = (a[i] >> mshift) | (a[i-1] << (16 - mshift));
  }

  /* Copy result to buffer
   */
  for (i = 0; i < mlen; i++)
      result[result[0]-i] = a[i+mlen];

  /* Free temporary arrays
   */
  for (i = 0; i < 2 * mlen; i++)
      a[i] = 0;
  free (a);
  for (i = 0; i < 2 * mlen; i++)
      b[i] = 0;
  free (b);
  for (i = 0; i < mlen; i++)
      m[i] = 0;
  free (m);
  for (i = 0; i < mlen; i++)
      n[i] = 0;
  free (n);
}

int RSAmakekey (BYTE *data, R_RSAKey *result, BYTE **keystr)
{
  BYTE   *p = data;
  Bignum *bn[2];
  int     i, j;
  int     w, b;

  result->bits = 0;
  for (i = 0; i < 4; i++)
      result->bits = (result->bits << 8) + *p++;

  for (j = 0; j < 2; j++)
  {
    w = 0;
    for (i = 0; i < 2; i++)
        w = (w << 8) + *p++;

    result->bytes = b = (w + 7) / 8; /* bits -> bytes */
    w = (w + 15) / 16;          /* bits -> words */

    bn[j] = newbn (w);

    if (keystr)
       *keystr = p;             /* point at key string, second time */

    for (i = 1; i <= w; i++)
        bn[j][i] = 0;
    for (i = b; i--;)
    {
      BYTE byte = *p++;

      if (i & 1)
           bn [j][1+i/2] |= byte << 8;
      else bn [j][1+i/2] |= byte;
    }
  }
  result->exponent = bn[0];
  result->modulus = bn[1];
  return (p - data);
}

void RSAencrypt (BYTE *data, int length, R_RSAKey *key)
{
  Bignum  *b1, *b2;
  BYTE    *p;
  unsigned i, w;

  memmove (data + key->bytes - length, data, length);
  data[0] = 0;
  data[1] = 2;

  for (i = 2; i < key->bytes - length - 1; i++)
  {
    do
    {
      data[i] = rand() % 256;
    }
    while (data[i] == 0);
  }
  data[key->bytes-length-1] = 0;

  w = (key->bytes + 1) / 2;

  b1 = newbn (w);
  b2 = newbn (w);

  p = data;
  for (i = 1; i <= w; i++)
      b1[i] = 0;
  for (i = key->bytes; i--;)
  {
    BYTE byte = *p++;

    if (i & 1)
         b1 [1+i/2] |= byte << 8;
    else b1 [1+i/2] |= byte;
  }
  modpow (b1, key->exponent, key->modulus, b2);
  p = data;
  for (i = key->bytes; i--;)
  {
    BYTE b;

    if (i & 1)
         b = b2 [1+i/2] >> 8;
    else b = b2 [1+i/2] & 0xFF;
    *p++ = b;
  }
  freebn (b1);
  freebn (b2);
}

