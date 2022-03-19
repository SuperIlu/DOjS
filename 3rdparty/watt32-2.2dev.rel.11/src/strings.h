/*!\file strings.h
 */
#ifndef _w32_STRINGS_H
#define _w32_STRINGS_H

extern  char   *_strlcpy    (char *dst, const char *src, size_t len);
extern  char   *strreplace  (int ch1, int ch2, char *str);
extern  size_t  strntrimcpy (char *dst, const char *src, size_t len);
extern  char   *strrtrim    (char *src);
extern  char   *strltrim    (const char *src);
extern  char   *strtrim     (const char *orig, char *dest, size_t len);
extern  char   *strreverse  (char *src);
extern  BYTE    atox        (const char *src);

#if defined(WIN32) || defined(WIN64)
  #define astring_acp   W32_NAMESPACE (astring_acp)
  #define astring_utf8  W32_NAMESPACE (astring_utf8)
  #define wstring_utf8  W32_NAMESPACE (wstring_utf8)
  #define wstring_acp   W32_NAMESPACE (wstring_acp)

  extern const char    *wstring_acp  (const wchar_t *in_str);
  extern const char    *wstring_utf8 (const wchar_t *in_str);
  extern const wchar_t *astring_acp  (const char *in_str);
  extern const wchar_t *astring_utf8 (const char *in_str);
#endif

#endif

