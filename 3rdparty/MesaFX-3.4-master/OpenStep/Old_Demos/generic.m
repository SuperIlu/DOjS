/* generic.m*/

/* Demo of OPENSTEP Mesa rendering */

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
 * OpenStep conversion by Pete French
 * pete@ohm.york.ac.uk
 */

#import <AppKit/AppKit.h>
#import <stdio.h>
#import <stdlib.h>
#import "GL/osmesa.h"

extern int gl_width,gl_height;
extern void render_image(void);

int main( int argc, char *argv[] )
{
   OSMesaContext ctx;
   unsigned char *buffer;
   NSWindow *myWindow;
   NSView *myView;
   NSMenu *myMenu;
   NSBitmapImageRep *bitmap;

   unsigned long start, end;
   char name[50];
   NSRect GR;
   NSPoint position;
     
   [[NSAutoreleasePool alloc] init];
   NSApp=[NSApplication sharedApplication];

   /* Create an RGBA-mode context */
   ctx = OSMesaCreateContext( GL_RGBA, NULL );

   /* Allocate the image buffer */
   buffer = malloc( gl_width * gl_height * 4 );
   
   /* Bind the buffer to the context and make it current */
   OSMesaMakeCurrent( ctx, buffer, GL_UNSIGNED_BYTE, gl_width, gl_height );
   OSMesaPixelStore( OSMESA_Y_UP, 0 );

   bitmap = [[ NSBitmapImageRep alloc] initWithBitmapDataPlanes:&buffer
                                       pixelsWide:gl_width
                                       pixelsHigh:gl_height
                                       bitsPerSample:8 samplesPerPixel:4
                                       hasAlpha:YES isPlanar:NO
                                       colorSpaceName:NSDeviceRGBColorSpace
                                       bytesPerRow:0 bitsPerPixel:0];
  GR = NSMakeRect(100, 100, gl_width, gl_height);

  myWindow = [[ NSWindow alloc] initWithContentRect:GR
                                styleMask:NSTitledWindowMask|
                                          NSMiniaturizableWindowMask
                                backing:NSBackingStoreBuffered defer:NO];

  sprintf(name, "Mesa demo: `%s'", argv[0]);

   myView = [[ NSView alloc] initWithFrame:GR];

   myMenu = [[ NSMenu alloc] initWithTitle:@"OpenStep Mesa"];
   [myMenu addItemWithTitle:@"Quit"
           action:@selector(terminate:)
           keyEquivalent:@"q"];
   [myMenu sizeToFit];

   [myWindow setTitle:[NSString stringWithCString:name]];
   [myWindow display];
   [myWindow setContentView:myView];
   [myWindow makeKeyAndOrderFront:nil];

   [NSApp setMainMenu:myMenu];

   [myView lockFocus];

   /* here is the Mesa call */
   start=time(0);
   render_image();
   end=time(0);
   printf("Rendering took %ld seconds\n",end-start);
   fflush(stdout);

   /* draw the bitmap */
   [bitmap draw];
   [bitmap release];
   [myWindow flushWindow];
   [myView unlockFocus];
   free( buffer );
 
   /* destroy the context */
   OSMesaDestroyContext( ctx );

   [NSApp run];
   [NSApp release];

   return 0;
}
