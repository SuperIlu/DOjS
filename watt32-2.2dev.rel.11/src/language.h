#ifndef _w32_LANGUAGE_H
#define _w32_LANGUAGE_H

/*!\file language.h
 *
 * Include file for Watt-32 foreign language translation facility.
 *
 * Based on idea from PGP, but totally rewritten (using flex)
 *
 * Strings with _LANG() around them are found by the `mklang' tool and
 * put into a text file to be translated into foreign language at run-time.
 */

#if defined(USE_LANGUAGE)
  extern void        lang_init  (const char *value);
  extern const char *lang_xlate (const char *str);
  #define _LANG(str) lang_xlate (str)
#else
  #define _LANG(str) str
#endif

/*
 * __LANG() must be used for array string constants. This macro is used by
 * the `mklang' to generate an entry in the language database.
 */
#define __LANG(str)  str

#endif
