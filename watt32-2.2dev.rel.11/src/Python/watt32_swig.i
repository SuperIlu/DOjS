
%module watt32

#ifndef SWIG
#define SWIG
#endif

#ifndef WATT32_BUILD
#define WATT32_BUILD
#endif

/* Don't prefix so callers may do e.g.
 * "cvar.watt32.debug_on=1" from Python.
 */
#ifndef WATT32_NO_NAMESPACE
#define WATT32_NO_NAMESPACE
#endif

#define W32_NAMESPACE(func) func

%{
  #include "wattcp.h"
  #include "sock_ini.h"
  #include "pcdbug.h"
  #include "pcicmp.h"
  #include "pcdns.h"
  #include "pctcp.h"
  #include "misc.h"
//#include "misc.c" /* Because misc.c must be compiled without__declspec(thread) */
%}


#define WINWATT   64
#define DOSX      64
#define MS_CDECL

#define W32_DATA
#define W32_FUNC
#define W32_CALL
#define W32_ATTR_PRINTF(_1,_2)
#define W32_ATTR_SCANF(_1,_2)
#define W32_ATTR_NORETURN()
#define W32_ATTR_NOINLINE()
#define W32_ARG_NONNULL(_1)

%include "wattcp.h"
%include "sock_ini.h"
%include "pcdbug.h"
%include "pcicmp.h"
%include "pcdns.h"
%include "pctcp.h"
%include "misc.h"

#undef udp_Socket

#ifdef SWIG_LINK_RUNTIME
  #define SWIGINTERN
  #define SWIGRUNTIME
  #define SWIGRUNTIMEINLINE

  %include "pyrun.swg"
#endif

//extern int sock_init (void);

