/* nextdemo.m*/

/* Demo of NEXTSTEP Mesa rendering */

/*
 * See Mesa/include/GL/osmesa.h for documentation of the OSMesa functions.
 *
 * If you want to render BIG images you'll probably have to increase
 * MAX_WIDTH and MAX_HEIGHT in src/config.h.
 *
 * This program is in the public domain.
 *
 * Brian Paul
 *
 * NEXTSTEP output provided by Pascal Thibaudeau 
 * pthibaud@frbdx11.cribx1.u-bordeaux.fr
 */

#import <dpsclient/wraps.h>
#import <appkit/Application.h>
#import <appkit/Window.h>
#import <appkit/Menu.h>
#import <appkit/View.h>
#import <appkit/color.h>
#import <appkit/NXBitmapImageRep.h>

#include <stdio.h>
#include <stdlib.h>
#include "GL/osmesa.h"

#define WIDTH 480
#define HEIGHT 480 

static void render_image()
{
   GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
   GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
   GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
   GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
   GLfloat red_mat[]   = { 1.0, 0.0, 0.0, 0.5 };
   GLfloat green_mat[] = { 0.0, 1.0, 0.0, 0.5 };
   GLfloat blue_mat[]  = { 0.0, 0.0, 1.0, 0.5};

   glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
   glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_DEPTH_TEST);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-2.5, 2.5, -2.5, 2.5, -10.0, 10.0);
   glMatrixMode(GL_MODELVIEW);

   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   glPushMatrix();
   glRotatef(20.0, 1.0, 0.0, 0.0);

   glPushMatrix();
   glTranslatef(-0.75, 0.5, 0.0); 
   glRotatef(90.0, 1.0, 0.0, 0.0);
   glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, red_mat );
   auxSolidTorus(0.275, 0.85);
   glPopMatrix();

   glPushMatrix();
   glTranslatef(-0.75, -0.5, 0.0); 
   glRotatef(270.0, 1.0, 0.0, 0.0);
   glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, green_mat );
   auxSolidCone(1.0, 2.0);
   glPopMatrix();

   glPushMatrix();
   glTranslatef(0.75, 0.0, -1.0); 
   glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, blue_mat );
   auxSolidSphere(1.0);
   glPopMatrix();

   glPopMatrix();
}



int main( int argc, char *argv[] )
{
   OSMesaContext ctx;
   void *buffer;
   id myWindow, myView, myMenu, image;
   char name[50];
   NXRect GR;
   NXPoint position;
     
   NXApp=[Application new];

   /* Create an RGBA-mode context */
   ctx = OSMesaCreateContext( GL_RGBA, NULL );

   /* Allocate the image buffer */
   buffer = malloc( WIDTH * HEIGHT * 4 );
   
   /* Bind the buffer to the context and make it current */
   OSMesaMakeCurrent( ctx, buffer, GL_UNSIGNED_BYTE, WIDTH, HEIGHT );

  image = [ [ NXBitmapImageRep alloc] 
		initData: buffer
		pixelsWide: WIDTH
		pixelsHigh: HEIGHT
		bitsPerSample: 8
		samplesPerPixel: 4
		hasAlpha: YES
		isPlanar: NO
		colorSpace: 2
		bytesPerRow: WIDTH*4
		bitsPerPixel: 32]; 
 
  NXSetRect(&GR,100,100,WIDTH,HEIGHT);

  myWindow = [ [ Window alloc]
			initContent: &GR
			style: NX_TITLEDSTYLE
			backing: NX_BUFFERED
			buttonMask: NX_MINIATURIZEBUTTONMASK
			defer: NO];

  sprintf(name, "Graphic Rectangle %d", getpid());

   myView = [ [ View alloc] initFrame:&GR];
   [myView setOpaque: NO];
   [myView setDrawOrigin: -WIDTH: -HEIGHT];
   [myView rotate: 180.0];

   myMenu = [ [ Menu alloc] initTitle: "NeXT-OpenGL"];
   [myMenu addItem: "Quit" action: @selector(terminate:) keyEquivalent: 'q'];
   [myMenu sizeToFit];

   [myWindow setTitle: name];
   [myWindow display];
   [myWindow setContentView: myView];
   [myWindow makeKeyAndOrderFront: nil];

   [NXApp setMainMenu: myMenu];

   [myView lockFocus];

   /* here is the Mesa call */
   render_image();

   [image draw];
   [image free];

   [myWindow flushWindow];
   [myView unlockFocus];

   /* free the image buffer */
   free( buffer );
 
   /* destroy the context */
   OSMesaDestroyContext( ctx );

   [NXApp run];
   [NXApp free];

   return 0;
}
