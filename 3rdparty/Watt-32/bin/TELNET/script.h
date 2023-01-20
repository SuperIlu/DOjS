/*
 * Written by Archie Cobbs <archie@whistle.com>
 * Copyright (c) 1995-1997 Whistle Communications, Inc. All rights reserved.
 */

#ifndef __SCRIPT_H
#define __SCRIPT_H

/* Bounds */

#define CHAT_MAX_NAME       32
#define CHAT_MAX_LABEL      32
#define CHAT_MAX_MATCHES    32
#define CHAT_MAX_TIMERS     32
#define CHAT_MAX_PATTERN    32
#define CHAT_NUM_VARIABLES  27
#define CHAT_READBUF_SIZE   48
#define CHAT_MAX_ARGS       10
#define CHAT_MAX_VARNAME    100

/* Structures */

struct chatvar {
       char  *name;
       char  *value;
       struct chatvar *next;
     };

struct chatmatch {
       char   set  [CHAT_MAX_NAME];
       char   pat  [CHAT_MAX_PATTERN];
       char   label[CHAT_MAX_LABEL];
       int    n_matched;
       struct chatmatch *next;
     };

struct chattimer {
       char   set  [CHAT_MAX_NAME];
       char   label[CHAT_MAX_LABEL];
       struct chatinfo *c;
       struct chattimer *next;
     };

struct chatframe {
       fpos_t posn;
       int    line_num;
       struct chatframe *up;
     };

struct chatinfo {
       FILE  *fp;
       int    state;
       void (*result) (int reason);
       char  *lastLog;
       char   readBuf[CHAT_READBUF_SIZE];
       int    readBufLen;
       int    dry_run;
       struct chattimer *timers;
       struct chatmatch *matches;
       struct chatframe *stack;
       struct chatvar   *globals;
       struct chatvar   *temps;
     };

void ChatPresetVar (struct chatinfo *, char *name, char *value);
void ChatCallback  (int stat);
int  ChatMain      (const char *script);
int  ChatStart     (struct chatinfo *, const char *, void (*)(int));
int  ChatInput     (struct chatinfo *);

#endif
