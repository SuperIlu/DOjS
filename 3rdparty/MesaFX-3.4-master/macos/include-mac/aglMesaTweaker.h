/*
	File:		fxgli_renderer.c

	Contains:	Special interface for the MesaTweaker application.

	Written by:	miklos

	Copyright:	Copyright © 1999 All right reversed.

	Change History (most recent first):

         <2>      7/4/99    miklos  Revision.
*/



#ifndef __AGL_MESA_TWEAKE_H__
#define __AGL_MESA_TWEAKE_H__

#include "aglMesa.h"
/*
** This defines the special interface between Mesa+tweaker.
*/
#define AGL_MESA_3DFX_SET_CONFIGURATION_FILE	0x1101D

typedef struct AGLMesaConfigurationFile {
	int lines_num;
	char **lines;
} AGLMesaConfigurationFile;

#endif