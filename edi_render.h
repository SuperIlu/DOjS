/*
MIT License

Copyright (c) 2019-2020 Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __EDI_RENDER_H__
#define __EDI_RENDER_H__

#include "edit.h"

/************
** defines **
************/
#ifndef EDI_FAST
#define edi_textcolor(c) textcolor(c)
#define edi_textbackground(c) textbackground(c)
#define edi_gotoxy(e, x, y) gotoxy(x, y)
#define edi_putch(e, c) putch(c)
#define edi_cputs(e, s) cputs(s)
#define edi_clreol(e) clreol()
#define edi_wherex(edi) wherex()
#define edi_refresh_screen(e) ;
#define edi_refresh_line(e, y) ;
#endif

#ifdef EDI_FAST
extern uint16_t edi_attributes;
#endif

/***********************
** exported functions **
***********************/
extern void edi_redraw(edi_t* edi);
extern void edi_get_cnp(edi_t* edi, cnp_t* cnp);

#ifdef EDI_FAST
/**
 * @brief set text color.
 *
 * @param c the new color
 */
static inline void edi_textcolor(char c) { edi_attributes = ((edi_attributes & 0xF000) | (c << 8)) & 0x7FFF; }

/**
 * @brief set background color.
 *
 * @param c the new color
 */
static inline void edi_textbackground(char c) { edi_attributes = ((edi_attributes & 0x0F00) | (c << 12)) & 0x7FFF; }

/**
 * @brief put character at current position.
 *
 * @param edi the edi system to work on.
 * @param c the character.
 */
static inline void edi_putch(edi_t* edi, char c) {
    edi->screen[edi->scr_idx] = edi_attributes | (c & 0xFF);
    edi->scr_idx++;
    edi->scr_x++;
}

/**
 * @brief put string at current position.
 *
 * @param edi the edi system to work on.
 * @param s the NULL terminated string.
 */
static inline void edi_cputs(edi_t* edi, char* s) {
    while (*s) {
        edi_putch(edi, *s);
        s++;
    }
}

/**
 * @brief set current rendering position. The screen cursor position is NOT altered by this function!
 *
 * @param edi the edi system to work on.
 * @param x x position.
 * @param y y position.
 */
static inline void edi_gotoxy(edi_t* edi, int x, int y) {
    edi->scr_idx = (x - 1) + (y - 1) * edi->scr.screenwidth;
    edi->scr_x = x;
}

/**
 * @brief update on-screen line from buffer.
 *
 * @param edi the edi system to work on.
 * @param y y position.
 */
static inline void edi_refresh_line(edi_t* edi, int y) { ScreenUpdateLine(&edi->screen[(y - 1) * edi->scr.screenwidth], y - 1); }

/**
 * @brief update the whole screen from buffer.
 *
 * @param edi the edi system to work on.
 */
static inline void edi_refresh_screen(edi_t* edi) { ScreenUpdate(edi->screen); }

/**
 * @brief clear buffer till the end of line (no color, blank character).
 *
 * @param edi the edi system to work on.
 */
static inline void edi_clreol(edi_t* edi) {
    while (edi->scr_x < edi->scr.screenwidth) {
        edi_putch(edi, ' ');
    }
}

/**
 * @brief get current x position.
 *
 * @param edi the edi system to work on.
 * @return int the current position in the buffer (NOT on screen).
 */
static inline int edi_wherex(edi_t* edi) { return edi->scr_x; }
#endif

#endif  // __EDI_RENDER_H__
