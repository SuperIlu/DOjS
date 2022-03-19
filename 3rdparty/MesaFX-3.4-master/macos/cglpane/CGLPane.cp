/* Copyright Mikl—s Fazekas © 1999 All Rights reversed! 
   This program is under Public Domain!
   author: boga@valerie.inf.elte.hu
   see:    http://www.mesa3d.org/mac/  */

// ===========================================================================
//	CGLPane.cp				   ©1999 Mikl—s Fazekas. 	  All rights reversed.
// ===========================================================================
//
//  This is under Public Domain!
//
//	PowerPlant Pane with OpenGL context
//


#include <CodeFragments.h>
#include "CGLPane.h"


// ---------------------------------------------------------------------------
//	¥ OpenGLIsPresent									 [static] [public]
// ---------------------------------------------------------------------------
//	Is OpenGL present?  

Boolean
UOpenGL::OpenGLIsPresent()
{
	return ((UInt32)aglCreateContext != kUnresolvedCFragSymbolAddress); 
}
#pragma mark -

// ---------------------------------------------------------------------------
//		¥ CGLPane													[public]
// ---------------------------------------------------------------------------
//	Default Constructor

CGLPane::CGLPane()
{
	mDisplayMode = 0;
	
	mAGLContext = nil;
	mAGLPixelFormat = nil;
	mAGLDrawable = nil;
	
	mOldContext = nil;
	mOldDrawable = nil;
	mGLLocked = false;
}

// ---------------------------------------------------------------------------
//		¥ CGLPane(SPaneInfo&, UInt32)							    [public]
// ---------------------------------------------------------------------------
//	Construct from parameters. Use this constructor to create a CGLPane
//	from runtime data.

CGLPane::CGLPane(
	const SPaneInfo	&inPaneInfo,
	UInt32 			inDisplayMode)
		: LPane(inPaneInfo)
{
#pragma unused(inDisplayMode)
	mDisplayMode = 0;
	
	mAGLContext = nil;
	mAGLPixelFormat = nil;
	mAGLDrawable = nil;
	
	mOldContext = nil;
	mOldDrawable = nil;	
	mGLLocked = false;
	
	InitOpenGLContext(inDisplayMode);
}	
// ---------------------------------------------------------------------------
//		¥ CGLPane(LStream*)											[public]
// ---------------------------------------------------------------------------
//	Construct from data in a Stream

CGLPane::CGLPane(
	LStream	*inStream)
		: LPane(inStream)
{
	
	// Read the display mode infos:
	UInt32 theDisplayMode = 0;
	{
		Boolean index,doubleBuffer,accum,alpha,depth,stencil;
	
		inStream->ReadData(&index, sizeof(Boolean));
		if (index)
			theDisplayMode |= CGL_INDEX;
		else
			theDisplayMode |= CGL_RGB;
			
		inStream->ReadData(&doubleBuffer, sizeof(Boolean));
		if (doubleBuffer)
			theDisplayMode |= CGL_DOUBLE;
		else
			theDisplayMode |= CGL_SINGLE;
			
		inStream->ReadData(&accum, sizeof(Boolean));
		if (accum)
			theDisplayMode |= CGL_ACCUM;
			
		inStream->ReadData(&alpha,sizeof(Boolean)); 
		if (alpha)
			theDisplayMode |= CGL_ALPHA;
			
		inStream->ReadData(&depth,sizeof(Boolean));
		if (depth)
			theDisplayMode |= CGL_DEPTH;
			
		inStream->ReadData(&stencil,sizeof(Boolean));
		if (stencil)
			theDisplayMode |= CGL_STENCIL;
	}
	
	InitOpenGLContext(theDisplayMode);
}
// ---------------------------------------------------------------------------
//	¥ InitOpenGLContext												[private]
// ---------------------------------------------------------------------------
//	Private Initializer.

void
CGLPane::InitOpenGLContext(
	UInt32	inDisplayMode)
{
	mDisplayMode = inDisplayMode;
	mAGLPixelFormat = CreateAGLPixelFormat(mDisplayMode);

	if (mAGLPixelFormat != nil)
		mAGLContext = aglCreateContext(mAGLPixelFormat,nil);
	else
		mAGLContext = nil;
	
	if (mAGLContext != nil)
		mAGLDrawable = CreateFakePort();
	else
		mAGLDrawable = nil;
	
	// If something went wrong...
	if (mAGLDrawable == nil)
	{
		mAGLContext = nil;
		mAGLPixelFormat = nil;
		mDisplayMode = 0;
	}
	
	mOldContext = nil;
	mOldDrawable = nil;
	mGLLocked = false;	
}

// ---------------------------------------------------------------------------
//		¥ CreateAGLPixelFormat								[static] [private]
// ---------------------------------------------------------------------------
//	Get a minimal pixel format from the inDisplayMode
AGLPixelFormat
CGLPane::CreateAGLPixelFormat(
		UInt32	inDisplayMode)
{
	int list[32];
	int n = 0;
	
	// Indexed mode
	if (inDisplayMode & CGL_INDEX)
	{
		list[n++] = AGL_BUFFER_SIZE;
  		list[n++] = 1;
	}
	else // Rgba mode
	{
		list[n++] = AGL_RGBA;
	}
	
	// Alpha buffer mode
	if (inDisplayMode & CGL_ALPHA)
	{
		list[n++] = AGL_ALPHA_SIZE;
		list[n++] = 1;
	}
	
	// Double-buffer mode
	if (inDisplayMode & CGL_DOUBLE)
	{
		list[n++] = AGL_DOUBLEBUFFER;
	}
	
	// Accumulation-buffer mode
	if (inDisplayMode & CGL_ACCUM)
	{
		list[n++] = AGL_ACCUM_RED_SIZE;
    	list[n++] = 1;
    	list[n++] = AGL_ACCUM_GREEN_SIZE;
    	list[n++] = 1;
    	list[n++] = AGL_ACCUM_BLUE_SIZE;
    	list[n++] = 1;
    	
    	if (inDisplayMode & CGL_ALPHA)
    	{
    		list[n++] = AGL_ACCUM_ALPHA_SIZE;
    		list[n++] = 1;
    	}
	}
	
	// Depth-buffer mode
	if (inDisplayMode & CGL_DEPTH)
	{
		list[n++] = AGL_DEPTH_SIZE;
		list[n++] = 1;
	}
	
	// Stencil mode
	if (inDisplayMode & CGL_STENCIL)
	{
		list[n++] = AGL_STENCIL_SIZE;
		list[n++] = 1;
	}
	
	// Terminate list
	list[n] = AGL_NONE;
	
	return aglChoosePixelFormat(NULL,0,list);
}

#pragma mark -

// ---------------------------------------------------------------------------
//		¥ CreateFakePort									[static] [private]
// ---------------------------------------------------------------------------
//	Create a fake GrafPort.
CGrafPtr
CGLPane::CreateFakePort()
{
	CGrafPtr 		newPort = nil;
	PixMapHandle 	newPixMap = nil;
	RgnHandle		clipRgn = nil;
	RgnHandle		visRgn = nil;
	
	try {
		ThrowIfNil_(newPixMap = (PixMapHandle)NewHandleClear(sizeof(PixMap)));
		ThrowIfNil_(clipRgn = NewRgn());
		ThrowIfNil_(visRgn = NewRgn());
		HLock((Handle)newPixMap);
		ThrowIfNil_(newPort = (CGrafPtr)NewPtrClear(sizeof(CGrafPort)));
		newPort->portPixMap = newPixMap;
		newPort->visRgn = visRgn;
		newPort->clipRgn = clipRgn;
	}
	catch (...)
	{
		if (newPort != nil)
		{
			DisposePtr((Ptr)newPort);
			newPort = nil;
		}
		
		if (visRgn != nil)
			DisposeRgn(visRgn);
			
		if (clipRgn != nil)
			DisposeRgn(clipRgn);
			
		if (newPixMap != nil)
			DisposeHandle((Handle)newPixMap);
		
		ThrowIfNil_(nil);
	}
	
	return newPort;
}


// ---------------------------------------------------------------------------
//		¥ UpdateFakePort									[static] [private]
// ---------------------------------------------------------------------------
//	Update's the fake GrafPort.
Boolean
CGLPane::UpdateFakePort(
			CGrafPtr 	ioPort,			// The result prot
			CGrafPtr 	inPort,			// The source port
			const Rect 	&inPortFrame)	// Frame in port coordinates
{
	PixMapHandle 	oldPixMap;
	RgnHandle		oldVisRgn;
	RgnHandle		oldClipRgn;
	
	if (inPort == nil)
		return false;
	
	if (ioPort == nil)
		return false;
	
	// Copy the port+pixMap data:
	oldPixMap 	= ioPort->portPixMap;
	oldVisRgn 	= ioPort->visRgn;
	oldClipRgn 	= ioPort->clipRgn;
	*ioPort 	= *(inPort);
	
	ioPort->portPixMap = oldPixMap;
	(**ioPort->portPixMap) = (**inPort->portPixMap);
	
	ioPort->visRgn = oldVisRgn;
	RectRgn(ioPort->visRgn,&inPortFrame);
	SectRgn(inPort->visRgn,ioPort->visRgn,ioPort->visRgn); 	
	OffsetRgn(ioPort->visRgn,-inPortFrame.left,-inPortFrame.top);
	
	ioPort->clipRgn = oldClipRgn;
	RectRgn(ioPort->clipRgn,&inPortFrame);
	SectRgn(inPort->clipRgn,ioPort->clipRgn,ioPort->clipRgn); 
	OffsetRgn(ioPort->clipRgn,-inPortFrame.left,-inPortFrame.top);
	
	// This is a hacked-up GrafPort!!!
	(**ioPort->portPixMap).bounds.left -= inPortFrame.left;
	(**ioPort->portPixMap).bounds.right -= inPortFrame.left;
	
	(**ioPort->portPixMap).bounds.top -= inPortFrame.top;
	(**ioPort->portPixMap).bounds.bottom -= inPortFrame.top;
	
	ioPort->portRect.bottom = ioPort->portRect.top+(inPortFrame.bottom-inPortFrame.top);
	ioPort->portRect.right = ioPort->portRect.left+(inPortFrame.right-inPortFrame.left);

	
	return true;
}
// ---------------------------------------------------------------------------
//		¥ DisposeFakePort									[static] [private]
// ---------------------------------------------------------------------------
//	Disposes a fake GrafPort.
void
CGLPane::DisposeFakePort(CGrafPtr 	inPort)
{
	if (inPort == nil)
		return;
		
	DisposeHandle((Handle)inPort->portPixMap);
	DisposeRgn(inPort->visRgn);
	DisposeRgn(inPort->clipRgn);
	DisposePtr((Ptr)inPort);
}
#pragma mark -


// ---------------------------------------------------------------------------
//		¥ ~CGLPane													  [public]
// ---------------------------------------------------------------------------
//	Destructor

CGLPane::~CGLPane()
{
	if (mAGLPixelFormat != nil)
		DisposePtr((Ptr)mAGLPixelFormat);
		
	if (mAGLContext != nil)
		aglDestroyContext(mAGLContext);
		
	if (mAGLDrawable != nil)
		DisposeFakePort(mAGLDrawable);
}
// ---------------------------------------------------------------------------
//		¥ SwapBuffers												[ public ]
// ---------------------------------------------------------------------------
//	Swaps the OpenGL buffers
void
CGLPane::SwapBuffers()
{
	if (mGLLocked)
		aglSwapBuffers();
}

// ---------------------------------------------------------------------------
//		¥ LockGL													[ public ]
// ---------------------------------------------------------------------------
//	Prepares the OpenGL context for drawing
Boolean			
CGLPane::LockGL()
{
	Rect frame;
	
	if (mAGLContext == nil)
		return false;
	
	if (GetMacPort() == nil)
		return false;
		
	if (!CalcPortFrameRect(frame))
		return false;
		
	UpdateFakePort(mAGLDrawable,(CGrafPtr)GetMacPort(),frame);
	
	
		
	mOldContext = aglGetCurrentContext();
	mOldDrawable = aglGetCurrentDrawable();
	
	if (!aglMakeCurrent(mAGLDrawable,mAGLContext))
		return (mGLLocked = false);
		
	aglUpdateCurrent(); // For Conix's OpenGL
	
	/*glResizeBufferMESA();	 */
	/* glViewport(0,0,(frame.right-frame.left)+100,(frame.bottom-frame.top)); */
		
	mGLLocked = true;
	
	return mGLLocked;
}
// ---------------------------------------------------------------------------
//		¥ ErrorCallback												  [public]
// ---------------------------------------------------------------------------
//	Called when an OpenGL error occured, overide this to use.
void
CGLPane::ErrorCallback(GLenum errorCode)
{
	#pragma unused (errorCode)
}


// ---------------------------------------------------------------------------
//		¥ UnlockGL													[ public ]
// ---------------------------------------------------------------------------
//	Ends the drawing to this OpenGL context
void
CGLPane::UnlockGL()
{
	mGLLocked = false;
	if (!aglMakeCurrent(mOldDrawable,mOldContext))
	{
		GLenum error;
		
		error = aglGetError();
	}
}

// ---------------------------------------------------------------------------
//		¥ ResizeFrameBy
// ---------------------------------------------------------------------------
//	Change the Frame size by the specified amounts.
void
CGLPane::ResizeFrameBy(
	SInt16		inWidthDelta,
	SInt16		inHeightDelta,
	Boolean		inRefresh)
{
									// Resize Pane
	LPane::ResizeFrameBy(inWidthDelta, inHeightDelta, inRefresh);
	LockGL();
	SDimension16 portSize;
	
	GetFrameSize(portSize);
	glViewport(0, 0, portSize.width, portSize.height);
	UnlockGL();
}



// ---------------------------------------------------------------------------
//		¥ DrawSelf													[ public ]
// ---------------------------------------------------------------------------
//	Draws something with OpenGL.
void
CGLPane::DrawSelf()
{
	if (!LockGL())
		return;
		
	/* Clear buffer */
	glClearColor(1.0, 1.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	/* Set the view */
	
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0 );
	
	/* Draw colored polygon in the window */
	
	glColor3f( 1.0, 0.5, 0.25);
	glBegin( GL_POLYGON );
	glColor3f(1.0,0.0,0.0);
	glVertex2f(	-0.5, -0.5);
	glColor3f(0.0,1.0,0.0);
	glVertex2f(	-0.5, 0.9);
	glColor3f(0.0,0.0,1.0);
	glVertex2f(	0.5, 0.9);
	glVertex2f( 0.5, -0.5 );
	glEnd();
	
	glBegin(GL_POLYGON);
	glColor3f(0.0,1.0,0.0);
	glVertex2f(-0.5,-0.5);
	glVertex2f(-0.5,0.5);
	glVertex2f(0.5,2.5);
	glEnd();
	
	glFinish();
	
	SwapBuffers();
		
	UnlockGL();
}
