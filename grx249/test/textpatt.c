/**
 ** textpatt.c
 **
 ** This is a test/demo file of the GRX graphics library.
 ** You can use GRX test/demo files as you want.
 **
 ** The GRX graphics library is free software; you can redistribute it
 ** and/or modify it under some conditions; see the "copying.grx" file
 ** for details.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 **
 **/

#include <stdio.h>
#include <string.h>
#include "grx20.h"
#include "grxkeys.h"

#define FONT "../fonts/tms38b.fnt"

int main(void)
{
  char bits[] = {0, 76, 50, 0, 0, 76, 60, 0};
  GrPattern *p1, *p2;
  GrFont *font;
  GrTextOption opt;

  GrSetMode(GR_width_height_color_graphics, 320, 200, (GrColor)256);
  p1 = GrBuildPixmapFromBits(bits, 8, 8, 11,  3);
  p2 = GrBuildPixmapFromBits(bits, 8, 8,  3, 11);
  font = GrLoadFont(FONT);
  if (font && p1 && p2) {
    memset(&opt, 0, sizeof(GrTextOption));
    opt.txo_font   = font;
    opt.txo_xalign = 0;
    opt.txo_yalign = 0;
    opt.txo_direct = GR_TEXT_RIGHT;
    opt.txo_fgcolor.v = GrNOCOLOR;
    opt.txo_bgcolor.v = GrNOCOLOR;
    GrPatternFilledBox(0, 0, GrMaxX(), GrMaxY(), p1);
    GrKeyRead();
    GrPatternDrawString(" Hello world !", 15, 40, 10, &opt, p1);
    GrPatternDrawString(" Hello world !", 15, 44, 50, &opt, p2);
    GrPatternDrawStringExt(" Hello world !!", 16, 40, 100, &opt, p1);
    GrPatternDrawStringExt(" Hello world !!", 16, 44, 140, &opt, p2);
    GrKeyRead();
    opt.txo_bgcolor.v = GrBlack();
    GrPatternDrawString(" Hello world !", 15, 40, 10, &opt, p1);
    GrPatternDrawString(" Hello world !", 15, 44, 50, &opt, p2);
    GrPatternDrawStringExt(" Hello world !!", 16, 40, 100, &opt, p1);
    GrPatternDrawStringExt(" Hello world !!", 16, 44, 140, &opt, p2);
    GrKeyRead();
  }
  if (p1)   GrDestroyPattern(p1);
  if (p2)   GrDestroyPattern(p2);
  if (font) GrUnloadFont(font);
  GrSetMode(GR_default_text);
  if (!p1) fprintf(stderr, "Couldn't create first pattern\n");
  if (!p2) fprintf(stderr, "Couldn't create second pattern\n");
  if (!font) fprintf(stderr, "Couldn't load font %s\n", FONT);

  return 0;
}

