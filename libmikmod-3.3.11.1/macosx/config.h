/* config.h.in.  Generated for Mac OS X builds.  */

/* Define if you have the posix_memalign function. */
/* posix_memalign this requires OSX 10.6 or newer! */
/* #undef HAVE_POSIX_MEMALIGN */

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
#define HAVE_SYS_WAIT_H 1

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* disable the high quality mixer (build only with the standart mixer) */
/* #define NO_HQMIXER */

/* Define if you want support for output to stdout */
#define DRV_STDOUT 1

/* Define if your system supports binary pipes (i.e. Unix) */
#define DRV_PIPE 1

/* Define if you want an .aiff file writer driver */
#define DRV_AIFF 1

/* Define if you want a raw pcm data file writer driver */
#define DRV_RAW 1

/* Define if you want a .wav file writer driver */
#define DRV_WAV 1

/* Define if the OpenAL driver is compiled */
/* #undef DRV_OPENAL */

/* Define this if you want the MacOS X CoreAudio driver */
#define DRV_OSX 1

/* Define if you want a debug version of the library */
/* #undef MIKMOD_DEBUG */

/* Define if your system provides POSIX.4 threads */
#define HAVE_PTHREAD 1

/* Define if your system defines random(3) and srandom(3) in math.h instead
   of stdlib.h */
/* #undef SRANDOM_IN_MATH_H */

/* Define if your system has RTLD_GLOBAL defined in <dlfcn.h> */
#define HAVE_RTLD_GLOBAL 1
/* Define if your system needs leading underscore to function names in dlsym() calls */
/* #undef DLSYM_NEEDS_UNDERSCORE */

/* Define if you have the setenv function.  */
#define HAVE_SETENV 1

/* Define if you have the snprintf function.  */
#define HAVE_SNPRINTF 1

/* Define if you have the srandom function.  */
#define HAVE_SRANDOM 1

/* Define if you have the strstr function.  */
#define HAVE_STRSTR 1

/* Define if you have the <dlfcn.h> header file.  */
#define HAVE_DLFCN_H 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define if you have the <malloc.h> header file.  */
/* #undef HAVE_MALLOC_H */

/* Define if you have the <memory.h> header file.  */
#define HAVE_MEMORY_H 1

/* Define if you have the <sys/ioctl.h> header file.  */
#define HAVE_SYS_IOCTL_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

