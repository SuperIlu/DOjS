/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "texusint.h"


FxBool 
_txReadPPMHeader( FILE *stream, FxU32 cookie, TxMip *info)
{ 
    char buffer[256];
    FxU32 state = 1;
    FxBool done = FXFALSE;
    
    if ( stream == NULL ) {
	txPanic("PPM file: Bad file handle.");
	return FXFALSE;
    }
    
    while( !done && fgets( buffer, 256, stream ) ) {
	char *token;
	
	if ( buffer[0] == '#' ) continue;
	for (token = strtok( buffer, " \t\n\r" ); token != NULL;
			token = strtok( NULL, " \t\n\r" )) {
	    switch( state ) {
	    case 1:	// Width
			info->width = atoi( token );
			state++;
			break;

	    case 2:	// height
			info->height = atoi( token );
			state++;
			break;

	    case 3:	// Color Depth
			info->format = atoi( token );
			if ( info->format != 255 ) {
			    txPanic("Unsupported PPM format: max != 255\n");
		    	    return FXFALSE;
			}
			state++;
			done = FXTRUE;
			break;

	    default:
			txPanic("PPM file: parse error\n");
			return FXFALSE;
			break;
	    }
	}
    }
    
    if ( state < 4 ) {
    	txPanic("PPM file: Read error before end of header.");
	return FXFALSE;
    }
    info->depth = 1;
    info->format = GR_TEXFMT_ARGB_8888;
    info->size = info->width * info->height * 4;
    return FXTRUE;
}

FxBool 
_txReadPPMData( FILE *stream, TxMip *info) 
{ 
    FxU32 numPixels;
    FxU32 *data32 = info->data[0];
    
    numPixels = info->width * info->height;

    if ( stream == NULL ) {
	txPanic("PPM file: Bad file handle.");
	return FXFALSE;
    }
    
    // Read in image data
    while (numPixels--) {
	int r, g, b;

	r = getc( stream );
	g = getc( stream );
	b = getc( stream );
	if ( b == EOF ) {
	    txPanic("PPM file: Unexpected End of File.");
	    return FXFALSE;
	}
	*data32++ = (0xFF << 24) | (r << 16) | (g << 8) | b;
    }
    return FXTRUE;
}
