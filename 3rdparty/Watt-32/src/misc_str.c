/*!\file misc_str.c
 *
 * Printing to console and general string handling.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "wattcp.h"
#include "misc.h"
#include "pcdbug.h"
#include "misc_str.h"

/*
 * This file contains some gross hacks. Take great care !
 */
#include "nochkstk.h"

#ifdef __WATCOMC__
  extern void _outchar (char c);
  #pragma aux _outchar =  \
          "mov ah, 2"     \
          "int 21h"       \
          __parm   [__dl] \
          __modify [__ax];
#endif

/*
 * Print a single character to stdout.
 */
static int W32_CALL outch (char chr)
{
  if (chr == (char)0x1A)     /* EOF (^Z) causes trouble to stdout */
     return (0);

#if defined(WIN32)
  if (stdout_hnd != INVALID_HANDLE_VALUE)
  {
    DWORD written = 0UL;

    WriteConsoleA (stdout_hnd, &chr, 1, &written, NULL);
    return (int) written;
  }
  fputc (chr, stdout);

#elif !(DOSX)
  /* Qemm doesn't like INT 21 without a valid ES reg. intdos() sets up that.
   */
  {
    union REGS r;
    r.h.ah = 2;
    r.h.dl = chr;
    intdos (&r, &r);
  }

#elif (DOSX & POWERPAK)
  {
    union REGS r;
    r.h.ah = 2;
    r.h.dl = chr;
    int386 (0x21, &r, &r);
  }

#elif defined(__BORLANDC__)  /* avoid spawning tasm.exe */
  _ES = _DS;
  _DL = chr;
  _AL = 2;
  geninterrupt (0x21);

#elif defined(_MSC_VER) || defined(__DMC__)
  asm mov dl, chr
  asm mov ah, 2
  asm int 21h

#elif defined(__CCDL__)
  asm mov dl, [chr]
  asm mov ah, 2
  asm int 0x21

#elif defined(__WATCOMC__)
  _outchar (chr);

#elif defined(__HIGHC__)     /* outch() must not be optimised! */
  _inline (0x8A,0x55,0x08);  /* mov dl,[ebp+8] */
  _inline (0xB4,0x02);       /* mov ah,2       */
  _inline (0xCD,0x21);       /* int 21h        */

#elif defined(__GNUC__)
  __asm__ __volatile__
          ("movb %b0, %%dl\n\t"
           "movb $2, %%ah\n\t"
           "int $0x21\n\t"
           :
           : "r" (chr)
           : "%eax", "%edx" );

#else
  #error Tell me how to do this
#endif

  return (1);
}

int (W32_CALL *_outch)(char c) = outch;

/*---------------------------------------------------*/

#if defined(USE_DEBUG)
  int (MS_CDECL *_printf) (const char*, ...) = printf;
#else
  static int MS_CDECL empty_printf (const char *fmt, ...)
  {
    outsnl ("`(*_printf)()' called outside `USE_DEBUG'");
    ARGSUSED (fmt);
    return (0);
  }
  int (MS_CDECL *_printf) (const char*, ...) = empty_printf;
#endif

/*---------------------------------------------------*/

void W32_CALL outs (const char *s)
{
  while (_outch && s && *s)
  {
    if (*s == '\n')
       (*_outch) ('\r');
    (*_outch) (*s++);
  }
}

void W32_CALL outsnl (const char *s)
{
  outs (s);
  outs ("\n");
}

void W32_CALL outsn (const char *s, int n)
{
  while (_outch && *s != '\0' && n-- >= 0)
  {
    if (*s == '\n')
       (*_outch) ('\r');
    (*_outch) (*s++);
  }
}

void W32_CALL outhex (char c)
{
  char lo, hi;

  if (!_outch)
     return;

  hi = (c & 0xF0) >> 4;
  lo = c & 15;

  if (hi > 9)
       (*_outch) ((char)(hi-10+'A'));
  else (*_outch) ((char)(hi+'0'));

  if (lo > 9)
       (*_outch) ((char)(lo-10+'A'));
  else (*_outch) ((char)(lo+'0'));
}

void W32_CALL outhexes (const char *s, int n)
{
  while (_outch && n-- > 0)
  {
    outhex (*s++);
    (*_outch) (' ');
  }
}

/**
 * Removes end-of-line termination from a string.
 * Removes "\n" (Unix), "\r" (MacOS) or "\r\n" (DOS/net-ascii)
 * terminations, but not single "\n\r" (who uses that?).
 */
char * W32_CALL rip (char *s)
{
  char *p;

  if ((p = strrchr(s,'\n')) != NULL) *p = '\0';
  if ((p = strrchr(s,'\r')) != NULL) *p = '\0';
  return (s);
}

/**
 * Convert hexstring "0x??" to hex. Just assumes "??"
 * are in the [0-9,a-fA-F] range. Don't call atox() unless
 * they are or before checking for a "0x" prefix.
 */
BYTE atox (const char *hex)
{
  unsigned hi = toupper ((int)hex[2]);
  unsigned lo = toupper ((int)hex[3]);

  hi -= isdigit (hi) ? '0' : 'A'-10;
  lo -= isdigit (lo) ? '0' : 'A'-10;
  return (BYTE) ((hi << 4) + lo);
}

/**
 * Replace 'ch1' to 'ch2' in string 'str'.
 */
char *strreplace (int ch1, int ch2, char *str)
{
  char *s = str;

  WATT_ASSERT (str != NULL);

  while (*s)
  {
    if (*s == ch1)
        *s = ch2;
    s++;
  }
  return (str);
}

/**
 * Similar to strncpy(), but always returns 'dst' with 0-termination.
 */
char *_strlcpy (char *dst, const char *src, size_t len)
{
  size_t slen;

  WATT_ASSERT (src != NULL);
  WATT_ASSERT (dst != NULL);
  WATT_ASSERT (len > 0);

  slen = strlen (src);
  if (slen < len)
     return strcpy (dst, src);

  memcpy (dst, src, len-1);
  dst [len-1] = '\0';
  return (dst);
}

/**
 * Return pointer to first non-blank (space/tab) in a string.
 */
char *strltrim (const char *s)
{
  WATT_ASSERT (s != NULL);

  while (s[0] && s[1] && isspace((int)s[0]))
       s++;
  return (char*)s;
}

/**
 * Return pointer to string with trailing blanks (space/tab) removed.
 */
char *strrtrim (char *s)
{
  size_t n;

  WATT_ASSERT (s != NULL);

  n = strlen (s);
  while (n)
  {
    if (!isspace((int)s[--n]))
       break;
    s[n] = '\0';
  }
  return (s);
}

/**
 * Copy a string (to 'dest') with all excessive blanks (space/tab) removed.
 */
char *strtrim (const char *orig, char *dest, size_t len)
{
  size_t i, j;
  int    ch, last = -1;

  WATT_ASSERT (orig != NULL);
  WATT_ASSERT (dest != NULL);

  for (i = j = 0; i < len && j < len-1; i++)
  {
    ch = orig[i];
    if (isspace(ch) && isspace(last))
    {
      last = ch;
      continue;
    }
    dest[j++] = ch;
    last = ch;
  }
  dest[j] = '\0';
  return (dest);
}

/*
 * Reverse string 'str' in place.
 */
char *strreverse (char *str)
{
  int i, j;

  for (i = 0, j = strlen(str)-1; i < j; i++, j--)
  {
    char c = str[i];
    str[i] = str[j];
    str[j] = c;
  }
  return (str);
}

/**
 * Copy a string, stripping leading and trailing blanks (space/tab).
 *
 * \note This function does not work exactly like strncpy(), in that it
 *       expects the destination buffer to be at last (n+1) chars long.
 */
size_t strntrimcpy (char *dst, const char *src, size_t n)
{
  size_t      len;
  const char *s;

  WATT_ASSERT (src != NULL);
  WATT_ASSERT (dst != NULL);

  len = 0;
  s   = src;

  if (n && s[0])
  {
    while (isspace((int)*s))
          ++s;
    len = strlen (s);
    if (len)
    {
      const char *e = &s[len-1];
      while (isspace((int)*e))
      {
        --e;
        --len;
      }
      if (len > n)
          len = n;
      strncpy (dst, s, len);
    }
  }
  if (len < n)
     dst[len] = '\0';
  return (len);
}

/*
 * A strtok_r() function taken from libcurl:
 *
 * Copyright (C) 1998 - 2007, Daniel Stenberg, <daniel@haxx.se>, et al.
  */
char *strtok_r (char *ptr, const char *sep, char **end)
{
  if (!ptr)
  {
    /* we got NULL input so then we get our last position instead */
    ptr = *end;
  }

  /* pass all letters that are including in the separator string */
  while (*ptr && strchr(sep, *ptr))
    ++ptr;

  if (*ptr)
  {
    /* so this is where the next piece of string starts */
    char *start = ptr;

    /* set the end pointer to the first byte after the start */
    *end = start + 1;

    /* scan through the string to find where it ends, it ends on a
     *  null byte or a character that exists in the separator string.
     */
    while (**end && !strchr(sep, **end))
      ++*end;

    if (**end)
    {
      /* the end is not a null byte */
      **end = '\0';  /* zero terminate it! */
      ++*end;        /* advance the last pointer to beyond the null byte */
    }
    return (start);  /* return the position where the string starts */
  }
  /* we ended up on a null byte, there are no more strings to find! */
  return (NULL);
}

#ifdef NOT_USED
int isstring (const char *str, size_t len)
{
  if (strlen(str) > len-1)
     return (0);

  while (*str)
  {
    if (!isprint(*str++))
    {
      str--;
      if (!isspace(*str++))
         return (0);
    }
  }
  return (1);
}
#endif

#if defined(WIN32) || defined(WIN64)    /* rest of file */
#include <wchar.h>

static const char *wide_string (const wchar_t *in_str, UINT cp)
{
  static char buf [300];

  buf[0] = '\0';
  if (WideCharToMultiByte(cp, 0, in_str, -1,
                          buf, sizeof(buf), NULL, NULL) == 0)
     SNPRINTF (buf, sizeof(buf),
               "WideCharToMultiByte() failed: %s", win_strerror(GetLastError()));
  return (buf);
}

static const wchar_t *ascii_string (const char *in_str, UINT cp)
{
  static wchar_t buf [300];

  buf[0] = '\0';
  if (MultiByteToWideChar(cp, 0, in_str, -1, buf, DIM(buf)) == 0)
  {
    TRACE_CONSOLE (2, "MultiByteToWideChar() failed: %s", win_strerror(GetLastError()));
    return (L"??");
  }
  return (buf);
}

const wchar_t *astring_acp (const char *in_str)
{
  return ascii_string (in_str, CP_ACP);
}

const wchar_t *astring_utf8 (const char *in_str)
{
  return ascii_string (in_str, CP_UTF8);
}

const char *wstring_acp (const wchar_t *in_str)
{
  return wide_string (in_str, CP_ACP);
}

const char *wstring_utf8 (const wchar_t *in_str)
{
  return wide_string (in_str, CP_UTF8);
}
#endif  /* WIN32 || WIN64 */


