/*!\file qmsg.c
 *
 * Functions for debug messages.
 * Can safely be used inside interrupt handlers or callbacks.
 */

#include <stdio.h>

#include "copyrigh.h"
#include "wattcp.h"
#include "misc.h"
#include "powerpak.h"
#include "x32vm.h"

#if defined(__MSDOS__)
/*
 * Disable stack-checking here because interrupt handlers might not
 * setup a legal stack.
 */
#include "nochkstk.h"

#define SCR_WIDTH    80              /** \todo detect real values */
#define SCR_HEIGHT   50
#define SCR_SEGMENT  0xB800
#define SCR_START   (2*2*SCR_WIDTH)  /* start at line 3 */

BYTE dbg_colour   = 14 + (16*1);     /* yellow on blue */
WORD dbg_scrpos   = SCR_START;
WORD dbg_scrstart = SCR_START;

static int curr_line = (SCR_START/(2*SCR_WIDTH));

void dputch (char x)
{
  if (x == '\r')
  {
    dbg_scrpos = 2 * SCR_WIDTH * curr_line;
  }
  else if (x == '\n')
  {
    dbg_scrpos += 2 * SCR_WIDTH;
    curr_line++;
  }
  else
  {
    POKEW (SCR_SEGMENT, dbg_scrpos, x | (dbg_colour << 8));
    dbg_scrpos += 2;
  }
  if (dbg_scrpos > 2*SCR_HEIGHT*SCR_WIDTH)
      dbg_scrpos = dbg_scrstart;
  if (curr_line >= SCR_HEIGHT)
      curr_line = (SCR_START/(2*SCR_WIDTH));
}

void dmsg (const char *s)
{
  while (s && *s)
    dputch (*s++);
}

void dhex1int (int x)
{
  x &= 0x0F;
  if (x > 9)
       x = 'A' + x - 10;
  else x += '0';
  dputch ((char)x);
}

void dhex2int (int x)
{
  x &= 0xFF;
  dhex1int (x >> 4);
  dhex1int (x);
}

void dhex4int (int x)
{
  x &= 0xFFFF;
  dhex2int (x >> 8);
  dhex2int (x & 0xFF);
}

void dhex8int (DWORD x)
{
  dhex4int ((int)(x >> 16));
  dhex4int ((int)(x & 0xFFFF));
  dputch (' ');
}
#endif  /* __MSDOS__ */

