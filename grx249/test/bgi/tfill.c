#include <stdio.h>
#include <stdlib.h>

#include <libbcc.h>
#include "stdfun.h"

int main(void)
{
  int gd, gm;
  int err;

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
  setviewport( 100, 100, 200, 200, 1);
  floodfill( 10, 10, WHITE);
  getch();
  setfillstyle( SOLID_FILL, DARKGRAY);
  cleardevice();
  setviewport( 100, 100, 200, 200, 0);
  floodfill( 10, 10, WHITE);
  getch();
  closegraph();
  return 0;
}
