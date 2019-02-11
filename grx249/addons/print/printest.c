#include <string.h>
#include <grx20.h>
#include <stdio.h>
#include "grxprint.h"


static void displaytext (GrFont *font,char *text,int len);


int GRXMain (int argc , char * argv [])
  {
      int i, j, rc, mc, MaxX, MaxY;
      unsigned AspX, AspY;
      double Asp, r;
      char * text = "Printing example from GRX";
      GrFont * Fnt = GrBuildConvertedFont(
		&GrDefaultFont,
		(GR_FONTCVT_SKIPCHARS | GR_FONTCVT_RESIZE),
		36,
		72,
		' ',
		'z'
	    );

/*      rc = GrPrintSetMode (LQ_180x180);             mc = 2; */
/*      rc = GrPrintSetMode (HPLJ_300x300);           mc = 2; */
/*      rc = GrPrintSetMode (HPLJ_1200x1200);         mc = 2; */
/*      rc = GrPrintSetMode (HPLJ_IV_600x600);        mc = 2; */
/*      rc = GrPrintSetMode (HPLJ_300x300_NC);        mc = 2; */
      rc = GrPrintSetMode (HPDJ500C_300x300x8_B);   mc = 8; 
      printf ("GrPrintSetMode : rc=%d\n",rc);

      MaxX = GrMaxX ();
      MaxY = GrMaxY ();
      GrPrintGetAspectRatio (&AspX,&AspY);
      Asp = ((double) AspX)/AspY;
      printf ("Size : (%d %d)\n",MaxX,MaxY);

      GrBox  (0,0,MaxX,MaxY,mc-1);       /* Draw box around page */
      GrLine (0,MaxY,MaxX,MaxY-30,mc-1);
      r = (int) (MaxY/20);
      GrEllipse ((int) (r*Asp), (int) r, (int) (r*Asp), (int) r, mc-1);
      displaytext (Fnt,text,strlen(text));

      if (mc>2) for (i=0; i<8; i++)
	{
	    int  x0 = 50*i;
	    GrFilledBox (x0,1,x0+30,31,i);
	}

      for (i=1; i<14; i++)
	{
	    r = (double) (MaxY*(0.45-0.03*i));
	    for (j=0; j<(mc>2 ? 3 : 1); j++)
	      {
		 GrEllipse (MaxX/2-100,MaxY/2,
			   (int) (r*Asp), (int) r, i>=mc ? mc-1 : i);
		 r--;
	      }
	}
      GrPrintToFile ("test.pcl");
/*    GrDoPrinting (); */
      return 0;
  }


void displaytext (GrFont *font,char *text,int len)
{
	GrTextOption opt;
	memset(&opt,0,sizeof(opt));
	opt.txo_font   = font;
	opt.txo_xalign = GR_ALIGN_LEFT;
	opt.txo_yalign = GR_ALIGN_TOP;
	opt.txo_direct    = GR_TEXT_RIGHT;
	opt.txo_fgcolor.v = 1;
	opt.txo_bgcolor.v = 0;
	GrDrawString(text,len,100,100,&opt);
}
