#include "conio.h"
#include "zipfile.h"
#include "glue.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define CIO_NUM_LINES 25
#define CIO_NUM_COLS 80

#define CIO_NUM_COLORS 15

#define CIO_FONT_FILE "jsboot/fonts/term16.fnt"
#define CIO_FONT_ZIP "JSBOOT.ZIP=jsboot/fonts/term16.fnt"

#define CIO_TO_COLCHAR(c, fg, bg) ((fg << 12) | (bg << 8) | (c))

#define CIO_CHAR(x) (x & 0xFF)
#define CIO_FG(x) (cio_colors[(x >> 12) & 0x0F])
#define CIO_BG(x) (cio_colors[(x >> 8) & 0x0F])

#define CIO_KUPPER(k) ((k >> 8) & 0xFF)
#define CIO_KLOWER(k) (k & 0xFF)

#ifdef CIO_DEBUG
#define CIOF(str, ...)                      \
    {                                       \
        FILE *f = fopen("CIOLOG.TXT", "a"); \
        if (f) {                            \
            fprintf(f, str, ##__VA_ARGS__); \
            fflush(f);                      \
            fclose(f);                      \
        }                                   \
    }
#else
#define CIOF(str, ...)
#endif

int _wscroll;

static unsigned int cio_x = 0;  //!< current x-pos, 0-based
static unsigned int cio_y = 0;  //!< current y-pos, 0-based

static uint16_t cio_bg;  //!< current background color
static uint16_t cio_fg;  //!< current foreground color

static int cio_fwidth;
static int cio_fheight;

static FONT *cio_font = NULL;                             //!< text font
static bool cio_window = false;                           //!< true indicates that allegro was initialized
static uint16_t cio_screen[CIO_NUM_LINES][CIO_NUM_COLS];  //!< screen buffer, upper 8 bits are the fg/bg colors, lower 8 bits the character

static uint32_t cio_colors[CIO_NUM_COLORS];

static uint32_t cio_cursor;

/**
 * @brief Sets just the background of the text attribute. See section textattr.
 */
void textbackground(int _color) {
    // CIOF("%s:%d %s(%d)\n", __FILE__, __LINE__, __FUNCTION__, _color);

    cio_bg = _color;
}

/**
 * @brief Sets just the foreground of the text attribute. See section textattr.
 */
void textcolor(int _color) {
    // CIOF("%s:%d %s(%d)\n", __FILE__, __LINE__, __FUNCTION__, _color);

    cio_fg = _color;
}

/**
 * @brief Put the character _c on the screen at the current cursor position. The
 * special characters return, linefeed, bell, and backspace are handled
 * properly, as is line wrap and scrolling. The cursor position is updated.
 */
int putch(int _c) {
    // CIOF("%s:%d %s(%d)\n", __FILE__, __LINE__, __FUNCTION__, _c);

    cio_screen[cio_y][cio_x] = CIO_TO_COLCHAR(_c, cio_fg, cio_bg);

    gotoxy(wherex() + 1, wherey());
    // CIOF("%s:%d %s(%d)\n", __FILE__, __LINE__, __FUNCTION__, _c);
    return _c;
}

/**
 * @brief Puts the string onto the console. The cursor position is updated.
 */
int cputs(const char *_str) {
    // CIOF("%s:%d %s(%s)\n", __FILE__, __LINE__, __FUNCTION__, _str);

    while (*_str) {
        putch(*_str);
        _str++;
    }
    // CIOF("%s:%d %s(%s)\n", __FILE__, __LINE__, __FUNCTION__, _str);
    return 0;
}

/**
 * @brief Move the cursor to row y, column x. The upper left corner of the
 * current window is (1,1).
 */
void gotoxy(int x, int y) {
    // CIOF("%s:%d %s(%d,%d)\n", __FILE__, __LINE__, __FUNCTION__, x, y);

    cio_x = x - 1;
    if (cio_x >= CIO_NUM_COLS) {
        cio_x = CIO_NUM_COLS - 1;
    }
    cio_y = y - 1;
    if (cio_y >= CIO_NUM_LINES) {
        cio_y = CIO_NUM_LINES - 1;
    }
    // CIOF("%s:%d %s(%d,%d)\n", __FILE__, __LINE__, __FUNCTION__, x, y);
}

/**
 * @brief Waits for the user to press one key, then returns that key. Alt-key
 * combinations have 0x100 added to them, and extended keys have 0x200 added to
 * them.
 */
int getxkey(void) {
    int key;
    char str[2] = {0x00, 0x00};

    // redraw screen
    // CIOF("%s:%d %s()\n", __FILE__, __LINE__, __FUNCTION__);
    clear_to_color(screen, cio_colors[BLACK]);
    for (int y = 0; y < CIO_NUM_LINES; y++) {
        int yPos = y * cio_fheight;
        for (int x = 0; x < CIO_NUM_COLS; x++) {
            int xPos = x * cio_fwidth;

            str[0] = CIO_CHAR(cio_screen[y][x]);

            rectfill(screen, xPos, yPos, xPos + cio_fwidth - 1, yPos + cio_fheight - 1, CIO_BG(cio_screen[y][x]));
            textout_ex(screen, cio_font, str, xPos, yPos, CIO_FG(cio_screen[y][x]), -1);
        }
    }

    int yPos = cio_y * cio_fheight + cio_fheight - 1;
    int xPos = cio_x * cio_fwidth;

    line(screen, xPos, yPos, xPos + cio_fwidth, yPos, cio_cursor);
    line(screen, xPos, yPos, xPos, yPos - 2, cio_cursor);
    line(screen, xPos + cio_fwidth, yPos, xPos + cio_fwidth, yPos - 2, cio_cursor);

    // wait for keypress with blinking cursor TODO:
    // CIOF("%s:%d %s()\n", __FILE__, __LINE__, __FUNCTION__);
    while (true) {
        poll_keyboard();

        if (keypressed()) {
            key = readkey();
            break;
        }

        usleep(100);
    }

    CIOF("%s:%d %s():=0x%04x\n", __FILE__, __LINE__, __FUNCTION__, key);

    switch (key) {
        case K_F1:
        case K_F2:
        case K_F3:
        case K_F4:
        case K_F5:
        case K_F6:
        case K_F7:
        case K_F8:
        case K_F9:
        case K_F10:
        case K_F11:
        case K_F12:
        case K_Tab:
        case 0x4000:  // SHIFT-TAB
            if (key_shifts & KB_SHIFT_FLAG) {
                key = CIO_SHIFT(key);
            }
            break;
        default:
            // return conio-key
            if (key_shifts & KB_CTRL_FLAG) {
                key = CIO_CTRL(key);
                if (CIO_KUPPER(key) == CIO_KLOWER(key)) {
                    // this is a CTRL-<char> key, clear bits 8-15
                    key &= 0xFF00FF;
                }
            } else {
                // if lower != 0 this should be a character, clear upper part
                if (CIO_KLOWER(key)) {
                    key = CIO_KLOWER(key);
                }
            }
            break;
    }

    // CIOF("%s:%d %s():=0x%04x\n", __FILE__, __LINE__, __FUNCTION__, key);

    // CIOF("ESC    = 0x%04x\n", K_Escape);
    // CIOF("F1     = 0x%04x\n", K_F1);
    // CIOF("UP     = 0x%04x\n", K_Up);
    // CIOF("DOWN   = 0x%04x\n", K_Down);
    // CIOF("TAB    = 0x%04x\n", K_Tab);
    // CIOF("DEL    = 0x%04x\n", K_Delete);
    // CIOF("BS     = 0x%04x\n", K_BackSpace);
    // CIOF("CTRL_C = 0x%04x\n", K_Control_C);

    return key;
}

/**
 * @brief The column the cursor is on. The leftmost column is 1.
 */
int wherex(void) {
    // CIOF("%s:%d %s()\n", __FILE__, __LINE__, __FUNCTION__);
    return cio_x + 1;
}

/**
 * @brief The row the cursor is on. The topmost row is 1.
 */
int wherey(void) {
    // CIOF("%s:%d %s()\n", __FILE__, __LINE__, __FUNCTION__);
    return cio_y + 1;
}

/**
 * @brief Sets the text mode of the screen.
 */
void textmode(int _mode) {
    CIOF("%s:%d %s(%d)\n", __FILE__, __LINE__, __FUNCTION__, _mode);
    if (!cio_window) {
        allegro_init();
        install_keyboard();

        cio_font = load_font(CIO_FONT_FILE, NULL, NULL);
        if (!cio_font) {
            PACKFILE *pf = open_zipfile1(CIO_FONT_ZIP);
            if (!pf) {
                CIOF("Could not load editor font (1)!\n");
                exit(1);
            }

            cio_font = load_grx_font_pf(pf, NULL, NULL);  // PACKFILE is closed by this function!
            if (!cio_font) {
                CIOF("Could not load editor font (3)!\n");
                exit(1);
            }
        }

        cio_fheight = cio_font->height;
        cio_fwidth = text_length(cio_font, "0");
        CIOF("FontHeight=%d\n", cio_fwidth);
        CIOF("FontWidth=%d\n", cio_fwidth);

        int wwidth = CIO_NUM_COLS * cio_fwidth;
        int wheight = CIO_NUM_LINES * cio_fheight;

        CIOF("WindowHeight=%d\n", wwidth);
        CIOF("WindowWidth=%d\n", wheight);

        set_color_depth(32);
        if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, wwidth, wheight, 0, 0) != 0) {
            CIOF("Could not open window!\n");
            exit(1);
        }

        // make colors
        cio_colors[BLACK] = makeacol32(0, 0, 0, 255);
        cio_colors[WHITE] = makeacol32(255, 255, 255, 255);
        cio_colors[RED] = makeacol32(170, 0, 0, 255);
        cio_colors[GREEN] = makeacol32(0, 170, 0, 255);
        cio_colors[BLUE] = makeacol32(0, 0, 170, 255);
        cio_colors[CYAN] = makeacol32(0, 170, 170, 255);
        cio_colors[MAGENTA] = makeacol32(170, 0, 170, 255);
        cio_colors[BROWN] = makeacol32(170, 85, 0, 255);
        cio_colors[LIGHTGRAY] = makeacol32(170, 170, 170, 255);
        cio_colors[LIGHTBLUE] = makeacol32(85, 85, 255, 255);
        cio_colors[LIGHTGREEN] = makeacol32(85, 255, 85, 255);
        cio_colors[LIGHTCYAN] = makeacol32(85, 255, 255, 255);
        cio_colors[LIGHTRED] = makeacol32(255, 85, 85, 255);
        cio_colors[LIGHTMAGENTA] = makeacol32(255, 85, 255, 255);
        cio_colors[YELLOW] = makeacol32(255, 255, 85, 255);

        cio_cursor = makeacol32(15, 255, 15, 255);

        cio_window = true;
    }
}

/**
 * @brief Clear to end of line.
 */
void clreol(void) {
    // CIOF("%s:%d %s() on line %d from %d to %d\n", __FILE__, __LINE__, __FUNCTION__, cio_y, cio_x, CIO_NUM_COLS - 1);
    for (int x = cio_x; x < CIO_NUM_COLS; x++) {
        cio_screen[cio_y][x] = CIO_TO_COLCHAR(' ', cio_fg, cio_bg);
    }
    // CIOF("%s:%d %s()\n", __FILE__, __LINE__, __FUNCTION__);
}

/**
 * @brief This function issues the BIOS keyboard interrupt 16h with command in
 * the AH register, and returns the results of that call.
 *
 * Note: only used to detect the state of the SHIFT key
 */
int bioskey(int command) {
    // CIOF("%s:%d %s(%d)\n", __FILE__, __LINE__, __FUNCTION__, command);
    poll_keyboard();
    if (key_shifts & KB_SHIFT_FLAG) {
        return 0x03;
    } else {
        return 0x00;
    }
}

/**
 * @brief Resets the text attribute to what it was before the program started.
 */
void normvideo(void) {
    CIOF("%s:%d %s()\n", __FILE__, __LINE__, __FUNCTION__);
    if (cio_window) {
        if (cio_font) {
            destroy_font(cio_font);
            cio_font = NULL;
        }

        allegro_exit();
        cio_window = false;
    }
    CIOF("%s:%d %s()\n", __FILE__, __LINE__, __FUNCTION__);
}

/**
 * @brief Sets the cursor type. _type is one of the following:
 */
void _setcursortype(int _type) {
    CIOF("%s:%d %s(%d)\n", __FILE__, __LINE__, __FUNCTION__, _type);
    // NO-OP
}

/**
 * @brief Clear the entire screen.
 */
void clrscr(void) {
    // CIOF("%s:%d %s()\n", __FILE__, __LINE__, __FUNCTION__);
    for (int y = 0; y < CIO_NUM_LINES; y++) {
        for (int x = 0; x < CIO_NUM_COLS; x++) {
            cio_screen[y][x] = CIO_TO_COLCHAR(' ', cio_fg, cio_bg);
        }
    }
    // CIOF("%s:%d %s()\n", __FILE__, __LINE__, __FUNCTION__);
}

/**
 * @brief This function returns the parameters of the current window on the
 * screen.
 */
void gettextinfo(struct text_info *_r) {
    // CIOF("%s:%d %s()\n", __FILE__, __LINE__, __FUNCTION__);
    _r->winleft = 1;
    _r->wintop = 1;
    _r->winright = 80;
    _r->winbottom = 25;
    _r->screenheight = 25;
    _r->screenwidth = 80;
    // CIOF("%s:%d %s()\n", __FILE__, __LINE__, __FUNCTION__);
}

#ifdef EDI_FAST
void ScreenUpdate(void *buf) {}
void ScreenUpdateLine(void *buf, int row) {}
#endif
