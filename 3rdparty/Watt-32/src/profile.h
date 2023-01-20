/*!\file profile.h
 */
#ifndef _w32_PROFILE_H
#define _w32_PROFILE_H

#if defined(USE_PROFILER)
  #define profile_start   W32_NAMESPACE (profile_start)
  #define profile_stop    W32_NAMESPACE (profile_stop)
  #define profile_init    W32_NAMESPACE (profile_init)
  #define profile_on      W32_NAMESPACE (profile_on)
  #define profile_dump    W32_NAMESPACE (profile_dump)
  #define profile_recv    W32_NAMESPACE (profile_recv)
  #define profile_file    W32_NAMESPACE (profile_file)
  #define profile_enable  W32_NAMESPACE (profile_enable)

  extern BOOL profile_enable;
  extern char profile_file [MAX_PATHLEN+1];

  extern void profile_start (const char *where);
  extern void profile_stop  (void);
  extern int  profile_init  (void);
  extern int  profile_on    (void);
  extern void profile_dump  (const uint64 *data, size_t num_elem);
  extern void profile_recv  (uint64 put, uint64 get);

  #define PROFILE_START(func)   profile_start (func)
  #define PROFILE_STOP()        profile_stop()
  #define PROFILE_RECV(put,get) profile_recv (put,get)

#else
  #define PROFILE_START(func)   ((void)0)
  #define PROFILE_STOP()        ((void)0)
  #define PROFILE_RECV(put,get) ((void)0)
#endif

#endif
