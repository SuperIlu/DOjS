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

#ifndef __EDI_H__
#define __EDI_H__

#include <conio.h>
#include <stdbool.h>
#include <sys/types.h>

/************
** defines **
************/
#define EDI_LINE_WIDTH 80         //!< default storage size of a line
#define EDI_TABSIZE 4             //!< our tabsize
#define EDI_INC_FACTOR 1.4f       //!< factor to increase memory for a line if full
#define EDI_MAX_LINE_LENGTH 4096  //!< max size when reading a line from disk

#define EDI_UL_LINE 218  //!< GFX upper left corner
#define EDI_LL_LINE 192  //!< GFX lower left corner
#define EDI_UR_LINE 191  //!< GFX upper right corner
#define EDI_LR_LINE 217  //!< GFX upper right corner
#define EDI_H_LINE 196   //!< horizontal line
#define EDI_V_LINE 179   //!< vertical line

/***********
** macros **
***********/
#ifdef DEBUG_ENABLED
#define EDIF(str, ...)                      \
    {                                       \
        FILE *f = fopen("EDILOG.TXT", "a"); \
        if (f) {                            \
            fprintf(f, str, ##__VA_ARGS__); \
            fflush(f);                      \
            fclose(f);                      \
        }                                   \
    }
#else
#define EDIF(str, ...)
#endif  // DEBUG_ENABLED

#define EDI_SHIFT_DOWN(s) (s & 0x03)
#define EDI_CTRL_DOWN(s) (s & 0x04)
#define EDI_ALT_DOWN(s) (s & 0x08)

#define EDI_IS_SEL(e) (edi->sel_line != -1)

//! add a keyord to array
#define EDI_SYNTAX(c, w) \
    { c, sizeof(w) - 1, w }

#define EDI_SYNTAX_EOL(c, w) \
    { c, -1, w }

//! end array
#define EDI_SYNTAX_END \
    { 0, 0, NULL }

/*************
** typedefs **
*************/
//! enum for editor return value
typedef enum { EDI_KEEPRUNNING, EDI_QUIT, EDI_RUNSCRIPT, EDI_ERROR } edi_exit_t;

//! typedef for struct text_info
typedef struct text_info screen_t;

/************
** structs **
************/
//! describes a single line in the editor
typedef struct _line {
    struct _line *next;   //!< next line or NULL
    struct _line *prev;   //!< previous line or NULL
    char *txt;            //!< text of this line, this is NOT NULL terminated!
    bool newline;         //!< does this line end with a newline?
    unsigned int size;    //!< max size of txt
    unsigned int length;  //!< current usage of txt
} line_t;

//! describes an active editor
typedef struct _edi {
    line_t *first;         //!< first line
    line_t *current;       //!< current line (cursor on)
    line_t *top;           //!< first line on screen
    int width;             //!< width of editor window
    int height;            //!< height of editor window
    int x;                 //!< cursor x pos on line (starting with 0)
    int y;                 //!< cursor y pos on screen (starting with 0)
    int num;               //!< current line number
    char *name;            //!< name of the file
    bool changed;          //!< file change indicator
    struct text_info scr;  //!< screen dimensions
    char *msg;             //!< message to display in next loop
    line_t *last_top;      //!< last y drawing position
    int last_offset;       //!< last x drawing position

    int sel_line;  //!< start of selection (line)
    int sel_char;  //!< start of selection (char)
    char *cnp;     //!< current copy buffer
    int cnp_pos;   //!< current usage of buffer
    int cnp_size;  //!< max size of buffer

#ifdef EDI_FAST
    unsigned short *screen;
    int scr_idx;
    int scr_x;
#endif

    char open_quote;  // indicates if we are in a string and which quote was used
} edi_t;

//! syntax highlight entry
typedef struct _syntax {
    int color;   //!< color to use
    int length;  //!< length of the keyword
    char *word;  //!< keyword
} syntax_t;

//! copy and paste helper struct where start and end of the selection is ordered from top to bottom
typedef struct _cnp {
    int startX;          //!< x-start of selection
    int startY;          //!< y-start of selection
    int endX;            //!< x-end of selection
    int endY;            //!< y-start of selection
    bool cursor_at_end;  //!< indicates if the cursor is at the end of the selection or the start
} cnp_t;

/***********************
** exported functions **
***********************/
extern edi_exit_t edi_edit(char *fname, bool highres);
extern void edi_clear_selection(edi_t *edi);

#endif  // __EDI_H__
