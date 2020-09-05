/*
 *  Errnos: program to produce list of all errno's and
 *          sys_errlist[] for any specific vendor.
 *
 *  This program must be compiled using compiler environment
 *  for each vendor.
 *
 *  G. Vanem <gvanem@yahoo.no - Jan 1999
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <time.h>

#if defined(_MSC_VER) && defined(_VCRUNTIME_H) && !defined(_CRT_NO_POSIX_ERROR_CODES) && 0
  #error "Add '-D_CRT_NO_POSIX_ERROR_CODES' to your CFLAGS."
#endif

#if defined(__MSDOS__) || defined(WIN32) || defined(_WIN32)
  #include <io.h>
#else /* assume building on unix for cross compilations (BROKEN!) */
  #include <sys/types.h>
  #include <unistd.h>
#endif

/*
 * Generating a mw64_err.exe for MinGW64 is a major hassle. Since:
 *  1) cannot run a x86-64 program on Win32.
 *  2) cannot use a mingw32-hosted environment that emulates mingw64.
 *
 * Hence we need to run 'i686-w64-mingw32-gcc' to build mw64_err.exe.
 * This MUST be the same gcc version as the 'x86_64-w64-mingw32-gcc'
 * that we're building for. Check with 'x86_64-w64-mingw32-gcc -v'.
 */
#if defined(MAKE_MINGW64_ERRNOS) && 0
  #define __MINGW64__
#else
  #include <errno.h>
#endif

#if defined(__CYGWIN__)
  /*
   * Since CygWin's <sys/errno.h> provided all the errno-values
   * we need, there is no need to use util/errno.c to create new
   * one for CygWin. Simply pull in <sys/errno.h>.
   */
  #error This program is not needed on CygWin
#endif

#if defined(__MSDOS__) || defined(WIN32) || defined(_WIN32)
  #include <sys/wtypes.h>     /* fd_set, FD_SET() */
#endif

#if defined(_MSC_VER) && (_MSC_VER <= 700)
  #define OLD_MSC
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1400) && 0
 /*
  * Not sure about the above MSVC version, but recent versions of MSVC's
  * <errno.h> has this for it's "POSIX SUPPLEMENT":
  *   #define EADDRINUSE  100
  *   ...
  * But strerror() for these (above 100) always return "Unknown error".
  * Not a very good "POSIX SUPPLEMENT". Hence we #undef them here.
  *
  * This doesn't work; have to define with same value, but add a sensible strerror().
  *
  * One option would be to define '_CRT_NO_POSIX_ERROR_CODES' everywhere when
  * compiling with MSVC and the Windows-Kit. But that sounds a bit extreme.
  */
  #undef EADDRINUSE
  #undef EADDRNOTAVAIL
  #undef EAFNOSUPPORT
  #undef EALREADY
  #undef EBADMSG
  #undef ECANCELED
  #undef ECONNABORTED
  #undef ECONNREFUSED
  #undef ECONNRESET
  #undef EDESTADDRREQ
  #undef EHOSTUNREACH
  #undef EIDRM
  #undef EINPROGRESS
  #undef EISCONN
  #undef ELOOP
  #undef EMSGSIZE
  #undef ENETDOWN
  #undef ENETRESET
  #undef ENETUNREACH
  #undef ENOBUFS
  #undef ENODATA
  #undef ENOLINK
  #undef ENOMSG
  #undef ENOPROTOOPT
  #undef ENOSR
  #undef ENOSTR
  #undef ENOTCONN
  #undef ENOTRECOVERABLE
  #undef ENOTSOCK
  #undef ENOTSUP
  #undef EOPNOTSUPP
  #undef EOTHER
  #undef EOVERFLOW
  #undef EOWNERDEAD
  #undef EPROTO
  #undef EPROTONOSUPPORT
  #undef EPROTOTYPE
  #undef ETIME
  #undef ETIMEDOUT
  #undef ETXTBSY
  #undef EWOULDBLOCK
#endif

#if defined(MAKE_MINGW64_ERRNOS) && 0
  #define EPERM        1
  #define ENOENT       2
  #define ENOFILE      ENOENT
  #define ESRCH        3
  #define EINTR        4
  #define EIO          5
  #define ENXIO        6
  #define E2BIG        7
  #define ENOEXEC      8
  #define EBADF        9
  #define ECHILD       10
  #define EAGAIN       11
  #define ENOMEM       12
  #define EACCES       13
  #define EFAULT       14
  #define EBUSY        16
  #define EEXIST       17
  #define EXDEV        18
  #define ENODEV       19
  #define ENOTDIR      20
  #define EISDIR       21
  #define ENFILE       23
  #define EMFILE       24
  #define ENOTTY       25
  #define EFBIG        27
  #define ENOSPC       28
  #define ESPIPE       29
  #define EROFS        30
  #define EMLINK       31
  #define EPIPE        32
  #define EDOM         33
  #define EDEADLK      36
  #define EDEADLOCK    EDEADLK
  #define ENAMETOOLONG 38
  #define ENOLCK       39
  #define ENOSYS       40
  #define ENOTEMPTY    41
  #define EINVAL       22
  #define ERANGE       34
  #define EILSEQ       42
#endif   /* MAKE_MINGW64_ERRNOS */


static int    print_errno   = 0;
static int    print_errlist = 0;
static int    print_test    = 0;
static int    last_errno    = 0;
static fd_set errno_set;
static const  char *prog_name;

static void process (void);
static void prologue (void);
static void epilogue (void);
static void add_errno (int errnum, const char *errnum_str, const char *strerr);

static void Usage (void)
{
  fprintf (stderr, "syntax: %s [-e | -s | -t]\n", prog_name);
  fprintf (stderr, "   -e: generate print of errno's\n");
  fprintf (stderr, "   -s: generate print of sys_errlist[]\n");
  fprintf (stderr, "   -t: test strerror() values of C-lib\n");
  exit (0);
}

int main (int argc, char **argv)
{
  prog_name = argv[0];

  if (argc > 1)
  {
    if (!strncmp(argv[1],"-e",2))
         print_errno = 1;
    else if (!strncmp(argv[1],"-s",2))
         print_errlist = 1;
    else if (!strncmp(argv[1],"-t",2))
         print_test = 1;
    else Usage();
  }

  if (!print_errno && !print_errlist && !print_test)
     Usage();

  FD_ZERO (&errno_set);
  prologue();
  process();
  epilogue();
  return (0);
}


static const char *VendorName (void)
{
#if defined(__DJGPP__)
  return ("__DJGPP__");

#elif defined(__HIGHC__)
  return ("__HIGHC__");

#elif defined(__BORLANDC__)
  return ("__BORLANDC__");

#elif defined(__TURBOC__)
  return ("__TURBOC__");

#elif defined(__WATCOMC__)
  return ("__WATCOMC__");

#elif defined(__POCC__)
  return ("__POCC__");

#elif defined(__clang__)
  return ("__clang__");

#elif defined(_MSC_VER)
  return ("_MSC_VER");

#elif defined(__DMC__)
  return ("__DMC__");

#elif defined(__MINGW64__)
  return ("__MINGW64__");

#elif defined(__MINGW32__)
  return ("__MINGW32__");

#elif defined(__CYGWIN__)
  return ("__CYGWIN__");

#elif defined(__CCDL__)
  return ("__CCDL__");

#elif defined(__LCC__)
  return ("__LCC__");

#else
  return ("??__UNKNOWN__");
#endif
}

static const char *VendorVersion (void)
{
  static char buf[10];

#if defined(__DJGPP__)
  sprintf (buf, "%d.%02d", __DJGPP__, __DJGPP_MINOR__);

#elif defined(__DMC__)
  sprintf (buf, "%X.%X", __DMC__ >> 8, __DMC__ & 0xFF);

#elif defined(__WATCOMC__)
  sprintf (buf, "%d.%d", __WATCOMC__/100, __WATCOMC__ % 100);

#elif defined(__BORLANDC__)
  sprintf (buf, "%X.%X", __BORLANDC__ >> 8, __BORLANDC__ & 0xFF);

#elif defined(__TURBOC__)
  sprintf (buf, "%X.%X", (__TURBOC__ >> 8) - 1, __TURBOC__ & 0xFF);

#elif defined(__POCC__)
  sprintf (buf, "%d.%d", __POCC__/100, __POCC__ % 100);

#elif defined(__clang__)
  sprintf (buf, "%d.%d", __clang_major__, __clang_minor__);

#elif defined(_MSC_VER)
  sprintf (buf, "%d.%d", _MSC_VER/100, _MSC_VER % 100);

#elif defined(__MINGW32__) && defined(__MINGW64_VERSION_MAJOR)
  sprintf (buf, "%d.%d", __MINGW64_VERSION_MAJOR, __MINGW64_VERSION_MINOR);

#elif defined(__MINGW32__) && defined(__MINGW_MAJOR_VERSION)  /* MingW-RT 4.0+ */
  sprintf (buf, "%d.%d", __MINGW_MAJOR_VERSION, __MINGW_MINOR_VERSION);

#elif defined(__MINGW32__)
  sprintf (buf, "%d.%d", __MINGW32_MAJOR_VERSION, __MINGW32_MINOR_VERSION);

#elif defined(__CYGWIN__)
  sprintf (buf, "%d.%d.%d", CYGWIN_VERSION_DLL_MAJOR/1000, CYGWIN_VERSION_DLL_MAJOR % 1000,
           CYGWIN_VERSION_DLL_MINOR);

#elif defined(__CCDL__)
  sprintf (buf, "%d.%d", __CCDL__/100, __CCDL__ % 100);

#elif defined(__LCC__)
/*sprintf (buf, "%d.%d", __LCC__/100, __LCC__ % 100); doesn't work */
  return (NULL);

#else
  buf[0] = '\0';
#endif

  return (buf);
}

static void prologue (void)
{
  const char *vendor_name = VendorName();
  const char *vendor_ver  = VendorVersion();
  const char *now;
  time_t t;

  time (&t);
  now = ctime (&t);

#if defined(__HIGHC__)       /* Metaware is wrong by one! */
  last_errno = sys_nerr - 1;
#elif defined(__WATCOMC__)   /* ditto for Watcom (misses EILSEQ) */
  last_errno = sys_nerr + 1;
#elif defined(__POCC__)      /* PellesC is rather limited */
  last_errno = EILSEQ;
#else
  last_errno = sys_nerr;
#endif

  if (print_errno)
  {
    const char *comment = "";

#if defined(WATT32_DJGPP_MINGW)
    comment = "But since djgpp programs normally does not run under Windows, "
              "it was generated by using MinGW/gcc.\n";
#endif

    printf ("#ifndef __SYS_WERRNO_ERR\n"
            "#define __SYS_WERRNO_ERR\n\n"
            "/*\n"
            " * THIS FILE WAS GENERATED BY %s at %.24s.\n"
            " * DO NOT EDIT.\n%s"
            " *\n"
            " * Watt-32 errnos are after vendor's errnos (1 - %d)\n"
            " */\n\n", prog_name, now, comment, last_errno-1);
  }

  if (print_errlist)
     printf ("/*\n"
             " * THIS FILE WAS GENERATED BY %s at %.24s.\n"
             " * DO NOT EDIT.\n"
             " *\n"
             " * Watt-32 sys_errlist replaces vendor's sys_errlist[]\n"
             " */\n\n", prog_name, now);

  if (print_test)
     printf ("The vendor's (%s) C-lib defines only these errnos (1 - %d):\n",
             vendor_name, last_errno-1);

#if defined(OLD_MSC)
   /* MSC 7.0 (or older) doesn't seem to handle the below construct
    */
#else
  if (print_errno || print_errlist)
  {
    printf ("#ifndef %s\n"
            "#error This file is only for use by \"%s\"\n"
            "#endif\n\n", vendor_name, vendor_name);
  }
#endif
  if (print_errno && vendor_ver)
     printf ("#define ERRNO_VENDOR_VERSION  \"%s\"\n\n", vendor_ver);

  if (print_errlist)
     printf ("char __syserr000[] = \"Unknown error\";\n");
}

static void epilogue (void)
{
  if (print_errno)
     printf ("\n#endif /* __SYS_WERRNO_ERR */\n");

  if (print_errlist)
  {
    int i;

    printf ("\nchar *SYS_ERRLIST[] = {");

    for (i = 0; i < last_errno; i++)
    {
      if (i % 5 == 0)
         printf ("\n   ");
      if (FD_ISSET(i, &errno_set))
           printf (" __syserr%03d, ", i);
      else printf (" __syserr000, ");
    }
    printf ("\n};\n\n");
  }
}

static const char *err_tab[] = {
  "Operation would block (EWOULDBLOCK)",                     /* 0  */
  "Input to function out of range (EDOM)",
  "Output of function out of range (ERANGE)",                /* 2  */
  "Argument list too long (E2BIG)",
  "Permission denied (EACCES)",                              /* 4  */
  "Resource temporarily unavailable (EAGAIN)",
  "Bad file descriptor (EBADF)",                             /* 6  */
  "Resource busy (EBUSY)",
  "No child processes (ECHILD)",                             /* 8  */
  "Resource deadlock avoided (EDEADLK)",
  "File exists (EEXIST)",                                    /* 10 */
  "Bad address (EFAULT)",
  "File too large (EFBIG)",                                  /* 12 */
  "Interrupted system call (EINTR)",
  "Invalid argument (EINVAL)",                               /* 14 */
  "Input or output error (EIO)",
  "Is a directory (EISDIR)",                                 /* 16 */
  "Too many open files (EMFILE)",
  "Too many links (EMLINK)",                                 /* 18 */
  "File name too long (ENAMETOOLONG)",
  "Too many open files in system (ENFILE)",                  /* 20 */
  "No such device (ENODEV)",
  "No such file or directory (ENOENT)",                      /* 22 */
  "Unable to execute file (ENOEXEC)",
  "No locks available (ENOLCK)",                             /* 24 */
  "Not enough memory (ENOMEM)",
  "No space left on drive (ENOSPC)",                         /* 26 */
  "Function not implemented (ENOSYS)",
  "Not a directory (ENOTDIR)",                               /* 28 */
  "Directory not empty (ENOTEMPTY)",
  "Inappropriate I/O control operation (ENOTTY)",            /* 30 */
  "No such device or address (ENXIO)",
  "Operation not permitted (EPERM)",                         /* 32 */
  "Broken pipe (EPIPE)",
  "Read-only file system (EROFS)",                           /* 34 */
  "Invalid seek (ESPIPE)",
  "No such process (ESRCH)",                                 /* 36 */
  "Improper link (EXDEV)",
  "No more files (ENMFILE)",                                 /* 38 */
  "Operation now in progress (EINPROGRESS)",
  "Operation already in progress (EALREADY)",                /* 40 */
  "Socket operation on non-socket (ENOTSOCK)",
  "Destination address required (EDESTADDRREQ)",             /* 42 */
  "Message too long (EMSGSIZE)",
  "Protocol wrong type for socket (EPROTOTYPE)",             /* 44 */
  "Protocol option not available (ENOPROTOOPT)",
  "Protocol not supported (EPROTONOSUPPORT)",                /* 46 */
  "Socket type not supported (ESOCKTNOSUPPORT)",
  "Operation not supported on socket (EOPNOTSUPP)",          /* 48 */
  "Protocol family not supported (EPFNOSUPPORT)",
  "Address family not supported by protocol (EAFNOSUPPORT)", /* 50 */
  "Address already in use (EADDRINUSE)",
  "Can't assign requested address (EADDRNOTAVAIL)",          /* 52 */
  "Network is down (ENETDOWN)",
  "Network is unreachable (ENETUNREACH)",                    /* 54 */
  "Network dropped connection on reset (ENETRESET)",
  "Software caused connection abort (ECONNABORTED)",         /* 56 */
  "Connection reset by peer (ECONNRESET)",
  "No buffer space available (ENOBUFS)",                     /* 58 */
  "Socket is already connected (EISCONN)",
  "Socket is not connected (ENOTCONN)",                      /* 60 */
  "Can't send after socket shutdown (ESHUTDOWN)",
  "Connection timed out (ETIMEDOUT)",                        /* 62 */
  "Connection refused (ECONNREFUSED)",
  "Host is down (EHOSTDOWN)",                                /* 64 */
  "No route to host (EHOSTUNREACH)",
  "Stale NFS file handle (ESTALE)",                          /* 66 */
  "Too many levels of remote in path (EREMOTE)",
  "RPC struct is bad (EBADRPC)",                             /* 68 */
  "RPC version wrong (ERPCMISMATCH)",
  "RPC prog. not avail (EPROGUNAVAIL)",                      /* 70 */
  "RPC Program version wrong (EPROGMISMATCH)",
  "Bad procedure for program (EPROCUNAVAIL)",                /* 72 */
  "Multibyte/wide character encoding error (EILSEQ)",

  /* Add special vendor codes here
   */
  "Invalid function number (EINVFNC)",                       /* 74 */
  "Path not found (ENOPATH)",
  "Memory area destroyed (ECONTR)",                          /* 76 */
  "Invalid memory block address (EINVMEM)",
  "Invalid environment (EINVENV)",                           /* 78 */
  "Invalid format (EINVFMT)",
  "Invalid access code (EINVACC)",                           /* 80 */
  "Invalid data (EINVDAT)",
  "Locking violation (EDEADLOCK)",                           /* 82 */
  "Attempt to remove current directory (ECURDIR)",
  "Not same device (ENOTSAM)",                               /* 84 */
  "Text file busy (ETXTBSY)",
  "Block device required (ENOTBLK)",                         /* 86 */
  "Structure needs cleaning (EUCLEAN)",
  "Too many references (ETOOMANYREFS)",                      /* 88 */
  "Too many levels of symbolic links (ELOOP)",
  "Too many processes (EPROCLIM)",                           /* 90 */
  "Too many users (EUSERS)",
  "Disc quota exceeded (EDQUOT)",                            /* 92 */
  "RVD related disk error (EVDBAD)",
  "Out of remote working directory stuctures (ENORMTWD)",    /* 94 */
  "Value too large (EOVERFLOW)",
};

#define ADD_ERRNO(err_num)  add_errno (err_num, #err_num, strerror(err_num))
#define NEW_ERRNO(tab_num)  add_errno (-1, NULL, err_tab[tab_num])

static void process (void)
{
#ifdef EDOM
  ADD_ERRNO (EDOM);
#else
  NEW_ERRNO (1);
#endif

#ifdef ERANGE
  ADD_ERRNO (ERANGE);
#else
  NEW_ERRNO (2);
#endif

#ifdef E2BIG
  ADD_ERRNO (E2BIG);
#else
  NEW_ERRNO (3);
#endif

#ifdef EACCES
  ADD_ERRNO (EACCES);
#else
  NEW_ERRNO (4);
#endif

#ifdef EAGAIN
  ADD_ERRNO (EAGAIN);
#else
  NEW_ERRNO (5);
#endif

#ifdef EBADF
  ADD_ERRNO (EBADF);
#else
  NEW_ERRNO (6);
#endif

#ifdef EBUSY
  ADD_ERRNO (EBUSY);
#else
  NEW_ERRNO (7);
#endif

#ifdef ECHILD
  ADD_ERRNO (ECHILD);
#else
  NEW_ERRNO (8);
#endif

#ifdef EDEADLK
  ADD_ERRNO (EDEADLK);
#else
  NEW_ERRNO (9);
#endif

#ifdef EEXIST
  ADD_ERRNO (EEXIST);
#else
  NEW_ERRNO (10);
#endif

#ifdef EFAULT
  ADD_ERRNO (EFAULT);
#else
  NEW_ERRNO (11);
#endif

#ifdef EFBIG
  ADD_ERRNO (EFBIG);
#else
  NEW_ERRNO (12);
#endif

#ifdef EINTR
  ADD_ERRNO (EINTR);
#else
  NEW_ERRNO (13);
#endif

#ifdef EINVAL
  ADD_ERRNO (EINVAL);
#else
  NEW_ERRNO (14);
#endif

#ifdef EIO
  ADD_ERRNO (EIO);
#else
  NEW_ERRNO (15);
#endif

#ifdef EISDIR
  ADD_ERRNO (EISDIR);
#else
  NEW_ERRNO (16);
#endif

#ifdef EMFILE
  ADD_ERRNO (EMFILE);
#else
  NEW_ERRNO (17);
#endif

#ifdef EMLINK
  ADD_ERRNO (EMLINK);
#else
  NEW_ERRNO (18);
#endif

#ifdef ENAMETOOLONG
  ADD_ERRNO (ENAMETOOLONG);
#else
  NEW_ERRNO (19);
#endif

#ifdef ENFILE
  ADD_ERRNO (ENFILE);
#else
  NEW_ERRNO (20);
#endif

#ifdef ENODEV
  ADD_ERRNO (ENODEV);
#else
  NEW_ERRNO (21);
#endif

#ifdef ENOENT
  ADD_ERRNO (ENOENT);
#else
  NEW_ERRNO (22);
#endif

#ifdef ENOEXEC
  ADD_ERRNO (ENOEXEC);
#else
  NEW_ERRNO (23);
#endif

#ifdef ENOLCK
  ADD_ERRNO (ENOLCK);
#else
  NEW_ERRNO (24);
#endif

#ifdef ENOMEM
  ADD_ERRNO (ENOMEM);
#else
  NEW_ERRNO (25);
#endif

#ifdef ENOSPC
  ADD_ERRNO (ENOSPC);
#else
  NEW_ERRNO (26);
#endif

#ifdef ENOSYS
  ADD_ERRNO (ENOSYS);
#else
  NEW_ERRNO (27);
#endif

#ifdef ENOTDIR
  ADD_ERRNO (ENOTDIR);
#else
  NEW_ERRNO (28);
#endif

#ifdef ENOTEMPTY
  ADD_ERRNO (ENOTEMPTY);
#else
  NEW_ERRNO (29);
#endif

#ifdef ENOTTY
  ADD_ERRNO (ENOTTY);
#else
  NEW_ERRNO (30);
#endif

#ifdef ENXIO
  ADD_ERRNO (ENXIO);
#else
  NEW_ERRNO (31);
#endif

#ifdef EPERM
  ADD_ERRNO (EPERM);
#else
  NEW_ERRNO (32);
#endif

#ifdef EPIPE
  ADD_ERRNO (EPIPE);
#else
  NEW_ERRNO (33);
#endif

#ifdef EROFS
  ADD_ERRNO (EROFS);
#else
  NEW_ERRNO (34);
#endif

#ifdef ESPIPE
  ADD_ERRNO (ESPIPE);
#else
  NEW_ERRNO (35);
#endif

#ifdef ESRCH
  ADD_ERRNO (ESRCH);
#else
  NEW_ERRNO (36);
#endif

#ifdef EXDEV
  ADD_ERRNO (EXDEV);
#else
  NEW_ERRNO (37);
#endif

#ifdef ENMFILE
  ADD_ERRNO (ENMFILE);
#else
  NEW_ERRNO (38);
#endif

#ifdef ELOOP
  ADD_ERRNO (ELOOP);
#else
  NEW_ERRNO (89);
#endif

#ifdef EOVERFLOW
  ADD_ERRNO (EOVERFLOW);
#else
  NEW_ERRNO (95);
#endif

#ifdef EILSEQ
  ADD_ERRNO (EILSEQ);
#else
  NEW_ERRNO (73);
#endif

#ifdef EWOULDBLOCK
  ADD_ERRNO (EWOULDBLOCK);
#else
  NEW_ERRNO (0);
#endif

#ifdef EINPROGRESS
  ADD_ERRNO (EINPROGRESS);
#else
  NEW_ERRNO (39);
#endif

#ifdef EALREADY
  ADD_ERRNO (EALREADY);
#else
  NEW_ERRNO (40);
#endif

#ifdef ENOTSOCK
  ADD_ERRNO (ENOTSOCK);
#else
  NEW_ERRNO (41);
#endif

#ifdef EDESTADDRREQ
  ADD_ERRNO (EDESTADDRREQ);
#else
  NEW_ERRNO (42);
#endif

#ifdef EMSGSIZE
  ADD_ERRNO (EMSGSIZE);
#else
  NEW_ERRNO (43);
#endif

#ifdef EPROTOTYPE
  ADD_ERRNO (EPROTOTYPE);
#else
  NEW_ERRNO (44);
#endif

#ifdef ENOPROTOOPT
  ADD_ERRNO (ENOPROTOOPT);
#else
  NEW_ERRNO (45);
#endif

#ifdef EPROTONOSUPPORT
  ADD_ERRNO (EPROTONOSUPPORT);
#else
  NEW_ERRNO (46);
#endif

#ifdef ESOCKTNOSUPPORT
  ADD_ERRNO (ESOCKTNOSUPPORT);
#else
  NEW_ERRNO (47);
#endif

#ifdef EOPNOTSUPP
  ADD_ERRNO (EOPNOTSUPP);
#else
  NEW_ERRNO (48);
#endif

#ifdef EPFNOSUPPORT
  ADD_ERRNO (EPFNOSUPPORT);
#else
  NEW_ERRNO (49);
#endif

#ifdef EAFNOSUPPORT
  ADD_ERRNO (EAFNOSUPPORT);
#else
  NEW_ERRNO (50);
#endif

#ifdef EADDRINUSE
  ADD_ERRNO (EADDRINUSE);
#else
  NEW_ERRNO (51);
#endif

#ifdef EADDRNOTAVAIL
  ADD_ERRNO (EADDRNOTAVAIL);
#else
  NEW_ERRNO (52);
#endif

#ifdef ENETDOWN
  ADD_ERRNO (ENETDOWN);
#else
  NEW_ERRNO (53);
#endif

#ifdef ENETUNREACH
  ADD_ERRNO (ENETUNREACH);
#else
  NEW_ERRNO (54);
#endif

#ifdef ENETRESET
  ADD_ERRNO (ENETRESET);
#else
  NEW_ERRNO (55);
#endif

#ifdef ECONNABORTED
  ADD_ERRNO (ECONNABORTED);
#else
  NEW_ERRNO (56);
#endif

#ifdef ECONNRESET
  ADD_ERRNO (ECONNRESET);
#else
  NEW_ERRNO (57);
#endif

#ifdef ENOBUFS
  ADD_ERRNO (ENOBUFS);
#else
  NEW_ERRNO (58);
#endif

#ifdef EISCONN
  ADD_ERRNO (EISCONN);
#else
  NEW_ERRNO (59);
#endif

#ifdef ENOTCONN
  ADD_ERRNO (ENOTCONN);
#else
  NEW_ERRNO (60);
#endif

#ifdef ESHUTDOWN
  ADD_ERRNO (ESHUTDOWN);
#else
  NEW_ERRNO (61);
#endif

#ifdef ETIMEDOUT
  ADD_ERRNO (ETIMEDOUT);
#else
  NEW_ERRNO (62);
#endif

#ifdef ECONNREFUSED
  ADD_ERRNO (ECONNREFUSED);
#else
  NEW_ERRNO (63);
#endif

#ifdef EHOSTDOWN
  ADD_ERRNO (EHOSTDOWN);
#else
  NEW_ERRNO (64);
#endif

#ifdef EHOSTUNREACH
  ADD_ERRNO (EHOSTUNREACH);
#else
  NEW_ERRNO (65);
#endif

#ifdef ESTALE
  ADD_ERRNO (ESTALE);
#else
  NEW_ERRNO (66);
#endif

#ifdef EREMOTE
  ADD_ERRNO (EREMOTE);
#else
  NEW_ERRNO (67);
#endif

#ifdef EBADRPC
  ADD_ERRNO (EBADRPC);
#else
  NEW_ERRNO (68);
#endif

#ifdef ERPCMISMATCH
  ADD_ERRNO (ERPCMISMATCH);
#else
  NEW_ERRNO (69);
#endif

#ifdef EPROGUNAVAIL
  ADD_ERRNO (EPROGUNAVAIL);
#else
  NEW_ERRNO (70);
#endif

#ifdef EPROGMISMATCH
  ADD_ERRNO (EPROGMISMATCH);
#else
  NEW_ERRNO (71);
#endif

#ifdef EPROCUNAVAIL
  ADD_ERRNO (EPROCUNAVAIL);
#else
  NEW_ERRNO (72);
#endif

#ifdef EINVFNC
  ADD_ERRNO (EINVFNC);
#else
  NEW_ERRNO (74);
#endif

#ifdef ENOPATH
  ADD_ERRNO (ENOPATH);
#else
  NEW_ERRNO (75);
#endif

#ifdef ECONTR
  ADD_ERRNO (ECONTR);
#else
  NEW_ERRNO (76);
#endif

#ifdef EINVMEM
  ADD_ERRNO (EINVMEM);
#else
  NEW_ERRNO (77);
#endif

#ifdef EINVENV
  ADD_ERRNO (EINVENV);
#else
  NEW_ERRNO (78);
#endif

#ifdef EINVFMT
  ADD_ERRNO (EINVFMT);
#else
  NEW_ERRNO (79);
#endif

#ifdef EINVACC
  ADD_ERRNO (EINVACC);
#else
  NEW_ERRNO (80);
#endif

#ifdef EINVDAT
  ADD_ERRNO (EINVDAT);
#else
  NEW_ERRNO (81);
#endif

#ifdef EDEADLOCK
  ADD_ERRNO (EDEADLOCK);
#else
  NEW_ERRNO (82);
#endif

#ifdef ECURDIR
  ADD_ERRNO (ECURDIR);
#else
  NEW_ERRNO (83);
#endif

#ifdef ENOTSAM
  ADD_ERRNO (ENOTSAM);
#else
  NEW_ERRNO (84);
#endif

#ifdef ETXTBSY
  ADD_ERRNO (ETXTBSY);
#else
  NEW_ERRNO (85);
#endif

#ifdef ENOTBLK
  ADD_ERRNO (ENOTBLK);
#else
  NEW_ERRNO (86);
#endif

#ifdef EUCLEAN
  ADD_ERRNO (EUCLEAN);
#else
  NEW_ERRNO (87);
#endif

#ifdef ETOOMANYREFS
  ADD_ERRNO (ETOOMANYREFS);
#else
  NEW_ERRNO (88);
#endif

#ifdef EPROCLIM
  ADD_ERRNO (EPROCLIM);
#else
  NEW_ERRNO (90);
#endif

#ifdef EUSERS
  ADD_ERRNO (EUSERS);
#else
  NEW_ERRNO (91);
#endif

#ifdef EDQUOT
  ADD_ERRNO (EDQUOT);
#else
  NEW_ERRNO (92);
#endif

#ifdef EVDBAD
  ADD_ERRNO (EVDBAD);
#else
  NEW_ERRNO (93);
#endif

#ifdef ENORMTWD
  ADD_ERRNO (ENORMTWD);
#else
  NEW_ERRNO (94);
#endif
}

/*
 * strip off newlines
 */
static const char *rip_nl (const char *s)
{
  static char buf[100];
  char  *p;

  strcpy (buf, s);
  if ((p = strchr(buf,'\n')) != NULL) *p = '\0';
  if ((p = strchr(buf,'\r')) != NULL) *p = '\0';
  return (buf);
}

/*
 * strip off paranthesis
 */
static const char *rip_par (const char *s)
{
  static char buf[100];
  char  *p;

  strcpy (buf, s);
  p = strchr (buf, '(');
  if (p)
  {
    *p-- = '\0';
    if (*p == ' ')
        *p = '\0';
  }
  return (buf);
}

static const char *get_errno (const char *s)
{
  static char buf[100];
  char  *p, *q;

  strcpy (buf, s);
  p = strchr (buf, '(');
  if (p)
     p++;

  q = strchr (buf, ')');
  if (q)
    *q = '\0';

  if (p && q)
     return ((const char*)p);
  return (NULL);
}

static void add_errno (int errnum, const char *errnum_str, const char *strerr)
{
  int dummy = 0;

  if (errnum < 0)
  {
    add_errno (last_errno++, errnum_str, strerr);
    return;
  }

 /* Since Turbo-C's strerror() is buggy
  */
#if defined(__TURBOC__) && !defined(__BORLANDC__)
  if ((errnum_str && sscanf(errnum_str,"%d",&dummy)) == 1 ||
      !strncmp(strerr,"-1",2))
     return;
#endif

  if (!errnum_str)          /* we're adding our own */
  {
    errnum_str = get_errno (strerr);
    assert (errnum_str);
  }
  else if (print_test)
  {
    /** \todo: should make the output sorted on 'errnum' */
    printf ("  %3d=%-15s -> \"%s\"\n", errnum, errnum_str, rip_nl(strerr));
    fflush (stdout);
    return;
  }

  if (print_errno)
  {
    /** \todo: should make the output sorted on 'errnum' */
    printf ("#define %-17s %d\n", errnum_str, errnum);
  }

  if (print_errlist && !FD_ISSET(errnum,&errno_set))
  {
    strerr = rip_par (rip_nl(strerr));
    if (*strerr == '\0')
    {
      char buf[30];
      sprintf (buf, "System error %d", errnum);
      strerr = buf;
    }

    /** \todo: should make the output sorted on "__syserr'errnum'" */

    printf ("char __syserr%03d[] = \"%s (%s)\";\n",
            errnum, strerr, errnum_str);
    FD_SET (errnum, &errno_set);
  }

  (void) dummy;
}

