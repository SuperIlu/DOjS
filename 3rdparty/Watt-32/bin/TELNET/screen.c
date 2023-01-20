#include <stdarg.h>
#include <malloc.h>
#include <assert.h>
#include <dos.h>

#include "telnet.h"
#include "keyb.h"
#include "config.h"
#include "screen.h"

#define ASCII     0         /* ASCII character set */
#define UK        1         /* UK character set */
#define SPECIAL   2         /* Special character set, graphics chars */

static int  G0 = ASCII;     /* Character set G0 */
static int  G1 = ASCII;     /* Character set G1 */
static int *GL = &G0;       /* Pointer to current mapped character set */

static char special_chars[32] = { /* Special characters */
            32,    4, 176,   9,  12,  13,  10, 248, 241,  18,  11,
            217, 191, 218, 192, 197, 196, 196, 196, 196, 196, 195,
            180, 193, 194, 179, 243, 242, 227, 216, 156,   7
          };

static int      attribute;
static int      fore         = LIGHTGRAY;
static int      back         = BLACK;
static unsigned blink        = 0;
static unsigned bold         = 0;
static unsigned scrollTop    = 0;
static unsigned scrollBottom = 0;
static int      scrollAttr   = -1;
static int      wrapMode     = 0;
static int      scr_init     = 0;
static char     tabs[132];   /* active tab stops */
static struct   text_info scr_info;

#if defined(__DJGPP__) && 0
static void patch_gotoxy (void);
#endif

int SCR_GetScrHeight (void)
{
  return (scr_info.winbottom - scr_info.wintop + 1);
}

int SCR_GetScrWidth (void)
{
  return (scr_info.winright - scr_info.winleft + 1);
}

void SCR_ScrollColour (int c)
{
  scrollAttr = c;
}

/*
 * Clear whole screen
 */
void SCR_Clear (void)
{
  textattr (scrollAttr == -1 ? attribute : scrollAttr);
  clrscr();
}

/*
 * Clear partial screen
 */
void SCR_Fill (int x1, int y1, int x2, int y2, int ch, int colour)
{
  int  i;
  int  size = (x2-x1+1) * (y2-y1+1);
  int  fill = (colour << 8) + ch;
  WORD *buf = alloca (2*size);

  for (i = 0; i < size; i++)
      buf[i] = fill;
  puttext (x1, y1, x2, y2, buf);
}

/*
 * Clear from cursor to end-of-line
 */
void SCR_ClearEOL (void)
{
  int attr = scrollAttr == -1 ? attribute : scrollAttr;
  int row  = scr_info.wintop + wherey() - 1;

  SCR_Fill (scr_info.winleft+wherex()-1, row,
            scr_info.winright, row, ' ', attr);
}

/*
 * Clear from start-of-line to cursor
 */
void SCR_ClearBOL (void)
{
  int attr = scrollAttr == -1 ? attribute : scrollAttr;
  int row  = scr_info.wintop + wherey() - 1;

  SCR_Fill (scr_info.winleft, row, scr_info.winleft+wherex()-1,
            row, ' ', attr);
}

/*
 * Clear from screen start to cursor
 */
void SCR_ClearBOD (void)
{
  int attr = scrollAttr == -1 ? attribute : scrollAttr;
  int row  = scr_info.wintop + wherey() - 1;

  SCR_ClearBOL();
  if (wherey() > 1)
     SCR_Fill (scr_info.winleft, scr_info.wintop, scr_info.winright,
               row, ' ', attr);
}

/*
 * Clear from cursor to end of screen
 */
void SCR_ClearEOD (void)
{
  int attr = scrollAttr == -1 ? attribute : scrollAttr;
  int row  = scr_info.wintop + wherey() - 1;

  SCR_ClearEOL();
  if (wherey() < scr_info.winbottom-scr_info.wintop)
     SCR_Fill (scr_info.winleft, row, scr_info.winright,
               scr_info.winbottom, ' ', attr);
}

/*
 * Scroll up area confined by co-ordinates (x1,y1) and (x2,y2)
 */
void SCR_ScrollUp (int x1, int y1, int x2, int y2)
{
  int attr = scrollAttr == -1 ? attribute : scrollAttr;

  movetext (x1, y1+1, x2, y2, x1, y1);
  SCR_Fill (x1, y2, x2, y2, ' ', attr);
}

/*
 * Scroll down area confined by co-ordinates (x1,y1) and (x2,y2)
 */
void SCR_ScrollDown (int x1, int y1, int x2, int y2)
{
  int attr = scrollAttr == -1 ? attribute : scrollAttr;

  movetext (x1, y1, x2, y2-1, x1, y1+1);
  SCR_Fill (x1, y1, x2, y1, ' ', attr);
}

/*
 * Insert 'num' characters at cursor location
 */
void SCR_InsertChar (int num, int chr)
{
  char *buf;
  int  i;
  int  row  = wherey() + scr_info.wintop - 1;    /* absolute row */
  int  col  = wherex() + scr_info.winleft - 1;   /* absoloute column */
  int  attr = scrollAttr == -1 ? attribute : scrollAttr;

  if (num < 1)
      num = 1;
  if (num > scr_info.winright - scr_info.winleft)
      num = scr_info.winright - scr_info.winleft;
  buf = alloca (2*num);

  movetext (col, row, min(1,scr_info.winright-num), row, col+num, row);

  for (i = 0; i < 2*num; i += 2)
  {
    buf[i]   = chr;
    buf[i+1] = attr;
  }
  puttext (col, row, col+num, row, buf);
}

/*
 * Delete 'num' characters at cursor location
 */
void SCR_DeleteChar (int num)
{
  int row  = wherey() + scr_info.wintop - 1;     /* absolute row */
  int col  = wherex() + scr_info.winleft - 1;    /* absoloute column */
  int attr = scrollAttr == -1 ? attribute : scrollAttr;

  if (num < 1)
      num = 1;
  if (num > scr_info.winright - scr_info.winleft)
      num = scr_info.winright - scr_info.winleft;

  movetext (col+num, row, scr_info.winright, row, col, row);
  SCR_Fill (scr_info.winright-num, row, scr_info.winright, row, ' ', attr);
}

/*
 * Move cursor to row/column
 */
void SCR_GotoRowCol (int row, int col)
{
  if (row < 1)
      row = 1;
  if (row > scr_info.winbottom-scr_info.wintop+1)
      row = scr_info.winbottom-scr_info.wintop+1;

  if (col < 1)
      col = 1;
  if (col > scr_info.winright-scr_info.winleft+1)
      col = scr_info.winright-scr_info.winleft+1;

  gotoxy (col, row);
}

/*
 * Define scroll area between top and bottom
 */
void SCR_SetScroll (int top, int bottom)
{
  if (top <= 0)
       top = 1;
  else top = min (1, scr_info.winbottom-1);

  if (bottom <= 0)
       bottom = scr_info.winbottom-scr_info.wintop+1;
  else bottom = min (top, scr_info.winbottom);

  scrollTop    = top;
  scrollBottom = bottom;
  SCR_GotoRowCol (1, 1);
}

void SCR_MoveLeft (int num)
{
  int col = wherex();

  if (col == 1)
     return;

  if (num < 1)
      num = 1;
  SCR_GotoRowCol (wherey(), col-num);
}

void SCR_MoveRight (int num)
{
  int col = wherex();

  if (col == scr_info.winright-scr_info.winleft+1)
     return;

  if (num < 1)
      num = 1;
  SCR_GotoRowCol (wherey(), col+num);
}

void SCR_MoveUp (int num)
{
  int row = wherey();

  if (row == 1)
     return;

  if (num < 1)
      num = 1;
  SCR_GotoRowCol (row-num, wherex());
}

void SCR_MoveUpScroll (void)
{
  unsigned y = wherey();

  if (y > 1 && (y + scr_info.wintop-1) < scrollTop)
       SCR_MoveUp (1);
  else SCR_ScrollDown (scr_info.winleft, scrollTop,
                       scr_info.winright, scrollBottom);
}

void SCR_MoveDown (int num)
{
  unsigned row = wherey();

  if (row == scr_info.winbottom-scr_info.wintop+1)
     return;

  if (num < 1)
      num = 1;
  SCR_GotoRowCol (row+num, wherex());
}

void SCR_MoveDownScroll (void)
{
  unsigned y = wherey();

  if ((y < scr_info.winbottom-scr_info.wintop+1) &&
      (y+scr_info.wintop-1) < scrollBottom)
       SCR_MoveDown (1);
  else SCR_ScrollUp (scr_info.winleft, scrollTop,
                     scr_info.winright, scrollBottom);
}

int SCR_GetRow (void)
{
  return wherey();
}

int SCR_GetColumn (void)
{
  return wherex();
}

void SCR_Wrap (int o)
{
  wrapMode = o ? 1 : 0;
}

void SCR_SetBlink (int o)
{
  blink = o ? BLINK : 0;
  attribute = (bold + blink + fore + (back << 4));
  textattr (attribute);
}

int SCR_GetBlink (void)
{
  return (blink > 0);
}

void SCR_SetBold (int bld)
{
  bold = bld ? 8 : 0;
  attribute = (bold + blink + fore + (back << 4));
  textattr (attribute);
}

void SCR_SetFore (int c)
{
  if (c < 0)
      c = 0;
  else if (c > 7)
  {
    if (c > 15)
        c = 15;
    c -= 8;
  }
  fore = c;
  attribute = (bold + blink + fore + (back << 4));
  textattr (attribute);
}

int SCR_GetFore (void)
{
  return (fore);
}

void SCR_SetBack (int c)
{
  if (c < 0)
      c = 0;
  else if (c > 7)
  {
    if (c > 15)
        c = 15;
    c -= 8;
  }
  back = c;
  attribute = (bold + blink + fore + (back << 4));
  textattr (attribute);
}

int SCR_GetBack (void)
{
  return (back);
}


void SCR_SetColour (int c)
{
  blink = (c & 128);
  bold  = (c & 8);
  fore  = (c & 7);
  back  = (c & 112) >> 4;
  attribute = (bold + blink + fore + (back << 4));
  textattr (attribute);
}

int SCR_GetColour (void)
{
  return (attribute);
}

void SCR_SetInverse (void)
{
  int bck = back;
  back = fore & 7;
  fore = bck;
  attribute = (bold + blink + fore + (back << 4));
  textattr (attribute);
}

void SCR_ColourDefault (void)
{
  blink = 0;
  bold  = 0;
  fore  = cfg.colour.data_fg;
  back  = cfg.colour.data_bg;
  attribute = (fore + (back << 4));
  textattr (attribute);
}

void SCR_PutChar (unsigned char ch)
{
  if (ch == '\n')
  {
    if ((wherey() + scr_info.wintop - 1) == scr_info.winbottom)
    {
      SCR_ScrollUp (scr_info.winleft, scr_info.wintop,
                    scr_info.winright, scr_info.winbottom);
      return;
    }
    if (((unsigned)wherey() + scr_info.wintop - 1) == scrollBottom)
    {
      SCR_ScrollUp (scr_info.winleft, scrollTop,
                    scr_info.winright, scrollBottom);
      return;
    }
  }
  if (*GL == SPECIAL)
  {                           /* if using the special character */
    if (ch > 94 && ch < 128)  /* then translate graphics characters */
        ch = special_chars [ch-95];
  }
  else if (*GL == UK)
  {                           /* If using the UK character set */
    if (ch == '#')            /* then watch for the number sign */
        ch = 'œ';             /* translating it to British pound */
  }
  putch (ch);
}

void SCR_PutString (const char *fmt, ...)
{
  char    buf[512];
  char   *s = buf;
  va_list args;

  va_start (args, fmt);
  VSPRINTF (buf, fmt, args);
  while (*s)
     SCR_PutChar (*s++);
  va_end (args);
}

void SCR_SetTabStop (void)
{
  tabs [wherex()] = 1;   /* Mark current cursor position as tab stop */
}

void SCR_ClearTabStop (void)
{
  tabs [wherex()] = 0;   /* Clear current cursor position tab stop */
}

void SCR_ClearAllTabs (void)
{
  memset (tabs, '\0', sizeof(tabs));
}

void SCR_PrintTab (void)
{
  int x, max_x = scr_info.winright - scr_info.winleft + 1;

  for (x = wherex()+1; x <= max_x; x++) /* Look for next tab stop */
  {
    if (tabs[x])
    {
      SCR_GotoRowCol (wherey(), x);
      break;
    }
  }
}

void SCR_MapCharSet (int charset)
{
  if (charset == 0)             /* If mapping G0 character set */
     GL = &G0;                  /* Point the current char set,GL to G0 */
  else if (charset == 1)        /* If mapping G1 character set */
     GL = &G1;                  /* Point the current char set,GL, to G1 */
}

void SCR_SetCharSet (int gset, int set)
{
  int *charset;

  if (gset == 0)                /* Check to see what character set is */
       charset = &G0;           /* going to be set */
  else if (gset == 1)
       charset = &G1;
  else return;                  /* If not valid set then return */

  switch (set)
  {
    case 'B':                   /* 'B' maps the character set to ASCII */
         *charset = ASCII;      /* this is the normal character set */
         break;
    case 'A':                   /* 'A' maps the character set to UK */
         *charset = UK;         /* only difference between UK and ASCII */
         break;                 /* is the pound sign,  # = œ */
    case '0':                   /* '0' maps the character set to SPECIAL */
         *charset = SPECIAL;    /* this character set is the 'graphics' */
         break;                 /* character set used for line drawing */
    default:;
  }
}

void SCR_NoCursor (void)
{
  union REGS regs;

  regs.h.ah = 0x01;
  regs.h.ch = 0x20;
#ifdef __WATCOMC__
  int386 (0x10, &regs, &regs);
#else
  int86 (0x10, &regs, &regs);
#endif
}

void SCR_BlockCursor (void)
{
  union REGS regs;

  regs.h.ah = 0x01;
  regs.h.ch = 0;
  regs.h.cl = 11;
#ifdef __WATCOMC__
  int386 (0x10, &regs, &regs);
#else
  int86 (0x10, &regs, &regs);
#endif
}

void SCR_LineCursor (void)
{
  union REGS regs;

  regs.h.ah = 0x01;
  regs.h.ch = 10;
  regs.h.cl = 11;
#ifdef __WATCOMC__
  int386 (0x10, &regs, &regs);
#else
  int86 (0x10, &regs, &regs);
#endif
}

/*-----------------------------------------------------------------------*/

void SetColour (enum TextDestination dest)
{
  switch (dest)
  {
    case UserText:
         textattr (cfg.colour.user_fg+16*cfg.colour.user_bg);
         break;
    case DataText:
         textattr (cfg.colour.data_fg+16*cfg.colour.data_bg);
         break;
    case WarnText:
         textattr (cfg.colour.warn_fg+16*cfg.colour.warn_bg);
         break;
  }
}

static int PutChar (char c)
{
  char   buf[3];
  static int last = 0;

  if (scr_init)
     SetColour (WarnText);

  if (c == '\n' && last != '\r')
     strcpy (buf, "\r\n");
  else
  {
    buf[0] = c;
    buf[1] = '\0';
  }
  last = c;
  cputs (buf);
  return (1);
}

static int WattPrintf (const char *fmt, ...)
{
  char    buf [LIN_SIZE];
  int     i, len;
  va_list args;

  va_start (args, fmt);
  len = VSPRINTF (buf, fmt, args);

  if (scr_init)
     SetColour (WarnText);
  for (i = 0; i < len; i++)
      PutChar (buf[i]);
  va_end (args);
  return (len);
}

/*-----------------------------------------------------------------------*/

static int   statusOfs[LastColumn+1] = { 1 };
static char *statusLine    = NULL;
static char  statusTxt[80] = "             ³"\
                             "                          ³"\
                             "                 ³"\
                             "          ³"\
                             "        ";

int SCR_Init (int first)
{
  gettextinfo (&scr_info);
  cfg.scr_width  = scr_info.screenwidth;
  cfg.scr_height = scr_info.screenheight;

  SCR_SetColour (LIGHTGRAY+16*BLACK);

  if (first)
  {
    int i, j, k, x;

    directvideo = 1;
    statusLine = malloc (2*cfg.scr_width);
    if (!statusLine)
       cfg.status_line = 0;
    else for (i = j = 0, k = 1; i < cfg.scr_width; j++, i++)
         {
           statusLine[2*i]   = statusTxt[j];
           statusLine[2*i+1] = cfg.colour.stat_fg + 16*cfg.colour.stat_bg;
           if (statusTxt[j] == '³' && k < LastColumn)
               statusOfs[k++] = j + 2;
         }
    statusOfs [LastColumn] = cfg.scr_width+1;

    if (cfg.status_line >= 1)
       window (1, cfg.status_line+1, cfg.scr_width, cfg.scr_height);

    if (cfg.status_line >= 25)
       window (1, 1, cfg.scr_width, cfg.scr_height-1);

    cfg.colour.data_fg &= 7;
    cfg.colour.data_bg &= 7;
    SCR_ColourDefault();
    clrscr();

    _outch  = PutChar;
    _printf = WattPrintf;
//  int29_init();

    SCR_GotoRowCol (2, 1);
    _setcursortype (_NORMALCURSOR);

    for (x = 0; x < scr_info.screenwidth; x++)
    {
      if (x % 8)
           tabs[x] = 0;
      else tabs[x] = 1;
    }
    scr_init = 1;
  }

  gettextinfo (&scr_info);
  scrollTop    = scr_info.wintop;
  scrollBottom = scr_info.winbottom;

  SCR_StatusLine (ProgramVer, TELNET_VERSION);
  return (1);
}

void SCR_StatusLine (enum StatusColumn column, const char *fmt, ...)
{
  char    buf [100];
  int     len, i, j;
  va_list args;

  if (!cfg.status_line)
     return;

  if (column >= LastColumn)
     return;

  va_start (args, fmt);
  if (!fmt)
       len = 0;
  else len = VSPRINTF (buf, fmt, args);

  i = statusOfs [column];
  for (j = 0; i < statusOfs[column+1]-2; j++,i++)
  {
    if (j < len)
         statusLine[2*i] = buf[j];
    else statusLine[2*i] = ' ';
  }
  puttext (1, cfg.status_line, cfg.scr_width, cfg.status_line, statusLine);
  va_end (args);
}


void SCR_StatusFill (const char *fmt, ...)
{
  char   *buf, *scr;
  int     len, i;
  va_list args;

  va_start (args, fmt);

  if (!cfg.status_line || !fmt)
     return;

  buf = alloca (cfg.scr_width);
  scr = alloca (cfg.scr_width << 1);
  len = vsnprintf (buf, cfg.scr_width-1, fmt, args);

  for (i = 0; i < cfg.scr_width; i++)
  {
    if (i < len)
         scr[2*i] = buf[i];
    else scr[2*i] = ' ';
    scr[2*i+1] = cfg.colour.stat_fg + 16*cfg.colour.stat_bg;
  }
  puttext (1, cfg.status_line, cfg.scr_width, cfg.status_line, scr);
  va_end (args);
}

/*
 * Replacement for djgpp's rather slow gotoxy() routine
 */
#if defined(__DJGPP__) && 0
#include <go32.h>
#include <pc.h>
#include <sys/farptr.h>

#define DELAY()  (volatile void) inportb(0x43)   /* 8254 PIT */

static WORD CRT_base = 0x34D;
static WORD scr_page = 0;

static void fast_gotoxy (int x, int y)
{
  WORD ofs;

  x  += scr_info.winleft - 1;
  y  += scr_info.wintop  - 1;
  ofs = x + y*scr_info.screenwidth;

  outportb (CRT_base, 0x0E);
  DELAY();
  outportb (CRT_base+1, ofs >> 8);
  DELAY();
  outportb (CRT_base, 0x0F);
  DELAY();
  outportb (CRT_base+1, ofs & 255);
  _farpokeb (_dos_ds, 0x450+2*scr_page, x);
  _farpokeb (_dos_ds, 0x451+2*scr_page, y);
}

static void patch_gotoxy (void)
{
  union REGS regs;
  DWORD ofs   = (char*)fast_gotoxy - (char*)gotoxy;
  BYTE  op[5] = { 0xE9 };     /* "jmp" (relative) */

  *(DWORD*)&op[1] = ofs - 5;  /* 5==sizeof('jmp xyz') */
  disable();
  memcpy (&gotoxy, op, sizeof(op));
  enable();
  CRT_base = _farpeekw (_dos_ds, 0x463);

  regs.h.ah = 0x0F;
  int86 (0x10, &regs, &regs);
  scr_page = regs.h.bh;
}
#endif

