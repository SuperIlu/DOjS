/*!\file sys/select.h
 *
 * Compatibility header.
 */

#if defined(__CYGWIN__)
  #include_next <sys/select.h>
#else
  #include <tcp.h>   /* select_s() */
#endif

