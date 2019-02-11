/**
 ** rand.h ---- a very simple random number generator
 **             (from "Numerical recipies")
 **
 ** This is a test/demo file of the GRX graphics library.
 ** You can use GRX test/demo files as you want.
 **
 ** The GRX graphics library is free software; you can redistribute it
 ** and/or modify it under some conditions; see the "copying.grx" file
 ** for details.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 **
 **/

#ifndef __RAND_H_INCLUDED
#define __RAND_H_INCLUDED

#define _IA   16807
#define _IM   2147483647L
#define _IQ   127773L
#define _IR   2836
#define _MASK 123459876UL

static long _idum = 0;

unsigned long ran0(void) {
  long k;
  _idum ^= _MASK;
  k = _idum / _IQ;
  _idum = _IA * (_idum - k * _IQ) - _IR * k;
  if (_idum < 0) _idum += _IM;
  return (unsigned long) _idum;
}

#define sran0(x) do _idum = (x); while(0)

#define RND()    ran0()
#define SRND(x)  sran0(x)
#define RND_MAX  (_MASK)

#endif
