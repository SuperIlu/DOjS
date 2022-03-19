#include "CodeFragments.h"
#include "Dialogs.h"

#include <Sound.h>


#include "myalert.h"
#include <string.h>
/*********************************************************************************
 *
 * ShowStopAlert(char *error,char *description);
 *
 *********************************************************************************/
void ShowStopAlert(char *error,char *description)
{
	SInt16 choice;
	char err[256];
	char desc[256];
	AlertStdAlertParamRec params = {false,false,NULL,"\pQuit",NULL,NULL,kAlertStdAlertOKButton,0,kWindowDefaultPosition};
	
	if (strlen(error) > 255)
		return;
	if (strlen(description) > 255)
		return;
	strcpy(&(err[1]),error);
	strcpy(&(desc[1]),description);
	err[0] = strlen(error);
	desc[0] = strlen(description);
	if (((UInt32)StandardAlert) == (kUnresolvedCFragSymbolAddress))
		SysBeep(0);
	else
		StandardAlert(kAlertCautionAlert,
			(StringPtr)err,
			(StringPtr)desc,
			&params,
			&choice);

}
