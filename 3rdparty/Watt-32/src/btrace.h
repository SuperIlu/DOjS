#ifndef _w32_BACKTRACE_H
#define _w32_BACKTRACE_H

/*!\file btrace.h
 *
 * A simple stack-tracer inspired by glib.
 * Handy for asserts and crashes.
 *
 * NOT FINISHED.
 */
#define backtrace             W32_NAMESPACE (backtrace)
#define backtrace_init        W32_NAMESPACE (backtrace_init)
#define backtrace_symbols_fd  W32_NAMESPACE (backtrace_symbols_fd)

extern int backtrace_init       (const char *prog);
extern int backtrace            (void **buf, int size);
extern int backtrace_symbols_fd (const void *buf, int size, int file);
extern int traceback_exit       (const void *caller);

#if defined(__GNUC__)
  #define CALLER_CS()     -1
  #define CALLER_EIP(stk) __builtin_return_address (0)

#elif (DOSX)
  #define CALLER_CS()     -1
  #define CALLER_EIP(stk) *(DWORD*)(&(stk)+1))

#elif defined(__LARGE__)
  #define CALLER_CS()     *(WORD*)(&(stk)+1))
  #define CALLER_EIP(stk) *(WORD*)(&(stk)+2))

#else  /* small model */
  #define CALLER_CS()     MY_CS()
  #define CALLER_EIP(stk) *(WORD*)(&(stk)+1))
#endif

#endif
