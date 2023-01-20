/*!\file sock_scn.c
 *
 * scanf-like function for sock_type.
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "copyrigh.h"
#include "wattcp.h"
#include "misc.h"
#include "pctcp.h"

#if defined(__MINGW32__) || defined(__MINGW64__) || defined(__CYGWIN__) || defined(__WATCOMC__)
  #define HAVE_VSSCANF
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1700)  /* VS-Express 2013, cl 18.x */
  #define HAVE_VSSCANF
#endif

#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x0700)  /* CBuilder (bcc32c v 7.0) */
  #define HAVE_VSSCANF
#endif

#if defined(_MSC_VER) && defined(WIN32) && !defined(_M_X64)   /* Not for 64-bit compilers */
/*
 * Converted to MSVC-asm by Gisle Vanem.
 * Originally written for gcc-asm:
 *  By aaronwl 2003-01-28 for mingw-msvcrt.
 *  Public domain: all copyrights disclaimed, absolutely no warranties.
 */
__declspec(naked) static int vsscanf_msvc_asm (const char *s, const char *format, va_list arg)
{
#if 0
  int ret;

  __asm__(

    // allocate stack (esp += frame - arg3 - (8[arg1,2] + 12))
    "movl  %%esp, %%ebx\n\t"
    "lea   0xFFFFFFEC(%%esp, %6), %%esp\n\t"
    "subl  %5, %%esp\n\t"

    // set up stack
    "movl  %1, 0xC(%%esp)\n\t"   // s
    "movl  %2, 0x10(%%esp)\n\t"  // format
    "lea   0x14(%%esp), %%edi\n\t"
    "movl  %%edi, (%%esp)\n\t"   // memcpy dest
    "movl  %5, 0x4(%%esp)\n\t"   // memcpy src
    "movl  %5, 0x8(%%esp)\n\t"
    "subl  %6, 0x8(%%esp)\n\t"   // memcpy len
    "call  _memcpy\n\t"
    "addl  $12, %%esp\n\t"

    "call  _sscanf\n\t"

    "movl  %%ebx, %%esp\n\t"     // restore stack

    : "=a"(ret), "=c"(s), "=d"(format)
    : "1"(s), "2"(format), "S"(arg),
      "a"(&ret)
    : "ebx");

  return ret;

#else
  _asm {
     mov ebx, esp
     lea esp, [esp+6-24]
     sub esp, 5

     mov eax, [ebx+4]
     mov [esp+0Ch], eax   // s
     mov eax, [ebx+8]
     mov [esp+10h], eax   // format

  }
#endif
}
#endif /* _MSC_VER && WIN32 && !_M_X64 */


/*
 * MSC/DMC/DJGPP 2.01 doesn't have vsscanf().
 * On Windows (MSVC) there's nothing to be done yet :-(
 */
#if defined(WIN32) && !defined(HAVE_VSSCANF)
  static int vsscanf (const char *buf, const char *fmt, va_list arglist)
  {
  #if defined(_MSC_VER) && 0
    return vsscanf_msvc_asm (buf, fmt, arglist);
  #else
    ARGSUSED (buf);
    ARGSUSED (fmt);
    ARGSUSED (arglist);
    UNIMPLEMENTED();
    return (0);
  #endif
  }
#elif defined(__DJGPP__) && (DJGPP_MINOR < 2)
  static int vsscanf (const char *buf, const char *fmt, va_list arglist)
  {
    FILE *fil = stdin;
    fil->_ptr = (char*) buf;
    return _doscan (fil, fmt, arglist);
  }
  /* MSVC (on DOS) or DMC (on any) */
#elif (defined(_MSC_VER) && defined(__MSDOS__)) || defined(__DMC__)
  static int vsscanf (const char *buf, const char *fmt, va_list arglist)
  {
    extern _input (FILE*, const char*, va_list);
    FILE *fil = stdin;
    fil->_ptr = (char*) buf;
    return _input (fil, fmt, arglist);
  }
#endif

/*
 * sock_scanf - Read a line and return number of fields matched.
 *
 * BIG WARNING: Don't use this for packetised protocols like
 *              SSH. Only suitable for ASCII orientented protocols
 *              like POP3/SMTP/NNTP etc.
 *
 * NOTE: Don't use {\r\n} in 'fmt' (it's stripped in sock_gets).
 */
int MS_CDECL sock_scanf (sock_type *s, const char *fmt, ...)
{
  char buf [tcp_MaxBufSize];
  int  fields = 0;
  int  status, len;

  while ((status = sock_dataready(s)) == 0)
  {
    if (status == -1)
       return (-1);

    len = sock_gets (s, (BYTE*)buf, sizeof(buf));
    if (len > 0)
    {
      va_list args;
      va_start (args, fmt);
      fields = vsscanf (buf, fmt, args);
      va_end (args);
      break;
    }
  }
  return (fields);
}

