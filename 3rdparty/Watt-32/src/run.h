#ifndef _w32_RUN_H
#define _w32_RUN_H

#define DAEMON_PERIOD  500       /* run daemons every 500msec */

#define startup_add    W32_NAMESPACE (startup_add)
#define startup_run    W32_NAMESPACE (startup_run)
#define startup_dump   W32_NAMESPACE (startup_dump)
#define rundown_add    W32_NAMESPACE (rundown_add)
#define rundown_run    W32_NAMESPACE (rundown_run)
#define rundown_dump   W32_NAMESPACE (rundown_dump)
#define daemon_add     W32_NAMESPACE (daemon_add)
#define daemon_del     W32_NAMESPACE (daemon_del)
#define daemon_run     W32_NAMESPACE (daemon_run)

/* This points to an internal Watt-32 function. ('__cdecl' on
 * 'Win32/Win64. Otherwise default calling-convention). Watch out for
 * $(CC) warnings when calling RUNDOWN_ADD(). The function MUST match
 * the expected calling-convention. Your $(CC) may ignore an illegal
 * cast of '(_VoidProc)func'.
 */
typedef void (W32_CALL *_VoidProc) (void);

extern int startup_add (_VoidProc func, const char *name,
                        int order, const char *file, unsigned line);

extern int rundown_add (_VoidProc func, const char *name,
                        int order, const char *file, unsigned line);

extern int daemon_add (_VoidProc func, const char *name,
                       const char *file, unsigned line);

extern int daemon_del (_VoidProc func, const char *name,
                       const char *file, unsigned line);

extern void startup_run  (void);
extern void startup_dump (void);

extern void rundown_run  (void);
extern void rundown_dump (void);

extern int  daemon_run   (void);
extern void daemon_clear (void);


#define STARTUP_ADD(func,order) \
        startup_add (func, #func, order, __FILE__, __LINE__)

#define RUNDOWN_ADD(func,order) \
        rundown_add (func, #func, order, __FILE__, __LINE__)

#define DAEMON_ADD(func) \
        daemon_add (func, #func, __FILE__, __LINE__)

#define DAEMON_DEL(func) \
        daemon_del (func, #func, __FILE__, __LINE__)

#endif
