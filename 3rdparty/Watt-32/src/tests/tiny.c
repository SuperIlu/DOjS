/*
 * Link with this module to reduce size of djgpp executables
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/nearptr.h>
#include <dpmi.h>
#include <crt0.h>

/*
 * To make `finger @foo' work if file `foo' is present.
 * Check using uninitialised memory.
 */
int _crt0_startup_flags =
      _CRT0_FLAG_DISALLOW_RESPONSE_FILES |
      _CRT0_FLAG_FILL_SBRK_MEMORY |
      _CRT0_FLAG_FILL_DEADBEEF |
      _CRT0_FLAG_LOCK_MEMORY;
   /* _CRT0_FLAG_NEARPTR */
   /* _CRT0_FLAG_UNIX_SBRK */

char **__crt0_glob_function (char *arg)
{
  (void)arg;
  return (char**)0;
}

void __crt0_load_environment_file (char *app_name)
{
  if (_crt0_startup_flags & _CRT0_FLAG_NEARPTR)
  {
    if (!__djgpp_nearptr_enable())
         fprintf (stderr, "Failed to enable near-pointers (DPMI err %d)\n",
                  __dpmi_error);
    else fprintf (stderr, "Enabled near-pointers okay\n");
  }
  (void)app_name;
  return;
}

