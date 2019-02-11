/*
   GRAPHICS DEMO FOR Borland C++

   Copyright (c) 1987,88,91 Borland International. All rights reserved.
*/

/* Partially copyrighted (c) 1993-97 by Hartmut Schirmer */

#ifdef __TINY__
#error BGIDEMO will not run in the tiny model.
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include "libbcc.h"
#include "bgiext.h"
#include "stdfun.h"

#include "../rand.h"
#define Random(r) ((unsigned) (((RND() % (r)) + 1)))
#define Seed(s) SRND(s)

#if defined(__MSDOS__) || defined(__WIN32__)
#define BGI_PATH "..\\..\\chr"
#else
#define BGI_PATH "../../chr"
#endif

#define itoa(value,str,radix) sprintf((str),"%d",(value))
#define getch() getkey()

#define ESC     0x1b                    /* Define the escape key        */
#ifndef TRUE
#  define TRUE  1                       /* Define some handy constants  */
#endif
#ifndef FALSE
#  define FALSE 0                       /* Define some handy constants  */
#endif
#ifndef PI
#  define PI    3.14159                 /* Define a value for PI        */
#endif
#define ON      1                       /* Define some handy constants  */
#define OFF     0                       /* Define some handy constants  */

#define NFONTS 11

char *Fonts[] = {
  "DefaultFont",   "TriplexFont",   "SmallFont",
  "SansSerifFont", "GothicFont", "ScriptFont", "SimplexFont", "TriplexScriptFont",
  "ComplexFont", "EuropeanFont", "BoldFont"
};

char *LineStyles[] = {
  "SolidLn",  "DottedLn",  "CenterLn",  "DashedLn",  "UserBitLn"
};

char *FillStyles[] = {
  "EmptyFill",  "SolidFill",      "LineFill",      "LtSlashFill",
  "SlashFill",  "BkSlashFill",    "LtBkSlashFill", "HatchFill",
  "XHatchFill", "InterleaveFill", "WideDotFill",   "CloseDotFill"
};

char *TextDirect[] = {
  "HorizDir",  "VertDir"
};

char *HorizJust[] = {
  "LeftText",   "CenterText",   "RightText"
};

char *VertJust[] = {
  "BottomText",  "CenterText",  "TopText"
};

struct PTS {
  int x, y;
};      /* Structure to hold vertex points      */

int    GraphDriver;             /* The Graphics device driver           */
int    GraphMode;               /* The Graphics mode value              */
double AspectRatio;             /* Aspect ratio of a pixel on the screen*/
int    MaxX, MaxY;              /* The maximum resolution of the screen */
int    MaxColors;               /* The maximum # of colors available    */
int    ErrorCode;               /* Reports any graphics errors          */
struct palettetype palette;             /* Used to read palette info    */
static char PauseMsg[] = "Esc aborts or press a key...";
static char StopMsg[]  = "ESC Aborts - Press a Key to stop";

/*                                                                      */
/*      GPRINTF: Used like PRINTF except the output is sent to the      */
/*      screen in graphics mode at the specified co-ordinate.           */
/*                                                                      */

int gprintf( int *xloc, int *yloc, char *fmt, ... )
{
  va_list  argptr;                      /* Argument list pointer        */
  char str[140];                        /* Buffer to build sting into   */
  int cnt;                              /* Result of SPRINTF for return */

  va_start( argptr, fmt );              /* Initialize va_ functions     */

  cnt = vsprintf( str, fmt, argptr );   /* prints string to buffer      */
  outtextxy( *xloc, *yloc, str );       /* Send string in graphics mode */
  *yloc += textheight( "H" ) + 2;       /* Advance to next line         */

  va_end( argptr );                     /* Close va_ functions          */

  return( cnt );                        /* Return the conversion count  */

}

/*                                                                      */
/*      CHANGETEXTSTYLE: similar to settextstyle, but checks for        */
/*      errors that might occur whil loading the font file.             */
/*                                                                      */

void changetextstyle(int font, int direction, int charsize)
{
  int ErrorCode;

  graphresult();                        /* clear error code             */
  settextstyle(font, direction, charsize);
  ErrorCode = graphresult();            /* check result                 */
  if( ErrorCode != grOk ){              /* if error occured             */
#if 0
    closegraph();
    printf(" Graphics System Error: %s\n", grapherrormsg( ErrorCode ) );
    exit( 1 );
#else
    settextstyle(DEFAULT_FONT, direction, charsize);
#endif
  }
}

/*                                                                      */
/*      DRAWBORDER: Draw a solid single line around the current         */
/*      viewport.                                                       */
/*                                                                      */

void DrawBorder(int color)
{
  struct viewporttype vp;

  setcolor( color);

  setlinestyle( SOLID_LINE, 0, NORM_WIDTH );

  getviewsettings( &vp );
  rectangle( 0, 0, vp.right-vp.left, vp.bottom-vp.top);

}

/*                                                                      */
/*      STATUSLINE: Display a status line at the bottom of the screen.  */
/*                                                                      */

void StatusLineColor( char *msg, int color )
{
  int height;

  setviewport( 0, 0, MaxX, MaxY, 1 );   /* Open port to full screen     */
  setcolor( color);                     /* Set requested color          */

  changetextstyle( DEFAULT_FONT, HORIZ_DIR, 1 );
  settextjustify( CENTER_TEXT, TOP_TEXT );
  setlinestyle( SOLID_LINE, 0, NORM_WIDTH );
  setfillstyle( EMPTY_FILL, 0 );

  height = textheight( "H" );           /* Detemine current height      */
  bar( 0, MaxY-(height+4), MaxX, MaxY );
  rectangle( 0, MaxY-(height+4), MaxX, MaxY );
  outtextxy( MaxX/2, MaxY-(height+2), msg );
  setviewport( 1, height+5, MaxX-1, MaxY-(height+5), 1 );
}

void StatusLine( char *msg )
{
  StatusLineColor(msg, WHITE);
}

/*                                                                      */
/*      MAINWINDOW: Establish the main window for the demo and set      */
/*      a viewport for the demo code.                                   */
/*                                                                      */

void DisplayTitle(char *header, int color) {
  struct viewporttype vp;

  getviewsettings( &vp );
  setcolor( color);                     /* Set current requested color  */
  setviewport( 0, 0, MaxX, MaxY, 1 );   /* Open port to full screen     */
  changetextstyle( DEFAULT_FONT, HORIZ_DIR, 1 );
  settextjustify( CENTER_TEXT, TOP_TEXT );
  outtextxy( MaxX/2, 2, header );
  setviewport(vp.left, vp.top, vp.right, vp.bottom, vp.clip);
}

void MainWindowColor( char *header, int color)
{
  int height;

  cleardevice();                        /* Clear graphics screen        */
  setcolor( color);                     /* Set current requested color  */
  setviewport( 0, 0, MaxX, MaxY, 1 );   /* Open port to full screen     */

  height = textheight( "H" );           /* Get basic text height        */

  DisplayTitle(header, color);
  setviewport( 0, height+4, MaxX, MaxY-(height+4), 1 );
  DrawBorder(color);
  setviewport( 1, height+5, MaxX-1, MaxY-(height+5), 1 );

}

void MainWindow( char *header )
{
  MainWindowColor( header, WHITE);
}

/*                                                                      */
/*      PAUSE: Pause until the user enters a keystroke. If the          */
/*      key is an ESC, then exit program, else simply return.           */
/*                                                                      */


void NewPause(int clear)
{
  int c;

  StatusLine( PauseMsg );               /* Put msg at bottom of screen  */

  c = getch();                          /* Read a character from kbd    */

  if( ESC == c ){                       /* Does user wish to leave?     */
    closegraph();                       /* Change to text mode          */
    exit( 1 );                          /* Return to OS                 */
  }

  if( 0 == c ){                         /* Did use hit a non-ASCII key? */
    c = getch();                        /* Read scan code for keyboard  */
  }

  if (clear)
    cleardevice();                      /* Clear the screen             */
}

#define Pause() NewPause(TRUE)

/*                                                                      */
/*      INITIALIZE: Initializes the graphics system and reports         */
/*      any errors which occured.                                       */
/*                                                                      */

void Initialize(void)
{
  int xasp, yasp;

  GraphDriver = DETECT;                 /* Request auto-detection       */
#ifdef __GNUC__
/*  set_BGI_mode_whc(&GraphDriver, &GraphMode, 640, 480, 16); */
    set_BGI_mode_pages(2);
#endif
  initgraph( &GraphDriver, &GraphMode, BGI_PATH );
  ErrorCode = graphresult();            /* Read result of initialization*/
  if( ErrorCode != grOk ){              /* Error occured during init    */
    printf(" Graphics System Error: %s\n", grapherrormsg( ErrorCode ) );
    exit( 1 );
  }

  getpalette( &palette );               /* Read the palette from board  */
  MaxColors = getmaxcolor() + 1;        /* Read maximum number of colors*/
  if (MaxColors == 256)
    setrgbdefaults();

  MaxX = getmaxx();
  MaxY = getmaxy();                     /* Read size of screen          */

  getaspectratio( &xasp, &yasp );       /* read the hardware aspect     */
  AspectRatio = (double)xasp / (double)yasp; /* Get correction factor   */

}


/*                                                                      */
/*      REPORTSTATUS: Report the current configuration of the system    */
/*      after the auto-detect initialization.                           */
/*                                                                      */

void ReportStatus(void)
{
  struct viewporttype     viewinfo;     /* Params for inquiry procedures*/
  struct linesettingstype lineinfo;
  struct fillsettingstype fillinfo;
  struct textsettingstype textinfo;
  struct palettetype      palette;

  char *driver, *mode, *fmt;            /* Strings for driver and mode  */
  int x, y, mno;

  getviewsettings( &viewinfo );
  getlinesettings( &lineinfo );
  getfillsettings( &fillinfo );
  gettextsettings( &textinfo );
  getpalette( &palette );

  x = 5;
  y = 4;

  MainWindow( "Status report after InitGraph" );
  settextjustify( LEFT_TEXT, TOP_TEXT );

  driver = getdrivername();
  mode = getmodename(GraphMode);        /* get current setting          */

  gprintf( &x, &y, "Graphics device    : %-20s (%d)", driver, GraphDriver );
  gprintf( &x, &y, "Graphics mode      : %-20s (%d)", mode, GraphMode );
#ifdef __GNUC__
  gprintf( &x, &y, "Available pages    : %d", get_BGI_mode_pages() );
#endif
  gprintf( &x, &y, "Screen resolution  : ( 0, 0, %d, %d )", getmaxx(), getmaxy() );
  gprintf( &x, &y, "Current view port  : ( %d, %d, %d, %d )",
  viewinfo.left, viewinfo.top, viewinfo.right, viewinfo.bottom );
  gprintf( &x, &y, "Clipping           : %s", viewinfo.clip ? "ON" : "OFF" );

  gprintf( &x, &y, "Current position   : ( %d, %d )", getx(), gety() );
  gprintf( &x, &y, "Colors available   : %d", MaxColors );
  gprintf( &x, &y, "Current color      : %d", getcolor() );

  gprintf( &x, &y, "Line style         : %s", LineStyles[ lineinfo.linestyle ] );
  gprintf( &x, &y, "Line thickness     : %d", lineinfo.thickness );

  gprintf( &x, &y, "Current fill style : %s", FillStyles[ fillinfo.pattern ] );
  gprintf( &x, &y, "Current fill color : %d", fillinfo.color );

  gprintf( &x, &y, "Current font       : %s", Fonts[ textinfo.font ] );
  gprintf( &x, &y, "Text direction     : %s", TextDirect[ textinfo.direction ] );
  gprintf( &x, &y, "Character size     : %d", textinfo.charsize );
  gprintf( &x, &y, "Horizontal justify : %s", HorizJust[ textinfo.horiz ] );
  gprintf( &x, &y, "Vertical justify   : %s", VertJust[ textinfo.vert ] );

  gprintf( &x, &y, "Aspect ratio       : %lf", AspectRatio);

  getviewsettings( &viewinfo );
  {
    int ybase;
    setfillstyle(SOLID_FILL, BLACK);
    if (MaxY<350-1) {
      x = 400;
      y = 4;
      fmt = "#%-3d: %s";
    } else {
      y += 5;
      fmt = " Mode #%-3d : %s";
    }
    gprintf( &x, &y, "Available modes :");
    y += 5;
    ybase = y;
    for (mno = 0; mno <= getmaxmode(); ++mno) {
      char sp[100];
      sprintf(sp, fmt, mno, getmodename(mno));
      bar(x-4, y + textheight(sp), x+textwidth(sp)+4, y);
      gprintf( &x, &y, "%s", sp);
      if (y+viewinfo.top>viewinfo.bottom-8) {
	y = ybase;
	x += (viewinfo.right-viewinfo.left) / 2;
      }
    }
  }

  Pause();                              /* Pause for user to read screen*/

}

/*                                                                      */
/*      TEXTDUMP: Display the all the characters in each of the         */
/*      available fonts.                                                */
/*                                                                      */

void TextDump(void)
{
  static int CGASizes[]  = {
    1, 3, 7, 3, 3, 2, 2, 2, 2, 2, 2  };
  static int NormSizes[] = {
    1, 4, 7, 4, 4, 2, 2, 2, 2, 2, 2  };

  char buffer[80];
  int font, ch, wwidth, lwidth, size;
  struct viewporttype vp;

  for( font=0 ; font<NFONTS ; ++font ){ /* For each available font      */
    sprintf( buffer, "%s Character Set", Fonts[font] );
    MainWindow( buffer );               /* Display fontname as banner   */
    getviewsettings( &vp );             /* read current viewport        */

    settextjustify( LEFT_TEXT, TOP_TEXT );
    moveto( 5, 3 );

    buffer[1] = '\0';                   /* Terminate string             */
    wwidth = vp.right - vp.left;        /* Determine the window width   */

    if( font == DEFAULT_FONT ){
      changetextstyle( font, HORIZ_DIR, 1 );
      lwidth = textwidth( "H" );        /* Get average letter width     */
      ch = 0;
      while( ch < 256 ){                /* For each possible character  */
	buffer[0] = ch;                 /* Put character into a string  */
	if( (getx() + lwidth) > wwidth-3)
	  moveto( 5, gety() + textheight("H") + 3 );
	outtext( buffer );              /* send string to screen        */
	++ch;                           /* Goto the next character      */
      }
    }
    else{

      size = (MaxY < 200) ? CGASizes[font] : NormSizes[font];
      changetextstyle( font, HORIZ_DIR, size );

      ch = '!';                         /* Begin at 1st printable       */
      while( ch < 256 ){                /* For each printable character */
	buffer[0] = ch;                 /* Put character into a string  */
	lwidth = textwidth( buffer);    /* Get letter width             */
	if( (lwidth+getx()) > wwidth-3) /* Are we still in window?      */
	  moveto( 5, gety()+textheight("H")+3 );
	outtext( buffer );              /* send string to screen        */
	++ch;                           /* Goto the next character      */
      }

    }

    Pause();                            /* Pause until user acks        */

  }                                     /* End of FONT loop             */

}

/*                                                                      */
/*      BAR3DDEMO: Display a 3-D bar chart on the screen.               */
/*                                                                      */

void Bar3DDemo(void)
{
  static int barheight[] = {
    1, 3, 5, 4, 3, 2, 1, 5, 4, 2, 3   };
  struct viewporttype vp;
  int xstep, ystep, deepth;
  int i, j, h, color, bheight;
  char buffer[10];

  MainWindow( "Bar 3-D / Rectangle Demonstration" );

  h = 3 * textheight( "H" );
  getviewsettings( &vp );
  settextjustify( CENTER_TEXT, TOP_TEXT );
  changetextstyle( TRIPLEX_FONT, HORIZ_DIR, 4 );
  outtextxy( MaxX/2, 6, "These are 3-D Bars" );
  changetextstyle( DEFAULT_FONT, HORIZ_DIR, 1 );
  setviewport( vp.left+50, vp.top+40, vp.right-50, vp.bottom-10, 1 );
  getviewsettings( &vp );

  line( h, h, h, vp.bottom-vp.top-h );
  line( h, (vp.bottom-vp.top)-h, (vp.right-vp.left)-h, (vp.bottom-vp.top)-h );
  xstep = ((vp.right-vp.left) - (2*h)) / 10;
  ystep = ((vp.bottom-vp.top) - (2*h)) / 5;
  j = (vp.bottom-vp.top) - h;
  deepth = (getmaxx() <= 400) ? 7 : 15;
  settextjustify( LEFT_TEXT, CENTER_TEXT );

  for( i=0 ; i<6 ; ++i ){
    line( h/2, j, h, j );
    itoa( i, buffer, 10 );
    outtextxy( 0, j, buffer );
    j -= ystep;
  }

  j = h;
  settextjustify( CENTER_TEXT, TOP_TEXT );

  for( i=0 ; i<11 ; ++i ){
    color = Random( MaxColors-1 ) + 1;
    setfillstyle( i+1, color );
    line( j, (vp.bottom-vp.top)-h, j, (vp.bottom-vp.top-3)-(h/2) );
    itoa( i, buffer, 10 );
    outtextxy( j, (vp.bottom-vp.top)-(h/2), buffer );
    if( i != 10 ){
      bheight = (vp.bottom-vp.top) - h - 1;
      bar3d( j, (vp.bottom-vp.top-h)-(barheight[i]*ystep), j+xstep-deepth, bheight, deepth, 1 );
    }
    j += xstep;
  }

  Pause();                              /* Pause for user's response    */

}

/*                                                                      */
/*      RandomBARS: Display random bars                                 */
/*                                                                      */

void RandomBars(void)
{
  int color;

  MainWindow( "Random Bars" );
  StatusLine( PauseMsg );               /* Put msg at bottom of screen   */
  while( !kbhit() ){                    /* Until user enters a key...   */
    color = Random( MaxColors-1 )+1;
    setcolor( color );
    setfillstyle( Random(11)+1, color );
    bar3d( Random( getmaxx() ), Random( getmaxy() ),
       Random( getmaxx() ), Random( getmaxy() ), 0, OFF);
  }

  Pause();                              /* Pause for user's response    */

}


/*                                                                      */
/*      TEXTDEMO: Show each font in several sizes to the user.          */
/*                                                                      */

void TextDemo(void)
{
  int charsize[] = {
    1, 3, 7, 3, 4, 2, 2, 2, 2, 2, 2   };
  int font, size;
  int h, x, y, i;
  struct viewporttype vp;
  char buffer[80];

  for( font=0 ; font<NFONTS ; ++font ){ /* For each of the avail. fonts */

    sprintf( buffer, "%s Demonstration", Fonts[font] );
    MainWindow( buffer );
    getviewsettings( &vp );

    changetextstyle( font, VERT_DIR, charsize[font] );
    settextjustify( CENTER_TEXT, BOTTOM_TEXT );
    outtextxy( 2*textwidth("M"), vp.bottom - 2*textheight("M"), "Vertical" );

    changetextstyle( font, HORIZ_DIR, charsize[font] );
    settextjustify( LEFT_TEXT, TOP_TEXT );
    outtextxy( 2*textwidth("M"), 2, "Horizontal" );

    settextjustify( CENTER_TEXT, CENTER_TEXT );
    x = (vp.right - vp.left) / 2;
    y = textheight( "H" );

    for( i=1 ; i<5 ; ++i ){             /* For each of the sizes */
      size = (font == SMALL_FONT) ? i+3 : i;
      changetextstyle( font, HORIZ_DIR, size );
      h = textheight( "H" );
      y += h;
      sprintf( buffer, "Size %d", size );
      outtextxy( x, y, buffer );

    }

    if( font != DEFAULT_FONT ){         /* Show user declared font size */
      y += h / 2;                       /* Move down the screen         */
      settextjustify( CENTER_TEXT, TOP_TEXT );
      setusercharsize( 5, 6, 3, 2 );
      changetextstyle( font, HORIZ_DIR, USER_CHAR_SIZE );
      outtextxy( (vp.right-vp.left)/2, y, "User Defined Size" );
    }

    Pause();                            /* Pause to let user look       */

  }                                     /* End of FONT loop             */

}

/*                                                                      */
/*      COLORDEMO: Display the current color palette on the screen.     */
/*                                                                      */

void ColorDemo(void)
{
  struct viewporttype vp;
  int color, height, width, ec;
  int x, y, i, j;
  char cnum[5];

  MainWindow( "Color Demonstration" );  /* Show demonstration name      */

  color = 1;
  getviewsettings( &vp );               /* Get the current window size  */
  width  = 2 * ( (vp.right+1) / 16 );      /* Get box dimensions           */
  height = 2 * ( (vp.bottom-10) / 10 );

  x = width / 2;
  y = height / 2;       /* Leave 1/2 box border         */

  for( j=0 ; j<3 ; ++j ){               /* Row loop                     */

    for( i=0 ; i<5 ; ++i ){             /* Column loop                  */

      ec = _ega_color(color);
      setfillstyle(SOLID_FILL, ec);     /* Set to solid fill in color   */
      setcolor( ec );                   /* Set the same border color    */

      bar( x, y, x+width, y+height );   /* Draw the rectangle           */
      rectangle( x, y, x+width, y+height );  /* outline the rectangle   */

      if( color == BLACK ){             /* If box was black...          */
	setcolor( _ega_color(WHITE) );  /* Set drawing color to white   */
	rectangle( x, y, x+width, y+height );  /* Outline black in white*/
      }

      itoa( color, cnum, 10 );          /* Convert # to ASCII           */
      outtextxy( x+(width/2), y+height+4, cnum );  /* Show color #      */

      color = (color + 1) % MaxColors;  /* Advance to the next color    */
      x += (width / 2) * 3;             /* move the column base         */
    }                           /* End of Column loop           */

    y += (height / 2) * 3;              /* move the row base            */
    x = width / 2;                      /* reset column base            */
  }                                     /* End of Row loop              */

  Pause();                              /* Pause for user's response    */

}

/*                                                                      */
/*      ARCDEMO: Display a random pattern of arcs on the screen */
/*      until the user says enough.                                     */
/*                                                                      */

void ArcDemo(void)
{
  int mradius;                          /* Maximum radius allowed       */
  int eangle;                           /* Random end angle of Arc      */
  struct arccoordstype ai;              /* Used to read Arc Cord info   */

  MainWindow( "Arc Demonstration" );
  StatusLine( StopMsg);

  mradius = (int)(MaxY / (10.0*AspectRatio));  /* Determine the maximum radius */

  while( !kbhit() ){                    /* Repeat until a key is hit    */
    setcolor( Random( MaxColors - 1 ) + 1 );    /* randomly select a color      */
    eangle = Random( 358 ) + 1;         /* Select an end angle          */
    arc( Random(MaxX), Random(MaxY), Random(eangle), eangle, mradius );
    getarccoords( &ai );                /* Read Cord data               */
    line( ai.x, ai.y, ai.xstart, ai.ystart ); /* line from start to center */
    line( ai.x, ai.y, ai.xend,   ai.yend );   /* line from end to center   */
  }                                     /* End of WHILE not KBHIT       */

  Pause();                              /* Wait for user's response     */

}

/*                                                                      */
/*      CIRCLEDEMO: Display a random pattern of circles on the screen   */
/*      until the user says enough.                                     */
/*                                                                      */

void CircleDemo(void)
{
  int mradius;                          /* Maximum radius allowed       */

  MainWindow( "Circle Demonstration" );
  StatusLine( StopMsg);

  mradius = (int)(MaxY / (10.0*AspectRatio));  /* Determine the maximum radius */

  while( !kbhit() ){                    /* Repeat until a key is hit    */
    setcolor( Random( MaxColors - 1 ) + 1 );    /* Randomly select a color      */
    circle( Random(MaxX), Random(MaxY), Random(mradius) );
  }                                     /* End of WHILE not KBHIT       */

  Pause();                              /* Wait for user's response     */

}

/*                                                                      */
/*      PIEDEMO: Display a pie chart on the screen.                     */
/*                                                                      */

#define adjasp( y )     ((int)(AspectRatio * (double)(y)))
#define torad( d )      (( (double)(d) * PI ) / 180.0 )

void PieDemo(void)
{
  struct viewporttype vp;
  int xcenter, ycenter, radius, lradius;
  int x, y;
  double radians, piesize;

  MainWindow( "Pie Chart Demonstration" );

  getviewsettings( &vp );               /* Get the current viewport     */
  xcenter = (vp.right - vp.left) / 2;   /* Center the Pie horizontally  */
  ycenter = (vp.bottom - vp.top) / 2+20;/* Center the Pie vertically    */
  radius  = (vp.bottom - vp.top) / 3;   /* It will cover 2/3rds screen  */
  piesize = (vp.bottom - vp.top) / 4.0; /* Optimum height ratio of pie  */

  if (AspectRatio >= 1.0)
    radius = (int)(radius / AspectRatio);
  while( (AspectRatio*radius) < piesize ) ++radius;

  lradius = radius + ( radius / 5 );    /* Labels placed 20% farther    */

  changetextstyle( TRIPLEX_FONT, HORIZ_DIR, 4 );
  settextjustify( CENTER_TEXT, TOP_TEXT );
  outtextxy( MaxX/2, 6, "This is a Pie Chart" );
  changetextstyle( TRIPLEX_FONT, HORIZ_DIR, 1 );
  settextjustify( CENTER_TEXT, TOP_TEXT );

  setfillstyle( SOLID_FILL, _ega_color(RED) );
  pieslice( xcenter+10, ycenter-adjasp(10), 0, 90, radius );
  radians = torad( 45 );
  x = xcenter + (int)( cos( radians ) * (double)lradius );
  y = ycenter - (int)( sin( radians ) * (double)lradius * AspectRatio );
  settextjustify( LEFT_TEXT, BOTTOM_TEXT );
  outtextxy( x, y, "25 %" );

  setfillstyle( WIDE_DOT_FILL, _ega_color(GREEN) );
  pieslice( xcenter, ycenter, 90, 135, radius );
  radians = torad( 113 );
  x = xcenter + (int)( cos( radians ) * (double)lradius );
  y = ycenter - (int)( sin( radians ) * (double)lradius * AspectRatio );
  settextjustify( RIGHT_TEXT, BOTTOM_TEXT );
  outtextxy( x, y, "12.5 %" );

  setfillstyle( INTERLEAVE_FILL, _ega_color(YELLOW) );
  settextjustify( RIGHT_TEXT, CENTER_TEXT );
  pieslice( xcenter-10, ycenter, 135, 225, radius );
  radians = torad( 180 );
  x = xcenter + (int)( cos( radians ) * (double)lradius );
  y = ycenter - (int)( sin( radians ) * (double)lradius * AspectRatio );
  settextjustify( RIGHT_TEXT, CENTER_TEXT );
  outtextxy( x, y, "25 %" );

  setfillstyle( HATCH_FILL, _ega_color(BLUE) );
  pieslice( xcenter, ycenter, 225, 360, radius );
  radians = torad( 293 );
  x = xcenter + (int)( cos( radians ) * (double)lradius );
  y = ycenter - (int)( sin( radians ) * (double)lradius * AspectRatio );
  settextjustify( LEFT_TEXT, TOP_TEXT );
  outtextxy( x, y, "37.5 %" );

  Pause();                              /* Pause for user's response    */

}

/*                                                                      */
/*      BARDEMO: Draw a 2-D bar chart using Bar and Rectangle.          */
/*                                                                      */

void BarDemo(void)
{
  int barheight[] = {
    1, 3, 5, 2, 4   };
  int styles[]    = {
    1, 3, 10, 5, 9, 1   };
  int xstep, ystep;
  int sheight, swidth;
  int i, j, h;
  struct viewporttype vp;
  char buffer[40];

  MainWindow( "Bar / Rectangle demonstration" );
  h = 3 * textheight( "H" );
  getviewsettings( &vp );
  settextjustify( CENTER_TEXT, TOP_TEXT );
  changetextstyle( TRIPLEX_FONT, HORIZ_DIR, 4 );
  outtextxy( MaxX /2, 6, "These are 2-D Bars" );
  changetextstyle( DEFAULT_FONT, HORIZ_DIR, 1 );
  setviewport( vp.left+50, vp.top+30, vp.right-50, vp.bottom-10, 1 );

  getviewsettings( &vp );
  sheight = vp.bottom - vp.top;
  swidth  = vp.right  - vp.left;

  line( h, h, h, sheight-h );
  line( h, sheight-h, sheight-h, sheight-h );
  ystep = (sheight - (2*h) ) / 5;
  xstep = (swidth  - (2*h) ) / 5;
  j = sheight - h;
  settextjustify( LEFT_TEXT, CENTER_TEXT );

  for( i=0 ; i<6 ; ++i ){
    line( h/2, j, h, j );
    itoa( i, buffer, 10 );
    outtextxy( 0, j, buffer );
    j -= ystep;
  }

  j = h;
  settextjustify( CENTER_TEXT, TOP_TEXT );
  for( i=0 ; i<6 ; ++i ){
    setfillstyle( styles[i], Random(MaxColors-1)+1 );
    line( j, sheight - h, j, sheight- 3 - (h/2) );
    itoa( i, buffer, 10 );
    outtextxy( j, sheight - (h/2), buffer );
    if( i != 5 ){
      bar( j, (sheight-h)-(barheight[i] * ystep), j+xstep, sheight-h-1 );
      rectangle( j, (sheight-h)-(barheight[i] * ystep), j+xstep, sheight-h);
    }
    j += xstep;
  }

  Pause();

}

/*                                                                      */
/*      LINERELDEMO: Display pattern using moverel and linerel cmds.    */
/*                                                                      */

void LineRelDemo(void)
{
  struct viewporttype vp;
  int h, w, dx, dy, cx, cy;
  struct PTS outs[7];


  MainWindow( "MoveRel / LineRel Demonstration" );
  StatusLine( StopMsg);

  getviewsettings( &vp );
  cx = (vp.right  - vp.left) / 2;       /* Center of the screen coords  */
  cy = (vp.bottom - vp.top ) / 2;

  h  = (vp.bottom - vp.top ) / 8;
  w  = (vp.right  - vp.left) / 9;

  dx = 2 * w;
  dy = 2 * h;

  setcolor( BLACK );

  setfillstyle( SOLID_FILL, _ega_color(BLUE) );
  bar( 0, 0, vp.right-vp.left, vp.bottom-vp.top );      /* Draw backgnd */

  outs[0].x = cx -  dx;
  outs[0].y = cy -  dy;
  outs[1].x = cx - (dx-w);
  outs[1].y = cy - (dy+h);
  outs[2].x = cx +  dx;
  outs[2].y = cy - (dy+h);
  outs[3].x = cx +  dx;
  outs[3].y = cy +  dy;
  outs[4].x = cx + (dx-w);
  outs[4].y = cy + (dy+h);
  outs[5].x = cx -  dx;
  outs[5].y = cy + (dy+h);
  outs[6].x = cx -  dx;
  outs[6].y = cy -  dy;

  setfillstyle( SOLID_FILL, WHITE );
  fillpoly( 7, (int far *)outs );

  outs[0].x = cx - (w/2);
  outs[0].y = cy + h;
  outs[1].x = cx + (w/2);
  outs[1].y = cy + h;
  outs[2].x = cx + (w/2);
  outs[2].y = cy - h;
  outs[3].x = cx - (w/2);
  outs[3].y = cy - h;
  outs[4].x = cx - (w/2);
  outs[4].y = cy + h;

  setfillstyle( SOLID_FILL, _ega_color(BLUE) );
  fillpoly( 5, (int far *)outs );

  /*    Draw a Tesseract object on the screen using the LineRel and     */
  /*    MoveRel drawing commands.                                       */

  moveto( cx-dx, cy-dy );
  linerel(  w, -h );
  linerel(  3*w,        0 );
  linerel(   0,  5*h );
  linerel( -w,  h );
  linerel( -3*w,        0 );
  linerel(   0, -5*h );

  moverel( w, -h );
  linerel(   0,  5*h );
  linerel( w+(w/2), 0 );
  linerel(   0, -3*h );
  linerel( w/2,   -h );
  linerel( 0, 5*h );

  moverel(  0, -5*h );
  linerel( -(w+(w/2)), 0 );
  linerel( 0, 3*h );
  linerel( -w/2, h );

  moverel( w/2, -h );
  linerel( w, 0 );

  moverel( 0, -2*h );
  linerel( -w, 0 );

  Pause();                              /* Wait for user's response     */

}

/*                                                                      */
/*      PUTPIXELDEMO: Display a pattern of random dots on the screen    */
/*      and pick them back up again.                                    */
/*                                                                      */

void PutPixelDemo(void)
{
  int seed = 1958;
  int i, x, y, h, w, color;
  struct viewporttype vp;

  MainWindow( "PutPixel / GetPixel Demonstration" );
  StatusLine( PauseMsg);                /* Put msg at bottom of screen   */

  getviewsettings( &vp );
  h = vp.bottom - vp.top;
  w = vp.right  - vp.left;

  do {
    Seed( seed );                          /* Restart random # function    */

    for( i=0 ; i<5000 ; ++i ){             /* Put 5000 pixels on screen    */
      x = 1 + Random( w - 1 );             /* Generate a random location   */
      y = 1 + Random( h - 1 );
      color = Random( MaxColors-1 ) + 1;
      putpixel( x, y, color );
    }

    Seed( seed );                          /* Restart random # at same #   */
    for( i=0 ; i<5000 ; ++i ){             /* Take the 5000 pixels off      */
      x = 1 + Random( w - 1 );             /* Generate a random location   */
      y = 1 + Random( h - 1 );
      color = getpixel( x, y );            /* Read the color pixel          */
      if( color==Random(MaxColors-1)+1 )   /* Used to keep random in sync   */
	putpixel( x, y, BLACK );           /* Write pixel to BLACK          */
    }
    if (!kbhit())
      delay(400);
  } while ( !kbhit());

  Pause();                              /* Wait for user's response     */

}

/*                                                                      */
/*   PUTIMAGEDEMO                                                       */
/*                                                                      */
#define PAUSETIME 20
#define PID_r     20
#define StartX    100
#define StartY    50
#define MAXXSTEP  (2*PID_r/3)
#define MAXYSTEP  (PID_r/2)
#define PID_STEPS 250

int SaucerMoveX(int *dx, int x) {
//    *dx += (Random( 2*MAXXSTEP+1) - MAXXSTEP + (MAXXSTEP*(MaxX/2-x))/MaxX) / 10;
    *dx += Random( 2*MAXXSTEP) - MAXXSTEP;
    if ( *dx >  MAXXSTEP) *dx =  MAXXSTEP; else
    if ( *dx < -MAXXSTEP) *dx = -MAXXSTEP;
    return *dx;
}
int SaucerMoveY(int *dy, int y) {
//    *dy += (Random( 2*MAXYSTEP+1) - MAXYSTEP + (MAXYSTEP*(MaxY/2-y))/MaxY) / 10;
    *dy += Random( 2*MAXYSTEP) - MAXYSTEP;
    if ( *dy >  MAXYSTEP) *dy =  MAXYSTEP; else
    if ( *dy < -MAXYSTEP) *dy = -MAXYSTEP;
    return *dy;
}

#define SaucerLimitX() do {                             \
    if (vp.left + nx + width - 1 > vp.right)            \
      nx = vp.right-vp.left-width + 1;                  \
    else                                                \
      if (nx < 0)                                       \
	nx = 0;                                         \
} while (0)

#define SaucerLimitY() do {                             \
    if (vp.top + ny + height - 1 > vp.bottom)           \
      ny = vp.bottom-vp.top-height + 1;                 \
    else                                                \
      if (ny < 0)                                       \
	ny = 0;                                         \
} while (0)

void PutImageDemo(void)
{
  struct viewporttype vp;
  int    x, y, ulx, uly, lrx, lry, size, i, width, height;
  int    nx, ny, dx, dy;
  void   *Saucer;
  int    old_xasp, old_yasp;

  MainWindow("GetImage / PutImage Demonstration");
  getviewsettings( &vp );

  /* DrawSaucer */
  getaspectratio(&old_xasp, &old_yasp);
  setaspectratio(1, 1);
  ellipse(StartX, StartY, 0, 360, PID_r, PID_r / 3 + 2);
  ellipse(StartX, StartY - 4, 190, 357, PID_r, PID_r / 3);
  line(StartX + 7, StartY - 6, StartX + 10, StartY - 12);
  circle(StartX + 10, StartY - 12, 2);
  line(StartX - 7, StartY - 6, StartX - 10, StartY - 12);
  circle(StartX - 10, StartY - 12, 2);
  setfillstyle(SOLID_FILL, WHITE);
  floodfill(StartX + 1, StartY + 4, getcolor());
  setaspectratio(old_xasp, old_yasp);

  /* Read saucer image */
  ulx = StartX-(PID_r+1);
  uly = StartY-14;
  lrx = StartX+(PID_r+1);
  lry = StartY+(PID_r/3)+3;
  width = lrx - ulx + 1;
  height = lry - uly + 1;
  size = imagesize(ulx, uly, lrx, lry);
  Saucer = malloc( size );
  if (Saucer == NULL) return;
  getimage(ulx, uly, lrx, lry, Saucer);
  putimage(ulx, uly, Saucer, XOR_PUT);

  /* Plot some "stars"  */
  for ( i=0 ; i<1000; ++i )
    putpixel(Random(MaxX), Random(MaxY), Random( MaxColors-1 )+1);
  x = MaxX / 2;
  y = MaxY / 2;
  dx = 1;
  dy = 0;

  StatusLine( PauseMsg);                /* Put msg at bottom of screen   */

  /* until a key is hit */
  while ( !kbhit() ) {

    /* Draw the Saucer */
    if (dx != 0 || dy != 0)
      putimage(x, y, Saucer, XOR_PUT);                /*  draw image  */
    delay(PAUSETIME);
    nx = x + SaucerMoveX(&dx,x);
    ny = y + SaucerMoveY(&dy,y);
    SaucerLimitX();
    SaucerLimitY();
    dx = nx-x;
    dy = ny-y;
    if (dx != 0 || dy != 0)
      putimage(x, y, Saucer, XOR_PUT);               /* erase image  */
    x = nx;
    y = ny;
  }

#ifdef __GNUC__
  if (get_BGI_mode_pages()>1) {
    int ActPage = 0;
    void *Screen = NULL;

    size = imagesize(0, 0, MaxX, MaxY);
    Screen = malloc( size );
    if (Screen != NULL) {
      if ( getch() == ESC) {
	closegraph();
	exit(1);
      }
      setviewport(0, 0, MaxX, MaxY, 1);
      getimage(0, 0, MaxX, MaxY, Screen);
      while ( !kbhit() ) {
	ActPage = (ActPage+1)&1;
	setactivepage(ActPage);
	putimage(0, 0, Screen, COPY_PUT);
	putimage(vp.left+x, vp.top+y, Saucer, XOR_PUT );
	setvisualpage(ActPage);
	nx = x + SaucerMoveX(&dx, x);
	ny = y + SaucerMoveY(&dy, y);
	SaucerLimitX();
	SaucerLimitY();
	dx = nx-x;
	dy = ny-y;
	x = nx;
	y = ny;
      }
      setactivepage(0);
      putimage(0, 0, Screen, COPY_PUT);
      setvisualpage(0);
      setactivepage(1);
      cleardevice();
      setactivepage(0);
      setviewport(vp.left, vp.top, vp.right, vp.bottom, vp.clip);
      free(Screen);
    }
  }
#endif

  free( Saucer );
  Pause();
}
#undef PAUSETIME
#undef PID_r
#undef StartX
#undef StartY
#undef MAXXSTEP
#undef MAXYSTEP
#undef PID_STEPS

/*                                                                      */
/*      LINETODEMO: Display a pattern using moveto and lineto commands. */
/*                                                                      */

#define MAXPTS  15

void LineToDemo(void)
{
  struct viewporttype vp;
  struct PTS points[MAXPTS];
  int i, j, h, w, xcenter, ycenter;
  int radius, angle, step;
  double  rads;

  MainWindow( "MoveTo / LineTo Demonstration" );

  getviewsettings( &vp );
  h = vp.bottom - vp.top;
  w = vp.right  - vp.left;

  xcenter = w / 2;                      /* Determine the center of circle */
  ycenter = h / 2;
  radius  = (int)( (h-30) / (AspectRatio*2) );
  step    = 360 / MAXPTS;               /* Determine # of increments    */

  angle = 0;                            /* Begin at zero degrees        */
  for( i=0 ; i<MAXPTS ; ++i ){          /* Determine circle intercepts  */
    rads = (double)angle * PI / 180.0;  /* Convert angle to radians     */
    points[i].x = xcenter + (int)( cos(rads) * radius );
    points[i].y = ycenter - (int)( sin(rads) * radius * AspectRatio );
    angle += step;                      /* Move to next increment       */
  }

  circle( xcenter, ycenter, radius );   /* Draw bounding circle         */

  for( i=0 ; i<MAXPTS ; ++i ){          /* Draw the cords to the circle */
    for( j=i ; j<MAXPTS ; ++j ){        /* For each remaining intersect */
      moveto(points[i].x, points[i].y); /* Move to beginning of cord    */
      lineto(points[j].x, points[j].y); /* Draw the cord                */
    }
  }

  Pause();                              /* Wait for user's response     */

}

/*                                                                      */
/*      LINESTYLEDEMO: Display a pattern using all of the standard      */
/*      line styles that are available.                                 */
/*                                                                      */

void LineStyleDemo(void)
{
  int style, step;
  int x, y, w;
  struct viewporttype vp;
  char buffer[40];

  MainWindow( "Pre-defined line styles" );

  getviewsettings( &vp );
  w = vp.right  - vp.left;

  x = 35;
  y = 10;
  step = w / 11;

  settextjustify( LEFT_TEXT, TOP_TEXT );
  outtextxy( x, y, "Normal Width" );

  settextjustify( CENTER_TEXT, TOP_TEXT );

  for( style=0 ; style<4 ; ++style ){
    setlinestyle( style, 0, NORM_WIDTH );
    line( x, y+20, x, vp.bottom-40 );
    itoa( style, buffer, 10 );
    outtextxy( x, vp.bottom-30, buffer );
    x += step;
  }

  x += 2 * step;

  settextjustify( LEFT_TEXT, TOP_TEXT );
  outtextxy( x, y, "Thick Width" );
  settextjustify( CENTER_TEXT, TOP_TEXT );

  for( style=0 ; style<4 ; ++style ){
    setlinestyle( style, 0, THICK_WIDTH );
    line( x, y+20, x, vp.bottom-40 );
    itoa( style, buffer, 10 );
    outtextxy( x, vp.bottom-30, buffer );
    x += step;
  }

  settextjustify( LEFT_TEXT, TOP_TEXT );

  Pause();                              /* Wait for user's response     */

}

/*                                                                      */
/*      CRTMODEDEMO: Demonstrate the effects of the change mode         */
/*      commands on the current screen.                                 */
/*                                                                      */

void CRTModeDemo(void)
{
  struct viewporttype vp;
  int mode;
  char m[41];

  MainWindow( "SetGraphMode / RestoreCRTMode demo" );
  getviewsettings( &vp );
  mode = getgraphmode();
  settextjustify( CENTER_TEXT, CENTER_TEXT );

  outtextxy( (vp.right-vp.left)/2, (vp.bottom-vp.top)/2,
  "Now you are in graphics mode..." );
  StatusLine( "Press any key for text mode..." );
  getch();

  restorecrtmode();
  printf( "Now you are in text mode.\n\n" );
  printf( "Press <CR> to go back to graphics..." );
  fflush(stdout);
  fgets(m,40,stdin);

  setgraphmode( mode );
  MainWindow( "SetGraphMode / RestoreCRTMode demo" );
  settextjustify( CENTER_TEXT, CENTER_TEXT );
  outtextxy( (vp.right-vp.left)/2, (vp.bottom-vp.top)/2,
  "Back in Graphics Mode..." );

  Pause();                              /* Wait for user's response     */

}

/*                                                                      */
/*      USERLINESTYLEDEMO: Display line styles showing the user         */
/*      defined line style functions.                                   */
/*                                                                      */

void UserLineStyleDemo(void)
{
  static unsigned msk_or[3]  = { 0x0000, 0x0000, 0x8001 };
  static unsigned msk_and[3] = { 0xFFFF, 0x7FFE, 0xFFFF };
  int x, y, i, h, flag;
  unsigned int style, msk_cnt;
  struct viewporttype vp;

  MainWindow( "User defined line styles" );

  getviewsettings( &vp );
  h = vp.bottom - vp.top;

  x = 4;
  y = 10;
  style = 0;
  msk_cnt = 0;
  i = 0;

  settextjustify( CENTER_TEXT, TOP_TEXT );
  flag = TRUE;                          /* Set the bits in this pass    */

  while( x < vp.right-2 ){              /* Draw lines across the screen */

    if( flag )                          /* If flag, set bits...         */
      style |= (1 << i);                /*    Set the Ith bit in word   */
    else                                /* If no flag, clear bits       */
      style &= ~(1<<i);                 /*    Clear the Ith bit in word */

    setlinestyle( USERBIT_LINE,
		  (style|msk_or[msk_cnt])&msk_and[msk_cnt],
		  NORM_WIDTH );
    line( x, y, x, h-y );               /* Draw the new line pattern    */

    x += 5;                             /* Move the X location of line  */
    i = (i + 1) % 16;                   /* Advance to next bit pattern  */

    if( style == 0xffff ){              /* Are all bits set?            */
      flag = FALSE;                     /*   begin removing bits        */
      i = 0;                            /* Start with whole pattern     */
    }
    else {                              /* Bits not all set...          */
      if( style == 0 ) {                /* Are all bits clear?          */
	flag = TRUE;                    /*   begin setting bits         */
	i = 0;
	if (++msk_cnt >= 3) msk_cnt = 0;
      }
    }
  }

  settextjustify( LEFT_TEXT, TOP_TEXT );

  Pause();                              /* Wait for user's response     */

}

/*                                                                      */
/*      FILLSTYLEDEMO: Display the standard fill patterns available.    */
/*                                                                      */

void FillStyleDemo(void)
{
  int h, w, style;
  int i, j, x, y;
  struct viewporttype vp;
  char buffer[40];

  MainWindow( "Pre-defined Fill Styles" );

  getviewsettings( &vp );
  w = 2 * ((vp.right  +  1) / 13);
  h = 2 * ((vp.bottom - 10) / 10);

  x = w / 2;
  y = h / 2;            /* Leave 1/2 blk margin         */
  style = 0;

  for( j=0 ; j<3 ; ++j ){               /* Three rows of boxes          */
    for( i=0 ; i<4 ; ++i ){             /* Four column of boxes         */
      setfillstyle(style, WHITE);       /* Set the fill style and WHITE */
      bar( x, y, x+w, y+h );            /* Draw the actual box          */
      rectangle( x, y, x+w, y+h );      /* Outline the box              */
      itoa( style, buffer, 10 );        /* Convert style 3 to ASCII     */
      outtextxy( x+(w / 2), y+h+4, buffer );
      ++style;                          /* Go on to next style #        */
      x += (w / 2) * 3;                 /* Go to next column            */
    }                           /* End of coulmn loop           */
    x = w / 2;                          /* Put base back to 1st column  */
    y += (h / 2) * 3;                   /* Advance to next row          */
  }                                     /* End of Row loop              */

  settextjustify( LEFT_TEXT, TOP_TEXT );

  Pause();                              /* Wait for user's response     */

}

/*                                                                      */
/*      FILLPATTERNDEMO: Demonstrate how to use the user definable      */
/*      fill patterns.                                                  */
/*                                                                      */

void FillPatternDemo(void)
{
  int style;
  int h, w;
  int x, y, i, j;
  char buffer[40];
  struct viewporttype vp;
  static char patterns[][8] = {
    { 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55 },
    { 0x33, 0x33, 0xCC, 0xCC, 0x33, 0x33, 0xCC, 0xCC },
    { 0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x0F, 0x0F, 0x0F },
    { 0x00, 0x10, 0x28, 0x44, 0x28, 0x10, 0x00, 0x00 },
    { 0x00, 0x70, 0x20, 0x27, 0x24, 0x24, 0x07, 0x00 },
    { 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x3C, 0x3C, 0x3C, 0x3C, 0x00, 0x00 },
    { 0x00, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x00 },
    { 0x00, 0x00, 0x22, 0x08, 0x00, 0x22, 0x1C, 0x00 },
    { 0xFF, 0x7E, 0x3C, 0x18, 0x18, 0x3C, 0x7E, 0xFF },
    { 0x00, 0x10, 0x10, 0x7C, 0x10, 0x10, 0x00, 0x00 },
    { 0x00, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x00 }
  };

  MainWindow( "User Defined Fill Styles" );

  getviewsettings( &vp );
  w = 2 * ((vp.right  +  1) / 13);
  h = 2 * ((vp.bottom - 10) / 10);

  x = w / 2;
  y = h / 2;            /* Leave 1/2 blk margin         */
  style = 0;

  for( j=0 ; j<3 ; ++j ){               /* Three rows of boxes          */
    for( i=0 ; i<4 ; ++i ){             /* Four column of boxes         */
      setfillpattern( &patterns[style][0], WHITE);
      bar( x, y, x+w, y+h );            /* Draw the actual box          */
      rectangle( x, y, x+w, y+h );      /* Outline the box              */
      itoa( style, buffer, 10 );        /* Convert style 3 to ASCII     */
      outtextxy( x+(w / 2), y+h+4, buffer );
      ++style;                          /* Go on to next style #        */
      x += (w / 2) * 3;                 /* Go to next column            */
    }                           /* End of coulmn loop           */
    x = w / 2;                          /* Put base back to 1st column  */
    y += (h / 2) * 3;                   /* Advance to next row          */
  }                                     /* End of Row loop              */

  settextjustify( LEFT_TEXT, TOP_TEXT );

  Pause();                              /* Wait for user's response     */

}

/*                                                                      */
/*      POLYDEMO: Display a random pattern of polygons on the screen    */
/*      until the user says enough.                                     */
/*                                                                      */
void PaletteDemo(void)
{
  int i, j, x, y, color, cols;
  struct viewporttype vp;
  int height, width;

  if (MaxColors > 16)
    return;
  MainWindow( "Palette Demonstration" );
  StatusLine( StopMsg);

  getviewsettings( &vp );
  width  = (vp.right - vp.left) / 15;   /* get width of the box         */
  height = (vp.bottom - vp.top) / 10;   /* Get the height of the box    */

  x = y = 0;                            /* Start in upper corner        */
  color = 1;                            /* Begin at 1st color           */
  cols = 16;
  if (MaxColors < cols) cols = MaxColors;

  for( j=0 ; j<10 ; ++j ){              /* For 10 rows of boxes         */
    for( i=0 ; i<15 ; ++i ){            /* For 15 columns of boxes      */
      setfillstyle( SOLID_FILL, color++ );      /* Set the color of box */
      bar( x, y, x+width, y+height );           /* Draw the box         */
      x += width + 1;                           /* Advance to next col  */
      color = 1 + (color % (cols- 2));          /* Set new color        */
    }                                   /* End of COLUMN loop           */
    x = 0;                              /* Goto 1st column              */
    y += height + 1;                    /* Goto next row                */
  }                                     /* End of ROW loop              */

  while( !kbhit() ){                    /* Until user enters a key...   */
    setpalette( 1+Random(cols - 2), Random(64) );
  }

  setallpalette( &palette );

  Pause();                              /* Wait for user's response     */

}

/*                                                                      */
/*      POLYDEMO: Display a random pattern of polygons on the screen    */
/*      until the user says enough.                                     */
/*                                                                      */

#define MaxPts          6               /* Maximum # of pts in polygon  */

void PolyDemo(void)
{
  struct PTS poly[ MaxPts ];            /* Space to hold datapoints     */
  int color;                            /* Current drawing color        */
  int i;

  MainWindow( "DrawPoly / FillPoly Demonstration" );
  StatusLine( StopMsg);

  while( !kbhit() ){                    /* Repeat until a key is hit    */

    color = 1 + Random( MaxColors-1 );  /* Get a random color # (no blk)*/
    setfillstyle( Random(10), color );  /* Set a random line style      */
    setcolor( color );                  /* Set the desired color        */

    for( i=0 ; i<(MaxPts-1) ; i++ ){    /* Determine a random polygon   */
      poly[i].x = Random( MaxX );       /* Set the x coord of point     */
      poly[i].y = Random( MaxY );       /* Set the y coord of point     */
    }

    poly[i].x = poly[0].x;              /* last point = first point     */
    poly[i].y = poly[1].y;

    fillpoly( MaxPts, (int far *)poly );    /* Draw the actual polygon      */
  }                                     /* End of WHILE not KBHIT       */

  Pause();                              /* Wait for user's response     */

}


/*                                                                      */
/*      SAYGOODBYE: Give a closing screen to the user before leaving.   */
/*                                                                      */

void SayGoodbye(void)
{
  struct viewporttype viewinfo;         /* Structure to read viewport   */
  int h, w;

  MainWindow( "== Finale ==" );

  getviewsettings( &viewinfo );         /* Read viewport settings       */
  changetextstyle( TRIPLEX_FONT, HORIZ_DIR, 4 );
  settextjustify( CENTER_TEXT, CENTER_TEXT );

  h = viewinfo.bottom - viewinfo.top;
  w = viewinfo.right  - viewinfo.left;
  outtextxy( w/2, h/2, "That's all, folks!" );

  StatusLine( "Press any key to EXIT" );
  getch();

  cleardevice();                        /* Clear the graphics screen    */

}

/* ------------------------------------------------------------------- */
/* ----                     New demo routines                     ---- */
/* ------------------------------------------------------------------- */

#ifdef __GNUC__
#define rgb2col(r,g,b) setrgbcolor((r),(g),(b))
#else
unsigned long rgb2color_15(int r, int g, int b) {
  return   ((r&0xf8)<<7)
	 | ((g&0xf8)<<2)
	 | ((b&0xf8)>>3);
}
unsigned long rgb2color_16(int r, int g, int b) {
  return   ((r&0xf8)<<8)
	 | ((g&0xfc)<<3)
	 | ((b&0xf8)>>3);
}
unsigned long rgb2color_24(int r, int g, int b) {
  return   ((r&0xff)<<16)
	 | ((g&0xff)<< 8)
	 | ((b&0xff)    );
}
#endif

void BigColorDemo(void) {
  struct viewporttype vp;
  unsigned long tc;
  int color, height, width;
  int x, y;

#ifdef __GNUC__
  MainWindow("High Color/True Color demonstration");

  getviewsettings( &vp );               /* Get the current window size  */
  height = (vp.bottom-vp.top)/4 - 1;
  width  = vp.right-vp.left;
  if (width < 1) width = 1;

  y = 1;
  for (x=width-1; x > 0; --x) {
    color = (x-1)*256/(width-1);
    /* red */
    tc = rgb2col(color,0,0);
    setcolor((int)tc);
    line(x,y,x,y+height-1);
    /* green */
    tc = rgb2col(0,color,0);
    setcolor((int)tc);
    line(x,y+height,x,y+2*height-1);
    /* blue */
    tc = rgb2col(0,0,color);
    setcolor((int)tc);
    line(x,y+2*height,x,y+3*height-1);
    /* gray */
    tc = rgb2col(color,color,color);
    setcolor((int)tc);
    line(x,y+3*height,x,y+4*height-1);
  }
  setcolor(WHITE);

  Pause();                              /* Pause for user's response    */
#endif
}

int _max(int a, int b)
{
  return ( a>b ? a : b );
}

int _min(int a, int b)
{
  return ( a<b ? a : b );
}

void ShiftDac(char pal[][3])
{
  int r,g,b;
  int k, l;

  for (k=0;k<=1;k++) {
    r=pal[254][0];
    g=pal[254][1];
    b=pal[254][2];
    for (l=254;l>=1;l--) {
      pal[l][0]=pal[l-1][0];
      pal[l][1]=pal[l-1][1];
      pal[l][2]=pal[l-1][2];
    }
    pal[1][0]=r;
    pal[1][1]=g;
    pal[1][2]=b;
  }
  for (k=0;k<=255;k++)
    setrgbpalette(k,pal[k][0],pal[k][1],pal[k][2]);
}

void PlayRGBpalette(void)
/* This is partially copyrighted by COPYRIGHT(C) 1990 by H+BEDV  */
{
  typedef char _PAL[256][3];

  int x,c, m, maxx, maxy, radius, height, ycenter;
  double pc;
  _PAL cpal;
  struct viewporttype     viewinfo;

  if ( getmaxcolor() != 255) return;

  for (c=1;c<=254;c++) {
    m= (c*3)>>1;
    if ((m<64)) {
      cpal[c][0]=63;
      cpal[c][1]=m;
      cpal[c][2]=0;
    }
    if ((m>63) && (m<128)) {
      cpal[c][0]=127-m;
      cpal[c][1]=63;
      cpal[c][2]=0;
    }
    if ((m>127) && (m<192)) {
      cpal[c][0]=0;
      cpal[c][1]=63;
      cpal[c][2]=m-128;
    }
    if ((m>191) && (m<256)) {
      cpal[c][0]=0;
      cpal[c][1]=255-m;
      cpal[c][2]=63;
    }
    if ((m>255) && (m<320)) {
      cpal[c][0]=m-256;
      cpal[c][1]=0;
      cpal[c][2]=63;
    }
    if ((m>319)) {
      cpal[c][0]=63;
      cpal[c][1]=0;
      cpal[c][2]=383-m;
    }
  }
  cpal[0][0]=0;
  cpal[0][1]=0;
  cpal[0][2]=0;
  cpal[255][0]=63;
  cpal[255][1]=63;
  cpal[255][2]=63;
  ShiftDac( cpal);

  MainWindowColor( "Play RGB palette", 255);
  getviewsettings( &viewinfo );
  maxx = abs(viewinfo.right-viewinfo.left)-1;
  maxy = abs(viewinfo.top-viewinfo.bottom)-1;
  setcolor(255);

  height = maxy/8;
  c=1;
  for (x=5; x <= maxx+1-5; ++x) {
    setcolor(c);
    if (++c > 254) c = 1;
    line(x,maxy-5,x,maxy-5-height);
  }

  pc=1.0;
  ycenter = (maxy-5-height) / 2;
  radius = _min((int)((maxy-5-height)/AspectRatio), maxx)*9/20;
  for (x=0;x<=356;x++) {
    setcolor((int)pc);
    setfillstyle(SOLID_FILL,(int)pc);
    pieslice(maxx/2,ycenter,x,x+4,radius);
    pc=pc+254.0/360.0;
  }

  StatusLineColor( StopMsg, 255);

  do {
    ShiftDac(cpal);
  } while (!(kbhit()));
  if ( getch() == ESC) {
    closegraph();
    exit(1);
  }
  for (c=1; c < 255; ++c) {
    cpal[c][0] = _dac_g256[c][0];
    cpal[c][1] = _dac_g256[c][1];
    cpal[c][2] = _dac_g256[c][2];
  }

  StatusLineColor( PauseMsg, 255);
  do {
    ShiftDac(cpal);
  } while (!(kbhit()));

  setbkcolor(BLACK);
  clearviewport();
  setrgbdefaults();
  Pause();                              /* Pause for user to read screen*/
}

/* The Sierpinski demo was mainly taken from
   N. Wirth: Algorithmen und Datenstrukturen  */
#define SIRP_N     4
#define SIRP_H0    320

static int SIRP_x, SIRP_y, h;

static void SIRP_a(int i);
static void SIRP_b(int i);
static void SIRP_c(int i);
static void SIRP_d(int i);

static void SIRP_a(int i)
{
  if (i>0) {
    SIRP_a(i-1); SIRP_x += h;   SIRP_y -= h; lineto( SIRP_x, SIRP_y);
    SIRP_b(i-1); SIRP_x += 2*h;              lineto( SIRP_x, SIRP_y);
    SIRP_d(i-1); SIRP_x += h;   SIRP_y += h; lineto( SIRP_x, SIRP_y);
    SIRP_a(i-1);
  }
}

static void SIRP_b(int i)
{
  if (i>0) {
    SIRP_b(i-1); SIRP_x -= h; SIRP_y -= h;   lineto( SIRP_x, SIRP_y);
    SIRP_c(i-1);              SIRP_y -= 2*h; lineto( SIRP_x, SIRP_y);
    SIRP_a(i-1); SIRP_x += h; SIRP_y -= h;   lineto( SIRP_x, SIRP_y);
    SIRP_b(i-1);
  }
}

static void SIRP_c(int i)
{
  if (i>0) {
    SIRP_c(i-1); SIRP_x -= h;   SIRP_y += h; lineto( SIRP_x, SIRP_y);
    SIRP_d(i-1); SIRP_x -= 2*h;              lineto( SIRP_x, SIRP_y);
    SIRP_b(i-1); SIRP_x -= h;   SIRP_y -= h; lineto( SIRP_x, SIRP_y);
    SIRP_c(i-1);
  }
}

static void SIRP_d(int i)
{
  if (i>0) {
    SIRP_d(i-1); SIRP_x += h; SIRP_y += h;   lineto( SIRP_x, SIRP_y);
    SIRP_a(i-1);              SIRP_y += 2*h; lineto( SIRP_x, SIRP_y);
    SIRP_c(i-1); SIRP_x -= h; SIRP_y += h;   lineto( SIRP_x, SIRP_y);
    SIRP_d(i-1);
  }
}

void sierpinski(void)
{
  int i, h0, x0, y0, bx, by;
  int color, border;
  struct viewporttype vp;
  struct fillsettingstype fs;

  if (MaxColors < 16)
    return;

  MainWindow( "Floodfill demo");
  StatusLine(PauseMsg);
  getviewsettings( &vp);
  getfillsettings( &fs);

  setviewport( (bx=_max((getmaxx() - SIRP_H0) / 2, vp.left)),
	       (by=_max((getmaxy() - SIRP_H0) / 2, vp.top)),
	       _min((getmaxx() + SIRP_H0) / 2 + 5, vp.right),
	       _min((getmaxy() + SIRP_H0) / 2 + 5, vp.bottom),
	       TRUE );

  border = _ega_color(YELLOW);
  setcolor( border);
  h0 = SIRP_H0;
  h = h0 / 4;
  x0 = 2*h;
  y0 = 3*h;
  for (i=1; i <= SIRP_N; ++i) {
    x0 -= h;
    h /= 2;
    y0 += h;
    SIRP_x = x0; SIRP_y = y0;
    moveto( SIRP_x, SIRP_y);
    SIRP_a(i); SIRP_x += h; SIRP_y -= h; lineto(SIRP_x,SIRP_y);
    SIRP_b(i); SIRP_x -= h; SIRP_y -= h; lineto(SIRP_x,SIRP_y);
    SIRP_c(i); SIRP_x -= h; SIRP_y += h; lineto(SIRP_x,SIRP_y);
    SIRP_d(i); SIRP_x += h; SIRP_y += h; lineto(SIRP_x,SIRP_y);
  }
  setviewport( vp.left, vp.top, vp.right, vp.bottom, vp.clip);
  bx += h0/2 - vp.left;
  by += h0/2 - vp.top;
  color = BLUE-1;
  do {
    if (++color >= YELLOW) color = BLUE;
    setfillstyle(Random(USER_FILL-1)+1, _ega_color(color));
    floodfill( bx, by, border);
    if (kbhit()) break;
    floodfill(  1,  1, border);
  } while ( !kbhit());
  setfillstyle( fs.pattern, fs.color);

  Pause();                              /* Pause for user to read screen*/
}

#ifdef __GNUC__
/* Borland C died frequently on this demo */
void snake(void)
{
  int i, x0, y0, x1, y1, x, y;
  int color, border;
  struct viewporttype vp;
  struct fillsettingstype fs;
  int dx, dy;

  if (MaxColors < 16)
    return;

  MainWindow( "Floodfill demo 2");
  StatusLine(PauseMsg);
  getviewsettings( &vp);
  getfillsettings( &fs);

  x0 = 0; y0 = 0;
  x1 = getmaxx(); y1 = getmaxy();
  if (x1-x0 < y1-y0) {
    dx = (x1-x0) / 24;
    dy = (int)(dx*AspectRatio + 0.5);
  } else {
    dy = (y1-y0) / 24;
    dx = (int)(dy/AspectRatio + 0.5);
  }

  border = _ega_color(YELLOW);
  setcolor( border);

  moveto(x=(x1-x0)/2, y=(y1-y0)/2);
  i = 0;
  while (x0<x && x<x1 && y0<y && y<y1) {
    ++i;
    if ((i&1) != 0) {
      y -= i*dy; lineto(x, y);
      x -= i*dx; lineto(x, y);
    } else {
      y += i*dy; lineto(x, y);
      x += i*dx; lineto(x, y);
    }
  }

  x = (x1-x0 - dx)/2;
  y = (y1-y0 + dy)/2;
  color = BLUE-1;
  do {
    if (++color >= YELLOW) color = BLUE;
    setfillstyle(Random(USER_FILL-1)+1, _ega_color(color));
    floodfill( x, y, border);
    delay(500);
  } while ( !kbhit());
  setfillstyle( fs.pattern, fs.color);

  Pause();                              /* Pause for user to read screen*/
}
#endif

void RandomSolidBars(void)
{
  int color;

  MainWindow( "Random Solid/Line Bars" );
  StatusLine( PauseMsg );               /* Put msg at bottom of screen   */
  while( !kbhit() ){                    /* Until user enters a key...   */
    color = Random( MaxColors-1 )+1;
    setcolor( color );
    /* SOLID_FILL && LINE_FILL are much faster */
    setfillstyle( SOLID_FILL+Random(2), color );
    bar3d( Random( getmaxx() ), Random( getmaxy() ),
	   Random( getmaxx() ), Random( getmaxy() ), 0, OFF);
  }

  Pause();                              /* Pause for user's response    */

}

#define Memory          100
#define Windows         4

typedef int ColorList[Windows];


typedef struct _REC_Line {
  int LX1, LY1, LX2, LY2;
  ColorList LColor;
} _REC_Line;

/* Local variables for LinePlay: */
struct LOC_LinePlay {
  int ViewXmax, ViewYmax;
  _REC_Line Line[Memory];
  int X1, X2, Y1, Y2, CurrentLine, ColorCount, IncrementCount, DeltaX1,
      DeltaY1, DeltaX2, DeltaY2;
  ColorList Colors;
  int MaxDelta;
} ;

void AdjustX(int *X, int *DeltaX, struct LOC_LinePlay *LINK)
{
  int TestX;

  TestX = *X + *DeltaX;
  if (TestX < 1 || TestX > LINK->ViewXmax) {
    TestX = *X;
    *DeltaX = -*DeltaX;
  }
  *X = TestX;
}

void AdjustY(int *Y, int *DeltaY, struct LOC_LinePlay *LINK)
{
  int TestY;

  TestY = *Y + *DeltaY;
  if (TestY < 1 || TestY > LINK->ViewYmax) {
    TestY = *Y;
    *DeltaY = -*DeltaY;
  }
  *Y = TestY;
}

int RandColor(void)
{
  return Random(MaxColors-1) + 1;
}

void SelectNewColors(struct LOC_LinePlay *LINK)
{
  LINK->Colors[0] = RandColor();
  LINK->Colors[1] = RandColor();
  LINK->Colors[2] = RandColor();
  LINK->Colors[3] = RandColor();
  LINK->ColorCount = (Random(5) + 1) * 3;
}

void SelectNewDeltaValues(struct LOC_LinePlay *LINK)
{
  LINK->DeltaX1 = Random(LINK->MaxDelta) - LINK->MaxDelta / 2;
  LINK->DeltaX2 = Random(LINK->MaxDelta) - LINK->MaxDelta / 2;
  LINK->DeltaY1 = Random(LINK->MaxDelta) - LINK->MaxDelta / 2;
  LINK->DeltaY2 = Random(LINK->MaxDelta) - LINK->MaxDelta / 2;
  LINK->IncrementCount = (Random(4) + 1) * 2;
}

void SaveCurrentLine(int *CurrentColors, struct LOC_LinePlay *LINK)
{
  _REC_Line *WITH;

  WITH = &LINK->Line[LINK->CurrentLine - 1];
  WITH->LX1 = LINK->X1;
  WITH->LY1 = LINK->Y1;
  WITH->LX2 = LINK->X2;
  WITH->LY2 = LINK->Y2;
  memcpy(WITH->LColor, CurrentColors, sizeof(ColorList));
}

void Draw(unsigned short x1, unsigned short y1, unsigned short x2,
		unsigned short y2, unsigned short color)
{
  setcolor(color);
  line(x1, y1, x2, y2);
}

void Updateline(struct LOC_LinePlay *LINK)
{
  LINK->CurrentLine++;
  if (LINK->CurrentLine > Memory)
    LINK->CurrentLine = 1;
  LINK->ColorCount--;
  LINK->IncrementCount--;
}

void DrawCurrentLine(struct LOC_LinePlay *LINK)
{
  Draw(LINK->X1, LINK->Y1, LINK->X2, LINK->Y2, LINK->Colors[0]);
  Draw(LINK->ViewXmax - LINK->X1, LINK->Y1, LINK->ViewXmax - LINK->X2,
       LINK->Y2, LINK->Colors[1]);
  Draw(LINK->X1, LINK->ViewYmax - LINK->Y1, LINK->X2,
       LINK->ViewYmax - LINK->Y2, LINK->Colors[2]);
  Draw(LINK->ViewXmax - LINK->X1, LINK->ViewYmax - LINK->Y1,
       LINK->ViewXmax - LINK->X2, LINK->ViewYmax - LINK->Y2, LINK->Colors[3]);
  SaveCurrentLine(LINK->Colors, LINK);
}

void EraseCurrentLine(struct LOC_LinePlay *LINK)
{
  _REC_Line *WITH;

  WITH = &LINK->Line[LINK->CurrentLine - 1];
  Draw(WITH->LX1, WITH->LY1, WITH->LX2, WITH->LY2, 0);
  Draw(LINK->ViewXmax - WITH->LX1, WITH->LY1, LINK->ViewXmax - WITH->LX2,
       WITH->LY2, 0);
  Draw(WITH->LX1, LINK->ViewYmax - WITH->LY1, WITH->LX2,
       LINK->ViewYmax - WITH->LY2, 0);
  Draw(LINK->ViewXmax - WITH->LX1, LINK->ViewYmax - WITH->LY1,
       LINK->ViewXmax - WITH->LX2, LINK->ViewYmax - WITH->LY2, 0);
}

void DoArt(struct LOC_LinePlay *LINK)
{
  SelectNewColors(LINK);
  do {
    EraseCurrentLine(LINK);
    if (LINK->ColorCount == 0)
      SelectNewColors(LINK);
    if (LINK->IncrementCount == 0)
      SelectNewDeltaValues(LINK);
    AdjustX(&LINK->X1, &LINK->DeltaX1, LINK);
    AdjustX(&LINK->X2, &LINK->DeltaX2, LINK);
    AdjustY(&LINK->Y1, &LINK->DeltaY1, LINK);
    AdjustY(&LINK->Y2, &LINK->DeltaY2, LINK);
    if (Random(5) == 3) {
      LINK->X1 = (LINK->X1 + LINK->X2) / 2;   /* shorten the lines */
      LINK->Y2 = (LINK->Y1 + LINK->Y2) / 2;
    }
    DrawCurrentLine(LINK);
    Updateline(LINK);
  } while (!kbhit());
}


void LinePlay(void)
{
  struct LOC_LinePlay V;
  struct viewporttype ViewInfo;
  int StartX, StartY, I;
  _REC_Line *WITH;

  MainWindow("Line demonstration");
  StatusLine("Esc aborts or press a key ...");
  getviewsettings(&ViewInfo);
  V.CurrentLine = 1;
  V.ColorCount = 0;
  V.IncrementCount = 0;
  V.MaxDelta = 16;
  V.ViewXmax = ViewInfo.right - 1;
  V.ViewYmax = ViewInfo.bottom - 3;
  StartX = ViewInfo.right / 2;
  StartY = ViewInfo.bottom / 2;
  for (I = 0; I < Memory; I++) {
    WITH = &V.Line[I];
    WITH->LX1 = StartX;
    WITH->LX2 = StartX;
    WITH->LY1 = StartY;
    WITH->LY2 = StartY;
  }
  V.X1 = StartX;
  V.X2 = StartX;
  V.Y1 = StartY;
  V.Y2 = StartY;
  DoArt(&V);
  Pause();
}

#undef Memory
#undef Windows

/* Local variables for ColorPlay: */
struct LOC_ColorPlay {
  unsigned short Color, Width, Height;
  struct viewporttype ViewInfo;
} ;

char *Int2Str(char *Result, long L)
{
  /* Converts an integer to a string for use with OutText, OutTextXY */
  char S[256];

  sprintf(S, "%ld", L);
  return strcpy(Result, S);
}  /* Int2Str */


void DrawBox__(unsigned short X, unsigned short Y,
		     struct LOC_ColorPlay *LINK)
{
  int bottom;
  char STR1[256];

  setfillstyle(SOLID_FILL, LINK->Color);
  setcolor(LINK->Color);
  if (LINK->Height / 2 >= textheight("M") + 4)
    bottom = Y + LINK->Height;
  else
    bottom = Y + LINK->Height / 2 * 3 - textheight("M") - 5;
  bar(X, Y, X + LINK->Width, bottom);
  rectangle(X, Y, X + LINK->Width, bottom);
  LINK->Color = getcolor();
  if (LINK->Color == 0) {
    setcolor(MaxColors);
    rectangle(X, Y, X + LINK->Width, bottom);
  }
  setcolor(WHITE);
  outtextxy(X + LINK->Width / 2, bottom + 3, Int2Str(STR1, LINK->Color));
  LINK->Color = (LINK->Color + 1) % (MaxColors + 1);
}  /* DrawBox */


void ColorPlay(void)
{
  /* Display all of the colors available for the current driver and mode */
  struct LOC_ColorPlay V;
  unsigned short X, Y, I, J;

  if (MaxColors != 256) {
    ColorDemo();
    if (MaxColors < 256)
      return;
  }
  if (MaxColors > 256) {
    BigColorDemo();
    return;
  }
  MainWindow("Color demonstration");
  V.Color = 1;
  getviewsettings(&V.ViewInfo);
  V.Width = (V.ViewInfo.right + 1) / 53 * 2;
  V.Height = (V.ViewInfo.bottom - 10) / 47 * 2;
  if (V.Height < textheight("M") + 4)
    V.Height = textheight("M") + 4;
  if (V.Width < textwidth("M") * 2)
    V.Width = textwidth("M") * 2;
  X = V.Width / 2;
  Y = V.Height / 2;
  for (J = 1; J <= 15; J++) {
    for (I = 1; I <= 17; I++) {
      if (!kbhit())
	DrawBox__(X, Y, &V);
      X += V.Width / 2 * 3;
    }
    X = V.Width / 2;
    Y += V.Height / 2 * 3;
  }
  Pause();
}  /* ColorPlay */




/*                                                                      */
/*      Begin main function                                             */
/*                                                                      */

int main(void)
{

#if 0 && defined(__GNUC__)
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
  Initialize();                 /* Set system into Graphics mode        */
  ReportStatus();               /* Report results of the initialization */
  ColorPlay();                  /* Begin actual demonstration           */
  if( GraphDriver==EGA || GraphDriver==EGA64 || GraphDriver==VGA )
    PaletteDemo();
  PutPixelDemo();
  PutImageDemo();
  Bar3DDemo();
  BarDemo();
  RandomBars();
  RandomSolidBars();
  sierpinski();
#ifdef __GNUC__
  snake();
#endif
  ArcDemo();
  CircleDemo();
  PieDemo();
  PlayRGBpalette();
  LinePlay();
  LineRelDemo();
  LineToDemo();
  LineStyleDemo();
  UserLineStyleDemo();
  TextDump();
  TextDemo();
  CRTModeDemo();
  FillStyleDemo();
  FillPatternDemo();
  PolyDemo();
  SayGoodbye();                 /* Give user the closing screen         */
  closegraph();                 /* Return the system to text mode       */
  return(0);
}

