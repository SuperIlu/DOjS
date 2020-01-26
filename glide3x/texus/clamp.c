/*
** Copyright (c) 1996, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "texusint.h"

static void _txImgClamp( FxU32 *out, int ox, int oy,
                         const FxU32 *in, int ix, int iy )
{
  int x, y;

  if( txVerbose )
    {
      printf( "clamping from %dx%d to %dx%d\n", 
              ix, iy, ox, oy );
    }

  for( y = 0; y < oy; y++ )
    {
      for( x = 0; x < ox; x++ )
        {
          out[y*ox+x] = in[ ( ( y < iy  )? y : ( iy - 1 ) ) * ix + ( ( x < ix ) ? x : ( ix - 1 ) ) ];
        }
    }
}

void txMipClamp( TxMip *dstMip, TxMip *srcMip )
{
  int i, sw, sh, dw, dh;

  if( dstMip->format != srcMip->format )
    {
      txPanic( "Image formats must be the same in txMipClamp." );
    }

  if( dstMip->format != GR_TEXFMT_ARGB_8888 )
    {
      txPanic( "txMipClamp only works on GR_TEXFMT_ARGB_8888 images." );
    }

  if( ( dstMip->width == srcMip->width ) && ( dstMip->height == srcMip->height ) &&
      ( dstMip->data[0] == srcMip->data[0] ) )
    {
      if( txVerbose )
        {
          printf("No Clamping necessary.\n");
        }
      return;
    }

  if ((srcMip->data[0] == NULL) || (dstMip->data[0] == NULL))
    txPanic("txImageClamp: Null buffer\n");
  
  sw = srcMip->width;
  sh = srcMip->height;
  dw = dstMip->width;
  dh = dstMip->height;

  for( i = 0; i < srcMip->depth; i++ ) 
    {
      if( !dstMip->data[i] )
        txPanic("txImageResize: no miplevel present\n");
      _txImgClamp( dstMip->data[i], dw, dh,
                   srcMip->data[i], sw, sh );
      if( txVerbose )
        {
          printf(" %dx%d", sw, sh); fflush(stdout);
        }

      if (sw > 1) sw >>= 1;
      if (sh > 1) sh >>= 1;
      if (dw > 1) dw >>= 1;
      if (dh > 1) dh >>= 1;
    }
  if( txVerbose )
    {
      printf(".\n");
    }
}
                 
