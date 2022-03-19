/*
	File:		maglAlert.c

	Contains:	Alerts the User if Mesa is used with a program not mean for Mesa.

	Written by:	Miklos Fazekas

	Copyright:	Copyright(C) 1995-97 Miklos Fazekas
				E-Mail: boga@valerie.inf.elte.hu
				WWW: http://www.mesa3d.org/mac/Mesa.html
				
				Part of the Mesa 3D Graphics Library for MacOS.

	Change History (most recent first):

         <2>      5/2/99    miklos  Reads for MESA_WARNING
         <1>     4/10/99    miklos  Initial revision.
*/


/* This file will alert when Mesa is not installed */


#include "maglConfig.h"
#include "CodeFragments.h"
#include "Dialogs.h"
#include "mgliGetEnv.h"

void AlertUser()
{
    is_mesa_enabled = GL_TRUE;
#if 0
	SInt16 choice;
	AlertStdAlertParamRec params = {false,false,NULL,"\pContinue","\pQuit",NULL,kAlertStdAlertOKButton,kAlertStdAlertCancelButton,kWindowDefaultPosition};


	if (MCFG_getEnv("MESA_DISABLE_WARNING") != NULL)
	{
		is_mesa_enabled = GL_TRUE;
		return;
	}
	

	StandardAlert(kAlertCautionAlert,
			"\pYou are using the Mesa Libraries!",
			"\pIf you continue this program might crash! Remove Mesa3DLibrary and Mesa3DUtility from your Extensions Folder!",
			&params,
			&choice);
	if (choice == kAlertStdAlertCancelButton)
		is_mesa_enabled = GL_FALSE;
	else
		is_mesa_enabled = GL_TRUE;
#endif
}