/*** drv_nas.c --- MikMod sound library NAS output driver  -*- C -*- */
/** $Id$ */

/*** Copyright (C) 2004 Ivan Shmakov */

/** This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Library General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  */

/** This program is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.  */

/** You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
    02111-1307, USA.  */

/*** Code: */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mikmod_internals.h"

#ifdef DRV_NAS

/* !!! HACK: avoid BOOL typedef clash */
#define BOOL NAS_BOOL
#include <audio/audiolib.h>
#undef BOOL /* HACK !!! */

#include <errno.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#define MALLOC_VAR(x)    ((x = (typeof (x))MikMod_malloc       (sizeof (*(x)))))
#define MALLOC_ARY(x, y) ((x = (typeof (x))MikMod_malloc ((y) * sizeof (*(x)))))

/* slow network configuration by-default: */
#define NAS_DEFAULT_BUFFER 5000
#define NAS_DEFAULT_LOW_WM 75

struct my_info {
  AuServer *server;
  AuDeviceID device;
  AuFlowID sole_flow;
  AuEventHandlerRec *handler;
  /* buffer */
  struct {
    char *ptr;
    size_t size, pend, want;
  } buf;
  int stopped_p;
};

struct my_info *nas_info = NULL;

/* command line params.: */
static char *audioserver = NULL;
static long buffer_msec = NAS_DEFAULT_BUFFER;
static long buffer_low = NAS_DEFAULT_LOW_WM;

static AuDeviceID
nas_find_device (AuServer *s, int channels)
{
  int i, max = AuServerNumDevices (s);
  for (i = 0; i < max; i++) {
    if ((AuDeviceKind (AuServerDevice (s, i))
         == AuComponentKindPhysicalOutput)
        && AuDeviceNumTracks (AuServerDevice (s, i)) == channels)
      return AuDeviceIdentifier (AuServerDevice (s, i));
  }
  return AuNone;
}

/*** Data Sending and Event Handling */

static void
nas_send_data (struct my_info *i)
{
  AuServer *s = i->server;
  AuFlowID flow = i->sole_flow;
  char *ptr = i->buf.ptr;
  const size_t bytes = i->buf.want;
  const size_t pending = i->buf.pend;

  i->buf.want = 0;
  if (bytes < pending) {
    AuWriteElement (s, flow, 0, bytes, ptr, AuFalse, NULL);
    memmove (ptr, ptr + bytes, pending - bytes);
    i->buf.pend -= bytes;
  } else {
    AuWriteElement (s, flow, 0, pending, ptr,
                    bytes > pending ? AuTrue : AuFalse,
                    NULL);
    i->buf.pend = 0;
  }
}

static AuBool
nas_event_handler (AuServer *s, AuEvent *ev, AuEventHandlerRec *handler)
{
  struct my_info *i = (struct my_info *)(handler->data);
  switch (ev->type) {
  case AuEventTypeElementNotify:
    {
      AuElementNotifyEvent *eev = (AuElementNotifyEvent *)ev;
      switch (eev->kind) {
      case AuElementNotifyKindLowWater:
        /* FIXME: am I right? */
        i->buf.want = (size_t)(eev->num_bytes);
        break;
      case AuElementNotifyKindState:
        switch (eev->cur_state) {
        case AuStatePause:
          i->stopped_p = 0;
          if (eev->reason != AuReasonUser)
            i->buf.want = (size_t)(eev->num_bytes);
          break;
        case AuStateStop:
          i->stopped_p = 1;
          break;
        }
        break;
      }
    }
    break;
  }

  return AuTrue;
}

static void
nas_check_event (AuServer *s)
{
  AuEvent e;

  AuNextEvent (s, AuTrue, &e);
  AuDispatchEvent (s, &e);
}

/*** Initialization */

static int
nas_init_flow (struct my_info *i)
{
  AuServer *s = i->server;
  AuDeviceID device;
  AuFlowID flow;
  AuElement elt[2];
  unsigned char format;
  const int rate = md_mixfreq;
  const int chans = (md_mode & DMODE_STEREO) ? 2 : 1;
  const AuInt32 samples = (AuInt32)1 * rate * buffer_msec / 1000;
  const AuInt32 samples_low = samples * buffer_low / 100;
  AuEventHandlerRec *handler;

  if ((device = nas_find_device (s, chans)) == AuNone
      || (flow = AuCreateFlow (s, NULL)) == AuNone) {
    _mm_errno = MMERR_OPENING_AUDIO;
    return -1;
  }

  format = (md_mode & DMODE_16BITS)
    ? ((((char) *(short *)"x") == 'x') /* FIXME: too ugly? */
       ? AuFormatLinearSigned16LSB
       : AuFormatLinearSigned16MSB)
    : AuFormatLinearUnsigned8;

  AuMakeElementImportClient (&elt[0], /* element */
                             rate, format, chans, /* format, etc. */
                             AuTrue, /* discard */
                             samples, samples_low, /* buf size */
                             0, NULL); /* actions */
  AuMakeElementExportDevice (&elt[1], /* element */
                             0, device,
                             rate, AuUnlimitedSamples,
                             0, NULL); /* actions */
  AuSetElements (s, flow, AuTrue, 2, elt, NULL);

  /* FIXME: check for errors? */
  handler
    = AuRegisterEventHandler (s,
                              AuEventHandlerIDMask, 0, flow,
                              nas_event_handler, (AuPointer)i);

  {
    const size_t bufsz
      = samples * chans * AuSizeofFormat (format);
    if ((MALLOC_ARY (i->buf.ptr, bufsz)) == NULL)
      return -1;
    i->buf.size = bufsz;
    i->buf.pend = i->buf.want = 0;
  }

  i->device = device;
  i->sole_flow = flow;
  i->handler = handler;

  i->stopped_p = 1;

  return 0;
}

static int
nas_init_server (struct my_info *i)
{
  AuServer *s;

  if (!(s = AuOpenServer (audioserver, 0, NULL, 0, NULL, NULL))) {
    _mm_errno = MMERR_OPENING_AUDIO;
    return -1;
  }

  i->server = s;
  if (nas_init_flow (i) < 0) {
    /* _mm_errno should be set in nas_init_flow */
    AuCloseServer (s);
    return -1;
  }

  return 0;
}

static int
NAS_init (void)
{
  struct my_info *i;

  /* FIXME: assert here? */
  if (nas_info != NULL) {
    _mm_errno = MMERR_OPENING_AUDIO;
    return 1;
  }

  if (MALLOC_VAR (i) == NULL)
    return 1;

  if (nas_init_server (i) < 0
      || VC_Init () != 0) {
    MikMod_free (i);
    return 1;
  }

  nas_info = i;
  return 0;
}

/*** Deinitialization & Reset */

static void
NAS_exit (void)
{
  struct my_info *i = nas_info;
  AuServer *s;

  if (i == NULL)
    return;

  VC_Exit ();

  s = i->server;

  AuUnregisterEventHandler (s, i->handler);
  AuDestroyFlow (s, i->sole_flow, NULL);
  AuCloseServer (s);
  MikMod_free (i->buf.ptr);
  MikMod_free (i);

  nas_info = NULL;
}

static int
NAS_reset (void)
{
  NAS_exit ();
  return NAS_init ();
}

/*** Flow Control */

static int
NAS_flow_start (void)
{
  struct my_info *i = nas_info;

  i->buf.pend = i->buf.want = 0;
  AuStartFlow (i->server, i->sole_flow, NULL);
  while (i->stopped_p)
    nas_check_event (i->server);

  if (VC_PlayStart () != 0)
    return 1;

  return 0;
}

static void
NAS_flow_stop (void)
{
  struct my_info *i = nas_info;

  VC_PlayStop ();

  while (!i->stopped_p) {
    nas_check_event (i->server);
	if (i->buf.want > 0)
		nas_send_data (i);
  }
  i->buf.pend = i->buf.want = 0;
}

static void
NAS_flow_pause (void)
{
  struct my_info *i = nas_info;
  AuPauseFlow (i->server, i->sole_flow, NULL);
  /* nas_check_event (i->server); */
}

/*** Update routine */

static void
NAS_update (void)
{
  struct my_info *i = nas_info;
  const size_t bufsz = i->buf.size;

  if (i->stopped_p) {
    /* underrun? */
    AuStartFlow (i->server, i->sole_flow, NULL);
    while (i->stopped_p)
      nas_check_event (i->server);
  }

  if (i->buf.pend < bufsz) {
    size_t bytes
      = (size_t)VC_WriteBytes ((SBYTE *)(i->buf.ptr + i->buf.pend),
                               (ULONG)(i->buf.size  - i->buf.pend));
    i->buf.pend += bytes;
  }

  while ((i->buf.want == 0) && !i->stopped_p)
    nas_check_event (i->server);
  if (!i->stopped_p)
    nas_send_data (i);
}

/*** Miscellaneous API routines */

static void
NAS_cmd_line (const CHAR *s)
{
  CHAR *p;

  if ((p = MD_GetAtom ("audioserver", s, 0))) {
    MikMod_free (audioserver);
    audioserver = p;
  }

  if ((p = MD_GetAtom ("buffer", s, 0))) {
    long v = atol (p);
    MikMod_free (p);
    buffer_msec = (v < 50 || v > 10000) ? NAS_DEFAULT_BUFFER : v;
  }

  if ((p = MD_GetAtom ("buffer_low", s, 0))) {
    long v = atol (p);
    MikMod_free (p);
    buffer_low = (v < 10 || v > 90) ? NAS_DEFAULT_LOW_WM : v;
  }
}

static BOOL
NAS_there_p (void)
{
  AuServer *s;

  if (!(s = AuOpenServer (NULL, 0, NULL, 0, NULL, NULL)))
    return 0;
  AuCloseServer (s);

  return 1;
}

/*** MDRIVER record */

MIKMODAPI MDRIVER drv_nas = {
  NULL,                         /* next */
  "Network Audio System",       /* Name */
  "Network Audio System driver v0.1", /* Version */
  0, 255,                       /* NB: 0 hardware voices? */
  "nas",                        /* Alias */
  "audioserver:t::NAS server name\n"
  "buffer:r:50:10000:Audio buffer size, ms\n"
  "buffer_low:r:10:90:Audio buffer low water mark, percents\n",
  NAS_cmd_line,                 /* CommandLine */
  NAS_there_p,                  /* IsPresent */
  VC_SampleLoad,                /* SampleLoad */
  VC_SampleUnload,              /* SampleUnload */
  VC_SampleSpace,               /* FreeSampleSpace */
  VC_SampleLength,              /* RealSampleLength */
  NAS_init,                     /* Init */
  NAS_exit,                     /* Exit */
  NAS_reset,                    /* Reset */
  VC_SetNumVoices,              /* SetNumVoices */
  NAS_flow_start,               /* PlayStart */
  NAS_flow_stop,                /* PlayStop */
  NAS_update,                   /* Update */
  NAS_flow_pause,               /* Pause */
  VC_VoiceSetVolume,            /* VoiceSetVolume */
  VC_VoiceGetVolume,            /* VoiceGetVolume */
  VC_VoiceSetFrequency,         /* VoiceSetFrequency */
  VC_VoiceGetFrequency,         /* VoiceGetFrequency */
  VC_VoiceSetPanning,           /* VoiceSetPanning */
  VC_VoiceGetPanning,           /* VoiceGetPanning */
  VC_VoicePlay,                 /* VoicePlay */
  VC_VoiceStop,                 /* VoiceStop */
  VC_VoiceStopped,              /* VoiceStopped */
  VC_VoiceGetPosition,          /* VoiceGetPosition */
  VC_VoiceRealVolume            /* VoiceRealVolume */
};

#else /* DRV_NAS */
MISSING(drv_nas);
#endif /* DRV_NAS */

/*** Emacs stuff */
/** Local variables: */
/** fill-column: 72 */
/** indent-tabs-mode: nil */
/** mode: outline-minor */
/** outline-regexp: "[/]\\*\\*\\*" */
/** End: */
/*** drv_nas.c ends here */
