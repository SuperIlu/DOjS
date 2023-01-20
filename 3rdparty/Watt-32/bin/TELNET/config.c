#include <stdarg.h>
#include <time.h>
#include <setjmp.h>
#include <dos.h>
#include <io.h>

#include "telnet.h"
#include "config.h"

static jmp_buf     bail_out;
static FILE       *log, *dump;
static char       *iniBuffer;
static char       *tok;
static const char *iniFile;

struct TelConfig cfg = { { LIGHTGRAY,BLACK,
                           LIGHTGRAY,BLACK,
                           LIGHTGRAY,BLACK,
                           BLACK,CYAN
                         },
                         NULL,             /* log-file name           */
                         NULL,             /* dump-file name          */
                         80,25,            /* screen width/height     */
                         0,                /* status-line row (0=off) */
                         10                /* connect timeout         */
                       };

static void  ParseIniFile (char *buf, char *end);
static char *NextToken    (void);
static void  FreeIniBuf   (void);
static void  UnknownTok   (char *token);
static void  ExitLogger   (void);


/*-----------------------------------------------------------------*/

int OpenIniFile (const char *name)
{
  FILE *fil;
  char *p;
  char  fname [_MAX_PATH];
  long  alloc;

  iniFile = name;
  sprintf (fname, "%s%c%s", progPath, SLASH, name);
  fil = fopen (fname, "rt");
  if (!fil)
  {
    fprintf (stderr, "Cannot open `%s'\n", fname);
    return (0);
  }
  alloc = 2 * filelength (fileno(fil));
  iniBuffer = calloc (alloc, 1);
  if (!iniBuffer)
  {
    fprintf (stderr, "No memory to process `%s'\n", fname);
    return (0);
  }

  p = iniBuffer;
  atexit (FreeIniBuf);

  while (!feof(fil) && p < iniBuffer + alloc)
  {
    char *env, *c;

    fgets (p, alloc - (int)(p-iniBuffer), fil);
    env = _w32_expand_var_str ((char*)p);
    if (env != p)
       strcpy (p, env);

    c = strchr (p, '#');  if (c) { p = c; *p = '\n'; }
    c = strchr (p, ';');  if (c) { p = c; *p = '\n'; }
    c = strchr (p, '\n'); if (c) p = c+1;
    while (*p == ' ') p--;  /* strip trailing spaces */
  }
  fclose (fil);
  if (!setjmp(bail_out))
     ParseIniFile (iniBuffer, p);

#if 0
  {
    struct Colour    *col = &cfg.colour;
    struct LoginData *login;

    printf ("log_file  = %s\n",    cfg.log_file);
    printf ("dump_file = %s\n",    cfg.dump_file);
    printf ("user_text = %d/%d\n", col->user_fg,col->user_bg);
    printf ("data_text = %d/%d\n", col->data_fg,col->data_bg);
    printf ("warn_text = %d/%d\n", col->warn_fg,col->warn_bg);
    printf ("stat_text = %d/%d\n", col->stat_fg,col->stat_bg);

    for (login = cfg.login; login; login = login->next)
    {
      printf ("default %d, machine \"%s\": (%s,%s,%s), term %.7s, keys %.3s\n",
              login->def, login->host, login->user,
              login->pass,login->acct,
              TermName(login->term), KeyMapName(login->keys));
    }
    exit (0);
  }
#endif

  return (1);
}

/*-----------------------------------------------------------------*/

static void ParseIniFile (char *buf, char *end)
{
  enum   Sections  section = General;
  struct Colour    *col    = &cfg.colour;

  while (buf < end)
  {
    buf = tok = strtok (buf," \t\n");
    if (!buf)
       break;

    if (!stricmp(tok,"[general]"))
            section = General;

    else if (!stricmp(tok,"[colours]"))
            section = Colours;

    else if (!stricmp(tok,"[KeyMap-ANSI]"))
            section = KeyMapANSI;

    else if (!stricmp(tok,"[KeyMap-VT]"))
            section = KeyMapVTERM;

    else if (!stricmp(tok,"[KeyMap-SCO]"))
            section = KeyMapSCO;

    else if (section == General)
    {
      if (!stricmp(tok,"log_file"))
              cfg.log_file = NextToken();

      if (!stricmp(tok,"dump_file"))
              cfg.dump_file = NextToken();

      else if (!stricmp(tok,"status_line"))
              cfg.status_line = atoi (NextToken());

      else if (!stricmp(tok,"connect_timeout"))
              cfg.timeout = atoi (NextToken());
    }

    else if (section == Colours)
    {
      if (!stricmp(tok,"user_text"))
              sscanf (NextToken(), "%d,%d", &col->user_fg, &col->user_bg);

      else if (!stricmp(tok,"data_text"))
              sscanf (NextToken(), "%d,%d", &col->data_fg, &col->data_bg);

      else if (!stricmp(tok,"warn_text"))
              sscanf (NextToken(), "%d,%d", &col->warn_fg, &col->warn_bg);

      else if (!stricmp(tok,"stat_text"))
              sscanf (NextToken(), "%d,%d", &col->stat_fg, &col->stat_bg);

      else UnknownTok (tok);
    }

#if 0
    else if (section == HostInfo)
    {
      if (!stricmp(tok,"default"))
      {
        login = (struct LoginData*) AllocLogin();
        login->def  = 1;
        login->host = "*";
        CheckMachine ("*");
      }
      else if (!stricmp(tok,"VT100"))
      {
        CheckTerminal (tok);
        login->term = VT100;
      }
      else if (!stricmp(tok,"VT102"))
      {
        CheckTerminal (tok);
        login->term = VT102;
      }
      else if (!stricmp(tok,"VT200"))
      {
        CheckTerminal (tok);
        login->term = VT200;
      }
      else if (!stricmp(tok,"VT52"))
      {
        CheckTerminal (tok);
        login->term = VT52;
      }
      else if (!stricmp(tok,"ANSI"))
      {
        CheckTerminal (tok);
        login->term = ANSI;
      }
      else if (!stricmp(tok,"HEATH19"))
      {
        CheckTerminal (tok);
        login->term = HEATH19;
      }

      else if (!stricmp(tok,"VT"))
      {
        CheckKeyMap (tok);
        login->keys = VT_keys;
      }
      else if (!stricmp(tok,"IBM"))
      {
        CheckKeyMap (tok);
        login->keys = IBM_keys;
      }
      else if (!stricmp(tok,"SCO"))
      {
        CheckKeyMap (tok);
        login->keys = SCO_keys;
      }

      else if (!stricmp(tok,"machine"))
      {
        login = (struct LoginData*) AllocLogin();
        login->host = NextToken();
      }
      else if (!stricmp(tok,"login"))
              login->user = NextToken();
      else if (!stricmp(tok,"password"))
              login->pass = NextToken();
      else UnknownTok (tok);
    }
#endif
    buf = 1 + strchr (tok,0);
  }
}

/*-----------------------------------------------------------------*/

static void FreeIniBuf (void)
{
  if (iniBuffer)
  {
    free (iniBuffer);
    iniBuffer = NULL;
  }
}

static char *NextToken (void)
{
  tok = strtok (NULL, "= \t\n");
  if (!tok)
     longjmp (bail_out, 1);
  return (tok);
}

static void UnknownTok (char *token)
{
  fprintf (stderr, "%s: Unknown keyword `%s'\n", iniFile, token);
  delay (500);
}

#if 0
static void CheckMachine (char *host)
{
  struct LoginData *login;
  int    num = 0;

  for (login = cfg.login; login; login = login->next)
      if (!stricmp(login->host,host))
         num++;

  if (num > 1)
  {
    if (host[0] == '*')
         fprintf (stderr, "%s: default login multiple defined\n", iniFile);
    else fprintf (stderr, "%s: machine `%s' multiple defined\n", iniFile, host);
    sleep (1);
  }
}

static void CheckTerminal (char *tok)
{
  if (!strnicmp(tok,"ANSI",  4) &&
      !strnicmp(tok,"VT52",  4) &&
      !strnicmp(tok,"VT100", 5) &&
      !strnicmp(tok,"VT102", 5) &&
      !strnicmp(tok,"VT200", 5) &&
      !strnicmp(tok,"HEATH19",6))
  {
    fprintf (stderr, "Illegal terminal `%s'\n", tok);
    longjmp (bail_out, 1);
  }
}

static void CheckKeyMap (char *tok)
{
  if (!strnicmp(tok,"VT", 2) &&
      !strnicmp(tok,"IBM",3) &&
      !strnicmp(tok,"SCO",3))
  {
    fprintf (stderr, "Illegal keymapping `%s'\n", tok);
    longjmp (bail_out, 1);
  }
}

static char *TermName (int term)
{
  switch (term)
  {
    case ANSI:
         return ("ANSI");
    case VT52:
         return ("VT52");
    case VT100:
         return ("VT100");
    case VT102:
         return ("VT102");
    case VT200:
         return ("VT200");
    case HEATH19:
         return ("HEATH19");
    default:
         return ("Uknown");
  }
}

static char *KeyMapName (int keymap)
{
  switch (keymap)
  {
    case VT_keys:
         return ("VT");
    case IBM_keys:
         return ("IBM");
    case SCO_keys:
         return ("SCO");
    default:
         return ("Unknown");
  }
}


/*-----------------------------------------------------------------*/

static void *AllocLogin (void)
{
  struct LoginData *login = calloc (sizeof(*login), 1);
  if (!login)
  {
    fprintf (stderr, "%s: No memory for login data\n", iniFile);
    longjmp (bail_out, 1);
  }
  login->next = cfg.login;
  cfg.login   = login;
  return (void*)login;
}

/*-----------------------------------------------------------------*/

int GetUserPass (struct URL *url)
{
  struct LoginData *login = NULL;
  struct LoginData *def   = NULL;

  for (login = cfg.login; login; login = login->next)
  {
    if (login->def)
    {
      def = login;
      continue;
    }
    if (!stricmp(login->host,url->host))
       break;
  }

  if (def && !login)
     login = def;
  if (login)
  {
    if (login->user)
         Strncpy (url->user, login->user, sizeof(url->user));
    else url->user[0] = 0;

    if (login->pass)
         Strncpy (url->pass, login->pass, sizeof(url->pass));
    else url->pass[0] = 0;

    if (login->acct)
         Strncpy (url->acct, login->acct, sizeof(url->acct));
    else url->acct[0] = 0;
  }
  return (login != NULL);
}

static char *Strncpy (char *dest, const char *src, int len)
{
  len = min (len, strlen(src));
  memcpy (dest, src, len);
  dest [len] = '\0';
  return (dest);
}
#endif

/*-----------------------------------------------------------------------*/

int tel_log_init (void)
{
  if (!cfg.log_file)
     return (1);

  if (cfg.log_file[0] == '~' &&    /* "~/" -> home-directory */
      cfg.log_file[1] == '/')
  {
    static char fname [_MAX_PATH];
    sprintf (fname, "%s%c%s", progPath, SLASH, cfg.log_file+2);
    cfg.log_file = fname;
  }

  log = fopen (cfg.log_file, "at");
  if (log && fputs("\n",log) != EOF)
  {
    tel_log ("%s started", progName);
    atexit (ExitLogger);
    return (1);
  }
  printf ("Cannot open `%s'\n", cfg.log_file);
  return (0);
}

int tel_log (const char *fmt, ...)
{
  if (log)
  {
    char    buf[300];
    va_list args;
    time_t  now;

    time (&now);
    va_start (args, fmt);
    VSPRINTF (buf, fmt, args);
    va_end (args);
    return fprintf (log, "%.24s:  %s\n", ctime(&now), buf);
  }
  return (0);
}

void tel_log_flush (void)
{
  if (log)
     fflush (log);
}

static void ExitLogger (void)
{
  if (log)
  {
    tel_log ("%s stopped", progName);
    fclose (log);
    log = NULL;
  }
}

static void ExitDumper (void)
{
  if (dump)
  {
    fclose (dump);
    dump = NULL;
  }
}

int tel_dump_init (void)
{
  if (!cfg.dump_file || !dbug_mode)
     return (1);

  if (cfg.dump_file[0] == '~' &&    /* "~/" -> home-directory */
      cfg.dump_file[1] == '/')
  {
    static char fname [_MAX_PATH];
    sprintf (fname, "%s%c%s", progPath, SLASH, cfg.dump_file+2);
    cfg.dump_file = fname;
  }

  dump = fopen (cfg.dump_file, "wb");
  if (dump && fputs("\r\n",dump) != EOF)
  {
    atexit (ExitDumper);
    return (1);
  }
  printf ("Cannot open `%s'\n", cfg.dump_file);
  return (0);
}

int tel_dump (const char *fmt, ...)
{
  if (dump)
  {
    char    buf[300];
    int     len;
    va_list args;

    va_start (args, fmt);
    len = VSPRINTF (buf, fmt, args);
    va_end (args);
    return fwrite (buf, len, 1, dump);
  }
  return (0);
}

void tel_dump_hex (const char *title, const BYTE *hexbuf, unsigned len)
{
  unsigned i;

  if (!dump)
     return;

  fputs ("\r\n", dump);
  fputs (title, dump);
  for (i = 0; len > 0; len--, i++)
  {
    fprintf (dump, " %02X", hexbuf[i]);
    if ((len % 16) == 0)
       fputc ('\n', dump);
  }
  fputs ("\r\n", dump);
}

