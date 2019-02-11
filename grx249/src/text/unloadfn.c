/**
 ** unloadfn.c ---- remove a font from memory
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu]
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
#include "arith.h"

void GrUnloadFont(GrFont *f)
{
	if((f != NULL) && !f->h.preloaded) {
	    unsigned int i;
	    free(f->h.name);
	    free(f->h.family);
	    farfree(f->bitmap);
	    if(f->auxmap) farfree(f->auxmap);
	    for(i = 0; i < itemsof(f->auxoffs); i++) {
		if(f->auxoffs[i]) free(f->auxoffs[i]);
	    }
	    free(f);
	}
}

