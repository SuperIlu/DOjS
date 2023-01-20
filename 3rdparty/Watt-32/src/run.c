
#include <stdio.h>
#include <stdlib.h>

#include "wattcp.h"
#include "pcdbug.h"
#include "misc.h"
#include "timer.h"
#include "run.h"

/**
 * \brief Register an on-startup/at-exit function.
 *
 * Insert the 'func' so functions with lowest order are run first.
 * All registered functions (added with startup_add() or rundown_add())
 * must have different orders.
 */
struct run_list {
       _VoidProc   func;
       const char *name;
       int         order;
       const char *file;
       unsigned    line;
       BOOL        running;   /* Prevent reentry. Uses in daemon_list[] only. */
     };

static struct run_list start_list  [40];
static struct run_list exit_list   [40];
static struct run_list daemon_list [20];    /* List of background processes */

static int num_start_func_inserts = 0;
static int num_exit_func_inserts  = 0;

static DWORD daemon_timer = 0UL;     /* When to run "background" processes */

static int MS_CDECL list_compare (const struct run_list *a, const struct run_list *b)
{
  return (a->order - b->order);
}

/**
 * Register a function to be called in sock_init().
 */
int startup_add (_VoidProc func, const char *name, int order,
                 const char *file, unsigned line)
{
  int i;

  for (i = 0; i < num_start_func_inserts; i++)
  {
    if (!start_list[i].func)
       continue;

    if (start_list[i].func == func)
       goto modify;

    if (start_list[i].order == order)
    {
      TRACE_CONSOLE (0, "%s(%u): startup_add (\"%s\",%d): order already "
                        "in start_list[]\n", file, line, name, order);
      return (-1);
    }
  }

  WATT_ASSERT (num_start_func_inserts < DIM(start_list));
  i = num_start_func_inserts++;

modify:
  start_list[i].func  = func;
  start_list[i].name  = name;
  start_list[i].order = order;
  start_list[i].file  = file;
  start_list[i].line  = line;

#if 1
  qsort (&start_list, DIM(start_list), sizeof(start_list[0]),
         (CmpFunc)list_compare);
#else
  if (i > 0)
     qsort (&start_list, i, sizeof(start_list[0]), (CmpFunc)list_compare);
#endif

  return (num_start_func_inserts);
}

/**
 * Register a function to be called from sock_exit().
 */
int rundown_add (_VoidProc func, const char *name, int order,
                 const char *file, unsigned line)
{
  int i;

  for (i = 0; i < num_exit_func_inserts; i++)
  {
    if (!exit_list[i].func)
       continue;

    if (exit_list[i].func == func)
       goto modify;

    if (exit_list[i].order == order)
    {
      TRACE_CONSOLE (0, "%s(%u): rundown_add (\"%s\",%d): order already "
                        "in exit_list[]\n", file, line, name, order);
      return (-1);
    }
  }

  WATT_ASSERT (num_exit_func_inserts < DIM(exit_list));
  i = num_exit_func_inserts++;

modify:
  exit_list[i].func  = func;
  exit_list[i].name  = name;
  exit_list[i].order = order;
  exit_list[i].file  = file;
  exit_list[i].line  = line;

#if 1
  qsort (&exit_list, DIM(exit_list), sizeof(exit_list[0]),
         (CmpFunc)list_compare);
#else
  if (i > 0)
     qsort (&exit_list, i, sizeof(exit_list[0]), (CmpFunc)list_compare);
#endif

  return (num_exit_func_inserts);
}

/**
 * Run the registered and sorted exit_list[] functions.
 */
void rundown_run (void)
{
  struct run_list *oe;
  _VoidProc func;
  int       i;

  for (i = 0, oe = exit_list; i < DIM(exit_list); i++, oe++)
  {
    if (!oe->func)
       continue;

    TRACE_CONSOLE (3, "Calling rundown-func `%s' at order %d\n",
                   oe->name, oe->order);
     func = oe->func;
     oe->func = NULL;    /* don't call it again */
     (*func)();
  }
}

#if defined(USE_DEBUG)
/**
 * Dump 'exit_list[]' list.
 */
void rundown_dump (void)
{
  const struct run_list *oe;
  int   i, num_active;

  (*_printf) ("rundown_dump():\n");
  for (i = num_active = 0, oe = exit_list; i < DIM(exit_list); i++, oe++)
  {
    if (!oe->func)
       continue;
    TRACE_CONSOLE (0, "  order %3d: `%s'  called from %s (%u)\n",
                   oe->order, oe->name, oe->file, oe->line);
    num_active++;
  }
  TRACE_CONSOLE (0, "  %s\n", num_active == i ? "possible overflow" : "okay");
}
#endif

#ifdef NOT_USED
/**
 * Called from sock_exit() to clear up things done here.
 */
void exit_misc (void)
{
  num_exit_func_inserts = 0;
}
#endif


/**
 * \todo Rename them daemon_add(), daemon_run(). Keep old
 *       functions for compatability.
 */

/**
 * Add a function to background daemon list.
 */
int daemon_add (_VoidProc func, const char *name,
                const char *file, unsigned line)
{
  struct run_list *r;
  int    i;

  /* see if the daemon is already in the list */
  for (i = 0, r = daemon_list; i < DIM(daemon_list); i++, r++)
      if (r->func && r->func == func)
      {
        TRACE_CONSOLE (3, "\ndaemon_add(): daemon exists\n");
        return (1);     /* operation can proceed normally */
      }

  for (i = 0, r = daemon_list; i < DIM(daemon_list); i++, r++)
      if (!r->func)
      {
        r->func    = func;
        r->name    = name;
        r->running = FALSE;
        return (1);
      }
  ARGSUSED (file);
  ARGSUSED (line);
  return (0);       /* didn't find a free slot */
}

/**
 * Remove a function from background daemon list.
 * 'func == NULL', means to stop all daemons.
 */
int daemon_del (_VoidProc func, const char *name,
                const char *file, unsigned line)
{
  struct run_list *r;
  int    i;

  for (i = 0, r = daemon_list; i < DIM(daemon_list); i++, r++)
      if (!func || func == r->func)
      {
        r->func    = NULL;
        r->running = FALSE;
        return (1);
      }
  ARGSUSED (name);
  ARGSUSED (file);
  ARGSUSED (line);
  return (0);    /* didn't find the daemon */
}

/* UPDATE: 26FEB2006 psuggs@pobox.com
 * Shutdown daemons and clear the list
 */
void daemon_clear (void)
{
  daemon_del (NULL, NULL, NULL, 0);
}

/**
 * Run Waterloo "background" daemon processes.
 * Checked from tcp_tick().
 */
int daemon_run (void)
{
  int rc = 0;

  if (daemon_timer == 0UL || chk_timeout(daemon_timer))
  {
    struct run_list *r;
    int    i;

    for (i = 0, r = daemon_list; i < DIM(daemon_list); i++, r++)
        if (r->func && !r->running)
        {
          if (r->name)
               TRACE_CONSOLE (3, "Running daemon %d (%s)\n", i, r->name);
          else TRACE_CONSOLE (3, "Running daemon %d (%p)\n", i, r->func);
          r->running = TRUE;
          (*r->func)();
          r->running = FALSE;
          rc++;
        }
  }
  daemon_timer = set_timeout (DAEMON_PERIOD);
  return (rc);
}

/*
 * Old names for above. Keep these public.
 */
int W32_CALL addwattcpd (VoidProc func)
{
  return daemon_add ((_VoidProc)func, NULL, NULL, 0);
}

int W32_CALL delwattcpd (VoidProc func)
{
  return daemon_del ((_VoidProc)func, NULL, NULL, 0);
}

