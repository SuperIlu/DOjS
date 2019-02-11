/**
 ** mousex.h ---- GRX 2.0 -> 1.0x mouse backward compatibility declarations
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
 **/

#ifndef __MOUSEX_H_INCLUDED__
#define __MOUSEX_H_INCLUDED__

#ifndef __GRX20_H_INCLUDED__
#include "grx20.h"
#endif

#ifndef M_MOTION			/* "eventque.h" also defines these */
#define M_MOTION			GR_M_MOTION
#define M_LEFT_DOWN			GR_M_LEFT_DOWN
#define M_LEFT_UP			GR_M_LEFT_UP
#define M_RIGHT_DOWN			GR_M_RIGHT_DOWN
#define M_RIGHT_UP			GR_M_RIGHT_UP
#define M_MIDDLE_DOWN			GR_M_MIDDLE_DOWN
#define M_MIDDLE_UP			GR_M_MIDDLE_UP
#define M_BUTTON_DOWN			GR_M_BUTTON_DOWN
#define M_BUTTON_UP			GR_M_BUTTON_UP
#define M_BUTTON_CHANGE			GR_M_BUTTON_CHANGE
#define M_LEFT				GR_M_LEFT
#define M_RIGHT				GR_M_RIGHT
#define M_MIDDLE			GR_M_MIDDLE
#endif					/* M_MOTION */

#define M_KEYPRESS			GR_M_KEYPRESS
#define M_POLL				GR_M_POLL
#define M_NOPAINT			GR_M_NOPAINT
#define M_EVENT				GR_M_EVENT

#ifndef KB_SHIFT			/* "eventque.h" also defines these */
#define KB_RIGHTSHIFT			GR_KB_RIGHTSHIFT
#define KB_LEFTSHIFT			GR_KB_LEFTSHIFT
#define KB_CTRL				GR_KB_CTRL
#define KB_ALT				GR_KB_ALT
#define KB_SCROLLOCK			GR_KB_SCROLLOCK
#define KB_NUMLOCK			GR_KB_NUMLOCK
#define KB_CAPSLOCK			GR_KB_CAPSLOCK
#define KB_INSERT			GR_KB_INSERT
#define KB_SHIFT			GR_KB_SHIFT
#endif					/* KB_SHIFT */

#define M_CUR_NORMAL			GR_M_CUR_NORMAL
#define M_CUR_RUBBER			GR_M_CUR_RUBBER
#define M_CUR_LINE			GR_M_CUR_LINE
#define M_CUR_BOX			GR_M_CUR_BOX

#define MouseEvent			GrMouseEvent
#define MouseDetect			GrMouseDetect
#define MouseEventMode			GrMouseEventMode
#define MouseInit			GrMouseInit
#define MouseUnInit			GrMouseUnInit
#define MouseSetSpeed(s)		GrMouseSetSpeed(1,s)
#define MouseSetAccel			GrMouseSetAccel
#define MouseSetLimits			GrMouseSetLimits
#define MouseGetLimits			GrMouseGetLimits
#define MouseWarp			GrMouseWarp
#define MouseEventEnable		GrMouseEventEnable
#define MouseGetEvent			GrMouseGetEvent
#define MousePendingEvent		GrMousePendingEvent
#define MouseGetCursor			GrMouseGetCursor
#define MouseSetCursor			GrMouseSetCursor
#define MouseSetColors			GrMouseSetColors
#define MouseSetCursorMode		GrMouseSetCursorMode
#define MouseDisplayCursor		GrMouseDisplayCursor
#define MouseEraseCursor		GrMouseEraseCursor
#define MouseBlock			GrMouseBlock
#define MouseUnBlock			GrMouseUnBlock
#define MouseCursorIsDisplayed		GrMouseCursorIsDisplayed

#endif  /* whole file */

