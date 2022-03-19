/*
 * A simple makefile preprocessor and generator.
 *
 * This program originally by John E. Davis for the S-Lang library
 *
 * Modified for Waterloo tcp/ip by G.Vanem 1998
 * Requires S-Lang compiled with djgpp 2.01+, MingW or MSVC.
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
char  line_cont_ch = '\\';  /* if this is not '\\' and a line ends in '\\' */
                            /* then change it to this. (Watcom needs '&') */
#if (SLANG_VERSION < 20000)
  SLPreprocess_Type _pt, *pt;
#else
  SLprep_Type *pt;
#endif

void Usage (void)
{
  fprintf (stderr,
           "Usage: mkmake [-v] [-ofile] [-ddir] makefile.all [DEF1 [DEF2 ...]]\n"
           "options:\n"
           "     -v:  be verbose\n"
           "     -o:  write parsed `makefile.all' to file (default is stdout)\n"
           "     -d:  creates subdirectory (or subdirectories)\n"
           "     DEF1 DEF2 ... are preprocessor defines in `makefile.all'\n");
  exit (1);
}

void process_makefile (const char *infname, const char *outfname)
{
  char  buf[1024], *p;
  FILE *out = stdout;
  FILE *in;

  if (verbose)
     fprintf (stdout, "infname = `%s', outfname = `%s'\n", infname, outfname);

  in = fopen (infname, "rt");
  if (!in)
  {
    fprintf (stderr, "Cannot open `%s'\n", infname);
    Usage();
  }

  if (outfname)
  {
    out = fopen (outfname, "wt");
    if (!out)
    {
      fprintf (stderr, "Cannot open `%s'\n", outfname);
      Usage();
    }
  }

  while (fgets(buf,sizeof(buf)-1,in))
  {
    p = buf;
#if 1
    while (*p == ' ')
         p++;
#endif
    if (!SLprep_line_ok(p,pt))
       continue;

    if (line_cont_ch != '\\')   /* WATCOM */
    {
      unsigned int len = strlen (p);

      while (len > 0 && isspace(p[len-1]))
         len--;
      if (len > 0 && p[len-1] == '\\')
         p[len-1] = line_cont_ch;
    }
    if (p > buf)
      fprintf (out, "%*s", p-buf, " ");
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
  char *p = strchr (dir, '\0');

#if (SLASH == '/')
  str_replace ('\\', SLASH, dir);
#else
  str_replace ('/', SLASH, dir);
#endif

  if (p[-1] != SLASH)
     *p++ = SLASH, *p = '\0';

  for (p = strchr(dir+1, SLASH); p; p = strchr(p+1, SLASH))
  {
    *p = '\0';
    if (MKDIR(dir) == -1)
    {
      if (errno != EEXIST)
      {
        *p = SLASH;
        return (-1);
      }
    }
    *p = SLASH;
  }
  return (0);
}

static int get_win_ver (SLprep_Type *pt, char *expr)
{
  char *p;

  p = strstr (expr, "@(get_win_ver)");

  if (verbose)
     fprintf (stdout, "expr: '%s', p: '%s'", expr, p);

  if (p)
     strcpy (p, "0x0501");
  return (1);
}

int main (int argc, char **argv)
{
  int i, ch;

  while ((ch = getopt(argc,argv,"?o:d:v")) != EOF)
     switch (ch)
     {
       case 'o': out_makefile = optarg;
                 break;
       case 'd': make_dirs (optarg);
                 break;
       case 'v': verbose++;
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
  SLprep_set_eval_hook (pt, get_win_ver);
#endif

  for (i = 0; i < argc; i++)
  {
    char *arg = strupr (argv[i]);

    if (!strcmp(arg,"WATCOM"))
       line_cont_ch = '&';
    SLdefine_for_ifdef (arg);
  }

  process_makefile (in_makefile, out_makefile);
  return (0);
}

