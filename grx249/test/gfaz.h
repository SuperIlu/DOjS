/**
 ** gfaz.h ---- gfaz headers
 **
 ** Copyright (C) 2000,2001 Mariano Alvarez Fernandez
 ** [e-mail: malfer@teleline.es]
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

extern GrColor *egacolors;

#define BLACK        egacolors[0]
#define BLUE         egacolors[1]
#define GREEN        egacolors[2]
#define CYAN         egacolors[3]
#define RED          egacolors[4]
#define MAGENTA      egacolors[5]
#define BROWN        egacolors[6]
#define LIGHTGRAY    egacolors[7]
#define DARKGRAY     egacolors[8]
#define LIGHTBLUE    egacolors[9]
#define LIGHTGREEN   egacolors[10]
#define LIGHTCYAN    egacolors[11]
#define LIGHTRED     egacolors[12]
#define LIGHTMAGENTA egacolors[13]
#define YELLOW       egacolors[14]
#define WHITE        egacolors[15]

#define IND_BLACK        0
#define IND_BLUE         1
#define IND_GREEN        2
#define IND_CYAN         3
#define IND_RED          4
#define IND_MAGENTA      5
#define IND_BROWN        6
#define IND_LIGHTGRAY    7
#define IND_DARKGRAY     8
#define IND_LIGHTBLUE    9
#define IND_LIGHTGREEN   10
#define IND_LIGHTCYAN    11
#define IND_LIGHTRED     12
#define IND_LIGHTMAGENTA 13
#define IND_YELLOW       14
#define IND_WHITE        15

#define EV_NULL    0
#define EV_KEY     1
#define EV_MOUSE   2
#define EV_COMMAND 3
#define EV_SELECT  4
#define EV_END     5

#define MOUSE_LB_PRESSED 1
#define MOUSE_RB_PRESSED 2
#define MOUSE_LB_RELEASED 3
#define MOUSE_RB_RELEASED 4

typedef struct{
  int type;
  long p1;
  long p2;
  long p3;
  } Event;

int gfaz_ini( int width, int height, int bpp );
int gfaz_fin( void );

void event_read( Event *ev );
void event_wait( Event *ev );
void event_queue( Event *ev );
void par_event_queue( int type, long p1, long p2, long p3 );
void set_hook_input_event( void (*fn)( Event * ) );

void show_mouse( void );
void hide_mouse( void );

#define BSTATUS_PRESSED  1
#define BSTATUS_SELECTED 2

typedef struct{
  int x, y;                   // left upper coordinates
  int wide, high;             // what do you mean
  int tbcolor, tfcolor;       // text background, foreground, ind colors
  char *text;                 // the text
  int status;                 // see BSTATUS defs
  int bid;                    // button id
  } Button;

void paint_button( int x, int y, Button *b );

typedef struct{
  int x, y;                   // left upper coordinates
  Button *b;                  // button array
  int nb;                     // button number
  int pb;                     // point actual button
  int abp;                    // actual button pressed
  } Button_Group;

void paint_button_group( Button_Group *bg );
int  pev_button_group( Event *ev, Button_Group *bg );

typedef struct{
  int x, y;                   // left upper coordinates
  int wide, high;             // what do you mean
  int aid;                    // area id
  int divx, divy;             // x, y divisors
  int inip1, inip2;           // par1, par2 initial values
  int incrp1, incrp2;         // par1, par2 increments
  int invert;                 // x,y -> par1,par2 or inverted if 1
  } Area;

typedef struct{
  int x, y;                   // left upper coordinates
  Area *a;                    // area array
  } Area_Group;

int  pev_area_group( Event *ev, Area_Group *ag );

typedef struct{
  int x, y;                   // left upper coordinates
  int wide, high;             // what do you mean
  int lcolor, bcolor, color;  // line, border, normal ind color
  int border;                 //
  } Board;

void paint_board( Board *b );
  
