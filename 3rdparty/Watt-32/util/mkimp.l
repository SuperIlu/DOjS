/*
 * Lexical scanner for mkimp tool.
 *
 * Gisle Vanem 2003
 */

%{

/*
generate:
  @if opt == --stub
  {
    @write to stdout
      static ret_type (*__imp_func) (args ...);

      ret_type func (args...)
      {
        load_dxe();
        @if ret_type == void
          (*__imp_func) (@arg-names...);
        @else
          return (*__imp_func) (@arg-names...);
        @endif
      }
  }
  @else if opt == --load
  {
    @write to stdout
      __imp_func = get_symbol (handle, "func");
  }
  @else if opt == --dxe
  {
    @write to stdout
      symbol = map_put (symbol, "func", func);
  }

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <time.h>

#include "sysdep.h"
#include "mkimp.h"

#define BUF_SIZE       500
#define MAC_SIZE      5000
#define INCLUDE_DEPTH   10

char  buf     [BUF_SIZE];
char *ptr      = NULL;
char *end      = buf + BUF_SIZE - 1;
char *ret_type = NULL;
char *filename = NULL;

int   line         = 0;
int   entries      = 0;
int   num_args     = 0;
int   num_lpar     = 0;
int   num_macros   = 0;
int   if_level     = 0;
int   verbose      = 0;

%}

%x DO_COMMENT DO_EXPORT DO_ARGS   /* INITIAL state is predefined */

NL       \r?\n
WS       [\t ]+
EXPORT   "_EXPORT"{WS}
COMMENT  "/*"+[^*/{NL}]*

%%

<INITIAL>{
      "/*"           BEGIN (DO_COMMENT);
      "//".*{NL}     line++;
      {NL}           line++;
      {EXPORT}       {
                       RESET_BUF();
                       BEGIN (DO_EXPORT);
                     }
      .              ;
}

<DO_COMMENT>{
      {NL}           line++;
      "*/"           BEGIN (INITIAL);
      "/*"           Abort ("Nested comment");
      .              ;
}

<DO_EXPORT>{
      \(             {
                       num_args = 0;
                       num_lpar = 0;
                       PUTC_BUF ('(');
                       BEGIN (DO_ARGS);
                     }
      {WS}           PUTC_BUF (' ');
      .              {
                       ret_type = yytext;
                       PUTC_BUF (yytext[0]);
                     }
}

<DO_ARGS>{
      ,              {
                       if (num_lpar == 0)
                           num_args++;
                       PUTC_BUF (',');
                     }
      \(             {
                       num_lpar++;
                       PUTC_BUF ('(');
                     }
      \)             {
                       PUTC_BUF (')');
                       if (num_lpar == 0)  /* last ')' in implementation */
                       {
                         FLUSH_BUF();
                         BEGIN (INITIAL);
                       }
                       else
                         num_lpar--;
                     }
      {NL}           line++;
      {WS}           PUTC_BUF (' ');
      .              PUTC_BUF (yytext[0]);
}

<*><<EOF>>           {
                       if (YY_START != INITIAL)
                            Abort ("Unexpected end-of-file");
                       else yyterminate();
                     }

%%


/*
 * terminate yylex() when EOF reached
 */
int yywrap (void)
{
  return (1);
}

const char *state_name (int state)
{
  static char buf[20];

  switch (state)
  {
    case INITIAL:
         return ("INITIAL");
    case DO_COMMENT:
         return ("DO_COMMENT");
    case DO_EXPORT:
         return ("DO_EXPORT");
    case DO_ARGS:
         return ("DO_ARGS");
    default:
         sprintf (buf,"?? %d", state);
         return (buf);
  }
}

/*
 * Parse file and build up an EXPORT list.
 */
void parse_file (const char *fname)
{
  FILE *file = fopen (fname, "rb");

  if (!file)
     Abort ("Cannot open file `%s'", fname);

  if (verbose)
     fprintf (stderr, "\nOpening file `%s'\n", fname);

  line = 1;
  yyin = file;
  filename = (char*) fname;

  yylex();
  fclose (file);

#if defined(TEST_PROG)
  printf ("file-name = `%s'\n", filename);
  printf ("entries   = %lu\n",  entries);
  printf ("lines     = %lu\n",  line);
  {
    struct Elem *elem;
    int    i;
    for (i = 0, elem = lang.array; i < entries; i++, elem++)
        printf ("CRC = %08lX, <%s>\n", elem->crc, elem->string);
  }
#endif
}

/*
 * Print message and abort program
 */
void Abort (const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);

  if (line > 0)
       fprintf (stderr, "\n%s (%d): ", filename, line);
  else putc ('\n', stderr);
  vfprintf (stderr, fmt, args);
  va_end (args);
  putc ('\n', stderr);
  exit (1);
}

/*
 * Print debug message
 */
void Debug (const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  fprintf (stderr, "%s (%d): ", filename, line);
  vfprintf (stderr, fmt, args);
  va_end (args);
}

/*
 * Print warning/error message for current file/line
 */
void Error (const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  fprintf (stderr, "%s (%d): ", filename, line);
  vfprintf (stderr, fmt, args);
  va_end (args);
  putc ('\n', stderr);
}

void yyerror (const char *msg)
{
  Error ("Bison: %s\n", msg);
}

void sig_handler (int sig)
{
  fprintf (stderr, "%s (%d): state %s, Break",
           filename, line, state_name(YY_START));
  (void) sig;
  exit (1);
}

void Usage (const char *why, const char *prog)
{
  if (why)
     fprintf (stderr, "%s.\n", why);
  fprintf (stderr, "syntax: %s [-vh?] <command> .c-files\n", prog);
  fprintf (stderr, "   -v      verbose mode\n"
                   "   <command> is one of these:\n"
                   "     --stub  to make stub code\n"
                   "     --load  to make loader code\n"
                   "     --dxe   to make dxe code\n");
  exit (1);
}

int main (int argc, char **argv)
{
  int   i;
  int   make_stub = 0;
  int   make_load = 0;
  int   make_dxe  = 0;
  const char  *prog = argv[0];
  time_t now;

  if (argc > 1 && !strcmp(argv[1],"-v"))
  {
    verbose++;
    argc--;
    argv++;
  }

  if (argc <= 2 ||
      (argc > 1 && (!strcmp(argv[1],"-h") || !strcmp(argv[1],"-?"))))
     Usage (NULL, prog);

  if (!strcmp(argv[1],"--stub"))
     make_stub = 1;
  else if (!strcmp(argv[1],"--load"))
     make_load = 1;
  else if (!strcmp(argv[1],"--dxe"))
     make_dxe = 1;
  else
     Usage ("Illegal command", prog);

  time (&now);
  printf ("/* Generated by %s at %.24s. DO NOT EDIT */\n\n",
          prog, ctime(&now));

  signal (SIGINT, sig_handler);

  for (i = 2; i < argc && argv[i]; i++)
  {
    parse_file (argv[i]);
#if 0
    if (make_stub)
       do_make_stub();
    else if (make_load)
       do_make_loader();
    else if (make_dxe)
       do_make_dxe_init();
#endif
  }
  return (0);
}

/*
 * Remove first newlines in 'str'.
 */
char *rip (char *str)
{
  char *p;

  if ((p = strchr(str,'\n')) != NULL) *p = '\0';
  if ((p = strchr(str,'\r')) != NULL) *p = '\0';
  return (str);
}

/*
 * Remove white-space at front and end of 'str'.
 */
char *trim (char *str)
{
  char *start, *p = str;

  if (!p || strlen(p) < 1)
     return (p);

  while (*p == ' ' || *p == '\t')
        p++;

  if (!*p)
     return (p);
  start = p;
  p += strlen (p) - 1;
  while (p > start && (*p == ' ' || *p == '\t'))
        *p-- = '\0';
  return (start);
}

/*
 * Return identifier in 'str'.
 * e.g. str = " struct foo ** bar " returns "bar".
 */
char *ident (char *str)
{
  char *p = trim (str);
  char *q = strrchr (p, '*');

  if (!q)
  {
    q = strrchr (p, ' ');
    return (q ? q+1 : p);
  }
  ++q;
  return trim (q);
}

/*
 * Convert args e.g. "int foo, const char *bar" into
 * "foo, bar". Or "void" into "".
 */
char *get_arg_names (char *args)
{
  static char buf[256];
  char *p, *q = args;

  buf[0] = '\0';
  while (1)
  {
    if (verbose)
       fprintf (stderr, "buf: `%s'\n", buf);
    p = strchr (q, ',');
    if (!p)
    {
      strcat (buf, ident(q));
      break;
    }
    *p++ = '\0';
    strcat (buf, ident(q));
    strcat (buf, ",");
    q = p;
  }
  q = strrchr (buf, ',');
  if (q)
     *q = '\0';
  return (buf);
}


char *_strncpy (char *dest, const char *src, int len)
{
  if (len < 0)
     Abort ("Illegal len (%d) to _strncpy()", len);
  memcpy (dest, src, len);
  dest [len] = '\0';
  return (dest);
}

#if 0
void do_make_stub (char *str)
{
  int   non_void = strncmp (str,"void ",5) != 0;
  char *p;
  char  ret_type[256];
  char  front[256];
  char  func [256];
  char  args [256];

  if (sscanf(str,"%[^(](%[^)])",front,args) != 2)
     Abort ("Missing args (`%s')\n", str);

  trim (front);
  if (verbose)
  {
    fprintf (stderr, "string:   `%s'\n", str);
    fprintf (stderr, "args:     `%s'\n", args);
    fprintf (stderr, "front:    `%s'\n", front);
  }

  p = ident (front);
  strcpy (func, p);
  if (verbose)
     fprintf (stderr, "func:     `%s'\n", func);

  _strncpy (ret_type, front, p-front);
  trim (ret_type);
  if (verbose)
     fprintf (stderr, "ret-type: `%s'\n", ret_type);

  printf ("static %s (*__imp_%s) (%s);\n\n", ret_type, func, args);

  printf ("%s %s (%s)\n{\n  load_dxe();\n  %s(*__imp_%s)",
          ret_type, func, args, non_void ? "return " : "", func);

  printf (" (%s);\n}\n\n", get_arg_names(args));
}

void do_make_loader (char *str)
{

}

void do_make_dxe_init (char *str)
{

}
#endif

