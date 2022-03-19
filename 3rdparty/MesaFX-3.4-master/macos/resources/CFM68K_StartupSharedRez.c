// This code is degenerated from Metrowerks' "CFM68K_StartupSharedLib.c". 
// It is a hack I made in order to implement a "shared resource" architecture.
// Note that you MUST NOT change any resources in the shared resource fork.
// Doing so will most likely end in a crash.
//
// Steven Bytnar <mailto:s-bytnar@uiuc.edu>  6/14/97
// 6/29/97 - This code can now be added to ANY shared library, without having
//				 to make any code modifications. Removed the #pragma unused(theInitBlock) line.

/*
 *	CFM68K_StartupSharedLib.c	-	init/startup/termination routines for Metrowerks C++ (CFM68K)
 *
 *	Copyright © 1995 metrowerks inc. All Rights Reserved.
 *
 *
 *	THEORY OF OPERATION
 *
 *	This version of the PPC startup code is intended to be used for shared library
 *	components of multi-fragment applications. It is designed to properly support
 *	exceptions thrown across fragment boundaries, and proper initialization and
 *	termination--including construction and destruction of static objects--for
 *	a shared library that is one component of a complex, multi-fragment program.
 *
 *	The startup file defines 2 entry-points:
 *
 *		__initialize	This must be the Initialization Entry-Point for the Application.
 *						__initialize() registers the exception-handling info for
 *						the application, and calls all static initializers.
 *
 *		__terminate		This must be the Termination Entry-Point for the Application
 *						__terminate() unregisters the exception-handling info for the
 *						shared application before the fragment is unloaded.
 *
 *	The new zero-runtime-overhead exception-handling mechanism requires that we
 *	keep track of all loaded fragments and their code/data section ranges--we
 *	cannot use the OS data structures to do this--so we must call routines which
 *	register and unregister the exception-handling info as a fragment is loaded/unloaded.
 *
 *	When the shared library is loaded, __InitCode__() will call constructors for static
 *	objects, linking them into the (per-application) global destructor chain. These
 *	objects will be destroyed when the application component of the program calls
 *	__destroy_global_chain() as part of its exit() handling. This will ensure the
 *	proper ordering of destruction wrt calling _atexit() handlers, cleaning up the
 *	I/O and console packages, etc.
 *
 *	This startup file must -not- be used for shared libraries with shared data sections,
 *	since these may be linked into multiple applications simultaneously, and cannot
 *	have its static objects linked into the application's global destructor chain.
 *
 */


#define IsFileLocation(where) 	\
	( ((where) == kDataForkCFragLocator)	||  \
	((where) == kResourceCFragLocator)	||  \
	((where) == kByteStreamCFragLocator) )

#include <CodeFragments.h>
#include <Exception68K.h>
#include <NMWException.h>
#include <Types.h>

	/*	external data	*/

extern far segment_map SEGMAPNAME[];

	/*	private data	*/

static long fragmentID;						/*	ID given to fragment by exception-handling	*/


	/*	prototypes	*/

#ifdef __cplusplus
extern "C" {
#endif

#pragma internal on
void __InitCode__(void);
#pragma internal reset

pascal OSErr __initialize(const CFragInitBlock *theInitBlock);
pascal void __terminate(void);

void DoNotification(StringPtr aString);

#ifdef __cplusplus
}
#endif


char *GetA5TOC(void) = 0x200D;			/* move.l a5,d0 */

/*
 *	The linker will replace this dummy routine with a linker generated routine containing
 *	a list of calls to all our static initialization routines ...
 *
 *					MOVEA.L		(A1),A5
 *					LINK		A6,#$FFF8
 *					BSR.L		__InitCode0__
 *					...
 *					BSR.L		__InitCodeN__
 *					UNLK		A6
 *					RTS
 *
 *	NB: for now this will only work under the single segment model
 */
/*void __InitCode__(void)
{
}*/

/*
 *	__initialize	-	Default initialization routine for Metrowerks C++ (CFM68K)
 *
 *	This routine should be specified as the PEF initialization routine in the container
 *	for any multi-fragment application.
 *
 */
static short gmyRefNum=0;

pascal OSErr __initialize(const CFragInitBlock *theInitBlock)
{
OSErr				err = noErr;
	FSSpec resSpec;
	Str32 rsrcName;
	/* register this code fragment with the Exception Handling mechanism */
	fragmentID = __register_fragment(SEGMAPNAME, GetA5TOC());

	// Find out where we're at!
	if (IsFileLocation(theInitBlock->fragLocator.where))
	{
		resSpec =*theInitBlock->fragLocator.u.onDisk.fileSpec; // This is where we are!!!
		//BlockMoveData((StringPtr)&"\paux.OpenGL68kLib",resSpec.name,32); // -sb not needed anymore.
		gmyRefNum = FSpOpenResFile(&resSpec, fsRdWrShPerm);
	}

	if (gmyRefNum == -1)
	{
		short resNum;
		DoNotification("\pCould not open the resource fork of the shared library! Looking for a '.rsrc' file.");
		BlockMoveData(resSpec.name,rsrcName,32); // -sb copy the resSpec name from theInitBlock.
		BlockMoveData(&rsrcName[rsrcName[0]],".rsrc",5);
		rsrcName[0] += 5;
		resNum = OpenResFile(rsrcName);
		if (resNum == -1)
		{
			DoNotification("\pCould not open a resource fork that is required for aux.OpenGL68kLib to work! This application will now quit.");
			//DoNotification();
			return(fnfErr);
		}
	}

	/* call all static initializers */
	__InitCode__();
	
	/* return success to Code Fragment Manager */
	return err;
}


/*
 *	__terminate	-	Default termination routine for Metrowerks C++ (CFM68K)
 *
 *	This routine should be specified as the PEF termination routine in the container
 *	for any multi-fragment application.
 *
 */

pascal void __terminate(void)
{
	/* unregister this code fragment with the Exception Handling mechanism */
	__unregister_fragment(fragmentID);
}

static Boolean gDisplayDone;
void NMDisplayDone(NMRecPtr nmReqPtr);
void NMDisplayDone(NMRecPtr nmReqPtr)
{
	long lastA5 = SetA5(nmReqPtr->nmRefCon);
	gDisplayDone = true;
	SetA5(lastA5);
}

void DoNotification(StringPtr aString)
{
	NMRec	myNotification;	//a notification record
	short	myResNum = 0;	//resource ID of small icon resource
	Handle	myResHand = 0;	//handle to small icon resource
//	Str255	myText = 
											//string to print in alert box

//	myResNum = 1234;			//resource ID in resource fork
//	myResHand = GetResource('SICN', myResNum);
											//get small icon from resource fork

	myNotification.qType = nmType;	//set queue type
	myNotification.nmMark = 0;	//put mark in Application menu
	myNotification.nmIcon = myResHand;//alternating icon
	myNotification.nmSound = 0;//(Handle)-1;	//play system alert
													// sound
	myNotification.nmStr = aString;
	myNotification.nmResp = NewNMProc(NMDisplayDone);		//no response procedure. 0=none, -1 = remove me!
	myNotification.nmRefCon = SetCurrentA5();				//not needed

	MaxApplZone();
	MoreMasters();
	
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	
	InitDialogs(0L);
	
	FlushEvents(everyEvent,0);
	InitCursor();

	gDisplayDone = false;
	NMInstall(&myNotification);
	while (gDisplayDone == false)
	{
		EventRecord ev;
		WaitNextEvent(everyEvent,&ev,0,0);
	}
}
