/* amiga-specific stuff for libmikmod example programs
 *
 * This example is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRENTY; without event the implied warrenty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(__amigaos4__)
#define __USE_INLINE__
#define __USE_OLD_TIMEVAL__
#endif

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/timer.h>

#include <stdio.h>
#include <stdlib.h>

struct timerequest *timerio;
struct MsgPort     *timerport;
#if defined(__MORPHOS__) || defined(__VBCC__)
struct Library *TimerBase;
#else
struct Device  *TimerBase;
#endif

static void amiga_atexit (void) {
    if (TimerBase) {
        WaitIO((struct IORequest *) timerio);
        CloseDevice((struct IORequest *) timerio);
        DeleteIORequest((struct IORequest *) timerio);
        DeleteMsgPort(timerport);
        TimerBase = NULL;
    }
}

void amiga_sysinit (void) {
    if ((timerport = CreateMsgPort())) {
        if ((timerio = (struct timerequest *)CreateIORequest(timerport, sizeof(struct timerequest)))) {
            if (OpenDevice((STRPTR) TIMERNAME, UNIT_MICROHZ, (struct IORequest *) timerio, 0) == 0) {
#if defined(__MORPHOS__) || defined(__VBCC__)
                TimerBase = (struct Library *)timerio->tr_node.io_Device;
#else
                TimerBase = timerio->tr_node.io_Device;
#endif
            }
            else {
                DeleteIORequest((struct IORequest *)timerio);
                DeleteMsgPort(timerport);
            }
        }
        else {
            DeleteMsgPort(timerport);
        }
    }
    if (!TimerBase) {
        fprintf(stderr, "Can't open timer.device\n");
        exit (-1);
    }

    /* 1us wait, for timer cleanup success */
    timerio->tr_node.io_Command = TR_ADDREQUEST;
    timerio->tr_time.tv_secs = 0;
    timerio->tr_time.tv_micro = 1;
    SendIO((struct IORequest *) timerio);
    WaitIO((struct IORequest *) timerio);

    atexit (amiga_atexit);
}

void amiga_usleep(unsigned long timeout) {
    timerio->tr_node.io_Command = TR_ADDREQUEST;
    timerio->tr_time.tv_secs = timeout / 1000000;
    timerio->tr_time.tv_micro = timeout % 1000000;
    SendIO((struct IORequest *) timerio);
    WaitIO((struct IORequest *) timerio);
}
