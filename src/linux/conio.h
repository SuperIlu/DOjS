/*
MIT License

Copyright (c) 2019-2021 Andre Seidelt <superilu@yahoo.com>

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

#ifndef __CONIO_H__
#define __CONIO_H__

#include <allegro.h>
#if WINDOWS==1
#include <winalleg.h>
#endif

#define TO_CIO(x) ((uint32_t)x << 8)
#define CIO_EXT(x) (((uint32_t)x) | (1U << 18))
#define CIO_CTRL(x) (((uint32_t)x) | (1U << 17))
#define CIO_SHIFT(x) (((uint32_t)x) | (1U << 16))

#define K_Escape 0x1b                         // OK
#define K_Right TO_CIO(KEY_RIGHT)             // OK
#define K_Left TO_CIO(KEY_LEFT)               // OK
#define K_Up TO_CIO(KEY_UP)                   // OK
#define K_Down TO_CIO(KEY_DOWN)               // OK
#define K_PageUp TO_CIO(KEY_PGUP)             // OK
#define K_PageDown TO_CIO(KEY_PGDN)           // OK
#define K_BackSpace 0x08                      // OK
#define K_Delete 0x7f                         // OK
#define K_Return 0xd                          // OK
#define K_Home TO_CIO(KEY_HOME)               // OK
#define K_End TO_CIO(KEY_END)                 // OK
#define K_Tab (TO_CIO(KEY_TAB) | 0x09)        // OK
#define K_BackTab CIO_SHIFT(TO_CIO(KEY_TAB))  // OK

#define K_Control_Backspace CIO_CTRL(TO_CIO(KEY_BACKSPACE) | K_BackSpace)  // OK
#define K_Control_Delete CIO_CTRL(TO_CIO(KEY_DEL) | K_Delete)              // OK
#define K_Control_Down CIO_CTRL(TO_CIO(KEY_DOWN))                          // OK
#define K_Control_Up CIO_CTRL(TO_CIO(KEY_UP))                              // OK
#define K_Control_Left CIO_CTRL(TO_CIO(KEY_LEFT))                          // OK
#define K_Control_Right CIO_CTRL(TO_CIO(KEY_RIGHT))                        // OK
#define K_Control_Home CIO_CTRL(TO_CIO(KEY_HOME))                          // OK
#define K_Control_End CIO_CTRL(TO_CIO(KEY_END))                            // OK

#define K_Control_C CIO_CTRL('C' - '@')  // OK
#define K_Control_D CIO_CTRL('D' - '@')  // OK
#define K_Control_L CIO_CTRL('L' - '@')  // OK
#define K_Control_X CIO_CTRL('X' - '@')  // OK
#define K_Control_V CIO_CTRL('V' - '@')  // OK
#define K_Control_F CIO_CTRL('F' - '@')  // OK

#define K_F1 TO_CIO(KEY_F1)    // OK
#define K_F2 TO_CIO(KEY_F2)    // OK
#define K_F3 TO_CIO(KEY_F3)    // OK
#define K_F4 TO_CIO(KEY_F4)    // OK
#define K_F5 TO_CIO(KEY_F5)    // OK
#define K_F6 TO_CIO(KEY_F6)    // OK
#define K_F7 TO_CIO(KEY_F7)    // OK
#define K_F8 TO_CIO(KEY_F8)    // OK
#define K_F9 TO_CIO(KEY_F9)    // OK
#define K_F10 TO_CIO(KEY_F10)  // OK
#define K_F11 TO_CIO(KEY_F11)  // OK
#define K_F12 TO_CIO(KEY_F12)  // OK

#define K_Shift_F1 CIO_SHIFT(TO_CIO(KEY_F1))    // OK
#define K_Shift_F2 CIO_SHIFT(TO_CIO(KEY_F2))    // OK
#define K_Shift_F3 CIO_SHIFT(TO_CIO(KEY_F3))    // OK
#define K_Shift_F4 CIO_SHIFT(TO_CIO(KEY_F4))    // OK
#define K_Shift_F5 CIO_SHIFT(TO_CIO(KEY_F5))    // OK
#define K_Shift_F6 CIO_SHIFT(TO_CIO(KEY_F6))    // OK
#define K_Shift_F7 CIO_SHIFT(TO_CIO(KEY_F7))    // OK
#define K_Shift_F8 CIO_SHIFT(TO_CIO(KEY_F8))    // OK
#define K_Shift_F9 CIO_SHIFT(TO_CIO(KEY_F9))    // OK
#define K_Shift_F10 CIO_SHIFT(TO_CIO(KEY_F10))  // OK
#define K_Shift_F11 CIO_SHIFT(TO_CIO(KEY_F11))  // OK
#define K_Shift_F12 CIO_SHIFT(TO_CIO(KEY_F12))  // OK

#define K_EDelete CIO_EXT(TO_CIO(KEY_DEL))
#define K_EHome CIO_EXT(TO_CIO(KEY_HOME))
#define K_EEnd CIO_EXT(TO_CIO(KEY_END))
#define K_EUp CIO_EXT(TO_CIO(KEY_UP))
#define K_EDown CIO_EXT(TO_CIO(KEY_DOWN))
#define K_EPageUp CIO_EXT(TO_CIO(KEY_PGUP))
#define K_EPageDown CIO_EXT(TO_CIO(KEY_PGDN))
#define K_ERight CIO_EXT(TO_CIO(KEY_LEFT))
#define K_ELeft CIO_EXT(TO_CIO(KEY_RIGHT))

#define K_Control_EBackSpace CIO_EXT(CIO_CTRL(TO_CIO(KEY_BACKSPACE)))
#define K_Control_EDelete CIO_EXT(CIO_CTRL(TO_CIO(KEY_DEL)))
#define K_Control_EDown CIO_EXT(CIO_CTRL(TO_CIO(KEY_DOWN)))
#define K_Control_EUp CIO_EXT(CIO_CTRL(TO_CIO(KEY_UP)))
#define K_Control_ELeft CIO_EXT(CIO_CTRL(TO_CIO(KEY_LEFT)))
#define K_Control_ERight CIO_EXT(CIO_CTRL(TO_CIO(KEY_RIGHT)))
#define K_Control_EHome CIO_EXT(CIO_CTRL(TO_CIO(KEY_HOME)))
#define K_Control_EEnd CIO_EXT(CIO_CTRL(TO_CIO(KEY_END)))

#define _NKEYBRD_SHIFTSTATUS 0
#define _NORMALCURSOR 0

typedef enum {
    BLACK = 0,
    WHITE = 1,
    RED = 2,
    GREEN = 3,
    BLUE = 4,
    YELLOW = 5,
    CYAN = 6,
    MAGENTA = 7,
    LIGHTGRAY = 8,
    LIGHTGREEN = 9,
    BROWN = 10,
    LIGHTBLUE = 11,
    LIGHTMAGENTA = 12,
    LIGHTRED = 13,
    LIGHTCYAN = 14
} conio_color_t;

typedef enum { C80, C4350 } conio_textmode_t;

struct text_info {
    unsigned int winleft;
    unsigned int wintop;
    unsigned int winright;
    unsigned int winbottom;
    // unsigned int attribute;
    // unsigned int normattr;
    // unsigned int currmode;
    unsigned int screenheight;
    unsigned int screenwidth;
    // unsigned int curx;
    // unsigned int cury;
};

extern int _wscroll;

/**
 * @brief Waits for the user to press one key, then returns that key. Alt-key combinations have 0x100 added to them, and extended keys have 0x200 added to them.
 */
extern int getxkey(void);

/**
 * @brief This function issues the BIOS keyboard interrupt 16h with command in the AH register, and returns the results of that call.
 */
extern int bioskey(int command);

/**
 * @brief Clear the entire screen.
 */
extern void clrscr(void);

/**
 * @brief Sets the cursor type. _type is one of the following:
 */
extern void _setcursortype(int _type);

/**
 * @brief This function returns the parameters of the current window on the screen.
 */
extern void gettextinfo(struct text_info *_r);

/**
 * @brief Sets the text mode of the screen.
 */
extern void textmode(int _mode);

/**
 * @brief Sets just the background of the text attribute. See section textattr.
 */
extern void textbackground(int _color);

/**
 * @brief Sets just the foreground of the text attribute. See section textattr.
 */
extern void textcolor(int _color);

/**
 * @brief Put the character _c on the screen at the current cursor position. The special characters return, linefeed, bell, and backspace are handled properly, as is line wrap and
 * scrolling. The cursor position is updated.
 */
extern int putch(int _c);

/**
 * @brief Puts the string onto the console. The cursor position is updated.
 */
extern int cputs(const char *_str);

/**
 * @brief Move the cursor to row y, column x. The upper left corner of the current window is (1,1).
 */
extern void gotoxy(int x, int y);

/**
 * @brief The column the cursor is on. The leftmost column is 1.
 */
extern int wherex(void);

/**
 * @brief The row the cursor is on. The topmost row is 1.
 */
extern int wherey(void);

/**
 * @brief Clear to end of line.
 */
extern void clreol(void);

/**
 * @brief Resets the text attribute to what it was before the program started.
 */
extern void normvideo(void);

#ifdef EDI_FAST
/**
 * @brief This function writes the contents of the buffer buf to the primary screen. The buffer should contain an exact replica of the video memory, including the characters and
 * their attributes.
 */
void ScreenUpdate(void *buf);

/**
 * @brief This function writes the contents of buf to the screen line number given in row (the topmost line is row 0), on the primary screen.
 */
void ScreenUpdateLine(void *buf, int row);
#endif

#endif  // __CONIO_H__
