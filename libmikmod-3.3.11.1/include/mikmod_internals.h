/*	MikMod sound library
	(c) 1998, 1999, 2005 Miodrag Vallat and others - see file AUTHORS for
	complete list.

	This library is free software; you can redistribute it and/or modify
	it under the terms of the GNU Library General Public License as
	published by the Free Software Foundation; either version 2 of
	the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
	02111-1307, USA.
*/

/*==============================================================================

  MikMod sound library internal definitions

  ==============================================================================*/

#ifndef _MIKMOD_INTERNALS_H
#define _MIKMOD_INTERNALS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

#if defined(_MSC_VER) && !defined(__cplusplus) && !defined(HAVE_CONFIG_H)
#define inline __inline
#endif

#include "mikmod.h"

#ifndef MIKMOD_UNIX
#if (defined(unix) || defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))) && \
   !(defined(_MIKMOD_WIN32) || defined(_MIKMOD_OS2) || defined(_MIKMOD_DOS) || defined(_MIKMOD_AMIGA) || defined(macintosh))
#define MIKMOD_UNIX 1
#else
#define MIKMOD_UNIX 0
#endif
#endif /* MIKMOD_UNIX */

/*========== More type definitions */

/* SLONGLONG: 64bit, signed */
#if !defined(_WIN32) && \
   (defined(_LP64) || defined(__LP64__) || defined(__arch64__) || defined(__alpha) || defined(__x64_64) || defined(__powerpc64__))
typedef long            SLONGLONG;
#define NATIVE_64BIT_INT
#elif defined(_WIN64) /* win64 is LLP64, not LP64  */
#define NATIVE_64BIT_INT
typedef long long       SLONGLONG;
#elif defined(__WATCOMC__)
typedef __int64         SLONGLONG;
#elif defined(_WIN32) && !defined(__MWERKS__)
typedef LONGLONG        SLONGLONG;
#elif defined(macintosh) && !TYPE_LONGLONG
#include <Types.h>
typedef SInt64          SLONGLONG;
#else
typedef long long       SLONGLONG;
#endif
typedef int __s64_typetest [(sizeof(SLONGLONG)==8) * 2 - 1];

/* pointer-sized signed int (ssize_t/intptr_t) : */
#if defined(_WIN64) /* win64 is LLP64, not LP64  */
typedef long long       SINTPTR_T;
#else
/* long should be pointer-sized for all others : */
typedef long            SINTPTR_T;
#endif
typedef int __iptr_typetest [(sizeof(SINTPTR_T)==sizeof(void*)) * 2 - 1];

/*========== Error handling */

#define _mm_errno MikMod_errno
#define _mm_critical MikMod_critical
extern MikMod_handler_t _mm_errorhandler;

/*========== MT stuff */

#ifdef HAVE_PTHREAD
#include <pthread.h>
#define DECLARE_MUTEX(name) \
        extern pthread_mutex_t _mm_mutex_##name
#define MUTEX_LOCK(name)    \
        pthread_mutex_lock(&_mm_mutex_##name)
#define MUTEX_UNLOCK(name)  \
        pthread_mutex_unlock(&_mm_mutex_##name)

#elif defined(__OS2__)||defined(__EMX__)
#define DECLARE_MUTEX(name) \
        extern HMTX _mm_mutex_##name
#define MUTEX_LOCK(name)    \
        if(_mm_mutex_##name)\
            DosRequestMutexSem(_mm_mutex_##name,SEM_INDEFINITE_WAIT)
#define MUTEX_UNLOCK(name)  \
        if(_mm_mutex_##name)\
            DosReleaseMutexSem(_mm_mutex_##name)

#elif defined(_WIN32)
#include <windows.h>
#define DECLARE_MUTEX(name) \
        extern HANDLE _mm_mutex_##name
#define MUTEX_LOCK(name)    \
        if(_mm_mutex_##name)\
            WaitForSingleObject(_mm_mutex_##name,INFINITE)
#define MUTEX_UNLOCK(name)  \
        if(_mm_mutex_##name)\
            ReleaseMutex(_mm_mutex_##name)

#else
#define DECLARE_MUTEX(name) \
        extern void *_mm_mutex_##name
#define MUTEX_LOCK(name)
#define MUTEX_UNLOCK(name)
#endif

DECLARE_MUTEX(lists);
DECLARE_MUTEX(vars);

/*========== Replacement funcs */

extern int _mm_strcasecmp (const char *__s1, const char *__s2);

/*========== Portable file I/O */

extern MREADER* _mm_new_mem_reader(const void *buffer, long len);
extern void _mm_delete_mem_reader(MREADER *reader);

extern MREADER* _mm_new_file_reader(FILE* fp);
extern void _mm_delete_file_reader(MREADER*);

extern MWRITER* _mm_new_file_writer(FILE *fp);
extern void _mm_delete_file_writer(MWRITER*);

extern BOOL _mm_FileExists(const CHAR *fname);

#define _mm_write_SBYTE(x,y)    y->Put(y,(int)x)
#define _mm_write_UBYTE(x,y)    y->Put(y,(int)x)

#define _mm_read_SBYTE(x)       (SBYTE)x->Get(x)
#define _mm_read_UBYTE(x)       (UBYTE)x->Get(x)
#define _mm_skip_BYTE(x)        (void)x->Get(x)

#define _mm_write_SBYTES(x,y,z) z->Write(z,(const void *)x,y)
#define _mm_write_UBYTES(x,y,z) z->Write(z,(const void *)x,y)
#define _mm_read_SBYTES(x,y,z)  z->Read(z,(void *)x,y)
#define _mm_read_UBYTES(x,y,z)  z->Read(z,(void *)x,y)

#define _mm_fseek(x,y,z)        x->Seek(x,y,z)
#define _mm_ftell(x)            x->Tell(x)
#define _mm_rewind(x)           _mm_fseek(x,0,SEEK_SET)

#define _mm_eof(x)              x->Eof(x)

extern void _mm_iobase_setcur(MREADER*);
extern void _mm_iobase_revert(MREADER*);
extern FILE* _mm_fopen(const CHAR *, const CHAR *);
extern int  _mm_fclose(FILE *);
extern void _mm_write_string(const CHAR*,MWRITER*);
extern BOOL _mm_read_string (CHAR*,int,MREADER*);

extern SWORD _mm_read_M_SWORD(MREADER*);
extern SWORD _mm_read_I_SWORD(MREADER*);
extern UWORD _mm_read_M_UWORD(MREADER*);
extern UWORD _mm_read_I_UWORD(MREADER*);

extern SLONG _mm_read_M_SLONG(MREADER*);
extern SLONG _mm_read_I_SLONG(MREADER*);
extern ULONG _mm_read_M_ULONG(MREADER*);
extern ULONG _mm_read_I_ULONG(MREADER*);

extern BOOL _mm_read_M_SWORDS(SWORD*,int,MREADER*);
extern BOOL _mm_read_I_SWORDS(SWORD*,int,MREADER*);
extern BOOL _mm_read_M_UWORDS(UWORD*,int,MREADER*);
extern BOOL _mm_read_I_UWORDS(UWORD*,int,MREADER*);

extern BOOL _mm_read_M_SLONGS(SLONG*,int,MREADER*);
extern BOOL _mm_read_I_SLONGS(SLONG*,int,MREADER*);
extern BOOL _mm_read_M_ULONGS(ULONG*,int,MREADER*);
extern BOOL _mm_read_I_ULONGS(ULONG*,int,MREADER*);

extern void _mm_write_M_SWORD(SWORD,MWRITER*);
extern void _mm_write_I_SWORD(SWORD,MWRITER*);
extern void _mm_write_M_UWORD(UWORD,MWRITER*);
extern void _mm_write_I_UWORD(UWORD,MWRITER*);

extern void _mm_write_M_SLONG(SLONG,MWRITER*);
extern void _mm_write_I_SLONG(SLONG,MWRITER*);
extern void _mm_write_M_ULONG(ULONG,MWRITER*);
extern void _mm_write_I_ULONG(ULONG,MWRITER*);

extern void _mm_write_M_SWORDS(SWORD*,int,MWRITER*);
extern void _mm_write_I_SWORDS(SWORD*,int,MWRITER*);
extern void _mm_write_M_UWORDS(UWORD*,int,MWRITER*);
extern void _mm_write_I_UWORDS(UWORD*,int,MWRITER*);

extern void _mm_write_M_SLONGS(SLONG*,int,MWRITER*);
extern void _mm_write_I_SLONGS(SLONG*,int,MWRITER*);
extern void _mm_write_M_ULONGS(ULONG*,int,MWRITER*);
extern void _mm_write_I_ULONGS(ULONG*,int,MWRITER*);

/*========== Samples */

#define MAX_SAMPLE_SIZE 0x10000000 /* a sane value guaranteed to not overflow an SLONG */

/* This is a handle of sorts attached to any sample registered with
   SL_RegisterSample.  Generally, this only need be used or changed by the
   loaders and drivers of mikmod. */
typedef struct SAMPLOAD {
    struct SAMPLOAD *next;

    ULONG    length;       /* length of sample (in samples!) */
    ULONG    loopstart;    /* repeat position (relative to start, in samples) */
    ULONG    loopend;      /* repeat end */
    UWORD    infmt,outfmt;
    int      scalefactor;
    SAMPLE*  sample;
    MREADER* reader;
} SAMPLOAD;

/*========== Sample and waves loading interface */

extern void      SL_HalveSample(SAMPLOAD*,int);
extern void      SL_Sample8to16(SAMPLOAD*);
extern void      SL_Sample16to8(SAMPLOAD*);
extern void      SL_SampleSigned(SAMPLOAD*);
extern void      SL_SampleUnsigned(SAMPLOAD*);
extern int       SL_LoadSamples(void);
extern SAMPLOAD* SL_RegisterSample(SAMPLE*,int,MREADER*);
extern int       SL_Load(void*,SAMPLOAD*,ULONG);
extern BOOL      SL_Init(SAMPLOAD*);
extern void      SL_Exit(SAMPLOAD*);

/*========== Internal module representation (UniMod) interface */

/* number of notes in an octave */
#define OCTAVE 12

extern void   UniSetRow(UBYTE*);
extern UBYTE  UniGetByte(void);
extern UWORD  UniGetWord(void);
extern UBYTE* UniFindRow(UBYTE*,UWORD);
extern void   UniSkipOpcode(void);
extern void   UniReset(void);
extern void   UniWriteByte(UBYTE);
extern void   UniWriteWord(UWORD);
extern void   UniNewline(void);
extern UBYTE* UniDup(void);
extern BOOL   UniInit(void);
extern void   UniCleanup(void);
extern void   UniEffect(UWORD,UWORD);
#define UniInstrument(x)   UniEffect(UNI_INSTRUMENT,x)
#define UniNote(x)         UniEffect(UNI_NOTE,x)
extern void   UniPTEffect(UBYTE,UBYTE);
extern void   UniVolEffect(UWORD,UBYTE);

/*========== Module Commands */

enum {
 /* Simple note */
    UNI_NOTE = 1,
 /* Instrument change */
    UNI_INSTRUMENT,
 /* Protracker effects */
    UNI_PTEFFECT0,  /* arpeggio */
    UNI_PTEFFECT1,  /* porta up */
    UNI_PTEFFECT2,  /* porta down */
    UNI_PTEFFECT3,  /* porta to note */
    UNI_PTEFFECT4,  /* vibrato */
    UNI_PTEFFECT5,  /* dual effect 3+A */
    UNI_PTEFFECT6,  /* dual effect 4+A */
    UNI_PTEFFECT7,  /* tremolo */
    UNI_PTEFFECT8,  /* pan */
    UNI_PTEFFECT9,  /* sample offset */
    UNI_PTEFFECTA,  /* volume slide */
    UNI_PTEFFECTB,  /* pattern jump */
    UNI_PTEFFECTC,  /* set volume */
    UNI_PTEFFECTD,  /* pattern break */
    UNI_PTEFFECTE,  /* extended effects */
    UNI_PTEFFECTF,  /* set speed */
 /* Scream Tracker effects */
    UNI_S3MEFFECTA, /* set speed */
    UNI_S3MEFFECTD, /* volume slide */
    UNI_S3MEFFECTE, /* porta down */
    UNI_S3MEFFECTF, /* porta up */
    UNI_S3MEFFECTI, /* tremor */
    UNI_S3MEFFECTQ, /* retrig */
    UNI_S3MEFFECTR, /* tremolo */
    UNI_S3MEFFECTT, /* set tempo */
    UNI_S3MEFFECTU, /* fine vibrato */
    UNI_KEYOFF,     /* note off */
 /* Fast Tracker effects */
    UNI_KEYFADE,    /* note fade */
    UNI_VOLEFFECTS, /* volume column effects */
    UNI_XMEFFECT4,  /* vibrato */
    UNI_XMEFFECT6,  /* dual effect 4+A */
    UNI_XMEFFECTA,  /* volume slide */
    UNI_XMEFFECTE1, /* fine porta up */
    UNI_XMEFFECTE2, /* fine porta down */
    UNI_XMEFFECTEA, /* fine volume slide up */
    UNI_XMEFFECTEB, /* fine volume slide down */
    UNI_XMEFFECTG,  /* set global volume */
    UNI_XMEFFECTH,  /* global volume slide */
    UNI_XMEFFECTL,  /* set envelope position */
    UNI_XMEFFECTP,  /* pan slide */
    UNI_XMEFFECTX1, /* extra fine porta up */
    UNI_XMEFFECTX2, /* extra fine porta down */
 /* Impulse Tracker effects */
    UNI_ITEFFECTG,  /* porta to note */
    UNI_ITEFFECTH,  /* vibrato */
    UNI_ITEFFECTI,  /* tremor (xy not incremented) */
    UNI_ITEFFECTM,  /* set channel volume */
    UNI_ITEFFECTN,  /* slide / fineslide channel volume */
    UNI_ITEFFECTP,  /* slide / fineslide channel panning */
    UNI_ITEFFECTT,  /* slide tempo */
    UNI_ITEFFECTU,  /* fine vibrato */
    UNI_ITEFFECTW,  /* slide / fineslide global volume */
    UNI_ITEFFECTY,  /* panbrello */
    UNI_ITEFFECTZ,  /* resonant filters */
    UNI_ITEFFECTS0,
 /* UltraTracker effects */
    UNI_ULTEFFECT9, /* Sample fine offset */
 /* OctaMED effects */
    UNI_MEDSPEED,
    UNI_MEDEFFECTF1,/* play note twice */
    UNI_MEDEFFECTF2,/* delay note */
    UNI_MEDEFFECTF3,/* play note three times */
 /* Oktalyzer effects */
    UNI_OKTARP,     /* arpeggio */

    UNI_LAST
};

extern const UWORD unioperands[UNI_LAST];

/* IT / S3M Extended SS effects: */
enum {
    SS_GLISSANDO = 1,
    SS_FINETUNE,
    SS_VIBWAVE,
    SS_TREMWAVE,
    SS_PANWAVE,
    SS_FRAMEDELAY,
    SS_S7EFFECTS,
    SS_PANNING,
    SS_SURROUND,
    SS_HIOFFSET,
    SS_PATLOOP,
    SS_NOTECUT,
    SS_NOTEDELAY,
    SS_PATDELAY
};

/* IT Volume column effects */
enum {
    VOL_VOLUME = 1,
    VOL_PANNING,
    VOL_VOLSLIDE,
    VOL_PITCHSLIDEDN,
    VOL_PITCHSLIDEUP,
    VOL_PORTAMENTO,
    VOL_VIBRATO
};

/* IT resonant filter information */

#define UF_MAXMACRO     0x10
#define UF_MAXFILTER    0x100

#define FILT_CUT        0x80
#define FILT_RESONANT   0x81

typedef struct FILTER {
    UBYTE filter,inf;
} FILTER;

/*========== Instruments */

/* Instrument format flags */
#define IF_OWNPAN       1
#define IF_PITCHPAN     2

/* Envelope flags: */
#define EF_ON           1
#define EF_SUSTAIN      2
#define EF_LOOP         4
#define EF_VOLENV       8

/* New Note Action Flags */
#define NNA_CUT         0
#define NNA_CONTINUE    1
#define NNA_OFF         2
#define NNA_FADE        3

#define NNA_MASK        3

#define DCT_OFF         0
#define DCT_NOTE        1
#define DCT_SAMPLE      2
#define DCT_INST        3

#define DCA_CUT         0
#define DCA_OFF         1
#define DCA_FADE        2

#define KEY_KICK        0
#define KEY_OFF         1
#define KEY_FADE        2
#define KEY_KILL        (KEY_OFF|KEY_FADE)

#define KICK_ABSENT     0
#define KICK_NOTE       1
#define KICK_KEYOFF     2
#define KICK_ENV        4

#define AV_IT           1   /* IT vs. XM vibrato info */

/*========== Playing */

#define POS_NONE        (-2)    /* no loop position defined */

#define LAST_PATTERN    (UWORD)(-1) /* special ``end of song'' pattern */

typedef struct ENVPR {
    UBYTE  flg;     /* envelope flag */
    UBYTE  pts;     /* number of envelope points */
    UBYTE  susbeg;  /* envelope sustain index begin */
    UBYTE  susend;  /* envelope sustain index end */
    UBYTE  beg;     /* envelope loop begin */
    UBYTE  end;     /* envelope loop end */
    SWORD  p;       /* current envelope counter */
    UWORD  a;       /* envelope index a */
    UWORD  b;       /* envelope index b */
    ENVPT* env;     /* envelope points */
} ENVPR;

typedef struct MP_CHANNEL {
    INSTRUMENT* i;
    SAMPLE *s;
    UBYTE  sample;      /* which sample number */
    UBYTE  note;        /* the audible note as heard, direct rep of period */
    SWORD  outvolume;   /* output volume (vol + sampcol + instvol) */
    SBYTE  chanvol;     /* channel's "global" volume */
    UWORD  fadevol;     /* fading volume rate */
    SWORD  panning;     /* panning position */
    UBYTE  kick;        /* if true = sample has to be restarted */
    UBYTE  kick_flag;   /* kick has been true */
    UWORD  period;      /* period to play the sample at */
    UBYTE  nna;         /* New note action type + master/slave flags */

    UBYTE  volflg;      /* volume envelope settings */
    UBYTE  panflg;      /* panning envelope settings */
    UBYTE  pitflg;      /* pitch envelope settings */

    UBYTE  keyoff;      /* if true = fade out and stuff */
    SWORD  handle;      /* which sample-handle */
    UBYTE  notedelay;   /* (used for note delay) */
    SLONG  start;       /* The starting byte index in the sample */
} MP_CHANNEL;

typedef struct MP_CONTROL {
    struct MP_CHANNEL   main;

    struct MP_VOICE*    slave;  /* Audio Slave of current effects control channel */

    UBYTE   slavechn;   /* Audio Slave of current effects control channel */
    UBYTE   muted;      /* if set, channel not played */
    UWORD   ultoffset;  /* fine sample offset memory */
    UBYTE   anote;      /* the note that indexes the audible */
    UBYTE   oldnote;
    SWORD   ownper;
    SWORD   ownvol;
    UBYTE   dca;        /* duplicate check action */
    UBYTE   dct;        /* duplicate check type */
    UBYTE*  row;        /* row currently playing on this channel */
    SBYTE   retrig;     /* retrig value (0 means don't retrig) */
    ULONG   speed;      /* what finetune to use */
    SWORD   volume;     /* amiga volume (0 t/m 64) to play the sample at */

    SWORD   tmpvolume;  /* tmp volume */
    UWORD   tmpperiod;  /* tmp period */
    UWORD   wantedperiod;/* period to slide to (with effect 3 or 5) */

    UBYTE   arpmem;     /* arpeggio command memory */
    UBYTE   pansspd;    /* panslide speed */
    UWORD   slidespeed;
    UWORD   portspeed;  /* noteslide speed (toneportamento) */

    UBYTE   s3mtremor;  /* s3m tremor (effect I) counter */
    UBYTE   s3mtronof;  /* s3m tremor ontime/offtime */
    UBYTE   s3mvolslide;/* last used volslide */
    SBYTE   sliding;
    UBYTE   s3mrtgspeed;/* last used retrig speed */
    UBYTE   s3mrtgslide;/* last used retrig slide */

    UBYTE   glissando;  /* glissando (0 means off) */
    UBYTE   wavecontrol;

    SBYTE   vibpos;     /* current vibrato position */
    UBYTE   vibspd;     /* "" speed */
    UBYTE   vibdepth;   /* "" depth */

    SBYTE   trmpos;     /* current tremolo position */
    UBYTE   trmspd;     /* "" speed */
    UBYTE   trmdepth;   /* "" depth */

    UBYTE   fslideupspd;
    UBYTE   fslidednspd;
    UBYTE   fportupspd; /* fx E1 (extra fine portamento up) data */
    UBYTE   fportdnspd; /* fx E2 (extra fine portamento dn) data */
    UBYTE   ffportupspd;/* fx X1 (extra fine portamento up) data */
    UBYTE   ffportdnspd;/* fx X2 (extra fine portamento dn) data */

    ULONG   hioffset;   /* last used high order of sample offset */
    UWORD   soffset;    /* last used low order of sample-offset (effect 9) */

    UBYTE   sseffect;   /* last used Sxx effect */
    UBYTE   ssdata;     /* last used Sxx data info */
    UBYTE   chanvolslide;/* last used channel volume slide */

    UBYTE   panbwave;   /* current panbrello waveform */
    UBYTE   panbpos;    /* current panbrello position */
    SBYTE   panbspd;    /* "" speed */
    UBYTE   panbdepth;  /* "" depth */

    UWORD   newsamp;    /* set to 1 upon a sample / inst change */
    UBYTE   voleffect;  /* Volume Column Effect Memory as used by IT */
    UBYTE   voldata;    /* Volume Column Data Memory */

    SWORD   pat_reppos; /* patternloop position */
    UWORD   pat_repcnt; /* times to loop */
} MP_CONTROL;

/* Used by NNA only player (audio control.  AUDTMP is used for full effects
   control). */
typedef struct MP_VOICE {
    struct MP_CHANNEL   main;

    ENVPR   venv;
    ENVPR   penv;
    ENVPR   cenv;

    UWORD   avibpos;    /* autovibrato pos */
    UWORD   aswppos;    /* autovibrato sweep pos */

    ULONG   totalvol;   /* total volume of channel (before global mixings) */

    BOOL    mflag;
    SWORD   masterchn;
    UWORD   masterperiod;

    MP_CONTROL* master; /* index of "master" effects channel */
} MP_VOICE;

/*========== Loaders */

typedef struct MLOADER {
    struct MLOADER*     next;
    const CHAR* type;
    const CHAR* version;
    BOOL  (*Init)(void);
    BOOL  (*Test)(void);
    BOOL  (*Load)(BOOL);
    void  (*Cleanup)(void);
    CHAR* (*LoadTitle)(void);
} MLOADER;

/* internal loader variables */
extern MREADER* modreader;
extern MODULE  of;                  /* static unimod loading space */
extern const UWORD finetune[16];
extern const UWORD npertab[7*OCTAVE];/* used by the original MOD loaders */

extern SBYTE   remap[UF_MAXCHAN];   /* for removing empty channels */
extern UBYTE*  poslookup;           /* lookup table for pattern jumps after
                                       blank pattern removal */
extern UWORD   poslookupcnt;
extern UWORD*  origpositions;

extern BOOL    filters;             /* resonant filters in use */
extern UBYTE   activemacro;         /* active midi macro number for Sxx */
extern UBYTE   filtermacros[UF_MAXMACRO];    /* midi macro settings */
extern FILTER  filtersettings[UF_MAXFILTER]; /* computed filter settings */

extern int*    noteindex;

/*========== Internal loader interface */

extern BOOL   ReadComment(UWORD);
extern BOOL   ReadLinedComment(UWORD,UWORD);
extern BOOL   AllocPositions(int);
extern BOOL   AllocPatterns(void);
extern BOOL   AllocTracks(void);
extern BOOL   AllocInstruments(void);
extern BOOL   AllocSamples(void);
extern CHAR*  DupStr(const CHAR*, UWORD, BOOL);

/* loader utility functions */
extern int*   AllocLinear(void);
extern void   FreeLinear(void);
extern int    speed_to_finetune(ULONG,int);
extern void   S3MIT_ProcessCmd(UBYTE,UBYTE,unsigned int);
extern void   S3MIT_CreateOrders(BOOL);

/* flags for S3MIT_ProcessCmd */
#define S3MIT_OLDSTYLE  1   /* behave as old scream tracker */
#define S3MIT_IT        2   /* behave as impulse tracker */
#define S3MIT_SCREAM    4   /* enforce scream tracker specific limits */

/* used to convert c4spd to linear XM periods (IT and IMF loaders). */
extern UWORD  getlinearperiod(UWORD,ULONG);
extern ULONG  getfrequency(UWORD,ULONG);

/* loader shared data */
#define STM_NTRACKERS   3
extern const CHAR *STM_Signatures[STM_NTRACKERS];

/*========== Player interface */

extern int    Player_Init(MODULE*);
extern void   Player_Exit(MODULE*);
extern void   Player_HandleTick(void);

/*========== UnPackers */

typedef BOOL (*MUNPACKER) (struct MREADER*,
                           void** /* unpacked data out */ ,
                           long* /* unpacked data size */ );
extern BOOL PP20_Unpack(MREADER*, void**, long*);
extern BOOL MMCMP_Unpack(MREADER*, void**, long*);
extern BOOL XPK_Unpack(MREADER*, void**, long*);
extern BOOL S404_Unpack(MREADER*, void**, long*);

/*========== Drivers */

/* max. number of handles a driver has to provide. (not strict) */
#define MAXSAMPLEHANDLES    384

/* These variables can be changed at ANY time and results will be immediate */
extern UWORD md_bpm;         /* current song / hardware BPM rate */

/* Variables below can be changed via MD_SetNumVoices at any time. However, a
   call to MD_SetNumVoicess while the driver is active will cause the sound to
   skip slightly. */
extern UBYTE md_numchn;      /* number of song + sound effects voices */
extern UBYTE md_sngchn;      /* number of song voices */
extern UBYTE md_sfxchn;      /* number of sound effects voices */
extern UBYTE md_hardchn;     /* number of hardware mixed voices */
extern UBYTE md_softchn;     /* number of software mixed voices */

/* This is for use by the hardware drivers only.  It points to the registered
   tickhandler function. */
extern void (*md_player)(void);

extern SWORD  MD_SampleLoad(SAMPLOAD*,int);
extern void   MD_SampleUnload(SWORD);
extern ULONG  MD_SampleSpace(int);
extern ULONG  MD_SampleLength(int,SAMPLE*);

/* uLaw conversion */
extern void unsignedtoulaw(char *,int);

/* Parameter extraction helper */
extern CHAR  *MD_GetAtom(const CHAR*, const CHAR*, BOOL);

/* Internal software mixer stuff */
extern void VC_SetupPointers(void);
extern int  VC1_Init(void);
extern int  VC2_Init(void);

#if (MIKMOD_UNIX)
/* POSIX helper functions */
extern BOOL MD_Access(const CHAR *);
extern int  MD_DropPrivileges(void);
#endif

/* Macro to define a missing driver, yet allowing binaries to dynamically link
   with the library without missing symbol errors */
#define MISSING(a) MDRIVER a = { NULL, NULL, NULL, 0, 0 }

/*========== Prototypes for non-MT safe versions of some public functions */

extern void _mm_registerdriver(struct MDRIVER*);
extern void _mm_registerloader(struct MLOADER*);
extern BOOL MikMod_Active_internal(void);
extern void MikMod_DisableOutput_internal(void);
extern int  MikMod_EnableOutput_internal(void);
extern void MikMod_Exit_internal(void);
extern int  MikMod_SetNumVoices_internal(int,int);
extern void Player_Exit_internal(MODULE*);
extern void Player_Stop_internal(void);
extern BOOL Player_Paused_internal(void);
extern void Sample_Free_internal(SAMPLE*);
extern void Voice_Play_internal(SBYTE,SAMPLE*,ULONG);
extern void Voice_SetFrequency_internal(SBYTE,ULONG);
extern void Voice_SetPanning_internal(SBYTE,ULONG);
extern void Voice_SetVolume_internal(SBYTE,UWORD);
extern void Voice_Stop_internal(SBYTE);
extern BOOL Voice_Stopped_internal(SBYTE);

extern int   VC1_PlayStart(void);
extern int   VC2_PlayStart(void);
extern void  VC1_PlayStop(void);
extern void  VC2_PlayStop(void);
extern int  VC1_SetNumVoices(void);
extern int  VC2_SetNumVoices(void);

extern MikMod_callback_t vc_callback;

#ifdef __cplusplus
}
#endif

/*========== SIMD mixing routines */
#undef HAVE_ALTIVEC
#undef HAVE_SSE2
#if defined(MIKMOD_SIMD)

#if (defined(__ppc__) || defined(__ppc64__)) && defined(__VEC__) && !(defined(__GNUC__) && (__GNUC__ < 3))
#define HAVE_ALTIVEC

#elif defined(__GNUC__) && defined(__SSE2__) /* x86 / x86_64 */
#define HAVE_SSE2

#elif defined(_MSC_VER) && (_MSC_VER >= 1300) && (defined(_M_IX86) || defined(_M_AMD64))
/* FIXME: emmintrin.h requires VC6 processor pack or VC2003+ */
#define HAVE_SSE2
/* avoid some warnings */
#pragma warning(disable:4761)
#pragma warning(disable:4391)
#pragma warning(disable:4244)

#endif /* AltiVec/SSE2 */
#endif /* MIKMOD_SIMD */

/*========== SIMD mixing helper functions =============*/

#if defined(_WIN64)
# if defined(_MSC_VER)
#  define IS_ALIGNED_16(ptr) (!((__int64)(ptr) & 15i64))
# else /* GCC, LCC, .. */
#  define IS_ALIGNED_16(ptr) (!((long long)(ptr) & 15LL))
# endif
#else /* long cast should be OK for all else */
#define IS_ALIGNED_16(ptr) (!((long)(ptr) & 15L))
#endif

/* Altivec helper function */
#if defined HAVE_ALTIVEC

#define simd_m128i vector signed int
#define simd_m128 vector float

#ifdef __GNUC__
#include <ppc_intrinsics.h>
#endif

/* Helper functions */

/* Set single float across the four values */
static inline vector float vec_mul(const vector float a, const vector float b) {
    return vec_madd(a, b, (const vector float)(0.f));
}

/* Set single float across the four values */
static inline vector float vec_load_ps1(const float *pF) {
    vector float data = vec_lde(0, pF);
    return vec_splat(vec_perm(data, data, vec_lvsl(0, pF)), 0);
}

/* Set vector to 0 */
static inline vector float vec_setzero() {
    return (vector float) (0.);
}

static inline vector signed char vec_set1_8(unsigned char splatchar) {
    vector unsigned char splatmap = vec_lvsl(0, &splatchar);
    vector unsigned char result = vec_lde(0, &splatchar);
    splatmap = vec_splat(splatmap, 0);
    return (vector signed char)vec_perm(result, result, splatmap);
}

#define PERM_A0 0x00,0x01,0x02,0x03
#define PERM_A1 0x04,0x05,0x06,0x07
#define PERM_A2 0x08,0x09,0x0A,0x0B
#define PERM_A3 0x0C,0x0D,0x0E,0x0F
#define PERM_B0 0x10,0x11,0x12,0x13
#define PERM_B1 0x14,0x15,0x16,0x17
#define PERM_B2 0x18,0x19,0x1A,0x1B
#define PERM_B3 0x1C,0x1D,0x1E,0x1F

/* Equivalent to _mm_unpacklo_epi32 */
static inline vector signed int vec_unpacklo(vector signed int a, vector signed int b) {
   return vec_perm(a, b, (vector unsigned char)(PERM_A0,PERM_A1,PERM_B0,PERM_B1));
}

/* Equivalent to _mm_srli_si128(a, 8) (without the zeroing in high part). */
static inline vector signed int vec_hiqq(vector signed int a) {
   vector signed int b = vec_splat_s32(0);
   return vec_perm(a, b, (vector unsigned char)(PERM_A2,PERM_A3,PERM_B2,PERM_B3));
}

/* vec_sra is max +15. We have to do in two times ... */
#define EXTRACT_SAMPLE_SIMD_F(srce, var, size, mul) var = vec_mul(vec_ctf(vec_sra(vec_ld(0, (vector signed int const *)(srce)), vec_splat_u32(BITSHIFT-size)),0), mul);
#define EXTRACT_SAMPLE_SIMD_0(srce, var) var = vec_sra(vec_sra(vec_ld(0, (vector signed int const *)(srce)), vec_splat_u32(15)), vec_splat_u32(BITSHIFT+16-15-0));
#define EXTRACT_SAMPLE_SIMD_8(srce, var) var = vec_sra(vec_sra(vec_ld(0, (vector signed int const *)(srce)), vec_splat_u32(15)), vec_splat_u32(BITSHIFT+16-15-8));
#define EXTRACT_SAMPLE_SIMD_16(srce, var) var = vec_sra(vec_ld(0, (vector signed int const *)(srce)), vec_splat_u32(BITSHIFT+16-16));
#define PUT_SAMPLE_SIMD_W(dste, v1, v2)  vec_st(vec_packs(v1, v2), 0, dste);
#define PUT_SAMPLE_SIMD_B(dste, v1, v2, v3, v4)  vec_st(vec_add(vec_packs((vector signed short)vec_packs(v1, v2), (vector signed short)vec_packs(v3, v4)), vec_set1_8(128)), 0, dste);
#define PUT_SAMPLE_SIMD_F(dste, v1)  vec_st(v1, 0, dste);
#define LOAD_PS1_SIMD(ptr) vec_load_ps1(ptr)

#elif defined HAVE_SSE2

#include <emmintrin.h>

/* SSE2 helper function */

static __inline __m128i mm_hiqq(const __m128i a) {
   return _mm_srli_si128(a, 8); /* get the 64bit upper part. new 64bit upper is undefined (zeroed is fine). */
}

/* 128-bit mixing macros */
#define EXTRACT_SAMPLE_SIMD(srce, var, size) var = _mm_srai_epi32(_mm_load_si128((__m128i const *)(srce)), BITSHIFT+16-size);
#define EXTRACT_SAMPLE_SIMD_F(srce, var, size, mul) var = _mm_mul_ps(_mm_cvtepi32_ps(_mm_srai_epi32(_mm_load_si128((__m128i const *)(srce)), BITSHIFT-size)), mul);
#define EXTRACT_SAMPLE_SIMD_0(srce, var) EXTRACT_SAMPLE_SIMD(srce, var, 0)
#define EXTRACT_SAMPLE_SIMD_8(srce, var) EXTRACT_SAMPLE_SIMD(srce, var, 8)
#define EXTRACT_SAMPLE_SIMD_16(srce, var) EXTRACT_SAMPLE_SIMD(srce, var, 16)
#define PUT_SAMPLE_SIMD_W(dste, v1, v2)  _mm_store_si128((__m128i*)(dste), _mm_packs_epi32(v1, v2));
#define PUT_SAMPLE_SIMD_B(dste, v1, v2, v3, v4)  _mm_store_si128((__m128i*)(dste), _mm_add_epi8(_mm_packs_epi16(_mm_packs_epi32(v1, v2), _mm_packs_epi32(v3, v4)), _mm_set1_epi8(128)));
#define PUT_SAMPLE_SIMD_F(dste, v1)  _mm_store_ps((float*)(dste), v1);
#define LOAD_PS1_SIMD(ptr) _mm_load_ps1(ptr)
#define simd_m128i __m128i
#define simd_m128 __m128

#endif

#if defined(HAVE_SSE2) || defined(HAVE_ALTIVEC)
/* MikMod_amalloc() returns a 16 byte aligned zero-filled
   memory in SIMD-enabled builds.
 - the returned memory can be freed with MikMod_afree()
 - the returned memory CAN NOT be realloc()'ed safely.  */
#ifdef __cplusplus
extern "C" {
#endif
void* MikMod_amalloc(size_t);
void MikMod_afree(void *);  /* frees if ptr != NULL */
#ifdef __cplusplus
}
#endif

#else /* NO SIMD */
#define MikMod_amalloc MikMod_malloc
#define MikMod_afree MikMod_free
#endif

#endif /* _MIKMOD_INTERNALS_H */

/* ex:set ts=4: */
