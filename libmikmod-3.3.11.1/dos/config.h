/* config.h.in.  Generated manually for DJGPP.  */

/* djgpp-v2.04 and newer provide snprintf() and vsnprintf().
 * djgpp-v2.05 is already released, so let's enable them by
 * default here. */
/* Define if you have the snprintf function.  */
#define HAVE_SNPRINTF
/* Define if you have the vsnprintf function.  */
#define HAVE_VSNPRINTF

/* Define to disable the high quality mixer (build only with the standart mixer) */
/*#define NO_HQMIXER*/

/* Define if your system supports binary pipes (i.e. Unix) */
/*#define DRV_PIPE*/
/* Define if you want support for output to stdout */
/*#define DRV_STDOUT*/

/* Define if you want an .aiff file writer driver */
#define DRV_AIFF
/* Define if you want a raw pcm data file writer driver */
#define DRV_RAW
/* Define if you want a .wav file writer driver */
#define DRV_WAV

/* Define if the Gravis UltraSound driver is compiled */
#define DRV_ULTRA
/* Define if the Windows Sound System driver is compiled */
#define DRV_WSS
/* Define if the SoundBlaster driver is compiled */
#define DRV_SB

/* Define if you want a debug version of the library */
#undef MIKMOD_DEBUG

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS

/* Define if you have the setenv function.  */
#define HAVE_SETENV

/* Define if you have the srandom function.  */
#define HAVE_SRANDOM

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H

/* Define if you have the <limits.h> header file. */
#define HAVE_LIMITS_H

/* Define if you have the <malloc.h> header file.  */
#define HAVE_MALLOC_H

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H

