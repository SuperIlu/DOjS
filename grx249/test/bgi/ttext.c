#include <stdio.h>
#include <stdlib.h>

#include <libbcc.h>
#include "stdfun.h"

void play_font(char *name, char *file, int *x, int *y)
{
  int font;

  if (*file != '\0')
    font = installuserfont(file);
  else {
    font = DEFAULT_FONT;
    file = "DEFAULT_FONT";
  }
  if (font >= 0) {
    settextstyle(font, HORIZ_DIR, 1);
    outtextxy( *x, *y, file);
    outtextxy( *x+textwidth(file), *y, ": ");
    outtextxy( *x+textwidth(file)+textwidth(": "), *y, name);
  } else {
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    outtextxy( *x, *y, "Couldn't install ");
    outtextxy( *x+textwidth("Couldn't install "), *y, file);
  }
  *y += textheight( "M");
}

int Max(int a, int b) {
  if (a>b) return a;
  return b;
}

void user_test(char *txt, int mx, int dx, int my, int dy) {
  graphresult();
  setusercharsize(mx, dx, my, dy);
/*  if (graphresult() == grOk) */{
    moveto(0, gety()+Max(textheight(txt), 8)+8);
    outtext( txt);
  }
}

void user_info(int fnt, int sze) {
  char info[20];
  int w = textwidth("H");
  int h = textheight("H");
int x = getx(); int y = gety();
  settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
  sprintf(info, " H:%dx%d", w, h);
  outtextxy(x,y, info);
  settextstyle(fnt, HORIZ_DIR, sze);
}

void all_user_tests(int fnt, int sze) {
  graphresult();
  settextstyle(fnt, HORIZ_DIR, sze);
  settextjustify(LEFT_TEXT, BOTTOM_TEXT);
  if (graphresult() != grOk)
    return;
  user_test("Norm", 1, 1, 1, 1);
  user_info(fnt, sze);
  user_test("Short", 1, 2, 1, 1);
  user_info(fnt, sze);
  user_test("Wide", 2, 1, 1, 1);
  user_info(fnt, sze);
  user_test("no width", 0, 1, 1, 1);
  user_info(fnt, sze);
  user_test("no height", 1, 1, 0, 1);
  user_info(fnt, sze);
  user_test("neg x", -1, 1, 1, 1);
  user_info(fnt, sze);
  user_test("neg y",  1, 1,-1, 1);
  user_info(fnt, sze);
}

int main(void)
{
  int gd, gm, x, y;
  int err;
  char ch[2];

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
  setviewport( 50, 100, 150, 200, 1);
  rectangle( 0, 0, 100, 100);
  outtextxy( -20, 20, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
  outtextxy( -15, 40, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
  outtextxy( -10, 60, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
  outtextxy( -05, 80, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
  setviewport( getmaxx()-150, 100, getmaxx()-50, 200, 1);
  settextstyle(DEFAULT_FONT, VERT_DIR, 1);
  rectangle( 0, 0, 100, 100);
  outtextxy(  5, -5, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
  outtextxy( 15,  5, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
  outtextxy( 95, -5, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
  outtextxy(105,  5, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
  x = 50; y = 105;
  ch[0] = 'A'; ch[1] = '\0';
  while (y > 0) {
    outtextxy(x,y,ch);
    y -= textwidth(ch);
    ++ch[0];
  }
  settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
  setviewport( 0, 0, getmaxx(), getmaxy(), 1);
#ifdef __TURBOC__
  outtextxy( 10, 10, "Turbo-C cuts text");
#else
  outtextxy( 10, 10, "BCC2GRX clips text");
#endif
#ifdef __GNUC__
  setviewport( 100, 250, getmaxx()-100, getmaxy(), 1);
  rectangle( 0, 0, getmaxx()-200, getmaxy()-250);
  x = 5; y = 5;
  play_font( "8x8 bit mapped characters",       "",             &x, &y);
  play_font( "8x14 bit mapped characters",      "pc8x14.fnt",   &x, &y);
  play_font( "8x14 bit mapped characters thin", "pc8x14t.fnt",  &x, &y);
  play_font( "8x16 bit mapped characters",      "pc8x16.fnt",   &x, &y);
  play_font( "courier 16 pixel high",           "cour16.fnt",   &x, &y);
  play_font( "helvetica 17 pixel high italic",  "helv17i.fnt",  &x, &y);
  play_font( "helvetica 29 pixel bold italic",  "helv29bi.fnt", &x, &y);
  setviewport( 0, 0, getmaxx(), getmaxy(), 1);
#endif
  getch();
  cleardevice();

#if 0 && defined(__GNUC__)
   registerbgifont(&_sans_font);
#endif

   moveto(0, 0);
   all_user_tests(SANS_SERIF_FONT, 4);
   all_user_tests(DEFAULT_FONT, 1);

  getch();
  closegraph();
  return 0;
}
