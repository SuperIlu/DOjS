/*
	File:		Mesa3DGraphicsLibrary.r

	Contains:	Version + Help files for Mesa3DGraphicsLibrary.

	Written by:	miklos

	Copyright:	Copyright ©

	Change History (most recent first):

        <1+>      6/8/99    ???     Initial revision.
         <1>      6/8/99    ???     Initial revision.
*/
#include "MacTypes.r"

/*------------------------------ version ------------------------ */
resource 'vers' (1, purgeable) {
	MESA_MAJOR_VERSION,
	MESA_MINOR_VERSION,
	MESA_RELEASE_STAGE,
	1,
	0,
	MESA_SHORT_VERSION_STRING,
	MESA_LONG_VERSION_STRING
	
};

resource 'vers' (2, purgeable) {
	MESA_MAJOR_VERSION,
	MESA_MINOR_VERSION,
	MESA_RELEASE_STAGE,
	1,
	0,
	MESA_SHORT_VERSION_STRING,
	MESA_SECOND_VERSION_STRING
};
