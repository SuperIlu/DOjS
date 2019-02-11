/**
 ** xwinkeys.c ---- DOS (TCC/BCC/DJGPP: "conio.h") style keyboard utilities
 **
 ** Author:     Ulrich Leodolter
 ** E-mail:     ulrich@lab1.psy.univie.ac.at
 ** Date:       Sun Oct  1 08:10:30 1995
 ** RCSId:      $Id: xwinkeys.c 1.1 1995/11/19 16:34:52 ulrich Exp $
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
 **/

#include "libgrx.h"
#include "libxwin.h"
#include "input.h"
#include "grxkeys.h"

#define _NOKEY_ (-1)
static int lastkey = _NOKEY_;
static int lastgetchkey = _NOKEY_;

static int getkey_w (int delay)
{
  GrMouseEvent ev;
  if(MOUINFO->msstatus < 2) {
     GrMouseInit();
     GrMouseEventEnable(1,0);
  }
  GrMouseGetEventT((GR_M_EVENT | GR_M_NOPAINT),&ev,delay);
  if(ev.flags & GR_M_KEYPRESS)
    return(ev.key);
  return _NOKEY_;
}

int getkey(void)
{
  int key;
  lastgetchkey = _NOKEY_;
  if (lastkey != _NOKEY_) {
    key = lastkey;
    lastkey = _NOKEY_;
  } 
  else do {
    key = getkey_w (1L);
  } while (key == _NOKEY_);
  return key;
}

int getch(void)
{
  int key;
  if (lastgetchkey != _NOKEY_) {
    key = lastgetchkey;
    lastgetchkey = _NOKEY_;
    return(key);
  }
  if (lastkey != _NOKEY_) {
    key = lastkey;
    lastkey = _NOKEY_;
  }
  else
    key = getkey();
  if(key < 0x100) {
    return(key);
  }
  lastgetchkey = key & 0xff;
  return(0);
}

int kbhit(void)
{
  int key;
  if (lastkey != _NOKEY_ || lastgetchkey != _NOKEY_)
    return TRUE;
  key = getkey_w (0);
  if (key != _NOKEY_) {
    lastkey = key;
    return TRUE;
  }
  return FALSE;
}

int getkbstat(void)
{
  if(MOUINFO->msstatus < 2) {
    GrMouseInit();
    GrMouseEventEnable(1,0);
  }
  return(_XGrKeyboardGetState());
}

/*
** new functions to replace the old style
**   kbhit / getch / getkey / getxkey / getkbstat
** keyboard interface
*/

int GrKeyPressed(void)
{
  int  key;
  if (lastkey != _NOKEY_)
    return TRUE;
  key = getkey_w (0);
  if (key==_NOKEY_)
    return FALSE;
  lastkey = key;
  return TRUE;    
}

GrKeyType GrKeyRead(void)
{
  int key;
  if (lastkey != _NOKEY_){
    key = lastkey;
    lastkey = _NOKEY_;
    return (GrKeyType) key;
  }
  do {
    key = getkey_w (1);
  } while (key == _NOKEY_);
  return key;
}

int GrKeyStat(void) {
  return getkbstat();
}
