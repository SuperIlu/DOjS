/*
** Delay
*/

#if defined(__MSDOS__)
#  include <dos.h>  /* for delay() */
#else
   extern void GrSleep(int msec);
#  define delay(ms) GrSleep(ms)
#endif

/*
** Versions of getch, getkey and kbhit are defined in
** the libgrx for Linux X11 and Win32
*/

extern int getch(void);
extern int getkey(void);
extern int kbhit(void);
