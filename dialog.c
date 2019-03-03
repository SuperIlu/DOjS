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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <conio.h>
#include <keys.h>
#include <pc.h>

#include "DOjS.h"
#include "dialog.h"
#include "edit.h"

/************
** defines **
************/
#define DIA_CONFIRM "<[y] or [n]>"        //!< text for confirm dialog
#define DIA_MESSAGE "<Press any key>"     //!< text for message dialog
#define DIA_TEXT "<UP/DOWN, ESC or DEL>"  //!< text for file dialog

/***********
** macros **
***********/
//! find start of next line in text buffer
#define DIA_LINE_DOWN()                            \
    {                                              \
        while (txt[start] && txt[start] != '\n') { \
            start++;                               \
        }                                          \
        if (txt[start]) {                          \
            start++;                               \
        }                                          \
    }

//! find start of previous line in text buffer
#define DIA_LINE_UP()                         \
    {                                         \
        if (start) {                          \
            start--;                          \
        }                                     \
        while (start && txt[start] != '\n') { \
            start--;                          \
        }                                     \
        if (start) {                          \
            start--;                          \
        }                                     \
        while (start && txt[start] != '\n') { \
            start--;                          \
        }                                     \
        if (txt[start] == '\n') {             \
            start++;                          \
        }                                     \
    }

/***********************
** exported functions **
***********************/
/**
 * @brief let the user enter a text.
 *
 * @param edi current editor.
 * @param buffer the buffer to store the text in.
 * @param allowed a string with allowed characters or NULL to allow everything.
 * @param msg message to show on lower border of dialog.
 *
 * @return true if the user used ENTER to exit the dialog.
 * @return false if the user used ESC to exit the dialog.
 */
bool dia_ask_text(edi_t* edi, char buffer[DIA_ASK_SIZE], char* allowed, char* msg) {
    int w = edi->scr.screenwidth - 4;
    int h = 3;

    int startX = (edi->scr.screenwidth - w) / 2;
    int startY = (edi->scr.screenheight - h) / 2;
    while (true) {
        int endpos = 0;

        // upper border
        textbackground(BLUE);
        textcolor(WHITE);
        gotoxy(startX, startY);
        putch(EDI_UL_LINE);
        for (int x = 1; x < w + 1; x++) {
            putch(EDI_H_LINE);
        }
        putch(EDI_UR_LINE);

        gotoxy(startX, startY + 1);
        putch(EDI_V_LINE);
        putch(' ');

        textbackground(BLACK);
        textcolor(WHITE);
        int pos = 0;
        while (pos < w - 2) {
            if (buffer[endpos]) {
                putch(buffer[endpos]);
                endpos++;
            } else {
                putch(' ');
            }
            pos++;
        }

        textbackground(BLUE);
        textcolor(WHITE);
        putch(' ');
        putch(EDI_V_LINE);

        // lower border
        gotoxy(startX, startY + 2);
        putch(EDI_LL_LINE);
        int x = 1;
        while (x < (w - strlen(msg)) / 2) {
            putch(EDI_H_LINE);
            x++;
        }
        cputs(msg);
        x += strlen(msg);
        while (x < w + 1) {
            putch(EDI_H_LINE);
            x++;
        }
        putch(EDI_LR_LINE);

        gotoxy(strlen(buffer) + startX + 2, startY + 1);
        int ch = getxkey();
        if (ch == K_BackSpace) {
            if (endpos) {
                endpos--;
                buffer[endpos] = 0;
            }
        } else if (ch == K_Escape) {
            return false;
        } else if (ch == K_Return) {
            return true;
        } else {
            bool isin = false;
            if (allowed) {
                for (int i = 0; i < strlen(allowed); i++) {
                    if (ch == allowed[i]) {
                        isin = true;
                        break;
                    }
                }
            } else {
                isin = true;
            }
            if (isin) {
                if (endpos < w) {
                    buffer[endpos] = ch;
                    endpos++;
                    buffer[endpos] = 0;
                }
            }
        }
    }
}

/**
 * @brief show a single line message on screen. returns after any keypress.
 *
 * @param edi current editor.
 * @param txt the message to display.
 */
void dia_show_message(edi_t* edi, char* txt) {
    int w = strlen(txt) + 4;
    int h = 3;

    if (w < strlen(DIA_MESSAGE) + 4) {
        w = strlen(DIA_MESSAGE) + 4;
    }

    int startX = (edi->scr.screenwidth - w) / 2;
    int startY = (edi->scr.screenheight - h) / 2;

    // upper border
    textbackground(YELLOW);
    textcolor(BLUE);
    gotoxy(startX, startY);
    putch(EDI_UL_LINE);
    for (int x = 1; x < w - 1; x++) {
        putch(EDI_H_LINE);
    }
    putch(EDI_UR_LINE);

    gotoxy(startX, startY + 1);
    putch(EDI_V_LINE);
    putch(' ');
    cputs(txt);
    putch(' ');
    putch(EDI_V_LINE);

    // lower border
    gotoxy(startX, startY + 2);
    putch(EDI_LL_LINE);
    int x = 1;
    while (x < (w - strlen(DIA_MESSAGE)) / 2) {
        putch(EDI_H_LINE);
        x++;
    }
    cputs(DIA_MESSAGE);
    x += strlen(DIA_MESSAGE);
    while (x < w - 1) {
        putch(EDI_H_LINE);
        x++;
    }
    putch(EDI_LR_LINE);
    getxkey();
}

/**
 * @brief show a single line yes/no question on screen. returns after Y/N keypress.
 *
 * @param edi current editor.
 * @param txt the question to display.
 */
bool dia_show_confirm(edi_t* edi, char* txt) {
    int w = strlen(txt) + 4;
    int h = 3;

    int startX = (edi->scr.screenwidth - w) / 2;
    int startY = (edi->scr.screenheight - h) / 2;

    // upper border
    textbackground(BLUE);
    textcolor(WHITE);
    gotoxy(startX, startY);
    putch(EDI_UL_LINE);
    for (int x = 1; x < w - 1; x++) {
        putch(EDI_H_LINE);
    }
    putch(EDI_UR_LINE);

    gotoxy(startX, startY + 1);
    putch(EDI_V_LINE);
    putch(' ');
    cputs(txt);
    putch(' ');
    putch(EDI_V_LINE);

    // lower border
    gotoxy(startX, startY + 2);
    putch(EDI_LL_LINE);
    int x = 1;
    while (x < (w - strlen(DIA_CONFIRM)) / 2) {
        putch(EDI_H_LINE);
        x++;
    }
    cputs(DIA_CONFIRM);
    x += strlen(DIA_CONFIRM);
    while (x < w - 1) {
        putch(EDI_H_LINE);
        x++;
    }
    putch(EDI_LR_LINE);
    while (true) {
        int ch = getxkey();
        if (ch == 'y' || ch == 'Y') {
            return true;
        } else if (ch == 'n' || ch == 'N' || ch == K_Escape) {
            return false;
        }
    }
}

/**
 * @brief load a text file and display it on screen.
 *
 * @param edi current editor.
 * @param fname name of the file.
 * @param pos (last) shown position.
 * @param deletable TRUE if the DEL-key shall delete the file after displaying.
 */
void dia_show_file(edi_t* edi, char* fname, int* pos, bool deletable) {
    char buff[1024];
    FILE* f;
    char* s;
    int n, t;

    f = fopen(fname, "rb");
    if (!f) {
        snprintf(buff, sizeof(buff), "cannot open file '%s': %s", fname, strerror(errno));
        dia_show_message(edi, buff);
        return;
    }

    if (fseek(f, 0, SEEK_END) < 0) {
        fclose(f);
        snprintf(buff, sizeof(buff), "cannot seek in file '%s': %s", fname, strerror(errno));
        dia_show_message(edi, buff);
        return;
    }

    n = ftell(f);
    if (n < 0) {
        fclose(f);
        snprintf(buff, sizeof(buff), "cannot tell in file '%s': %s", fname, strerror(errno));
        dia_show_message(edi, buff);
        return;
    }

    if (fseek(f, 0, SEEK_SET) < 0) {
        fclose(f);
        snprintf(buff, sizeof(buff), "cannot seek in file '%s': %s", fname, strerror(errno));
        dia_show_message(edi, buff);
        return;
    }

    s = malloc(n + 1);
    if (!s) {
        fclose(f);
        dia_show_message(edi, "out of memory");
        return;
    }

    t = fread(s, 1, n, f);
    if (t != n) {
        free(s);
        fclose(f);
        snprintf(buff, sizeof(buff), "cannot read data from file '%s': %s", fname, strerror(errno));
        dia_show_message(edi, buff);
        return;
    }
    s[n] = 0;
    fclose(f);

    // check if pos is in range for file
    if (pos && *pos > n) {
        *pos = 0;
    }

    bool del = dia_show_text(edi, s, pos);
    free(s);
    if (deletable && del) {
        f = fopen(fname, "w");
        fflush(f);
        fclose(f);
    }
}

/**
 * @brief show multi-line text on screen.
 *
 * @param edi current editor.
 * @param txt pointer to null terminated text.
 * @param pos (last) shown position.
 *
 * @return true the user exited via DEL key.
 * @return false the user exited via any other key.
 */
bool dia_show_text(edi_t* edi, char* txt, int* pos) {
    int start = 0;
    int lastStart = -1;
    bool end = false;

    if (pos && *pos < strlen(txt)) {
        start = *pos;
    }

    while (true) {
        int color = WHITE;
        int w = edi->scr.screenwidth - 2;
        int h = edi->height;
        if (start != lastStart) {
            int startX = 2;
            int startY = 2;

            // upper border
            textbackground(CYAN);
            textcolor(WHITE);
            gotoxy(startX, startY);
            putch(EDI_UL_LINE);
            for (int x = 1; x < w - 2; x++) {
                putch(EDI_H_LINE);
            }
            if (start == 0) {
                putch(EDI_H_LINE);
            } else {
                putch('^');
            }
            putch(EDI_UR_LINE);

            int idx = start;
            int y = startY + 1;
            if (txt[idx] == '#' && txt[idx + 1] == '#') {  // this is possible because idx+1 is always either a character or the terminating '\0'
                color = RED;
            } else if (txt[idx] == '#') {
                color = YELLOW;
            }
            while (y < h) {
                textcolor(WHITE);
                gotoxy(startX, y);
                putch(EDI_V_LINE);
                putch(' ');
                textcolor(color);
                while (true) {  // line loop
                    if (!txt[idx]) {
                        end = true;
                        break;
                    } else if (txt[idx] == '\r') {
                        // ignored
                        idx++;
                    } else if (txt[idx] == '\n') {
                        // newline
                        color = WHITE;
                        idx++;
                        // this is possible because idx+1 is always either a character or the terminating '\0'
                        if (txt[idx] == '#' && txt[idx + 1] == '#' && txt[idx + 2] == '#') {
                            color = MAGENTA;
                        } else if (txt[idx] == '#' && txt[idx + 1] == '#') {
                            color = RED;
                        } else if (txt[idx] == '#') {
                            color = YELLOW;
                        }
                        break;
                    } else if (!iscntrl(txt[idx])) {
                        // print char
                        end = false;
                        putch(txt[idx]);
                        idx++;
                    } else {
                        idx++;
                    }
                    if (wherex() > w - 2) {  // line wrap
                        break;
                    }
                }

                while (wherex() <= w) {  // fill line with spaces and append |
                    putch(' ');
                }
                textcolor(WHITE);
                putch(EDI_V_LINE);
                y++;
            }

            // lower border
            gotoxy(startX, y);
            putch(EDI_LL_LINE);
            int x = 1;
            while (x < (w - strlen(DIA_TEXT)) / 2) {
                putch(EDI_H_LINE);
                x++;
            }
            cputs(DIA_TEXT);
            x += strlen(DIA_TEXT);
            while (x < w - 2) {
                putch(EDI_H_LINE);
                x++;
            }
            if (end) {
                putch(EDI_H_LINE);
            } else {
                putch('v');
            }
            putch(EDI_LR_LINE);
            gotoxy(1, 1);

            lastStart = start;
        }

        int ch = getxkey();
        if (ch == K_Up || ch == K_EUp) {
            DIA_LINE_UP();
        } else if (!end && (ch == K_Down || ch == K_EDown)) {
            DIA_LINE_DOWN();
        } else if (ch == K_PageUp || ch == K_EPageUp) {
            int num = h - 4;
            while (num > 0) {
                DIA_LINE_UP();
                num--;
            }
        } else if (!end && (ch == K_PageDown || ch == K_EPageDown)) {
            int num = h - 4;
            while (num > 0) {
                DIA_LINE_DOWN();
                num--;
            }
        } else if (ch == K_Home || ch == K_EHome) {
            start = 0;
        } else if (ch == K_Escape || ch == K_F9 || ch == K_F1) {
            return false;
        } else if (ch == K_Delete || ch == K_EDelete) {
            return true;
        }
        if (pos) {
            *pos = start;
        }
    }
}
