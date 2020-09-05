/*!\file lang.l
 *
 * Foreign language lexical scanner for Watt-32 TCP/IP.
 *
 * This Flex generated lang.c must be included inside language.c.
 * This was need for Watcom C.
 *
 * Gisle Vanem <gvanem@yahoo.no> 1997
 */

%{

#if !defined(WATT32_COMPILE_LANG_C)
#error lang.c MUST be included inside language.c.
#endif

#if !defined(_w32_MISC_H)
#error Include misc.h prior to this.
#endif

#if defined(USE_LANGUAGE)  /* Rest of file */

#if defined(TEST_PROG)
  #ifndef __CYGWIN__
  #include <conio.h>
  #endif
  #define INCLINE()       fprintf (stderr, " <line %u>\n", lang.line++)
  #define DEBUG(fmt,arg)  fprintf (stderr, fmt, arg)
#else
  #define INCLINE()       lang.line++
  #define DEBUG(fmt,arg)  ((void)0)
#endif

#define BUF_SIZE    500

static int  yylex      (void);
static void AddChar    (int ch);
static void AddDecimal (const char *buf);
static void AddHexcode (const char *buf);
static void AddOctal   (const char *buf);
static void AddEntry   (void);
static void Fatal      (const char *s1, const char *s2);
static void Error      (const char *err);

static char    buf [BUF_SIZE];
static char   *ptr       = NULL;
static char   *end       = buf+BUF_SIZE-1;
static int     line_cont = 0;
static BOOL    do_ref    = TRUE;  /* We're parsing the English reference string */
static DWORD   crc_ref   = 0;     /* The CRC-32 of the English reference string */
static jmp_buf bail_out;          /* The jmp_buf for getting out of here quickly */

struct Node {
       DWORD        crc;
       const  char *string;
       struct Node *next;
     };

struct Elem {
       DWORD crc;
       const char *string;
     };

static struct Language {
       struct Node *list;
       struct Elem *array;
       char         prefix[4];     /* "en:" etc. */
       size_t       entries;
       unsigned     line;
     } lang = {
       NULL, NULL, "en", 0L, 0L
     };

/*---------------------------------------------------------------------*/

#if (W32_GCC_VERSION >= 43000)
  /*
   * Suppress warning
   *   lang.l:234:1: warning: label 'find_rule' defined but not used [-Wunused-label]
   */
  #pragma GCC diagnostic ignored  "-Wunused-label"

  W32_GCC_PRAGMA (GCC diagnostic push)
  W32_GCC_PRAGMA (GCC diagnostic ignored "-Wunused-function")
#endif


%}

/**
 * Language prefix.
 *
 * Prefix normal yacc parser interface names (yyparse, yylex, yyerror, etc),
 * as well as gratuitiously global symbol names, so we can have multiple
 * yacc generated parsers in the same program.  Note that these are only
 * the variables produced by yacc.  If other parser generators (bison,
 * byacc, etc) produce additional global names that conflict at link time,
 * then those parser generators need to be fixed instead of adding those
 * names to this list.
 */

%option prefix = "_w32_"
%option caseless
%option never-interactive
%option yywrap yylineno

%x STRING EO_LINE PREFIX

NL         \r?\n
WS         [\t ]+
HEX        (x[0-9a-fA-F]{1,2})
OCTAL      (0[0-7]{1,3})
DECIMAL    ([0-9]{1,3})
LANG       "de"|"no"|"sv"

/**
 *  Possible LANG codes are according to ISO 639.
 *
 *  Ref. http://userpage.chemie.fu-berlin.de/diverse/doc/ISO_3166.html
 *
 *  aa Afar          ba Bashkir           ca Catalan
 *  ab Abkhazian     be Byelorussian      co Corsican
 *  af Afrikaans     bg Bulgarian         cs Czech
 *  am Amharic       bh Bihari            cy Welsh
 *  ar Arabic        bi Bislama
 *  as Assamese      bn Bengali; Bangla   da Danish
 *  ay Aymara        bo Tibetan           de German
 *  az Azerbaijani   br Breton            dz Bhutani
 *
 *  el Greek         fa Persian           ga Irish
 *  en English       fi Finnish           gd Scots Gaelic
 *  eo Esperanto     fj Fiji              gl Galician
 *  es Spanish       fo Faroese           gn Guarani
 *  et Estonian      fr French            gu Gujarati
 *  eu Basque        fy Frisian
 *
 *  ha Hausa         ia Interlingua       ja Japanese
 *  he Hebrew        id Indonesian        jw Javanese
 *  hi Hindi         ie Interlingue
 *  hr Croatian      ik Inupiak
 *  hu Hungarian     is Icelandic
 *  hy Armenian      it Italian
 *                   iu Inuktitut
 *  ka Georgian                           mg Malagasy
 *  kk Kazakh        la Latin             mi Maori
 *  kl Greenlandic   ln Lingala           mk Macedonian
 *  km Cambodian     lo Laothian          ml Malayalam
 *  kn Kannada       lt Lithuanian        mn Mongolian
 *  ko Korean        lv Latvian, Lettish  mo Moldavian
 *  ks Kashmiri                           mr Marathi
 *  ku Kurdish                            ms Malay
 *  ky Kirghiz                            mt Maltese
 *                                        my Burmese
 *  na Nauru         oc Occitan
 *  ne Nepali        om (Afan) Oromo      pa Punjabi
 *  nl Dutch         or Oriya             pl Polish
 *  no Norwegian                          ps Pashto, Pushto
 *                                        pt Portuguese
 *  qu Quechua
 *
 *  rm Rhaeto-Romance  sa Sanskrit        ta Tamil
 *  rn Kirundi         sd Sindhi          te Telugu
 *  ro Romanian        sg Sangro          tg Tajik
 *  ru Russian         sh Serbo-Croatian  th Thai
 *  rw Kinyarwanda     si Sinhalese       ti Tigrinya
 *                     sk Slovak          tk Turkmen
 *  ug Uighur          sl Slovenian       tl Tagalog
 *  uk Ukrainian       sm Samoan          tn Setswana
 *  ur Urdu            sn Shona           to Tonga
 *  uz Uzbek           so Somali          tr Turkish
 *                     sq Albanian        ts Tsonga
 *  vi Vietnamese      sr Serbian         tt Tatar
 *  vo Volapuk         ss Siswati         tw Twi
 *                     st Sesotho
 *  wo Wolof           su Sundanese
 *                     sv Swedish
 *  xh Xhosa           sw Swahili
 *
 *  yi Yiddish (formerly ji)
 *  yo Yoruba
 *
 *  za Zhuang
 *  zh Chinese
 *  zu Zulu
 */

%%

<INITIAL>{                             /* get reference string */
        {WS}         /* discard */
        [#;].*{NL}   INCLINE();
        {NL}         INCLINE();
        \"           {
                       if (!line_cont)
                          ptr = buf;
                       BEGIN (STRING);
                     }
        .            {
                       unput (yytext[0]);
                       BEGIN (PREFIX);
                     }
}

<STRING>{
        \"           line_cont = 0; BEGIN (EO_LINE);
        \\a          AddChar ('\7');
        \\b          AddChar ('\b');
        \\t          AddChar ('\t');
        \\f          AddChar ('\f');
        \\e          AddChar (27);
        \\r          AddChar ('\r');
        \\n          AddChar ('\n');
        \\\\         AddChar ('\\');
        \\\"         AddChar ('\"');
        \\{HEX}      AddHexcode (yytext+2);
        \\{OCTAL}    AddOctal   (yytext+1);
        \\{DECIMAL}  AddDecimal (yytext+1);
        \\.          Fatal ("Unknown ESC code: ", yytext);
        {NL}         Fatal ("Unterminated line", NULL);
        .            AddChar (yytext[0]);
        <<EOF>>      Fatal ("Unexpected end-of-file", NULL);
}

<EO_LINE>{
        \\\\         {
                       line_cont = 1;
                       BEGIN (INITIAL);
                     }
        {NL}         {
                       *ptr = '\0';
                       ptr  = NULL;
                       if (do_ref)
                            crc_ref = crc_bytes (buf, strlen(buf));
                       else AddEntry();
                       INCLINE();
                       BEGIN (INITIAL);
                     }
        .            /* discard */
        <<EOF>>      Fatal ("Unexpected end-of-string", NULL);
}

<PREFIX>{
        {LANG}":"    {
                       DEBUG ("lang `%s',\n", yytext);
                       if (!strncmp(yytext,lang.prefix,3))
                       {
                         ptr    = buf;
                         do_ref = FALSE;
                       }
                       else
                       {
                         ptr    = NULL;
                         do_ref = TRUE;
                       }
                       BEGIN (INITIAL);
                     }
        [#;].*{NL}   INCLINE();
        {NL}         INCLINE();
        .            Fatal ("Unknown language prefix: ", yytext);
}

%%

/*---------------------------------------------------------------------*/

/**
 * terminate yylex() when EOF reached.
 */
int yywrap (void)
{
  return (1);
}

/**
 * Add a character to output buffer.
 */
static void AddChar (int ch)
{
  if (ptr)
  {
    DEBUG ("%c", ch);
    *ptr++ = ch;
    if (ptr >= end)
       Error ("string too large");
  }
}

/**
 * Add a decimal code to output buffer.
 */
static void AddDecimal (const char *buf)
{
  if (ptr)
  {
    int dec;
    sscanf (buf, "%3d", &dec);
    DEBUG ("'dec <%d>'", dec);
    *ptr++ = dec;
    if (dec > 255)
       Fatal ("Illegal decimal", NULL);
    if (ptr >= end)
       Error ("String too large");
  }
}

/**
 * Add a hex code to output buffer.
 */
static void AddHexcode (const char *buf)
{
  if (ptr)
  {
    WORD hex;
    sscanf (buf, "%2hx", &hex);
    DEBUG ("'hex <%X>'", hex);
    *ptr++ = loBYTE (hex);
    if (ptr >= end)
       Error ("string too large");
  }
}

/**
 * Add an octal code to output buffer.
 */
static void AddOctal (const char *buf)
{
  if (ptr)
  {
    unsigned oct;
    sscanf (buf, "%3o", &oct);
    DEBUG ("'oct <%o>'", oct);
    *ptr++ = loBYTE (oct);
    if (ptr >= end)
       Error ("string too large");
  }
}

/**
 * Add an language entry for translation.
 */
static void AddEntry (void)
{
  const  char *str  = NULL;
  struct Node *list = NULL;

  if ((str = strdup(buf)) == NULL ||
      (list = calloc(sizeof(*list),1)) == NULL)
  {
    (*_printf) ("Language file too big!\n");
    DO_FREE (str);
    DO_FREE (list);
    longjmp (bail_out, 1);
  }

  list->next   = lang.list;
  list->crc    = crc_ref;
  list->string = str;
  lang.list    = list;
  lang.entries++;
  do_ref = TRUE;
}

/**
 * Comparision routine needed by quick-sort and bsearch routines.
 * Don't use unsigned arithmetic.
 */
static int MS_CDECL Compare (const struct Elem *a, const struct Elem *b)
{
  return ((long)a->crc - (long)b->crc);
}

/**
 * Sort the linked list on accending order of CRC value.
 */
static int SortLanguageEntries (size_t num)
{
  struct Node *lng, *next;
  struct Elem *array = calloc (num, sizeof(*array));

  lang.array = array;
  if (!array)
     return (0);

  for (lng = lang.list; lng; lng = next, array++)
  {
    array->crc    = lng->crc;
    array->string = lng->string;
    next = lng->next;
    free (lng);
  }
  lang.list = NULL;
  qsort (lang.array, num, sizeof(*array), (CmpFunc)Compare);
  return (1);
}

/*
 * _w32_lex_destroy() isn't generated in all Flex'es. So make our own.
 */
static void __lex_destroy (void)
{
  /* Old Flex doesn't have this. But probably only on MSDOS where
   * freeing memory isn't so imporant.
   */
#if defined(YY_CURRENT_BUFFER_LVALUE)
  /* Pop the buffer stack, destroying each element. */
  while (YY_CURRENT_BUFFER)
  {
   _w32__delete_buffer (YY_CURRENT_BUFFER);
   YY_CURRENT_BUFFER_LVALUE = NULL;
   _w32_pop_buffer_state();
  }

  /* Destroy the stack itself. */
  free (yy_buffer_stack);
  yy_buffer_stack = NULL;
#else
  yy_delete_buffer (YY_CURRENT_BUFFER);
#endif
}

/*
 * Free memory allocated above.
 */
static void W32_CALL lang_exit (void)
{
  struct Elem *elem;
  size_t i;

  if (crc_table)
     free (crc_table);
  crc_table = NULL;

  if (!lang.array)
     return;

  for (i = 0, elem = lang.array; i < lang.entries; i++, elem++)
      free ((void*)elem->string);
  free (lang.array);
  lang.array = NULL;
  lang.entries = 0;
  __lex_destroy();
}

/**
 * Initialize the linked list for language.
 *
 * The wattcp.cfg entry has format LANGUAGE = lang,language-file
 * e.g. "LANGUAGE = de,$(ETC)\language.txt"
 *
 * Language-code MUST be two characters (see ISO 639 list above).
 *
 * May be called multiple times to extend language entries from
 * more than 1 file.
 */
void lang_init (const char *value)
{
  FILE *langf;
  char *s, *fname;

  if (!value)
     return;

  s = strchr (value, ',');
  if (!s || (s - value) != 2)
  {
    Fatal ("Illegal LANGUAGE syntax", NULL);
    return;
  }

  lang.line = 1;
  lang.prefix[0] = value[0];
  lang.prefix[1] = value[1];
  lang.prefix[2] = ':';
  lang.prefix[3] = '\0';
  strlwr (lang.prefix);

  if (!strnicmp(lang.prefix,"en",2))  /* english is default language */
     return;

  if (!crc_init())
  {
    Fatal ("CRC init failed", NULL);
    return;
  }

  fname = ++s;
  if (!FOPEN_BIN(langf,fname))
  {
    Fatal ("Cannot open language file ", fname);
    return;
  }

  if (!setjmp(bail_out))
  {
    yyin = langf;
    yylex();

    if (!SortLanguageEntries(lang.entries))
    {
      FCLOSE (langf);
      Fatal ("Could not sort LANGUAGE entries", NULL);
      return;
    }
#if defined(TEST_PROG)
    printf ("file-name    = `%s'\n",   fname);
    printf ("lang.prefix  = `%s'\n",   lang.prefix);
    printf ("lang.entries = %u\n",     lang.entries);
    printf ("lang.lines   = %u\n",     lang.line);
    printf ("lang.list    = %08lX\n",  (DWORD)lang.list);
    printf ("lang.array   = %08lX\n\n",(DWORD)lang.array);

    {
      const struct Elem *elem;
      size_t i;

      for (i = 0, elem = lang.array; i < lang.entries; i++, elem++)
          printf ("CRC = %08lX, <%s>\n", elem->crc, elem->string);
    }
#endif
  }

  FCLOSE (langf);
  RUNDOWN_ADD (lang_exit, 302);
}

/**
 * Return foreign translation of string `str'.
 */
const char *lang_xlate (const char *str)
{
  const struct Elem *elem;
  DWORD  hash;
  size_t len;

  if (!lang.entries || !str)
     return (str);

  len = strlen (str);
  if (len == 0)
     return (str);

  hash = crc_bytes (str, len);

  elem = (const struct Elem*) bsearch (&hash, lang.array, lang.entries,
                                       sizeof(*elem), (CmpFunc)Compare);

  if (!elem || !elem->string || !elem->string[0])
     return (str);

  return (elem->string);
}

/**
 * Print message and don't use any lang_xlate() again.
 */
static void Fatal (const char *s1, const char *s2)
{
  (*_printf) ("\n");
  if (lang.line > 0)
    (*_printf) ("line %u: ", lang.line);
  (*_printf) ("%s%s\n", s1, s2 ? s2 : "");
  lang_exit();
}

/**
 * Print error message for current line
 */
static void Error (const char *err)
{
  (*_printf) ("%s at line %u\n", err, lang.line);
}

#if defined(TEST_PROG)
int main (void)
{
  puts ("You must define a non-english language to parse the language-file.\n"
        "Use e.g: \"language = no,$(ETC)\\watlang.txt\" in WATTCP.CFG\n"
        "Press any key to initialise Watt-32.");
  getch();
  sock_init();
  return (0);
}
#endif  /* TEST_PROG    */
#endif  /* USE_LANGUAGE */
