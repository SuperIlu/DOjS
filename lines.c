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

#include <assert.h>
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
#include "lines.h"

/**
 * @brief initialize edi system with a single line.
 *
 * @param name name of the file.
 *
 * @return an empty edi system.
 */
edi_t* lin_init(char* name) {
    edi_t* edi = malloc(sizeof(edi_t));
    if (edi) {
        gettextinfo(&edi->scr);

        line_t* l = lin_newline();

        edi->first = edi->current = edi->top = l;
        edi->x = 0;
        edi->y = 0;
        edi->width = edi->scr.screenwidth;
        edi->height = edi->scr.screenheight - 1;
        edi->name = name;
        edi->changed = false;
        edi->num = 1;
        edi->msg = NULL;
        edi->last_top = NULL;
        edi->last_offset = -1;

        edi->cnp = NULL;
        edi->cnp_pos = 0;
        edi->cnp_size = 0;
        edi_clear_selection(edi);

        edi->open_quote = 0;

#ifdef EDI_FAST
        // alloc screen buffer
        edi->screen = calloc(edi->scr.screenwidth * edi->scr.screenheight, sizeof(uint16_t));
        if (!edi->screen) {
            free(edi);
            return NULL;
        }
#endif

        return edi;
    } else {
        return NULL;
    }
}

/**
 * @brief shutdown edi system
 *
 * @param edi the edi system to free.
 */
void lin_shutdown(edi_t* edi) {
    line_t* l = edi->first;
    while (l) {
        line_t* next = l->next;
        lin_removeline(edi, l);
        l = next;
    }
#ifdef EDI_FAST
    free(edi->screen);
#endif
    free(edi);
}

/**
 * @brief find given line number.
 *
 * @param edi the edi system to work on.
 * @param num wanted line number starting at 0.
 *
 * @return the found line or NULL.
 */
line_t* lin_find(edi_t* edi, int num) {
    line_t* l = edi->first;

    while (num > 0 && l->next) {
        l = l->next;
        num--;
    }
    if (num) {
        return NULL;
    } else {
        return l;
    }
}

/**
 * @brief create new line
 *
 * @return a line ready to use in the system.
 */
line_t* lin_newline() {
    line_t* l = malloc(sizeof(line_t));
    if (l) {
        l->txt = malloc(EDI_LINE_WIDTH);
        if (!l->txt) {
            free(l);
            return NULL;
        } else {
            l->next = l->prev = NULL;
            l->length = 0;
            l->size = EDI_LINE_WIDTH;
            l->newline = false;
            return l;
        }
    } else {
        return NULL;
    }
}

/**
 * @brief free ressources for a line.
 *
 * @param l the line to free.
 */
void lin_freeline(line_t* l) {
    assert(l->txt);
    assert(l->prev == NULL);
    assert(l->next == NULL);

    free(l->txt);  // free text
    free(l);       // free node
}

/**
 * @brief insert a line after an existing
 *
 * @param edi the edi system to work on.
 * @param pred the line to insert after
 * @param l the line to insert.
 */
void lin_insertline(edi_t* edi, line_t* pred, line_t* l) {
    assert(edi);
    assert(l);
    assert(pred);

    line_t* succ = pred->next;

    // set fields of new line
    l->prev = pred;
    l->next = succ;

    pred->next = l;  // fix predecessor
    if (succ) {
        succ->prev = l;  // fix successor
    }
    edi->last_top = NULL;
    edi->changed = true;
    pred->newline = true;
}

/**
 * @brief remove line.
 *
 * @param edi the edi system to work on.
 * @param l the line to remove (MUST be part of the edi system).
 */
void lin_removeline(edi_t* edi, line_t* l) {
    assert(edi);
    assert(l);

    line_t* pred = l->prev;
    line_t* succ = l->next;

    if (!succ) {
        // deleting last line --> just truncate
        l->length = 0;
        l->newline = false;
        edi->x = 0;
    } else {
        if (pred) {
            pred->next = succ;
        }
        if (succ) {
            succ->prev = pred;
            if (l == edi->current) {
                edi->current = succ;
            }
        }

        // fix up first line in list and on screen
        if (l == edi->first) {
            edi->first = edi->current;
        }
        if (l == edi->top) {
            edi->top = edi->current;
        }

        l->next = l->prev = NULL;
        lin_freeline(l);
    }
    edi->changed = true;
    edi->last_top = NULL;
}

/**
 * @brief append a char to a line. If there is no space left in the line it is expanded.
 *
 * @param edi the edi system to work on.
 * @param l the line where the char shall be appended.
 * @param ch the char to append.
 */
void lin_appendch(edi_t* edi, line_t* l, char ch) {
    assert(l);
    assert(l->txt);
    if (l->length + 1 > l->size) {
        unsigned int new_size = (unsigned int)(l->size * EDI_INC_FACTOR);
        char* new = realloc(l->txt, new_size);
        if (!new) {
            return;
        }
        l->txt = new;
        l->size = new_size;
    }

    l->txt[l->length] = ch;
    l->length++;
    edi->changed = true;
    assert(l->length <= l->size);
}

/**
 * @brief insert a char into a line. If there is no space left in the line it is expanded.
 *
 * @param edi the edi system to work on.
 * @param l the line where the char shall be inserted.
 * @param x the position where the char shall be inserted. Characters after this position will be shifted right.
 * @param ch the char to append.
 */
void lin_insertch(edi_t* edi, line_t* l, unsigned int x, char ch) {
    assert(l);
    if (x == l->length) {
        lin_appendch(edi, l, ch);
    } else {
        if (l->length + 1 > l->size) {
            unsigned int new_size = (unsigned int)(l->size * EDI_INC_FACTOR);
            EDIF("l->length + 1 > l->size = %d > %d, new=%d\n", l->length + 1, l->size, new_size);

            char* new = realloc(l->txt, new_size);
            if (!new) {
                return;
            }
            l->txt = new;
            l->size = new_size;
        }

        memmove(l->txt + x + 1, l->txt + x, l->length - x);
        l->txt[x] = ch;
        l->length++;
        edi->changed = true;
    }
}

/**
 * @brief join the current line with the previous (must exist). Used for BACKSPACE.
 *
 * @param edi the edi system to work on.
 * @param l the line to join with its predecessor.
 */
void lin_joinprev(edi_t* edi, line_t* l) {
    line_t* pred = l->prev;
    assert(pred);

    for (int i = 0; i < l->length; i++) {
        lin_appendch(edi, pred, l->txt[i]);
    }
    lin_removeline(edi, l);
}

/**
 * @brief join the current line with the next (must exists). Used for DELETE.
 *
 * @param edi the edi system to work on.
 * @param l the line to join with its successor.
 */
void lin_joinnext(edi_t* edi, line_t* l) {
    line_t* succ = l->next;
    assert(succ);

    for (int i = 0; i < succ->length; i++) {
        lin_appendch(edi, l, succ->txt[i]);
    }
    lin_removeline(edi, succ);
}

/**
 * @brief split the current line into two. Used for NEWLINE.
 *
 * @param edi the edi system to work on.
 * @param l the line to split.
 * @param x split position.
 */
void lin_splitline(edi_t* edi, line_t* l, unsigned int x) {
    line_t* new = lin_newline();
    if (new) {
        new->newline = true;
        lin_insertline(edi, l, new);
        for (int i = x; i < l->length; i++) {
            lin_appendch(edi, new, l->txt[i]);
        }
        l->length = x;
    }
}

/**
 * @brief delete character to the right.
 *
 * @param edi the edi system to work on.
 * @param l current line.
 * @param x cursor x position.
 */
void lin_delch_right(edi_t* edi, line_t* l, unsigned int x) {
    assert(l->length);
    assert(x < l->length);

    memmove(l->txt + x, l->txt + x + 1, l->length - x);
    l->length--;
    edi->changed = true;
}

/**
 * @brief delete character to the left.
 *
 * @param edi the edi system to work on.
 * @param l current line.
 * @param x cursor x position.
 */
void lin_delch_left(edi_t* edi, line_t* l, unsigned int x) {
    assert(l->length);
    assert(x > 0);

    memmove(l->txt + x - 1, l->txt + x, l->length - x);
    l->length--;
    edi->changed = true;
}
