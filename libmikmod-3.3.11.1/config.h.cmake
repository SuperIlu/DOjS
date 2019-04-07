/* config.h --  Generated CMake. */

/* Version number of package */
#cmakedefine VERSION "${VERSION}"

/* ========== Features selection */

/* Define if your system supports binary pipes (i.e. Unix) */
#cmakedefine DRV_PIPE 1
/* Define if you want support for output to stdout */
#cmakedefine DRV_STDOUT 1
/* Define if you want an .aiff file writer driver */
#cmakedefine DRV_AIFF 1
/* Define if you want a .wav file writer driver */
#cmakedefine DRV_WAV 1
/* Define if you want a raw pcm data file writer driver */
#cmakedefine DRV_RAW 1
/* Define if the DEC AudioFile server driver is compiled */
#cmakedefine DRV_AF 1
/* Define if the Amiga AHI driver is compiled */
#cmakedefine DRV_AHI 1
/* Define if the AIX audio driver is compiled */
#cmakedefine DRV_AIX 1
/* Define if the Linux ALSA driver is compiled */
#cmakedefine DRV_ALSA 1
/* Define if the PulseAudio driver is compiled */
#cmakedefine DRV_PULSEAUDIO 1
/* Define if the Enlightened Sound Daemon driver is compiled */
#cmakedefine DRV_ESD 1
/* Define if the HP-UX audio driver is compiled */
#cmakedefine DRV_HP 1
/* Define if the Network Audio System driver is compiled */
#cmakedefine DRV_NAS 1
/* Define if the OpenAL driver is compiled */
#cmakedefine DRV_OPENAL 1
/* Define if the OpenSL ES driver is compiled */
#cmakedefine DRV_OSLES 1
/* Define if the Open Sound System driver is compiled */
#cmakedefine DRV_OSS 1
/* Define if the Linux SAM9407 driver is compiled */
#cmakedefine DRV_SAM9407 1
/* Define if the SDL audio driver is compiled */
#cmakedefine DRV_SDL 1
/* Define if the SGI audio driver is compiled */
#cmakedefine DRV_SGI 1
/* Define if the OpenBSD sndio driver is compiled */
#cmakedefine DRV_SNDIO 1
/* Define if the Sun audio driver or compatible (NetBSD, OpenBSD)
   is compiled */
#cmakedefine DRV_SUN 1
/* Define if the Linux Ultra driver is compiled */
#cmakedefine DRV_ULTRA 1
/* Define this if you want the MacOS X CoreAudio driver */
#cmakedefine DRV_OSX 1
/* Define this if you want the Carbon Mac Audio driver */
#cmakedefine DRV_MAC 1
/* Define if the Windows DirectSound driver is compiled */
#cmakedefine DRV_DS 1
/* Define if the Windows MCI driver is compiled */
#cmakedefine DRV_WIN 1
/* Define if the Windows XAudio2 driver is compiled */
#cmakedefine DRV_XAUDIO2 1
/* Define if using XAudio 2.8 for Windows8 with XAudio2 driver */
#cmakedefine DRV_XAUDIO28 1

/* Define if you want a debug version of the library */
#cmakedefine MIKMOD_DEBUG 1

/* Define if you want runtime dynamic linking of ALSA and EsounD drivers */
#cmakedefine MIKMOD_DYNAMIC 1

/* Define if you want to use SIMD (AltiVec or SSE2) optimizations (Unstable!)  */
#cmakedefine MIKMOD_SIMD 1

/* Define to 0 or 1 to override MIKMOD_UNIX in mikmod_internals.h. */
#cmakedefine MIKMOD_UNIX 1

/* disable the high quality mixer (build only with the standart mixer) */
#cmakedefine  NO_HQMIXER 1

/* ========== Build environment information */

/* Define if your system is SunOS 4.* */
#cmakedefine SUNOS 1
/* Define if your system is AIX 3.* - might be needed for 4.* too */
#cmakedefine AIX 1

/* Define if your system defines random(3) and srandom(3) in math.h instead
   of stdlib.h */
#cmakedefine SRANDOM_IN_MATH_H 1

/* Define if your system has RTLD_GLOBAL defined in <dlfcn.h> */
#cmakedefine HAVE_RTLD_GLOBAL 1

/* Define if your system provides POSIX.4 threads */
#cmakedefine HAVE_PTHREAD 1

/* Define if your system needs leading underscore to function names in dlsym()
   calls */
#cmakedefine DLSYM_NEEDS_UNDERSCORE 1

/* define this if you are running a bigendian system (motorola, sparc, etc) */
#cmakedefine WORDS_BIGENDIAN 1

/* Define to 1 if you have the <AF/AFlib.h> header file. */
#cmakedefine HAVE_AF_AFLIB_H 1

/* Define to 1 if you have the <AL/al.h> header file. */
#cmakedefine HAVE_AL_AL_H 1

/* Define to 1 if you have the <AL/alc.h> header file. */
#cmakedefine HAVE_AL_ALC_H 1

/* Define to 1 if you have the <AL/alext.h> header file. */
#cmakedefine HAVE_AL_ALEXT_H 1

/* Define to 1 if you have the <alsa/asoundlib.h> header file. */
#cmakedefine HAVE_ALSA_ASOUNDLIB_H 1

/* Define to 1 if you have the <audio/audiolib.h> header file. */
#cmakedefine HAVE_AUDIO_AUDIOLIB_H 1

/* Define to 1 if you have the <devices/ahi.h> header file. */
#cmakedefine HAVE_DEVICES_AHI_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H 1

/* Define to 1 if you have the <dl.h> header file. */
#cmakedefine HAVE_DL_H 1

/* Define to 1 if you have the <dmedia/audio.h> header file. */
#cmakedefine HAVE_DMEDIA_AUDIO_H 1

/* Define to 1 if you have the <dsound.h> header file. */
#cmakedefine HAVE_DSOUND_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#cmakedefine HAVE_FCNTL_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H 1

/* Define to 1 if you have the <libgus.h> header file. */
#cmakedefine HAVE_LIBGUS_H 1

/* Define to 1 if you have the <limits.h> header file. */
#cmakedefine HAVE_LIMITS_H 1

/* Define to 1 if you have the <machine/soundcard.h> header file. */
#cmakedefine HAVE_MACHINE_SOUNDCARD_H 1

/* Define to 1 if you have the <malloc.h> header file. */
#cmakedefine HAVE_MALLOC_H 1

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H 1

/* Define to 1 if you have the `posix_memalign' function. */
#cmakedefine HAVE_POSIX_MEMALIGN 1

/* Define to 1 if you have the <pulse/simple.h> header file. */
#cmakedefine HAVE_PULSE_SIMPLE_H 1

/* Define to 1 if you have the `setenv' function. */
#cmakedefine HAVE_SETENV 1

/* Define to 1 if you have the `snprintf' function. */
#cmakedefine HAVE_SNPRINTF 1

/* Define to 1 if you have the `srandom' function. */
#cmakedefine HAVE_SRANDOM 1

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H 1

/* Define to 1 if you have the `memcmp' function. */
#cmakedefine HAVE_MEMCMP 1

/* Define to 1 if you have the <soundcard.h> header file. */
#cmakedefine HAVE_SOUNDCARD_H 1

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H 1

/* Define to 1 if you have the `strstr' function. */
#cmakedefine HAVE_STRSTR 1

/* Define to 1 if you have the <sun/audioio.h> header file. */
#cmakedefine HAVE_SUN_AUDIOIO_H 1

/* Define to 1 if you have the <sys/acpa.h> header file. */
#cmakedefine HAVE_SYS_ACPA_H 1

/* Define to 1 if you have the <sys/audioio.h> header file. */
#cmakedefine HAVE_SYS_AUDIOIO_H 1

/* Define to 1 if you have the <sys/audio.h> header file. */
#cmakedefine HAVE_SYS_AUDIO_H 1

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#cmakedefine HAVE_SYS_IOCTL_H 1

/* Define to 1 if you have the <sys/sam9407.h> header file. */
#cmakedefine HAVE_SYS_SAM9407_H 1

/* Define to 1 if you have the <sys/soundcard.h> header file. */
#cmakedefine HAVE_SYS_SOUNDCARD_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H 1

/* Define to 1 if you have <sys/wait.h> that is POSIX.1 compatible. */
#cmakedefine HAVE_SYS_WAIT_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H 1

/* Define to 1 if you have the <windows.h> header file. */
#cmakedefine HAVE_WINDOWS_H 1

/* Define to 1 if you have the <xaudio2.h> header file. */
#cmakedefine HAVE_XAUDIO2_H 1

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS

/* Define if the C compiler supports the `inline' keyword. */
#cmakedefine HAVE_C_INLINE
/* Define if the C compiler supports the `__inline__' keyword. */
#cmakedefine HAVE_C___INLINE__
/* Define if the C compiler supports the `__inline' keyword. */
#cmakedefine HAVE_C___INLINE
#if !defined(HAVE_C_INLINE) && !defined(__cplusplus)
# ifdef HAVE_C___INLINE__
#  define inline __inline__
# elif defined(HAVE_C___INLINE)
#  define inline __inline
# else
#  define inline
# endif
#endif

/* Define to empty if `const' does not conform to ANSI C. */
#cmakedefine const

/* Define to empty if compiler does not understand the `signed' keyword. */
#cmakedefine signed

/* Define to `int' if <sys/types.h> does not define. */
#cmakedefine pid_t

/* Define to `unsigned int' if <sys/types.h> does not define. */
#cmakedefine size_t
