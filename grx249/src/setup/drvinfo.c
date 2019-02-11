/**
 ** drvinfo.c ---- the driver info data structure
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
#include "grdriver.h"

#undef GrDriverInfo

static GrColor dummyframefn(void);

const
struct _GR_driverInfo * const GrDriverInfo = &_GrDriverInfo;
struct _GR_driverInfo _GrDriverInfo = {
    NULL,                               /* video driver */
    &DRVINFO->actmode,                  /* current video mode pointer */
    {                                   /* current video mode struct */
	FALSE,                          /* present */
	4,                              /* bpp */
	80,25,                          /* geometry */
	3,                              /* BIOS mode */
	160,                            /* lineoffset */
	0,                              /* private */
#ifdef __MSDOS__
	&_GrViDrvEGAVGAtextModeExt      /* extended info */
#else
        NULL
#endif
    },
    {                                   /* current frame driver */
	GR_frameUndef,                  /* frame mode */
	GR_frameUndef,                  /* compatible RAM frame mode */
	FALSE,                          /* onscreen */
	1,                              /* line width alignment */
	1,                              /* number of planes */
	0,                              /* bits per pixel */
	0L,                             /* max plane size the code can handle */
	NULL,
	(GrColor (*)(GrFrame*,int,int))                               dummyframefn,
	(void (*)(int,int,GrColor))                                   dummyframefn,
	(void (*)(int,int,int,int,GrColor))                           dummyframefn,
	(void (*)(int,int,int,GrColor))                               dummyframefn,
	(void (*)(int,int,int,GrColor))                               dummyframefn,
	(void (*)(int,int,int,int,GrColor))                           dummyframefn,
	(void (*)(int,int,int,int,char*,int,int,GrColor,GrColor))     dummyframefn,
	(void (*)(int,int,int,char,GrColor,GrColor))                  dummyframefn,
	(void (*)(GrFrame*,int,int,GrFrame*,int,int,int,int,GrColor)) dummyframefn,
	(void (*)(GrFrame*,int,int,GrFrame*,int,int,int,int,GrColor)) dummyframefn,
	(void (*)(GrFrame*,int,int,GrFrame*,int,int,int,int,GrColor)) dummyframefn
    },
    {                                   /* screen frame driver */
	GR_frameUndef,                  /* frame mode */
	GR_frameUndef,                  /* compatible RAM frame mode */
	FALSE,                          /* onscreen */
	1,                              /* line width alignment */
	1,                              /* number of planes */
	0,                              /* bits per pixel */
	0L,                             /* max plane size the code can handle */
	NULL,
	(GrColor (*)(GrFrame*,int,int))                               dummyframefn,
	(void (*)(int,int,GrColor))                                   dummyframefn,
	(void (*)(int,int,int,int,GrColor))                           dummyframefn,
	(void (*)(int,int,int,GrColor))                               dummyframefn,
	(void (*)(int,int,int,GrColor))                               dummyframefn,
	(void (*)(int,int,int,int,GrColor))                           dummyframefn,
	(void (*)(int,int,int,int,char*,int,int,GrColor,GrColor))     dummyframefn,
	(void (*)(int,int,int,char,GrColor,GrColor))                  dummyframefn,
	(void (*)(GrFrame*,int,int,GrFrame*,int,int,int,int,GrColor)) dummyframefn,
	(void (*)(GrFrame*,int,int,GrFrame*,int,int,int,int,GrColor)) dummyframefn,
	(void (*)(GrFrame*,int,int,GrFrame*,int,int,int,int,GrColor)) dummyframefn
    },
    {                                   /* dummy text mode frame driver */
	GR_frameText,                   /* frame mode */
	GR_frameUndef,                  /* compatible RAM frame mode */
	TRUE,                           /* onscreen */
	1,                              /* line width alignment */
	1,                              /* number of planes */
	16,                             /* bits per pixel */
	0L,                             /* max plane size the code can handle */
	NULL,
	(GrColor (*)(GrFrame*,int,int))                               dummyframefn,
	(void (*)(int,int,GrColor))                                   dummyframefn,
	(void (*)(int,int,int,int,GrColor))                           dummyframefn,
	(void (*)(int,int,int,GrColor))                               dummyframefn,
	(void (*)(int,int,int,GrColor))                               dummyframefn,
	(void (*)(int,int,int,int,GrColor))                           dummyframefn,
	(void (*)(int,int,int,int,char*,int,int,GrColor,GrColor))     dummyframefn,
	(void (*)(int,int,int,char,GrColor,GrColor))                  dummyframefn,
	(void (*)(GrFrame*,int,int,GrFrame*,int,int,int,int,GrColor)) dummyframefn,
	(void (*)(GrFrame*,int,int,GrFrame*,int,int,int,int,GrColor)) dummyframefn,
	(void (*)(GrFrame*,int,int,GrFrame*,int,int,int,int,GrColor)) dummyframefn
    },
    GR_default_text,                            /* current mode code */
    80,25,                                      /* default text size */
    640,480,                                    /* default graphics size */
    16L,16L,                                    /* default txt and gr colors */
    0,0,                                        /* virtual position */
    TRUE,                                       /* exit upon errors */
    TRUE,                                       /* restore startup mode */
    FALSE,                                      /* split banks */
    (-1),                                       /* current bank */
    NULL,                                       /* mode set hook */
    (void (*)(int)    )_GrDummyFunction,        /* banking func */
    (void (*)(int,int))_GrDummyFunction         /* split banking func */
};

static GrColor dummyframefn(void)
{
	if(DRVINFO->errsfatal) {
	    _GrCloseVideoDriver();
	    fprintf(stderr,
		"GRX Error: graphics operation attempted %s\n",
		(DRVINFO->fdriver.mode == GR_frameText) ? "in text mode" : "before mode set"
	    );
	    exit(1);
	}
	return(GrNOCOLOR);
}

void _GrDummyFunction(void)
{
	return;
}

