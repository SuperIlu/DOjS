/*
 * timer.c - script timer handling stuff
 *
 */

#include "telnet.h"

#define TIMER_DEBUG

struct callout {
       DWORD           c_time;          /* time at which to call routine */
       void           *c_arg;           /* argument to routine */
       void          (*c_func)(void*);  /* routine */
       struct callout *c_next;
     };

static struct callout *callout = NULL;  /* Callout list */

#ifdef TIMER_DEBUG
static void print_callouts (char *prefix)
{
  struct callout *cop, **copp;

  if (dbug_mode)
  {
    char  buf[256] = "none";
    char *p = buf;

    for (copp = &callout; (cop = *copp) != NULL; copp = &cop->c_next)
        p += sprintf (p, "%lu, ", cop->c_time);

    cprintf ("%s: Callouts %s\r\n", prefix, buf);
  }
}
#endif

/*
 *  Schedule a timeout after `time' seconds
 */
void timeout (void (*func)(void*), void *arg, int time)
{
  struct callout *newp, *p, **pp;

  /*
   * Allocate timer callout record
   */
  newp = malloc (sizeof(*newp));
  if (!newp)
     fatal ("Out of memory in timeout()!");

  newp->c_arg  = arg;
  newp->c_func = func;
  newp->c_time = set_timeout (18*(DWORD)time);

  /*
   * Find correct place and link it in.
   */
  for (pp = &callout; (p = *pp) != NULL; pp = &p->c_next)
      if (cmp_timers(newp->c_time,p->c_time) < 0)
         break;

  newp->c_next = p;
  *pp = newp;

#ifdef TIMER_DEBUG
  cprintf ("Timeout %p(%p) in %d sec (%lu) now %lu\r\n",
           func, arg, time, newp->c_time, set_timeout(0L));
  print_callouts ("timeout()");
#endif
}

/*
 *  Unschedule a timeout. i.e. remove the event from the callout list
 */
void untimeout (void (*func)(void*), void *arg)
{
  struct callout **copp, *freep;

  /*
   * Find first matching timeout and remove it from the list.
   */
  for (copp = &callout; (freep = *copp) != NULL; copp = &freep->c_next)
      if (freep->c_func == func && freep->c_arg == arg)
      {
        *copp = freep->c_next;
        free (freep);
        break;
      }

#ifdef TIMER_DEBUG
  cprintf ("Untimeout %p(%p).\r\n", func, arg);
  print_callouts ("untimeout()");
#endif
}

/*
 *  Call any timeout routines which are now due.
 */
void calltimeout (void)
{
  while (callout)
  {
    struct callout *p = callout;

#ifdef TIMER_DEBUG
    cprintf ("calltimeout(): now %lu, 1st timeout %lu\r\n",
             set_timeout(0L), p->c_time);
    print_callouts ("calltimeout() 1");
#endif

    if (!chk_timeout(p->c_time))
       return;

    callout = p->c_next;

#ifdef TIMER_DEBUG
    print_callouts ("calltimeout() 2");
#endif

    (*p->c_func)(p->c_arg);
    free (p);
  }
}

