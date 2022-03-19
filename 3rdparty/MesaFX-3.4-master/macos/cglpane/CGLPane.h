/* Copyright Mikl—s Fazekas © 1999 All Rights reversed! 
   This program is under Public Domain!
   author: boga@valerie.inf.elte.hu
   see:    http://www.mesa3d.org/mac/  */

// ===========================================================================
//	CGLPane.h		   ©1999 Mikl—s Fazekas. All rights reserved.
// ===========================================================================
//
//  This is under Public Domain.
//
// OpenGL support for power plant.
// Usage:
//  You should make a subclass of CGLPane. 
//	You should implement the Stream constructor, if your 
//  class will be created from a PPop  resource. 
//  You should implement the (SPaneInfo,UInt32,yourdata) constructor,
//  if your class will be created on the fly.
// 	You should also reimplement the DrawSelf method.
//  In the form: if (!LockGL()) handle_open_gl_error; 
//					my_open_gl_drawing; 
//				 SwapBuffers();
//				 UnlockGL();
//
//	class MyOpenGLPane : public CGLPane {
//		enum { class_ID  = 'Mopl' }; // this is your unique class id
//		
//		MyOpenGLPane(LStream *inStream);			// Stream constructor
//		MyOpenGLPane(const SPaneInfo	&inPaneInfo, // On the fly constructor
//					 UInt32			inDisplayMode)
//	private:
//		virtual void DrawSelf();
//	}
//
//  MyOpenGLPane::MyOpenGLPane(
//					LStream *inStream)
//		:CGLPane(inStream)		// Contsruct CGLPane...
//  {
//      // Initialize your stuff here, also read from the stream if you defined it.
//	}
//
//  void MyOpenGLPane::DrawSelf()
//  {
//		if (!LockGL())	// We can't initalize the OpenGL, handle error here
//		  return;
//		
//		// Here comes your OpenGL code:
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//		...
//		SwapBuffers();	// This will swap the back buffer.
//		UnlockGL();		// This will unlock GL, and it will let another 
//						// Pane use the OpenGL.
//  }
//
// For using your new OpenGL class in your application:
// 1. For create your pane on the fly:
//			LWindow	*theWindow = LWindow::CreateWindow(PPob_SampleWindow, this);
//			ThrowIfNil_(theWindow);
//			{
//				SPaneInfo			paneInfo;
//				MyOpenGLPane		*openGLPane;
//								
//				paneInfo.paneID = 'OPGL';
//				paneInfo.width = 200;
//				paneInfo.height = 200;
//				paneInfo.visible = true;
//				paneInfo.enabled = true;
//				paneInfo.bindings.left = true;
//				paneInfo.bindings.right = true;
//				paneInfo.bindings.top = true;
//				paneInfo.bindings.bottom = true;
//				paneInfo.left = 10;
//				paneInfo.top = 10;
//				paneInfo.userCon = 0;
//				paneInfo.superView = theWindow;
//				
//				openGLPane = new MyOpenGLPane(paneInfo,CGL_RGB);
//				ThrowIfNil_(openGLPane);
//			}
//   2. For create your pane from a PPob resource:
//		You should use the CGLPane.CTYP resource, to create the CGLPane class,
//		in your PPob file. (The Class ID should match with your class's class_id.
//		(Mopl) in this case.
//		At your application constructor you should register your PaneClass:
//			RegisterClass_(MyOpenGLPane);
//		Now your program should work!
//
//  If you have problems please mail me:
//		boga@valeire.inf.elte.hu
//	See the homepage for lasts release:	
//		http://www.mesa3d.org/mac/
//
#ifndef _H_CGLPane
#define _H_CGLPane
#pragma once

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif

#include <MacTypes.h>

#include <LPane.h>
#include <LCaption.h>

#include <agl.h>
#include <gl.h>

#define CGL_RGB			0
#define CGL_RGBA		CGL_RGB
#define CGL_INDEX		1
#define CGL_SINGLE		0
#define CGL_DOUBLE		2
#define CGL_ACCUM		4
#define CGL_ALPHA		8
#define CGL_DEPTH		16
#define	CGL_STENCIL		32


// ---------------------------------------------------------------------------

class UOpenGL {
	public:
		static Boolean	OpenGLIsPresent();
};

// ---------------------------------------------------------------------------

class	CGLPane : public LPane {
public:
	enum { class_ID = 'OpGL' };
	
						CGLPane();
						
						CGLPane(
							const SPaneInfo	&inPaneInfo,
							UInt32			inDisplayMode);
							
						CGLPane(
							LStream			*inStream);
							
	virtual				~CGLPane();
	
	
	Boolean				LockGL();
	void				UnlockGL();
	void				SwapBuffers();
	
	// ¥ Drawing you should override this method.
	virtual	void		DrawSelf();
	
	virtual void		ErrorCallback(GLenum errorCode);
protected:
	static AGLDrawable	CreateFakePort();
	static Boolean		UpdateFakePort(
							CGrafPtr 	ioPort,
							CGrafPtr 	inPort,
							const Rect 	&inPortRect);
	static void				DisposeFakePort(
							CGrafPtr 	inPort);
	static AGLPixelFormat	CreateAGLPixelFormat(
							UInt32 inDisplayMode);
							
	virtual void		ResizeFrameBy(
							SInt16 		inWidthDelta,
							SInt16		inHeightDelta,
							Boolean		inRefresh);
				
	
	UInt32			mDisplayMode;
	AGLContext		mAGLContext;
	AGLDrawable		mAGLDrawable;
	AGLPixelFormat	mAGLPixelFormat;
	
	AGLContext		mOldContext;
	AGLDrawable		mOldDrawable;
	
	Boolean			mGLLocked;
private:
	void				InitOpenGLContext(
							UInt32	inDisplayMode);
};

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif

#endif 