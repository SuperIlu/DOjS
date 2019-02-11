#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libbcc.h>
#include <sys/time.h>
#include "stdfun.h"

#ifdef __GNUC__
#  define getch() getkey()
#else
#  error fontplay requires DJGPP/GRX based compiler
#endif


long long Time(void) {
  struct timeval tm;
  
  gettimeofday(&tm,NULL);
  return ((long long)tm.tv_sec)*1000+(tm.tv_usec/1000);
}
  
int main(int argc, char *argv[])
{
  int gd, gm, i, font;
  int err;
  long long start, stop;

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
  
#if 0
  registerbgifont( &_bold_font);
  registerbgifont( &_euro_font);
  registerbgifont( &_goth_font);
  registerbgifont( &_lcom_font);
  registerbgifont( &_litt_font);
  registerbgifont( &_sans_font);
  registerbgifont( &_scri_font);
  registerbgifont( &_simp_font);
  registerbgifont( &_trip_font);
  registerbgifont( &_tscr_font);
#endif

  start = Time();
  for (i=0; i < 2500; ++i) {
    setcolor(i&15);
    for (font=TRIPLEX_FONT; font <= BOLD_FONT; ++font) {
      settextstyle(font, HORIZ_DIR, 1);
      outtextxy( 10, 10, "M");
    }
  }  
  stop = Time();  
  closegraph();
  printf("Time : %1.3fs\n", (stop-start)/1000.0);
  return 0;
}

