/*
 * VT102 - ANSI/VT100/VT102 decoder
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "telnet.h"
#include "config.h"
#include "screen.h"

static int par_num    = 0, param[16];
static int mode       = VT100;
static int extend     = 0;
static int insertmode = 0;
static int newline    = 0;

static char term_id_str[] = "[?1;2c";  /* VT100 id string */

static int oldX, oldY;

static void SetColours (void);
static void TakeAction (int);
static void PutChar (int);
static void AnsiModeSet (char, int);
static void TransmitId (void);

static void normal_state (int);
static void got_ESC_state (int);
static void got_lpar_state (int);
static void get_arg_state (int);
static void char0_state (int);
static void char1_state (int);
static void double_state (int);

static void (*state)(int) = normal_state;

void VT_Init (void)
{
  SCR_ColourDefault();
  SCR_SetCharSet (0, 'B');
  SCR_SetCharSet (1, 'B');
  SCR_MapCharSet (0);
}

int VT_SetMode (int i)
{
  return (mode = i);
}

const char *VT_GetMode (void)
{
  switch (mode)
  {
    case ANSI:
         return ("ANSI");
    case HEATH19:
         return ("HEATH19");
    case VT52:
         return ("VT52");
    case VT100:
         return ("VT100");
    case VT102:
         return ("VT102");
    case VT200:
         return ("VT200");
    case LINUX:
         return ("LINUX");
  }
  fprintf (stderr, "Unknown VT_Mode: %d\n", mode);
  return (NULL);
}


void VT_Process (int ch)
{
  tel_dump ("%c", ch);
  (*state) (ch);
}

/*
 * Normal parser state. Check for ESC
 */
static void normal_state (int ch)
{
  switch (ch)
  {
    case '\x1B':
         state = got_ESC_state;
         break;
    case 14:      /* Map G1 to current */
         SCR_MapCharSet (1);
         break;
    case 15:      /* Map G0 to current */
         SCR_MapCharSet (0);
         break;
    default:
         PutChar (ch);
  }
}

/*
 * Parser for starting "ESC" sequence
 */
static void got_ESC_state (int ch)
{
  switch (ch)
  {
    case '[':
         memset (&param, 0, sizeof(param));
         par_num = 0;
         state = got_lpar_state;
         break;

    case 'E':
         SCR_MoveUp (1);
         SCR_PutChar ('\r');
         state = normal_state;
         break;

    case 'D':         /* \eD - move cursor down, scroll if needed */
         SCR_MoveDownScroll();
         state = normal_state;
         break;

    case 'M':         /* \eM - move cursor up, scroll if needed */
         SCR_MoveUpScroll();
         state = normal_state;
         break;

    case 'H':
         SCR_SetTabStop();
         state = normal_state;
         break;

    case '7':
         // SaveCursor();
         state = normal_state;
         break;

    case '8':
         // RestoreCursor();
         state = normal_state;
         break;

    case 'P':
         state = normal_state;
         break;

    case '=':
         // SetKeyPad (1);
         state = normal_state;
         break;

    case '>':               /* Enable numeric keypad */
         // SetKeyPad (0);
         state = normal_state;
         break;

    case '<':
         state = normal_state;
         break;

    case '(':         /* Select character set G0 */
         state = char0_state;
         break;

    case ')':         /* Select character set G1 */
         state = char1_state;
         break;

    case '#':         /* Set double high/wide characters */
         state = double_state;
         break;

    case '\\':
         state = normal_state;
         break;

    case '*':
         state = normal_state;
         break;

    case 24:          /* Cancel escape sequence */
    case 26:
         state = normal_state;
         break;

    case 'Z':         /* Transmit the terminal ID */
         TransmitId();
         state = normal_state;
         break;

    case '\x1B':
         break;

    default:
      // PutChar (ch);
         state = normal_state;
  }
}

/*
 * Parser for "ESC[" sequence
 */
static void got_lpar_state (int ch)
{
  if (isdigit(ch))     /* got "ESC[<digit>" */
  {
    param[par_num] = ch - '0';
    state = get_arg_state;
  }
  else if (ch == '?')
  {
    param[0] = 0;
    state = get_arg_state;
  }
  else                 /* got "ESC[<char>" */
  {
    TakeAction (ch);
    state = normal_state;
  }
}

/*
 * Parser for "ESC[..;..x" sequence
 */
static void get_arg_state (int ch)
{
  if (isdigit(ch))
  {
    param[par_num] *= 10;
    param[par_num] += ch - '0';
  }
  else
  {
    if (ch == ';')
    {
      par_num++;
      state = got_lpar_state;
    }
    else
    {
      TakeAction (ch);
      state = normal_state;
    }
  }
}

/*
 * Set the current character set for G0
 */
static void char0_state (int ch)
{
  SCR_SetCharSet (0, ch);
  state = normal_state;
}

/*
 * Set the current character set for G1
 */
static void char1_state (int ch)
{
  SCR_SetCharSet (1, ch);
  state = normal_state;
}

/*
 * Set the current line to double high and/or wide
 */
static void double_state (int ch)
{
  switch (ch)
  {
    case '5':        /* Single width */
    case '6':        /* Double width */
    case '3':        /* Double height/width */
    case '4':        /* Bottom half of double height/width */
    default:
         ;
  }
  state = normal_state;
}


static void TakeAction (int ch)
{
  int i, j;

  switch (ch)
  {
    case 'f':               /* Fall-through */
    case 'H':
         if (param[0] == 0)
              i = 1;
         else i = param[0];
         if (param[1] == 0)
              j = 1;
         else j = param[1];
         SCR_GotoRowCol (i, j);
         break;

    case 'A':               /* move up relative current row */
         if (param[0] == 0)
              i = 1;
         else i = param[0];
         SCR_MoveUp (i);
         break;

    case 'B':               /* move down relative current row */
         if (param[0] == 0)
              i = 1;
         else i = param[0];
         SCR_MoveDown (i);
         break;

    case 'C':               /* move right relative current column */
         if (param[0] == 0)
              i = 1;
         else i = param[0];
         SCR_MoveRight (i);
         break;

    case 'D':               /* move left relative current column */
         if (param[0] == 0)
              i = 1;
         else i = param[0];
         SCR_MoveLeft (i);
         break;

    case 's':
         oldX = wherex();
         oldY = wherey();
         break;

    case 'u':
         SCR_GotoRowCol (oldY, oldX);
         break;

    case 'J':
         if (param[0] == 0)
            SCR_ClearEOD();
         else if (param[0] == 1)
            SCR_ClearBOD();
         else if (param[0] == 2)
            SCR_Clear();
         break;

    case 'K':
         if (param[0] == 0)
            SCR_ClearEOL();
         else if (param[0] == 1)
            SCR_ClearBOL();
         else if (param[0] == 2)
         {
           SCR_ClearBOL();
           SCR_ClearEOL();
         }
         break;

    case 'l':               /* Reset ANSI mode */
    case 'h':               /* Set ANSI mode */
         for (i = 0; i < par_num; i++)
             AnsiModeSet (param[i], 0);
         break;

    case 'm':
         SetColours();
         break;

    case 'r':
         SCR_SetScroll (param[0], param[1]);
         break;

    case 'd':
         i = param[0];
         if (i == 0)
             i = 1;
         SCR_GotoRowCol (1, i);
         break;

    case 'G':
         i = param[0];
         if (i == 0)
             i = 1;
         SCR_GotoRowCol (i, 1);
         break;

    case 'g':
         if (param[0] == 0)
            SCR_ClearTabStop();
         else if (param[0] == 3)
            SCR_ClearAllTabs();
         break;

    case 'P':
         SCR_DeleteChar (1);
         break;

    case 'c':
         TransmitId();
         break;

    case 24:
    case 26:
         break;
  }
}

static void SetColours (void)
{
  static int colourMap[] = { 0, 4, 2, 6, 1, 5, 3, 7 };
  int    i;

  for (i = 0; i <= par_num; i++)
  {
    switch (param[i])
    {
      case 0:               /* reset attributes to default */
           SCR_ColourDefault();
           break;
      case 1:
           SCR_SetBold (1);
           break;
      case 2:
           SCR_SetBold (0);
           break;
      case 3:
           break;
      case 4:               /* underline on */
           SCR_SetBold (1);
           break;
      case 5:
      case 6:
           SCR_SetBlink (1);
           break;
      case 7:
           SCR_SetInverse();
           break;
      case 8:
           SCR_SetFore (SCR_GetBack());
           break;
      case 10:
           extend = 0;
           break;
      case 12:
           extend = 1;
           break;
      default:
           if (param[i] >= 30 && param[i] <= 37)
              SCR_SetFore (colourMap[param[i] - 30]);
           else if (param[i] >= 40 && param[i] <= 47)
              SCR_SetBack (colourMap[param[i] - 40]);
    }
  }
}

static void PutChar (int ch)
{
  switch (ch)
  {
    case 7:
         sound (1000);
         delay (20);
         nosound();
         break;
    case 8:
         SCR_PutChar ('\b');
         break;
    case 9:
         SCR_PrintTab();
         break;
    case 0:
    case 11:
    case 12:
    case 15:
         break;
    default:
         if (extend)
            ch |= 128;
         SCR_PutChar (ch);
         break;
  }
}

static void AnsiModeSet (char ch, int mode)
{
  switch (ch)
  {
    case 2:                 /* Lock/unlock keyboard */
         break;
    case 4:                 /* Insert/Replace setting */
         insertmode = mode;
         break;
    case 12:                /* Echo */
         break;
    case 20:                /* New Line mode */
         newline = mode;
         break;
    default:
         break;
  }
}

static void TransmitId (void)
{
  PutStringTN ("\x1B%s", term_id_str);
}

