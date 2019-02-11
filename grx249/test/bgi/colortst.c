/* From VGAlib, changed for svgalib */
/* partially copyrighted (C) 1993,1995 by Hartmut Schirmer */


#include <stdio.h>
#include <string.h>
#include "libbcc.h"
#include "stdfun.h"

static void testmode(int mode) {
	int xmax, ymax;
	int i, oldmode;
	int x, y, yw;
	int c;

	if (graphresult() == grNoInitGraph)
	  return;
#ifdef __GNUC__
	if (getmodemaxcolor(mode)+1 < 256)
	  return;
#endif
	oldmode = getgraphmode();
	if (mode != oldmode) {
	  graphresult();
	  setgraphmode(mode);
	  if (graphresult() != grOk)
	    return;
	}
	if (getmaxcolor() < 255) {
	  if (oldmode != mode)
	    setgraphmode(oldmode);
	  return;
	}
	xmax = getmaxx();
	ymax = getmaxy();

	yw = (ymax - 0) / 4;
	switch (getmaxcolor()+1) {
	case 256:
		#define std_c  (16)
		#define free_c (256-std_c)
		#define avail  (free_c / 4)
		#define nrval  (256/4)
		for (i = 0; i < avail; ++i) {
			c = (i * nrval) / avail;
			setrgbpalette(i + std_c + (0 * avail), c, c, c);
			setrgbpalette(i + std_c + (1 * avail), c, 0, 0);
			setrgbpalette(i + std_c + (2 * avail), 0, c, 0);
			setrgbpalette(i + std_c + (3 * avail), 0, 0, c);
		}
		for (x = 2; x < xmax - 1; ++x) {
		  c = (((x-2)*avail) / (xmax-3)) + std_c;
		  for (i=0; i < 4; ++i) {
		    setcolor(c+(avail*i));
		    line( x, i*yw, x, (i+1)*yw);
		  }
		}
		break;
#ifdef __GNUC__
	case 1 << 15:
	case 1 << 16:
	case 1 << 24:
		for (x = 2; x < xmax - 1; ++x) {
			c = ((x - 2) * 256) / (xmax - 3);
			y = 0;
			setrgbcolor(c, c, c);
			line(x, y, x, y + yw - 1);
			y += yw;
			setrgbcolor(c, 0, 0);
			line(x, y, x, y + yw - 1);
			y += yw;
			setrgbcolor(0, c, 0);
			line(x, y, x, y + yw - 1);
			y += yw;
			setrgbcolor(0, 0, c);
			line(x, y, x, y + yw - 1);
		}
		break;
#endif
	default:
		if (oldmode != mode)
		  setgraphmode(oldmode);
		return;
	}
	{
	  char *mn = getmodename(mode);
	  setcolor(WHITE);
	  settextjustify( CENTER_TEXT, BOTTOM_TEXT );
	  outtextxy(getmaxx()/2, getmaxy(), mn);
	}
	getch();

	if (oldmode != mode)
	  setgraphmode(oldmode);
}

int main(void)
{
  int gd, gm;
  int err;
  int lomode, himode;

  gd = DETECT;
#if defined(__MSDOS__) || defined(__WIN32__)
  initgraph(&gd,&gm,"..\\..\\chr");
#else
  initgraph(&gd,&gm,"../../chr");
#endif
  err = graphresult();
  if (err != grOk) {
    fprintf(stderr, "Couldn't initialize graphics\n");
    return 1;
  }
  getmoderange(gd, &lomode, &himode);
  gm = lomode;
#ifdef __GNUC__
  if (gm < __FIRST_DRIVER_SPECIFIC_MODE)
    gm = __FIRST_DRIVER_SPECIFIC_MODE;
#endif
  for ( ; gm <= himode; ++gm)
    testmode(gm);
  closegraph();
  return 0;
}
