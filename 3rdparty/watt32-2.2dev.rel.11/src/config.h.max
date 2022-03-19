/*!\file config.h
 *
 * Defines global library options.
 */

#ifndef _w32_CONFIG_H
#define _w32_CONFIG_H

#if defined(_w32_WATTCP_H) && !defined(_w32_TARGET_H)
#error CONFIG.H must be included after TARGET.H
#endif

/**
 * Change '#undef' to '#define' to include an option.
 *
 * \note
 *   Unless you know exactly what you're doing, you MUST do "make clean"
 *   after adding/removing any of the following defines. Not all make
 *   programs are smart enough to figure out the dependencies.
 */
#undef USE_DEBUG       /* Include debug code */
#undef USE_MULTICAST   /* Include IP multicast code by Jim Martin */
#undef USE_BIND        /* Include Bind resolver code */
#undef USE_BSD_API     /* Include BSD-sockets */
#undef USE_BSD_FATAL   /* Exit application on BSD-socket fatal errors */
#undef USE_BOOTP       /* Include BOOTP client code */
#undef USE_DHCP        /* Include DHCP boot client code */
#undef USE_RARP        /* Include RARP boot client code. Contributed by Dan Kegel. */
#undef USE_GEOIP       /* Include GeoIP support. From Tor's geoip.c */
#undef USE_IPV6        /* Include IPv6 dual-stack support */
#undef USE_LANGUAGE    /* Include Language translation code */
#undef USE_FRAGMENTS   /* Include IP-fragment handling */
#undef USE_STATISTICS  /* Include protocol statistics count */
#undef USE_FORTIFY     /* Use Fortify malloc-lib for allocations */
#undef USE_MPATROL     /* Use mpatrol malloc-lib for allocations etc. */
#undef USE_STACKWALKER /* Use StackWalker for allocations and crash-dumps. */
#undef USE_FSEXT       /* Use djgpp's File Extensions for file I/O functions */
#undef USE_LOOPBACK    /* Use the simple loopback device */
#undef USE_EMBEDDED    /* Make an embeddable (ROM-able) target. See note at end */
#undef USE_BUFFERED_IO /* Use buffered file I/O in pcconfig.c */
#undef USE_TFTP        /* Include TFTP protocol for simple file retrival */
#undef USE_UDP_ONLY    /* Exclude all stuff related to the TCP protocol */
#undef USE_TCP_SACK    /* Include TCP Selective ACK feature (not yet) */
#undef USE_ECHO_DISC   /* Include echo/discard servers */
#undef USE_PPPOE       /* PPP-over-Ethernet encapsulation */
#undef USE_RS232_DBG   /* Include RS232 serial port writing of trace-info */
#undef USE_FAST_PKT    /* Use faster all real-mode PKTDRVR receiver (DOSX only) */
#undef USE_DEAD_GWD    /* Dead Gateway detection in ARP module */
#undef USE_GZIP        /* Use gzip compression for PPP and/or pcap debug file. Also for SCTP. */
#undef USE_PROFILER    /* Include simple execution profiler (only DOSX with 64-bit types) */
#undef USE_TCP_MD5     /* TCP MD5 signature option */
#undef USE_IDNA        /* Support national characters in DNS name lookups. */
#undef USE_DYNIP_CLI   /* Support DynDNS.org name/IP client */
#undef USE_SCTP        /* Stream Control Transfer Protocol; experimental */

/*
 * Building small-model applications doesn't leave
 * much room for the fancy stuff :-(
 */
#if !defined(OPT_DEFINED) && defined(__SMALL__) && !defined(__SMALL32__)
  #define USE_UDP_ONLY     /* test udp-only (cookie,ping) */
  #define OPT_DEFINED
#endif

/*
 * MS's Quick-C (6.0 at least) is severly limited in many ways..
 */
#if !defined(OPT_DEFINED) && (defined(_MSC_VER) && (_MSC_VER <= 600) && defined(__LARGE__))
  #define OPT_DEFINED
#endif

/*
 * For other large models define these options:
 */
#if !defined(OPT_DEFINED) && defined(__LARGE__)
  #define USE_DEBUG
  #define OPT_DEFINED
#endif

/*
 * Otherwise, for all targets define these options:
 */
#if !defined(OPT_DEFINED)
  #define USE_DEBUG
  #define USE_BOOTP
  #define USE_DHCP
  #define USE_LANGUAGE
  #define USE_FRAGMENTS
  #define USE_STATISTICS
  #define USE_BIND
  #define USE_BSD_API
  #define USE_BSD_FATAL
  #define USE_LOOPBACK
  #define USE_BUFFERED_IO
  #define USE_TFTP
  #define USE_MULTICAST
  #define OPT_DEFINED
#endif

/*
 * Add some more options for djgpp, HighC, 32-bit Watcom/DMC and Win32/64
 */
#if defined(__DJGPP__) || defined(__HIGHC__) || defined(WATCOM386) || \
    defined(DMC386) || defined(WIN32) || defined(WIN64)
  #define USE_ECHO_DISC
  #define USE_RARP
  #define USE_IPV6
  #define USE_IDNA
  #define USE_TCP_MD5
  #define USE_GZIP
  #define USE_DYNIP_CLI
//#define USE_SCTP
  #define USE_DEAD_GWD
#endif

#if defined(__DJGPP__)
  #define USE_FSEXT
/*#define USE_RS232_DBG */
/*#define USE_FORTIFY */
#endif

#if defined(__HIGHC__) || defined(__BORLANDC__)
/* #define USE_FORTIFY */
#endif

#if (DOSX && DOSX != WINWATT)
  #define USE_FAST_PKT
#endif

#if (DOSX) && defined(HAVE_UINT64)
  #define USE_PROFILER
#endif


/*
 * SCTP needs gzip compress/uncompress.
 */
#if defined(USE_SCTP)
  #define USE_GZIP
#endif

/*
 * Select malloc debuggers on Windows
 */
#if defined(WIN32) && defined(_MSC_VER) && defined(_DEBUG)  /* cl -MDd -D_DEBUG */
  #define USE_CRTDBG                       /* use CrtDebug; faster than Fortify? */
/*#define USE_MPATROL */

#elif defined(__MINGW32__) && !defined(NDEBUG)
/*#define USE_MPATROL */
/*#define USE_FORTIFY */

#elif defined(WIN32) && defined(__WATCOMC__)
/* #define USE_STACKWALKER */ /* Not possible */

#elif defined(__CYGWIN__) && !defined(NDEBUG)
/* #define USE_FORTIFY */
#endif

#if defined(WIN32) && defined(_MSC_VER) && !defined(__clang__)
 /*
  * Use BugTrap DLL loaded at runtime.
  * Ref: http://www.intellesoft.net/default.shtml and
  *      https://github.com/bchavez/BugTrap
  */
/*#define USE_BUGTRAP */
/*#define USE_STACKWALKER */
#endif

/*
 * When generating a Swig wrapper for Python, misc.c is included
 * in the Swig interface file (watt32_swig.i). Then we must use
 * whatever mallocer python*.lib was built with.
 */
#if defined(SWIG)
  #undef USE_IPV6
  #undef USE_SCTP
  #undef USE_CRTDBG
  #undef USE_BUGTRAP
  #undef USE_MPATROL
  #undef USE_FORTIFY
#endif

#if defined(_MSC_VER) && 0
  #undef USE_FRAGMENTS
  #undef USE_MULTICAST
  #undef USE_DHCP
  #undef USE_RARP
  #undef USE_IPV6
  #undef USE_SCTP
  #undef USE_LANGUAGE
  #undef USE_BUFFERED_IO
  #undef USE_TFTP
  #undef USE_LOOPBACK
  #undef USE_FORTIFY
  #undef USE_BIND
#endif

/*
 * Test some illegal combinations.
 */
#if defined(USE_IPV6) && !defined(USE_BSD_API)
#error USE_IPV6 requires USE_BSD_API
#endif

#if defined(USE_BIND) && !defined(USE_BSD_API)
#error USE_BIND requires USE_BSD_API
#endif

#if defined(USE_SCTP) && !defined(USE_IPV6)
#error USE_SCTP requires USE_IPV6
#endif

#if defined(USE_UDP_ONLY) && defined(USE_BSD_API)
#error USE_UDP_ONLY and USE_BSD_API are meaningless
#endif

#if defined(USE_UDP_ONLY) && defined(USE_DYNIP_CLI)
#error USE_UDP_ONLY and USE_DYNIP_CLI are meaningless
#endif

#if defined(USE_PROFILER) && (!DOSX || !defined(HAVE_UINT64))
#error Cannot define USE_PROFILER for this target
#endif

#if defined(__TURBOC__) && (__TURBOC__ <= 0x301)
  #if defined(USE_BIND) || defined(USE_SOCKET) || defined(USE_STATISTICS)
  #error tcc 2.0 cannot compile resolver, socket code or statistics
  #endif
#endif

#if defined(USE_FORTIFY) && defined(USE_MPATROL)
  #error Cannot use both Fortify and mpatrol malloc libs.
#endif

#if defined(USE_FORTIFY) && defined(USE_CRTDBG)
  #error Cannot use both Fortify and CrtDebug.
#endif

#if defined(USE_MPATROL) && defined(USE_CRTDBG)
  #error Cannot use both mpatrol malloc libs and CrtDebug.
#endif

#if defined(USE_MPATROL) && defined(USE_STACKWALKER)
  #error Cannot use mpatrol with StackWalker.
#endif

/**
 * \note
 *  If including `#define USE_EMBEDDED', it's users responsibility to
 *  decide what code should be disabled for embedded targets. Sprinkle
 *  the code with `#if defined(USE_EMBEDDED)' to include special embedded
 *  features; like making `outch()' send to a RS-232 port.
 *  Insert `#if !defined(USE_EMBEDDED)' exclude things;
 *  such as preventing calls to file/disk related functions.
 */

/*
 * Hack fix: When building with djgpp 2.04, but the
 *           ./util/dj_err.exe was compiled with 2.03, it's
 *           necessary to define the following.
 */
#undef MIXING_DJGPP_203_AND_204

/*
 * If building .html-documentation with doxygen, document everything.
 * Add some stuff doxygen doesn't know about.
 */
#if defined(DOXYGEN)
  #define W32_NAMESPACE(x)  x
  #define W32_CALL
  #define __far
  #define far
  #define __interrupt
  #define interrupt
  #define __cdecl
  #define cdecl
  #define USE_BIND
  #define USE_BSD_API
  #define USE_BSD_FATAL
  #define USE_BOOTP
  #define USE_DEBUG
  #define USE_DHCP
  #define USE_MULTICAST
  #define USE_GEOIP
  #define USE_RARP
  #define USE_IPV6
  #define USE_LANGUAGE
  #define USE_FRAGMENTS
  #define USE_STATISTICS
  #define USE_FORTIFY
  #define USE_FSEXT
  #define USE_LOOPBACK
  #define USE_EMBEDDED
  #define USE_BUFFERED_IO
  #define USE_TFTP
  #define USE_UDP_ONLY
  #define USE_TCP_SACK
  #define USE_ECHO_DISC
  #define USE_RS232_DBG
  #define USE_FAST_PKT
  #define USE_TCP_MD5
#endif

#if defined(USE_GZIP)
/*#define Z_NO_INFLATE */ /* we don't need to read gzip streams */
  #define Z_PREFIX        /* use 'z_' prefix for zlib functions */
  #define FASTEST         /* use fastest deflate method */
/*#define ASMV */         /* use assembly version of longest_match() */
  #define NO_vsnprintf
  #define Z_PRINTF_BUFSIZE 1024
#endif

/*
 * A small HAVE_x section here.
 */
#if defined(WIN32) || defined(WIN64)
  #define HAVE_WINDNS_H
#endif

#if (defined(__BORLANDC__) && (__BORLANDC__ < 0x5600)) || \
     defined(__SALFORDC__) || defined(__DMC__)
  #undef HAVE_WINDNS_H
#endif

#endif /* _w32_CONFIG_H */

