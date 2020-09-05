#include <stdio.h>
#include <conio.h>
#include <bios.h>
#include <dos.h>
#include <string.h>

int BCDtoDecimal (int in)
{
  int nu, rest;
  int d, result;
  int p16 = 4096; /* don't use pow() for speed */
  int p10 = 1000; /* don't use pow() for speed */

  rest = in;
  result = 0;
  for (d = 3 ; d >= 0 ; d--)
  {
    nu = (int)(rest/p16);
    rest -= nu * p16;
    result += nu * p10;
    p16 >>= 4;
    p10 /= 10;
  }
  return (result);
}

int main (void)
{
  union REGS regs;
  long  tick, lasttick = 0, key = 0;
  int   dx, sec, lastsec;

  printf ("Press Esc to quit...\n");
  do
  {
    if (kbhit())
       key = getch();

    /* get BIOS timerticks through int 0x1A/0 */
    _bios_timeofday (_TIME_GETCLOCK, &tick);

    /* get RTC time through int 0x1A/2 */
    memset (&regs, 0, sizeof(union REGS));
    regs.w.ax = 0x0200;
    int386 (0x1A, &regs, &regs);
#ifdef __WATCOMC__
    dx = regs.x.edx & 0xFF00;    /* only need seconds */
#else
    dx = regs.x.dx & 0xFF00;
#endif

    sec = dx >> 8;             /* BCD */
    sec = BCDtoDecimal (sec);  /* decimal */

    if (sec != lastsec || tick != lasttick)
       printf ("tick=%ld  seconds=%d\n", tick, sec);

    lastsec = sec;
    lasttick = tick;

    if (sec == 1)
       key = 27;

  }
  while (key != 27);
  return (0);
}
