/**
 ** lnxinput.c ---- mouse and keyboard interface for Linux
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu]
 **
 ** This file is part of the GRX graphics library.
 **
 ** The GRX graphics library is free software; you can redistribute it
 ** and/or modify it under some conditions; see the "copying.grx" file
 ** for details.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** Contributions by: (See "doc/credits.doc" for details)
 ** Mauro Condarelli (mc5686@mclink.it)
 ** Hartmut Schirmer (hsc@techfak.uni-kiel.de)
 ** Andris Pavenis (pavenis@acad.latnet.lv)
 **
 **/

#include <stdio.h>
#include <termio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
#include <vga.h>
#include <vgamouse.h>

#include "libgrx.h"
#include "input.h"
#include "arith.h"
#include "memcopy.h"
#include "memfill.h"

#include "grxkeys.h"

/*
 * keyboard stuff
 */
static struct termio kbd_setup;
static struct termio kbd_reset;
static int    kbd_initted = FALSE;
static int    kbd_enabled = TRUE;
static int    kbd_isatty;
static int    kbd_lastchr;
static int    kbd_filedsc;
static enum { normal, test, wait } kbd_mode;

static void kbd_restore(void)
{
	if(kbd_initted && kbd_isatty && (kbd_mode != normal)) {
	    ioctl(kbd_filedsc,TCSETA,&kbd_reset);
	    kbd_mode = normal;
	}
}

static void kbd_init(void)
{
	if(!kbd_initted) {
	    kbd_initted = TRUE;
	    kbd_lastchr = EOF;
	    kbd_filedsc = fileno(stdin);
	    kbd_isatty  = isatty(kbd_filedsc);
	    if(kbd_isatty) {
		ioctl(kbd_filedsc,TCGETA,&kbd_setup);
		ioctl(kbd_filedsc,TCGETA,&kbd_reset);
		kbd_setup.c_lflag &= ~(ICANON | ECHO );
		kbd_setup.c_iflag &= ~(INLCR  | ICRNL);
		atexit(kbd_restore);
		kbd_mode = normal;
	    }
	}
}

static int n = 0;
static int getByte(int remove)
  {
      static unsigned char keybuf[10];
      int  ch, f;

      if (n < 0) n = 0;
      if (n<=0) {
	   f = read(kbd_filedsc, keybuf+n, 10-n);
	   if (f>0) n += f;
      }
      if (n<=0) return EOF;
      ch = keybuf[0];
      if (remove) {
	   --n;
	   memmove(&keybuf[0],&keybuf[1], n*sizeof(unsigned char));
      }
      return ch;
  }


static const GrKeyType fnk[] = {
    GrKey_F6,  GrKey_F7, GrKey_F8, GrKey_F9, GrKey_F10, GrKey_Escape,
    /* shift F1 == F11, shift F2 == F12  What code should be returned ?? */
    GrKey_Shift_F1, GrKey_Shift_F2,
    GrKey_Shift_F3, GrKey_Shift_F4,
    GrKey_Escape, GrKey_Shift_F5, GrKey_Shift_F6, GrKey_Escape,
    GrKey_Shift_F7, GrKey_Shift_F8, GrKey_Shift_F9, GrKey_Shift_F10 };

static const GrKeyType alt[] = {
    GrKey_Escape, GrKey_Alt_Escape,
    0x7f,         GrKey_Alt_Backspace,
    'q',          GrKey_Alt_Q,
    'w',          GrKey_Alt_W,
    'e',          GrKey_Alt_E,
    'r',          GrKey_Alt_R,
    't',          GrKey_Alt_T,
    'y',          GrKey_Alt_Y,
    'u',          GrKey_Alt_U,
    'i',          GrKey_Alt_I,
    'o',          GrKey_Alt_O,
    'p',          GrKey_Alt_P,
    '[',          GrKey_Alt_LBracket,
    ']',          GrKey_Alt_RBracket,
    GrKey_Return, GrKey_Alt_Return,
    'a',          GrKey_Alt_A,
    's',          GrKey_Alt_S,
    'd',          GrKey_Alt_D,
    'f',          GrKey_Alt_F,
    'g',          GrKey_Alt_G,
    'h',          GrKey_Alt_H,
    'j',          GrKey_Alt_J,
    'k',          GrKey_Alt_K,
    'l',          GrKey_Alt_L,
    ';',          GrKey_Alt_Semicolon,
    '\'',         GrKey_Alt_Quote,
    '\x60',       GrKey_Alt_Backquote,
    '\\',         GrKey_Alt_Backslash,
    'z',          GrKey_Alt_Z,
    'x',          GrKey_Alt_X,
    'c',          GrKey_Alt_C,
    'v',          GrKey_Alt_V,
    'b',          GrKey_Alt_B,
    'n',          GrKey_Alt_N,
    'm',          GrKey_Alt_M,
    ',',          GrKey_Alt_Comma,
    '.',          GrKey_Alt_Period,
    '/',          GrKey_Alt_Slash,
/*                GrKey_Alt_F1 */
/*                GrKey_Alt_F2 */
/*                GrKey_Alt_F3 */
/*                GrKey_Alt_F4 */
/*                GrKey_Alt_F5 */
/*                GrKey_Alt_F6 */
/*                GrKey_Alt_F7 */
/*                GrKey_Alt_F8 */
/*                GrKey_Alt_F9 */
/*                GrKey_Alt_F10 */
    '1',          GrKey_Alt_1,
    '2',          GrKey_Alt_2,
    '3',          GrKey_Alt_3,
    '4',          GrKey_Alt_4,
    '5',          GrKey_Alt_5,
    '6',          GrKey_Alt_6,
    '7',          GrKey_Alt_7,
    '8',          GrKey_Alt_8,
    '9',          GrKey_Alt_9,
    '0',          GrKey_Alt_0,
    '-',          GrKey_Alt_Dash,
    '=',          GrKey_Alt_Equals,
/*                GrKey_Alt_F11 */
/*                GrKey_Alt_F12 */
/*                GrKey_Alt_KPSlash */
    GrKey_Tab,    GrKey_Alt_Tab,
/*                GrKey_Alt_Enter */
    '<',          GrKey_Alt_LAngle,
    '>',          GrKey_Alt_RAngle,
    '@',          GrKey_Alt_At,
    '{',          GrKey_Alt_LBrace,
    '|',          GrKey_Alt_Pipe,
    '}',          GrKey_Alt_RBrace,
    0x0000,       GrKey_NoKey
};

static GrKeyType keytrans(GrKeyType k, const GrKeyType *t) {
  while (t[0] != 0x0000 && t[1] != GrKey_NoKey) {
    if (k == t[0]) return t[1];
    t += 2;
  }
  return GrKey_OutsideValidRange;
}

static
int inkey(void) {
   int  Key, Key2;

Restart:
   Key = getByte(1);
   if (Key!=GrKey_Escape) {
     if (Key == 0x7f) return(GrKey_BackSpace);
     return(Key);
   }
   if (n==0) return(GrKey_Escape);
   /* We have ^[ and something more after that */

   Key = getByte(1);
   if (Key == EOF) return(GrKey_Escape);
   if (Key!='[') return(keytrans(Key,alt));   /* Check for Alt+Key */

   /* We have ^[[ */
   if (n==0) return(GrKey_Alt_LBracket);

   Key = getByte(1);
   if (Key == EOF) return(GrKey_Alt_LBracket);;
   switch (Key)
     {
	  case 'A': return(GrKey_Up);
	  case 'B': return(GrKey_Down);
	  case 'C': return(GrKey_Right);
	  case 'D': return(GrKey_Left);
	  case 'G': return(GrKey_Center);
	  case '[': Key = getByte(1);
		    switch (Key)
		      {
			   case 'A': return(GrKey_F1);
			   case 'B': return(GrKey_F2);
			   case 'C': return(GrKey_F3);
			   case 'D': return(GrKey_F4);
			   case 'E': return(GrKey_F5);
			   default: goto Restart;
		      }
	  default:  if (Key>='1' && Key<='6')
		      {
			  Key2 = getByte(1);
			  if (Key2=='~') switch (Key)
			    {
			       case '1': return(GrKey_Home);
			       case '2': return(GrKey_Insert);
			       case '3': return(GrKey_Delete);
			       case '4': return(GrKey_End);
			       case '5': return(GrKey_PageUp);
			       case '6': return(GrKey_PageDown);
			    }
			  else if (Key2>='0' && Key2<='9')
			    {
			       int  index = 10*(Key-'0') + (Key2 - '0');
				if (getByte(1)!='~') goto Restart;
				if (17 <= index && index <= 34)
				     return(fnk[index-17]);
				goto Restart;
			    }
		      }
	  goto Restart;
     }
}

static int  validKey(int key, int valid) {
  if (key < 0 || key > GrKey_LastDefinedKeycode || key == EOF)
    return valid;
  return key;
}

int _GrCheckKeyboardHit(void)
{
	if(!kbd_initted) {
	    kbd_init();
	}
	if(!kbd_isatty) {
	    return(TRUE);
	}
	if(kbd_lastchr != EOF) {
	    return(TRUE);
	}
	if(kbd_mode != test) {
	    kbd_setup.c_cc[VMIN]  = 0;
	    kbd_setup.c_cc[VTIME] = 0;
	    if(ioctl(kbd_filedsc,TCSETAW,&kbd_setup) == EOF) return(FALSE);
	    kbd_mode = test;
	}
	kbd_lastchr = validKey(inkey(), EOF);
	return (kbd_lastchr != EOF);
}

int _GrReadCharFromKeyboard(void)
{
    int key = EOF;

    if(!kbd_initted) {
	kbd_init();
    }
    if(!kbd_isatty) {
	return(getc(stdin));
    }
    do {
	if(kbd_lastchr != EOF) {
	    key = kbd_lastchr;
	    kbd_lastchr = EOF;
	    break;
	}
	if(kbd_mode == test) {
	  /* we're in test mode. Look for key without mode switch ... */
	  _GrCheckKeyboardHit();
	  if(kbd_lastchr != EOF) {
	      key = kbd_lastchr;
	      kbd_lastchr = EOF;
	      break;
	  }
	}
	/* no key till now. Wait of it ... */
	if(kbd_mode != wait) {
	    kbd_setup.c_cc[VMIN]  = 1;
	    kbd_setup.c_cc[VTIME] = 0;
	    if(ioctl(kbd_filedsc,TCSETAW,&kbd_setup) == EOF) return(EOF);
	    kbd_mode = wait;
	}
	key = inkey();
    } while (0);
    return validKey(key, GrKey_OutsideValidRange);
}

/*
 * Mouse stuff
 */

static int  mou_filedsc;
static int  mou_lastxpos;
static int  mou_lastypos;
static int  mou_buttons;
static int  mou_enabled = TRUE;
static long evt_lasttime;

int GrMouseDetect(void)
{
	if(MOUINFO->msstatus == 0) {
	    MOUINFO->msstatus = (-1);           /* assume missing */
	    mou_filedsc  = mouse_init_return_fd(
		"/dev/mouse",
		vga_getmousetype(),
		MOUSE_DEFAULTSAMPLERATE
	    );
	    if(mou_filedsc >= 0) {
		MOUINFO->msstatus = 1;          /* present, but not initted */
		atexit(mouse_close);
		mouse_setxrange(0,32767);
		mouse_setyrange(0,32767);
		mouse_setwrap(MOUSE_NOWRAP);
		mouse_setscale(16);
	    }
	}
	return((MOUINFO->msstatus > 0) ? TRUE : FALSE);
}

static void uninit(void)
{
	if(MOUINFO->msstatus > 1) MOUINFO->msstatus = 1;
	kbd_restore();
}

void GrMouseInitN(int queue_size)
{
	uninit();
	queue_size = umax(4,umin(256,queue_size));
	init_queue(queue_size);
	kbd_init();
	if(GrMouseDetect()) {
	    GrMouseSetSpeed(1,1);
	    GrMouseSetAccel(100,1);
	    GrMouseSetLimits(0,0,SCRN->gc_xmax,SCRN->gc_ymax);
	    GrMouseWarp((SCRN->gc_xmax >> 1),(SCRN->gc_ymax >> 1));
	    _GrInitMouseCursor();
	    MOUINFO->msstatus = 2;
	}
	GrMouseEventEnable(TRUE,TRUE);
	real_time(evt_lasttime);
	MOUINFO->uninit = uninit;
}

void GrMouseSetSpeed(int spmult,int spdiv)
{
	MOUINFO->spmult = umin(16,umax(1,spmult));
	MOUINFO->spdiv  = umin(16,umax(1,spdiv));
}

void GrMouseSetAccel(int thresh,int accel)
{
	MOUINFO->thresh = umin(64,umax(1,thresh));
	MOUINFO->accel  = umin(16,umax(1,accel));
}

void GrMouseSetLimits(int x1,int y1,int x2,int y2)
{
	isort(x1,x2);
	isort(y1,y2);
	MOUINFO->xmin = imax(0,imin(x1,SCRN->gc_xmax));
	MOUINFO->ymin = imax(0,imin(y1,SCRN->gc_ymax));
	MOUINFO->xmax = imax(0,imin(x2,SCRN->gc_xmax));
	MOUINFO->ymax = imax(0,imin(y2,SCRN->gc_ymax));
}

void GrMouseWarp(int x,int y)
{
	MOUINFO->xpos = imax(MOUINFO->xmin,imin(MOUINFO->xmax,x));
	MOUINFO->ypos = imax(MOUINFO->ymin,imin(MOUINFO->ymax,y));
	mou_lastxpos  = 16384;
	mou_lastypos  = 16384;
	mouse_setposition(16384,16384);
	GrMouseUpdateCursor();
}

void GrMouseEventEnable(int enable_kb,int enable_ms)
{
	if(!enable_kb) kbd_restore();
	kbd_enabled = enable_kb;
	mou_enabled = enable_ms;
}

void _GrUpdateInputs(void)
{
	for( ; ; ) {
	    GrMouseEvent ev;
	    int gotevt = FALSE;
	    if(mou_enabled && (MOUINFO->msstatus == 2) && mouse_update()) {
		int mx = mouse_getx();
		int my = mouse_gety();
		int mb = mouse_getbutton();
		update_coord(x,(mx - mou_lastxpos));
		update_coord(y,(my - mou_lastypos));
		mou_lastxpos = mx;
		mou_lastypos = my;
		if(mb != mou_buttons) {
		    fill_mouse_ev(
			ev,
			mou_buttons,mb,
			MOUSE_LEFTBUTTON,
			MOUSE_MIDDLEBUTTON,
			MOUSE_RIGHTBUTTON,
			MOUSE_FOURTHBUTTON,
			MOUSE_FIFTHBUTTON,
			0
		    );
		    real_dtime(ev.dtime,evt_lasttime);
		    enqueue_event(ev);
		    MOUINFO->moved = FALSE;
		    mou_buttons = mb;
		}
		gotevt = TRUE;
	    }
	    if(kbd_enabled && kbd_isatty && _GrCheckKeyboardHit()) {
		int key = _GrReadCharFromKeyboard();
		fill_keybd_ev(ev,key,0);
		real_dtime(ev.dtime,evt_lasttime);
		enqueue_event(ev);
		MOUINFO->moved = FALSE;
		gotevt = TRUE;
	    }
	    if(!gotevt) break;
	}
}

void GrMouseGetEventT(int flags,GrMouseEvent *ev,long tout)
{
	int msdraw;
	if(MOUINFO->msstatus == 0) GrMouseInit();
	msdraw = !MOUINFO->displayed && !(flags & GR_M_NOPAINT);
	if(msdraw) GrMouseDisplayCursor();
	for( ; ; ) {
	    fd_set readfds;
	    struct timeval wtime;
	    int    N;
	    _GrUpdateInputs();
	    GrMouseUpdateCursor();
	    while(MOUINFO->qlength > 0) {
		dequeue_event((*ev));
		if(ev->flags & flags) {
		    if(msdraw) GrMouseEraseCursor();
		    return;
		}
	    }
	    if((flags & GR_M_POLL) ||
	       (tout == 0) ||
	       (MOUINFO->moved && (flags & GR_M_MOTION))) {
		fill_mouse_ev(
		    (*ev),
		    mou_buttons,mou_buttons,
		    MOUSE_LEFTBUTTON,
		    MOUSE_MIDDLEBUTTON,
		    MOUSE_RIGHTBUTTON,
		    MOUSE_FOURTHBUTTON,
		    MOUSE_FIFTHBUTTON,
		    0
		);
		if ( ev->flags ) /* something happend */
		  real_dtime(ev->dtime,evt_lasttime);
		else
		  ev->dtime = -1; /* special time is nothing happend */
		MOUINFO->moved = FALSE;
		if(msdraw) GrMouseEraseCursor();
		return;
	    }
	    FD_ZERO(&readfds);
	    N = 0;
	    if(kbd_enabled && kbd_isatty) {
		FD_SET(kbd_filedsc,&readfds);
		N = kbd_filedsc + 1;
	    }
	    if(mou_enabled && (MOUINFO->msstatus == 2)) {
		FD_SET(mou_filedsc,&readfds);
		N = umax(N,(mou_filedsc + 1));
	    }
	    if(N == 0) {
		if(tout > 0) usleep(tout * 1000);
		tout = 0;
		continue;
	    }
	    if(tout > 0) {
		wtime.tv_sec  = (tout / 1000);
		wtime.tv_usec = (tout % 1000) * 1000;
		if(select(N,&readfds,NULL,NULL,&wtime) < 1)
		    tout = 0;
		continue;
	    }
	    select(N,&readfds,NULL,NULL,NULL);
	}
}

