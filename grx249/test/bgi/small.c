#include <libbcc.h>

int main(void)
{
  int  gd, gm, err;

  gd = DETECT;
#if defined(__MSDOS__) || defined(__WIN32__)
  initgraph(&gd,&gm,"..\\..\\chr");
#else
  initgraph(&gd,&gm,"../../chr");
#endif
  err = graphresult();
  closegraph();
  return 0;
}
