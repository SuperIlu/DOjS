#import <AppKit/AppKit.h>
#import <GL/osmesa.h>
#import "mesadraw.h"

@interface MesaView : NSView
{
    float zoomDist;
    float spinAngle;
    float elevAngle;
    id zoomSlider;
    id spinSlider;
    id elevSlider;
    unsigned char *buffer;
    OSMesaContext ctx;
    BOOL draggingBase;
    BOOL whiteBackground;
    int averagingNormals;
    int colourDrape;
    NSTimer *timer_object;
}
- changeZoom:sender;
- changeElev:sender;
- changeSpin:sender;
- changeNormal:sender;
- changeDrape:sender;
- changeBackground:sender;
- remakeSurface:sender;
@end
