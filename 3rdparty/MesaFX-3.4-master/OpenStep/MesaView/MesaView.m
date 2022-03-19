#import <MesaView.h>

@interface MesaView(Private)
-(void)_renderImage;
-(void)_createViewport;
-(void)_startDrag;
-(void)_stopDrag:(NSTimer*)the_timer;
@end

@implementation MesaView

- initWithFrame:(NSRect)frameRect
{
    [super initWithFrame: frameRect];
    ctx = NULL;
    buffer = NULL;
    zoomDist=130;
    spinAngle=0;
    elevAngle=30;
    draggingBase=NO;
    averagingNormals=0;
    colourDrape=0;
    whiteBackground=NO;
    timer_object=nil;
    make_matrix();
    [self _createViewport];
    return self;
}

- (void)awakeFromNib
{
    [zoomSlider setFloatValue:zoomDist];
    [spinSlider setFloatValue:spinAngle];
    [elevSlider setFloatValue:elevAngle];
}


- (void)dealloc
{
    OSMesaDestroyContext(ctx);
    free(buffer);
    [timer_object invalidate];
    [timer_object release];
    [super dealloc];
}


- (BOOL)isOpaque
{
    return YES;
}

- (void)_createViewport
{
    if(buffer)
      free(buffer);
    buffer=malloc([self bounds].size.width*[self bounds].size.height*4);

    if(ctx)
      OSMesaDestroyContext(ctx);
    ctx = OSMesaCreateContext( GL_RGBA, NULL );
    OSMesaMakeCurrent( ctx, buffer, GL_UNSIGNED_BYTE,
                       [self bounds].size.width, [self bounds].size.height );
    OSMesaPixelStore( OSMESA_Y_UP, 0 );


    my_init([self bounds].size.width,[self bounds].size.height);
}

- (void)resizeWithOldSuperviewSize:(NSSize)old
{
    [super resizeWithOldSuperviewSize:old];
    [self _createViewport];
    [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)rect
{
    set_viewpoint(zoomDist, elevAngle, spinAngle);
    if(draggingBase)
    {
      if(whiteBackground)
       {
        PSsetgray(1);
        NSRectFill(rect);
        PSsetgray(0);
       }
      else
       {
        PSsetgray(0);
        NSRectFill(rect);
        PSsetgray(1);
       }
       outline_scene();
       
    }
    else
    {
      [self _renderImage];
    }
}

- (void)_renderImage
{
    NSBitmapImageRep *bitmap;

    draw_scene(averagingNormals,colourDrape,whiteBackground);

    bitmap = [[ NSBitmapImageRep alloc] initWithBitmapDataPlanes:&buffer
                                        pixelsWide:[self bounds].size.width
                                        pixelsHigh:[self bounds].size.height
                                        bitsPerSample:8 samplesPerPixel:4
                                        hasAlpha:YES isPlanar:NO
                                        colorSpaceName:NSDeviceRGBColorSpace
                                        bytesPerRow:0 bitsPerPixel:0];
    [bitmap autorelease];
    [bitmap draw];
}

- (void)_startDrag
{
    draggingBase=YES;
    timer_object = [NSTimer
                    scheduledTimerWithTimeInterval:(NSTimeInterval)0.0
                    target:self selector:@selector(_stopDrag:)
                    userInfo:nil repeats:NO];
    [timer_object retain];
}

- (void)_stopDrag:(NSTimer*)the_timer
{
    [timer_object invalidate];
    [timer_object release];
    timer_object=nil;
    draggingBase=NO;
    [self setNeedsDisplay:YES];
}

- changeZoom:sender
{
    if(!draggingBase)
       [self _startDrag];
    zoomDist = [zoomSlider floatValue];
    [self display];
    return self;
}

- changeSpin:sender
{
    if(!draggingBase)
       [self _startDrag];
    spinAngle = [spinSlider floatValue];
    [self display];
    return self;
}

- changeElev:sender
{
    if(!draggingBase)
       [self _startDrag];
    elevAngle = [elevSlider floatValue];
    [self display];
    return self;
}

- changeNormal:sender
{
    averagingNormals=[sender state];
    [self setNeedsDisplay:YES];
    return self;
}

- changeDrape:sender
{
    colourDrape=[sender state];
    [self setNeedsDisplay:YES];
    return self;
}


- remakeSurface:sender
{
    make_matrix();
    [self setNeedsDisplay:YES];
    return self;
}

- changeBackground:sender
{
    whiteBackground=[sender state];
    [self setNeedsDisplay:YES];
    return self;
}



@end
