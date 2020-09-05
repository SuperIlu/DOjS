/*
 * Code for enabling lookup of names with non-ASCII letters via
 * ACE and IDNA (Internationalizing Domain Names in Applications)
 * Ref. RFC-3490.
 *
 */

/*  \version 0.1: Mar 19, 2004 :
 *    G. Vanem - Created.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>

#include "wattcp.h"
#include "misc.h"
#include "strings.h"
#include "pcdbug.h"
#include "punycode.h"
#include "idna.h"

#if defined(USE_IDNA)

#define _FLUSHWC_H  /* normal_flushwc() not needed */

#ifdef _MSC_VER
#pragma warning (disable:4244)
#endif

#include "iconv/ascii.h"
#include "iconv/jisx0201.h"
#include "iconv/jisx0208.h"
#include "iconv/cp437.h"
#include "iconv/cp737.h"
#include "iconv/cp775.h"
#include "iconv/cp850.h"
#include "iconv/cp852.h"
#include "iconv/cp853.h"
#include "iconv/cp855.h"
#include "iconv/cp856.h"
#include "iconv/cp857.h"
#include "iconv/cp858.h"
#include "iconv/cp860.h"
#include "iconv/cp861.h"
#include "iconv/cp862.h"
#include "iconv/cp863.h"
#include "iconv/cp864.h"
#include "iconv/cp865.h"
#include "iconv/cp866.h"
#include "iconv/cp869.h"
#include "iconv/cp874.h"
#include "iconv/cp922.h"
#include "iconv/cp932.h"
#include "iconv/cp943.h"
#include "iconv/ksc5601.h"
#include "iconv/cp949.h"
#include "iconv/big5.h"
#include "iconv/cp950.h"
#include "iconv/cp1046.h"
#include "iconv/cp1124.h"
#include "iconv/cp1125.h"
#include "iconv/cp1129.h"
#include "iconv/cp1133.h"
#include "iconv/cp1161.h"
#include "iconv/cp1162.h"
#include "iconv/cp1163.h"
#include "iconv/cp1250.h"
#include "iconv/cp1251.h"
#include "iconv/cp1252.h"
#include "iconv/cp1253.h"
#include "iconv/cp1254.h"
#include "iconv/cp1255.h"
#include "iconv/cp1256.h"
#include "iconv/cp1257.h"
#include "iconv/cp1258.h"

typedef int (*toUnicode) (conv_t, ucs4_t *, const unsigned char *, int);
typedef int (*toAscii)   (conv_t, unsigned char *, ucs4_t, int);

struct iconv_table {
       const char *name;
       WORD        codepage;
       toUnicode   mbtowc;
       toAscii     wctomb;
     };

static const struct iconv_table mappings[] = {
       { "CP437",  437,  cp437_mbtowc,  cp437_wctomb  },
       { "CP737",  737,  cp737_mbtowc,  cp737_wctomb  },
       { "CP775",  775,  cp775_mbtowc,  cp775_wctomb  },
       { "CP850",  850,  cp850_mbtowc,  cp850_wctomb  },
       { "CP852",  852,  cp852_mbtowc,  cp852_wctomb  },
       { "CP853",  853,  cp853_mbtowc,  cp853_wctomb  },
       { "CP855",  855,  cp855_mbtowc,  cp855_wctomb  },
       { "CP856",  856,  cp856_mbtowc,  cp856_wctomb  },
       { "CP857",  857,  cp857_mbtowc,  cp857_wctomb  },
       { "CP858",  858,  cp858_mbtowc,  cp858_wctomb  },
       { "CP860",  860,  cp860_mbtowc,  cp860_wctomb  },
       { "CP861",  861,  cp861_mbtowc,  cp861_wctomb  },
       { "CP862",  862,  cp862_mbtowc,  cp862_wctomb  },
       { "CP863",  863,  cp863_mbtowc,  cp863_wctomb  },
       { "CP864",  864,  cp864_mbtowc,  cp864_wctomb  },
       { "CP865",  865,  cp865_mbtowc,  cp865_wctomb  },
       { "CP866",  866,  cp866_mbtowc,  cp866_wctomb  },
       { "CP869",  869,  cp869_mbtowc,  cp869_wctomb  },
       { "CP874",  874,  cp874_mbtowc,  cp874_wctomb  },
       { "CP922",  922,  cp922_mbtowc,  cp922_wctomb  },
       { "CP932",  932,  cp932_mbtowc,  cp932_wctomb  },
       { "CP943",  943,  cp943_mbtowc,  cp943_wctomb  },
       { "CP949",  949,  cp949_mbtowc,  cp949_wctomb  },
       { "CP950",  950,  cp950_mbtowc,  cp950_wctomb  },
       { "CP1046", 1046, cp1046_mbtowc, cp1046_wctomb },
       { "CP1124", 1124, cp1124_mbtowc, cp1124_wctomb },
       { "CP1125", 1125, cp1125_mbtowc, cp1125_wctomb },
       { "CP1129", 1129, cp1129_mbtowc, cp1129_wctomb },
       { "CP1133", 1133, cp1133_mbtowc, cp1133_wctomb },
       { "CP1161", 1161, cp1161_mbtowc, cp1161_wctomb },
       { "CP1162", 1162, cp1162_mbtowc, cp1162_wctomb },
       { "CP1163", 1163, cp1163_mbtowc, cp1163_wctomb },
       { "CP1250", 1250, cp1250_mbtowc, cp1250_wctomb },
       { "CP1251", 1251, cp1251_mbtowc, cp1251_wctomb },
       { "CP1252", 1252, cp1252_mbtowc, cp1252_wctomb },
       { "CP1253", 1253, cp1253_mbtowc, cp1253_wctomb },
       { "CP1254", 1254, cp1254_mbtowc, cp1254_wctomb },
       { "CP1255", 1255, cp1255_mbtowc, cp1255_wctomb },
       { "CP1256", 1256, cp1256_mbtowc, cp1256_wctomb },
       { "CP1257", 1257, cp1257_mbtowc, cp1257_wctomb },
       { "CP1258", 1258, cp1258_mbtowc, cp1258_wctomb }
     };

static const struct iconv_table *curr_mapping = NULL;
static conv_t iconv;

/* The following string is used to convert printable
 * characters between ASCII and the native charset:
 */
static const char print_ascii[] = "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                                  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                                  " !\"#$%&'()*+,-./" "0123456789:;<=>?"
                                  "@ABCDEFGHIJKLMNO"
                                  "PQRSTUVWXYZ[\\]^_"
                                  "`abcdefghijklmno"
                                  "pqrstuvwxyz{|}~\n";
#if defined(WIN32)
  #define GET_CODEPAGE()  GetACP()
#else
  #define GET_CODEPAGE()  GetCodePage()
#endif

/**
 * Initialise iconv; find codepage and mapping functions to use.
 */
BOOL iconv_init (WORD cp)
{
  int i;

  if (cp == 0)
     cp = GET_CODEPAGE();

  IDNA_DEBUG (2, ("iconv_init: codepage %u\n", cp));
  if (!cp)
     return (FALSE);

  for (i = 0; i < DIM(mappings); i++)
      if (cp == mappings[i].codepage)
         break;
  if (i == DIM(mappings))
     return (FALSE);
  curr_mapping = mappings + i;
  return (TRUE);
}

/**
 * Return textual error for 'rc'.
 */
const char *iconv_strerror (int rc)
{
  switch (rc)
  {
    case RET_ILUNI:
         return ("Illegal Unicode");
    case RET_ILSEQ:
         return ("Illegal sequence");
    case RET_TOOSMALL:
         return ("Output buffer too small");
    case RET_TOOFEW(0):
         return ("Input sequence too short");
    case RET_TOOFEW(1):
         return ("Input sequence 1 byte too short");
    case RET_TOOFEW(2):
         return ("Input sequence 2 bytes too short");
    case RET_TOOFEW(3):
         return ("Input sequence 3 bytes too short");
    default:
         return ("Unknown");
  }
}

/**
 * Convert a single ASCII codepoint from active codepage to Unicode.
 */
static int iconv_to_unicode (char ch, ucs4_t *uc)
{
  ucs4_t res = 0;
  int    rc;

  if (!curr_mapping)
     return (0);
  rc = (*curr_mapping->mbtowc) (iconv, &res, (unsigned char*)&ch, 1);
  if (rc < 1)
  {
    IDNA_DEBUG (1, ("iconv_to_unicode failed; %d, %s\n",
                rc, iconv_strerror(rc)));
    return (0);
  }
  *uc = res;
  return (rc);
}

/**
 * Convert a single Unicode codepoint to ASCII in active codepage.
 */
static int iconv_to_ascii (ucs4_t uc, char *ch)
{
  int rc = 0;

  if (curr_mapping)
  {
    unsigned char res[4] = { 0,0,0,0 };

    rc = (*curr_mapping->wctomb) (iconv, res, uc, sizeof(res));
    if (rc == 1)
       *ch = (char) res[0];
    else if (rc == 2)
       *(WORD*)ch = *(WORD*)&res;
    else if (rc > 2)
       memcpy (ch, res, rc);
    else
       IDNA_DEBUG (1, ("iconv_to_ascii failed; %d, %s\n",
                   rc, iconv_strerror(rc)));
  }
  return (rc);
}

/**
 * Split a domain-name into lables (not trailing dots).
 */
static char **split_labels (const char *name)
{
  static char  buf [MAX_LABELS][MAX_HOSTLEN];
  static char *res [MAX_LABELS+1];
  const  char *p = name;
  int    i;

  for (i = 0; i < MAX_LABELS && *p; i++)
  {
    const char *dot = strchr (p, '.');

    if (!dot)
    {
      res[i] = _strlcpy (buf[i], p, sizeof(buf[i]));
      i++;
      break;
    }
    res[i] = _strlcpy (buf[i], p, dot-p+1);
    p = ++dot;
  }
  res[i] = NULL;
  IDNA_DEBUG (3, ("split_labels: `%s', %d labels\n", name, i));
  return (res);
}

/**
 * Convert a single name label to ACE form.
 */
static char *convert_to_ACE (const char *name)
{
  int          i, c;
  DWORD        utf_input[MAX_HOSTLEN];
  BYTE         utf_case [MAX_HOSTLEN];
  const  char *p;
  size_t       in_len, out_len;
  static char  out_buf [2*MAX_HOSTLEN];
  enum punycode_status status;

  for (i = 0, p = name; *p; i++)
  {
    ucs4_t utf = 0;

    c = (*p++) & 255;
    iconv_to_unicode (c, &utf);
    utf_input[i] = utf;
    utf_case[i]  = (BYTE) isupper (c);
    if (utf > 0xFFFF)
         IDNA_DEBUG (3, ("%c -> u+%08lX\n", c, utf));
    else IDNA_DEBUG (3, ("%c -> u+%04lX\n", c, utf));
  }
  in_len  = i;
  out_len = sizeof(out_buf);
  status  = punycode_encode (in_len, utf_input, utf_case, &out_len, out_buf);

  if (status != punycode_success)
     out_len = 0;

  for (i = 0; i < (int)out_len; i++)
  {
    int c = out_buf[i];

    if (c < 0 || c > 127)
    {
      IDNA_DEBUG (1, ("illegal Punycode result: %c (%d)\n", c, c));
      return (NULL);
    }
    if (!print_ascii[c])
    {
      IDNA_DEBUG (1, ("Punycode not ASCII: %c (%d)\n", c, c));
      return (NULL);
    }
    out_buf[i] = print_ascii[c];
  }
  out_buf[i] = '\0';
  IDNA_DEBUG (2, ("punycode_encode: status %d, out_len %lu, out_buf `%s'\n",
              status, (u_long)out_len, out_buf));
  return (status == punycode_success ? out_buf : NULL);
}

/**
 * Convert a single ACE encoded label to national ASCII encoded form.
 */
static char *convert_from_ACE (const char *name)
{
  DWORD       utf_output[MAX_HOSTLEN];
  BYTE        utf_case  [MAX_HOSTLEN];
  static char out_buf [MAX_HOSTLEN];
  size_t      utf_len, i, j;
  enum punycode_status status;

  utf_len = sizeof(utf_output);
  status = punycode_decode (strlen(name), name, &utf_len, utf_output, utf_case);

  if (status != punycode_success)
     utf_len = 0;

  for (i = j = 0; i < utf_len && j < sizeof(out_buf); i++)
  {
    ucs4_t utf = utf_output[i];
    int    len = iconv_to_ascii (utf, out_buf+j);

    if (len <= 0)
       break;
    IDNA_DEBUG (3, ("%c+%04lX -> %*.s\n",
                utf_case[i] ? 'U' : 'u', utf, len, out_buf+j));
    j += len;
  }
  out_buf[j] = '\0';
  IDNA_DEBUG (2, ("punycode_decode: status %d, out_len %lu, out_buf `%s'\n",
              status, (u_long)utf_len, out_buf));
  return (status == punycode_success ? out_buf : NULL);
}


/**
 * Convert a possibly non-ASCII name into ACE-form.
 *
 * E.g. convert "www.troms›.no" to ACE:
 *
 * 1) Convert each label separately; "www", "troms›" and "no"
 * 2) "troms›" -> u+0074 u+0072 u+006F u+006D u+0073 u+00F8
 * 3) Pass this through `punycode_encode()' which gives "troms-zua".
 * 4) Repeat for all labels with non-ASCII letters.
 * 5) Prepending "xn--" for each converted label gives "www.xn--troms-zua.no".
 *
 * E.g. 2:
 *   "bl†b‘r.syltet›y.no" -> "xn--blbr-roah.xn--syltety-v1a.no"
 *
 * Ref. http://www.imc.org/idna/do-idna.cgi
 *      http://www.norid.no/domenenavnbaser/ace/ace_technical.en.html
 */
BOOL IDNA_convert_to_ACE (
          char   *name,   /* IN/OUT: native ASCII/ACE name */
          size_t *size)   /* IN:     length of name buf, */
{                         /* OUT:    ACE encoded length */
  const  char *ace;
  char  *in_name = name;
  char **labels = split_labels (name);
  int    i;
  size_t len = 0;

  for (i = 0; labels[i]; i++)
  {
    const BYTE *p;
    const char *label = labels[i];

    ace = NULL;
    if (strnicmp(label,"xn--",4))  /* if not already encoded */
    {
      for (p = (const BYTE*)label; *p; p++)
          if (*p >= 0x80)
          {
            ace = convert_to_ACE (label);
            if (!ace)
               return (FALSE);
            break;
          }
    }

    if (ace)
    {
      if (len + 5 + strlen(ace) > *size)
      {
        IDNA_DEBUG (1, ("input length exceeded\n"));
        return (FALSE);
      }
      name += sprintf (name, "xn--%s.", ace);
    }
    else  /* pass through unchanged */
    {
      if (len + 1 + strlen(label) > *size)
      {
        IDNA_DEBUG (1, ("input length exceeded\n"));
        return (FALSE);
      }
      name += sprintf (name, "%s.", label);
    }
  }
  if (i > 0)   /* drop trailing '.' */
     name--;
  len = name - in_name;
  *name = '\0';
  *size = len;
  IDNA_DEBUG (2, ("IDNA_convert_to_ACE: `%s', %lu bytes\n",
              in_name, (u_long)len));
  return (TRUE);
}

/**
 * Convert a possibly ACE-encoded name to a name in native codepage.
 *
 * \todo Check for over-run on output.
 *
 * 1) Pass through labels w/o "xn--" prefix unaltered.
 * 2) Strip "xn--" prefix and pass to punycode_decode()
 * 3) Repeat for all labels with "xn--" prefix.
 * 4) Collect Unicode strings and convert to original codepage.
 */
BOOL IDNA_convert_from_ACE (
          char   *name,   /* IN/OUT: ACE/native ASCII name */
          size_t *size)   /* IN:     ACE raw string length, */
{                         /* OUT:    ASCII deccoded length */
  char  *in_name = name;
  char **labels  = split_labels (name);
  int    i;
  u_long dsize;

  for (i = 0; labels[i]; i++)
  {
    const char *ascii = NULL;
    const char *label = labels[i];

    if (!strncmp(label,"xn--",4) && label[4])
    {
      ascii = convert_from_ACE (label+4);
      if (!ascii)
         return (FALSE);
    }
    name += sprintf (name, "%s.", ascii ? ascii : label);
  }
  *name = '\0';
  *size = name - in_name;
  dsize = *(u_long*) size;
  IDNA_DEBUG (2, ("IDNA_convert_from_ACE: `%s', %lu bytes\n",
              in_name, dsize));
  return (TRUE);
}

#if defined(TEST_PROG)

#include <netdb.h>
#include <arpa/inet.h>

#include "sock_ini.h"
#include "pcdns.h"
#include "pcdbug.h"

void dump_cp_list (void)
{
  int i;

  printf ("Supported codepages:\n");
  for (i = 0; i < DIM(mappings); i++)
      printf ("  %s\n", mappings[i].name);
  exit (0);
}

void usage (void)
{
  printf ("IDNA [-d] [-c codepage] hostname | ip-address\n"
          "   -d debug level, \"-dd\" for more details\n"
          "   -c select codepage (active is CP%d).\n", GET_CODEPAGE());
  printf ("      use \"-c?\" to list supported codepages\n");
  exit (0);
}

int main (int argc, char **argv)
{
  struct in_addr addr;
  struct hostent *he;
  const  char    *host;
  WORD   cp = 0;
  int    ch;
  int    debug = 0;

  while ((ch = getopt(argc, argv, "c:dh?")) != EOF)
     switch (ch)
     {
       case 'c':
            if (*optarg == '?')
               dump_cp_list();
            cp = atoi (optarg);
            break;
       case 'd':
            debug++;
            break;
       case '?':
       case 'h':
       default:
            usage();
            break;
  }

  argc -= optind;
  argv += optind;
  if (!*argv)
     usage();

  if (debug)
     dbug_init();
  sock_init();

  if (!iconv_init(cp))
  {
    printf ("iconv_init() failed for CP %d\n", cp);
    return (1);
  }

  debug_on = debug;
  dns_do_idna = TRUE;
  host = argv[0];
  printf ("Resolving `%s'...", host);
  fflush (stdout);

  if (inet_aton(host,&addr))
  {
    he = gethostbyaddr ((char*)&addr, sizeof(addr), AF_INET);
    if (he)
         printf ("%s\n", he->h_name);
    else printf ("failed; %s)\n", hstrerror(h_errno));
  }
  else
  {
    he = gethostbyname (host);
    if (he)
         printf ("%s\n", inet_ntoa(*(struct in_addr*)he->h_addr));
    else printf ("failed; %s\n", hstrerror(h_errno));
  }

  debug_on = 0;
  return (0);
}
#endif  /* TEST_PROG */
#endif  /* USE_IDNA */


