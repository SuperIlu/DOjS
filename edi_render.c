/*
MIT License

Copyright (c) 2019 Andre Seidelt <superilu@yahoo.com>

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
#include <ctype.h>
#include <string.h>

#include "DOjS.h"
#include "dialog.h"
#include "edi_render.h"
#include "syntax.h"

/************************
** function prototypes **
************************/
static void edi_draw_line(edi_t* edi, line_t** l, int y, int offset, int line_num);
static void edi_draw_status(edi_t* edi);
static void edi_draw_commands(edi_t* edi);
static int edi_syntax(line_t* l, int pos);
static void edi_sel_color(edi_t* edi, int x, int y);
static bool edi_colcmp(const syntax_t* sy, char* txt, int remainder);

/************
** defines **
************/
#define EDI_NUM_COMMANDS 10  //!< number of commands

/********************
** local variables **
********************/
//! array with command texts
static const char* edi_f_keys[] = {"Help", "", "Save", "Run", "", "", "Find", "", "Log", "Exit"};

#ifdef EDI_FAST
//! current text attributes
uint16_t edi_attributes = 0x0000;
#endif

/**
 * @brief match a syntax highlight string with current cursor position.
 *
 * @param sy word to find
 * @param txt start of text to compare with
 * @param remainder remaining length of txt.
 * @return true if this is a match
 * @return false if no match was found
 */
static bool edi_colcmp(const syntax_t* sy, char* txt, int remainder) {
    char* find = sy->word;
    if (remainder < strlen(find)) {
        return false;
    }

    while (*find) {
        if (*find != *txt) {
            return false;
        }
        txt++;
        find++;
        remainder--;
    }

    if (remainder == 0 || !isalnum(*txt) || sy->length == -1) {
        return true;
    } else {
        return false;
    }
}

/**
 * @brief switch display color for syntax highlighting word.
 *
 * @param l the line we are working on.
 * @param pos the cursor position in the line.
 *
 * @return int the number of characters to draw in the just set color or 0 if no match was found.
 */
static int edi_syntax(line_t* l, int pos) {
    int w = 0;
    while (edi_wordlist[w].word) {
        if (pos == 0 || edi_wordlist[w].word[0] == '.' || !isalnum(l->txt[pos - 1])) {
            if (edi_colcmp(&edi_wordlist[w], &l->txt[pos], l->length - pos)) {
                edi_textcolor(edi_wordlist[w].color);
                if (edi_wordlist[w].length == -1) {
                    return l->length - pos;
                } else {
                    return edi_wordlist[w].length;
                }
            }
        }
        w++;
    }
    return 0;
}

/**
 * @brief set background color for text according to current select status.
 *
 * @param edi the edi system to work on.
 * @param x x-position in whole text.
 * @param y y-position in whole text.
 */
static void edi_sel_color(edi_t* edi, int x, int y) {
    edi_textbackground(BLACK);
    if (EDI_IS_SEL(edi)) {  // check selection is active
        cnp_t cnp;
        edi_get_cnp(edi, &cnp);

        if ((cnp.startY == cnp.endY) && (y == cnp.startY)) {
            if ((x >= cnp.startX) && (x < cnp.endX)) {
                edi_textbackground(BLUE);
            }
        } else {
            if ((y == cnp.startY) && (x >= cnp.startX)) {
                edi_textbackground(BLUE);
            } else if ((y > cnp.startY) && (y < cnp.endY)) {
                edi_textbackground(BLUE);
            } else if ((y == cnp.endY) && (x < cnp.endX)) {
                edi_textbackground(BLUE);
            }
        }
    }
}

/**
 * @brief draw given line onto screen.
 *
 * @param edi the edi system to work on.
 * @param l the line to draw.
 * @param y y position on the currently drawn screen.
 * @param offset x-offset for the currently drawn screen (horizontal scrolling of long lines).
 * @param line_num the line number of the currently drawn line (total number).
 */
static void edi_draw_line(edi_t* edi, line_t** l, int y, int offset, int line_num) {
    edi_textcolor(WHITE);
    edi_gotoxy(edi, 1, y);
    int hskip = 0;
    if (*l) {
        for (int i = offset; i < (*l)->length && edi_wherex(edi) < edi->width; i++) {
            if (hskip <= 0) {
                edi_textcolor(WHITE);
                hskip = edi_syntax(*l, i);
            }
            edi_sel_color(edi, i, line_num);
            edi_putch(edi, (*l)->txt[i]);
            if (hskip > 0) {
                hskip--;
            }
        }
        *l = (*l)->next;
    }
    edi_textbackground(BLACK);
    edi_clreol(edi);
}

/**
 * @brief draw status bar.
 *
 * @param edi the edi system to work on.
 */
static void edi_draw_status(edi_t* edi) {
    edi_textbackground(LIGHTGRAY);
    edi_textcolor(BLACK);

    // draw 'modified' indicator
    edi_gotoxy(edi, edi->scr.winleft, edi->scr.wintop);
    edi_cputs(edi, "DOjS " DOSJS_VERSION_STR " ");
    if (edi->changed) {
        edi_cputs(edi, "* ");
    } else {
        edi_cputs(edi, "  ");
    }

    // draw file/script name
    if (edi->name) {
        edi_cputs(edi, edi->name);
    } else {
        edi_cputs(edi, "<unnamed>");
    }

    for (int i = edi_wherex(edi); i < edi->scr.winright; i++) {
        edi_cputs(edi, " ");
    }

    // draw cursor position
    char buff[10];
    edi_gotoxy(edi, edi->scr.winright - 8, edi->scr.wintop);
    snprintf(buff, sizeof(buff), "%04d/%04d", edi->num, edi->x + 1);
    edi_cputs(edi, buff);
}

/**
 * @brief draw command bar.
 *
 * @param edi the edi system to work on.
 */
static void edi_draw_commands(edi_t* edi) {
    char buff[10];

    // 10 f-keys @ 80 col := 6 chars+SPACE+num
    edi_gotoxy(edi, edi->scr.winleft, edi->scr.winbottom);
    for (int i = 0; i < EDI_NUM_COMMANDS; i++) {
        edi_textbackground(BLACK);
        edi_textcolor(WHITE);
        if (i == 0) {
            snprintf(buff, sizeof(buff), "%d", i + 1);
            edi_cputs(edi, buff);
        } else {
            snprintf(buff, sizeof(buff), " %d", i + 1);
            edi_cputs(edi, buff);
        }

        edi_textbackground(BLUE);
        edi_textcolor(LIGHTGREEN);
        snprintf(buff, sizeof(buff), "%-6s", edi_f_keys[i]);
        edi_cputs(edi, buff);
    }
}

/***********************
** exported functions **
***********************/
/**
 * @brief convert the current selection in the edi struct to an ordered description of the selection in a cnp struct.
 *
 * @param edi the edi system to work on.
 * @param cnp an ordered version of the start-stop of the selection.
 */
void edi_get_cnp(edi_t* edi, cnp_t* cnp) {
    if (edi->num > edi->sel_line) {  // selection stretches downwards
        cnp->startX = edi->sel_char;
        cnp->startY = edi->sel_line;
        cnp->endX = edi->x;
        cnp->endY = edi->num;
        cnp->cursor_at_end = true;
    } else if (edi->num < edi->sel_line) {  // selection stretches upwards
        cnp->startX = edi->x;
        cnp->startY = edi->num;
        cnp->endX = edi->sel_char;
        cnp->endY = edi->sel_line;
        cnp->cursor_at_end = false;
    } else {  // all on same line
        cnp->startY = cnp->endY = edi->num;

        if (edi->x > edi->sel_char) {  // selection goes to the right
            cnp->startX = edi->sel_char;
            cnp->endX = edi->x;
            cnp->cursor_at_end = true;
        } else if (edi->x < edi->sel_char) {  // selection goes to the left
            cnp->startX = edi->x;
            cnp->endX = edi->sel_char;
            cnp->cursor_at_end = false;
        } else {  // selection is on itself
            cnp->startX = cnp->endX = edi->x;
            cnp->cursor_at_end = true;
        }
    }
}

/**
 * @brief redraw editor screen.
 *
 * @param edi the edi system to work on.
 */
void edi_redraw(edi_t* edi) {
    int refresh_line = -1;
    int offset = 0;  // x drawing offset
    if (edi->x > edi->width - 1) {
        offset = edi->x - edi->width + 1;
    }

    edi_textbackground(BLACK);
    edi_textcolor(WHITE);
    edi_draw_status(edi);

    edi_textbackground(BLACK);
    edi_textcolor(WHITE);

    // if offsets are changes redraw the whole screen
    if ((edi->last_top != edi->top) || (edi->last_offset != offset) || EDI_IS_SEL(edi)) {
        line_t* l = edi->top;
        int line_num = edi->num - edi->y;
        for (int y = 2; y <= edi->height; y++) {
            edi_draw_line(edi, &l, y, offset, line_num);
            line_num++;
        }

        edi_draw_commands(edi);
    } else {
        // if not, only redraw current line
        line_t* l = edi->current;
        edi_draw_line(edi, &l, edi->y + 2, offset, edi->num);
        refresh_line = edi->y + 2;
    }

    if (edi->msg) {
        dia_show_message(edi, edi->msg);
        edi->msg = NULL;
        edi->last_offset = -1;
        edi_redraw(edi);
    }
    edi->last_top = edi->top;
    edi->last_offset = offset;

    if (refresh_line == -1) {
        edi_refresh_screen(edi);
    } else {
        edi_refresh_line(edi, 1);
        edi_refresh_line(edi, refresh_line);
    }

    // set cursor to current position
    if (offset) {
        gotoxy(edi->width, edi->y + 2);
    } else {
        gotoxy(edi->x + 1, edi->y + 2);  // set cursor to current position
    }
}
