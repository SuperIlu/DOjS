#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <libbcc.h>
#include "stdfun.h"

int main(void)
{
  int  gd, gm;
  int  err;
  int  x, y, xr, i;

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
  x = getmaxx()/2;
  y = getmaxy()/2;
  for (i=-10; i <= 10; i+=2) {
    cleardevice();
    for (xr=1; xr <= x && xr < y ; xr += x/16)
      ellipse(x,y,0,360+i,xr,xr*y/x);
    getch();
  }
  for (i=1; i <= 10; i++) {
    cleardevice();
    for (xr=1; xr <= x && xr < y ; xr += x/16)
      ellipse(x,y,0,360*i,xr,xr*y/x);
    getch();
  }
  closegraph();
  return 0;
}
