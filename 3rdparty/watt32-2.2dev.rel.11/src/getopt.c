/*!\file getopt.c
 * Parse command-line options.
 */

/* (emx+gcc) -- Copyright (c) 1990-1993 by Eberhard Mattes
 * Adapted for Watt-32 TCP/IP by G. Vanem Nov-96
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wattcp.h"
#include "language.h"
#include "strings.h"

char *watt_optarg    = NULL;
int   watt_optopt    = 0;
int   watt_optind    = 0;      /* Default: first call */
int   watt_opterr    = 1;      /* Default: error messages enabled */
char *watt_optswchar = "-/";   /* '-' or '/' starts options */

enum  _watt_optmode watt_optmode = GETOPT_UNIX;

static char *next_opt;         /* Next character in cluster of options */
static char *empty = "";       /* Empty string */

static BOOL done;
static char sw_char;

static char **options;         /* List of entries which are options */
static char **non_options;     /* List of entries which are not options */
static int    options_count;
static int    non_options_count;

#define PUT(dst) do {                                         \
                   if (watt_optmode == GETOPT_ANY)            \
                      dst[dst##_count++] = argv[watt_optind]; \
                  } while (0)

#undef ERROR

#if defined(USE_DEBUG)
  #define ERROR(str,fmt,ch)  printf (str), printf (fmt, ch), puts ("")
#else    /* avoid pulling in printf() */
  #define ERROR(str,fmt,ch)  outsnl (str)
#endif


int W32_CALL watt_getopt (int argc, char *const *_argv, const char *opt_str)
{
  char  c;
  char *q;
  int   i, j;
  char **argv = (char **) _argv;

  if (watt_optind == 0)
  {
    watt_optind   = 1;
    done     = FALSE;
    next_opt = empty;
    if (watt_optmode == GETOPT_ANY)
    {
      options     = malloc (argc * sizeof(char*));
      non_options = malloc (argc * sizeof(char*));
      if (!options || !non_options)
      {
        outsnl ("out of memory (getopt)");
        exit (255);
      }
      options_count     = 0;
      non_options_count = 0;
    }
  }
  if (done)
     return (EOF);

restart:
  watt_optarg = NULL;
  if (*next_opt == 0)
  {
    if (watt_optind >= argc)
    {
      if (watt_optmode == GETOPT_ANY)
      {
        j = 1;
        for (i = 0; i < options_count; ++i)
            argv[j++] = options[i];
        for (i = 0; i < non_options_count; ++i)
            argv[j++] = non_options[i];
        watt_optind = options_count+1;
        free (options);
        free (non_options);
      }
      done = TRUE;
      return (EOF);
    }
    else if (!strchr (watt_optswchar, argv[watt_optind][0]) || argv[watt_optind][1] == 0)
    {
      if (watt_optmode == GETOPT_UNIX)
      {
        done = TRUE;
        return (EOF);
      }
      PUT (non_options);
      watt_optarg = argv[watt_optind++];
      if (watt_optmode == GETOPT_ANY)
         goto restart;
      /* watt_optmode==GETOPT_KEEP */
      return (0);
    }
    else if (argv[watt_optind][0] == argv[watt_optind][1] && argv[watt_optind][2] == 0)
    {
      if (watt_optmode == GETOPT_ANY)
      {
        j = 1;
        for (i = 0; i < options_count; ++i)
            argv[j++] = options[i];
        argv[j++] = argv[watt_optind];
        for (i = 0; i < non_options_count; ++i)
            argv[j++] = non_options[i];
        for (i = watt_optind+1; i < argc; ++i)
            argv[j++] = argv[i];
        watt_optind = options_count + 2;
        free (options);
        free (non_options);
      }
      ++watt_optind;
      done = TRUE;
      return (EOF);
    }
    else
    {
      PUT (options);
      sw_char  = argv[watt_optind][0];
      next_opt = argv[watt_optind] + 1;
    }
  }
  c = *next_opt++;
  if (*next_opt == 0)  /* Move to next argument if end of argument reached */
     watt_optind++;
  if (c == ':' || (q = strchr (opt_str, c)) == NULL)
  {
    if (watt_opterr)
    {
      if (c < ' ' || c >= 127)
           ERROR ("Invalid option", "; character code=0x%.2x", c);
      else ERROR ("Invalid option", " -%c", c);
    }
    watt_optopt = '?';
    return ('?');
  }
  if (q[1] == ':')
  {
    if (*next_opt != 0)         /* Argument given */
    {
      watt_optarg = next_opt;
      next_opt = empty;
      ++watt_optind;
    }
    else if (q[2] == ':')
      watt_optarg = NULL;            /* Optional argument missing */
    else if (watt_optind >= argc)
    {                           /* Required argument missing */
      if (watt_opterr)
         ERROR ("No argument for option", " `-%c'", c);
      c = '?';
    }
    else
    {
      PUT (options);
      watt_optarg = argv[watt_optind++];
    }
  }
  watt_optopt = c;
  return (c);
}


#if defined(WATT32_ON_WINDOWS)

/** \todo make a wide-char version of getopt()
 */
W32_DATA wchar_t *_w_watt_optarg;
W32_DATA wchar_t *_w_watt_optswchar;

wchar_t          *_w_next_opt;       /* Next character in cluster of options */
wchar_t          *_w_empty = L"";    /* Empty string */
wchar_t           _w_sw_char;

wchar_t          **_w_options;        /* List of entries which are options */
wchar_t          **_w_non_options;    /* List of entries which are not options */

int W32_CALL _w_watt_getopt (int argc, wchar_t *const *argv, const wchar_t *opt_str)
{
  ARGSUSED (argc);
  ARGSUSED (argv);
  ARGSUSED (opt_str);
  return (0);
}
#endif


