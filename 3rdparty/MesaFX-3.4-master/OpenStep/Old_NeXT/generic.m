/* generic.m*/

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
 * pthibaud@cribx1.u-bordeaux.fr
 * OpenStep conversion by Pete French
 * pete@ohm.york.ac.uk
 */

#import <appkit/Application.h>
#import <appkit/Window.h>
#import <appkit/Menu.h>
#import <appkit/View.h>
#import <appkit/color.h>
#import <appkit/NXBitmapImageRep.h>
#import <stdio.h>
#import <stdlib.h>
#import "GL/osmesa.h"

extern int gl_width,gl_height;
extern void render_image(void);

int main( int argc, char *argv[] )
{
	OSMesaContext ctx;
	unsigned char *buffer;
	id myWindow;
	id myView;
	id myMenu;
	char name[50];
	unsigned long start, end;
	NXBitmapImageRep *bitmap;
	NXRect GR;

	NXApp=[Application new];

	/* Create an RGBA-mode context */
	ctx = OSMesaCreateContext( GL_RGBA, NULL );

	/* Allocate the image buffer */
	buffer = malloc( gl_width * gl_height * 4 );

	/* Bind the buffer to the context and make it current */
	OSMesaMakeCurrent( ctx, buffer, GL_UNSIGNED_BYTE, gl_width, gl_height );
	OSMesaPixelStore( OSMESA_Y_UP, 0 );

	/* Fill the bitmap with the buffer */
	bitmap = [[ NXBitmapImageRep alloc]
				 initData: buffer
				 pixelsWide:gl_width
				 pixelsHigh:gl_height
				 bitsPerSample:8
				 samplesPerPixel:4
				 hasAlpha:YES
				 isPlanar:NO
				 colorSpace: 2
				 bytesPerRow:gl_width*4
				 bitsPerPixel: 32];

	NXSetRect(&GR,100, 100, gl_width, gl_height);

	myWindow = [[ Window alloc]
				   initContent: &GR
				   style:NX_TITLEDSTYLE
				   backing: NX_BUFFERED
				   buttonMask: NX_MINIATURIZEBUTTONMASK
				   defer: NO];

	sprintf(name, "Mesa demo: `%s'", argv[0]);

	myView = [[ View alloc] initFrame:&GR];

	myMenu = [[ Menu alloc] initTitle: "NeXTStep Mesa"];
	[myMenu addItem: "Quit"
			action:@selector(terminate:)
			keyEquivalent: 'q'];
	[myMenu sizeToFit];

	[myWindow setTitle: name];
	[myWindow display];
	[myWindow setContentView:myView];
	[myWindow makeKeyAndOrderFront:nil];

	[NXApp setMainMenu:myMenu];

	[myView lockFocus];

	/* here is the Mesa call */
	start=time(0);
	render_image();
	end=time(0);
	printf("Rendering took %ld seconds\n",end-start);
	fflush(stdout);

	/* draw the bitmap */
	[bitmap draw];
	/* free the bitmap */
	[bitmap free];
	[myWindow flushWindow];
	[myView unlockFocus];
	free( buffer );

   /* destroy the context */
	OSMesaDestroyContext( ctx );

	[NXApp run];
	[NXApp free];

	return 0;
}
