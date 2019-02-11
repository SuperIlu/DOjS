/*
 * Copyright (C) 1993-97 by Hartmut Schirmer
 */

#include <stdlib.h>    /* NULL, exit() */
#include <string.h>
#include "libbcc.h"

static void check_linked( void *ptr )
{
    if ( ptr == NULL )
    {
        exit(1);
    }
}

int main(void)
{
   check_linked(restorecrtmode);
   check_linked(closegraph);
   check_linked(getgraphmode);
   check_linked(getmaxmode);
   check_linked(getmoderange);
   check_linked(graphresult);
   check_linked(getx);
   check_linked(gety);
   check_linked(moveto);
   check_linked(moverel);
   check_linked(getbkcolor);
   check_linked(getcolor);
   check_linked(cleardevice);
   check_linked(setbkcolor);
   check_linked(setcolor);
   check_linked(line);
   check_linked(linerel);
   check_linked(lineto);
   check_linked(drawpoly);
   check_linked(bar);
   check_linked(circle);
   check_linked(ellipse);
   check_linked(arc);
   check_linked(getaspectratio);
   check_linked(setaspectratio);
   check_linked(getfillsettings);
   check_linked(getfillpattern);
   check_linked(sector);
   check_linked(pieslice);
   check_linked(setgraphbufsize);
   check_linked(getdefaultpalette);
   check_linked(installbgidriver);
   check_linked(registerfarbgidriver);
   check_linked(registerfarbgifont);
   check_linked(textlinestyle);
   check_linked(setpalette);
   check_linked(set_BGI_mode_whc);
   check_linked(set_BGI_mode_pages);
   check_linked(get_BGI_mode_pages);
   check_linked(getmodemaxcolor);
   check_linked(getmodemaxx);
   check_linked(getmodemaxy);
   check_linked(setrgbcolor);
   check_linked(setactivepage);
   check_linked(getactivepage);
   check_linked(setvisualpage);
   check_linked(getvisualpage);
   return 0;
}
