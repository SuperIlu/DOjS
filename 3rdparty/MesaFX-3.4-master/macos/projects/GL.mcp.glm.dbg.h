/*
	File:		GL.mcp.dbg.h

	Contains:	Debug header for Mesa.

	Written by:	Miklos Fazekas

	Copyright:	Copyright © 1999 Mikl—s Fazekas. All rights reversed.
    See http://www.mesa3d.org/mac/ for more information.

	Change History (most recent first):

         <4>      4/6/99    miklos  Revision for Mesa 3.1b2
*/


/* Enable debugging of Mesa.... */
#define DEBUG		1

/* ToDo(Remove this) Build the FX version of Mesa. */
#define FX			1 

#include <stdlib.h>

#include "glm.h"
extern char *myAllocate(GLint size);
#define free glmFree
#define calloc glmCalloc
#define malloc 	myAllocate

