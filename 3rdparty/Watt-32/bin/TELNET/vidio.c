/* Copyright (c) 1988 Jerry Joplin
 *
 * Portions copyright (c) 1981, 1988
 * Trustees of Columbia University in the City of New York
 *
 * Permission is granted to any individual or institution
 * to use, copy, or redistribute this program and
 * documentation as long as it is not sold for profit and
 * as long as the Columbia copyright notice is retained.
 *
 *
 * Modified by Nagy Daniel - tailored for DJGPP
 *      - added ChrDelete function
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <dos.h>
#include <conio.h>
#include <mem.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <sys/nearptr.h>
#include <crt0.h>
#include <tcp.h>

void *MK_FP (WORD seg, WORD ofs)
{
  if (!(_crt0_startup_flags & _CRT0_FLAG_NEARPTR))
     if (!__djgpp_nearptr_enable())
        return (NULL);
  return (void*) (seg * 16 + ofs + __djgpp_conventional_base);
}

unsigned FP_SEG (void *p)
{
  return ((int) ((long) p << 4));
}

unsigned FP_OFF (void *p)
{
  return ((int) ((long) p % 16));
}

#define NORMAL        0x7       /* Normal video attribute */
#define BOLD          0x8       /* Bold video attribute */
#define UNDERLINED    0xA       /* Underlined video attribute */
#define REVERSE       0x70      /* Reverse video attribute */
#define SCREEN        0x10      /* BIOS video interrupt number */
#define RETRACE       0x3da     /* Video Retrace port address for CGA */
#define ASCII         0         /* ASCII character set */
#define UK            1         /* UK character set */
#define SPECIAL       2         /* Special character set, graphics chars */


/*
 * Function prototypes
 */
void VidInit (void);            /* Initialize the video system */
void SetVattr (BYTE);           /* Set the video attribute */
void AddVattr (BYTE);           /* Add attribute to current video attribute */
void SubVattr (BYTE);           /* Sub attribute from current vid attribute */
void BrkAtt (BYTE);             /* Break attribute into extra and base */
BYTE AddAtt (void);             /* Add extra and base attributes to get */
                                /* a resulting displayable video attribute */
void ChrWrite (BYTE);           /* Write character to the screen */
void ChrDelete (void);          /* Erase character at current position */

void SetScroll (int, int);      /* Set the scrolling region */
void ScrollDown (void);         /* Move down a row scrolling if necessary */
void ScrollUp (void);           /* Move up a row scrolling if necessary */
void IndexDown (void);          /* Scroll the screen down */
void IndexUp (void);            /* Scroll the screen up */
void SetCurs (int, int);        /* Set the cursor to absolute coordinates */
void SetRelCurs (int, int);     /* Set the cursor to relative coordinates */
void PosCurs (void);            /* Position the cursor to cursx,cursy */
void ClearScreen (void);        /* Clear the terminal screen */
void ClearEOS (void);           /* Clear from cursor to end of screen */
void ClearBOS (void);           /* Clear from cursor to top of screen */
void ClearEOL (void);           /* Clear from cursor to end of line */
void ClearBOL (void);           /* Clear from cursor to start of line */
void ClearBox (BYTE,            /* Clear a box on the video screen */
               BYTE, BYTE, BYTE, BYTE);
void MapCharSet (int);          /* Map a character set */
void SetCharSet (int, BYTE);    /* Set a character set */
void SaveCursor (void);         /* Save the cursor description */
void RestoreCursor (void);      /* Restore the cursor description */
void SetCursorVisibility (int); /* Set the cursor visibility mode */
void SetBackGround (int);       /* Set background video attribute */
void SetColor (void);           /* Set the screen colors */
void InitTabs (void);           /* Initialize the tab settings */
void DoTab (void);              /* Perform a tab */
void SetTabStop (void);         /* Set a tab stop at cursor position */
void ClearTabStop (void);       /* Clear a tab stop at the cursor position */
void ClearAllTabs (void);       /* Clear all the defined tab stops */
void SetScreenWidth (int);      /* Set the logical width of the screen */
void StartScreen (int, int);    /* Start a screen access */
void EndScreen (void);          /* End a screen access */
void WriteOneChar (BYTE,        /* Write one character to the screen */
                   int, int);
int vtprintf (int, int, int,    /* Printf for Emulator to row and column */
              char *, ...);     /* in regular or reverse video */
void SaveScreen (void);         /* Save contents of video memory */
void RestoreScreen (void);      /* Restore contents of video memory */

static void interrupt10 (unsigned, /* Issue a BIOS video interrupt */
                         unsigned, unsigned, unsigned);
/*
 * Global Data
 */
BYTE retracemode = 0;           /* Flag indicating No Video refresh wait */
BYTE forecolor;                 /* Foreground color */
BYTE backcolor;                 /* Background color */

BYTE vidmode;                   /* Screen video mode */


/*
 * External variables
 */
extern unsigned originmode;     /* Origin mode, relative or absolute */
extern unsigned insertmode;     /* Insert mode, off or on */
extern unsigned autowrap;       /* Automatic wrap mode, off or on */
extern unsigned newline;        /* Newline mode, off or on,  GLOBAL data! */
extern unsigned cursorvisible;  /* Cursor visibility, on or hidden */
extern unsigned reversebackground; /* Reverse background attribute, on or off */
extern unsigned screenwid;      /* Absolute screen width */
extern BYTE username[20];
extern BYTE *remotehost;
extern char statusline;


/*
 * Local static data
 */
static BYTE screentop;          /* Absolute top of screen */
static BYTE screenbot;          /* Absolute bottom of screen */
static BYTE scrolltop;          /* Top row of scrolling region */
static BYTE scrollbot;          /* Bottom row of scrolling region */

static int cursx;               /* X cursor position */
static int cursy;               /* Y cursor position */

static unsigned scroff;         /* Screen memory offset */
static unsigned scrseg;         /* Screen memory segment */
static unsigned scrchars;       /* Number of chars written to video memory */
static BYTE tvmode;             /* Flag to indicate control program present */
char  *screen;                  /* Pointer to video screen */
static unsigned savebp;         /* Static storage for BP through int 0x10 */

static BYTE video_state;        /* State of video, reversed or normal */
static BYTE scbattr;            /* Video attribute of empty video cell */
BYTE curattr;                   /* Video attribute of displayable chars */
static BYTE baseattr;           /* Base attribute for video attributes */
static BYTE extrattr;           /* Extra attribute for video attributes */

static BYTE att_reverse;        /* Reverse attribute bits */
static BYTE att_normal;         /* Normal attribute bits */
static BYTE att_low_mask = 0x6; /* Low attribute mask */
static BYTE att_underline = 0x1; /* Underlined attribute bit */
static BYTE att_intensity = 0x8; /* Bold attribute bit */
static BYTE att_blink = 0x80;   /* Blinking attribute bit */

static unsigned columns;        /* Columns on logical terminal screen */
static unsigned lines;          /* Lines on logical terminal screen */

static char tabs[132];          /* active tab stops */
static char deftabs[132];       /* default tab stops, 9,17,26 .... */

static int G0 = ASCII;          /* Character set G0 */
static int G1 = ASCII;          /* Character set G1 */
static int *GL = &G0;           /* Pointer to current mapped character set */

static char special_chars[32] = { /* Special characters */
            32, 4, 176, 9, 12, 13, 10, 248, 241, 18, 11, 217, 191, 218, 192,
            197, 196, 196, 196, 196, 196, 195, 180, 193, 194, 179, 243, 242,
            227, 216, 156, 7
          };

static struct SaveCursorStruct
{                               /* Structure to save cursor description */
  int cursx;                    /* X cursor position, column */
  int cursy;                    /* Y cursor position, row */
  int *GL;                      /* pointer to mapped character set */
  int G0;                       /* character set G0 */
  int G1;                       /* character set G1 */
  int mode;                     /* origin mode */
} save = { 1, 1, &G0, ASCII, ASCII, 0 };

                                 /* Pointer to memory allocated to hold */
static char *screen_save = NULL; /* the contents of video memory */

/****************************************************************************/
/****************************************************************************/

/* V I D I N I T  -- Initialize the video system */

void VidInit (void)
{
  char *bptr;
  __dpmi_regs r;

  /* Obtain video information from BIOS */
  r.h.ah = 0x0F;                /* Use function F of interrupt 10 */
  savebp = r.x.bp;              /* Precautionary save of register BP */
  __dpmi_int (0x10, &r);        /* Issue BIOS video interrupt */
  r.x.bp = savebp;              /* Restore saved BP register */
  vidmode = r.h.al;             /* Save the video mode */
  columns = r.h.ah;             /* Save the number of columns */

  lines = 25;                   /* Lines = 25, (sorry no 43,50 lines) */
  screenbot = lines - 1;        /* Bottom of screen is 24 */
  screentop = 1;                /* Top of screen is line 1 */

  tvmode = 0;                   /* Assume no control program present */

  /* First determine if snow is a problem */
  if (vidmode != 7)             /* Assume video adapter is snowy if */
    retracemode = 1;            /* it is not a MonoChrome */

                                /* First query Video BIOS to see if */
  r.x.ax = 0x1A00;              /* VGA is present, no "snow" problem on VGA */
  savebp = r.x.bp;              /* Precautionary save of register BP */
  __dpmi_int (0x10, &r);        /* Issue BIOS video interrupt */
  r.x.bp = savebp;              /* Restore saved BP register */
  if (r.h.al == 0x1A)           /* If VGA is detected */
    retracemode = 0;            /* No snow protection needed */
  else
  {                             /* Else look for an EGA */
    r.h.cl = 0xC;               /* Test the Video BIOS to see if */
    r.x.bx = 0xFF10;            /*  an EGA can be detected */
    r.x.ax = 0x1200;            /*  EGA's don't have "snow" problem either */
    savebp = r.x.bp;            /* Precautionary save of register BP */
    __dpmi_int (0x10, &r);      /* Issue BIOS video interrupt */
    r.x.bp = savebp;            /* Restore saved BP register */
    if (r.h.cl < 0xC)
    {                            /* If EGA is detected */
      bptr = MK_FP (0x40, 0x87); /* Check BIOS data to see if the */
      if ((*bptr & 0x8) == 0)    /* EGA is the active adapter */
         retracemode = 0;        /* No snow protection required */
    }
  }

  /* Determine the default screen attributes */
  r.h.ah = 0x8;                 /* Issue function 8 or interrupt 10 */
  r.h.bh = 0x0;                 /* for page 0 */
  savebp = r.x.bp;              /* Precautionary save of register BP */
  __dpmi_int (0x10, &r);;       /* Get video attribute at cursor pos */
  r.x.bp = savebp;              /* Restore saved BP register */
  scbattr = r.h.ah;             /* Save this attribute */

  forecolor = scbattr & 0xf;    /* Save values for determined colors */
  backcolor = scbattr >> 4;

  att_normal = scbattr;
  BrkAtt (scbattr);             /* Break the attribute into base,extra */

  /* Reverse the foreground and background */
  baseattr = (baseattr >> 4 | baseattr << 4);
  att_reverse = AddAtt();      /* Put the attributes back together */
                               /* in order to get reverse attribute */

  /* Clear screen to established attribute */
  interrupt10 (0x0600, scbattr << 8, 0, (lines << 8) | (columns - 1));

#if 0
  if (statusline)
  {
    /* Clear the top line setting it to reverse */
    interrupt10 (0x0600, att_reverse << 8, 0, columns - 1);
    vtprintf (0, 0, 1, "%s@%s", username, remotehost);
    /* Display the mode line in reverse */
  }
#endif

  if (screen_save == NULL)
  {                             /* If first time to be initialized */
    /* Attempt to allocate screen mem */
    if ((screen_save = malloc (lines * columns * 2)) == NULL)
    {
      vtprintf (0, 0, 1, "No video memory");
      exit (1);
    }
  }
}

/* S E T V A T T R  --  Set the video attribute */

void SetVattr (BYTE attr)
{
  video_state = 0;              /* Reset the video state */
  BrkAtt (scbattr);             /* Break apart the default screen attribute */

  switch (attr)
  {                             /* See what video attribute is requested */
    case BLINK:             /* Blinking characters */
         extrattr = att_blink;
         break;
    case REVERSE:           /* Reversed video characters */
         video_state = 1;
         baseattr = (baseattr >> 4 | baseattr << 4);
         break;
    case UNDERLINED:        /* Underlined characters */
         if (vidmode == 0x7)    /* Monochrome can underline */
           extrattr = att_underline;
         else
         {                      /* others can't use reverse video */
           video_state = 1;
           baseattr = (baseattr >> 4 | baseattr << 4);
         }
         break;
    case BOLD:              /* High intensity, bold, characters */
         extrattr = att_intensity;
         break;
    case NORMAL:            /* Normal characters */
    default:
         extrattr = 0;
         break;
  }
  curattr = AddAtt();          /* Put the video attributes back together */
}

/* A D D V A T T R  --  Add an attribute bit to the current video attribute */

void AddVattr (BYTE attr)
{

  BrkAtt (curattr);             /* Break apart the current video attribute */

  switch (attr)
  {                             /* See what attribute wants to be added */
    case BLINK:             /* Blinking attribute */
         extrattr |= att_blink;
         break;
    case BOLD:              /* High intensity, bold, attribute */
         extrattr |= att_intensity;
         break;
    case REVERSE:           /* Reversed attribute */
         if (video_state == 0)
         {
           video_state = 1;
           baseattr = (baseattr >> 4 | baseattr << 4);
         }
         break;
    case UNDERLINED:        /* Underlined characters */
         if (vidmode == 0x7)    /* Monochrom can underline */
           extrattr = att_underline;
         /* others cant use reversed video */
         else if (video_state == 0)
         {
           video_state = 1;
           baseattr = (baseattr >> 4 | baseattr << 4);
         }
         break;
    default:
         break;
  }
  curattr = AddAtt();          /* Put the video attributes back together */
}

/* S U B V A T T R  --  Remove attribute bit to the current video attribute */

void SubVattr (BYTE attr)
{
  BrkAtt (curattr);             /* Break apart the current video attribute */

  switch (attr)
  {                             /* See what video attribute to remove */
    case BLINK:                 /* Remove the blinking attribute */
         extrattr &= ~att_blink;
         break;
    case BOLD:                  /* Remove the high intensity, bold */
         extrattr &= ~att_intensity;
         break;
    case REVERSE:               /* Remove reversed attribute */
         if (video_state == 1)
         {
           video_state = 0;
           baseattr = (baseattr >> 4 | baseattr << 4);
         }
         break;
    case UNDERLINED:            /* Remove underlined attribute */
         if (vidmode == 0x7)    /* Monochrome could have underlined */
            extrattr &= ~att_underline;
         /* others couldn't remove reverse attribute */
         else if (video_state == 1)
         {
           video_state = 0;
           baseattr = (baseattr >> 4 | baseattr << 4);
         }
         break;
    default:
         break;
  }
  curattr = AddAtt();          /* Put the video attributes back together */
}

/* B R K A T T R -- Break an attribute into its video components */

void BrkAtt (BYTE attribute)
{
  extrattr = 0;                 /* Clear extra attributes */
  baseattr = attribute;         /* Start specified base attribute */

  if (vidmode == 0x7)
  {                             /* If a Monochrome monitor */
    if (attribute & att_low_mask)
    {                           /* Any Low mask attributes on? */
      baseattr |= att_normal;   /* if yes then set normal bits on */
    }
    else
    {                           /* else check other attributes */
      if (attribute & att_underline)
      {                         /* Underline attribute ? */
        extrattr |= att_underline; /* yes then set underline bit */
        if (attribute & 0x70)   /* Reverse video ? */
          baseattr &= ~att_underline; /* If yes then clear underline */
        else                    /* monochrome can't do both */
          baseattr |= att_normal; /* Else set normal bits on */
      }
    }
  }

  if (baseattr & att_intensity) /* If bold attribute is on */
    extrattr |= att_intensity;  /* then set intensity bit */

  if (baseattr & att_blink)     /* If blink attribute is on */
    extrattr |= att_blink;      /* then set blink bit */

  /* Turn off blink,bold in base attribute */
  baseattr &= ~(att_intensity + att_blink);
}

/* A D D A T R -- Build video attribute from base and extra attributes */

BYTE AddAtt (void)
{
  if (extrattr & att_underline) /* If underline is requested */
     baseattr &= ~att_low_mask;  /* Clear low mask */

  return (baseattr | extrattr); /* return the or'ed attributes */
}

/* C H R W R I T E  -- Write a character to a row and column of the screen */

void ChrWrite (BYTE chr)
{
  BYTE c[2], attr[2];
  int row, ws;

  if (*GL == ASCII)             /* Check character set being used */
     ;                          /* if regular ASCII then char is OK */
  else if (*GL == SPECIAL)
  {                             /* if using the special character */
    if (chr > 94 && chr < 128)  /* then translate graphics characters */
      chr = special_chars[chr-95];
  }
  else if (*GL == UK)
  {                             /* If using the UK character set */
    if (chr == '#')             /* then watch for the number sign */
      chr = 'œ';                /* translating it to British pound */
  }

  /* NOTE:  Inserting a character using this technique is *very* slow      */
  /* for snowy CGA systems                                                 */
  if (insertmode)
  {                             /* If insert mode, scoot rest of line over */
    StartScreen (cursy, cursx - 1); /* Start direct video memory access        */

    c[0] = *screen;             /* Save character at current position      */
    attr[0] = *(screen + 1);    /* Save attribute at current position      */
    ws = 1;
    for (row = cursx; row < columns; row++)
    {
      c[ws] = *(screen + 2);    /* Save character at next position        */
      attr[ws] = *(screen + 3); /* Save attribute at next position     */
      ws ^= 1;                  /* Flop save char,attribute array index   */
      *(screen + 2) = c[ws];    /* Write saved character and attribute   */
      *(screen + 3) = attr[ws];
      screen += 2;              /* Increment to next character position   */
      scrchars++;               /* Increment the number of chars written  */
    }
    EndScreen();               /* Update screen in control programs      */
  }

  if (cursx > screenwid)
  {                             /* If trying to go beyond the screen width */
    if (autowrap)
    {                           /* when autowrap is on */
      ScrollUp();              /* scroll the screen up */
      SetCurs (1, 0);           /* set cursor to column 1 of next line */
    }
    else
      cursx = screenwid;        /* else put the cursor on right margin */
  }

  WriteOneChar (chr, cursy, cursx - 1);

  ++cursx;                      /* Increment the cursor X position */
  PosCurs();                   /* Move the cursor to the new position */
}


/* C H R D E L E T E  -- Erase a character at current position */

void ChrDelete (void)
{
  int row, ws;

  StartScreen (cursy, cursx - 1); /* Start direct video memory access        */

  ws = 1;
  for (row = cursx; row < columns; row++)
  {
    *(screen) = *(screen + 2);
    *(screen + 1) = *(screen + 3);
    screen += 2;                /* Increment to next character position   */
    scrchars++;                 /* Increment the number of chars written  */
  }
  EndScreen();                 /* Update screen in control programs      */
}

/* S E T S C R O L L  -- Establish the scrolling boundaries */

void SetScroll (register int top, register int bottom)
{
  if (top == 0)                 /* If the top scroll boundary is 0 */
    top = 1;                    /* interpret this as the top screen row */
  if (bottom == 0)              /* If the bottom scroll boundary is 0 */
    bottom = screenbot;         /* interpret this as bottom screen row */

  /* Set scrolling region if valid coords */
  if (top > 0 && top <= screenbot && bottom >= top && bottom <= screenbot)
  {
    scrolltop = top;            /* save top boundary */
    scrollbot = bottom;         /* save bottom boundary */
    SetCurs (1, 1);             /* this also homes the cursor */
  }
}

/* S C R O L L D O W N  -- Move up a row scrolling if necessary */

void ScrollDown (void)
{
  if (cursy == scrolltop)       /* If on the top of the scrolling region */
    IndexDown();               /* scroll the rest of the region down */
  else
  {                             /* Else */
    --cursy;                    /* just decrement cursor Y position */
    PosCurs();                 /* and request the reposition */
  }
}

/* S C R O L L U P  -- Move down a row scrolling if necessary */

void ScrollUp (void)
{
  if (cursy == scrollbot)       /* If on the bottom of the scrolling region */
    IndexUp();                 /* scroll the rest of the region down */
  else
  {                             /* Else */
    ++cursy;                    /* just increment the cursor Y position */
    PosCurs();                 /* and request the reposition */
  }
}

/* I N D E X D O W N  -- Scroll the screen down */

void IndexDown (void)
{
  register unsigned attr;

  attr = scbattr << 8;          /* Get the attribute for new line */
  /* Call the BIOS to scroll the region */
  interrupt10 (0x0701, attr, scrolltop << 8, (scrollbot << 8) | (columns - 1));
  PosCurs();                   /* Position the cursor */
}

/* I N D E X U P  -- Scroll the screen up */

void IndexUp (void)
{
  unsigned attr = scbattr << 8; /* Get the attribute for new line */

  /* Call the BIOS to scroll the region */
  interrupt10 (0x0601, attr, scrolltop << 8, (scrollbot << 8) | (columns - 1));
  PosCurs();                   /* Position the cursor */
}

/* S E T C U R S -- Set absolute cursor position on the logical screen */

void SetCurs (register int col, register int row)
{
  if (col == 0)                 /* If called with X coordinate = zero */
    col = cursx;                /* then default to current coordinate */
  if (row == 0)                 /* If called with Y coordinate = zero */
    row = cursy;                /* then default to current coordinate */

  if (originmode)
  {                             /* If origin mode is relative */
    row += (scrolltop - 1);     /* adjust the row */
    if (row < scrolltop || row > scrollbot)
      return;                   /* Can not position cursor out of scroll */
    /* region in relative cursor mode */
  }
  /* Can only position the cursor if it lies */
  /* within the logical screen limits */
  if (col <= screenwid && row <= screenbot)
  {
    cursx = col;                /* Set the X cursor coordinate, column */
    cursy = row;                /* Set the Y cursor coordinate, row */
    PosCurs();                 /* Request the physical positioning */
  }
}

/* S E T R E L C U R S -- Set relative curs pos on the logical screen */

void SetRelCurs (register int col, register int row)
{
  if (col == 0)                 /* If called with X coordinate = zero */
    col = cursx;                /* then default to current X coordinate */
  else                          /* Else */
    col = cursx + col;          /* add col value to X cursor position */
  /* Note:  col can be negative */

  if (row == 0)                 /* If called with Y coordinate = zero */
    row = cursy;                /* then default to current Y coordinate */
  else                          /* Else */
    row = cursy + row;          /* add row value to Y cursor position */
  /* Note:  row can be negative */

  if (originmode)
  {                             /* If origin mode is relative */
    row += (scrolltop - 1);     /* adjust the row */
    if (row < scrolltop || row > scrollbot)
      return;                   /* Can not position cursor out of scroll */
  }

  /* region in relative cursor mode */
  /* Can only position the cursor if it lies */
  /* within the logical screen limits */
  if (col > 0 && col <= screenwid && row > 0 && row <= screenbot)
  {
    cursy = row;                /* Set the X cursor coordinate, column */
    cursx = col;                /* Set the Y cursor coordinate, row */
    PosCurs();                 /* Request the physical positioning */
  }
}

/* P O S C U R S -- Position the cursor on the physical screen */

void PosCurs (void)
{
  int col = cursx;
  int row = cursy;

  if (col > columns)            /* Check validity of requested column */
    col = columns;              /* put cursor on the right bound */

  if (row > lines)              /* Check validity of requested row */
    row = lines;                /* put cursor on the bottom */

  if (cursorvisible)            /* Only position the cursor if its visible */
    interrupt10 (0x0200, 0, 0, (row << 8) | --col);
}

/* C L E A R S C R E E N -- Clear the screen setting it to a normal attr */

void ClearScreen (void)
{
  ClearBox (1, screentop, columns, screenbot, scbattr);
}

/* C L E A R E O S -- Clear from the cursor to the end of screen */

void ClearEOS (void)
{
  ClearEOL();                  /* First clear to the End of the Line */
  if (cursy < screenbot)        /* Then clear every line below it */
    ClearBox (1, cursy + 1, columns, screenbot, scbattr);
}

/* C L E A R B O S -- Clear from the cursor to the beggining of screen */

void ClearBOS (void)
{
  ClearBOL();                  /* First clear to the Beginning of the Line */
  if (cursy > screentop)        /* Then clear every line above it */
    ClearBox (1, screentop, columns, cursy - 1, scbattr);
}

/* C L E A R E O L -- Clear to the end of the current line */

void ClearEOL (void)
{
  ClearBox (cursx, cursy, columns, cursy, scbattr);
}

/* C L E A R B O L -- Clear to the beginning of the current line */

void ClearBOL()
{
  ClearBox (1, cursy, cursx, cursy, scbattr);
}

/* C L E A R B O X -- Clear a window on the screen with the specified attr */

void ClearBox (BYTE left, BYTE top, BYTE right, BYTE bottom, BYTE attr)
{
  /* Use BIOS scroll window function to clear */
  interrupt10 (0x0600, attr << 8, /* a window to a specified attribute */
               (top << 8) | (--left), (bottom << 8) | --right);
}

/* M A P C H A R S E T -- Map an established character set to current */

void MapCharSet (int charset)
{
  if (charset == 0)             /* If mapping G0 character set */
    GL = &G0;                   /* Point the current char set,GL to G0 */
  else if (charset == 1)        /* If mapping G1 character set */
    GL = &G1;                   /* Point the current char set,GL, to G1 */
}

/* S E T C H A R S E T -- Establish a character set */

void SetCharSet (int gset, BYTE set)
{
  int *charset;

  if (gset == 0)                /* Check to see what character set is */
    charset = &G0;              /* going to be set */
  else if (gset == 1)
    charset = &G1;
  else
    return;                     /* If not valid set then return */

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

/* S A V E C U R S O R  --  Save the cursor description into memory */

void SaveCursor (void)
{
  save.cursx = cursx;           /* Save the X cursor position */
  save.cursy = cursy;           /* Save the Y cursor position */
  save.GL = GL;                 /* Save the current mapped character set */
  save.G0 = G0;                 /* Save G0 character set */
  save.G1 = G1;                 /* Save G1 character set */
  save.mode = originmode;       /* Also save the origin mode */
}

/* R E S T O R E C U R S O R  --  Restore the cursor description from memory */

void RestoreCursor (void)
{
  cursx = save.cursx;           /* Restore the saved X cursor position */
  cursy = save.cursy;           /* Restore the saved Y cursor position */
  GL = save.GL;                 /* Restore the saved mapped character set */
  G0 = save.G0;                 /* Restore the saved G0 character set */
  G1 = save.G1;                 /* Restore the saved G1 character set */
  originmode = save.mode;       /* Also restore the saved origin mode */
  PosCurs();                   /* Then reposition the cursor */
}

/* S E T C U R S O R V I S I B I L I T Y -- Show/Hide the cursor */

void SetCursorVisibility (int mode)
{
  if (mode)
  {                             /* If invisible cursor is specified, then */
    cursorvisible = 1;          /* the cursor will not appear on the */
    SetCurs (0, 0);             /* terminal screen */
  }
  else
  {                             /* Else the cursor will be shown at the */
    cursorvisible = 0;          /* current cursor position */
    interrupt10 (0x0200, 0, 0, lines << 8);
  }
}

/* S E T B A C K G R O U N D -- Set the background attribute */

void SetBackGround (int mode)
{
  int reverse_screen = 0;       /* Flag to indicate screen is to be reversed */
  int i;

  if (mode)
  {                             /* If reversed background is specified, */
    if (reversebackground != 1)
    {                           /* only reverse the screen if it is */
      reverse_screen = 1;       /* not already set to reverse */
      reversebackground = 1;
    }
  }
  else
  {                             /* Else if normal background is specified */
    if (reversebackground != 0)
    {                           /* only reverse the screen if it is */
      reverse_screen = 1;       /* currently set to reverse */
      reversebackground = 0;
    }
  }

  if (reverse_screen)
  {                             /* If reverse screen flag is set */
    /* first save the contents of screen */

    StartScreen (0, 0);

    for (i = 0; i < lines * columns; i++)
    {
      screen++;
      BrkAtt (*screen);         /* Break attribute apart and reverse it */
      baseattr = (baseattr >> 4 | baseattr << 4);
      *screen++ = AddAtt();    /* Put attribute together as displayable */
    }
    scrchars = i;
    EndScreen();

    BrkAtt (scbattr);           /* reverse the default character attr */
    baseattr = (baseattr >> 4 | baseattr << 4);
    scbattr = AddAtt();
    BrkAtt (curattr);           /* reverse the current character attr */
    baseattr = (baseattr >> 4 | baseattr << 4);
    curattr = AddAtt();
  }
}

/* S E T C O L O R -- Set the screen color */

void SetColor (void)
{
  int i;
  BYTE attr;

  BrkAtt (att_normal);          /* Break apart the current screen color */
  attr = baseattr;              /* Save this attribute */

  /* Create the new screen attribute */
  att_normal = curattr = scbattr = ((backcolor << 4) | forecolor);
  BrkAtt (att_normal);          /* and the new reverse attribute */
  baseattr = (baseattr >> 4 | baseattr << 4);
  att_reverse = AddAtt();

  StartScreen (0, 0);

  for (i = 0; i < lines * columns; i++)
  {
    screen++;
    BrkAtt (*screen);           /* Break attribute apart and reverse it */
    if (baseattr == attr)
    {                           /* If this chars base attributes are the */
      baseattr = att_normal;    /* old screen color then this is a normal */
    }                           /* character so set it to new normal attr */
    else                        /* Else set this character to new reverse */
      baseattr = att_reverse;

    *screen++ = AddAtt();      /* Put attribute together as displayable */
  }
  scrchars = i;
  EndScreen();
}

/* I N I T T A B S -- Initialize Tab stops to default settings */

void InitTabs (void)
{
  int i;

  for (i = 1; i < 131; i++)
  {                             /* Set tabs for mod 8 = 0 positions */
    if (i % 8)                  /* 9, 17, 26 .... */
      deftabs[i + 1] = tabs[i + 1] = 0; /* Zero indicates no tab here */
    else
      deftabs[i + 1] = tabs[i + 1] = 1; /* One indicates a tab stop */
  }
}

/* D O T A B -- Perform a tab */

void DoTab (void)
{
  int i;

  /* Look for next tab stop */
  for (i = cursx + 1; i <= screenwid; i++)
  {
    if (tabs[i] == 1)
    {                           /* If a tab stop is found */
      SetCurs (i, cursy);       /* request cursor position here */
      return;                   /* and finished */
    }
  }
}

/* S E T T A B S T O P  -- set a tab stop at the current cursor position */

void SetTabStop (void)
{
  tabs[cursx] = 1;              /* Mark current cursor position as tab stop */
}

/* C L E A R T A B S T O P  -- clear a tab stop at the current curs position */

void ClearTabStop (void)
{
  tabs[cursx] = 0;              /* Clear current cursor position tab stop */
}

/* C L E A R A L L T A B S  -- clear all tab stops */

void ClearAllTabs (void)
{
  /* Clear all of the tab stop marks */
  memset (tabs, '\0', sizeof (tabs));
}

/* S E T S C R E E N W I D T H -- set the screen width */

void SetScreenWidth (int width)
{
  if (width == 132)             /* When the screen is set to 132 columns */
    screenwid = 132;            /* set the logical right boundary */
  else if (width == 80)         /* Else if the screen width is 80 */
    screenwid = 80;             /* set the logical right boundary */

  /* Setting the screen width also */
  ClearScreen();               /* Clears the screen */
  originmode = 0;               /* Sets the origin mode to absolute */
  SetScroll (0, 0);             /* Resets the scrolling region */
  SetCurs (1, 1);               /* Sets the cursor to the home position */
}

/* S T A R T S C R E E N -- Start a direct screen addressing "transaction" */

void StartScreen (register int row, register int col)
{
  __dpmi_regs r;

  unsigned vscroff, vscrseg;

  if (vidmode == 7)
  {                             /* If in Monochrome video mode     */
    scrseg = 0xb000;            /* Assume video segment of B000 */
  }
  else
  {                             /* If other mode then assumme */
    scrseg = 0xb800;            /* video memory segment is B800 */
  }

  scroff = ((row * columns) + col) * 2; /* Calculate offset from beginning of */
  /* screen memory */
  scrchars = 0;                 /* Initialize count of characters written */

  savebp = r.x.bp;              /* Save reg BP, in case old BIOS trashes it */
  /* Query the address of video memory for */
  r.x.es = scrseg;              /* this process under Windows, DesQview, ... */
  r.x.di = 0;                   /* ES:DI = assumed screen address */
  r.h.ah = 0xFE;                /* Issue Video interrupt function FE */
  __dpmi_int (0x10, &r);        /*  in order to check for control program */
  vscroff = r.x.di;             /* ES:DI now holds actual address of screen */
  vscrseg = r.x.es;
  r.x.bp = savebp;              /* Restore register BP */

  tvmode = 0;                   /* Assume no control program */
  if (vscroff != 0 || vscrseg != scrseg)
  {
    scrseg = vscrseg;           /* If address of screen is different from */
    scroff += vscroff;          /* assumed address then a control program */
    tvmode = 1;                 /* is present and has remapped the screen */
  }
  else if (retracemode == 0) ;  /* No refresh wait for these situations */
  else
  {
    /* Wait till screen is refreshing */
    while ((inportb (RETRACE) & 0x8) != 0) ; /* Wait till refresh is through */
    while ((inportb (RETRACE) & 0x8) == 0) ;
    outportb (0x3d8, 0x25);     /* Turn off the screen refresh */
  }
  /* Construct a pointer to video memory */
  (void *) screen = MK_FP (scrseg, scroff);
}

/* E N D S C R E E N -- End a direct screen addressing "transaction" */

void EndScreen (void)
{
  __dpmi_regs r;

  /* Values to turn the screen back on */
  static BYTE modeset[] =       /* after video refresh is turned off */
  { 0x2C, 0x28, 0x2D, 0x29, 0x2A, 0x2E, 0x1E, 0x29,
  };

  /* If control program video update required */
  if (tvmode == 1 && scrchars > 0)
  {
    r.x.es = scrseg;            /* Point ES:DI to beginning of video memory */
    r.x.di = scroff;            /*  to update */
    r.x.cx = scrchars;          /* CX holds the number of chars to update */
    r.h.ah = 0xFF;              /* Use function FF of BIOS video interrupt */
    __dpmi_int (0x10, &r);      /*  to copy video buffer to screen */
  }
  else if (retracemode == 0)    /* Situations screen was not turned off */
    ;
  else
  {                             /* Else if screen is off turn it back on */
    outportb (0x3d8, modeset[vidmode]);
  }
}

/* W R I T E O N E C H A R -- writes on character to video memory      */
/* NOTE: this function does not shut down video refresh in order       */
/* to avoid snow on CGA, so it is *much* faster than using StartScreen */
/* and an EndScreen transaction for one character screen writes.       */

void WriteOneChar (BYTE c, register int row, register int col)
{
  __dpmi_regs r;

  unsigned vscroff, vscrseg;
  int retrace_wait = 0;

  if (vidmode == 7)
  {                             /* If in Monochrome video mode     */
    scrseg = 0xb000;            /* Assume video segment of B000 */
  }
  else
  {                             /* If other mode then assumme */
    scrseg = 0xb800;            /* video memory segment is B800 */
  }

  scroff = ((row * columns) + col) * 2; /* Calculate offset from beginning of */
  /* screen memory */

  savebp = r.x.bp;              /* Save reg BP, in case old BIOS trashes it */
  /* Query the address of video memory for */
  r.x.es = scrseg;              /* this process under Windows, DesQview, ... */
  r.x.di = 0;                   /* ES:DI = assumed screen address */
  r.h.ah = 0xFE;                /* Issue Video interrupt function FE */
  __dpmi_int (0x10, &r);        /*  in order to check for control program */
  vscroff = r.x.di;             /* ES:DI now holds actual address of screen */
  vscrseg = r.x.es;
  r.x.bp = savebp;              /* Restore register BP */

  tvmode = 0;                   /* Assume no control program */
  if (vscroff != 0 || vscrseg != scrseg)
  {
    scrseg = vscrseg;           /* If address of screen is different from */
    scroff += vscroff;          /* assumed address then a control program */
    tvmode = 1;                 /* is present and has remapped the screen */
  }
  else if (retracemode == 0) ;  /* No refresh wait for these situations */
  else
  {
    retrace_wait = 1;           /* Else assume snowy CGA */
  }

  /* Construct a pointer to video memory */
  (void *) screen = MK_FP (scrseg, scroff);

  if (retrace_wait == 0)
  {                             /* If no snow then */
    *screen++ = c;              /* write character into screen memory */
    *screen = curattr;          /* write attribute into screen memory */
  }
  else
  {                             /* Else if snowy CGA then try and sneak */
    if (inportb (RETRACE) & 0x8)
    {                           /* the character and attribute into */
      *screen++ = c;            /* video memory in between video */
      *screen = curattr;        /* refreshes */
    }
    else
    {
      while (inportb (RETRACE) & 0x8) ;
      while (inportb (RETRACE) & 0x1) ;
      *screen++ = c;
      *screen = curattr;
    }
  }

  if (tvmode == 1)
  {                             /* If control program video update required */
    r.x.es = scrseg;            /* Point ES:DI to beginning of video memory */
    r.x.di = scroff;            /*  to update */
    r.x.cx = 1;                 /* CX holds the number of chars to update */
    r.h.ah = 0xFF;              /* Use function FF of BIOS video interrupt */
    __dpmi_int (0x10, &r);      /*  to copy video buffer to screen */
  }
}

/* S A V E S C R E E N -- Save the contents of the video screen */

void SaveScreen (void)
{
  StartScreen (0, 0);           /* Start the video direct access */
  movedata (scrseg, scroff,     /* Move the video memory to save buffer */
            FP_SEG (screen_save), FP_OFF (screen_save), lines * columns * 2);
  scrchars = 0;                 /* Zero characters were written to screen */
  EndScreen();                 /* End the screen access */
  interrupt10 (0x0200, 0, 0, lines << 8); /* Hide the cursor */
}

/* R E S T O R E S C R E E N -- Restore contents of the video screen */

void RestoreScreen (void)
{
  StartScreen (0, 0);           /* Start the video direct access */
  /* Move the save buffer to video memory */
  movedata (FP_SEG (screen_save), FP_OFF (screen_save), scrseg, scroff, lines * columns * 2);
  scrchars = lines * columns;   /* Number of chars written to screen */
  EndScreen();                 /* End the screen access */
  PosCurs();                   /* Reposition the cursor */
}

/* V T P R I N T F -- print a formatted string to the video screen */

int vtprintf (int row, int col, int reverse, char *strformat, ...)
{
  unsigned attr;
  va_list argptr;
  char str[132];
  char *sptr = str;

  if (reverse)
  {                             /* If reversed attribute specified */
    attr = att_reverse;
  }
  else
  {                             /* Else use normal attribute */
    attr = att_normal;
  }

  va_start (argptr, strformat); /* Start variable length argument list */
  StartScreen (row, col);       /* Start a screen update */
  scrchars = vsprintf (str, strformat, argptr); /* Format the string */
  while (*sptr != '\0')
  {                             /* Move the formatted string */
    *screen++ = *sptr++;        /* to video memory */
    *screen++ = attr;
  }
  EndScreen();                 /* End the screen update */
  va_end (argptr);              /* End the va_start */
  return (scrchars);            /* Return number of characters written */
}

/* I N T E R R U P T 1 0  -- Call on the BIOS video software interrupt */

void interrupt10 (unsigned ax, unsigned bx, unsigned cx, unsigned dx)
{
  __dpmi_regs r;
  static unsigned savebp;

  savebp = r.x.bp;              /* Save BP since some BIOS'es may destroy it */
  r.x.cx = cx;                  /* Load contents of register parameters */
  r.x.dx = dx;
  r.x.bx = bx;
  r.x.ax = ax;
  __dpmi_int (0x10, &r);        /* Issue Video interrupt */
  r.x.bp = savebp;              /* Restore BP register */
}
