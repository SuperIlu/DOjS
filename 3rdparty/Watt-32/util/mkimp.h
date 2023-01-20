#ifndef __MKIMP_H
#define __MKIMP_H

#define YY_NO_UNPUT

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

#undef  BEGIN
#define BEGIN(state)  do {                                         \
                        int st = state;                            \
                        yy_start = 1 + (2 * st);                   \
                        if (verbose > 1)                           \
                           Debug ("State `%s'\n", state_name(st)); \
                      } while (0)

#define DEBUG(fmt)    do {               \
                        if (verbose > 0) \
                           Debug fmt ;   \
                      } while (0)

#define RESET_BUF()   do {                          \
                        ptr = buf;                  \
                        memset (buf,0,sizeof(buf)); \
                      } while (0)

#define FLUSH_BUF()   do {                            \
                        DEBUG (("buf: `%s'\n", buf)); \
                        RESET_BUF();                  \
                      } while (0)

#define PUTC_BUF(c)   do {                            \
                        if (ptr >= end)               \
                           Abort ("buffer overflow"); \
                        *ptr++ = c;                   \
                      } while (0)

#define RESET_MAC()   do {                                  \
                        mac_ptr = mac_buf;                  \
                        memset (mac_buf,0,sizeof(mac_buf)); \
                      } while (0)

#define FLUSH_MAC()   do {                               \
                        DEBUG (("macro: `%s' = `%s'\n",  \
                                macro_lvalue, mac_buf)); \
                        RESET_MAC();                     \
                      } while (0)

#define PUTC_MAC(c)   do {                                  \
                        if (mac_ptr >= mac_end)             \
                           Abort ("macro-buffer overflow"); \
                        *mac_ptr++ = c;                     \
                      } while (0)

struct MacroNode {
       const char       *lvalue;
       const char       *rvalue;
       const char       *fname;
       unsigned          line;
       struct MacroNode *next;
     };

int   yylex (void);
void  Abort (const char *fmt, ...);
void  Debug (const char *fmt, ...);
void  Error (const char *fmt, ...);

const char *state_name (int state);

char *rip (char *str);
char *trim (char *str);
char *ident (char *str);

#endif
