/*!\file profile.c
 *
 * Stuff for precision timing of functions.
 */

/*  Copyright (c) 1997-2007 Gisle Vanem <gvanem@yahoo.no>
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. All advertising materials mentioning features or use of this software
 *     must display the following acknowledgement:
 *       This product includes software developed by Gisle Vanem
 *       Bergen, Norway.
 *
 *  THIS SOFTWARE IS PROVIDED BY ME (Gisle Vanem) AND CONTRIBUTORS ``AS IS''
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL I OR CONTRIBUTORS BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "wattcp.h"
#include "cpumodel.h"
#include "timer.h"
#include "misc.h"
#include "run.h"
#include "strings.h"
#include "language.h"
#include "pcconfig.h"
#include "profile.h"

#if defined(USE_PROFILER)

#if defined(WIN32)
  #undef  GET_RDTSC
  #define GET_RDTSC() win_get_perf_count()
#endif

/*
 * A simple execution profiler
 */
BOOL profile_enable = FALSE;
char profile_file [MAX_PATHLEN+1] = ".\\WATTCP.PRO";

static FILE  *prof_fout;
static uint64 prof_start;
static uint64 num_profs;

/*
 * Called on entry of something to profile.
 * \ref macro 'PROFILE_START(func)'.
 */
void profile_start (const char *str)
{
  if (prof_fout)
  {
    if (str)
       fprintf (prof_fout, "  profiling %-12.12s ", str);
    prof_start = GET_RDTSC();
    num_profs++;
  }
}

/*
 * Dumps the time taken (milli-sec and CPU clocks) since
 * 'PROFILE_STOP(func)' was used.
 * \ref macro 'PROFILE_STOP()'.
 */
void profile_stop (void)
{
  if (prof_fout && clocks_per_usec)
  {
    int64  clocks = (int64) (GET_RDTSC() - prof_start);
    double msec   = (double)clocks / ((double)clocks_per_usec * 1000.0);
#if 0
    fprintf (prof_fout, "%10.3f msec (%10" S64_FMT " clk)\n", msec, clocks);
#else
    fprintf (prof_fout, "%10.3f msec (%12s clk)\n", msec, qword_str(clocks));
#endif
  }
}

static void W32_CALL profile_exit (void)
{
  if (prof_fout)
  {
    FCLOSE (prof_fout);
    prof_fout = NULL;
    if (num_profs)
       (*_printf) ("profiling info written to \"%s\"\n", profile_file);
  }
}

/*
 * Dump some arbitrary data-array to profile dump file.
 */
void profile_dump (const uint64 *data, size_t num)
{
  unsigned i;

  if (!prof_fout || !clocks_per_usec)
     return;

  for (i = 0; i < num; i++)
  {
    double msec = (double)data[i] / ((double)clocks_per_usec * 1000.0);

#if 0
    fprintf (prof_fout, "profile_dump: %3u: %12.3f msec (%10" U64_FMT " clk)\n",
             i, msec, data[i]);
#else
    fprintf (prof_fout, "profile_dump: %3u: %12.3f msec (%12s clk)\n",
             i, msec, qword_str(data[i]));
#endif
  }
}

/*
 * Profile speed of receiving a packet. Write the difference of
 *   put - DOS:   timestamp for 2nd packet upcall.
 *         WIN32: timestamp when recv-thread captured the packet.
 *   get - timestamp when we called 'pkt_poll_recv()'.
 *
 * DOS:   the 'put' and 'get' are CPU clock timestamps (RDTSC instruction).
 * WIN32: these are values from 'QueryPerformanceCounter()'.
 */
void profile_recv (uint64 put, uint64 get)
{
  if (!prof_fout || !clocks_per_usec)
     return;

  fputs ("  profiling pkt_receiver ", prof_fout);
  if (get == U64_SUFFIX(0) || put == U64_SUFFIX(0))
     fputs ("<not enabled>\n", prof_fout);
  else
  {
    int64  clocks = (int64) (get - put);
    double msec   = (double)clocks / ((double)clocks_per_usec * 1000.0);
#if 0
    fprintf (prof_fout, "%10.3f msec (%10" S64_FMT " clk)\n", msec, clocks);
#else
    fprintf (prof_fout, "%10.3f msec (%12s clk)\n", msec, qword_str(clocks));
#endif
  }
}

/*
 * Must be called after init_timers() or init_misc().
 */
int profile_init (void)
{
  uint64 overhead, start;
  time_t now;
  char   ct_buf [30];

  if (!profile_enable || !profile_file[0])
     return (0);

  if (!has_rdtsc || !clocks_per_usec)
  {
    profile_enable = FALSE;
    (*_printf) (_LANG("Profiling not available (no RDTSC or $USE_RDTSC=1)\n"));
    return (0);
  }

  if (!FOPEN_APP(prof_fout,profile_file))
  {
    (*_printf) (_LANG("Failed to open %s: %s\n"),
                profile_file, strerror(errno));
    return (0);
  }

#if !defined(__BORLANDC__)  /* buggy output with this */
  {
    static char fbuf [256];
    setvbuf (prof_fout, fbuf, _IOLBF, sizeof(fbuf));  /* line-buffered */
  }
#endif

  now = time (NULL);
  fprintf (prof_fout, "\nProfiling %s, %.15s, %s\n",
           get_argv0(), ctime_r(&now, ct_buf) + 4, dos_extender_name());

  num_profs = 0;
  start = GET_RDTSC();
  profile_start ("dummy");
  profile_stop();
  overhead = GET_RDTSC() - start;

#if 0
  fprintf (prof_fout, "  CPU-speed %lu MHz, overhead %" U64_FMT " clocks\n",
           (u_long)clocks_per_usec, overhead);
#else
  fprintf (prof_fout, "  CPU-speed %lu MHz, overhead %s clocks\n",
           (u_long)clocks_per_usec, qword_str(overhead));
#endif

  RUNDOWN_ADD (profile_exit, 90);
  return (1);
}

int profile_on (void)
{
  return (prof_fout != NULL);
}
#endif  /* USE_PROFILER */

