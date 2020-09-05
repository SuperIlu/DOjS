/*!\file misc.h
 */
#ifndef _w32_MISC_H
#define _w32_MISC_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __SYS_SWAP_BYTES_H
#include <sys/swap.h>       /* intel() etc. */
#endif

#ifndef __SYS_WTIME_H
#include <sys/wtime.h>      /* struct timeval */
#endif

#ifndef __SYS_CDEFS_H
#include <sys/cdefs.h>
#endif

#if defined(__DJGPP__) || defined(__HIGHC__) || defined(__WATCOMC__) || \
    defined(__CYGWIN__) || defined(__DMC__)

  #if !defined(__CYGWIN__)
    #undef _WIN32          /* Needed for __DMC__ */
  #endif

  #include <unistd.h>
#endif

#if defined(__LCC__) && !defined(W32_LCC_INTRINSICS_INCLUDED)
  #include <intrinsics.h>

#elif defined(__POCC__)
  #include <intrin.h>
#endif

/*
 * Because kbhit() will pull in more conio function that we
 * really need, use the simple kbhit() variant (without ungetch()
 * option). This also prevents multiple definition trouble when
 * linking e.g. PD-curses and Watt-32 library.
 */
#if defined(__DJGPP__)
  #ifdef __dj_include_conio_h_
    #error "Don't include <conio.h>"
  #endif
  #include <pc.h>     /* simple kbhit() */

#elif defined(_MSC_VER) && defined(__386__)
  /*
   * Problems including <conio.h> from Visual C 4.0
   */
  extern int __cdecl kbhit (void);

#elif defined(__BORLANDC__)
  /*
   * Some trouble with multiple symbols in Borland's <conio.h>.
   * Hence just don't include it.
   */
  #define __CONIO_H
  int _RTLENTRY _EXPFUNC kbhit (void);
  int _RTLENTRY _EXPFUNC getch (void);

#elif defined(__CYGWIN__)
  /*
   * More CygWin quirks. No <tchar.h>
   */
  #include <string.h>
  #include <wchar.h>

  #if defined(UNICODE) || defined(_UNICODE)
    #define _T(str)                L##str
    #define _tcsicmp(s1, s2)      wcsncmp (s1, s2)
    #define _tcsncpy(s1, s2, len)  wcsncpy (s1, s2, len)
  #else
    #define _T(str)                str
    #define _tcsicmp(s1, s2)       strcasecmp (s1, s2)
    #define _tcsncpy(s1, s2, len)  strncpy (s1, s2, len)
  #endif

  #define stricmp(s1, s2)        strcasecmp (s1, s2)
  #define strnicmp(s1, s2, len)  strncasecmp (s1, s2, len)

  /* Use kbhit(), getch() below
   */
#else                                   /* no other option */
  #include <conio.h>
#endif

/*
 * In misc.c
 *
 * Protect internal symbols with a namespace
 */
#define Wait             W32_NAMESPACE (Wait)
#define valid_addr       W32_NAMESPACE (valid_addr)
#define is_in_stack      W32_NAMESPACE (is_in_stack)
#define used_stack       W32_NAMESPACE (used_stack)
#define os_yield         W32_NAMESPACE (os_yield)
#define hex_chars_lower  W32_NAMESPACE (hex_chars_lower)
#define hex_chars_upper  W32_NAMESPACE (hex_chars_upper)
#define win32_dos_box    W32_NAMESPACE (win32_dos_box)
#define get_day_num      W32_NAMESPACE (get_day_num)
#define dword_str        W32_NAMESPACE (dword_str)
#define qword_str        W32_NAMESPACE (qword_str)
#define shell_exec       W32_NAMESPACE (shell_exec)
#define assert_fail      W32_NAMESPACE (assert_fail)
#define ctime_r          W32_NAMESPACE (ctime_r)
#define localtime_r      W32_NAMESPACE (localtime_r)
#define strtok_r         W32_NAMESPACE (strtok_r)

#define memdbg_init      W32_NAMESPACE (memdbg_init)
#define memdbg_post_init W32_NAMESPACE (memdbg_post_init)
#define fopen_excl       W32_NAMESPACE (fopen_excl)
#define unfinished       W32_NAMESPACE (unfinished)
#define unimplemented    W32_NAMESPACE (unimplemented)

#if !defined(SEARCH_LIST_DEFINED)
  #define SEARCH_LIST_DEFINED
  struct search_list {
         DWORD       type;
         const char *name;
       };
#endif

#define DO_FREE(x)  do {                    \
                      if (x) {              \
                         free ((void*)(x)); \
                         x = NULL;          \
                      }                     \
                    } while (0)

#if defined(WIN32) || defined(WIN64)
  #define SOCK_ERRNO(err)  WSASetLastError (err)
#else
  #define SOCK_ERRNO(err)  errno = _w32_errno = err
#endif

extern const char hex_chars_lower[];
extern const char hex_chars_upper[];

extern BOOL     win32_dos_box;
extern BOOL    _watt_fatal_error;
extern BOOL    _watt_crtdbg_check;
extern WORD    _watt_os_ver;
extern char    _watt_assert_buf[256];

extern void     Wait         (unsigned msec);
extern BOOL     is_in_stack  (const void *ptr);
extern unsigned used_stack   (void);
extern BOOL     valid_addr   (const void *addr, unsigned len);

extern void     os_yield     (void);
extern void     assert_fail  (const char *file, unsigned line, const char *what);

extern   DWORD  get_day_num      (void);
extern   void   memdbg_init      (void);
extern   void   memdbg_post_init (void);
extern   FILE  *fopen_excl       (const char *file, const char *mode);

extern void     unfinished     (const char *func, const char *file, unsigned line);
extern void     unimplemented  (const char *func, const char *file, unsigned line);

#define UNFINISHED()    unfinished (__FUNCTION__, __FILE__, __LINE__)
#define UNIMPLEMENTED() unimplemented (__FUNCTION__, __FILE__, __LINE__)

extern char       *ctime_r     (const time_t *t, char *res);
extern struct tm  *localtime_r (const time_t *t, struct tm *res);
extern char       *strtok_r    (char *ptr, const char *sep, char **end);

extern const char *dos_extender_name (void);
extern const char *dword_str (DWORD val);

#if defined(HAVE_UINT64)
extern const char *qword_str (uint64 val);
#endif

/*
 */
#if defined(__POCC__) && defined(WIN64)
  #undef  HandleToUlong
  #define HandleToUlong(h)  ( (unsigned long)(ULONG_PTR)(h) )
#endif

/*
 * A patch from Ozkan Sezer <sezeroz@gmail.com>.
 *   Using 'access()' in djgpp pulls in unnecessary big dependencies, so
 *  '_chmod()' is smaller and cheaper to use. And plenty good enough.
 */
#if defined(__DJGPP__)
  #define FILE_EXIST(fname)  (_chmod(fname, 0) != -1)
#else
  #define FILE_EXIST(fname)  (access(fname, 0) == 0)
#endif

/*
 * Yielding while waiting on network reduces CPU-loading.
 */
#if (DOSX & (DJGPP|DOS4GW|PHARLAP|X32VM|WINWATT))
  #define WATT_YIELD()   os_yield()  /* includes kbhit() for non-djgpp */
#else
  #define WATT_YIELD()   watt_kbhit()
#endif

#if defined(USE_DEBUG)
  #define list_lookup   W32_NAMESPACE (list_lookup)
  #define list_lookupX  W32_NAMESPACE (list_lookupX)
  #define MAC_address   W32_NAMESPACE (MAC_address)

  extern const char *list_lookup   (DWORD, const struct search_list *, int);
  extern const char *list_lookupX  (DWORD, const struct search_list *, int);
  extern const char *MAC_address   (const void *addr);
#endif

#if defined(__LARGE__)
  extern void watt_large_check (const void *sock, int size,
                                const char *file, unsigned line);
  #define WATT_LARGE_CHECK(sock, size) \
          watt_large_check(sock, size, __FILE__, __LINE__)
#else
  #define WATT_LARGE_CHECK(sock, size)  ((void)0)
#endif

#if defined(USE_DEBUG) && !defined(NDEBUG)
  #define WATT_ASSERT(x)  do {                                       \
                            if (!(x))                                \
                               assert_fail (__FILE__, __LINE__, #x); \
                          } while (0)
#else
  #define WATT_ASSERT(x)  ((void)0)
#endif

/*
 * Compare function used in qsort() + bsearch().
 * Cast your function with this.
 */
#if !defined(_CmpFunc_T)
  #define _CmpFunc_T
  typedef int (MS_CDECL *CmpFunc) (const void *, const void *);
#endif

#if defined(__CYGWIN__)
  /*
   * We use these only under CygWin, but compile them for all Win32 DLLs
   * since we want e.g. a MinGW built watt-32.dll to work with CygWin.
   */
  #define kbhit W32_NAMESPACE (kbhit)
  #define getch W32_NAMESPACE (getch)
  #define itoa  W32_NAMESPACE (itoa)

  extern int    kbhit (void);
  extern int    getch (void);
  extern char  *itoa (int val, char *buf, int radix);
#endif

#if defined(WIN32) || defined(WIN64)
  /*
   * In winmisc.c
   */
  #define init_win_misc        W32_NAMESPACE (init_win_misc)
  #define print_thread_times   W32_NAMESPACE (print_thread_times)
  #define print_perf_times     W32_NAMESPACE (print_perf_times)
  #define print_process_times  W32_NAMESPACE (print_process_times)

  #define WinDnsQueryA4        W32_NAMESPACE (WinDnsQueryA4)
  #define WinDnsQueryA6        W32_NAMESPACE (WinDnsQueryA6)
  #define WinDnsQueryPTR4      W32_NAMESPACE (WinDnsQueryPTR4)
  #define WinDnsQueryPTR6      W32_NAMESPACE (WinDnsQueryPTR6)
  #define WinDnsCachePut_A4    W32_NAMESPACE (WinDnsCachePut_A4)
  #define WinDnsCachePut_A6    W32_NAMESPACE (WinDnsCachePut_A6)
  #define get_file_version     W32_NAMESPACE (get_file_version)

  extern HANDLE      stdin_hnd, stdout_hnd;
  extern BOOL       _watt_is_win9x, _watt_is_wow64;
  extern BOOL       _watt_is_gui_app, _watt_use_bugtrap;

  extern CONSOLE_SCREEN_BUFFER_INFO console_info;

  extern BOOL        init_win_misc (void);

  extern BOOL        WinDnsQuery_A4     (const char *name, DWORD *ip4);
  extern BOOL        WinDnsQuery_A6     (const char *name, void *ip6);
  extern BOOL        WinDnsQuery_PTR4   (DWORD ip4, TCHAR *name, size_t size);
  extern BOOL        WinDnsQuery_PTR6   (const void *ip6, TCHAR *name, size_t size);
  extern BOOL        WinDnsCachePut_A4  (const char *name, DWORD ip4);
  extern BOOL        WinDnsCachePut_A6  (const char *name, const void *ip6);
  extern BOOL        get_file_version   (const char *file, char *buf, size_t buf_len);
  extern void        print_thread_times (HANDLE thread);
  extern void        print_perf_times   (void);
  extern void        print_process_times(void);
#endif  /* WIN32 || WIN64 */

#if defined(HAVE_UINT64)
  /*
   * Format for printing 64-bit types. E.g. "%10"S64_FMT.
   * Watt-32 code should not try to print a 64-bit number unless
   * 'HAVE_UINT64' is defined.
   */

  /* gcc on Win32 and Win64 are 2 different things regarding
   * printf-parameters and '-Wformat'. gcc erroneously assumes
   * that only Microsoft's non-standard formatting is valid on
   * Windows. Both MinGW and MinGW-w64 provides an ISO compatible
   * alternative printf(). Hence this:
   */

  #if defined(__GNUC__)
    #if defined(__DJGPP__) || defined(__CYGWIN__) || defined(__MINGW64_VERSION_MAJOR)
      #define S64_FMT        "lld"
      #define U64_FMT        "llu"
      #define X64_FMT        "llX"
      #define S64_SUFFIX(x)  (x##LL)
      #define U64_SUFFIX(x)  (x##ULL)

    #elif defined(__MINGW32__)
      #define S64_FMT        "I64d"
      #define U64_FMT        "I64u"
      #define X64_FMT        "l64X"
      #define S64_SUFFIX(x)  (x##LL)
      #define U64_SUFFIX(x)  (x##ULL)
    #endif

  #elif defined(_MSC_VER) || defined(_MSC_EXTENSIONS) || \
        defined(__WATCOMC__) || defined(__LCC__) || defined(__BORLANDC__)
    #define S64_FMT          "I64d"
    #define U64_FMT          "I64u"
    #define X64_FMT          "I64X"
    #define S64_SUFFIX(x)    (x##i64)
    #define U64_SUFFIX(x)    (x##Ui64)

  #else
    #define S64_FMT          "Ld"
    #define U64_FMT          "Lu"
    #define X64_FMT          "LX"
    #define S64_SUFFIX(x)    (x##LL)
    #define U64_SUFFIX(x)    (x##ULL)
  #endif
#endif

/* Printing a wide string on Windows.
 * E.g. printf (buf, "%"WIDESTR_FMT, wide_str);
 *
 * Note: we don't build with -D_UNICODE. Hence Ascii formats sometimes
 *       need to print wide-chars using this format.
 */
#if defined(__GNUC__)
  #define WIDESTR_FMT  "S"
#elif defined(__POCC__)
  #define WIDESTR_FMT  "ls"
#else
  #define WIDESTR_FMT  "ws"
#endif

/* Printing an hex linear address.
 * Printing an decimal value from the address-bus (e.g. a stack-limit)
 * E.g. printf (buf, "0x%"ADDR_FMT, ADDR_CAST(ptr));
 */
#if defined(WIN64) && defined(_MSC_VER)
  #define ADDR_FMT     "016I64X"
  #define ADDR_CAST(x) ((uint64)(x))
  #define ABUS_VAL_FMT U64_FMT

 /* 64-bit CygWin or MinGW64
  */
#elif defined(WIN64) && (defined(__CYGWIN__) || defined(__MINGW64_VERSION_MAJOR))
  #define ADDR_FMT     "016llX"
  #define ADDR_CAST(x) ((uint64)(x))
  #define ABUS_VAL_FMT U64_FMT

#elif defined(WIN64) && defined(__GNUC__)    /* (MinGW64 usually) */
  #define ADDR_FMT     "016I64X"             /* Or "016llX" ? */
  #define ADDR_CAST(x) ((uint64)(x))
  #define ABUS_VAL_FMT U64_FMT

#elif (DOSX != 0)
  #define ADDR_FMT     "08lX"
  #define ADDR_CAST(x) ((DWORD_PTR)(x)) /* "cl -Wp64" warns here. Ignore it. */
  #define ABUS_VAL_FMT "u"              /* should match an UINT_PTR */

#else
  #define ADDR_FMT     "04X"     /* real-mode segment or offset. */
  #define ADDR_CAST(x) ((WORD)(x))
  #define ABUS_VAL_FMT "u"
#endif

/*
 * Setting console colours. Windows only.
 */
#if defined(WIN32) || defined(WIN64)
  #define SET_ATTR(a)   SetConsoleTextAttribute (stdout_hnd, (console_info.wAttributes & ~7) | \
                                                 (FOREGROUND_INTENSITY | (a)))
  #define NORM_TEXT()   SetConsoleTextAttribute (stdout_hnd, console_info.wAttributes & ~FOREGROUND_INTENSITY)
  #define HIGH_TEXT()   SetConsoleTextAttribute (stdout_hnd, console_info.wAttributes | FOREGROUND_INTENSITY)
  #define YELLOW_TEXT() SET_ATTR (FOREGROUND_GREEN | FOREGROUND_RED)
  #define RED_TEXT()    SET_ATTR (FOREGROUND_RED)
#else
  #define SET_ATTR(a)   ((void)0)
  #define NORM_TEXT()   ((void)0)
  #define HIGH_TEXT()   ((void)0)
  #define YELLOW_TEXT() ((void)0)
  #define RED_TEXT()    ((void)0)
#endif

/*
 * In pc_cbrk.c
 */
extern WORD         _watt_handle_cbreak;
extern volatile int _watt_cbroke;

extern BOOL          tcp_cbreak_off (void);
extern int           set_cbreak (int want_brk);
extern void MS_CDECL sig_handler_watt (int sig);

#define NEW_BREAK_MODE(old_mode,new_mode) \
        old_mode = _watt_handle_cbreak, \
        _watt_handle_cbreak = new_mode,  /* enable special interrupt mode */ \
        _watt_cbroke = 0

#define OLD_BREAK_MODE(old_mode) \
        _watt_cbroke = 0,                /* always clean up */ \
        _watt_handle_cbreak = old_mode


#if defined(__MSDOS__)
  /*
   * In country.c. Used only in IDNA code.
   */
  #define GetCountryCode  W32_NAMESPACE (GetCountryCode)
  #define GetCodePage     W32_NAMESPACE (GetCodePage)

  extern int GetCountryCode (void);
  extern int GetCodePage (void);

  /*
   * In crit.c; Sets up a critical error handler.
   * No longer used.
   */
  #define CRIT_VECT      0x24
  #define int24_init     W32_NAMESPACE (int24_init)
  #define int24_restore  W32_NAMESPACE (int24_restore)

  extern void int24_init (void);
  extern void int24_restore (void);

  /*
   * In qmsg.c
   */
  #define dbg_colour    W32_NAMESPACE (dbg_colour)
  #define dbg_scrpos    W32_NAMESPACE (dbg_scrpos)
  #define dbg_scrstart  W32_NAMESPACE (dbg_scrstart)
  #define dputch        W32_NAMESPACE (dputch)
  #define dmsg          W32_NAMESPACE (dmsg)
  #define dhex1int      W32_NAMESPACE (dhex1int)
  #define dhex2int      W32_NAMESPACE (dhex2int)
  #define dhex4int      W32_NAMESPACE (dhex4int)
  #define dhex8int      W32_NAMESPACE (dhex8int)

  extern BYTE dbg_colour;
  extern WORD dbg_scrpos;
  extern WORD dbg_scrstart;

  extern void dputch   (char x);
  extern void dmsg     (const char *s);
  extern void dhex1int (int x);
  extern void dhex2int (int x);
  extern void dhex4int (int x);
  extern void dhex8int (DWORD x);
#endif  /* __MSDOS__ */

/*
 * In crc.c
 */
#define crc_init   W32_NAMESPACE (crc_init)
#define crc_bytes  W32_NAMESPACE (crc_bytes)
#define crc_table  W32_NAMESPACE (crc_table)

extern DWORD *crc_table;
extern BOOL   crc_init  (void);
extern DWORD  crc_bytes (const char *buf, size_t len);

/*
 * In neterr.c
 */
#define short_strerror      W32_NAMESPACE (short_strerror)
#define pull_neterr_module  W32_NAMESPACE (pull_neterr_module)

extern int         pull_neterr_module;
extern const char *short_strerror (int errnum);


/*
 * IREGS structures for pkt_api_entry() etc.
 * Defines to simplify issuing real-mode interrupts
 */
#if (DOSX & PHARLAP)
  #define IREGS      SWI_REGS     /* in Pharlap's <pharlap.h> */
  #define r_flags    flags
  #define r_ax       eax
  #define r_bx       ebx
  #define r_dx       edx
  #define r_cx       ecx
  #define r_si       esi
  #define r_di       edi
  #define r_ds       ds
  #define r_es       es
  #define r_fs       fs
  #define r_gs       gs

#elif (DOSX & DJGPP)
  #define IREGS      __dpmi_regs  /* in <dpmi.h> */
  #define r_flags    x.flags
  #define r_ax       d.eax
  #define r_bx       d.ebx
  #define r_dx       d.edx
  #define r_cx       d.ecx
  #define r_si       d.esi
  #define r_di       d.edi
  #define r_ip       x.ip
  #define r_cs       x.cs
  #define r_ds       x.ds
  #define r_es       x.es
  #define r_fs       x.fs
  #define r_gs       x.gs
  #define r_ss       x.ss
  #define r_sp       x.sp

#elif (DOSX & X32VM)
  #define IREGS      SWI_REGS           /* in "x32vm.h" */

#elif defined(__CCDL__)
  #define IREGS      union _dpmi_regs_  /* in <dpmi.h> */
  #define r_flags    h.flags
  #define r_ax       d.eax
  #define r_bx       d.ebx
  #define r_dx       d.edx
  #define r_cx       d.ecx
  #define r_si       d.esi
  #define r_di       d.edi
  #define r_ds       h.ds
  #define r_es       h.es
  #define r_fs       h.fs
  #define r_gs       h.gs

#elif (DOSX & (DOS4GW|POWERPAK))
  typedef struct DPMI_regs {
          DWORD  r_di;
          DWORD  r_si;
          DWORD  r_bp;
          DWORD  reserved;
          DWORD  r_bx;
          DWORD  r_dx;
          DWORD  r_cx;
          DWORD  r_ax;
          WORD   r_flags;
          WORD   r_es, r_ds, r_fs, r_gs;
          WORD   r_ip, r_cs, r_sp, r_ss;
        } IREGS;

#elif defined(__MSDOS__)  /* r-mode targets */

  /* IREGS must have same layout and size as Borland's 'struct REGPACK'
   * and Watcom's 'union REGPACK'. This is checked in `check_reg_struct()'.
   */
  #include <sys/pack_on.h>

  typedef struct IREGS {
          WORD  r_ax, r_bx, r_cx, r_dx, r_bp;
          WORD  r_si, r_di, r_ds, r_es, r_flags;
        } IREGS;

  #include <sys/pack_off.h>

  #if (defined(_MSC_VER) || defined(__DMC__)) && (DOSX == 0)
    #define intr W32_NAMESPACE (intr)
    extern void intr (int int_no, IREGS *regs);
  #endif
#endif

#if !defined(WIN32)
  #define CARRY_BIT  1   /* LSB in r_flags */
#endif

/* Macro to ease generation of real-mode interrupts.
 * `_r' should be `IREGS'.
 */
#if (DOSX & DJGPP)
  #define GEN_INTERRUPT(_i,_r)   __dpmi_int ((int)(_i), _r)

#elif (DOSX & (PHARLAP|X32VM))
  #define GEN_INTERRUPT(_i,_r)   _dx_real_int ((UINT)(_i), _r)

#elif defined(__CCDL__)
  #define GEN_INTERRUPT(_i,_r)   dpmi_simulate_real_interrupt (_i, _r)

#elif (DOSX & (DOS4GW|POWERPAK))
  #define GEN_INTERRUPT(_i,_r)   dpmi_real_interrupt ((_i), _r)

#elif (DOSX == 0)
  #if defined(_MSC_VER) || defined(__DMC__)
    #define GEN_INTERRUPT(_i,_r)  intr ((int)(_i), _r) /* our own version */

  #elif defined(__WATCOMC__)
    #define GEN_INTERRUPT(_i,_r)  intr ((int)(_i), (union REGPACK*)(_r))

  #else
    #define GEN_INTERRUPT(_i,_r)  intr ((int)(_i), (struct REGPACK*)(_r))
  #endif

#elif (DOSX & WINWATT)
  /* No interrupts on Win32 */

#else
  #error Help, unknown target.
#endif


/*
 * Fixes for Quick-C and Digital Mars to allow an address as lvalue
 * in FP_SEG/OFF macros (only used in wdpmi.c)
 */
#if (!DOSX)
  #if defined(_MSC_VER)
  /*#pragma warning (disable:4759) */
    #undef  FP_SEG
    #undef  FP_OFF
    #define FP_SEG(p) ((unsigned)(_segment)(void _far *)(p))
    #define FP_OFF(p) ((unsigned)(p))

  #elif defined(__DMC__)
    #undef  FP_SEG
    #undef  FP_OFF
    #define FP_SEG(p) ((unsigned)((DWORD)(void _far*)(p) >> 16))
    #define FP_OFF(p) ((unsigned)(p))
  #endif

#elif defined(BORLAND386)
  #undef  FP_SEG
  #undef  FP_OFF
  #define FP_SEG(p)   _DS  /* segment of something is always our DS */
  #define FP_OFF(p)   (DWORD)(p)

#elif (defined(DMC386) || defined(MSC386)) && defined(__MSDOS__)
  #undef  FP_SEG
  #undef  FP_OFF
  #define FP_SEG(p)   ((unsigned)((DWORD)(void _far*)(p) >> 16))
  #define FP_OFF(p)   ((unsigned)(p))
#endif


#if (defined(MSC386) || defined(__HIGHC__)) && defined(__MSDOS__)
  #include <pldos32.h>
  #define dosdate_t  _dosdate_t
  #define dostime_t  _dostime_t
#endif

#if defined(__DMC__) && defined(__MSDOS__)
  #define dosdate_t  dos_date_t
  #define dostime_t  dos_time_t
#endif


/*
 * The Watcom STACK_SET() function is from Dan Kegel's RARP implementation
 */
#if defined(__WATCOMC__) && !defined(__386__)  /* 16-bit Watcom */
  extern void STACK_SET (void far *stack);
  #pragma aux STACK_SET = \
          "mov  ax, ss"   \
          "mov  bx, sp"   \
          "mov  ss, dx"   \
          "mov  sp, si"   \
          "push ax"       \
          "push bx"       \
          parm [dx si]    \
          modify [ax bx];

  extern void STACK_RESTORE (void);
  #pragma aux STACK_RESTORE = \
          "pop bx"            \
          "pop ax"            \
          "mov ss, ax"        \
          "mov sp, bx"        \
          modify [ax bx];

  extern void PUSHF_CLI (void);
  #pragma aux PUSHF_CLI = \
          "pushf"         \
          "cli";

  extern void POPF (void);
  #pragma aux POPF = \
          "popf";

#elif defined(WATCOM386)       /* 32-bit Watcom targets */
  extern void STACK_SET (void *stack);
  #pragma aux STACK_SET =  \
          "mov  ax, ss"    \
          "mov  ebx, esp"  \
          "mov  cx, ds"    \
          "mov  ss, cx"    \
          "mov  esp, esi"  \
          "push eax"       \
          "push ebx"       \
          parm [esi]       \
          modify [eax ebx ecx];

  extern void STACK_RESTORE (void);
  #pragma aux STACK_RESTORE = \
          "lss esp, [esp]";

  extern void PUSHF_CLI (void);
  #pragma aux PUSHF_CLI = \
          "pushfd"        \
          "cli";

  extern void POPF (void);
  #pragma aux POPF = \
          "popfd";

  extern WORD watcom_MY_CS (void);
  #pragma aux watcom_MY_CS = \
          "mov ax, cs"       \
          modify [ax];

  extern WORD watcom_MY_DS(void);
  #pragma aux watcom_MY_DS = \
          "mov ax, ds"       \
          modify [ax];

  extern DWORD GET_LIMIT (WORD sel);
  #pragma aux GET_LIMIT = \
          ".386p"         \
          "lsl eax, eax"  \
          parm [eax];

  #define get_cs_limit() GET_LIMIT (watcom_MY_CS())
  #define get_ds_limit() GET_LIMIT (watcom_MY_DS())

#elif defined(BORLAND386) || defined(DMC386) || defined(MSC386)
  #define STACK_SET(stk)  __asm { mov  ax, ss;   \
                                  mov  ebx, esp; \
                                  mov  cx, ds;   \
                                  mov  ss, cx;   \
                                  mov  esp, stk; \
                                  push eax;      \
                                  push ebx       \
                                }

  #define STACK_RESTORE() __asm lss esp, [esp]
  #define PUSHF_CLI()     __emit__ (0x9C,0xFA)  /* pushfd; cli */
  #define POPF()          __emit__ (0x9D)       /* popfd */

  extern DWORD get_ds_limit (void);
  extern DWORD get_cs_limit (void);
  extern DWORD get_ss_limit (void);  /* only needed for X32 */

#elif defined(__CCDL__)
  #define STACK_SET(stk)  asm { mov  ax, ss;   \
                                mov  ebx, esp; \
                                mov  cx, ds;   \
                                mov  ss, cx;   \
                                mov  esp, stk; \
                                push eax;      \
                                push ebx       \
                              }

  #define STACK_RESTORE() asm lss esp, [esp]
  #define PUSHF_CLI()     asm { pushfd; cli }
  #define POPF()          asm popfd

  extern DWORD get_ds_limit (void);
  extern DWORD get_cs_limit (void);

#elif defined(__HIGHC__)
  #define PUSHF_CLI()     _inline (0x9C,0xFA)  /* pushfd; cli */
  #define POPF()          _inline (0x9D)       /* popfd */

#elif defined(__GNUC__) && defined(__i386__)
  #define PUSHF_CLI()     __asm__ __volatile__ ("pushfl; cli" ::: "memory")
  #define POPF()          __asm__ __volatile__ ("popfl"       ::: "memory")

#elif (defined(__SMALL__) || defined(__LARGE__)) && !defined(__SMALL32__)
  #if defined(__BORLANDC__) /* prevent spawning tasm.exe */
    #define PUSHF_CLI()   __emit__ (0x9C,0xFA)
    #define POPF()        __emit__ (0x9D)

  #elif defined(__DMC__) || (defined(_MSC_VER) && !defined(NO_INLINE_ASM))
    #define PUSHF_CLI()   __asm pushf; \
                          __asm cli
    #define POPF()        __asm popf
  #endif
#endif

#if !defined(__WATCOMC__)  /* Because these are pragmas on Watcom */
  #ifndef PUSHF_CLI
  #define PUSHF_CLI() ((void)0)
  #endif

  #ifndef POPF
  #define POPF()      ((void)0)
  #endif
#endif


#define BEEP_FREQ   7000
#define BEEP_MSEC   1

#if defined(WIN32) || defined(WIN64)
  #define BEEP()    MessageBeep (MB_OK)

#elif defined(_MSC_VER)
  #define BEEP()    ((void)0) /* doesn't even have delay() :-< */

#elif defined(__DMC__)
  #define BEEP()    sound_note (BEEP_FREQ, BEEP_MSEC)

#elif defined(__DJGPP__)
  #define BEEP()    do { \
                      sound (BEEP_FREQ); \
  /* delay() don't */ usleep (1000*BEEP_MSEC); \
  /* work under NT */ nosound(); \
                    } while (0)

#elif defined(__CCDL__)
  #define BEEP()    do { \
                      _sound (BEEP_FREQ); \
                      _delay (BEEP_MSEC); \
                      _nosound(); \
                    } while (0)

#elif defined(__HIGHC__)   /* Limited <conio.h> */
  #define BEEP()    putchar (7)

#else
  #define BEEP()    do { \
                      sound (BEEP_FREQ); \
                      delay (BEEP_MSEC); \
                      nosound(); \
                    } while (0)
#endif

/* Macros for sleeping. Reference on djgpp's usleep().
 */
#if (defined(WIN32) || defined(WIN64)) && !defined(WATT32_DJGPP_MINGW)
  #ifndef __MINGW32__
  #define usleep(us)      Sleep((us)/1000)
  #endif
  #define uclock()        clock()
  #define UCLOCKS_PER_SEC CLOCKS_PER_SEC
  #define uclock_t        clock_t

#elif defined(__WATCOMC__) || defined(__HIGHC__)
  #define usleep(us)      delay((us)/1000)
  #define uclock()        clock()
  #define UCLOCKS_PER_SEC CLOCKS_PER_SEC
  #define uclock_t        clock_t
#endif


/*
 * Defines for real/pmode-mode interrupt handlers (crit.c, netback.c and pcintr.c)
 */
#if !defined(INTR_PROTOTYPE)
  #if (DOSX == 0)
    #if (__WATCOMC__ >= 1200)   /* OpenWatcom 1.0+ */
      #define INTR_PROTOTYPE  void interrupt far
      typedef INTR_PROTOTYPE (*W32_IntrHandler)();

    #elif defined(__TURBOC__)
      #define INTR_PROTOTYPE  void cdecl interrupt
      typedef void interrupt (*W32_IntrHandler)(void);

    #else
      #define INTR_PROTOTYPE  void cdecl interrupt
      typedef void (cdecl interrupt *W32_IntrHandler)();
    #endif

  #elif defined(WIN32) || defined(WIN64)
    /* No need for this on Windows */

  #elif defined(WATCOM386)
    #define INTR_PROTOTYPE  void __interrupt __far
    typedef INTR_PROTOTYPE (*W32_IntrHandler)();

  #elif defined(__HIGHC__)
    #define INTR_PROTOTYPE  _Far void _CC (_INTERRUPT|_CALLING_CONVENTION)
    typedef INTR_PROTOTYPE (*W32_IntrHandler)();

  #elif defined(__DJGPP__)
    #define INTR_PROTOTYPE  void   /* simply a SIGALRM handler */
    typedef void (*W32_IntrHandler)(int);

  #else
    /* Figure out what to do for others .. */
  #endif
#endif


#if defined(__i386__) && (DOSX & DJGPP) /* also for gcc -m486 and better */
 /*
  * This is not used yet since the benefits/drawbacks are unknown.
  * Define 32-bit multiplication asm macros.
  *
  * umul_ppmm (high_prod, low_prod, multiplier, multiplicand)
  * multiplies two unsigned long integers multiplier and multiplicand,
  * and generates a two unsigned word product in high_prod and
  * low_prod.
  */
  #define umul_ppmm(w1,w0,u,v)                           \
          __asm__ __volatile__ (                         \
                 "mull %3"                               \
               : "=a" ((DWORD)(w0)), "=d" ((DWORD)(w1))  \
               : "%0" ((DWORD)(u)),  "rm" ((DWORD)(v)))

  #define mul32(u,v) ({ union ulong_long w;               \
                        umul_ppmm (w.s.hi, w.s.lo, u, v); \
                        w.ull; })
  /* Use as:
   *  DWORD x,y;
   *  ..
   *  uint64 z = mul32 (x,y);
   */

  /* Borrowed from djgpp's <sys/farptr.h>
   * Needed in pkt_receiver_pm() because djgpp 2.03 doesn't
   * save/restore FS/GS registers in the rmode callback stub.
   * Not needed if 'USE_FAST_PKT' is used.
   */
  W32_GCC_INLINE WORD get_fs_reg (void)
  {
     WORD sel;
     __asm__ __volatile__ (
             "movw %%fs, %w0"
           : "=r" (sel) : );
     return (sel);
  }
  W32_GCC_INLINE WORD get_gs_reg (void)
  {
     WORD sel;
     __asm__ __volatile__ (
             "movw %%gs, %w0"
           : "=r" (sel) : );
     return (sel);
  }
  W32_GCC_INLINE void set_fs_reg (WORD sel)
  {
    __asm__ __volatile__ (
            "movw %w0, %%fs"
         :: "rm" (sel));
  }
  W32_GCC_INLINE void set_gs_reg (WORD sel)
  {
    __asm__ __volatile__ (
            "movw %w0, %%gs"
         :: "rm" (sel));
  }
#else
  #define get_fs_reg()    (1)
  #define get_gs_reg()    (1)
  #define set_fs_reg(fs)  ((void)0)
  #define set_gs_reg(gs)  ((void)0)
#endif /* __i386__ && (DOSX & DJGPP) */


/*
 * Macros for formatted printing.
 *
 * Note: Some versions of snprintf() return -1 if they truncate
 *       the output. Others return size of truncated output.
 */
#undef SNPRINTF
#undef VSNPRINTF

#if defined(__CYGWIN__)
  #define SNPRINTF   snprintf
  #define VSNPRINTF  vsnprintf

#elif defined(WIN32)
  #define SNPRINTF   _snprintf
  #define VSNPRINTF  _vsnprintf

#elif defined(__HIGHC__) || defined(__WATCOMC__)
  #define SNPRINTF   _bprintf
  #define VSNPRINTF  _vbprintf

#elif defined(__DMC__) || (defined(_MSC_VER) && (_MSC_VER >= 700))
  #define SNPRINTF   _snprintf
  #define VSNPRINTF  _vsnprintf

#elif defined(__DJGPP__) && (__DJGPP_MINOR__ >= 4)
  #define SNPRINTF   snprintf
  #define VSNPRINTF  vsnprintf
#endif

#define UNCONST(type, var, val)   (*(type *)&(var)) = val

#ifdef __cplusplus
};
#endif

#endif /* _w32_MISC_H */

