/*
	File:		fxconsole.c

	Contains:	Converts console writes to error dialogs.

	Written by:	miklos

	Copyright:	Copyright © 1999 Mikl—s Fazekas. All rights reversed.

	Change History (most recent first):

         <3>      7/5/99    miklos  Revison.
         <2>      7/4/99    miklos  Revision.
*/

/*
 * We use this file beacuse the FXMesa error output is:
 * printf(stderr,....).
 * What we can do is:
 * a. Print to an opengl.err file
 * b. Display dialogs. And Quit the program.
 *
 * Currently b. is implemented, but only for appereance.
 */


#include "fxgli.h"
#include "fxMesa.h"
#include "fxdrv.h"
#include "CodeFragments.h"
#include "Dialogs.h"


#ifndef __CONSOLE__
#include <console.h>
#endif
#include <stdio.h>
/*
 *	The following four functions provide the UI for the console package.
 *	Users wishing to replace SIOUX with their own console package need
 *	only provide the four functions below in a library.
 */

/*
 *	extern short InstallConsole(short fd);
 *
 *	Installs the Console package, this function will be called right
 *	before any read or write to one of the standard streams.
 *
 *	short fd:		The stream which we are reading/writing to/from.
 *	returns short:	0 no error occurred, anything else error.
 */
Boolean error = false;
short InstallConsole(short fd)
{	

	return 0;
}

/*
 *	extern void RemoveConsole(void);
 *
 *	Removes the console package.  It is called after all other streams
 *	are closed and exit functions (installed by either atexit or _atexit)
 *	have been called.  Since there is no way to recover from an error,
 *	this function doesn't need to return any.
 */

void RemoveConsole(void)
{
}

/*
 *	extern long WriteCharsToConsole(char *buffer, long n);
 *
 *	Writes a stream of output to the Console window.  This function is
 *	called by write.
 *
 *	char *buffer:	Pointer to the buffer to be written.
 *	long n:			The length of the buffer to be written.
 *	returns short:	Actual number of characters written to the stream,
 *					-1 if an error occurred.
 */

long WriteCharsToConsole(char *buffer, long n)
{
    char str[255];
    char line[255];
    SInt16 choice;
	AlertStdAlertParamRec params = {false,false,NULL,"\pQuit",NULL,NULL,kAlertStdAlertOKButton,0,kWindowDefaultPosition};

		
	/* First deactivate the screen to show error messages to the user */
	FX_grSstControl(GR_CONTROL_DEACTIVATE);
	if (current_context)
	{
		/* Detach the device for some memory... */
		gliAttachDrawable((GLIContext)current_context,GLI_NONE,NULL);
	
		/* Then show the dialog */ 	
		if (n > 100)
			n = 100;
		strncpy(str,buffer,n);
		str[n] = 0;
		sprintf(&line[1],"Error: %s\r Probably there is not enough memory for Mesa3DfxEngine! Try to quit other applications or lower(!) the applications memory settings!",str);
		line[0] = strlen(&line[1]);


		if ( (UInt32)StandardAlert == (UInt32)kUnresolvedCFragSymbolAddress )
 			return 0;
			StandardAlert(kAlertStopAlert,
			"\pMesa3DfxEngine: Fatal Error!",
			(void*)line,
			&params,
			&choice);

		exit(-1);
	}
	fxCloseHardware();
	return 0;

}

/*
 *	extern long ReadCharsFromConsole(char *buffer, long n);
 *
 *	Reads from the Console into a buffer.  This function is called by
 *	read.
 *
 *	char *buffer:	Pointer to the buffer which will recieve the input.
 *	long n:			The maximum amount of characters to be read (size of
 *					buffer).
 *	returns short:	Actual number of characters read from the stream,
 *					-1 if an error occurred.
 */

long ReadCharsFromConsole(char *buffer, long n)
{
#pragma unused (buffer, n)

	return 0;
}

/*
 *	extern char *__ttyname(long fildes);
 *
 *	Return the name of the current terminal (only valid terminals are
 *	the standard stream (ie stdin, stdout, stderr).
 *
 *	long fildes:	The stream to query.
 *
 *	returns char*:	A pointer to static global data which contains a C string
 *					or NULL if the stream is not valid.
 */

extern char *__ttyname(long fildes)
{
#pragma unused (fildes)
	/* all streams have the same name */
	static char *__devicename = "null device";

	if (fildes >= 0 && fildes <= 2)
		return (__devicename);

	return (0L);
}

/*     Change record
*/
