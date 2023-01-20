#ifndef __CONFIG_H
#define __CONFIG_H

enum   Sections {
       General = 1,
       Colours,
       KeyMapANSI,
       KeyMapVTERM,
       KeyMapSCO
     };

enum   KeyMapping {
       VT_keys  = 1,
       IBM_keys = 2,
       SCO_keys = 3
     };

struct Colour {
       int user_fg, user_bg;
       int data_fg, data_bg;
       int warn_fg, warn_bg;
       int stat_fg, stat_bg;
     };

struct TelConfig  {
       struct Colour colour;
       const char   *log_file;
       const char   *dump_file;
       int           scr_width;
       int           scr_height;
       int           status_line;
       unsigned      timeout;
     };

extern struct TelConfig cfg;

extern char *_w32_expand_var_str (const char *str);  /* in wattxx.lib, pcconfig.c */

extern int   OpenIniFile  (const char *name);
extern int   tel_log      (const char *fmt, ...);
extern int   tel_log_init (void);
extern int   tel_dump     (const char *fmt, ...);
extern void  tel_dump_hex (const char *title, const BYTE *hexbuf, unsigned len);
extern int   tel_dump_init(void);

#endif

