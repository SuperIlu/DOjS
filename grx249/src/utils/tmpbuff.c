/**
 ** tmpbuff.c ---- temporary buffer support
 **
 ** Copyright (c) 1998 Hartmut Schirmer
 **
 ** This file is part of the GRX graphics library.
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

#include "libgrx.h"
#include "allocate.h"

void far *_GrTempBuffer = NULL;
unsigned  _GrTempBufferBytes = 0;

void far *_GrTempBufferAlloc_(size_t bytes) {
  GRX_ENTER();
  if (bytes > _GrTempBufferBytes || _GrTempBuffer == NULL) {
    void far *neu = farrealloc(_GrTempBuffer, bytes);
    if (neu) {
      _GrTempBuffer = neu;
      _GrTempBufferBytes = bytes;
    }
  }
  GRX_RETURN( (bytes<=_GrTempBufferBytes && _GrTempBuffer)
	     ? _GrTempBuffer : NULL);
}

void _GrTempBufferFree(void) {
  if (_GrTempBuffer) farfree(_GrTempBuffer);
  _GrTempBuffer = NULL;
  _GrTempBufferBytes = 0;
}
