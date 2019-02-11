#include <stdio.h>
#include <stdlib.h>

#include <libbcc.h>
#include "stdfun.h"

int main(void)
{
  int gd, gm;
  int err;
  int poly[50];

  gd = DETECT;
#if defined(__MSDOS__) || defined(__WIN32__)
  initgraph(&gd,&gm,"..\\..\\chr");
#else
  initgraph(&gd,&gm,"../../chr");
#endif
  err = graphresult();
  if (err != grOk) {
    fprintf(stderr, "Couldn't initialize graphics\n");
    exit(1);
  }
  setfillstyle( SOLID_FILL, GREEN);
  poly[ 0] =  10;          poly[ 1] =  10;
  poly[ 2] =  20;          poly[ 3] =  20;
  poly[ 4] =  20;          poly[ 5] = getmaxy()-20;
  poly[ 6] = getmaxx()-20; poly[ 7] = getmaxy()-20;
  poly[ 8] = getmaxx()-20; poly[ 9] =  20;
  poly[10] =  20;          poly[11] =  20;
  poly[12] =  75;          poly[13] =  35;
  poly[14] =  75;          poly[15] = getmaxy()-50;
  poly[16] = getmaxx()-60; poly[17] = getmaxy()-50;
  poly[18] = getmaxx()-60; poly[19] =  35;
  poly[20] =  75;          poly[21] =  35;
  drawpoly( 11, poly);
  getch();
  cleardevice();
  fillpoly( 11, poly);
  getch();
  cleardevice();
  drawpoly( 10, &poly[2]);
  getch();
  cleardevice();
  fillpoly( 10, &poly[2]);
  getch();
  closegraph();
  return 0;
}
