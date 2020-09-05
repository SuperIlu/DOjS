/*!\file idna.h
 */
#ifndef _w32_IDNA_H
#define _w32_IDNA_H

typedef long  ucs4_t;

typedef struct {
        int istate;
      } *conv_t;

typedef struct {
        WORD  indx;  /* index into big table */
        WORD  used;  /* bitmask of used entries */
      } Summary16;

#define iconv_init            W32_NAMESPACE (iconv_init)
#define iconv_strerror        W32_NAMESPACE (iconv_strerror)
#define IDNA_convert_to_ACE   W32_NAMESPACE (IDNA_convert_to_ACE)
#define IDNA_convert_from_ACE W32_NAMESPACE (IDNA_convert_from_ACE)

extern const char *iconv_strerror (int rc);
extern BOOL        iconv_init (WORD code_page);

extern BOOL IDNA_convert_to_ACE   (char *name, size_t *size);
extern BOOL IDNA_convert_from_ACE (char *name, size_t *size);

#ifndef MAX_LABELS
#define MAX_LABELS 8
#endif

#if defined(USE_DEBUG) || defined(TEST_PROG)
  #define IDNA_DEBUG(lvl, args)                            \
          do {                                             \
            if (debug_on >= lvl) {                         \
              (*_printf) ("%s(%u): ", __FILE__, __LINE__); \
              (*_printf) args;                             \
              fflush (stdout);                             \
            }                                              \
          } while (0)
#else
  #define IDNA_DEBUG(lvl, args)  ((void)0)
#endif

#define RET_ILUNI      -1
#define RET_ILSEQ      -2
#define RET_TOOSMALL   -3
#define RET_TOOFEW(n) (-4-(n))

#endif  /* _w32_IDNA_H */

