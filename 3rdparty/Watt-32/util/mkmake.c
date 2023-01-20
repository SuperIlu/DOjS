/*
 * A simple makefile preprocessor and generator.
 *
 * This program originally by John E. Davis for the S-Lang library
 *
 * Modified for Waterloo tcp/ip by G.Vanem 1998
 * Requires S-Lang compiled with djgpp 2.01+, MinGW or MSVC.
 */

#include <stdio.h>
#include <stdlib.h>
#include <slang.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>

#include "sysdep.h"

int   verbose = 0;
char *in_makefile  = NULL;
char *out_makefile = NULL;

/* Option '-w' converts '\\' to '&'.
 *
 * This is used in './src/makefile.all' which should generate Wmake makefiles for WATCOM targets.
 * This is NOT used in './src/tests/makefile.all' which generates GNU-makefiles only.
 */
int watcom_endings = 0;

/* If this is '&' and a line ends in '\\',
 * then change line-endings to '&' for Watcom's wmake.
 */
char line_cont_ch = '\\';

#if (SLANG_VERSION < 20000)
  SLPreprocess_Type _pt, *pt;
#else
  SLprep_Type *pt;
#endif

void Usage (void)
{
  fprintf (stderr,
           "Usage: mkmake [-v] [-w] [-o file] [-d dir] makefile.all [DEF1 [DEF2 ...]]\n"
           "options:\n"
           "     -v:  be verbose\n"
           "     -w:  convert '\\' line-endings to '&' for DEF1 == WATCOM\n"
           "     -o:  write parsed `makefile.all' to file (default is stdout)\n"
           "     -d:  creates subdirectory (or subdirectories)\n"
           "     DEF1 DEF2 ... are preprocessor defines in `makefile.all'\n");
  exit (1);
}

void process_makefile (const char *in_fname, const char *out_fname)
{
  char  buf[1024], *p;
  FILE *out = stdout;
  FILE *in;

  if (verbose)
     fprintf (stdout, "in_fname = `%s', out_fname = `%s'\n", in_fname, out_fname);

  in = fopen (in_fname, "rt");
  if (!in)
  {
    fprintf (stderr, "Cannot open `%s'\n", in_fname);
    Usage();
  }

  if (out_fname)
  {
    out = fopen (out_fname, "wt");
    if (!out)
    {
      fprintf (stderr, "Cannot open `%s'\n", out_fname);
      Usage();
    }
  }

  while (fgets(buf, sizeof(buf)-1, in))
  {
    p = buf;
#if 1
    while (*p == ' ')
         p++;
#endif
    if (!SLprep_line_ok(p, pt))
       continue;

    if (line_cont_ch == '&')   /* WATCOM */
    {
      unsigned int len = strlen (p);

      while (len > 0 && (p[len-1] == '\n' || p[len-1] == '\r'))
         len--;
      if (len > 0 && p[len-1] == '\\')
         p[len-1] = '&';
    }
    if (p > buf)
       fprintf (out, "%*s", (int)(p-buf), " ");
    fputs (p, out);
  }
  if (out != stdout)
     fclose (out);
}

/*
 * Replace 'ch1' to 'ch2' in string 'str'.
 */
void str_replace (int ch1, int ch2, char *str)
{
  char *s = str;

  while (*s)
  {
    if (*s == ch1)
        *s = ch2;
    s++;
  }
}

int make_dirs (char *dir)
{
  char *p;
  int ret;

#if (SLASH == '/')
  str_replace ('\\', SLASH, dir);
#else
  str_replace ('/', SLASH, dir);
#endif

  for (p = dir + 1; *p; ++p)
  {
    if (*p == SLASH)
    {
      *p = '\0';
      ret = MKDIR (dir);
      *p = SLASH;
      if (ret != 0 && errno != EEXIST)
         return ret;
    }
  }
  return MKDIR (dir);
}

int main (int argc, char **argv)
{
  int i, ch;

  while ((ch = getopt(argc, argv, "?o:d:vw")) != EOF)
     switch (ch)
     {
       case 'o': out_makefile = optarg;
                 break;
       case 'd': make_dirs (optarg);
                 break;
       case 'v': verbose++;
                 break;
       case 'w': watcom_endings = 1;
                 break;
       case '?':
       default:  Usage();
     }

  argc -= optind;
  argv += optind;
  if (argc <= 1)
     Usage();

  in_makefile = *argv;
  argv++;
  argc--;

#if (SLANG_VERSION < 20000)
  SLprep_open_prep (&_pt);
  pt = &_pt;
  pt->preprocess_char = '@';
  pt->comment_char    = '#';
  pt->flags           = SLPREP_BLANK_LINES_OK | SLPREP_COMMENT_LINES_OK;
#else
  pt = SLprep_new();
  assert (pt != NULL);
  SLprep_set_prefix (pt, "@");
  SLprep_set_comment (pt, "#", "#");
  SLprep_set_flags (pt, SLPREP_BLANK_LINES_OK | SLPREP_COMMENT_LINES_OK);
#endif

  for (i = 0; i < argc; i++)
  {
    char *arg = strupr (argv[i]);

    if (watcom_endings && !strcmp(arg, "WATCOM"))
       line_cont_ch = '&';
    SLdefine_for_ifdef (arg);
  }

  process_makefile (in_makefile, out_makefile);
  return (0);
}

