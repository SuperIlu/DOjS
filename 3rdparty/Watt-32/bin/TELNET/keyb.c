#include <ctype.h>
#include <signal.h>
#include <dos.h>

#include "telnet.h"
#include "keyb.h"  
#include "nochkstk.h"

#define XLAT_BUFSIZE   2048
#define MAX_XLAT_KEYS  100
#define LoByte(x)      ((BYTE)((x) & 0xFF))
#define HiByte(x)      ((BYTE)((WORD)(x) >> 8))

typedef struct {
        DWORD       key;
        const char *name;
      } XLATKEY;

static XLATKEY keyNames[] = {
               { Key_CTRL_A     ,"Ctrl-A"         },
               { Key_CTRL_B     ,"Ctrl-A"         },
               { Key_CTRL_C     ,"Ctrl-C"         },
               { Key_CTRL_D     ,"Ctrl-D"         },
               { Key_CTRL_E     ,"Ctrl-E"         },
               { Key_CTRL_F     ,"Ctrl-F"         },
               { Key_CTRL_G     ,"Ctrl-G"         },
               { Key_CTRL_H     ,"Ctrl-H"         },
               { Key_CTRL_I     ,"Ctrl-I"         },
               { Key_CTRL_J     ,"Ctrl-J"         },
               { Key_CTRL_K     ,"Ctrl-K"         },
               { Key_CTRL_L     ,"Ctrl-L"         },
               { Key_CTRL_M     ,"Ctrl-M"         },
               { Key_CTRL_N     ,"Ctrl-N"         },
               { Key_CTRL_O     ,"Ctrl-O"         },
               { Key_CTRL_P     ,"Ctrl-P"         },
               { Key_CTRL_Q     ,"Ctrl-Q"         },
               { Key_CTRL_R     ,"Ctrl-R"         },
               { Key_CTRL_S     ,"Ctrl-S"         },
               { Key_CTRL_T     ,"Ctrl-T"         },
               { Key_CTRL_U     ,"Ctrl-U"         },
               { Key_CTRL_V     ,"Ctrl-V"         },
               { Key_CTRL_W     ,"Ctrl-W"         },
               { Key_CTRL_X     ,"Ctrl-X"         },
               { Key_CTRL_Y     ,"Ctrl-Y"         },
               { Key_CTRL_Z     ,"Ctrl-Z"         },
               { Key_ESC        ,"ESC"            },
               { Key_ENTER      ,"Enter"          },
               { Key_TAB        ,"Tab"            },
               { Key_BACKSPC    ,"BackSpace"      },
               { Key_NL         ,"NewLine"        },
               { Key_LFEED      ,"LineFeed"       },
               { Key_FFEED      ,"FormFeed"       },

               { Key_F1         ,"F1",            },
               { Key_F2         ,"F2",            },
               { Key_F3         ,"F3",            },
               { Key_F4         ,"F4",            },
               { Key_F5         ,"F5",            },
               { Key_F6         ,"F6",            },
               { Key_F7         ,"F7",            },
               { Key_F8         ,"F8",            },
               { Key_F9         ,"F9",            },
               { Key_F10        ,"F10",           },
               { Key_F11        ,"F11",           },
               { Key_F12        ,"F12",           },
               { Key_CF1        ,"Ctrl-F1",       },
               { Key_CF2        ,"Ctrl-F2",       },
               { Key_CF3        ,"Ctrl-F3",       },
               { Key_CF4        ,"Ctrl-F4",       },
               { Key_CF5        ,"Ctrl-F5",       },
               { Key_CF6        ,"Ctrl-F6",       },
               { Key_CF7        ,"Ctrl-F7",       },
               { Key_CF8        ,"Ctrl-F8",       },
               { Key_CF9        ,"Ctrl-F9",       },
               { Key_CF10       ,"Ctrl-F10",      },
               { Key_CF11       ,"Ctrl-F11",      },
               { Key_CF12       ,"Ctrl-F12",      },
               { Key_SF1        ,"Shift-F1",      },
               { Key_SF2        ,"Shift-F2",      },
               { Key_SF3        ,"Shift-F3",      },
               { Key_SF4        ,"Shift-F4",      },
               { Key_SF5        ,"Shift-F5",      },
               { Key_SF6        ,"Shift-F6",      },
               { Key_SF7        ,"Shift-F7",      },
               { Key_SF8        ,"Shift-F8",      },
               { Key_SF9        ,"Shift-F9",      },
               { Key_SF10       ,"Shift-F10",     },
               { Key_SF11       ,"Shift-F11",     },
               { Key_SF12       ,"Shift-F12",     },
               { Key_AF1        ,"Alt-F1",        },
               { Key_AF2        ,"Alt-F2",        },
               { Key_AF3        ,"Alt-F3",        },
               { Key_AF4        ,"Alt-F4",        },
               { Key_AF5        ,"Alt-F5",        },
               { Key_AF6        ,"Alt-F6",        },
               { Key_AF7        ,"Alt-F7",        },
               { Key_AF8        ,"Alt-F8",        },
               { Key_AF9        ,"Alt-F9",        },
               { Key_AF10       ,"Alt-F10",       },
               { Key_AF11       ,"Alt-F11",       },
               { Key_AF12       ,"Alt-F12",       },

               { Key_INS        ,"Ins"            },
               { Key_DEL        ,"Del"            },
               { Key_HOME       ,"Home"           },
               { Key_END        ,"End"            },
               { Key_PGUP       ,"PgUp"           },
               { Key_PGDN       ,"PgDn"           },
               { Key_UPARROW    ,"Up"             },
               { Key_DNARROW    ,"Down"           },
               { Key_LTARROW    ,"Left"           },
               { Key_RTARROW    ,"Right"          },
               { Key_PADMIDDLE  ,"Pad5"           },

               { Key_PADEQ      ,"PadEq"          },
               { Key_PADPLUS    ,"Pad+"           },
               { Key_PADMINUS   ,"Pad-"           },
               { Key_PADASTERISK,"Pad*"           },
               { Key_PADSLASH   ,"Pad/"           },
               { Key_PADENTER   ,"PadEnter"       },

               { Key_CEND       ,"Ctrl-End"       },
               { Key_CDNARROW   ,"Ctrl-Down"      },
               { Key_CPGDN      ,"Ctrl-PgDn"      },
               { Key_CLTARROW   ,"Ctrl-Left"      },
               { Key_CPADMIDDLE ,"Ctrl-Pad5"      },
               { Key_CRTARROW   ,"Ctrl-Right"     },
               { Key_CHOME      ,"Ctrl-Home"      },
               { Key_CUPARROW   ,"Ctrl-Up"        },
               { Key_CPGUP      ,"Ctrl-PgUp"      },
               { Key_CINS       ,"Ctrl-Ins"       },
               { Key_CDEL       ,"Ctrl-Del"       },

               { Key_PINS       ,"Ins-"           },
               { Key_PDEL       ,"Del-"           },
               { Key_PHOME      ,"Home-"          },
               { Key_PEND       ,"End-"           },
               { Key_PPGUP      ,"PgUp-"          },
               { Key_PPGDN      ,"PgDn-"          },
               { Key_PUPARROW   ,"Up-"            },
               { Key_PDNARROW   ,"Down-"          },
               { Key_PLTARROW   ,"Left-"          },
               { Key_PRTARROW   ,"Right-"         },

               { Key_CPEND      ,"Ctrl-End-"      },
               { Key_CPDNARROW  ,"Ctrl-Down-"     },
               { Key_CPPGDN     ,"Ctrl-PgDn-"     },
               { Key_CPLTARROW  ,"Ctrl-Left-"     },
               { Key_CPRTARROW  ,"Ctrl-Right-"    },
               { Key_CPHOME     ,"Ctrl-Home-"     },
               { Key_CPUPARROW  ,"Ctrl-Up-"       },
               { Key_CPPGUP     ,"Ctrl-PgUp-"     },
               { Key_CPINS      ,"Ctrl-Ins-"      },
               { Key_CPDEL      ,"Ctrl-Del-"      },

               { Key_ALTPPLUS   ,"Alt-Pad+"       },
               { Key_ALTPMINUS  ,"Alt-Pad-"       },
               { Key_ALTPASTRSK ,"Alt-Pad*"       },
               { Key_ALTPEQUALS ,"Alt-Pad="       },
               { Key_ALTPSLASH  ,"Alt-Pad/"       },
               { Key_ALTPENTER  ,"Alt-PadEnter"   },

               { Key_ALTBACKSPC ,"Alt-BkSp"       },
               { Key_CTRLBACKSPC,"Ctrl-BkSp"      },
               { Key_SHIFTTAB   ,"Shift-Tab"      },
               { Key_CTRLTAB    ,"Ctrl-Tab"       },
               { Key_ALTESC     ,"Alt-ESC"        },

               { Key_ALT1       ,"Alt-1"          },
               { Key_ALT2       ,"Alt-2"          },
               { Key_ALT3       ,"Alt-3"          },
               { Key_ALT4       ,"Alt-4"          },
               { Key_ALT5       ,"Alt-5"          },
               { Key_ALT6       ,"Alt-6"          },
               { Key_ALT7       ,"Alt-7"          },
               { Key_ALT8       ,"Alt-8"          },
               { Key_ALT9       ,"Alt-9"          },
               { Key_ALT0       ,"Alt-0"          },
               { Key_ALTMINUS   ,"Alt--"          },
               { Key_ALTEQUALS  ,"Alt-="          },

               { Key_ALTQ       ,"Alt-Q"          },
               { Key_ALTW       ,"Alt-W"          },
               { Key_ALTE       ,"Alt-E"          },
               { Key_ALTR       ,"Alt-R"          },
               { Key_ALTT       ,"Alt-T"          },
               { Key_ALTY       ,"Alt-Y"          },
               { Key_ALTU       ,"Alt-U"          },
               { Key_ALTI       ,"Alt-I"          },
               { Key_ALTO       ,"Alt-O"          },
               { Key_ALTP       ,"Alt-P"          },
               { Key_ALTLBRACE  ,"Alt-["          },
               { Key_ALTRBRACE  ,"Alt-]"          },

               { Key_ALTA       ,"Alt-A"          },
               { Key_ALTS       ,"Alt-S"          },
               { Key_ALTD       ,"Alt-D"          },
               { Key_ALTF       ,"Alt-F"          },
               { Key_ALTG       ,"Alt-G"          },
               { Key_ALTH       ,"Alt-H"          },
               { Key_ALTJ       ,"Alt-J"          },
               { Key_ALTK       ,"Alt-K"          },
               { Key_ALTL       ,"Alt-L"          },
               { Key_ALTCOLON   ,"Alt-:"          },
               { Key_ALTQUOTE   ,"Alt-'"          },
               { Key_ALTENTER   ,"Alt-CR"         },

               { Key_ALTZ       ,"Alt-Z"          },
               { Key_ALTX       ,"Alt-X"          },
               { Key_ALTC       ,"Alt-C"          },
               { Key_ALTV       ,"Alt-V"          },
               { Key_ALTB       ,"Alt-B"          },
               { Key_ALTN       ,"Alt-N"          },
               { Key_ALTM       ,"Alt-M"          },
               { Key_ALTCOMMA   ,"Alt-,"          },
               { Key_ALTPERIOD  ,"Alt-."          },
               { Key_ALTSLASH   ,"Alt-/"          },
               { Key_ALTBSLASH  ,"Alt-\\"         },
               { Key_ALTTILDE   ,"Alt-~"          },
               { 0              ,"??"             }
             };

static XLATKEY xlatKey [MAX_XLAT_KEYS];
static char    xlatBuf [XLAT_BUFSIZE];
static int     xlatFlag [8];
static char   *xlatPtr       = xlatBuf;
static int     xlatCount     = 0;
static int     filterPadKeys = 0;

/*-------------------------------------------------------------------*/

static int Compare (const void *x1, const void *x2)
{
  return (((XLATKEY*)x1)->key - ((XLATKEY*)x2)->key);
}

static DWORD LookupKeyName (const char *s, unsigned line)
{
  const XLATKEY *k = &keyNames[0];

  for ( ; k->key; k++)
      if (!stricmp(s,k->name))
         return (k->key);

  printf ("Warning: unknown key `%s' at line %u\n", s, line);
  return (0L);
}

const char *KeyName (DWORD key)
{
  const  XLATKEY *k = &keyNames[0];
  static char buf[2];

  for ( ; k->key; k++)
      if (key == k->key)
         return (k->name);
  buf[0] = (char)key;
  buf[1] = 0;
  return (buf);
}

/*-------------------------------------------------------------------*/

static void AddKeyDefinition (DWORD key, const char *value)
{
  char *buf = xlatPtr;
  const char *max = xlatBuf + XLAT_BUFSIZE - strlen (value);
  int   i;

  if (key == 0L)
     return;

  if (xlatCount > MAX_XLAT_KEYS || xlatPtr >= max)
  {
    printf ("Warning: key-table full. Dropping key %lu (%s).\n", key, value);
    return;
  }

  for (i = 0; i < xlatCount; i++)
      if (xlatKey[i].key == key)
         break;

  xlatKey[i].key  = key;
  xlatKey[i].name = buf;

  while (*value)
  {
    if (*value >= '0' && *value <= '7')
        *buf++ = *value++ - '0';

    else if (*value == '^')
    {
      value++;
      *buf++ = *value++ - '@';
    }
    else if (*value == '\\')
    {
      int d1 = value[1];
      int d2 = value[2];
      int d3 = value[3];

      value++;
      if (isdigit(d1) && isdigit(d2) && isdigit(d3))
      {
        *buf++ = 100*(d1-'0') + 10*(d2-'0') + (d3-'0');
        value += 3;
      }
      else
        *buf++ = *value++;
    }
    else
      *buf++ = *value++;
  }
  *buf++  = 0;
  xlatPtr = buf;
  xlatCount++;
  qsort (xlatKey, xlatCount, sizeof(XLATKEY), Compare);
}

/*-------------------------------------------------------------------*/

static DWORD ungetKey = 0;

void KeyUngetKey (DWORD key)
{
  while (KeyGetKey()) /* flush buffer */
        ;
  ungetKey = key;

}

DWORD KeyGetKey (void)
{
  int   key;
  union REGS regs;

  if (ungetKey)
  {
    DWORD rc = ungetKey;
    ungetKey = 0;
    return (rc);
  }

  if (!kbhit())
     return (0);

  regs.h.ah = 0x10;

#ifdef __WATCOMC__
  int386 (0x16, &regs, &regs);
  key = regs.w.ax;
#else
  int86 (0x16, &regs, &regs);
  key = regs.x.ax;
#endif

  switch (LoByte(key))
  {
    case 0:
         key = HiByte(key) + 256;
         break;

    case 0xE0:
         key = HiByte(key) + 512;
         break;

    default:
         if ((HiByte(key) == 0xE0) ||
             ((ispunct(LoByte(key)) && (HiByte(key) > 0x36))))
              key = LoByte (key) + 512;
         else key = LoByte (key);
  }

  if (!filterPadKeys)  /* if not separating normal/pad keys */
  {
    switch (key)
    {
      #define KEY(x) key=x; break
      case Key_PADASTERISK: KEY ('*');
  //  case Key_PADPLUS    : KEY ('+');
  //  case Key_PADMINUS   : KEY ('-');
      case Key_PADSLASH   : KEY ('/');
      case Key_PADENTER   : KEY (13);
      case Key_PINS       : KEY (Key_INS);
      case Key_PDEL       : KEY (Key_DEL);
      case Key_PHOME      : KEY (Key_HOME);
      case Key_PEND       : KEY (Key_END);
      case Key_PPGUP      : KEY (Key_PGUP);
      case Key_PPGDN      : KEY (Key_PGDN);
      case Key_PUPARROW   : KEY (Key_UPARROW);
      case Key_PDNARROW   : KEY (Key_DNARROW);
      case Key_PLTARROW   : KEY (Key_LTARROW);
      case Key_PRTARROW   : KEY (Key_RTARROW);
      case Key_CPEND      : KEY (Key_CEND);
      case Key_CPDNARROW  : KEY (Key_CDNARROW);
      case Key_CPPGDN     : KEY (Key_CPGDN);
      case Key_CPLTARROW  : KEY (Key_CLTARROW);
      case Key_CPRTARROW  : KEY (Key_CRTARROW);
      case Key_CPHOME     : KEY (Key_CHOME);
      case Key_CPUPARROW  : KEY (Key_CUPARROW);
      case Key_CPPGUP     : KEY (Key_CPGUP);
      case Key_CPINS      : KEY (Key_CINS);
      case Key_CPDEL      : KEY (Key_CDEL);
    }
  }

#if 0
  {
    int x = wherex();
    int y = wherey();

    gotoxy (1, 49);
    cprintf ("key = %04Xh  %-20.20s", key, KeyName(key));
    gotoxy (x, y);
  }
#endif
  return (key);
}

int KeyFilterPad (int on)
{
  int rc = filterPadKeys;
  filterPadKeys = on;
  return (rc);
}

/*-------------------------------------------------------------------*/

int KeyDriver (const char *file)
{
  FILE *fp;
  char  line [_MAX_PATH];
  char  path [_MAX_PATH];
  WORD  lineno = 0;

  xlatCount = 0;
  xlatPtr   = xlatBuf;
  memset (&xlatFlag, 0, sizeof(xlatFlag));

  if (strchr(file,'/') || strchr(file,'\\'))
       strncpy (path, file, sizeof(path)-1);
  else sprintf (path, "%s%c%s", progPath, SLASH, file);

  fp = fopen (path, "r");
  if (!fp)
  {
    printf ("Cannot load keyboard definitions `%s'\n", path);
    return (0);
  }

  while (fgets(line,sizeof(line)-1,fp) && !feof(fp))
  {
    char *key = strtok (line, " \t");
    char *value;

    lineno++;
    if (*key == 0 || *key == ';' || *key == '#')
       continue;

    value = strtok (NULL, "= \t\r\n");
    if (!value || *key == '\r' || *key == '\n')
       continue;

    if (*value == '"')
    {
      char *s = strchr (++value, '"');
      if (s)
         *s = '\0';
      else
      {
        printf ("Warning: %s (%u): unterminated string\n", path, lineno);
        continue;
      }
    }
    if (*value == 0)
       continue;

    if (isdigit(*key))
         AddKeyDefinition (atol(key), value);
    else AddKeyDefinition (LookupKeyName(key,lineno), value);
  }
  fclose (fp);
  return (1);
}

/*-------------------------------------------------------------------*/

const char *KeyTranslate (DWORD key)
{
  const XLATKEY *x = NULL;

  if (key)
     x = (XLATKEY*) bsearch (&key, xlatKey, xlatCount, sizeof(*x), Compare);
  return (x ? x->name : NULL);
}

int KeyGetFlag (int flg)
{
  if (flg >= 0 && flg <= 7)
     return (xlatFlag[flg]);
  return (0);
}

void KeySetFlag (int flg, int val)
{
  if (flg >= 0 && flg < DIM(xlatFlag))
     xlatFlag [flg] = val;
}


/********************************************************************/

#ifdef TEST

char progPath [_MAX_PATH] = ".\\";

void sig_handler (int sig)
{
  KeyUngetKey (Key_CTRL_C);
  signal (sig, sig_handler);
}

int main (void)
{
  int i, quit = 0;

  signal (SIGINT, sig_handler);

#ifdef __HIGHC__
  InstallExcHandler (NULL);
#endif

  if (!KeyDriver("ANSI-AT.KBD"))
     return (1);

#if 1
  puts ("Key translations:");
  for (i = 0; i < xlatCount; i++)
      printf ("key = %04lX, val = `%s', name = %s\n",
              xlatKey[i].key, xlatKey[i].name, KeyName(xlatKey[i].key));
#endif

  puts ("\nPress <Alt-X> to quit");

  while (!quit)
  {
    DWORD key = KeyGetKey();

    if (key == Key_ALTX)
       quit = 1;

    if (key == 0)
       continue;

    printf ("%10s = %04lX -> `%s'\n", KeyName(key), key, KeyTranslate(key));
  }
  return (0);
}
#endif
