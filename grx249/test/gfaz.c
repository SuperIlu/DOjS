/**
 ** gfaz.c ---- Mini GUI for GRX
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

#include <stdlib.h>
#include <string.h>
#include <grx20.h>
#include <grxkeys.h>
#include "gfaz.h"

GrColor *egacolors;

static int mouse_found = 0;
static int mouse_count = 0;

#define MAX_EVQUEUE 10

static Event evqueue[MAX_EVQUEUE];
static int num_evqueue = 0;

static void (*hook_input_event)( Event * ) = NULL;

/* Internal routines */

static int read_input( void );
static void input_event_queue( Event *ev );
static int coord_into( int x, int y, int xo, int yo, int xl, int yl );

/**************************************************************************/

int gfaz_ini( int width, int height, int bpp )
{
/*  __djgpp_set_ctrl_c( 0 );*/
/*  _go32_want_ctrl_break( 1 );*/
/*  GrSetMode( GR_default_graphics );*/

  GrSetMode( GR_width_height_bpp_graphics,width,height,bpp );
  
  egacolors = GrAllocEgaColors();

  if( GrMouseDetect() ){
    mouse_found = 1;
    GrMouseInit();
    GrMouseSetColors( GrWhite(),GrBlack() );
    show_mouse();
    }
  GrMouseEventEnable( 1,mouse_found );

  return 0;
}

/**************************************************************************/

int gfaz_fin( void )
{
  if( mouse_found ){
    hide_mouse();
    GrMouseUnInit();
    }
    
  GrSetMode( GR_default_text );
  
  return 0;
}

/**************************************************************************/

void event_read( Event *ev )
{
  while( 1 ){
    if( num_evqueue > 0 ){
      num_evqueue--;
      *ev = evqueue[num_evqueue];
      return;
      }
    if( read_input() ){
      continue;
      }
    ev->type = EV_NULL;
    ev->p1 = 0;
    ev->p2 = 0;
    ev->p3 = 0;
    return;
    }
}

/**************************************************************************/

void event_wait( Event *ev )
{
  while( 1 ){
    event_read( ev );
    if( ev->type != EV_NULL )
      return;
    }
}

/**************************************************************************/

void event_queue( Event *ev )
{
  if( num_evqueue < MAX_EVQUEUE )
    evqueue[num_evqueue++] = *ev;
}

/**************************************************************************/

void par_event_queue( int type, long p1, long p2, long p3 )
{
  Event ev;

  ev.type = type;
  ev.p1 = p1;
  ev.p2 = p2;
  ev.p3 = p3;
  event_queue( &ev );
}

/**************************************************************************/

void set_hook_input_event( void (*fn)( Event * ) )
{
  hook_input_event = fn;
}

/**************************************************************************/

static void input_event_queue( Event *ev )
{
  if( hook_input_event != NULL )
    hook_input_event( ev );
  else
    event_queue( ev );
}

/************************************************************************/

static int read_input( void )
{
  GrMouseEvent evt;
  Event evaux;

  GrMouseGetEventT( GR_M_EVENT,&evt,10L );
  if( evt.dtime == -1 ) return 0;
  
  if( evt.flags & GR_M_KEYPRESS ){
    evaux.type = EV_KEY;
    evaux.p1 = evt.key;
    evaux.p2 = 0;
    evaux.p3 = 0;
    input_event_queue( &evaux );
    }
  evaux.type = EV_MOUSE;
  evaux.p2 = evt.x;
  evaux.p3 = evt.y;
  if( evt.flags & GR_M_LEFT_DOWN ){
    evaux.p1 = MOUSE_LB_PRESSED;
    input_event_queue( &evaux );
    }
  if( evt.flags & GR_M_RIGHT_DOWN ){
    evaux.p1 = MOUSE_RB_PRESSED;
    input_event_queue( &evaux );
    }
  if( evt.flags & GR_M_LEFT_UP ){
    evaux.p1 = MOUSE_LB_RELEASED;
    input_event_queue( &evaux );
    }
  if( evt.flags & GR_M_RIGHT_UP ){
    evaux.p1 = MOUSE_RB_RELEASED;
    input_event_queue( &evaux );
    }
  return 1;
}

/**************************************************************************/

void show_mouse( void )
{
  if( (mouse_count == 0) && mouse_found )
    GrMouseDisplayCursor();

  mouse_count++;
}

/************************************************************************/

void hide_mouse( void )
{
  mouse_count--;
  
  if( (mouse_count == 0) && mouse_found )
    GrMouseEraseCursor();
}

/**************************************************************************/

void dboton( int x, int y, int an, int al,
             GrColor c, GrColor ct, char * s, int t )

//   x, y posici¢n de la esquina izquierda
//   an, al ancho y alto
//   c, ct color del fondo y del texto
//   t, tipo bit 0 0=normal, 1=apretado
//           bit 1 0=no selec, 1=seleccionado

{
  int pol[7][2], prof, pulsd, selec, despl;
  GrTextOption grt;
  GrLineOption glo;
  int mouseblock;

  prof = (t & 0x1) ? 2 : 4;
  pulsd = (t & 0x1) ? 1 : 0;
  selec = (t & 0x2) ? 1 : 0;
  despl = (t & 0x1) ? 1 : 0;

  mouseblock = GrMouseBlock( NULL,x,y,x+an-1,y+al-1 );
  GrBox( x,y,x+an-1,y+al-1,BLACK );
  x = x + 1; y = y + 1;
  an = an - 2; al = al - 2;

  pol[0][0] = x;                 pol[0][1] = y;
  pol[1][0] = x + an - 1;        pol[1][1] = y;
  pol[2][0] = x + an - 2 - prof; pol[2][1] = y + 1 + prof;
  pol[3][0] = x + 1 + prof;      pol[3][1] = y + 1 + prof;
  pol[4][0] = x + 1 + prof;      pol[4][1] = y + al - 2 - prof;
  pol[5][0] = x;                 pol[5][1] = y + al - 1;
  pol[6][0] = pol[0][0];         pol[6][1] = pol[0][1];
  GrFilledPolygon( 7,pol,pulsd ? DARKGRAY : LIGHTGRAY );
  GrPolygon( 7,pol,BLACK );
  GrLine( pol[0][0],pol[0][1],pol[3][0],pol[3][1],BLACK );
  pol[0][0] = x + an - 1;        pol[0][1] = y + al - 1;
  pol[3][0] = x + an - 2 - prof; pol[3][1] = y + al - 2 - prof;
  pol[6][0] = pol[0][0];         pol[6][1] = pol[0][1];
  GrFilledPolygon( 7,pol,pulsd ? LIGHTGRAY : DARKGRAY );
  GrPolygon( 7,pol,BLACK );
  GrLine( pol[0][0],pol[0][1],pol[3][0],pol[3][1],BLACK );
  GrFilledBox( x+2+prof,y+2+prof,x+an-3-prof,y+al-3-prof,c );

  grt.txo_font = &GrFont_PC8x14;
  grt.txo_fgcolor.v = ct;
  grt.txo_bgcolor.v = GrNOCOLOR;
  grt.txo_direct = GR_TEXT_RIGHT;
  grt.txo_xalign = GR_ALIGN_CENTER;
  grt.txo_yalign = GR_ALIGN_CENTER;
  grt.txo_chrtype = GR_BYTE_TEXT;
  if( despl )
    GrDrawString( s,strlen( s ),x+an/2+1,y+al/2+1,&grt );
  else
    GrDrawString( s,strlen( s ),x+an/2,y+al/2,&grt );

  if( selec ){
    glo.lno_color = ct;
    glo.lno_width = 1;
    glo.lno_pattlen = 2;
    glo.lno_dashpat = "\2\1";
    GrCustomBox( x+8,y+al/2-6,x+an-8,y+al/2+5,&glo );
    }

  GrMouseUnBlock( mouseblock );
}

/**************************************************************************/

void paint_button( int x, int y, Button *b )
{
  dboton( x+b->x,y+b->y,b->wide,b->high,
          egacolors[b->tbcolor],egacolors[b->tfcolor],
          b->text,b->status );
}

/**************************************************************************/

void paint_button_group( Button_Group *bg )
{
  int i;

  for( i=0; i<bg->nb; i++ )
    paint_button( bg->x,bg->y,&(bg->b[i]) );
}

/**************************************************************************/

int pev_button_group( Event *ev, Button_Group *bg )
{
  int i;

  if( ev->type == EV_MOUSE ){
    if( ev->p1 == MOUSE_LB_PRESSED ){
      for( i=0; i<bg->nb; i++ )
        if( coord_into( ev->p2,ev->p3,
                        bg->x+bg->b[i].x,bg->y+bg->b[i].y,
                        bg->b[i].wide,bg->b[i].high ) ){
          bg->b[bg->pb].status &= ~BSTATUS_SELECTED;
          paint_button( bg->x,bg->y,&(bg->b[bg->pb]) );
          bg->b[i].status |= BSTATUS_PRESSED | BSTATUS_SELECTED;
          paint_button( bg->x,bg->y,&(bg->b[i]) );
          bg->pb = i;
          bg->abp = 1;
          par_event_queue( EV_SELECT,bg->b[i].bid,0,0 );
          return 1;
          }
      }
    if( ev->p1 == MOUSE_LB_RELEASED ){
      if( bg->abp ){
        i = bg->pb;
        bg->b[i].status &= ~BSTATUS_PRESSED;
        paint_button( bg->x,bg->y,&(bg->b[i]) );
        bg->abp = 0;
        if( coord_into( ev->p2,ev->p3,
                        bg->x+bg->b[i].x,bg->y+bg->b[i].y,
                        bg->b[i].wide,bg->b[i].high ) ){
          par_event_queue( EV_COMMAND,bg->b[i].bid,0,0 );
          }
        return 1;
        }
      }
    }
  else if( ev->type == EV_KEY ){
    if( ev->p1 == GrKey_Right ||
        ev->p1 == GrKey_Down ||
        ev->p1 == GrKey_Tab ){
      if( bg->pb < bg->nb-1 ){
        bg->b[bg->pb].status &= ~BSTATUS_SELECTED;
        paint_button( bg->x,bg->y,&(bg->b[bg->pb]) );
        bg->pb++;
        bg->b[bg->pb].status |= BSTATUS_SELECTED;
        paint_button( bg->x,bg->y,&(bg->b[bg->pb]) );
        par_event_queue( EV_SELECT,bg->b[bg->pb].bid,0,0 );
        }
      return 1;
      }
    else if( ev->p1 == GrKey_Left ||
             ev->p1 == GrKey_Up ||
             ev->p1 == GrKey_BackTab ){
      if( bg->pb > 0 ){
        bg->b[bg->pb].status &= ~BSTATUS_SELECTED;
        paint_button( bg->x,bg->y,&(bg->b[bg->pb]) );
        bg->pb--;
        bg->b[bg->pb].status |= BSTATUS_SELECTED;
        paint_button( bg->x,bg->y,&(bg->b[bg->pb]) );
        par_event_queue( EV_SELECT,bg->b[bg->pb].bid,0,0 );
        }
      return 1;
      }
    else if( ev->p1 == GrKey_Return ){
      par_event_queue( EV_COMMAND,bg->b[bg->pb].bid,0,0 );
      return 1;
      }
    }

  return 0;
}

/**************************************************************************/

static int coord_into( int x, int y, int xo, int yo, int xl, int yl )
{
  if( x < xo ) return 0;
  if( x >= xo+xl ) return 0;
  if( y < yo ) return 0;
  if( y >= yo+yl ) return 0;
  return 1;
}

/**************************************************************************/

void paint_board( Board *b )
{
  int x1, y1, x2, y2;

  x1 = b->x;
  y1 = b->y;
  x2 = b->x + b->wide - 1;
  y2 = b->y + b->high - 1;
  
  GrBox( x1,y1,x2,y2,egacolors[b->lcolor] );
  GrBox( x1+1,y1+1,x2-1,y2-1,egacolors[b->bcolor] );
  GrBox( x1+2,y1+2,x2-2,y2-2,egacolors[b->bcolor] );
  GrBox( x1+3,y1+3,x2-3,y2-3,egacolors[b->lcolor] );
  GrFilledBox( x1+4,y1+4,x2-4,y2-4,egacolors[b->color] );
}

