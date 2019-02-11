#include "grx20.h"
#include "bmp.c"

int main ( void )
{
  GrBmpImage *bmp256, *bmp;
  GrPattern *p256, *p;
  GrSetMode(GR_width_height_color_graphics, 800, 600, 256);
  bmp256 = GrLoadBmpImage("mysha256.bmp");
  bmp = GrLoadBmpImage("some1.bmp");
  GrAllocBmpImageColors(bmp, NULL);
  GrAllocBmpImageColors(bmp256, NULL);
  p256 = GrConvertBmpImageToPattern(bmp256);
  p = GrConvertBmpImageToPattern(bmp);
  if ( p ) {
    GrImageDisplay(0, 0, GrImageFromPattern(p));
    getkey();
  }
  if ( p256 ) {
    GrImageDisplay(300, 300, GrImageFromPattern(p256));
    getkey();
  }
  if ( p ) GrDestroyPattern(p);
  if ( p256 ) GrDestroyPattern(p256);
  GrSaveBmpImage("save.bmp", NULL, 0, 0, 400, 400);
  GrUnloadBmpImage(bmp);
  GrUnloadBmpImage(bmp256);
  return 0;
};

