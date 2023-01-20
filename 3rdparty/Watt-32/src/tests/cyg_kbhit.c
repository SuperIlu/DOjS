/*
 * How hard should a 'kbhit()' for Cygwin be?
 * Stolen from:
 *   https://rosettacode.org/wiki/Keyboard_input/Obtain_a_Y_or_N_response#C
 *
 * and adapted for '__USE_W32_SOCKETS'
 */
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>

#if !defined(__USE_W32_SOCKETS)

void set_mode (int want_key)
{
  static struct termios old, new;

  if (!want_key)
  {
    tcsetattr (STDIN_FILENO, TCSANOW, &old);
    return;
  }
  tcgetattr (STDIN_FILENO, &old);
  new = old;
  new.c_lflag &= ~(ICANON);
  tcsetattr (STDIN_FILENO, TCSANOW, &new);
}

int get_key (int no_timeout)
{
  struct timeval tv = { 0, 0 };
  int c = 0;
  fd_set fs;

  FD_ZERO (&fs);
  FD_SET (STDIN_FILENO, &fs);

  select (STDIN_FILENO + 1, &fs, 0, 0, no_timeout ? 0 : &tv);
  if (FD_ISSET(STDIN_FILENO, &fs))
  {
    c = getchar();
    set_mode (0);
  }
  return (c);
}

int kbhit (void)
{
  set_mode (1);
  return get_key (0);
}

#else
#include <windows.h>

/*
 * Copied from ../winmisc.c
 */
static HANDLE stdin_hnd = INVALID_HANDLE_VALUE;

static int is_real_key (const INPUT_RECORD *k)
{
  if (k->EventType != KEY_EVENT)
     return (0);

  if (k->Event.KeyEvent.bKeyDown)
  {
    switch (k->Event.KeyEvent.wVirtualKeyCode)
    {
      case VK_SHIFT:
      case VK_CONTROL:
      case VK_MENU:       /* Alt */
           return(0);
    }
    return (1);
  }
  return (0);
}

int kbhit (void)
{
  INPUT_RECORD r;
  DWORD num;

  if (stdin_hnd == INVALID_HANDLE_VALUE)
  {
    stdin_hnd = GetStdHandle (STD_INPUT_HANDLE);
    return kbhit();
  }

  while (1)
  {
    PeekConsoleInput (stdin_hnd, &r, 1, &num);
    if (num == 0)
       break;
    if (is_real_key(&r))
       break;
    ReadConsoleInput (stdin_hnd, &r, 1, &num);
  }
  return (num);
}
#endif
