/*
 * A simple Windows-only test program for:
 *  - BugTrap reporting. (MSVC only)
 *  - GUI MessageBox() on assert and abort().
 *  - Test StackWalker in combination with:
 *     - leak detection; CrtDbg (with MSVC) or Fortify for all.
 *     - exception reporting.
 */

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <tchar.h>
#include <tcp.h>

/* to-do: add all printf's to a buffer and write all text to
 *        one MessageBox window.
 */
#if defined(IS_GUI)
  #undef  printf
  #define printf gui_printf
#endif

int do_size_fail, do_fail, do_except, do_abort, do_leak, do_bugtrap;

void show_help (const char *argv0)
{
  printf ("Usage: %s [-abdfehlsv]\n"
          "\t -a: abort() test.\n"
          "\t -b:  BugTraq test (MSVC only).\n"
          "\t -d:  enable debug mode.\n"
          "\t -f:  assert() fail test.\n"
          "\t -e:  exception test.\n"
          "\t -h?: this help message.\n"
          "\t -l:  memory leak test.\n"
          "\t -s:  sock_init() size fail.\n"
          "\t -v:  show version\n", argv0);
  exit (0);
}

int MS_CDECL _tmain (int argc, TCHAR **argv)
{
  int ch;

  if (!strstr(wattcpBuildCC(), "_MSC_VER")  &&
      !strstr(wattcpBuildCC(), "__clang__") &&
      !strstr(wattcpBuildCC(), "__BORLANDC__"))
     printf ("This program is more useful with Visual-C, Clang or CBuilder.\n");

  while ((ch = getopt(argc, argv, "abdfeh?lsv")) != EOF)
        switch (ch)
        {
          case 'a': do_abort = 1;
                    break;
          case 'b': do_bugtrap = 1;
                    break;
          case 'd': printf ("Debug-mode enabled.");
                    dbug_init();
                    break;
          case 'e': do_except = 1;
                    break;
          case 'f': do_fail = 1;
                    break;
          case '?':
          case 'h': show_help (argv[0]);
                    break;
          case 'l': do_leak = 1;
                    break;
          case 's': do_size_fail = 1;
                    break;
          case 'v': printf ("%s (%s)\n", wattcpVersion(), wattcpBuildCC());
                    break;
        }

  if (!do_size_fail && !do_fail && !do_except && !do_abort && !do_leak && !do_bugtrap)
     show_help (argv[0]);

  if (do_size_fail)
       watt_sock_init (sizeof(tcp_Socket)-1, sizeof(udp_Socket)-1, sizeof(long));
  else sock_init();

  if (do_fail)
     assert_fail_test();
  if (do_abort)
     abort_test();
  if (do_except)
     except_test();
  if (do_leak)
     leak_test();

  return (0);
}

#if defined(IS_GUI)
#if defined(__CYGWIN__)
  extern int    _argc;
  extern char **_argv;
  #define __argc  _argc
  #define __targv _argv
#endif

int WINAPI _tWinMain (HINSTANCE instance, HINSTANCE prevInstance,
                      LPTSTR cmdLine, int cmdShow)
{
  (void) instance;
  (void) prevInstance;
  (void) cmdLine;
  (void) cmdShow;

  return _tmain (__argc, __targv);
}
#endif
