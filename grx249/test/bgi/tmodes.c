#include <stdio.h>
#include <libbcc.h>

void test(char *dn, char *mn, int gd, int gm)
{
  int ogd = gd;
  int ogm = gm;

  __gr_Result = grOk;
  set_BGI_mode(&gd, &gm);
  printf("%-12s %-12s : ", dn,  mn);
  if (__gr_Result == grOk) {
    printf("%4dx%4dx%3d", __gr_BGI_w, __gr_BGI_h, __gr_BGI_c);
    if (gd != ogd || gm != ogm)
      printf( "  gd: %2d -> %2d,   gm: %2d -> %2d", ogd, gd, ogm, gm);
  } else
    printf("No translation available !");
  printf("\n");
}

#define  TEST(drv,mode)      test(#drv,#mode,drv,mode)
#define ETEST(drv,drvn,mode) test(drvn,#mode,drv,mode)
#define extended(drv) ETEST(drv,#drv,RES640x350);  \
		      ETEST(drv,#drv,RES640x480);  \
		      ETEST(drv,#drv,RES800x600);  \
		      ETEST(drv,#drv,RES1024x768); \
		      ETEST(drv,#drv,RES1280x1024)

int main(void)
{
  int i;

  printf( "Available modes :\n");
  for (i = 0; i <= getmaxmode(); ++i)
    printf( "\tgraphics mode %2d : %s\n" , i, getmodename(i));
  printf("\n");

  TEST(VGA     , VGALO     );
  TEST(VGA     , VGAMED    );
  TEST(VGA     , VGAHI     );
  TEST(IBM8514 , IBM8514LO );
  TEST(IBM8514 , IBM8514HI );
  TEST(HERCMONO, HERCMONOHI);
  TEST(CGA     , CGAC0     );
  TEST(CGA     , CGAC1     );
  TEST(CGA     , CGAC2     );
  TEST(CGA     , CGAC3     );
  TEST(CGA     , CGAHI     );
  TEST(MCGA    , MCGAC0    );
  TEST(MCGA    , MCGAC1    );
  TEST(MCGA    , MCGAC2    );
  TEST(MCGA    , MCGAC3    );
  TEST(MCGA    , MCGAMED   );
  TEST(MCGA    , MCGAHI    );
  TEST(ATT400  , ATT400C0  );
  TEST(ATT400  , ATT400C1  );
  TEST(ATT400  , ATT400C2  );
  TEST(ATT400  , ATT400C3  );
  TEST(ATT400  , ATT400MED );
  TEST(ATT400  , ATT400HI  );
  TEST(EGA64   , EGA64LO   );
  TEST(EGA64   , EGA64HI   );
  TEST(EGA     , EGALO     );
  TEST(EGA     , EGAHI     );
  TEST(EGAMONO , EGAMONOHI );
  TEST(PC3270  , PC3270HI  );
  /* Extended modes from BC++ 4.5 */
  extended(VGA256);
  extended(ATTDEB);
  extended(TOSHIBA);
  extended(SVGA16);
  extended(SVGA256);
  extended(SVGA32K);
  extended(SVGA64K);
  extended(VESA16);
  extended(VESA256);
  extended(VESA32K);
  extended(VESA64K);
  extended(VESA16M);
  extended(ATI16);
  extended(ATI256);
  extended(ATI32K);
  extended(COMPAQ);
  extended(TSENG316);
  extended(TSENG3256);
  extended(TSENG416);
  extended(TSENG4256);
  extended(TSENG432K);
  extended(GENOA5);
  extended(GENOA6);
  extended(OAK);
  extended(PARADIS16);
  extended(PARADIS256);
  extended(TECMAR);
  extended(TRIDENT16);
  extended(TRIDENT256);
  extended(VIDEO7);
  extended(VIDEO7II);
  extended(S3);
  extended(ATIGUP);
  return 0;
}
