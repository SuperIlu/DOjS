/* Customized config.h for Android builds */

/* disable the high quality mixer (build only with the standart mixer) */
/* #define NO_HQMIXER */

/* Define if the OpenSL ES driver is compiled */
#define DRV_OSLES 1
/* Define if you want an .aiff file writer driver */
/* #define DRV_AIFF */
/* Define if you want a raw pcm data file writer driver */
/* #define DRV_RAW */
/* Define if you want a .wav file writer driver */
/* #define DRV_WAV */

/* Define if your system provides POSIX.4 threads */
#define HAVE_PTHREAD 1

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#undef WORDS_BIGENDIAN

#define HAVE_UNISTD_H 1
#define HAVE_FCNTL_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_LIMITS_H 1
#define HAVE_SETENV 1
#define HAVE_SNPRINTF 1
#define HAVE_SRANDOM 1
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_STRSTR 1
#define HAVE_SYS_IOCTL_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_WAIT_H 1

